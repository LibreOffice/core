/*************************************************************************
 *
 *  $RCSfile: optpage.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: os $ $Date: 2001-04-05 05:50:13 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _XTABLE_HXX //autogen
#include <svx/xtable.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX //autogen
#include <svx/dlgutil.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _SV_WALL_HXX
#include <vcl/wall.hxx>
#endif


#ifndef _FMTCOL_HXX //autogen
#include <fmtcol.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _OPTDLG_HXX
#include <optdlg.hxx>
#endif
#ifndef _CFGITEMS_HXX
#include <cfgitems.hxx> //Items fuer Sw-Seiten
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>
#endif
#ifndef _OPTPAGE_HXX
#include <optpage.hxx>
#endif
#ifndef _SWPRTOPT_HXX
#include <swprtopt.hxx>
#endif
#ifndef _FONTCFG_HXX
#include <fontcfg.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _SRCVCFG_HXX
#include <srcvcfg.hxx>
#endif
#ifndef _SRCVIEW_HXX
#include <srcview.hxx>
#endif
#ifndef _SRCEDTW_HXX
#include <srcedtw.hxx>
#endif
#ifndef _CRSTATE_HXX
#include <crstate.hxx>
#endif

#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _CONFIG_HRC
#include <config.hrc>
#endif
#ifndef _REDLOPT_HRC
#include <redlopt.hrc>
#endif
#ifndef _OPTDLG_HRC
#include <optdlg.hrc>
#endif

#define C2S(cChar) String::CreateFromAscii(cChar)
/*******************************************************
 ******************************************************/

/*-----------------31.08.96 10.16-------------------
    TabPage Anzeige/Inhalt
--------------------------------------------------*/

SwContentOptPage::SwContentOptPage( Window* pParent,
                                      const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, SW_RES( TP_CONTENT_OPT ), rCoreSet ),
    aLineGB       ( this,   SW_RES( GB_LINE     ) ),
    aTxtbegCB     ( this,   SW_RES( CB_TXTBEG   ) ),
    aTblbegCB     ( this,   SW_RES( CB_TABBEG   ) ),
    aSectBoundsCB( this,    SW_RES( CB_SECT_BOUNDS ) ),
    aCrossCB      ( this,   SW_RES( CB_CROSS     ) ),
    aSolidHandleCB( this,   SW_RES( CB_HANDLE   ) ),
    aBigHandleCB  ( this,   SW_RES( CB_BIGHANDLE) ),
    aWindowGB     ( this,   SW_RES( GB_WINDOW   ) ),
    aHScrollBox   ( this,   SW_RES( CB_HSCROLL   ) ),
    aVScrollBox   ( this,   SW_RES( CB_VSCROLL   ) ),
    aHRulerCBox   ( this,   SW_RES( CB_HRULER   ) ),
    aHMetric      ( this,   SW_RES( LB_HMETRIC    ) ),
    aVRulerCBox   ( this,   SW_RES( CB_VRULER    ) ),
    aVMetric      ( this,   SW_RES( LB_VMETRIC    ) ),
    aSmoothCBox   ( this,   SW_RES( CB_SMOOTH_SCROLL    ) ),
    aDispBox      ( this,   SW_RES( GB_DISP     ) ),
    aGrfCB        ( this,   SW_RES( CB_GRF          ) ),
    aTblCB        ( this,   SW_RES( CB_TBL      ) ),
    aDrwCB        ( this,   SW_RES( CB_DRWFAST   ) ),
    aFldNameCB    ( this,   SW_RES( CB_FIELD    ) ),
    aPostItCB     ( this,   SW_RES( CB_POSTIT   ) ),
    aBackBox      ( this,   SW_RES( GB_BACK     ) ),
    aIdxEntryCB   ( this,   SW_RES( CB_IDX_ENTRY     ) ),
    aIdxBackCB   ( this,    SW_RES( CB_INDEX     ) ),
    aFootBackCB   ( this,   SW_RES( CB_FOOTBACK  ) ),
    aFldBackCB    ( this,   SW_RES( CB_FLDBACK      ) )
{
    FreeResource();
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, FALSE, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aFootBackCB .Hide();
        aIdxEntryCB .Hide();
        aIdxBackCB.Hide();
        aFldBackCB.SetPosPixel(aIdxEntryCB.GetPosPixel());
    }
}

/*-----------------31.08.96 13.58-------------------

--------------------------------------------------*/


SwContentOptPage::~SwContentOptPage()
{
}

/*-----------------31.08.96 13.58-------------------

--------------------------------------------------*/


SfxTabPage* SwContentOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwContentOptPage(pParent, rAttrSet);
}

/*-----------------31.08.96 13.58-------------------

--------------------------------------------------*/


void SwContentOptPage::Reset(const SfxItemSet& rSet)
{
    const SwElemItem* pElemAttr = 0;

    rSet.GetItemState( FN_PARAM_ELEM , FALSE,
                                    (const SfxPoolItem**)&pElemAttr );
    if(pElemAttr)
    {
        aIdxEntryCB .Check  (pElemAttr->bIndexEntry           );
        aIdxBackCB.Check    (pElemAttr->bIndexBackground      );
        aFootBackCB .Check  (pElemAttr->bFootnoteBackground   );
        aFldBackCB  .Check  (pElemAttr->bField                );
        aTblCB      .Check  (pElemAttr->bTable                );
        aGrfCB      .Check  (pElemAttr->bGraphic              );
        aDrwCB      .Check  (pElemAttr->bDrawing              );
        aFldNameCB  .Check  (pElemAttr->bFieldName            );
        aPostItCB   .Check  (pElemAttr->bNotes                );
        aTxtbegCB  .Check( pElemAttr->bBounds           );
        aTblbegCB  .Check( pElemAttr->bTableBounds      );
        aSectBoundsCB.Check( pElemAttr->bSectionBounds  );
        aCrossCB   .Check( pElemAttr->bCrosshair        );
        aSolidHandleCB.Check( !pElemAttr->bHandles          );
        aBigHandleCB.Check(pElemAttr->bBigHandles       );
        aHScrollBox.Check( pElemAttr->bHorzScrollbar     );
        aVScrollBox.Check( pElemAttr->bVertScrollbar     );
        aHRulerCBox.Check( pElemAttr->bHorzRuler         );
        aVRulerCBox.Check( pElemAttr->bVertRuler         );
        aSmoothCBox.Check( pElemAttr->bSmoothScroll      );
//                                            bHtmlMode
    }

}

/*-----------------31.08.96 13.58-------------------

--------------------------------------------------*/


BOOL SwContentOptPage::FillItemSet(SfxItemSet& rSet)
{
    const SwElemItem*   pOldAttr = (const SwElemItem*)
                        GetOldItem(GetItemSet(), FN_PARAM_ELEM);

    SwElemItem aElem;
    if(pOldAttr)
        aElem = *pOldAttr;
    aElem.bIndexEntry           = aIdxEntryCB   .IsChecked();
    aElem.bIndexBackground      = aIdxBackCB    .IsChecked();
    aElem.bFootnoteBackground   = aFootBackCB   .IsChecked();
    aElem.bField                = aFldBackCB    .IsChecked();
    aElem.bTable                = aTblCB        .IsChecked();
    aElem.bGraphic              = aGrfCB        .IsChecked();
    aElem.bDrawing              = aDrwCB        .IsChecked();
    aElem.bFieldName            = aFldNameCB    .IsChecked();
    aElem.bNotes                = aPostItCB     .IsChecked();
    aElem.bBounds        = aTxtbegCB  .IsChecked();
    aElem.bTableBounds   = aTblbegCB  .IsChecked();
    aElem.bSectionBounds = aSectBoundsCB.IsChecked();
    aElem.bCrosshair     = aCrossCB   .IsChecked();
    aElem.bHandles       = !aSolidHandleCB.IsChecked();
    aElem.bBigHandles    = aBigHandleCB.IsChecked();
    aElem.bHorzScrollbar = aHScrollBox.IsChecked();
    aElem.bVertScrollbar = aVScrollBox.IsChecked();
    aElem.bHorzRuler     = aHRulerCBox.IsChecked();
    aElem.bVertRuler     = aVRulerCBox.IsChecked();
    aElem.bSmoothScroll  = aSmoothCBox.IsChecked();


    BOOL bRet = !pOldAttr || aElem != *pOldAttr;
    if(bRet)
        bRet = 0 != rSet.Put(aElem);
    return bRet;
}
/*----------------- OS 27.01.95  -----------------------
 TabPage Drucker Zusatzeinstellungen
-------------------------------------------------------*/
SwAddPrinterTabPage::SwAddPrinterTabPage( Window* pParent,
                                      const SfxItemSet& rCoreSet) :
    SfxTabPage( pParent, SW_RES( TP_OPTPRINT_PAGE ), rCoreSet),
    bPreview  ( FALSE ),
    aGrfCB           (this, SW_RES(CB_PGRF)),
    aTabCB           (this, SW_RES(CB_PTAB)),
    aDrawCB          (this, SW_RES(CB_PDRAW)),
    aCtrlFldCB       (this, SW_RES(CB_CTRLFLD)),
    aBackgroundCB    (this, SW_RES(CB_BACKGROUND)),
    aBlackFontCB     (this, SW_RES(CB_BLACK_FONT)),
    aGroup1          (this, SW_RES(GRP_1)),
    aLeftPageCB      (this, SW_RES(CB_LEFTP)),
    aRightPageCB     (this, SW_RES(CB_RIGHTP)),
    aReverseCB       (this, SW_RES(CB_REVERSE)),
    aProspectCB      (this, SW_RES(CB_PROSPECT)),
    aGroup2          (this, SW_RES(GRP_2)),
    aNoRB            (this, SW_RES(RB_NO)),
    aOnlyRB          (this, SW_RES(RB_ONLY)),
    aEndRB           (this, SW_RES(RB_END)),
    aEndPageRB       (this, SW_RES(RB_PAGEEND)),
    aGroup3          (this, SW_RES(GRP_3)),
    aGroup4          (this, SW_RES(GRP_4)),
    aPaperFromSetupCB(this, SW_RES(CB_PAPERFROMSETUP)),
    aSingleJobsCB    (this, SW_RES(CB_SINGLEJOBS)),
    aFaxFT           (this, SW_RES(FT_FAX)),
    aFaxLB           (this, SW_RES(LB_FAX)),
    bAttrModified( FALSE )
{
    Init();
    FreeResource();

    Link aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
    aGrfCB.SetClickHdl( aLk );
    aRightPageCB.SetClickHdl( aLk );
    aLeftPageCB.SetClickHdl( aLk );
    aTabCB.SetClickHdl( aLk );
    aDrawCB.SetClickHdl( aLk );
    aCtrlFldCB.SetClickHdl( aLk );
    aBackgroundCB.SetClickHdl( aLk );
    aBlackFontCB.SetClickHdl( aLk );
    aReverseCB.SetClickHdl( aLk );
    aProspectCB.SetClickHdl( aLk );
    aPaperFromSetupCB.SetClickHdl( aLk );
    aEndPageRB.SetClickHdl( aLk );
    aEndRB.SetClickHdl( aLk );
    aOnlyRB.SetClickHdl( aLk );
    aNoRB.SetClickHdl( aLk );
    aSingleJobsCB.SetClickHdl( aLk );
    aFaxLB.SetSelectHdl( LINK( this, SwAddPrinterTabPage, SelectHdl ) );

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, FALSE, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aDrawCB      .Hide();
        aLeftPageCB  .Hide();
        aRightPageCB .Hide();
        aReverseCB.SetPosPixel(aLeftPageCB  .GetPosPixel());
        aProspectCB.SetPosPixel(aRightPageCB .GetPosPixel());
        aBlackFontCB.SetPosPixel(aBackgroundCB.GetPosPixel());
        aBackgroundCB.SetPosPixel(aCtrlFldCB.GetPosPixel());
        aCtrlFldCB.SetPosPixel(aDrawCB.GetPosPixel());
    }

}

