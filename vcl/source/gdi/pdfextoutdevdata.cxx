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

#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/graph.hxx"
#include "vcl/outdev.hxx"
#include "vcl/gfxlink.hxx"
#include "vcl/dllapi.h"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"


#include <boost/shared_ptr.hpp>
#include <set>
#include <map>

namespace vcl
{
struct SAL_DLLPRIVATE PDFExtOutDevDataSync
{
    enum Action{    CreateNamedDest,
                    CreateDest,
                    CreateLink,
                    SetLinkDest,
                    SetLinkURL,
                    RegisterDest,
                    CreateOutlineItem,
                    SetOutlineItemParent,
                    SetOutlineItemText,
                    SetOutlineItemDest,
                    CreateNote,
                    SetAutoAdvanceTime,
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
                    EndGroup,
                    EndGroupGfxLink
    };

    sal_uInt32  nIdx;
    Action      eAct;
};

struct SAL_DLLPRIVATE PDFLinkDestination
{
    Rectangle               mRect;
    MapMode                 mMapMode;
    sal_Int32               mPageNr;
    PDFWriter::DestAreaType mAreaType;
};

struct SAL_DLLPRIVATE GlobalSyncData
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

        DBG_ASSERT( nLinkId >= 0, "unmapped id in GlobalSyncData" );
    }

    return nLinkId;
}

