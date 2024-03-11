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

#include <scitems.hxx>
#include <editeng/eeitem.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/unit_conversion.hxx>
#include <o3tl/string_view.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/justifyitem.hxx>

#include <vcl/svapp.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>

#include <viewdata.hxx>
#include <docoptio.hxx>
#include <scmod.hxx>
#include <global.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <attrib.hxx>
#include <tabview.hxx>
#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <patattr.hxx>
#include <editutil.hxx>
#include <scextopt.hxx>
#include <miscuno.hxx>
#include <unonames.hxx>
#include <inputopt.hxx>
#include <inputhdl.hxx>
#include <inputwin.hxx>
#include <viewutil.hxx>
#include <markdata.hxx>
#include <ViewSettingsSequenceDefines.hxx>
#include <gridwin.hxx>
#include <transobj.hxx>
#include <clipparam.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

using namespace com::sun::star;

#define SC_GROWY_SMALL_EXTRA    100
#define SC_GROWY_BIG_EXTRA      200

constexpr OUString TAG_TABBARWIDTH = u"tw:"_ustr;

namespace {

void lcl_LOKRemoveWindow(ScTabViewShell* pTabViewShell, ScSplitPos eWhich)
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        auto lRemoveWindows =
                [pTabViewShell, eWhich] (ScTabViewShell* pOtherViewShell)
                { pOtherViewShell->RemoveWindowFromForeignEditView(pTabViewShell, eWhich); };

        SfxLokHelper::forEachOtherView(pTabViewShell, lRemoveWindows);
    }
}

} // anonymous namespace

namespace {

void collectUIInformation(std::map<OUString, OUString>&& aParameters, const OUString& rAction)
{
    EventDescription aDescription;
    aDescription.aID = "grid_window";
    aDescription.aAction = rAction;
    aDescription.aParameters = std::move(aParameters);
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "ScGridWinUIObject";

    UITestLogger::getInstance().logEvent(aDescription);
}
}

const ScPositionHelper::index_type ScPositionHelper::null; // definition

bool ScPositionHelper::Comp::operator() (const value_type& rValue1, const value_type& rValue2) const
{
    if (rValue1.first == null || rValue2.first == null)
    {
        return rValue1.second < rValue2.second;
    }
    else
    {
        return rValue1.first < rValue2.first;
    }
}

ScPositionHelper::ScPositionHelper(const ScDocument *pDoc, bool bColumn)
    : MAX_INDEX(bColumn ? (pDoc ? pDoc->MaxCol() : -1) : MAXTILEDROW)
{
    mData.insert(std::make_pair(-1, 0));
}

void ScPositionHelper::setDocument(const ScDocument& rDoc, bool bColumn)
{
    MAX_INDEX = bColumn ? rDoc.MaxCol() : MAXTILEDROW;
}

void ScPositionHelper::insert(index_type nIndex, tools::Long nPos)
{
    if (nIndex < 0) return;
    SAL_INFO("sc.lok.poshelper", "ScPositionHelper::insert: nIndex: "
            << nIndex << ", nPos: " << nPos << ", size: " << mData.size());
    value_type aValue = std::make_pair(nIndex, nPos);
    mData.erase(aValue);
    mData.insert(aValue);
    SAL_INFO("sc.lok.poshelper",
            "ScPositionHelper::insert: after insert: size: " << mData.size());
}

void ScPositionHelper::removeByIndex(index_type nIndex)
{
    if (nIndex < 0)
        return;
    SAL_INFO("sc.lok.poshelper", "ScPositionHelper::remove: nIndex: " << nIndex
            << ", size: " << mData.size());
    auto it = mData.find(std::make_pair(nIndex, 0));
    if (it == mData.end()) return;
    mData.erase(it);
    SAL_INFO("sc.lok.poshelper",
            "ScPositionHelper::remove: after erase: size: " << mData.size());
}

void ScPositionHelper::invalidateByIndex(index_type nIndex)
{
    SAL_INFO("sc.lok.poshelper", "ScPositionHelper::invalidate: nIndex: " << nIndex);
    if (nIndex < 0)
    {
        mData.clear();
        mData.insert(std::make_pair(-1, 0));
    }
    else
    {
        auto it = mData.lower_bound(std::make_pair(nIndex, 0));
        mData.erase(it, mData.end());
    }
}

void ScPositionHelper::invalidateByPosition(tools::Long nPos)
{
    SAL_INFO("sc.lok.poshelper", "ScPositionHelper::invalidate: nPos: " << nPos);
    if (nPos <= 0)
    {
        mData.clear();
        mData.insert(std::make_pair(-1, 0));
    }
    else
    {
        auto it = mData.lower_bound(std::make_pair(null, nPos));
        mData.erase(it, mData.end());
    }
}

const ScPositionHelper::value_type&
ScPositionHelper::getNearestByIndex(index_type nIndex) const
{
    SAL_INFO("sc.lok.poshelper",
            "ScPositionHelper::getNearest: nIndex: " << nIndex << ", size: " << mData.size());
    auto posUB = mData.upper_bound(std::make_pair(nIndex, 0));
    if (posUB == mData.begin())
    {
        return *posUB;
    }

    auto posLB = std::prev(posUB);
    // coverity[copy_paste_error : FALSE] - posUB is correct
    if (posUB == mData.end())
    {
        return *posLB;
    }

    tools::Long nDiffUB = posUB->first - nIndex;
    tools::Long nDiffLB = posLB->first - nIndex;
    if (nDiffUB < -nDiffLB)
    {
        return *posUB;
    }
    else
    {
        return *posLB;
    }
}

const ScPositionHelper::value_type&
ScPositionHelper::getNearestByPosition(tools::Long nPos) const
{
    SAL_INFO("sc.lok.poshelper",
            "ScPositionHelper::getNearest: nPos: " << nPos << ", size: " << mData.size());
    auto posUB = mData.upper_bound(std::make_pair(null, nPos));

    if (posUB == mData.begin())
    {
        return *posUB;
    }

    auto posLB = std::prev(posUB);
    // coverity[copy_paste_error : FALSE] - posUB is correct
    if (posUB == mData.end())
    {
        return *posLB;
    }

    tools::Long nDiffUB = posUB->second - nPos;
    tools::Long nDiffLB = posLB->second - nPos;

    if (nDiffUB < -nDiffLB)
    {
        return *posUB;
    }
    else
    {
        return *posLB;
    }
}

tools::Long ScPositionHelper::getPosition(index_type nIndex) const
{
    auto it = mData.find(std::make_pair(nIndex, 0));
    if (it == mData.end()) return -1;
    return it->second;
}

tools::Long ScPositionHelper::computePosition(index_type nIndex, const std::function<long (index_type)>& getSizePx)
{
    assert(MAX_INDEX > 0);
    if (nIndex < 0) nIndex = 0;
    if (nIndex > MAX_INDEX) nIndex = MAX_INDEX;

    const auto& rNearest = getNearestByIndex(nIndex);
    index_type nStartIndex = rNearest.first;
    tools::Long nTotalPixels = rNearest.second;

    if (nStartIndex < nIndex)
    {
        for (index_type nIdx = nStartIndex + 1; nIdx <= nIndex; ++nIdx)
        {
            nTotalPixels += getSizePx(nIdx);
        }
    }
    else
    {
        for (index_type nIdx = nStartIndex; nIdx > nIndex; --nIdx)
        {
            nTotalPixels -= getSizePx(nIdx);
        }
    }
    return nTotalPixels;
}

ScBoundsProvider::ScBoundsProvider(const ScViewData &rView, SCTAB nT, bool bColHeader)
    : rDoc(rView.GetDocument())
    , nTab(nT)
    , bColumnHeader(bColHeader)
    , MAX_INDEX(bColHeader ? rDoc.MaxCol() : MAXTILEDROW)
    , mfPPTX(rView.GetPPTX())
    , mfPPTY(rView.GetPPTY())
    , nFirstIndex(-1)
    , nSecondIndex(-1)
    , nFirstPositionPx(-1)
    , nSecondPositionPx(-1)
{}

void ScBoundsProvider::GetStartIndexAndPosition(SCCOL& nIndex, tools::Long& nPosition) const
{
    assert(bColumnHeader);
    nIndex = nFirstIndex;
    nPosition = nFirstPositionPx;
}

void ScBoundsProvider::GetEndIndexAndPosition(SCCOL& nIndex, tools::Long& nPosition) const
{
    assert(bColumnHeader);
    nIndex = nSecondIndex;
    nPosition = nSecondPositionPx;
}

void ScBoundsProvider::GetStartIndexAndPosition(SCROW& nIndex, tools::Long& nPosition) const
{
    assert(!bColumnHeader);
    nIndex = nFirstIndex;
    nPosition = nFirstPositionPx;
}

void ScBoundsProvider::GetEndIndexAndPosition(SCROW& nIndex, tools::Long& nPosition) const
{
    assert(!bColumnHeader);
    nIndex = nSecondIndex;
    nPosition = nSecondPositionPx;
}

tools::Long ScBoundsProvider::GetSize(index_type nIndex) const
{
    const sal_uInt16 nSize = bColumnHeader ? rDoc.GetColWidth(nIndex, nTab) : rDoc.GetRowHeight(nIndex, nTab);
    return ScViewData::ToPixel(nSize, bColumnHeader ? mfPPTX : mfPPTY);
}

void ScBoundsProvider::GetIndexAndPos(index_type nNearestIndex, tools::Long nNearestPosition,
                    tools::Long nBound, index_type& nFoundIndex, tools::Long& nPosition,
                    bool bTowards, tools::Long nDiff)
{
    if (nDiff > 0) // nBound < nNearestPosition
        GeIndexBackwards(nNearestIndex, nNearestPosition, nBound,
                         nFoundIndex, nPosition, bTowards);
    else
        GetIndexTowards(nNearestIndex, nNearestPosition, nBound,
                        nFoundIndex, nPosition, bTowards);
}

void ScBoundsProvider::Compute(
            value_type aFirstNearest, value_type aSecondNearest,
            tools::Long nFirstBound, tools::Long nSecondBound)
{
    SAL_INFO("sc.lok.header", "BoundsProvider: nFirstBound: " << nFirstBound
            << ", nSecondBound: " << nSecondBound);

    tools::Long nFirstDiff = aFirstNearest.second - nFirstBound;
    tools::Long nSecondDiff = aSecondNearest.second - nSecondBound;
    SAL_INFO("sc.lok.header", "BoundsProvider: rTopNearest: index: " << aFirstNearest.first
            << ", pos: " << aFirstNearest.second << ", diff: " << nFirstDiff);
    SAL_INFO("sc.lok.header", "BoundsProvider: rBottomNearest: index: " << aSecondNearest.first
            << ", pos: " << aSecondNearest.second << ", diff: " << nSecondDiff);

    bool bReverse = (std::abs(nFirstDiff) >= std::abs(nSecondDiff));

    if(bReverse)
    {
        std::swap(aFirstNearest, aSecondNearest);
        std::swap(nFirstBound, nSecondBound);
        std::swap(nFirstDiff, nSecondDiff);
    }

    index_type nNearestIndex = aFirstNearest.first;
    tools::Long nNearestPosition = aFirstNearest.second;
    SAL_INFO("sc.lok.header", "BoundsProvider: nearest to first bound:  nNearestIndex: "
            << nNearestIndex << ", nNearestPosition: " << nNearestPosition);

    GetIndexAndPos(nNearestIndex, nNearestPosition, nFirstBound,
                   nFirstIndex, nFirstPositionPx, !bReverse, nFirstDiff);
    SAL_INFO("sc.lok.header", "BoundsProvider: nFirstIndex: " << nFirstIndex
            << ", nFirstPositionPx: " << nFirstPositionPx);

    if (std::abs(nSecondDiff) < std::abs(nSecondBound - nFirstPositionPx))
    {
        nNearestIndex = aSecondNearest.first;
        nNearestPosition = aSecondNearest.second;
    }
    else
    {
        nNearestPosition = nFirstPositionPx;
        nNearestIndex = nFirstIndex;
        nSecondDiff = !bReverse ? -1 : 1;
    }
    SAL_INFO("sc.lok.header", "BoundsProvider: nearest to second bound: nNearestIndex: "
            << nNearestIndex << ", nNearestPosition: " << nNearestPosition
            << ", diff: " << nSecondDiff);

    GetIndexAndPos(nNearestIndex, nNearestPosition, nSecondBound,
                   nSecondIndex, nSecondPositionPx, bReverse, nSecondDiff);
    SAL_INFO("sc.lok.header", "BoundsProvider: nSecondIndex: " << nSecondIndex
            << ", nSecondPositionPx: " << nSecondPositionPx);

    if (bReverse)
    {
        std::swap(nFirstIndex, nSecondIndex);
        std::swap(nFirstPositionPx, nSecondPositionPx);
    }
}

void ScBoundsProvider::EnlargeStartBy(tools::Long nOffset)
{
    const index_type nNewFirstIndex =
            std::max(static_cast<index_type>(-1),
                     static_cast<index_type>(nFirstIndex - nOffset));
    for (index_type nIndex = nFirstIndex; nIndex > nNewFirstIndex; --nIndex)
    {
        const tools::Long nSizePx = GetSize(nIndex);
        nFirstPositionPx -= nSizePx;
    }
    nFirstIndex = nNewFirstIndex;
    SAL_INFO("sc.lok.header", "BoundsProvider: added offset: nFirstIndex: " << nFirstIndex
            << ", nFirstPositionPx: " << nFirstPositionPx);
}

void ScBoundsProvider::EnlargeEndBy(tools::Long nOffset)
{
    const index_type nNewSecondIndex = std::min(MAX_INDEX, static_cast<index_type>(nSecondIndex + nOffset));
    for (index_type nIndex = nSecondIndex + 1; nIndex <= nNewSecondIndex; ++nIndex)
    {
        const tools::Long nSizePx = GetSize(nIndex);
        nSecondPositionPx += nSizePx;
    }
    nSecondIndex = nNewSecondIndex;
    SAL_INFO("sc.lok.header", "BoundsProvider: added offset: nSecondIndex: " << nSecondIndex
            << ", nSecondPositionPx: " << nSecondPositionPx);
}

void ScBoundsProvider::GeIndexBackwards(
            index_type nNearestIndex, tools::Long nNearestPosition,
            tools::Long nBound, index_type& nFoundIndex, tools::Long& nPosition, bool bTowards)
{
    nFoundIndex = -1;
    for (index_type nIndex = nNearestIndex; nIndex >= 0; --nIndex)
    {
        if (nBound >= nNearestPosition)
        {
            nFoundIndex = nIndex; // last index whose nPosition is less than nBound
            nPosition = nNearestPosition;
            break;
        }

        const tools::Long nSizePx = GetSize(nIndex);
        nNearestPosition -= nSizePx;
    }
    if (!bTowards && nFoundIndex != -1)
    {
        nFoundIndex += 1;
        nPosition += GetSize(nFoundIndex);
    }
}

void ScBoundsProvider::GetIndexTowards(
            index_type nNearestIndex, tools::Long nNearestPosition,
            tools::Long nBound, index_type& nFoundIndex, tools::Long& nPosition, bool bTowards)
{
    nFoundIndex = -2;
    for (index_type nIndex = nNearestIndex + 1; nIndex <= MAX_INDEX; ++nIndex)
    {
        const tools::Long nSizePx = GetSize(nIndex);
        nNearestPosition += nSizePx;

        if (nNearestPosition > nBound)
        {
            nFoundIndex = nIndex; // first index whose nPosition is greater than nBound
            nPosition = nNearestPosition;
            break;
        }
    }
    if (nFoundIndex == -2)
    {
        nFoundIndex = MAX_INDEX;
        nPosition = nNearestPosition;
    }
    else if (bTowards)
    {
        nPosition -= GetSize(nFoundIndex);
        nFoundIndex -= 1;
    }
}

ScViewDataTable::ScViewDataTable(const ScDocument *pDoc) :
                eZoomType( SvxZoomType::PERCENT ),
                aZoomX( 1,1 ),
                aZoomY( 1,1 ),
                aPageZoomX( 3,5 ),              // Page-Default: 60%
                aPageZoomY( 3,5 ),
                nHSplitPos( 0 ),
                nVSplitPos( 0 ),
                eHSplitMode( SC_SPLIT_NONE ),
                eVSplitMode( SC_SPLIT_NONE ),
                eWhichActive( SC_SPLIT_BOTTOMLEFT ),
                nFixPosX( 0 ),
                nFixPosY( 0 ),
                nCurX( 0 ),
                nCurY( 0 ),
                nOldCurX( 0 ),
                nOldCurY( 0 ),
                aWidthHelper(pDoc, true),
                aHeightHelper(pDoc, false),
                nMaxTiledCol( 20 ),
                nMaxTiledRow( 50 ),
                bShowGrid( true ),
                mbOldCursorValid( false )
{
    nPosX[0]=nPosX[1]=0;
    nPosY[0]=nPosY[1]=0;
    nTPosX[0]=nTPosX[1]=0;
    nTPosY[0]=nTPosY[1]=0;
    nMPosX[0]=nMPosX[1]=0;
    nMPosY[0]=nMPosY[1]=0;
    nPixPosX[0]=nPixPosX[1]=0;
    nPixPosY[0]=nPixPosY[1]=0;
}

void ScViewDataTable::InitData(const ScDocument& rDoc)
{
    aWidthHelper.setDocument(rDoc, true);
    aHeightHelper.setDocument(rDoc, false);
}

void ScViewDataTable::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rSettings, const ScViewData& rViewData, SCTAB nTab) const
{
    rSettings.realloc(SC_TABLE_VIEWSETTINGS_COUNT);
    beans::PropertyValue* pSettings = rSettings.getArray();

    ScSplitMode eExHSplitMode = eHSplitMode;
    ScSplitMode eExVSplitMode = eVSplitMode;
    SCCOL nExFixPosX = nFixPosX;
    SCROW nExFixPosY = nFixPosY;
    tools::Long nExHSplitPos = nHSplitPos;
    tools::Long nExVSplitPos = nVSplitPos;

    if (comphelper::LibreOfficeKit::isActive())
    {
        rViewData.OverrideWithLOKFreeze(eExHSplitMode, eExVSplitMode,
                                        nExFixPosX, nExFixPosY,
                                        nExHSplitPos, nExVSplitPos, nTab);
    }

    pSettings[SC_CURSOR_X].Name = SC_CURSORPOSITIONX;
    pSettings[SC_CURSOR_X].Value <<= sal_Int32(nCurX);
    pSettings[SC_CURSOR_Y].Name = SC_CURSORPOSITIONY;
    pSettings[SC_CURSOR_Y].Value <<= sal_Int32(nCurY);

    // Write freezepan data only when freeze pans are set
    if(nExFixPosX != 0 || nExFixPosY != 0 || nExHSplitPos != 0 || nExVSplitPos != 0)
    {
        pSettings[SC_HORIZONTAL_SPLIT_MODE].Name = SC_HORIZONTALSPLITMODE;
        pSettings[SC_HORIZONTAL_SPLIT_MODE].Value <<= sal_Int16(eExHSplitMode);
        pSettings[SC_VERTICAL_SPLIT_MODE].Name = SC_VERTICALSPLITMODE;
        pSettings[SC_VERTICAL_SPLIT_MODE].Value <<= sal_Int16(eExVSplitMode);
        pSettings[SC_HORIZONTAL_SPLIT_POSITION].Name = SC_HORIZONTALSPLITPOSITION;
        if (eExHSplitMode == SC_SPLIT_FIX)
            pSettings[SC_HORIZONTAL_SPLIT_POSITION].Value <<= sal_Int32(nExFixPosX);
        else
            pSettings[SC_HORIZONTAL_SPLIT_POSITION].Value <<= sal_Int32(nExHSplitPos);
        pSettings[SC_VERTICAL_SPLIT_POSITION].Name = SC_VERTICALSPLITPOSITION;
        if (eExVSplitMode == SC_SPLIT_FIX)
            pSettings[SC_VERTICAL_SPLIT_POSITION].Value <<= sal_Int32(nExFixPosY);
        else
            pSettings[SC_VERTICAL_SPLIT_POSITION].Value <<= sal_Int32(nExVSplitPos);
    }

    // Prevent writing odd settings that would make crash versions that
    // don't apply SanitizeWhichActive() when reading the settings.
    // See tdf#117093
    const ScSplitPos eActiveSplitRange = SanitizeWhichActive();
    // And point out to give us a chance to inspect weird things (if anyone
    // remembers what s/he did).
    assert(eWhichActive == eActiveSplitRange);
    pSettings[SC_ACTIVE_SPLIT_RANGE].Name = SC_ACTIVESPLITRANGE;
    pSettings[SC_ACTIVE_SPLIT_RANGE].Value <<= sal_Int16(eActiveSplitRange);
    pSettings[SC_POSITION_LEFT].Name = SC_POSITIONLEFT;
    pSettings[SC_POSITION_LEFT].Value <<= sal_Int32(nPosX[SC_SPLIT_LEFT]);
    pSettings[SC_POSITION_RIGHT].Name = SC_POSITIONRIGHT;
    pSettings[SC_POSITION_RIGHT].Value <<= sal_Int32(nPosX[SC_SPLIT_RIGHT]);
    pSettings[SC_POSITION_TOP].Name = SC_POSITIONTOP;
    pSettings[SC_POSITION_TOP].Value <<= sal_Int32(nPosY[SC_SPLIT_TOP]);
    pSettings[SC_POSITION_BOTTOM].Name = SC_POSITIONBOTTOM;
    pSettings[SC_POSITION_BOTTOM].Value <<= sal_Int32(nPosY[SC_SPLIT_BOTTOM]);

    sal_Int32 nZoomValue = tools::Long(aZoomY * 100);
    sal_Int32 nPageZoomValue = tools::Long(aPageZoomY * 100);
    pSettings[SC_TABLE_ZOOM_TYPE].Name = SC_ZOOMTYPE;
    pSettings[SC_TABLE_ZOOM_TYPE].Value <<= sal_Int16(eZoomType);
    pSettings[SC_TABLE_ZOOM_VALUE].Name = SC_ZOOMVALUE;
    pSettings[SC_TABLE_ZOOM_VALUE].Value <<= nZoomValue;
    pSettings[SC_TABLE_PAGE_VIEW_ZOOM_VALUE].Name = SC_PAGEVIEWZOOMVALUE;
    pSettings[SC_TABLE_PAGE_VIEW_ZOOM_VALUE].Value <<= nPageZoomValue;

    pSettings[SC_TABLE_SHOWGRID].Name = SC_UNO_SHOWGRID;
    pSettings[SC_TABLE_SHOWGRID].Value <<= bShowGrid;

    // Common SdrModel processing
    rViewData.GetDocument().GetDrawLayer()->WriteUserDataSequence(rSettings);
}

