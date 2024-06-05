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

#include <vcl/canvastools.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/pdf/PDFNote.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>

#include <memory>
#include <map>
#include <variant>

namespace vcl
{
namespace {

struct CreateNamedDest {
    OUString maDestName;
    MapMode maParaMapMode;
    PDFWriter::DestAreaType mnParaDestAreaType;
    tools::Rectangle maParaRect;
    sal_Int32 mnPage;
};
struct CreateDest {
    MapMode maParaMapMode;
    PDFWriter::DestAreaType mnParaDestAreaType;
    tools::Rectangle maParaRect;
    sal_Int32 mnPage;
};
struct CreateControlLink { sal_Int32 mnControlId; };
struct CreateLink {
    OUString maAltText;
    MapMode maParaMapMode;
    tools::Rectangle maParaRect;
    sal_Int32 mnPage;
};
struct CreateScreen {
    OUString maAltText;
    OUString maMimeType;
    MapMode maParaMapMode;
    tools::Rectangle maParaRect;
    sal_Int32 mnPage;
};
struct SetLinkDest {
    sal_Int32 mnLinkId;
    sal_Int32 mnDestId;
};
struct SetLinkURL {
    OUString maLinkURL;
    sal_Int32 mnLinkId;
};
struct SetScreenURL {
    OUString maScreenURL;
    sal_Int32 mnScreenId;
};
struct SetScreenStream {
    OUString maScreenStream;
    sal_Int32 mnScreenId;
};
struct RegisterDest { sal_Int32 mnDestId; };
struct CreateOutlineItem {
    OUString maText;
    sal_Int32 mnParent;
    sal_Int32 mnDestID;
};

struct CreateNote {
    MapMode maParaMapMode;
    vcl::pdf::PDFNote maParaPDFNote;
    tools::Rectangle maParaRect;
    sal_Int32 mnPage;
};

struct SetPageTransition {
    PDFWriter::PageTransition maParaPageTransition;
    sal_uInt32 mnMilliSec;
    sal_Int32 mnPage;
};
struct EnsureStructureElement { sal_Int32 mnId; };
struct InitStructureElement {
    PDFWriter::StructElement mParaStructElement;
    OUString maAlias;
    sal_Int32 mnId;
};
struct BeginStructureElement { sal_Int32 mnId; };
struct EndStructureElement{};
struct SetCurrentStructureElement { sal_Int32 mnStructId; };
struct SetStructureAttribute {
    PDFWriter::StructAttribute mParaStructAttribute;
    PDFWriter::StructAttributeValue mParaStructAttributeValue;
};
struct SetStructureAttributeNumerical { PDFWriter::StructAttribute mParaStructAttribute; sal_Int32 mnId; };
struct SetStructureBoundingBox { tools::Rectangle mRect; };
struct SetStructureAnnotIds {
    ::std::vector<sal_Int32> annotIds;
};
struct SetActualText { OUString maText; };
struct SetAlternateText { OUString maText; };
struct CreateControl {
    std::shared_ptr< PDFWriter::AnyWidget > mxControl;
};
struct BeginGroup {};
struct EndGroupGfxLink {
    Graphic maGraphic;
    tools::Rectangle maOutputRect, maVisibleOutputRect;
    sal_Int32 mnTransparency;
};

typedef std::variant<CreateNamedDest,
                    CreateDest,
                    CreateControlLink,
                    CreateLink,
                    CreateScreen,
                    SetLinkDest,
                    SetLinkURL,
                    SetScreenURL,
                    SetScreenStream,
                    RegisterDest,
                    CreateOutlineItem,
                    CreateNote,
                    SetPageTransition> GlobalActionData;

typedef std::variant<EnsureStructureElement,
                    InitStructureElement,
                    BeginStructureElement,
                    EndStructureElement,
                    SetCurrentStructureElement,
                    SetStructureAttribute,
                    SetStructureAttributeNumerical,
                    SetStructureBoundingBox,
                    SetStructureAnnotIds,
                    SetActualText,
                    SetAlternateText,
                    CreateControl,
                    BeginGroup,
                    EndGroupGfxLink> PageActionData;

struct PDFExtOutDevDataSyncPage
{
    sal_uInt32  nIdx;
    PageActionData  eAct;
};

struct PDFLinkDestination
{
    tools::Rectangle               mRect;
    MapMode                 mMapMode;
    sal_Int32               mPageNr;
    PDFWriter::DestAreaType mAreaType;
};
}

struct GlobalSyncData
{
    std::deque< GlobalActionData >              mActions;
    ::std::map< sal_Int32, PDFLinkDestination > mFutureDestinations;

    sal_Int32 GetMappedId(sal_Int32 nLinkId);

