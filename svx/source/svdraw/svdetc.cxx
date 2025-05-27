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

#include <sal/config.h>

#include <algorithm>

#if defined _WIN32 && !defined _WIN64
#include <officecfg/Office/Common.hxx>
#endif
#include <svtools/colorcfg.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdedxv.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/svdoole2.hxx>
#include <svx/xfltrit.hxx>
#include <svl/itempool.hxx>
#include <comphelper/configuration.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <svx/xflbckit.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdotable.hxx>
#include <svx/sdrhittesthelper.hxx>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

using namespace ::com::sun::star;

// Global data of the DrawingEngine
SdrGlobalData::SdrGlobalData()
{
    if (!comphelper::IsFuzzing())
    {
        svx::ExtrusionBar::RegisterInterface();
        svx::FontworkBar::RegisterInterface();
    }
}

const LocaleDataWrapper& SdrGlobalData::GetLocaleData()
{
    return GetSysLocale().GetLocaleData();
}

namespace {

struct TheSdrGlobalData: public rtl::Static<SdrGlobalData, TheSdrGlobalData> {};

}

SdrGlobalData & GetSdrGlobalData() {
    return TheSdrGlobalData::get();
}

OLEObjCache::OLEObjCache()
{
    if (!comphelper::IsFuzzing())
    {
// This limit is only useful on 32-bit windows, where we can run out of virtual memory (see tdf#95579)
// For everything else, we are better off keeping it in main memory rather than using our hacky page-out thing
#if defined _WIN32 && !defined _WIN64
        mnSize = officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::get();
#else
        mnSize = SAL_MAX_INT32; // effectively disable the page-out mechanism
#endif
    }
    else
        mnSize = 100;
    mpTimer.reset( new AutoTimer( "svx OLEObjCache pTimer UnloadCheck" ) );
    mpTimer->SetInvokeHandler( LINK(this, OLEObjCache, UnloadCheckHdl) );
    mpTimer->SetTimeout(20000);
    mpTimer->SetStatic();
}

OLEObjCache::~OLEObjCache()
{
    mpTimer->Stop();
}

IMPL_LINK_NOARG(OLEObjCache, UnloadCheckHdl, Timer*, void)
{
    if (mnSize >= maObjs.size())
        return;

    // more objects than configured cache size try to remove objects
    // of course not the freshly inserted one at nIndex=0
    size_t nCount2 = maObjs.size();
    size_t nIndex = nCount2-1;
    while( nIndex && nCount2 > mnSize )
    {
        SdrOle2Obj* pUnloadObj = maObjs[nIndex--];
        if (!pUnloadObj)
            continue;

        try
        {
            // it is important to get object without reinitialization to avoid reentrance
            const uno::Reference< embed::XEmbeddedObject > & xUnloadObj = pUnloadObj->GetObjRef_NoInit();

            bool bUnload = !xUnloadObj || SdrOle2Obj::CanUnloadRunningObj( xUnloadObj, pUnloadObj->GetAspect() );

            // check whether the object can be unloaded before looking for the parent objects
            if ( xUnloadObj.is() && bUnload )
            {
                uno::Reference< frame::XModel > xUnloadModel( xUnloadObj->getComponent(), uno::UNO_QUERY );
                if ( xUnloadModel.is() )
                {
                    for (SdrOle2Obj* pCacheObj : maObjs)
                    {
                        if ( pCacheObj && pCacheObj != pUnloadObj )
                        {
                            uno::Reference< frame::XModel > xParentModel = pCacheObj->GetParentXModel();
                            if ( xUnloadModel == xParentModel )
                            {
                                bUnload = false; // the object has running embedded objects
                                break;
                            }
                        }
                    }
                }
            }

            if (bUnload && UnloadObj(*pUnloadObj))
            {
                // object was successfully unloaded
                RemoveObj(pUnloadObj);
                nCount2 = std::min(nCount2 - 1, maObjs.size());
                if (nIndex >= nCount2)
                    nIndex = nCount2 - 1;
            }
        }
        catch( uno::Exception& )
        {}
    }
}