void ScViewDataTable::ReadUserDataSequence(const uno::Sequence <beans::PropertyValue>& aSettings, ScViewData& rViewData, SCTAB nTab, bool& rHasZoom )
{
    rHasZoom = false;

    sal_Int32 nTemp32(0);
    sal_Int16 nTemp16(0);
    sal_Int32 nTempPosV(0);
    sal_Int32 nTempPosH(0);
    sal_Int32 nTempPosVTw(0);
    sal_Int32 nTempPosHTw(0);
    bool bHasVSplitInTwips = false;
    bool bHasHSplitInTwips = false;
    for (const auto& rSetting : aSettings)
    {
        OUString sName(rSetting.Name);
        if (sName == SC_CURSORPOSITIONX)
        {
            rSetting.Value >>= nTemp32;
            nCurX = rViewData.GetDocument().SanitizeCol( static_cast<SCCOL>(nTemp32));
        }
        else if (sName == SC_CURSORPOSITIONY)
        {
            rSetting.Value >>= nTemp32;
            nCurY = rViewData.GetDocument().SanitizeRow( static_cast<SCROW>(nTemp32));
        }
        else if (sName == SC_HORIZONTALSPLITMODE)
        {
            if ((rSetting.Value >>= nTemp16) && nTemp16 <= ScSplitMode::SC_SPLIT_MODE_MAX_ENUM)
                eHSplitMode = static_cast<ScSplitMode>(nTemp16);
        }
        else if (sName == SC_VERTICALSPLITMODE)
        {
            if ((rSetting.Value >>= nTemp16) && nTemp16 <= ScSplitMode::SC_SPLIT_MODE_MAX_ENUM)
                eVSplitMode = static_cast<ScSplitMode>(nTemp16);
        }
        else if (sName == SC_HORIZONTALSPLITPOSITION)
        {
            rSetting.Value >>= nTempPosH;
            bHasHSplitInTwips = false;
        }
        else if (sName == SC_VERTICALSPLITPOSITION)
        {
            rSetting.Value >>= nTempPosV;
            bHasVSplitInTwips = false;
        }
        else if (sName == SC_HORIZONTALSPLITPOSITION_TWIPS)
        {
            rSetting.Value >>= nTempPosHTw;
            bHasHSplitInTwips = true;
        }
        else if (sName == SC_VERTICALSPLITPOSITION_TWIPS)
        {
            rSetting.Value >>= nTempPosVTw;
            bHasVSplitInTwips = true;
        }
        else if (sName == SC_ACTIVESPLITRANGE)
        {
            if ((rSetting.Value >>= nTemp16) && nTemp16 <= ScSplitPos::SC_SPLIT_POS_MAX_ENUM)
                eWhichActive = static_cast<ScSplitPos>(nTemp16);
        }
        else if (sName == SC_POSITIONLEFT)
        {
            rSetting.Value >>= nTemp32;
            nPosX[SC_SPLIT_LEFT] = rViewData.GetDocument().SanitizeCol( static_cast<SCCOL>(nTemp32));
        }
        else if (sName == SC_POSITIONRIGHT)
        {
            rSetting.Value >>= nTemp32;
            nPosX[SC_SPLIT_RIGHT] = rViewData.GetDocument().SanitizeCol( static_cast<SCCOL>(nTemp32));
        }
        else if (sName == SC_POSITIONTOP)
        {
            rSetting.Value >>= nTemp32;
            nPosY[SC_SPLIT_TOP] = rViewData.GetDocument().SanitizeRow( static_cast<SCROW>(nTemp32));
        }
        else if (sName == SC_POSITIONBOTTOM)
        {
            rSetting.Value >>= nTemp32;
            nPosY[SC_SPLIT_BOTTOM] = rViewData.GetDocument().SanitizeRow( static_cast<SCROW>(nTemp32));
        }
        else if (sName == SC_ZOOMTYPE)
        {
            rSetting.Value >>= nTemp16;
            eZoomType = SvxZoomType(nTemp16);
            rHasZoom = true;        // set if there is any zoom information
        }
        else if (sName == SC_ZOOMVALUE)
        {
            rSetting.Value >>= nTemp32;
            Fraction aZoom(nTemp32, 100);
            aZoomX = aZoomY = aZoom;
            rHasZoom = true;
        }
        else if (sName == SC_PAGEVIEWZOOMVALUE)
        {
            rSetting.Value >>= nTemp32;
            Fraction aZoom(nTemp32, 100);
            aPageZoomX = aPageZoomY = aZoom;
            rHasZoom = true;
        }
        else if (sName == SC_UNO_SHOWGRID)
        {
            rSetting.Value >>= bShowGrid;
        }
        else if (sName == SC_TABLESELECTED)
        {
            bool bSelected = false;
            rSetting.Value >>= bSelected;
            rViewData.GetMarkData().SelectTable( nTab, bSelected );
        }
        else if (sName == SC_UNONAME_TABCOLOR)
        {
            // There are documents out there that have their tab color defined as a view setting.
            Color aColor = COL_AUTO;
            rSetting.Value >>= aColor;
            if (aColor != COL_AUTO)
            {
                ScDocument& rDoc = rViewData.GetDocument();
                rDoc.SetTabBgColor(nTab, aColor);
            }
        }
        // Fallback to common SdrModel processing
        else rViewData.GetDocument().GetDrawLayer()->ReadUserDataSequenceValue(&rSetting);
    }

    if (eHSplitMode == SC_SPLIT_FIX)
        nFixPosX = rViewData.GetDocument().SanitizeCol( static_cast<SCCOL>( bHasHSplitInTwips ? nTempPosHTw : nTempPosH ));
    else
        nHSplitPos = bHasHSplitInTwips ? static_cast< tools::Long >( nTempPosHTw * rViewData.GetPPTX() ) : nTempPosH;

    if (eVSplitMode == SC_SPLIT_FIX)
        nFixPosY = rViewData.GetDocument().SanitizeRow( static_cast<SCROW>( bHasVSplitInTwips ? nTempPosVTw : nTempPosV ));
    else
        nVSplitPos = bHasVSplitInTwips ? static_cast< tools::Long >( nTempPosVTw * rViewData.GetPPTY() ) : nTempPosV;

    eWhichActive = SanitizeWhichActive();
}

ScSplitPos ScViewDataTable::SanitizeWhichActive() const
{
    if ((WhichH(eWhichActive) == SC_SPLIT_RIGHT && eHSplitMode == SC_SPLIT_NONE) ||
            (WhichV(eWhichActive) == SC_SPLIT_TOP && eVSplitMode == SC_SPLIT_NONE))
    {
        SAL_WARN("sc.ui","ScViewDataTable::SanitizeWhichActive - bad eWhichActive " << eWhichActive);
        // The default always initialized grid window is SC_SPLIT_BOTTOMLEFT.
        return SC_SPLIT_BOTTOMLEFT;
    }
    return eWhichActive;
}

ScViewData::ScViewData(ScDocShell& rDocSh, ScTabViewShell* pViewSh)
    : ScViewData(nullptr, &rDocSh, pViewSh)
{
}

ScViewData::ScViewData(ScDocument& rDoc)
    : ScViewData(&rDoc, nullptr, nullptr)
{
}

static ScViewOptions DefaultOptions()
{
    ScViewOptions aOptions;
    aOptions.SetOption(VOPT_GRID, true);
    aOptions.SetOption(VOPT_SYNTAX, false);
    aOptions.SetOption(VOPT_HEADER, true);
    aOptions.SetOption(VOPT_TABCONTROLS, true);
    aOptions.SetOption(VOPT_VSCROLL, true);
    aOptions.SetOption(VOPT_HSCROLL, true);
    aOptions.SetOption(VOPT_OUTLINER, true);
    return aOptions;
}

// Either pDoc or pDocSh must be valid
ScViewData::ScViewData(ScDocument* pDoc, ScDocShell* pDocSh, ScTabViewShell* pViewSh) :
        nPPTX(0.0),
        nPPTY(0.0),
        maMarkData  (pDocSh ? pDocSh->GetDocument().GetSheetLimits() : pDoc->GetSheetLimits()),
        maHighlightData (pDocSh ? pDocSh->GetDocument().GetSheetLimits() : pDoc->GetSheetLimits()),
        pDocShell   ( pDocSh ),
        mrDoc       (pDocSh ? pDocSh->GetDocument() : *pDoc),
        pView       ( pViewSh ),
        maOptions   (pDocSh ? pDocSh->GetDocument().GetViewOptions() : DefaultOptions()),
        pSpellingView ( nullptr ),
        aLogicMode  ( MapUnit::Map100thMM ),
        eDefZoomType( SvxZoomType::PERCENT ),
        aDefZoomX   ( 1,1 ),
        aDefZoomY   ( 1,1 ),
        aDefPageZoomX( 3,5 ),
        aDefPageZoomY( 3,5 ),
        eRefType    ( SC_REFTYPE_NONE ),
        nTabNo      ( 0 ),
        nRefTabNo   ( 0 ),
        nRefStartX(0),
        nRefStartY(0),
        nRefStartZ(0),
        nRefEndX(0),
        nRefEndY(0),
        nRefEndZ(0),
        nFillStartX(0),
        nFillStartY(0),
        nFillEndX(0),
        nFillEndY(0),
        nPasteFlags ( ScPasteFlags::NONE ),
        eEditActivePart( SC_SPLIT_BOTTOMLEFT ),
        nFillMode   ( ScFillMode::NONE ),
        eEditAdjust ( SvxAdjust::Left ),
        bActive     ( true ),                   // how to initialize?
        bIsRefMode  ( false ),
        bDelMarkValid( false ),
        bPagebreak  ( false ),
        bSelCtrlMouseClick( false ),
        bMoveArea ( false ),
        bGrowing (false),
        nFormulaBarLines(1),
        m_nLOKPageUpDownOffset( 0 )
{
    assert(bool(pDoc) != bool(pDocSh)); // either one or the other, not both
    maMarkData.SelectOneTable(0); // Sync with nTabNo

    aScrSize = Size( o3tl::convert(STD_COL_WIDTH * OLE_STD_CELLS_X, o3tl::Length::twip, o3tl::Length::px),
                    o3tl::convert(mrDoc.GetSheetOptimalMinRowHeight(nTabNo) * OLE_STD_CELLS_Y,
                                  o3tl::Length::twip, o3tl::Length::px));
    maTabData.emplace_back( new ScViewDataTable(nullptr) );
    pThisTab = maTabData[nTabNo].get();

    nEditEndCol = nEditStartCol = nEditCol = 0;
    nEditEndRow = nEditRow = 0;
    nTabStartCol = SC_TABSTART_NONE;

    // don't show hidden tables
    if (!mrDoc.IsVisible(nTabNo))
    {
        while (!mrDoc.IsVisible(nTabNo) && mrDoc.HasTable(nTabNo + 1))
        {
            ++nTabNo;
            maTabData.emplace_back(nullptr);
        }
        maTabData[nTabNo].reset( new ScViewDataTable(nullptr) );
        pThisTab = maTabData[nTabNo].get();
    }

    SCTAB nTableCount = mrDoc.GetTableCount();
    EnsureTabDataSize(nTableCount);

    for (auto& xTabData : maTabData)
    {
        if (xTabData)
            xTabData->InitData(mrDoc);
    }

    CalcPPT();
}

ScViewData::~ScViewData() COVERITY_NOEXCEPT_FALSE
{
    KillEditView();
}

ScDBFunc* ScViewData::GetView() const { return pView; }

void ScViewData::UpdateCurrentTab()
{
    assert(0 <= nTabNo && o3tl::make_unsigned(nTabNo) < maTabData.size());
    pThisTab = maTabData[nTabNo].get();
    while (!pThisTab)
    {
        if (nTabNo > 0)
            pThisTab = maTabData[--nTabNo].get();
        else
        {
            maTabData[0].reset(new ScViewDataTable(&mrDoc));
            pThisTab = maTabData[0].get();
        }
    }
}

void ScViewData::InsertTab( SCTAB nTab )
{
    if( nTab >= static_cast<SCTAB>(maTabData.size()))
        maTabData.resize(nTab+1);
    else
        maTabData.insert( maTabData.begin() + nTab, nullptr );
    CreateTabData( nTab );

    UpdateCurrentTab();
    maMarkData.InsertTab(nTab);

    collectUIInformation({{}}, "InsertTab");
}

void ScViewData::InsertTabs( SCTAB nTab, SCTAB nNewSheets )
{
    if (nTab >= static_cast<SCTAB>(maTabData.size()))
        maTabData.resize(nTab+nNewSheets);
    else
    {
        // insert nNewSheets new tables at position nTab
        auto prevSize = maTabData.size();
        maTabData.resize(prevSize + nNewSheets);
        std::move_backward(maTabData.begin() + nTab, maTabData.begin() + prevSize, maTabData.end());
    }
    for (SCTAB i = nTab; i < nTab + nNewSheets; ++i)
    {
        CreateTabData( i );
        maMarkData.InsertTab(i);
    }
    UpdateCurrentTab();
}

void ScViewData::DeleteTab( SCTAB nTab )
{
    assert(nTab < static_cast<SCTAB>(maTabData.size()));
    maTabData.erase(maTabData.begin() + nTab);

    if (o3tl::make_unsigned(nTabNo) >= maTabData.size())
    {
        EnsureTabDataSize(1);
        nTabNo = maTabData.size() - 1;
    }
    UpdateCurrentTab();
    maMarkData.DeleteTab(nTab);
}

void ScViewData::DeleteTabs( SCTAB nTab, SCTAB nSheets )
{
    for (SCTAB i = 0; i < nSheets; ++i)
    {
        maMarkData.DeleteTab(nTab + i);
    }
    maTabData.erase(maTabData.begin() + nTab, maTabData.begin()+ nTab+nSheets);
    if (o3tl::make_unsigned(nTabNo) >= maTabData.size())
    {
        EnsureTabDataSize(1);
        nTabNo = maTabData.size() - 1;
    }
    UpdateCurrentTab();
}

void ScViewData::CopyTab( SCTAB nSrcTab, SCTAB nDestTab )
{
    if (nDestTab==SC_TAB_APPEND)
        nDestTab = mrDoc.GetTableCount() - 1;   // something had to have been copied

    if (nDestTab > MAXTAB)
    {
        OSL_FAIL("too many sheets");
        return;
    }

    if (nSrcTab >= static_cast<SCTAB>(maTabData.size()))
        OSL_FAIL("pTabData out of bounds, FIX IT");

    EnsureTabDataSize(nDestTab + 1);

    if ( maTabData[nSrcTab] )
        maTabData.emplace(maTabData.begin() + nDestTab, new ScViewDataTable( *maTabData[nSrcTab] ));
    else
        maTabData.insert(maTabData.begin() + nDestTab, nullptr);

    UpdateCurrentTab();
    maMarkData.InsertTab(nDestTab);
}

void ScViewData::MoveTab( SCTAB nSrcTab, SCTAB nDestTab )
{
    if (nDestTab==SC_TAB_APPEND)
        nDestTab = mrDoc.GetTableCount() - 1;
    std::unique_ptr<ScViewDataTable> pTab;
    if (nSrcTab < static_cast<SCTAB>(maTabData.size()))
    {
        pTab = std::move(maTabData[nSrcTab]);
        maTabData.erase( maTabData.begin() + nSrcTab );
    }

    if (nDestTab < static_cast<SCTAB>(maTabData.size()))
        maTabData.insert( maTabData.begin() + nDestTab, std::move(pTab) );
    else
    {
        EnsureTabDataSize(nDestTab + 1);
        maTabData[nDestTab] = std::move(pTab);
    }

    UpdateCurrentTab();
    maMarkData.DeleteTab(nSrcTab);
    maMarkData.InsertTab(nDestTab); // adapted if needed
}

void ScViewData::CreateTabData( std::vector< SCTAB >& rvTabs )
{
    for ( const auto& rTab : rvTabs )
        CreateTabData(rTab);
}

void ScViewData::SetZoomType( SvxZoomType eNew, std::vector< SCTAB >& tabs )
{
    bool bAll = tabs.empty();

    if ( !bAll ) // create associated table data
        CreateTabData( tabs );

    if ( bAll )
    {
        for ( auto & i: maTabData )
        {
            if ( i )
                i->eZoomType = eNew;
        }
        eDefZoomType = eNew;
    }
    else
    {
        for ( const SCTAB& i : tabs )
        {
            if ( i < static_cast<SCTAB>(maTabData.size()) && maTabData[i] )
                maTabData[i]->eZoomType = eNew;
        }
    }
}

void ScViewData::SetZoomType( SvxZoomType eNew, bool bAll )
{
    std::vector< SCTAB > vTabs; // Empty for all tabs
    if ( !bAll ) // get selected tabs
    {
        ScMarkData::const_iterator itr = maMarkData.begin(), itrEnd = maMarkData.end();
        vTabs.insert(vTabs.begin(), itr, itrEnd);
    }
    SetZoomType( eNew, vTabs );
}

void ScViewData::SetZoom( const Fraction& rNewX, const Fraction& rNewY, std::vector< SCTAB >& tabs )
{
    bool bAll = tabs.empty();
    if ( !bAll ) // create associated table data
        CreateTabData( tabs );

    // sanity check - we shouldn't need something this low / big
    SAL_WARN_IF(rNewX < Fraction(1, 100) || rNewX > Fraction(100, 1), "sc.viewdata",
                "fraction rNewX not sensible: " << static_cast<double>(rNewX));
    SAL_WARN_IF(rNewY < Fraction(1, 100) || rNewY > Fraction(100, 1), "sc.viewdata",
                "fraction rNewY not sensible: " << static_cast<double>(rNewY));

    if ( bAll )
    {
        for ( auto & i: maTabData )
        {
            if ( i )
            {
                if ( bPagebreak )
                {
                    i->aPageZoomX = rNewX;
                    i->aPageZoomY = rNewY;
                }
                else
                {
                    i->aZoomX = rNewX;
                    i->aZoomY = rNewY;
                }
            }
        }
        if ( bPagebreak )
        {
            aDefPageZoomX = rNewX;
            aDefPageZoomY = rNewY;
        }
        else
        {
            aDefZoomX = rNewX;
            aDefZoomY = rNewY;
        }
    }
    else
    {
        for ( const SCTAB& i : tabs )
        {
            if ( i < static_cast<SCTAB>(maTabData.size()) && maTabData[i] )
            {
                if ( bPagebreak )
                {
                    maTabData[i]->aPageZoomX = rNewX;
                    maTabData[i]->aPageZoomY = rNewY;
                }
                else
                {
                    maTabData[i]->aZoomX = rNewX;
                    maTabData[i]->aZoomY = rNewY;
                }
            }
        }
    }
    RefreshZoom();
}

void ScViewData::SetZoom( const Fraction& rNewX, const Fraction& rNewY, bool bAll )
{
    std::vector< SCTAB > vTabs;
    if ( !bAll ) // get selected tabs
    {
        ScMarkData::const_iterator itr = maMarkData.begin(), itrEnd = maMarkData.end();
        vTabs.insert(vTabs.begin(), itr, itrEnd);
    }
    SetZoom( rNewX, rNewY, vTabs );
}

void ScViewData::SetShowGrid( bool bShow )
{
    CreateSelectedTabData();
    maTabData[nTabNo]->bShowGrid = bShow;
}

void ScViewData::RefreshZoom()
{
    // recalculate zoom-dependent values (only for current sheet)

    CalcPPT();
    RecalcPixPos();
    aScenButSize = Size(0,0);
    aLogicMode.SetScaleX( GetZoomX() );
    aLogicMode.SetScaleY( GetZoomY() );
}

void ScViewData::SetPagebreakMode( bool bSet )
{
    bPagebreak = bSet;

    RefreshZoom();
}

ScMarkType ScViewData::GetSimpleArea( ScRange & rRange, ScMarkData & rNewMark ) const
{
    ScMarkType eMarkType = SC_MARK_NONE;

    if ( rNewMark.IsMarked() || rNewMark.IsMultiMarked() )
    {
        if ( rNewMark.IsMultiMarked() )
            rNewMark.MarkToSimple();

        if ( rNewMark.IsMarked() && !rNewMark.IsMultiMarked() )
        {
            rRange = rNewMark.GetMarkArea();
            if (ScViewUtil::HasFiltered(rRange, GetDocument()))
                eMarkType = SC_MARK_SIMPLE_FILTERED;
            else
                eMarkType = SC_MARK_SIMPLE;
        }
        else
            eMarkType = SC_MARK_MULTI;
    }
    if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
    {
        if (eMarkType == SC_MARK_NONE)
            eMarkType = SC_MARK_SIMPLE;
        const ScPatternAttr* pMarkPattern = mrDoc.GetPattern(GetCurX(), GetCurY(), GetTabNo());
        if (pMarkPattern && pMarkPattern->GetItemSet().GetItemState(ATTR_MERGE, false) == SfxItemState::SET)
        {
            SCROW nRow = pMarkPattern->GetItem(ATTR_MERGE).GetRowMerge();
            SCCOL nCol = pMarkPattern->GetItem(ATTR_MERGE).GetColMerge();
            if ( nRow < 1 || nCol < 1 )
            {
                // This kind of cells do exist. Not sure if that is intended or a bug.
                rRange = ScRange(GetCurX(), GetCurY(), GetTabNo());
            }
            else
            {
                rRange = ScRange(GetCurX(), GetCurY(), GetTabNo(),
                                GetCurX() + nCol - 1, GetCurY() + nRow - 1, GetTabNo());
                if ( ScViewUtil::HasFiltered(rRange, GetDocument()) )
                    eMarkType = SC_MARK_SIMPLE_FILTERED;
            }
        }
        else
            rRange = ScRange(GetCurX(), GetCurY(), GetTabNo());
    }
    return eMarkType;
}

