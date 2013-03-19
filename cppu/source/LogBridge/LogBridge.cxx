/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "osl/mutex.hxx"
#include "osl/thread.h"
#include "uno/dispatcher.h"
#include "typelib/typedescription.hxx"
#include "cppu/helper/purpenv/Environment.hxx"
#include "cppu/helper/purpenv/Mapping.hxx"
#include "cppu/EnvDcp.hxx"
#include "rtl/logfile.hxx"
#include "uno/environment.hxx"
#include <com/sun/star/uno/Type.hxx>
#include <boost/unordered_map.hpp>
#include <memory>

namespace
{
class LogBridge : public cppu::Enterable
{
    osl::Mutex          m_mutex;
    sal_Int32           m_count;
    oslThreadIdentifier m_threadId;

    virtual  ~LogBridge(void);

public:
    explicit LogBridge(void);

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam);
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam);

    virtual void v_enter(void);
    virtual void v_leave(void);

    virtual int  v_isValid(rtl::OUString * pReason);
};

LogBridge::LogBridge(void)
    : m_count   (0)
      ,m_threadId(0)
{
}

LogBridge::~LogBridge(void)
{
    OSL_ASSERT(m_count >= 0);
}

void LogBridge::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    enter();
    pCallee(pParam);
    leave();
}

void LogBridge::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    OSL_ASSERT(m_count > 0);

    -- m_count;
    pCallee(pParam);
    ++ m_count;

    if (!m_threadId)
        m_threadId = osl_getThreadIdentifier(NULL);
}

void LogBridge::v_enter(void)
{
    m_mutex.acquire();

    OSL_ASSERT(m_count >= 0);

    if (m_count == 0)
        m_threadId = osl_getThreadIdentifier(NULL);

    ++ m_count;
}

void LogBridge::v_leave(void)
{
    OSL_ASSERT(m_count > 0);

    -- m_count;
    if (!m_count)
        m_threadId = 0;


    m_mutex.release();
}

int LogBridge::v_isValid(rtl::OUString * pReason)
{
    int result = m_count > 0;
    if (!result)
    {
        *pReason = rtl::OUString("not entered");
    }
    else
    {
        result = m_threadId == osl_getThreadIdentifier(NULL);

        if (!result)
            *pReason = rtl::OUString("wrong thread");
    }

    if (result)
        *pReason = rtl::OUString("OK");

    return result;
}

    void traceValue(typelib_TypeDescriptionReference* _pTypeRef,void* pArg)
    {
        switch(_pTypeRef->eTypeClass)
        {
            case typelib_TypeClass_STRING:
                {
                    const ::rtl::OString sValue( ::rtl::OUStringToOString(*static_cast< ::rtl::OUString*>(pArg),osl_getThreadTextEncoding()));
                    rtl_logfile_trace( "%s", sValue.getStr());
                }
                break;
            case typelib_TypeClass_BOOLEAN:
                rtl_logfile_trace( "%d", *static_cast<sal_Bool*>(pArg));
                break;
            case typelib_TypeClass_BYTE:
                rtl_logfile_trace( "%d", *static_cast<sal_Int8*>(pArg));
                break;
            case typelib_TypeClass_CHAR:
                rtl_logfile_trace( "%c", *static_cast<sal_Char*>(pArg));
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                rtl_logfile_trace( "%d", *static_cast<sal_Int16*>(pArg));
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
                rtl_logfile_trace( "%d", *static_cast<sal_Int32*>(pArg));
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                rtl_logfile_trace( "%d", *static_cast<sal_Int64*>(pArg));
                break;
            case typelib_TypeClass_FLOAT:
                rtl_logfile_trace( "%f", *static_cast<float*>(pArg));
                break;
            case typelib_TypeClass_DOUBLE:
                rtl_logfile_trace( "%f", *static_cast<double*>(pArg));
                break;
            case typelib_TypeClass_TYPE:
                {
                    const ::rtl::OString sValue( ::rtl::OUStringToOString(((com::sun::star::uno::Type*)pArg)->getTypeName(),osl_getThreadTextEncoding()));
                    rtl_logfile_trace( "%s", sValue.getStr());
                }
                break;
            case typelib_TypeClass_ANY:
                if ( static_cast<uno_Any*>(pArg)->pData )
                    traceValue(static_cast<uno_Any*>(pArg)->pType,static_cast<uno_Any*>(pArg)->pData);
                else
                    rtl_logfile_trace( "void");
                break;
            case typelib_TypeClass_EXCEPTION:
                rtl_logfile_trace( "exception");
                break;
            case typelib_TypeClass_INTERFACE:
                {
                    const ::rtl::OString sValue( ::rtl::OUStringToOString(_pTypeRef->pTypeName,osl_getThreadTextEncoding()));
                    rtl_logfile_trace( "%s 0x%p", sValue.getStr(),pArg);
                }
                break;
            case typelib_TypeClass_VOID:
                rtl_logfile_trace( "void");
                break;
            default:
                rtl_logfile_trace( "0x%p", pArg);
                break;
        } // switch(pParams[i].pTypeRef->eTypeClass)
    }
}

void LogProbe(
    bool                                pre,
    SAL_UNUSED_PARAMETER void         * /*pThis*/,
    SAL_UNUSED_PARAMETER void         * /*pContext*/,
    typelib_TypeDescriptionReference  * pReturnTypeRef,
    typelib_MethodParameter           * pParams,
    sal_Int32                           nParams,
    typelib_TypeDescription     const * pMemberType,
    void                              * pReturn,
    void                              * pArgs[],
    uno_Any                          ** ppException )
{
    static ::std::auto_ptr< ::rtl::Logfile> pLogger;
    ::rtl::OString sTemp;
    if ( pMemberType && pMemberType->pTypeName )
        sTemp = ::rtl::OUStringToOString(pMemberType->pTypeName,RTL_TEXTENCODING_ASCII_US);
    if ( pre  )
    {
        rtl_logfile_longTrace( "{ LogBridge () %s", sTemp.getStr() );
        if ( nParams )
        {
            rtl_logfile_trace( "\n| : ( LogBridge ");
            for(sal_Int32 i = 0;i < nParams;++i)
            {
                if ( i > 0 )
                    rtl_logfile_trace( ",");
                traceValue(pParams[i].pTypeRef,pArgs[i]);

            }
            rtl_logfile_trace( ")");
        } // if ( nParams )
        rtl_logfile_trace( "\n");
    }
    else if ( !pre )
    {
        rtl_logfile_longTrace( "} LogBridge () %s",sTemp.getStr());
        if ( ppException && *ppException )
        {
            rtl_logfile_trace( " excption occurred : ");
            typelib_TypeDescription * pElementTypeDescr = 0;
            TYPELIB_DANGER_GET( &pElementTypeDescr, (*ppException)->pType );
            const ::rtl::OString sValue( ::rtl::OUStringToOString(pElementTypeDescr->pTypeName,osl_getThreadTextEncoding()));
            rtl_logfile_trace( "%s", sValue.getStr());
            TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        }
        else if ( pReturnTypeRef )
        {
            rtl_logfile_trace( " return : ");
            traceValue(pReturnTypeRef,pReturn);
        } // if ( pReturn && pReturnTypeRef )

        rtl_logfile_trace( "\n");
    }
}

#ifdef DISABLE_DYNLOADING

#define uno_initEnvironment log_uno_uno_initEnvironment
#define uno_ext_getMapping log_uno_uno_ext_getMapping

#endif

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_initEnvironment(uno_Environment * pEnv)
    SAL_THROW_EXTERN_C()
{
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new LogBridge());
}

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo,LogProbe);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
