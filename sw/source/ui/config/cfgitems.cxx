/*************************************************************************
 *
 *  $RCSfile: cfgitems.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-03-22 09:28:06 $
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
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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
    bHorzRuler     =
    bVertRuler     =
    bTableBounds   =
    bSectionBounds =
    bCrosshair     =
    bBounds        =
    bHandles       =
    bBigHandles     =
    bSmoothScroll  =
    bIndexEntry         =
    bIndexBackground    =
    bFootnoteBackground =
    bField              =
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
    bHorzRuler      = rVOpt.IsViewTabwin();
    bVertRuler      = rVOpt.IsViewVLin();
    bTableBounds    = rVOpt.IsSubsTable();
    bSectionBounds  = rVOpt.IsSectionBounds();
    bCrosshair      = rVOpt.IsCrossHair();
    bBounds         = rVOpt.IsSubsLines();
    bHandles        = rVOpt.IsSolidMarkHdl();
    bBigHandles     = rVOpt.IsBigMarkHdl();
    bSmoothScroll   = rVOpt.IsSmoothScroll();
    bIndexEntry         = rVOpt.IsTox();
    bIndexBackground    = rVOpt.IsIndexBackground();
    bFootnoteBackground = rVOpt.IsFootNote();
    bField              = rVOpt.IsField();
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
                bHorzRuler      == rItem.bHorzRuler     &&
                bVertRuler      == rItem.bVertRuler     &&
                bTableBounds    == rItem.bTableBounds   &&
                bSectionBounds  == rItem.bSectionBounds &&
                bCrosshair      == rItem.bCrosshair     &&
                bBounds         == rItem.bBounds        &&
                bHandles        == rItem.bHandles       &&
                bBigHandles     == rItem.bBigHandles    &&
                bSmoothScroll   == rItem.bSmoothScroll  &&
                bIndexEntry           == rItem.bIndexEntry         &&
                bIndexBackground      == rItem.bIndexBackground    &&
                bFootnoteBackground   == rItem.bFootnoteBackground &&
                bField                == rItem.bField              &&
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
    bHorzRuler      = rElemItem.  bHorzRuler        ;
    bVertRuler      = rElemItem.  bVertRuler        ;
    bTableBounds    = rElemItem.  bTableBounds      ;
    bSectionBounds  = rElemItem.  bSectionBounds    ;
    bCrosshair      = rElemItem.  bCrosshair        ;
    bBounds         = rElemItem.  bBounds           ;
    bHandles        = rElemItem.  bHandles          ;
    bBigHandles     = rElemItem.  bBigHandles       ;
    bSmoothScroll   = rElemItem.  bSmoothScroll     ;
    bIndexEntry         = rElemItem.bIndexEntry           ;
    bIndexBackground    = rElemItem.bIndexBackground      ;
    bFootnoteBackground = rElemItem.bFootnoteBackground   ;
    bField              = rElemItem.bField                ;
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
    rVOpt.SetViewTabwin(bHorzRuler    );
    rVOpt.SetViewVLin(bVertRuler      );
    rVOpt.SetSubsTable(bTableBounds   );
    rVOpt.SetSectionBounds(bSectionBounds);
    rVOpt.SetCrossHair(bCrosshair     );
    rVOpt.SetSubsLines(bBounds        );
    rVOpt.SetSolidMarkHdl(bHandles    );
    rVOpt.SetBigMarkHdl(bBigHandles );
    rVOpt.SetSmoothScroll(bSmoothScroll);
    rVOpt.SetTox        (bIndexEntry        );
    rVOpt.SetFootNote   (bFootnoteBackground);
    rVOpt.SetIndexBackground(bIndexBackground);
    rVOpt.SetField      (bField             );
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
    bPrintGraphic       = rAddPrinterItem.bPrintGraphic  ;
    bPrintTable         = rAddPrinterItem.bPrintTable    ;
    bPrintDrawing       = rAddPrinterItem.bPrintDrawing  ;
    bPrintControl       = rAddPrinterItem.bPrintControl  ;
    bPrintLeftPage      = rAddPrinterItem.bPrintLeftPage ;
    bPrintRightPage     = rAddPrinterItem.bPrintRightPage;
    bPrintReverse       = rAddPrinterItem.bPrintReverse  ;
    bPaperFromSetup     = rAddPrinterItem.bPaperFromSetup;
    bPrintProspect      = rAddPrinterItem.bPrintProspect ;
    bPrintPageBackground= rAddPrinterItem.bPrintPageBackground;
    bPrintBlackFont     = rAddPrinterItem.bPrintBlackFont;
    bPrintSingleJobs    = rAddPrinterItem.bPrintSingleJobs;
    nPrintPostIts       = rAddPrinterItem.nPrintPostIts  ;
    sFaxName            = rAddPrinterItem.sFaxName       ;

};

/*--------------------------------------------------------------------
    Beschreibung: CTOR fuer leeres Item
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( USHORT nWhich):
                SfxPoolItem(nWhich),
                bPrintGraphic(FALSE),
                bPrintTable(FALSE),
                bPrintDrawing(FALSE),
                bPrintControl(FALSE),
                bPrintLeftPage(FALSE),
                bPrintRightPage(FALSE),
                bPrintReverse(FALSE),
                bPaperFromSetup(FALSE),
                bPrintProspect(FALSE),
                bPrintPageBackground(FALSE),
                nPrintPostIts(0),
                sFaxName( aEmptyStr )
{
}
/*--------------------------------------------------------------------
    Beschreibung: CTOR aus SwPrintOptions
 --------------------------------------------------------------------*/

