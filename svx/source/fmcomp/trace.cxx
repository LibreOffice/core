/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: trace.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 17:06:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#ifndef _TRACE_HXX_
#include "trace.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#if defined(DBG_UTIL)

//==============================================================================

//------------------------------------------------------------------------------
::vos::OMutex Tracer::s_aMapSafety;
::std::map< ::vos::OThread::TThreadIdentifier, INT32, ::std::less< ::vos::OThread::TThreadIdentifier > >
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

    ByteString sThread( ByteString::CreateFromInt32( (INT32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += " {";
    DBG_TRACE(sMessage.GetBuffer());
}

//------------------------------------------------------------------------------
Tracer::~Tracer()
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = --s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (INT32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += "} // ";
    sMessage += m_sBlockDescription;
    DBG_TRACE(sMessage.GetBuffer());
}

//------------------------------------------------------------------------------
void Tracer::TraceString(const char* _pMessage)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (INT32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += ": ";
    sMessage += _pMessage;
    DBG_TRACE(sMessage.GetBuffer());
}

//------------------------------------------------------------------------------
void Tracer::TraceString1StringParam(const char* _pMessage, const char* _pParam)
{
    ::vos::OGuard aGuard(s_aMapSafety);
    INT32 nIndent = s_aThreadIndents[ ::vos::OThread::getCurrentIdentifier() ];

    ByteString sIndent;
    while (nIndent--)
        sIndent += '\t';

    ByteString sThread( ByteString::CreateFromInt32( (INT32)::vos::OThread::getCurrentIdentifier() ) );
    sThread += '\t';

    ByteString sMessage(sThread);
    sMessage += sIndent;
    sMessage += m_sBlockDescription;
    sMessage += ": ";
    sMessage += _pMessage;
    DBG_TRACE1(sMessage.GetBuffer(), _pParam);
}
#endif
