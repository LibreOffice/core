/*************************************************************************
 *
 *  $RCSfile: trace.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:16 $
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
#include "trace.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#if defined(DBG_UTIL) && defined(ENABLE_RANGE_TRACING)

//==============================================================================

//------------------------------------------------------------------------------
::vos::OMutex Tracer::s_aMapSafety;
map< ::vos::OThread::TThreadIdentifier, INT32, ::std::less< ::vos::OThread::TThreadIdentifier > >
        Tracer::s_aThreadIndents;

//------------------------------------------------------------------------------
Tracer::Tracer(const char* _pBlockDescription)
    :m_sBlockDescription(_pBlockDescription)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ]++;

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread((INT32)::vos::OThread::getCurrentIdentifier());
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += "=>\t";
    sMessage += m_sBlockDescription;
    DBG_TRACE(sMessage);
}

//------------------------------------------------------------------------------
Tracer::~Tracer()
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = --s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread((INT32)::vos::OThread::getCurrentIdentifier());
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += "<=\t";
    sMessage += m_sBlockDescription;
    DBG_TRACE(sMessage);
}

//------------------------------------------------------------------------------
void Tracer::TraceString(const char* _pMessage)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread((INT32)::vos::OThread::getCurrentIdentifier());
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += ": ";
    sMessage += _pMessage;
    DBG_TRACE(sMessage);
}

//------------------------------------------------------------------------------
void Tracer::TraceString1StringParam(const char* _pMessage, const char* _pParam)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread((INT32)::vos::OThread::getCurrentIdentifier());
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += ": ";
    sMessage += _pMessage;
    DBG_TRACE1(sMessage, _pParam);
}
#endif
