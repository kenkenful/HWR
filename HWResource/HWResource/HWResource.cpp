#include <windows.h>
#include <memory>
#include <iostream>

#define CmResourceTypeNull                0   // ResType_All or ResType_None (0x0000)
#define CmResourceTypePort                1   // ResType_IO (0x0002)
#define CmResourceTypeInterrupt           2   // ResType_IRQ (0x0004)
#define CmResourceTypeMemory              3   // ResType_Mem (0x0001)
#define CmResourceTypeDma                 4   // ResType_DMA (0x0003)
#define CmResourceTypeDeviceSpecific      5   // ResType_ClassSpecific (0xFFFF)
#define CmResourceTypeBusNumber           6   // ResType_BusNumber (0x0006)
#define CmResourceTypeMemoryLarge         7   // ResType_MemLarge (0x0007)

#define CmResourceTypeNonArbitrated     128   // Not arbitrated if 0x80 bit set
#define CmResourceTypeConfigData        128   // ResType_Reserved (0x8000)
#define CmResourceTypeDevicePrivate     129   // ResType_DevicePrivate (0x8001)
#define CmResourceTypePcCardConfig      130   // ResType_PcCardConfig (0x8002)
#define CmResourceTypeMfCardConfig      131   // ResType_MfCardConfig (0x8003)
#define CmResourceTypeConnection        132   // ResType_Connection (0x8004)

typedef enum _INTERFACE_TYPE {
    InterfaceTypeUndefined = -1,
    Internal,
    Isa,
    Eisa,
    MicroChannel,
    TurboChannel,
    PCIBus,
    VMEBus,
    NuBus,
    PCMCIABus,
    CBus,
    MPIBus,
    MPSABus,
    ProcessorInternal,
    InternalPowerBus,
    PNPISABus,
    PNPBus,
    Vmcs,
    ACPIBus,
    MaximumInterfaceType
}INTERFACE_TYPE, * PINTERFACE_TYPE;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, * PPHYSICAL_ADDRESS;


#include "pshpack4.h"
typedef struct _CM_PARTIAL_RESOURCE_DESCRIPTOR {
    UCHAR Type;
    UCHAR ShareDisposition;
    USHORT Flags;
    union {

        //
        // Range of resources, inclusive.  These are physical, bus relative.
        // It is known that Port and Memory below have the exact same layout
        // as Generic.
        //

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Generic;

        //
        //

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length;
        } Port;

        //
        //

        struct {
#if defined(NT_PROCESSOR_GROUPS)
            USHORT Level;
            USHORT Group;
#else
            ULONG Level;
#endif
            ULONG Vector;
            KAFFINITY Affinity;
        } Interrupt;

        //
        // Values for message signaled interrupts are distinct in the
        // raw and translated cases.
        //

        struct {
            union {
                struct {
#if defined(NT_PROCESSOR_GROUPS)
                    USHORT Group;
#else
                    USHORT Reserved;
#endif
                    USHORT MessageCount;
                    ULONG Vector;
                    KAFFINITY Affinity;
                } Raw;

                struct {
#if defined(NT_PROCESSOR_GROUPS)
                    USHORT Level;
                    USHORT Group;
#else
                    ULONG Level;
#endif
                    ULONG Vector;
                    KAFFINITY Affinity;
                } Translated;
            } DUMMYUNIONNAME;
        } MessageInterrupt;

        //
        // Range of memory addresses, inclusive. These are physical, bus
        // relative. The value should be the same as the one passed to
        // HalTranslateBusAddress().
        //

        struct {
            PHYSICAL_ADDRESS Start;    // 64 bit physical addresses.
            ULONG Length;
        } Memory;

        //
        // Physical DMA channel.
        //

        struct {
            ULONG Channel;
            ULONG Port;
            ULONG Reserved1;
        } Dma;

        struct {
            ULONG Channel;
            ULONG RequestLine;
            UCHAR TransferWidth;
            UCHAR Reserved1;
            UCHAR Reserved2;
            UCHAR Reserved3;
        } DmaV3;

        //
        // Device driver private data, usually used to help it figure
        // what the resource assignments decisions that were made.
        //

        struct {
            ULONG Data[3];
        } DevicePrivate;

        //
        // Bus Number information.
        //

        struct {
            ULONG Start;
            ULONG Length;
            ULONG Reserved;
        } BusNumber;

        //
        // Device Specific information defined by the driver.
        // The DataSize field indicates the size of the data in bytes. The
        // data is located immediately after the DeviceSpecificData field in
        // the structure.
        //

        struct {
            ULONG DataSize;
            ULONG Reserved1;
            ULONG Reserved2;
        } DeviceSpecificData;

        // The following structures provide support for memory-mapped
        // IO resources greater than MAXULONG
        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length40;
        } Memory40;

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length48;
        } Memory48;

        struct {
            PHYSICAL_ADDRESS Start;
            ULONG Length64;
        } Memory64;

        struct {
            UCHAR Class;
            UCHAR Type;
            UCHAR Reserved1;
            UCHAR Reserved2;
            ULONG IdLowPart;
            ULONG IdHighPart;
        } Connection;

    } u;
} CM_PARTIAL_RESOURCE_DESCRIPTOR, * PCM_PARTIAL_RESOURCE_DESCRIPTOR;
#include "poppack.h"



