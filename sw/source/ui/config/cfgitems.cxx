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

#include "viewopt.hxx"
#include "swtypes.hxx"
#include "cmdid.h"
#include "prtopt.hxx"
#include "cfgitems.hxx"
#include "crstate.hxx"

TYPEINIT1_AUTOFACTORY(SwDocDisplayItem, SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SwElemItem, SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SwAddPrinterItem, SfxPoolItem)
TYPEINIT1_AUTOFACTORY(SwShadowCursorItem, SfxPoolItem)

SwDocDisplayItem::SwDocDisplayItem( sal_uInt16 _nWhich ) :
        SfxPoolItem(_nWhich),
        aIndexBackgrndCol(COL_GRAY)
{
    bParagraphEnd       =
    bTab                =
    bSpace              =
    bNonbreakingSpace   =
    bSoftHyphen         =
    bFldHiddenText      =
    bCharHiddenText     =
    bManualBreak        =
    bShowHiddenPara     = sal_False;

};

/*-------------------------------------------------------------------
    Item for the Settings dialog, page document view
--------------------------------------------------------------------*/
SwDocDisplayItem::SwDocDisplayItem( const SwDocDisplayItem& rDocDisplayItem ):
            SfxPoolItem(rDocDisplayItem)
{
    *this = rDocDisplayItem;
};

SwDocDisplayItem::SwDocDisplayItem(const SwViewOption& rVOpt, sal_uInt16 _nWhich ) :
            SfxPoolItem( _nWhich )
{
    bParagraphEnd       = rVOpt.IsParagraph(sal_True);
    bTab                = rVOpt.IsTab(sal_True);
    bSpace              = rVOpt.IsBlank(sal_True);
    bNonbreakingSpace   = rVOpt.IsHardBlank();
    bSoftHyphen         = rVOpt.IsSoftHyph();
    bCharHiddenText     = rVOpt.IsShowHiddenChar(sal_True);
    bFldHiddenText      = rVOpt.IsShowHiddenField();
    bManualBreak        = rVOpt.IsLineBreak(sal_True);
    bShowHiddenPara     = rVOpt.IsShowHiddenPara();
}

SfxPoolItem* SwDocDisplayItem::Clone( SfxItemPool*  ) const
{
    return new SwDocDisplayItem( *this );
}

bool SwDocDisplayItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "different types" );

    const SwDocDisplayItem& rItem = (SwDocDisplayItem&)rAttr;

    return (  bParagraphEnd         == rItem.bParagraphEnd       &&
              bTab                  == rItem.bTab                &&
              bSpace                == rItem.bSpace              &&
              bNonbreakingSpace     == rItem.bNonbreakingSpace   &&
              bSoftHyphen           == rItem.bSoftHyphen         &&
              bCharHiddenText       == rItem.bCharHiddenText         &&
              bFldHiddenText        == rItem.bFldHiddenText         &&
              bManualBreak          == rItem.bManualBreak        &&
              bShowHiddenPara       == rItem.bShowHiddenPara );
}

void  SwDocDisplayItem::operator=( const SwDocDisplayItem& rDocDisplayItem)
{
    bParagraphEnd       = rDocDisplayItem.bParagraphEnd         ;
    bTab                = rDocDisplayItem.bTab                  ;
    bSpace              = rDocDisplayItem.bSpace                ;
    bNonbreakingSpace   = rDocDisplayItem.bNonbreakingSpace     ;
    bSoftHyphen         = rDocDisplayItem.bSoftHyphen           ;
    bCharHiddenText     = rDocDisplayItem.bCharHiddenText           ;
    bFldHiddenText      = rDocDisplayItem.bFldHiddenText           ;
    bManualBreak        = rDocDisplayItem.bManualBreak          ;
    bShowHiddenPara     = rDocDisplayItem.bShowHiddenPara       ;
}

void SwDocDisplayItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetParagraph  (bParagraphEnd      );
    rVOpt.SetTab        (bTab               );
    rVOpt.SetBlank      (bSpace             );
    rVOpt.SetHardBlank  (bNonbreakingSpace  );
    rVOpt.SetSoftHyph   (bSoftHyphen        );
    rVOpt.SetShowHiddenChar(bCharHiddenText );
    rVOpt.SetShowHiddenField(bFldHiddenText        );
    rVOpt.SetLineBreak  (bManualBreak       );
    rVOpt.SetShowHiddenPara(bShowHiddenPara );
}