SwAddPrinterItem::SwAddPrinterItem( USHORT nWhich, SwPrintOptions* pPrtOpt ) :
    SfxPoolItem(nWhich)
{
    bPrintGraphic  = pPrtOpt->IsPrintGraphic();
    bPrintTable    = pPrtOpt->IsPrintTable();
    bPrintDrawing  = pPrtOpt->IsPrintDraw();
    bPrintControl  = pPrtOpt->IsPrintControl();
    bPrintLeftPage = pPrtOpt->IsPrintLeftPage();
    bPrintRightPage= pPrtOpt->IsPrintRightPage();
    bPrintReverse  = pPrtOpt->IsPrintReverse();
    bPaperFromSetup= pPrtOpt->IsPaperFromSetup();
    bPrintProspect = pPrtOpt->IsPrintProspect();
    bPrintPageBackground= pPrtOpt->IsPrintPageBackground();
    nPrintPostIts       = pPrtOpt->GetPrintPostIts();
    bPrintBlackFont     = pPrtOpt->IsPrintBlackFont();
    bPrintSingleJobs    = pPrtOpt->IsPrintSingleJobs();
    sFaxName     = pPrtOpt->GetFaxName();
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

    return (    bPrintGraphic   == rItem.bPrintGraphic   &&
                bPrintDrawing   == rItem.bPrintDrawing   &&
                bPrintControl   == rItem.bPrintControl   &&
                bPrintTable     == rItem.bPrintTable     &&
                bPrintLeftPage  == rItem.bPrintLeftPage  &&
                bPrintRightPage == rItem.bPrintRightPage &&
                bPrintReverse   == rItem.bPrintReverse   &&
                bPaperFromSetup == rItem.bPaperFromSetup &&
                bPrintProspect  == rItem.bPrintProspect  &&
                bPrintPageBackground == rItem.bPrintPageBackground &&
                bPrintBlackFont == rItem.bPrintBlackFont &&
                bPrintSingleJobs== rItem.bPrintSingleJobs&&
                nPrintPostIts   == rItem.nPrintPostIts   &&
                sFaxName        == rItem.sFaxName    );
}
// -----------------------------------------------------------------------