sal_Int32 GlobalSyncData::GetMappedStructId( sal_Int32 nStructId )
{
    if ( (sal_uInt32)nStructId < mStructIdMap.size() )
        nStructId = mStructIdMap[ nStructId ];
    else
        nStructId = -1;

    DBG_ASSERT( nStructId >= 0, "unmapped structure id in GlobalSyncData" );

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
                 rWriter.Push( PUSH_MAPMODE );
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
                rWriter.Push( PUSH_MAPMODE );
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
                rWriter.Push( PUSH_MAPMODE );
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
            case PDFExtOutDevDataSync::RegisterDest :
            {
                const sal_Int32 nDestId = mParaInts.front();
                mParaInts.pop_front();
                OSL_ENSURE( mFutureDestinations.find( nDestId ) != mFutureDestinations.end(),
                    "GlobalSyncData::PlayGlobalActions: DescribeRegisteredRequest has not been called for that destination!" );

                PDFLinkDestination& rDest = mFutureDestinations[ nDestId ];

                rWriter.Push( PUSH_MAPMODE );
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
            case PDFExtOutDevDataSync::SetOutlineItemParent :
            {
                sal_Int32 nItem = GetMappedId();
                sal_Int32 nNewParent = GetMappedId();
                rWriter.SetOutlineItemParent( nItem, nNewParent );
            }
            break;
            case PDFExtOutDevDataSync::SetOutlineItemText :
            {
                sal_Int32 nItem = GetMappedId();
                rWriter.SetOutlineItemText( nItem, mParaOUStrings.front() );
                mParaOUStrings.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetOutlineItemDest :
            {
                sal_Int32 nItem = GetMappedId();
                sal_Int32 nDestId = GetMappedId();
                rWriter.SetOutlineItemDest( nItem, nDestId );
            }
            break;
            case PDFExtOutDevDataSync::CreateNote :
            {
                rWriter.Push( PUSH_MAPMODE );
                rWriter.SetMapMode( mParaMapModes.front() );
                rWriter.CreateNote( mParaRects.front(), mParaPDFNotes.front(), mParaInts.front() );
                mParaMapModes.pop_front();
                mParaRects.pop_front();
                mParaPDFNotes.pop_front();
                mParaInts.pop_front();
            }
            break;
            case PDFExtOutDevDataSync::SetAutoAdvanceTime :
            {
                rWriter.SetAutoAdvanceTime( mParauInts.front(), mParaInts.front() );
                mParauInts.pop_front();
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
            case PDFExtOutDevDataSync::EndGroup:
            case PDFExtOutDevDataSync::EndGroupGfxLink:
                break;
        }
    }
}

struct SAL_DLLPRIVATE PageSyncData
{
    std::deque< PDFExtOutDevDataSync >              mActions;
    std::deque< Rectangle >                         mParaRects;
    std::deque< sal_Int32 >                         mParaInts;
    std::deque< OUString >                     mParaOUStrings;
    std::deque< PDFWriter::StructElement >          mParaStructElements;
    std::deque< PDFWriter::StructAttribute >        mParaStructAttributes;
    std::deque< PDFWriter::StructAttributeValue >   mParaStructAttributeValues;
    std::deque< Graphic >                           mGraphics;
    std::deque< ::boost::shared_ptr< PDFWriter::AnyWidget > >
                                                    mControls;
    GlobalSyncData*                                 mpGlobalData;

    bool                                        mbGroupIgnoreGDIMtfActions;

    PageSyncData( GlobalSyncData* pGlobal ) : mbGroupIgnoreGDIMtfActions ( false ) { mpGlobalData = pGlobal; }

    void PushAction( const OutputDevice& rOutDev, const PDFExtOutDevDataSync::Action eAct );
    sal_Bool PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction, const PDFExtOutDevData& rOutDevData );
};
void PageSyncData::PushAction( const OutputDevice& rOutDev, const PDFExtOutDevDataSync::Action eAct )
{
    GDIMetaFile* pMtf = rOutDev.GetConnectMetaFile();
    DBG_ASSERT( pMtf, "PageSyncData::PushAction -> no ConnectMetaFile !!!" );

    PDFExtOutDevDataSync aSync;
    aSync.eAct = eAct;
    if ( pMtf )
        aSync.nIdx = pMtf->GetActionSize();
    else
        aSync.nIdx = 0x7fffffff;    // sync not possible
    mActions.push_back( aSync );
}
sal_Bool PageSyncData::PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction, const PDFExtOutDevData& rOutDevData )
{
    sal_Bool bRet = sal_False;
    if ( mActions.size() && ( mActions.front().nIdx == rCurGDIMtfAction ) )
    {
        bRet = sal_True;
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
                ::boost::shared_ptr< PDFWriter::AnyWidget > pControl( mControls.front() );
                DBG_ASSERT( pControl.get(), "PageSyncData::PlaySyncPageAct: invalid widget!" );
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
                    if ( aBeg->eAct == PDFExtOutDevDataSync::EndGroup )
                    {
                        break;
                    }
                    else if ( aBeg->eAct == PDFExtOutDevDataSync::EndGroupGfxLink )
                    {
                        if ( rOutDevData.GetIsLosslessCompression() && !rOutDevData.GetIsReduceImageResolution() )
                        {
                            Graphic& rGraphic = mGraphics.front();
                            if ( rGraphic.IsLink() && rGraphic.GetLink().GetType() == GFX_LINK_TYPE_NATIVE_JPG )
                            {
                                mbGroupIgnoreGDIMtfActions = true;
                            }
                        }
                        break;
                    }
                    ++aBeg;
                }
            }
            break;
            case PDFExtOutDevDataSync::EndGroup :
            {
                mbGroupIgnoreGDIMtfActions = false;
            }
            break;
            case PDFExtOutDevDataSync::EndGroupGfxLink :
            {
                Rectangle aOutputRect, aVisibleOutputRect;
                Graphic   aGraphic( mGraphics.front() );

                mGraphics.pop_front();
                mParaInts.pop_front(); //Transparency
                aOutputRect = mParaRects.front();
                mParaRects.pop_front();
                aVisibleOutputRect = mParaRects.front();
                mParaRects.pop_front();

                if ( mbGroupIgnoreGDIMtfActions )
                {
                    bool bClippingNeeded = ( aOutputRect != aVisibleOutputRect ) && !aVisibleOutputRect.IsEmpty();

                    GfxLink   aGfxLink( aGraphic.GetLink() );
                    if ( aGfxLink.GetType() == GFX_LINK_TYPE_NATIVE_JPG )
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
                        SvMemoryStream aTmp;
                        const sal_uInt8* pData = aGfxLink.GetData();
                        sal_uInt32 nBytes = aGfxLink.GetDataSize();
                        if( pData && nBytes )
                        {
                            aTmp.Write( pData, nBytes );
                            rWriter.DrawJPGBitmap( aTmp, aGraphic.GetBitmap().GetBitCount() > 8, aGraphic.GetSizePixel(), aOutputRect, aMask );
                        }

                        if ( bClippingNeeded )
                            rWriter.Pop();
                    }
                    mbGroupIgnoreGDIMtfActions = false;
                }
            }
            break;
            case PDFExtOutDevDataSync::CreateNamedDest:
            case PDFExtOutDevDataSync::CreateDest:
            case PDFExtOutDevDataSync::CreateLink:
            case PDFExtOutDevDataSync::SetLinkDest:
            case PDFExtOutDevDataSync::SetLinkURL:
            case PDFExtOutDevDataSync::RegisterDest:
            case PDFExtOutDevDataSync::CreateOutlineItem:
            case PDFExtOutDevDataSync::SetOutlineItemParent:
            case PDFExtOutDevDataSync::SetOutlineItemText:
            case PDFExtOutDevDataSync::SetOutlineItemDest:
            case PDFExtOutDevDataSync::CreateNote:
            case PDFExtOutDevDataSync::SetAutoAdvanceTime:
            case PDFExtOutDevDataSync::SetPageTransition:
                break;
        }
    }
    else if ( mbGroupIgnoreGDIMtfActions )
    {
        rCurGDIMtfAction++;
        bRet = sal_True;
    }
    return bRet;
}