void OLEObjCache::InsertObj(SdrOle2Obj* pObj)
{
    if (!maObjs.empty())
    {
        SdrOle2Obj* pExistingObj = maObjs.front();
        if ( pObj == pExistingObj )
            // the object is already on the top, nothing has to be changed
            return;
    }

    // get the old position of the object to know whether it is already in container
    std::vector<SdrOle2Obj*>::iterator it = std::find(maObjs.begin(), maObjs.end(), pObj);
    bool bFound = it != maObjs.end();

    if (bFound)
        maObjs.erase(it);
    // insert object into first position
    maObjs.insert(maObjs.begin(), pObj);

    // if a new object was inserted, recalculate the cache
    if (!bFound)
        mpTimer->Invoke();

    if (!bFound || !mpTimer->IsActive())
        mpTimer->Start();
}

void OLEObjCache::RemoveObj(SdrOle2Obj* pObj)
{
    std::vector<SdrOle2Obj*>::iterator it = std::find(maObjs.begin(), maObjs.end(), pObj);
    if (it != maObjs.end())
        maObjs.erase(it);
    if (maObjs.empty())
        mpTimer->Stop();
}

size_t OLEObjCache::size() const
{
    return maObjs.size();
}

SdrOle2Obj* OLEObjCache::operator[](size_t nPos)
{
    return maObjs[nPos];
}

const SdrOle2Obj* OLEObjCache::operator[](size_t nPos) const
{
    return maObjs[nPos];
}

bool OLEObjCache::UnloadObj(SdrOle2Obj& rObj)
{
    bool bUnloaded = false;

    //#i80528# The old mechanism is completely useless, only taking into account if
    // in all views the GrafDraft feature is used. This will nearly never have been the
    // case since no one ever used this option.

    // A much better (and working) criteria would be the VOC contact count.
    // The question is what will happen when i make it work now suddenly? I
    // will try it for 2.4.
    const sdr::contact::ViewContact& rViewContact = rObj.GetViewContact();
    const bool bVisible(rViewContact.HasViewObjectContacts());

    if(!bVisible)
    {
        bUnloaded = rObj.Unload();
    }

    return bUnloaded;
}

