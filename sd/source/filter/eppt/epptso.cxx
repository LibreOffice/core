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

#include <memory>
#include <sal/config.h>

#include <basegfx/numeric/ftools.hxx>
#include <o3tl/any.hxx>
#include "eppt.hxx"
#include "text.hxx"
#include "epptdef.hxx"
#include "escherex.hxx"
#include <tools/poly.hxx>
#include <tools/stream.hxx>
#include <tools/fontenum.hxx>
#include <sot/storage.hxx>
#include <vcl/graph.hxx>
#include <editeng/svxenum.hxx>
#include <svx/svdobj.hxx>
#include <svx/unoapi.hxx>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <tools/urlobj.hxx>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <oox/ole/olehelper.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;

#define ANSI_CHARSET            0
#define SYMBOL_CHARSET          2

/* Font Families */
#define FF_ROMAN                0x10
#define FF_SWISS                0x20
#define FF_MODERN               0x30
#define FF_SCRIPT               0x40
#define FF_DECORATIVE           0x50

#define DEFAULT_PITCH           0x00
#define FIXED_PITCH             0x01

PPTExBulletProvider::PPTExBulletProvider()
    : pGraphicProv( new EscherGraphicProvider( EscherGraphicProviderFlags::UseInstances ) )
{
}

PPTExBulletProvider::~PPTExBulletProvider()
{
}

sal_uInt16 PPTExBulletProvider::GetId(Graphic const & rGraphic, Size& rGraphicSize )
{
    sal_uInt16 nRetValue = 0xffff;

    if (rGraphic)
    {
        Graphic         aMappedGraphic, aGraphic(rGraphic);
        std::unique_ptr<GraphicObject> xGraphicObject(new GraphicObject(aGraphic));
        Size            aPrefSize( aGraphic.GetPrefSize() );
        BitmapEx        aBmpEx( aGraphic.GetBitmapEx() );

        if ( rGraphicSize.Width() && rGraphicSize.Height() )
        {
            Size aNewSize;
            bool changed = false;
            if (aPrefSize.Width() == 0 || aPrefSize.Height() == 0)
            {
                aBmpEx.Scale(aPrefSize);
                aNewSize = aPrefSize;
                changed = true;
            }
            else
            {
                double          fQ1 = static_cast<double>(aPrefSize.Width()) / static_cast<double>(aPrefSize.Height());
                double          fQ2 = static_cast<double>(rGraphicSize.Width()) / static_cast<double>(rGraphicSize.Height());
                double          fXScale = 1;
                double          fYScale = 1;

                if ( fQ1 > fQ2 )
                    fYScale = fQ1 / fQ2;
                else if ( fQ1 < fQ2 )
                    fXScale = fQ2 / fQ1;

                if ( ( fXScale != 1.0 ) || ( fYScale != 1.0 ) )
                {
                    aBmpEx.Scale( fXScale, fYScale );
                    aNewSize = Size( static_cast<sal_Int32>(static_cast<double>(rGraphicSize.Width()) / fXScale + 0.5 ),
                                     static_cast<sal_Int32>(static_cast<double>(rGraphicSize.Height()) / fYScale + 0.5 ) );
                    changed = true;
                }

                if (changed)
                {
                    rGraphicSize = aNewSize;

                    aMappedGraphic = Graphic( aBmpEx );
                    xGraphicObject.reset(new GraphicObject(aMappedGraphic));
                }
            }
        }
        sal_uInt32 nId = pGraphicProv->GetBlibID(aBuExPictureStream, *xGraphicObject);

        if ( nId && ( nId < 0x10000 ) )
            nRetValue = static_cast<sal_uInt16>(nId) - 1;
    }
    return nRetValue;
}

sal_uInt32 PPTWriter::ImplVBAInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 28;
    if ( pStrm )
    {
        pStrm->WriteUInt32( 0x1f | ( EPP_VBAInfo << 16 ) )
              .WriteUInt32( nSize - 8 )
              .WriteUInt32( 2 | ( EPP_VBAInfoAtom << 16 ) )
              .WriteUInt32( 12 );
        mpPptEscherEx->InsertPersistOffset( EPP_Persist_VBAInfoAtom, pStrm->Tell() );
        pStrm->WriteUInt32( 0 )
              .WriteUInt32( 0 )
              .WriteUInt32( 1 );
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplSlideViewInfoContainer( sal_uInt32 nInstance, SvStream* pStrm )
{
    sal_uInt32 nSize = 111;
    if ( pStrm )
    {
        sal_uInt8 bShowGuides = 0;
        sal_uInt8 const bSnapToGrid = 1;
        sal_uInt8 const bSnapToShape = 0;

        sal_Int32 nScaling = 85;
        sal_Int32 nMasterCoordinate = 0xdda;
        sal_Int32 nXOrigin = -780;
        sal_Int32 nYOrigin = -84;

        sal_Int32 nPosition1 = 0x870;
        sal_Int32 nPosition2 = 0xb40;

        if ( nInstance )
        {
            bShowGuides = 1;
            nScaling = 0x3b;
            nMasterCoordinate = 0xf0c;
            nXOrigin = -1752;
            nYOrigin = -72;
            nPosition1 = 0xb40;
            nPosition2 = 0x870;
        }
        pStrm->WriteUInt32( 0xf | ( EPP_SlideViewInfo << 16 ) | ( nInstance << 4 ) )
              .WriteUInt32( nSize - 8 )
              .WriteUInt32( EPP_SlideViewInfoAtom << 16 ).WriteUInt32( 3 )
              .WriteUChar( bShowGuides ).WriteUChar( bSnapToGrid ).WriteUChar( bSnapToShape )
              .WriteUInt32( EPP_ViewInfoAtom << 16 ).WriteUInt32( 52 )
              .WriteInt32( nScaling ).WriteInt32( 100 ).WriteInt32( nScaling ).WriteInt32( 100 )  // scaling atom - Keeps the current scale
              .WriteInt32( nScaling ).WriteInt32( 100 ).WriteInt32( nScaling ).WriteInt32( 100 )  // scaling atom - Keeps the previous scale
              .WriteInt32( 0x17ac ).WriteInt32( nMasterCoordinate )// Origin - Keeps the origin in master coordinates
              .WriteInt32( nXOrigin ).WriteInt32( nYOrigin )              // Origin
              .WriteUChar( 1 )                          // Bool1 varScale - Set if zoom to fit is set
              .WriteUChar( 0 )                          // bool1 draftMode - Not used
              .WriteUInt16( 0 )                         // padword
              .WriteUInt32( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ).WriteUInt32( 8 )
              .WriteUInt32( 0 )     // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
              .WriteInt32( nPosition1 )    // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
              .WriteUInt32( ( 7 << 4 ) | ( EPP_GuideAtom << 16 ) ).WriteUInt32( 8 )
              .WriteInt32( 1 )      // Type of the guide. If the guide is horizontal this value is zero. If it's vertical, it's one.
              .WriteInt32( nPosition2 );   // Position of the guide in master coordinates. X coordinate if it's vertical, and Y coordinate if it's horizontal.
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplOutlineViewInfoContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 68;
    if ( pStrm )
    {
        pStrm->WriteUInt32( 0xf | ( EPP_OutlineViewInfo << 16 ) ).WriteUInt32( nSize - 8 )
              .WriteUInt32( EPP_ViewInfoAtom << 16 ).WriteUInt32( 52 )
              .WriteInt32( 170 ).WriteInt32( 200 ).WriteInt32( 170 ).WriteInt32( 200 )  // scaling atom - Keeps the current scale
              .WriteInt32( 170 ).WriteInt32( 200 ).WriteInt32( 170 ).WriteInt32( 200 )  // scaling atom - Keeps the previous scale
              .WriteInt32( 0x17ac ).WriteInt32( 0xdda )    // Origin - Keeps the origin in master coordinates
              .WriteInt32( -780 ).WriteInt32( -84 ) // Origin
              .WriteUChar( 1 )                  // bool1 varScale - Set if zoom to fit is set
              .WriteUChar( 0 )                  // bool1 draftMode - Not used
              .WriteUInt16( 0 );                // padword
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplProgBinaryTag( SvStream* pStrm )
{
    sal_uInt32 nPictureStreamSize, nOutlineStreamSize, nSize = 8;

    nPictureStreamSize = aBuExPictureStream.Tell();
    if ( nPictureStreamSize )
        nSize += nPictureStreamSize + 8;

    nOutlineStreamSize = aBuExOutlineStream.Tell();
    if ( nOutlineStreamSize )
        nSize += nOutlineStreamSize + 8;

    if ( pStrm )
    {
        pStrm->WriteUInt32( EPP_BinaryTagData << 16 ).WriteUInt32( nSize - 8 );
        if ( nPictureStreamSize )
        {
            pStrm->WriteUInt32( 0xf | ( EPP_PST_ExtendedBuGraContainer << 16 ) ).WriteUInt32( nPictureStreamSize );
            pStrm->WriteBytes(aBuExPictureStream.GetData(), nPictureStreamSize);
        }
        if ( nOutlineStreamSize )
        {
            pStrm->WriteUInt32( 0xf | ( EPP_PST_ExtendedPresRuleContainer << 16 ) ).WriteUInt32( nOutlineStreamSize );
            pStrm->WriteBytes(aBuExOutlineStream.GetData(), nOutlineStreamSize);
        }
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplProgBinaryTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 8 + 8 + 14;
    if ( pStrm )
    {
        pStrm->WriteUInt32( 0xf | ( EPP_ProgBinaryTag << 16 ) ).WriteUInt32( 0 )
              .WriteUInt32( EPP_CString << 16 ).WriteUInt32( 14 )
              .WriteUInt32( 0x5f005f ).WriteUInt32( 0x50005f )
              .WriteUInt32( 0x540050 ).WriteUInt16( 0x39 );
    }
    if ( pStrm && pBinTagStrm )
    {
        sal_uInt32 nLen = pBinTagStrm->Tell();
        nSize += nLen + 8;
        pStrm->WriteUInt32( EPP_BinaryTagData << 16 ).WriteUInt32( nLen );
        pStrm->WriteBytes(pBinTagStrm->GetData(), nLen);
    }
    else
        nSize += ImplProgBinaryTag( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( static_cast<sal_Int32>(nSize) - 4 ) );
        pStrm->WriteUInt32( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplProgTagContainer( SvStream* pStrm, SvMemoryStream* pBinTagStrm )
{
    sal_uInt32 nSize = 0;
    if ( aBuExPictureStream.Tell() || aBuExOutlineStream.Tell() || pBinTagStrm )
    {
        nSize = 8;
        if ( pStrm )
        {
            pStrm->WriteUInt32( 0xf | ( EPP_ProgTags << 16 ) ).WriteUInt32( 0 );
        }
        nSize += ImplProgBinaryTagContainer( pStrm, pBinTagStrm );
        if ( pStrm )
        {
            pStrm->SeekRel( - ( static_cast<sal_Int32>(nSize) - 4 ) );
            pStrm->WriteUInt32( nSize - 8 );
            pStrm->SeekRel( nSize - 8 );
        }
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplDocumentListContainer( SvStream* pStrm )
{
    sal_uInt32 nSize = 8;
    if ( pStrm )
    {
        pStrm->WriteUInt32( ( EPP_List << 16 ) | 0xf ).WriteUInt32( 0 );
    }

    nSize += ImplVBAInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 0, pStrm );
    nSize += ImplOutlineViewInfoContainer( pStrm );
    nSize += ImplSlideViewInfoContainer( 1, pStrm );
    nSize += ImplProgTagContainer( pStrm );

    if ( pStrm )
    {
        pStrm->SeekRel( - ( static_cast<sal_Int32>(nSize) - 4 ) );
        pStrm->WriteUInt32( nSize - 8 );
        pStrm->SeekRel( nSize - 8 );
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplMasterSlideListContainer( SvStream* pStrm )
{
    sal_uInt32 i, nSize = 28 * mnMasterPages + 8;
    if ( pStrm )
    {
        pStrm->WriteUInt32( 0x1f | ( EPP_SlideListWithText << 16 ) ).WriteUInt32( nSize - 8 );

        for ( i = 0; i < mnMasterPages; i++ )
        {
            pStrm->WriteUInt32( EPP_SlidePersistAtom << 16 ).WriteUInt32( 20 );
            mpPptEscherEx->InsertPersistOffset( EPP_MAINMASTER_PERSIST_KEY | i, pStrm->Tell() );
            pStrm->WriteUInt32( 0 )                 // psrReference - logical reference to the slide persist object ( EPP_MAINMASTER_PERSIST_KEY )
                  .WriteUInt32( 0 )                 // flags - only bit 3 used, if set then slide contains shapes other than placeholders
                  .WriteInt32( 0 )                  // numberTexts - number of placeholder texts stored with the persist object. Allows to display outline view without loading the slide persist objects
                  .WriteInt32( 0x80000000 | i )     // slideId - Unique slide identifier, used for OLE link monikers for example
                  .WriteUInt32( 0 );                // reserved, usually 0
        }
    }
    return nSize;
}

sal_uInt32 PPTWriter::ImplInsertBookmarkURL( const OUString& rBookmarkURL, const sal_uInt32 nType,
    const OUString& rStringVer0, const OUString& rStringVer1, const OUString& rStringVer2, const OUString& rStringVer3 )
{
    sal_uInt32 nHyperId = ++mnExEmbed;

    OUString sBookmarkURL( rBookmarkURL );
    INetURLObject aBaseURI( maBaseURI );
    INetURLObject aBookmarkURI( rBookmarkURL );
    if( aBaseURI.GetProtocol() == aBookmarkURI.GetProtocol() )
    {
        OUString aRelUrl( INetURLObject::GetRelURL( maBaseURI, rBookmarkURL ) );
        if ( !aRelUrl.isEmpty() )
            sBookmarkURL = aRelUrl;
    }
    maHyperlink.emplace_back( sBookmarkURL, nType );

    mpExEmbed->WriteUInt16( 0xf )
               .WriteUInt16( EPP_ExHyperlink )
               .WriteUInt32( 0 );
    sal_uInt32 nHyperSize, nHyperStart = mpExEmbed->Tell();
    mpExEmbed->WriteUInt16( 0 )
               .WriteUInt16( EPP_ExHyperlinkAtom )
               .WriteUInt32( 4 )
               .WriteUInt32( nHyperId );

    PPTWriter::WriteCString( *mpExEmbed, rStringVer0 );
    PPTWriter::WriteCString( *mpExEmbed, rStringVer1, 1 );
    PPTWriter::WriteCString( *mpExEmbed, rStringVer2, 2 );
    PPTWriter::WriteCString( *mpExEmbed, rStringVer3, 3 );

    nHyperSize = mpExEmbed->Tell() - nHyperStart;
    mpExEmbed->SeekRel( - ( static_cast<sal_Int32>(nHyperSize) + 4 ) );
    mpExEmbed->WriteUInt32( nHyperSize );
    mpExEmbed->SeekRel( nHyperSize );
    return nHyperId;
}

bool PPTWriter::ImplCloseDocument()
{
    sal_uInt32 nOfs = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_Document );
    if ( nOfs )
    {
        mpPptEscherEx->PtReplaceOrInsert( EPP_Persist_CurrentPos, mpStrm->Tell() );
        mpStrm->Seek( nOfs );

        // creating the TxMasterStyleAtom
        SvMemoryStream aTxMasterStyleAtomStrm( 0x200, 0x200 );
        {
            EscherExAtom aTxMasterStyleAtom( aTxMasterStyleAtomStrm, EPP_TxMasterStyleAtom, EPP_TEXTTYPE_Other );
            aTxMasterStyleAtomStrm.WriteUInt16( 5 );        // paragraph count
            sal_uInt16 nLev;
            for ( nLev = 0; nLev < 5; nLev++ )
            {
                mpStyleSheet->mpParaSheet[ EPP_TEXTTYPE_Other ]->Write( aTxMasterStyleAtomStrm, nLev, false, mXPagePropSet );
                mpStyleSheet->mpCharSheet[ EPP_TEXTTYPE_Other ]->Write( aTxMasterStyleAtomStrm, nLev, false, mXPagePropSet );
            }
        }

        sal_uInt32 nExEmbedSize = mpExEmbed->TellEnd();

        // nEnvironment : whole size of the environment container
        sal_uInt32 nEnvironment = maFontCollection.GetCount() * 76      // 68 bytes per Fontenityatom and 8 Bytes per header
                                + 8                                     // 1 FontCollection container
                                + 20                                    // SrKinsoku container
                                + 18                                    // 1 TxSiStyleAtom
                                + aTxMasterStyleAtomStrm.Tell()         // 1 TxMasterStyleAtom;
                                + PPTExStyleSheet::SizeOfTxCFStyleAtom();

        sal_uInt32 nBytesToInsert = nEnvironment + 8;

        if ( nExEmbedSize )
            nBytesToInsert += nExEmbedSize + 8 + 12;

        nBytesToInsert += maSoundCollection.GetSize();
        nBytesToInsert += mpPptEscherEx->DrawingGroupContainerSize();
        nBytesToInsert += ImplMasterSlideListContainer(nullptr);
        nBytesToInsert += ImplDocumentListContainer(nullptr);

        // insert nBytes into stream and adjust depending container
        mpPptEscherEx->InsertAtCurrentPos( nBytesToInsert );

        // CREATE HYPERLINK CONTAINER
        if ( nExEmbedSize )
        {
            mpStrm->WriteUInt16( 0xf )
                   .WriteUInt16( EPP_ExObjList )
                   .WriteUInt32( nExEmbedSize + 12 )
                   .WriteUInt16( 0 )
                   .WriteUInt16( EPP_ExObjListAtom )
                   .WriteUInt32( 4 )
                   .WriteUInt32( mnExEmbed );
            mpPptEscherEx->InsertPersistOffset( EPP_Persist_ExObj, mpStrm->Tell() );
            mpStrm->WriteBytes(mpExEmbed->GetData(), nExEmbedSize);
        }

        // CREATE ENVIRONMENT
        mpStrm->WriteUInt16( 0xf ).WriteUInt16( EPP_Environment ).WriteUInt32( nEnvironment );

        // Open Container ( EPP_SrKinsoku )
        mpStrm->WriteUInt16( 0x2f ).WriteUInt16( EPP_SrKinsoku ).WriteUInt32( 12 );
        mpPptEscherEx->AddAtom( 4, EPP_SrKinsokuAtom, 0, 3 );
        mpStrm->WriteInt32( 0 );                        // SrKinsoku Level 0

        // Open Container ( EPP_FontCollection )
        mpStrm->WriteUInt16( 0xf ).WriteUInt16( EPP_FontCollection ).WriteUInt32( maFontCollection.GetCount() * 76 );

        for ( sal_uInt32 i = 0; i < maFontCollection.GetCount(); i++ )
        {
            mpPptEscherEx->AddAtom( 68, EPP_FontEnityAtom, 0, i );
            const FontCollectionEntry* pDesc = maFontCollection.GetById( i );
            sal_Int32 nFontLen = pDesc->Name.getLength();
            if ( nFontLen > 31 )
                nFontLen = 31;
            for ( sal_Int32 n = 0; n < 32; n++ )
            {
                sal_Unicode nUniCode = 0;
                if ( n < nFontLen )
                    nUniCode = pDesc->Name[n];
                mpStrm->WriteUInt16( nUniCode );
            }
            sal_uInt8   lfCharSet = ANSI_CHARSET;
            sal_uInt8 const lfClipPrecision = 0;
            sal_uInt8 const lfQuality = 6;
            sal_uInt8   lfPitchAndFamily = 0;

            if ( pDesc->CharSet == RTL_TEXTENCODING_SYMBOL )
                lfCharSet = SYMBOL_CHARSET;

            switch( pDesc->Family )
            {
                case css::awt::FontFamily::ROMAN :
                    lfPitchAndFamily |= FF_ROMAN;
                break;

                case css::awt::FontFamily::SWISS :
                    lfPitchAndFamily |= FF_SWISS;
                break;

                case css::awt::FontFamily::MODERN :
                    lfPitchAndFamily |= FF_MODERN;
                break;

                case css::awt::FontFamily::SCRIPT:
                    lfPitchAndFamily |= FF_SCRIPT;
                break;

                case css::awt::FontFamily::DECORATIVE:
                     lfPitchAndFamily |= FF_DECORATIVE;
                break;

                default:
                    lfPitchAndFamily |= FAMILY_DONTKNOW;
                break;
            }
            switch( pDesc->Pitch )
            {
                case css::awt::FontPitch::FIXED:
                    lfPitchAndFamily |= FIXED_PITCH;
                break;

                default:
                    lfPitchAndFamily |= DEFAULT_PITCH;
                break;
            }
            mpStrm->WriteUChar( lfCharSet )
                   .WriteUChar( lfClipPrecision )
                   .WriteUChar( lfQuality )
                   .WriteUChar( lfPitchAndFamily );
        }
        mpStyleSheet->WriteTxCFStyleAtom( *mpStrm );        // create style that is used for new standard objects
        mpPptEscherEx->AddAtom( 10, EPP_TxSIStyleAtom );
        mpStrm->WriteUInt32( 7 )                        // ?
               .WriteInt16( 2 )                         // ?
               .WriteUChar( 9 )                         // ?
               .WriteUChar( 8 )                         // ?
               .WriteInt16( 0 );                        // ?

        mpStrm->WriteBytes(aTxMasterStyleAtomStrm.GetData(), aTxMasterStyleAtomStrm.Tell());
        maSoundCollection.Write( *mpStrm );
        mpPptEscherEx->WriteDrawingGroupContainer( *mpStrm );
        ImplMasterSlideListContainer( mpStrm.get() );
        ImplDocumentListContainer( mpStrm.get() );

        sal_uInt32 nOldPos = mpPptEscherEx->PtGetOffsetByID( EPP_Persist_CurrentPos );
        if ( nOldPos )
        {
            mpStrm->Seek( nOldPos );
            return true;
        }
    }
    return false;
}

bool PropValue::GetPropertyValue(
    css::uno::Any& rAny,
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
    const OUString& rString,
    bool bTestPropertyAvailability )
{
    bool bRetValue = true;
    if ( bTestPropertyAvailability )
    {
        bRetValue = false;
        try
        {
            css::uno::Reference< css::beans::XPropertySetInfo >  aXPropSetInfo( rXPropSet->getPropertySetInfo() );
            if ( aXPropSetInfo.is() )
                bRetValue = aXPropSetInfo->hasPropertyByName( rString );
        }
        catch( css::uno::Exception& )
        {
            bRetValue = false;
        }
    }
    if ( bRetValue )
    {
        try
        {
            rAny = rXPropSet->getPropertyValue( rString );
            if ( !rAny.hasValue() )
                bRetValue = false;
        }
        catch( css::uno::Exception& )
        {
            bRetValue = false;
        }
    }
    return bRetValue;
}

css::beans::PropertyState PropValue::GetPropertyState(
    const css::uno::Reference< css::beans::XPropertySet > & rXPropSet,
    const OUString& rPropertyName )
{
    css::beans::PropertyState eRetValue = css::beans::PropertyState_AMBIGUOUS_VALUE;
    try
    {
        css::uno::Reference< css::beans::XPropertyState > aXPropState( rXPropSet, css::uno::UNO_QUERY );
        if ( aXPropState.is() )
            eRetValue = aXPropState->getPropertyState( rPropertyName );
    }
    catch( css::uno::Exception& )
    {

    }
    return eRetValue;
}

bool PropValue::ImplGetPropertyValue( const OUString& rString )
{
    return GetPropertyValue( mAny, mXPropSet, rString );
}

bool PropValue::ImplGetPropertyValue( const css::uno::Reference< css::beans::XPropertySet > & aXPropSet, const OUString& rString )
{
    return GetPropertyValue( mAny, aXPropSet, rString );
}

bool PropStateValue::ImplGetPropertyValue( const OUString& rString, bool bGetPropertyState )
{
    ePropState = css::beans::PropertyState_AMBIGUOUS_VALUE;
    bool bRetValue = true;
#ifdef UNX
    css::uno::Reference< css::beans::XPropertySetInfo >
            aXPropSetInfo( mXPropSet->getPropertySetInfo() );
    if ( !aXPropSetInfo.is() )
            return false;
#endif
    try
    {
        mAny = mXPropSet->getPropertyValue( rString );
        if ( !mAny.hasValue() )
            bRetValue = false;
        else if ( bGetPropertyState )
            ePropState = mXPropState->getPropertyState( rString );
        else
            ePropState = css::beans::PropertyState_DIRECT_VALUE;
    }
    catch( css::uno::Exception& )
    {
        bRetValue = false;
    }
    return bRetValue;
}

void PPTWriter::ImplWriteParagraphs( SvStream& rOut, TextObj& rTextObj )
{
    bool                bFirstParagraph = true;
    sal_uInt32          nCharCount;
    sal_uInt32          nPropertyFlags = 0;
    sal_Int16           nLineSpacing;
    int                 nInstance = rTextObj.GetInstance();

    for ( sal_uInt32 i = 0;  i < rTextObj.ParagraphCount(); ++i, bFirstParagraph = false )
    {
        ParagraphObj* pPara = rTextObj.GetParagraph(i);
        const PortionObj& rPortion = pPara->front();
        nCharCount = pPara->CharacterCount();

        if ( ( pPara->meTextAdjust == css::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_Adjust, pPara->mnTextAdjust ) ) )
            nPropertyFlags |= 0x00000800;
        nLineSpacing = pPara->mnLineSpacing;

        const FontCollectionEntry* pDesc = maFontCollection.GetById( rPortion.mnFont );
        sal_Int16 nNormalSpacing = 100;
        if ( !mbFontIndependentLineSpacing && pDesc )
        {
            double fN = 100.0;
            fN *= pDesc->Scaling;
            nNormalSpacing = static_cast<sal_Int16>( fN + 0.5 );
        }
        if ( !mbFontIndependentLineSpacing && bFirstParagraph && ( nLineSpacing > nNormalSpacing ) )    // sj: i28747, no replacement for fixed linespacing
        {
            nLineSpacing = nNormalSpacing;
            nPropertyFlags |= 0x00001000;
        }
        else
        {
            if ( nLineSpacing > 0 )
            {
                if ( !mbFontIndependentLineSpacing && pDesc )
                     nLineSpacing = static_cast<sal_Int16>( static_cast<double>(nLineSpacing) * pDesc->Scaling + 0.5 );
            }
            else
            {
                if ( !pPara->mbFixedLineSpacing && rPortion.mnCharHeight > static_cast<sal_uInt16>( static_cast<double>(-nLineSpacing) * 0.001 * 72.0 / 2.54 ) ) // 1/100mm to point
                    nLineSpacing = nNormalSpacing;
                else
                    nLineSpacing = static_cast<sal_Int16>( static_cast<double>(nLineSpacing) / 4.40972 );
            }
            if ( ( pPara->meLineSpacing == css::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_LineFeed, nLineSpacing ) ) )
                nPropertyFlags |= 0x00001000;
        }
        if ( ( pPara->meLineSpacingTop == css::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_UpperDist, pPara->mnLineSpacingTop ) ) )
            nPropertyFlags |= 0x00002000;
        if ( ( pPara->meLineSpacingBottom == css::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_LowerDist, pPara->mnLineSpacingBottom ) ) )
            nPropertyFlags |= 0x00004000;
        if ( ( pPara->meForbiddenRules == css::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_UpperDist, pPara->mbForbiddenRules ? 1 : 0 ) ) )
            nPropertyFlags |= 0x00020000;
        if ( ( pPara->meParagraphPunctation == css::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_UpperDist, pPara->mbParagraphPunctation ? 1 : 0 ) ) )
            nPropertyFlags |= 0x00080000;
        if ( ( pPara->meBiDi == css::beans::PropertyState_DIRECT_VALUE ) ||
            ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_BiDi, pPara->mnBiDi ) ) )
            nPropertyFlags |= 0x00200000;

        sal_Int32 nBuRealSize = pPara->nBulletRealSize;
        sal_Int16 nBulletFlags = pPara->nBulletFlags;

        if ( pPara->bExtendedParameters )
            nPropertyFlags |= pPara->nParaFlags;
        else
        {
            nPropertyFlags |= 1;            // turn off bullet explicit
            nBulletFlags = 0;
        }

        // Write nTextOfs and nBullets
        if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_TextOfs, pPara->nTextOfs ) )
            nPropertyFlags |= 0x100;
        if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, ParaAttr_BulletOfs, pPara->nBulletOfs ))
            nPropertyFlags |= 0x400;

        FontCollectionEntry aFontDescEntry( pPara->aFontDesc.Name, pPara->aFontDesc.Family, pPara->aFontDesc.Pitch, pPara->aFontDesc.CharSet );
        sal_uInt16  nFontId = static_cast<sal_uInt16>(maFontCollection.GetId( aFontDescEntry ));

        rOut.WriteUInt32( nCharCount )
            .WriteUInt16( pPara->nDepth )       // Level
            .WriteUInt32( nPropertyFlags );     // Paragraph Attribut Set

        if ( nPropertyFlags & 0xf )
            rOut.WriteInt16( nBulletFlags );
        if ( nPropertyFlags & 0x80 )
            rOut.WriteUInt16( pPara->cBulletId );
        if ( nPropertyFlags & 0x10 )
            rOut.WriteUInt16( nFontId );
        if ( nPropertyFlags & 0x40 )
            rOut.WriteInt16( nBuRealSize );
        if ( nPropertyFlags & 0x20 )
        {
            sal_uInt32 nBulletColor = pPara->nBulletColor;
            if ( nBulletColor == sal_uInt32(COL_AUTO) )
            {
                bool bIsDark = false;
                css::uno::Any aAny;
                if ( PropValue::GetPropertyValue( aAny, mXPagePropSet, "IsBackgroundDark", true ) )
                    aAny >>= bIsDark;
                nBulletColor = bIsDark ? 0xffffff : 0x000000;
            }
            nBulletColor &= 0xffffff;
            nBulletColor |= 0xfe000000;
            rOut.WriteUInt32( nBulletColor );
        }
        if ( nPropertyFlags & 0x00000800 )
            rOut.WriteUInt16( pPara->mnTextAdjust );
        if ( nPropertyFlags & 0x00001000 )
            rOut.WriteUInt16( nLineSpacing );
        if ( nPropertyFlags & 0x00002000 )
            rOut.WriteUInt16( pPara->mnLineSpacingTop );
        if ( nPropertyFlags & 0x00004000 )
            rOut.WriteUInt16( pPara->mnLineSpacingBottom );
        if ( nPropertyFlags & 0x100 )
            rOut.WriteUInt16( pPara->nTextOfs );
        if (  nPropertyFlags & 0x400 )
            rOut.WriteUInt16( pPara->nBulletOfs );
        if ( nPropertyFlags & 0x000e0000 )
        {
            sal_uInt16 nAsianSettings = 0;
            if ( pPara->mbForbiddenRules )
                nAsianSettings |= 1;
            if ( pPara->mbParagraphPunctation )
                nAsianSettings |= 4;
            rOut.WriteUInt16( nAsianSettings );
        }
        if ( nPropertyFlags & 0x200000 )
            rOut.WriteUInt16( pPara->mnBiDi );
    }
}