ScMarkType ScViewData::GetSimpleArea( SCCOL& rStartCol, SCROW& rStartRow, SCTAB& rStartTab,
                                SCCOL& rEndCol, SCROW& rEndRow, SCTAB& rEndTab ) const
{
    //  parameter bMergeMark is no longer needed: The view's selection is never modified
    //  (a local copy is used), and a multi selection that adds to a single range can always
    //  be treated like a single selection (GetSimpleArea isn't used in selection
    //  handling itself)

    ScRange aRange;
    ScMarkData aNewMark(maMarkData);       // use a local copy for MarkToSimple
    ScMarkType eMarkType = GetSimpleArea( aRange, aNewMark);
    aRange.GetVars( rStartCol, rStartRow, rStartTab, rEndCol, rEndRow, rEndTab);
    return eMarkType;
}

ScMarkType ScViewData::GetSimpleArea( ScRange& rRange ) const
{
    //  parameter bMergeMark is no longer needed, see above

    ScMarkData aNewMark(maMarkData);       // use a local copy for MarkToSimple
    return GetSimpleArea( rRange, aNewMark);
}

void ScViewData::GetMultiArea( ScRangeListRef& rRange ) const
{
    //  parameter bMergeMark is no longer needed, see GetSimpleArea

    ScMarkData aNewMark(maMarkData);       // use a local copy for MarkToSimple

    bool bMulti = aNewMark.IsMultiMarked();
    if (bMulti)
    {
        aNewMark.MarkToSimple();
        bMulti = aNewMark.IsMultiMarked();
    }
    if (bMulti)
    {
        rRange = new ScRangeList;
        aNewMark.FillRangeListWithMarks( rRange.get(), false );
    }
    else
    {
        ScRange aSimple;
        GetSimpleArea(aSimple);
        rRange = new ScRangeList(aSimple);
    }
}

bool ScViewData::SimpleColMarked()
{
    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;
    if (GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
        if (nStartRow == 0 && nEndRow == mrDoc.MaxRow())
            return true;

    return false;
}

bool ScViewData::SimpleRowMarked()
{
    SCCOL nStartCol;
    SCROW nStartRow;
    SCTAB nStartTab;
    SCCOL nEndCol;
    SCROW nEndRow;
    SCTAB nEndTab;
    if (GetSimpleArea(nStartCol,nStartRow,nStartTab,nEndCol,nEndRow,nEndTab) == SC_MARK_SIMPLE)
        if (nStartCol == 0 && nEndCol == mrDoc.MaxCol())
            return true;

    return false;
}

bool ScViewData::IsMultiMarked() const
{
    // Test for "real" multi selection, calling MarkToSimple on a local copy,
    // and taking filtered in simple area marks into account.

    ScRange aDummy;
    ScMarkType eType = GetSimpleArea(aDummy);
    return (eType & SC_MARK_SIMPLE) != SC_MARK_SIMPLE;
}

bool ScViewData::SelectionForbidsPaste( ScDocument* pClipDoc )
{
    if (!pClipDoc)
    {
        // Same as checkDestRanges() in sc/source/ui/view/cellsh.cxx but
        // different return details.

        vcl::Window* pWin = GetActiveWin();
        if (!pWin)
            // No window doesn't mean paste would be forbidden.
            return false;

        const ScTransferObj* pOwnClip = ScTransferObj::GetOwnClipboard(ScTabViewShell::GetClipData(pWin));
        if (!pOwnClip)
            // Foreign content does not get repeatedly replicated.
            return false;

        pClipDoc = pOwnClip->GetDocument();
        if (!pClipDoc)
            // No clipdoc doesn't mean paste would be forbidden.
            return false;
    }

    const ScRange aSrcRange = pClipDoc->GetClipParam().getWholeRange();
    const SCROW nRowSize = aSrcRange.aEnd.Row() - aSrcRange.aStart.Row() + 1;
    const SCCOL nColSize = aSrcRange.aEnd.Col() - aSrcRange.aStart.Col() + 1;

    return SelectionForbidsPaste( nColSize, nRowSize);
}

bool ScViewData::SelectionForbidsPaste( SCCOL nSrcCols, SCROW nSrcRows )
{
    ScRange aSelRange( ScAddress::UNINITIALIZED );
    ScMarkType eMarkType = GetSimpleArea( aSelRange);

    if (eMarkType == SC_MARK_MULTI)
        // Not because of DOOM.
        return false;

    if (aSelRange.aEnd.Row() - aSelRange.aStart.Row() + 1 == nSrcRows)
        // This also covers entire col(s) copied to be pasted on entire cols.
        return false;

    if (aSelRange.aEnd.Col() - aSelRange.aStart.Col() + 1 == nSrcCols)
        // This also covers entire row(s) copied to be pasted on entire rows.
        return false;

    return SelectionFillDOOM( aSelRange);
}

bool ScViewData::SelectionForbidsCellFill()
{
    ScRange aSelRange( ScAddress::UNINITIALIZED );
    ScMarkType eMarkType = GetSimpleArea( aSelRange);
    return eMarkType != SC_MARK_MULTI && SelectionFillDOOM( aSelRange);
}

// static
bool ScViewData::SelectionFillDOOM( const ScRange& rRange )
{
    // Assume that more than 23 full columns (23M cells) will not be
    // successful... Even with only 10 bytes per cell that would already be
    // 230MB, formula cells would be 100 bytes and more per cell.
    // rows * columns > 23m => rows > 23m / columns
    // to not overflow in case number of available columns or rows would be
    // arbitrarily increased.
    // We could refine this and take some actual cell size into account,
    // evaluate available memory and what not, but...
    const sal_Int32 kMax = 23 * 1024 * 1024;    // current MAXROWCOUNT1 is 1024*1024=1048576
    return (rRange.aEnd.Row() - rRange.aStart.Row() + 1) > (kMax / (rRange.aEnd.Col() - rRange.aStart.Col() + 1));
}

void ScViewData::SetFillMode( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    nFillMode   = ScFillMode::FILL;
    nFillStartX = nStartCol;
    nFillStartY = nStartRow;
    nFillEndX   = nEndCol;
    nFillEndY   = nEndRow;
}

void ScViewData::SetDragMode( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                ScFillMode nMode )
{
    nFillMode   = nMode;
    nFillStartX = nStartCol;
    nFillStartY = nStartRow;
    nFillEndX   = nEndCol;
    nFillEndY   = nEndRow;
}

void ScViewData::ResetFillMode()
{
    nFillMode   = ScFillMode::NONE;
}

void ScViewData::GetFillData( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow )
{
    rStartCol = nFillStartX;
    rStartRow = nFillStartY;
    rEndCol   = nFillEndX;
    rEndRow   = nFillEndY;
}

SCCOL ScViewData::GetOldCurX() const
{
    if (pThisTab->mbOldCursorValid)
        return pThisTab->nOldCurX;
    else
        return pThisTab->nCurX;
}

SCROW ScViewData::GetOldCurY() const
{
    if (pThisTab->mbOldCursorValid)
        return pThisTab->nOldCurY;
    else
        return pThisTab->nCurY;
}

void ScViewData::SetOldCursor( SCCOL nNewX, SCROW nNewY )
{
    pThisTab->nOldCurX = nNewX;
    pThisTab->nOldCurY = nNewY;
    pThisTab->mbOldCursorValid = true;
}

void ScViewData::ResetOldCursor()
{
    pThisTab->mbOldCursorValid = false;
}

SCCOL ScViewData::GetPosX( ScHSplitPos eWhich, SCTAB nForTab ) const
{
    if (comphelper::LibreOfficeKit::isActive())
        return 0;

    if (nForTab == -1)
        return pThisTab->nPosX[eWhich];

    if (!ValidTab(nForTab) || (nForTab >= static_cast<SCTAB>(maTabData.size())))
        return -1;

    return maTabData[nForTab]->nPosX[eWhich];
}

SCROW ScViewData::GetPosY( ScVSplitPos eWhich, SCTAB nForTab ) const
{
    if (comphelper::LibreOfficeKit::isActive())
        return 0;

    if (nForTab == -1)
        return pThisTab->nPosY[eWhich];

    if (!ValidTab(nForTab) || (nForTab >= static_cast<SCTAB>(maTabData.size())))
        return -1;

    return maTabData[nForTab]->nPosY[eWhich];
}

ScViewDataTable* ScViewData::FetchTableData(SCTAB nTabIndex) const
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
        return nullptr;
    ScViewDataTable* pRet = maTabData[nTabIndex].get();
    SAL_WARN_IF(!pRet, "sc.viewdata", "ScViewData::FetchTableData: hidden sheet = " << nTabIndex);
    return pRet;
}

SCCOL ScViewData::GetCurXForTab( SCTAB nTabIndex ) const
{
    ScViewDataTable* pTabData = FetchTableData(nTabIndex);
    return pTabData ? pTabData->nCurX : -1;
}

SCROW ScViewData::GetCurYForTab( SCTAB nTabIndex ) const
{
    ScViewDataTable* pTabData = FetchTableData(nTabIndex);
    return pTabData ? pTabData->nCurY : -1;
}

void ScViewData::SetCurXForTab( SCCOL nNewCurX, SCTAB nTabIndex )
{
    if (ScViewDataTable* pTabData = FetchTableData(nTabIndex))
        pTabData->nCurX = nNewCurX;
}

void ScViewData::SetCurYForTab( SCCOL nNewCurY, SCTAB nTabIndex )
{
    if (ScViewDataTable* pTabData = FetchTableData(nTabIndex))
        pTabData->nCurY = nNewCurY;
}

void ScViewData::SetMaxTiledCol( SCCOL nNewMaxCol )
{
    nNewMaxCol = std::clamp(nNewMaxCol, SCCOL(0), mrDoc.MaxCol());

    const SCTAB nTab = GetTabNo();
    auto GetColWidthPx = [this, nTab](SCCOL nCol) {
        const sal_uInt16 nSize = this->mrDoc.GetColWidth(nCol, nTab);
        const tools::Long nSizePx = ScViewData::ToPixel(nSize, nPPTX);
        return nSizePx;
    };

    tools::Long nTotalPixels = GetLOKWidthHelper().computePosition(nNewMaxCol, GetColWidthPx);

    SAL_INFO("sc.lok.docsize", "ScViewData::SetMaxTiledCol: nNewMaxCol: "
            << nNewMaxCol << ", nTotalPixels: " << nTotalPixels);

    GetLOKWidthHelper().removeByIndex(pThisTab->nMaxTiledCol);
    GetLOKWidthHelper().insert(nNewMaxCol, nTotalPixels);

    pThisTab->nMaxTiledCol = nNewMaxCol;
}

void ScViewData::SetMaxTiledRow( SCROW nNewMaxRow )
{
    if (nNewMaxRow < 0)
        nNewMaxRow = 0;
    if (nNewMaxRow > MAXTILEDROW)
        nNewMaxRow = MAXTILEDROW;

    const SCTAB nTab = GetTabNo();
    auto GetRowHeightPx = [this, nTab](SCROW nRow) {
        const sal_uInt16 nSize = this->mrDoc.GetRowHeight(nRow, nTab);
        const tools::Long nSizePx = ScViewData::ToPixel(nSize, nPPTY);
        return nSizePx;
    };

    tools::Long nTotalPixels = GetLOKHeightHelper().computePosition(nNewMaxRow, GetRowHeightPx);

    SAL_INFO("sc.lok.docsize", "ScViewData::SetMaxTiledRow: nNewMaxRow: "
            << nNewMaxRow << ", nTotalPixels: " << nTotalPixels);

    GetLOKHeightHelper().removeByIndex(pThisTab->nMaxTiledRow);
    GetLOKHeightHelper().insert(nNewMaxRow, nTotalPixels);

    pThisTab->nMaxTiledRow = nNewMaxRow;
}

tools::Rectangle ScViewData::GetEditArea( ScSplitPos eWhich, SCCOL nPosX, SCROW nPosY,
                                          vcl::Window* pWin, const ScPatternAttr* pPattern,
                                          bool bForceToTop, bool bInPrintTwips )
{
    Point aCellTopLeft = bInPrintTwips ?
            GetPrintTwipsPos(nPosX, nPosY) : GetScrPos(nPosX, nPosY, eWhich, true);
    return ScEditUtil(&mrDoc, nPosX, nPosY, nTabNo, aCellTopLeft,
                        pWin->GetOutDev(), nPPTX, nPPTY, GetZoomX(), GetZoomY(), bInPrintTwips ).
                            GetEditArea( pPattern, bForceToTop );
}

namespace {

void notifyCellCursorAt(const ScTabViewShell* pViewShell, SCCOL nCol, SCROW nRow,
                        const tools::Rectangle& rCursor)
{
    std::stringstream ss;
    ss << rCursor.getX() << ", " << rCursor.getY() << ", " << rCursor.GetWidth() << ", "
       << rCursor.GetHeight() << ", " << nCol << ", " << nRow;

    pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_CELL_CURSOR, ss.str().c_str());
    SfxLokHelper::notifyOtherViews(pViewShell, LOK_CALLBACK_CELL_VIEW_CURSOR, "rectangle",
                                   ss.str().c_str());
}

}

void ScViewData::SetEditEngine( ScSplitPos eWhich,
                                ScEditEngineDefaulter* pNewEngine,
                                vcl::Window* pWin, SCCOL nNewX, SCROW nNewY )
{
    bool bLayoutRTL = mrDoc.IsLayoutRTL(nTabNo);
    ScHSplitPos eHWhich = WhichH(eWhich);
    ScVSplitPos eVWhich = WhichV(eWhich);
    bool bLOKActive = comphelper::LibreOfficeKit::isActive();
    bool bLOKPrintTwips = bLOKActive && comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);
    bool bLOKLayoutRTL = bLOKActive && bLayoutRTL;

    bool bWasThere = false;
    if (pEditView[eWhich])
    {
        //  if the view is already there don't call anything that changes the cursor position
        if (bEditActive[eWhich])
        {
            bWasThere = true;
        }
        else
        {
            lcl_LOKRemoveWindow(GetViewShell(), eWhich);
            pEditView[eWhich]->SetEditEngine(pNewEngine);
        }

        if (pEditView[eWhich]->GetWindow() != pWin)
        {
            lcl_LOKRemoveWindow(GetViewShell(), eWhich);
            pEditView[eWhich]->SetWindow(pWin);
            OSL_FAIL("EditView Window has changed");
        }
    }
    else
    {
        pEditView[eWhich].reset(new EditView( pNewEngine, pWin ));

        if (bLOKActive)
        {
            // We can broadcast the view-cursor message in print-twips for all views.
            pEditView[eWhich]->SetBroadcastLOKViewCursor(bLOKPrintTwips);
            pEditView[eWhich]->RegisterViewShell(pView);
        }
    }

    // add windows from other views
    if (!bWasThere && bLOKActive)
    {
        ScTabViewShell* pThisViewShell = GetViewShell();
        SCTAB nThisTabNo = GetTabNo();
        auto lAddWindows =
                [pThisViewShell, nThisTabNo, eWhich] (ScTabViewShell* pOtherViewShell)
                {
                    ScViewData& rOtherViewData = pOtherViewShell->GetViewData();
                    SCTAB nOtherTabNo = rOtherViewData.GetTabNo();
                    if (nThisTabNo == nOtherTabNo)
                        pOtherViewShell->AddWindowToForeignEditView(pThisViewShell, eWhich);
                };

        SfxLokHelper::forEachOtherView(pThisViewShell, lAddWindows);
    }

    // if view is gone then during IdleFormat sometimes a cursor is drawn

    EEControlBits nEC = pNewEngine->GetControlWord();
    pNewEngine->SetControlWord(nEC & ~EEControlBits::DOIDLEFORMAT);

    EVControlBits nVC = pEditView[eWhich]->GetControlWord();
    pEditView[eWhich]->SetControlWord(nVC & ~EVControlBits::AUTOSCROLL);

    bEditActive[eWhich] = true;

    const ScPatternAttr* pPattern = mrDoc.GetPattern(nNewX, nNewY, nTabNo);
    SvxCellHorJustify eJust = pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue();

    bool bBreak = ( eJust == SvxCellHorJustify::Block ) ||
                    pPattern->GetItem(ATTR_LINEBREAK).GetValue();

    bool bAsianVertical = pNewEngine->IsEffectivelyVertical();     // set by InputHandler

    tools::Rectangle aPixRect = ScEditUtil(&mrDoc, nNewX, nNewY, nTabNo, GetScrPos(nNewX, nNewY, eWhich),
                                        pWin->GetOutDev(), nPPTX,nPPTY,GetZoomX(),GetZoomY() ).
                                            GetEditArea( pPattern, true );

    tools::Rectangle aPTwipsRect;
    if (bLOKPrintTwips)
    {
        aPTwipsRect = ScEditUtil(&mrDoc, nNewX, nNewY, nTabNo, GetPrintTwipsPos(nNewX, nNewY),
                                 pWin->GetOutDev(), nPPTX, nPPTY, GetZoomX(), GetZoomY(), true /* bInPrintTwips */).
                                        GetEditArea(pPattern, true);
    }

    //  when right-aligned, leave space for the cursor
    //  in vertical mode, editing is always right-aligned
    if ( GetEditAdjust() == SvxAdjust::Right || bAsianVertical )
    {
        aPixRect.AdjustRight(1 );
        if (bLOKPrintTwips)
            aPTwipsRect.AdjustRight(o3tl::convert(1, o3tl::Length::px, o3tl::Length::twip));
    }

    if (bLOKPrintTwips)
    {
        if (!pEditView[eWhich]->HasLOKSpecialPositioning())
            pEditView[eWhich]->InitLOKSpecialPositioning(MapUnit::MapTwip, aPTwipsRect, Point());
        else
            pEditView[eWhich]->SetLOKSpecialOutputArea(aPTwipsRect);
    }

    if (bLOKActive && pEditView[eWhich]->HasLOKSpecialPositioning())
        pEditView[eWhich]->SetLOKSpecialFlags(bLOKLayoutRTL ? LOKSpecialFlags::LayoutRTL : LOKSpecialFlags::NONE);

    tools::Rectangle aOutputArea = pWin->PixelToLogic( aPixRect, GetLogicMode() );
    pEditView[eWhich]->SetOutputArea( aOutputArea );

    if (bLOKPrintTwips)
        notifyCellCursorAt(GetViewShell(), nNewX, nNewY, aPTwipsRect);

    if ( bActive && eWhich == GetActivePart() )
    {
        // keep the part that has the active edit view available after
        // switching sheets or reference input on a different part
        eEditActivePart = eWhich;

        //  modify members nEditCol etc. only if also extending for needed area
        nEditCol = nNewX;
        nEditRow = nNewY;
        const ScMergeAttr* pMergeAttr = &pPattern->GetItem(ATTR_MERGE);
        nEditEndCol = nEditCol;
        if (pMergeAttr->GetColMerge() > 1)
            nEditEndCol += pMergeAttr->GetColMerge() - 1;
        nEditEndRow = nEditRow;
        if (pMergeAttr->GetRowMerge() > 1)
            nEditEndRow += pMergeAttr->GetRowMerge() - 1;
        nEditStartCol = nEditCol;

        //  For growing use only the alignment value from the attribute, numbers
        //  (existing or started) with default alignment extend to the right.
        bool bGrowCentered = ( eJust == SvxCellHorJustify::Center );
        bool bGrowToLeft = ( eJust == SvxCellHorJustify::Right );      // visual left
        bool bLOKRTLInvert = (bLOKActive && bLayoutRTL);
        if ( bAsianVertical )
            bGrowCentered = bGrowToLeft = false;   // keep old behavior for asian mode

        tools::Long nSizeXPix, nSizeXPTwips = 0;

        const tools::Long nGridWidthPx = pView->GetGridWidth(eHWhich);
        const tools::Long nGridHeightPx = pView->GetGridHeight(eVWhich);
        tools::Long nGridWidthTwips = 0, nGridHeightTwips = 0;
        if (bLOKPrintTwips)
        {
            Size aGridSize(nGridWidthPx, nGridHeightPx);
            const MapMode& rWinMapMode = GetLogicMode();
            aGridSize = OutputDevice::LogicToLogic(
                pWin->PixelToLogic(aGridSize, rWinMapMode),
                rWinMapMode, MapMode(MapUnit::MapTwip));
            nGridWidthTwips = aGridSize.Width();
            nGridHeightTwips = aGridSize.Height();
        }

        if (bBreak && !bAsianVertical)
        {
            nSizeXPix = aPixRect.GetWidth();    // papersize -> no horizontal scrolling
            if (bLOKPrintTwips)
                nSizeXPTwips = aPTwipsRect.GetWidth();
        }
        else
        {
            OSL_ENSURE(pView,"no View for EditView");

            if ( bGrowCentered )
            {
                //  growing into both directions until one edge is reached
                //! should be limited to whole cells in both directions
                tools::Long nLeft = aPixRect.Left();
                tools::Long nRight = nGridWidthPx - aPixRect.Right();
                nSizeXPix = aPixRect.GetWidth() + 2 * std::min( nLeft, nRight );
                if (bLOKPrintTwips)
                {
                    tools::Long nLeftPTwips = aPTwipsRect.Left();
                    tools::Long nRightPTwips = nGridWidthTwips - aPTwipsRect.Right();
                    nSizeXPTwips = aPTwipsRect.GetWidth() + 2 * std::min(nLeftPTwips, nRightPTwips);
                }
            }
            else if ( (bGrowToLeft && !bLOKRTLInvert) || (!bGrowToLeft && bLOKRTLInvert) )
            {
                nSizeXPix = aPixRect.Right();   // space that's available in the window when growing to the left
                if (bLOKPrintTwips)
                    nSizeXPTwips = aPTwipsRect.Right();
            }
            else
            {
                nSizeXPix = nGridWidthPx - aPixRect.Left();
                if (bLOKPrintTwips)
                    nSizeXPTwips = nGridWidthTwips - aPTwipsRect.Left();
            }

            if ( nSizeXPix <= 0 )
            {
                nSizeXPix = aPixRect.GetWidth();    // editing outside to the right of the window -> keep cell width
                if (bLOKPrintTwips)
                    nSizeXPTwips = aPTwipsRect.GetWidth();
            }
        }
        OSL_ENSURE(pView,"no View for EditView");
        tools::Long nSizeYPix = nGridHeightPx - aPixRect.Top();
        tools::Long nSizeYPTwips = bLOKPrintTwips ? (nGridHeightTwips - aPTwipsRect.Top()) : 0;

        if ( nSizeYPix <= 0 )
        {
            nSizeYPix = aPixRect.GetHeight();   // editing outside below the window -> keep cell height
            if (bLOKPrintTwips)
                nSizeYPTwips = aPTwipsRect.GetHeight();
        }

        Size aPaperSize = pView->GetActiveWin()->PixelToLogic( Size( nSizeXPix, nSizeYPix ), GetLogicMode() );
        Size aPaperSizePTwips(nSizeXPTwips, nSizeYPTwips);
        // In the LOK case the following code can make the cell background and visible area larger
        // than needed which makes selecting the adjacent right cell impossible in some cases.
        if ( bBreak && !bAsianVertical && SC_MOD()->GetInputOptions().GetTextWysiwyg() && !bLOKActive )
        {
            //  if text is formatted for printer, use the exact same paper width
            //  (and same line breaks) as for output.

            Fraction aFract(1,1);
            constexpr auto HMM_PER_TWIPS = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100);
            tools::Rectangle aUtilRect = ScEditUtil(&mrDoc, nNewX, nNewY, nTabNo, Point(0, 0), pWin->GetOutDev(),
                                    HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract ).GetEditArea( pPattern, false );
            aPaperSize.setWidth( aUtilRect.GetWidth() );
            if (bLOKPrintTwips)
            {
                aPaperSizePTwips.setWidth(o3tl::convert(aUtilRect.GetWidth(), o3tl::Length::mm100, o3tl::Length::twip));
            }
        }

        pNewEngine->SetPaperSize( aPaperSize );
        if (bLOKPrintTwips)
            pNewEngine->SetLOKSpecialPaperSize(aPaperSizePTwips);

        // sichtbarer Ausschnitt
        Size aPaper = pNewEngine->GetPaperSize();
        tools::Rectangle aVis = pEditView[eWhich]->GetVisArea();
        tools::Rectangle aVisPTwips;
        if (bLOKPrintTwips)
            aVisPTwips = pEditView[eWhich]->GetLOKSpecialVisArea();

        tools::Long nDiff = aVis.Right() - aVis.Left();
        tools::Long nDiffPTwips = bLOKPrintTwips ? (aVisPTwips.Right() - aVisPTwips.Left()) : 0;
        if ( GetEditAdjust() == SvxAdjust::Right )
        {
            aVis.SetRight( aPaper.Width() - 1 );
            if (bLOKPrintTwips)
                aVisPTwips.SetRight( aPaperSizePTwips.Width() - 1 );
            bMoveArea = !bLayoutRTL;
        }
        else if ( GetEditAdjust() == SvxAdjust::Center )
        {
            aVis.SetRight( ( aPaper.Width() - 1 + nDiff ) / 2 );
            if (bLOKPrintTwips)
                aVisPTwips.SetRight( ( aPaperSizePTwips.Width() - 1 + nDiffPTwips ) / 2 );
            bMoveArea = true;   // always
        }
        else
        {
            aVis.SetRight( nDiff );
            if (bLOKPrintTwips)
                aVisPTwips.SetRight(nDiffPTwips);
            bMoveArea = bLayoutRTL;
        }
        aVis.SetLeft( aVis.Right() - nDiff );
        if (bLOKPrintTwips)
            aVisPTwips.SetLeft(aVisPTwips.Right() - nDiffPTwips);
        // #i49561# Important note:
        // The set offset of the visible area of the EditView for centered and
        // right alignment in horizontal layout is consider by instances of
        // class <ScEditObjectViewForwarder> in its methods <LogicToPixel(..)>
        // and <PixelToLogic(..)>. This is needed for the correct visibility
        // of paragraphs in edit mode at the accessibility API.
        pEditView[eWhich]->SetVisArea(aVis);
        if (bLOKPrintTwips)
            pEditView[eWhich]->SetLOKSpecialVisArea(aVisPTwips);
        //  UpdateMode has been disabled in ScInputHandler::StartTable
        //  must be enabled before EditGrowY (GetTextHeight)
        pNewEngine->SetUpdateLayout( true );

        pNewEngine->SetStatusEventHdl( LINK( this, ScViewData, EditEngineHdl ) );

        EditGrowY( true );      // adjust to existing text content
        EditGrowX();

        Point aDocPos = pEditView[eWhich]->GetWindowPosTopLeft(0);
        if (aDocPos.Y() < aOutputArea.Top())
            pEditView[eWhich]->Scroll( 0, aOutputArea.Top() - aDocPos.Y() );
    }

                                                    // here bEditActive needs to be set already
                                                    // (due to Map-Mode during Paint)
    if (!bWasThere)
        pNewEngine->InsertView(pEditView[eWhich].get());

    //      background color of the cell
    Color aBackCol = pPattern->GetItem(ATTR_BACKGROUND).GetColor();

    ScModule* pScMod = SC_MOD();
    if ( aBackCol.IsTransparent() )
    {
        aBackCol = pScMod->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    }
    pEditView[eWhich]->SetBackgroundColor( aBackCol );

    pEditView[eWhich]->Invalidate();            //  needed?
    //  needed, if position changed
}

