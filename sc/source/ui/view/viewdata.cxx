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

#include <sfx2/lokhelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editview.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/justifyitem.hxx>

#include <vcl/svapp.hxx>
#include <rtl/math.hxx>

#include <sax/tools/converter.hxx>

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
#include <sc.hrc>
#include <patattr.hxx>
#include <editutil.hxx>
#include <scextopt.hxx>
#include <miscuno.hxx>
#include <unonames.hxx>
#include <inputopt.hxx>
#include <viewutil.hxx>
#include <markdata.hxx>
#include <stlalgorithm.hxx>
#include <ViewSettingsSequenceDefines.hxx>
#include <gridwin.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>

using namespace com::sun::star;

#define SC_GROWY_SMALL_EXTRA    100
#define SC_GROWY_BIG_EXTRA      200

#define TAG_TABBARWIDTH "tw:"

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

ScPositionHelper::ScPositionHelper(bool bColumn)
    : MAX_INDEX(bColumn ? MAXCOL : MAXTILEDROW)
{
    mData.insert(std::make_pair(-1, 0));
}

void ScPositionHelper::insert(index_type nIndex, long nPos)
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

void ScPositionHelper::invalidateByPosition(long nPos)
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
    if (posUB == mData.end())
    {
        return *posLB;
    }

    long nDiffUB = posUB->first - nIndex;
    long nDiffLB = posLB->first - nIndex;
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
ScPositionHelper::getNearestByPosition(long nPos) const
{
    SAL_INFO("sc.lok.poshelper",
            "ScPositionHelper::getNearest: nPos: " << nPos << ", size: " << mData.size());
    auto posUB = mData.upper_bound(std::make_pair(null, nPos));

    if (posUB == mData.begin())
    {
        return *posUB;
    }

    auto posLB = std::prev(posUB);
    if (posUB == mData.end())
    {
        return *posLB;
    }

    long nDiffUB = posUB->second - nPos;
    long nDiffLB = posLB->second - nPos;

    if (nDiffUB < -nDiffLB)
    {
        return *posUB;
    }
    else
    {
        return *posLB;
    }
}

long ScPositionHelper::getPosition(index_type nIndex) const
{
    auto it = mData.find(std::make_pair(nIndex, 0));
    if (it == mData.end()) return -1;
    return it->second;
}

long ScPositionHelper::computePosition(index_type nIndex, const std::function<long (index_type)>& getSizePx)
{
    if (nIndex < 0) nIndex = 0;
    if (nIndex > MAX_INDEX) nIndex = MAX_INDEX;

    const auto& rNearest = getNearestByIndex(nIndex);
    index_type nStartIndex = rNearest.first;
    long nTotalPixels = rNearest.second;

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

ScBoundsProvider::ScBoundsProvider(ScDocument* pD, SCTAB nT, bool bColHeader)
    : pDoc(pD)
    , nTab(nT)
    , bColumnHeader(bColHeader)
    , MAX_INDEX(bColHeader ? MAXCOL : MAXTILEDROW)
    , nFirstIndex(-1)
    , nSecondIndex(-1)
    , nFirstPositionPx(-1)
    , nSecondPositionPx(-1)
{}

void ScBoundsProvider::GetStartIndexAndPosition(SCCOL& nIndex, long& nPosition) const
{
    assert(bColumnHeader);
    nIndex = nFirstIndex;
    nPosition = nFirstPositionPx;
}

void ScBoundsProvider::GetEndIndexAndPosition(SCCOL& nIndex, long& nPosition) const
{
    assert(bColumnHeader);
    nIndex = nSecondIndex;
    nPosition = nSecondPositionPx;
}

void ScBoundsProvider::GetStartIndexAndPosition(SCROW& nIndex, long& nPosition) const
{
    assert(!bColumnHeader);
    nIndex = nFirstIndex;
    nPosition = nFirstPositionPx;
}

void ScBoundsProvider::GetEndIndexAndPosition(SCROW& nIndex, long& nPosition) const
{
    assert(!bColumnHeader);
    nIndex = nSecondIndex;
    nPosition = nSecondPositionPx;
}

long ScBoundsProvider::GetSize(index_type nIndex) const
{
    const sal_uInt16 nSize = bColumnHeader ? pDoc->GetColWidth(nIndex, nTab) : pDoc->GetRowHeight(nIndex, nTab);
    return ScViewData::ToPixel(nSize, 1.0 / TWIPS_PER_PIXEL);
}

void ScBoundsProvider::GetIndexAndPos(index_type nNearestIndex, long nNearestPosition,
                    long nBound, index_type& nFoundIndex, long& nPosition,
                    bool bTowards, long nDiff)
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
            long nFirstBound, long nSecondBound)
{
    SAL_INFO("sc.lok.header", "BoundsProvider: nFirstBound: " << nFirstBound
            << ", nSecondBound: " << nSecondBound);

    long nFirstDiff = aFirstNearest.second - nFirstBound;
    long nSecondDiff = aSecondNearest.second - nSecondBound;
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
    long nNearestPosition = aFirstNearest.second;
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

void ScBoundsProvider::EnlargeStartBy(long nOffset)
{
    const index_type nNewFirstIndex =
            std::max(static_cast<index_type>(-1),
                     static_cast<index_type>(nFirstIndex - nOffset));
    for (index_type nIndex = nFirstIndex; nIndex > nNewFirstIndex; --nIndex)
    {
        const long nSizePx = GetSize(nIndex);
        nFirstPositionPx -= nSizePx;
    }
    nFirstIndex = nNewFirstIndex;
    SAL_INFO("sc.lok.header", "BoundsProvider: added offset: nFirstIndex: " << nFirstIndex
            << ", nFirstPositionPx: " << nFirstPositionPx);
}

void ScBoundsProvider::EnlargeEndBy(long nOffset)
{
    const index_type nNewSecondIndex = std::min(MAX_INDEX, static_cast<index_type>(nSecondIndex + nOffset));
    for (index_type nIndex = nSecondIndex + 1; nIndex <= nNewSecondIndex; ++nIndex)
    {
        const long nSizePx = GetSize(nIndex);
        nSecondPositionPx += nSizePx;
    }
    nSecondIndex = nNewSecondIndex;
    SAL_INFO("sc.lok.header", "BoundsProvider: added offset: nSecondIndex: " << nSecondIndex
            << ", nSecondPositionPx: " << nSecondPositionPx);
}

void ScBoundsProvider::GeIndexBackwards(
            index_type nNearestIndex, long nNearestPosition,
            long nBound, index_type& nFoundIndex, long& nPosition, bool bTowards)
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

        const long nSizePx = GetSize(nIndex);
        nNearestPosition -= nSizePx;
    }
    if (!bTowards && nFoundIndex != -1)
    {
        nFoundIndex += 1;
        nPosition += GetSize(nFoundIndex);
    }
}