void PPTWriter::ImplWritePortions( SvStream& rOut, TextObj& rTextObj )
{
    sal_uInt32  nPropertyFlags;
    int nInstance = rTextObj.GetInstance();

    for ( sal_uInt32 i = 0; i < rTextObj.ParagraphCount(); ++i )
    {
        ParagraphObj* pPara = rTextObj.GetParagraph(i);
        for ( std::vector<std::unique_ptr<PortionObj> >::const_iterator it = pPara->begin(); it != pPara->end(); ++it )
        {
            const PortionObj& rPortion = **it;
            nPropertyFlags = 0;
            sal_uInt32 nCharAttr = rPortion.mnCharAttr;
            sal_uInt32 nCharColor = rPortion.mnCharColor;

            if ( nCharColor == sal_uInt32(COL_AUTO) )   // nCharColor depends to the background color
            {
                bool bIsDark = false;
                css::uno::Any aAny;
                if ( PropValue::GetPropertyValue( aAny, mXPagePropSet, "IsBackgroundDark", true ) )
                    aAny >>= bIsDark;
                nCharColor = bIsDark ? 0xffffff : 0x000000;
            }

            nCharColor &= 0xffffff;

            /* the portion is using the embossed or engraved attribute, which we want to map to the relief feature of PPT.
            Because the relief feature of PPT is dependent to the background color, such a mapping can not always be used. */
            if ( nCharAttr & 0x200 )
            {
                sal_uInt32 nBackgroundColor = 0xffffff;

                if ( !nCharColor )          // special treatment for
                    nCharColor = 0xffffff;  // black fontcolor

                css::uno::Any aAny;
                css::drawing::FillStyle aFS( css::drawing::FillStyle_NONE );
                if ( PropValue::GetPropertyValue( aAny, mXPropSet, "FillStyle" ) )
                    aAny >>= aFS;
                switch( aFS )
                {
                    case css::drawing::FillStyle_GRADIENT :
                    {
                        ::tools::Rectangle aRect( Point(), Size( 28000, 21000 ) );
                        EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm.get(), aRect );
                        aPropOpt.CreateGradientProperties( mXPropSet );
                        aPropOpt.GetOpt( ESCHER_Prop_fillColor, nBackgroundColor );
                    }
                    break;
                    case css::drawing::FillStyle_SOLID :
                    {
                        if ( PropValue::GetPropertyValue( aAny, mXPropSet, "FillColor" ) )
                            nBackgroundColor = EscherEx::GetColor( *o3tl::doAccess<sal_uInt32>(aAny) );
                    }
                    break;
                    case css::drawing::FillStyle_NONE :
                    {
                        css::uno::Any aBackAny;
                        css::drawing::FillStyle aBackFS( css::drawing::FillStyle_NONE );
                        if ( PropValue::GetPropertyValue( aBackAny, mXBackgroundPropSet, "FillStyle" ) )
                            aBackAny >>= aBackFS;
                        switch( aBackFS )
                        {
                            case css::drawing::FillStyle_GRADIENT :
                            {
                                ::tools::Rectangle aRect( Point(), Size( 28000, 21000 ) );
                                EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm.get(), aRect );
                                aPropOpt.CreateGradientProperties( mXBackgroundPropSet );
                                aPropOpt.GetOpt( ESCHER_Prop_fillColor, nBackgroundColor );
                            }
                            break;
                            case css::drawing::FillStyle_SOLID :
                            {
                                if ( PropValue::GetPropertyValue( aAny, mXBackgroundPropSet, "FillColor" ) )
                                    nBackgroundColor = EscherEx::GetColor( *o3tl::doAccess<sal_uInt32>(aAny) );
                            }
                            break;
                            default:
                                break;
                        }
                    }
                    break;
                    default:
                        break;
                }

                sal_Int32 nB = nBackgroundColor & 0xff;
                nB += static_cast<sal_uInt8>( nBackgroundColor >> 8  );
                nB += static_cast<sal_uInt8>( nBackgroundColor >> 16 );
                // if the background color is nearly black, relief can't been used, because the text would not be visible
                if ( nB < 0x60 || ( nBackgroundColor != nCharColor ) )
                {
                    nCharAttr &=~ 0x200;

                    // now check if the text is part of a group, and if the previous object has the same color than the fontcolor
                    // ( and if fillcolor is not available the background color ), it is sometimes
                    // not possible to export the 'embossed' flag
                    if ( ( GetCurrentGroupLevel() > 0 ) && ( GetCurrentGroupIndex() >= 1 ) )
                    {
                        css::uno::Reference< css::drawing::XShape > aGroupedShape( GetCurrentGroupAccess()->getByIndex( GetCurrentGroupIndex() - 1 ), uno::UNO_QUERY );
                        if( aGroupedShape.is() )
                        {
                            css::uno::Reference< css::beans::XPropertySet > aPropSetOfNextShape
                                ( aGroupedShape, css::uno::UNO_QUERY );
                            if ( aPropSetOfNextShape.is() )
                            {
                                if ( PropValue::GetPropertyValue( aAny, aPropSetOfNextShape,
                                                    "FillColor", true ) )
                                {
                                    if ( nCharColor == EscherEx::GetColor( *o3tl::doAccess<sal_uInt32>(aAny) ) )
                                    {
                                        nCharAttr |= 0x200;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            nCharColor |= 0xfe000000;
            if ( nInstance == 4 )                       // special handling for normal textobjects:
                nPropertyFlags |= nCharAttr & 0x217;    // not all attributes ar inherited
            else
            {
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Bold, nCharAttr ) )
                    nPropertyFlags |= 1;
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Italic, nCharAttr ) )
                    nPropertyFlags |= 2;
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Underline, nCharAttr ) )
                    nPropertyFlags |= 4;
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Shadow, nCharAttr ) )
                    nPropertyFlags |= 0x10;
                if ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Embossed, nCharAttr ) )
                    nPropertyFlags |= 512;
            }
            if ( rTextObj.HasExtendedBullets() )
            {
                nPropertyFlags |= ( i & 0x3f ) << 10 ;
                nCharAttr  |= ( i & 0x3f ) << 10;
            }
            if ( ( rPortion.meFontName == css::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Font, rPortion.mnFont ) ) )
                nPropertyFlags |= 0x00010000;
            if ( ( rPortion.meAsianOrComplexFont == css::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_AsianOrComplexFont, rPortion.mnAsianOrComplexFont ) ) )
                nPropertyFlags |= 0x00200000;
            if ( ( rPortion.meCharHeight == css::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_FontHeight, rPortion.mnCharHeight ) ) )
                nPropertyFlags |= 0x00020000;
            if ( ( rPortion.meCharColor == css::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_FontColor, nCharColor & 0xffffff ) ) )
                nPropertyFlags |= 0x00040000;
            if ( ( rPortion.meCharEscapement == css::beans::PropertyState_DIRECT_VALUE ) ||
                ( mpStyleSheet->IsHardAttribute( nInstance, pPara->nDepth, CharAttr_Escapement, rPortion.mnCharEscapement ) ) )
                nPropertyFlags |= 0x00080000;

            sal_uInt32 nCharCount = rPortion.Count();

            rOut.WriteUInt32( nCharCount )
                .WriteUInt32( nPropertyFlags );          //PropertyFlags

            if ( nPropertyFlags & 0xffff )
                rOut.WriteUInt16( nCharAttr );
            if ( nPropertyFlags & 0x00010000 )
                rOut.WriteUInt16( rPortion.mnFont );
            if ( nPropertyFlags & 0x00200000 )
                rOut.WriteUInt16( rPortion.mnAsianOrComplexFont );
            if ( nPropertyFlags & 0x00020000 )
                rOut.WriteUInt16( rPortion.mnCharHeight );
            if ( nPropertyFlags & 0x00040000 )
                rOut.WriteUInt32( nCharColor );
            if ( nPropertyFlags & 0x00080000 )
                rOut.WriteInt16( rPortion.mnCharEscapement );
        }
    }
}