//------------------------------------------------------------------------

void SwAddPrinterTabPage::SetPreview(BOOL bPrev)
{
    bPreview = bPrev;

    if (bPreview)
    {
        aLeftPageCB.Disable();
        aRightPageCB.Disable();
        aProspectCB.Disable();
        aGroup3.Disable();
        aNoRB.Disable();
        aOnlyRB.Disable();
        aEndRB.Disable();
        aEndPageRB.Disable();
    }
}

//------------------------------------------------------------------------

SfxTabPage* SwAddPrinterTabPage::Create( Window* pParent,
                                       const SfxItemSet& rAttrSet )
{
    return ( new SwAddPrinterTabPage( pParent, rAttrSet ) );
}
//------------------------------------------------------------------------


BOOL    SwAddPrinterTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    if ( bAttrModified )
    {
        SwAddPrinterItem aAddPrinterAttr (FN_PARAM_ADDPRINTER);
        aAddPrinterAttr.bPrintGraphic   = aGrfCB.IsChecked();
        aAddPrinterAttr.bPrintTable     = aTabCB.IsChecked();
        aAddPrinterAttr.bPrintDrawing   = aDrawCB.IsChecked();
        aAddPrinterAttr.bPrintControl   = aCtrlFldCB.IsChecked();
        aAddPrinterAttr.bPrintPageBackground = aBackgroundCB.IsChecked();
        aAddPrinterAttr.bPrintBlackFont = aBlackFontCB.IsChecked();

        aAddPrinterAttr.bPrintLeftPage  = aLeftPageCB.IsChecked();
        aAddPrinterAttr.bPrintRightPage = aRightPageCB.IsChecked();
        aAddPrinterAttr.bPrintReverse   = aReverseCB.IsChecked();
        aAddPrinterAttr.bPrintProspect  = aProspectCB.IsChecked();
        aAddPrinterAttr.bPaperFromSetup = aPaperFromSetupCB.IsChecked();
        aAddPrinterAttr.bPrintSingleJobs = aSingleJobsCB.IsChecked();

        if (aNoRB.IsChecked())  aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_NONE;
        if (aOnlyRB.IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ONLY;
        if (aEndRB.IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ENDDOC;
        if (aEndPageRB.IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ENDPAGE;

        aAddPrinterAttr.sFaxName = aFaxLB.GetSelectEntry();
        rCoreSet.Put(aAddPrinterAttr);
    }
    return bAttrModified;
}
//------------------------------------------------------------------------


void    SwAddPrinterTabPage::Reset( const SfxItemSet&  )
{
    const   SfxItemSet&         rSet = GetItemSet();
    const   SwAddPrinterItem*   pAddPrinterAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ADDPRINTER , FALSE,
                                    (const SfxPoolItem**)&pAddPrinterAttr ))
    {
        aGrfCB.Check(           pAddPrinterAttr->bPrintGraphic);
        aTabCB.Check(           pAddPrinterAttr->bPrintTable);
        aDrawCB.Check(          pAddPrinterAttr->bPrintDrawing);
        aCtrlFldCB.Check(       pAddPrinterAttr->bPrintControl);
        aBackgroundCB.Check(    pAddPrinterAttr->bPrintPageBackground);
        aBlackFontCB.Check(     pAddPrinterAttr->bPrintBlackFont);
        aLeftPageCB.Check(      pAddPrinterAttr->bPrintLeftPage);
        aRightPageCB.Check(     pAddPrinterAttr->bPrintRightPage);
        aReverseCB.Check(       pAddPrinterAttr->bPrintReverse);
        aPaperFromSetupCB.Check(pAddPrinterAttr->bPaperFromSetup);
        aProspectCB.Check(      pAddPrinterAttr->bPrintProspect);
        aSingleJobsCB.Check(    pAddPrinterAttr->bPrintSingleJobs);

        aNoRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_NONE ) ;
        aOnlyRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ONLY ) ;
        aEndRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ENDDOC ) ;
        aEndPageRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ENDPAGE ) ;
        aFaxLB.SelectEntry( pAddPrinterAttr->sFaxName );
    }
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------


void    SwAddPrinterTabPage::Init()
{

}
//------------------------------------------------------------------------


IMPL_LINK_INLINE_START( SwAddPrinterTabPage, AutoClickHdl, CheckBox *, EMPTYARG )
{
    bAttrModified = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SwAddPrinterTabPage, AutoClickHdl, CheckBox *, EMPTYARG )

//------------------------------------------------------------------------


void  SwAddPrinterTabPage::SetFax( const SvStringsDtor& rFaxLst )
{
    for ( USHORT i = 0; i < rFaxLst.Count(); ++i )
        aFaxLB.InsertEntry( *rFaxLst.GetObject(i) );
}

//------------------------------------------------------------------------


IMPL_LINK_INLINE_START( SwAddPrinterTabPage, SelectHdl, ListBox *, EMPTYARG )
{
    bAttrModified=TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SwAddPrinterTabPage, SelectHdl, ListBox *, EMPTYARG )

/*-----------------03.09.96 11.53-------------------
    Tabpage Standardfonts
--------------------------------------------------*/


SwStdFontTabPage::SwStdFontTabPage( Window* pParent,
                                       const SfxItemSet& rSet ) :
    SfxTabPage( pParent, SW_RES( TP_STD_FONT ), rSet),
    aStandardLbl(this, SW_RES(FT_STANDARD)),
    aStandardBox(this, SW_RES(LB_STANDARD)),
    aTitleLbl   (this, SW_RES(FT_TITLE   )),
    aTitleBox   (this, SW_RES(LB_TITLE   )),
    aListLbl    (this, SW_RES(FT_LIST    )),
    aListBox    (this, SW_RES(LB_LIST    )),
    aLabelLbl   (this, SW_RES(FT_LABEL   )),
    aLabelBox   (this, SW_RES(LB_LABEL   )),
    aIdxLbl     (this, SW_RES(FT_IDX     )),
    aIdxBox     (this, SW_RES(LB_IDX     )),
    aStdChrFrm  (this, SW_RES(GB_STDCHR  )),
    aDocOnlyCB  (this, SW_RES(CB_DOCONLY )),
    aStandardPB (this, SW_RES(PB_STANDARD)),
    pPrt(0),
    pFontConfig(0),
    pWrtShell(0),
    bListDefault(FALSE),
    bLabelDefault(FALSE),
    bSetListDefault(TRUE),
    bSetLabelDefault(TRUE),
    bSetIdxDefault(TRUE),
    bIdxDefault(FALSE),
    bDeletePrinter(FALSE)
{
    FreeResource();
    aStandardPB.SetClickHdl(LINK(this, SwStdFontTabPage, StandardHdl));
    aStandardBox.SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    aListBox    .SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    aLabelBox   .SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    aIdxBox     .SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));

    aDocOnlyCB.Check(SW_MOD()->GetModuleConfig()->IsDefaultFontInCurrDocOnly());
}

/*-----------------03.09.96 11.53-------------------

--------------------------------------------------*/

SwStdFontTabPage::~SwStdFontTabPage()
{
    if(bDeletePrinter)
        delete pPrt;
}

/*-----------------03.09.96 11.53-------------------

--------------------------------------------------*/


SfxTabPage* SwStdFontTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwStdFontTabPage(pParent, rAttrSet);
}

/*-----------------03.09.96 11.53-------------------

--------------------------------------------------*/


void lcl_SetColl(SwWrtShell* pWrtShell, USHORT nType,
                    SfxPrinter* pPrt, const String& rStyle)
{
    BOOL bDelete = FALSE;
    const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(rStyle): 0;
    if(!pFnt)
    {
        pFnt = new SfxFont(FAMILY_DONTKNOW, rStyle);
        bDelete = TRUE;
    }
    SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(nType);
    pColl->SetAttr(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
                aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet()));
    if(bDelete)
    {
        delete (SfxFont*) pFnt;
        bDelete = FALSE;
    }
}


/*-----------------03.09.96 11.53-------------------

--------------------------------------------------*/


