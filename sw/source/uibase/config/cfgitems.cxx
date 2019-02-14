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
    rVOpt.SetLineBreak  (bManualBreak       );
}

SwElemItem::SwElemItem() :
    SfxPoolItem(FN_PARAM_ELEM)
{
    bVertRuler     =
    bVertRulerRight=
    bCrosshair     =
    bSmoothScroll  =
    bTable              =
    bGraphic            =
    bDrawing            =
    bFieldName          =
    bNotes              = false;
    bShowInlineTooltips = true;
    bFieldHiddenText =
    bShowHiddenPara  = false;
}

SwElemItem::SwElemItem(const SwViewOption& rVOpt) :
            SfxPoolItem( FN_PARAM_ELEM )
{
    bVertRuler      = rVOpt.IsViewVRuler(true);
    bVertRulerRight = rVOpt.IsVRulerRight();
    bCrosshair      = rVOpt.IsCrossHair();
    bSmoothScroll   = rVOpt.IsSmoothScroll();
    bTable              = rVOpt.IsTable();
    bGraphic            = rVOpt.IsGraphic();
    bDrawing            = rVOpt.IsDraw() && rVOpt.IsControl();
    bFieldName          = rVOpt.IsFieldName();
    bNotes              = rVOpt.IsPostIts();
    bShowInlineTooltips = rVOpt.IsShowInlineTooltips();
    bFieldHiddenText = rVOpt.IsShowHiddenField();
    bShowHiddenPara  = rVOpt.IsShowHiddenPara();
}

SfxPoolItem* SwElemItem::Clone( SfxItemPool* ) const
{
    return new SwElemItem( *this );
}

bool SwElemItem::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));

    const SwElemItem& rItem = static_cast<const SwElemItem&>(rAttr);

    return (    bVertRuler      == rItem.bVertRuler     &&
                bVertRulerRight == rItem.bVertRulerRight&&
                bCrosshair      == rItem.bCrosshair     &&
                bSmoothScroll   == rItem.bSmoothScroll  &&
                bTable                == rItem.bTable              &&
                bGraphic              == rItem.bGraphic            &&
                bDrawing              == rItem.bDrawing            &&
                bFieldName            == rItem.bFieldName          &&
                bNotes                == rItem.bNotes              &&
                bShowInlineTooltips   == rItem.bShowInlineTooltips &&
                bFieldHiddenText == rItem.bFieldHiddenText &&
                bShowHiddenPara  == rItem.bShowHiddenPara);
}

void SwElemItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetViewVRuler(bVertRuler    );
    rVOpt.SetVRulerRight(bVertRulerRight );
    rVOpt.SetCrossHair(bCrosshair     );
    rVOpt.SetSmoothScroll(bSmoothScroll);
    rVOpt.SetTable      (bTable             );
    rVOpt.SetGraphic    (bGraphic           );
    rVOpt.SetDraw       (bDrawing           );
    rVOpt.SetControl    (bDrawing           );
    rVOpt.SetFieldName  (bFieldName         );
    rVOpt.SetPostIts    (bNotes             );
    rVOpt.SetShowInlineTooltips( bShowInlineTooltips );
    rVOpt.SetShowHiddenField(bFieldHiddenText );
    rVOpt.SetShowHiddenPara(bShowHiddenPara );
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
    return  IsOn() == static_cast<const SwShadowCursorItem&>(rCmp).IsOn() &&
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

    return ( bTest1==rItem.bTest1&&
             bTest2==rItem.bTest2&&
             bTest3==rItem.bTest3&&
             bTest4==rItem.bTest4&&
             bTest5==rItem.bTest5&&
             bTest6==rItem.bTest6&&
             bTest7==rItem.bTest7&&
             bTest8==rItem.bTest8&&
             bTest9==rItem.bTest9&&
             bTest10==rItem.bTest10);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
