#ifndef _RTL_TRES_H_
#define _RTL_TRES_H_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

 /* comandline flags */
#define rtl_tres_Flag_BOOM      0x00000001
#define rtl_tres_Flag_VERBOSE   0x00000002
#define rtl_tres_Flag_SKIP      0x00000004
#define rtl_tres_Flag_LOG       0x00000010
#define rtl_tres_Flag_HIS       0x00000100
#define rtl_tres_Flag_TIME      0x00000200
#define rtl_tres_Flag_MSG       0x00000400

 /* state flags */
#define rtl_tres_Flag_SUB       0x01000000
#define rtl_tres_Flag_PASSED    0x10000000
#define rtl_tres_Flag_OK        0x20000000

#ifdef __cplusplus
extern "C" {
#endif

typedef void* rtlTestResult;

rtlTestResult SAL_CALL rtl_tres_create
                                (
                                    const sal_Char* meth,
                                    sal_uInt32 flags
                                );

sal_Bool SAL_CALL rtl_tres_state
                            (
                                rtlTestResult res,
                                sal_Bool bSuccess,
                                sal_Char* pTestMsg,
                                sal_Char* pFuncName
                            );

sal_Bool SAL_CALL rtl_tres_destroy( rtlTestResult res );

#ifdef __cplusplus
}
#endif

#endif
