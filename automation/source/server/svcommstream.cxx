/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svcommstream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:27:48 $
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

#include "svcommstream.hxx"

SvCommStream::SvCommStream( SvStream* pIO ) { pStream = pIO; }
SvCommStream::~SvCommStream() {}

ICommStream& SvCommStream::operator>>( USHORT& rUShort ) { *pStream >> rUShort; return *this; }
ICommStream& SvCommStream::operator>>( ULONG& rULong ) { *pStream >> rULong; return *this; }
ICommStream& SvCommStream::operator>>( unsigned char& rChar ) { *pStream >> rChar; return *this; }

ICommStream& SvCommStream::operator<<( USHORT nUShort ) { *pStream << nUShort; return *this; }
ICommStream& SvCommStream::operator<<( ULONG nULong ) { *pStream << nULong; return *this; }
ICommStream& SvCommStream::operator<<( unsigned char nChar ) { *pStream << nChar; return *this; }

ULONG SvCommStream::Read( void* pData, ULONG nSize ) { return pStream->Read( pData, nSize ); }
ULONG SvCommStream::Write( const void* pData, ULONG nSize ) { return pStream->Write( pData, nSize ); }

BOOL SvCommStream::IsEof() const { return pStream->IsEof(); }
ULONG SvCommStream::SeekRel( long nPos ) { return pStream->SeekRel( nPos ); }
