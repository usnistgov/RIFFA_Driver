#if !defined(_RIFFA_H_)
#define _RIFFA_H_

// Adjusts register offsets for each channel
#define CHNL_REG(c, o) (((c)<<4) + o)

// Register offsets
#define RIFFA_RX_SG_LEN_REG			0x0
#define RIFFA_RX_SG_ADDR_LO_REG		0x1
#define RIFFA_RX_SG_ADDR_HI_REG		0x2
#define RIFFA_RX_LEN_REG			0x3
#define RIFFA_RX_OFFLAST_REG		0x4
#define RIFFA_TX_SG_LEN_REG			0x5
#define RIFFA_TX_SG_ADDR_LO_REG		0x6
#define RIFFA_TX_SG_ADDR_HI_REG		0x7
#define RIFFA_TX_LEN_REG			0x8
#define RIFFA_TX_OFFLAST_REG		0x9
#define RIFFA_INFO_REG				0xA
#define RIFFA_IRQ_0_REG				0xB
#define RIFFA_IRQ_1_REG				0xC
#define RIFFA_RX_TNFR_LEN_REG		0xD
#define RIFFA_TX_TNFR_LEN_REG		0xE

// Size of common buffer for scatter gather elements
#define RIFFA_MIN_SG_BUF_SIZE (4*1024)

// Size of common buffer for receive data spill
#define RIFFA_SPILL_BUF_SIZE (4*1024)

// Maximum number of scatter gather elements for each transfer
#define RIFFA_MIN_NUM_SG_ELEMS (200)

// Maximum bus width (multiply by 32 to get bit width)
#define RIFFA_MAX_BUS_WIDTH_PARAM (4)

// Maximum DMA transfer size (in bytes).
#define RIFFA_MAX_TNFR_LEN (0xFFFFFFFF)

// Number of DMA channels supported
#define RIFFA_MAX_NUM_CHNLS (12)

// Maximum number of RIFFA FPGAs
#define RIFFA_MAX_NUM_FPGAS (5)

// Error codes
#define EVT_EXCEPTION_HANDLED_MESSAGE    ((NTSTATUS)0x802A0002L)

// The structure used to hold data transfer information
typedef struct RIFFA_FPGA_CHNL_IO {
	UINT64					Id;
	UINT64					Chnl;
	UINT64					Length;
	UINT32					Offset;
	UINT32					Last;
	UINT64					Timeout;
} RIFFA_FPGA_CHNL_IO, * PRIFFA_FPGA_CHNL_IO;

// The structure used to hold FPGA information
typedef struct RIFFA_FPGA_INFO {
	UINT64					num_fpgas;
	UINT64					id[RIFFA_MAX_NUM_FPGAS];
	UINT64					num_chnls[RIFFA_MAX_NUM_FPGAS];
	CHAR					name[RIFFA_MAX_NUM_FPGAS][16];
	UINT64					vendor_id[RIFFA_MAX_NUM_FPGAS];
	UINT32					device_id[RIFFA_MAX_NUM_FPGAS];
} RIFFA_FPGA_INFO, * PRIFFA_FPGA_INFO;

// Struct for holding DMA transaction state
typedef struct CHNL_DIR_STATE {
	LONG					Ready;
	LONG					InUse;
	LONG					ReqdDone;
	UINT64					Length;
	UINT32					Offset;
	UINT32					Last;
	UINT64					Timeout;
	UINT64					Capacity;
	UINT64					Provided;
	UINT64					ProvidedPrev;
	UINT64					Confirmed;
	UINT64					ConfirmedPrev;
	UINT64					SpillAfter;
	PMDL					Mdl;
	PSCATTER_GATHER_LIST	SgList;
	UINT64					SgPos;
	WDFSPINLOCK				SpinLock;
	WDFTIMER				Timer;
	WDFQUEUE 				PendingQueue;
	WDFDMATRANSACTION 		DmaTransaction;
	WDFCOMMONBUFFER 		CommonBuffer;
	PULONG 					CommonBufferBase;
	PHYSICAL_ADDRESS 		CommonBufferBaseLA;	// Logical Address
	BOOLEAN                 DmaTransactionValid;
} CHNL_DIR_STATE, *PCHNL_DIR_STATE;

// Struct for holding interrupt signals and data
typedef struct INTR_CHNL_DIR_DATA {
	BOOLEAN 				NewTxn;
	UINT32 					OffLast;
	UINT32 					Length;
	BOOLEAN 				SgRead;
	BOOLEAN 				Done;
} INTR_CHNL_DIR_DATA, *PINTR_CHNL_DIR_DATA;