//
// A Partial Resource List is what can be found in the ARC firmware
// or will be generated by ntdetect.com.
// The configuration manager will transform this structure into a Full
// resource descriptor when it is about to store it in the regsitry.
//
// Note: There must a be a convention to the order of fields of same type,
// (defined on a device by device basis) so that the fields can make sense
// to a driver (i.e. when multiple memory ranges are necessary).
//

typedef struct _CM_PARTIAL_RESOURCE_LIST {
    USHORT Version;
    USHORT Revision;
    ULONG Count;
    CM_PARTIAL_RESOURCE_DESCRIPTOR PartialDescriptors[1];
} CM_PARTIAL_RESOURCE_LIST, * PCM_PARTIAL_RESOURCE_LIST;

//
// A Full Resource Descriptor is what can be found in the registry.
// This is what will be returned to a driver when it queries the registry
// to get device information; it will be stored under a key in the hardware
// description tree.
//
// Note: There must a be a convention to the order of fields of same type,
// (defined on a device by device basis) so that the fields can make sense
// to a driver (i.e. when multiple memory ranges are necessary).
//

typedef struct _CM_FULL_RESOURCE_DESCRIPTOR {
    INTERFACE_TYPE InterfaceType; // unused for WDM
    ULONG BusNumber; // unused for WDM
    CM_PARTIAL_RESOURCE_LIST PartialResourceList;
} CM_FULL_RESOURCE_DESCRIPTOR, * PCM_FULL_RESOURCE_DESCRIPTOR;

//
// The Resource list is what will be stored by the drivers into the
// resource map via the IO API.
//

typedef struct _CM_RESOURCE_LIST {
    ULONG Count;
    CM_FULL_RESOURCE_DESCRIPTOR List[1];
} CM_RESOURCE_LIST, * PCM_RESOURCE_LIST;



#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