std::optional<Color> GetDraftFillColor(const SfxItemSet& rSet)
{
    drawing::FillStyle eFill=rSet.Get(XATTR_FILLSTYLE).GetValue();
    Color aResult;
    switch(eFill)
    {
        case drawing::FillStyle_SOLID:
        {
            aResult = rSet.Get(XATTR_FILLCOLOR).GetColorValue();
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            Color aCol1(rSet.Get(XATTR_FILLHATCH).GetHatchValue().GetColor());
            Color aCol2(COL_WHITE);

            // when hatched background is activated, use object fill color as hatch color
            bool bFillHatchBackground = rSet.Get(XATTR_FILLBACKGROUND).GetValue();
            if(bFillHatchBackground)
            {
                aCol2 = rSet.Get(XATTR_FILLCOLOR).GetColorValue();
            }

            const basegfx::BColor aAverageColor(basegfx::average(aCol1.getBColor(), aCol2.getBColor()));
            aResult = Color(aAverageColor);
            break;
        }
        case drawing::FillStyle_GRADIENT: {
            const basegfx::BGradient& rGrad=rSet.Get(XATTR_FILLGRADIENT).GetGradientValue();
            Color aCol1(Color(rGrad.GetColorStops().front().getStopColor()));
            Color aCol2(Color(rGrad.GetColorStops().back().getStopColor()));
            const basegfx::BColor aAverageColor(basegfx::average(aCol1.getBColor(), aCol2.getBColor()));
            aResult = Color(aAverageColor);
            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            Bitmap aBitmap(rSet.Get(XATTR_FILLBITMAP).GetGraphicObject().GetGraphic().GetBitmapEx().GetBitmap());
            const Size aSize(aBitmap.GetSizePixel());
            const sal_uInt32 nWidth = aSize.Width();
            const sal_uInt32 nHeight = aSize.Height();
            if (nWidth <= 0 || nHeight <= 0)
                return {};

            BitmapScopedReadAccess pAccess(aBitmap);

            if (pAccess)
            {
                sal_uInt32 nRt(0);
                sal_uInt32 nGn(0);
                sal_uInt32 nBl(0);
                const sal_uInt32 nMaxSteps(8);
                const sal_uInt32 nXStep((nWidth > nMaxSteps) ? nWidth / nMaxSteps : 1);
                const sal_uInt32 nYStep((nHeight > nMaxSteps) ? nHeight / nMaxSteps : 1);
                sal_uInt32 nCount(0);

                for(sal_uInt32 nY(0); nY < nHeight; nY += nYStep)
                {
                    for(sal_uInt32 nX(0); nX < nWidth; nX += nXStep)
                    {
                        const BitmapColor aCol2 = pAccess->GetColor(nY, nX);

                        nRt += aCol2.GetRed();
                        nGn += aCol2.GetGreen();
                        nBl += aCol2.GetBlue();
                        nCount++;
                    }
                }

                nRt /= nCount;
                nGn /= nCount;
                nBl /= nCount;

                aResult = Color(sal_uInt8(nRt), sal_uInt8(nGn), sal_uInt8(nBl));
            }
            break;
        }
        default:
            return {};
    }

    sal_uInt16 nTransparencyPercentage = rSet.Get(XATTR_FILLTRANSPARENCE).GetValue();
    if (!nTransparencyPercentage)
        return aResult;

    auto nTransparency = nTransparencyPercentage / 100.0;
    auto nOpacity = 1 - nTransparency;

    svtools::ColorConfig aColorConfig;
    Color aBackground(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);

    // https://en.wikipedia.org/wiki/Alpha_compositing
    // We are here calculating transparency fill color against background with
    // To put it is simple words with example
    // I.E: fill is Red (FF0000) and background is pure white (FFFFFF)
    // If we add 50% transparency to fill color will look like Pink(ff7777)

    // TODO: calculate this colors based on object in background  and not just the doc color
    aResult.SetRed(
        std::min(aResult.GetRed() * nOpacity + aBackground.GetRed() * nTransparency, 255.0));
    aResult.SetGreen(
        std::min(aResult.GetGreen() * nOpacity + aBackground.GetGreen() * nTransparency, 255.0));
    aResult.SetBlue(
        std::min(aResult.GetBlue() * nOpacity + aBackground.GetBlue() * nTransparency, 255.0));
    return aResult;
}

std::unique_ptr<SdrOutliner> SdrMakeOutliner(OutlinerMode nOutlinerMode, SdrModel& rModel)
{
    SfxItemPool* pPool = &rModel.GetItemPool();
    std::unique_ptr<SdrOutliner> pOutl(new SdrOutliner( pPool, nOutlinerMode ));
    pOutl->SetEditTextObjectPool( pPool );
    pOutl->SetStyleSheetPool( static_cast<SfxStyleSheetPool*>(rModel.GetStyleSheetPool()));
    pOutl->SetDefTab(rModel.GetDefaultTabulator());
    Outliner::SetForbiddenCharsTable(rModel.GetForbiddenCharsTable());
    pOutl->SetAsianCompressionMode(rModel.GetCharCompressType());
    pOutl->SetKernAsianPunctuation(rModel.IsKernAsianPunctuation());
    pOutl->SetAddExtLeading(rModel.IsAddExtLeading());
    return pOutl;
}

std::vector<Link<SdrObjCreatorParams, rtl::Reference<SdrObject>>>& ImpGetUserMakeObjHdl()
{
    SdrGlobalData& rGlobalData=GetSdrGlobalData();
    return rGlobalData.maUserMakeObjHdl;
}