/**
 * Loads and converts text from shape, value is stored in mnTextSize.
 */
bool PPTWriter::ImplGetText()
{
    mnTextSize = 0;
    mbFontIndependentLineSpacing = false;
    mXText.set( mXShape, css::uno::UNO_QUERY );

    if ( mXText.is() )
    {
        mnTextSize = mXText->getString().getLength();
        css::uno::Any aAny;
        if ( GetPropertyValue( aAny, mXPropSet, "FontIndependentLineSpacing", true ) )
            aAny >>= mbFontIndependentLineSpacing;
    }
    return ( mnTextSize != 0 );
}

void PPTWriter::ImplFlipBoundingBox( EscherPropertyContainer& rPropOpt )
{
    if ( mnAngle < 0 )
        mnAngle = ( 36000 + mnAngle ) % 36000;
    else
        mnAngle = ( 36000 - ( mnAngle % 36000 ) );

    double  fCos = cos( static_cast<double>(mnAngle) * F_PI18000 );
    double  fSin = sin( static_cast<double>(mnAngle) * F_PI18000 );

    double  fWidthHalf = maRect.GetWidth() / 2.0;
    double  fHeightHalf = maRect.GetHeight() / 2.0;

    double  fXDiff = fCos * fWidthHalf + fSin * (-fHeightHalf);
    double  fYDiff = - ( fSin * fWidthHalf - fCos * ( -fHeightHalf ) );

    maRect.Move( static_cast<sal_Int32>( -( fWidthHalf - fXDiff ) ), static_cast<sal_Int32>(  - ( fHeightHalf + fYDiff ) ) );
    mnAngle *= 655;
    mnAngle += 0x8000;
    mnAngle &=~0xffff;                                  // round nAngle to full grads
    rPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );

    if ( ( mnAngle >= ( 45 << 16 ) && mnAngle < ( 135 << 16 ) ) ||
            ( mnAngle >= ( 225 << 16 ) && mnAngle < ( 315 << 16 ) ) )
    {
        // Maddeningly, in those two areas of PPT is the BoundingBox already
        // vertical. Therefore, we need to put down it BEFORE THE ROTATION.
        css::awt::Point aTopLeft( static_cast<sal_Int32>( maRect.Left() + fWidthHalf - fHeightHalf ), static_cast<sal_Int32>( maRect.Top() + fHeightHalf - fWidthHalf ) );
        const long nRotatedWidth(maRect.GetHeight());
        const long nRotatedHeight(maRect.GetWidth());
        const Size aNewSize(nRotatedWidth, nRotatedHeight);
        maRect = ::tools::Rectangle( Point( aTopLeft.X, aTopLeft.Y ), aNewSize );
    }
}

void PPTWriter::ImplAdjustFirstLineLineSpacing( TextObj& rTextObj, EscherPropertyContainer& rPropOpt )
{
    if ( mbFontIndependentLineSpacing )
        return;

    if ( !rTextObj.ParagraphCount() )
        return;

    ParagraphObj* pPara = rTextObj.GetParagraph(0);
    if ( pPara->empty() )
        return;

    const PortionObj& rPortion = pPara->front();
    sal_Int16 nLineSpacing = pPara->mnLineSpacing;
    const FontCollectionEntry* pDesc = maFontCollection.GetById( rPortion.mnFont );
    if ( pDesc )
         nLineSpacing = static_cast<sal_Int16>( static_cast<double>(nLineSpacing) * pDesc->Scaling + 0.5 );

    if ( ( nLineSpacing > 0 ) && ( nLineSpacing < 100 ) )
    {
        double fCharHeight = rPortion.mnCharHeight;
        fCharHeight *= 2540 / 72.0;
        fCharHeight *= 100 - nLineSpacing;
        fCharHeight /= 100;

        sal_uInt32 nUpperDistance = 0;
        rPropOpt.GetOpt( ESCHER_Prop_dyTextTop, nUpperDistance );
        nUpperDistance += static_cast< sal_uInt32 >( fCharHeight * 360.0 );
        rPropOpt.AddOpt( ESCHER_Prop_dyTextTop, nUpperDistance );
    }
}

void PPTWriter::ImplWriteTextStyleAtom( SvStream& rOut, int nTextInstance, sal_uInt32 nAtomInstance,
    TextRuleEntry* pTextRule, SvStream& rExtBuStr, EscherPropertyContainer* pPropOpt )
{
    PPTExParaSheet& rParaSheet = mpStyleSheet->GetParaSheet( nTextInstance );

    rOut.WriteUInt32( ( EPP_TextHeaderAtom << 16 ) | ( nAtomInstance << 4 ) ).WriteUInt32( 4 )
        .WriteInt32( nTextInstance );

    if ( mbEmptyPresObj )
        mnTextSize = 0;
    if ( mbEmptyPresObj )
        return;

    ParagraphObj* pPara;
    TextObjBinary aTextObj( mXText, nTextInstance, maFontCollection, static_cast<PPTExBulletProvider&>(*this) );

    // leaving out EPP_TextCharsAtom w/o text - still write out
    // attribute info though
    if ( mnTextSize )
        aTextObj.Write( &rOut );

    if ( pPropOpt && mType != "drawing.Table" )
        ImplAdjustFirstLineLineSpacing( aTextObj, *pPropOpt );

    sal_uInt32 nSize, nPos = rOut.Tell();

    rOut.WriteUInt32( EPP_StyleTextPropAtom << 16 ).WriteUInt32( 0 );
    ImplWriteParagraphs( rOut, aTextObj );
    ImplWritePortions( rOut, aTextObj );
    nSize = rOut.Tell() - nPos;
    rOut.SeekRel( - ( static_cast<sal_Int32>(nSize) - 4 ) );
    rOut.WriteUInt32( nSize - 8 );
    rOut.SeekRel( nSize - 8 );

    for ( sal_uInt32 i = 0; i < aTextObj.ParagraphCount(); ++i )
    {
        pPara = aTextObj.GetParagraph(i);
        for ( std::vector<std::unique_ptr<PortionObj> >::const_iterator it = pPara->begin(); it != pPara->end(); ++it )
        {
            const PortionObj& rPortion = **it;
            if ( rPortion.mpFieldEntry )
            {
                const FieldEntry* pFieldEntry = rPortion.mpFieldEntry.get();

                switch ( pFieldEntry->nFieldType >> 28 )
                {
                    case 1 :
                    case 2 :
                    {
                        rOut.WriteUInt32( EPP_DateTimeMCAtom << 16 ).WriteUInt32( 8 )
                            .WriteUInt32( pFieldEntry->nFieldStartPos )             // TxtOffset to TxtField;
                            .WriteUChar( pFieldEntry->nFieldType & 0xff )       // Type
                            .WriteUChar( 0 ).WriteUInt16( 0 );                      // PadBytes
                    }
                    break;
                    case 3 :
                    {
                        rOut.WriteUInt32( EPP_SlideNumberMCAtom << 16 ).WriteUInt32( 4 )
                            .WriteUInt32( pFieldEntry->nFieldStartPos );
                    }
                    break;
                    case 4 :
                    {
                        sal_uInt32 nPageIndex = 0;
                        OUString aPageUrl;
                        OUString aFile( pFieldEntry->aFieldUrl );
                        OUString aTarget( pFieldEntry->aFieldUrl );
                        INetURLObject aUrl( pFieldEntry->aFieldUrl );
                        if ( INetProtocol::File == aUrl.GetProtocol() )
                            aFile = aUrl.PathToFileName();
                        else if ( INetProtocol::Smb == aUrl.GetProtocol() )
                        {
                            // Convert smb notation to '\\' and skip the 'smb:' part
                            aFile = aUrl.GetMainURL(INetURLObject::DecodeMechanism::NONE).copy(4);
                            aFile = aFile.replaceAll( "/", "\\" );
                            aTarget = aFile;
                        }
                        else if ( pFieldEntry->aFieldUrl.startsWith("#") )
                        {
                            OUString aPage( INetURLObject::decode( pFieldEntry->aFieldUrl, INetURLObject::DecodeMechanism::WithCharset ) );
                            aPage = aPage.copy( 1 );

                            std::vector<OUString>::const_iterator pIter = std::find(
                                        maSlideNameList.begin(),maSlideNameList.end(),aPage);

                            if ( pIter != maSlideNameList.end() )
                            {
                                nPageIndex = pIter - maSlideNameList.begin();
                                aPageUrl = OUString::number(256 + nPageIndex);
                                aPageUrl += ",";
                                aPageUrl += OUString::number(nPageIndex + 1);
                                aPageUrl += ",Slide ";
                                aPageUrl += OUString::number(nPageIndex + 1);
                            }
                        }
                        sal_uInt32 nHyperId(0);
                        if ( !aPageUrl.isEmpty() )
                            nHyperId = ImplInsertBookmarkURL( aPageUrl, 1 | ( nPageIndex << 8 ) | ( 1U << 31 ), pFieldEntry->aRepresentation, "", "", aPageUrl );
                        else
                            nHyperId = ImplInsertBookmarkURL( pFieldEntry->aFieldUrl, 2 | ( nHyperId << 8 ), aFile, aTarget, "", "" );

                        rOut.WriteUInt32( ( EPP_InteractiveInfo << 16 ) | 0xf ).WriteUInt32( 24 )
                            .WriteUInt32( EPP_InteractiveInfoAtom << 16 ).WriteUInt32( 16 )
                            .WriteUInt32( 0 )                                   // soundref
                            .WriteUInt32( nHyperId )                                        // hyperlink id
                            .WriteUChar( 4 )                                    // hyperlink action
                            .WriteUChar( 0 )                                    // ole verb
                            .WriteUChar( 0 )                                    // jump
                            .WriteUChar( 0 )                                    // flags
                            .WriteUChar( 8 )                                    // hyperlink type ?
                            .WriteUChar( 0 ).WriteUChar( 0 ).WriteUChar( 0 )
                            .WriteUInt32( EPP_TxInteractiveInfoAtom << 16 ).WriteUInt32( 8 )
                            .WriteUInt32( pFieldEntry->nFieldStartPos )
                            .WriteUInt32( pFieldEntry->nFieldEndPos );
                    }
                    break;
                    case 5 :
                    {
                        rOut.WriteUInt32( EPP_GenericDateMCAtom << 16 ).WriteUInt32( 4 )
                            .WriteUInt32( pFieldEntry->nFieldStartPos );
                    }
                    break;
                    case 6 :
                    {
                        rOut.WriteUInt32( EPP_HeaderMCAtom << 16 ).WriteUInt32( 4 )
                            .WriteUInt32( pFieldEntry->nFieldStartPos );
                    }
                    break;
                    case 7 :
                    {
                        rOut.WriteUInt32( EPP_FooterMCAtom << 16 ).WriteUInt32( 4 )
                            .WriteUInt32( pFieldEntry->nFieldStartPos );
                    }
                    break;
                    default:
                    break;
                }
            }
        }
    }

    aTextObj.WriteTextSpecInfo( &rOut );

    // write Star Office Default TabSizes (if necessary)
    if ( aTextObj.ParagraphCount() )
    {
        pPara = aTextObj.GetParagraph(0);
        sal_uInt32  nParaFlags = 0x1f;
        sal_Int16   nMask, nNumberingRule[ 10 ];
        sal_uInt32  nTextOfs = pPara->nTextOfs;
        sal_uInt32  nTabs = pPara->maTabStop.getLength();
        const css::style::TabStop* pTabStop = pPara->maTabStop.getConstArray();

        for ( sal_uInt32 i = 0; i < aTextObj.ParagraphCount(); ++i )
        {
            pPara = aTextObj.GetParagraph(i);
            if ( pPara->bExtendedParameters )
            {
                nMask = 1 << pPara->nDepth;
                if ( nParaFlags & nMask )
                {
                    nParaFlags &=~ nMask;
                    if ( ( rParaSheet.maParaLevel[ pPara->nDepth ].mnTextOfs != pPara->nTextOfs ) ||
                        ( rParaSheet.maParaLevel[ pPara->nDepth ].mnBulletOfs != pPara->nBulletOfs ) )
                    {
                        nParaFlags |= nMask << 16;
                        nNumberingRule[ pPara->nDepth << 1 ] = pPara->nTextOfs;
                        nNumberingRule[ ( pPara->nDepth << 1 ) + 1 ] = static_cast<sal_Int16>(pPara->nBulletOfs);
                    }
                }
            }
        }
        nParaFlags >>= 16;

        sal_Int32 nDefaultTabSizeSrc = 2011; // I've no idea where this number came from, honestly
        const uno::Reference< beans::XPropertySet > xPropSet( mXModel, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            if(ImplGetPropertyValue( xPropSet, "TabStop" ))
            {
                sal_Int32 nTabStop( 0 );
                if ( mAny >>= nTabStop )
                    nDefaultTabSizeSrc = nTabStop;
            }
        }
        const sal_uInt32 nDefaultTabSize = MapSize( awt::Size( nDefaultTabSizeSrc, 1 ) ).Width;
        sal_uInt32  nDefaultTabs = std::abs( maRect.GetWidth() ) / nDefaultTabSize;
        if ( nTabs )
            nDefaultTabs -= static_cast<sal_Int32>( ( ( pTabStop[ nTabs - 1 ].Position / 4.40972 ) + nTextOfs ) / nDefaultTabSize );
        if ( static_cast<sal_Int32>(nDefaultTabs) < 0 )
            nDefaultTabs = 0;

        sal_uInt32 nTabCount = nTabs + nDefaultTabs;
        sal_uInt32 i, nTextRulerAtomFlags = 0;

        if ( nTabCount )
            nTextRulerAtomFlags |= 4;
        if ( nParaFlags )
            nTextRulerAtomFlags |= ( ( nParaFlags << 3 ) | ( nParaFlags << 8 ) );

        if ( nTextRulerAtomFlags )
        {
            SvStream* pRuleOut = &rOut;
            if ( pTextRule )
            {
                pTextRule->pOut.reset( new SvMemoryStream( 0x100, 0x100 ) );
                pRuleOut = pTextRule->pOut.get();
            }

            sal_uInt32 nRulePos = pRuleOut->Tell();
            pRuleOut->WriteUInt32( EPP_TextRulerAtom << 16 ).WriteUInt32( 0 );
            pRuleOut->WriteUInt32( nTextRulerAtomFlags );
            if ( nTextRulerAtomFlags & 4 )
            {
                pRuleOut->WriteUInt16( nTabCount );
                for ( i = 0; i < nTabs; i++ )
                {
                    sal_uInt16 nPosition = static_cast<sal_uInt16>( ( pTabStop[ i ].Position / 4.40972 ) + nTextOfs );
                    sal_uInt16 nType;
                    switch ( pTabStop[ i ].Alignment )
                    {
                        case css::style::TabAlign_DECIMAL :    nType = 3; break;
                        case css::style::TabAlign_RIGHT :      nType = 2; break;
                        case css::style::TabAlign_CENTER :     nType = 1; break;

                        case css::style::TabAlign_LEFT :
                        default:                               nType = 0;
                    };
                    pRuleOut->WriteUInt16( nPosition )
                             .WriteUInt16( nType );
                }

                sal_uInt32 nWidth = 1;
                if ( nTabs )
                    nWidth += static_cast<sal_Int32>( ( pTabStop[ nTabs - 1 ].Position / 4.40972 + nTextOfs ) / nDefaultTabSize );
                nWidth *= nDefaultTabSize;
                for ( i = 0; i < nDefaultTabs; i++, nWidth += nDefaultTabSize )
                    pRuleOut->WriteUInt32( nWidth );
            }
            for ( i = 0; i < 5; i++ )
            {
                if ( nTextRulerAtomFlags & ( 8 << i ) )
                    pRuleOut->WriteInt16( nNumberingRule[ i << 1 ] );
                if ( nTextRulerAtomFlags & ( 256 << i ) )
                    pRuleOut->WriteInt16( nNumberingRule[ ( i << 1 ) + 1 ] );
            }
            sal_uInt32 nBufSize = pRuleOut->Tell() - nRulePos;
            pRuleOut->SeekRel( - ( static_cast<sal_Int32>(nBufSize) - 4 ) );
            pRuleOut->WriteUInt32( nBufSize - 8 );
            pRuleOut->SeekRel( nBufSize - 8 );
        }
    }
    if ( !aTextObj.HasExtendedBullets() )
        return;

    if ( !aTextObj.ParagraphCount() )
        return;

    sal_uInt32  nNumberingType = 0, nPos2 = rExtBuStr.Tell();

    rExtBuStr.WriteUInt32( EPP_PST_ExtendedParagraphAtom << 16 ).WriteUInt32( 0 );

    for ( sal_uInt32 i = 0; i < aTextObj.ParagraphCount(); ++i )
    {
        ParagraphObj* pBulletPara = aTextObj.GetParagraph(i);
        sal_uInt32  nBulletFlags = 0;
        sal_uInt16 nBulletId = pBulletPara->nBulletId;

        if ( pBulletPara->bExtendedBulletsUsed )
        {
            nBulletFlags = 0x800000;
            if ( pBulletPara->nNumberingType != SVX_NUM_BITMAP )
                nBulletFlags = 0x3000000;
        }
        rExtBuStr.WriteUInt32( nBulletFlags );

        if ( nBulletFlags & 0x800000 )
            rExtBuStr.WriteUInt16( nBulletId );
        if ( nBulletFlags & 0x1000000 )
        {
            switch( pBulletPara->nNumberingType )
            {
                case SVX_NUM_NUMBER_NONE :
                case SVX_NUM_CHAR_SPECIAL :
                    nNumberingType = 0;
                break;
                case SVX_NUM_CHARS_UPPER_LETTER :
                case SVX_NUM_CHARS_UPPER_LETTER_N :
                case SVX_NUM_CHARS_LOWER_LETTER :
                case SVX_NUM_CHARS_LOWER_LETTER_N :
                case SVX_NUM_ROMAN_UPPER :
                case SVX_NUM_ROMAN_LOWER :
                case SVX_NUM_ARABIC :
                case SVX_NUM_NUMBER_UPPER_ZH:
                case SVX_NUM_CIRCLE_NUMBER:
                case SVX_NUM_NUMBER_UPPER_ZH_TW:
                case SVX_NUM_NUMBER_LOWER_ZH:
                case SVX_NUM_FULL_WIDTH_ARABIC:
                    nNumberingType = pBulletPara->nMappedNumType;
                break;

                case SVX_NUM_BITMAP :
                    nNumberingType = 0;
                break;
                default: break;
            }
            rExtBuStr.WriteUInt32( nNumberingType );
        }
        if ( nBulletFlags & 0x2000000 )
            rExtBuStr.WriteUInt16( pBulletPara->nStartWith );
        rExtBuStr.WriteUInt32( 0 ).WriteUInt32( 0 );
    }
    sal_uInt32 nBulletSize = ( rExtBuStr.Tell() - nPos2 ) - 8;
    rExtBuStr.SeekRel( - ( static_cast<sal_Int32>(nBulletSize) + 4 ) );
    rExtBuStr.WriteUInt32( nBulletSize );
    rExtBuStr.SeekRel( nBulletSize );
}

