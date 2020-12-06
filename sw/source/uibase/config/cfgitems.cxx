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

#include <viewopt.hxx>
#include <cmdid.h>
#include <cfgitems.hxx>
#include <crstate.hxx>


SwDocDisplayItem::SwDocDisplayItem() :
        SfxPoolItem(FN_PARAM_DOCDISP)
{
    m_bParagraphEnd       =
    m_bTab                =
    m_bSpace              =
    m_bNonbreakingSpace   =
    m_bSoftHyphen         =
    m_bCharHiddenText     =
    m_bBookmarks          =
    m_bManualBreak        = true;
    m_xDefaultAnchor      = 1; //FLY_TO_CHAR
};

// Item for the Settings dialog, page document view
SwDocDisplayItem::SwDocDisplayItem(const SwViewOption& rVOpt ) :
            SfxPoolItem( FN_PARAM_DOCDISP )
{
    m_bParagraphEnd       = rVOpt.IsParagraph(true);
    m_bTab                = rVOpt.IsTab(true);
    m_bSpace              = rVOpt.IsBlank(true);
    m_bNonbreakingSpace   = rVOpt.IsHardBlank();
    m_bSoftHyphen         = rVOpt.IsSoftHyph();
    m_bCharHiddenText     = rVOpt.IsShowHiddenChar(true);
    m_bBookmarks          = rVOpt.IsShowBookmarks(true);
    m_bManualBreak        = rVOpt.IsLineBreak(true);
    m_xDefaultAnchor      = rVOpt.GetDefaultAnchor();
}

SwDocDisplayItem* SwDocDisplayItem::Clone( SfxItemPool*  ) const
{
    return new SwDocDisplayItem( *this );
}

bool SwDocDisplayItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwDocDisplayItem& rItem = static_cast<const SwDocDisplayItem&>(rAttr);

    return (  m_bParagraphEnd         == rItem.m_bParagraphEnd       &&
              m_bTab                  == rItem.m_bTab                &&
              m_bSpace                == rItem.m_bSpace              &&
              m_bNonbreakingSpace     == rItem.m_bNonbreakingSpace   &&
              m_bSoftHyphen           == rItem.m_bSoftHyphen         &&
              m_bCharHiddenText       == rItem.m_bCharHiddenText     &&
              m_bBookmarks            == rItem.m_bBookmarks          &&
              m_bManualBreak          == rItem.m_bManualBreak        &&
              m_xDefaultAnchor        == rItem.m_xDefaultAnchor);
}

void SwDocDisplayItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetParagraph  (m_bParagraphEnd      );
    rVOpt.SetTab        (m_bTab               );
    rVOpt.SetBlank      (m_bSpace             );
    rVOpt.SetHardBlank  (m_bNonbreakingSpace  );
    rVOpt.SetSoftHyph   (m_bSoftHyphen        );
    rVOpt.SetShowHiddenChar(m_bCharHiddenText );
    rVOpt.SetShowBookmarks(m_bBookmarks       );
    rVOpt.SetLineBreak  (m_bManualBreak       );
    rVOpt.SetDefaultAnchor( m_xDefaultAnchor  );
}

SwElemItem::SwElemItem() :
    SfxPoolItem(FN_PARAM_ELEM)
{
    m_bVertRuler     =
    m_bVertRulerRight=
    m_bCrosshair     =
    m_bSmoothScroll  =
    m_bTable              =
    m_bGraphic            =
    m_bDrawing            =
    m_bNotes              = false;
    m_bShowInlineTooltips = true;
    m_bShowOutlineContentVisibilityButton =
    m_bTreatSubOutlineLevelsAsContent =
    m_bShowChangesInMargin =
    m_bFieldHiddenText =
    m_bShowHiddenPara  = false;
}

SwElemItem::SwElemItem(const SwViewOption& rVOpt) :
            SfxPoolItem( FN_PARAM_ELEM )
{
    m_bVertRuler      = rVOpt.IsViewVRuler(true);
    m_bVertRulerRight = rVOpt.IsVRulerRight();
    m_bCrosshair      = rVOpt.IsCrossHair();
    m_bSmoothScroll   = rVOpt.IsSmoothScroll();
    m_bTable              = rVOpt.IsTable();
    m_bGraphic            = rVOpt.IsGraphic();
    m_bDrawing            = rVOpt.IsDraw() && rVOpt.IsControl();
    m_bNotes              = rVOpt.IsPostIts();
    m_bShowInlineTooltips = rVOpt.IsShowInlineTooltips();
    m_bShowOutlineContentVisibilityButton = rVOpt.IsShowOutlineContentVisibilityButton();
    m_bTreatSubOutlineLevelsAsContent = rVOpt.IsTreatSubOutlineLevelsAsContent();
    m_bShowChangesInMargin = rVOpt.IsShowChangesInMargin();
    m_bFieldHiddenText = rVOpt.IsShowHiddenField();
    m_bShowHiddenPara  = rVOpt.IsShowHiddenPara();
}

SwElemItem* SwElemItem::Clone( SfxItemPool* ) const
{
    return new SwElemItem( *this );
}

