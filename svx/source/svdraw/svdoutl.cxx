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

#include <optional>
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdpage.hxx>
#include <editeng/editstat.hxx>
#include <svl/itempool.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>


SdrOutliner::SdrOutliner( SfxItemPool* pItemPool, OutlinerMode nMode )
:   Outliner( pItemPool, nMode ),
    mpVisualizedPage(nullptr)
{
}


SdrOutliner::~SdrOutliner()
{
}


void SdrOutliner::SetTextObj( const SdrTextObj* pObj )
{
    if( pObj && pObj != mxWeakTextObj.get().get() )
    {
        SetUpdateLayout(false);
        OutlinerMode nOutlinerMode2 = OutlinerMode::OutlineObject;
        if ( !pObj->IsOutlText() )
            nOutlinerMode2 = OutlinerMode::TextObject;
        Init( nOutlinerMode2 );

        resetScalingParameters();

        EEControlBits nStat = GetControlWord();
        nStat &= ~EEControlBits( EEControlBits::STRETCHING | EEControlBits::AUTOPAGESIZE );
        SetControlWord(nStat);

        Size aMaxSize( 100000,100000 );
        SetMinAutoPaperSize( Size() );
        SetMaxAutoPaperSize( aMaxSize );
        SetPaperSize( aMaxSize );
        SetTextColumns(pObj->GetTextColumnsNumber(), pObj->GetTextColumnsSpacing());
        ClearPolygon();

        if (pObj->GetTextEditOutliner())
            SetBackgroundColor(pObj->GetTextEditOutliner()->GetEditEngine().GetBackgroundColor());
    }

    mxWeakTextObj = const_cast< SdrTextObj* >(pObj);
}

void SdrOutliner::SetTextObjNoInit( const SdrTextObj* pObj )
{
    mxWeakTextObj = const_cast< SdrTextObj* >(pObj);
}

OUString SdrOutliner::CalcFieldValue(const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos,
                                     std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor,
                                     std::optional<FontLineStyle>& rpFldLineStyle)
{
    bool bOk = false;
    OUString aRet;

    if(auto pTextObj = mxWeakTextObj.get())
        bOk = pTextObj->CalcFieldValue(rField, nPara, nPos, false, rpTxtColor, rpFldColor, rpFldLineStyle, aRet);

    if (!bOk)
        aRet = Outliner::CalcFieldValue(rField, nPara, nPos, rpTxtColor, rpFldColor, rpFldLineStyle);

    return aRet;
}

const SdrTextObj* SdrOutliner::GetTextObj() const
{
    return mxWeakTextObj.get().get();
}

bool SdrOutliner::hasEditViewCallbacks() const
{
    for (size_t a(0); a < GetViewCount(); a++)
    {
        OutlinerView* pOutlinerView = GetView(a);

        if (pOutlinerView && pOutlinerView->GetEditView().getEditViewCallbacks())
        {
            return true;
        }
    }

    return false;
}

std::optional<bool> SdrOutliner::GetCompatFlag(SdrCompatibilityFlag eFlag) const
{
    if( mpVisualizedPage )
    {
        return {mpVisualizedPage->getSdrModelFromSdrPage().GetCompatibilityFlag(eFlag)};
    }
    return {};
}

void TextHierarchyBreakupBlockText::processDrawPortionInfo(const DrawPortionInfo& rDrawPortionInfo)
{
    // Is clipping wanted? This is text clipping; only accept a portion
    // if it's completely in the range
    if(!mrClipRange.isEmpty())
    {
        // Test start position first; this allows to not get the text range at
        // all if text is far outside
        const basegfx::B2DPoint aStartPosition(rDrawPortionInfo.mrStartPos.X(), rDrawPortionInfo.mrStartPos.Y());

        if(!mrClipRange.isInside(aStartPosition))
        {
            return;
        }

        // Start position is inside. Get TextBoundRect and TopLeft next
        drawinglayer::primitive2d::TextLayouterDevice aTextLayouterDevice;
        aTextLayouterDevice.setFont(rDrawPortionInfo.mrFont);

        const basegfx::B2DRange aTextBoundRect(
            aTextLayouterDevice.getTextBoundRect(
                rDrawPortionInfo.maText, rDrawPortionInfo.mnTextStart, rDrawPortionInfo.mnTextLen));
        const basegfx::B2DPoint aTopLeft(aTextBoundRect.getMinimum() + aStartPosition);

        if(!mrClipRange.isInside(aTopLeft))
        {
            return;
        }

        // TopLeft is inside. Get BottomRight and check
        const basegfx::B2DPoint aBottomRight(aTextBoundRect.getMaximum() + aStartPosition);

        if(!mrClipRange.isInside(aBottomRight))
        {
            return;
        }

        // all inside, clip was successful
    }

    TextHierarchyBreakupOutliner::processDrawPortionInfo(rDrawPortionInfo);
}

TextHierarchyBreakupBlockText::TextHierarchyBreakupBlockText(
    SdrOutliner& rOutliner,
    const basegfx::B2DHomMatrix& rNewTransformA,
    const basegfx::B2DHomMatrix& rNewTransformB,
    const basegfx::B2DRange& rClipRange)
: TextHierarchyBreakupOutliner(
    rOutliner,
    rNewTransformA,
    rNewTransformB)
, mrClipRange(rClipRange)
{
}

void TextHierarchyBreakupContourText::processDrawPortionInfo(const DrawPortionInfo& rDrawPortionInfo)
    {
        // for contour text, ignore (clip away) all portions which are below
        // the visible area given by maScale
        if(static_cast<double>(rDrawPortionInfo.mrStartPos.Y()) < maScale.getY())
        {
            TextHierarchyBreakupOutliner::processDrawPortionInfo(rDrawPortionInfo);
        }
    }

TextHierarchyBreakupContourText::TextHierarchyBreakupContourText(
    SdrOutliner& rOutliner,
    const basegfx::B2DHomMatrix& rNewTransformA,
    const basegfx::B2DHomMatrix& rNewTransformB,
    const basegfx::B2DVector& rScale)
: TextHierarchyBreakupOutliner(
    rOutliner,
    rNewTransformA,
    rNewTransformB)
, maScale(rScale)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