TYPEINIT1(PDFExtOutDevData,ExtOutDevData);
PDFExtOutDevData::PDFExtOutDevData( const OutputDevice& rOutDev ) :
    mrOutDev                ( rOutDev ),
    mbTaggedPDF             ( sal_False ),
    mbExportNotes           ( sal_True ),
    mbExportNotesPages      ( sal_False ),
    mbTransitionEffects     ( sal_True ),
    mbUseLosslessCompression( sal_True ),
    mbReduceImageResolution ( sal_False ),
    mbExportHiddenSlides    ( sal_False ),
    mbExportNDests          ( sal_False ),
    mnFormsFormat           ( 0 ),
    mnPage                  ( -1 ),
    mpPageSyncData          ( NULL ),
    mpGlobalSyncData        ( new GlobalSyncData() )
{
    mpPageSyncData = new PageSyncData( mpGlobalSyncData );
}

PDFExtOutDevData::~PDFExtOutDevData()
{
    delete mpPageSyncData;
    delete mpGlobalSyncData;
}

const com::sun::star::lang::Locale& PDFExtOutDevData::GetDocumentLocale() const
{
    return maDocLocale;
}
void PDFExtOutDevData::SetDocumentLocale( const com::sun::star::lang::Locale& rLoc )
{
    maDocLocale = rLoc;
}
sal_Int32 PDFExtOutDevData::GetCurrentPageNumber() const
{
    return mnPage;
}
void PDFExtOutDevData::SetCurrentPageNumber( const sal_Int32 nPage )
{
    mnPage = nPage;
}
sal_Bool PDFExtOutDevData::GetIsLosslessCompression() const
{
    return mbUseLosslessCompression;
}
void PDFExtOutDevData::SetIsLosslessCompression( const sal_Bool bUseLosslessCompression )
{
    mbUseLosslessCompression = bUseLosslessCompression;
}
sal_Bool PDFExtOutDevData::GetIsReduceImageResolution() const
{
    return mbReduceImageResolution;
}
void PDFExtOutDevData::SetIsReduceImageResolution( const sal_Bool bReduceImageResolution )
{
    mbReduceImageResolution = bReduceImageResolution;
}
sal_Bool PDFExtOutDevData::GetIsExportNotes() const
{
    return mbExportNotes;
}
void PDFExtOutDevData::SetIsExportNotes( const sal_Bool bExportNotes )
{
    mbExportNotes = bExportNotes;
}
sal_Bool PDFExtOutDevData::GetIsExportNotesPages() const
{
    return mbExportNotesPages;
}
void PDFExtOutDevData::SetIsExportNotesPages( const sal_Bool bExportNotesPages )
{
    mbExportNotesPages = bExportNotesPages;
}
sal_Bool PDFExtOutDevData::GetIsExportTaggedPDF() const
{
    return mbTaggedPDF;
}
void PDFExtOutDevData::SetIsExportTaggedPDF( const sal_Bool bTaggedPDF )
{
    mbTaggedPDF = bTaggedPDF;
}
sal_Bool PDFExtOutDevData::GetIsExportTransitionEffects() const
{
    return mbTransitionEffects;
}
void PDFExtOutDevData::SetIsExportTransitionEffects( const sal_Bool bTransitionEffects )
{
    mbTransitionEffects = bTransitionEffects;
}
sal_Bool PDFExtOutDevData::GetIsExportFormFields() const
{
    return mbExportFormFields;
}
void PDFExtOutDevData::SetIsExportFormFields( const sal_Bool bExportFomtFields )
{
    mbExportFormFields = bExportFomtFields;
}
void PDFExtOutDevData::SetFormsFormat( const sal_Int32 nFormsFormat )
{
    mnFormsFormat = nFormsFormat;
}
sal_Bool PDFExtOutDevData::GetIsExportBookmarks() const
{
    return mbExportBookmarks;
}
void PDFExtOutDevData::SetIsExportBookmarks( const sal_Bool bExportBookmarks )
{
    mbExportBookmarks = bExportBookmarks;
}
sal_Bool PDFExtOutDevData::GetIsExportHiddenSlides() const
{
    return mbExportHiddenSlides;
}
void PDFExtOutDevData::SetIsExportHiddenSlides( const sal_Bool bExportHiddenSlides )
{
    mbExportHiddenSlides = bExportHiddenSlides;
}
std::vector< PDFExtOutDevBookmarkEntry >& PDFExtOutDevData::GetBookmarks()
{
    return maBookmarks;
}
sal_Bool PDFExtOutDevData::GetIsExportNamedDestinations() const
{
    return mbExportNDests;
}
void PDFExtOutDevData::SetIsExportNamedDestinations( const sal_Bool bExportNDests )
{
    mbExportNDests = bExportNDests;
}
void PDFExtOutDevData::ResetSyncData()
{
    *mpPageSyncData = PageSyncData( mpGlobalSyncData );
}
sal_Bool PDFExtOutDevData::PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rIdx )
{
    return mpPageSyncData->PlaySyncPageAct( rWriter, rIdx, *this );
}
void PDFExtOutDevData::PlayGlobalActions( PDFWriter& rWriter )
{
    mpGlobalSyncData->PlayGlobalActions( rWriter );
}

