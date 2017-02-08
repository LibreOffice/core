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

#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gfxlink.hxx>
#include "vcl/dllapi.h"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"

#include <set>
#include <memory>
#include <map>

namespace vcl
{
struct PDFExtOutDevDataSync
{
    enum Action{    CreateNamedDest,
                    CreateDest,
                    CreateLink,
                    CreateScreen,
                    SetLinkDest,
                    SetLinkURL,
                    SetScreenURL,
                    SetScreenStream,
                    RegisterDest,
                    CreateOutlineItem,
                    CreateNote,
                    SetPageTransition,

                    BeginStructureElement,
                    EndStructureElement,
                    SetCurrentStructureElement,
                    SetStructureAttribute,
                    SetStructureAttributeNumerical,
                    SetStructureBoundingBox,
                    SetActualText,
                    SetAlternateText,
                    CreateControl,
                    BeginGroup,
                    EndGroupGfxLink
    };

    sal_uInt32  nIdx;
    Action      eAct;
};

struct PDFLinkDestination
{
    Rectangle               mRect;
    MapMode                 mMapMode;
    sal_Int32               mPageNr;
    PDFWriter::DestAreaType mAreaType;
};

struct GlobalSyncData
{
    std::deque< PDFExtOutDevDataSync::Action >  mActions;
    std::deque< MapMode >                       mParaMapModes;
    std::deque< Rectangle >                     mParaRects;
    std::deque< sal_Int32 >                     mParaInts;
    std::deque< sal_uInt32 >                    mParauInts;
    std::deque< OUString >                 mParaOUStrings;
    std::deque< PDFWriter::DestAreaType >       mParaDestAreaTypes;
    std::deque< PDFNote >                       mParaPDFNotes;
    std::deque< PDFWriter::PageTransition >     mParaPageTransitions;
    ::std::map< sal_Int32, PDFLinkDestination > mFutureDestinations;

    sal_Int32 GetMappedId();
    sal_Int32 GetMappedStructId( sal_Int32 );

    sal_Int32                   mCurId;
    std::vector< sal_Int32 >    mParaIds;
    std::vector< sal_Int32 >    mStructIdMap;