IMPL_LINK( ScViewData, EditEngineHdl, EditStatus&, rStatus, void )
{
    EditStatusFlags nStatus = rStatus.GetStatusWord();
    if (nStatus & (EditStatusFlags::HSCROLL | EditStatusFlags::TextHeightChanged | EditStatusFlags::TEXTWIDTHCHANGED | EditStatusFlags::CURSOROUT))
    {
        EditGrowY();
        EditGrowX();

        if (nStatus & EditStatusFlags::CURSOROUT)
        {
            ScSplitPos eWhich = GetActivePart();
            if (pEditView[eWhich])
                pEditView[eWhich]->ShowCursor(false);
        }
    }
}

void ScViewData::EditGrowX()
{
    // It is insane to call EditGrowX while the output area is already growing.
    // That could occur because of the call to SetDefaultItem later.
    // We end up with wrong start/end edit columns and the changes
    // to the output area performed by the inner call to this method are
    // useless since they are discarded by the outer call.
    if (bGrowing)
        return;

    comphelper::FlagRestorationGuard aFlagGuard(bGrowing, true);

    bool bLOKActive = comphelper::LibreOfficeKit::isActive();
    bool bLOKPrintTwips = bLOKActive && comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    ScDocument& rLocalDoc = GetDocument();

    ScSplitPos eWhich = GetActivePart();
    ScHSplitPos eHWhich = WhichH(eWhich);
    EditView* pCurView = pEditView[eWhich].get();

    if ( !pCurView || !bEditActive[eWhich])
        return;

    bool bLayoutRTL = rLocalDoc.IsLayoutRTL( nTabNo );

    ScEditEngineDefaulter* pEngine =
        static_cast<ScEditEngineDefaulter*>( pCurView->GetEditEngine() );
    vcl::Window* pWin = pCurView->GetWindow();

    // Get the left- and right-most column positions.
    SCCOL nLeft = GetPosX(eHWhich);
    SCCOL nRight = nLeft + VisibleCellsX(eHWhich);

    Size        aSize = pEngine->GetPaperSize();
    Size aSizePTwips;
    if (bLOKPrintTwips)
        aSizePTwips = pEngine->GetLOKSpecialPaperSize();

    tools::Rectangle   aArea = pCurView->GetOutputArea();
    tools::Rectangle aAreaPTwips;
    if (bLOKPrintTwips)
        aAreaPTwips = pCurView->GetLOKSpecialOutputArea();

    tools::Long        nOldRight = aArea.Right();

    // Margin is already included in the original width.
    tools::Long nTextWidth = pEngine->CalcTextWidth();

    bool bChanged = false;
    bool bAsianVertical = pEngine->IsEffectivelyVertical();

    //  get bGrow... variables the same way as in SetEditEngine
    const ScPatternAttr* pPattern = rLocalDoc.GetPattern( nEditCol, nEditRow, nTabNo );
    SvxCellHorJustify eJust = pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue();
    bool bGrowCentered = ( eJust == SvxCellHorJustify::Center );
    bool bGrowToLeft = ( eJust == SvxCellHorJustify::Right );      // visual left
    bool bGrowBackwards = bGrowToLeft;                          // logical left
    if ( bLayoutRTL )
        bGrowBackwards = !bGrowBackwards;                       // invert on RTL sheet
    if ( bAsianVertical )
        bGrowCentered = bGrowToLeft = bGrowBackwards = false;   // keep old behavior for asian mode

    bool bUnevenGrow = false;
    if ( bGrowCentered )
    {
        while (aArea.GetWidth() + 0 < nTextWidth && ( nEditStartCol > nLeft || nEditEndCol < nRight ) )
        {
            tools::Long nLogicLeft = 0;
            tools::Long nLogicLeftPTwips = 0;
            if ( nEditStartCol > nLeft )
            {
                --nEditStartCol;
                tools::Long nColWidth = rLocalDoc.GetColWidth( nEditStartCol, nTabNo );
                tools::Long nLeftPix = ToPixel( nColWidth, nPPTX );
                nLogicLeft = pWin->PixelToLogic(Size(nLeftPix,0)).Width();
                if (bLOKPrintTwips)
                    nLogicLeftPTwips = nColWidth;
            }
            tools::Long nLogicRight = 0;
            tools::Long nLogicRightPTwips = 0;
            if ( nEditEndCol < nRight )
            {
                ++nEditEndCol;
                tools::Long nColWidth = rLocalDoc.GetColWidth( nEditEndCol, nTabNo );
                tools::Long nRightPix = ToPixel( nColWidth, nPPTX );
                nLogicRight = pWin->PixelToLogic(Size(nRightPix,0)).Width();
                if (bLOKPrintTwips)
                    nLogicRightPTwips = nColWidth;
            }

            aArea.AdjustLeft( -((bLayoutRTL && !bLOKActive) ? nLogicRight : nLogicLeft) );
            aArea.AdjustRight((bLayoutRTL && !bLOKActive) ? nLogicLeft : nLogicRight );
            if (bLOKPrintTwips)
            {
                aAreaPTwips.AdjustLeft(-nLogicLeftPTwips);
                aAreaPTwips.AdjustRight(nLogicRightPTwips);
            }

            if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            {
                tools::Long nCenter = ( aArea.Left() + aArea.Right() ) / 2;
                tools::Long nHalf = aSize.Width() / 2;
                aArea.SetLeft( nCenter - nHalf + 1 );
                aArea.SetRight( nCenter + aSize.Width() - nHalf - 1 );

                if (bLOKPrintTwips)
                {
                    tools::Long nCenterPTwips = ( aAreaPTwips.Left() + aAreaPTwips.Right() ) / 2;
                    tools::Long nHalfPTwips = aSizePTwips.Width() / 2;
                    aAreaPTwips.SetLeft( nCenterPTwips - nHalfPTwips + 1 );
                    aAreaPTwips.SetRight( nCenterPTwips + aSizePTwips.Width() - nHalfPTwips - 1 );
                }
            }

            bChanged = true;
            if ( nLogicLeft != nLogicRight )
                bUnevenGrow = true;
        }
    }
    else if ( bGrowBackwards )
    {
        while (aArea.GetWidth() + 0 < nTextWidth && nEditStartCol > nLeft)
        {
            --nEditStartCol;
            tools::Long nColWidth = rLocalDoc.GetColWidth( nEditStartCol, nTabNo );
            tools::Long nPix = ToPixel( nColWidth, nPPTX );
            tools::Long nLogicWidth = pWin->PixelToLogic(Size(nPix,0)).Width();
            tools::Long& nLogicWidthPTwips = nColWidth;

            if ( !bLayoutRTL || bLOKActive )
            {
                aArea.AdjustLeft( -nLogicWidth );
                if (bLOKPrintTwips)
                    aAreaPTwips.AdjustLeft( -nLogicWidthPTwips );
            }
            else
            {
                aArea.AdjustRight(nLogicWidth );
                if (bLOKPrintTwips)
                    aAreaPTwips.AdjustRight(nLogicWidthPTwips);
            }

            if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            {
                if ( !bLayoutRTL || bLOKActive )
                {
                    aArea.SetLeft( aArea.Right() - aSize.Width() + 1 );
                    if (bLOKPrintTwips)
                        aAreaPTwips.SetLeft( aAreaPTwips.Right() - aSizePTwips.Width() + 1 );
                }
                else
                {
                    aArea.SetRight( aArea.Left() + aSize.Width() - 1 );
                    if (bLOKPrintTwips)
                        aAreaPTwips.SetRight( aAreaPTwips.Left() + aSizePTwips.Width() - 1 );
                }
            }

            bChanged = true;
        }
    }
    else
    {
        while (aArea.GetWidth() + 0 < nTextWidth && nEditEndCol < nRight)
        {
            ++nEditEndCol;
            tools::Long nColWidth = rLocalDoc.GetColWidth( nEditEndCol, nTabNo );
            tools::Long nPix = ToPixel( nColWidth, nPPTX );
            tools::Long nLogicWidth = pWin->PixelToLogic(Size(nPix,0)).Width();
            tools::Long& nLogicWidthPTwips = nColWidth;
            if ( bLayoutRTL && !bLOKActive )
            {
                aArea.AdjustLeft( -nLogicWidth );
            }
            else
            {
                aArea.AdjustRight(nLogicWidth );
                if (bLOKPrintTwips)
                    aAreaPTwips.AdjustRight(nLogicWidthPTwips);
            }

            if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            {
                if ( bLayoutRTL && !bLOKActive )
                {
                    aArea.SetLeft( aArea.Right() - aSize.Width() + 1 );
                }
                else
                {
                    aArea.SetRight( aArea.Left() + aSize.Width() - 1 );
                    if (bLOKPrintTwips)
                        aAreaPTwips.SetRight( aAreaPTwips.Left() + aSizePTwips.Width() - 1 );
                }
            }

            bChanged = true;
        }
    }

    if (!bChanged)
        return;

    if ( bMoveArea || bGrowCentered || bGrowBackwards || bLayoutRTL )
    {
        tools::Rectangle aVis = pCurView->GetVisArea();
        tools::Rectangle aVisPTwips;
        if (bLOKPrintTwips)
            aVisPTwips = pCurView->GetLOKSpecialVisArea();

        if ( bGrowCentered )
        {
            //  switch to center-aligned (undo?) and reset VisArea to center

            pEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );

            tools::Long nCenter = aSize.Width() / 2;
            tools::Long nVisSize = aArea.GetWidth();
            aVis.SetLeft( nCenter - nVisSize / 2 );
            aVis.SetRight( aVis.Left() + nVisSize - 1 );

            if (bLOKPrintTwips)
            {
                tools::Long nCenterPTwips = aSizePTwips.Width() / 2;
                tools::Long nVisSizePTwips = aAreaPTwips.GetWidth();
                aVisPTwips.SetLeft( nCenterPTwips - nVisSizePTwips / 2 );
                aVisPTwips.SetRight( aVisPTwips.Left() + nVisSizePTwips - 1 );
            }
        }
        else if ( bGrowToLeft )
        {
            //  switch to right-aligned (undo?) and reset VisArea to the right

            pEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );

            aVis.SetRight( aSize.Width() - 1 );
            aVis.SetLeft( aSize.Width() - aArea.GetWidth() );     // with the new, increased area

            if (bLOKPrintTwips)
            {
                aVisPTwips.SetRight( aSizePTwips.Width() - 1 );
                aVisPTwips.SetLeft( aSizePTwips.Width() - aAreaPTwips.GetWidth() ); // with the new, increased area
            }
        }
        else
        {
            //  switch to left-aligned (undo?) and reset VisArea to the left

            pEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );

            tools::Long nMove = aVis.Left();
            aVis.SetLeft( 0 );
            aVis.AdjustRight( -nMove );

            if (bLOKPrintTwips)
            {
                tools::Long nMovePTwips = aVisPTwips.Left();
                aVisPTwips.SetLeft( 0 );
                aVisPTwips.AdjustRight( -nMovePTwips );
            }
        }

        pCurView->SetVisArea( aVis );
        if (bLOKPrintTwips)
            pCurView->SetLOKSpecialVisArea( aVisPTwips );

        bMoveArea = false;
    }

    if (bLOKPrintTwips)
        pCurView->SetLOKSpecialOutputArea(aAreaPTwips);

    pCurView->SetOutputArea(aArea);

    if (bLOKPrintTwips)
        notifyCellCursorAt(GetViewShell(), nEditCol, nEditRow, aAreaPTwips);

    //  In vertical mode, the whole text is moved to the next cell (right-aligned),
    //  so everything must be repainted. Otherwise, paint only the new area.
    //  If growing in centered alignment, if the cells left and right have different sizes,
    //  the whole text will move, and may not even obscure all of the original display.
    if ( bUnevenGrow )
    {
        aArea.SetLeft( pWin->PixelToLogic( Point(0,0) ).X() );
        aArea.SetRight( pWin->PixelToLogic( aScrSize ).Width() );
    }
    else if ( !bAsianVertical && !bGrowToLeft && !bGrowCentered )
        aArea.SetLeft( nOldRight );
    pWin->Invalidate(aArea);

    // invalidate other views
    pCurView->InvalidateOtherViewWindows(aArea);
}

void ScViewData::EditGrowY( bool bInitial )
{
    if (bGrowing)
        return;

    comphelper::FlagRestorationGuard aFlagGuard(bGrowing, true);

    bool bLOKActive = comphelper::LibreOfficeKit::isActive();
    bool bLOKPrintTwips = bLOKActive && comphelper::LibreOfficeKit::isCompatFlagSet(
            comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);

    ScSplitPos eWhich = GetActivePart();
    ScVSplitPos eVWhich = WhichV(eWhich);
    EditView* pCurView = pEditView[eWhich].get();

    if ( !pCurView || !bEditActive[eWhich])
        return;

    EVControlBits nControl = pEditView[eWhich]->GetControlWord();
    if ( nControl & EVControlBits::AUTOSCROLL )
    {
        //  if end of screen had already been reached and scrolling enabled,
        //  don't further try to grow the edit area

        pCurView->SetOutputArea( pCurView->GetOutputArea() );   // re-align to pixels
        return;
    }

    EditEngine* pEngine = pCurView->GetEditEngine();
    vcl::Window* pWin = pCurView->GetWindow();

    SCROW nBottom = GetPosY(eVWhich) + VisibleCellsY(eVWhich);

    Size        aSize = pEngine->GetPaperSize();
    Size aSizePTwips;
    tools::Rectangle   aArea = pCurView->GetOutputArea();
    tools::Rectangle aAreaPTwips;

    if (bLOKPrintTwips)
    {
        aSizePTwips = pEngine->GetLOKSpecialPaperSize();
        aAreaPTwips = pCurView->GetLOKSpecialOutputArea();
    }

    tools::Long        nOldBottom = aArea.Bottom();
    tools::Long        nTextHeight = pEngine->GetTextHeight();

    //  When editing a formula in a cell with optimal height, allow a larger portion
    //  to be clipped before extending to following rows, to avoid obscuring cells for
    //  reference input (next row is likely to be useful in formulas).
    tools::Long nAllowedExtra = SC_GROWY_SMALL_EXTRA;
    if (nEditEndRow == nEditRow && !(mrDoc.GetRowFlags(nEditRow, nTabNo) & CRFlags::ManualSize) &&
            pEngine->GetParagraphCount() <= 1 )
    {
        //  If the (only) paragraph starts with a '=', it's a formula.
        //  If this is the initial call and the text is empty, allow the larger value, too,
        //  because this occurs in the normal progress of editing a formula.
        //  Subsequent calls with empty text might involve changed attributes (including
        //  font height), so they are treated like normal text.
        OUString aText = pEngine->GetText(  0 );
        if ( ( aText.isEmpty() && bInitial ) || aText.startsWith("=") )
            nAllowedExtra = SC_GROWY_BIG_EXTRA;
    }

    bool bChanged = false;
    bool bMaxReached = false;
    while (aArea.GetHeight() + nAllowedExtra < nTextHeight && nEditEndRow < nBottom && !bMaxReached)
    {
        ++nEditEndRow;
        ScDocument& rLocalDoc = GetDocument();
        tools::Long nRowHeight = rLocalDoc.GetRowHeight( nEditEndRow, nTabNo );
        tools::Long nPix = ToPixel( nRowHeight, nPPTY );
        aArea.AdjustBottom(pWin->PixelToLogic(Size(0,nPix)).Height() );
        if (bLOKPrintTwips)
            aAreaPTwips.AdjustBottom(nRowHeight);

        if ( aArea.Bottom() > aArea.Top() + aSize.Height() - 1 )
        {
            aArea.SetBottom( aArea.Top() + aSize.Height() - 1 );
            if (bLOKPrintTwips)
                aAreaPTwips.SetBottom( aAreaPTwips.Top() + aSizePTwips.Height() - 1 );
            bMaxReached = true;     // don't occupy more cells beyond paper size
        }

        bChanged = true;
        nAllowedExtra = SC_GROWY_SMALL_EXTRA;   // larger value is only for first row
    }

    if (!bChanged)
        return;

    if (bLOKPrintTwips)
        pCurView->SetLOKSpecialOutputArea(aAreaPTwips);

    pCurView->SetOutputArea(aArea);

    if (bLOKPrintTwips)
        notifyCellCursorAt(GetViewShell(), nEditCol, nEditRow, aAreaPTwips);

    if (nEditEndRow >= nBottom || bMaxReached)
    {
        if (!(nControl & EVControlBits::AUTOSCROLL))
            pCurView->SetControlWord( nControl | EVControlBits::AUTOSCROLL );
    }

    aArea.SetTop( nOldBottom );
    pWin->Invalidate(aArea);

    // invalidate other views
    pCurView->InvalidateOtherViewWindows(aArea);
}