void PPTWriter::ImplWriteClickAction( SvStream& rSt, css::presentation::ClickAction eCa, bool bMediaClickAction )
{
    sal_uInt32 nSoundRef = 0;   // a reference to a sound in the sound collection, or NULL.
    sal_uInt32 nHyperLinkID = 0;// a persistent unique identifier to an external hyperlink object (only valid when action == HyperlinkAction).
    sal_uInt8   nAction = 0;     // Action See Action Table
    sal_uInt8   const nOleVerb = 0;    // OleVerb Only valid when action == OLEAction. OLE verb to use, 0 = first verb, 1 = second verb, etc.
    sal_uInt8   nJump = 0;       // Jump See Jump Table
    sal_uInt8   const nFlags = 0;      // Bit 1: Animated. If 1, then button is animated
                            // Bit 2: Stop sound. If 1, then stop current sound when button is pressed.
                            // Bit 3: CustomShowReturn. If 1, and this is a jump to custom show, then return to this slide after custom show.
    sal_uInt8   nHyperLinkType = 0;// HyperlinkType a value from the LinkTo enum, such as LT_URL (only valid when action == HyperlinkAction).

    OUString aFile;

    /*
        Action Table:       Action Value
        NoAction            0
        MacroAction         1
        RunProgramAction    2
        JumpAction          3
        HyperlinkAction     4
        OLEAction           5
        MediaAction         6
        CustomShowAction    7

        Jump Table:     Jump Value
        NoJump          0
        NextSlide,      1
        PreviousSlide,  2
        FirstSlide,     3
        LastSlide,      4
        LastSlideViewed 5
        EndShow         6
    */

    if ( bMediaClickAction )
        nAction = 6;
    else switch( eCa )
    {
        case css::presentation::ClickAction_STOPPRESENTATION :
            nJump += 2;
            [[fallthrough]];
        case css::presentation::ClickAction_LASTPAGE :
            nJump++;
            [[fallthrough]];
        case css::presentation::ClickAction_FIRSTPAGE :
            nJump++;
            [[fallthrough]];
        case css::presentation::ClickAction_PREVPAGE :
            nJump++;
            [[fallthrough]];
        case css::presentation::ClickAction_NEXTPAGE :
        {
            nJump++;
            nAction = 3;
        }
        break;
        case css::presentation::ClickAction_SOUND :
        {
            if ( ImplGetPropertyValue( "Bookmark" ) )
                nSoundRef = maSoundCollection.GetId( *o3tl::doAccess<OUString>(mAny) );
        }
        break;
        case css::presentation::ClickAction_PROGRAM :
        {
            if ( ImplGetPropertyValue( "Bookmark" ) )
            {
                INetURLObject aUrl( *o3tl::doAccess<OUString>(mAny) );
                if ( INetProtocol::File == aUrl.GetProtocol() )
                {
                    aFile = aUrl.PathToFileName();
                    nAction = 2;
                }
            }
        }
        break;

        case css::presentation::ClickAction_BOOKMARK :
        {
            if ( ImplGetPropertyValue( "Bookmark" ) )
            {
                OUString  aBookmark( *o3tl::doAccess<OUString>(mAny) );
                sal_uInt32 nIndex = 0;
                for ( const auto& rSlideName : maSlideNameList )
                {
                    if ( rSlideName == aBookmark )
                    {
                        // Bookmark is a link to a document page
                        nAction = 4;
                        nHyperLinkType = 7;

                        OUString aHyperString = OUString::number(256 + nIndex);
                        aHyperString += ",";
                        aHyperString += OUString::number(nIndex + 1);
                        aHyperString += ",Slide ";
                        aHyperString += OUString::number(nIndex + 1);
                        nHyperLinkID = ImplInsertBookmarkURL( aHyperString, 1 | ( nIndex << 8 ) | ( 1U << 31 ), aBookmark, "", "", aHyperString );
                    }
                    nIndex++;
                }
            }
        }
        break;

        case css::presentation::ClickAction_DOCUMENT :
        {
            if ( ImplGetPropertyValue( "Bookmark" ) )
            {
                OUString aBookmark( *o3tl::doAccess<OUString>(mAny) );
                if ( !aBookmark.isEmpty() )
                {
                    nAction = 4;
                    nHyperLinkType = 8;

                    OUString aBookmarkFile( aBookmark );
                    INetURLObject aUrl( aBookmark );
                    if ( INetProtocol::File == aUrl.GetProtocol() )
                        aBookmarkFile = aUrl.PathToFileName();
                    nHyperLinkID = ImplInsertBookmarkURL( aBookmark, sal_uInt32(2 | ( 1U << 31 )), aBookmarkFile, aBookmark, "", "" );
                }
            }
        }
        break;

        case css::presentation::ClickAction_INVISIBLE :
        case css::presentation::ClickAction_VERB :
        case css::presentation::ClickAction_VANISH :
        case css::presentation::ClickAction_MACRO :
        default :
        break;
    }

    sal_uInt32 nContainerSize = 24;
    if ( nAction == 2 )
        nContainerSize += ( aFile.getLength() * 2 ) + 8;
    rSt.WriteUInt32( ( EPP_InteractiveInfo << 16 ) | 0xf ).WriteUInt32( nContainerSize )
       .WriteUInt32( EPP_InteractiveInfoAtom << 16 ).WriteUInt32( 16 )
       .WriteUInt32( nSoundRef )
       .WriteUInt32( nHyperLinkID )
       .WriteUChar( nAction )
       .WriteUChar( nOleVerb )
       .WriteUChar( nJump )
       .WriteUChar( nFlags )
       .WriteUInt32( nHyperLinkType );

    if ( nAction == 2 )     // run program Action
    {
        sal_Int32 nLen = aFile.getLength();
        rSt.WriteUInt32( ( EPP_CString << 16 ) | 0x20 ).WriteUInt32( nLen * 2 );
        for ( sal_Int32 i = 0; i < nLen; i++ )
            rSt.WriteUInt16( aFile[i] );
    }

    rSt.WriteUInt32( ( EPP_InteractiveInfo << 16 ) | 0x1f ).WriteUInt32( 24 )   // Mouse Over Action
       .WriteUInt32( EPP_InteractiveInfo << 16 ).WriteUInt32( 16 );
    for ( int i = 0; i < 4; i++, rSt.WriteUInt32( 0 ) ) ;
}

bool PPTWriter::ImplGetEffect( const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
                                css::presentation::AnimationEffect& eEffect,
                                css::presentation::AnimationEffect& eTextEffect,
                                bool& bIsSound )
{
    css::uno::Any aAny;
    if ( GetPropertyValue( aAny, rPropSet, "Effect" ) )
        aAny >>= eEffect;
    else
        eEffect = css::presentation::AnimationEffect_NONE;

    if ( GetPropertyValue( aAny, rPropSet, "TextEffect" ) )
        aAny >>= eTextEffect;
    else
        eTextEffect = css::presentation::AnimationEffect_NONE;
    if ( GetPropertyValue( aAny, rPropSet, "SoundOn" ) )
        aAny >>= bIsSound;
    else
        bIsSound = false;

    bool bHasEffect = ( ( eEffect != css::presentation::AnimationEffect_NONE )
                        || ( eTextEffect != css::presentation::AnimationEffect_NONE )
                        || bIsSound );
    return bHasEffect;
};

bool PPTWriter::ImplCreatePresentationPlaceholder( const bool bMasterPage,
                                                   const sal_uInt32 nStyleInstance, const sal_uInt8 nPlaceHolderId )
{
    bool bRet = ImplGetText();
    if ( bRet && bMasterPage )
    {
        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        sal_uInt32 nPresShapeID = mpPptEscherEx->GenerateShapeId();
        mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty, nPresShapeID );
        EscherPropertyContainer aPropOpt;
        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
        mnTxId += 0x60;
        aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId );
        aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
        aPropOpt.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
        aPropOpt.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
        aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
        sal_uInt32 nLineFlags = 0x90001;
        if ( aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
            nLineFlags |= 0x10001;  // draw dashed line if no line
        aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );

        SvMemoryStream  aExtBu( 0x200, 0x200 );
        SvMemoryStream  aClientTextBox( 0x200, 0x200 );
        ImplWriteTextStyleAtom( aClientTextBox, nStyleInstance, 0, nullptr, aExtBu, &aPropOpt );

        mnTxId += 0x60;
        aPropOpt.CreateTextProperties( mXPropSet, mnTxId );
        aPropOpt.CreateShapeProperties( mXShape );
        aPropOpt.Commit( *mpStrm );
        mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
        mpStrm->WriteInt16( maRect.Top() ).WriteInt16( maRect.Left() ).WriteInt16( maRect.Right() ).WriteInt16( maRect.Bottom() );      // top, left, right, bottom ????
        mpPptEscherEx->OpenContainer( ESCHER_ClientData );
        mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
        mpStrm->WriteUInt32( 0 )                // PlacementID
               .WriteUChar( nPlaceHolderId )    // PlaceHolderID
               .WriteUChar( 0 )                 // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
               .WriteUInt16( 0 );               // padword
        mpPptEscherEx->CloseContainer();        // ESCHER_ClientData

        if ( aClientTextBox.Tell() )
        {
            mpStrm->WriteUInt32( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                   .WriteUInt32( aClientTextBox.Tell() );

            mpStrm->WriteBytes(aClientTextBox.GetData(), aClientTextBox.Tell());
        }
        mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
    }
    else
        bRet = false;
    return bRet;
}

void PPTWriter::ImplCreateShape( sal_uInt32 nType, ShapeFlag nFlags, EscherSolverContainer& rSolver )
{
    sal_uInt32 nId = mpPptEscherEx->GenerateShapeId();
    mpPptEscherEx->AddShape( nType, nFlags, nId );
    rSolver.AddShape( mXShape, nId );
}

void PPTWriter::ImplCreateTextShape( EscherPropertyContainer& rPropOpt, EscherSolverContainer& rSolver, bool bFill )
{
    mnTextStyle = EPP_TEXTSTYLE_TEXT;
    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
    ImplCreateShape( ESCHER_ShpInst_TextBox, ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty, rSolver );
    if ( bFill )
        rPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
    if ( ImplGetText() )
    {
        mnTxId += 0x60;
        rPropOpt.CreateTextProperties( mXPropSet, mnTxId );
    }
}