bool SearchOutlinerItems(const SfxItemSet& rSet, bool bInklDefaults, bool* pbOnlyEE)
{
    bool bHas=false;
    bool bOnly=true;
    bool bLookOnly=pbOnlyEE!=nullptr;
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich=aIter.FirstWhich();
    while (((bLookOnly && bOnly) || !bHas) && nWhich!=0) {
        // For bInklDefaults, the entire Which range is decisive,
        // in other cases only the set items are.
        // Disabled and DontCare are regarded as holes in the Which range.
        SfxItemState eState=aIter.GetItemState();
        if ((eState==SfxItemState::DEFAULT && bInklDefaults) || eState==SfxItemState::SET) {
            if (nWhich<EE_ITEMS_START || nWhich>EE_ITEMS_END) bOnly=false;
            else bHas=true;
        }
        nWhich=aIter.NextWhich();
    }
    if (!bHas) bOnly=false;
    if (pbOnlyEE!=nullptr) *pbOnlyEE=bOnly;
    return bHas;
}

WhichRangesContainer RemoveWhichRange(const WhichRangesContainer& pOldWhichTable, sal_uInt16 nRangeBeg, sal_uInt16 nRangeEnd)
{
    // Six possible cases (per range):
    //         [Beg..End]          [nRangeBeg, nRangeEnd], to delete
    // [b..e]    [b..e]    [b..e]  Cases 1,3,2: doesn't matter, delete, doesn't matter  + Ranges
    // [b........e]  [b........e]  Cases 4,5  : shrink range                            | in
    // [b......................e]  Case  6    : splitting                               + pOldWhichTable
    std::vector<WhichPair> buf;
    for (const auto & rPair : pOldWhichTable) {
        auto const begin = rPair.first;
        auto const end = rPair.second;
        if (end < nRangeBeg || begin > nRangeEnd) { // cases 1, 2
            buf.push_back({begin, end});
        } else if (begin >= nRangeBeg && end <= nRangeEnd) { // case 3
            // drop
        } else if (end <= nRangeEnd) { // case 4
            buf.push_back({begin, nRangeBeg - 1});
        } else if (begin >= nRangeBeg) { // case 5
            buf.push_back({nRangeEnd + 1, end});
        } else { // case 6
            buf.push_back({begin, nRangeBeg - 1});
            buf.push_back({nRangeEnd + 1, end});
        }
    }
    std::unique_ptr<WhichPair[]> pNewWhichTable(new WhichPair[buf.size()]);
    std::copy(buf.begin(), buf.end(), pNewWhichTable.get());
    return WhichRangesContainer(std::move(pNewWhichTable), buf.size());
}


SvdProgressInfo::SvdProgressInfo( const Link<void*,bool>&_rLink )
{
    maLink = _rLink;
    m_nSumCurAction   = 0;

    m_nObjCount = 0;
    m_nCurObj   = 0;

    m_nActionCount = 0;
    m_nCurAction   = 0;

    m_nInsertCount = 0;
    m_nCurInsert   = 0;
}

void SvdProgressInfo::Init( size_t nObjCount )
{
    m_nObjCount = nObjCount;
}

bool SvdProgressInfo::ReportActions( size_t nActionCount )
{
    m_nSumCurAction += nActionCount;
    m_nCurAction += nActionCount;
    if(m_nCurAction > m_nActionCount)
        m_nCurAction = m_nActionCount;

    return maLink.Call(nullptr);
}

void SvdProgressInfo::ReportInserts( size_t nInsertCount )
{
    m_nSumCurAction += nInsertCount;
    m_nCurInsert += nInsertCount;

    maLink.Call(nullptr);
}

void SvdProgressInfo::ReportRescales( size_t nRescaleCount )
{
    m_nSumCurAction += nRescaleCount;
    maLink.Call(nullptr);
}