    sal_Int32                   mCurrentStructElement;
    std::vector< sal_Int32 >    mStructParents;
    GlobalSyncData() :
            mCurId ( 0 ),
            mCurrentStructElement( 0 )
    {
        mStructParents.push_back( 0 );
        mStructIdMap.push_back( 0 );
    }
    void PlayGlobalActions( PDFWriter& rWriter );
};

sal_Int32 GlobalSyncData::GetMappedId()
{
    sal_Int32 nLinkId = mParaInts.front();
    mParaInts.pop_front();

    /*  negative values are intentionally passed as invalid IDs
     *  e.g. to create a new top level outline item
     */
    if( nLinkId >= 0 )
    {
        if ( (sal_uInt32)nLinkId < mParaIds.size() )
            nLinkId = mParaIds[ nLinkId ];
        else
            nLinkId = -1;

        SAL_WARN_IF( nLinkId < 0, "vcl", "unmapped id in GlobalSyncData" );
    }

    return nLinkId;
}

sal_Int32 GlobalSyncData::GetMappedStructId( sal_Int32 nStructId )
{
    if ( (sal_uInt32)nStructId < mStructIdMap.size() )
        nStructId = mStructIdMap[ nStructId ];
    else
        nStructId = -1;

    SAL_WARN_IF( nStructId < 0, "vcl", "unmapped structure id in GlobalSyncData" );

    return nStructId;
}

void GlobalSyncData::PlayGlobalActions( PDFWriter& rWriter )
{
    for (std::deque< PDFExtOutDevDataSync::Action >::const_iterator aIter( mActions.begin() ), aEnd( mActions.end() ) ;
         aIter != aEnd ; ++aIter)
    {
        switch( *aIter )
        {
            case PDFExtOutDevDataSync::CreateNamedDest : //i56629
            {
                 rWriter.Push( PushFlags::MAPMODE );
                rWriter.SetMapMode( mParaMapModes.front() );
                mParaMapModes.pop_front();
                mParaIds.push_back( rWriter.CreateNamedDest( mParaOUStrings.front(), mParaRects.front(), mParaInts.front(), mParaDestAreaTypes.front() ) );
                mParaOUStrings.pop_front();
                mParaRects.pop_front();
                mParaInts.pop_front();
                mParaDestAreaTypes.pop_front();
                rWriter.Pop();
            }
            break;
            case PDFExtOutDevDataSync::CreateDest :
            {
                rWriter.Push( PushFlags::MAPMODE );
                rWriter.SetMapMode( mParaMapModes.front() );
                mParaMapModes.pop_front();
                mParaIds.push_back( rWriter.CreateDest( mParaRects.front(), mParaInts.front(), mParaDestAreaTypes.front() ) );
                mParaRects.pop_front();
                mParaInts.pop_front();
                mParaDestAreaTypes.pop_front();
                rWriter.Pop();
            }
            break;
            case PDFExtOutDevDataSync::CreateLink :
            {
                rWriter.Push( PushFlags::MAPMODE );
                rWriter.SetMapMode( mParaMapModes.front() );
                mParaMapModes.pop_front();
                mParaIds.push_back( rWriter.CreateLink( mParaRects.front(), mParaInts.front() ) );
                // resolve LinkAnnotation structural attribute
                rWriter.SetLinkPropertyID( mParaIds.back(), sal_Int32(mParaIds.size()-1) );
                mParaRects.pop_front();
                mParaInts.pop_front();
                rWriter.Pop();
            }
            break;
            case PDFExtOutDevDataSync::CreateScreen:
            {
                rWriter.Push(PushFlags::MAPMODE);
                rWriter.SetMapMode(mParaMapModes.front());
                mParaMapModes.pop_front();
                mParaIds.push_back(rWriter.CreateScreen(mParaRects.front(), mParaInts.front()));
                mParaRects.pop_front();
                mParaInts.pop_front();
                rWriter.Pop();
            }
            break;
            case PDFExtOutDevDataSync::SetLinkDest :
            {
                sal_Int32 nLinkId = GetMappedId();
                sal_Int32 nDestId = GetMappedId();
                rWriter.SetLinkDest( nLinkId, nDestId );
            }
            break;
            case PDFExtOutDevDataSync::SetLinkURL :
            {
                sal_Int32 nLinkId = GetMappedId();
                rWriter.SetLinkURL( nLinkId, mParaOUStrings.front() );
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetScreenURL:
            {
                sal_Int32 nScreenId = GetMappedId();
                rWriter.SetScreenURL(nScreenId, mParaOUStrings.front());
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetScreenStream:
            {
                sal_Int32 nScreenId = GetMappedId();
                rWriter.SetScreenStream(nScreenId, mParaOUStrings.front());
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::RegisterDest :
            {
                const sal_Int32 nDestId = mParaInts.front();
                mParaInts.pop_front();
                OSL_ENSURE( mFutureDestinations.find( nDestId ) != mFutureDestinations.end(),
                    "GlobalSyncData::PlayGlobalActions: DescribeRegisteredRequest has not been called for that destination!" );

                PDFLinkDestination& rDest = mFutureDestinations[ nDestId ];

                rWriter.Push( PushFlags::MAPMODE );
                rWriter.SetMapMode( rDest.mMapMode );
                mParaIds.push_back( rWriter.RegisterDestReference( nDestId, rDest.mRect, rDest.mPageNr, rDest.mAreaType ) );
                rWriter.Pop();
            }
            break;
            case PDFExtOutDevDataSync::CreateOutlineItem :
            {
                sal_Int32 nParent = GetMappedId();
                sal_Int32 nLinkId = GetMappedId();
                mParaIds.push_back( rWriter.CreateOutlineItem( nParent, mParaOUStrings.front(), nLinkId ) );
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::CreateNote :
            {
                rWriter.Push( PushFlags::MAPMODE );
                rWriter.SetMapMode( mParaMapModes.front() );
                rWriter.CreateNote( mParaRects.front(), mParaPDFNotes.front(), mParaInts.front() );
                mParaMapModes.pop_front();
                mParaRects.pop_front();
                mParaPDFNotes.pop_front();
                mParaInts.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetPageTransition :
            {
                rWriter.SetPageTransition( mParaPageTransitions.front(), mParauInts.front(), mParaInts.front() );
                mParaPageTransitions.pop_front();
                mParauInts.pop_front();
                mParaInts.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::BeginStructureElement:
            case PDFExtOutDevDataSync::EndStructureElement:
            case PDFExtOutDevDataSync::SetCurrentStructureElement:
            case PDFExtOutDevDataSync::SetStructureAttribute:
            case PDFExtOutDevDataSync::SetStructureAttributeNumerical:
            case PDFExtOutDevDataSync::SetStructureBoundingBox:
            case PDFExtOutDevDataSync::SetActualText:
            case PDFExtOutDevDataSync::SetAlternateText:
            case PDFExtOutDevDataSync::CreateControl:
            case PDFExtOutDevDataSync::BeginGroup:
            case PDFExtOutDevDataSync::EndGroupGfxLink:
                break;
        }
    }
}

struct PageSyncData
{
    std::deque< PDFExtOutDevDataSync >              mActions;
    std::deque< Rectangle >                         mParaRects;
    std::deque< sal_Int32 >                         mParaInts;
    std::deque< OUString >                     mParaOUStrings;
    std::deque< PDFWriter::StructElement >          mParaStructElements;
    std::deque< PDFWriter::StructAttribute >        mParaStructAttributes;
    std::deque< PDFWriter::StructAttributeValue >   mParaStructAttributeValues;
    std::deque< Graphic >                           mGraphics;
    Graphic                                         mCurrentGraphic;
    std::deque< std::shared_ptr< PDFWriter::AnyWidget > >
                                                    mControls;
    GlobalSyncData*                                 mpGlobalData;

    bool                                        mbGroupIgnoreGDIMtfActions;


    explicit PageSyncData( GlobalSyncData* pGlobal )
        : mbGroupIgnoreGDIMtfActions ( false )
    { mpGlobalData = pGlobal; }

    void PushAction( const OutputDevice& rOutDev, const PDFExtOutDevDataSync::Action eAct );
    bool PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction, const PDFExtOutDevData& rOutDevData );
};

void PageSyncData::PushAction( const OutputDevice& rOutDev, const PDFExtOutDevDataSync::Action eAct )
{
    GDIMetaFile* pMtf = rOutDev.GetConnectMetaFile();
    SAL_WARN_IF( !pMtf, "vcl", "PageSyncData::PushAction -> no ConnectMetaFile !!!" );

    PDFExtOutDevDataSync aSync;
    aSync.eAct = eAct;
    if ( pMtf )
        aSync.nIdx = pMtf->GetActionSize();
    else
        aSync.nIdx = 0x7fffffff;    // sync not possible
    mActions.push_back( aSync );
}
bool PageSyncData::PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction, const PDFExtOutDevData& rOutDevData )
{
    bool bRet = false;
    if ( mActions.size() && ( mActions.front().nIdx == rCurGDIMtfAction ) )
    {
        bRet = true;
        PDFExtOutDevDataSync aDataSync = mActions.front();
        mActions.pop_front();
        switch( aDataSync.eAct )
        {
            case PDFExtOutDevDataSync::BeginStructureElement :
            {
                sal_Int32 nNewEl = rWriter.BeginStructureElement( mParaStructElements.front(), mParaOUStrings.front() ) ;
                mParaStructElements.pop_front();
                mParaOUStrings.pop_front();
                mpGlobalData->mStructIdMap.push_back( nNewEl );
            }
            break;
            case PDFExtOutDevDataSync::EndStructureElement :
            {
                rWriter.EndStructureElement();
            }
            break;
            case PDFExtOutDevDataSync::SetCurrentStructureElement:
            {
                rWriter.SetCurrentStructureElement( mpGlobalData->GetMappedStructId( mParaInts.front() ) );
                mParaInts.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetStructureAttribute :
            {
                rWriter.SetStructureAttribute( mParaStructAttributes.front(), mParaStructAttributeValues.front() );
                mParaStructAttributeValues.pop_front();
                mParaStructAttributes.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetStructureAttributeNumerical :
            {
                rWriter.SetStructureAttributeNumerical( mParaStructAttributes.front(), mParaInts.front() );
                mParaStructAttributes.pop_front();
                mParaInts.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetStructureBoundingBox :
            {
                rWriter.SetStructureBoundingBox( mParaRects.front() );
                mParaRects.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetActualText :
            {
                rWriter.SetActualText( mParaOUStrings.front() );
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetAlternateText :
            {
                rWriter.SetAlternateText( mParaOUStrings.front() );
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::CreateControl:
            {
                std::shared_ptr< PDFWriter::AnyWidget > pControl( mControls.front() );
                SAL_WARN_IF( !pControl.get(), "vcl", "PageSyncData::PlaySyncPageAct: invalid widget!" );
                if ( pControl.get() )
                    rWriter.CreateControl( *pControl );
                mControls.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::BeginGroup :
            {
                /* first determining if this BeginGroup is starting a GfxLink,
                   by searching for a EndGroup or a EndGroupGfxLink */
                mbGroupIgnoreGDIMtfActions = false;
                std::deque< PDFExtOutDevDataSync >::iterator aBeg = mActions.begin();
                std::deque< PDFExtOutDevDataSync >::iterator aEnd = mActions.end();
                while ( aBeg != aEnd )
                {
                    if ( aBeg->eAct == PDFExtOutDevDataSync::EndGroupGfxLink )
                    {
                        Graphic& rGraphic = mGraphics.front();
                        if ( rGraphic.IsLink() )
                        {
                            GfxLinkType eType = rGraphic.GetLink().GetType();
                            if ( eType == GfxLinkType::NativeJpg && mParaRects.size() >= 2 )
                            {
                                mbGroupIgnoreGDIMtfActions =
                                rOutDevData.HasAdequateCompression(
                                        rGraphic, mParaRects[0], mParaRects[1]);
                                if ( !mbGroupIgnoreGDIMtfActions )
                                    mCurrentGraphic = rGraphic;
                            }
                            else if ( eType == GfxLinkType::NativePng && mParaRects.size() >= 2 )
                            {
                                if ( rOutDevData.HasAdequateCompression(rGraphic, mParaRects[0], mParaRects[1]) )
                                    mCurrentGraphic = rGraphic;
                            }
                        }
                        break;
                    }
                    ++aBeg;
                }
            }
            break;
            case PDFExtOutDevDataSync::EndGroupGfxLink :
            {
                Rectangle aOutputRect, aVisibleOutputRect;
                Graphic   aGraphic( mGraphics.front() );

                mGraphics.pop_front();
                sal_Int32 nTransparency = mParaInts.front();
                mParaInts.pop_front();
                aOutputRect = mParaRects.front();
                mParaRects.pop_front();
                aVisibleOutputRect = mParaRects.front();
                mParaRects.pop_front();

                if ( mbGroupIgnoreGDIMtfActions )
                {
                    bool bClippingNeeded = ( aOutputRect != aVisibleOutputRect ) && !aVisibleOutputRect.IsEmpty();

                    GfxLink   aGfxLink( aGraphic.GetLink() );
                    if ( aGfxLink.GetType() == GfxLinkType::NativeJpg )
                    {
                        if ( bClippingNeeded )
                        {
                            rWriter.Push();
                            basegfx::B2DPolyPolygon aRect( basegfx::tools::createPolygonFromRect(
                                basegfx::B2DRectangle( aVisibleOutputRect.Left(), aVisibleOutputRect.Top(),
                                                       aVisibleOutputRect.Right(), aVisibleOutputRect.Bottom() ) ) );
                            rWriter.SetClipRegion( aRect);
                        }

                        Bitmap aMask;
                        if (nTransparency)
                        {
                            AlphaMask aAlphaMask(aGraphic.GetSizePixel());
                            aAlphaMask.Erase(nTransparency);
                            aMask = aAlphaMask.GetBitmap();
                        }

                        SvMemoryStream aTmp;
                        const sal_uInt8* pData = aGfxLink.GetData();
                        sal_uInt32 nBytes = aGfxLink.GetDataSize();
                        if( pData && nBytes )
                        {
                            aTmp.WriteBytes( pData, nBytes );
                            rWriter.DrawJPGBitmap( aTmp, aGraphic.GetBitmap().GetBitCount() > 8, aGraphic.GetSizePixel(), aOutputRect, aMask );
                        }

                        if ( bClippingNeeded )
                            rWriter.Pop();
                    }
                    mbGroupIgnoreGDIMtfActions = false;
                }
                mCurrentGraphic.Clear();
            }
            break;
            case PDFExtOutDevDataSync::CreateNamedDest:
            case PDFExtOutDevDataSync::CreateDest:
            case PDFExtOutDevDataSync::CreateLink:
            case PDFExtOutDevDataSync::CreateScreen:
            case PDFExtOutDevDataSync::SetLinkDest:
            case PDFExtOutDevDataSync::SetLinkURL:
            case PDFExtOutDevDataSync::SetScreenURL:
            case PDFExtOutDevDataSync::SetScreenStream:
            case PDFExtOutDevDataSync::RegisterDest:
            case PDFExtOutDevDataSync::CreateOutlineItem:
            case PDFExtOutDevDataSync::CreateNote:
            case PDFExtOutDevDataSync::SetPageTransition:
                break;
        }
    }
    else if ( mbGroupIgnoreGDIMtfActions )
    {
        rCurGDIMtfAction++;
        bRet = true;
    }
    return bRet;
}

PDFExtOutDevData::PDFExtOutDevData( const OutputDevice& rOutDev ) :
    mrOutDev                ( rOutDev ),
    mbTaggedPDF             ( false ),
    mbExportNotes           ( true ),
    mbExportNotesPages      ( false ),
    mbTransitionEffects     ( true ),
    mbUseLosslessCompression( true ),
    mbReduceImageResolution ( false ),
    mbExportFormFields      ( false ),
    mbExportBookmarks       ( false ),
    mbExportHiddenSlides    ( false ),
    mbExportNDests          ( false ),
    mnFormsFormat           ( 0 ),
    mnPage                  ( -1 ),
    mnCompressionQuality    ( 90 ),
    mnMaxImageResolution    ( 300 ),
    mpPageSyncData          ( nullptr ),
    mpGlobalSyncData        ( new GlobalSyncData() )
{
    mpPageSyncData = new PageSyncData( mpGlobalSyncData );
}

PDFExtOutDevData::~PDFExtOutDevData()
{
    delete mpPageSyncData;
    delete mpGlobalSyncData;
}

const Graphic& PDFExtOutDevData::GetCurrentGraphic() const
{
    return mpPageSyncData->mCurrentGraphic;
}

void PDFExtOutDevData::SetDocumentLocale( const css::lang::Locale& rLoc )
{
    maDocLocale = rLoc;
}
void PDFExtOutDevData::SetCurrentPageNumber( const sal_Int32 nPage )
{
    mnPage = nPage;
}
void PDFExtOutDevData::SetIsLosslessCompression( const bool bUseLosslessCompression )
{
    mbUseLosslessCompression = bUseLosslessCompression;
}
void PDFExtOutDevData::SetCompressionQuality( const sal_Int32 nQuality )
{
    mnCompressionQuality = nQuality;
}
void PDFExtOutDevData::SetMaxImageResolution( const sal_Int32 nMaxImageResolution )
{
    mnMaxImageResolution = nMaxImageResolution;
}
void PDFExtOutDevData::SetIsReduceImageResolution( const bool bReduceImageResolution )
{
    mbReduceImageResolution = bReduceImageResolution;
}
void PDFExtOutDevData::SetIsExportNotes( const bool bExportNotes )
{
    mbExportNotes = bExportNotes;
}
void PDFExtOutDevData::SetIsExportNotesPages( const bool bExportNotesPages )
{
    mbExportNotesPages = bExportNotesPages;
}
void PDFExtOutDevData::SetIsExportTaggedPDF( const bool bTaggedPDF )
{
    mbTaggedPDF = bTaggedPDF;
}
void PDFExtOutDevData::SetIsExportTransitionEffects( const bool bTransitionEffects )
{
    mbTransitionEffects = bTransitionEffects;
}
void PDFExtOutDevData::SetIsExportFormFields( const bool bExportFomtFields )
{
    mbExportFormFields = bExportFomtFields;
}
void PDFExtOutDevData::SetFormsFormat( const sal_Int32 nFormsFormat )
{
    mnFormsFormat = nFormsFormat;
}
void PDFExtOutDevData::SetIsExportBookmarks( const bool bExportBookmarks )
{
    mbExportBookmarks = bExportBookmarks;
}
void PDFExtOutDevData::SetIsExportHiddenSlides( const bool bExportHiddenSlides )
{
    mbExportHiddenSlides = bExportHiddenSlides;
}
void PDFExtOutDevData::SetIsExportNamedDestinations( const bool bExportNDests )
{
    mbExportNDests = bExportNDests;
}
void PDFExtOutDevData::ResetSyncData()
{
    *mpPageSyncData = PageSyncData( mpGlobalSyncData );
}
bool PDFExtOutDevData::PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rIdx )
{
    return mpPageSyncData->PlaySyncPageAct( rWriter, rIdx, *this );
}
void PDFExtOutDevData::PlayGlobalActions( PDFWriter& rWriter )
{
    mpGlobalSyncData->PlayGlobalActions( rWriter );
}

/* global actions, synchronisation to the recorded metafile isn't needed,
   all actions will be played after the last page was recorded
*/
//--->i56629
sal_Int32 PDFExtOutDevData::CreateNamedDest(const OUString& sDestName,  const Rectangle& rRect, sal_Int32 nPageNr )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::CreateNamedDest );
    mpGlobalSyncData->mParaOUStrings.push_back( sDestName );
    mpGlobalSyncData->mParaRects.push_back( rRect );
    mpGlobalSyncData->mParaMapModes.push_back( mrOutDev.GetMapMode() );
    mpGlobalSyncData->mParaInts.push_back( nPageNr == -1 ? mnPage : nPageNr );
    mpGlobalSyncData->mParaDestAreaTypes.push_back( PDFWriter::DestAreaType::XYZ );

    return mpGlobalSyncData->mCurId++;
}
//<---i56629
sal_Int32 PDFExtOutDevData::RegisterDest()
{
    const sal_Int32 nLinkDestID = mpGlobalSyncData->mCurId++;
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::RegisterDest );
    mpGlobalSyncData->mParaInts.push_back( nLinkDestID );

    return nLinkDestID;
}
void PDFExtOutDevData::DescribeRegisteredDest( sal_Int32 nDestId, const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    OSL_PRECOND( nDestId != -1, "PDFExtOutDevData::DescribeRegisteredDest: invalid destination Id!" );
    PDFLinkDestination aLinkDestination;
    aLinkDestination.mRect = rRect;
    aLinkDestination.mMapMode = mrOutDev.GetMapMode();
    aLinkDestination.mPageNr = nPageNr == -1 ? mnPage : nPageNr;
    aLinkDestination.mAreaType = eType;
    mpGlobalSyncData->mFutureDestinations[ nDestId ] = aLinkDestination;
}
sal_Int32 PDFExtOutDevData::CreateDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::CreateDest );
    mpGlobalSyncData->mParaRects.push_back( rRect );
    mpGlobalSyncData->mParaMapModes.push_back( mrOutDev.GetMapMode() );
    mpGlobalSyncData->mParaInts.push_back( nPageNr == -1 ? mnPage : nPageNr );
    mpGlobalSyncData->mParaDestAreaTypes.push_back( eType );
    return mpGlobalSyncData->mCurId++;
}
sal_Int32 PDFExtOutDevData::CreateLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::CreateLink );
    mpGlobalSyncData->mParaRects.push_back( rRect );
    mpGlobalSyncData->mParaMapModes.push_back( mrOutDev.GetMapMode() );
    mpGlobalSyncData->mParaInts.push_back( nPageNr == -1 ? mnPage : nPageNr );
    return mpGlobalSyncData->mCurId++;
}

sal_Int32 PDFExtOutDevData::CreateScreen(const Rectangle& rRect, sal_Int32 nPageNr)
{
    mpGlobalSyncData->mActions.push_back(PDFExtOutDevDataSync::CreateScreen);
    mpGlobalSyncData->mParaRects.push_back(rRect);
    mpGlobalSyncData->mParaMapModes.push_back(mrOutDev.GetMapMode());
    mpGlobalSyncData->mParaInts.push_back(nPageNr);
    return mpGlobalSyncData->mCurId++;
}

sal_Int32 PDFExtOutDevData::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::SetLinkDest );
    mpGlobalSyncData->mParaInts.push_back( nLinkId );
    mpGlobalSyncData->mParaInts.push_back( nDestId );
    return 0;
}
sal_Int32 PDFExtOutDevData::SetLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::SetLinkURL );
    mpGlobalSyncData->mParaInts.push_back( nLinkId );
    mpGlobalSyncData->mParaOUStrings.push_back( rURL );
    return 0;
}