// The device extension for the device object
typedef struct _DEVICE_EXTENSION {
	WDFDEVICE 				Device;
	PDRIVER_OBJECT          DriverObject;
	PULONG 					Bar0;
	UINT64 					Bar0Length;
	UINT64					MaxNumScatterGatherElems;
	WDFINTERRUPT 			Interrupt;
	WDFDMAENABLER 			DmaEnabler;
	WDFQUEUE 				IoctlQueue;
	UINT64 					NumChnls;
	UINT32 					VendorId;
	UINT32 					DeviceId;
	CHAR 					Name[16];
	INTR_CHNL_DIR_DATA		IntrData[2 * RIFFA_MAX_NUM_CHNLS];	// Send is the first bank,
	CHNL_DIR_STATE			Chnl[(2*RIFFA_MAX_NUM_CHNLS)];		// recv is the second bank
	WDFCOMMONBUFFER 		SpillBuffer;
	PUCHAR 					SpillBufferBase;
	PHYSICAL_ADDRESS 		SpillBufferBaseLA;	// Logical Address
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// The request extension for the request object
typedef struct _REQUEST_EXTENSION {
	UINT64 					Chnl;
} REQUEST_EXTENSION, *PREQUEST_EXTENSION;

// The timer extension for the timer object
typedef struct _TIMER_EXTENSION {
	UINT64 					Chnl;
} TIMER_EXTENSION, *PTIMER_EXTENSION;

// This will generate the function named RiffaGetDeviceContext to be used for
// retreiving the DEVICE_EXTENSION pointer.
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_EXTENSION, RiffaGetDeviceContext)

// This will generate the function named RiffaGetTimerContext to be used for
// retreiving the TIMER_EXTENSION pointer.
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(TIMER_EXTENSION, RiffaGetTimerContext)

// This will generate the function named RiffaGetRequestContext to be used for
// retreiving the REQUEST_EXTENSION pointer.
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_EXTENSION, RiffaGetRequestContext)


// Function prototypes
DRIVER_INITIALIZE DriverEntry;

EVT_WDF_DRIVER_DEVICE_ADD RiffaEvtDeviceAdd;
EVT_WDF_OBJECT_CONTEXT_CLEANUP RiffaEvtDriverContextCleanup;

EVT_WDF_DEVICE_PREPARE_HARDWARE RiffaEvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE RiffaEvtDeviceReleaseHardware;
NTSTATUS RiffaReadHardwareIds(IN PDEVICE_EXTENSION DevExt);

EVT_WDF_INTERRUPT_ISR RiffaEvtInterruptIsr;
EVT_WDF_INTERRUPT_DPC RiffaEvtInterruptDpc;
BOOLEAN RiffaProcessInterrupt(IN PDEVICE_EXTENSION DevExt, IN UINT32 Offset, IN UINT32 Vect);

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL  RiffaEvtIoDeviceControl;
VOID RiffaIoctlSend(IN PDEVICE_EXTENSION DevExt, IN WDFREQUEST Request,
	IN size_t OutputBufferLength, IN size_t InputBufferLength);
VOID RiffaIoctlRecv(IN PDEVICE_EXTENSION DevExt, IN WDFREQUEST Request,
	IN size_t OutputBufferLength, IN size_t InputBufferLength);
VOID RiffaIoctlList(IN PDEVICE_EXTENSION DevExt, IN WDFREQUEST Request,
	IN size_t OutputBufferLength, IN size_t InputBufferLength);
VOID RiffaIoctlReset(IN PDEVICE_EXTENSION DevExt, IN WDFREQUEST Request);

VOID RiffaCompleteRequest(IN PDEVICE_EXTENSION DevExt, IN UINT64 Chnl, IN NTSTATUS Status, IN BOOLEAN TimedOut);
EVT_WDF_TIMER RiffaEvtTimerFunc;

VOID RiffaStartRecvTransaction(IN PDEVICE_EXTENSION DevExt, IN UINT64 Chnl);
NTSTATUS RiffaStartDmaTransaction(IN PDEVICE_EXTENSION DevExt, IN UINT64 Chnl,
	IN UINT64 Length, IN UINT32 Offset, IN WDF_DMA_DIRECTION DmaDirection);
VOID RiffaProgramScatterGather(IN PDEVICE_EXTENSION DevExt, IN UINT64 Chnl);
VOID RiffaTransactionComplete(IN PDEVICE_EXTENSION DevExt, IN UINT64 Chnl,
	IN UINT64 Transferred, IN NTSTATUS Status);
VOID RiffaProgramSend(IN PDEVICE_EXTENSION DevExt, IN UINT64 Chnl, IN UINT64 Length,
	IN UINT32 Offset, IN UINT32 Last);
EVT_WDF_PROGRAM_DMA RiffaEvtProgramDma;

BOOLEAN RiffaLogError(
	_In_							 PDRIVER_OBJECT Device,
	_In_                             NTSTATUS SpecificIOStatus,
	_In_                             ULONG LengthOfInsert1,
	_In_reads_bytes_opt_(LengthOfInsert1) PWCHAR Insert1,
	_In_                             ULONG LengthOfInsert2,
	_In_reads_bytes_opt_(LengthOfInsert2) PWCHAR Insert2);
#pragma alloc_text(PAGE, RiffaLogError)

#pragma warning(disable:4127) // avoid conditional expression is constant error with W4

#endif