void SvdProgressInfo::SetActionCount( size_t nActionCount )
{
    m_nActionCount = nActionCount;
}

void SvdProgressInfo::SetInsertCount( size_t nInsertCount )
{
    m_nInsertCount = nInsertCount;
}

void SvdProgressInfo::SetNextObject()
{
    m_nActionCount = 0;
    m_nCurAction   = 0;

    m_nInsertCount = 0;
    m_nCurInsert   = 0;

    m_nCurObj++;
    ReportActions(0);
}

// #i101872# isolate GetTextEditBackgroundColor to tooling; it will anyways only be used as long
// as text edit is not running on overlay

namespace
{
    std::optional<Color> impGetSdrObjListFillColor(
        const SdrObjList& rList,
        const Point& rPnt,
        const SdrPageView& rTextEditPV,
        const SdrLayerIDSet& rVisLayers)
    {
        bool bMaster(rList.getSdrPageFromSdrObjList() && rList.getSdrPageFromSdrObjList()->IsMasterPage());

        for(size_t no(rList.GetObjCount()); no > 0; )
        {
            no--;
            SdrObject* pObj = rList.GetObj(no);
            SdrObjList* pOL = pObj->GetSubList();

            if(pOL)
            {
                // group object
                if (auto oColor = impGetSdrObjListFillColor(*pOL, rPnt, rTextEditPV, rVisLayers))
                    return oColor;
            }
            else
            {
                SdrTextObj* pText = DynCastSdrTextObj(pObj);

                // Exclude zero master page object (i.e. background shape) from color query
                if(pText
                    && pObj->IsClosedObj()
                    && (!bMaster || (!pObj->IsNotVisibleAsMaster() && 0 != no))
                    && pObj->GetCurrentBoundRect().Contains(rPnt)
                    && !pText->IsHideContour()
                    && SdrObjectPrimitiveHit(*pObj, rPnt, {0, 0}, rTextEditPV, &rVisLayers, false))
                {
                    if (auto oColor = GetDraftFillColor(pObj->GetMergedItemSet()))
                        return oColor;
                }
            }
        }

        return {};
    }

    std::optional<Color> impGetSdrPageFillColor(
        const SdrPage& rPage,
        const Point& rPnt,
        const SdrPageView& rTextEditPV,
        const SdrLayerIDSet& rVisLayers,
        bool bSkipBackgroundShape)
    {
        if (auto oColor = impGetSdrObjListFillColor(rPage, rPnt, rTextEditPV, rVisLayers))
            return oColor;

        if(!rPage.IsMasterPage())
        {
            if(rPage.TRG_HasMasterPage())
            {
                SdrLayerIDSet aSet(rVisLayers);
                aSet &= rPage.TRG_GetMasterPageVisibleLayers();
                SdrPage& rMasterPage = rPage.TRG_GetMasterPage();

                // Don't fall back to background shape on
                // master pages. This is later handled by
                // GetBackgroundColor, and is necessary to cater for
                // the silly ordering: 1. shapes, 2. master page
                // shapes, 3. page background, 4. master page
                // background.
                if (auto oColor = impGetSdrPageFillColor(rMasterPage, rPnt, rTextEditPV, aSet, true))
                    return oColor;
            }
        }

        // Only now determine background color from background shapes
        if(!bSkipBackgroundShape)
        {
            return rPage.GetPageBackgroundColor();
        }

        return {};
    }