bool SwElemItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwElemItem& rItem = static_cast<const SwElemItem&>(rAttr);

    return (    m_bVertRuler      == rItem.m_bVertRuler     &&
                m_bVertRulerRight == rItem.m_bVertRulerRight&&
                m_bCrosshair      == rItem.m_bCrosshair     &&
                m_bSmoothScroll   == rItem.m_bSmoothScroll  &&
                m_bTable                == rItem.m_bTable              &&
                m_bGraphic              == rItem.m_bGraphic            &&
                m_bDrawing              == rItem.m_bDrawing            &&
                m_bNotes                == rItem.m_bNotes              &&
                m_bShowInlineTooltips   == rItem.m_bShowInlineTooltips &&
                m_bShowOutlineContentVisibilityButton == rItem.m_bShowOutlineContentVisibilityButton &&
                m_bTreatSubOutlineLevelsAsContent == rItem.m_bTreatSubOutlineLevelsAsContent &&
                m_bShowChangesInMargin  == rItem.m_bShowChangesInMargin &&
                m_bFieldHiddenText == rItem.m_bFieldHiddenText &&
                m_bShowHiddenPara  == rItem.m_bShowHiddenPara);
}

void SwElemItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetViewVRuler(m_bVertRuler    );
    rVOpt.SetVRulerRight(m_bVertRulerRight );
    rVOpt.SetCrossHair(m_bCrosshair     );
    rVOpt.SetSmoothScroll(m_bSmoothScroll);
    rVOpt.SetTable      (m_bTable             );
    rVOpt.SetGraphic    (m_bGraphic           );
    rVOpt.SetDraw       (m_bDrawing           );
    rVOpt.SetControl    (m_bDrawing           );
    rVOpt.SetPostIts    (m_bNotes             );
    rVOpt.SetShowInlineTooltips( m_bShowInlineTooltips );
    rVOpt.SetShowOutlineContentVisibilityButton(m_bShowOutlineContentVisibilityButton);
    rVOpt.SetTreatSubOutlineLevelsAsContent(m_bTreatSubOutlineLevelsAsContent);
    rVOpt.SetShowChangesInMargin( m_bShowChangesInMargin );
    rVOpt.SetShowHiddenField(m_bFieldHiddenText );
    rVOpt.SetShowHiddenPara(m_bShowHiddenPara );
}

// CTOR for empty Item
SwAddPrinterItem::SwAddPrinterItem():
                SfxPoolItem(FN_PARAM_ADDPRINTER)
{
}

// CTOR from SwPrintOptions
SwAddPrinterItem::SwAddPrinterItem( const SwPrintData& rPrtData ) :
    SfxPoolItem(FN_PARAM_ADDPRINTER)
{
    SwPrintData::operator=(rPrtData);
}

SwAddPrinterItem* SwAddPrinterItem::Clone( SfxItemPool* ) const
{
    return new SwAddPrinterItem( *this );
}

bool SwAddPrinterItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwAddPrinterItem& rItem = static_cast<const SwAddPrinterItem&>(rAttr);

    return  SwPrintData::operator==(rItem);
}

// Item for Settings dialog, ShadowCursorPage
SwShadowCursorItem::SwShadowCursorItem()
    : SfxPoolItem( FN_PARAM_SHADOWCURSOR ),
    m_eMode( SwFillMode::Tab )
    ,m_bOn( false )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwViewOption& rVOpt )
    : SfxPoolItem( FN_PARAM_SHADOWCURSOR ),
    m_eMode( rVOpt.GetShdwCursorFillMode() )
    ,m_bOn( rVOpt.IsShadowCursor() )

{
}

SwShadowCursorItem* SwShadowCursorItem::Clone( SfxItemPool* ) const
{
    return new SwShadowCursorItem( *this );
}

bool SwShadowCursorItem::operator==( const SfxPoolItem& rCmp ) const
{
    return SfxPoolItem::operator==(rCmp) &&
        IsOn() == static_cast<const SwShadowCursorItem&>(rCmp).IsOn() &&
        GetMode() == static_cast<const SwShadowCursorItem&>(rCmp).GetMode();
}

void SwShadowCursorItem::FillViewOptions( SwViewOption& rVOpt ) const
{
    rVOpt.SetShadowCursor( m_bOn );
    rVOpt.SetShdwCursorFillMode( m_eMode );
}

#ifdef DBG_UTIL
SwTestItem* SwTestItem::Clone( SfxItemPool* ) const
{
    return new SwTestItem( *this );
}

bool SwTestItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwTestItem& rItem = static_cast<const SwTestItem&>( rAttr);

    return ( m_bTest1==rItem.m_bTest1&&
             m_bTest2==rItem.m_bTest2&&
             m_bTest3==rItem.m_bTest3&&
             m_bTest4==rItem.m_bTest4&&
             m_bTest5==rItem.m_bTest5&&
             m_bTest6==rItem.m_bTest6&&
             m_bTest7==rItem.m_bTest7&&
             m_bTest8==rItem.m_bTest8&&
             m_bTest9==rItem.m_bTest9&&
             m_bTest10==rItem.m_bTest10);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
