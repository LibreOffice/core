/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// Provenance of this code unclear. From crosswin32-dtrans-mingw.diff,
// but from where it got there, I don't know.


#ifndef _SEHANDLER_HXX
#define _SEHANDLER_HXX

#ifndef __MINGW32__
#error This file should be included only in a MinGW compilation
#endif

#include <windows.h>
#include <setjmp.h>

#ifndef EH_UNWINDING
// See _EH_UNWINDING in MSVS9/VC/crt/src/except.inc
#define EH_UNWINDING 2
#endif

namespace {
class __SEHandler
{
public:
    __SEHandler() {}
    ~__SEHandler() {}
    typedef int (*PF)(void *, LPEXCEPTION_POINTERS);
    typedef void (*PH)(void *, LPEXCEPTION_POINTERS);
    typedef void (*PN)(void *);
    void Set(jmp_buf jb, void *pdata=NULL, PF pfilter=NULL, PH phandlerbody=NULL, PN pfinal=NULL)
    {
        __builtin_memcpy(m_jmpbuf, jb, sizeof(jmp_buf));
        m_pData=pdata;
        switch (reinterpret_cast<int>(pfilter))
            {
            default:
                m_filter=pfilter;
                break;
            case EXCEPTION_CONTINUE_EXECUTION:
                m_filter=DefaultFilterContinueExecution;
                break;
            case EXCEPTION_EXECUTE_HANDLER:
                m_filter=DefaultFilterExecuteHandler;
                break;
            case EXCEPTION_CONTINUE_SEARCH:
                m_filter=DefaultFilterContinueSearch;
                break;
            }
        if (phandlerbody)
            m_handlerbody=phandlerbody;
        else
            m_handlerbody=DefaultHandler;
        if (pfinal)
            m_final=pfinal;
        else
            m_final=DefaultFinal;
        m_ER.pHandlerClass = this;
        m_ER.hp = handler;
        asm("movl %%fs:0, %%eax\n\t"
            "movl %%eax, %0": : "m" (m_ER.prev): "%eax" );
        asm("movl %0, %%eax\n\t"
            "movl %%eax, %%fs:0": : "r" (&m_ER): "%eax" );
    }
    void Reset()
    {
        m_final(m_pData);
        asm("movl %0, %%eax \n\t"
            "movl %%eax, %%fs:0"
            : : "m" (m_ER.prev): "%eax");
    }
private:
    __SEHandler(const __SEHandler&);
    __SEHandler& operator=(const __SEHandler&);
    struct _ER {
        _ER* prev;
        PEXCEPTION_HANDLER hp;
        __SEHandler *pHandlerClass;
    };
    static EXCEPTION_DISPOSITION handler(struct _EXCEPTION_RECORD *pExceptionRecord,
                                         void * EstablisherFrame,
                                         struct _CONTEXT *ContextRecord,
                                         void * /*DispatcherContext*/)
    {
        __SEHandler* pThis = reinterpret_cast< _ER * >(EstablisherFrame)->pHandlerClass;
        if  (pExceptionRecord->ExceptionFlags & EH_UNWINDING)
            {
                pThis->m_final(pThis->m_pData);
                return ExceptionContinueSearch;
            }
        EXCEPTION_POINTERS ep={pExceptionRecord, ContextRecord};
        switch (pThis->m_filter(pThis->m_pData, &ep))
            {
            case EXCEPTION_EXECUTE_HANDLER:
                RtlUnwind(EstablisherFrame, &&__set_label, pExceptionRecord, 0);
            __set_label:
                pThis->m_handlerbody(pThis->m_pData, &ep);
                ContextRecord->Ebp = pThis->m_jmpbuf[0];
                ContextRecord->Eip = pThis->m_jmpbuf[1];
                ContextRecord->Esp = pThis->m_jmpbuf[2];
                return ExceptionContinueExecution;
            case EXCEPTION_CONTINUE_SEARCH:
                return ExceptionContinueSearch;
            case EXCEPTION_CONTINUE_EXECUTION:
                return ExceptionContinueExecution;
            }
        return ExceptionContinueExecution;
    }
    static int DefaultFilterContinueSearch(void *, LPEXCEPTION_POINTERS) { return EXCEPTION_CONTINUE_SEARCH; }
    static int DefaultFilterContinueExecution(void *, LPEXCEPTION_POINTERS) { return EXCEPTION_CONTINUE_EXECUTION; }
    static int DefaultFilterExecuteHandler(void *, LPEXCEPTION_POINTERS) { return EXCEPTION_EXECUTE_HANDLER; }
    static void DefaultHandler(void *, LPEXCEPTION_POINTERS) {}
    static void DefaultFinal(void *) {}
    typedef int (*handler_p)(struct _EXCEPTION_RECORD *ExceptionRecord,
                             void * EstablisherFrame,
                             struct _CONTEXT *ContextRecord,
                             void * DispatcherContext);
    _ER m_ER;
    void *m_pData;
    PN m_final;
    PH m_handlerbody;
    PF m_filter;
    jmp_buf m_jmpbuf;
};

} // namespace {

#endif // _SEHANDLER_HXX