void ScViewData::ResetEditView()
{
    LOKEditViewHistory::Update(/*bRemove: */ true);
    EditEngine* pEngine = nullptr;
    for (sal_uInt16 i=0; i<4; i++)
        if (pEditView[i])
        {
            if (bEditActive[i])
            {
                lcl_LOKRemoveWindow(GetViewShell(), static_cast<ScSplitPos>(i));
                pEngine = pEditView[i]->GetEditEngine();
                pEngine->RemoveView(pEditView[i].get());
                pEditView[i]->SetOutputArea( tools::Rectangle() );
            }
            bEditActive[i] = false;
        }

    if (pEngine)
        pEngine->SetStatusEventHdl( Link<EditStatus&,void>() );
}

void ScViewData::KillEditView()
{
    LOKEditViewHistory::Update(/*bRemove: */ true);
    EditEngine* pEngine = nullptr;
    for (sal_uInt16 i=0; i<4; i++)
        if (pEditView[i])
        {
            if (bEditActive[i])
            {
                pEngine = pEditView[i]->GetEditEngine();
                if (pEngine)
                    pEngine->RemoveView(pEditView[i].get());
            }
            pEditView[i].reset();
        }
}

void ScViewData::GetEditView( ScSplitPos eWhich, EditView*& rViewPtr, SCCOL& rCol, SCROW& rRow )
{
    rViewPtr = pEditView[eWhich].get();
    rCol = nEditCol;
    rRow = nEditRow;
}

void ScViewData::CreateTabData( SCTAB nNewTab )
{
    EnsureTabDataSize(nNewTab + 1);

    if (!maTabData[nNewTab])
    {
        maTabData[nNewTab].reset(new ScViewDataTable(&mrDoc));

        maTabData[nNewTab]->eZoomType  = eDefZoomType;
        maTabData[nNewTab]->aZoomX     = aDefZoomX;
        maTabData[nNewTab]->aZoomY     = aDefZoomY;
        maTabData[nNewTab]->aPageZoomX = aDefPageZoomX;
        maTabData[nNewTab]->aPageZoomY = aDefPageZoomY;
    }
}

void ScViewData::CreateSelectedTabData()
{
    for (const auto& rTab : maMarkData)
        CreateTabData(rTab);
}

void ScViewData::EnsureTabDataSize(size_t nSize)
{
    if (nSize > maTabData.size())
        maTabData.resize(nSize);
}

void ScViewData::SetTabNo( SCTAB nNewTab )
{
    if (!ValidTab(nNewTab))
    {
        OSL_FAIL("wrong sheet number");
        return;
    }

    nTabNo = nNewTab;
    CreateTabData(nTabNo);
    pThisTab = maTabData[nTabNo].get();

    CalcPPT();          //  for common column width correction
    RecalcPixPos();     //! not always needed!
}

ScPositionHelper* ScViewData::GetLOKWidthHelper(SCTAB nTabIndex)
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())) ||
        !maTabData[nTabIndex])
    {
        return nullptr;
    }
    return &(maTabData[nTabIndex]->aWidthHelper);
}

ScPositionHelper* ScViewData::GetLOKHeightHelper(SCTAB nTabIndex)
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())) ||
        !maTabData[nTabIndex])
    {
        return nullptr;
    }
    return &(maTabData[nTabIndex]->aHeightHelper);
}

void ScViewData::SetActivePart( ScSplitPos eNewActive )
{
    pThisTab->eWhichActive = eNewActive;

    // Let's hope we find the culprit for tdf#117093
    // Don't sanitize the real value (yet?) because this function might be
    // called before setting the then corresponding split modes. For which in
    // fact then the order should be changed.
    assert(eNewActive == pThisTab->SanitizeWhichActive());
}

Point ScViewData::GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScHSplitPos eWhich ) const
{
    OSL_ENSURE( eWhich==SC_SPLIT_LEFT || eWhich==SC_SPLIT_RIGHT, "wrong position" );
    ScSplitPos ePos = ( eWhich == SC_SPLIT_LEFT ) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
    return GetScrPos( nWhereX, nWhereY, ePos );
}

Point ScViewData::GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScVSplitPos eWhich ) const
{
    OSL_ENSURE( eWhich==SC_SPLIT_TOP || eWhich==SC_SPLIT_BOTTOM, "wrong position" );
    ScSplitPos ePos = ( eWhich == SC_SPLIT_TOP ) ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
    return GetScrPos( nWhereX, nWhereY, ePos );
}

Point ScViewData::GetScrPos( SCCOL nWhereX, SCROW nWhereY, ScSplitPos eWhich,
                                bool bAllowNeg, SCTAB nForTab ) const
{
    ScHSplitPos eWhichX = SC_SPLIT_LEFT;
    ScVSplitPos eWhichY = SC_SPLIT_BOTTOM;
    switch( eWhich )
    {
        case SC_SPLIT_TOPLEFT:
            eWhichX = SC_SPLIT_LEFT;
            eWhichY = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_TOPRIGHT:
            eWhichX = SC_SPLIT_RIGHT;
            eWhichY = SC_SPLIT_TOP;
            break;
        case SC_SPLIT_BOTTOMLEFT:
            eWhichX = SC_SPLIT_LEFT;
            eWhichY = SC_SPLIT_BOTTOM;
            break;
        case SC_SPLIT_BOTTOMRIGHT:
            eWhichX = SC_SPLIT_RIGHT;
            eWhichY = SC_SPLIT_BOTTOM;
            break;
    }

    if (nForTab == -1)
        nForTab = nTabNo;
    bool bForCurTab = (nForTab == nTabNo);
    if (!bForCurTab && (!ValidTab(nForTab) || (nForTab >= static_cast<SCTAB>(maTabData.size()))))
    {
        SAL_WARN("sc.viewdata", "ScViewData::GetScrPos :  invalid nForTab = " << nForTab);
        nForTab = nTabNo;
        bForCurTab = true;
    }

    ScViewDataTable* pViewTable = bForCurTab ? pThisTab : maTabData[nForTab].get();

    if (pView)
    {
        const_cast<ScViewData*>(this)->aScrSize.setWidth( pView->GetGridWidth(eWhichX) );
        const_cast<ScViewData*>(this)->aScrSize.setHeight( pView->GetGridHeight(eWhichY) );
    }

    sal_uInt16 nTSize;
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();

    SCCOL nPosX = GetPosX(eWhichX, nForTab);
    tools::Long nScrPosX = 0;

    if (bAllowNeg || nWhereX >= nPosX)
    {
        SCROW nStartPosX = nPosX;
        if (bIsTiledRendering)
        {
            OSL_ENSURE(nPosX == 0, "Unsupported case.");
            const auto& rNearest = pViewTable->aWidthHelper.getNearestByIndex(nWhereX - 1);
            nStartPosX = rNearest.first + 1;
            nScrPosX = rNearest.second;
        }

        if (nWhereX >= nStartPosX)
        {
            for (SCCOL nX = nStartPosX; nX < nWhereX && (bAllowNeg || bIsTiledRendering || nScrPosX <= aScrSize.Width()); nX++)
            {
                if (nX > mrDoc.MaxCol())
                    nScrPosX = 0x7FFFFFFF;
                else
                {
                    nTSize = mrDoc.GetColWidth(nX, nForTab);
                    if (nTSize)
                    {
                        tools::Long nSizeXPix = ToPixel( nTSize, nPPTX );
                        nScrPosX += nSizeXPix;
                    }
                    else
                    {   // If the width is zero, the column is possibly hidden, skips groups of such columns.
                        SCCOL lastHidden = -1;
                        if(mrDoc.ColHidden(nX, nForTab, nullptr, &lastHidden) && lastHidden > nX)
                            nX = lastHidden - 1;
                    }
                }
            }
        }
        else
        {
            for (SCCOL nX = nStartPosX; nX > nWhereX;)
            {
                --nX;
                nTSize = mrDoc.GetColWidth(nX, nForTab);
                if (nTSize)
                {
                    tools::Long nSizeXPix = ToPixel( nTSize, nPPTX );
                    nScrPosX -= nSizeXPix;
                }
                else
                {   // If the width is zero, the column is possibly hidden, skips groups of such columns.
                    SCCOL firstHidden = -1;
                    if(mrDoc.ColHidden(nX, nForTab, &firstHidden, nullptr) && firstHidden >= 0)
                        nX = firstHidden;
                }
            }
        }

    }


    SCROW nPosY = GetPosY(eWhichY, nForTab);
    tools::Long nScrPosY = 0;

    if (bAllowNeg || nWhereY >= nPosY)
    {
        SCROW nStartPosY = nPosY;
        if (bIsTiledRendering)
        {
            OSL_ENSURE(nPosY == 0, "Unsupported case.");
            const auto& rNearest = pViewTable->aHeightHelper.getNearestByIndex(nWhereY - 1);
            nStartPosY = rNearest.first + 1;
            nScrPosY = rNearest.second;
        }

        if (nWhereY >= nStartPosY)
        {
            for (SCROW nY = nStartPosY; nY < nWhereY && (bAllowNeg || bIsTiledRendering || nScrPosY <= aScrSize.Height()); nY++)
            {
                if ( nY > mrDoc.MaxRow() )
                    nScrPosY = 0x7FFFFFFF;
                else
                {
                    nTSize = mrDoc.GetRowHeight( nY, nTabNo );
                    if (nTSize)
                    {
                        tools::Long nSizeYPix = ToPixel( nTSize, nPPTY );
                        nScrPosY += nSizeYPix;
                    }
                    else if ( nY < mrDoc.MaxRow() )
                    {
                        // skip multiple hidden rows (forward only for now)
                        SCROW nNext = mrDoc.FirstVisibleRow(nY + 1, mrDoc.MaxRow(), nTabNo);
                        if ( nNext > mrDoc.MaxRow() )
                            nY = mrDoc.MaxRow();
                        else
                            nY = nNext - 1;     // +=nDir advances to next visible row
                    }
                }
            }
        }
        else
        {
            for (SCROW nY = nStartPosY; nY > nWhereY;)
            {
                --nY;
                nTSize = mrDoc.GetRowHeight(nY, nForTab);
                if (nTSize)
                {
                    tools::Long nSizeYPix = ToPixel( nTSize, nPPTY );
                    nScrPosY -= nSizeYPix;
                }
                else
                {   // If the height is zero, the row is possibly hidden, skips groups of such rows.
                    SCROW firstHidden = -1;
                    if(mrDoc.RowHidden(nY, nForTab, &firstHidden, nullptr) && firstHidden >= 0)
                        nY = firstHidden;
                }
            }
        }
    }

    if (mrDoc.IsLayoutRTL(nForTab) && !bIsTiledRendering)
    {
        //  mirror horizontal position
        nScrPosX = aScrSize.Width() - 1 - nScrPosX;
    }

    return Point( nScrPosX, nScrPosY );
}

Point ScViewData::GetPrintTwipsPos(SCCOL nCol, SCROW nRow) const
{
    // hidden ones are given 0 sizes by these by default.
    // TODO: rewrite this to loop over spans (matters for jumbosheets).
    tools::Long nPosX = nCol ? mrDoc.GetColWidth(0, nCol - 1, nTabNo) : 0;
    // This is now fast as it loops over spans.
    tools::Long nPosY = nRow ? mrDoc.GetRowHeight(0, nRow - 1, nTabNo) : 0;
    // TODO: adjust for RTL layout case.

    return Point(nPosX, nPosY);
}

Point ScViewData::GetPrintTwipsPosFromTileTwips(const Point& rTileTwipsPos) const
{
    const tools::Long nPixelX = static_cast<tools::Long>(rTileTwipsPos.X() * nPPTX);
    const tools::Long nPixelY = static_cast<tools::Long>(rTileTwipsPos.Y() * nPPTY);
    SCCOL nCol = 0;
    SCROW nRow = 0;

    // The following call (with bTestMerge = false) will not modify any members.
    const_cast<ScViewData*>(this)->GetPosFromPixel(nPixelX, nPixelY, SC_SPLIT_TOPLEFT, nCol, nRow, false /* bTestMerge */);
    const Point aPixCellPos = GetScrPos(nCol, nRow, SC_SPLIT_TOPLEFT, true /* bAllowNeg */);
    const Point aTileTwipsCellPos(aPixCellPos.X() / nPPTX, aPixCellPos.Y() / nPPTY);
    const Point aPrintTwipsCellPos = GetPrintTwipsPos(nCol, nRow);
    return aPrintTwipsCellPos + (rTileTwipsPos - aTileTwipsCellPos);
}

OString ScViewData::describeCellCursorAt(SCCOL nX, SCROW nY, bool bPixelAligned) const
{
    const bool bPosSizeInPixels = bPixelAligned;
    Point aCellPos = bPosSizeInPixels ? GetScrPos( nX, nY, SC_SPLIT_BOTTOMRIGHT, true ) :
            GetPrintTwipsPos(nX, nY);

    tools::Long nSizeX;
    tools::Long nSizeY;
    if (bPosSizeInPixels)
        GetMergeSizePixel( nX, nY, nSizeX, nSizeY );
    else
        GetMergeSizePrintTwips(nX, nY, nSizeX, nSizeY);

    std::stringstream ss;
    if (bPosSizeInPixels)
    {
        double fPPTX = GetPPTX();
        double fPPTY = GetPPTY();

        // make it a slim cell cursor, but not empty
        if (nSizeX == 0)
            nSizeX = 1;

        if (nSizeY == 0)
            nSizeY = 1;

        tools::Long nPosXTw = rtl::math::round(aCellPos.getX() / fPPTX);
        tools::Long nPosYTw = rtl::math::round(aCellPos.getY() / fPPTY);
        // look at Rectangle( const Point& rLT, const Size& rSize ) for the '- 1'
        tools::Long nSizeXTw = rtl::math::round(nSizeX / fPPTX) - 1;
        tools::Long nSizeYTw = rtl::math::round(nSizeY / fPPTY) - 1;

        ss << nPosXTw << ", " << nPosYTw << ", " << nSizeXTw << ", " << nSizeYTw << ", "
            << nX << ", " << nY;
    }
    else
    {
        // look at Rectangle( const Point& rLT, const Size& rSize ) for the decrement.
        if (nSizeX)
            --nSizeX;
        if (nSizeY)
            --nSizeY;
        ss << aCellPos.getX() << ", " << aCellPos.getY()
            << ", " << nSizeX << ", " << nSizeY << ", "
            << nX << ", " << nY;
    }

    return OString(ss.str());
}

//      Number of cells on a screen
SCCOL ScViewData::CellsAtX( SCCOL nPosX, SCCOL nDir, ScHSplitPos eWhichX, sal_uInt16 nScrSizeX ) const
{
    OSL_ENSURE( nDir==1 || nDir==-1, "wrong CellsAt call" );

    if (pView)
        const_cast<ScViewData*>(this)->aScrSize.setWidth( pView->GetGridWidth(eWhichX) );

    SCCOL  nX;
    sal_uInt16  nScrPosX = 0;
    if (nScrSizeX == SC_SIZE_NONE) nScrSizeX = static_cast<sal_uInt16>(aScrSize.Width());

    if (nDir==1)
        nX = nPosX;             // forwards
    else
        nX = nPosX-1;           // backwards

    bool bOut = false;
    for ( ; nScrPosX<=nScrSizeX && !bOut; nX = sal::static_int_cast<SCCOL>(nX + nDir) )
    {
        SCCOL  nColNo = nX;
        if (nColNo < 0 || nColNo > mrDoc.MaxCol())
            bOut = true;
        else
        {
            sal_uInt16 nTSize = mrDoc.GetColWidth(nColNo, nTabNo);
            if (nTSize)
            {
                tools::Long nSizeXPix = ToPixel( nTSize, nPPTX );
                nScrPosX = sal::static_int_cast<sal_uInt16>( nScrPosX + static_cast<sal_uInt16>(nSizeXPix) );
            }
        }
    }

    if (nDir==1)
        nX = sal::static_int_cast<SCCOL>( nX - nPosX );
    else
        nX = (nPosX-1)-nX;

    if (nX>0) --nX;
    return nX;
}

SCROW ScViewData::CellsAtY( SCROW nPosY, SCROW nDir, ScVSplitPos eWhichY, sal_uInt16 nScrSizeY ) const
{
    OSL_ENSURE( nDir==1 || nDir==-1, "wrong CellsAt call" );

    if (pView)
        const_cast<ScViewData*>(this)->aScrSize.setHeight( pView->GetGridHeight(eWhichY) );

    if (nScrSizeY == SC_SIZE_NONE) nScrSizeY = static_cast<sal_uInt16>(aScrSize.Height());

    SCROW nY;

    if (nDir==1)
    {
        // forward
        nY = nPosY;
        tools::Long nScrPosY = 0;
        AddPixelsWhile(nScrPosY, nScrSizeY, nY, mrDoc.MaxRow(), nPPTY, &mrDoc, nTabNo);
        // Original loop ended on last evaluated +1 or if that was MaxRow even on MaxRow+2.
        nY += (nY == mrDoc.MaxRow() ? 2 : 1);
        nY -= nPosY;
    }
    else
    {
        // backward
        nY = nPosY-1;
        tools::Long nScrPosY = 0;
        AddPixelsWhileBackward(nScrPosY, nScrSizeY, nY, 0, nPPTY, &mrDoc, nTabNo);
        // Original loop ended on last evaluated -1 or if that was 0 even on -2.
        nY -= (nY == 0 ? 2 : 1);
        nY = (nPosY-1)-nY;
    }

    if (nY>0) --nY;
    return nY;
}

SCCOL ScViewData::VisibleCellsX( ScHSplitPos eWhichX ) const
{
    return CellsAtX( GetPosX( eWhichX ), 1, eWhichX );
}

SCROW ScViewData::VisibleCellsY( ScVSplitPos eWhichY ) const
{
    return CellsAtY( GetPosY( eWhichY ), 1, eWhichY );
}

SCCOL ScViewData::PrevCellsX( ScHSplitPos eWhichX ) const
{
    return CellsAtX( GetPosX( eWhichX ), -1, eWhichX );
}

SCROW ScViewData::PrevCellsY( ScVSplitPos eWhichY ) const
{
    return CellsAtY( GetPosY( eWhichY ), -1, eWhichY );
}

bool ScViewData::GetMergeSizePixel( SCCOL nX, SCROW nY, tools::Long& rSizeXPix, tools::Long& rSizeYPix ) const
{
    const ScMergeAttr* pMerge = mrDoc.GetAttr(nX, nY, nTabNo, ATTR_MERGE);
    if ( pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1 )
    {
        tools::Long nOutWidth = 0;
        tools::Long nOutHeight = 0;
        SCCOL nCountX = pMerge->GetColMerge();
        for (SCCOL i=0; i<nCountX; i++)
            nOutWidth += ToPixel(mrDoc.GetColWidth(nX + i, nTabNo), nPPTX);
        SCROW nCountY = pMerge->GetRowMerge();

        for (SCROW nRow = nY; nRow <= nY+nCountY-1; ++nRow)
        {
            SCROW nLastRow = nRow;
            if (mrDoc.RowHidden(nRow, nTabNo, nullptr, &nLastRow))
            {
                nRow = nLastRow;
                continue;
            }

            sal_uInt16 nHeight = mrDoc.GetRowHeight(nRow, nTabNo);
            nOutHeight += ToPixel(nHeight, nPPTY);
        }

        rSizeXPix = nOutWidth;
        rSizeYPix = nOutHeight;
        return true;
    }
    else
    {
        rSizeXPix = ToPixel(mrDoc.GetColWidth(nX, nTabNo), nPPTX);
        rSizeYPix = ToPixel(mrDoc.GetRowHeight(nY, nTabNo), nPPTY);
        return false;
    }
}

bool ScViewData::GetMergeSizePrintTwips(SCCOL nX, SCROW nY, tools::Long& rSizeXTwips, tools::Long& rSizeYTwips) const
{
    const ScMergeAttr* pMerge = mrDoc.GetAttr(nX, nY, nTabNo, ATTR_MERGE);
    SCCOL nCountX = pMerge->GetColMerge();
    if (!nCountX)
        nCountX = 1;
    rSizeXTwips = mrDoc.GetColWidth(nX, nX + nCountX - 1, nTabNo);

    SCROW nCountY = pMerge->GetRowMerge();
    if (!nCountY)
        nCountY = 1;
    rSizeYTwips = mrDoc.GetRowHeight(nY, nY + nCountY - 1, nTabNo);

    return (nCountX > 1 || nCountY > 1);
}