void PPTWriter::ImplWritePage( const PHLayout& rLayout, EscherSolverContainer& aSolverContainer, PageType ePageType, bool bMasterPage, int nPageNumber )
{
    // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
    // sal_uInt32  nGroupLevel = 0;

    sal_uInt32  nGroups, nShapes, nShapeCount, nPer, nLastPer, nIndices, nOlePictureId;
    css::awt::Point   aTextRefPoint;

    nShapes = mXShapes->getCount();
    ResetGroupTable( nShapes );

    nIndices = nLastPer = nShapeCount = 0;

    bool bIsTitlePossible = true;           // powerpoint is not able to handle more than one title

    sal_uInt32  nOutlinerCount = 0;                 // the outline objects have to conform to the layout,
    sal_uInt32  nPrevTextStyle = 0;                 // there are no more than two allowed

    nOlePictureId = 0;

    bool bAdditionalText = false;

    bool bSecOutl = false;
    sal_uInt32 nPObjects = 0;

    std::unique_ptr<SvMemoryStream> pClientTextBox;
    std::unique_ptr<SvMemoryStream> pClientData;

    while( GetNextGroupEntry() )
    {
        nShapeCount++;

        nPer = ( 5 * nShapeCount ) / nShapes;
        if ( nPer != nLastPer )
        {
            nLastPer = nPer;
            sal_uInt32 nValue = mnPagesWritten * 5 + nPer;
            if ( nValue > mnStatMaxValue )
                nValue = mnStatMaxValue;
            if ( mbStatusIndicator && ( nValue > mnLatestStatValue ) )
            {
                mXStatusIndicator->setValue( nValue );
                mnLatestStatValue = nValue;
            }
        }
        nGroups = GetGroupsClosed();
        for ( sal_uInt32 i = 0; i < nGroups; i++, mpPptEscherEx->LeaveGroup() ) ;

        if ( GetShapeByIndex( GetCurrentGroupIndex(), true ) )
        {
            bool bIsSound;
            bool bMediaClickAction = false;
            css::presentation::AnimationEffect eAe;
            css::presentation::AnimationEffect eTe;

            bool bEffect = ImplGetEffect( mXPropSet, eAe, eTe, bIsSound );
            css::presentation::ClickAction eCa = css::presentation::ClickAction_NONE;
            if ( ImplGetPropertyValue( "OnClick" ) )
                mAny >>= eCa;

            bool bGroup = mType == "drawing.Group";
            bool bOpenBezier   = mType == "drawing.OpenBezier";
            bool bClosedBezier = mType == "drawing.ClosedBezier";
            bool bPolyPolygon  = mType == "drawing.PolyPolygon";
            bool bPolyLine = mType == "drawing.PolyLine";

            const css::awt::Size   aSize100thmm( mXShape->getSize() );
            const css::awt::Point  aPoint100thmm( mXShape->getPosition() );
            ::tools::Rectangle   aRect100thmm( Point( aPoint100thmm.X, aPoint100thmm.Y ), Size( aSize100thmm.Width, aSize100thmm.Height ) );
            EscherPropertyContainer aPropOpt( mpPptEscherEx->GetGraphicProvider(), mpPicStrm.get(), aRect100thmm );

            if ( bGroup )
            {
                css::uno::Reference< css::container::XIndexAccess >
                    aXIndexAccess( mXShape, css::uno::UNO_QUERY );
                if ( EnterGroup( aXIndexAccess ) )
                {
                    std::unique_ptr<SvMemoryStream> pTmp;
                    if ( eCa != css::presentation::ClickAction_NONE )
                    {
                        pTmp.reset(new SvMemoryStream(0x200, 0x200));
                        ImplWriteClickAction( *pTmp, eCa, bMediaClickAction );
                    }
                    sal_uInt32 nShapeId = mpPptEscherEx->EnterGroup(&maRect, pTmp.get());
                    aSolverContainer.AddShape( mXShape, nShapeId );
                }
            }
            else
            {
                bool bIsFontwork = false;
                bool bIsHatching = false;
                css::uno::Any aAny;
                css::drawing::FillStyle eFS;
                if ( GetPropertyValue( aAny, mXPropSet, "IsFontwork", true ) )
                    aAny >>= bIsFontwork;
                if ( GetPropertyValue( aAny, mXPropSet, "FillStyle", true ) )
                {
                    aAny >>= eFS;
                    bIsHatching = eFS == css::drawing::FillStyle_HATCH;
                }
                if ( bIsHatching || bIsFontwork || ( mType == "drawing.Measure" ) || ( mType == "drawing.Caption" ) )
                {
                    if ( ImplGetPropertyValue( "BoundRect" ) )
                    {
                        auto aRect = o3tl::doAccess<css::awt::Rectangle>(mAny);
                        maPosition = MapPoint( css::awt::Point( aRect->X, aRect->Y ) );
                        maSize = MapSize( css::awt::Size( aRect->Width, aRect->Height ) );
                        maRect = ::tools::Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                    }
                    mType = "drawing.dontknow";
                }
            }
            sal_uInt8 nPlaceHolderAtom = EPP_PLACEHOLDER_NONE;

            mnTextSize = 0;
            mnTextStyle = EPP_TEXTSTYLE_NORMAL;

            if ( mType == "drawing.Custom" )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ShapeFlag nMirrorFlags;
                OUString sCustomShapeType;
                MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType( mXShape, nMirrorFlags, sCustomShapeType );
                if ( sCustomShapeType == "col-502ad400" || sCustomShapeType == "col-60da8460" )
                {   // sj: creating metafile for customshapes that can't be saved to ms format properly
                    ImplCreateShape( ESCHER_ShpInst_PictureFrame,
                                     ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                     aSolverContainer );
                    if ( aPropOpt.CreateGraphicProperties( mXPropSet, "MetaFile", false ) )
                    {
                        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                        SdrObject* pObj = GetSdrObjectFromXShape( mXShape );
                        if ( pObj )
                        {
                            ::tools::Rectangle aBound = pObj->GetCurrentBoundRect();
                            maPosition = MapPoint( css::awt::Point( aBound.Left(), aBound.Top() ) );
                            maSize = MapSize( css::awt::Size ( aBound.GetWidth(), aBound.GetHeight() ) );
                            maRect = ::tools::Rectangle( Point( maPosition.X, maPosition.Y ), Size( maSize.Width, maSize.Height ) );
                            mnAngle = 0;
                        }
                    }
                }
                else
                {
                    ImplCreateShape( eShapeType,
                                     nMirrorFlags | ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                     aSolverContainer );
                    aPropOpt.CreateCustomShapeProperties( eShapeType, mXShape );
                    aPropOpt.CreateFillProperties( mXPropSet, true, mXShape);
                    if ( ImplGetText() )
                    {
                        if ( !aPropOpt.IsFontWork() )
                        {
                            mnTxId += 0x60;
                            aPropOpt.CreateTextProperties( mXPropSet, mnTxId, true );
                        }
                    }
                }
            }
            else if ( mType == "drawing.Rectangle" )
            {
                sal_Int32 nRadius = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                if ( ImplGetPropertyValue( "CornerRadius" ) )
                {
                    mAny >>= nRadius;
                    nRadius = MapSize( css::awt::Size( nRadius, 0 ) ).Width;
                }
                if ( nRadius )
                {
                    ImplCreateShape( ESCHER_ShpInst_RoundRectangle,
                                     ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                     aSolverContainer );
                    sal_Int32 nLength = maRect.GetWidth();
                    if ( nLength > maRect.GetHeight() )
                        nLength = maRect.GetHeight();
                    nLength >>= 1;
                    if ( nRadius >= nLength )
                        nRadius = 0x2a30;                           // 0x2a30 is PPTs maximum radius
                    else
                        nRadius = ( 0x2a30 * nRadius ) / nLength;
                    aPropOpt.AddOpt( ESCHER_Prop_adjustValue, nRadius );
                }
                else
                {
                    ImplCreateShape( ESCHER_ShpInst_Rectangle,
                                     ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                     aSolverContainer );
                }
                aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                if ( ImplGetText() )
                {
                    mnTxId += 0x60;
                    aPropOpt.CreateTextProperties( mXPropSet, mnTxId, false, false );
                }
            }
            else if ( mType == "drawing.Ellipse" )
            {
                css::drawing::CircleKind  eCircleKind( css::drawing::CircleKind_FULL );
                PolyStyle ePolyKind = PolyStyle::Chord;
                if ( ImplGetPropertyValue( "CircleKind" ) )
                {
                    mAny >>= eCircleKind;
                    switch ( eCircleKind )
                    {
                        case css::drawing::CircleKind_SECTION :
                        {
                            ePolyKind = PolyStyle::Pie;
                        }
                        break;
                        case css::drawing::CircleKind_ARC :
                        {
                            ePolyKind = PolyStyle::Arc;
                        }
                        break;

                        case css::drawing::CircleKind_CUT :
                        {
                            ePolyKind = PolyStyle::Chord;
                        }
                        break;

                        default:
                            eCircleKind = css::drawing::CircleKind_FULL;
                    }
                }
                if ( eCircleKind == css::drawing::CircleKind_FULL )
                {
                    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                    ImplCreateShape( ESCHER_ShpInst_Ellipse,
                                     ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                     aSolverContainer );
                    aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                    if ( ImplGetText() )
                    {
                        mnTxId += 0x60;
                        aPropOpt.CreateTextProperties( mXPropSet, mnTxId, false, false );
                    }
                }
                else
                {
                    sal_Int32 nStartAngle, nEndAngle;
                    if ( !ImplGetPropertyValue( "CircleStartAngle" ) )
                        continue;
                    nStartAngle = *o3tl::doAccess<sal_Int32>(mAny);
                    if( !ImplGetPropertyValue( "CircleEndAngle" ) )
                        continue;
                    nEndAngle = *o3tl::doAccess<sal_Int32>(mAny);
                    css::awt::Point aPoint( mXShape->getPosition() );
                    css::awt::Size  aSize( mXShape->getSize() );
                    css::awt::Point aStart, aEnd, aCenter;
                    ::tools::Rectangle aRect( Point( aPoint.X, aPoint.Y ), Size( aSize.Width, aSize.Height ) );
                    aStart.X =   static_cast<sal_Int32>( cos( nStartAngle * F_PI18000 ) * 100.0 );
                    aStart.Y = - static_cast<sal_Int32>( sin( nStartAngle * F_PI18000 ) * 100.0 );
                    aEnd.X =   static_cast<sal_Int32>( cos( nEndAngle * F_PI18000 ) * 100.0 );
                    aEnd.Y = - static_cast<sal_Int32>( sin( nEndAngle * F_PI18000 ) * 100.0 ) ;
                    aCenter.X = aPoint.X + ( aSize.Width / 2 );
                    aCenter.Y = aPoint.Y + ( aSize.Height / 2 );
                    aStart.X += aCenter.X;
                    aStart.Y += aCenter.Y;
                    aEnd.X += aCenter.X;
                    aEnd.Y += aCenter.Y;
                    tools::Polygon aPolygon( aRect, Point( aStart.X, aStart.Y ), Point( aEnd.X, aEnd.Y ), ePolyKind );
                    bool bNeedText = true;
                    if ( mnAngle )
                    {
                        aPolygon.Rotate( aRect.TopLeft(), static_cast<sal_uInt16>( mnAngle / 10 ) );
                        if ( ImplGetText() )
                        {
                            // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                            // mpPptEscherEx->EnterGroup( 0,0 );
                            // nGroupLevel = mpPptEscherEx->GetGroupLevel();
                            bNeedText = false;
                            bAdditionalText = true;
                            mnTextSize = 0;
                        }
                        mnAngle = 0;
                    }
                    mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                    ImplCreateShape( ESCHER_ShpInst_NotPrimitive,
                                     ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                     aSolverContainer );
                    css::awt::Rectangle aNewRect;
                    switch ( ePolyKind )
                    {
                        case PolyStyle::Pie :
                        case PolyStyle::Chord :
                        {
                            if ( aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, false, aNewRect, &aPolygon ) )
                                aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                        }
                        break;

                        case PolyStyle::Arc :
                        {
                            if ( aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, false, aNewRect, &aPolygon ) )
                                aPropOpt.CreateLineProperties( mXPropSet, false );
                        }
                        break;
                    }
                    maRect = MapRectangle( aNewRect );
                    maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                    maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                    if ( bNeedText && ImplGetText() )
                    {
                        mnTxId += 0x60;
                        aPropOpt.CreateTextProperties( mXPropSet, mnTxId, false, false );
                    }
                }
            }
            else if ( mType == "drawing.Control" )
            {
                css::uno::Reference< css::drawing::XControlShape  > aXControlShape( mXShape, css::uno::UNO_QUERY );
                if ( !aXControlShape.is() )
                    continue;
                css::uno::Reference< css::awt::XControlModel > aXControlModel( aXControlShape->getControl() );
                if ( !aXControlModel.is() )
                    continue;

                sal_Int64 nAspect = css::embed::Aspects::MSOLE_CONTENT;
                try
                {
                    // try to get the aspect when available
                    css::uno::Reference< css::beans::XPropertySet > xShapeProps( mXShape, css::uno::UNO_QUERY_THROW );
                    xShapeProps->getPropertyValue("Aspect") >>= nAspect;
                }
                catch( css::uno::Exception& )
                {}

                mpExEmbed->WriteUInt32( 0xf | ( EPP_ExControl << 16 ) )
                           .WriteUInt32( 0 );               // Size of this container

                sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                sal_uInt32 nPageId = nPageNumber;
                if ( ePageType == MASTER )
                    nPageId |= 0x80000000;
                else
                    nPageId += 0x100;
                mpExEmbed->WriteUInt32( EPP_ExControlAtom << 16 )
                           .WriteUInt32( 4 )
                           .WriteUInt32( nPageId );
                std::unique_ptr<PPTExOleObjEntry> pEntry( new PPTExOleObjEntry( OCX_CONTROL, mpExEmbed->Tell() ) );
                pEntry->xControlModel = aXControlModel;
                pEntry->xShape = mXShape;
                maExOleObj.push_back( std::move(pEntry) );

                mnExEmbed++;

                mpExEmbed->WriteUInt32( 1 | ( EPP_ExOleObjAtom << 16 ) )
                           .WriteUInt32( 24 )
                           .WriteUInt32( nAspect )
                           .WriteUInt32( 2 )
                           .WriteUInt32( mnExEmbed )
                           .WriteUInt32( 0 )
                           .WriteUInt32( 4 )    // index to the persist table
                           .WriteUInt32( 0x0012de00 );

                css::awt::Size     aSize;
                OUString          aControlName;
                tools::SvRef<SotStorage>    xTemp( new SotStorage( new SvMemoryStream(), true ) );
                if ( oox::ole::MSConvertOCXControls::WriteOCXStream( mXModel, xTemp, aXControlModel, aSize, aControlName ) )
                {
                    OUString aUserName( xTemp->GetUserName() );
                    OUString aOleIdentifier;
                    if ( !aUserName.isEmpty() )
                    {
                        tools::SvRef<SotStorageStream> xCompObj = xTemp->OpenSotStream(
                            "\1CompObj",
                                StreamMode::READ | StreamMode::NOCREATE | StreamMode::SHARE_DENYALL );
                        sal_uInt32 const nStreamLen = xCompObj->remainingSize();
                        sal_Int16   nVersion, nByteOrder;
                        sal_Int32   nWinVersion, nVal, nStringLen;
                        xCompObj->ReadInt16( nVersion )
                                 .ReadInt16( nByteOrder )
                                 .ReadInt32( nWinVersion )
                                 .ReadInt32( nVal );
                        xCompObj->SeekRel( 16 );    // skipping clsid
                        xCompObj->ReadInt32( nStringLen );
                        if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                        {
                            xCompObj->SeekRel( nStringLen );        // now skipping the UserName;
                            xCompObj->ReadInt32( nStringLen );
                            if ( ( xCompObj->Tell() + nStringLen ) < nStreamLen )
                            {
                                xCompObj->SeekRel( nStringLen );    // now skipping the clipboard formatname
                                xCompObj->ReadInt32( nStringLen );
                                if ( ( nStringLen > 1 ) && ( ( xCompObj->Tell() + nStringLen ) < nStreamLen ) )
                                {   // i think that the OleIdentifier will follow
                                    OString aTemp = read_uInt8s_ToOString(*xCompObj, nStringLen - 1);
                                    aOleIdentifier = OStringToOUString(aTemp, RTL_TEXTENCODING_MS_1252);
                                }
                            }
                        }
                    }

                    PPTWriter::WriteCString( *mpExEmbed, aControlName, 1 );
                    PPTWriter::WriteCString( *mpExEmbed, aOleIdentifier, 2 );
                    PPTWriter::WriteCString( *mpExEmbed, aUserName, 3 );
                }
                nSize = mpExEmbed->Tell() - nOldPos;
                mpExEmbed->Seek( nOldPos - 4 );
                mpExEmbed->WriteUInt32( nSize );
                mpExEmbed->Seek( STREAM_SEEK_TO_END );
                nOlePictureId = mnExEmbed;

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ShapeFlag const nSpFlags = ShapeFlag::HaveShapeProperty | ShapeFlag::HaveAnchor | ShapeFlag::OLEShape;
                ImplCreateShape( ESCHER_ShpInst_HostControl, nSpFlags, aSolverContainer );
                if ( aPropOpt.CreateGraphicProperties( mXPropSet, "MetaFile", false  ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                //export form control graphic
                else if ( aPropOpt.CreateBlipPropertiesforOLEControl(mXPropSet,mXShape))
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                aPropOpt.AddOpt( ESCHER_Prop_pictureId, mnExEmbed );
                aPropOpt.AddOpt( ESCHER_Prop_pictureActive, 0x10000 );

                if ( !aControlName.isEmpty() )
                {
                    aPropOpt.AddOpt(ESCHER_Prop_wzName, aControlName);
                }
            }
            else if ( mType == "drawing.Connector" )
            {
                sal_uInt16 nSpType;
                ShapeFlag nSpFlags;
                css::awt::Rectangle aNewRect;
                if ( !aPropOpt.CreateConnectorProperties( mXShape, aSolverContainer, aNewRect, nSpType, nSpFlags ) )
                    continue;

                maRect = MapRectangle( aNewRect );
                maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( nSpType, nSpFlags, aSolverContainer );

                // #119459# for connector shape, the start point and end point is fixed, and should not be rotated.
                mnAngle = 0;
            }
            else if ( mType == "drawing.Measure" )
            {
                continue;
            }
            else if ( mType == "drawing.Line" )
            {
                css::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_LINE, false, aNewRect );
                maRect = MapRectangle( aNewRect );
                maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                if ( ImplGetText() )
                {
                    aTextRefPoint = css::awt::Point( maRect.Left(), maRect.Top() );
                    mnTextSize = 0;
                    bAdditionalText = true;
                    // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                    // mpPptEscherEx->EnterGroup( &maRect,0 );
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ShapeFlag nFlags = ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty;

                if ( maRect.Top() > maRect.Bottom() )
                    nFlags |= ShapeFlag::FlipV;
                if ( maRect.Left() > maRect.Right() )
                    nFlags |= ShapeFlag::FlipH;

                ImplCreateShape( ESCHER_ShpInst_Line, nFlags, aSolverContainer );
                aPropOpt.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
                aPropOpt.CreateLineProperties( mXPropSet, false );
                mnAngle = 0;
            }
            else if ( bPolyPolygon )
            {
                if ( ImplGetText() )
                {
                    // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                    // mpPptEscherEx->EnterGroup( 0,0 );
                    // nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = true;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );
                css::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, false, aNewRect );
                maRect = MapRectangle( aNewRect );
                maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                mnAngle = 0;
            }
            else if ( bPolyLine )
            {
                if ( ImplGetText() )
                {
                    // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                    // mpPptEscherEx->EnterGroup( 0,0 );
                    // nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = true;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );
                css::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, false, aNewRect );
                maRect = MapRectangle( aNewRect );
                maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateLineProperties( mXPropSet, false );
                mnAngle = 0;
            }
            else if ( bOpenBezier )
            {
                if ( ImplGetText() )
                {
                    // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                    // mpPptEscherEx->EnterGroup( 0,0 );
                    // nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = true;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );
                css::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYLINE, true, aNewRect );
                maRect = MapRectangle( aNewRect );
                maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateLineProperties( mXPropSet, false );
                mnAngle = 0;
            }
            else if ( bClosedBezier )
            {
                if ( ImplGetText() )
                {
                    // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                    // mpPptEscherEx->EnterGroup( 0,0 );
                    // nGroupLevel = mpPptEscherEx->GetGroupLevel();
                    bAdditionalText = true;
                    mnTextSize = 0;
                }
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_NotPrimitive,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );
                css::awt::Rectangle aNewRect;
                aPropOpt.CreatePolygonProperties( mXPropSet, ESCHER_CREATEPOLYGON_POLYPOLYGON, true, aNewRect );
                maRect = MapRectangle( aNewRect );
                maPosition = css::awt::Point( maRect.Left(), maRect.Top() );
                maSize = css::awt::Size( maRect.GetWidth(), maRect.GetHeight() );
                aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                mnAngle = 0;
            }
            else if ( ( mType == "drawing.GraphicObject" ) || ( mType == "presentation.GraphicObject" ) )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );

                // a GraphicObject can also be a ClickMe element
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = rLayout.nUsedObjectPlaceHolder;
                    ImplCreateShape( ESCHER_ShpInst_Rectangle, ShapeFlag::HaveAnchor | ShapeFlag::HaveMaster, aSolverContainer );
                    mnTxId += 0x60;
                    aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    mXText.set( mXShape, css::uno::UNO_QUERY );

                    if ( mXText.is() )
                        mnTextSize = mXText->getString().getLength();

                    if ( mnTextSize )                                       // graphic object or area fill
                    {
                        /* SJ #i34951#: because M. documents are not allowing GraphicObjects containing text, we
                        have to create a simple Rectangle with fill bitmap instead (while not allowing BitmapMode_Repeat).
                        */
                        ImplCreateShape( ESCHER_ShpInst_Rectangle,
                                         ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                         aSolverContainer );
                        if ( aPropOpt.CreateGraphicProperties( mXPropSet, "Graphic", true, true, false ) )
                        {
                            aPropOpt.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapNone );
                            aPropOpt.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
                            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x140014 );
                            aPropOpt.AddOpt( ESCHER_Prop_fillBackColor, 0x8000000 );
                            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x80000 );
                            if ( ImplGetText() )
                            {
                                mnTxId += 0x60;
                                aPropOpt.CreateTextProperties( mXPropSet, mnTxId, false, false );
                            }
                        }
                    }
                    else
                    {
                        ImplCreateShape( ESCHER_ShpInst_PictureFrame,
                                         ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                         aSolverContainer );

                        if ( aPropOpt.CreateGraphicProperties( mXPropSet, "Graphic", false, true ) )
                        {
                            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                        }
                    }
                }
            }
            else if ( ( mType == "drawing.Text" ) || ( mType == "presentation.Notes" ) )
            {
                if ( ( ePageType == NOTICE ) && mbPresObj )
                {
                    if ( ImplCreatePresentationPlaceholder( bMasterPage, EPP_TEXTTYPE_Notes, EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE ) )
                        continue;
                    else
                        nPlaceHolderAtom = EPP_PLACEHOLDER_NOTESBODY;
                }
                ImplCreateTextShape( aPropOpt, aSolverContainer, true );
            }
            else if ( mType == "presentation.TitleText" )
            {
                if ( mbPresObj )
                {
                    if ( ( ePageType == NOTICE ) && mbEmptyPresObj )
                    {
                        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                        nPlaceHolderAtom = EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE;
                        ImplCreateShape( ESCHER_ShpInst_Rectangle, ShapeFlag::HaveAnchor, aSolverContainer );
                        aPropOpt.CreateLineProperties( mXPropSet, false );
                        aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    }
                    else if ( rLayout.bTitlePossible && bIsTitlePossible )
                    {
                        bIsTitlePossible = false;

                        ImplGetText();
                        TextObjBinary aTextObj( mXText, EPP_TEXTTYPE_Title, maFontCollection, static_cast<PPTExBulletProvider&>(*this) );
                        if ( ePageType == MASTER )
                        {
                            if ( mnTextSize )
                            {
                                OUString aUString( mXText->getString() );
                                sal_uInt16 nChar;

                                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                                mnShapeMasterTitle = mpPptEscherEx->GenerateShapeId();
                                mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle,
                                                         ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                                         mnShapeMasterTitle );
                                EscherPropertyContainer aPropertyOptions;
                                aPropertyOptions.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
                                mnTxId += 0x60;
                                aPropertyOptions.AddOpt( ESCHER_Prop_lTxid, mnTxId );
                                aPropertyOptions.AddOpt( ESCHER_Prop_AnchorText, ESCHER_AnchorMiddle );
                                aPropertyOptions.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
                                aPropertyOptions.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
                                aPropertyOptions.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
                                aPropertyOptions.CreateFillProperties( mXPropSet, true, mXShape );
                                sal_uInt32 nLineFlags = 0x90001;
                                if ( aPropertyOptions.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
                                    nLineFlags |= 0x10001;  // draw dashed line if no line
                                aPropertyOptions.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
                                mnTxId += 0x60;
                                aPropertyOptions.CreateTextProperties( mXPropSet, mnTxId );
                                ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt );
                                aPropertyOptions.Commit( *mpStrm );
                                mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
                                mpStrm->WriteInt16( maRect.Top() ).WriteInt16( maRect.Left() ).WriteInt16( maRect.Right() ).WriteInt16( maRect.Bottom() );      // top, left, right, bottom ????
                                mpPptEscherEx->OpenContainer( ESCHER_ClientData );
                                mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
                                mpStrm->WriteUInt32( 0 )                                                        // PlacementID
                                       .WriteUChar( EPP_PLACEHOLDER_MASTERTITLE )                               // PlaceHolderID
                                       .WriteUChar( 0 )                                                         // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                                       .WriteUInt16( 0 );                                                       // padword
                                mpPptEscherEx->CloseContainer();    // ESCHER_ClientData
                                mpPptEscherEx->OpenContainer( ESCHER_ClientTextbox );
                                mpPptEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
                                mpStrm->WriteUInt32( EPP_TEXTTYPE_Title );
                                mpPptEscherEx->AddAtom( mnTextSize << 1, EPP_TextCharsAtom );
                                const sal_Unicode* pString = aUString.getStr();
                                for ( sal_uInt32 i = 0; i < mnTextSize; i++ )
                                {
                                    nChar = pString[ i ];       // 0xa -> 0xb soft newline
                                    if ( nChar == 0xa )
                                        nChar++;                // 0xd -> 0xd hard newline
                                    mpStrm->WriteUInt16( nChar );
                                }
                                mpPptEscherEx->AddAtom( 6, EPP_BaseTextPropAtom );
                                mpStrm->WriteUInt32( mnTextSize + 1 ).WriteUInt16( 0 );
                                mpPptEscherEx->AddAtom( 10, EPP_TextSpecInfoAtom );
                                mpStrm->WriteUInt32( mnTextSize + 1 ).WriteUInt32( 1 ).WriteUInt16( 0 );
                                mpPptEscherEx->CloseContainer();    // ESCHER_ClientTextBox
                                mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
                            }
                            continue;
                        }
                        else
                        {
                            mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                            mnTextStyle = EPP_TEXTSTYLE_TITLE;
                            nPlaceHolderAtom = rLayout.nTypeOfTitle;
                            ImplCreateShape( ESCHER_ShpInst_Rectangle,
                                             ShapeFlag::HaveAnchor | ShapeFlag::HaveMaster,
                                             aSolverContainer );
                            aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterTitle );
                            aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                            mnTxId += 0x60;
                            aPropOpt.CreateTextProperties( mXPropSet, mnTxId );
                            ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt );
                            if ( mbEmptyPresObj )
                            {
                                sal_uInt32 nNoLineDrawDash = 0;
                                aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                                nNoLineDrawDash |= 0x10001;
                                aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                            }
                        }
                    }
                    else
                        mbPresObj = false;
                }
                if ( !mbPresObj )
                {
                    mType = "drawing.Text";
                    ImplCreateTextShape( aPropOpt, aSolverContainer, true );
                }
            }
            else if ( ( mType == "presentation.Outliner" ) || ( mType == "presentation.Subtitle" ) )
            {
                if ( mbPresObj )
                {
                    nOutlinerCount++;
                    if ( (rLayout.bOutlinerPossible && ( nOutlinerCount == 1 )) ||
                         (( rLayout.bSecOutlinerPossible && ( nOutlinerCount == 2 ) ) && ( nPrevTextStyle == EPP_TEXTSTYLE_BODY ))
                       )
                    {
                        ImplGetText();
                        TextObjBinary aTextObj( mXText, EPP_TEXTTYPE_Body, maFontCollection, static_cast<PPTExBulletProvider&>(*this) );
                        if ( ePageType == MASTER )
                        {
                            nPrevTextStyle = EPP_TEXTSTYLE_TITLE;
                            if ( mnTextSize )
                            {
                                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                                mnShapeMasterBody = mpPptEscherEx->GenerateShapeId();
                                mpPptEscherEx->AddShape( ESCHER_ShpInst_Rectangle,
                                                         ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                                         mnShapeMasterBody );
                                EscherPropertyContainer aPropOpt2;
                                aPropOpt2.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x50001 );
                                mnTxId += 0x60;
                                aPropOpt2.AddOpt( ESCHER_Prop_lTxid, mnTxId );
                                aPropOpt2.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x110001 );
                                aPropOpt2.AddOpt( ESCHER_Prop_lineColor, 0x8000001 );
                                aPropOpt2.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90001 );
                                aPropOpt2.AddOpt( ESCHER_Prop_shadowColor, 0x8000002 );
                                aPropOpt2.CreateFillProperties( mXPropSet, true, mXShape  );
                                sal_uInt32 nLineFlags = 0x90001;
                                if ( aPropOpt2.GetOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags ) )
                                    nLineFlags |= 0x10001;  // draw dashed line if no line
                                aPropOpt2.AddOpt( ESCHER_Prop_fNoLineDrawDash, nLineFlags );
                                mnTxId += 0x60;
                                aPropOpt2.CreateTextProperties( mXPropSet, mnTxId );
                                ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt2 );
                                aPropOpt2.Commit( *mpStrm );
                                mpPptEscherEx->AddAtom( 8, ESCHER_ClientAnchor );
                                mpStrm->WriteInt16( maRect.Top() ).WriteInt16( maRect.Left() ).WriteInt16( maRect.Right() ).WriteInt16( maRect.Bottom() );  // top, left, right, bottom ????
                                mpPptEscherEx->OpenContainer( ESCHER_ClientData );
                                mpPptEscherEx->AddAtom( 8, EPP_OEPlaceholderAtom );
                                sal_uInt8 PlaceHolderID = ( mType == "presentation.Subtitle") ? EPP_PLACEHOLDER_MASTERSUBTITLE:EPP_PLACEHOLDER_MASTERBODY;
                                mpStrm->WriteUInt32( 1 )                                                        // PlacementID
                                       .WriteUChar( PlaceHolderID )/*(sal_uInt8)EPP_PLACEHOLDER_MASTERBODY */                                   // PlaceHolderID
                                       .WriteUChar( 0 )                                                         // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                                       .WriteUInt16( 0 );                                                       // padword
                                mpPptEscherEx->CloseContainer();    // ESCHER_ClientData
                                mpPptEscherEx->OpenContainer( ESCHER_ClientTextbox );       // printf
                                mpPptEscherEx->AddAtom( 4, EPP_TextHeaderAtom );
                                if ( mType == "presentation.Subtitle")
                                    mpStrm->WriteUInt32( EPP_TEXTTYPE_CenterBody );
                                else
                                    mpStrm->WriteUInt32( EPP_TEXTTYPE_Body );
                                mnTextSize = aTextObj.Count();
                                aTextObj.Write( mpStrm.get() );
                                mpPptEscherEx->BeginAtom();
                                for ( sal_uInt32 i = 0; i < aTextObj.ParagraphCount() ; ++i )
                                {
                                    ParagraphObj* pPara = aTextObj.GetParagraph(i);
                                    mpStrm->WriteUInt32( pPara->CharacterCount() )
                                           .WriteUInt16( pPara->nDepth );
                                }
                                mpPptEscherEx->EndAtom( EPP_BaseTextPropAtom );
                                mpPptEscherEx->AddAtom( 10, EPP_TextSpecInfoAtom );
                                mpStrm->WriteUInt32( mnTextSize ).WriteUInt32( 1 ).WriteUInt16( 0 );

                                mpPptEscherEx->CloseContainer();    // ESCHER_ClientTextBox
                                mpPptEscherEx->CloseContainer();    // ESCHER_SpContainer
                            }
                            continue;
                        }
                        else
                        {
                            mnTextStyle = EPP_TEXTSTYLE_BODY;
                            nPlaceHolderAtom = rLayout.nTypeOfOutliner;
                            mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                            ImplCreateShape( ESCHER_ShpInst_Rectangle,
                                             ShapeFlag::HaveAnchor | ShapeFlag::HaveMaster,
                                             aSolverContainer );
                            aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                            aPropOpt.CreateFillProperties( mXPropSet, true, mXShape );
                            mnTxId += 0x60;
                            aPropOpt.CreateTextProperties( mXPropSet, mnTxId );
                            ImplAdjustFirstLineLineSpacing( aTextObj, aPropOpt );
                            if ( mbEmptyPresObj )
                            {
                                sal_uInt32 nNoLineDrawDash = 0;
                                aPropOpt.GetOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                                nNoLineDrawDash |= 0x10001;
                                aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, nNoLineDrawDash );
                            }
                        }
                    }
                    else
                        mbPresObj = false;
                }
                if ( !mbPresObj )
                {
                    if (ePageType == MASTER )
                    {
                        SdrObject* pObj = GetSdrObjectFromXShape( mXShape );
                        if (pObj && pObj->IsNotVisibleAsMaster())
                            continue;
                    }

                    mType = "drawing.Text";
                    ImplCreateTextShape( aPropOpt, aSolverContainer, true );
                }
            }
            else if ( ( mType == "drawing.Page" ) || ( mType == "presentation.Page" ) )
            {
                if ( ( ePageType == NOTICE ) && mbPresObj )
                {
                    if ( ImplCreatePresentationPlaceholder( bMasterPage, EPP_TEXTTYPE_Notes, EPP_PLACEHOLDER_MASTERNOTESSLIDEIMAGE ) )
                        continue;
                    else
                        nPlaceHolderAtom = EPP_PLACEHOLDER_NOTESSLIDEIMAGE;
                }
                ImplCreateTextShape( aPropOpt, aSolverContainer, true );
            }
            else if ( mType == "drawing.Frame" )
            {
                continue;
            }
            else if ( ( mType == "drawing.OLE2" ) || ( mType == "presentation.OLE2" )
                        || ( mType == "presentation.Chart" ) || ( mType == "presentation.Calc" )
                            || ( mType == "presentation.OrgChart" ) )
            {
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                if ( mbEmptyPresObj && ( ePageType == NORMAL ) )
                {
                    nPlaceHolderAtom = rLayout.nUsedObjectPlaceHolder;
                    ImplCreateShape( ESCHER_ShpInst_Rectangle,
                                     ShapeFlag::HaveAnchor | ShapeFlag::HaveMaster,
                                     aSolverContainer );
                    mnTxId += 0x60;
                    aPropOpt.AddOpt( ESCHER_Prop_lTxid, mnTxId );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x10001 );
                    aPropOpt.AddOpt( ESCHER_Prop_hspMaster, mnShapeMasterBody );
                }
                else
                {
                    mpExEmbed->WriteUInt32( 0xf | ( EPP_ExEmbed << 16 ) )
                               .WriteUInt32( 0 );               // Size of this container

                    sal_uInt32 nSize, nOldPos = mpExEmbed->Tell();

                    mpExEmbed->WriteUInt32( EPP_ExEmbedAtom << 16 )
                               .WriteUInt32( 8 )
                               .WriteUInt32( 0 )    // follow colorscheme : 0->do not follow
                                                    //                      1->follow colorscheme
                                                    //                      2->follow text and background scheme
                               .WriteUChar( 1 )     // (bool)set if embedded server can not be locked
                               .WriteUChar( 0 )     // (bool)do not need to send dimension
                               .WriteUChar( 0 )     // (bool)is object a world table
                               .WriteUChar( 0 );    // pad byte

                    std::unique_ptr<PPTExOleObjEntry> pE( new PPTExOleObjEntry( NORMAL_OLE_OBJECT, mpExEmbed->Tell() ) );
                    pE->xShape = mXShape;
                    maExOleObj.push_back( std::move(pE) );

                    mnExEmbed++;

                    sal_Int64 nAspect = css::embed::Aspects::MSOLE_CONTENT;
                    try
                    {
                        // try to get the aspect when available
                        css::uno::Reference< css::beans::XPropertySet > xShapeProps( mXShape, css::uno::UNO_QUERY_THROW );
                        xShapeProps->getPropertyValue("Aspect") >>= nAspect;
                    }
                    catch( css::uno::Exception& )
                    {}

                    mpExEmbed->WriteUInt32( 1 | ( EPP_ExOleObjAtom << 16 ) )
                               .WriteUInt32( 24 )
                               .WriteUInt32( nAspect )      // Aspect
                               .WriteUInt32( 0 )
                               .WriteUInt32( mnExEmbed )    // index to the persist table
                               .WriteUInt32( 0 )            // subtype
                               .WriteUInt32( 0 )
                               .WriteUInt32( 0x0012b600 );

                    nSize = mpExEmbed->Tell() - nOldPos;
                    mpExEmbed->Seek( nOldPos - 4 );
                    mpExEmbed->WriteUInt32( nSize );
                    mpExEmbed->Seek( STREAM_SEEK_TO_END );
                    nOlePictureId = mnExEmbed;

                    ShapeFlag nSpFlags = ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty;
                    if ( nOlePictureId )
                        nSpFlags |= ShapeFlag::OLEShape;
                    ImplCreateShape( ESCHER_ShpInst_PictureFrame, nSpFlags, aSolverContainer );
                    if ( aPropOpt.CreateOLEGraphicProperties( mXShape ) )
                        aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                    if ( nOlePictureId )
                        aPropOpt.AddOpt( ESCHER_Prop_pictureId, nOlePictureId );
                }
            }
            else if ( mType == "presentation.Header" )
            {
                if ( ImplCreatePresentationPlaceholder( bMasterPage, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERHEADER ) )
                    continue;
                else
                {
                    mbPresObj = false;
                    mType = "drawing.Text";
                    ImplCreateTextShape( aPropOpt, aSolverContainer, true );
                }
            }
            else if ( mType == "presentation.Footer" )
            {
                if ( ImplCreatePresentationPlaceholder( bMasterPage, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERFOOTER ) )
                    continue;
                else
                {
                    mbPresObj = false;
                    mType = "drawing.Text";
                    ImplCreateTextShape( aPropOpt, aSolverContainer, true );
                }
            }
            else if ( mType == "presentation.DateTime" )
            {
                if ( ImplCreatePresentationPlaceholder( bMasterPage, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERDATE ) )
                    continue;
                else
                {
                    mbPresObj = false;
                    mType = "drawing.Text";
                    ImplCreateTextShape( aPropOpt, aSolverContainer, true );
                }
            }
            else if ( mType == "presentation.SlideNumber" )
            {
                if ( ImplCreatePresentationPlaceholder( bMasterPage, EPP_TEXTTYPE_Other, EPP_PLACEHOLDER_MASTERSLIDENUMBER ) )
                    continue;
                else
                {
                    mbPresObj = false;
                    mType = "drawing.Text";
                    ImplCreateTextShape( aPropOpt, aSolverContainer, true );
                }
            }
            else if ( (mType.getLength() > 9) && (mType[8] == '3') && (mType[9] == 'D') )  // drawing.3D
            {
                // SceneObject, CubeObject, SphereObject, LatheObject, ExtrudeObject, PolygonObject
                if ( !ImplGetPropertyValue( "Bitmap" ) )
                    continue;

                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_PictureFrame,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );

                if ( aPropOpt.CreateGraphicProperties( mXPropSet, "Bitmap", false ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
            }
            else if ( mType == "drawing.Media" )
            {
                mnAngle = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_PictureFrame,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );
                if ( aPropOpt.CreateMediaGraphicProperties( mXShape ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
                css::uno::Any aAny;
                if ( PropValue::GetPropertyValue( aAny, mXPropSet, "MediaURL", true ) )
                {
                    OUString aMediaURL;
                    if ( (aAny >>= aMediaURL ) &&  !aMediaURL.isEmpty() )
                    {
                        // SJ: creating the Media RefObj
                        sal_uInt32 nRefId = ++mnExEmbed;

                        mpExEmbed->WriteUInt16( 0xf )
                                   .WriteUInt16( EPP_ExMCIMovie )       // PPT_PST_ExAviMovie
                                   .WriteUInt32( 0 );
                        sal_uInt32 nSize, nStart = mpExEmbed->Tell();
                        mpExEmbed->WriteUInt16( 0 )
                                   .WriteUInt16( EPP_ExObjRefAtom )
                                   .WriteUInt32( 4 )
                                   .WriteUInt32( nRefId );
                        mpExEmbed->WriteUInt16( 0xf )
                                   .WriteUInt16( EPP_ExVideo )
                                   .WriteUInt32( 0 );

                        mpExEmbed->WriteUInt16( 0 )
                                   .WriteUInt16( EPP_ExMediaAtom )
                                   .WriteUInt32( 8 )
                                   .WriteUInt32( nRefId )
                                   .WriteUInt16( 0 )
                                   .WriteUInt16( 0x435 );

                        sal_uInt16 i, nStringLen = static_cast<sal_uInt16>(aMediaURL.getLength());
                        mpExEmbed->WriteUInt32( EPP_CString << 16 ).WriteUInt32( nStringLen * 2 );
                        for ( i = 0; i < nStringLen; i++ )
                        {
                            sal_Unicode nChar = aMediaURL[ i ];
                            mpExEmbed->WriteUInt16( nChar );
                        }
                        nSize = mpExEmbed->Tell() - nStart;
                        mpExEmbed->SeekRel( - ( static_cast<sal_Int32>(nSize) + 4 ) );
                        mpExEmbed->WriteUInt32( nSize );    // size of PPT_PST_ExMCIMovie
                        mpExEmbed->SeekRel( 0x10 );
                        nSize -= 20;
                        mpExEmbed->WriteUInt32( nSize );    // PPT_PST_ExMediaAtom
                        mpExEmbed->SeekRel( nSize );

                        if ( !pClientData )
                            pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));
                        pClientData->WriteUInt16( 0 )
                                    .WriteUInt16( EPP_ExObjRefAtom )
                                    .WriteUInt32( 4 )
                                    .WriteUInt32( nRefId );
                        // write EPP_InteractiveInfo container for no_action
                        pClientData->WriteUInt32( ( EPP_InteractiveInfo << 16 ) | 0xf ).WriteUInt32( 24 );
                        pClientData->WriteUInt16( 0 )
                                    .WriteUInt16( EPP_InteractiveInfoAtom )
                                    .WriteUInt32( 16 )
                                    .WriteUInt32( 0 )
                                    .WriteUInt32( 0 )
                                    .WriteUChar( 6 )
                                    .WriteUChar( 0 )
                                    .WriteUChar( 0 )
                                    .WriteUChar( 0 )
                                    .WriteUInt32( 0 );
                    }
                }
            }
            else if ( (mType == "drawing.Table") || (mType == "presentation.Table") )
            {
                if ( eCa != css::presentation::ClickAction_NONE )
                {
                    SvMemoryStream aTmp(0x200, 0x200);
                    ImplWriteClickAction( aTmp, eCa, bMediaClickAction );
                }
                ImplCreateTable( mXShape, aSolverContainer, aPropOpt );
                continue;
            }
            else if ( mType == "drawing.dontknow" )
            {
                mnAngle = 0;
                mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
                ImplCreateShape( ESCHER_ShpInst_PictureFrame,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty,
                                 aSolverContainer );
                if ( aPropOpt.CreateGraphicProperties( mXPropSet, "MetaFile", false ) )
                    aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x800080 );
            }
            else
            {
                continue;
            }

            bool bClientData = ( bEffect || ( eCa != css::presentation::ClickAction_NONE ) ||
                                        nPlaceHolderAtom || nOlePictureId );
            if ( bClientData )
            {
                if ( nPlaceHolderAtom )
                {
                    sal_Int32 nPlacementID = -1;
                    if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
                        nPlacementID = nIndices++;
                    else
                    {
                        switch ( nPlaceHolderAtom )
                        {
                            default :
                            {
                                if ( nPlaceHolderAtom < 19 )
                                    break;
                                [[fallthrough]];
                            }
                            case EPP_PLACEHOLDER_NOTESBODY :
                            case EPP_PLACEHOLDER_MASTERDATE :
                            case EPP_PLACEHOLDER_NOTESSLIDEIMAGE :
                            case EPP_PLACEHOLDER_MASTERNOTESBODYIMAGE :
                                nPlacementID = nIndices++;
                        }
                    }
                    if ( !pClientData )
                        pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));

                    pClientData->WriteUInt32( EPP_OEPlaceholderAtom << 16 ).WriteUInt32( 8 )
                                .WriteInt32( nPlacementID )                // PlacementID
                                .WriteUChar( nPlaceHolderAtom ) // PlaceHolderID
                                .WriteUChar( 0 )                // Size of PlaceHolder ( 0 = FULL, 1 = HALF, 2 = QUARTER )
                                .WriteUInt16( 0 );              // padword
                }
                if ( nOlePictureId )
                {
                    if ( !pClientData )
                        pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));

                    pClientData->WriteUInt32( EPP_ExObjRefAtom << 16 ).WriteUInt32( 4 )
                                .WriteUInt32( nOlePictureId );
                    nOlePictureId = 0;
                }
                if ( bEffect && !pClientData )
                {
                    pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));
                }

                if ( eCa != css::presentation::ClickAction_NONE )
                {
                    if ( !pClientData )
                        pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));
                    ImplWriteClickAction( *pClientData, eCa, bMediaClickAction );
                }
            }
            if ( ( mnTextStyle == EPP_TEXTSTYLE_TITLE ) || ( mnTextStyle == EPP_TEXTSTYLE_BODY ) )
            {
                if ( !pClientTextBox )
                    pClientTextBox.reset(new SvMemoryStream( 0x200, 0x200 ));

                if ( !mbEmptyPresObj )
                {
                    if ( ( ePageType == NORMAL ) && !bMasterPage )
                    {
                        sal_uInt32 nTextType = EPP_TEXTTYPE_Body;
                        if ( mnTextStyle == EPP_TEXTSTYLE_BODY )
                        {
                            if ( bSecOutl )
                                nTextType = EPP_TEXTTYPE_HalfBody;
                            else if ( mType == "presentation.Subtitle" )
                                nTextType = EPP_TEXTTYPE_CenterBody;
                            bSecOutl = true;
                        }
                        else
                            nTextType = EPP_TEXTTYPE_Title;

                        TextRuleEntry aTextRule;
                        SvMemoryStream aExtBu( 0x200, 0x200 );
                        ImplGetText();
                        ImplWriteTextStyleAtom( *pClientTextBox, nTextType, nPObjects, &aTextRule, aExtBu, nullptr );
                        ImplWriteExtParaHeader( aExtBu, nPObjects++, nTextType, nPageNumber + 0x100 );
                        SvMemoryStream* pOut = aTextRule.pOut.get();
                        if ( pOut )
                        {
                            pClientTextBox->WriteBytes(pOut->GetData(), pOut->Tell());
                            aTextRule.pOut.reset();
                        }
                        if ( aExtBu.Tell() )
                        {
                            if ( !pClientData )
                                pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));
                            ImplProgTagContainer( pClientData.get(), &aExtBu );
                        }
                    }
                }
            }
            else
            {
                if ( !aPropOpt.IsFontWork() )
                {
                    if ( mnTextSize || ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
                    {
                        int nInstance2;
                        if ( ( nPlaceHolderAtom == EPP_PLACEHOLDER_MASTERDATE ) || ( nPlaceHolderAtom == EPP_PLACEHOLDER_NOTESBODY ) )
                            nInstance2 = 2;
                        else
                            nInstance2 = EPP_TEXTTYPE_Other;     // Text in a Shape

                        if ( !pClientTextBox )
                            pClientTextBox.reset(new SvMemoryStream( 0x200, 0x200 ));

                        SvMemoryStream  aExtBu( 0x200, 0x200 );
                        ImplWriteTextStyleAtom( *pClientTextBox, nInstance2, 0, nullptr, aExtBu, &aPropOpt );
                        if ( aExtBu.Tell() )
                        {
                            if ( !pClientData )
                                pClientData.reset(new SvMemoryStream( 0x200, 0x200 ));
                            ImplProgTagContainer( pClientData.get(), &aExtBu );
                        }
                    }
                    else if ( nPlaceHolderAtom >= 19 )
                    {
                        if ( !pClientTextBox )
                            pClientTextBox.reset(new SvMemoryStream( 12 ));

                        pClientTextBox->WriteUInt32( EPP_TextHeaderAtom << 16 ).WriteUInt32( 4 )
                                       .WriteUInt32( 7 );
                    }
                }
            }

            aPropOpt.CreateShadowProperties( mXPropSet );
            maRect.Justify();
            if ( mnAngle )
                ImplFlipBoundingBox( aPropOpt );
            aPropOpt.CreateShapeProperties( mXShape );
            aPropOpt.Commit( *mpStrm );
            if ( GetCurrentGroupLevel() > 0 )
                mpPptEscherEx->AddChildAnchor( maRect );
            else
                mpPptEscherEx->AddClientAnchor( maRect );

            if ( pClientData )
            {
                mpStrm->WriteUInt32( ( ESCHER_ClientData << 16 ) | 0xf )
                       .WriteUInt32( pClientData->Tell() );

                mpStrm->WriteBytes(pClientData->GetData(), pClientData->Tell());
                pClientData.reset();
            }
            if ( pClientTextBox )
            {
                mpStrm->WriteUInt32( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                       .WriteUInt32( pClientTextBox->Tell() );

                mpStrm->WriteBytes(pClientTextBox->GetData(), pClientTextBox->Tell());
                pClientTextBox.reset();
            }
            mpPptEscherEx->CloseContainer();      // ESCHER_SpContainer
        }
        nPrevTextStyle = mnTextStyle;

        if ( bAdditionalText )
        {
            bAdditionalText = false;

            css::uno::Any  aAny;
            EscherPropertyContainer     aPropOpt;
            mnAngle = ( PropValue::GetPropertyValue( aAny,
                mXPropSet, "RotateAngle", true ) )
                    ? *o3tl::doAccess<sal_Int32>(aAny)
                    : 0;

            aPropOpt.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
            aPropOpt.AddOpt( ESCHER_Prop_fNoFillHitTest, 0x100000 );
            if ( mType == "drawing.Line" )
            {
                double fDist = hypot( maRect.GetWidth(), maRect.GetHeight() );
                maRect = ::tools::Rectangle( Point( aTextRefPoint.X, aTextRefPoint.Y ),
                                        Point( static_cast<sal_Int32>( aTextRefPoint.X + fDist ), aTextRefPoint.Y - 1 ) );
                ImplCreateTextShape( aPropOpt, aSolverContainer, false );
                aPropOpt.AddOpt( ESCHER_Prop_FitTextToShape, 0x60006 );        // Size Shape To Fit Text
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                if ( mnAngle )
                    ImplFlipBoundingBox( aPropOpt );
            }
            else
            {
                ImplCreateTextShape( aPropOpt, aSolverContainer, false );
                if ( mnAngle < 0 )
                    mnAngle = ( 36000 + mnAngle ) % 36000;
                else
                    mnAngle = ( 36000 - ( mnAngle % 36000 ) );

                mnAngle *= 655;
                mnAngle += 0x8000;
                mnAngle &=~0xffff;  // round nAngle to full grad
                aPropOpt.AddOpt( ESCHER_Prop_Rotation, mnAngle );

                // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
                // mpPptEscherEx->SetGroupSnapRect( nGroupLevel, maRect );
                // mpPptEscherEx->SetGroupLogicRect( nGroupLevel, maRect );
            }
            if ( !pClientTextBox )
                pClientTextBox.reset(new SvMemoryStream( 0x200, 0x200 ));

            SvMemoryStream  aExtBu( 0x200, 0x200 );
            ImplWriteTextStyleAtom( *pClientTextBox, EPP_TEXTTYPE_Other, 0, nullptr, aExtBu, &aPropOpt );

            aPropOpt.CreateShapeProperties( mXShape );
            aPropOpt.Commit( *mpStrm );
            if ( GetCurrentGroupLevel() > 0 )
                mpPptEscherEx->AddChildAnchor( maRect );
            else
                mpPptEscherEx->AddClientAnchor( maRect );

            mpStrm->WriteUInt32( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                   .WriteUInt32( pClientTextBox->Tell() );

            mpStrm->WriteBytes(pClientTextBox->GetData(), pClientTextBox->Tell());
            pClientTextBox.reset();

            mpPptEscherEx->CloseContainer();  // ESCHER_SpContainer

            // #i119551# PPT does not support groups of polygons and text (MS patch KB2289187)
            // mpPptEscherEx->LeaveGroup();
        }
    }
    ClearGroupTable();                              // storing groups if any are still open, which should not be the case
    nGroups = GetGroupsClosed();
    for ( sal_uInt32 i = 0; i < nGroups; i++, mpPptEscherEx->LeaveGroup() ) ;
    mnPagesWritten++;
}