BOOL SwStdFontTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bNotDocOnly = !aDocOnlyCB.IsChecked();
    SW_MOD()->GetModuleConfig()->SetDefaultFontInCurrDocOnly(!bNotDocOnly);

    String sStandard    = aStandardBox.GetText();
    String sTitle       =  aTitleBox   .GetText();
    String sList        =  aListBox    .GetText();
    String sLabel       =  aLabelBox   .GetText();
    String sIdx         =  aIdxBox     .GetText();
    String sStandardBak = aStandardBox.GetSavedValue();
    String sTitleBak    = aTitleBox   .GetSavedValue();
    String sListBak     = aListBox    .GetSavedValue();
    String sLabelBak    = aLabelBox   .GetSavedValue();
    String sIdxBak      = aIdxBox     .GetSavedValue();


    if(bNotDocOnly)
    {
        pFontConfig->SetFontStandard(sStandard);
        pFontConfig->SetFontOutline(sTitle);
        pFontConfig->SetFontList(sList);
        pFontConfig->SetFontCaption(sLabel);
        pFontConfig->SetFontIndex(sIdx);
    }
    if(pWrtShell)
    {
        pWrtShell->StartAllAction();
        SfxPrinter* pPrt = pWrtShell->GetPrt();
        BOOL bMod = FALSE;
        if(sStandard != sShellStd)
        {
            BOOL bDelete = FALSE;
            const SfxFont* pFnt = pPrt ? pPrt->GetFontByName(sStandard): 0;
            if(!pFnt)
            {
                pFnt = new SfxFont(FAMILY_DONTKNOW, sStandard);
                bDelete = TRUE;
            }
            pWrtShell->SetDefault(SvxFontItem(pFnt->GetFamily(), pFnt->GetName(),
                                aEmptyStr, pFnt->GetPitch(), pFnt->GetCharSet()));
            SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetAttr(RES_CHRATR_FONT);
            if(bDelete)
            {
                delete (SfxFont*) pFnt;
                bDelete = FALSE;
            }
//          lcl_SetColl(pWrtShell, RES_POOLCOLL_STANDARD, pPrt, sStandard);
            bMod = TRUE;
        }
        if(sTitle != sShellTitle )
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_HEADLINE_BASE, pPrt, sTitle);
            bMod = TRUE;
        }
        if(sList != sShellList && (!bListDefault || !bSetListDefault ))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_NUMBUL_BASE, pPrt, sList);
            bMod = TRUE;
        }
        if(sLabel != sShellLabel && (!bLabelDefault || !bSetLabelDefault))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_LABEL, pPrt, sLabel);
            bMod = TRUE;
        }
        if(sIdx != sShellIndex && (!bIdxDefault || !bSetIdxDefault))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_REGISTER_BASE, pPrt, sIdx);
            bMod = TRUE;
        }
        if ( bMod )
            pWrtShell->SetModified();
        pWrtShell->EndAllAction();
    }

    return FALSE;
}

/*-----------------03.09.96 11.53-------------------

--------------------------------------------------*/

void SwStdFontTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem;

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_PRINTER, FALSE, &pItem))
    {
        pPrt = (SfxPrinter*)((const SwPtrItem*)pItem)->GetValue();
    }
    else
    {
        SfxItemSet* pSet = new SfxItemSet( *rSet.GetPool(),
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );
        pPrt = new SfxPrinter(pSet);
        bDeletePrinter = TRUE;
    }
    const USHORT nCount = pPrt->GetFontCount();
    for (USHORT i = 0; i < nCount; ++i)
    {
        const String &rString = pPrt->GetFont(i)->GetName();
        aStandardBox.InsertEntry( rString );
        aTitleBox   .InsertEntry( rString );
        aListBox    .InsertEntry( rString );
        aLabelBox   .InsertEntry( rString );
        aIdxBox     .InsertEntry( rString );
    }
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_STDFONTS, FALSE, &pItem))
    {
         pFontConfig = (SwStdFontConfig*)((const SwPtrItem*)pItem)->GetValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_WRTSHELL, FALSE, &pItem))
    {
        pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();
    }
    String sStdBackup;
    String sOutBackup;
    String sListBackup;
    String sCapBackup;
    String sIdxBackup;

    if(!pWrtShell)
    {
       sStdBackup = pFontConfig->GetFontStandard();
       sOutBackup = pFontConfig->GetFontOutline();
       sListBackup= pFontConfig->GetFontList();
       sCapBackup = pFontConfig->GetFontCaption();
       sIdxBackup = pFontConfig->GetFontIndex();
       aDocOnlyCB.Enable(FALSE);
    }
    else
    {
        SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
        sShellStd = sStdBackup =  pColl->GetFont().GetFamilyName();

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_HEADLINE_BASE);
        sShellTitle = sOutBackup = pColl->GetFont().GetFamilyName();

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_NUMBUL_BASE);
        bListDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(RES_CHRATR_FONT, FALSE);
        sShellList = sListBackup = pColl->GetFont().GetFamilyName();

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_LABEL);
        bLabelDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(RES_CHRATR_FONT, FALSE);
        sShellLabel = sCapBackup = pColl->GetFont().GetFamilyName();

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_REGISTER_BASE);
        bIdxDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(RES_CHRATR_FONT, FALSE);
        sShellIndex = sIdxBackup = pColl->GetFont().GetFamilyName();
    }
    aStandardBox.SetText(sStdBackup );
    aTitleBox   .SetText(sOutBackup );
    aListBox    .SetText(sListBackup);
    aLabelBox   .SetText(sCapBackup );
    aIdxBox     .SetText(sIdxBackup );

    aStandardBox.SaveValue();
    aTitleBox   .SaveValue();
    aListBox    .SaveValue();
    aLabelBox   .SaveValue();
    aIdxBox     .SaveValue();
}

/*-----------------07.09.96 12.28-------------------

--------------------------------------------------*/


IMPL_LINK( SwStdFontTabPage, StandardHdl, PushButton *, EMPTYARG )
{
    aStandardBox.SetText(SwStdFontConfig::GetDefaultFor(FONT_STANDARD));
    aTitleBox   .SetText(SwStdFontConfig::GetDefaultFor(FONT_OUTLINE));
    aListBox    .SetText(SwStdFontConfig::GetDefaultFor(FONT_LIST));
    aLabelBox   .SetText(SwStdFontConfig::GetDefaultFor(FONT_CAPTION));
    aIdxBox     .SetText(SwStdFontConfig::GetDefaultFor(FONT_INDEX));
    aStandardBox.SaveValue();
    aTitleBox   .SaveValue();
    aListBox    .SaveValue();
    aLabelBox   .SaveValue();
    aIdxBox     .SaveValue();

    return 0;
}
/*-----------------17.01.97 15.43-------------------

--------------------------------------------------*/

IMPL_LINK( SwStdFontTabPage, ModifyHdl, ComboBox*, pBox )
{
    if(pBox == &aStandardBox)
    {
        String sEntry = pBox->GetText();
        if(bSetListDefault && bListDefault)
            aListBox.SetText(sEntry);
        if(bSetLabelDefault && bLabelDefault)
            aLabelBox.SetText(sEntry);
        if(bSetIdxDefault && bIdxDefault)
            aIdxBox.SetText(sEntry);
    }
    else if(pBox == &aListBox)
    {
        bSetListDefault = FALSE;
    }
    else if(pBox == &aLabelBox)
    {
        bSetLabelDefault = FALSE;
    }
    else if(pBox == &aIdxBox)
    {
        bSetIdxDefault = FALSE;
    }
    return 0;
}

/*-----------------18.01.97 12.14-------------------
    Optionen Tabelle
--------------------------------------------------*/