void SwAddPrinterItem::SetPrintOptions( SwPrintOptions* pPrtOpt ) const
{
    pPrtOpt->SetPrintGraphic    (bPrintGraphic);
    pPrtOpt->SetPrintTable      (bPrintTable);
    pPrtOpt->SetPrintDraw       (bPrintDrawing);
    pPrtOpt->SetPrintControl    (bPrintControl);
    pPrtOpt->SetPrintLeftPage   (bPrintLeftPage);
    pPrtOpt->SetPrintRightPage  (bPrintRightPage);
    pPrtOpt->SetPrintReverse    (bPrintReverse);
    pPrtOpt->SetPaperFromSetup  (bPaperFromSetup);
    pPrtOpt->SetPrintPostIts    (nPrintPostIts);
    pPrtOpt->SetPrintProspect   (bPrintProspect);
    pPrtOpt->SetPrintPageBackground( bPrintPageBackground );
    pPrtOpt->SetPrintBlackFont( bPrintBlackFont );
    pPrtOpt->SetPrintSingleJobs( bPrintSingleJobs );
}


/*-----------------03.11.97 10:00-------------------
 Item fuer Einstellungsdialog, ShadowCursorSeite
--------------------------------------------------*/

SwShadowCursorItem::SwShadowCursorItem( USHORT nWhich )
    : SfxPoolItem( nWhich ),
    bOn( FALSE ), eMode( FILL_TAB ), aColor( COL_BLUE )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwShadowCursorItem& rCpy )
    : SfxPoolItem( rCpy.Which() ),
    bOn( rCpy.IsOn() ), eMode( rCpy.GetMode() ), aColor( rCpy.GetColor() )
{
}

SwShadowCursorItem::SwShadowCursorItem( const SwViewOption& rVOpt, USHORT nWhich )
    : SfxPoolItem( nWhich ),
    bOn( rVOpt.IsShadowCursor() ),
    eMode( rVOpt.GetShdwCrsrFillMode() ),
    aColor( rVOpt.GetShdwCrsrColor() )
{
}

SfxPoolItem* SwShadowCursorItem::Clone( SfxItemPool* ) const
{
    return new SwShadowCursorItem( *this );
}

int SwShadowCursorItem::operator==( const SfxPoolItem& rCmp ) const
{
    return  IsOn() == ((SwShadowCursorItem&)rCmp).IsOn() &&
            GetMode() == ((SwShadowCursorItem&)rCmp).GetMode() &&
            GetColor() == ((SwShadowCursorItem&)rCmp).GetColor();
}

void SwShadowCursorItem::operator=( const SwShadowCursorItem& rCpy )
{
    SetOn( rCpy.IsOn() );
    SetMode( rCpy.GetMode() );
    SetColor( rCpy.GetColor() );
}


