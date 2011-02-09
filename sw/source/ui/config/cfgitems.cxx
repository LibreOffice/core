/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <basic/sbxvar.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
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

/*------------OS 12.01.95 -------------------------------------------
    Item fuer Einstellungsdialog, Seite Dokumentansicht
--------------------------------------------------------------------*/

SwDocDisplayItem::SwDocDisplayItem( const SwDocDisplayItem& rDocDisplayItem ):
            SfxPoolItem(rDocDisplayItem)
{
    *this = rDocDisplayItem;
};

/*----------------------- -------------------------------------------

--------------------------------------------------------------------*/

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
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem* SwDocDisplayItem::Clone( SfxItemPool*  ) const
{
    return new SwDocDisplayItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SwDocDisplayItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

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


/*-----------------31.08.96 14.14-------------------

--------------------------------------------------*/


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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
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


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
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
    bHandles       =
    bBigHandles     =
    bSmoothScroll  =
    bTable              =
    bGraphic            =
    bDrawing            =
    bFieldName          =
    bNotes              = sal_False;
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwElemItem::SwElemItem( const SwElemItem& rElemItem ):
            SfxPoolItem(rElemItem)
{
    *this = rElemItem;
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

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
    bHandles        = rVOpt.IsSolidMarkHdl();
    bBigHandles     = rVOpt.IsBigMarkHdl();
    bSmoothScroll   = rVOpt.IsSmoothScroll();
    bTable              = rVOpt.IsTable();
    bGraphic            = rVOpt.IsGraphic();
    bDrawing            = rVOpt.IsDraw() && rVOpt.IsControl();
    bFieldName          = rVOpt.IsFldName();
    bNotes              = rVOpt.IsPostIts();

}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem* SwElemItem::Clone( SfxItemPool* ) const
{
    return new SwElemItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SwElemItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SwElemItem& rItem = (SwElemItem&)rAttr;

    return (    bHorzScrollbar  == rItem.bHorzScrollbar &&
                bVertScrollbar  == rItem.bVertScrollbar &&
                bAnyRuler       == rItem.bAnyRuler &&
                bHorzRuler      == rItem.bHorzRuler     &&
                bVertRuler      == rItem.bVertRuler     &&
                bVertRulerRight == rItem.bVertRulerRight&&
                bCrosshair      == rItem.bCrosshair     &&
                bHandles        == rItem.bHandles       &&
                bBigHandles     == rItem.bBigHandles    &&
                bSmoothScroll   == rItem.bSmoothScroll  &&
                bTable                == rItem.bTable              &&
                bGraphic              == rItem.bGraphic            &&
                bDrawing              == rItem.bDrawing            &&
                bFieldName            == rItem.bFieldName          &&
                bNotes                == rItem.bNotes             );
}

/*-----------------31.08.96 14.13-------------------

--------------------------------------------------*/


void  SwElemItem::operator=( const SwElemItem& rElemItem)
{
    bHorzScrollbar  = rElemItem.  bHorzScrollbar    ;
    bVertScrollbar  = rElemItem.  bVertScrollbar    ;
    bAnyRuler       = rElemItem.  bAnyRuler;
    bHorzRuler      = rElemItem.  bHorzRuler        ;
    bVertRuler      = rElemItem.  bVertRuler        ;
    bVertRulerRight = rElemItem.  bVertRulerRight   ;
    bCrosshair      = rElemItem.  bCrosshair        ;
    bHandles        = rElemItem.  bHandles          ;
    bBigHandles     = rElemItem.  bBigHandles       ;
    bSmoothScroll   = rElemItem.  bSmoothScroll     ;
    bTable              = rElemItem.bTable                ;
    bGraphic            = rElemItem.bGraphic              ;
    bDrawing            = rElemItem.bDrawing              ;
    bFieldName          = rElemItem.bFieldName            ;
    bNotes              = rElemItem.bNotes                ;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwElemItem::FillViewOptions( SwViewOption& rVOpt) const
{
    rVOpt.SetViewHScrollBar(bHorzScrollbar );
    rVOpt.SetViewVScrollBar(bVertScrollbar );
    rVOpt.SetViewAnyRuler(bAnyRuler);
    rVOpt.SetViewHRuler(bHorzRuler    );
    rVOpt.SetViewVRuler(bVertRuler    );
    rVOpt.SetVRulerRight(bVertRulerRight );
    rVOpt.SetCrossHair(bCrosshair     );
    rVOpt.SetSolidMarkHdl(bHandles    );
    rVOpt.SetBigMarkHdl(bBigHandles );
    rVOpt.SetSmoothScroll(bSmoothScroll);
    rVOpt.SetTable      (bTable             );
    rVOpt.SetGraphic    (bGraphic           );
    rVOpt.SetDraw       (bDrawing           );
    rVOpt.SetControl    (bDrawing           );
    rVOpt.SetFldName    (bFieldName         );
    rVOpt.SetPostIts    (bNotes             );
}


/*--------------------------------------------------------------------
    Beschreibung: CopyCTOR
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( const SwAddPrinterItem& rAddPrinterItem ):
            SfxPoolItem(rAddPrinterItem),
            SwPrintData( rAddPrinterItem )
{
}

/*--------------------------------------------------------------------
    Beschreibung: CTOR fuer leeres Item
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( sal_uInt16 _nWhich):
                SfxPoolItem(_nWhich)
{
}
/*--------------------------------------------------------------------
    Beschreibung: CTOR aus SwPrintOptions
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( sal_uInt16 _nWhich, const SwPrintData& rPrtData ) :
    SfxPoolItem(_nWhich)
{
    SwPrintData::operator=(rPrtData);
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem* SwAddPrinterItem::Clone( SfxItemPool* ) const
{
    return new SwAddPrinterItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SwAddPrinterItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

    const SwAddPrinterItem& rItem = (SwAddPrinterItem&)rAttr;

    return  SwPrintData::operator==(rItem);
}
/*-----------------03.11.97 10:00-------------------
 Item fuer Einstellungsdialog, ShadowCursorSeite
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

int SwShadowCursorItem::operator==( const SfxPoolItem& rCmp ) const
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
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SfxPoolItem* SwTestItem::Clone( SfxItemPool* ) const
{
    return new SwTestItem( *this );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

int SwTestItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unterschiedliche Typen" );

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