SwTableOptionsTabPage::SwTableOptionsTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES(TP_OPTTABLE_PAGE), rSet),
    aHeaderCB       (this, ResId(CB_HEADER          )),
    aRepeatHeaderCB (this, ResId(CB_REPEAT_HEADER   )),
    aDontSplitCB    (this, ResId(CB_DONT_SPLIT      )),
    aBorderCB       (this, ResId(CB_BORDER          )),
    aNumFormattingCB(this, ResId(CB_NUMFORMATTING   )),
    aNumFmtFormattingCB(this, ResId(CB_NUMFMT_FORMATTING    )),
    aNumAlignmentCB (this, ResId(CB_NUMALIGNMENT    )),
    aTableGB        (this, ResId(GB_TABLE           )),
    aTableInsertGB  (this, ResId(GB_TABLE_INSERT    )),
    aMoveGB(        this, ResId(GB_MOVE     )),
    aMoveFT(        this, ResId(FT_MOVE     )),
    aRowMoveFT(     this, ResId(FT_ROWMOVE  )),
    aRowMoveMF(     this, ResId(MF_ROWMOVE  )),
    aColMoveFT(     this, ResId(FT_COLMOVE  )),
    aColMoveMF(     this, ResId(MF_COLMOVE  )),
    aInsertFT(      this, ResId(FT_INSERT   )),
    aRowInsertFT(   this, ResId(FT_ROWINSERT)),
    aRowInsertMF(   this, ResId(MF_ROWINSERT)),
    aColInsertFT(   this, ResId(FT_COLINSERT)),
    aColInsertMF(   this, ResId(MF_COLINSERT)),
    aHandlingFT(    this, ResId(FT_HANDLING )),
    aFixRB(         this, ResId(RB_FIX      )),
    aFixFT(         this, ResId(FT_FIX      )),
    aFixPropRB(     this, ResId(RB_FIXPROP  )),
    aFixPropFT(     this, ResId(FT_FIXPROP  )),
    aVarRB(         this, ResId(RB_VAR      )),
    aVarFT(         this, ResId(FT_VAR      )),
    pWrtShell(0),
    bHTMLMode(FALSE)
{
    FreeResource();

    Link aLnk(LINK(this, SwTableOptionsTabPage, CheckBoxHdl));
    aNumFormattingCB.SetClickHdl(aLnk);
    aNumFmtFormattingCB.SetClickHdl(aLnk);
    aHeaderCB.SetClickHdl(aLnk);
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SwTableOptionsTabPage::~SwTableOptionsTabPage()
{
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SfxTabPage* SwTableOptionsTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwTableOptionsTabPage(pParent, rAttrSet );
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

BOOL SwTableOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    if(aRowMoveMF.IsModified())
        pModOpt->SetTblHMove( (USHORT)aRowMoveMF.Denormalize( aRowMoveMF.GetValue(FUNIT_TWIP)));

    if(aColMoveMF.IsModified())
        pModOpt->SetTblVMove( (USHORT)aColMoveMF.Denormalize( aColMoveMF.GetValue(FUNIT_TWIP)));

    if(aRowInsertMF.IsModified())
        pModOpt->SetTblHInsert((USHORT)aRowInsertMF.Denormalize( aRowInsertMF.GetValue(FUNIT_TWIP)));

    if(aColInsertMF.IsModified())
        pModOpt->SetTblVInsert((USHORT)aColInsertMF.Denormalize( aColInsertMF.GetValue(FUNIT_TWIP)));

    TblChgMode eMode;
    if(aFixRB.IsChecked())
        eMode = TBLFIX_CHGABS;
    else if(aFixPropRB.IsChecked())
        eMode = TBLFIX_CHGPROP;
    else
        eMode = TBLVAR_CHGABS;
    if(eMode != pModOpt->GetTblMode())
    {
        pModOpt->SetTblMode(eMode);
        // der Tabellen-Tastatur-Modus hat sich geaendert, das soll
        // jetzt auch die aktuelle Tabelle erfahren
        if(pWrtShell && SwWrtShell::SEL_TBL & pWrtShell->GetSelectionType())
        {
            pWrtShell->SetTblChgMode(eMode);
        }
        static USHORT __READONLY_DATA aInva[] =
                            {   FN_TABLE_MODE_FIX,
                                FN_TABLE_MODE_FIX_PROP,
                                FN_TABLE_MODE_VARIABLE,
                                0
                            };
        pWrtShell->GetView().GetViewFrame()->GetBindings().Invalidate( aInva );
        bRet = TRUE;
    }

    USHORT nInsTblFlags = 0;

    if (aHeaderCB.IsChecked())
        nInsTblFlags |= HEADLINE;

    if (aRepeatHeaderCB.IsEnabled() && aRepeatHeaderCB.IsChecked())
        nInsTblFlags |= HEADLINE_REPEAT;

    if (!aDontSplitCB.IsChecked())
        nInsTblFlags |= SPLIT_LAYOUT;

    if (aBorderCB.IsChecked())
        nInsTblFlags |= DEFAULT_BORDER;

    if (aHeaderCB.GetSavedValue() != aHeaderCB.GetState() ||
        aRepeatHeaderCB.GetSavedValue() != aRepeatHeaderCB.GetState() ||
        aDontSplitCB.GetSavedValue() != aDontSplitCB.GetState() ||
        aBorderCB.GetSavedValue() != aBorderCB.GetState())
    {
        pModOpt->SetInsTblFlags(bHTMLMode, nInsTblFlags);
    }

    if (aNumFormattingCB.GetSavedValue() != aNumFormattingCB.GetState())
    {
        pModOpt->SetInsTblFormatNum(bHTMLMode, aNumFormattingCB.IsChecked());
        bRet = TRUE;
    }

    if (aNumFmtFormattingCB.GetSavedValue() != aNumFmtFormattingCB.GetState())
    {
        pModOpt->SetInsTblChangeNumFormat(bHTMLMode, aNumFmtFormattingCB.IsChecked());
        bRet = TRUE;
    }

    if (aNumAlignmentCB.GetSavedValue() != aNumAlignmentCB.GetState())
    {
        pModOpt->SetInsTblAlignNum(bHTMLMode, aNumAlignmentCB.IsChecked());
        bRet = TRUE;
    }

    return bRet;
}
/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/
void SwTableOptionsTabPage::Reset( const SfxItemSet& rSet)
{
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    if ( rSet.GetItemState( SID_ATTR_METRIC ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rSet.Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();
        ::SetFieldUnit( aRowMoveMF, eFieldUnit );
        ::SetFieldUnit( aColMoveMF, eFieldUnit );
        ::SetFieldUnit( aRowInsertMF, eFieldUnit );
        ::SetFieldUnit( aColInsertMF, eFieldUnit );
    }

    aRowMoveMF  .SetValue(aRowMoveMF.Normalize(pModOpt->GetTblHMove()), FUNIT_TWIP);
    aColMoveMF  .SetValue(aColMoveMF.Normalize(pModOpt->GetTblVMove()), FUNIT_TWIP);
    aRowInsertMF.SetValue(aRowInsertMF.Normalize(pModOpt->GetTblHInsert()), FUNIT_TWIP);
    aColInsertMF.SetValue(aColInsertMF.Normalize(pModOpt->GetTblVInsert()), FUNIT_TWIP);

    switch(pModOpt->GetTblMode())
    {
        case TBLFIX_CHGABS:     aFixRB.Check();     break;
        case TBLFIX_CHGPROP:    aFixPropRB.Check(); break;
        case TBLVAR_CHGABS:     aVarRB.Check(); break;
    }
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    //bestimmte Controls fuer HTML verstecken
    if(bHTMLMode)
    {
/*        Point aPos(aCaptionGB.GetPosPixel());
        long nYDiff = aTableGB.GetPosPixel().Y() - aPos.Y();
        aTableGB.SetPosPixel(aPos);

        aPos = aHeaderCB.GetPosPixel();
        aPos.Y() -= nYDiff;
        aHeaderCB.SetPosPixel(aPos);

        aPos = aRepeatHeaderCB.GetPosPixel();
        aPos.Y() -= nYDiff;
        aRepeatHeaderCB.SetPosPixel(aPos);

        aPos = aDontSplitCB.GetPosPixel(); // hier muss eine Luecke geschlossen werden
        aPos.Y() -= nYDiff;
        aBorderCB.SetPosPixel(aPos);

        aPos = aTableInsertGB.GetPosPixel();
        aPos.Y() -= nYDiff;
        aTableInsertGB.SetPosPixel(aPos);

        aPos = aNumFormattingCB.GetPosPixel();
        aPos.Y() -= nYDiff;
        aNumFormattingCB.SetPosPixel(aPos);

        aPos = aNumFmtFormattingCB.GetPosPixel();
        aPos.Y() -= nYDiff;
        aNumFmtFormattingCB.SetPosPixel(aPos);

        aPos = aNumAlignmentCB.GetPosPixel();
        aPos.Y() -= nYDiff;
        aNumAlignmentCB.SetPosPixel(aPos);

        aCaptionGB.Hide();
        aCaptionCB.Hide();
        aCaptionFT.Hide();
        aCaptionPB.Hide();
*/
        aDontSplitCB.Hide();
    }

    USHORT nInsTblFlags = pModOpt->GetInsTblFlags(bHTMLMode);

    aHeaderCB.Check(nInsTblFlags & HEADLINE);
    aRepeatHeaderCB.Check(nInsTblFlags & REPEAT);
    aDontSplitCB.Check(!(nInsTblFlags & SPLIT_LAYOUT));
    aBorderCB.Check(nInsTblFlags & DEFAULT_BORDER);

    aNumFormattingCB.Check(pModOpt->IsInsTblFormatNum(bHTMLMode));
    aNumFmtFormattingCB.Check(pModOpt->IsInsTblChangeNumFormat(bHTMLMode));
    aNumAlignmentCB.Check(pModOpt->IsInsTblAlignNum(bHTMLMode));

    aHeaderCB.SaveValue();
    aRepeatHeaderCB.SaveValue();
    aDontSplitCB.SaveValue();
    aBorderCB.SaveValue();
    aNumFormattingCB.SaveValue();
    aNumFmtFormattingCB.SaveValue();
    aNumAlignmentCB.SaveValue();

    CheckBoxHdl(0);
}
/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/
IMPL_LINK(SwTableOptionsTabPage, CheckBoxHdl, CheckBox*, EMPTYARG)
{
    aNumFmtFormattingCB.Enable(aNumFormattingCB.IsChecked());
    aNumAlignmentCB.Enable(aNumFormattingCB.IsChecked());
    aRepeatHeaderCB.Enable(aHeaderCB.IsChecked());
    return 0;
}
/*-----------------19.04.97 13:20-------------------

--------------------------------------------------*/
SwSourceViewOptionsTabPage::SwSourceViewOptionsTabPage(
                    Window* pParent,   const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES(TP_OPTSRCVIEW), rSet),
    aColorGB(this,      ResId(GB_COLOR  )),
    aSGMLFT(this,       ResId(FT_SGML   )),
    aSGMLLB(this,       ResId(LB_SGML   )),
    aCommentFT(this,    ResId(FT_COMMENT)),
    aCommentLB(this,    ResId(LB_COMMENT)),
    aKeywdFT(this,      ResId(FT_KEYWD  )),
    aKeywdLB(this,      ResId(LB_KEYWD  )),
    aUnknownFT(this,    ResId(FT_UNKNOWN)),
    aUnknownLB(this,    ResId(LB_UNKNOWN))
{
    FreeResource();
    String sSGML    ;
    String sComment ;
    String sKeywd   ;
    String sUnknown ;

    XColorTable* pColorTbl = OFF_APP()->GetStdColorTable();
    SwSrcViewConfig* pSrcVwCfg = SW_MOD()->GetSourceViewConfig();
    USHORT nCount = (USHORT)pColorTbl->Count();
    aSGMLLB     .SetUpdateMode(FALSE);
    aCommentLB  .SetUpdateMode(FALSE);
    aKeywdLB    .SetUpdateMode(FALSE);
    aUnknownLB  .SetUpdateMode(FALSE);

    Color aTmpColSGML(pSrcVwCfg->GetSyntaxColor(SRC_SYN_SGML));
    Color aTmpColComment(pSrcVwCfg->GetSyntaxColor(SRC_SYN_COMMENT));
    Color aTmpColKey(pSrcVwCfg->GetSyntaxColor(SRC_SYN_KEYWRD));
    Color aTmpColUnknown(pSrcVwCfg->GetSyntaxColor(SRC_SYN_UNKNOWN));

    for(USHORT i = 0; i < nCount; i++)
    {
        XColorEntry* pEntry = pColorTbl->Get(i);
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();
        if(aColor == aTmpColSGML )
            sSGML = sName;
        if(aColor == aTmpColComment )
            sComment = sName;
        if(aColor == aTmpColKey )
            sKeywd = sName;
        if(aColor == aTmpColUnknown )
            sUnknown = sName;
        aSGMLLB     .InsertEntry(aColor, sName);
        aCommentLB  .InsertEntry(aColor, sName);
        aKeywdLB    .InsertEntry(aColor, sName);
        aUnknownLB  .InsertEntry(aColor, sName);
    }
    aSGMLLB     .SetUpdateMode(TRUE);
    aCommentLB  .SetUpdateMode(TRUE);
    aKeywdLB    .SetUpdateMode(TRUE);
    aUnknownLB  .SetUpdateMode(TRUE);
    aSGMLLB     .SelectEntry(sSGML      );
    aCommentLB  .SelectEntry(sComment   );
    aKeywdLB    .SelectEntry(sKeywd     );
    aUnknownLB  .SelectEntry(sUnknown   );

}

/*-----------------19.04.97 13:22-------------------

--------------------------------------------------*/
SwSourceViewOptionsTabPage::~SwSourceViewOptionsTabPage()
{
}

/*-----------------19.04.97 13:22-------------------

--------------------------------------------------*/
SfxTabPage* SwSourceViewOptionsTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwSourceViewOptionsTabPage(pParent, rAttrSet);
}

/*-----------------19.04.97 13:23-------------------

--------------------------------------------------*/
BOOL SwSourceViewOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    SwSrcViewConfig* pSrcVwCfg = SW_MOD()->GetSourceViewConfig();
    BOOL bModified = FALSE;
    if(aSGMLLB.GetSelectEntry().Len() &&
        !aSGMLLB.GetSelectEntryColor().IsRGBEqual(pSrcVwCfg->GetSyntaxColor(SRC_SYN_SGML)))
    {
        bModified = TRUE;
        pSrcVwCfg->SetSyntaxColor(SRC_SYN_SGML, aSGMLLB.GetSelectEntryColor());
    }
    if(aCommentLB.GetSelectEntry().Len() &&
        !aCommentLB.GetSelectEntryColor().IsRGBEqual(pSrcVwCfg->GetSyntaxColor(SRC_SYN_COMMENT)))
    {
        bModified = TRUE;
        pSrcVwCfg->SetSyntaxColor(SRC_SYN_COMMENT, aCommentLB.GetSelectEntryColor());
    }
    if(aKeywdLB.GetSelectEntry().Len() &&
        !aKeywdLB.GetSelectEntryColor().IsRGBEqual(pSrcVwCfg->GetSyntaxColor(SRC_SYN_KEYWRD)))
    {
        bModified = TRUE;
        pSrcVwCfg->SetSyntaxColor(SRC_SYN_KEYWRD, aKeywdLB.GetSelectEntryColor());
    }
    if(aUnknownLB.GetSelectEntry().Len() &&
        !aUnknownLB.GetSelectEntryColor().IsRGBEqual(pSrcVwCfg->GetSyntaxColor(SRC_SYN_UNKNOWN)))
    {
        bModified = TRUE;
        pSrcVwCfg->SetSyntaxColor(SRC_SYN_UNKNOWN, aUnknownLB.GetSelectEntryColor());
    }
    if(bModified)
    {
        TypeId aType(TYPE(SwSrcView));
        SfxViewShell* pShell = SfxViewShell::GetFirst(&aType);
        while( pShell )
        {
            ((SwSrcView*)pShell)->GetEditWin().SyntaxColorsChanged();
            pShell = SfxViewShell::GetNext(*pShell, &aType);
        }

    }
    return FALSE;
}