SwElemItem::SwElemItem( sal_uInt16 _nWhich ) :
    SfxPoolItem(_nWhich)
{
    bHorzScrollbar =
    bVertScrollbar =
    bAnyRuler =
    bHorzRuler     =
    bVertRuler     =
    bVertRulerRight=
    bCrosshair     =
    bSmoothScroll  =
    bTable              =
    bGraphic            =
    bDrawing            =
    bFieldName          =
    bNotes              = false;
}

SwElemItem::SwElemItem( const SwElemItem& rElemItem ):
            SfxPoolItem(rElemItem)
{
    *this = rElemItem;
}

SwElemItem::SwElemItem(const SwViewOption& rVOpt, sal_uInt16 _nWhich) :
            SfxPoolItem( _nWhich )
{
    bHorzScrollbar  = rVOpt.IsViewHScrollBar();
    bVertScrollbar  = rVOpt.IsViewVScrollBar();
    bAnyRuler       = rVOpt.IsViewAnyRuler();
    bHorzRuler      = rVOpt.IsViewHRuler(sal_True);
    bVertRuler      = rVOpt.IsViewVRuler(sal_True);
    bVertRulerRight = rVOpt.IsVRulerRight();
    bCrosshair      = rVOpt.IsCrossHair();
    bSmoothScroll   = rVOpt.IsSmoothScroll();
    bTable              = rVOpt.IsTable();
    bGraphic            = rVOpt.IsGraphic();
    bDrawing            = rVOpt.IsDraw() && rVOpt.IsControl();
    bFieldName          = rVOpt.IsFldName();
    bNotes              = rVOpt.IsPostIts();

}

SfxPoolItem* SwElemItem::Clone( SfxItemPool* ) const
{
    return new SwElemItem( *this );
}

bool SwElemItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "different types" );

    const SwElemItem& rItem = (SwElemItem&)rAttr;

    return (    bHorzScrollbar  == rItem.bHorzScrollbar &&
                bVertScrollbar  == rItem.bVertScrollbar &&
                bAnyRuler       == rItem.bAnyRuler &&
                bHorzRuler      == rItem.bHorzRuler     &&
                bVertRuler      == rItem.bVertRuler     &&
                bVertRulerRight == rItem.bVertRulerRight&&
                bCrosshair      == rItem.bCrosshair     &&
                bSmoothScroll   == rItem.bSmoothScroll  &&
                bTable                == rItem.bTable              &&
                bGraphic              == rItem.bGraphic            &&
                bDrawing              == rItem.bDrawing            &&
                bFieldName            == rItem.bFieldName          &&
                bNotes                == rItem.bNotes             );
}

void  SwElemItem::operator=( const SwElemItem& rElemItem)
{
    bHorzScrollbar  = rElemItem.  bHorzScrollbar    ;
    bVertScrollbar  = rElemItem.  bVertScrollbar    ;
    bAnyRuler       = rElemItem.  bAnyRuler;
    bHorzRuler      = rElemItem.  bHorzRuler        ;
    bVertRuler      = rElemItem.  bVertRuler        ;
    bVertRulerRight = rElemItem.  bVertRulerRight   ;
    bCrosshair      = rElemItem.  bCrosshair        ;
    bSmoothScroll   = rElemItem.  bSmoothScroll     ;
    bTable              = rElemItem.bTable                ;
    bGraphic            = rElemItem.bGraphic              ;
    bDrawing            = rElemItem.bDrawing              ;
    bFieldName          = rElemItem.bFieldName            ;
    bNotes              = rElemItem.bNotes                ;
}

void SwElemItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetViewHScrollBar(bHorzScrollbar );
    rVOpt.SetViewVScrollBar(bVertScrollbar );
    rVOpt.SetViewAnyRuler(bAnyRuler);
    rVOpt.SetViewHRuler(bHorzRuler    );
    rVOpt.SetViewVRuler(bVertRuler    );
    rVOpt.SetVRulerRight(bVertRulerRight );
    rVOpt.SetCrossHair(bCrosshair     );
    rVOpt.SetSmoothScroll(bSmoothScroll);
    rVOpt.SetTable      (bTable             );
    rVOpt.SetGraphic    (bGraphic           );
    rVOpt.SetDraw       (bDrawing           );
    rVOpt.SetControl    (bDrawing           );
    rVOpt.SetFldName    (bFieldName         );
    rVOpt.SetPostIts    (bNotes             );
}

