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

#include "trace.hxx"
#include <tools/debug.hxx>
#include <rtl/strbuf.hxx>

#if defined(DBG_UTIL)

//==============================================================================

//------------------------------------------------------------------------------
::osl::Mutex Tracer::s_aMapSafety;
::std::map< ::oslThreadIdentifier, sal_Int32, ::std::less< oslThreadIdentifier > >
        Tracer::s_aThreadIndents;

//------------------------------------------------------------------------------
Tracer::Tracer(const char* _pBlockDescription)
    :m_sBlockDescription(_pBlockDescription)
{
    ::osl::MutexGuard aGuard(s_aMapSafety);
    sal_uInt32 nIndent = s_aThreadIndents[ ::osl::Thread::getCurrentIdentifier() ]++;

    rtl::OStringBuffer sMessage;
    sMessage.append(static_cast<sal_Int32>(
        ::osl::Thread::getCurrentIdentifier()));
    sMessage.append('\t');
    while (nIndent--)
        sMessage.append('\t');
    sMessage.append(m_sBlockDescription);
    sMessage.append(RTL_CONSTASCII_STRINGPARAM(" {"));
    OSL_TRACE(sMessage.getStr());
}

//------------------------------------------------------------------------------
Tracer::~Tracer()
{
    ::osl::MutexGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = --s_aThreadIndents[ ::osl::Thread::getCurrentIdentifier() ];

    rtl::OStringBuffer sMessage;
    sMessage.append(static_cast<sal_Int32>(
        ::osl::Thread::getCurrentIdentifier()));
    sMessage.append('\t');
    while (nIndent--)
        sMessage.append('\t');
    sMessage.append(RTL_CONSTASCII_STRINGPARAM("} // "));
    sMessage.append(m_sBlockDescription);
    OSL_TRACE(sMessage.getStr());
}

//------------------------------------------------------------------------------
void Tracer::TraceString(const char* _pMessage)
{
    ::osl::MutexGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = s_aThreadIndents[ ::osl::Thread::getCurrentIdentifier() ];

    rtl::OStringBuffer sMessage;
    sMessage.append(static_cast<sal_Int32>(
        ::osl::Thread::getCurrentIdentifier()));
    sMessage.append('\t');
    while (nIndent--)
        sMessage.append('\t');
    sMessage.append(m_sBlockDescription);
    sMessage.append(RTL_CONSTASCII_STRINGPARAM(": "));
    sMessage.append(_pMessage);
    OSL_TRACE(sMessage.getStr());
}

//------------------------------------------------------------------------------
void Tracer::TraceString1StringParam(const char* _pMessage, const char* _pParam)
{
    ::osl::MutexGuard aGuard(s_aMapSafety);
    sal_Int32 nIndent = s_aThreadIndents[ ::osl::Thread::getCurrentIdentifier() ];

    rtl::OStringBuffer sMessage;
    sMessage.append(static_cast<sal_Int32>(
        ::osl::Thread::getCurrentIdentifier()));
    sMessage.append('\t');
    while (nIndent--)
        sMessage.append('\t');
    sMessage.append(m_sBlockDescription);
    sMessage.append(RTL_CONSTASCII_STRINGPARAM(": "));
    sMessage.append(_pMessage);
    OSL_TRACE(sMessage.getStr(), _pParam);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
