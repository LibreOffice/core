/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

XclExpXmlElementRecord::XclExpXmlElementRecord(sal_Int32 const nElement)
    : mnElement( nElement )
{
}

XclExpXmlElementRecord::~XclExpXmlElementRecord()
{
}

// ----------------------------------------------------------------------------

XclExpXmlStartElementRecord::XclExpXmlStartElementRecord(sal_Int32 const nElement)
    : XclExpXmlElementRecord(nElement)
{
}

XclExpXmlStartElementRecord::~XclExpXmlStartElementRecord()
{
}

void XclExpXmlStartElementRecord::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStream = rStrm.GetCurrentStream();
    // TODO: no generic way to add attributes here, but it appears to
    // not be needed yet
    rStream->startElement( mnElement, FSEND );
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

XclExpXmlStartSingleElementRecord::XclExpXmlStartSingleElementRecord(
            sal_Int32 const nElement)
    : XclExpXmlElementRecord( nElement )
{
}

XclExpXmlStartSingleElementRecord::~XclExpXmlStartSingleElementRecord()
{
}

void XclExpXmlStartSingleElementRecord::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStream = rStrm.GetCurrentStream();
    rStream->write( "<" )->writeId( mnElement );
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
    OSL_ENSURE( mnRecId != EXC_ID_UNKNOWN, "XclExpRecord::Save - record ID uninitialized" );
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
        mnAttribute,    OString::number( maValue ).getStr(),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