/*--------------------------------------------------------------------
    Description: CopyCTOR
 --------------------------------------------------------------------*/
SwAddPrinterItem::SwAddPrinterItem( const SwAddPrinterItem& rAddPrinterItem ):
            SfxPoolItem(rAddPrinterItem),
            SwPrintData( rAddPrinterItem )
{
}

/*--------------------------------------------------------------------
    Description: CTOR for empty Item
 --------------------------------------------------------------------*/
SwAddPrinterItem::SwAddPrinterItem( sal_uInt16 _nWhich):
                SfxPoolItem(_nWhich)
{
}

/*--------------------------------------------------------------------
    Description: CTOR from SwPrintOptions
 --------------------------------------------------------------------*/
SwAddPrinterItem::SwAddPrinterItem( sal_uInt16 _nWhich, const SwPrintData& rPrtData ) :
    SfxPoolItem(_nWhich)
{
    SwPrintData::operator=(rPrtData);
}

SfxPoolItem* SwAddPrinterItem::Clone( SfxItemPool* ) const
{
    return new SwAddPrinterItem( *this );
}

bool SwAddPrinterItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "different types" );

    const SwAddPrinterItem& rItem = (SwAddPrinterItem&)rAttr;

    return  SwPrintData::operator==(rItem);
}

/*--------------------------------------------------
 Item for Settings dialog, ShadowCursorPage
--------------------------------------------------*/
SwShadowCursorItem::SwShadowCursorItem( sal_uInt16 _nWhich )
    : SfxPoolItem( _nWhich ),
    eMode( FILL_TAB )
    ,bOn( sal_False )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwShadowCursorItem& rCpy )
    : SfxPoolItem( rCpy.Which() ),
    eMode( rCpy.GetMode() )
    ,bOn( rCpy.IsOn() )

{
}

SwShadowCursorItem::SwShadowCursorItem( const SwViewOption& rVOpt, sal_uInt16 _nWhich )
    : SfxPoolItem( _nWhich ),
    eMode( rVOpt.GetShdwCrsrFillMode() )
    ,bOn( rVOpt.IsShadowCursor() )

{
}

SfxPoolItem* SwShadowCursorItem::Clone( SfxItemPool* ) const
{
    return new SwShadowCursorItem( *this );
}

bool SwShadowCursorItem::operator==( const SfxPoolItem& rCmp ) const
{
    return  IsOn() == ((SwShadowCursorItem&)rCmp).IsOn() &&
            GetMode() == ((SwShadowCursorItem&)rCmp).GetMode();
}

void SwShadowCursorItem::operator=( const SwShadowCursorItem& rCpy )
{
    SetOn( rCpy.IsOn() );
    SetMode( rCpy.GetMode() );
}

void SwShadowCursorItem::FillViewOptions( SwViewOption& rVOpt ) const
{
    rVOpt.SetShadowCursor( bOn );
    rVOpt.SetShdwCrsrFillMode( eMode );
}

#ifdef DBG_UTIL
SwTestItem::SwTestItem( const SwTestItem& rTestItem ):
            SfxPoolItem(rTestItem)
{
    bTest1=rTestItem.bTest1;
    bTest2=rTestItem.bTest2;
    bTest3=rTestItem.bTest3;
    bTest4=rTestItem.bTest4;
    bTest5=rTestItem.bTest5;
    bTest6=rTestItem.bTest6;
    bTest7=rTestItem.bTest7;
    bTest8=rTestItem.bTest8;
    bTest9=rTestItem.bTest9;
    bTest10=rTestItem.bTest10;
};

SfxPoolItem* SwTestItem::Clone( SfxItemPool* ) const
{
    return new SwTestItem( *this );
}

bool SwTestItem::operator==( const SfxPoolItem& rAttr ) const
{
    OSL_ENSURE( SfxPoolItem::operator==(rAttr), "different types" );

    const SwTestItem& rItem = (const SwTestItem&) rAttr;

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