void PDFExtOutDevData::SetScreenURL(sal_Int32 nScreenId, const OUString& rURL)
{
    mpGlobalSyncData->mActions.push_back(PDFExtOutDevDataSync::SetScreenURL);
    mpGlobalSyncData->mParaInts.push_back(nScreenId);
    mpGlobalSyncData->mParaOUStrings.push_back(rURL);
}

void PDFExtOutDevData::SetScreenStream(sal_Int32 nScreenId, const OUString& rURL)
{
    mpGlobalSyncData->mActions.push_back(PDFExtOutDevDataSync::SetScreenStream);
    mpGlobalSyncData->mParaInts.push_back(nScreenId);
    mpGlobalSyncData->mParaOUStrings.push_back(rURL);
}

sal_Int32 PDFExtOutDevData::CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::CreateOutlineItem );
    mpGlobalSyncData->mParaInts.push_back( nParent );
    mpGlobalSyncData->mParaOUStrings.push_back( rText );
    mpGlobalSyncData->mParaInts.push_back( nDestID );
    return mpGlobalSyncData->mCurId++;
}
void PDFExtOutDevData::CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::CreateNote );
    mpGlobalSyncData->mParaRects.push_back( rRect );
    mpGlobalSyncData->mParaMapModes.push_back( mrOutDev.GetMapMode() );
    mpGlobalSyncData->mParaPDFNotes.push_back( rNote );
    mpGlobalSyncData->mParaInts.push_back( nPageNr == -1 ? mnPage : nPageNr );
}
void PDFExtOutDevData::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::SetPageTransition );
    mpGlobalSyncData->mParaPageTransitions.push_back( eType );
    mpGlobalSyncData->mParauInts.push_back( nMilliSec );
    mpGlobalSyncData->mParaInts.push_back( mnPage );
}

