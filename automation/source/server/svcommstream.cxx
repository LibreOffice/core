/*************************************************************************
 *
 *  $RCSfile: svcommstream.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mh $ $Date: 2002-11-18 15:29:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
