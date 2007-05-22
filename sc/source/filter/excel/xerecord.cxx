/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xerecord.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:48:13 $
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
#include "precompiled_sc.hxx"

#ifndef SC_XERECORD_HXX
#include "xerecord.hxx"
#endif

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

// Base classes to export Excel records =======================================

XclExpRecordBase::~XclExpRecordBase()
{
}

void XclExpRecordBase::Save( XclExpStream& /*rStrm*/ )
{
}

void XclExpRecordBase::SaveRepeated( XclExpStream& rStrm, size_t nCount )
{
    for( size_t nIndex = 0; nIndex < nCount; ++nIndex )
        Save( rStrm );
}

// ----------------------------------------------------------------------------

XclExpRecord::XclExpRecord( sal_uInt16 nRecId, sal_Size nRecSize ) :
    mnRecSize( nRecSize ),
    mnRecId( nRecId )
{
}

XclExpRecord::~XclExpRecord()
{
}

void XclExpRecord::SetRecHeader( sal_uInt16 nRecId, sal_Size nRecSize )
{
    SetRecId( nRecId );
    SetRecSize( nRecSize );
}

void XclExpRecord::WriteBody( XclExpStream& /*rStrm*/ )
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

XclExpDummyRecord::XclExpDummyRecord( sal_uInt16 nRecId, const void* pRecData, sal_Size nRecSize ) :
    XclExpRecord( nRecId )
{
    SetData( pRecData, nRecSize );
}

void XclExpDummyRecord::SetData( const void* pRecData, sal_Size nRecSize )
{
    mpData = pRecData;
    SetRecSize( pRecData ? nRecSize : 0 );
}

void XclExpDummyRecord::WriteBody( XclExpStream& rStrm )
{
    rStrm.Write( mpData, GetRecSize() );
}

// ============================================================================

XclExpSubStream::XclExpSubStream( sal_uInt16 nSubStrmType ) :
    mnSubStrmType( nSubStrmType )
{
}

void XclExpSubStream::Save( XclExpStream& rStrm )
{
    // BOF record
    switch( rStrm.GetRoot().GetBiff() )
    {
        case EXC_BIFF2:
            rStrm.StartRecord( EXC_ID2_BOF, 4 );
            rStrm << sal_uInt16( 7 ) << mnSubStrmType;
            rStrm.EndRecord();
        break;
        case EXC_BIFF3:
            rStrm.StartRecord( EXC_ID3_BOF, 6 );
            rStrm << sal_uInt16( 0 ) << mnSubStrmType << sal_uInt16( 2104 );
            rStrm.EndRecord();
        break;
        case EXC_BIFF4:
            rStrm.StartRecord( EXC_ID4_BOF, 6 );
            rStrm << sal_uInt16( 0 ) << mnSubStrmType << sal_uInt16( 1705 );
            rStrm.EndRecord();
        break;
        case EXC_BIFF5:
            rStrm.StartRecord( EXC_ID5_BOF, 8 );
            rStrm << EXC_BOF_BIFF5 << mnSubStrmType << sal_uInt16( 4915 ) << sal_uInt16( 1994 );
            rStrm.EndRecord();
        break;
        case EXC_BIFF8:
            rStrm.StartRecord( EXC_ID5_BOF, 16 );
            rStrm << EXC_BOF_BIFF8 << mnSubStrmType << sal_uInt16( 3612 ) << sal_uInt16( 1996 );
            rStrm << sal_uInt32( 1 ) << sal_uInt32( 6 );
            rStrm.EndRecord();
        break;
        default:
            DBG_ERROR_BIFF();
    }

    // substream records
    XclExpRecordList<>::Save( rStrm );

    // EOF record
    rStrm.StartRecord( EXC_ID_EOF, 0 );
    rStrm.EndRecord();
}

// ============================================================================

