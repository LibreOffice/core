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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "xerecord.hxx"
#include "xeroot.hxx"

using namespace ::oox;

// Base classes to export Excel records =======================================

XclExpRecordBase::~XclExpRecordBase()
{
}

void XclExpRecordBase::Save( XclExpStream& /*rStrm*/ )
{
}

void XclExpRecordBase::SaveXml( XclExpXmlStream& /*rStrm*/ )
{
}

// ----------------------------------------------------------------------------

XclExpDelegatingRecord::XclExpDelegatingRecord( XclExpRecordBase* pRecord ) :
    mpRecord( pRecord )
{
}

XclExpDelegatingRecord::~XclExpDelegatingRecord()
{
    // Do Nothing; we use Delegating Record for other objects we "know" will
    // survive...
}

void XclExpDelegatingRecord::SaveXml( XclExpXmlStream& rStrm )
{
    if( mpRecord )
        mpRecord->SaveXml( rStrm );
}

// ----------------------------------------------------------------------------

XclExpXmlElementRecord::XclExpXmlElementRecord( sal_Int32 nElement, void (*pAttributes)( XclExpXmlStream& rStrm) )
    : mnElement( nElement ), mpAttributes( pAttributes )
{
}

XclExpXmlElementRecord::~XclExpXmlElementRecord()
{
}

// ----------------------------------------------------------------------------

XclExpXmlStartElementRecord::XclExpXmlStartElementRecord( sal_Int32 nElement, void (*pAttributes)( XclExpXmlStream& rStrm) )
    : XclExpXmlElementRecord( nElement, pAttributes )
{
}

XclExpXmlStartElementRecord::~XclExpXmlStartElementRecord()
{
}

void XclExpXmlStartElementRecord::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStream = rStrm.GetCurrentStream();
    if( ! mpAttributes )
    {
        rStream->startElement( mnElement, FSEND );
    }
    else
    {
        rStream->write( "<" )->writeId( mnElement );
        (*mpAttributes)( rStrm );
    }
}

// ----------------------------------------------------------------------------

XclExpXmlEndElementRecord::XclExpXmlEndElementRecord( sal_Int32 nElement )
    : XclExpXmlElementRecord( nElement )
{
}

XclExpXmlEndElementRecord::~XclExpXmlEndElementRecord()
{
}

void XclExpXmlEndElementRecord::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->endElement( mnElement );
}

// ----------------------------------------------------------------------------

XclExpXmlStartSingleElementRecord::XclExpXmlStartSingleElementRecord( sal_Int32 nElement, void (*pAttributes)( XclExpXmlStream& rStrm) )
    : XclExpXmlElementRecord( nElement, pAttributes )
{
}

XclExpXmlStartSingleElementRecord::~XclExpXmlStartSingleElementRecord()
{
}

void XclExpXmlStartSingleElementRecord::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStream = rStrm.GetCurrentStream();
    rStream->write( "<" )->writeId( mnElement );
    if( mpAttributes )
        (*mpAttributes)( rStrm );
}

// ----------------------------------------------------------------------------

XclExpXmlEndSingleElementRecord::XclExpXmlEndSingleElementRecord()
{
}

XclExpXmlEndSingleElementRecord::~XclExpXmlEndSingleElementRecord()
{
}

void XclExpXmlEndSingleElementRecord::SaveXml( XclExpXmlStream& rStrm )
{
    rStrm.GetCurrentStream()->write( "/>" );
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

template<>
void XclExpValueRecord<double>::SaveXml( XclExpXmlStream& rStrm )
{
    if( mnAttribute == -1 )
        return;
    rStrm.WriteAttributes(
        mnAttribute,    rtl::OString::valueOf( maValue ).getStr(),
        FSEND );
}

// ----------------------------------------------------------------------------

void XclExpBoolRecord::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast< sal_uInt16 >( mbValue ? 1 : 0 );
}

void XclExpBoolRecord::SaveXml( XclExpXmlStream& rStrm )
{
    if( mnAttribute == -1 )
        return;

    rStrm.WriteAttributes(
            // HACK: HIDEOBJ (excdoc.cxx) should be its own object to handle XML_showObjects
            mnAttribute, mnAttribute == XML_showObjects ? "all" : XclXmlUtils::ToPsz( mbValue ),
            FSEND );
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

// Future records =============================================================

XclExpFutureRecord::XclExpFutureRecord( XclFutureRecType eRecType, sal_uInt16 nRecId, sal_Size nRecSize ) :
    XclExpRecord( nRecId, nRecSize ),
    meRecType( eRecType )
{
}

void XclExpFutureRecord::Save( XclExpStream& rStrm )
{
    rStrm.StartRecord( GetRecId(), GetRecSize() + ((meRecType == EXC_FUTUREREC_UNUSEDREF) ? 12 : 4) );
    rStrm << GetRecId() << sal_uInt16( 0 );
    if( meRecType == EXC_FUTUREREC_UNUSEDREF )
        rStrm.WriteZeroBytes( 8 );
    WriteBody( rStrm );
    rStrm.EndRecord();
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