void QueryKey(HKEY hKey)
{
    CHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD    cbName = 0;                   // size of name string 
    CHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys = 0;               // number of subkeys 
    DWORD    cbMaxSubKey = 0;              // longest subkey size 
    DWORD    cchMaxClass = 0;              // longest class string 
    DWORD    cValues = 0;              // number of values for key 
    DWORD    cchMaxValue = 0;          // longest value name 
    DWORD    cbMaxValueData = 0;       // longest value data 
    DWORD    cbSecurityDescriptor = 0; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i = 0, j = 0, retCode = 0;

    CHAR  achValue[MAX_VALUE_NAME] = { '\0' };
    DWORD cchValue = MAX_VALUE_NAME;

    //CHAR DeviceValue[MAX_PATH];
    DWORD dwType;
    CHAR szbuf[512];
    DWORD dwSize = sizeof(szbuf);

    // Get the class name and the value count. 
    retCode = ::RegQueryInfoKey(
        hKey,                    // key handle 
        achClass,                // buffer for class name 
        &cchClassName,           // size of class string 
        NULL,                    // reserved 
        &cSubKeys,               // number of subkeys 
        &cbMaxSubKey,            // longest subkey size 
        &cchMaxClass,            // longest class string 
        &cValues,                // number of values for this key 
        &cchMaxValue,            // longest value name 
        &cbMaxValueData,         // longest value data 
        &cbSecurityDescriptor,   // security descriptor 
        &ftLastWriteTime);       // last write time 


    // Enumerate the subkeys, until RegEnumKeyEx fails.

    if (cSubKeys)
    {
        printf("\nNumber of subkeys: %d\n", cSubKeys);

        for (i = 0; i < cSubKeys; i++)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = ::RegEnumKeyEx(hKey, i,
                achKey,
                &cbName,
                NULL,
                NULL,
                NULL,
                &ftLastWriteTime);

            if (retCode == ERROR_SUCCESS)
            {
                printf(TEXT("(%d) %s\n"), i + 1, achKey);
            }
        }
    }

    // Enumerate the key values. 

    if (cValues)
    {
        printf("\nNumber of values: %d\n", cValues);

        for (i = 0; i < cValues; i++)
        {
            cchValue = MAX_VALUE_NAME;
            achValue[0] = '\0';
            retCode = ::RegEnumValue(hKey, i,
                achValue,
                &cchValue,
                NULL,
                NULL,
                NULL,
                NULL);

            if (retCode == ERROR_SUCCESS)
            {
                //   printf(TEXT("(%d) %s\n"), i + 1, achValue);

                if (/* strstr(achValue, "\\Device\\NTPNP_PCI") != NULL &&*/ strstr(achValue, "Translated") != NULL) {

                    std::cout << "pci" << std::endl;
                    memset(szbuf, 0, sizeof(szbuf));
                    retCode = ::RegQueryValueEx(hKey, achValue, 0, &dwType, (LPBYTE)szbuf, &dwSize);

                    if (retCode == ERROR_SUCCESS) {
                        CM_RESOURCE_LIST* pResourceList = (CM_RESOURCE_LIST*)szbuf;

                        for (ULONG i = 0; i < pResourceList->Count; ++i)
                        {
                            CM_FULL_RESOURCE_DESCRIPTOR* pFullDescriptor = &pResourceList->List[i];
                            //if (pFullDescriptor->BusNumber == 0x40 || pFullDescriptor->BusNumber == 0x41) {

                            printf("BusNumber: %x\n", pFullDescriptor->BusNumber);
                            for (ULONG j = 0; j < pFullDescriptor->PartialResourceList.Count; ++j)
                            {
                                CM_PARTIAL_RESOURCE_DESCRIPTOR* pPartialDescriptor = &pFullDescriptor->PartialResourceList.PartialDescriptors[j];

                                switch (pPartialDescriptor->Type)
                                {
                                case CmResourceTypeMemory:
                                    //  printf("CmResourceTypeMemory \n");

                                    break;
                                case CmResourceTypePort:
                                    // printf("CmResourceTypePort \n");

                                    break;
                                case CmResourceTypeInterrupt:
                                    printf("CmResourceTypeInterrupt \n");
                                    printf("Interrupt Level: %x\n", (UCHAR)pPartialDescriptor->u.MessageInterrupt.Translated.Level);
                                    printf("Interrupt Vector: %x\n", (UCHAR)pPartialDescriptor->u.MessageInterrupt.Translated.Vector);
                                    printf("Interrupt Affnity: %x\n", (UCHAR)pPartialDescriptor->u.MessageInterrupt.Translated.Affinity);
                                    printf("flags : %x\n", (UCHAR)pPartialDescriptor->Flags);

                                    break;
                                default:
                                    break;
                                }

                                //}
                            }
                        }

                    }


                }
                else {
                    printf("Error   \n");
                }
            }
        }
    }
}


int Error(DWORD code = GetLastError()) {
    printf("Error: %d\n", ::GetLastError());
    return 1;
}

int wmain(int argc, const wchar_t* argv[]) {



    HKEY hKey = NULL;
    const char* pszSubKey = "Hardware\\ResourceMap\\PnP Manager\\PnPManager";
    const char* pszValueName = "\\Device\\NTPNP_PCI0023.Translated";

    LONG result;
    DWORD i;

    // result = RegOpenKey(HKEY_LOCAL_MACHINE, pszSubKey, &hKey);
    result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszSubKey, 0, KEY_READ | KEY_QUERY_VALUE, &hKey);

    if (result != ERROR_SUCCESS) {
        std::cout << "RegOpenKey failed" << std::endl;
        getchar();
        return 0;
    }

    QueryKey(hKey);

    RegCloseKey(hKey);

}