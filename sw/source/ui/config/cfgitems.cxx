/*************************************************************************
 *
 *  $RCSfile: cfgitems.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:17:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef __SBX_SBXVARIABLE_HXX //autogen
#include <svtools/sbxvar.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif

#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif


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


SwDocDisplayItem::SwDocDisplayItem( USHORT nWhich ) :
        SfxPoolItem(nWhich),
        aIndexBackgrndCol(COL_GRAY)
{
    bParagraphEnd       =
    bTab                =
    bSpace              =
    bNonbreakingSpace   =
    bSoftHyphen         =
    bHiddenText         =
    bManualBreak        =
    bShowHiddenPara     = FALSE;

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

SwDocDisplayItem::SwDocDisplayItem(const SwViewOption& rVOpt, USHORT nWhich ) :
            SfxPoolItem( nWhich )
{
    bParagraphEnd       = rVOpt.IsParagraph(TRUE);
    bTab                = rVOpt.IsTab(TRUE);
    bSpace              = rVOpt.IsBlank(TRUE);
    bNonbreakingSpace   = rVOpt.IsHardBlank();
    bSoftHyphen         = rVOpt.IsSoftHyph();
    bHiddenText         = rVOpt.IsHidden();
    bManualBreak        = rVOpt.IsLineBreak(TRUE);
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
              bHiddenText           == rItem.bHiddenText         &&
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
    bHiddenText         = rDocDisplayItem.bHiddenText           ;
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
    rVOpt.SetHidden     (bHiddenText        );
    rVOpt.SetLineBreak  (bManualBreak       );
    rVOpt.SetShowHiddenPara(bShowHiddenPara );
}


/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
SwElemItem::SwElemItem( USHORT nWhich ) :
    SfxPoolItem(nWhich)
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
    bNotes              = FALSE;
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

SwElemItem::SwElemItem(const SwViewOption& rVOpt, USHORT nWhich) :
            SfxPoolItem( nWhich )
{
    bHorzScrollbar  = rVOpt.IsViewHScrollBar();
    bVertScrollbar  = rVOpt.IsViewVScrollBar();
    bAnyRuler       = rVOpt.IsViewAnyRuler();
    bHorzRuler      = rVOpt.IsViewHRuler(TRUE);
    bVertRuler      = rVOpt.IsViewVRuler(TRUE);
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
            SfxPoolItem(rAddPrinterItem)
{
    *((SwPrintData*)this) = rAddPrinterItem;
};

/*--------------------------------------------------------------------
    Beschreibung: CTOR fuer leeres Item
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( USHORT nWhich):
                SfxPoolItem(nWhich)
{
}
/*--------------------------------------------------------------------
    Beschreibung: CTOR aus SwPrintOptions
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( USHORT nWhich, const SwPrintData& rPrtData ) :
    SfxPoolItem(nWhich)
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

SwShadowCursorItem::SwShadowCursorItem( USHORT nWhich )
    : SfxPoolItem( nWhich ),
    bOn( FALSE ), eMode( FILL_TAB )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwShadowCursorItem& rCpy )
    : SfxPoolItem( rCpy.Which() ),
    bOn( rCpy.IsOn() ),
    eMode( rCpy.GetMode() )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwViewOption& rVOpt, USHORT nWhich )
    : SfxPoolItem( nWhich ),
    bOn( rVOpt.IsShadowCursor() ),
    eMode( rVOpt.GetShdwCrsrFillMode() )
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

#ifndef PRODUCT
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



