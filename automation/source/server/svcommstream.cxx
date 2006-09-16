/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svcommstream.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:37:22 $
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
#include "precompiled_automation.hxx"

#include "svcommstream.hxx"

SvCommStream::SvCommStream( SvStream* pIO ) { pStream = pIO; }
SvCommStream::~SvCommStream() {}

ICommStream& SvCommStream::operator>>( comm_USHORT& rUShort ) { *pStream >> rUShort; return *this; }
ICommStream& SvCommStream::operator>>( comm_ULONG& rULong ) { *pStream >> rULong; return *this; }
ICommStream& SvCommStream::operator>>( comm_BOOL& rChar ) { *pStream >> rChar; return *this; }

ICommStream& SvCommStream::operator<<( comm_USHORT nUShort ) { *pStream << nUShort; return *this; }
ICommStream& SvCommStream::operator<<( comm_ULONG nULong ) { *pStream << nULong; return *this; }
ICommStream& SvCommStream::operator<<( comm_BOOL nChar ) { *pStream << nChar; return *this; }

comm_ULONG SvCommStream::Read( void* pData, comm_ULONG nSize ) { return pStream->Read( pData, nSize ); }
comm_ULONG SvCommStream::Write( const void* pData, comm_ULONG nSize ) { return pStream->Write( pData, nSize ); }

comm_BOOL SvCommStream::IsEof() const { return pStream->IsEof(); }
comm_ULONG SvCommStream::SeekRel( long nPos ) { return pStream->SeekRel( nPos ); }