void ScViewData::GetPosFromPixel( tools::Long nClickX, tools::Long nClickY, ScSplitPos eWhich,
                                        SCCOL& rPosX, SCROW& rPosY,
                                        bool bTestMerge, bool bRepair, SCTAB nForTab )
{
    //  special handling of 0 is now in ScViewFunctionSet::SetCursorAtPoint

    if (nForTab == -1)
        nForTab = nTabNo;
    bool bForCurTab = (nForTab == nTabNo);
    if (!bForCurTab && (!ValidTab(nForTab) || (nForTab >= static_cast<SCTAB>(maTabData.size()))))
    {
        SAL_WARN("sc.viewdata", "ScViewData::GetPosFromPixel :  invalid nForTab = " << nForTab);
        nForTab = nTabNo;
        bForCurTab = true;
    }

    ScHSplitPos eHWhich = WhichH(eWhich);
    ScVSplitPos eVWhich = WhichV(eWhich);

    if (mrDoc.IsLayoutRTL(nForTab))
    {
        if (!comphelper::LibreOfficeKit::isActive())
        {
            //  mirror horizontal position
            if (pView)
                aScrSize.setWidth( pView->GetGridWidth(eHWhich) );
            nClickX = aScrSize.Width() - 1 - nClickX;
        }
    }

    SCCOL nStartPosX = GetPosX(eHWhich, nForTab);
    SCROW nStartPosY = GetPosY(eVWhich, nForTab);
    rPosX = nStartPosX;
    rPosY = nStartPosY;
    tools::Long nScrX = 0;
    tools::Long nScrY = 0;

    if (nClickX > 0)
    {
        while (rPosX <= mrDoc.MaxCol() && nClickX >= nScrX)
        {
            nScrX += ToPixel(mrDoc.GetColWidth(rPosX, nForTab), nPPTX);
            ++rPosX;
        }
        --rPosX;
    }
    else
    {
        while ( rPosX>0 && nClickX < nScrX )
        {
            --rPosX;
            nScrX -= ToPixel(mrDoc.GetColWidth(rPosX, nForTab), nPPTX);
        }
    }

    if (nClickY > 0)
        AddPixelsWhile(nScrY, nClickY, rPosY, mrDoc.MaxRow(), nPPTY, &mrDoc, nForTab);
    else
    {
        /* TODO: could need some "SubPixelsWhileBackward" method */
        while ( rPosY>0 && nClickY < nScrY )
        {
            --rPosY;
            nScrY -= ToPixel(mrDoc.GetRowHeight(rPosY, nForTab), nPPTY);
        }
    }

    //  cells too big?
    if (rPosX == nStartPosX && nClickX > 0)
    {
         if (pView)
            aScrSize.setWidth( pView->GetGridWidth(eHWhich) );
         if ( nClickX > aScrSize.Width() )
            ++rPosX;
    }
    if (rPosY == nStartPosY && nClickY > 0)
    {
        if (pView)
            aScrSize.setHeight( pView->GetGridHeight(eVWhich) );
        if ( nClickY > aScrSize.Height() )
            ++rPosY;
    }

    rPosX = std::clamp(rPosX, SCCOL(0), mrDoc.MaxCol());
    rPosY = std::clamp(rPosY, SCROW(0), mrDoc.MaxRow());

    if (!(bTestMerge && bForCurTab))
        return;

    // public method to adapt position
    SCCOL nOrigX = rPosX;
    SCROW nOrigY = rPosY;
    mrDoc.SkipOverlapped(rPosX, rPosY, nTabNo);
    bool bHOver = (nOrigX != rPosX);
    bool bVOver = (nOrigY != rPosY);

    if ( !(bRepair && ( bHOver || bVOver )) )
        return;

    const ScMergeAttr* pMerge = mrDoc.GetAttr(rPosX, rPosY, nTabNo, ATTR_MERGE);
    if ( ( bHOver && pMerge->GetColMerge() <= 1 ) ||
         ( bVOver && pMerge->GetRowMerge() <= 1 ) )
    {
        OSL_FAIL("merge error found");

        mrDoc.RemoveFlagsTab(0, 0, mrDoc.MaxCol(), mrDoc.MaxRow(), nTabNo, ScMF::Hor | ScMF::Ver);
        SCCOL nEndCol = mrDoc.MaxCol();
        SCROW nEndRow = mrDoc.MaxRow();
        mrDoc.ExtendMerge(0, 0, nEndCol, nEndRow, nTabNo, true);
        if (pDocShell)
            pDocShell->PostPaint(ScRange(0, 0, nTabNo, mrDoc.MaxCol(), mrDoc.MaxRow(), nTabNo),
                                 PaintPartFlags::Grid);
    }
}

void ScViewData::GetMouseQuadrant( const Point& rClickPos, ScSplitPos eWhich,
                                        SCCOL nPosX, SCROW nPosY, bool& rLeft, bool& rTop )
{
    bool bLayoutRTL = mrDoc.IsLayoutRTL(nTabNo);
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    Point aCellStart = GetScrPos( nPosX, nPosY, eWhich, true );
    tools::Long nSizeX;
    tools::Long nSizeY;
    GetMergeSizePixel( nPosX, nPosY, nSizeX, nSizeY );
    rLeft = ( rClickPos.X() - aCellStart.X() ) * nLayoutSign <= nSizeX / 2;
    rTop  = rClickPos.Y() - aCellStart.Y() <= nSizeY / 2;
}

void ScViewData::SetPosX( ScHSplitPos eWhich, SCCOL nNewPosX )
{
    // in the tiled rendering case, nPosX [the leftmost visible column] must be 0
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    if (nNewPosX != 0 && !bIsTiledRendering)
    {
        SCCOL nOldPosX = pThisTab->nPosX[eWhich];
        tools::Long nTPosX = pThisTab->nTPosX[eWhich];
        tools::Long nPixPosX = pThisTab->nPixPosX[eWhich];
        SCCOL i;
        if ( nNewPosX > nOldPosX )
            for ( i=nOldPosX; i<nNewPosX; i++ )
            {
                tools::Long nThis = mrDoc.GetColWidth(i, nTabNo);
                nTPosX -= nThis;
                nPixPosX -= ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTX);
            }
        else
            for ( i=nNewPosX; i<nOldPosX; i++ )
            {
                tools::Long nThis = mrDoc.GetColWidth(i, nTabNo);
                nTPosX += nThis;
                nPixPosX += ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTX);
            }

        pThisTab->nPosX[eWhich] = nNewPosX;
        pThisTab->nTPosX[eWhich] = nTPosX;
        pThisTab->nMPosX[eWhich] = o3tl::convert(nTPosX, o3tl::Length::twip, o3tl::Length::mm100);
        pThisTab->nPixPosX[eWhich] = nPixPosX;
    }
    else
    {
        pThisTab->nPixPosX[eWhich] =
        pThisTab->nTPosX[eWhich] =
        pThisTab->nMPosX[eWhich] =
        pThisTab->nPosX[eWhich] = 0;
    }
}

void ScViewData::SetPosY( ScVSplitPos eWhich, SCROW nNewPosY )
{
    // in the tiled rendering case, nPosY [the topmost visible row] must be 0
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    if (nNewPosY != 0 && !bIsTiledRendering)
    {
        SCROW nOldPosY = pThisTab->nPosY[eWhich];
        tools::Long nTPosY = pThisTab->nTPosY[eWhich];
        tools::Long nPixPosY = pThisTab->nPixPosY[eWhich];
        SCROW i, nHeightEndRow;
        if ( nNewPosY > nOldPosY )
            for ( i=nOldPosY; i<nNewPosY; i++ )
            {
                tools::Long nThis = mrDoc.GetRowHeight(i, nTabNo, nullptr, &nHeightEndRow);
                SCROW nRows = std::min( nNewPosY, nHeightEndRow + 1) - i;
                i = nHeightEndRow;
                nTPosY -= nThis * nRows;
                nPixPosY -= ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTY) * nRows;
            }
        else
            for ( i=nNewPosY; i<nOldPosY; i++ )
            {
                tools::Long nThis = mrDoc.GetRowHeight(i, nTabNo, nullptr, &nHeightEndRow);
                SCROW nRows = std::min( nOldPosY, nHeightEndRow + 1) - i;
                i = nHeightEndRow;
                nTPosY += nThis * nRows;
                nPixPosY += ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTY) * nRows;
            }

        pThisTab->nPosY[eWhich] = nNewPosY;
        pThisTab->nTPosY[eWhich] = nTPosY;
        pThisTab->nMPosY[eWhich] = o3tl::convert(nTPosY, o3tl::Length::twip, o3tl::Length::mm100);
        pThisTab->nPixPosY[eWhich] = nPixPosY;
    }
    else
    {
        pThisTab->nPixPosY[eWhich] =
        pThisTab->nTPosY[eWhich] =
        pThisTab->nMPosY[eWhich] =
        pThisTab->nPosY[eWhich] = 0;
    }
}

void ScViewData::RecalcPixPos()             // after zoom changes
{
    for (sal_uInt16 eWhich=0; eWhich<2; eWhich++)
    {
        tools::Long nPixPosX = 0;
        SCCOL nPosX = pThisTab->nPosX[eWhich];
        for (SCCOL i=0; i<nPosX; i++)
            nPixPosX -= ToPixel(mrDoc.GetColWidth(i, nTabNo), nPPTX);
        pThisTab->nPixPosX[eWhich] = nPixPosX;

        tools::Long nPixPosY = 0;
        SCROW nPosY = pThisTab->nPosY[eWhich];
        tools::Long nRowHeight = -1;
        SCROW nLastSameHeightRow = -1;
        for (SCROW j=0; j<nPosY; j++)
        {
            if(nLastSameHeightRow < j)
                nRowHeight = ToPixel(mrDoc.GetRowHeight(j, nTabNo, nullptr, &nLastSameHeightRow), nPPTY);
            nPixPosY -= nRowHeight;
        }
        pThisTab->nPixPosY[eWhich] = nPixPosY;
    }
}

const MapMode& ScViewData::GetLogicMode( ScSplitPos eWhich )
{
    aLogicMode.SetOrigin( Point( pThisTab->nMPosX[WhichH(eWhich)],
                                    pThisTab->nMPosY[WhichV(eWhich)] ) );
    return aLogicMode;
}

const MapMode& ScViewData::GetLogicMode()
{
    aLogicMode.SetOrigin( Point() );
    return aLogicMode;
}

void ScViewData::SetScreen( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    SCCOL nCol;
    SCROW nRow;
    sal_uInt16 nTSize;
    tools::Long nSizePix;
    tools::Long nScrPosX = 0;
    tools::Long nScrPosY = 0;

    SetActivePart( SC_SPLIT_BOTTOMLEFT );
    SetPosX( SC_SPLIT_LEFT, nCol1 );
    SetPosY( SC_SPLIT_BOTTOM, nRow1 );

    for (nCol=nCol1; nCol<=nCol2; nCol++)
    {
        nTSize = mrDoc.GetColWidth(nCol, nTabNo);
        if (nTSize)
        {
            nSizePix = ToPixel( nTSize, nPPTX );
            nScrPosX += static_cast<sal_uInt16>(nSizePix);
        }
    }

    for (nRow=nRow1; nRow<=nRow2; nRow++)
    {
        nTSize = mrDoc.GetRowHeight(nRow, nTabNo);
        if (nTSize)
        {
            nSizePix = ToPixel( nTSize, nPPTY );
            nScrPosY += static_cast<sal_uInt16>(nSizePix);
        }
    }

    aScrSize = Size( nScrPosX, nScrPosY );
}

void ScViewData::SetScreenPos( const Point& rVisAreaStart )
{
    tools::Long nSize;
    tools::Long nTwips;
    tools::Long nAdd;
    bool bEnd;

    nSize = 0;
    nTwips = o3tl::convert(rVisAreaStart.X(), o3tl::Length::mm100, o3tl::Length::twip);
    if (mrDoc.IsLayoutRTL(nTabNo))
        nTwips = -nTwips;
    SCCOL nX1 = 0;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = static_cast<tools::Long>(mrDoc.GetColWidth(nX1, nTabNo));
        if (nSize + nAdd <= nTwips + 1 && nX1 < mrDoc.MaxCol())
        {
            nSize += nAdd;
            ++nX1;
        }
        else
            bEnd = true;
    }

    nSize = 0;
    nTwips = o3tl::convert(rVisAreaStart.Y(), o3tl::Length::mm100, o3tl::Length::twip);
    SCROW nY1 = 0;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = static_cast<tools::Long>(mrDoc.GetRowHeight(nY1, nTabNo));
        if (nSize + nAdd <= nTwips + 1 && nY1 < mrDoc.MaxRow())
        {
            nSize += nAdd;
            ++nY1;
        }
        else
            bEnd = true;
    }

    SetActivePart( SC_SPLIT_BOTTOMLEFT );
    SetPosX( SC_SPLIT_LEFT, nX1 );
    SetPosY( SC_SPLIT_BOTTOM, nY1 );

    SetCurX( nX1 );
    SetCurY( nY1 );
}

void ScViewData::SetScreen( const tools::Rectangle& rVisArea )
{
    SetScreenPos( rVisArea.TopLeft() );

    //  here without GetOutputFactor(), since it's for the output into a Metafile

    aScrSize = rVisArea.GetSize();
    aScrSize.setWidth(std::round(o3tl::convert( aScrSize.Width(), o3tl::Length::mm100, o3tl::Length::twip) * ScGlobal::nScreenPPTX));
    aScrSize.setHeight(std::round(o3tl::convert( aScrSize.Height(), o3tl::Length::mm100, o3tl::Length::twip) * ScGlobal::nScreenPPTY));
}

ScDocFunc& ScViewData::GetDocFunc() const
{
    return pDocShell->GetDocFunc();
}

SfxBindings& ScViewData::GetBindings()
{
    assert(pView && "GetBindings() without ViewShell");
    return pView->GetViewFrame().GetBindings();
}

SfxDispatcher& ScViewData::GetDispatcher()
{
    assert(pView && "GetDispatcher() without ViewShell");
    return *pView->GetViewFrame().GetDispatcher();
}

ScMarkData& ScViewData::GetMarkData()
{
    return maMarkData;
}

ScMarkData& ScViewData::GetHighlightData()
{
    return maHighlightData;
}

const ScMarkData& ScViewData::GetMarkData() const
{
    return maMarkData;
}

weld::Window* ScViewData::GetDialogParent()
{
    assert(pView && "GetDialogParent() without ViewShell");
    return pView->GetDialogParent();
}

ScGridWindow* ScViewData::GetActiveWin()
{
    assert(pView && "GetActiveWin() without View");
    return pView->GetActiveWin();
}

const ScGridWindow* ScViewData::GetActiveWin() const
{
    assert(pView && "GetActiveWin() without View");
    return pView->GetActiveWin();
}

ScDrawView* ScViewData::GetScDrawView()
{
    assert(pView && "GetScDrawView() without View");
    return pView->GetScDrawView();
}

bool ScViewData::IsMinimized() const
{
    assert(pView && "IsMinimized() without View");
    return pView->IsMinimized();
}

void ScViewData::UpdateScreenZoom( const Fraction& rNewX, const Fraction& rNewY )
{
    Fraction aOldX = GetZoomX();
    Fraction aOldY = GetZoomY();

    SetZoom( rNewX, rNewY, false );

    Fraction aWidth = GetZoomX();
    aWidth *= Fraction( aScrSize.Width(),1 );
    aWidth /= aOldX;

    Fraction aHeight = GetZoomY();
    aHeight *= Fraction( aScrSize.Height(),1 );
    aHeight /= aOldY;

    aScrSize.setWidth( static_cast<tools::Long>(aWidth) );
    aScrSize.setHeight( static_cast<tools::Long>(aHeight) );
}

void ScViewData::CalcPPT()
{
    double nOldPPTX = nPPTX;
    double nOldPPTY = nPPTY;
    nPPTX = ScGlobal::nScreenPPTX * static_cast<double>(GetZoomX());
    if (pDocShell)
        nPPTX = nPPTX / pDocShell->GetOutputFactor();   // Factor is printer to screen
    nPPTY = ScGlobal::nScreenPPTY * static_cast<double>(GetZoomY());

    //  if detective objects are present,
    //  try to adjust horizontal scale so the most common column width has minimal rounding errors,
    //  to avoid differences between cell and drawing layer output

    if (mrDoc.HasDetectiveObjects(nTabNo))
    {
        SCCOL nEndCol = 0;
        SCROW nDummy = 0;
        mrDoc.GetTableArea(nTabNo, nEndCol, nDummy);
        if (nEndCol<20)
            nEndCol = 20;           // same end position as when determining draw scale

        sal_uInt16 nTwips = mrDoc.GetCommonWidth(nEndCol, nTabNo);
        if ( nTwips )
        {
            double fOriginal = nTwips * nPPTX;
            if ( fOriginal < static_cast<double>(nEndCol) )
            {
                //  if one column is smaller than the column count,
                //  rounding errors are likely to add up to a whole column.

                double fRounded = ::rtl::math::approxFloor( fOriginal + 0.5 );
                if ( fRounded > 0.0 )
                {
                    double fScale = fRounded / fOriginal + 1E-6;
                    if ( fScale >= 0.9 && fScale <= 1.1 )
                        nPPTX *= fScale;
                }
            }
        }
    }

    if (!comphelper::LibreOfficeKit::isActive())
        return;

    SCTAB nTabCount = maTabData.size();
    bool bResetWidths = (nPPTX != nOldPPTX);
    bool bResetHeights = (nPPTY != nOldPPTY);
    for (SCTAB nTabIdx = 0; nTabIdx < nTabCount; ++nTabIdx)
    {
        if (!maTabData[nTabIdx])
            continue;

        if (bResetWidths)
            if (auto* pWHelper = GetLOKWidthHelper(nTabIdx))
                pWHelper->invalidateByPosition(0L);

        if (bResetHeights)
            if (auto* pHHelper = GetLOKHeightHelper(nTabIdx))
                pHHelper->invalidateByPosition(0L);
    }
}

#define SC_OLD_TABSEP   '/'
#define SC_NEW_TABSEP   '+'

void ScViewData::WriteUserData(OUString& rData)
{
    // nZoom (until 364v) or nZoom/nPageZoom/bPageMode (from 364w)
    // nTab
    // Tab control width
    // per sheet:
    // CursorX/CursorY/HSplitMode/VSplitMode/HSplitPos/VSplitPos/SplitActive/
    // PosX[left]/PosX[right]/PosY[top]/PosY[bottom]
    // when rows bigger than 8192, "+" instead of "/"

    sal_uInt16 nZoom = static_cast<sal_uInt16>(tools::Long(pThisTab->aZoomY * 100));
    rData = OUString::number( nZoom ) + "/";
    nZoom = static_cast<sal_uInt16>(tools::Long(pThisTab->aPageZoomY * 100));
    rData += OUString::number( nZoom ) + "/";
    if (bPagebreak)
        rData += "1";
    else
        rData += "0";

    rData += ";" + OUString::number( nTabNo ) + ";" + TAG_TABBARWIDTH +
             OUString::number( pView->GetTabBarWidth() );

    SCTAB nTabCount = mrDoc.GetTableCount();
    for (SCTAB i=0; i<nTabCount; i++)
    {
        rData += ";";                   // Numbering must not get mixed up under any circumstances
        if (i < static_cast<SCTAB>(maTabData.size()) && maTabData[i])
        {
            OUString cTabSep(SC_OLD_TABSEP);                // like 3.1
            if ( maTabData[i]->nCurY > MAXROW_30 ||
                 maTabData[i]->nPosY[0] > MAXROW_30 || maTabData[i]->nPosY[1] > MAXROW_30 ||
                 ( maTabData[i]->eVSplitMode == SC_SPLIT_FIX &&
                    maTabData[i]->nFixPosY > MAXROW_30 ) )
            {
                cTabSep = OUStringChar(SC_NEW_TABSEP); // in order to not kill a 3.1-version
            }

            rData += OUString::number( maTabData[i]->nCurX ) + cTabSep +
                     OUString::number( maTabData[i]->nCurY ) + cTabSep +
                     OUString::number( maTabData[i]->eHSplitMode ) + cTabSep +
                     OUString::number( maTabData[i]->eVSplitMode ) + cTabSep;
            if ( maTabData[i]->eHSplitMode == SC_SPLIT_FIX )
                rData += OUString::number( maTabData[i]->nFixPosX );
            else
                rData += OUString::number( maTabData[i]->nHSplitPos );
            rData += cTabSep;
            if ( maTabData[i]->eVSplitMode == SC_SPLIT_FIX )
                rData += OUString::number( maTabData[i]->nFixPosY );
            else
                rData += OUString::number( maTabData[i]->nVSplitPos );
            rData += cTabSep +
                     OUString::number( maTabData[i]->eWhichActive ) + cTabSep +
                     OUString::number( maTabData[i]->nPosX[0] ) + cTabSep +
                     OUString::number( maTabData[i]->nPosX[1] ) + cTabSep +
                     OUString::number( maTabData[i]->nPosY[0] ) + cTabSep +
                     OUString::number( maTabData[i]->nPosY[1] );
        }
    }
}

void ScViewData::ReadUserData(std::u16string_view rData)
{
    if (rData.empty())    // empty string on "reload"
        return;             // then exit without assertion

    if ( comphelper::string::getTokenCount(rData, ';') <= 2 )
    {
        // when reload, in page preview, the preview UserData may have been left intact.
        // we don't want the zoom from the page preview here.
        OSL_FAIL("ReadUserData: This is not my data");
        return;
    }

    sal_Int32 nMainIdx {0};
    sal_Int32 nIdx {0};

    std::u16string_view aZoomStr = o3tl::getToken(rData, 0, ';', nMainIdx);       // Zoom/PageZoom/Mode
    sal_Unicode cMode = o3tl::getToken(aZoomStr, 2, '/', nIdx)[0];     // 0 or "0"/"1"
    SetPagebreakMode( cMode == '1' );
    // SetPagebreakMode must always be called due to CalcPPT / RecalcPixPos()

    // sheet may have become invalid (for instance last version):
    SCTAB nNewTab = static_cast<SCTAB>(o3tl::toUInt32(o3tl::getToken(rData, 0, ';', nMainIdx)));
    if (mrDoc.HasTable(nNewTab))
        SetTabNo(nNewTab);

    // if available, get tab bar width:
    const sal_Int32 nMainIdxRef {nMainIdx};
    std::u16string_view aTabOpt = o3tl::getToken(rData, 0, ';', nMainIdx);

    std::u16string_view aRest;
    if (o3tl::starts_with(aTabOpt, TAG_TABBARWIDTH, &aRest))
    {
        pView->SetTabBarWidth(o3tl::toInt32(aRest));
    }
    else
    {
        // Tab bar width not specified, token to be processed again
        nMainIdx = nMainIdxRef;
    }

    // per sheet
    SCTAB nPos = 0;
    while ( nMainIdx>0 )
    {
        aTabOpt = o3tl::getToken(rData, 0, ';', nMainIdx);
        EnsureTabDataSize(nPos + 1);
        if (!maTabData[nPos])
            maTabData[nPos].reset(new ScViewDataTable(&mrDoc));

        sal_Unicode cTabSep = 0;
        if (comphelper::string::getTokenCount(aTabOpt, SC_OLD_TABSEP) >= 11)
            cTabSep = SC_OLD_TABSEP;
        else if (comphelper::string::getTokenCount(aTabOpt, SC_NEW_TABSEP) >= 11)
            cTabSep = SC_NEW_TABSEP;
        // '+' is only allowed, if we can deal with rows > 8192

        if (cTabSep)
        {
            nIdx = 0;
            maTabData[nPos]->nCurX = mrDoc.SanitizeCol(static_cast<SCCOL>(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx))));
            maTabData[nPos]->nCurY = mrDoc.SanitizeRow(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx)));
            maTabData[nPos]->eHSplitMode = static_cast<ScSplitMode>(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx)));
            maTabData[nPos]->eVSplitMode = static_cast<ScSplitMode>(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx)));

            sal_Int32 nTmp = o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx));
            if ( maTabData[nPos]->eHSplitMode == SC_SPLIT_FIX )
            {
                maTabData[nPos]->nFixPosX = mrDoc.SanitizeCol(static_cast<SCCOL>(nTmp));
                UpdateFixX(nPos);
            }
            else
                maTabData[nPos]->nHSplitPos = nTmp;

            nTmp = o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx));
            if ( maTabData[nPos]->eVSplitMode == SC_SPLIT_FIX )
            {
                maTabData[nPos]->nFixPosY = mrDoc.SanitizeRow(nTmp);
                UpdateFixY(nPos);
            }
            else
                maTabData[nPos]->nVSplitPos = nTmp;

            maTabData[nPos]->eWhichActive = static_cast<ScSplitPos>(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx)));
            maTabData[nPos]->nPosX[0] = mrDoc.SanitizeCol(static_cast<SCCOL>(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx))));
            maTabData[nPos]->nPosX[1] = mrDoc.SanitizeCol(static_cast<SCCOL>(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx))));
            maTabData[nPos]->nPosY[0] = mrDoc.SanitizeRow(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx)));
            maTabData[nPos]->nPosY[1] = mrDoc.SanitizeRow(o3tl::toInt32(o3tl::getToken(aTabOpt, 0, cTabSep, nIdx)));

            maTabData[nPos]->eWhichActive = maTabData[nPos]->SanitizeWhichActive();
        }
        ++nPos;
    }

    RecalcPixPos();
}