void ScBoundsProvider::GetIndexTowards(
            index_type nNearestIndex, long nNearestPosition,
            long nBound, index_type& nFoundIndex, long& nPosition, bool bTowards)
{
    nFoundIndex = -2;
    for (index_type nIndex = nNearestIndex + 1; nIndex <= MAX_INDEX; ++nIndex)
    {
        const long nSizePx = GetSize(nIndex);
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

ScViewDataTable::ScViewDataTable() :
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
                nLOKOldCurX( 0 ),
                nLOKOldCurY( 0 ),
                aWidthHelper(true),
                aHeightHelper(false),
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

void ScViewDataTable::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rSettings, const ScViewData& rViewData) const
{
    rSettings.realloc(SC_TABLE_VIEWSETTINGS_COUNT);
    beans::PropertyValue* pSettings = rSettings.getArray();
    if (pSettings)
    {
        pSettings[SC_CURSOR_X].Name = SC_CURSORPOSITIONX;
        pSettings[SC_CURSOR_X].Value <<= sal_Int32(nCurX);
        pSettings[SC_CURSOR_Y].Name = SC_CURSORPOSITIONY;
        pSettings[SC_CURSOR_Y].Value <<= sal_Int32(nCurY);
        pSettings[SC_HORIZONTAL_SPLIT_MODE].Name = SC_HORIZONTALSPLITMODE;
        pSettings[SC_HORIZONTAL_SPLIT_MODE].Value <<= sal_Int16(eHSplitMode);
        pSettings[SC_VERTICAL_SPLIT_MODE].Name = SC_VERTICALSPLITMODE;
        pSettings[SC_VERTICAL_SPLIT_MODE].Value <<= sal_Int16(eVSplitMode);
        pSettings[SC_HORIZONTAL_SPLIT_POSITION].Name = SC_HORIZONTALSPLITPOSITION;
        if (eHSplitMode == SC_SPLIT_FIX)
            pSettings[SC_HORIZONTAL_SPLIT_POSITION].Value <<= sal_Int32(nFixPosX);
        else
            pSettings[SC_HORIZONTAL_SPLIT_POSITION].Value <<= sal_Int32(nHSplitPos);
        pSettings[SC_VERTICAL_SPLIT_POSITION].Name = SC_VERTICALSPLITPOSITION;
        if (eVSplitMode == SC_SPLIT_FIX)
            pSettings[SC_VERTICAL_SPLIT_POSITION].Value <<= sal_Int32(nFixPosY);
        else
            pSettings[SC_VERTICAL_SPLIT_POSITION].Value <<= sal_Int32(nVSplitPos);
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

        sal_Int32 nZoomValue = long(aZoomY * 100);
        sal_Int32 nPageZoomValue = long(aPageZoomY * 100);
        pSettings[SC_TABLE_ZOOM_TYPE].Name = SC_ZOOMTYPE;
        pSettings[SC_TABLE_ZOOM_TYPE].Value <<= sal_Int16(eZoomType);
        pSettings[SC_TABLE_ZOOM_VALUE].Name = SC_ZOOMVALUE;
        pSettings[SC_TABLE_ZOOM_VALUE].Value <<= nZoomValue;
        pSettings[SC_TABLE_PAGE_VIEW_ZOOM_VALUE].Name = SC_PAGEVIEWZOOMVALUE;
        pSettings[SC_TABLE_PAGE_VIEW_ZOOM_VALUE].Value <<= nPageZoomValue;

        pSettings[SC_TABLE_SHOWGRID].Name = SC_UNO_SHOWGRID;
        pSettings[SC_TABLE_SHOWGRID].Value <<= bShowGrid;
    }

    // Common SdrModel processing
    rViewData.GetDocument()->GetDrawLayer()->WriteUserDataSequence(rSettings);
}

void ScViewDataTable::ReadUserDataSequence(const uno::Sequence <beans::PropertyValue>& aSettings, ScViewData& rViewData, SCTAB nTab, bool& rHasZoom )
{
    rHasZoom = false;

    sal_Int32 nCount(aSettings.getLength());
    sal_Int32 nTemp32(0);
    sal_Int16 nTemp16(0);
    sal_Int32 nTempPosV(0);
    sal_Int32 nTempPosH(0);
    sal_Int32 nTempPosVTw(0);
    sal_Int32 nTempPosHTw(0);
    bool bHasVSplitInTwips = false;
    bool bHasHSplitInTwips = false;
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();
    for (sal_Int32 i = 0; i < nCount; i++)
    {
        OUString sName(aSettings[i].Name);
        if (sName == SC_CURSORPOSITIONX)
        {
            aSettings[i].Value >>= nTemp32;
            nCurX = SanitizeCol( static_cast<SCCOL>(nTemp32));
        }
        else if (sName == SC_CURSORPOSITIONY)
        {
            aSettings[i].Value >>= nTemp32;
            nCurY = SanitizeRow( static_cast<SCROW>(nTemp32));
        }
        else if (sName == SC_HORIZONTALSPLITMODE)
        {
            if ((aSettings[i].Value >>= nTemp16) && nTemp16 <= ScSplitMode::SC_SPLIT_MODE_MAX_ENUM)
                eHSplitMode = static_cast<ScSplitMode>(nTemp16);
        }
        else if (sName == SC_VERTICALSPLITMODE)
        {
            if ((aSettings[i].Value >>= nTemp16) && nTemp16 <= ScSplitMode::SC_SPLIT_MODE_MAX_ENUM)
                eVSplitMode = static_cast<ScSplitMode>(nTemp16);
        }
        else if (sName == SC_HORIZONTALSPLITPOSITION)
        {
            aSettings[i].Value >>= nTempPosH;
            bHasHSplitInTwips = false;
        }
        else if (sName == SC_VERTICALSPLITPOSITION)
        {
            aSettings[i].Value >>= nTempPosV;
            bHasVSplitInTwips = false;
        }
        else if (sName == SC_HORIZONTALSPLITPOSITION_TWIPS)
        {
            aSettings[i].Value >>= nTempPosHTw;
            bHasHSplitInTwips = true;
        }
        else if (sName == SC_VERTICALSPLITPOSITION_TWIPS)
        {
            aSettings[i].Value >>= nTempPosVTw;
            bHasVSplitInTwips = true;
        }
        else if (sName == SC_ACTIVESPLITRANGE)
        {
            if ((aSettings[i].Value >>= nTemp16) && nTemp16 <= ScSplitPos::SC_SPLIT_POS_MAX_ENUM)
                eWhichActive = static_cast<ScSplitPos>(nTemp16);
        }
        else if (sName == SC_POSITIONLEFT)
        {
            aSettings[i].Value >>= nTemp32;
            nPosX[SC_SPLIT_LEFT] = bIsTiledRendering ? 0 :
                                   SanitizeCol( static_cast<SCCOL>(nTemp32));
        }
        else if (sName == SC_POSITIONRIGHT)
        {
            aSettings[i].Value >>= nTemp32;
            nPosX[SC_SPLIT_RIGHT] = bIsTiledRendering ? 0 :
                                    SanitizeCol( static_cast<SCCOL>(nTemp32));
        }
        else if (sName == SC_POSITIONTOP)
        {
            aSettings[i].Value >>= nTemp32;
            nPosY[SC_SPLIT_TOP] = bIsTiledRendering ? 0 :
                                  SanitizeRow( static_cast<SCROW>(nTemp32));
        }
        else if (sName == SC_POSITIONBOTTOM)
        {
            aSettings[i].Value >>= nTemp32;
            nPosY[SC_SPLIT_BOTTOM] = bIsTiledRendering ? 0 :
                                     SanitizeRow( static_cast<SCROW>(nTemp32));
        }
        else if (sName == SC_ZOOMTYPE)
        {
            aSettings[i].Value >>= nTemp16;
            eZoomType = SvxZoomType(nTemp16);
            rHasZoom = true;        // set if there is any zoom information
        }
        else if (sName == SC_ZOOMVALUE)
        {
            aSettings[i].Value >>= nTemp32;
            Fraction aZoom(nTemp32, 100);
            aZoomX = aZoomY = aZoom;
            rHasZoom = true;
        }
        else if (sName == SC_PAGEVIEWZOOMVALUE)
        {
            aSettings[i].Value >>= nTemp32;
            Fraction aZoom(nTemp32, 100);
            aPageZoomX = aPageZoomY = aZoom;
            rHasZoom = true;
        }
        else if (sName == SC_UNO_SHOWGRID)
        {
            aSettings[i].Value >>= bShowGrid;
        }
        else if (sName == SC_TABLESELECTED)
        {
            bool bSelected = false;
            aSettings[i].Value >>= bSelected;
            rViewData.GetMarkData().SelectTable( nTab, bSelected );
        }
        else if (sName == SC_UNONAME_TABCOLOR)
        {
            // There are documents out there that have their tab color defined as a view setting.
            Color aColor = COL_AUTO;
            aSettings[i].Value >>= aColor;
            if (aColor != COL_AUTO)
            {
                ScDocument* pDoc = rViewData.GetDocument();
                pDoc->SetTabBgColor(nTab, aColor);
            }
        }
        // Fallback to common SdrModel processing
        else rViewData.GetDocument()->GetDrawLayer()->ReadUserDataSequenceValue(&aSettings[i]);
    }

    if (eHSplitMode == SC_SPLIT_FIX)
        nFixPosX = SanitizeCol( static_cast<SCCOL>( bHasHSplitInTwips ? nTempPosHTw : nTempPosH ));
    else
        nHSplitPos = bHasHSplitInTwips ? static_cast< long >( nTempPosHTw * rViewData.GetPPTX() ) : nTempPosH;

    if (eVSplitMode == SC_SPLIT_FIX)
        nFixPosY = SanitizeRow( static_cast<SCROW>( bHasVSplitInTwips ? nTempPosVTw : nTempPosV ));
    else
        nVSplitPos = bHasVSplitInTwips ? static_cast< long >( nTempPosVTw * rViewData.GetPPTY() ) : nTempPosV;

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

ScViewData::ScViewData( ScDocShell* pDocSh, ScTabViewShell* pViewSh ) :
        nPPTX(0.0),
        nPPTY(0.0),
        mpMarkData(new ScMarkData),
        pDocShell   ( pDocSh ),
        pDoc        ( nullptr ),
        pView       ( pViewSh ),
        pViewShell  ( pViewSh ),
        pOptions    ( new ScViewOptions ),
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
        m_nLOKPageUpDownOffset( 0 )
{
    mpMarkData->SelectOneTable(0); // Sync with nTabNo

    SetGridMode     ( true );
    SetSyntaxMode   ( false );
    SetHeaderMode   ( true );
    SetTabMode      ( true );
    SetVScrollMode  ( true );
    SetHScrollMode  ( true );
    SetOutlineMode  ( true );

    aScrSize = Size( long( STD_COL_WIDTH           * PIXEL_PER_TWIPS * OLE_STD_CELLS_X ),
                     static_cast<long>( ScGlobal::nStdRowHeight * PIXEL_PER_TWIPS * OLE_STD_CELLS_Y ) );
    maTabData.emplace_back( new ScViewDataTable );
    pThisTab = maTabData[nTabNo].get();
    for (sal_uInt16 j=0; j<4; j++)
    {
        pEditView[j] = nullptr;
        bEditActive[j] = false;
    }

    nEditEndCol = nEditStartCol = nEditCol = 0;
    nEditEndRow = nEditRow = 0;
    nTabStartCol = SC_TABSTART_NONE;

    if (pDocShell)
    {
        pDoc = &pDocShell->GetDocument();
        *pOptions = pDoc->GetViewOptions();
    }

    // don't show hidden tables
    if (pDoc && !pDoc->IsVisible(nTabNo))
    {
        while ( !pDoc->IsVisible(nTabNo) && pDoc->HasTable(nTabNo+1) )
        {
            ++nTabNo;
            maTabData.emplace_back(nullptr);
        }
        maTabData[nTabNo].reset( new ScViewDataTable() );
        pThisTab = maTabData[nTabNo].get();
    }

    if (pDoc)
    {
        SCTAB nTableCount = pDoc->GetTableCount();
        EnsureTabDataSize(nTableCount);
    }

    CalcPPT();
}

void ScViewData::InitData( ScDocument* pDocument )
{
    pDoc = pDocument;
    *pOptions = pDoc->GetViewOptions();
}

ScDocument* ScViewData::GetDocument() const
{
    if (pDoc)
        return pDoc;
    else if (pDocShell)
        return &pDocShell->GetDocument();

    OSL_FAIL("no document on ViewData");
    return nullptr;
}

ScViewData::~ScViewData() COVERITY_NOEXCEPT_FALSE
{
    KillEditView();
    pOptions.reset();
}

void ScViewData::UpdateCurrentTab()
{
    assert(0 <= nTabNo && static_cast<size_t>(nTabNo) < maTabData.size());
    pThisTab = maTabData[nTabNo].get();
    while (!pThisTab)
    {
        if (nTabNo > 0)
            pThisTab = maTabData[--nTabNo].get();
        else
        {
            maTabData[0].reset(new ScViewDataTable);
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
    mpMarkData->InsertTab( nTab );
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
        mpMarkData->InsertTab( i );
    }
    UpdateCurrentTab();
}

void ScViewData::DeleteTab( SCTAB nTab )
{
    assert(nTab < static_cast<SCTAB>(maTabData.size()));
    maTabData.erase(maTabData.begin() + nTab);

    if (static_cast<size_t>(nTabNo) >= maTabData.size())
    {
        EnsureTabDataSize(1);
        nTabNo = maTabData.size() - 1;
    }
    UpdateCurrentTab();
    mpMarkData->DeleteTab( nTab );
}

void ScViewData::DeleteTabs( SCTAB nTab, SCTAB nSheets )
{
    for (SCTAB i = 0; i < nSheets; ++i)
    {
        mpMarkData->DeleteTab( nTab + i );
    }
    maTabData.erase(maTabData.begin() + nTab, maTabData.begin()+ nTab+nSheets);
    if (static_cast<size_t>(nTabNo) >= maTabData.size())
    {
        EnsureTabDataSize(1);
        nTabNo = maTabData.size() - 1;
    }
    UpdateCurrentTab();
}

void ScViewData::CopyTab( SCTAB nSrcTab, SCTAB nDestTab )
{
    if (nDestTab==SC_TAB_APPEND)
        nDestTab = pDoc->GetTableCount() - 1;   // something had to have been copied

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
    mpMarkData->InsertTab( nDestTab );
}

void ScViewData::MoveTab( SCTAB nSrcTab, SCTAB nDestTab )
{
    if (nDestTab==SC_TAB_APPEND)
        nDestTab = pDoc->GetTableCount() - 1;
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
    mpMarkData->DeleteTab( nSrcTab );
    mpMarkData->InsertTab( nDestTab );            // adapted if needed
}

void ScViewData::CreateTabData( std::vector< SCTAB >& rvTabs )
{
    std::vector< SCTAB >::iterator it_end = rvTabs.end();
    for ( std::vector< SCTAB >::iterator it = rvTabs.begin(); it != it_end; ++it )
        CreateTabData(*it);
}

void ScViewData::SetZoomType( SvxZoomType eNew, std::vector< SCTAB >& tabs )
{
    bool bAll = tabs.empty();

    if ( !bAll ) // create associated table data
        CreateTabData( tabs );

    if ( bAll )
    {
        for ( SCTAB i = 0; i < static_cast<SCTAB>(maTabData.size()); ++i )
        {
            if ( maTabData[i] )
                maTabData[i]->eZoomType = eNew;
        }
        eDefZoomType = eNew;
    }
    else
    {
        std::vector< SCTAB >::iterator it_end = tabs.end();
        std::vector< SCTAB >::iterator it = tabs.begin();
        for ( ; it != it_end; ++it )
        {
            SCTAB i = *it;
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
        ScMarkData::iterator itr = mpMarkData->begin(), itrEnd = mpMarkData->end();
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
        for ( SCTAB i = 0; i < static_cast<SCTAB>(maTabData.size()); ++i )
        {
            if ( maTabData[i] )
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
        std::vector< SCTAB >::iterator it_end = tabs.end();
        std::vector< SCTAB >::iterator it = tabs.begin();
        for ( ; it != it_end; ++it )
        {
            SCTAB i = *it;
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
        ScMarkData::iterator itr = mpMarkData->begin(), itrEnd = mpMarkData->end();
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
            rNewMark.GetMarkArea( rRange );
            if (ScViewUtil::HasFiltered( rRange, GetDocument()))
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
        rRange = ScRange( GetCurX(), GetCurY(), GetTabNo() );
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
    ScMarkData aNewMark(*mpMarkData);       // use a local copy for MarkToSimple
    ScMarkType eMarkType = GetSimpleArea( aRange, aNewMark);
    aRange.GetVars( rStartCol, rStartRow, rStartTab, rEndCol, rEndRow, rEndTab);
    return eMarkType;
}

ScMarkType ScViewData::GetSimpleArea( ScRange& rRange ) const
{
    //  parameter bMergeMark is no longer needed, see above

    ScMarkData aNewMark(*mpMarkData);       // use a local copy for MarkToSimple
    return GetSimpleArea( rRange, aNewMark);
}

void ScViewData::GetMultiArea( ScRangeListRef& rRange ) const
{
    //  parameter bMergeMark is no longer needed, see GetSimpleArea

    ScMarkData aNewMark(*mpMarkData);       // use a local copy for MarkToSimple

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
        if (nStartRow==0 && nEndRow==MAXROW)
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
        if (nStartCol==0 && nEndCol==MAXCOL)
            return true;

    return false;
}

bool ScViewData::IsMultiMarked()
{
    // Test for "real" multi selection, calling MarkToSimple on a local copy,
    // and taking filtered in simple area marks into account.

    ScRange aDummy;
    ScMarkType eType = GetSimpleArea(aDummy);
    return (eType & SC_MARK_SIMPLE) != SC_MARK_SIMPLE;
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
    // successful.. Even with only 10 bytes per cell that would already be
    // 230MB, formula cells would be 100 bytes and more per cell.
    // rows * columns > 23m => rows > 23m / columns
    // to not overflow in case number of available columns or rows would be
    // arbitrarily increased.
    // We could refine this and take some actual cell size into account,
    // evaluate available memory and what not, but..
    const sal_Int32 kMax = 23 * 1024 * 1024;    // current MAXROWCOUNT is 1024*1024=1048576
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

SCCOL ScViewData::GetCurXForTab( SCTAB nTabIndex ) const
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
        return -1;

    return maTabData[nTabIndex]->nCurX;
}

SCROW ScViewData::GetCurYForTab( SCTAB nTabIndex ) const
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
            return -1;

    return maTabData[nTabIndex]->nCurY;
}

void ScViewData::SetCurXForTab( SCCOL nNewCurX, SCTAB nTabIndex )
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
            return;

    maTabData[nTabIndex]->nCurX = nNewCurX;
}

void ScViewData::SetCurYForTab( SCCOL nNewCurY, SCTAB nTabIndex )
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
            return;

    maTabData[nTabIndex]->nCurY = nNewCurY;
}

void ScViewData::SetMaxTiledCol( SCCOL nNewMaxCol )
{
    if (nNewMaxCol < 0)
        nNewMaxCol = 0;
    if (nNewMaxCol > MAXCOL)
        nNewMaxCol = MAXCOL;

    const SCTAB nTab = GetTabNo();
    ScDocument* pThisDoc = pDoc;
    auto GetColWidthPx = [pThisDoc, nTab](SCCOL nCol) {
        const sal_uInt16 nSize = pThisDoc->GetColWidth(nCol, nTab);
        const long nSizePx = ScViewData::ToPixel(nSize, 1.0 / TWIPS_PER_PIXEL);
        return nSizePx;
    };

    long nTotalPixels = GetLOKWidthHelper().computePosition(nNewMaxCol, GetColWidthPx);

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
    ScDocument* pThisDoc = pDoc;
    auto GetRowHeightPx = [pThisDoc, nTab](SCROW nRow) {
        const sal_uInt16 nSize = pThisDoc->GetRowHeight(nRow, nTab);
        const long nSizePx = ScViewData::ToPixel(nSize, 1.0 / TWIPS_PER_PIXEL);
        return nSizePx;
    };

    long nTotalPixels = GetLOKHeightHelper().computePosition(nNewMaxRow, GetRowHeightPx);

    SAL_INFO("sc.lok.docsize", "ScViewData::SetMaxTiledRow: nNewMaxRow: "
            << nNewMaxRow << ", nTotalPixels: " << nTotalPixels);

    GetLOKHeightHelper().removeByIndex(pThisTab->nMaxTiledRow);
    GetLOKHeightHelper().insert(nNewMaxRow, nTotalPixels);

    pThisTab->nMaxTiledRow = nNewMaxRow;
}

tools::Rectangle ScViewData::GetEditArea( ScSplitPos eWhich, SCCOL nPosX, SCROW nPosY,
                                          vcl::Window* pWin, const ScPatternAttr* pPattern,
                                          bool bForceToTop )
{
    return ScEditUtil( pDoc, nPosX, nPosY, nTabNo, GetScrPos(nPosX,nPosY,eWhich,true),
                        pWin, nPPTX, nPPTY, GetZoomX(), GetZoomY() ).
                            GetEditArea( pPattern, bForceToTop );
}

void ScViewData::SetEditEngine( ScSplitPos eWhich,
                                ScEditEngineDefaulter* pNewEngine,
                                vcl::Window* pWin, SCCOL nNewX, SCROW nNewY )
{
    bool bLayoutRTL = pDoc->IsLayoutRTL( nTabNo );
    ScHSplitPos eHWhich = WhichH(eWhich);

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

        if (comphelper::LibreOfficeKit::isActive())
        {
            pEditView[eWhich]->RegisterViewShell(pViewShell);
        }
    }

    // add windows from other views
    if (!bWasThere && comphelper::LibreOfficeKit::isActive())
    //if (comphelper::LibreOfficeKit::isActive())
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

    const ScPatternAttr* pPattern = pDoc->GetPattern( nNewX, nNewY, nTabNo );
    SvxCellHorJustify eJust = pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue();

    bool bBreak = ( eJust == SvxCellHorJustify::Block ) ||
                    pPattern->GetItem(ATTR_LINEBREAK).GetValue();

    bool bAsianVertical = pNewEngine->IsVertical();     // set by InputHandler

    tools::Rectangle aPixRect = ScEditUtil( pDoc, nNewX,nNewY,nTabNo, GetScrPos(nNewX,nNewY,eWhich),
                                        pWin, nPPTX,nPPTY,GetZoomX(),GetZoomY() ).
                                            GetEditArea( pPattern, true );

    //  when right-aligned, leave space for the cursor
    //  in vertical mode, editing is always right-aligned
    if ( GetEditAdjust() == SvxAdjust::Right || bAsianVertical )
        aPixRect.AdjustRight(1 );

    tools::Rectangle aOutputArea = pWin->PixelToLogic( aPixRect, GetLogicMode() );
    pEditView[eWhich]->SetOutputArea( aOutputArea );

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
        if ( bAsianVertical )
            bGrowCentered = bGrowToLeft = false;   // keep old behavior for asian mode

        long nSizeXPix;
        if (bBreak && !bAsianVertical)
            nSizeXPix = aPixRect.GetWidth();    // papersize -> no horizontal scrolling
        else
        {
            OSL_ENSURE(pView,"no View for EditView");

            if ( bGrowCentered )
            {
                //  growing into both directions until one edge is reached
                //! should be limited to whole cells in both directions
                long nLeft = aPixRect.Left();
                long nRight = pView->GetGridWidth(eHWhich) - aPixRect.Right();
                nSizeXPix = aPixRect.GetWidth() + 2 * std::min( nLeft, nRight );
            }
            else if ( bGrowToLeft )
                nSizeXPix = aPixRect.Right();   // space that's available in the window when growing to the left
            else
                nSizeXPix = pView->GetGridWidth(eHWhich) - aPixRect.Left();

            if ( nSizeXPix <= 0 )
                nSizeXPix = aPixRect.GetWidth();    // editing outside to the right of the window -> keep cell width
        }
        OSL_ENSURE(pView,"no View for EditView");
        long nSizeYPix = pView->GetGridHeight(WhichV(eWhich)) - aPixRect.Top();
        if ( nSizeYPix <= 0 )
            nSizeYPix = aPixRect.GetHeight();   // editing outside below the window -> keep cell height

        Size aPaperSize = pView->GetActiveWin()->PixelToLogic( Size( nSizeXPix, nSizeYPix ), GetLogicMode() );
        if ( bBreak && !bAsianVertical && SC_MOD()->GetInputOptions().GetTextWysiwyg() )
        {
            //  if text is formatted for printer, use the exact same paper width
            //  (and same line breaks) as for output.

            Fraction aFract(1,1);
            tools::Rectangle aUtilRect = ScEditUtil( pDoc,nNewX,nNewY,nTabNo, Point(0,0), pWin,
                                    HMM_PER_TWIPS, HMM_PER_TWIPS, aFract, aFract ).GetEditArea( pPattern, false );
            aPaperSize.setWidth( aUtilRect.GetWidth() );
        }
        pNewEngine->SetPaperSize( aPaperSize );

        // sichtbarer Ausschnitt
        Size aPaper = pNewEngine->GetPaperSize();
        tools::Rectangle aVis = pEditView[eWhich]->GetVisArea();
        long nDiff = aVis.Right() - aVis.Left();
        if ( GetEditAdjust() == SvxAdjust::Right )
        {
            aVis.SetRight( aPaper.Width() - 1 );
            bMoveArea = !bLayoutRTL;
        }
        else if ( GetEditAdjust() == SvxAdjust::Center )
        {
            aVis.SetRight( ( aPaper.Width() - 1 + nDiff ) / 2 );
            bMoveArea = true;   // always
        }
        else
        {
            aVis.SetRight( nDiff );
            bMoveArea = bLayoutRTL;
        }
        aVis.SetLeft( aVis.Right() - nDiff );
        // #i49561# Important note:
        // The set offset of the visible area of the EditView for centered and
        // right alignment in horizontal layout is consider by instances of
        // class <ScEditObjectViewForwarder> in its methods <LogicToPixel(..)>
        // and <PixelToLogic(..)>. This is needed for the correct visibility
        // of paragraphs in edit mode at the accessibility API.
        pEditView[eWhich]->SetVisArea(aVis);
        //  UpdateMode has been disabled in ScInputHandler::StartTable
        //  must be enabled before EditGrowY (GetTextHeight)
        pNewEngine->SetUpdateMode( true );

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
    if ( aBackCol.GetTransparency() > 0 )
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

    ScDocument* pLocalDoc = GetDocument();

    ScSplitPos eWhich = GetActivePart();
    ScHSplitPos eHWhich = WhichH(eWhich);
    EditView* pCurView = pEditView[eWhich].get();

    if ( !pCurView || !bEditActive[eWhich])
        return;

    bool bLayoutRTL = pLocalDoc->IsLayoutRTL( nTabNo );

    ScEditEngineDefaulter* pEngine =
        static_cast<ScEditEngineDefaulter*>( pCurView->GetEditEngine() );
    vcl::Window* pWin = pCurView->GetWindow();

    // Get the left- and right-most column positions.
    SCCOL nLeft = GetPosX(eHWhich);
    SCCOL nRight = nLeft + VisibleCellsX(eHWhich);

    Size        aSize = pEngine->GetPaperSize();
    tools::Rectangle   aArea = pCurView->GetOutputArea();
    long        nOldRight = aArea.Right();

    // Margin is already included in the original width.
    long nTextWidth = pEngine->CalcTextWidth();

    bool bChanged = false;
    bool bAsianVertical = pEngine->IsVertical();

    //  get bGrow... variables the same way as in SetEditEngine
    const ScPatternAttr* pPattern = pLocalDoc->GetPattern( nEditCol, nEditRow, nTabNo );
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
            long nLogicLeft = 0;
            if ( nEditStartCol > nLeft )
            {
                --nEditStartCol;
                long nLeftPix = ToPixel( pLocalDoc->GetColWidth( nEditStartCol, nTabNo ), nPPTX );
                nLogicLeft = pWin->PixelToLogic(Size(nLeftPix,0)).Width();
            }
            long nLogicRight = 0;
            if ( nEditEndCol < nRight )
            {
                ++nEditEndCol;
                long nRightPix = ToPixel( pLocalDoc->GetColWidth( nEditEndCol, nTabNo ), nPPTX );
                nLogicRight = pWin->PixelToLogic(Size(nRightPix,0)).Width();
            }

            aArea.AdjustLeft( -(bLayoutRTL ? nLogicRight : nLogicLeft) );
            aArea.AdjustRight(bLayoutRTL ? nLogicLeft : nLogicRight );

            if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            {
                long nCenter = ( aArea.Left() + aArea.Right() ) / 2;
                long nHalf = aSize.Width() / 2;
                aArea.SetLeft( nCenter - nHalf + 1 );
                aArea.SetRight( nCenter + aSize.Width() - nHalf - 1 );
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
            long nPix = ToPixel( pLocalDoc->GetColWidth( nEditStartCol, nTabNo ), nPPTX );
            long nLogicWidth = pWin->PixelToLogic(Size(nPix,0)).Width();
            if ( !bLayoutRTL )
                aArea.AdjustLeft( -nLogicWidth );
            else
                aArea.AdjustRight(nLogicWidth );

            if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            {
                if ( !bLayoutRTL )
                    aArea.SetLeft( aArea.Right() - aSize.Width() + 1 );
                else
                    aArea.SetRight( aArea.Left() + aSize.Width() - 1 );
            }

            bChanged = true;
        }
    }
    else
    {
        while (aArea.GetWidth() + 0 < nTextWidth && nEditEndCol < nRight)
        {
            ++nEditEndCol;
            long nPix = ToPixel( pLocalDoc->GetColWidth( nEditEndCol, nTabNo ), nPPTX );
            long nLogicWidth = pWin->PixelToLogic(Size(nPix,0)).Width();
            if ( bLayoutRTL )
                aArea.AdjustLeft( -nLogicWidth );
            else
                aArea.AdjustRight(nLogicWidth );

            if ( aArea.Right() > aArea.Left() + aSize.Width() - 1 )
            {
                if ( bLayoutRTL )
                    aArea.SetLeft( aArea.Right() - aSize.Width() + 1 );
                else
                    aArea.SetRight( aArea.Left() + aSize.Width() - 1 );
            }

            bChanged = true;
        }
    }

    if (bChanged)
    {
        if ( bMoveArea || bGrowCentered || bGrowBackwards || bLayoutRTL )
        {
            tools::Rectangle aVis = pCurView->GetVisArea();

            if ( bGrowCentered )
            {
                //  switch to center-aligned (undo?) and reset VisArea to center

                pEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );

                long nCenter = aSize.Width() / 2;
                long nVisSize = aArea.GetWidth();
                aVis.SetLeft( nCenter - nVisSize / 2 );
                aVis.SetRight( aVis.Left() + nVisSize - 1 );
            }
            else if ( bGrowToLeft )
            {
                //  switch to right-aligned (undo?) and reset VisArea to the right

                pEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );

                aVis.SetRight( aSize.Width() - 1 );
                aVis.SetLeft( aSize.Width() - aArea.GetWidth() );     // with the new, increased area
            }
            else
            {
                //  switch to left-aligned (undo?) and reset VisArea to the left

                pEngine->SetDefaultItem( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );

                long nMove = aVis.Left();
                aVis.SetLeft( 0 );
                aVis.AdjustRight( -nMove );
            }
            pCurView->SetVisArea( aVis );
            bMoveArea = false;
        }

        pCurView->SetOutputArea(aArea);

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
}

void ScViewData::EditGrowY( bool bInitial )
{
    if (bGrowing)
        return;

    comphelper::FlagRestorationGuard aFlagGuard(bGrowing, true);

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
    tools::Rectangle   aArea = pCurView->GetOutputArea();
    long        nOldBottom = aArea.Bottom();
    long        nTextHeight = pEngine->GetTextHeight();

    //  When editing a formula in a cell with optimal height, allow a larger portion
    //  to be clipped before extending to following rows, to avoid obscuring cells for
    //  reference input (next row is likely to be useful in formulas).
    long nAllowedExtra = SC_GROWY_SMALL_EXTRA;
    if ( nEditEndRow == nEditRow && !( pDoc->GetRowFlags( nEditRow, nTabNo ) & CRFlags::ManualSize ) &&
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
        ScDocument* pLocalDoc = GetDocument();
        long nPix = ToPixel( pLocalDoc->GetRowHeight( nEditEndRow, nTabNo ), nPPTY );
        aArea.AdjustBottom(pWin->PixelToLogic(Size(0,nPix)).Height() );

        if ( aArea.Bottom() > aArea.Top() + aSize.Height() - 1 )
        {
            aArea.SetBottom( aArea.Top() + aSize.Height() - 1 );
            bMaxReached = true;     // don't occupy more cells beyond paper size
        }

        bChanged = true;
        nAllowedExtra = SC_GROWY_SMALL_EXTRA;   // larger value is only for first row
    }

    if (bChanged)
    {
        pCurView->SetOutputArea(aArea);

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
}

void ScViewData::ResetEditView()
{
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
        maTabData[nNewTab].reset( new ScViewDataTable );

        maTabData[nNewTab]->eZoomType  = eDefZoomType;
        maTabData[nNewTab]->aZoomX     = aDefZoomX;
        maTabData[nNewTab]->aZoomY     = aDefZoomY;
        maTabData[nNewTab]->aPageZoomX = aDefPageZoomX;
        maTabData[nNewTab]->aPageZoomY = aDefPageZoomY;
    }
}

void ScViewData::CreateSelectedTabData()
{
    ScMarkData::iterator itr = mpMarkData->begin(), itrEnd = mpMarkData->end();
    for (; itr != itrEnd; ++itr)
        CreateTabData(*itr);
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
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
    {
        return nullptr;
    }
    return &(maTabData[nTabIndex]->aWidthHelper);
}

ScPositionHelper* ScViewData::GetLOKHeightHelper(SCTAB nTabIndex)
{
    if (!ValidTab(nTabIndex) || (nTabIndex >= static_cast<SCTAB>(maTabData.size())))
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
                                bool bAllowNeg ) const
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

    if (pView)
    {
        const_cast<ScViewData*>(this)->aScrSize.setWidth( pView->GetGridWidth(eWhichX) );
        const_cast<ScViewData*>(this)->aScrSize.setHeight( pView->GetGridHeight(eWhichY) );
    }

    sal_uInt16 nTSize;
    bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();


    SCCOL nPosX = GetPosX(eWhichX);
    long nScrPosX = 0;

    if (bAllowNeg || nWhereX >= nPosX)
    {
        SCROW nStartPosX = nPosX;
        if (bIsTiledRendering)
        {
            OSL_ENSURE(nPosX == 0, "Unsupported case.");
            const auto& rNearest = pThisTab->aWidthHelper.getNearestByIndex(nWhereX - 1);
            nStartPosX = rNearest.first + 1;
            nScrPosX = rNearest.second;
        }

        if (nWhereX >= nStartPosX)
        {
            for (SCCOL nX = nStartPosX; nX < nWhereX && (bAllowNeg || bIsTiledRendering || nScrPosX <= aScrSize.Width()); nX++)
            {
                if ( nX > MAXCOL )
                    nScrPosX = 0x7FFFFFFF;
                else
                {
                    nTSize = pDoc->GetColWidth( nX, nTabNo );
                    if (nTSize)
                    {
                        long nSizeXPix = ToPixel( nTSize, nPPTX );
                        nScrPosX += nSizeXPix;
                    }
                }
            }
        }
        else
        {
            for (SCCOL nX = nStartPosX; nX > nWhereX;)
            {
                --nX;
                nTSize = pDoc->GetColWidth( nX, nTabNo );
                if (nTSize)
                {
                    long nSizeXPix = ToPixel( nTSize, nPPTX );
                    nScrPosX -= nSizeXPix;
                }
            }
        }

    }


    SCROW nPosY = GetPosY(eWhichY);
    long nScrPosY = 0;

    if (bAllowNeg || nWhereY >= nPosY)
    {
        SCROW nStartPosY = nPosY;
        if (bIsTiledRendering)
        {
            OSL_ENSURE(nPosY == 0, "Unsupported case.");
            const auto& rNearest = pThisTab->aHeightHelper.getNearestByIndex(nWhereY - 1);
            nStartPosY = rNearest.first + 1;
            nScrPosY = rNearest.second;
        }

        if (nWhereY >= nStartPosY)
        {
            for (SCROW nY = nStartPosY; nY < nWhereY && (bAllowNeg || bIsTiledRendering || nScrPosY <= aScrSize.Height()); nY++)
            {
                if ( nY > MAXROW )
                    nScrPosY = 0x7FFFFFFF;
                else
                {
                    nTSize = pDoc->GetRowHeight( nY, nTabNo );
                    if (nTSize)
                    {
                        long nSizeYPix = ToPixel( nTSize, nPPTY );
                        nScrPosY += nSizeYPix;
                    }
                    else if ( nY < MAXROW )
                    {
                        // skip multiple hidden rows (forward only for now)
                        SCROW nNext = pDoc->FirstVisibleRow(nY + 1, MAXROW, nTabNo);
                        if ( nNext > MAXROW )
                            nY = MAXROW;
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
                nTSize = pDoc->GetRowHeight( nY, nTabNo );
                if (nTSize)
                {
                    long nSizeYPix = ToPixel( nTSize, nPPTY );
                    nScrPosY -= nSizeYPix;
                }
            }
        }
    }

    if ( pDoc->IsLayoutRTL( nTabNo ) )
    {
        //  mirror horizontal position
        nScrPosX = aScrSize.Width() - 1 - nScrPosX;
    }

    return Point( nScrPosX, nScrPosY );
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
        if ( nColNo < 0 || nColNo > MAXCOL )
            bOut = true;
        else
        {
            sal_uInt16 nTSize = pDoc->GetColWidth( nColNo, nTabNo );
            if (nTSize)
            {
                long nSizeXPix = ToPixel( nTSize, nPPTX );
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
        long nScrPosY = 0;
        AddPixelsWhile( nScrPosY, nScrSizeY, nY, MAXROW, nPPTY, pDoc, nTabNo);
        // Original loop ended on last evaluated +1 or if that was MAXROW even
        // on MAXROW+2.
        nY += (nY == MAXROW ? 2 : 1);
        nY -= nPosY;
    }
    else
    {
        // backward
        nY = nPosY-1;
        long nScrPosY = 0;
        AddPixelsWhileBackward( nScrPosY, nScrSizeY, nY, 0, nPPTY, pDoc, nTabNo);
        // Original loop ended on last evaluated -1 or if that was 0 even on
        // -2.
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

bool ScViewData::GetMergeSizePixel( SCCOL nX, SCROW nY, long& rSizeXPix, long& rSizeYPix ) const
{
    const ScMergeAttr* pMerge = pDoc->GetAttr( nX,nY,nTabNo, ATTR_MERGE );
    if ( pMerge->GetColMerge() > 1 || pMerge->GetRowMerge() > 1 )
    {
        long nOutWidth = 0;
        long nOutHeight = 0;
        SCCOL nCountX = pMerge->GetColMerge();
        for (SCCOL i=0; i<nCountX; i++)
            nOutWidth += ToPixel( pDoc->GetColWidth(nX+i,nTabNo), nPPTX );
        SCROW nCountY = pMerge->GetRowMerge();

        for (SCROW nRow = nY; nRow <= nY+nCountY-1; ++nRow)
        {
            SCROW nLastRow = nRow;
            if (pDoc->RowHidden(nRow, nTabNo, nullptr, &nLastRow))
            {
                nRow = nLastRow;
                continue;
            }

            sal_uInt16 nHeight = pDoc->GetRowHeight(nRow, nTabNo);
            nOutHeight += ToPixel(nHeight, nPPTY);
        }

        rSizeXPix = nOutWidth;
        rSizeYPix = nOutHeight;
        return true;
    }
    else
    {
        rSizeXPix = ToPixel( pDoc->GetColWidth( nX, nTabNo ), nPPTX );
        rSizeYPix = ToPixel( pDoc->GetRowHeight( nY, nTabNo ), nPPTY );
        return false;
    }
}

void ScViewData::GetPosFromPixel( long nClickX, long nClickY, ScSplitPos eWhich,
                                        SCCOL& rPosX, SCROW& rPosY,
                                        bool bTestMerge, bool bRepair )
{
    //  special handling of 0 is now in ScViewFunctionSet::SetCursorAtPoint

    ScHSplitPos eHWhich = WhichH(eWhich);
    ScVSplitPos eVWhich = WhichV(eWhich);

    if ( pDoc->IsLayoutRTL( nTabNo ) )
    {
        //  mirror horizontal position
        if (pView)
            aScrSize.setWidth( pView->GetGridWidth(eHWhich) );
        nClickX = aScrSize.Width() - 1 - nClickX;
    }

    SCCOL nStartPosX = GetPosX(eHWhich);
    SCROW nStartPosY = GetPosY(eVWhich);
    rPosX = nStartPosX;
    rPosY = nStartPosY;
    long nScrX = 0;
    long nScrY = 0;

    if (nClickX > 0)
    {
        while ( rPosX<=MAXCOL && nClickX >= nScrX )
        {
            nScrX += ToPixel( pDoc->GetColWidth( rPosX, nTabNo ), nPPTX );
            ++rPosX;
        }
        --rPosX;
    }
    else
    {
        while ( rPosX>0 && nClickX < nScrX )
        {
            --rPosX;
            nScrX -= ToPixel( pDoc->GetColWidth( rPosX, nTabNo ), nPPTX );
        }
    }

    if (nClickY > 0)
        AddPixelsWhile( nScrY, nClickY, rPosY, MAXROW, nPPTY, pDoc, nTabNo );
    else
    {
        /* TODO: could need some "SubPixelsWhileBackward" method */
        while ( rPosY>0 && nClickY < nScrY )
        {
            --rPosY;
            nScrY -= ToPixel( pDoc->GetRowHeight( rPosY, nTabNo ), nPPTY );
        }
    }

    //  cells to big?
    if ( rPosX == nStartPosX && nClickX > 0 )
    {
         if (pView)
            aScrSize.setWidth( pView->GetGridWidth(eHWhich) );
         if ( nClickX > aScrSize.Width() )
            ++rPosX;
    }
    if ( rPosY == nStartPosY && nClickY > 0 )
    {
        if (pView)
            aScrSize.setHeight( pView->GetGridHeight(eVWhich) );
        if ( nClickY > aScrSize.Height() )
            ++rPosY;
    }

    if (rPosX<0) rPosX=0;
    if (rPosX>MAXCOL) rPosX=MAXCOL;
    if (rPosY<0) rPosY=0;
    if (rPosY>MAXROW) rPosY=MAXROW;

    if (bTestMerge)
    {
        // public method to adapt position
        SCCOL nOrigX = rPosX;
        SCROW nOrigY = rPosY;
        pDoc->SkipOverlapped(rPosX, rPosY, nTabNo);
        bool bHOver = (nOrigX != rPosX);
        bool bVOver = (nOrigY != rPosY);

        if ( bRepair && ( bHOver || bVOver ) )
        {
            const ScMergeAttr* pMerge = pDoc->GetAttr( rPosX, rPosY, nTabNo, ATTR_MERGE );
            if ( ( bHOver && pMerge->GetColMerge() <= 1 ) ||
                 ( bVOver && pMerge->GetRowMerge() <= 1 ) )
            {
                OSL_FAIL("merge error found");

                pDoc->RemoveFlagsTab( 0,0, MAXCOL,MAXROW, nTabNo, ScMF::Hor | ScMF::Ver );
                SCCOL nEndCol = MAXCOL;
                SCROW nEndRow = MAXROW;
                pDoc->ExtendMerge( 0,0, nEndCol,nEndRow, nTabNo, true );
                if (pDocShell)
                    pDocShell->PostPaint( ScRange(0,0,nTabNo,MAXCOL,MAXROW,nTabNo), PaintPartFlags::Grid );
            }
        }
    }
}

void ScViewData::GetMouseQuadrant( const Point& rClickPos, ScSplitPos eWhich,
                                        SCCOL nPosX, SCROW nPosY, bool& rLeft, bool& rTop )
{
    bool bLayoutRTL = pDoc->IsLayoutRTL( nTabNo );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Point aCellStart = GetScrPos( nPosX, nPosY, eWhich, true );
    long nSizeX;
    long nSizeY;
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
        long nTPosX = pThisTab->nTPosX[eWhich];
        long nPixPosX = pThisTab->nPixPosX[eWhich];
        SCCOL i;
        if ( nNewPosX > nOldPosX )
            for ( i=nOldPosX; i<nNewPosX; i++ )
            {
                long nThis = pDoc->GetColWidth( i,nTabNo );
                nTPosX -= nThis;
                nPixPosX -= ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTX);
            }
        else
            for ( i=nNewPosX; i<nOldPosX; i++ )
            {
                long nThis = pDoc->GetColWidth( i,nTabNo );
                nTPosX += nThis;
                nPixPosX += ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTX);
            }

        pThisTab->nPosX[eWhich] = nNewPosX;
        pThisTab->nTPosX[eWhich] = nTPosX;
        pThisTab->nMPosX[eWhich] = static_cast<long>(nTPosX * HMM_PER_TWIPS);
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
        long nTPosY = pThisTab->nTPosY[eWhich];
        long nPixPosY = pThisTab->nPixPosY[eWhich];
        SCROW i, nHeightEndRow;
        if ( nNewPosY > nOldPosY )
            for ( i=nOldPosY; i<nNewPosY; i++ )
            {
                long nThis = pDoc->GetRowHeight( i, nTabNo, nullptr, &nHeightEndRow );
                SCROW nRows = std::min( nNewPosY, nHeightEndRow + 1) - i;
                i = nHeightEndRow;
                nTPosY -= nThis * nRows;
                nPixPosY -= ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTY) * nRows;
            }
        else
            for ( i=nNewPosY; i<nOldPosY; i++ )
            {
                long nThis = pDoc->GetRowHeight( i, nTabNo, nullptr, &nHeightEndRow );
                SCROW nRows = std::min( nOldPosY, nHeightEndRow + 1) - i;
                i = nHeightEndRow;
                nTPosY += nThis * nRows;
                nPixPosY += ToPixel(sal::static_int_cast<sal_uInt16>(nThis), nPPTY) * nRows;
            }

        pThisTab->nPosY[eWhich] = nNewPosY;
        pThisTab->nTPosY[eWhich] = nTPosY;
        pThisTab->nMPosY[eWhich] = static_cast<long>(nTPosY * HMM_PER_TWIPS);
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
        long nPixPosX = 0;
        SCCOL nPosX = pThisTab->nPosX[eWhich];
        for (SCCOL i=0; i<nPosX; i++)
            nPixPosX -= ToPixel(pDoc->GetColWidth(i,nTabNo), nPPTX);
        pThisTab->nPixPosX[eWhich] = nPixPosX;

        long nPixPosY = 0;
        SCROW nPosY = pThisTab->nPosY[eWhich];
        for (SCROW j=0; j<nPosY; j++)
            nPixPosY -= ToPixel(pDoc->GetRowHeight(j,nTabNo), nPPTY);
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
    long nSizePix;
    long nScrPosX = 0;
    long nScrPosY = 0;

    SetActivePart( SC_SPLIT_BOTTOMLEFT );
    SetPosX( SC_SPLIT_LEFT, nCol1 );
    SetPosY( SC_SPLIT_BOTTOM, nRow1 );

    for (nCol=nCol1; nCol<=nCol2; nCol++)
    {
        nTSize = pDoc->GetColWidth( nCol, nTabNo );
        if (nTSize)
        {
            nSizePix = ToPixel( nTSize, nPPTX );
            nScrPosX += static_cast<sal_uInt16>(nSizePix);
        }
    }

    for (nRow=nRow1; nRow<=nRow2; nRow++)
    {
        nTSize = pDoc->GetRowHeight( nRow, nTabNo );
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
    long nSize;
    long nTwips;
    long nAdd;
    bool bEnd;

    nSize = 0;
    nTwips = static_cast<long>(rVisAreaStart.X() / HMM_PER_TWIPS);
    if ( pDoc->IsLayoutRTL( nTabNo ) )
        nTwips = -nTwips;
    SCCOL nX1 = 0;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = static_cast<long>(pDoc->GetColWidth(nX1,nTabNo));
        if (nSize+nAdd <= nTwips+1 && nX1<MAXCOL)
        {
            nSize += nAdd;
            ++nX1;
        }
        else
            bEnd = true;
    }

    nSize = 0;
    nTwips = static_cast<long>(rVisAreaStart.Y() / HMM_PER_TWIPS);
    SCROW nY1 = 0;
    bEnd = false;
    while (!bEnd)
    {
        nAdd = static_cast<long>(pDoc->GetRowHeight(nY1,nTabNo));
        if (nSize+nAdd <= nTwips+1 && nY1<MAXROW)
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
    aScrSize.setWidth( static_cast<long>( aScrSize.Width() * ScGlobal::nScreenPPTX / HMM_PER_TWIPS ) );
    aScrSize.setHeight( static_cast<long>( aScrSize.Height() * ScGlobal::nScreenPPTY / HMM_PER_TWIPS ) );
}

ScDocFunc& ScViewData::GetDocFunc() const
{
    return pDocShell->GetDocFunc();
}

SfxBindings& ScViewData::GetBindings()
{
    OSL_ENSURE( pViewShell, "GetBindings() without ViewShell" );
    return pViewShell->GetViewFrame()->GetBindings();
}

SfxDispatcher& ScViewData::GetDispatcher()
{
    OSL_ENSURE( pViewShell, "GetDispatcher() without ViewShell" );
    return *pViewShell->GetViewFrame()->GetDispatcher();
}

ScMarkData& ScViewData::GetMarkData()
{
    return *mpMarkData;
}

const ScMarkData& ScViewData::GetMarkData() const
{
    return *mpMarkData;
}

vcl::Window* ScViewData::GetDialogParent()
{
    OSL_ENSURE( pViewShell, "GetDialogParent() without ViewShell" );
    return pViewShell->GetDialogParent();
}

ScGridWindow* ScViewData::GetActiveWin()
{
    OSL_ENSURE( pView, "GetActiveWin() without View" );
    return pView->GetActiveWin();
}

const ScGridWindow* ScViewData::GetActiveWin() const
{
    OSL_ENSURE( pView, "GetActiveWin() without View" );
    return pView->GetActiveWin();
}

ScDrawView* ScViewData::GetScDrawView()
{
    OSL_ENSURE( pView, "GetScDrawView() without View" );
    return pView->GetScDrawView();
}

bool ScViewData::IsMinimized()
{
    OSL_ENSURE( pView, "IsMinimized() without View" );
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

    aScrSize.setWidth( static_cast<long>(aWidth) );
    aScrSize.setHeight( static_cast<long>(aHeight) );
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

    if ( pDoc && pDoc->HasDetectiveObjects(nTabNo) )
    {
        SCCOL nEndCol = 0;
        SCROW nDummy = 0;
        pDoc->GetTableArea( nTabNo, nEndCol, nDummy );
        if (nEndCol<20)
            nEndCol = 20;           // same end position as when determining draw scale

        sal_uInt16 nTwips = pDoc->GetCommonWidth( nEndCol, nTabNo );
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

    if (nPPTX != nOldPPTX)
        GetLOKWidthHelper().invalidateByPosition(0L);
    if (nPPTY != nOldPPTY)
        GetLOKHeightHelper().invalidateByPosition(0L);
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

    sal_uInt16 nZoom = static_cast<sal_uInt16>(long(pThisTab->aZoomY * 100));
    rData = OUString::number( nZoom ) + "/";
    nZoom = static_cast<sal_uInt16>(long(pThisTab->aPageZoomY * 100));
    rData += OUString::number( nZoom ) + "/";
    if (bPagebreak)
        rData += "1";
    else
        rData += "0";

    rData += ";" + OUString::number( nTabNo ) + ";" TAG_TABBARWIDTH +
             OUString::number( pView->GetTabBarWidth() );

    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB i=0; i<nTabCount; i++)
    {
        rData += ";";                   // Numbering must not get mixed up under any circumstances
        if (i < static_cast<SCTAB>(maTabData.size()) && maTabData[i])
        {
            OUString cTabSep = OUString(SC_OLD_TABSEP);                // like 3.1
            if ( maTabData[i]->nCurY > MAXROW_30 ||
                 maTabData[i]->nPosY[0] > MAXROW_30 || maTabData[i]->nPosY[1] > MAXROW_30 ||
                 ( maTabData[i]->eVSplitMode == SC_SPLIT_FIX &&
                    maTabData[i]->nFixPosY > MAXROW_30 ) )
            {
                cTabSep = OUStringLiteral1(SC_NEW_TABSEP); // in order to not kill a 3.1-version
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

void ScViewData::ReadUserData(const OUString& rData)
{
    if (rData.isEmpty())    // empty string on "reload"
        return;             // then exit without assertion

    sal_Int32 nCount = comphelper::string::getTokenCount(rData, ';');
    if ( nCount <= 2 )
    {
        // when reload, in page preview, the preview UserData may have been left intact.
        // we don't want the zoom from the page preview here.
        OSL_FAIL("ReadUserData: This is not my data");
        return;
    }

    // not per sheet:
    SCTAB nTabStart = 2;

    Fraction aZoomX, aZoomY, aPageZoomX, aPageZoomY;    // evaluate (all sheets?)

    OUString aZoomStr = rData.getToken(0, ';');                 // Zoom/PageZoom/Mode
    sal_uInt16 nNormZoom = sal::static_int_cast<sal_uInt16>(aZoomStr.getToken(0,'/').toInt32());
    if ( nNormZoom >= MINZOOM && nNormZoom <= MAXZOOM )
        aZoomX = aZoomY = Fraction( nNormZoom, 100 );           //  "normal" zoom (always)
    sal_uInt16 nPageZoom = sal::static_int_cast<sal_uInt16>(aZoomStr.getToken(1,'/').toInt32());
    if ( nPageZoom >= MINZOOM && nPageZoom <= MAXZOOM )
        aPageZoomX = aPageZoomY = Fraction( nPageZoom, 100 );   // Pagebreak zoom, if set
    sal_Unicode cMode = aZoomStr.getToken(2,'/')[0];            // 0 or "0"/"1"
    SetPagebreakMode( cMode == '1' );
    // SetPagebreakMode must always be called due to CalcPPT / RecalcPixPos()

    // sheet may have become invalid (for instance last version):
    SCTAB nNewTab = static_cast<SCTAB>(rData.getToken(1, ';').toInt32());
    if (pDoc->HasTable( nNewTab ))
        SetTabNo(nNewTab);

    // if available, get tab bar width:
    OUString aTabOpt = rData.getToken(2, ';');

    OUString aRest;
    if (aTabOpt.startsWith(TAG_TABBARWIDTH, &aRest))
    {
        pView->SetTabBarWidth(aRest.toInt32());
        nTabStart = 3;
    }

    // per sheet
    SCTAB nPos = 0;
    while ( nCount > nPos+nTabStart )
    {
        aTabOpt = rData.getToken(static_cast<sal_Int32>(nPos+nTabStart), ';');
        EnsureTabDataSize(nPos + 1);
        if (!maTabData[nPos])
            maTabData[nPos].reset( new ScViewDataTable );

        sal_Unicode cTabSep = 0;
        if (comphelper::string::getTokenCount(aTabOpt, SC_OLD_TABSEP) >= 11)
            cTabSep = SC_OLD_TABSEP;
        else if (comphelper::string::getTokenCount(aTabOpt, SC_NEW_TABSEP) >= 11)
            cTabSep = SC_NEW_TABSEP;
        // '+' is only allowed, if we can deal with rows > 8192

        if (cTabSep)
        {
            maTabData[nPos]->nCurX = SanitizeCol( static_cast<SCCOL>(aTabOpt.getToken(0,cTabSep).toInt32()));
            maTabData[nPos]->nCurY = SanitizeRow( aTabOpt.getToken(1,cTabSep).toInt32());
            maTabData[nPos]->eHSplitMode = static_cast<ScSplitMode>(aTabOpt.getToken(2,cTabSep).toInt32());
            maTabData[nPos]->eVSplitMode = static_cast<ScSplitMode>(aTabOpt.getToken(3,cTabSep).toInt32());

            if ( maTabData[nPos]->eHSplitMode == SC_SPLIT_FIX )
            {
                maTabData[nPos]->nFixPosX = SanitizeCol( static_cast<SCCOL>(aTabOpt.getToken(4,cTabSep).toInt32()));
                UpdateFixX(nPos);
            }
            else
                maTabData[nPos]->nHSplitPos = aTabOpt.getToken(4,cTabSep).toInt32();

            if ( maTabData[nPos]->eVSplitMode == SC_SPLIT_FIX )
            {
                maTabData[nPos]->nFixPosY = SanitizeRow( aTabOpt.getToken(5,cTabSep).toInt32());
                UpdateFixY(nPos);
            }
            else
                maTabData[nPos]->nVSplitPos = aTabOpt.getToken(5,cTabSep).toInt32();

            maTabData[nPos]->eWhichActive = static_cast<ScSplitPos>(aTabOpt.getToken(6,cTabSep).toInt32());
            maTabData[nPos]->nPosX[0] = SanitizeCol( static_cast<SCCOL>(aTabOpt.getToken(7,cTabSep).toInt32()));
            maTabData[nPos]->nPosX[1] = SanitizeCol( static_cast<SCCOL>(aTabOpt.getToken(8,cTabSep).toInt32()));
            maTabData[nPos]->nPosY[0] = SanitizeRow( aTabOpt.getToken(9,cTabSep).toInt32());
            maTabData[nPos]->nPosY[1] = SanitizeRow( aTabOpt.getToken(10,cTabSep).toInt32());

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

    // sheet settings
    for( SCTAB nTab = 0; nTab < static_cast<SCTAB>(maTabData.size()); ++nTab )
    {
        if( const ScViewDataTable* pViewTab = maTabData[ nTab ].get() )
        {
            ScExtTabSettings& rTabSett = rDocOpt.GetOrCreateTabSettings( nTab );

            // split mode
            ScSplitMode eHSplit = pViewTab->eHSplitMode;
            ScSplitMode eVSplit = pViewTab->eVSplitMode;
            bool bHSplit = eHSplit != SC_SPLIT_NONE;
            bool bVSplit = eVSplit != SC_SPLIT_NONE;
            bool bRealSplit = (eHSplit == SC_SPLIT_NORMAL) || (eVSplit == SC_SPLIT_NORMAL);
            bool bFrozen    = (eHSplit == SC_SPLIT_FIX)    || (eVSplit == SC_SPLIT_FIX);
            OSL_ENSURE( !bRealSplit || !bFrozen, "ScViewData::WriteExtOptions - split and freeze in same sheet" );
            rTabSett.mbFrozenPanes = !bRealSplit && bFrozen;

            // split and freeze position
            rTabSett.maSplitPos = Point( 0, 0 );
            rTabSett.maFreezePos.Set( 0, 0, nTab );
            if( bRealSplit )
            {
                Point& rSplitPos = rTabSett.maSplitPos;
                rSplitPos = Point( bHSplit ? pViewTab->nHSplitPos : 0, bVSplit ? pViewTab->nVSplitPos : 0 );
                rSplitPos = Application::GetDefaultDevice()->PixelToLogic( rSplitPos, MapMode( MapUnit::MapTwip ) );
                if( pDocShell )
                    rSplitPos.setX( static_cast<long>(static_cast<double>(rSplitPos.X()) / pDocShell->GetOutputFactor()) );
            }
            else if( bFrozen )
            {
                if( bHSplit ) rTabSett.maFreezePos.SetCol( pViewTab->nFixPosX );
                if( bVSplit ) rTabSett.maFreezePos.SetRow( pViewTab->nFixPosY );
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

            // grid color
            rTabSett.maGridColor = COL_AUTO;
            if( pOptions )
            {
                const Color& rGridColor = pOptions->GetGridColor();
                if( rGridColor != SC_STD_GRIDCOLOR )
                    rTabSett.maGridColor = rGridColor;
            }
            rTabSett.mbShowGrid = pViewTab->bShowGrid;

            // view mode and zoom
            rTabSett.mbPageMode = bPagebreak;
            rTabSett.mnNormalZoom = static_cast< long >( pViewTab->aZoomY * Fraction( 100.0 ) );
            rTabSett.mnPageZoom = static_cast< long >( pViewTab->aPageZoomY * Fraction( 100.0 ) );
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
                maTabData[ nTab ].reset( new ScViewDataTable );

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
                    aPixel.setX( static_cast<long>( aPixel.X() * nFactor + 0.5 ) );
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
                rViewTab.aZoomX = rViewTab.aZoomY = Fraction( rTabSett.mnNormalZoom, 100L );
            if( rTabSett.mnPageZoom )
                rViewTab.aPageZoomX = rViewTab.aPageZoomY = Fraction( rTabSett.mnPageZoom, 100L );

            rViewTab.bShowGrid = rTabSett.mbShowGrid;

            // get some settings from displayed Excel sheet, set at Calc document
            if( nTab == GetTabNo() )
            {
                // grid color -- #i47435# set automatic grid color explicitly
                if( pOptions )
                {
                    Color aGridColor( rTabSett.maGridColor );
                    if( aGridColor == COL_AUTO )
                        aGridColor = SC_STD_GRIDCOLOR;
                    pOptions->SetGridColor( aGridColor, EMPTY_OUSTRING );
                }

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

    // RecalcPixPos or so - also nMPos - also for ReadUserData ??!?!
}

void ScViewData::WriteUserDataSequence(uno::Sequence <beans::PropertyValue>& rSettings) const
{
    rSettings.realloc(SC_VIEWSETTINGS_COUNT);
    // + 1, because we have to put the view id in the sequence
    beans::PropertyValue* pSettings = rSettings.getArray();
    if (pSettings)
    {
        sal_uInt16 nViewID(pViewShell->GetViewFrame()->GetCurViewId());
        pSettings[SC_VIEW_ID].Name = SC_VIEWID;
        pSettings[SC_VIEW_ID].Value <<= SC_VIEW + OUString::number(nViewID);

        uno::Reference<container::XNameContainer> xNameContainer =
             document::NamedPropertyValues::create( comphelper::getProcessComponentContext() );
        for (SCTAB nTab=0; nTab<static_cast<SCTAB>(maTabData.size()); nTab++)
        {
            if (maTabData[nTab])
            {
                uno::Sequence <beans::PropertyValue> aTableViewSettings;
                maTabData[nTab]->WriteUserDataSequence(aTableViewSettings, *this);
                OUString sTabName;
                GetDocument()->GetName( nTab, sTabName );
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
        GetDocument()->GetName( nTabNo, sName );
        pSettings[SC_ACTIVE_TABLE].Name = SC_ACTIVETABLE;
        pSettings[SC_ACTIVE_TABLE].Value <<= sName;
        pSettings[SC_HORIZONTAL_SCROLL_BAR_WIDTH].Name = SC_HORIZONTALSCROLLBARWIDTH;
        pSettings[SC_HORIZONTAL_SCROLL_BAR_WIDTH].Value <<= sal_Int32(pView->GetTabBarWidth());
        sal_Int32 nZoomValue = long(pThisTab->aZoomY * 100);
        sal_Int32 nPageZoomValue = long(pThisTab->aPageZoomY * 100);
        pSettings[SC_ZOOM_TYPE].Name = SC_ZOOMTYPE;
        pSettings[SC_ZOOM_TYPE].Value <<= sal_Int16(pThisTab->eZoomType);
        pSettings[SC_ZOOM_VALUE].Name = SC_ZOOMVALUE;
        pSettings[SC_ZOOM_VALUE].Value <<= nZoomValue;
        pSettings[SC_PAGE_VIEW_ZOOM_VALUE].Name = SC_PAGEVIEWZOOMVALUE;
        pSettings[SC_PAGE_VIEW_ZOOM_VALUE].Value <<= nPageZoomValue;
        pSettings[SC_PAGE_BREAK_PREVIEW].Name = SC_SHOWPAGEBREAKPREVIEW;
        pSettings[SC_PAGE_BREAK_PREVIEW].Value <<= bPagebreak;

        if (pOptions)
        {
            pSettings[SC_SHOWZERO].Name = SC_UNO_SHOWZERO;
            pSettings[SC_SHOWZERO].Value <<= pOptions->GetOption( VOPT_NULLVALS );
            pSettings[SC_SHOWNOTES].Name = SC_UNO_SHOWNOTES;
            pSettings[SC_SHOWNOTES].Value <<= pOptions->GetOption( VOPT_NOTES );
            pSettings[SC_SHOWGRID].Name = SC_UNO_SHOWGRID;
            pSettings[SC_SHOWGRID].Value <<= pOptions->GetOption( VOPT_GRID );
            pSettings[SC_GRIDCOLOR].Name = SC_UNO_GRIDCOLOR;
            OUString aColorName;
            Color aColor = pOptions->GetGridColor(&aColorName);
            pSettings[SC_GRIDCOLOR].Value <<= aColor;
            pSettings[SC_SHOWPAGEBR].Name = SC_UNO_SHOWPAGEBR;
            pSettings[SC_SHOWPAGEBR].Value <<= pOptions->GetOption( VOPT_PAGEBREAKS );
            pSettings[SC_COLROWHDR].Name = SC_UNO_COLROWHDR;
            pSettings[SC_COLROWHDR].Value <<= pOptions->GetOption( VOPT_HEADER );
            pSettings[SC_SHEETTABS].Name = SC_UNO_SHEETTABS;
            pSettings[SC_SHEETTABS].Value <<= pOptions->GetOption( VOPT_TABCONTROLS );
            pSettings[SC_OUTLSYMB].Name = SC_UNO_OUTLSYMB;
            pSettings[SC_OUTLSYMB].Value <<= pOptions->GetOption( VOPT_OUTLINER );
            pSettings[SC_VALUE_HIGHLIGHTING].Name = SC_UNO_VALUEHIGH;
            pSettings[SC_VALUE_HIGHLIGHTING].Value <<= pOptions->GetOption( VOPT_SYNTAX );

            const ScGridOptions& aGridOpt = pOptions->GetGridOptions();
            pSettings[SC_SNAPTORASTER].Name = SC_UNO_SNAPTORASTER;
            pSettings[SC_SNAPTORASTER].Value <<= aGridOpt.GetUseGridSnap();
            pSettings[SC_RASTERVIS].Name = SC_UNO_RASTERVIS;
            pSettings[SC_RASTERVIS].Value <<= aGridOpt.GetGridVisible();
            pSettings[SC_RASTERRESX].Name = SC_UNO_RASTERRESX;
            pSettings[SC_RASTERRESX].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDrawX() );
            pSettings[SC_RASTERRESY].Name = SC_UNO_RASTERRESY;
            pSettings[SC_RASTERRESY].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDrawY() );
            pSettings[SC_RASTERSUBX].Name = SC_UNO_RASTERSUBX;
            pSettings[SC_RASTERSUBX].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDivisionX() );
            pSettings[SC_RASTERSUBY].Name = SC_UNO_RASTERSUBY;
            pSettings[SC_RASTERSUBY].Value <<= static_cast<sal_Int32> ( aGridOpt.GetFieldDivisionY() );
            pSettings[SC_RASTERSYNC].Name = SC_UNO_RASTERSYNC;
            pSettings[SC_RASTERSYNC].Value <<= aGridOpt.GetSynchronize();
        }
    }

    // Common SdrModel processing
    GetDocument()->GetDrawLayer()->WriteUserDataSequence(rSettings);
}

void ScViewData::ReadUserDataSequence(const uno::Sequence <beans::PropertyValue>& rSettings)
{
    std::vector<bool> aHasZoomVect( GetDocument()->GetTableCount(), false );

    sal_Int32 nCount(rSettings.getLength());
    sal_Int32 nTemp32(0);
    sal_Int16 nTemp16(0);
    bool bPageMode(false);

    EnsureTabDataSize(GetDocument()->GetTableCount());

    for (sal_Int32 i = 0; i < nCount; i++)
    {
        // SC_VIEWID has to parse and use by mba
        OUString sName(rSettings[i].Name);
        if (sName == SC_TABLES)
        {
            uno::Reference<container::XNameContainer> xNameContainer;
            if ((rSettings[i].Value >>= xNameContainer) && xNameContainer->hasElements())
            {
                uno::Sequence< OUString > aNames(xNameContainer->getElementNames());
                for (sal_Int32 nTabPos = 0; nTabPos < aNames.getLength(); nTabPos++)
                {
                    OUString sTabName(aNames[nTabPos]);
                    SCTAB nTab(0);
                    if (GetDocument()->GetTable(sTabName, nTab))
                    {
                        uno::Any aAny = xNameContainer->getByName(aNames[nTabPos]);
                        uno::Sequence<beans::PropertyValue> aTabSettings;
                        if (aAny >>= aTabSettings)
                        {
                            EnsureTabDataSize(nTab + 1);
                            if (!maTabData[nTab])
                                maTabData[nTab].reset( new ScViewDataTable );

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
            if(rSettings[i].Value >>= sTabName)
            {
                SCTAB nTab(0);
                if (GetDocument()->GetTable(sTabName, nTab))
                    nTabNo = nTab;
            }
        }
        else if (sName == SC_HORIZONTALSCROLLBARWIDTH)
        {
            if (rSettings[i].Value >>= nTemp32)
                pView->SetTabBarWidth(nTemp32);
        }
        else if (sName == SC_RELHORIZONTALTABBARWIDTH)
        {
            double fWidth = 0.0;
            if (rSettings[i].Value >>= fWidth)
                pView->SetPendingRelTabBarWidth( fWidth );
        }
        else if (sName == SC_ZOOMTYPE)
        {
            if (rSettings[i].Value >>= nTemp16)
                eDefZoomType = SvxZoomType(nTemp16);
        }
        else if (sName == SC_ZOOMVALUE)
        {
            if (rSettings[i].Value >>= nTemp32)
            {
                Fraction aZoom(nTemp32, 100);
                aDefZoomX = aDefZoomY = aZoom;
            }
        }
        else if (sName == SC_PAGEVIEWZOOMVALUE)
        {
            if (rSettings[i].Value >>= nTemp32)
            {
                Fraction aZoom(nTemp32, 100);
                aDefPageZoomX = aDefPageZoomY = aZoom;
            }
        }
        else if (sName == SC_SHOWPAGEBREAKPREVIEW)
            bPageMode = ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value );
        else if ( sName == SC_UNO_SHOWZERO )
            pOptions->SetOption(VOPT_NULLVALS, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_SHOWNOTES )
            pOptions->SetOption(VOPT_NOTES, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_SHOWGRID )
            pOptions->SetOption(VOPT_GRID, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_GRIDCOLOR )
        {
            Color aColor;
            if (rSettings[i].Value >>= aColor)
            {
                // #i47435# set automatic grid color explicitly
                if( aColor == COL_AUTO )
                    aColor = SC_STD_GRIDCOLOR;
                pOptions->SetGridColor(aColor, OUString());
            }
        }
        else if ( sName == SC_UNO_SHOWPAGEBR )
            pOptions->SetOption(VOPT_PAGEBREAKS, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_COLROWHDR )
            pOptions->SetOption(VOPT_HEADER, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_SHEETTABS )
            pOptions->SetOption(VOPT_TABCONTROLS, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_OUTLSYMB )
            pOptions->SetOption(VOPT_OUTLINER, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else if ( sName == SC_UNO_SHOWOBJ )
        {
            // #i80528# placeholders not supported anymore
            if ( rSettings[i].Value >>= nTemp16 )
                pOptions->SetObjMode( VOBJ_TYPE_OLE, (nTemp16 == 1) ? VOBJ_MODE_HIDE : VOBJ_MODE_SHOW );
        }
        else if ( sName == SC_UNO_SHOWCHARTS )
        {
            // #i80528# placeholders not supported anymore
            if ( rSettings[i].Value >>= nTemp16 )
                pOptions->SetObjMode( VOBJ_TYPE_CHART, (nTemp16 == 1) ? VOBJ_MODE_HIDE : VOBJ_MODE_SHOW );
        }
        else if ( sName == SC_UNO_SHOWDRAW )
        {
            // #i80528# placeholders not supported anymore
            if ( rSettings[i].Value >>= nTemp16 )
                pOptions->SetObjMode( VOBJ_TYPE_DRAW, (nTemp16 == 1) ? VOBJ_MODE_HIDE : VOBJ_MODE_SHOW );
        }
        else if ( sName.compareToAscii( SC_UNO_VALUEHIGH ) == 0 )
            pOptions->SetOption( VOPT_SYNTAX, ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
        else
        {
            ScGridOptions aGridOpt(pOptions->GetGridOptions());
            if ( sName == SC_UNO_SNAPTORASTER )
                aGridOpt.SetUseGridSnap( ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
            else if ( sName == SC_UNO_RASTERVIS )
                aGridOpt.SetGridVisible( ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
            else if ( sName == SC_UNO_RASTERRESX )
                aGridOpt.SetFieldDrawX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSettings[i].Value ) ) );
            else if ( sName == SC_UNO_RASTERRESY )
                aGridOpt.SetFieldDrawY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSettings[i].Value ) ) );
            else if ( sName == SC_UNO_RASTERSUBX )
                aGridOpt.SetFieldDivisionX( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSettings[i].Value ) ) );
            else if ( sName == SC_UNO_RASTERSUBY )
                aGridOpt.SetFieldDivisionY( static_cast <sal_uInt32> ( ScUnoHelpFunctions::GetInt32FromAny( rSettings[i].Value ) ) );
            else if ( sName == SC_UNO_RASTERSYNC )
                aGridOpt.SetSynchronize( ScUnoHelpFunctions::GetBoolFromAny( rSettings[i].Value ) );
            // Fallback to common SdrModel processing
            else GetDocument()->GetDrawLayer()->ReadUserDataSequenceValue(&rSettings[i]);

            pOptions->SetGridOptions(aGridOpt);
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

    if (nCount)
        SetPagebreakMode( bPageMode );

    // #i47426# write view options to document, needed e.g. for Excel export
    pDoc->SetViewOptions( *pOptions );
}

void ScViewData::SetOptions( const ScViewOptions& rOpt )
{
    //  if visibility of horizontal ScrollBar is changed, TabBar may have to be resized...
    bool bHScrollChanged = ( rOpt.GetOption(VOPT_HSCROLL) != pOptions->GetOption(VOPT_HSCROLL) );

    //  if graphics are turned on or off, animation has to be started or stopped
    //  graphics are controlled by VOBJ_TYPE_OLE
    bool bGraphicsChanged = ( pOptions->GetObjMode(VOBJ_TYPE_OLE) !=
                                   rOpt.GetObjMode(VOBJ_TYPE_OLE) );

    *pOptions = rOpt;
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
    if (pViewShell)
        pViewShell->UpdateInputHandler( bForce );
}

bool ScViewData::IsOle()
{
    return pDocShell && pDocShell->IsOle();
}

bool ScViewData::UpdateFixX( SCTAB nTab )                   // true = value changed
{
    if (!ValidTab(nTab))        // Default
        nTab=nTabNo;            // current table

    if (!pView || maTabData[nTab]->eHSplitMode != SC_SPLIT_FIX)
        return false;

    ScDocument* pLocalDoc = GetDocument();
    if (!pLocalDoc->HasTable(nTab))          // if called from reload, the sheet may not exist
        return false;

    SCCOL nFix = maTabData[nTab]->nFixPosX;
    long nNewPos = 0;
    for (SCCOL nX=maTabData[nTab]->nPosX[SC_SPLIT_LEFT]; nX<nFix; nX++)
    {
        sal_uInt16 nTSize = pLocalDoc->GetColWidth( nX, nTab );
        if (nTSize)
        {
            long nPix = ToPixel( nTSize, nPPTX );
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

    ScDocument* pLocalDoc = GetDocument();
    if (!pLocalDoc->HasTable(nTab))          // if called from reload, the sheet may not exist
        return false;

    SCROW nFix = maTabData[nTab]->nFixPosY;
    long nNewPos = 0;
    for (SCROW nY=maTabData[nTab]->nPosY[SC_SPLIT_TOP]; nY<nFix; nY++)
    {
        sal_uInt16 nTSize = pLocalDoc->GetRowHeight( nY, nTab );
        if (nTSize)
        {
            long nPix = ToPixel( nTSize, nPPTY );
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
    ScDocument* pLocalDoc = GetDocument();
    bool bOnlineSpell = pLocalDoc->GetDocOptions().IsAutoSpell();

    EEControlBits nCntrl = rOutl.GetControlWord();
    nCntrl |= EEControlBits::MARKFIELDS;
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
        pLocalDoc->GetEditTextDirection( nTabNo ) );
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

void ScViewData::AddPixelsWhile( long & rScrY, long nEndPixels, SCROW & rPosY,
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
            nRow = nHeightEndRow + 1;
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
            rScrY += static_cast<long>(nAdd);
            nRow += nRows;
        }
    }
    if (nRow > rPosY)
        --nRow;
    rPosY = nRow;
}

void ScViewData::AddPixelsWhileBackward( long & rScrY, long nEndPixels,
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
            rScrY += static_cast<long>(nAdd);
            nRow -= nRows;
        }
    }
    if (nRow < rPosY)
        ++nRow;
    rPosY = nRow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