/*-----------------19.04.97 13:23-------------------

--------------------------------------------------*/
void SwSourceViewOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    SwSrcViewConfig* pSrcVwCfg = SW_MOD()->GetSourceViewConfig();

    aSGMLLB     .SelectEntry( pSrcVwCfg->GetSyntaxColor(SRC_SYN_SGML) );
    aCommentLB  .SelectEntry( pSrcVwCfg->GetSyntaxColor(SRC_SYN_COMMENT) );
    aKeywdLB    .SelectEntry( pSrcVwCfg->GetSyntaxColor(SRC_SYN_KEYWRD) );
    aUnknownLB  .SelectEntry( pSrcVwCfg->GetSyntaxColor(SRC_SYN_UNKNOWN) );
}

/*  */

/*-----------------31.10.97 17:55-------------------
 TabPage fuer ShadowCrsr
--------------------------------------------------*/

SwShdwCrsrOptionsTabPage::SwShdwCrsrOptionsTabPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_OPTSHDWCRSR), rSet),
    aFlagGB( this, SW_RES( GB_SHDWCRSFLAG )),
    aOnOffCB( this, SW_RES( CB_SHDWCRSONOFF )),
    aFillModeFT( this, SW_RES( FT_SHDWCRSFILLMODE )),
    aFillMarginRB( this, SW_RES( RB_SHDWCRSFILLMARGIN )),
    aFillIndentRB( this, SW_RES( RB_SHDWCRSFILLINDENT )),
    aFillTabRB( this, SW_RES( RB_SHDWCRSFILLTAB )),
    aFillSpaceRB( this, SW_RES( RB_SHDWCRSFILLSPACE )),
//  aColorGB( this, SW_RES( GB_SHDWCRSCOLOR )),
    aColorFT( this, SW_RES( FT_SHDWCRSCOLOR )),
    aColorLB( this, SW_RES( LB_SHDWCRSCOLOR )),
    aCrsrOptGB   ( this, SW_RES( GB_CRSR_OPT)),
    aCrsrInProtCB( this, SW_RES( CB_ALLOW_IN_PROT )),
    aUnprintBox   ( this,   SW_RES( GB_NOPRINT  ) ),
    aParaCB       ( this,   SW_RES( CB_PARA      ) ),
    aSHyphCB      ( this,   SW_RES( CB_SHYPH        ) ),
    aSpacesCB     ( this,   SW_RES( CB_SPACE    ) ),
    aHSpacesCB    ( this,   SW_RES( CB_HSPACE   ) ),
    aTabCB        ( this,   SW_RES( CB_TAB      ) ),
    aBreakCB      ( this,   SW_RES( CB_BREAK        ) ),
    aHiddenCB     ( this,   SW_RES( CB_HIDDEN   ) ),
    aHiddenParaCB ( this,   SW_RES( CB_HIDDEN_PARA ) )
{
    FreeResource();

    const SfxPoolItem* pItem = 0;
    SwShadowCursorItem aOpt;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, FALSE, &pItem ))
        aOpt = *(SwShadowCursorItem*)pItem;

    aOnOffCB.Check( aOpt.IsOn() );

    BYTE eMode = aOpt.GetMode();
    aFillIndentRB.Check( FILL_INDENT == eMode );
    aFillMarginRB.Check( FILL_MARGIN == eMode );
    aFillTabRB.Check( FILL_TAB == eMode );
    aFillSpaceRB.Check( FILL_SPACE == eMode );

    const Color& rShdwCol = aOpt.GetColor();

    String sColor;
    aColorLB.SetUpdateMode(FALSE);

    XColorTable* pColorTbl = OFF_APP()->GetStdColorTable();
    for( USHORT i = 0; i < pColorTbl->Count(); ++i )
    {
        XColorEntry* pEntry = pColorTbl->Get( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();
        if( aColor == rShdwCol )
            sColor = sName;
        aColorLB.InsertEntry( aColor, sName );
    }
    aColorLB.SetUpdateMode( TRUE );
    aColorLB.SelectEntry( sColor );

    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aTabCB      .Hide();
        aHiddenCB   .Hide();
        aHiddenParaCB.Hide();
        aBreakCB.SetPosPixel(aTabCB.GetPosPixel());
    }
}

SwShdwCrsrOptionsTabPage::~SwShdwCrsrOptionsTabPage()
{
}


SfxTabPage* SwShdwCrsrOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SwShdwCrsrOptionsTabPage( pParent, rSet );
}

BOOL SwShdwCrsrOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    SwShadowCursorItem aOpt;
    aOpt.SetOn( aOnOffCB.IsChecked() );

    BYTE eMode;
    if( aFillIndentRB.IsChecked() )
        eMode= FILL_INDENT;
    else if( aFillMarginRB.IsChecked() )
        eMode = FILL_MARGIN;
    else if( aFillTabRB.IsChecked() )
        eMode = FILL_TAB;
    else
        eMode = FILL_SPACE;
    aOpt.SetMode( eMode );
    aOpt.SetColor( aColorLB.GetSelectEntryColor() );

    BOOL bRet = FALSE;
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET != rSet.GetItemState( FN_PARAM_SHADOWCURSOR, FALSE, &pItem )
        ||  ((SwShadowCursorItem&)*pItem) != aOpt )
    {
        rSet.Put( aOpt );
        bRet = TRUE;
    }

    if( aCrsrInProtCB.IsChecked() != aCrsrInProtCB.GetSavedValue())
    {
        rSet.Put(SfxBoolItem(FN_PARAM_CRSR_IN_PROTECTED, aCrsrInProtCB.IsChecked()));
        bRet |= TRUE;
    }

    const SwDocDisplayItem* pOldAttr = (const SwDocDisplayItem*)
                        GetOldItem(GetItemSet(), FN_PARAM_DOCDISP);

    SwDocDisplayItem aDisp;
    if(pOldAttr)
        aDisp = *pOldAttr;
    //
    aDisp.bParagraphEnd         = aParaCB       .IsChecked();
    aDisp.bTab                  = aTabCB        .IsChecked();
    aDisp.bSpace                = aSpacesCB     .IsChecked();
    aDisp.bNonbreakingSpace     = aHSpacesCB    .IsChecked();
    aDisp.bSoftHyphen           = aSHyphCB      .IsChecked();
    aDisp.bHiddenText           = aHiddenCB     .IsChecked();
    aDisp.bShowHiddenPara       = aHiddenParaCB .IsChecked();
    aDisp.bManualBreak          = aBreakCB      .IsChecked();

    bRet |= (!pOldAttr || aDisp != *pOldAttr);
    if(bRet)
        bRet = 0 != rSet.Put(aDisp);

    return bRet;
}

void SwShdwCrsrOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = 0;
    SwShadowCursorItem aOpt;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, FALSE, &pItem ))
        aOpt = *(SwShadowCursorItem*)pItem;

    aOnOffCB.Check( aOpt.IsOn() );

    BYTE eMode = aOpt.GetMode();
    aFillIndentRB.Check( FILL_INDENT == eMode );
    aFillMarginRB.Check( FILL_MARGIN == eMode );
    aFillTabRB.Check( FILL_TAB == eMode );
    aFillSpaceRB.Check( FILL_SPACE == eMode );

    aColorLB.SelectEntry( aOpt.GetColor() );

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_CRSR_IN_PROTECTED, FALSE, &pItem ))
        aCrsrInProtCB.Check(((const SfxBoolItem*)pItem)->GetValue());
    aCrsrInProtCB.SaveValue();

    const SwDocDisplayItem* pDocDisplayAttr = 0;

    rSet.GetItemState( FN_PARAM_DOCDISP, FALSE,
                                    (const SfxPoolItem**)&pDocDisplayAttr );
    if(pDocDisplayAttr)
    {
        aParaCB     .Check  (pDocDisplayAttr->bParagraphEnd         );
        aTabCB      .Check  (pDocDisplayAttr->bTab                  );
        aSpacesCB   .Check  (pDocDisplayAttr->bSpace                );
        aHSpacesCB  .Check  (pDocDisplayAttr->bNonbreakingSpace     );
        aSHyphCB    .Check  (pDocDisplayAttr->bSoftHyphen           );
        aHiddenCB   .Check  (pDocDisplayAttr->bHiddenText           );
        aHiddenParaCB.Check (pDocDisplayAttr->bShowHiddenPara       );
        aBreakCB    .Check  (pDocDisplayAttr->bManualBreak          );
    }
}