    /** the way this appears to work: (only) everything that increments mCurId
        at recording time must put an item into mParaIds at playback time,
        so that the mCurId becomes the eventual index into mParaIds.
     */
    sal_Int32                   mCurId;
    std::vector< sal_Int32 >    mParaIds;
    std::map<void const*, sal_Int32> mSEMap;

    sal_Int32                   mCurrentStructElement;
    std::vector< sal_Int32 >    mStructParents;
    GlobalSyncData() :
            mCurId ( 0 ),
            mCurrentStructElement( 0 )
    {
        mStructParents.push_back(0); // because PDFWriterImpl has a dummy root
    }
    void PlayGlobalActions( PDFWriter& rWriter );
};

sal_Int32 GlobalSyncData::GetMappedId(sal_Int32 nLinkId)
{
    /*  negative values are intentionally passed as invalid IDs
     *  e.g. to create a new top level outline item
     */
    if( nLinkId >= 0 )
    {
        if ( o3tl::make_unsigned(nLinkId) < mParaIds.size() )
            nLinkId = mParaIds[ nLinkId ];
        else
            nLinkId = -1;

        SAL_WARN_IF( nLinkId < 0, "vcl", "unmapped id in GlobalSyncData" );
    }

    return nLinkId;
}

void GlobalSyncData::PlayGlobalActions( PDFWriter& rWriter )
{
    for (auto const& action : mActions)
    {
        if (std::holds_alternative<CreateNamedDest>(action)) { //i56629
            const vcl::CreateNamedDest& rCreateNamedDest = std::get<CreateNamedDest>(action);
            rWriter.Push( PushFlags::MAPMODE );
            rWriter.SetMapMode( rCreateNamedDest.maParaMapMode );
            mParaIds.push_back( rWriter.CreateNamedDest( rCreateNamedDest.maDestName, rCreateNamedDest.maParaRect, rCreateNamedDest.mnPage, rCreateNamedDest.mnParaDestAreaType ) );
            rWriter.Pop();
        }
        else if (std::holds_alternative<CreateDest>(action)) {
            const vcl::CreateDest& rCreateDest = std::get<CreateDest>(action);
            rWriter.Push( PushFlags::MAPMODE );
            rWriter.SetMapMode( rCreateDest.maParaMapMode );
            mParaIds.push_back( rWriter.CreateDest( rCreateDest.maParaRect, rCreateDest.mnPage, rCreateDest.mnParaDestAreaType ) );
            rWriter.Pop();
        }
        else if (std::holds_alternative<CreateControlLink>(action)) {
            const vcl::CreateControlLink& rCreateControlLink = std::get<CreateControlLink>(action);
            // tdf#157397: this must be called *in order* with CreateLink etc.
            rWriter.SetLinkPropertyID(rCreateControlLink.mnControlId, sal_Int32(mParaIds.size()));
            mParaIds.push_back(rCreateControlLink.mnControlId);
        }
        else if (std::holds_alternative<CreateLink>(action)) {
            const vcl::CreateLink& rCreateLink = std::get<CreateLink>(action);
            rWriter.Push( PushFlags::MAPMODE );
            rWriter.SetMapMode( rCreateLink.maParaMapMode );
            mParaIds.push_back( rWriter.CreateLink(rCreateLink.maParaRect, rCreateLink.mnPage, rCreateLink.maAltText) );
            // resolve LinkAnnotation structural attribute
            rWriter.SetLinkPropertyID( mParaIds.back(), sal_Int32(mParaIds.size()-1) );
            rWriter.Pop();
        }
        else if (std::holds_alternative<CreateScreen>(action)) {
            const vcl::CreateScreen& rCreateScreen = std::get<CreateScreen>(action);
            rWriter.Push(PushFlags::MAPMODE);
            rWriter.SetMapMode(rCreateScreen.maParaMapMode);
            mParaIds.push_back(rWriter.CreateScreen(rCreateScreen.maParaRect, rCreateScreen.mnPage, rCreateScreen.maAltText, rCreateScreen.maMimeType));
            // resolve AnnotIds structural attribute
            rWriter.SetLinkPropertyID(mParaIds.back(), sal_Int32(mParaIds.size()-1));
            rWriter.Pop();
        }
        else if (std::holds_alternative<SetLinkDest>(action)) {
            const vcl::SetLinkDest& rSetLinkDest = std::get<SetLinkDest>(action);
            sal_Int32 nLinkId = GetMappedId(rSetLinkDest.mnLinkId);
            sal_Int32 nDestId = GetMappedId(rSetLinkDest.mnDestId);
            rWriter.SetLinkDest( nLinkId, nDestId );
        }
        else if (std::holds_alternative<SetLinkURL>(action)) {
            const vcl::SetLinkURL& rSetLinkURL = std::get<SetLinkURL>(action);
            sal_Int32 nLinkId = GetMappedId(rSetLinkURL.mnLinkId);
            rWriter.SetLinkURL( nLinkId, rSetLinkURL.maLinkURL );
        }
        else if (std::holds_alternative<SetScreenURL>(action)) {
            const vcl::SetScreenURL& rSetScreenURL = std::get<SetScreenURL>(action);
            sal_Int32 nScreenId = GetMappedId(rSetScreenURL.mnScreenId);
            rWriter.SetScreenURL(nScreenId, rSetScreenURL.maScreenURL);
        }
        else if (std::holds_alternative<SetScreenStream>(action)) {
            const vcl::SetScreenStream& rSetScreenStream = std::get<SetScreenStream>(action);
            sal_Int32 nScreenId = GetMappedId(rSetScreenStream.mnScreenId);
            rWriter.SetScreenStream(nScreenId, rSetScreenStream.maScreenStream);
        }
        else if (std::holds_alternative<RegisterDest>(action)) {
            const vcl::RegisterDest& rRegisterDest = std::get<RegisterDest>(action);
            const sal_Int32 nDestId = rRegisterDest.mnDestId;
            OSL_ENSURE( mFutureDestinations.find( nDestId ) != mFutureDestinations.end(),
                "GlobalSyncData::PlayGlobalActions: DescribeRegisteredRequest has not been called for that destination!" );

            PDFLinkDestination& rDest = mFutureDestinations[ nDestId ];

            rWriter.Push( PushFlags::MAPMODE );
            rWriter.SetMapMode( rDest.mMapMode );
            mParaIds.push_back( rWriter.RegisterDestReference( nDestId, rDest.mRect, rDest.mPageNr, rDest.mAreaType ) );
            rWriter.Pop();
        }
        else if (std::holds_alternative<CreateOutlineItem>(action)) {
            const vcl::CreateOutlineItem& rCreateOutlineItem = std::get<CreateOutlineItem>(action);
            sal_Int32 nParent = GetMappedId(rCreateOutlineItem.mnParent);
            sal_Int32 nLinkId = GetMappedId(rCreateOutlineItem.mnDestID);
            mParaIds.push_back( rWriter.CreateOutlineItem( nParent, rCreateOutlineItem.maText, nLinkId ) );
        }
        else if (std::holds_alternative<CreateNote>(action)) {
            const vcl::CreateNote& rCreateNote = std::get<CreateNote>(action);
            rWriter.Push( PushFlags::MAPMODE );
            rWriter.SetMapMode( rCreateNote.maParaMapMode );
            rWriter.CreateNote( rCreateNote.maParaRect, rCreateNote.maParaPDFNote, rCreateNote.mnPage );
        }
        else if (std::holds_alternative<SetPageTransition>(action)) {
            const vcl::SetPageTransition& rSetPageTransition = std::get<SetPageTransition>(action);
            rWriter.SetPageTransition( rSetPageTransition.maParaPageTransition, rSetPageTransition.mnMilliSec, rSetPageTransition.mnPage );
        }
    }
}

struct PageSyncData
{
    std::deque< PDFExtOutDevDataSyncPage >          mActions;
    Graphic                                         mCurrentGraphic;
    GlobalSyncData*                                 mpGlobalData;

