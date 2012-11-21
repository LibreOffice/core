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
#ifndef _TRACE_HXX_
#define _TRACE_HXX_

#if defined(DBG_UTIL)

#include <tools/string.hxx>
#include <osl/thread.hxx>
#include <osl/mutex.hxx>
#include <comphelper/stl_types.hxx>


class Tracer
{
    rtl::OString m_sBlockDescription;

    DECLARE_STL_STDKEY_MAP( ::oslThreadIdentifier, sal_Int32, MapThreadId2Int );
    static MapThreadId2Int s_aThreadIndents;

    static ::osl::Mutex s_aMapSafety;

public:
    Tracer(const char* _pBlockDescription);
    ~Tracer();

    void TraceString(const char* _pMessage);
    void TraceString1StringParam(const char* _pMessage, const char* _pParam);
};


#define TRACE_RANGE(range_description)          Tracer aTrace(range_description);
#define TRACE_RANGE_MESSAGE(message)            { aTrace.TraceString(message); }
#define TRACE_RANGE_MESSAGE1(message, param)    { aTrace.TraceString1StringParam(message, param); }

#else

#define TRACE_RANGE(range_description)          ;
#define TRACE_RANGE_MESSAGE(message)            ;
#define TRACE_RANGE_MESSAGE1(message, param)    ;

#endif

#endif // _TRACE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