void ScViewData::WriteExtOptions( ScExtDocOptions& rDocOpt ) const
{
    // *** Fill extended document data for export filters ***

    // document settings
    ScExtDocSettings& rDocSett = rDocOpt.GetDocSettings();

    // displayed sheet
    rDocSett.mnDisplTab = GetTabNo();

    // width of the tabbar, relative to frame window width
    rDocSett.mfTabBarWidth = pView->GetPendingRelTabBarWidth();
    if( rDocSett.mfTabBarWidth < 0.0 )
        rDocSett.mfTabBarWidth = ScTabView::GetRelTabBarWidth();

    bool bLOKActive = comphelper::LibreOfficeKit::isActive();

    // sheet settings
    for( SCTAB nTab = 0; nTab < static_cast<SCTAB>(maTabData.size()); ++nTab )
    {
        if( const ScViewDataTable* pViewTab = maTabData[ nTab ].get() )
        {
            ScExtTabSettings& rTabSett = rDocOpt.GetOrCreateTabSettings( nTab );

            // split mode
            ScSplitMode eExHSplit = pViewTab->eHSplitMode;
            ScSplitMode eExVSplit = pViewTab->eVSplitMode;
            SCCOL nExFixPosX = pViewTab->nFixPosX;
            SCROW nExFixPosY = pViewTab->nFixPosY;
            tools::Long nExHSplitPos = pViewTab->nHSplitPos;
            tools::Long nExVSplitPos = pViewTab->nVSplitPos;

            if (bLOKActive)
            {
                OverrideWithLOKFreeze(eExHSplit, eExVSplit,
                                      nExFixPosX, nExFixPosY,
                                      nExHSplitPos, nExVSplitPos, nTab);
            }

            bool bHSplit = eExHSplit != SC_SPLIT_NONE;
            bool bVSplit = eExVSplit != SC_SPLIT_NONE;
            bool bRealSplit = (eExHSplit == SC_SPLIT_NORMAL) || (eExVSplit == SC_SPLIT_NORMAL);
            bool bFrozen    = (eExHSplit == SC_SPLIT_FIX)    || (eExVSplit == SC_SPLIT_FIX);
            OSL_ENSURE( !bRealSplit || !bFrozen, "ScViewData::WriteExtOptions - split and freeze in same sheet" );
            rTabSett.mbFrozenPanes = !bRealSplit && bFrozen;

            // split and freeze position
            rTabSett.maSplitPos = Point( 0, 0 );
            rTabSett.maFreezePos.Set( 0, 0, nTab );
            if( bRealSplit )
            {
                Point& rSplitPos = rTabSett.maSplitPos;
                rSplitPos = Point( bHSplit ? nExHSplitPos : 0, bVSplit ? nExVSplitPos : 0 );
                rSplitPos = Application::GetDefaultDevice()->PixelToLogic( rSplitPos, MapMode( MapUnit::MapTwip ) );
                if( pDocShell )
                    rSplitPos.setX( static_cast<tools::Long>(static_cast<double>(rSplitPos.X()) / pDocShell->GetOutputFactor()) );
            }
            else if( bFrozen )
            {
                if( bHSplit ) rTabSett.maFreezePos.SetCol( nExFixPosX );
                if( bVSplit ) rTabSett.maFreezePos.SetRow( nExFixPosY );
            }

            // first visible cell in top-left and additional panes
            rTabSett.maFirstVis.Set( pViewTab->nPosX[ SC_SPLIT_LEFT ], pViewTab->nPosY[ bVSplit ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM ], nTab );
            rTabSett.maSecondVis.Set( pViewTab->nPosX[ SC_SPLIT_RIGHT ], pViewTab->nPosY[ SC_SPLIT_BOTTOM ], nTab );

            // active pane
            switch( pViewTab->eWhichActive )
            {
                // no horizontal split -> always use left panes
                // no vertical split -> always use top panes
                case SC_SPLIT_TOPLEFT:
                    rTabSett.meActivePane = SCEXT_PANE_TOPLEFT;
                break;
                case SC_SPLIT_TOPRIGHT:
                    rTabSett.meActivePane = bHSplit ? SCEXT_PANE_TOPRIGHT : SCEXT_PANE_TOPLEFT;
                break;
                case SC_SPLIT_BOTTOMLEFT:
                    rTabSett.meActivePane = bVSplit ? SCEXT_PANE_BOTTOMLEFT : SCEXT_PANE_TOPLEFT;
                break;
                case SC_SPLIT_BOTTOMRIGHT:
                    rTabSett.meActivePane = bHSplit ?
                        (bVSplit ? SCEXT_PANE_BOTTOMRIGHT : SCEXT_PANE_TOPRIGHT) :
                        (bVSplit ? SCEXT_PANE_BOTTOMLEFT : SCEXT_PANE_TOPLEFT);
                break;
            }

            // cursor position
            rTabSett.maCursor.Set( pViewTab->nCurX, pViewTab->nCurY, nTab );

            // sheet selection and selected ranges
            const ScMarkData& rMarkData = GetMarkData();
            rTabSett.mbSelected = rMarkData.GetTableSelect( nTab );
            rMarkData.FillRangeListWithMarks( &rTabSett.maSelection, true );
            rTabSett.mbShowGrid = pViewTab->bShowGrid;

            // view mode and zoom
            rTabSett.mbPageMode = bPagebreak;
            rTabSett.mnNormalZoom = static_cast< tools::Long >( pViewTab->aZoomY * Fraction( 100.0 ) );
            rTabSett.mnPageZoom = static_cast< tools::Long >( pViewTab->aPageZoomY * Fraction( 100.0 ) );
        }
    }
}

void ScViewData::ReadExtOptions( const ScExtDocOptions& rDocOpt )
{
    // *** Get extended document data from import filters ***

    if( !rDocOpt.IsChanged() ) return;

    // document settings
    const ScExtDocSettings& rDocSett = rDocOpt.GetDocSettings();

    // displayed sheet
    SetTabNo( rDocSett.mnDisplTab );

    /*  Width of the tabbar, relative to frame window width. We do not have the
        correct width of the frame window here -> store in ScTabView, which sets
        the size in the next resize. */
    pView->SetPendingRelTabBarWidth( rDocSett.mfTabBarWidth );

    // sheet settings
    SCTAB nLastTab = rDocOpt.GetLastTab();
    if (static_cast<SCTAB>(maTabData.size()) <= nLastTab)
        maTabData.resize(nLastTab+1);

    for( SCTAB nTab = 0; nTab < static_cast<SCTAB>(maTabData.size()); ++nTab )
    {
        if( const ScExtTabSettings* pTabSett = rDocOpt.GetTabSettings( nTab ) )
        {
            if( !maTabData[ nTab ] )
                maTabData[nTab].reset(new ScViewDataTable(&mrDoc));

            const ScExtTabSettings& rTabSett = *pTabSett;
            ScViewDataTable& rViewTab = *maTabData[ nTab ];

            // split mode initialization
            bool bFrozen = rTabSett.mbFrozenPanes;
            bool bHSplit = bFrozen ? (rTabSett.maFreezePos.Col() > 0) : (rTabSett.maSplitPos.X() > 0);
            bool bVSplit = bFrozen ? (rTabSett.maFreezePos.Row() > 0) : (rTabSett.maSplitPos.Y() > 0);

            // first visible cell of top-left pane and additional panes
            if (rTabSett.maFirstVis.IsValid())
            {
                rViewTab.nPosX[ SC_SPLIT_LEFT ] = rTabSett.maFirstVis.Col();
                rViewTab.nPosY[ bVSplit ? SC_SPLIT_TOP : SC_SPLIT_BOTTOM ] = rTabSett.maFirstVis.Row();
            }

            if (rTabSett.maSecondVis.IsValid())
            {
                if (bHSplit)
                    rViewTab.nPosX[ SC_SPLIT_RIGHT ] = rTabSett.maSecondVis.Col();
                if (bVSplit)
                    rViewTab.nPosY[ SC_SPLIT_BOTTOM ] = rTabSett.maSecondVis.Row();
            }

            // split mode, split and freeze position
            rViewTab.eHSplitMode = rViewTab.eVSplitMode = SC_SPLIT_NONE;
            rViewTab.nHSplitPos = rViewTab.nVSplitPos = 0;
            rViewTab.nFixPosX = 0;
            rViewTab.nFixPosY = 0;
            if( bFrozen )
            {
                if( bHSplit )
                {
                    rViewTab.eHSplitMode = SC_SPLIT_FIX;
                    rViewTab.nFixPosX = rTabSett.maFreezePos.Col();
                    UpdateFixX( nTab );
                }
                if( bVSplit )
                {
                    rViewTab.eVSplitMode = SC_SPLIT_FIX;
                    rViewTab.nFixPosY = rTabSett.maFreezePos.Row();
                    UpdateFixY( nTab );
                }
            }
            else
            {
                Point aPixel = Application::GetDefaultDevice()->LogicToPixel(
                                rTabSett.maSplitPos, MapMode( MapUnit::MapTwip ) );  //! Zoom?
                // the test for use of printer metrics for text formatting here
                // effectively results in the nFactor = 1.0 regardless of the Option setting.
                if( pDocShell && SC_MOD()->GetInputOptions().GetTextWysiwyg())
                {
                    double nFactor = pDocShell->GetOutputFactor();
                    aPixel.setX( static_cast<tools::Long>( aPixel.X() * nFactor + 0.5 ) );
                }

                bHSplit = bHSplit && aPixel.X() > 0;
                bVSplit = bVSplit && aPixel.Y() > 0;
                if( bHSplit )
                {
                    rViewTab.eHSplitMode = SC_SPLIT_NORMAL;
                    rViewTab.nHSplitPos = aPixel.X();
                }
                if( bVSplit )
                {
                    rViewTab.eVSplitMode = SC_SPLIT_NORMAL;
                    rViewTab.nVSplitPos = aPixel.Y();
                }
            }

            // active pane
            ScSplitPos ePos = SC_SPLIT_BOTTOMLEFT;
            switch( rTabSett.meActivePane )
            {
                // no horizontal split -> always use left panes
                // no vertical split -> always use *bottom* panes
                case SCEXT_PANE_TOPLEFT:
                    ePos = bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT;
                break;
                case SCEXT_PANE_TOPRIGHT:
                    ePos = bHSplit ?
                        (bVSplit ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT) :
                        (bVSplit ? SC_SPLIT_TOPLEFT : SC_SPLIT_BOTTOMLEFT);
                break;
                case SCEXT_PANE_BOTTOMLEFT:
                    ePos = SC_SPLIT_BOTTOMLEFT;
                break;
                case SCEXT_PANE_BOTTOMRIGHT:
                    ePos = bHSplit ? SC_SPLIT_BOTTOMRIGHT : SC_SPLIT_BOTTOMLEFT;
                break;
            }
            rViewTab.eWhichActive = ePos;

            // cursor position
            const ScAddress& rCursor = rTabSett.maCursor;
            if( rCursor.IsValid() )
            {
                rViewTab.nCurX = rCursor.Col();
                rViewTab.nCurY = rCursor.Row();
            }

            // sheet selection and selected ranges
            ScMarkData& rMarkData = GetMarkData();
            rMarkData.SelectTable( nTab, rTabSett.mbSelected );

            // zoom for each sheet
            if( rTabSett.mnNormalZoom )
                rViewTab.aZoomX = rViewTab.aZoomY = Fraction( rTabSett.mnNormalZoom, 100 );
            if( rTabSett.mnPageZoom )
                rViewTab.aPageZoomX = rViewTab.aPageZoomY = Fraction( rTabSett.mnPageZoom, 100 );

            rViewTab.bShowGrid = rTabSett.mbShowGrid;

            // get some settings from displayed Excel sheet, set at Calc document
            if( nTab == GetTabNo() )
            {
                // view mode and default zoom (for new sheets) from current sheet
                if( rTabSett.mnNormalZoom )
                    aDefZoomX = aDefZoomY = Fraction( rTabSett.mnNormalZoom, 100L );
                if( rTabSett.mnPageZoom )
                    aDefPageZoomX = aDefPageZoomY = Fraction( rTabSett.mnPageZoom, 100L );
                /*  #i46820# set pagebreak mode via SetPagebreakMode(), this will
                    update map modes that are needed to draw text correctly. */
                SetPagebreakMode( rTabSett.mbPageMode );
            }
        }
    }

    if (comphelper::LibreOfficeKit::isActive())
        DeriveLOKFreezeAllSheets();

    // RecalcPixPos or so - also nMPos - also for ReadUserData ??!?!
}

void ScViewData::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rSettings) const
{
    rSettings.realloc(SC_VIEWSETTINGS_COUNT);
    // + 1, because we have to put the view id in the sequence
    beans::PropertyValue* pSettings = rSettings.getArray();

    sal_uInt16 nViewID(pView->GetViewFrame().GetCurViewId());
    pSettings[SC_VIEW_ID].Name = SC_VIEWID;
    pSettings[SC_VIEW_ID].Value <<= SC_VIEW + OUString::number(nViewID);

    uno::Reference<container::XNameContainer> xNameContainer =
         document::NamedPropertyValues::create( comphelper::getProcessComponentContext() );
    for (SCTAB nTab=0; nTab<static_cast<SCTAB>(maTabData.size()); nTab++)
    {
        if (maTabData[nTab])
        {
            uno::Sequence <beans::PropertyValue> aTableViewSettings;
            maTabData[nTab]->WriteUserDataSequence(aTableViewSettings, *this, nTab);
            OUString sTabName;
            GetDocument().GetName( nTab, sTabName );
            try
            {
                xNameContainer->insertByName(sTabName, uno::Any(aTableViewSettings));
            }
            //#101739#; two tables with the same name are possible
            catch ( container::ElementExistException& )
            {
                OSL_FAIL("seems there are two tables with the same name");
            }
            catch ( uno::RuntimeException& )
            {
                OSL_FAIL("something went wrong");
            }
        }
    }
    pSettings[SC_TABLE_VIEWSETTINGS].Name = SC_TABLES;
    pSettings[SC_TABLE_VIEWSETTINGS].Value <<= xNameContainer;

    OUString sName;
    GetDocument().GetName( nTabNo, sName );
    pSettings[SC_ACTIVE_TABLE].Name = SC_ACTIVETABLE;
    pSettings[SC_ACTIVE_TABLE].Value <<= sName;
    pSettings[SC_HORIZONTAL_SCROLL_BAR_WIDTH].Name = SC_HORIZONTALSCROLLBARWIDTH;
    pSettings[SC_HORIZONTAL_SCROLL_BAR_WIDTH].Value <<= sal_Int32(pView->GetTabBarWidth());
    sal_Int32 nZoomValue = tools::Long(pThisTab->aZoomY * 100);
    sal_Int32 nPageZoomValue = tools::Long(pThisTab->aPageZoomY * 100);
    pSettings[SC_ZOOM_TYPE].Name = SC_ZOOMTYPE;
    pSettings[SC_ZOOM_TYPE].Value <<= sal_Int16(pThisTab->eZoomType);
    pSettings[SC_ZOOM_VALUE].Name = SC_ZOOMVALUE;
    pSettings[SC_ZOOM_VALUE].Value <<= nZoomValue;
    pSettings[SC_PAGE_VIEW_ZOOM_VALUE].Name = SC_PAGEVIEWZOOMVALUE;
    pSettings[SC_PAGE_VIEW_ZOOM_VALUE].Value <<= nPageZoomValue;
    pSettings[SC_PAGE_BREAK_PREVIEW].Name = SC_SHOWPAGEBREAKPREVIEW;
    pSettings[SC_PAGE_BREAK_PREVIEW].Value <<= bPagebreak;

    pSettings[SC_SHOWZERO].Name = SC_UNO_SHOWZERO;
    pSettings[SC_SHOWZERO].Value <<= maOptions.GetOption(VOPT_NULLVALS);
    pSettings[SC_SHOWNOTES].Name = SC_UNO_SHOWNOTES;
    pSettings[SC_SHOWNOTES].Value <<= maOptions.GetOption(VOPT_NOTES);
    pSettings[SC_SHOWFORMULASMARKS].Name = SC_UNO_SHOWFORMULASMARKS;
    pSettings[SC_SHOWFORMULASMARKS].Value <<= maOptions.GetOption(VOPT_FORMULAS_MARKS);
    pSettings[SC_SHOWGRID].Name = SC_UNO_SHOWGRID;
    pSettings[SC_SHOWGRID].Value <<= maOptions.GetOption(VOPT_GRID);
    pSettings[SC_GRIDCOLOR].Name = SC_UNO_GRIDCOLOR;
    OUString aColorName;
    Color aColor = maOptions.GetGridColor(&aColorName);
    pSettings[SC_GRIDCOLOR].Value <<= aColor;
    pSettings[SC_SHOWPAGEBR].Name = SC_UNO_SHOWPAGEBR;
    pSettings[SC_SHOWPAGEBR].Value <<= maOptions.GetOption(VOPT_PAGEBREAKS);
    pSettings[SC_COLROWHDR].Name = SC_UNO_COLROWHDR;
    pSettings[SC_COLROWHDR].Value <<= maOptions.GetOption(VOPT_HEADER);
    pSettings[SC_SHEETTABS].Name = SC_UNO_SHEETTABS;
    pSettings[SC_SHEETTABS].Value <<= maOptions.GetOption(VOPT_TABCONTROLS);
    pSettings[SC_OUTLSYMB].Name = SC_UNO_OUTLSYMB;
    pSettings[SC_OUTLSYMB].Value <<= maOptions.GetOption(VOPT_OUTLINER);
    pSettings[SC_VALUE_HIGHLIGHTING].Name = SC_UNO_VALUEHIGH;
    pSettings[SC_VALUE_HIGHLIGHTING].Value <<= maOptions.GetOption(VOPT_SYNTAX);
    pSettings[SC_FORMULA_BAR_HEIGHT_VALUE].Name = SC_FORMULABARHEIGHT;
    pSettings[SC_FORMULA_BAR_HEIGHT_VALUE].Value <<= GetFormulaBarLines();;

    const ScGridOptions& aGridOpt = maOptions.GetGridOptions();
    pSettings[SC_SNAPTORASTER].Name = SC_UNO_SNAPTORASTER;
    pSettings[SC_SNAPTORASTER].Value <<= aGridOpt.GetUseGridSnap();
    pSettings[SC_RASTERVIS].Name = SC_UNO_RASTERVIS;
    pSettings[SC_RASTERVIS].Value <<= aGridOpt.GetGridVisible();
    pSettings[SC_RASTERRESX].Name = SC_UNO_RASTERRESX;
    pSettings[SC_RASTERRESX].Value <<= static_cast<sal_Int32>(aGridOpt.GetFieldDrawX());
    pSettings[SC_RASTERRESY].Name = SC_UNO_RASTERRESY;
    pSettings[SC_RASTERRESY].Value <<= static_cast<sal_Int32>(aGridOpt.GetFieldDrawY());
    pSettings[SC_RASTERSUBX].Name = SC_UNO_RASTERSUBX;
    pSettings[SC_RASTERSUBX].Value <<= static_cast<sal_Int32>(aGridOpt.GetFieldDivisionX());
    pSettings[SC_RASTERSUBY].Name = SC_UNO_RASTERSUBY;
    pSettings[SC_RASTERSUBY].Value <<= static_cast<sal_Int32>(aGridOpt.GetFieldDivisionY());
    pSettings[SC_RASTERSYNC].Name = SC_UNO_RASTERSYNC;
    pSettings[SC_RASTERSYNC].Value <<= aGridOpt.GetSynchronize();

    // Common SdrModel processing
    GetDocument().GetDrawLayer()->WriteUserDataSequence(rSettings);
}

