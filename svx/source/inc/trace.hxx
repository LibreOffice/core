/*************************************************************************
 *
 *  $RCSfile: trace.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 07:57:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _TRACE_HXX_
#define _TRACE_HXX_

#if defined(DBG_UTIL) && defined(ENABLE_RANGE_TRACING)

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif


class Tracer
{
    ByteString m_sBlockDescription;

    DECLARE_STL_STDKEY_MAP( ::vos::OThread::TThreadIdentifier, sal_Int32, MapThreadId2Int );
    static MapThreadId2Int s_aThreadIndents;

    static ::vos::OMutex    s_aMapSafety;

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