struct CellBorder
{
    sal_Int32                       mnPos;      // specifies the distance to the top/left position of the table
    table::BorderLine               maCellBorder;

    CellBorder() : mnPos ( 0 ) {};
};

bool PPTWriter::ImplCreateCellBorder( const CellBorder* pCellBorder, sal_Int32 nX1, sal_Int32 nY1, sal_Int32 nX2, sal_Int32 nY2)
{
    sal_Int32 nLineWidth = pCellBorder->maCellBorder.OuterLineWidth + pCellBorder->maCellBorder.InnerLineWidth;
    if ( nLineWidth )
    {
        nLineWidth *= 2;
        mnAngle = 0;
        mpPptEscherEx->OpenContainer( ESCHER_SpContainer );
        EscherPropertyContainer aPropOptSp;

        sal_uInt32 nId = mpPptEscherEx->GenerateShapeId();
        mpPptEscherEx->AddShape( ESCHER_ShpInst_Line,
                                 ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty | ShapeFlag::Child,
                                 nId );
        aPropOptSp.AddOpt( ESCHER_Prop_shapePath, ESCHER_ShapeComplex );
        aPropOptSp.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0xa0008 );
        aPropOptSp.AddOpt( ESCHER_Prop_fshadowObscured, 0x20000 );

        sal_uInt32 nBorderColor = pCellBorder->maCellBorder.Color & 0xff00;                 // green
        nBorderColor |= static_cast< sal_uInt8 >( pCellBorder->maCellBorder.Color ) << 16;  // red
        nBorderColor |= static_cast< sal_uInt8 >( pCellBorder->maCellBorder.Color >> 16 );  // blue
        aPropOptSp.AddOpt( ESCHER_Prop_lineColor, nBorderColor );

        aPropOptSp.AddOpt( ESCHER_Prop_lineWidth, nLineWidth * 360 );
        aPropOptSp.AddOpt( ESCHER_Prop_fc3DLightFace, 0x80000 );
        aPropOptSp.Commit( *mpStrm );
        mpPptEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
        mpStrm    ->WriteInt32( nX1 )
                   .WriteInt32( nY1 )
                   .WriteInt32( nX2 )
                   .WriteInt32( nY2 );
        mpPptEscherEx->CloseContainer();
        return true;
    }
    return false;
}

