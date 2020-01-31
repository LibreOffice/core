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

#include <basic/sbxvar.hxx>

#include <svx/svxids.hrc>
#include <editeng/svxenum.hxx>

#include <viewopt.hxx>
#include <swtypes.hxx>
#include <cmdid.h>
#include <prtopt.hxx>
#include <cfgitems.hxx>
#include <crstate.hxx>


SwDocDisplayItem::SwDocDisplayItem() :
        SfxPoolItem(FN_PARAM_DOCDISP)
{
    bParagraphEnd       =
    bTab                =
    bSpace              =
    bNonbreakingSpace   =
    bSoftHyphen         =
    bCharHiddenText     =
    bBookmarks          =
    bManualBreak        = true;
};

// Item for the Settings dialog, page document view
SwDocDisplayItem::SwDocDisplayItem(const SwViewOption& rVOpt ) :
            SfxPoolItem( FN_PARAM_DOCDISP )
{
    bParagraphEnd       = rVOpt.IsParagraph(true);
    bTab                = rVOpt.IsTab(true);
    bSpace              = rVOpt.IsBlank(true);
    bNonbreakingSpace   = rVOpt.IsHardBlank();
    bSoftHyphen         = rVOpt.IsSoftHyph();
    bCharHiddenText     = rVOpt.IsShowHiddenChar(true);
    bBookmarks          = rVOpt.IsShowBookmarks(true);
    bManualBreak        = rVOpt.IsLineBreak(true);
}

SfxPoolItem* SwDocDisplayItem::Clone( SfxItemPool*  ) const
{
    return new SwDocDisplayItem( *this );
}

bool SwDocDisplayItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwDocDisplayItem& rItem = static_cast<const SwDocDisplayItem&>(rAttr);

    return (  bParagraphEnd         == rItem.bParagraphEnd       &&
              bTab                  == rItem.bTab                &&
              bSpace                == rItem.bSpace              &&
              bNonbreakingSpace     == rItem.bNonbreakingSpace   &&
              bSoftHyphen           == rItem.bSoftHyphen         &&
              bCharHiddenText       == rItem.bCharHiddenText         &&
              bBookmarks            == rItem.bBookmarks          &&
              bManualBreak          == rItem.bManualBreak );
}

void SwDocDisplayItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetParagraph  (bParagraphEnd      );
    rVOpt.SetTab        (bTab               );
    rVOpt.SetBlank      (bSpace             );
    rVOpt.SetHardBlank  (bNonbreakingSpace  );
    rVOpt.SetSoftHyph   (bSoftHyphen        );
    rVOpt.SetShowHiddenChar(bCharHiddenText );
    rVOpt.SetShowBookmarks(bBookmarks       );
    rVOpt.SetLineBreak  (bManualBreak       );
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
    m_bFieldName          =
    m_bNotes              = false;
    m_bShowInlineTooltips = true;
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
    m_bFieldName          = rVOpt.IsFieldName();
    m_bNotes              = rVOpt.IsPostIts();
    m_bShowInlineTooltips = rVOpt.IsShowInlineTooltips();
    m_bFieldHiddenText = rVOpt.IsShowHiddenField();
    m_bShowHiddenPara  = rVOpt.IsShowHiddenPara();
}

SfxPoolItem* SwElemItem::Clone( SfxItemPool* ) const
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
                m_bFieldName            == rItem.m_bFieldName          &&
                m_bNotes                == rItem.m_bNotes              &&
                m_bShowInlineTooltips   == rItem.m_bShowInlineTooltips &&
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
    rVOpt.SetFieldName  (m_bFieldName         );
    rVOpt.SetPostIts    (m_bNotes             );
    rVOpt.SetShowInlineTooltips( m_bShowInlineTooltips );
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

SfxPoolItem* SwAddPrinterItem::Clone( SfxItemPool* ) const
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
    eMode( FILL_TAB )
    ,bOn( false )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwViewOption& rVOpt )
    : SfxPoolItem( FN_PARAM_SHADOWCURSOR ),
    eMode( rVOpt.GetShdwCursorFillMode() )
    ,bOn( rVOpt.IsShadowCursor() )

{
}

SfxPoolItem* SwShadowCursorItem::Clone( SfxItemPool* ) const
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
    rVOpt.SetShadowCursor( bOn );
    rVOpt.SetShdwCursorFillMode( eMode );
}

#ifdef DBG_UTIL
SfxPoolItem* SwTestItem::Clone( SfxItemPool* ) const
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