/* local (page), actions have to be played synchronously to the actions of
   of the recorded metafile (created by each xRenderable->render()) */
   sal_Int32 PDFExtOutDevData::BeginStructureElement( PDFWriter::StructElement eType, const OUString& rAlias )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::BeginStructureElement );
    mpPageSyncData->mParaStructElements.push_back( eType );
    mpPageSyncData->mParaOUStrings.push_back( rAlias );
    // need a global id
    sal_Int32 nNewId = mpGlobalSyncData->mStructParents.size();
    mpGlobalSyncData->mStructParents.push_back( mpGlobalSyncData->mCurrentStructElement );
    mpGlobalSyncData->mCurrentStructElement = nNewId;
    return nNewId;
}
void PDFExtOutDevData::EndStructureElement()
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::EndStructureElement );
    mpGlobalSyncData->mCurrentStructElement = mpGlobalSyncData->mStructParents[ mpGlobalSyncData->mCurrentStructElement ];
}
bool PDFExtOutDevData::SetCurrentStructureElement( sal_Int32 nStructId )
{
    bool bSuccess = false;
    if( sal_uInt32(nStructId) < mpGlobalSyncData->mStructParents.size() )
    {
        mpGlobalSyncData->mCurrentStructElement = nStructId;
        mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::SetCurrentStructureElement );
        mpPageSyncData->mParaInts.push_back( nStructId );
        bSuccess = true;
    }
    return bSuccess;
}
sal_Int32 PDFExtOutDevData::GetCurrentStructureElement()
{
    return mpGlobalSyncData->mCurrentStructElement;
}
bool PDFExtOutDevData::SetStructureAttribute( PDFWriter::StructAttribute eAttr, PDFWriter::StructAttributeValue eVal )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::SetStructureAttribute );
    mpPageSyncData->mParaStructAttributes.push_back( eAttr );
    mpPageSyncData->mParaStructAttributeValues.push_back( eVal );
    return true;
}
bool PDFExtOutDevData::SetStructureAttributeNumerical( PDFWriter::StructAttribute eAttr, sal_Int32 nValue )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::SetStructureAttributeNumerical );
    mpPageSyncData->mParaStructAttributes.push_back( eAttr );
    mpPageSyncData->mParaInts.push_back( nValue );
    return true;
}
void PDFExtOutDevData::SetStructureBoundingBox( const Rectangle& rRect )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::SetStructureBoundingBox );
    mpPageSyncData->mParaRects.push_back( rRect );
}
void PDFExtOutDevData::SetActualText( const OUString& rText )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::SetActualText );
    mpPageSyncData->mParaOUStrings.push_back( rText );
}
void PDFExtOutDevData::SetAlternateText( const OUString& rText )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::SetAlternateText );
    mpPageSyncData->mParaOUStrings.push_back( rText );
}