/* global actions, syncronisation to the recorded metafile isn't needed,
   all actions will be played after the last page was recorded
*/
//--->i56629
sal_Int32 PDFExtOutDevData::CreateNamedDest(const OUString& sDestName,  const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::CreateNamedDest );
    mpGlobalSyncData->mParaOUStrings.push_back( sDestName );
    mpGlobalSyncData->mParaRects.push_back( rRect );
    mpGlobalSyncData->mParaMapModes.push_back( mrOutDev.GetMapMode() );
    mpGlobalSyncData->mParaInts.push_back( nPageNr == -1 ? mnPage : nPageNr );
    mpGlobalSyncData->mParaDestAreaTypes.push_back( eType );

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
void PDFExtOutDevData::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    mpGlobalSyncData->mActions.push_back( PDFExtOutDevDataSync::SetPageTransition );
    mpGlobalSyncData->mParaPageTransitions.push_back( eType );
    mpGlobalSyncData->mParauInts.push_back( nMilliSec );
    mpGlobalSyncData->mParaInts.push_back( nPageNr == -1 ? mnPage : nPageNr );
}

/* local (page), actions have to be played synchroniously to the actions of
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

void PDFExtOutDevData::CreateControl( const PDFWriter::AnyWidget& rControlType, sal_Int32 /*nPageNr*/ )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::CreateControl );

    ::boost::shared_ptr< PDFWriter::AnyWidget > pClone( rControlType.Clone() );
    mpPageSyncData->mControls.push_back( pClone );
}

void PDFExtOutDevData::BeginGroup()
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::BeginGroup );
}

void PDFExtOutDevData::EndGroup( const Graphic&     rGraphic,
                                 sal_uInt8              nTransparency,
                                 const Rectangle&   rOutputRect,
                                 const Rectangle&   rVisibleOutputRect )
{
    mpPageSyncData->PushAction( mrOutDev, PDFExtOutDevDataSync::EndGroupGfxLink );
    mpPageSyncData->mGraphics.push_back( rGraphic );
    mpPageSyncData->mParaInts.push_back( nTransparency );
    mpPageSyncData->mParaRects.push_back( rOutputRect );
    mpPageSyncData->mParaRects.push_back( rVisibleOutputRect );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