void ScViewData::ReadUserDataSequence(const uno::Sequence <beans::PropertyValue>& rSettings)
{
    std::vector<bool> aHasZoomVect( GetDocument().GetTableCount(), false );

    sal_Int32 nTemp32(0);
    sal_Int16 nTemp16(0);
    sal_Int16 nFormulaBarLineCount(0);
    bool bPageMode(false);

    EnsureTabDataSize(GetDocument().GetTableCount());

    for (const auto& rSetting : rSettings)
    {
        // SC_VIEWID has to parse and use by mba
        OUString sName(rSetting.Name);
        if (sName == SC_TABLES)
        {
            uno::Reference<container::XNameContainer> xNameContainer;
            if ((rSetting.Value >>= xNameContainer) && xNameContainer->hasElements())
            {
                const uno::Sequence< OUString > aNames(xNameContainer->getElementNames());
                for (const OUString& sTabName : aNames)
                {
                    SCTAB nTab(0);
                    if (GetDocument().GetTable(sTabName, nTab))
                    {
                        uno::Any aAny = xNameContainer->getByName(sTabName);
                        uno::Sequence<beans::PropertyValue> aTabSettings;
                        if (aAny >>= aTabSettings)
                        {
                            EnsureTabDataSize(nTab + 1);
                            if (!maTabData[nTab])
                                maTabData[nTab].reset(new ScViewDataTable(&mrDoc));

                            bool bHasZoom = false;
                            maTabData[nTab]->ReadUserDataSequence(aTabSettings, *this, nTab, bHasZoom);
                            aHasZoomVect[nTab] = bHasZoom;
                        }
                    }
                }
            }
        }
        else if (sName == SC_ACTIVETABLE)
        {
            OUString sTabName;
            if(rSetting.Value >>= sTabName)
            {
                SCTAB nTab(0);
                if (GetDocument().GetTable(sTabName, nTab))
                    nTabNo = nTab;
            }
        }
        else if (sName == SC_HORIZONTALSCROLLBARWIDTH)
        {
            if (rSetting.Value >>= nTemp32)
                pView->SetTabBarWidth(nTemp32);
        }
        else if (sName == SC_RELHORIZONTALTABBARWIDTH)
        {
            double fWidth = 0.0;
            if (rSetting.Value >>= fWidth)
                pView->SetPendingRelTabBarWidth( fWidth );
        }
        else if (sName == SC_ZOOMTYPE)
        {
            if (rSetting.Value >>= nTemp16)
                eDefZoomType = SvxZoomType(nTemp16);
        }
        else if (sName == SC_ZOOMVALUE)
        {
            if (rSetting.Value >>= nTemp32)
            {
                Fraction aZoom(nTemp32, 100);
                aDefZoomX = aDefZoomY = aZoom;
            }
        }
        else if (sName == SC_PAGEVIEWZOOMVALUE)
        {
            if (rSetting.Value >>= nTemp32)
            {
                Fraction aZoom(nTemp32, 100);
                aDefPageZoomX = aDefPageZoomY = aZoom;
            }
        }
        else if (sName == SC_FORMULABARHEIGHT)
        {
            if (rSetting.Value >>= nFormulaBarLineCount)
            {
                SetFormulaBarLines(nFormulaBarLineCount);
                // Notify formula bar about changed lines
                ScInputHandler* pInputHdl = SC_MOD()->GetInputHdl();
                if (pInputHdl)
                {
                    ScInputWindow* pInputWin = pInputHdl->GetInputWindow();
                    if (pInputWin)
                        pInputWin->NumLinesChanged();
                }
            }
        }
        else if (sName == SC_SHOWPAGEBREAKPREVIEW)
            bPageMode = ScUnoHelpFunctions::GetBoolFromAny( rSetting.Value );
        else if ( sName == SC_UNO_SHOWZERO )
            maOptions.SetOption(VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_SHOWNOTES )
            maOptions.SetOption(VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_SHOWFORMULASMARKS )
            maOptions.SetOption(VOPT_FORMULAS_MARKS, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_SHOWGRID )
            maOptions.SetOption(VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_GRIDCOLOR )
        {
            Color aColor;
            if (rSetting.Value >>= aColor)
                maOptions.SetGridColor(aColor, OUString());
        }
        else if ( sName == SC_UNO_SHOWPAGEBR )
            maOptions.SetOption(VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_COLROWHDR )
            maOptions.SetOption(VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_SHEETTABS )
            maOptions.SetOption(VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_OUTLSYMB )
            maOptions.SetOption(VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else if ( sName == SC_UNO_SHOWOBJ )
        {
            // #i80528# placeholders not supported anymore
            if ( rSetting.Value >>= nTemp16 )
                maOptions.SetObjMode(VOBJ_TYPE_OLE, (nTemp16 == 1) ? VOBJ_MODE_HIDE : VOBJ_MODE_SHOW);
        }
        else if ( sName == SC_UNO_SHOWCHARTS )
        {
            // #i80528# placeholders not supported anymore
            if ( rSetting.Value >>= nTemp16 )
                maOptions.SetObjMode(VOBJ_TYPE_CHART, (nTemp16 == 1) ? VOBJ_MODE_HIDE : VOBJ_MODE_SHOW);
        }
        else if ( sName == SC_UNO_SHOWDRAW )
        {
            // #i80528# placeholders not supported anymore
            if ( rSetting.Value >>= nTemp16 )
                maOptions.SetObjMode(VOBJ_TYPE_DRAW, (nTemp16 == 1) ? VOBJ_MODE_HIDE : VOBJ_MODE_SHOW);
        }
        else if ( sName == SC_UNO_VALUEHIGH && !comphelper::LibreOfficeKit::isActive() )
            maOptions.SetOption(VOPT_SYNTAX, ScUnoHelpFunctions::GetBoolFromAny(rSetting.Value));
        else
        {
            ScGridOptions aGridOpt(maOptions.GetGridOptions());
            if ( sName == SC_UNO_SNAPTORASTER )
                aGridOpt.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( rSetting.Value ) );
            else if ( sName == SC_UNO_RASTERVIS )
                aGridOpt.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( rSetting.Value ) );
            else if ( sName == SC_UNO_RASTERRESX )
                aGridOpt.SetFieldDrawX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSetting.Value ) ) );
            else if ( sName == SC_UNO_RASTERRESY )
                aGridOpt.SetFieldDrawY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSetting.Value ) ) );
            else if ( sName == SC_UNO_RASTERSUBX )
                aGridOpt.SetFieldDivisionX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSetting.Value ) ) );
            else if ( sName == SC_UNO_RASTERSUBY )
                aGridOpt.SetFieldDivisionY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSetting.Value ) ) );
            else if ( sName == SC_UNO_RASTERSYNC )
                aGridOpt.SetSynchronize( ScUnoHelpFunctions::GetBoolFromAny( rSetting.Value ) );
            // Fallback to common SdrModel processing
            else GetDocument().GetDrawLayer()->ReadUserDataSequenceValue(&rSetting);

            maOptions.SetGridOptions(aGridOpt);
        }
    }

    // copy default zoom to sheets where a different one wasn't specified
    for (SCTAB nZoomTab=0; nZoomTab< static_cast<SCTAB>(maTabData.size()); ++nZoomTab)
        if (maTabData[nZoomTab] && ( nZoomTab >= static_cast<SCTAB>(aHasZoomVect.size()) || !aHasZoomVect[nZoomTab] ))
        {
            maTabData[nZoomTab]->eZoomType  = eDefZoomType;
            maTabData[nZoomTab]->aZoomX     = aDefZoomX;
            maTabData[nZoomTab]->aZoomY     = aDefZoomY;
            maTabData[nZoomTab]->aPageZoomX = aDefPageZoomX;
            maTabData[nZoomTab]->aPageZoomY = aDefPageZoomY;
        }

    if (rSettings.hasElements())
        SetPagebreakMode( bPageMode );

    // #i47426# write view options to document, needed e.g. for Excel export
    mrDoc.SetViewOptions(maOptions);

    if (comphelper::LibreOfficeKit::isActive())
        DeriveLOKFreezeAllSheets();
}

void ScViewData::SetOptions( const ScViewOptions& rOpt )
{
    //  if visibility of horizontal ScrollBar is changed, TabBar may have to be resized...
    bool bHScrollChanged = (rOpt.GetOption(VOPT_HSCROLL) != maOptions.GetOption(VOPT_HSCROLL));

    //  if graphics are turned on or off, animation has to be started or stopped
    //  graphics are controlled by VOBJ_TYPE_OLE
    bool bGraphicsChanged = (maOptions.GetObjMode(VOBJ_TYPE_OLE) !=
                                   rOpt.GetObjMode(VOBJ_TYPE_OLE) );

    maOptions = rOpt;
    OSL_ENSURE( pView, "No View" );

    if( pView )
    {
        pView->ViewOptionsHasChanged( bHScrollChanged, bGraphicsChanged );
    }
}

Point ScViewData::GetMousePosPixel()
{
    OSL_ENSURE( pView, "GetMousePosPixel() without View" );
    return pView->GetMousePosPixel();
}

void ScViewData::UpdateInputHandler( bool bForce )
{
    if (pView)
        pView->UpdateInputHandler(bForce);
}

bool ScViewData::IsOle() const
{
    return pDocShell && pDocShell->IsOle();
}

bool ScViewData::UpdateFixX( SCTAB nTab )                   // true = value changed
{
    if (!ValidTab(nTab))        // Default
        nTab=nTabNo;            // current table

    if (!pView || maTabData[nTab]->eHSplitMode != SC_SPLIT_FIX)
        return false;

    ScDocument& rLocalDoc = GetDocument();
    if (!rLocalDoc.HasTable(nTab))          // if called from reload, the sheet may not exist
        return false;

    SCCOL nFix = maTabData[nTab]->nFixPosX;
    tools::Long nNewPos = 0;
    for (SCCOL nX=maTabData[nTab]->nPosX[SC_SPLIT_LEFT]; nX<nFix; nX++)
    {
        sal_uInt16 nTSize = rLocalDoc.GetColWidth( nX, nTab );
        if (nTSize)
        {
            tools::Long nPix = ToPixel( nTSize, nPPTX );
            nNewPos += nPix;
        }
    }
    nNewPos += pView->GetGridOffset().X();
    if (nNewPos != maTabData[nTab]->nHSplitPos)
    {
        maTabData[nTab]->nHSplitPos = nNewPos;
        if (nTab == nTabNo)
            RecalcPixPos();                 // should not be needed
        return true;
    }

    return false;
}

bool ScViewData::UpdateFixY( SCTAB nTab )               // true = value changed
{
    if (!ValidTab(nTab))        // Default
        nTab=nTabNo;        // current table

    if (!pView || maTabData[nTab]->eVSplitMode != SC_SPLIT_FIX)
        return false;

    ScDocument& rLocalDoc = GetDocument();
    if (!rLocalDoc.HasTable(nTab))          // if called from reload, the sheet may not exist
        return false;

    SCROW nFix = maTabData[nTab]->nFixPosY;
    tools::Long nNewPos = 0;
    for (SCROW nY=maTabData[nTab]->nPosY[SC_SPLIT_TOP]; nY<nFix; nY++)
    {
        sal_uInt16 nTSize = rLocalDoc.GetRowHeight( nY, nTab );
        if (nTSize)
        {
            tools::Long nPix = ToPixel( nTSize, nPPTY );
            nNewPos += nPix;
        }
    }
    nNewPos += pView->GetGridOffset().Y();
    if (nNewPos != maTabData[nTab]->nVSplitPos)
    {
        maTabData[nTab]->nVSplitPos = nNewPos;
        if (nTab == nTabNo)
            RecalcPixPos();                 // should not be needed
        return true;
    }

    return false;
}

void ScViewData::UpdateOutlinerFlags( Outliner& rOutl ) const
{
    ScDocument& rLocalDoc = GetDocument();
    const ScTabViewShell* pTabViewShell = GetViewShell();
    const bool bOnlineSpell = pTabViewShell && pTabViewShell->IsAutoSpell();

    EEControlBits nCntrl = rOutl.GetControlWord();
    nCntrl |= EEControlBits::MARKNONURLFIELDS;
    nCntrl &= ~EEControlBits::MARKURLFIELDS;    // URLs not shaded for output
    nCntrl |= EEControlBits::AUTOCORRECT;
    if( bOnlineSpell )
        nCntrl |= EEControlBits::ONLINESPELLING;
    else
        nCntrl &= ~EEControlBits::ONLINESPELLING;
    rOutl.SetControlWord(nCntrl);

    rOutl.SetCalcFieldValueHdl( LINK( SC_MOD(), ScModule, CalcFieldValueHdl ) );

    //  don't call GetSpellChecker if online spelling isn't enabled.
    //  The language for AutoCorrect etc. is taken from the pool defaults
    //  (set in ScDocument::UpdateDrawLanguages)

    if ( bOnlineSpell )
    {
        css::uno::Reference<css::linguistic2::XSpellChecker1> xXSpellChecker1( LinguMgr::GetSpellChecker() );
        rOutl.SetSpeller( xXSpellChecker1 );
    }

    rOutl.SetDefaultHorizontalTextDirection(
        rLocalDoc.GetEditTextDirection( nTabNo ) );
}

ScAddress ScViewData::GetCurPos() const
{
    return ScAddress( GetCurX(), GetCurY(), GetTabNo() );
}

void ScViewData::SetRefStart( SCCOL nNewX, SCROW nNewY, SCTAB nNewZ )
{
    nRefStartX = nNewX; nRefStartY = nNewY; nRefStartZ = nNewZ;
}

void ScViewData::SetRefEnd( SCCOL nNewX, SCROW nNewY, SCTAB nNewZ )
{
    nRefEndX = nNewX; nRefEndY = nNewY; nRefEndZ = nNewZ;
}

void ScViewData::AddPixelsWhile( tools::Long & rScrY, tools::Long nEndPixels, SCROW & rPosY,
        SCROW nEndRow, double nPPTY, const ScDocument * pDoc, SCTAB nTabNo )
{
    SCROW nRow = rPosY;
    while (rScrY <= nEndPixels && nRow <= nEndRow)
    {
        SCROW nHeightEndRow;
        sal_uInt16 nHeight = pDoc->GetRowHeight( nRow, nTabNo, nullptr, &nHeightEndRow);
        if (nHeightEndRow > nEndRow)
            nHeightEndRow = nEndRow;
        if (!nHeight)
        {
            if (ValidTab(nTabNo) && nTabNo <= pDoc->GetMaxTableNumber())
                nRow = nHeightEndRow + 1;
            else
                break;
        }
        else
        {
            SCROW nRows = nHeightEndRow - nRow + 1;
            sal_Int64 nPixel = ToPixel( nHeight, nPPTY);
            sal_Int64 nAdd = nPixel * nRows;
            if (nAdd + rScrY > nEndPixels)
            {
                sal_Int64 nDiff = rScrY + nAdd - nEndPixels;
                nRows -= static_cast<SCROW>(nDiff / nPixel);
                nAdd = nPixel * nRows;
                // We're looking for a value that satisfies loop condition.
                if (nAdd + rScrY <= nEndPixels)
                {
                    ++nRows;
                    nAdd += nPixel;
                }
            }
            rScrY += static_cast<tools::Long>(nAdd);
            nRow += nRows;
        }
    }
    if (nRow > rPosY)
        --nRow;
    rPosY = nRow;
}

void ScViewData::AddPixelsWhileBackward( tools::Long & rScrY, tools::Long nEndPixels,
        SCROW & rPosY, SCROW nStartRow, double nPPTY, const ScDocument * pDoc,
        SCTAB nTabNo )
{
    SCROW nRow = rPosY;
    while (rScrY <= nEndPixels && nRow >= nStartRow)
    {
        SCROW nHeightStartRow;
        sal_uInt16 nHeight = pDoc->GetRowHeight( nRow, nTabNo, &nHeightStartRow, nullptr);
        if (nHeightStartRow < nStartRow)
            nHeightStartRow = nStartRow;
        if (!nHeight)
            nRow = nHeightStartRow - 1;
        else
        {
            SCROW nRows = nRow - nHeightStartRow + 1;
            sal_Int64 nPixel = ToPixel( nHeight, nPPTY);
            sal_Int64 nAdd = nPixel * nRows;
            if (nAdd + rScrY > nEndPixels)
            {
                sal_Int64 nDiff = nAdd + rScrY - nEndPixels;
                nRows -= static_cast<SCROW>(nDiff / nPixel);
                nAdd = nPixel * nRows;
                // We're looking for a value that satisfies loop condition.
                if (nAdd + rScrY <= nEndPixels)
                {
                    ++nRows;
                    nAdd += nPixel;
                }
            }
            rScrY += static_cast<tools::Long>(nAdd);
            nRow -= nRows;
        }
    }
    if (nRow < rPosY)
        ++nRow;
    rPosY = nRow;
}

SCCOLROW ScViewData::GetLOKSheetFreezeIndex(bool bIsCol) const
{
    SCCOLROW nFreezeIndex = bIsCol ? mrDoc.GetLOKFreezeCol(nTabNo) : mrDoc.GetLOKFreezeRow(nTabNo);
    return nFreezeIndex >= 0 ? nFreezeIndex : 0;
}

bool ScViewData::SetLOKSheetFreezeIndex(const SCCOLROW nFreezeIndex, bool bIsCol, SCTAB nForTab)
{
    if (nForTab == -1)
    {
        nForTab = nTabNo;
    }
    else if (!ValidTab(nForTab) || (nForTab >= static_cast<SCTAB>(maTabData.size())))
    {
        SAL_WARN("sc.viewdata", "ScViewData::SetLOKSheetFreezeIndex :  invalid nForTab = " << nForTab);
        return false;
    }

    return bIsCol ? mrDoc.SetLOKFreezeCol(static_cast<SCCOL>(nFreezeIndex), nForTab)
                  : mrDoc.SetLOKFreezeRow(static_cast<SCROW>(nFreezeIndex), nForTab);
}

bool ScViewData::RemoveLOKFreeze()
{
    bool colUnfreezed = SetLOKSheetFreezeIndex(0, true);
    bool rowUnfreezed = SetLOKSheetFreezeIndex(0, false);
    return colUnfreezed || rowUnfreezed;
}

void ScViewData::DeriveLOKFreezeAllSheets()
{
    SCTAB nMaxTab = static_cast<SCTAB>(maTabData.size()) - 1;
    for (SCTAB nTab = 0; nTab <= nMaxTab; ++nTab)
        DeriveLOKFreezeIfNeeded(nTab);
}

void ScViewData::DeriveLOKFreezeIfNeeded(SCTAB nForTab)
{
    if (!ValidTab(nForTab) || (nForTab >= static_cast<SCTAB>(maTabData.size())))
    {
        SAL_WARN("sc.viewdata", "ScViewData::DeriveLOKFreezeIfNeeded :  invalid nForTab = " << nForTab);
        return;
    }

    ScViewDataTable* pViewTable = maTabData[nForTab].get();
    if (!pViewTable)
        return;

    bool bConvertToFreezeX = false;
    bool bConvertToFreezeY = false;
    SCCOL nFreezeCol = mrDoc.GetLOKFreezeCol(nForTab);
    SCROW nFreezeRow = mrDoc.GetLOKFreezeRow(nForTab);

    if (nFreezeCol == -1)
    {
        ScSplitMode eSplitMode = pViewTable->eHSplitMode;
        if (eSplitMode == SC_SPLIT_FIX)
            nFreezeCol = pViewTable->nFixPosX;
        else if (eSplitMode == SC_SPLIT_NORMAL)
            bConvertToFreezeX = true;
        else
            nFreezeCol = 0;
    }

    if (nFreezeRow == -1)
    {
        ScSplitMode eSplitMode = pViewTable->eVSplitMode;
        if (eSplitMode == SC_SPLIT_FIX)
            nFreezeRow = pViewTable->nFixPosY;
        else if (eSplitMode == SC_SPLIT_NORMAL)
            bConvertToFreezeY = true;
        else
            nFreezeRow = 0;
    }

    if (bConvertToFreezeX || bConvertToFreezeY)
    {
        SCCOL nCol;
        SCROW nRow;
        GetPosFromPixel(bConvertToFreezeX ? pViewTable->nHSplitPos : 0,
                        bConvertToFreezeY ? pViewTable->nVSplitPos : 0,
                        SC_SPLIT_BOTTOMLEFT, nCol, nRow,
                        false /* bTestMerge */, false /* bRepair */,
                        nForTab);
        if (bConvertToFreezeX)
            nFreezeCol = nCol;
        if (bConvertToFreezeY)
            nFreezeRow = nRow;
    }

    mrDoc.SetLOKFreezeCol(nFreezeCol, nForTab);
    mrDoc.SetLOKFreezeRow(nFreezeRow, nForTab);
}

void ScViewData::OverrideWithLOKFreeze(ScSplitMode& eExHSplitMode, ScSplitMode& eExVSplitMode,
                                       SCCOL& nExFixPosX, SCROW& nExFixPosY,
                                       tools::Long& nExHSplitPos, tools::Long& nExVSplitPos, SCTAB nForTab) const
{
    SCCOL nFreezeCol = mrDoc.GetLOKFreezeCol(nForTab);
    SCROW nFreezeRow = mrDoc.GetLOKFreezeRow(nForTab);

    bool bConvertToScrPosX = false;
    bool bConvertToScrPosY = false;

    if (nFreezeCol >= 0)
    {
        if (eExHSplitMode == SC_SPLIT_NONE)
            eExHSplitMode = SC_SPLIT_FIX;

        if (eExHSplitMode == SC_SPLIT_FIX)
        {
            nExFixPosX = nFreezeCol;
            pThisTab->nPosX[SC_SPLIT_RIGHT] = nFreezeCol;
        }
        else
            bConvertToScrPosX = true;
    }

    if (nFreezeRow >= 0)
    {
        if (eExVSplitMode == SC_SPLIT_NONE)
            eExVSplitMode = SC_SPLIT_FIX;

        if (eExVSplitMode == SC_SPLIT_FIX)
        {
            nExFixPosY = nFreezeRow;
            pThisTab->nPosY[SC_SPLIT_BOTTOM] = nFreezeRow;
        }
        else
            bConvertToScrPosY = true;
    }

    if (bConvertToScrPosX || bConvertToScrPosY)
    {
        Point aExSplitPos = GetScrPos(nFreezeCol, nFreezeRow, SC_SPLIT_BOTTOMLEFT, true, nForTab);
        if (bConvertToScrPosX)
            nExHSplitPos = aExSplitPos.X();
        if (bConvertToScrPosY)
            nExVSplitPos = aExSplitPos.Y();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