    Color impCalcBackgroundColor(
        const tools::Rectangle& rArea,
        const SdrPageView& rTextEditPV,
        const SdrPage& rPage)
    {
        svtools::ColorConfig aColorConfig;
        Color aBackground(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        if(!rStyleSettings.GetHighContrastMode())
        {
            // search in page
            const sal_uInt16 SPOTCOUNT(5);
            Point aSpotPos[SPOTCOUNT];
            Color aSpotColor[SPOTCOUNT];
            sal_uInt32 nHeight( rArea.GetSize().Height() );
            sal_uInt32 nWidth( rArea.GetSize().Width() );
            sal_uInt32 nWidth14  = nWidth / 4;
            sal_uInt32 nHeight14 = nHeight / 4;
            sal_uInt32 nWidth34  = ( 3 * nWidth ) / 4;
            sal_uInt32 nHeight34 = ( 3 * nHeight ) / 4;

            sal_uInt16 i;
            for ( i = 0; i < SPOTCOUNT; i++ )
            {
                // five spots are used
                switch ( i )
                {
                    case 0 :
                    {
                        // Center-Spot
                        aSpotPos[i] = rArea.Center();
                    }
                    break;

                    case 1 :
                    {
                        // TopLeft-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].AdjustX(nWidth14 );
                        aSpotPos[i].AdjustY(nHeight14 );
                    }
                    break;

                    case 2 :
                    {
                        // TopRight-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].AdjustX(nWidth34 );
                        aSpotPos[i].AdjustY(nHeight14 );
                    }
                    break;

                    case 3 :
                    {
                        // BottomLeft-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].AdjustX(nWidth14 );
                        aSpotPos[i].AdjustY(nHeight34 );
                    }
                    break;

                    case 4 :
                    {
                        // BottomRight-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].AdjustX(nWidth34 );
                        aSpotPos[i].AdjustY(nHeight34 );
                    }
                    break;

                }

                aSpotColor[i] =
                    impGetSdrPageFillColor(rPage, aSpotPos[i], rTextEditPV, rTextEditPV.GetVisibleLayers(), false).value_or(COL_WHITE);
            }

            sal_uInt16 aMatch[SPOTCOUNT];

            for ( i = 0; i < SPOTCOUNT; i++ )
            {
                // were same spot colors found?
                aMatch[i] = 0;

                for ( sal_uInt16 j = 0; j < SPOTCOUNT; j++ )
                {
                    if( j != i )
                    {
                        if( aSpotColor[i] == aSpotColor[j] )
                        {
                            aMatch[i]++;
                        }
                    }
                }
            }

            // highest weight to center spot
            aBackground = aSpotColor[0];

            for ( sal_uInt16 nMatchCount = SPOTCOUNT - 1; nMatchCount > 1; nMatchCount-- )
            {
                // which spot color was found most?
                for ( i = 0; i < SPOTCOUNT; i++ )
                {
                    if( aMatch[i] == nMatchCount )
                    {
                        aBackground = aSpotColor[i];
                        nMatchCount = 1;   // break outer for-loop
                        break;
                    }
                }
            }
        }

        return aBackground;
    }
} // end of anonymous namespace

Color GetTextEditBackgroundColor(const SdrObjEditView& rView)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if(!rStyleSettings.GetHighContrastMode())
    {
        SdrTextObj* pText = rView.GetTextEditObject();

        if(pText && pText->IsClosedObj())
        {
            sdr::table::SdrTableObj* pTable = dynamic_cast< sdr::table::SdrTableObj * >( pText );

            if( pTable )
                if (auto oColor = GetDraftFillColor(pTable->GetActiveCellItemSet()))
                    return *oColor;

            if (auto oColor = GetDraftFillColor(pText->GetMergedItemSet()))
                return *oColor;
        }

        if (pText)
        {
            SdrPageView* pTextEditPV = rView.GetTextEditPageView();

            if(pTextEditPV)
            {
                Point aPvOfs(pText->GetTextEditOffset());
                const SdrPage* pPg = pTextEditPV->GetPage();

                if(pPg)
                {
                    tools::Rectangle aSnapRect( pText->GetSnapRect() );
                    aSnapRect.Move(aPvOfs.X(), aPvOfs.Y());

                    return impCalcBackgroundColor(aSnapRect, *pTextEditPV, *pPg);
                }
            }
        }
    }

    return svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