/*-----------------31.10.97 17:55-------------------
 TabPage fuer Redlining
--------------------------------------------------*/

struct CharAttr
{
    USHORT nItemId;
    USHORT nAttr;
};

// Editieren entspricht Einfuegen-Attributen
static CharAttr __FAR_DATA aInsertAttr[] =
{
    SID_ATTR_CHAR_CASEMAP,      SVX_CASEMAP_NOT_MAPPED,
    SID_ATTR_CHAR_WEIGHT,       WEIGHT_BOLD,
    SID_ATTR_CHAR_POSTURE,      ITALIC_NORMAL,
    SID_ATTR_CHAR_UNDERLINE,    UNDERLINE_SINGLE,
    SID_ATTR_CHAR_UNDERLINE,    UNDERLINE_DOUBLE,
    SID_ATTR_CHAR_STRIKEOUT,    STRIKEOUT_SINGLE,
    SID_ATTR_CHAR_CASEMAP,      SVX_CASEMAP_VERSALIEN,
    SID_ATTR_CHAR_CASEMAP,      SVX_CASEMAP_GEMEINE,
    SID_ATTR_CHAR_CASEMAP,      SVX_CASEMAP_KAPITAELCHEN,
    SID_ATTR_CHAR_CASEMAP,      SVX_CASEMAP_TITEL,
    SID_ATTR_BRUSH,             0,
};


/*-----------------------------------------------------------------------
    Beschreibung: Markierungsvorschau
 -----------------------------------------------------------------------*/

SwMarkPreview::SwMarkPreview( Window *pParent, const ResId& rResID ) :

    Window(pParent, rResID),

    aTransColor (COL_TRANSPARENT),
    aMarkColor  (COL_LIGHTRED),

    nMarkPos(0)

{
    SetMapMode(MAP_PIXEL);

    const Size aSz(GetOutputSizePixel());

    // Seite
    aPage.SetSize(Size(aSz.Width() - 3, aSz.Height() - 3));

    ULONG nOutWPix = aPage.GetWidth();
    ULONG nOutHPix = aPage.GetHeight();

    // PrintArea
    ULONG nLBorder = 8;
    ULONG nRBorder = 8;
    ULONG nTBorder = 4;
    ULONG nBBorder = 4;

    aLeftPagePrtArea = Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));
    USHORT nWidth = (USHORT)aLeftPagePrtArea.GetWidth();
    USHORT nKorr = (nWidth & 1) != 0 ? 0 : 1;
    aLeftPagePrtArea.SetSize(Size(nWidth / 2 - (nLBorder + nRBorder) / 2 + nKorr, aLeftPagePrtArea.GetHeight()));

    aRightPagePrtArea = aLeftPagePrtArea;
    aRightPagePrtArea.Move(aLeftPagePrtArea.GetWidth() + nLBorder + nRBorder + 1, 0);
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

