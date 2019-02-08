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


#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <osl/thread.hxx>
#include <osl/diagnose.h>
#include <cppu/Enterable.hxx>
#include <cppu/helper/purpenv/Environment.hxx>
#include <cppu/helper/purpenv/Mapping.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <sal/log.hxx>

namespace
{
class LogBridge : public cppu::Enterable
{
    osl::Mutex          m_mutex;
    sal_Int32           m_count;
    oslThreadIdentifier m_threadId;

    virtual  ~LogBridge() override;

public:
    explicit LogBridge();

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam) override;
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam) override;

    virtual void v_enter() override;
    virtual void v_leave() override;

    virtual bool v_isValid(OUString * pReason) override;
};

LogBridge::LogBridge()
    : m_count   (0)
      ,m_threadId(0)
{
}

LogBridge::~LogBridge()
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
        m_threadId = osl::Thread::getCurrentIdentifier();
}

void LogBridge::v_enter()
{
    m_mutex.acquire();

    OSL_ASSERT(m_count >= 0);

    if (m_count == 0)
        m_threadId = osl::Thread::getCurrentIdentifier();

    ++ m_count;
}

void LogBridge::v_leave()
{
    OSL_ASSERT(m_count > 0);

    -- m_count;
    if (!m_count)
        m_threadId = 0;


    m_mutex.release();
}

bool LogBridge::v_isValid(OUString * pReason)
{
    bool result = m_count > 0;
    if (!result)
    {
        *pReason = "not entered";
    }
    else
    {
        result = m_threadId == osl::Thread::getCurrentIdentifier();

        if (!result)
            *pReason = "wrong thread";
    }

    if (result)
        *pReason = "OK";

    return result;
}

    void traceValue(typelib_TypeDescriptionReference* _pTypeRef,void* pArg)
    {
        switch(_pTypeRef->eTypeClass)
        {
            case typelib_TypeClass_STRING:
                SAL_INFO("cppu.log", "" << *static_cast< OUString*>(pArg));
                break;
            case typelib_TypeClass_BOOLEAN:
               SAL_INFO("cppu.log", "" << *static_cast<sal_Bool*>(pArg));
                break;
            case typelib_TypeClass_BYTE:
               SAL_INFO("cppu.log", "" << *static_cast<sal_Int8*>(pArg));
                break;
            case typelib_TypeClass_CHAR:
               SAL_INFO("cppu.log", "" << *static_cast<sal_Char*>(pArg));
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
               SAL_INFO("cppu.log", "" << *static_cast<sal_Int16*>(pArg));
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
               SAL_INFO("cppu.log", "" << *static_cast<sal_Int32*>(pArg));
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
               SAL_INFO("cppu.log", "" << *static_cast<sal_Int64*>(pArg));
                break;
            case typelib_TypeClass_FLOAT:
               SAL_INFO("cppu.log", "" << *static_cast<float*>(pArg));
                break;
            case typelib_TypeClass_DOUBLE:
               SAL_INFO("cppu.log", "" << *static_cast<double*>(pArg));
                break;
            case typelib_TypeClass_TYPE:
                SAL_INFO("cppu.log", "" << static_cast<css::uno::Type*>(pArg)->getTypeName());
                break;
            case typelib_TypeClass_ANY:
                if ( static_cast<uno_Any*>(pArg)->pData )
                    traceValue(static_cast<uno_Any*>(pArg)->pType,static_cast<uno_Any*>(pArg)->pData);
                else
                   SAL_INFO("cppu.log", "void");
                break;
            case typelib_TypeClass_EXCEPTION:
               SAL_INFO("cppu.log", "exception");
                break;
            case typelib_TypeClass_INTERFACE:
                SAL_INFO("cppu.log", "" << _pTypeRef->pTypeName << "0x" << std::hex << pArg);
                break;
            case typelib_TypeClass_VOID:
               SAL_INFO("cppu.log", "void");
                break;
            default:
               SAL_INFO("cppu.log", "0x" << std::hex << pArg);
                break;
        } // switch(pParams[i].pTypeRef->eTypeClass)
    }
}

static void LogProbe(
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
    OString sTemp;
    if ( pMemberType && pMemberType->pTypeName )
        sTemp = OUStringToOString(pMemberType->pTypeName,RTL_TEXTENCODING_ASCII_US);
    if ( pre  )
    {
        SAL_INFO("cppu.log", "{ LogBridge () " << sTemp );
        if ( nParams )
        {
            SAL_INFO("cppu.log", "\n| : ( LogBridge ");
            for(sal_Int32 i = 0;i < nParams;++i)
            {
                if ( i > 0 )
                   SAL_INFO("cppu.log", ",");
                traceValue(pParams[i].pTypeRef,pArgs[i]);

            }
            SAL_INFO("cppu.log", ")");
        } // if ( nParams )
        SAL_INFO("cppu.log", "\n");
    }
    else if ( !pre )
    {
        SAL_INFO("cppu.log", "} LogBridge () " << sTemp);
        if ( ppException && *ppException )
        {
            SAL_INFO("cppu.log", " exception occurred : ");
            typelib_TypeDescription * pElementTypeDescr = nullptr;
            TYPELIB_DANGER_GET( &pElementTypeDescr, (*ppException)->pType );
            SAL_INFO("cppu.log", "" << pElementTypeDescr->pTypeName);
            TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        }
        else if ( pReturnTypeRef )
        {
            SAL_INFO("cppu.log", " return : ");
            traceValue(pReturnTypeRef,pReturn);
        } // if ( pReturn && pReturnTypeRef )

        SAL_INFO("cppu.log", "\n");
    }
}

#ifdef DISABLE_DYNLOADING

#define uno_initEnvironment log_uno_uno_initEnvironment
#define uno_ext_getMapping log_uno_uno_ext_getMapping

#endif

extern "C" void SAL_DLLPUBLIC_EXPORT uno_initEnvironment(uno_Environment * pEnv)
    SAL_THROW_EXTERN_C()
{
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new LogBridge());
}

extern "C" void SAL_DLLPUBLIC_EXPORT uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo,LogProbe);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
