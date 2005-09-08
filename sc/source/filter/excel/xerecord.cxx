/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xerecord.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:01:47 $
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

#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

// Base classes to export Excel records =======================================

XclExpRecordBase::~XclExpRecordBase()
{
}

void XclExpRecordBase::Save( XclExpStream& rStrm )
{
}

void XclExpRecordBase::SaveRepeated( XclExpStream& rStrm, sal_uInt32 nCount )
{
    for( sal_uInt32 nIndex = 0; nIndex < nCount; ++nIndex )
        Save( rStrm );
}

// ----------------------------------------------------------------------------

XclExpRecord::XclExpRecord( sal_uInt16 nRecId, sal_uInt32 nRecSize ) :
    mnRecId( nRecId ),
    mnRecSize( nRecSize )
{
}

XclExpRecord::~XclExpRecord()
{
}

void XclExpRecord::SetRecHeader( sal_uInt16 nRecId, sal_uInt32 nRecSize )
{
    SetRecId( nRecId );
    SetRecSize( nRecSize );
}

void XclExpRecord::WriteBody( XclExpStream& rStrm )
{
}

void XclExpRecord::Save( XclExpStream& rStrm )
{
    DBG_ASSERT( mnRecId != EXC_ID_UNKNOWN, "XclExpRecord::Save - record ID uninitialized" );
    rStrm.StartRecord( mnRecId, mnRecSize );
    WriteBody( rStrm );
    rStrm.EndRecord();
}

// ----------------------------------------------------------------------------

void XclExpBoolRecord::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast< sal_uInt16 >( mbValue ? 1 : 0 );
}

// ----------------------------------------------------------------------------

XclExpDummyRecord::XclExpDummyRecord( sal_uInt16 nRecId, const void* pRecData, sal_uInt32 nRecSize ) :
    XclExpRecord( nRecId )
{
    SetData( pRecData, nRecSize );
}

void XclExpDummyRecord::SetData( const void* pRecData, sal_uInt32 nRecSize )
{
    mpData = pRecData;
    SetRecSize( pRecData ? nRecSize : 0 );
}

void XclExpDummyRecord::WriteBody( XclExpStream& rStrm )
{
    rStrm.Write( mpData, GetRecSize() );
}

// ============================================================================

