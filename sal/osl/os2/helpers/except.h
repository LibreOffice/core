
/*
 *@@sourcefile except.h:
 *      header file for except.c. See remarks there.
 *
 *      Note: Version numbering in this file relates to XWorkplace version
 *            numbering.
 *
 *@@include #define INCL_DOSEXCEPTIONS
 *@@include #define INCL_DOSPROCESS
 *@@include #include <os2.h>
 *@@include #include <stdio.h>
 *@@include #include <setjmp.h>
 *@@include #include "helpers\except.h"
 */

/*
 *      Copyright (C) 1999-2000 Ulrich M”ller.
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, in version 2 as it comes in the COPYING
 *      file of the XFolder main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#if __cplusplus
extern "C" {
#endif

#ifndef EXCEPT_HEADER_INCLUDED
    #define EXCEPT_HEADER_INCLUDED

    #if defined __IBMCPP__ || defined __IBMC__
        #ifndef INCL_DOSEXCEPTIONS
            #error except.h requires INCL_DOSEXCEPTIONS to be defined.
        #endif
        #ifndef INCL_DOSPROCESS
            #error except.h requires INCL_DOSPROCESS to be defined.
        #endif

        #ifndef __stdio_h
            #error except.h requires stdio.h to be included.
        #endif
        #ifndef __setjmp_h
            #error except.h requires setjmp.h to be included.
        #endif
    #endif

    /********************************************************************
     *
     *   Declarations
     *
     ********************************************************************/

    // forward declaration
    typedef struct _EXCEPTIONREGISTRATIONRECORD2 *PEXCEPTIONREGISTRATIONRECORD2;

    // "OnKill" function prototype for EXCEPTIONREGISTRATIONRECORD2
    // added V0.9.0 (99-10-22) [umoeller]
    // removed V0.9.7 (2000-12-08) [umoeller]
    // typedef VOID APIENTRY FNEXCONKILL(PEXCEPTIONREGISTRATIONRECORD2);
    // typedef FNEXCONKILL *PFNEXCONKILL;

    /*
     *@@ EXCEPTIONREGISTRATIONRECORD2:
     *      replacement EXCEPTIONREGISTRATIONRECORD
     *      struct for thread exception handling.
     *
     *@@changed V0.9.0 (99-10-22) [umoeller]: pfnOnKill added
     *@@changed V0.9.0 (99-10-22) [umoeller]: renamed from REGREC2
     */

    typedef struct _EXCEPTIONREGISTRATIONRECORD2
    {
        PVOID           pNext;              // as in EXCEPTIONREGISTRATIONRECORD
        PFN             pfnHandler;         // as in EXCEPTIONREGISTRATIONRECORD
        jmp_buf         jmpThread;          // additional buffer for setjmp
        EXCEPTIONREPORTRECORD err;          // exception handlers copy the report rec here
        PVOID           pvUser;             // user ptr
    } EXCEPTIONREGISTRATIONRECORD2;

    /*
     *@@ EXCEPTSTRUCT:
     *      structure used with TRY_xxx macros.
     */

    typedef struct _EXCEPTSTRUCT
    {
        EXCEPTIONREGISTRATIONRECORD2 RegRec2;
        ULONG                 ulExcpt;  // != NULL if exception caught
        APIRET                arc;      // rc of DosSetExceptionHandler
    } EXCEPTSTRUCT, *PEXCEPTSTRUCT;

    // function prototypes for exception hooks (V0.9.0)

    // "open traplog file" hook
    typedef FILE* APIENTRY FNEXCOPENFILE(VOID);
    typedef FNEXCOPENFILE *PFNEXCOPENFILE;

    // "exception" hook
    typedef VOID APIENTRY FNEXCHOOK(FILE*, PTIB, ULONG);  // V0.9.16 (2001-12-02) [pr]
    typedef FNEXCHOOK *PFNEXCHOOK;

    // "error" hook
    typedef VOID APIENTRY FNEXCHOOKERROR(const char *pcszFile,
                                         ULONG ulLine,
                                         const char *pcszFunction,
                                         APIRET arc);
    typedef FNEXCHOOKERROR *PFNEXCHOOKERROR;

    /********************************************************************
     *
     *   Prototypes
     *
     ********************************************************************/

    VOID excExplainException(FILE *file,
                                      PSZ pszHandlerName,
                                      PEXCEPTIONREPORTRECORD pReportRec,
                                      PCONTEXTRECORD pContextRec);

    VOID excRegisterHooks(PFNEXCOPENFILE pfnExcOpenFileNew,
                          PFNEXCHOOK pfnExcHookNew,
                          PFNEXCHOOKERROR pfnExcHookError,
                          BOOL fBeepOnExceptionNew);

    ULONG _System excHandlerLoud(PEXCEPTIONREPORTRECORD pReportRec,
                                 PEXCEPTIONREGISTRATIONRECORD2 pRegRec2,
                                 PCONTEXTRECORD pContextRec,
                                 PVOID pv);

    ULONG _System excHandlerQuiet(PEXCEPTIONREPORTRECORD pReportRec,
                                  PEXCEPTIONREGISTRATIONRECORD2 pRegRec2,
                                  PCONTEXTRECORD pContextRec,
                                  PVOID pv);

    extern PFNEXCHOOKERROR G_pfnExcHookError;

    extern ULONG G_ulExplainExceptionRunning;

    /********************************************************************
     *
     *   Macros
     *
     ********************************************************************/

    /* See except.c for explanations how to use these. */

    #ifdef __NO_EXCEPTION_HANDLERS__
        // exception handlers can completely be disabled
        #define TRY_LOUD(excptstruct)
    #else
        #ifdef __NO_LOUD_EXCEPTION_HANDLERS__
            #define TRY_LOUD(e) TRY_QUIET(e)
        #else // __NO_LOUD_EXCEPTION_HANDLERS__
        #define TRY_LOUD(excptstruct)                                           \
                {                                                               \
                    EXCEPTSTRUCT          excptstruct = {0};                    \
                    excptstruct.RegRec2.pfnHandler = (PFN)excHandlerLoud;       \
                    excptstruct.arc = DosSetExceptionHandler(                   \
                                (PEXCEPTIONREGISTRATIONRECORD)&(excptstruct.RegRec2)); \
                    if (excptstruct.arc)                                        \
                        if (G_pfnExcHookError)                                  \
                            G_pfnExcHookError(__FILE__, __LINE__, __FUNCTION__, excptstruct.arc);  \
                        else                                                    \
                            DosBeep(1000, 1000);                                \
                    excptstruct.ulExcpt = setjmp(excptstruct.RegRec2.jmpThread); \
                    if (excptstruct.ulExcpt == 0)                               \
                    {

        #endif // __NO_LOUD_EXCEPTION_HANDLERS__
    #endif

    #ifdef __NO_EXCEPTION_HANDLERS__
        // exception handlers can completely be disabled
        #define TRY_QUIET(excptstruct)
    #else
        #define TRY_QUIET(excptstruct)                                          \
                {                                                               \
                    EXCEPTSTRUCT          excptstruct = {0};                    \
                    excptstruct.RegRec2.pfnHandler = (PFN)excHandlerQuiet;      \
                    excptstruct.arc = DosSetExceptionHandler(                   \
                                (PEXCEPTIONREGISTRATIONRECORD)&(excptstruct.RegRec2)); \
                    if (excptstruct.arc)                                        \
                        if (G_pfnExcHookError)                                  \
                            G_pfnExcHookError(__FILE__, __LINE__, __FUNCTION__, excptstruct.arc);  \
                        else                                                    \
                            DosBeep(1000, 1000);                                \
                    excptstruct.ulExcpt = setjmp(excptstruct.RegRec2.jmpThread); \
                    if (excptstruct.ulExcpt == 0)                               \
                    {

    #endif

    #ifdef __NO_EXCEPTION_HANDLERS__
        // exception handlers can completely be disabled
        #define CATCH(excptstruct) if (FALSE) {
    #else
        #define CATCH(excptstruct)                                              \
                        DosUnsetExceptionHandler(                               \
                                (PEXCEPTIONREGISTRATIONRECORD)&(excptstruct.RegRec2)); \
                    } /* end of TRY block */                                    \
                    else                                                        \
                    { /* exception occured: */                                  \
                        DosUnsetExceptionHandler((PEXCEPTIONREGISTRATIONRECORD)&(excptstruct.RegRec2));
    #endif

    #ifdef __NO_EXCEPTION_HANDLERS__
        // exception handlers can completely be disabled
        #define END_CATCH() }
    #else
        #define END_CATCH()                                                     \
                    } /* end of exception-occured block */                      \
                }
    #endif

    /*
     * CRASH:
     *      this macro is helpful for testing
     *      the exception handlers.
     *      This is not for general use. ;-)
     */

    #define CRASH {PSZ p = NULL; *p = 'a'; }

#endif // EXCEPT_HEADER_INCLUDED

#if __cplusplus
}
#endif

