/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _TRACE_HXX_
#define _TRACE_HXX_

#if defined(DBG_UTIL) && defined(ENABLE_RANGE_TRACING)

namespace binfilter {


class Tracer
{
    ByteString m_sBlockDescription;

    DECLARE_STL_STDKEY_MAP( ::vos::OThread::TThreadIdentifier, sal_Int32, MapThreadId2Int );
    static MapThreadId2Int s_aThreadIndents;

    static ::vos::OMutex	s_aMapSafety;

public:
    Tracer(const char* _pBlockDescription);
    ~Tracer();

    void TraceString(const char* _pMessage);
    void TraceString1StringParam(const char* _pMessage, const char* _pParam);
};


#define TRACE_RANGE(range_description)			Tracer aTrace(range_description);
#define TRACE_RANGE_MESSAGE(message)			{ aTrace.TraceString(message); }
#define TRACE_RANGE_MESSAGE1(message, param)	{ aTrace.TraceString1StringParam(message, param); }

}//end of namespace binfilter
#else

#define TRACE_RANGE(range_description)			;
#define TRACE_RANGE_MESSAGE(message)			;
#define TRACE_RANGE_MESSAGE1(message, param)	;

#endif

#endif // _TRACE_HXX_