//get merged cell's width
static sal_Int32 GetCellRight( sal_Int32 nColumn,
    ::tools::Rectangle const & rect,
    std::vector< std::pair< sal_Int32, sal_Int32 > >& aColumns,
    uno::Reference< table::XMergeableCell > const & xCell )
{
    sal_Int32 nRight = aColumns[ nColumn ].first + aColumns[ nColumn ].second;
    for ( sal_Int32 nColumnSpan = 1; nColumnSpan < xCell->getColumnSpan(); nColumnSpan++ )
    {
        sal_uInt32 nC = nColumnSpan + nColumn;
        if ( nC < aColumns.size() )
            nRight += aColumns[ nC ].second;
        else
            nRight = rect.Right();
    }
    return nRight;
}
//get merged cell's height
static sal_Int32 GetCellBottom( sal_Int32 nRow,
    ::tools::Rectangle const & rect,
    std::vector< std::pair< sal_Int32, sal_Int32 > >& aRows,
    uno::Reference< table::XMergeableCell > const & xCell )
{
    sal_Int32 nBottom = aRows[nRow].first + aRows[nRow].second;
    for ( sal_Int32 nRowSpan = 1; nRowSpan < xCell->getRowSpan(); nRowSpan++ )
    {
        sal_uInt32 nR = nRowSpan + nRow;
        if ( nR < aRows.size() )
            nBottom += aRows[ nR ].second;
        else
            nBottom = rect.Bottom();
    }
    return nBottom;
}