void PDFExtOutDevData::CreateControl( const PDFWriter::AnyWidget& rControlType )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::CreateControl );

    std::shared_ptr< PDFWriter::AnyWidget > pClone( rControlType.Clone() );
    mpPageSyncData->mControls.push_back( pClone );
}

void PDFExtOutDevData::BeginGroup()
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::BeginGroup );
}

void PDFExtOutDevData::EndGroup( const Graphic&     rGraphic,
                                 sal_uInt8          nTransparency,
                                 const Rectangle&   rOutputRect,
                                 const Rectangle&   rVisibleOutputRect )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::EndGroupGfxLink );
    mpPageSyncData->mGraphics.push_back( rGraphic );
    mpPageSyncData->mParaInts.push_back( nTransparency );
    mpPageSyncData->mParaRects.push_back( rOutputRect );
    mpPageSyncData->mParaRects.push_back( rVisibleOutputRect );
}

// Avoids expensive de-compression and re-compression of large images.
bool PDFExtOutDevData::HasAdequateCompression( const Graphic &rGraphic,
                                               const Rectangle & /* rOutputRect */,
                                               const Rectangle & /* rVisibleOutputRect */ ) const
{
    bool bReduceResolution = false;

    assert( rGraphic.IsLink() && (rGraphic.GetLink().GetType() ==  GfxLinkType::NativeJpg || rGraphic.GetLink().GetType() == GfxLinkType::NativePng));

    // small items better off as PNG anyway
    if ( rGraphic.GetSizePixel().Width() < 32 &&
         rGraphic.GetSizePixel().Height() < 32 )
        return false;

    // FIXME: ideally we'd also pre-empt the DPI related scaling too.

    Size aSize = rGraphic.GetSizePixel();
    sal_Int32 nCurrentRatio = (100 * aSize.Width() * aSize.Height() * 4) /
                               rGraphic.GetLink().GetDataSize();

    if ( GetIsLosslessCompression() )
        return !bReduceResolution && !GetIsReduceImageResolution();
    else
    {
        static const struct {
            sal_Int32 mnQuality;
            sal_Int32 mnRatio;
        } aRatios[] = { // minimum tolerable compression ratios
            { 100, 400 }, { 95, 700 }, { 90, 1000 }, { 85, 1200 },
            { 80, 1500 }, { 75, 1700 }
        };
        sal_Int32 nTargetRatio = 10000;
        bool bIsTargetRatioReached = false;
        for (auto & rRatio : aRatios)
        {
            if ( mnCompressionQuality > rRatio.mnQuality )
            {
                bIsTargetRatioReached = true;
                break;
            }
            nTargetRatio = rRatio.mnRatio;
        }

        return ((nCurrentRatio > nTargetRatio) && bIsTargetRatioReached);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