void SwShadowCursorItem::FillViewOptions( SwViewOption& rVOpt ) const
{
    rVOpt.SetShadowCursor( bOn );
    rVOpt.SetShdwCrsrFillMode( eMode );
    rVOpt.SetShdwCrsrColor( aColor );
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



/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:32  hr
    initial import

    Revision 1.80  2000/09/18 16:05:15  willem.vandorp
    OpenOffice header added.

    Revision 1.79  2000/09/07 08:26:13  os
    FaxName now in SwPrintOptions

    Revision 1.78  2000/05/26 07:21:28  os
    old SW Basic API Slots removed

    Revision 1.77  1999/12/14 14:29:10  jp
    Bug #69595#: print can create single Jobs

    Revision 1.76  1999/09/22 12:37:43  os
    big handles

    Revision 1.75  1999/09/01 08:42:36  OS
    useless GetPresentation() removed


      Rev 1.74   01 Sep 1999 10:42:36   OS
   useless GetPresentation() removed

      Rev 1.73   16 Aug 1999 14:09:42   OS
   #68153# show drawings and controls: call SetControl()

      Rev 1.72   09 Jun 1999 13:20:54   OS
   index background

      Rev 1.71   27 Jan 1999 10:03:12   OS
   #58677# Cursor in Readonly-Bereichen

      Rev 1.70   05 Mar 1998 14:34:08   OM
   Redline-Attribute in Module-Cfg speichern

      Rev 1.69   20 Feb 1998 17:10:50   OM
   ConfigItem fuer Redlining-Darstellungsoptionen

      Rev 1.68   24 Nov 1997 17:58:50   MA
   include

      Rev 1.67   11 Nov 1997 14:04:30   MA
   precomp entfernt

      Rev 1.66   03 Nov 1997 16:12:16   JP
   neu: Optionen/-Page/Basic-Schnittst. fuer ShadowCursor

      Rev 1.65   19 Aug 1997 10:36:08   OS
   PrintPreview aus den ViewSettings raus #42797#

      Rev 1.64   16 Aug 1997 09:33:06   OS
   Col/RowPreView aus den PrintSettings entfernt #42797#

      Rev 1.63   09 Jun 1997 14:28:40   MA
   chg: Browse-Flag nur noch am Doc

      Rev 1.62   06 Jun 1997 12:44:30   MA
   chg: versteckte Absaetze ausblenden

      Rev 1.61   25 Feb 1997 10:21:16   OS
   farbige Handles

      Rev 1.60   27 Jan 1997 16:29:42   OS
   HtmlMode entfernt

      Rev 1.59   11 Dec 1996 18:16:28   NF
   Cut-Copy-Paste Fehler behoben !?

      Rev 1.58   11 Dec 1996 14:12:02   OS
   Warnings auch fuers Basic ausbauen

      Rev 1.57   10 Dec 1996 17:00:06   OS
   Warnungen von der OFA

      Rev 1.56   28 Nov 1996 15:20:24   OS
   neu: Schwarz drucken

      Rev 1.55   06 Sep 1996 14:31:54   OS
   Postits beruecksichtigen

      Rev 1.54   31 Aug 1996 17:14:02   OS
   neue Optionendialoge

      Rev 1.53   25 Jul 1996 15:34:56   OS
   neu: HtmlMode

      Rev 1.52   17 Jul 1996 13:41:44   OS
   PrintBackground auch fuer Basic

      Rev 1.51   16 Jul 1996 20:19:52   MA
   includes

      Rev 1.50   16 Jul 1996 15:52:20   MA
   new: PrintPageBackground

      Rev 1.49   25 Jun 1996 17:58:08   HJS
   include prtopt.hxx

      Rev 1.48   25 Apr 1996 16:23:14   OS
   ViewOptions-Umbau: ElemItem wird nur noch aus ViewOptions erzeugt

      Rev 1.47   15 Mar 1996 15:28:18   JP
   TestFlags erweitert

      Rev 1.46   07 Mar 1996 16:54:38   JP
   AddPrintItem um weiters Flag erweitert

      Rev 1.45   01 Feb 1996 16:42:42   OS
   Reihenfolge der Printoptionen berichtigt

      Rev 1.44   04 Dec 1995 16:09:14   OS
   Set/FillVariable impl.

      Rev 1.43   28 Nov 1995 23:37:36   JP
   uisys.hxx gibt es nicht mehr

      Rev 1.42   27 Nov 1995 19:50:50   JP
   TYPEINIT_AUTOFACTORY segmentiert

      Rev 1.41   27 Nov 1995 18:55:22   OS
   Umstellung 303a

      Rev 1.40   24 Nov 1995 16:58:50   OM
   PCH->PRECOMPILED

      Rev 1.39   23 Nov 1995 19:49:30   OS
   Typeinfo und Default CTOR

      Rev 1.38   16 Nov 1995 18:23:22   OS
   neu: Get/SetVariable, teilweise impl.

      Rev 1.37   06 Nov 1995 17:15:00   OS
   SwElemItem bearbeitet nur noch die ViewOptions, keine UsrPrefs

      Rev 1.36   17 Oct 1995 19:20:20   OS
   AddPrinterItem Ctor(SwPrintOptions) PageView-Member initalisieren

      Rev 1.35   10 Oct 1995 18:55:44   OS
   GridItem geloescht

      Rev 1.34   15 Sep 1995 12:39:10   OS
   GetName() implementiert und ColorSetItem entfernt

      Rev 1.33   04 Sep 1995 17:14:32   OS
   +include sbx.hxx

      Rev 1.32   04 Sep 1995 13:40:24   HJS
   add: #include <sbx.hxx>

      Rev 1.31   30 Aug 1995 16:31:48   OS
   Einbau Warnungen fuer Papiergroesse und -orientierung

      Rev 1.30   09 Aug 1995 19:00:28   AMA
   kein GetPresentation

      Rev 1.29   07 Aug 1995 18:19:24   AMA
   Umbau: GetValueText -> GetPresentation

      Rev 1.28   27 Jul 1995 10:39:54   OS
   bDraw auch in Viewopt setzen

      Rev 1.27   05 Jul 1995 19:10:50   OS
   +bCtrlFld fuer Kontrollfelder

      Rev 1.26   03 Jul 1995 19:28:36   OS
   CTOR TestItem korrigiert

      Rev 1.25   27 Jun 1995 19:11:20   OS
   bTest9 fuer MD

      Rev 1.24   27 Jun 1995 17:50:06   ER
   __EXPORT

      Rev 1.23   27 Jun 1995 14:47:48   ER
   segprag

      Rev 1.22   25 Jun 1995 19:31:24   OS
   cfgitems werden jetzt direkt mit swusrpref bzw. swviewoption bearbeitet

      Rev 1.21   24 Jun 1995 10:37:40   SWG
   syntax

      Rev 1.20   23 Jun 1995 17:49:52   OS
   AddPrinterItem setzt PrintOptions selbst, +bPrintDrawing

      Rev 1.19   20 Jun 1995 19:20:42   OS
   CrossHair vom Grid zum DocDisp

      Rev 1.18   24 May 1995 18:09:26   ER
   Segmentierung

      Rev 1.17   12 May 1995 18:15:58   OS
   -SVLook

      Rev 1.16   10 May 1995 19:03:14   OS
   AddPrinterItem umgestellt

      Rev 1.15   10 May 1995 10:59:48   OS
   == > =

      Rev 1.14   28 Apr 1995 17:07:48   OS
   AddPrinterItem veraendert und um Fax und Preview-Optionen erweitert

      Rev 1.13   27 Feb 1995 12:56:18   OS
   segdefs aktualisiert

      Rev 1.12   24 Feb 1995 17:27:34   OS
   Raster-Page vollstaendig

      Rev 1.11   24 Feb 1995 13:00:00   OS
   Grid Page

      Rev 1.10   24 Feb 1995 00:54:08   ER
   sexport

      Rev 1.9   24 Feb 1995 00:32:12   OS
   (ER) add: GridItems

      Rev 1.8   05 Feb 1995 16:08:56   OS
   NT und product

      Rev 1.7   05 Feb 1995 11:33:12   OS
   keine structs mehr

      Rev 1.6   03 Feb 1995 21:20:30   ER
   wegen unresolved externals bei product einige Funktionen
   vor das ifndef product gezogen
   an OS: checken, ob die bei product als inline in das .hxx sollen!

      Rev 1.5   03 Feb 1995 09:33:22   OS
   addprintoptions - item

      Rev 1.4   01 Feb 1995 20:01:16   OS
   SwxxxStruct statisch

      Rev 1.3   24 Jan 1995 19:33:38   OS
   pch Anpassung

      Rev 1.2   23 Jan 1995 07:36:44   OS
   struct in items erzeugt

      Rev 1.1   17 Jan 1995 19:22:18   OS
   Header eingefuegt

------------------------------------------------------------------------*/