    bool                                        mbGroupIgnoreGDIMtfActions;


    explicit PageSyncData( GlobalSyncData* pGlobal )
        : mbGroupIgnoreGDIMtfActions ( false )
    { mpGlobalData = pGlobal; }

    void PushAction( const OutputDevice& rOutDev, PageActionData eAct );
    bool PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction, const GDIMetaFile& rMtf, const PDFExtOutDevData& rOutDevData );
};

void PageSyncData::PushAction( const OutputDevice& rOutDev, PageActionData eAct )
{
    GDIMetaFile* pMtf = rOutDev.GetConnectMetaFile();
    SAL_WARN_IF( !pMtf, "vcl", "PageSyncData::PushAction -> no ConnectMetaFile !!!" );

    PDFExtOutDevDataSyncPage aSync;
    aSync.eAct = std::move(eAct);
    if ( pMtf )
        aSync.nIdx = pMtf->GetActionSize();
    else
        aSync.nIdx = 0x7fffffff;    // sync not possible
    mActions.emplace_back( std::move(aSync) );
}
bool PageSyncData::PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rCurGDIMtfAction, const GDIMetaFile& rMtf, const PDFExtOutDevData& rOutDevData )
{
    bool bRet = false;
    if ( !mActions.empty() && ( mActions.front().nIdx == rCurGDIMtfAction ) )
    {
        bRet = true;
        PDFExtOutDevDataSyncPage aDataSync = std::move(mActions.front());
        mActions.pop_front();
        if (std::holds_alternative<EnsureStructureElement>(aDataSync.eAct)) {
#ifndef NDEBUG
            const vcl::EnsureStructureElement& rEnsureStructureElement = std::get<EnsureStructureElement>(aDataSync.eAct);
            sal_Int32 const id =
#endif
                rWriter.EnsureStructureElement();
            assert(id == -1 || id == rEnsureStructureElement.mnId); // identity mapping
        }
        else if (std::holds_alternative<InitStructureElement>(aDataSync.eAct)) {
            const vcl::InitStructureElement& rInitStructureElement = std::get<InitStructureElement>(aDataSync.eAct);
            rWriter.InitStructureElement(rInitStructureElement.mnId, rInitStructureElement.mParaStructElement, rInitStructureElement.maAlias);
        }
        else if (std::holds_alternative<BeginStructureElement>(aDataSync.eAct)) {
            const vcl::BeginStructureElement& rBeginStructureElement = std::get<BeginStructureElement>(aDataSync.eAct);
            rWriter.BeginStructureElement(rBeginStructureElement.mnId);
        }
        else if (std::holds_alternative<EndStructureElement>(aDataSync.eAct)) {
            rWriter.EndStructureElement();
        }
        else if (std::holds_alternative<SetCurrentStructureElement>(aDataSync.eAct)) {
            const vcl::SetCurrentStructureElement& rSetCurrentStructureElement = std::get<SetCurrentStructureElement>(aDataSync.eAct);
            rWriter.SetCurrentStructureElement(rSetCurrentStructureElement.mnStructId);
        }
        else if (std::holds_alternative<SetStructureAttribute>(aDataSync.eAct)) {
            const vcl::SetStructureAttribute& rSetStructureAttribute = std::get<SetStructureAttribute>(aDataSync.eAct);
            rWriter.SetStructureAttribute( rSetStructureAttribute.mParaStructAttribute, rSetStructureAttribute.mParaStructAttributeValue );
        }
        else if (std::holds_alternative<SetStructureAttributeNumerical>(aDataSync.eAct)) {
            const vcl::SetStructureAttributeNumerical& rSetStructureAttributeNumerical = std::get<SetStructureAttributeNumerical>(aDataSync.eAct);
            rWriter.SetStructureAttributeNumerical( rSetStructureAttributeNumerical.mParaStructAttribute, rSetStructureAttributeNumerical.mnId );
        }
        else if (std::holds_alternative<SetStructureBoundingBox>(aDataSync.eAct)) {
            const vcl::SetStructureBoundingBox& rSetStructureBoundingBox = std::get<SetStructureBoundingBox>(aDataSync.eAct);
            rWriter.SetStructureBoundingBox( rSetStructureBoundingBox.mRect );
        }
        else if (std::holds_alternative<SetStructureAnnotIds>(aDataSync.eAct)) {
            const vcl::SetStructureAnnotIds& rSetStructureAnnotIds = std::get<SetStructureAnnotIds>(aDataSync.eAct);
            rWriter.SetStructureAnnotIds(rSetStructureAnnotIds.annotIds);
        }
        else if (std::holds_alternative<SetActualText>(aDataSync.eAct)) {
            const vcl::SetActualText& rSetActualText = std::get<SetActualText>(aDataSync.eAct);
            rWriter.SetActualText( rSetActualText.maText );
        }
        else if (std::holds_alternative<SetAlternateText>(aDataSync.eAct)) {
            const vcl::SetAlternateText& rSetAlternateText = std::get<SetAlternateText>(aDataSync.eAct);
            rWriter.SetAlternateText( rSetAlternateText.maText );
        }
        else if (std::holds_alternative<CreateControl>(aDataSync.eAct)) {
            const vcl::CreateControl& rCreateControl = std::get<CreateControl>(aDataSync.eAct);
            std::shared_ptr< PDFWriter::AnyWidget > pControl( rCreateControl.mxControl );
            SAL_WARN_IF( !pControl, "vcl", "PageSyncData::PlaySyncPageAct: invalid widget!" );
            if ( pControl )
            {
                sal_Int32 const n = rWriter.CreateControl(*pControl);
                // resolve AnnotIds structural attribute
                ::std::vector<sal_Int32> const annotIds{ sal_Int32(mpGlobalData->mCurId) };
                rWriter.SetStructureAnnotIds(annotIds);
                // tdf#157397: this must be called *in order* with CreateLink etc.
                mpGlobalData->mActions.push_back(CreateControlLink{n});
                mpGlobalData->mCurId++;
            }
        }
        else if (std::holds_alternative<BeginGroup>(aDataSync.eAct)) {
            /* first determining if this BeginGroup is starting a GfxLink,
               by searching for an EndGroup or an EndGroupGfxLink */
            mbGroupIgnoreGDIMtfActions = false;
            auto itStartingGfxLink = std::find_if(mActions.begin(), mActions.end(),
                [](const PDFExtOutDevDataSyncPage& rAction) { return std::holds_alternative<EndGroupGfxLink>(rAction.eAct); });
            if ( itStartingGfxLink != mActions.end() )
            {
                EndGroupGfxLink& rEndGroup = std::get<EndGroupGfxLink>(itStartingGfxLink->eAct);
                Graphic& rGraphic = rEndGroup.maGraphic;
                if ( rGraphic.IsGfxLink() )
                {
                    GfxLinkType eType = rGraphic.GetGfxLink().GetType();
                    if ( eType == GfxLinkType::NativeJpg )
                    {
                        mbGroupIgnoreGDIMtfActions = rOutDevData.HasAdequateCompression(rGraphic, rEndGroup.maOutputRect, rEndGroup.maVisibleOutputRect);
                        if ( !mbGroupIgnoreGDIMtfActions )
                            mCurrentGraphic = rGraphic;
                    }
                    else if ( eType == GfxLinkType::NativePng || eType == GfxLinkType::NativePdf )
                    {
                        if ( eType == GfxLinkType::NativePdf || rOutDevData.HasAdequateCompression(rGraphic, rEndGroup.maOutputRect, rEndGroup.maVisibleOutputRect) )
                            mCurrentGraphic = rGraphic;
                    }
                }
            }
        }
        else if (std::holds_alternative<EndGroupGfxLink>(aDataSync.eAct)) {
            EndGroupGfxLink& rEndGroup = std::get<EndGroupGfxLink>(aDataSync.eAct);
            tools::Rectangle aOutputRect, aVisibleOutputRect;
            Graphic   aGraphic( rEndGroup.maGraphic );

            sal_Int32 nTransparency = rEndGroup.mnTransparency;
            aOutputRect = rEndGroup.maOutputRect;
            aVisibleOutputRect = rEndGroup.maVisibleOutputRect;

            if ( mbGroupIgnoreGDIMtfActions )
            {
                bool bClippingNeeded = ( aOutputRect != aVisibleOutputRect ) && !aVisibleOutputRect.IsEmpty();

                GfxLink   aGfxLink( aGraphic.GetGfxLink() );
                if ( aGfxLink.GetType() == GfxLinkType::NativeJpg )
                {
                    if ( bClippingNeeded )
                    {
                        rWriter.Push();
                        basegfx::B2DPolyPolygon aRect( basegfx::utils::createPolygonFromRect(
                            vcl::unotools::b2DRectangleFromRectangle(aVisibleOutputRect) ) );
                        rWriter.SetClipRegion( aRect);
                    }

                    AlphaMask aAlphaMask;
                    if (nTransparency)
                    {
                        aAlphaMask = AlphaMask(aGraphic.GetSizePixel());
                        aAlphaMask.Erase(nTransparency);
                    }

                    SvMemoryStream aTmp;
                    const sal_uInt8* pData = aGfxLink.GetData();
                    sal_uInt32 nBytes = aGfxLink.GetDataSize();
                    if( pData && nBytes )
                    {
                        aTmp.WriteBytes( pData, nBytes );

                        // Look up the output rectangle from the previous
                        // bitmap scale action if possible. This has the
                        // correct position and size for images with a
                        // custom translation (Writer header) or scaling
                        // (Impress notes page).
                        if (rCurGDIMtfAction > 0)
                        {
                            const MetaAction* pAction = rMtf.GetAction(rCurGDIMtfAction - 1);
                            if (pAction && pAction->GetType() == MetaActionType::BMPSCALE)
                            {
                                const MetaBmpScaleAction* pA
                                    = static_cast<const MetaBmpScaleAction*>(pAction);
                                aOutputRect.SetPos(pA->GetPoint());
                                aOutputRect.SetSize(pA->GetSize());
                            }
                        }
                        auto ePixelFormat = aGraphic.GetBitmapEx().getPixelFormat();
                        rWriter.DrawJPGBitmap(aTmp, ePixelFormat > vcl::PixelFormat::N8_BPP, aGraphic.GetSizePixel(), aOutputRect, aAlphaMask, aGraphic);
                    }

                    if ( bClippingNeeded )
                        rWriter.Pop();
                }
                mbGroupIgnoreGDIMtfActions = false;
            }
            mCurrentGraphic.Clear();
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
    mbExportNotesInMargin   ( false ),
    mbExportNotesPages      ( false ),
    mbTransitionEffects     ( true ),
    mbUseLosslessCompression( true ),
    mbReduceImageResolution ( false ),
    mbExportFormFields      ( false ),
    mbExportBookmarks       ( false ),
    mbExportHiddenSlides    ( false ),
    mbSinglePageSheets      ( false ),
    mbExportNDests          ( false ),
    mnPage                  ( -1 ),
    mnCompressionQuality    ( 90 ),
    mpGlobalSyncData        ( new GlobalSyncData() )
{
    mpPageSyncData.reset( new PageSyncData( mpGlobalSyncData.get() ) );
}

PDFExtOutDevData::~PDFExtOutDevData()
{
    mpPageSyncData.reset();
    mpGlobalSyncData.reset();
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
void PDFExtOutDevData::SetIsReduceImageResolution( const bool bReduceImageResolution )
{
    mbReduceImageResolution = bReduceImageResolution;
}
void PDFExtOutDevData::SetIsExportNotes( const bool bExportNotes )
{
    mbExportNotes = bExportNotes;
}
void PDFExtOutDevData::SetIsExportNotesInMargin( const bool bExportNotesInMargin )
{
    mbExportNotesInMargin = bExportNotesInMargin;
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
void PDFExtOutDevData::SetIsExportBookmarks( const bool bExportBookmarks )
{
    mbExportBookmarks = bExportBookmarks;
}
void PDFExtOutDevData::SetIsExportHiddenSlides( const bool bExportHiddenSlides )
{
    mbExportHiddenSlides = bExportHiddenSlides;
}
void PDFExtOutDevData::SetIsSinglePageSheets( const bool bSinglePageSheets )
{
    mbSinglePageSheets = bSinglePageSheets;
}
void PDFExtOutDevData::SetIsExportNamedDestinations( const bool bExportNDests )
{
    mbExportNDests = bExportNDests;
}
void PDFExtOutDevData::ResetSyncData(PDFWriter *const pWriter)
{
    if (pWriter != nullptr)
    {
        // tdf#157182 HACK: all PDF actions on this page will be deleted; to have
        // matching SE IDs on the *next* page, replay EnsureStructureElement actions
        for (PDFExtOutDevDataSyncPage const& rAction : mpPageSyncData->mActions)
        {
            if (std::holds_alternative<struct EnsureStructureElement>(rAction.eAct))
            {
                pWriter->EnsureStructureElement();
            }
        }
    }
    *mpPageSyncData = PageSyncData( mpGlobalSyncData.get() );
}
bool PDFExtOutDevData::PlaySyncPageAct( PDFWriter& rWriter, sal_uInt32& rIdx, const GDIMetaFile& rMtf )
{
    return mpPageSyncData->PlaySyncPageAct( rWriter, rIdx, rMtf, *this );
}
void PDFExtOutDevData::PlayGlobalActions( PDFWriter& rWriter )
{
    mpGlobalSyncData->PlayGlobalActions( rWriter );
}

/* global actions, synchronisation to the recorded metafile isn't needed,
   all actions will be played after the last page was recorded
*/
//--->i56629
sal_Int32 PDFExtOutDevData::CreateNamedDest(const OUString& sDestName,  const tools::Rectangle& rRect, sal_Int32 nPageNr )
{
    mpGlobalSyncData->mActions.push_back(
        vcl::CreateNamedDest{ sDestName, mrOutDev.GetMapMode(), PDFWriter::DestAreaType::XYZ, rRect, nPageNr == -1 ? mnPage : nPageNr } );

    return mpGlobalSyncData->mCurId++;
}
//<---i56629
sal_Int32 PDFExtOutDevData::RegisterDest()
{
    const sal_Int32 nLinkDestID = mpGlobalSyncData->mCurId++;
    mpGlobalSyncData->mActions.push_back( vcl::RegisterDest{ nLinkDestID } );

    return nLinkDestID;
}
void PDFExtOutDevData::DescribeRegisteredDest( sal_Int32 nDestId, const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    OSL_PRECOND( nDestId != -1, "PDFExtOutDevData::DescribeRegisteredDest: invalid destination Id!" );
    PDFLinkDestination aLinkDestination;
    aLinkDestination.mRect = rRect;
    aLinkDestination.mMapMode = mrOutDev.GetMapMode();
    aLinkDestination.mPageNr = nPageNr == -1 ? mnPage : nPageNr;
    aLinkDestination.mAreaType = eType;
    mpGlobalSyncData->mFutureDestinations[ nDestId ] = aLinkDestination;
}
sal_Int32 PDFExtOutDevData::CreateDest( const tools::Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    mpGlobalSyncData->mActions.push_back(
        vcl::CreateDest{ mrOutDev.GetMapMode(), eType, rRect, nPageNr == -1 ? mnPage : nPageNr } );
    return mpGlobalSyncData->mCurId++;
}
sal_Int32 PDFExtOutDevData::CreateLink(const tools::Rectangle& rRect, OUString const& rAltText, sal_Int32 nPageNr)
{
    mpGlobalSyncData->mActions.push_back(
        vcl::CreateLink{rAltText, mrOutDev.GetMapMode(), rRect, nPageNr == -1 ? mnPage : nPageNr } );
    return mpGlobalSyncData->mCurId++;
}

sal_Int32 PDFExtOutDevData::CreateScreen(const tools::Rectangle& rRect,
        OUString const& rAltText, OUString const& rMimeType,
        sal_Int32 nPageNr, SdrObject const*const pObj)
{
    mpGlobalSyncData->mActions.push_back(vcl::CreateScreen{ rAltText, rMimeType, mrOutDev.GetMapMode(), rRect, nPageNr });
    auto const ret(mpGlobalSyncData->mCurId++);
    m_ScreenAnnotations[pObj].push_back(ret);
    return ret;
}

::std::vector<sal_Int32> const& PDFExtOutDevData::GetScreenAnnotIds(SdrObject const*const pObj) const
{
    auto const it(m_ScreenAnnotations.find(pObj));
    if (it == m_ScreenAnnotations.end())
    {
        assert(false); // expected?
    }
    return it->second;
}

void PDFExtOutDevData::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    mpGlobalSyncData->mActions.push_back( vcl::SetLinkDest{ nLinkId, nDestId } );
}
void PDFExtOutDevData::SetLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    mpGlobalSyncData->mActions.push_back( vcl::SetLinkURL{ rURL, nLinkId } );
}

void PDFExtOutDevData::SetScreenURL(sal_Int32 nScreenId, const OUString& rURL)
{
    mpGlobalSyncData->mActions.push_back(vcl::SetScreenURL{ rURL, nScreenId });
}

void PDFExtOutDevData::SetScreenStream(sal_Int32 nScreenId, const OUString& rURL)
{
    mpGlobalSyncData->mActions.push_back(vcl::SetScreenStream{ rURL, nScreenId });
}

sal_Int32 PDFExtOutDevData::CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    if (nParent == -1)
        // Has no parent, it's a chapter / heading 1.
        maChapterNames.push_back(rText);

    mpGlobalSyncData->mActions.push_back( vcl::CreateOutlineItem{ rText, nParent, nDestID } );
    return mpGlobalSyncData->mCurId++;
}
void PDFExtOutDevData::CreateNote( const tools::Rectangle& rRect, const vcl::pdf::PDFNote& rNote, sal_Int32 nPageNr )
{
    mpGlobalSyncData->mActions.push_back(
        vcl::CreateNote{ mrOutDev.GetMapMode(), rNote, rRect, nPageNr == -1 ? mnPage : nPageNr } );
}
void PDFExtOutDevData::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec )
{
    mpGlobalSyncData->mActions.push_back( vcl::SetPageTransition{ eType, nMilliSec, mnPage } );
}

/* local (page), actions have to be played synchronously to the actions of
   of the recorded metafile (created by each xRenderable->render()) */

sal_Int32 PDFExtOutDevData::EnsureStructureElement(void const*const key)
{
    sal_Int32 id(-1);
    if (key != nullptr)
    {
        auto const it(mpGlobalSyncData->mSEMap.find(key));
        if (it != mpGlobalSyncData->mSEMap.end())
        {
            id = it->second;
        }
    }
    if (id == -1)
    {
        id = mpGlobalSyncData->mStructParents.size();
        mpPageSyncData->PushAction(mrOutDev, vcl::EnsureStructureElement{ id });
        mpGlobalSyncData->mStructParents.push_back(mpGlobalSyncData->mCurrentStructElement);
        if (key != nullptr)
        {
            mpGlobalSyncData->mSEMap.emplace(key, id);
        }
    }
    return id;
}

void PDFExtOutDevData::InitStructureElement(sal_Int32 const id,
        PDFWriter::StructElement const eType, const OUString& rAlias)
{
    mpPageSyncData->PushAction(mrOutDev, vcl::InitStructureElement{ eType, rAlias, id });
    // update parent: required for hell fly anchor frames in sw, so that on the actual
    // anchor frame EndStructureElement() resets mCurrentStructElement properly.
    mpGlobalSyncData->mStructParents[id] = mpGlobalSyncData->mCurrentStructElement;
}

void PDFExtOutDevData::BeginStructureElement(sal_Int32 const id)
{
    mpPageSyncData->PushAction( mrOutDev, vcl::BeginStructureElement{ id } );
    mpGlobalSyncData->mCurrentStructElement = id;
}

sal_Int32 PDFExtOutDevData::WrapBeginStructureElement(
        PDFWriter::StructElement const eType, const OUString& rAlias)
{
    sal_Int32 const id = EnsureStructureElement(nullptr);
    InitStructureElement(id, eType, rAlias);
    BeginStructureElement(id);
    return id;
}

void PDFExtOutDevData::EndStructureElement()
{
    assert(mpGlobalSyncData->mCurrentStructElement != 0); // underflow?
    mpPageSyncData->PushAction( mrOutDev, vcl::EndStructureElement{} );
    mpGlobalSyncData->mCurrentStructElement = mpGlobalSyncData->mStructParents[ mpGlobalSyncData->mCurrentStructElement ];
}

void PDFExtOutDevData::SetCurrentStructureElement(sal_Int32 const nStructId)
{
    assert(o3tl::make_unsigned(nStructId) < mpGlobalSyncData->mStructParents.size());
    mpGlobalSyncData->mCurrentStructElement = nStructId;
    mpPageSyncData->PushAction( mrOutDev, vcl::SetCurrentStructureElement{ nStructId } );
}

sal_Int32 PDFExtOutDevData::GetCurrentStructureElement() const
{
    return mpGlobalSyncData->mCurrentStructElement;
}

void PDFExtOutDevData::SetStructureAttribute( PDFWriter::StructAttribute eAttr, PDFWriter::StructAttributeValue eVal )
{
    mpPageSyncData->PushAction( mrOutDev, vcl::SetStructureAttribute{ eAttr, eVal } );
}
void PDFExtOutDevData::SetStructureAttributeNumerical( PDFWriter::StructAttribute eAttr, sal_Int32 nValue )
{
    mpPageSyncData->PushAction( mrOutDev, vcl::SetStructureAttributeNumerical { eAttr, nValue } );
}
void PDFExtOutDevData::SetStructureBoundingBox( const tools::Rectangle& rRect )
{
    mpPageSyncData->PushAction( mrOutDev, vcl::SetStructureBoundingBox{ rRect } );
}

void PDFExtOutDevData::SetStructureAnnotIds(::std::vector<sal_Int32> const& rAnnotIds)
{
    mpPageSyncData->PushAction(mrOutDev, vcl::SetStructureAnnotIds{ rAnnotIds });
}

void PDFExtOutDevData::SetActualText( const OUString& rText )
{
    mpPageSyncData->PushAction( mrOutDev, vcl::SetActualText{ rText } );
}
void PDFExtOutDevData::SetAlternateText( const OUString& rText )
{
    mpPageSyncData->PushAction( mrOutDev, vcl::SetAlternateText{ rText } );
}

void PDFExtOutDevData::CreateControl( const PDFWriter::AnyWidget& rControlType )
{
    mpPageSyncData->PushAction(mrOutDev, vcl::CreateControl{ rControlType.Clone() });
}

void PDFExtOutDevData::BeginGroup()
{
    mpPageSyncData->PushAction( mrOutDev, vcl::BeginGroup{} );
}

void PDFExtOutDevData::EndGroup( const Graphic&     rGraphic,
                                 sal_uInt8          nTransparency,
                                 const tools::Rectangle&   rOutputRect,
                                 const tools::Rectangle&   rVisibleOutputRect )
{
    mpPageSyncData->PushAction( mrOutDev,
        vcl::EndGroupGfxLink{rGraphic, rOutputRect, rVisibleOutputRect, nTransparency} );
}

// Avoids expensive de-compression and re-compression of large images.
bool PDFExtOutDevData::HasAdequateCompression( const Graphic &rGraphic,
                                               const tools::Rectangle & rOutputRect,
                                               const tools::Rectangle & rVisibleOutputRect ) const
{
    assert(rGraphic.IsGfxLink() &&
           (rGraphic.GetGfxLink().GetType() == GfxLinkType::NativeJpg ||
            rGraphic.GetGfxLink().GetType() == GfxLinkType::NativePng ||
            rGraphic.GetGfxLink().GetType() == GfxLinkType::NativePdf));

    if (rOutputRect != rVisibleOutputRect)
        // rOutputRect is the crop rectangle, re-compress cropped image.
        return false;

    if (mbReduceImageResolution)
        // Reducing resolution was requested, implies that re-compressing is
        // wanted.
        return false;

    auto nSize = rGraphic.GetGfxLink().GetDataSize();
    if (nSize == 0)
        return false;

    GfxLink aLink = rGraphic.GetGfxLink();
    SvMemoryStream aMemoryStream(const_cast<sal_uInt8*>(aLink.GetData()), aLink.GetDataSize(),
                                 StreamMode::READ | StreamMode::WRITE);
    GraphicDescriptor aDescriptor(aMemoryStream, nullptr);
    if (aDescriptor.Detect(true) && aDescriptor.GetNumberOfImageComponents() == 4)
        // 4 means CMYK, which is not handled.
        return false;

    const Size aSize = rGraphic.GetSizePixel();

    // small items better off as PNG anyway
    if ( aSize.Width() < 32 &&
         aSize.Height() < 32 )
        return false;

    if (GetIsLosslessCompression())
        return !GetIsReduceImageResolution();

    // FIXME: ideally we'd also pre-empt the DPI related scaling too.
    sal_Int32 nCurrentRatio = (100 * aSize.Width() * aSize.Height() * 4) /
                               nSize;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
