#ifndef _RTL_TRES_H_
#define _RTL_TRES_H_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif

#ifdef __cplusplus
extern "C" {
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



 /* forward declaration and type definition */
struct _rtl_TestResult;
typedef struct _rtl_TestResult rtl_TestResult;


typedef void* rtl_funcstate;
typedef void* rtl_cmpstate;

 /* type definitions of function pointers wich can be overloaded */
typedef sal_Bool (SAL_CALL *rtl_tres_state_ptr)(
                                                rtl_TestResult*,
                                                sal_Bool,
                                                const sal_Char*,
                                                const sal_Char*,
                                                sal_Bool
                                                );

typedef void (SAL_CALL *rtl_tres_end_ptr)( rtl_TestResult*, const sal_Char* );
typedef sal_Bool (SAL_CALL *rtl_tres_ispassed_ptr)( rtl_TestResult* );
typedef sal_Bool (SAL_CALL *rtl_tres_isok_ptr)( rtl_TestResult* );
typedef rtl_funcstate (SAL_CALL *rtl_tres_funcstate_ptr)( rtl_TestResult* );
typedef sal_Bool (SAL_CALL *rtl_tres_isbit_ptr)( rtl_TestResult*,
                                                                sal_uInt32 );

typedef rtl_funcstate (SAL_CALL *rtl_tres_getnextfuncstate_ptr)
                                                            ( rtl_funcstate );
typedef rtl_funcstate (SAL_CALL *rtl_tres_getprevfuncstate_ptr)
                                                            ( rtl_funcstate );
typedef sal_uInt32 (SAL_CALL *rtl_tres_getflags_ptr)( rtl_funcstate );
typedef rtl_String* (SAL_CALL *rtl_tres_getname_ptr)( rtl_funcstate );
typedef sal_uInt32 (SAL_CALL *rtl_tres_getstart_ptr)( rtl_funcstate );
typedef sal_uInt32 (SAL_CALL *rtl_tres_getstop_ptr)( rtl_funcstate );
typedef rtl_cmpstate (SAL_CALL *rtl_tres_getcmpstate_ptr)( rtl_funcstate );

typedef sal_Bool (SAL_CALL *rtl_tres_getstat_ptr)( rtl_cmpstate );
typedef rtl_String* (SAL_CALL *rtl_tres_getmsg_ptr)( rtl_cmpstate );
typedef rtl_cmpstate (SAL_CALL *rtl_tres_getnextcmpstate_ptr)( rtl_cmpstate );

 /* type definition of vtable structure for testresult */
typedef struct _rtl_TestResult_vtable
{
    sal_uInt32                      vtablesize;
    rtl_tres_state_ptr              state_;
    rtl_tres_end_ptr                end_;
    rtl_tres_ispassed_ptr           ispassed;
    rtl_tres_isok_ptr               isok;
    rtl_tres_funcstate_ptr          funcstate;
    rtl_tres_isbit_ptr              isbit;
    rtl_tres_getnextfuncstate_ptr   nextfuncstate;
    rtl_tres_getprevfuncstate_ptr   prevfuncstate;
    rtl_tres_getflags_ptr           flags;
    rtl_tres_getname_ptr            name;
    rtl_tres_getstart_ptr           start;
    rtl_tres_getstop_ptr            stop;
    rtl_tres_getcmpstate_ptr        cmpstate;
    rtl_tres_getstat_ptr            stat;
    rtl_tres_getmsg_ptr             msg;
    rtl_tres_getnextcmpstate_ptr    nextcmpstate;

} rtl_TestResult_vtable;

 /* type definition of testresult structure */
struct _rtl_TestResult
{
    rtl_TestResult_vtable * pFuncs;
    void *                  pExternalData;

};


 /* exports */
rtl_TestResult* SAL_CALL rtl_tres_create( const sal_Char* meth, sal_uInt32 flags );
void SAL_CALL rtl_tres_destroy( rtl_TestResult* res );

#ifdef __cplusplus
}
#endif

#endif  /* _RTL_TRES_H_ */