SwMarkPreview::~SwMarkPreview()
{
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void SwMarkPreview::Paint(const Rectangle &rRect)
{
    // Schatten zeichnen
    Rectangle aShadow(aPage);
    aShadow += Point(3, 3);
    DrawRect(aShadow, Color(COL_GRAY), aTransColor);

    // Seite zeichnen
    DrawRect(aPage, Color(COL_WHITE), Color(COL_BLACK));

    // Separator zeichnen
    Rectangle aPageSeparator(aPage);
    aPageSeparator.SetSize(Size(2, aPageSeparator.GetHeight()));
    aPageSeparator.Move(aPage.GetWidth() / 2 - 1, 0);
    DrawRect(aPageSeparator, Color(COL_BLACK), aTransColor);

    PaintPage(aLeftPagePrtArea);
    PaintPage(aRightPagePrtArea);

    Rectangle aLeftMark(Point(aPage.Left() + 2, aLeftPagePrtArea.Top() + 4), Size(aLeftPagePrtArea.Left() - 4, 2));
    Rectangle aRightMark(Point(aRightPagePrtArea.Right() + 2, aRightPagePrtArea.Bottom() - 6), Size(aLeftPagePrtArea.Left() - 4, 2));

    switch (nMarkPos)
    {
        case 1:     // Links
            aRightMark.SetPos(Point(aRightPagePrtArea.Left() - 2 - aRightMark.GetWidth(), aRightMark.Top()));
            break;

        case 2:     // Rechts
            aLeftMark.SetPos(Point(aLeftPagePrtArea.Right() + 2, aLeftMark.Top()));
            break;

        case 3:     // Aussen
            break;

        case 4:     // Innen
            aLeftMark.SetPos(Point(aLeftPagePrtArea.Right() + 2, aLeftMark.Top()));
            aRightMark.SetPos(Point(aRightPagePrtArea.Left() - 2 - aRightMark.GetWidth(), aRightMark.Top()));
            break;

        case 0:     // Keine
        default:
            return;
    }
    DrawRect(aLeftMark, aMarkColor, aTransColor);
    DrawRect(aRightMark, aMarkColor, aTransColor);
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void SwMarkPreview::PaintPage(const Rectangle &rRect)
{
    // PrintArea zeichnen
    DrawRect(rRect, aTransColor, Color(COL_GRAY));

    // Testabsatz zeichnen
    ULONG nLTxtBorder = 4;
    ULONG nRTxtBorder = 4;
    ULONG nTTxtBorder = 4;

    Rectangle aTextLine = rRect;
    aTextLine.SetSize(Size(aTextLine.GetWidth(), 2));
    aTextLine.Left()    += nLTxtBorder;
    aTextLine.Right()   -= nRTxtBorder;
    aTextLine.Move(0, nTTxtBorder);

    USHORT nStep, nLines;
    const long nTxtLineHeight = aTextLine.GetHeight();

    nStep = aTextLine.GetHeight() + 2;
    nLines = (USHORT)(rRect.GetHeight() / (aTextLine.GetHeight() + 2)) - 1;

    // Text simulieren
    //
    for (USHORT i = 0; i < nLines; ++i)
    {
        if (i == (nLines - 1))
            aTextLine.SetSize(Size(aTextLine.GetWidth() / 2, aTextLine.GetHeight()));

        if (aPage.IsInside(aTextLine))
            DrawRect(aTextLine, Color(COL_GRAY), aTransColor);

        aTextLine.Move(0, nStep);
    }
    aTextLine.Move(0, -nStep);
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void SwMarkPreview::DrawRect(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor)
{
    SetFillColor(rFillColor);
    SetLineColor(rLineColor);
    Window::DrawRect(rRect);
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

SwRedlineOptionsTabPage::SwRedlineOptionsTabPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_REDLINE_OPT), rSet),

    aInsertGB(          this, SW_RES( GB_TE )),
    aInsertFT(          this, SW_RES( FT_INS_ATTR     )),
    aInsertLB(          this, SW_RES( LB_INS_ATTR     )),
    aInsertColorFT(     this, SW_RES( FT_INS_COL      )),
    aInsertColorLB(     this, SW_RES( LB_INS_COL      )),
    aInsertedPreviewWN( this, SW_RES( WIN_INS         )),
    aDeletedFT(         this, SW_RES( FT_DEL_ATTR     )),
    aDeletedLB(         this, SW_RES( LB_DEL_ATTR     )),
    aDeletedColorFT(    this, SW_RES( FT_DEL_COL      )),
    aDeletedColorLB(    this, SW_RES( LB_DEL_COL      )),
    aDeletedPreviewWN(  this, SW_RES( WIN_DEL         )),
    aChangedFT(         this, SW_RES( FT_CHG_ATTR     )),
    aChangedLB(         this, SW_RES( LB_CHG_ATTR     )),
    aChangedColorFT(    this, SW_RES( FT_CHG_COL      )),
    aChangedColorLB(    this, SW_RES( LB_CHG_COL      )),
    aChangedPreviewWN(  this, SW_RES( WIN_CHG         )),

    aMarkPosFT          ( this, SW_RES( FT_MARKPOS )),
    aMarkPosLB          ( this, SW_RES( LB_MARKPOS )),
    aMarkColorFT        ( this, SW_RES( FT_LC_COL )),
    aMarkColorLB        ( this, SW_RES( LB_LC_COL )),
    aMarkPreviewWN      ( this, SW_RES( WIN_MARK )),
    aChangedGB          ( this, SW_RES( GB_LC )),
    sAuthor             ( SW_RES( STR_AUTHOR )),
    sNone               ( SW_RES( STR_NOTHING ))

{
    FreeResource();

    for(USHORT i = 0; i < aInsertLB.GetEntryCount(); i++)
    {
        String sEntry(aInsertLB.GetEntry(i));
        aDeletedLB.InsertEntry(sEntry);
        aChangedLB.InsertEntry(sEntry);
    };

    Link aLk = LINK(this, SwRedlineOptionsTabPage, AttribHdl);
    aInsertLB.SetSelectHdl( aLk );
    aDeletedLB.SetSelectHdl( aLk );
    aChangedLB.SetSelectHdl( aLk );

    aLk = LINK(this, SwRedlineOptionsTabPage, ColorHdl);
    aInsertColorLB.SetSelectHdl( aLk );
    aDeletedColorLB.SetSelectHdl( aLk );
    aChangedColorLB.SetSelectHdl( aLk );

    aLk = LINK(this, SwRedlineOptionsTabPage, ChangedMaskPrevHdl);
    aMarkPosLB.SetSelectHdl( aLk );
    aMarkColorLB.SetSelectHdl( aLk );
}
/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/
SwRedlineOptionsTabPage::~SwRedlineOptionsTabPage()
{
}
/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/
SfxTabPage* SwRedlineOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SwRedlineOptionsTabPage( pParent, rSet );
}
/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/
BOOL SwRedlineOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    CharAttr *pAttr;
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    AuthorCharAttr aInsertAttr;
    AuthorCharAttr aDeletedAttr;
    AuthorCharAttr aChangedAttr;

    AuthorCharAttr aOldInsertAttr(pOpt->GetInsertAuthorAttr());
    AuthorCharAttr aOldDeletedAttr(pOpt->GetDeletedAuthorAttr());
    AuthorCharAttr aOldChangedAttr(pOpt->GetFormatAuthorAttr());

    ULONG nOldMarkColor = pOpt->GetMarkAlignColor().GetColor();
    USHORT nOldMarkMode = pOpt->GetMarkAlignMode();

    USHORT nPos = aInsertLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)aInsertLB.GetEntryData(nPos);
        aInsertAttr.nItemId = pAttr->nItemId;
        aInsertAttr.nAttr = pAttr->nAttr;

        nPos = aInsertColorLB.GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aInsertAttr.nColor = COL_NONE;
                break;
            case 1:
            case LISTBOX_ENTRY_NOTFOUND:
                aInsertAttr.nColor = COL_TRANSPARENT;
                break;
            default:
                aInsertAttr.nColor = aInsertColorLB.GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetInsertAuthorAttr(aInsertAttr);
    }

    nPos = aDeletedLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)aDeletedLB.GetEntryData(nPos);
        aDeletedAttr.nItemId = pAttr->nItemId;
        aDeletedAttr.nAttr = pAttr->nAttr;

        nPos = aDeletedColorLB.GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aDeletedAttr.nColor = COL_NONE;
                break;
            case 1:
            case LISTBOX_ENTRY_NOTFOUND:
                aDeletedAttr.nColor = COL_TRANSPARENT;
                break;
            default:
                aDeletedAttr.nColor = aDeletedColorLB.GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetDeletedAuthorAttr(aDeletedAttr);
    }

    nPos = aChangedLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)aChangedLB.GetEntryData(nPos);
        aChangedAttr.nItemId = pAttr->nItemId;
        aChangedAttr.nAttr = pAttr->nAttr;

        nPos = aChangedColorLB.GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aChangedAttr.nColor = COL_NONE;
                break;
            case 1:
            case LISTBOX_ENTRY_NOTFOUND:
                aChangedAttr.nColor = COL_TRANSPARENT;
                break;
            default:
                aChangedAttr.nColor = aChangedColorLB.GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetFormatAuthorAttr(aChangedAttr);
    }

    nPos = 0;
    switch (aMarkPosLB.GetSelectEntryPos())
    {
        case 0: nPos = HORI_NONE;       break;
        case 1: nPos = HORI_LEFT;       break;
        case 2: nPos = HORI_RIGHT;      break;
        case 3: nPos = HORI_OUTSIDE;    break;
        case 4: nPos = HORI_INSIDE;     break;
    }
    pOpt->SetMarkAlignMode(nPos);

    pOpt->SetMarkAlignColor(aMarkColorLB.GetSelectEntryColor());

    if (!(aInsertAttr == aOldInsertAttr) ||
        !(aDeletedAttr == aOldDeletedAttr) ||
        !(aChangedAttr == aOldChangedAttr) ||
       nOldMarkColor != pOpt->GetMarkAlignColor().GetColor() ||
       nOldMarkMode != pOpt->GetMarkAlignMode())
    {
        // Alle Dokumente aktualisieren
        TypeId aType(TYPE(SwDocShell));
        SwDocShell* pDocShell = (SwDocShell*)SfxObjectShell::GetFirst(&aType);

        while( pDocShell )
        {
            pDocShell->GetWrtShell()->UpdateRedlineAttr();
            pDocShell = (SwDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
        }
    }

    return FALSE;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void SwRedlineOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    const SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    const AuthorCharAttr &rInsertAttr = pOpt->GetInsertAuthorAttr();
    const AuthorCharAttr &rDeletedAttr = pOpt->GetDeletedAuthorAttr();
    const AuthorCharAttr &rChangedAttr = pOpt->GetFormatAuthorAttr();

    // Preview initialisieren
    InitFontStyle(aInsertedPreviewWN);
    InitFontStyle(aDeletedPreviewWN);
    InitFontStyle(aChangedPreviewWN);

    // Farblistboxen initialisieren
    String sColor;
    aInsertColorLB.SetUpdateMode(FALSE);
    aDeletedColorLB.SetUpdateMode(FALSE);
    aChangedColorLB.SetUpdateMode(FALSE);
    aMarkColorLB.SetUpdateMode(FALSE);

    aInsertColorLB.InsertEntry(sNone);
    aDeletedColorLB.InsertEntry(sNone);
    aChangedColorLB.InsertEntry(sNone);

    aInsertColorLB.InsertEntry(sAuthor);
    aDeletedColorLB.InsertEntry(sAuthor);
    aChangedColorLB.InsertEntry(sAuthor);

    XColorTable* pColorTbl = OFF_APP()->GetStdColorTable();
    for( USHORT i = 0; i < pColorTbl->Count(); ++i )
    {
        XColorEntry* pEntry = pColorTbl->Get( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();

        aInsertColorLB.InsertEntry( aColor, sName );
        aDeletedColorLB.InsertEntry( aColor, sName );
        aChangedColorLB.InsertEntry( aColor, sName );
        aMarkColorLB.InsertEntry( aColor, sName );
    }
    aInsertColorLB.SetUpdateMode( TRUE );
    aDeletedColorLB.SetUpdateMode( TRUE );
    aChangedColorLB.SetUpdateMode( TRUE );
    aMarkColorLB.SetUpdateMode( TRUE );

    ULONG nColor = rInsertAttr.nColor;

    switch (nColor)
    {
        case COL_TRANSPARENT:
            aInsertColorLB.SelectEntryPos(1);
            break;
        case COL_NONE:
            aInsertColorLB.SelectEntryPos(0);
            break;
        default:
            aInsertColorLB.SelectEntry(Color(nColor));
    }

    nColor = rDeletedAttr.nColor;

    switch (nColor)
    {
        case COL_TRANSPARENT:
            aDeletedColorLB.SelectEntryPos(1);
            break;
        case COL_NONE:
            aDeletedColorLB.SelectEntryPos(0);
            break;
        default:
            aDeletedColorLB.SelectEntry(Color(nColor));
    }

    nColor = rChangedAttr.nColor;

    switch (nColor)
    {
        case COL_TRANSPARENT:
            aChangedColorLB.SelectEntryPos(1);
            break;
        case COL_NONE:
            aChangedColorLB.SelectEntryPos(0);
            break;
        default:
            aChangedColorLB.SelectEntry(Color(nColor));
    }

    aMarkColorLB.SelectEntry(pOpt->GetMarkAlignColor());

    // Attributlistboxen initialisieren
    USHORT nNum = sizeof(aInsertAttr) / sizeof(CharAttr);

    aInsertLB.SelectEntryPos(0);
    aDeletedLB.SelectEntryPos(0);
    aChangedLB.SelectEntryPos(0);
    for (i = 0; i < nNum; i++)
    {
        aInsertLB.SetEntryData(i, &aInsertAttr[i]);
        if (aInsertAttr[i].nItemId == rInsertAttr.nItemId &&
            aInsertAttr[i].nAttr == rInsertAttr.nAttr)
            aInsertLB.SelectEntryPos(i);
    }

    for (i = 0; i < nNum; i++)
    {
        aDeletedLB.SetEntryData(i, &aInsertAttr[i]);
        if (aInsertAttr[i].nItemId == rDeletedAttr.nItemId &&
            aInsertAttr[i].nAttr == rDeletedAttr.nAttr)
            aDeletedLB.SelectEntryPos(i);
    }

    for (i = 0; i < nNum; i++)
    {
        aChangedLB.SetEntryData(i, &aInsertAttr[i]);
        if (aInsertAttr[i].nItemId == rChangedAttr.nItemId &&
            aInsertAttr[i].nAttr == rChangedAttr.nAttr)
            aChangedLB.SelectEntryPos(i);
    }
    //remove strikethrough from insert and delete and underline+double underline from delete
    aInsertLB.RemoveEntry(5);
    aChangedLB.RemoveEntry(5);
    aDeletedLB.RemoveEntry(4);
    aDeletedLB.RemoveEntry(3);


    USHORT nPos = 0;
    switch (pOpt->GetMarkAlignMode())
    {
        case HORI_NONE:     nPos = 0;   break;
        case HORI_LEFT:     nPos = 1;   break;
        case HORI_RIGHT:    nPos = 2;   break;
        case HORI_OUTSIDE:  nPos = 3;   break;
        case HORI_INSIDE:   nPos = 4;   break;
    }
    aMarkPosLB.SelectEntryPos(nPos);

    // Einstellungen in Preview anzeigen
    AttribHdl(&aInsertLB);
    ColorHdl(&aInsertColorLB);

    ChangedMaskPrevHdl();
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

IMPL_LINK( SwRedlineOptionsTabPage, AttribHdl, ListBox *, pLB )
{
    SvxFontPrevWindow *pPrev = 0;
    ColorListBox *pColorLB;

    if (pLB == &aInsertLB)
    {
        pColorLB = &aInsertColorLB;
        pPrev = &aInsertedPreviewWN;
    }
    else if (pLB == &aDeletedLB)
    {
        pColorLB = &aDeletedColorLB;
        pPrev = &aDeletedPreviewWN;
    }
    else
    {
        pColorLB = &aChangedColorLB;
        pPrev = &aChangedPreviewWN;
    }

    SvxFont& rFont = pPrev->GetFont();
    rFont.SetWeight(WEIGHT_NORMAL);
    rFont.SetItalic(ITALIC_NONE);
    rFont.SetUnderline(UNDERLINE_NONE);
    rFont.SetStrikeout(STRIKEOUT_NONE);
    rFont.SetCaseMap(SVX_CASEMAP_NOT_MAPPED);
    pPrev->SetColor(Color(COL_WHITE));

    USHORT nPos = pColorLB->GetSelectEntryPos();

    switch (nPos)
    {
        case 0:
            rFont.SetColor(Color(COL_BLACK));
            break;
        case 1:
        case LISTBOX_ENTRY_NOTFOUND:
            rFont.SetColor(Color(COL_RED));
            break;
        default:
            rFont.SetColor(pColorLB->GetEntryColor(nPos));
            break;
    }

    nPos = pLB->GetSelectEntryPos();
    if (nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    CharAttr *pAttr = (CharAttr *)pLB->GetEntryData(nPos);

    switch (pAttr->nItemId)
    {
        case SID_ATTR_CHAR_WEIGHT:
            rFont.SetWeight((FontWeight)pAttr->nAttr);
            break;

        case SID_ATTR_CHAR_POSTURE:
            rFont.SetItalic((FontItalic)pAttr->nAttr);
            break;

        case SID_ATTR_CHAR_UNDERLINE:
            rFont.SetUnderline((FontUnderline)pAttr->nAttr);
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            rFont.SetStrikeout((FontStrikeout)pAttr->nAttr);
            break;

        case SID_ATTR_CHAR_CASEMAP:
            rFont.SetCaseMap((SvxCaseMap)pAttr->nAttr);
            break;

        case SID_ATTR_BRUSH:
        {
            nPos = pColorLB->GetSelectEntryPos();
            if (nPos)
            {
                pPrev->SetColor(pColorLB->GetSelectEntryColor());
            }
            else
            {
                pPrev->SetColor(Color(COL_LIGHTGRAY));
            }
            rFont.SetColor(Color(COL_BLACK));
        }
        break;
    }

    pPrev->Invalidate();

    return 0;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

IMPL_LINK( SwRedlineOptionsTabPage, ColorHdl, ColorListBox *, pColorLB )
{
    SvxFontPrevWindow *pPrev = 0;
    ListBox *pLB;

    if (pColorLB == &aInsertColorLB)
    {
        pLB = &aInsertLB;
        pPrev = &aInsertedPreviewWN;
    }
    else if (pColorLB == &aDeletedColorLB)
    {
        pLB = &aDeletedLB;
        pPrev = &aDeletedPreviewWN;
    }
    else
    {
        pLB = &aChangedLB;
        pPrev = &aChangedPreviewWN;
    }

    SvxFont& rFont = pPrev->GetFont();
    USHORT nPos = pLB->GetSelectEntryPos();
    if (nPos == LISTBOX_ENTRY_NOTFOUND)
        nPos = 0;

    CharAttr *pAttr = (CharAttr *)pLB->GetEntryData(nPos);

    if (pAttr->nItemId == SID_ATTR_BRUSH)
    {
        rFont.SetColor(Color(COL_BLACK));
        nPos = pColorLB->GetSelectEntryPos();
        if (nPos && nPos != LISTBOX_ENTRY_NOTFOUND)
        {
            pPrev->SetColor(pColorLB->GetSelectEntryColor());
        }
        else
        {
            pPrev->SetColor(Color(COL_LIGHTGRAY));
        }
    }
    else
    {
        nPos = pColorLB->GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                rFont.SetColor(Color(COL_BLACK));
                break;
            case 1:
            case LISTBOX_ENTRY_NOTFOUND:
                rFont.SetColor(Color(COL_RED));
                break;
            default:
                rFont.SetColor(pColorLB->GetEntryColor(nPos));
                break;
        }
    }

    pPrev->Invalidate();

    return 0;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

IMPL_LINK( SwRedlineOptionsTabPage, ChangedMaskPrevHdl, ListBox *, pLB )
{
    aMarkPreviewWN.SetMarkPos(aMarkPosLB.GetSelectEntryPos());
    aMarkPreviewWN.SetColor(aMarkColorLB.GetSelectEntryColor().GetColor());

    aMarkPreviewWN.Invalidate();

    return 0;
}

/*-----------------------------------------------------------------------
    Beschreibung:
 -----------------------------------------------------------------------*/

void SwRedlineOptionsTabPage::InitFontStyle(SvxFontPrevWindow& rExampleWin)
{
    SvxFont& rFont = rExampleWin.GetFont();

    Font aFont;
    aFont.SetCharSet( gsl_getSystemTextEncoding() );
    aFont.SetSize( Size( 0, 12 ) );
    aFont.SetPitch( PITCH_VARIABLE );

    aFont.SetName( C2S("Times") );
    aFont.SetFamily( FAMILY_ROMAN );

//  Font aFont = System::GetStandardFont(STDFONT_ROMAN);
    aFont.SetFillColor(Color(COL_WHITE));
    aFont.SetWeight(WEIGHT_NORMAL);

    rFont = aFont;
    const Size aLogSize( rExampleWin.GetOutputSize() );
    rFont.SetSize(Size(0, aLogSize.Height() * 2 / 3));
    rExampleWin.SetFont(rFont);
    rExampleWin.UseResourceText();

    Color aWhiteColor = COL_WHITE;
    Wallpaper aWall(aWhiteColor);
//  Wallpaper aWall(Color( COL_WHITE ));
    rExampleWin.SetBackground(aWall);
    rExampleWin.Invalidate();
}


#ifndef PRODUCT
/*******************************************************
 ******************************************************/
/*----------------- OS 11.01.95  -----------------------
 TabPage Testeinstellungen
-------------------------------------------------------*/

void lcl_SetPosSize(Window& rWin, Point aPos, Size aSize)
{
    aPos = rWin.LogicToPixel(aPos, MAP_APPFONT);
    aSize = rWin.OutputDevice::LogicToPixel(aSize, MAP_APPFONT);
    rWin.SetPosSizePixel(aPos, aSize);
}

SwTestTabPage::SwTestTabPage( Window* pParent,
                                      const SfxItemSet& rCoreSet) :
    SfxTabPage( pParent, WB_HIDE, rCoreSet),
    aTestGBox       ( this, WB_GROUP ),
    aTest1CBox      ( this, 0 ),
    aTest2CBox      ( this, 0 ),
    aTest3CBox      ( this, 0 ),
    aTest4CBox      ( this, 0 ),
    aTest5CBox      ( this, 0 ),
    aTest6CBox      ( this, 0 ),
    aTest7CBox      ( this, 0 ),
    aTest8CBox      ( this, 0 ),
    aTest9CBox      ( this, 0 ),
    aTest10CBox     ( this, 0 ),
    bAttrModified( FALSE )
{
    lcl_SetPosSize(*this,       Point(0,0), Size(260 , 135));
    lcl_SetPosSize(aTestGBox,   Point(6,2), Size(209,8));
    lcl_SetPosSize(aTest1CBox  ,    Point(12 , 14), Size(74 , 10));
    lcl_SetPosSize(aTest2CBox  ,    Point(12 , 27), Size(74 , 10));
    lcl_SetPosSize(aTest3CBox  ,    Point(12 , 40), Size(74 , 10));
    lcl_SetPosSize(aTest4CBox  ,    Point(12 , 53), Size(74 , 10));
    lcl_SetPosSize(aTest5CBox  ,    Point(12 , 66), Size(74 , 10));
    lcl_SetPosSize(aTest6CBox  ,    Point(116, 14), Size(74 , 10));
    lcl_SetPosSize(aTest7CBox  ,    Point(116, 27), Size(74 , 10));
    lcl_SetPosSize(aTest8CBox  ,    Point(116, 40), Size(74 , 10));
    lcl_SetPosSize(aTest9CBox  ,    Point(116, 53), Size(74 , 10));
    lcl_SetPosSize(aTest10CBox  ,   Point(116, 66), Size(74 , 10));

    aTestGBox.SetText( String(ByteString("Einstellungen nur für Testzwecke" ), RTL_TEXTENCODING_MS_1252));
    aTest1CBox .SetText( C2S("Layout not loading"));
    aTest2CBox .SetText( C2S("dynamic"));
    aTest3CBox .SetText( C2S("No calm"               ));
    aTest4CBox .SetText( C2S("WYSIWYG debug"         ));
    aTest5CBox .SetText( C2S("No idle format"        ));
    aTest6CBox .SetText( C2S("No screen adj"         ));
    aTest7CBox .SetText( C2S("win format"            ));
    aTest8CBox .SetText( C2S("No Scroll"             ));
    aTest9CBox .SetText( C2S("DrawingLayerNotLoading"));
    aTest10CBox.SetText( C2S("AutoFormat by Input"   ));
    aTestGBox.Show();
    aTest1CBox .Show();
    aTest2CBox .Show();
    aTest3CBox .Show();
    aTest4CBox .Show();
    aTest5CBox .Show();
    aTest6CBox .Show();
    aTest7CBox .Show();
    aTest8CBox .Show();
    aTest9CBox .Show();
    aTest10CBox.Show();
    Init();

}


//------------------------------------------------------------------------


SfxTabPage* SwTestTabPage::Create( Window* pParent,
                                       const SfxItemSet& rAttrSet )
{
    return ( new SwTestTabPage( pParent, rAttrSet ) );
}
//------------------------------------------------------------------------


BOOL    SwTestTabPage::FillItemSet( SfxItemSet& rCoreSet )
{

    if ( bAttrModified )
    {
        SwTestItem aTestItem(FN_PARAM_SWTEST);
            aTestItem.bTest1=aTest1CBox.IsChecked();
            aTestItem.bTest2=aTest2CBox.IsChecked();
            aTestItem.bTest3=aTest3CBox.IsChecked();
            aTestItem.bTest4=aTest4CBox.IsChecked();
            aTestItem.bTest5=aTest5CBox.IsChecked();
            aTestItem.bTest6=aTest6CBox.IsChecked();
            aTestItem.bTest7=aTest7CBox.IsChecked();
            aTestItem.bTest8=aTest8CBox.IsChecked();
            aTestItem.bTest9=aTest9CBox.IsChecked();
            aTestItem.bTest10=aTest10CBox.IsChecked();
        rCoreSet.Put(aTestItem);
    }
    return bAttrModified;
}
//------------------------------------------------------------------------


void SwTestTabPage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();
    const SwTestItem* pTestAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SWTEST , FALSE,
                                    (const SfxPoolItem**)&pTestAttr ))
    {
        aTest1CBox.Check(pTestAttr->bTest1);
        aTest2CBox.Check(pTestAttr->bTest2);
        aTest3CBox.Check(pTestAttr->bTest3);
        aTest4CBox.Check(pTestAttr->bTest4);
        aTest5CBox.Check(pTestAttr->bTest5);
        aTest6CBox.Check(pTestAttr->bTest6);
        aTest7CBox.Check(pTestAttr->bTest7);
        aTest8CBox.Check(pTestAttr->bTest8);
        aTest9CBox.Check(pTestAttr->bTest9);
        aTest10CBox.Check(pTestAttr->bTest10);
    }
}
//------------------------------------------------------------------------


void SwTestTabPage::Init()
{
    // handler
    Link aLk = LINK( this, SwTestTabPage, AutoClickHdl );
    aTest1CBox.SetClickHdl( aLk );
    aTest2CBox.SetClickHdl( aLk );
    aTest3CBox.SetClickHdl( aLk );
    aTest4CBox.SetClickHdl( aLk );
    aTest5CBox.SetClickHdl( aLk );
    aTest6CBox.SetClickHdl( aLk );
    aTest7CBox.SetClickHdl( aLk );
    aTest8CBox.SetClickHdl( aLk );
    aTest9CBox.SetClickHdl( aLk );
    aTest10CBox.SetClickHdl( aLk );
}
//------------------------------------------------------------------------


IMPL_LINK_INLINE_START( SwTestTabPage, AutoClickHdl, CheckBox *, EMPTYARG )
{
    bAttrModified = TRUE;
    return 0;
}
IMPL_LINK_INLINE_END( SwTestTabPage, AutoClickHdl, CheckBox *, EMPTYARG )
#endif