void PPTWriter::WriteCString( SvStream& rSt, const OUString& rString, sal_uInt32 nInstance )
{
    sal_Int32 nLen = rString.getLength();
    if ( nLen )
    {
        rSt.WriteUInt32( ( nInstance << 4 ) | ( EPP_CString << 16 ) )
           .WriteUInt32( nLen << 1 );
        for ( sal_Int32 i = 0; i < nLen; i++ )
            rSt.WriteUInt16( rString[i] );
    }
}

class ContainerGuard
{
private:
    PptEscherEx* m_pPptEscherEx;
public:
    ContainerGuard(PptEscherEx* pPptEscherEx, sal_uInt16 nRecord)
        : m_pPptEscherEx(pPptEscherEx)
    {
        m_pPptEscherEx->OpenContainer(nRecord);
    }
    ~ContainerGuard()
    {
        m_pPptEscherEx->CloseContainer();
    }
};

void PPTWriter::ImplCreateTable( uno::Reference< drawing::XShape > const & rXShape, EscherSolverContainer& aSolverContainer,
                                EscherPropertyContainer& aPropOpt )
{
    try
    {
        uno::Reference< table::XTable > xTable;
        if ( mXPropSet->getPropertyValue( "Model" ) >>= xTable )
        {
            uno::Reference< table::XColumnRowRange > xColumnRowRange( xTable, uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xColumns( xColumnRowRange->getColumns(), uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xRows( xColumnRowRange->getRows(), uno::UNO_QUERY_THROW );
            sal_uInt16 nRowCount = static_cast< sal_uInt16 >( xRows->getCount() );
            sal_uInt16 nColumnCount = static_cast< sal_uInt16 >( xColumns->getCount() );

            std::vector< std::pair< sal_Int32, sal_Int32 > > aColumns;
            std::vector< std::pair< sal_Int32, sal_Int32 > > aRows;

            awt::Point aPosition( MapPoint( rXShape->getPosition() ) );
            sal_Int32 nPosition = aPosition.X;
            for ( sal_Int32 x = 0; x < nColumnCount; x++ )
            {
                uno::Reference< beans::XPropertySet > xPropSet( xColumns->getByIndex( x ), uno::UNO_QUERY_THROW );
                awt::Size aS( 0, 0 );
                xPropSet->getPropertyValue( "Width" ) >>= aS.Width;
                awt::Size aM( MapSize( aS ) );
                aColumns.emplace_back( nPosition, aM.Width );
                nPosition += aM.Width;
                if ( x == nColumnCount - 1  && nPosition != maRect.Right() )
                    maRect.SetRight( nPosition );
            }

            nPosition = aPosition.Y;
            for ( sal_Int32 y = 0; y < nRowCount; y++ )
            {
                uno::Reference< beans::XPropertySet > xPropSet( xRows->getByIndex( y ), uno::UNO_QUERY_THROW );
                awt::Size aS( 0, 0 );
                xPropSet->getPropertyValue( "Height" ) >>= aS.Height;
                awt::Size aM( MapSize( aS ) );
                aRows.emplace_back( nPosition, aM.Height );
                nPosition += aM.Height;
                if ( y == nRowCount - 1 && nPosition != maRect.Bottom())
                    maRect.SetBottom( nPosition );
            }
            std::unique_ptr<ContainerGuard> xSpgrContainer(new ContainerGuard(mpPptEscherEx.get(), ESCHER_SpgrContainer));
            std::unique_ptr<ContainerGuard> xSpContainer(new ContainerGuard(mpPptEscherEx.get(), ESCHER_SpContainer));
            mpPptEscherEx->AddAtom( 16, ESCHER_Spgr, 1 );
            mpStrm    ->WriteInt32( maRect.Left() ) // Bounding box for the grouped shapes to which they are attached
                       .WriteInt32( maRect.Top() )
                       .WriteInt32( maRect.Right() )
                       .WriteInt32( maRect.Bottom() );

            sal_uInt32 nShapeId = mpPptEscherEx->GenerateShapeId();
            mpPptEscherEx->AddShape( ESCHER_ShpInst_Min, ShapeFlag::HaveAnchor | ShapeFlag::Group, nShapeId );
            // TODO: check flags, comment does not match code // Flags: Group | Patriarch
            aSolverContainer.AddShape( rXShape, nShapeId );
            EscherPropertyContainer aPropOpt2;

            SvMemoryStream aMemStrm;
            aMemStrm.WriteUInt16( nRowCount )
                    .WriteUInt16( nRowCount )
                    .WriteUInt16( 4 );

            for( const auto& rRow : aRows )
                aMemStrm.WriteInt32( rRow.second );

            aPropOpt.AddOpt( ESCHER_Prop_LockAgainstGrouping, 0x1000100 );
            aPropOpt2.AddOpt( ESCHER_Prop_tableProperties, 1 );
            aPropOpt2.AddOpt(ESCHER_Prop_tableRowProperties, true, 0, aMemStrm);
            aPropOpt.CreateShapeProperties( rXShape );
            aPropOpt.Commit( *mpStrm );
            aPropOpt2.Commit( *mpStrm, 3, ESCHER_UDefProp );
            if ( GetCurrentGroupLevel() > 0 )
                mpPptEscherEx->AddChildAnchor( maRect );
            else
                mpPptEscherEx->AddClientAnchor( maRect );
            xSpContainer.reset(); //ESCHER_SpContainer

            uno::Reference< table::XCellRange > xCellRange( xTable, uno::UNO_QUERY_THROW );
            for( sal_Int32 nRow = 0; nRow < xRows->getCount(); nRow++ )
            {
                for( sal_Int32 nColumn = 0; nColumn < xColumns->getCount(); nColumn++ )
                {
                    uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nRow ), uno::UNO_QUERY_THROW );
                    if ( !xCell->isMerged() )
                    {
                        sal_Int32 nLeft   = aColumns[ nColumn ].first;
                        sal_Int32 nTop    = aRows[ nRow ].first;
                        sal_Int32 nRight  = GetCellRight( nColumn, maRect,aColumns,xCell );
                        sal_Int32 nBottom = GetCellBottom( nRow,  maRect,aRows,xCell );

                        mbFontIndependentLineSpacing = false;
                        mXPropSet.set( xCell, uno::UNO_QUERY_THROW );
                        mXText.set( xCell, uno::UNO_QUERY_THROW );
                        mnTextSize = mXText->getString().getLength();

                        css::uno::Any aAny;
                        if ( GetPropertyValue( aAny, mXPropSet, "FontIndependentLineSpacing", true ) )
                            aAny >>= mbFontIndependentLineSpacing;

                        EscherPropertyContainer aPropOptSp;
                        std::unique_ptr<ContainerGuard> xCellContainer(new ContainerGuard(mpPptEscherEx.get(), ESCHER_SpContainer));
                        ImplCreateShape( ESCHER_ShpInst_Rectangle,
                                         ShapeFlag::HaveAnchor | ShapeFlag::HaveShapeProperty | ShapeFlag::Child,
                                         aSolverContainer );
                        aPropOptSp.CreateFillProperties( mXPropSet, true );
                        aPropOptSp.AddOpt( ESCHER_Prop_fNoLineDrawDash, 0x90000 );
                        mnTxId += 0x60;
                        aPropOptSp.CreateTextProperties( mXPropSet, mnTxId );
                        aPropOptSp.AddOpt( ESCHER_Prop_WrapText, ESCHER_WrapSquare );

                        SvMemoryStream aClientTextBox( 0x200, 0x200 );
                        SvMemoryStream  aExtBu( 0x200, 0x200 );

                        ImplWriteTextStyleAtom( aClientTextBox, EPP_TEXTTYPE_Other, 0, nullptr, aExtBu, &aPropOptSp );

                        // need write client data for extend bullet
                        if ( aExtBu.Tell() )
                        {
                            std::unique_ptr<SvMemoryStream> pClientData(new SvMemoryStream( 0x200, 0x200 ));
                            ImplProgTagContainer( pClientData.get(), &aExtBu );
                            mpStrm->WriteUInt32( ( ESCHER_ClientData << 16 ) | 0xf )
                               .WriteUInt32( pClientData->Tell() );

                            mpStrm->WriteBytes(pClientData->GetData(), pClientData->Tell());
                        }

                        aPropOptSp.Commit( *mpStrm );
                        mpPptEscherEx->AddAtom( 16, ESCHER_ChildAnchor );
                        mpStrm    ->WriteInt32( nLeft )
                           .WriteInt32( nTop )
                           .WriteInt32( nRight )
                           .WriteInt32( nBottom );

                        mpStrm->WriteUInt32( ( ESCHER_ClientTextbox << 16 ) | 0xf )
                           .WriteUInt32( aClientTextBox.Tell() );

                        mpStrm->WriteBytes(aClientTextBox.GetData(), aClientTextBox.Tell());
                        xCellContainer.reset();
                    }
                }
            }

            // creating horz lines
            for( sal_Int32 nLine = 0; nLine < ( xRows->getCount() + 1 ); nLine++ )
            {
                for( sal_Int32 nColumn = 0; nColumn < xColumns->getCount(); nColumn++ )
                {
                    CellBorder aCellBorder;
                    aCellBorder.mnPos = aColumns[ nColumn ].first;
                    bool bTop = false;
                    //write nLine*nColumn cell's top border
                    if ( nLine < xRows->getCount() )
                    {   // top border
                        uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nLine ), uno::UNO_QUERY_THROW );
                        if ( !xCell->isMerged()  )
                        {
                            uno::Reference< beans::XPropertySet > xPropSet2( xCell, uno::UNO_QUERY_THROW );
                            table::BorderLine aBorderLine;
                            if ( xPropSet2->getPropertyValue( "TopBorder" ) >>= aBorderLine )
                                aCellBorder.maCellBorder = aBorderLine;
                            sal_Int32 nRight  = GetCellRight( nColumn, maRect,aColumns,xCell );
                            bTop = ImplCreateCellBorder( &aCellBorder, aCellBorder.mnPos,
                                aRows[ nLine ].first, nRight,  aRows[ nLine ].first );
                        }
                    }

                    //if nLine*nColumn cell's top border is empty, check (nLine-1)*nColumn cell's bottom border
                    //and write the last row's bottom border
                    if (( nLine && !bTop ) || (nLine == xRows->getCount()))
                    {   // bottom border
                        sal_Int32 nRow =  nLine;

                        while( nRow )
                        {   //find last no merged cell
                            uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn, nRow - 1 ), uno::UNO_QUERY_THROW );
                            if ( !xCell->isMerged()  )
                            {
                                sal_Int32 nRight  = GetCellRight( nColumn,  maRect,aColumns,xCell );
                                sal_Int32 nBottom = GetCellBottom( nRow - 1, maRect,aRows,xCell );
                                if ( nBottom == ( aRows[ nLine-1 ].first + aRows[ nLine-1 ].second ) )
                                {
                                    uno::Reference< table::XMergeableCell > xCellOwn( xCellRange->getCellByPosition( nColumn, nRow - 1 ), uno::UNO_QUERY_THROW );
                                    uno::Reference< beans::XPropertySet > xPropSet2( xCellOwn, uno::UNO_QUERY_THROW );
                                    table::BorderLine aBorderLine;
                                    if ( xPropSet2->getPropertyValue( "BottomBorder" ) >>= aBorderLine )
                                        aCellBorder.maCellBorder = aBorderLine;
                                    ImplCreateCellBorder( &aCellBorder, aCellBorder.mnPos,
                                        nBottom, nRight, nBottom);
                                }
                                nRow=0;
                            }
                            else
                                nRow--;
                        }
                    }
                }
            }

            // creating vertical lines
            for( sal_Int32 nLine = 0; nLine < ( xColumns->getCount() + 1 ); nLine++ )
            {
                for( sal_Int32 nRow = 0; nRow < xRows->getCount(); nRow++ )
                {

                    CellBorder aCellBorder;
                    aCellBorder.mnPos = aRows[ nRow].first;
                    bool bLeft = false;
                    if ( nLine < xColumns->getCount() )
                    {   // left border
                        uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nLine, nRow ), uno::UNO_QUERY_THROW );
                        if (!xCell->isMerged() )
                        {
                            uno::Reference< beans::XPropertySet > xCellSet( xCell, uno::UNO_QUERY_THROW );
                            table::BorderLine aBorderLine;
                            if ( xCellSet->getPropertyValue( "LeftBorder" ) >>= aBorderLine )
                                aCellBorder.maCellBorder = aBorderLine;
                            sal_Int32 nBottom = GetCellBottom( nRow, maRect, aRows,xCell );
                            bLeft = ImplCreateCellBorder( &aCellBorder, aColumns[nLine].first, aCellBorder.mnPos,
                                aColumns[nLine].first, nBottom );
                        }
                    }
                    if ( ( nLine && !bLeft )||(nLine == xColumns->getCount()))
                    {   // right border
                        sal_Int32 nColumn = nLine;
                        while ( nColumn )
                        {
                            uno::Reference< table::XMergeableCell > xCell( xCellRange->getCellByPosition( nColumn - 1, nRow ), uno::UNO_QUERY_THROW );
                            if (!xCell->isMerged() )
                            {
                                sal_Int32 nRight  = GetCellRight( nColumn-1, maRect, aColumns,xCell );
                                sal_Int32 nBottom = GetCellBottom( nRow,   maRect, aRows, xCell );
                                if ( nRight == (aColumns[nLine-1].first + aColumns[nLine-1].second) )
                                {
                                    uno::Reference< table::XMergeableCell > xCellOwn( xCellRange->getCellByPosition( nColumn - 1, nRow ), uno::UNO_QUERY_THROW );
                                    uno::Reference< beans::XPropertySet > xCellSet( xCellOwn, uno::UNO_QUERY_THROW );
                                    table::BorderLine aBorderLine;
                                    if ( xCellSet->getPropertyValue( "RightBorder" ) >>= aBorderLine )
                                        aCellBorder.maCellBorder = aBorderLine;
                                    ImplCreateCellBorder( &aCellBorder, nRight, aCellBorder.mnPos,
                                        nRight,  nBottom );
                                }
                                nColumn = 0;
                            }
                            else
                                nColumn --;
                        }
                    }
                }
            }

            xSpgrContainer.reset(); //ESCHER_SpgrContainer
        }
    }
    catch( uno::Exception& )
    {
    }
}

void TextObjBinary::Write( SvStream* pStrm )
{
    sal_uInt32 nSize, nPos = pStrm->Tell();
    pStrm->WriteUInt32( EPP_TextCharsAtom << 16 ).WriteUInt32( 0 );
    for ( sal_uInt32 i = 0; i < ParagraphCount(); ++i )
        GetParagraph(i)->Write( pStrm );
    nSize = pStrm->Tell() - nPos;
    pStrm->SeekRel( - ( static_cast<sal_Int32>(nSize) - 4 ) );
    pStrm->WriteUInt32( nSize - 8 );
    pStrm->SeekRel( nSize - 8 );
}

void TextObjBinary::WriteTextSpecInfo( SvStream* pStrm )
{
    sal_uInt32 nCharactersLeft( Count() );
    if ( nCharactersLeft < 1 )
        return;

    EscherExAtom aAnimationInfoAtom( *pStrm, EPP_TextSpecInfoAtom, 0, 0 );
    for ( sal_uInt32 i = 0; nCharactersLeft && i < ParagraphCount(); ++i )
    {
        ParagraphObj* pPtr = GetParagraph(i);
        for ( std::vector<std::unique_ptr<PortionObj> >::const_iterator it = pPtr->begin(); nCharactersLeft && it != pPtr->end(); ++it )
        {
            const PortionObj& rPortion = **it;
            sal_Int32 nPortionSize = rPortion.mnTextSize >= nCharactersLeft ? nCharactersLeft : rPortion.mnTextSize;
            sal_Int32 const nFlags = 7;
            nCharactersLeft -= nPortionSize;
            pStrm ->WriteUInt32( nPortionSize )
                   .WriteInt32( nFlags )
                   .WriteInt16( 1 )    // spellinfo -> needs rechecking
                   .WriteInt16( static_cast<sal_uInt16>(LanguageTag( rPortion.meCharLocale ).makeFallback().getLanguageType()) )
                   .WriteInt16( 0 );   // alt language
        }
    }
    if ( nCharactersLeft )
        pStrm->WriteUInt32( nCharactersLeft ).WriteInt32( 1 ).WriteInt16( 1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
