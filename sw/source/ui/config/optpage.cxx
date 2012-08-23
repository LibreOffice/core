/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <optpage.hxx>
#include <doc.hxx>
#include <hintids.hxx>
#include <cmdid.h>
#include <fmtcol.hxx>
#include <charatr.hxx>
#include <swtypes.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <uitool.hxx>
#include <cfgitems.hxx> //Items for Sw-pages
#include <poolfmt.hxx>
#include <uiitems.hxx>
#include <initui.hxx>
#include <printdata.hxx>
#include <modcfg.hxx>
#include <srcview.hxx>
#include <crstate.hxx>
#include <viewopt.hxx>
#include <globals.hrc>
#include <config.hrc>
#include <redlopt.hrc>
#include <optdlg.hrc>
#include <swwrtshitem.hxx>
#include <unomid.h>

#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <svl/slstitm.hxx>
#include <svl/ctloptions.hxx>
#include <svl/eitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svx/htmlmode.hxx>
#include <svx/xtable.hxx>
#include <svx/dlgutil.hxx>
#include <svx/strarray.hxx>
#include <vcl/svapp.hxx>


using namespace ::com::sun::star;

SwContentOptPage::SwContentOptPage( Window* pParent,
                                      const SfxItemSet& rCoreSet ) :
    SfxTabPage( pParent, SW_RES( TP_CONTENT_OPT ), rCoreSet ),
    aLineFL       ( this,   SW_RES( FL_LINE     ) ),
    aCrossCB      ( this,   SW_RES( CB_CROSS     ) ),

    aWindowFL     ( this,   SW_RES( FL_WINDOW   ) ),
    aHScrollBox   ( this,   SW_RES( CB_HSCROLL   ) ),
    aVScrollBox   ( this,   SW_RES( CB_VSCROLL   ) ),
    aAnyRulerCB   ( this,   SW_RES( CB_ANY_RULER ) ),
    aHRulerCBox   ( this,   SW_RES( CB_HRULER   ) ),
    aHMetric      ( this,   SW_RES( LB_HMETRIC    ) ),
    aVRulerCBox   ( this,   SW_RES( CB_VRULER    ) ),
    aVRulerRightCBox( this, SW_RES( CB_VRULER_RIGHT    ) ),
    aVMetric      ( this,   SW_RES( LB_VMETRIC    ) ),
    aSmoothCBox   ( this,   SW_RES( CB_SMOOTH_SCROLL    ) ),

    aDispFL      ( this,   SW_RES( FL_DISP     ) ),
    aGrfCB        ( this,   SW_RES( CB_GRF          ) ),
    aTblCB        ( this,   SW_RES( CB_TBL      ) ),
    aDrwCB        ( this,   SW_RES( CB_DRWFAST   ) ),
    aFldNameCB    ( this,   SW_RES( CB_FIELD    ) ),
    aPostItCB     ( this,   SW_RES( CB_POSTIT   ) ),

    aSettingsFL   ( this,   SW_RES( FL_SETTINGS   ) ),
    aMetricFT     ( this,   SW_RES( FT_METRIC   ) ),
    aMetricLB     ( this,   SW_RES( LB_METRIC   ) )
{
    FreeResource();
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, sal_False, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aMetricLB.Show();
        aSettingsFL.Show();
        aMetricFT.Show();
    }
    SvtCJKOptions aCJKOptions;
    if(aCJKOptions.IsVerticalTextEnabled() )
    {
        Point aSmoothPos(aSmoothCBox.GetPosPixel());
        aSmoothPos.Y() += aSmoothPos.Y() - aVRulerCBox.GetPosPixel().Y();
        aSmoothCBox.SetPosPixel(aSmoothPos);
    }
    else
        aVRulerRightCBox.Hide();
    aVRulerCBox.SetClickHdl(LINK(this, SwContentOptPage, VertRulerHdl ));
    aAnyRulerCB.SetClickHdl(LINK(this, SwContentOptPage, AnyRulerHdl));

    SvxStringArray aMetricArr( SW_RES( STR_ARR_METRIC ) );
    for ( sal_uInt16 i = 0; i < aMetricArr.Count(); ++i )
    {
        String sMetric = aMetricArr.GetStringByPos( i );
        FieldUnit eFUnit = (FieldUnit)aMetricArr.GetValue( i );

        switch ( eFUnit )
        {
            case FUNIT_MM:
            case FUNIT_CM:
            case FUNIT_POINT:
            case FUNIT_PICA:
            case FUNIT_INCH:
            case FUNIT_CHAR:    // add two units , 'character' and 'line' , their ticks are not fixed
            case FUNIT_LINE:
            {
                // only use these metrics
                // a horizontal ruler has not the 'line' unit
                // there isn't 'line' unit in HTML format
                if ( eFUnit != FUNIT_LINE )
                {
                   sal_uInt16 nPos = aMetricLB.InsertEntry( sMetric );
                   aMetricLB.SetEntryData( nPos, (void*)(long)eFUnit );
                   aHMetric.InsertEntry( sMetric );
                   aHMetric.SetEntryData( nPos, (void*)(long)eFUnit );
                }
                // a vertical ruler has not the 'character' unit
                if ( eFUnit != FUNIT_CHAR )
                {
                   sal_uInt16 nPos = aVMetric.InsertEntry( sMetric );
                   aVMetric.SetEntryData( nPos, (void*)(long)eFUnit );
                }
            }
            default:;//prevent warning
        }
    }
}

SwContentOptPage::~SwContentOptPage()
{
}

SfxTabPage* SwContentOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new SwContentOptPage(pParent, rAttrSet);
}

static void lcl_SelectMetricLB(ListBox& rMetric, sal_uInt16 nSID, const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if( rSet.GetItemState( nSID, sal_False, &pItem ) >= SFX_ITEM_AVAILABLE )
    {
        FieldUnit eFieldUnit = (FieldUnit)((SfxUInt16Item*)pItem)->GetValue();
        for ( sal_uInt16 i = 0; i < rMetric.GetEntryCount(); ++i )
        {
            if ( (int)(sal_IntPtr)rMetric.GetEntryData( i ) == (int)eFieldUnit )
            {
                rMetric.SelectEntryPos( i );
                break;
            }
        }
    }
    rMetric.SaveValue();
}

void SwContentOptPage::Reset(const SfxItemSet& rSet)
{
    const SwElemItem* pElemAttr = 0;

    rSet.GetItemState( FN_PARAM_ELEM , sal_False,
                                    (const SfxPoolItem**)&pElemAttr );
    if(pElemAttr)
    {
        aTblCB      .Check  (pElemAttr->bTable                );
        aGrfCB      .Check  (pElemAttr->bGraphic              );
        aDrwCB      .Check  (pElemAttr->bDrawing              );
        aFldNameCB  .Check  (pElemAttr->bFieldName            );
        aPostItCB   .Check  (pElemAttr->bNotes                );
        aCrossCB   .Check( pElemAttr->bCrosshair        );
        aHScrollBox.Check( pElemAttr->bHorzScrollbar     );
        aVScrollBox.Check( pElemAttr->bVertScrollbar     );
        aAnyRulerCB.Check( pElemAttr->bAnyRuler );
        aHRulerCBox.Check( pElemAttr->bHorzRuler         );
        aVRulerCBox.Check( pElemAttr->bVertRuler         );
        aVRulerRightCBox.Check(pElemAttr->bVertRulerRight);
        aSmoothCBox.Check( pElemAttr->bSmoothScroll      );
    }
    aMetricLB.SetNoSelection();
    lcl_SelectMetricLB(aMetricLB, SID_ATTR_METRIC, rSet);
    lcl_SelectMetricLB(aHMetric, FN_HSCROLL_METRIC, rSet);
    lcl_SelectMetricLB(aVMetric, FN_VSCROLL_METRIC, rSet);
    AnyRulerHdl(&aAnyRulerCB);
}

sal_Bool SwContentOptPage::FillItemSet(SfxItemSet& rSet)
{
    const SwElemItem*   pOldAttr = (const SwElemItem*)
                        GetOldItem(GetItemSet(), FN_PARAM_ELEM);

    SwElemItem aElem;
    if(pOldAttr)
        aElem = *pOldAttr;
    aElem.bTable                = aTblCB        .IsChecked();
    aElem.bGraphic              = aGrfCB        .IsChecked();
    aElem.bDrawing              = aDrwCB        .IsChecked();
    aElem.bFieldName            = aFldNameCB    .IsChecked();
    aElem.bNotes                = aPostItCB     .IsChecked();
    aElem.bCrosshair     = aCrossCB   .IsChecked();
    aElem.bHorzScrollbar = aHScrollBox.IsChecked();
    aElem.bVertScrollbar = aVScrollBox.IsChecked();
    aElem.bAnyRuler = aAnyRulerCB.IsChecked();
    aElem.bHorzRuler     = aHRulerCBox.IsChecked();
    aElem.bVertRuler     = aVRulerCBox.IsChecked();
    aElem.bVertRulerRight= aVRulerRightCBox.IsChecked();
    aElem.bSmoothScroll  = aSmoothCBox.IsChecked();


    sal_Bool bRet = !pOldAttr || aElem != *pOldAttr;
    if(bRet)
        bRet = 0 != rSet.Put(aElem);
    sal_uInt16 nMPos = aMetricLB.GetSelectEntryPos();
    sal_uInt16 nGlobalMetricPos = nMPos;
    if ( nMPos != aMetricLB.GetSavedValue() )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(long)aMetricLB.GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }

    nMPos = aHMetric.GetSelectEntryPos();
    if ( nMPos != aHMetric.GetSavedValue() || nMPos != nGlobalMetricPos )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(long)aHMetric.GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( FN_HSCROLL_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }
    nMPos = aVMetric.GetSelectEntryPos();
    if ( nMPos != aVMetric.GetSavedValue() || nMPos != nGlobalMetricPos )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(long)aVMetric.GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( FN_VSCROLL_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }
    return bRet;
}

IMPL_LINK(SwContentOptPage, VertRulerHdl, CheckBox*, pBox)
{
    aVRulerRightCBox.Enable(pBox->IsEnabled() && pBox->IsChecked());
    return 0;
}

IMPL_LINK( SwContentOptPage, AnyRulerHdl, CheckBox*, pBox)
{
    sal_Bool bChecked = pBox->IsChecked();
    aHRulerCBox      .Enable(bChecked);
    aHMetric         .Enable(bChecked);
    aVRulerCBox      .Enable(bChecked);
    aVMetric         .Enable(bChecked);
    VertRulerHdl(&aVRulerCBox);
    return 0;
}
/*------------------------------------------------------
 TabPage Printer additional settings
-------------------------------------------------------*/
SwAddPrinterTabPage::SwAddPrinterTabPage( Window* pParent,
                                      const SfxItemSet& rCoreSet) :
    SfxTabPage( pParent, SW_RES( TP_OPTPRINT_PAGE ), rCoreSet),
    aFL1          (this, SW_RES(FL_1)),
    aGrfCB           (this, SW_RES(CB_PGRF)),
    aCtrlFldCB       (this, SW_RES(CB_CTRLFLD)),
    aBackgroundCB    (this, SW_RES(CB_BACKGROUND)),
    aBlackFontCB     (this, SW_RES(CB_BLACK_FONT)),
    aPrintHiddenTextCB(this, SW_RES(CB_HIDDEN_TEXT)),
    aPrintTextPlaceholderCB(this, SW_RES(CB_TEXT_PLACEHOLDER)),
    aSeparatorLFL    (this, SW_RES(FL_SEP_PRT_LEFT )),
    aFL2          (this, SW_RES(FL_2)),
    aLeftPageCB      (this, SW_RES(CB_LEFTP)),
    aRightPageCB     (this, SW_RES(CB_RIGHTP)),
    aProspectCB      (this, SW_RES(CB_PROSPECT)),
    aProspectCB_RTL      (this, SW_RES(CB_PROSPECT_RTL)),
    aSeparatorRFL    (this, SW_RES(FL_SEP_PRT_RIGHT)),
    aFL3          (this, SW_RES(FL_3)),
    aNoRB            (this, SW_RES(RB_NO)),
    aOnlyRB          (this, SW_RES(RB_ONLY)),
    aEndRB           (this, SW_RES(RB_END)),
    aEndPageRB       (this, SW_RES(RB_PAGEEND)),
    aFL4          (this, SW_RES(FL_4)),
    aPrintEmptyPagesCB(this, SW_RES(CB_PRINTEMPTYPAGES)),
    aPaperFromSetupCB(this, SW_RES(CB_PAPERFROMSETUP)),
    aFaxFT           (this, SW_RES(FT_FAX)),
    aFaxLB           (this, SW_RES(LB_FAX)),
    sNone(SW_RES(ST_NONE)),
    bAttrModified( sal_False ),
    bPreview  ( sal_False )
{
    Init();
    FreeResource();
    Link aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
    aGrfCB.SetClickHdl( aLk );
    aRightPageCB.SetClickHdl( aLk );
    aLeftPageCB.SetClickHdl( aLk );
    aCtrlFldCB.SetClickHdl( aLk );
    aBackgroundCB.SetClickHdl( aLk );
    aBlackFontCB.SetClickHdl( aLk );
    aPrintHiddenTextCB.SetClickHdl( aLk );
    aPrintTextPlaceholderCB.SetClickHdl( aLk );
    aProspectCB.SetClickHdl( aLk );
    aProspectCB_RTL.SetClickHdl( aLk );
    aPaperFromSetupCB.SetClickHdl( aLk );
    aPrintEmptyPagesCB.SetClickHdl( aLk );
    aEndPageRB.SetClickHdl( aLk );
    aEndRB.SetClickHdl( aLk );
    aOnlyRB.SetClickHdl( aLk );
    aNoRB.SetClickHdl( aLk );
    aFaxLB.SetSelectHdl( LINK( this, SwAddPrinterTabPage, SelectHdl ) );

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, sal_False, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aLeftPageCB  .Hide();
        aRightPageCB .Hide();
        aPrintHiddenTextCB.Hide();
        aPrintTextPlaceholderCB.Hide();
        aProspectCB.SetPosPixel(aLeftPageCB.GetPosPixel());
        Point aPt( aRightPageCB.GetPosPixel() );
        aPt.setX(aPt.getX() + 15); // indent
        aProspectCB_RTL.SetPosPixel(aPt);

        // hide aPrintEmptyPagesCB and move everything below up accordingly
        long nDeltaY = aPaperFromSetupCB.GetPosPixel().getY() - aPrintEmptyPagesCB.GetPosPixel().getY();
        aPrintEmptyPagesCB.Hide();
        aPt = aPaperFromSetupCB.GetPosPixel();
        aPt.setY( aPt.getY() - nDeltaY );
        aPaperFromSetupCB.SetPosPixel( aPt );
        aPt = aFaxFT.GetPosPixel();
        aPt.setY( aPt.getY() - nDeltaY );
        aFaxFT.SetPosPixel( aPt );
        aPt = aFaxLB.GetPosPixel();
        aPt.setY( aPt.getY() - nDeltaY );
        aFaxLB.SetPosPixel( aPt );
    }
    aProspectCB_RTL.Disable();
    SvtCTLOptions aCTLOptions;
    aProspectCB_RTL.Show(aCTLOptions.IsCTLFontEnabled());
}

void SwAddPrinterTabPage::SetPreview(sal_Bool bPrev)
{
    bPreview = bPrev;

    if (bPreview)
    {
        aLeftPageCB.Disable();
        aRightPageCB.Disable();
        aProspectCB.Disable();
        aProspectCB_RTL.Disable();
        aFL3.Disable();
        aNoRB.Disable();
        aOnlyRB.Disable();
        aEndRB.Disable();
        aEndPageRB.Disable();
    }
}

SfxTabPage* SwAddPrinterTabPage::Create( Window* pParent,
                                       const SfxItemSet& rAttrSet )
{
    return ( new SwAddPrinterTabPage( pParent, rAttrSet ) );
}

sal_Bool    SwAddPrinterTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    if ( bAttrModified )
    {
        SwAddPrinterItem aAddPrinterAttr (FN_PARAM_ADDPRINTER);
        aAddPrinterAttr.bPrintGraphic   = aGrfCB.IsChecked();
        aAddPrinterAttr.bPrintTable     = sal_True; // always enabled since CWS printerpullgpages /*aTabCB.IsChecked();*/
        aAddPrinterAttr.bPrintDraw      = aGrfCB.IsChecked(); // UI merged with aGrfCB in CWS printerpullgpages /*aDrawCB.IsChecked()*/;
        aAddPrinterAttr.bPrintControl   = aCtrlFldCB.IsChecked();
        aAddPrinterAttr.bPrintPageBackground = aBackgroundCB.IsChecked();
        aAddPrinterAttr.bPrintBlackFont = aBlackFontCB.IsChecked();
        aAddPrinterAttr.bPrintHiddenText = aPrintHiddenTextCB.IsChecked();
        aAddPrinterAttr.bPrintTextPlaceholder = aPrintTextPlaceholderCB.IsChecked();

        aAddPrinterAttr.bPrintLeftPages     = aLeftPageCB.IsChecked();
        aAddPrinterAttr.bPrintRightPages    = aRightPageCB.IsChecked();
        aAddPrinterAttr.bPrintReverse       = sal_False; // handled by vcl itself since CWS printerpullpages /*aReverseCB.IsChecked()*/;
        aAddPrinterAttr.bPrintProspect      = aProspectCB.IsChecked();
        aAddPrinterAttr.bPrintProspectRTL   = aProspectCB_RTL.IsChecked();
        aAddPrinterAttr.bPaperFromSetup     = aPaperFromSetupCB.IsChecked();
        aAddPrinterAttr.bPrintEmptyPages    = aPrintEmptyPagesCB.IsChecked();
        aAddPrinterAttr.bPrintSingleJobs    = sal_True; // handled by vcl in new print dialog since CWS printerpullpages /*aSingleJobsCB.IsChecked()*/;

        if (aNoRB.IsChecked())  aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_NONE;
        if (aOnlyRB.IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ONLY;
        if (aEndRB.IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ENDDOC;
        if (aEndPageRB.IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ENDPAGE;

        String sFax = aFaxLB.GetSelectEntry();
        aAddPrinterAttr.sFaxName = sNone == sFax ? aEmptyStr : sFax;
        rCoreSet.Put(aAddPrinterAttr);
    }
    return bAttrModified;
}

void    SwAddPrinterTabPage::Reset( const SfxItemSet&  )
{
    const   SfxItemSet&         rSet = GetItemSet();
    const   SwAddPrinterItem*   pAddPrinterAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ADDPRINTER , sal_False,
                                    (const SfxPoolItem**)&pAddPrinterAttr ))
    {
        aGrfCB.Check(           pAddPrinterAttr->bPrintGraphic || pAddPrinterAttr->bPrintDraw );
        aCtrlFldCB.Check(       pAddPrinterAttr->bPrintControl);
        aBackgroundCB.Check(    pAddPrinterAttr->bPrintPageBackground);
        aBlackFontCB.Check(     pAddPrinterAttr->bPrintBlackFont);
        aPrintHiddenTextCB.Check( pAddPrinterAttr->bPrintHiddenText);
        aPrintTextPlaceholderCB.Check(pAddPrinterAttr->bPrintTextPlaceholder);
        aLeftPageCB.Check(      pAddPrinterAttr->bPrintLeftPages);
        aRightPageCB.Check(     pAddPrinterAttr->bPrintRightPages);
        aPaperFromSetupCB.Check(pAddPrinterAttr->bPaperFromSetup);
        aPrintEmptyPagesCB.Check(pAddPrinterAttr->bPrintEmptyPages);
        aProspectCB.Check(      pAddPrinterAttr->bPrintProspect);
        aProspectCB_RTL.Check(      pAddPrinterAttr->bPrintProspectRTL);

        aNoRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_NONE ) ;
        aOnlyRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ONLY ) ;
        aEndRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ENDDOC ) ;
        aEndPageRB.Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ENDPAGE ) ;
        aFaxLB.SelectEntry( pAddPrinterAttr->sFaxName );
    }
    if (aProspectCB.IsChecked())
    {
        aProspectCB_RTL.Enable(sal_True);
        aNoRB.Enable( sal_False );
        aOnlyRB.Enable( sal_False );
        aEndRB.Enable( sal_False );
        aEndPageRB.Enable( sal_False );
    }
    else
        aProspectCB_RTL.Enable( sal_False );
}

void    SwAddPrinterTabPage::Init()
{

}

IMPL_LINK_NOARG_INLINE_START(SwAddPrinterTabPage, AutoClickHdl)
{
    bAttrModified = sal_True;
    bool bIsProspect = aProspectCB.IsChecked();
    if (!bIsProspect)
        aProspectCB_RTL.Check( sal_False );
    aProspectCB_RTL.Enable( bIsProspect );
    aNoRB.Enable( !bIsProspect );
    aOnlyRB.Enable( !bIsProspect );
    aEndRB.Enable( !bIsProspect );
    aEndPageRB.Enable( !bIsProspect );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwAddPrinterTabPage, AutoClickHdl)

void  SwAddPrinterTabPage::SetFax( const std::vector<String>& rFaxLst )
{
    aFaxLB.InsertEntry(sNone);
    for(size_t i = 0; i < rFaxLst.size(); ++i)
    {
        aFaxLB.InsertEntry(rFaxLst[i]);
    }
    aFaxLB.SelectEntryPos(0);
}

IMPL_LINK_NOARG_INLINE_START(SwAddPrinterTabPage, SelectHdl)
{
    bAttrModified=sal_True;
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwAddPrinterTabPage, SelectHdl)

void SwAddPrinterTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pListItem,SfxBoolItem,SID_FAX_LIST,sal_False);
    SFX_ITEMSET_ARG (&aSet,pPreviewItem,SfxBoolItem,SID_PREVIEWFLAG_TYPE,sal_False);
    if (pPreviewItem)
    {
        SetPreview(pPreviewItem->GetValue());
        Reset(aSet);
    }
    if (pListItem && pListItem->GetValue())
    {
        std::vector<String> aFaxList;
        const std::vector<rtl::OUString>& rPrinters = Printer::GetPrinterQueues();
        for (unsigned int i = 0; i < rPrinters.size(); ++i)
            aFaxList.insert(aFaxList.begin(), rPrinters[i]);
        SetFax( aFaxList );
    }
}

/*--------------------------------------------------
    Tabpage Standardfonts
--------------------------------------------------*/
SwStdFontTabPage::SwStdFontTabPage( Window* pParent,
                                       const SfxItemSet& rSet ) :
    SfxTabPage( pParent, SW_RES( TP_STD_FONT ), rSet),
    aStdChrFL  (this, SW_RES(FL_STDCHR  )),
    aTypeFT(        this, SW_RES( FT_TYPE          )),

    aStandardLbl(this, SW_RES(FT_STANDARD)),
    aStandardBox(this, SW_RES(LB_STANDARD)),

    aHeightFT(        this, SW_RES( FT_SIZE          )),
    aStandardHeightLB(this, SW_RES( LB_STANDARD_SIZE )),

    aTitleLbl   (this, SW_RES(FT_TITLE   )),
    aTitleBox   (this, SW_RES(LB_TITLE   )),
    aTitleHeightLB(   this, SW_RES( LB_TITLE_SIZE    )),

    aListLbl    (this, SW_RES(FT_LIST    )),
    aListBox    (this, SW_RES(LB_LIST    )),
    aListHeightLB(    this, SW_RES( LB_LIST_SIZE     )),

    aLabelLbl   (this, SW_RES(FT_LABEL   )),
    aLabelBox   (this, SW_RES(LB_LABEL   )),
    aLabelHeightLB(   this, SW_RES( LB_LABEL_SIZE    )),

    aIdxLbl     (this, SW_RES(FT_IDX     )),
    aIdxBox     (this, SW_RES(LB_IDX     )),
    aIndexHeightLB(   this, SW_RES( LB_INDEX_SIZE    )),

    aDocOnlyCB  (this, SW_RES(CB_DOCONLY )),
    aStandardPB (this, SW_RES(PB_STANDARD)),
    pPrt(0),
    pFontList(0),
    pFontConfig(0),
    pWrtShell(0),
    eLanguage( GetAppLanguage() ),

    bListDefault(sal_False),
    bSetListDefault(sal_True),
    bLabelDefault(sal_False),
    bSetLabelDefault(sal_True),
    bIdxDefault(sal_False),
    bSetIdxDefault(sal_True),
    bDeletePrinter(sal_False),

    bListHeightDefault    (sal_False),
    bSetListHeightDefault (sal_False),
    bLabelHeightDefault   (sal_False),
    bSetLabelHeightDefault(sal_False),
    bIndexHeightDefault     (sal_False),
    bSetIndexHeightDefault  (sal_False),

    nFontGroup(FONT_GROUP_DEFAULT),

    sScriptWestern(SW_RES(ST_SCRIPT_WESTERN)),
    sScriptAsian(SW_RES(ST_SCRIPT_ASIAN)),
    sScriptComplex(SW_RES(ST_SCRIPT_CTL))
{
    FreeResource();
    aStandardPB.SetClickHdl(LINK(this, SwStdFontTabPage, StandardHdl));
    aStandardBox.SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    aListBox    .SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    aLabelBox   .SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    aIdxBox     .SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    Link aFocusLink = LINK( this, SwStdFontTabPage, LoseFocusHdl);
    aStandardBox.SetLoseFocusHdl( aFocusLink );
    aTitleBox   .SetLoseFocusHdl( aFocusLink );
    aListBox    .SetLoseFocusHdl( aFocusLink );
    aLabelBox   .SetLoseFocusHdl( aFocusLink );
    aIdxBox     .SetLoseFocusHdl( aFocusLink );

    Link aModifyHeightLink( LINK( this, SwStdFontTabPage, ModifyHeightHdl));
    aStandardHeightLB.SetModifyHdl( aModifyHeightLink );
    aTitleHeightLB.   SetModifyHdl( aModifyHeightLink );
    aListHeightLB.    SetModifyHdl( aModifyHeightLink );
    aLabelHeightLB.   SetModifyHdl( aModifyHeightLink );
    aIndexHeightLB.   SetModifyHdl( aModifyHeightLink );

    aDocOnlyCB.Check(SW_MOD()->GetModuleConfig()->IsDefaultFontInCurrDocOnly());
}

SwStdFontTabPage::~SwStdFontTabPage()
{
    if(bDeletePrinter)
        delete pPrt;
}

SfxTabPage* SwStdFontTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwStdFontTabPage(pParent, rAttrSet);
}

void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    SfxPrinter* pPrt, const String& rStyle,
                    sal_uInt16 nFontWhich)
{
    Font aFont( rStyle, Size( 0, 10 ) );
    if( pPrt )
        aFont = pPrt->GetFontMetric( aFont );
    SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(nType);
    pColl->SetFmtAttr(SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                aEmptyStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
}

void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    sal_Int32 nHeight, sal_uInt16 nFontHeightWhich)
{
    float fSize = (float)nHeight / 10;
    nHeight = CalcToUnit( fSize, SFX_MAPUNIT_TWIP );
    SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(nType);
    pColl->SetFmtAttr(SvxFontHeightItem(nHeight, 100, nFontHeightWhich));
}

sal_Bool SwStdFontTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bNotDocOnly = !aDocOnlyCB.IsChecked();
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

    bool bStandardHeightChanged = aStandardHeightLB.GetSavedValue() != aStandardHeightLB.GetText();
    bool bTitleHeightChanged = aTitleHeightLB.GetSavedValue() != aTitleHeightLB.GetText();
    bool bListHeightChanged = aListHeightLB.GetSavedValue() != aListHeightLB.GetText() && (!bListHeightDefault || !bSetListHeightDefault );
    bool bLabelHeightChanged = aLabelHeightLB.GetSavedValue() != aLabelHeightLB.GetText() && (!bLabelHeightDefault || !bSetLabelHeightDefault );
    bool bIndexHeightChanged = aIndexHeightLB.GetSavedValue() != aIndexHeightLB.GetText() && (!bIndexHeightDefault || !bSetIndexHeightDefault );
    if(bNotDocOnly)
    {
        pFontConfig->SetFontStandard(sStandard, nFontGroup);
        pFontConfig->SetFontOutline(sTitle, nFontGroup);
        pFontConfig->SetFontList(sList, nFontGroup);
        pFontConfig->SetFontCaption(sLabel, nFontGroup);
        pFontConfig->SetFontIndex(sIdx, nFontGroup);
        if(bStandardHeightChanged)
        {
            float fSize = (float)aStandardHeightLB.GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_STANDARD, nFontGroup );
        }
        if(bTitleHeightChanged)
        {
            float fSize = (float)aTitleHeightLB.GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_OUTLINE, nFontGroup );
        }
        if(bListHeightChanged)
        {
            float fSize = (float)aListHeightLB.GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_LIST, nFontGroup );
        }
        if(bLabelHeightChanged)
        {
            float fSize = (float)aLabelHeightLB.GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_CAPTION, nFontGroup );
        }
        if(bIndexHeightChanged)
        {
            float fSize = (float)aIndexHeightLB.GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_INDEX, nFontGroup );
        }
    }
    if(pWrtShell)
    {
        pWrtShell->StartAllAction();
        SfxPrinter* pPrinter = pWrtShell->getIDocumentDeviceAccess()->getPrinter( false );
        sal_Bool bMod = sal_False;
        sal_uInt16 nFontWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT);
        sal_uInt16 nFontHeightWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE);
        if(sStandard != sShellStd)
        {
            Font aFont( sStandard, Size( 0, 10 ) );
            if( pPrinter )
                aFont = pPrinter->GetFontMetric( aFont );
            pWrtShell->SetDefault(SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                                  aEmptyStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
            SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFmtAttr(nFontWhich);
            bMod = sal_True;
        }
        if(bStandardHeightChanged)
        {
            float fSize = (float)aStandardHeightLB.GetValue() / 10;
            pWrtShell->SetDefault(SvxFontHeightItem( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), 100, nFontHeightWhich ) );
            SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFmtAttr(nFontHeightWhich);
            bMod = sal_True;
        }

        if(sTitle != sShellTitle )
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_HEADLINE_BASE, pPrinter, sTitle, nFontWhich);
            bMod = sal_True;
        }
        if(bTitleHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_HEADLINE_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(aTitleHeightLB.GetValue()), nFontHeightWhich);
            bMod = sal_True;
        }
        if(sList != sShellList && (!bListDefault || !bSetListDefault ))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_NUMBUL_BASE, pPrinter, sList, nFontWhich);
            bMod = sal_True;
        }
        if(bListHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_NUMBUL_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(aListHeightLB.GetValue()), nFontHeightWhich);
            bMod = sal_True;
        }
        if(sLabel != sShellLabel && (!bLabelDefault || !bSetLabelDefault))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_LABEL, pPrinter, sLabel, nFontWhich);
            bMod = sal_True;
        }
        if(bLabelHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_LABEL,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(aLabelHeightLB.GetValue()), nFontHeightWhich);
            bMod = sal_True;
        }
        if(sIdx != sShellIndex && (!bIdxDefault || !bSetIdxDefault))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_REGISTER_BASE, pPrinter, sIdx, nFontWhich);
            bMod = sal_True;
        }
        if(bIndexHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_REGISTER_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(aIndexHeightLB.GetValue()), nFontHeightWhich);
            bMod = sal_True;
        }
        if ( bMod )
            pWrtShell->SetModified();
        pWrtShell->EndAllAction();
    }

    return sal_False;
}

void SwStdFontTabPage::Reset( const SfxItemSet& rSet)
{
    const SfxPoolItem* pLang;
    sal_uInt16 nLangSlot = nFontGroup == FONT_GROUP_DEFAULT  ? SID_ATTR_LANGUAGE :
        FONT_GROUP_CJK == nFontGroup ? SID_ATTR_CHAR_CJK_LANGUAGE : SID_ATTR_CHAR_CTL_LANGUAGE;


    if( SFX_ITEM_SET == rSet.GetItemState(nLangSlot, sal_False, &pLang))
        eLanguage = ((const SvxLanguageItem*)pLang)->GetValue();

    String sTmp(aStdChrFL.GetText());
    String sToReplace = sScriptWestern;
    if(FONT_GROUP_CJK == nFontGroup )
        sToReplace = sScriptAsian;
    else if(FONT_GROUP_CTL == nFontGroup )
        sToReplace = sScriptComplex;

    sTmp.SearchAndReplaceAscii("%1", sToReplace);
    aStdChrFL.SetText(sTmp);
    const SfxPoolItem* pItem;

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_PRINTER, sal_False, &pItem))
    {
        pPrt = (SfxPrinter*)((const SwPtrItem*)pItem)->GetValue();
    }
    else
    {
        SfxItemSet* pPrinterSet = new SfxItemSet( *rSet.GetPool(),
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );
        pPrt = new SfxPrinter(pPrinterSet);
        bDeletePrinter = sal_True;
    }
    pFontList = new FontList( pPrt );
    // #i94536# prevent duplication of font entries when 'reset' button is pressed
    if( !aStandardBox.GetEntryCount() )
    {
        // get the set of disctinct available family names
        std::set< String > aFontNames;
        int nFontNames = pPrt->GetDevFontCount();
        for( int i = 0; i < nFontNames; i++ )
        {
            FontInfo aInf( pPrt->GetDevFont( i ) );
            aFontNames.insert( aInf.GetName() );
        }

        // insert to listboxes
        for( std::set< String >::const_iterator it = aFontNames.begin();
             it != aFontNames.end(); ++it )
        {
            aStandardBox.InsertEntry( *it );
            aTitleBox   .InsertEntry( *it );
            aListBox    .InsertEntry( *it );
            aLabelBox   .InsertEntry( *it );
            aIdxBox     .InsertEntry( *it );
        }
    }
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_STDFONTS, sal_False, &pItem))
    {
         pFontConfig = (SwStdFontConfig*)((const SwPtrItem*)pItem)->GetValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_WRTSHELL, sal_False, &pItem))
    {
        pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();
    }
    String sStdBackup;
    String sOutBackup;
    String sListBackup;
    String sCapBackup;
    String sIdxBackup;
    sal_Int32 nStandardHeight = -1;
    sal_Int32 nTitleHeight = -1;
    sal_Int32 nListHeight = -1;
    sal_Int32 nLabelHeight = -1;
    sal_Int32 nIndexHeight = -1;

    if(!pWrtShell)
    {
        sStdBackup = pFontConfig->GetFontStandard(nFontGroup);
        sOutBackup = pFontConfig->GetFontOutline(nFontGroup);
        sListBackup= pFontConfig->GetFontList(nFontGroup);
        sCapBackup = pFontConfig->GetFontCaption(nFontGroup);
        sIdxBackup = pFontConfig->GetFontIndex(nFontGroup);
        nStandardHeight = pFontConfig->GetFontHeight( FONT_STANDARD, nFontGroup, eLanguage );
        nTitleHeight =    pFontConfig->GetFontHeight( FONT_OUTLINE , nFontGroup, eLanguage );
        nListHeight =     pFontConfig->GetFontHeight( FONT_LIST    , nFontGroup, eLanguage );
        nLabelHeight =    pFontConfig->GetFontHeight( FONT_CAPTION , nFontGroup, eLanguage );
        nIndexHeight =    pFontConfig->GetFontHeight( FONT_INDEX   , nFontGroup, eLanguage );
        if( nStandardHeight <= 0)
            nStandardHeight = pFontConfig->GetDefaultHeightFor( FONT_STANDARD + nFontGroup * FONT_PER_GROUP, eLanguage);
        if( nTitleHeight <= 0)
            nTitleHeight = pFontConfig->GetDefaultHeightFor( FONT_OUTLINE + nFontGroup * FONT_PER_GROUP, eLanguage);
        if( nListHeight <= 0)
            nListHeight = pFontConfig->GetDefaultHeightFor( FONT_LIST + nFontGroup * FONT_PER_GROUP, eLanguage);
        if( nLabelHeight <= 0)
            nLabelHeight = pFontConfig->GetDefaultHeightFor( FONT_CAPTION + nFontGroup * FONT_PER_GROUP, eLanguage);
        if( nIndexHeight <= 0)
            nIndexHeight = pFontConfig->GetDefaultHeightFor( FONT_INDEX + nFontGroup * FONT_PER_GROUP, eLanguage);

       aDocOnlyCB.Enable(sal_False);
    }
    else
    {
        SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
        const SvxFontItem& rFont = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        sShellStd = sStdBackup =  rFont.GetFamilyName();

        sal_uInt16 nFontHeightWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE );
        const SvxFontHeightItem& rFontHeightStandard = (const SvxFontHeightItem& )pColl->GetFmtAttr(nFontHeightWhich);
        nStandardHeight = (sal_Int32)rFontHeightStandard.GetHeight();

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_HEADLINE_BASE);
        const SvxFontItem& rFontHL = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        sShellTitle = sOutBackup = rFontHL.GetFamilyName();

        const SvxFontHeightItem& rFontHeightTitle = (const SvxFontHeightItem&)pColl->GetFmtAttr( nFontHeightWhich, sal_True );
        nTitleHeight = (sal_Int32)rFontHeightTitle.GetHeight();

        sal_uInt16 nFontWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT);
        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_NUMBUL_BASE);
        const SvxFontItem& rFontLS = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        bListDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, sal_False);
        sShellList = sListBackup = rFontLS.GetFamilyName();

        const SvxFontHeightItem& rFontHeightList = (const SvxFontHeightItem&)pColl->GetFmtAttr(nFontHeightWhich, sal_True);
        nListHeight = (sal_Int32)rFontHeightList.GetHeight();
        bListHeightDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, sal_False);


        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_LABEL);
        bLabelDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, sal_False);
        const SvxFontItem& rFontCP = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        sShellLabel = sCapBackup = rFontCP.GetFamilyName();
        const SvxFontHeightItem& rFontHeightLabel = (const SvxFontHeightItem&)pColl->GetFmtAttr(nFontHeightWhich, sal_True);
        nLabelHeight = (sal_Int32)rFontHeightLabel.GetHeight();
        bLabelHeightDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, sal_False);

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_REGISTER_BASE);
        bIdxDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, sal_False);
        const SvxFontItem& rFontIDX = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        sShellIndex = sIdxBackup = rFontIDX.GetFamilyName();
        const SvxFontHeightItem& rFontHeightIndex = (const SvxFontHeightItem&)pColl->GetFmtAttr(nFontHeightWhich, sal_True);
        nIndexHeight = (sal_Int32)rFontHeightIndex.GetHeight();
        bIndexHeightDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, sal_False);
    }
    aStandardBox.SetText(sStdBackup );
    aTitleBox   .SetText(sOutBackup );
    aListBox    .SetText(sListBackup);
    aLabelBox   .SetText(sCapBackup );
    aIdxBox     .SetText(sIdxBackup );

    FontInfo aFontInfo( pFontList->Get(sStdBackup, sStdBackup) );
    aStandardHeightLB.Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sOutBackup, sOutBackup );
    aTitleHeightLB.Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sListBackup,sListBackup);
    aListHeightLB.Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sCapBackup, sCapBackup );
    aLabelHeightLB.Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sIdxBackup, sIdxBackup );
    aIndexHeightLB.Fill( &aFontInfo, pFontList );

    aStandardHeightLB.SetValue( CalcToPoint( nStandardHeight, SFX_MAPUNIT_TWIP, 10 ) );
    aTitleHeightLB.   SetValue( CalcToPoint( nTitleHeight   , SFX_MAPUNIT_TWIP, 10 ) );
    aListHeightLB.    SetValue( CalcToPoint( nListHeight    , SFX_MAPUNIT_TWIP, 10 ) );
    aLabelHeightLB.   SetValue( CalcToPoint( nLabelHeight   , SFX_MAPUNIT_TWIP, 10 ));
    aIndexHeightLB.   SetValue( CalcToPoint( nIndexHeight   , SFX_MAPUNIT_TWIP, 10 ));

    aStandardBox.SaveValue();
    aTitleBox   .SaveValue();
    aListBox    .SaveValue();
    aLabelBox   .SaveValue();
    aIdxBox     .SaveValue();

    aStandardHeightLB.SaveValue();
    aTitleHeightLB.   SaveValue();
    aListHeightLB.    SaveValue();
    aLabelHeightLB.   SaveValue();
    aIndexHeightLB.   SaveValue();
}

IMPL_LINK_NOARG(SwStdFontTabPage, StandardHdl)
{
    sal_uInt8 nFontOffset = nFontGroup * FONT_PER_GROUP;
    aStandardBox.SetText(SwStdFontConfig::GetDefaultFor(FONT_STANDARD + nFontOffset, eLanguage));
    aTitleBox   .SetText(SwStdFontConfig::GetDefaultFor(FONT_OUTLINE  + nFontOffset, eLanguage));
    aListBox    .SetText(SwStdFontConfig::GetDefaultFor(FONT_LIST     + nFontOffset, eLanguage));
    aLabelBox   .SetText(SwStdFontConfig::GetDefaultFor(FONT_CAPTION  + nFontOffset, eLanguage));
    aIdxBox     .SetText(SwStdFontConfig::GetDefaultFor(FONT_INDEX    + nFontOffset, eLanguage));

    aStandardBox.SaveValue();
    aTitleBox   .SaveValue();
    aListBox    .SaveValue();
    aLabelBox   .SaveValue();
    aIdxBox     .SaveValue();

    aStandardHeightLB.SetValue( CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_STANDARD + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ) );
    aTitleHeightLB   .SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_OUTLINE  +
            nFontOffset, eLanguage), SFX_MAPUNIT_TWIP, 10 ));
    aListHeightLB    .SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_LIST + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    aLabelHeightLB   .SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_CAPTION  + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    aIndexHeightLB   .SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_INDEX    + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));

    return 0;
}

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
        bSetListDefault = sal_False;
    }
    else if(pBox == &aLabelBox)
    {
        bSetLabelDefault = sal_False;
    }
    else if(pBox == &aIdxBox)
    {
        bSetIdxDefault = sal_False;
    }
    return 0;
}

IMPL_LINK( SwStdFontTabPage, ModifyHeightHdl, FontSizeBox*, pBox )
{
    if(pBox == &aStandardHeightLB)
    {
        sal_Int64 nValue = pBox->GetValue(FUNIT_TWIP);
        if(bSetListHeightDefault && bListHeightDefault)
            aListHeightLB.SetValue(nValue, FUNIT_TWIP);
        if(bSetLabelHeightDefault && bLabelHeightDefault)
            aLabelHeightLB.SetValue(nValue, FUNIT_TWIP);
        if(bSetIndexHeightDefault && bIndexHeightDefault)
            aIndexHeightLB.SetValue(nValue, FUNIT_TWIP);
    }
    else if(pBox == &aListHeightLB)
    {
        bSetListHeightDefault = sal_False;
    }
    else if(pBox == &aLabelHeightLB)
    {
        bSetLabelHeightDefault = sal_False;
    }
    else if(pBox == &aIndexHeightLB)
    {
        bSetIndexHeightDefault = sal_False;
    }
    return 0;
}

IMPL_LINK( SwStdFontTabPage, LoseFocusHdl, ComboBox*, pBox )
{
    FontSizeBox* pHeightLB = 0;
    String sEntry = pBox->GetText();
    if(pBox == &aStandardBox)
    {
        pHeightLB = &aStandardHeightLB;
    }
    else if(pBox == &aTitleBox)
    {
        pHeightLB = &aTitleHeightLB;
    }
    else if(pBox == &aListBox)
    {
        pHeightLB = &aListHeightLB;
    }
    else if(pBox == &aLabelBox)
    {
        pHeightLB = &aLabelHeightLB;
    }
    else /*if(pBox == &aIdxBox)*/
    {
        pHeightLB = &aIndexHeightLB;
    }
    FontInfo aFontInfo( pFontList->Get(sEntry, sEntry) );
    pHeightLB->Fill( &aFontInfo, pFontList );

    return 0;
}


void SwStdFontTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt16Item, SID_FONTMODE_TYPE, sal_False);
    if (pFlagItem)
        SetFontMode(sal::static_int_cast< sal_uInt8, sal_uInt16>( pFlagItem->GetValue()));
}

SwTableOptionsTabPage::SwTableOptionsTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES(TP_OPTTABLE_PAGE), rSet),
    aTableFL        (this, SW_RES(FL_TABLE           )),
    aHeaderCB       (this, SW_RES(CB_HEADER          )),
    aRepeatHeaderCB (this, SW_RES(CB_REPEAT_HEADER    )),
    aDontSplitCB    (this, SW_RES(CB_DONT_SPLIT       )),
    aBorderCB       (this, SW_RES(CB_BORDER           )),

    aSeparatorFL     (this, SW_RES(FL_TABLE_SEPARATOR)),

    aTableInsertFL  (this, SW_RES(FL_TABLE_INSERT    )),
    aNumFormattingCB(this, SW_RES(CB_NUMFORMATTING   )),
    aNumFmtFormattingCB(this, SW_RES(CB_NUMFMT_FORMATTING )),
    aNumAlignmentCB (this, SW_RES(CB_NUMALIGNMENT )),

    aMoveFL(        this, SW_RES(FL_MOVE     )),
    aMoveFT(        this, SW_RES(FT_MOVE     )),
    aRowMoveFT(     this, SW_RES(FT_ROWMOVE  )),
    aRowMoveMF(     this, SW_RES(MF_ROWMOVE   )),
    aColMoveFT(     this, SW_RES(FT_COLMOVE   )),
    aColMoveMF(     this, SW_RES(MF_COLMOVE   )),

    aInsertFT(      this, SW_RES(FT_INSERT   )),
    aRowInsertFT(   this, SW_RES(FT_ROWINSERT)),
    aRowInsertMF(   this, SW_RES(MF_ROWINSERT)),
    aColInsertFT(   this, SW_RES(FT_COLINSERT)),
    aColInsertMF(   this, SW_RES(MF_COLINSERT)),

    aHandlingFT(    this, SW_RES(FT_HANDLING )),
    aFixRB(         this, SW_RES(RB_FIX       )),
    aFixPropRB(     this, SW_RES(RB_FIXPROP  )),
    aVarRB(         this, SW_RES(RB_VAR      )),
    aFixFT(         this, SW_RES(FT_FIX      )),
    aFixPropFT(     this, SW_RES(FT_FIXPROP   )),
    aVarFT(         this, SW_RES(FT_VAR       )),
    pWrtShell(0),
    bHTMLMode(sal_False)
{
    FreeResource();

    Link aLnk(LINK(this, SwTableOptionsTabPage, CheckBoxHdl));
    aNumFormattingCB.SetClickHdl(aLnk);
    aNumFmtFormattingCB.SetClickHdl(aLnk);
    aHeaderCB.SetClickHdl(aLnk);
}

SwTableOptionsTabPage::~SwTableOptionsTabPage()
{
}

SfxTabPage* SwTableOptionsTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwTableOptionsTabPage(pParent, rAttrSet );
}

sal_Bool SwTableOptionsTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bRet = sal_False;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    if(aRowMoveMF.IsModified())
        pModOpt->SetTblHMove( (sal_uInt16)aRowMoveMF.Denormalize( aRowMoveMF.GetValue(FUNIT_TWIP)));

    if(aColMoveMF.IsModified())
        pModOpt->SetTblVMove( (sal_uInt16)aColMoveMF.Denormalize( aColMoveMF.GetValue(FUNIT_TWIP)));

    if(aRowInsertMF.IsModified())
        pModOpt->SetTblHInsert((sal_uInt16)aRowInsertMF.Denormalize( aRowInsertMF.GetValue(FUNIT_TWIP)));

    if(aColInsertMF.IsModified())
        pModOpt->SetTblVInsert((sal_uInt16)aColInsertMF.Denormalize( aColInsertMF.GetValue(FUNIT_TWIP)));

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
        // the table-keyboard-mode has changed, now the current
        // table should know about that too.
        if(pWrtShell && nsSelectionType::SEL_TBL & pWrtShell->GetSelectionType())
        {
            pWrtShell->SetTblChgMode(eMode);
            static sal_uInt16 aInva[] =
                                {   FN_TABLE_MODE_FIX,
                                    FN_TABLE_MODE_FIX_PROP,
                                    FN_TABLE_MODE_VARIABLE,
                                    0
                                };
            pWrtShell->GetView().GetViewFrame()->GetBindings().Invalidate( aInva );
        }

        bRet = sal_True;
    }

    SwInsertTableOptions aInsOpts( 0, 0 );

    if (aHeaderCB.IsChecked())
        aInsOpts.mnInsMode |= tabopts::HEADLINE;

    if (aRepeatHeaderCB.IsEnabled() )
        aInsOpts.mnRowsToRepeat = aRepeatHeaderCB.IsChecked()? 1 : 0;

    if (!aDontSplitCB.IsChecked())
        aInsOpts.mnInsMode |= tabopts::SPLIT_LAYOUT;

    if (aBorderCB.IsChecked())
        aInsOpts.mnInsMode |= tabopts::DEFAULT_BORDER;

    if (aHeaderCB.GetSavedValue() != aHeaderCB.GetState() ||
        aRepeatHeaderCB.GetSavedValue() != aRepeatHeaderCB.GetState() ||
        aDontSplitCB.GetSavedValue() != aDontSplitCB.GetState() ||
        aBorderCB.GetSavedValue() != aBorderCB.GetState())
    {
        pModOpt->SetInsTblFlags(bHTMLMode, aInsOpts);
    }

    if (aNumFormattingCB.GetSavedValue() != aNumFormattingCB.GetState())
    {
        pModOpt->SetInsTblFormatNum(bHTMLMode, aNumFormattingCB.IsChecked());
        bRet = sal_True;
    }

    if (aNumFmtFormattingCB.GetSavedValue() != aNumFmtFormattingCB.GetState())
    {
        pModOpt->SetInsTblChangeNumFormat(bHTMLMode, aNumFmtFormattingCB.IsChecked());
        bRet = sal_True;
    }

    if (aNumAlignmentCB.GetSavedValue() != aNumAlignmentCB.GetState())
    {
        pModOpt->SetInsTblAlignNum(bHTMLMode, aNumAlignmentCB.IsChecked());
        bRet = sal_True;
    }

    return bRet;
}

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
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    // hide certain controls for html
    if(bHTMLMode)
    {
        aRepeatHeaderCB.Hide();
        aDontSplitCB.Hide();

        long nMoveUpBy =
        aRepeatHeaderCB.LogicToPixel( Size( 13, 13 ), MAP_APPFONT ).Height();

        Point aPos = aRepeatHeaderCB.GetPosPixel();
        aRepeatHeaderCB.SetPosPixel( Point( aPos.X(), aPos.Y() - nMoveUpBy ) );

        nMoveUpBy +=
        aDontSplitCB.LogicToPixel( Size( 13, 13 ), MAP_APPFONT ).Height();

        aPos = aBorderCB.GetPosPixel();
        aBorderCB.SetPosPixel( Point( aPos.X(), aPos.Y() - nMoveUpBy ) );
    }

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTblFlags(bHTMLMode);
    sal_uInt16 nInsTblFlags = aInsOpts.mnInsMode;

    aHeaderCB.Check(0 != (nInsTblFlags & tabopts::HEADLINE));
    aRepeatHeaderCB.Check((!bHTMLMode) && (aInsOpts.mnRowsToRepeat > 0));
    aDontSplitCB.Check(!(nInsTblFlags & tabopts::SPLIT_LAYOUT));
    aBorderCB.Check(0 != (nInsTblFlags & tabopts::DEFAULT_BORDER));

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

IMPL_LINK_NOARG(SwTableOptionsTabPage, CheckBoxHdl)
{
    aNumFmtFormattingCB.Enable(aNumFormattingCB.IsChecked());
    aNumAlignmentCB.Enable(aNumFormattingCB.IsChecked());
    aRepeatHeaderCB.Enable(aHeaderCB.IsChecked());
    return 0;
}

void SwTableOptionsTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pWrtSh,SwWrtShellItem,SID_WRT_SHELL,sal_False);
    if (pWrtSh)
        SetWrtShell(pWrtSh->GetValue());
}

SwShdwCrsrOptionsTabPage::SwShdwCrsrOptionsTabPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_OPTSHDWCRSR), rSet),
    aUnprintFL   ( this,   SW_RES( FL_NOPRINT  ) ),
    aParaCB       ( this,   SW_RES( CB_PARA      ) ),
    aSHyphCB      ( this,   SW_RES( CB_SHYPH        ) ),
    aSpacesCB     ( this,   SW_RES( CB_SPACE    ) ),
    aHSpacesCB    ( this,   SW_RES( CB_HSPACE   ) ),
    aTabCB        ( this,   SW_RES( CB_TAB      ) ),
    aBreakCB      ( this,   SW_RES( CB_BREAK        ) ),
    aCharHiddenCB     ( this,   SW_RES( CB_CHAR_HIDDEN   ) ),
    aFldHiddenCB     ( this,   SW_RES( CB_FLD_HIDDEN   ) ),
    aFldHiddenParaCB ( this,   SW_RES( CB_FLD_HIDDEN_PARA ) ),

    aSeparatorFL(   this, SW_RES( FL_SEPARATOR_SHDW)),

    aFlagFL( this, SW_RES( FL_SHDWCRSFLAG )),
    aOnOffCB( this, SW_RES( CB_SHDWCRSONOFF )),

    aFillModeFT( this, SW_RES( FT_SHDWCRSFILLMODE )),
    aFillMarginRB( this, SW_RES( RB_SHDWCRSFILLMARGIN )),
    aFillIndentRB( this, SW_RES( RB_SHDWCRSFILLINDENT )),
    aFillTabRB( this, SW_RES( RB_SHDWCRSFILLTAB )),
    aFillSpaceRB( this, SW_RES( RB_SHDWCRSFILLSPACE )),
    aCrsrOptFL   ( this, SW_RES( FL_CRSR_OPT)),
    aCrsrInProtCB( this, SW_RES( CB_ALLOW_IN_PROT )),
    m_aLayoutOptionsFL( this, SW_RES( FL_LAYOUT_OPTIONS ) ),
    m_aMathBaselineAlignmentCB( this, SW_RES( CB_MATH_BASELINE_ALIGNMENT ) ),
    m_pWrtShell( NULL )
{
    FreeResource();
    const SfxPoolItem* pItem = 0;

    SwShadowCursorItem aOpt;
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, sal_False, &pItem ))
        aOpt = *(SwShadowCursorItem*)pItem;
    aOnOffCB.Check( aOpt.IsOn() );

    sal_uInt8 eMode = aOpt.GetMode();
    aFillIndentRB.Check( FILL_INDENT == eMode );
    aFillMarginRB.Check( FILL_MARGIN == eMode );
    aFillTabRB.Check( FILL_TAB == eMode );
    aFillSpaceRB.Check( FILL_SPACE == eMode );

    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aTabCB      .Hide();
        aCharHiddenCB.Hide();
        aFldHiddenCB    .Hide();
        aFldHiddenParaCB.Hide();
        aBreakCB.SetPosPixel(aTabCB.GetPosPixel());
        aFlagFL         .Hide();
        aOnOffCB        .Hide();
        aFillModeFT     .Hide();
        aFillMarginRB   .Hide();
        aFillIndentRB   .Hide();
        aFillTabRB      .Hide();
        aFillSpaceRB    .Hide();
        aCrsrOptFL      .Hide();
        aCrsrInProtCB   .Hide();
        aSeparatorFL.Hide();
        long nWidth = aFlagFL.GetSizePixel().Width() + aFlagFL.GetPosPixel().X()
                                                        - aUnprintFL.GetPosPixel().X();
        Size aSize(aUnprintFL.GetSizePixel());
        aSize.Width() = nWidth;
        aUnprintFL.SetSizePixel(aSize);
    }
}

SwShdwCrsrOptionsTabPage::~SwShdwCrsrOptionsTabPage()
{
}

SfxTabPage* SwShdwCrsrOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SwShdwCrsrOptionsTabPage( pParent, rSet );
}


void SwShdwCrsrOptionsTabPage::PageCreated( SfxAllItemSet aSet )
{
    SFX_ITEMSET_ARG (&aSet,pWrtSh,SwWrtShellItem,SID_WRT_SHELL,sal_False);
    if (pWrtSh)
        SetWrtShell(pWrtSh->GetValue());
}


sal_Bool SwShdwCrsrOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    SwShadowCursorItem aOpt;
    aOpt.SetOn( aOnOffCB.IsChecked() );

    sal_uInt8 eMode;
    if( aFillIndentRB.IsChecked() )
        eMode= FILL_INDENT;
    else if( aFillMarginRB.IsChecked() )
        eMode = FILL_MARGIN;
    else if( aFillTabRB.IsChecked() )
        eMode = FILL_TAB;
    else
        eMode = FILL_SPACE;
    aOpt.SetMode( eMode );

    sal_Bool bRet = sal_False;
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET != rSet.GetItemState( FN_PARAM_SHADOWCURSOR, sal_False, &pItem )
        ||  ((SwShadowCursorItem&)*pItem) != aOpt )
    {
        rSet.Put( aOpt );
        bRet = sal_True;
    }

    if (m_pWrtShell) {
        m_pWrtShell->GetDoc()->set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT,
                                    m_aMathBaselineAlignmentCB.IsChecked() );
        bRet |= m_aMathBaselineAlignmentCB.IsChecked() != m_aMathBaselineAlignmentCB.GetSavedValue();
    }

    if( aCrsrInProtCB.IsChecked() != aCrsrInProtCB.GetSavedValue())
    {
        rSet.Put(SfxBoolItem(FN_PARAM_CRSR_IN_PROTECTED, aCrsrInProtCB.IsChecked()));
        bRet |= sal_True;
    }

    const SwDocDisplayItem* pOldAttr = (const SwDocDisplayItem*)
                        GetOldItem(GetItemSet(), FN_PARAM_DOCDISP);

    SwDocDisplayItem aDisp;
    if(pOldAttr)
        aDisp = *pOldAttr;

    aDisp.bParagraphEnd         = aParaCB       .IsChecked();
    aDisp.bTab                  = aTabCB        .IsChecked();
    aDisp.bSpace                = aSpacesCB     .IsChecked();
    aDisp.bNonbreakingSpace     = aHSpacesCB    .IsChecked();
    aDisp.bSoftHyphen           = aSHyphCB      .IsChecked();
    aDisp.bFldHiddenText        = aFldHiddenCB     .IsChecked();
    aDisp.bCharHiddenText       = aCharHiddenCB.IsChecked();
    aDisp.bShowHiddenPara       = aFldHiddenParaCB .IsChecked();
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
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, sal_False, &pItem ))
        aOpt = *(SwShadowCursorItem*)pItem;
    aOnOffCB.Check( aOpt.IsOn() );

    sal_uInt8 eMode = aOpt.GetMode();
    aFillIndentRB.Check( FILL_INDENT == eMode );
    aFillMarginRB.Check( FILL_MARGIN == eMode );
    aFillTabRB.Check( FILL_TAB == eMode );
    aFillSpaceRB.Check( FILL_SPACE == eMode );

    if (m_pWrtShell) {
        m_aMathBaselineAlignmentCB.Check( m_pWrtShell->GetDoc()->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT ) );
        m_aMathBaselineAlignmentCB.SaveValue();
    } else {
        m_aMathBaselineAlignmentCB.Hide();
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_CRSR_IN_PROTECTED, sal_False, &pItem ))
        aCrsrInProtCB.Check(((const SfxBoolItem*)pItem)->GetValue());
    aCrsrInProtCB.SaveValue();

    const SwDocDisplayItem* pDocDisplayAttr = 0;

    rSet.GetItemState( FN_PARAM_DOCDISP, sal_False,
                                    (const SfxPoolItem**)&pDocDisplayAttr );
    if(pDocDisplayAttr)
    {
        aParaCB     .Check  (pDocDisplayAttr->bParagraphEnd         );
        aTabCB      .Check  (pDocDisplayAttr->bTab                  );
        aSpacesCB   .Check  (pDocDisplayAttr->bSpace                );
        aHSpacesCB  .Check  (pDocDisplayAttr->bNonbreakingSpace     );
        aSHyphCB    .Check  (pDocDisplayAttr->bSoftHyphen           );
        aCharHiddenCB.Check (pDocDisplayAttr->bCharHiddenText );
        aFldHiddenCB   .Check  (pDocDisplayAttr->bFldHiddenText );
        aFldHiddenParaCB.Check (pDocDisplayAttr->bShowHiddenPara       );
        aBreakCB    .Check  (pDocDisplayAttr->bManualBreak          );
    }
}

/*--------------------------------------------------
 TabPage for Redlining
--------------------------------------------------*/
struct CharAttr
{
    sal_uInt16 nItemId;
    sal_uInt16 nAttr;
};

// Edit corresponds to Paste-attributes
static CharAttr aRedlineAttr[] =
{
    { SID_ATTR_CHAR_CASEMAP,        SVX_CASEMAP_NOT_MAPPED },
    { SID_ATTR_CHAR_WEIGHT,         WEIGHT_BOLD },
    { SID_ATTR_CHAR_POSTURE,        ITALIC_NORMAL },
    { SID_ATTR_CHAR_UNDERLINE,      UNDERLINE_SINGLE },
    { SID_ATTR_CHAR_UNDERLINE,      UNDERLINE_DOUBLE },
    { SID_ATTR_CHAR_STRIKEOUT,      STRIKEOUT_SINGLE },
    { SID_ATTR_CHAR_CASEMAP,        SVX_CASEMAP_VERSALIEN },
    { SID_ATTR_CHAR_CASEMAP,        SVX_CASEMAP_GEMEINE },
    { SID_ATTR_CHAR_CASEMAP,        SVX_CASEMAP_KAPITAELCHEN },
    { SID_ATTR_CHAR_CASEMAP,        SVX_CASEMAP_TITEL },
    { SID_ATTR_BRUSH,               0 }
};
// Items from aRedlineAttr relevant for InsertAttr: strikethrough is
// not used
static sal_uInt16 aInsertAttrMap[] = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10 };

// Items from aRedlineAttr relevant for DeleteAttr: underline and
// double underline is not used
static sal_uInt16 aDeletedAttrMap[] = { 0, 1, 2, 5, 6, 7, 8, 9, 10 };

// Items from aRedlineAttr relevant for ChangeAttr: strikethrough is
// not used
static sal_uInt16 aChangedAttrMap[] = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10 };

/*-----------------------------------------------------------------------
    Description: Preview of selection
 -----------------------------------------------------------------------*/
SwMarkPreview::SwMarkPreview( Window *pParent, const ResId& rResID ) :

    Window(pParent, rResID),
    m_aTransCol( COL_TRANSPARENT ),
    m_aMarkCol( COL_LIGHTRED ),
    nMarkPos(0)

{
    InitColors();
    SetMapMode(MAP_PIXEL);

    const Size aSz(GetOutputSizePixel());

    // Page
    aPage.SetSize(Size(aSz.Width() - 3, aSz.Height() - 3));

    sal_uLong nOutWPix = aPage.GetWidth();
    sal_uLong nOutHPix = aPage.GetHeight();

    // PrintArea
    sal_uLong nLBorder = 8;
    sal_uLong nRBorder = 8;
    sal_uLong nTBorder = 4;
    sal_uLong nBBorder = 4;

    aLeftPagePrtArea = Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));
    sal_uInt16 nWidth = (sal_uInt16)aLeftPagePrtArea.GetWidth();
    sal_uInt16 nKorr = (nWidth & 1) != 0 ? 0 : 1;
    aLeftPagePrtArea.SetSize(Size(nWidth / 2 - (nLBorder + nRBorder) / 2 + nKorr, aLeftPagePrtArea.GetHeight()));

    aRightPagePrtArea = aLeftPagePrtArea;
    aRightPagePrtArea.Move(aLeftPagePrtArea.GetWidth() + nLBorder + nRBorder + 1, 0);
}

SwMarkPreview::~SwMarkPreview()
{
}

void SwMarkPreview::InitColors( void )
{
    // m_aTransCol and m_aMarkCol are _not_ changed because they are set from outside!

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    m_aBgCol = Color( rSettings.GetWindowColor() );

    sal_Bool bHC = rSettings.GetHighContrastMode();
    m_aLineCol = bHC? SwViewOption::GetFontColor() : Color( COL_BLACK );
    m_aShadowCol = bHC? m_aBgCol : rSettings.GetShadowColor();
    m_aTxtCol = bHC? SwViewOption::GetFontColor() : Color( COL_GRAY );
    m_aPrintAreaCol = m_aTxtCol;
}

void SwMarkPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if( rDCEvt.GetType() == DATACHANGED_SETTINGS && ( rDCEvt.GetFlags() & SETTINGS_STYLE ) )
        InitColors();
}

void SwMarkPreview::Paint(const Rectangle &/*rRect*/)
{
    // draw shadow
    Rectangle aShadow(aPage);
    aShadow += Point(3, 3);
    DrawRect( aShadow, m_aShadowCol, m_aTransCol );

    // draw page
    DrawRect( aPage, m_aBgCol, m_aLineCol );

    // draw separator
    Rectangle aPageSeparator(aPage);
    aPageSeparator.SetSize(Size(2, aPageSeparator.GetHeight()));
    aPageSeparator.Move(aPage.GetWidth() / 2 - 1, 0);
    DrawRect( aPageSeparator, m_aLineCol, m_aTransCol );

    PaintPage(aLeftPagePrtArea);
    PaintPage(aRightPagePrtArea);

    Rectangle aLeftMark(Point(aPage.Left() + 2, aLeftPagePrtArea.Top() + 4), Size(aLeftPagePrtArea.Left() - 4, 2));
    Rectangle aRightMark(Point(aRightPagePrtArea.Right() + 2, aRightPagePrtArea.Bottom() - 6), Size(aLeftPagePrtArea.Left() - 4, 2));

    switch (nMarkPos)
    {
        case 1:     // left
            aRightMark.SetPos(Point(aRightPagePrtArea.Left() - 2 - aRightMark.GetWidth(), aRightMark.Top()));
            break;

        case 2:     // right
            aLeftMark.SetPos(Point(aLeftPagePrtArea.Right() + 2, aLeftMark.Top()));
            break;

        case 3:     // outside
            break;

        case 4:     // inside
            aLeftMark.SetPos(Point(aLeftPagePrtArea.Right() + 2, aLeftMark.Top()));
            aRightMark.SetPos(Point(aRightPagePrtArea.Left() - 2 - aRightMark.GetWidth(), aRightMark.Top()));
            break;

        case 0:     // none
        default:
            return;
    }
    DrawRect( aLeftMark, m_aMarkCol, m_aTransCol );
    DrawRect( aRightMark, m_aMarkCol, m_aTransCol );
}

void SwMarkPreview::PaintPage(const Rectangle &rRect)
{
    // draw PrintArea
    DrawRect(rRect, m_aTransCol, m_aPrintAreaCol );

    // draw Testparagraph
    sal_uLong nLTxtBorder = 4;
    sal_uLong nRTxtBorder = 4;
    sal_uLong nTTxtBorder = 4;

    Rectangle aTextLine = rRect;
    aTextLine.SetSize(Size(aTextLine.GetWidth(), 2));
    aTextLine.Left()    += nLTxtBorder;
    aTextLine.Right()   -= nRTxtBorder;
    aTextLine.Move(0, nTTxtBorder);

    sal_Int32 nStep;
    sal_uInt16 nLines;

    nStep = aTextLine.GetHeight() + 2;
    nLines = (sal_uInt16)(rRect.GetHeight() / (aTextLine.GetHeight() + 2)) - 1;

    // simulate text
    for (sal_uInt16 i = 0; i < nLines; ++i)
    {
        if (i == (nLines - 1))
            aTextLine.SetSize(Size(aTextLine.GetWidth() / 2, aTextLine.GetHeight()));

        if (aPage.IsInside(aTextLine))
            DrawRect(aTextLine, m_aTxtCol, m_aTransCol );

        aTextLine.Move(0, nStep);
    }
    aTextLine.Move(0, -nStep);
}

void SwMarkPreview::DrawRect(const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor)
{
    SetFillColor(rFillColor);
    SetLineColor(rLineColor);
    Window::DrawRect(rRect);
}

namespace
{
    void lcl_FillRedlineAttrListBox(
            ListBox& rLB, const AuthorCharAttr& rAttrToSelect,
            const sal_uInt16* pAttrMap, const sal_uInt16 nAttrMapSize)
    {
        for (sal_uInt16 i = 0; i != nAttrMapSize; ++i)
        {
            CharAttr& rAttr(aRedlineAttr[pAttrMap[i]]);
            rLB.SetEntryData(i, &rAttr);
            if (rAttr.nItemId == rAttrToSelect.nItemId &&
                rAttr.nAttr == rAttrToSelect.nAttr)
                rLB.SelectEntryPos(i);
        }
    }
}

SwRedlineOptionsTabPage::SwRedlineOptionsTabPage( Window* pParent,
                                                    const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_REDLINE_OPT), rSet),

    aInsertFL(          this, SW_RES( FL_TE )),

    aInsertFT(          this, SW_RES( FT_CHG_INSERT     )),
    aInsertAttrFT(          this, SW_RES( FT_INS_ATTR     )),
    aInsertLB(          this, SW_RES( LB_INS_ATTR     )),
    aInsertColorFT(     this, SW_RES( FT_INS_COL      )),
    aInsertColorLB(     this, SW_RES( LB_INS_COL      )),
    aInsertedPreviewWN( this, SW_RES( WIN_INS         )),

    aDeletedFT(         this, SW_RES( FT_CHG_DELETE     )),
    aDeletedAttrFT(         this, SW_RES( FT_DEL_ATTR     )),
    aDeletedLB(         this, SW_RES( LB_DEL_ATTR     )),
    aDeletedColorFT(    this, SW_RES( FT_DEL_COL      )),
    aDeletedColorLB(    this, SW_RES( LB_DEL_COL      )),
    aDeletedPreviewWN(  this, SW_RES( WIN_DEL         )),

    aChangedFT(         this, SW_RES( FT_CHG_CHANGE     )),
    aChangedAttrFT(         this, SW_RES( FT_CHG_ATTR     )),
    aChangedLB(         this, SW_RES( LB_CHG_ATTR     )),
    aChangedColorFT(    this, SW_RES( FT_CHG_COL      )),
    aChangedColorLB(    this, SW_RES( LB_CHG_COL      )),
    aChangedPreviewWN(  this, SW_RES( WIN_CHG         )),

    aChangedFL          ( this, SW_RES( FL_LC )),

    aMarkPosFT          ( this, SW_RES( FT_MARKPOS )),
    aMarkPosLB          ( this, SW_RES( LB_MARKPOS )),
    aMarkColorFT        ( this, SW_RES( FT_LC_COL )),
    aMarkColorLB        ( this, SW_RES( LB_LC_COL )),
    aMarkPreviewWN      ( this, SW_RES( WIN_MARK )),

    sAuthor             ( SW_RES( STR_AUTHOR )),
    sNone               ( SW_RES( STR_NOTHING ))

{
    FreeResource();

    for(sal_uInt16 i = 0; i < aInsertLB.GetEntryCount(); i++)
    {
        String sEntry(aInsertLB.GetEntry(i));
        aDeletedLB.InsertEntry(sEntry);
        aChangedLB.InsertEntry(sEntry);
    };

    // remove strikethrough from insert and change and underline + double
    // underline from delete
    aInsertLB.RemoveEntry(5);
    aChangedLB.RemoveEntry(5);
    aDeletedLB.RemoveEntry(4);
    aDeletedLB.RemoveEntry(3);

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

    //solution: set different accessible name of four color box
    aInsertColorLB.SetAccessibleName(::rtl::OUString( aInsertFT.GetDisplayText()) + ::rtl::OUString(aInsertColorFT.GetDisplayText()));
    aDeletedColorLB.SetAccessibleName(::rtl::OUString( aDeletedFT.GetDisplayText()) + ::rtl::OUString( aDeletedColorFT.GetDisplayText()));
    aChangedColorLB.SetAccessibleName(::rtl::OUString( aChangedFT.GetDisplayText()) + ::rtl::OUString( aChangedColorFT.GetDisplayText()));
    aMarkColorLB.SetAccessibleName(::rtl::OUString( aMarkPosFT.GetDisplayText()) + ::rtl::OUString( aMarkColorFT.GetDisplayText()));
}

SwRedlineOptionsTabPage::~SwRedlineOptionsTabPage()
{
}

SfxTabPage* SwRedlineOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet)
{
    return new SwRedlineOptionsTabPage( pParent, rSet );
}

sal_Bool SwRedlineOptionsTabPage::FillItemSet( SfxItemSet& )
{
    CharAttr *pAttr;
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    AuthorCharAttr aInsertedAttr;
    AuthorCharAttr aDeletedAttr;
    AuthorCharAttr aChangedAttr;

    AuthorCharAttr aOldInsertAttr(pOpt->GetInsertAuthorAttr());
    AuthorCharAttr aOldDeletedAttr(pOpt->GetDeletedAuthorAttr());
    AuthorCharAttr aOldChangedAttr(pOpt->GetFormatAuthorAttr());

    ColorData nOldMarkColor = pOpt->GetMarkAlignColor().GetColor();
    sal_uInt16 nOldMarkMode = pOpt->GetMarkAlignMode();

    sal_uInt16 nPos = aInsertLB.GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)aInsertLB.GetEntryData(nPos);
        aInsertedAttr.nItemId = pAttr->nItemId;
        aInsertedAttr.nAttr = pAttr->nAttr;

        nPos = aInsertColorLB.GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aInsertedAttr.nColor = COL_NONE;
                break;
            case 1:
            case LISTBOX_ENTRY_NOTFOUND:
                aInsertedAttr.nColor = COL_TRANSPARENT;
                break;
            default:
                aInsertedAttr.nColor = aInsertColorLB.GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetInsertAuthorAttr(aInsertedAttr);
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
        case 0: nPos = text::HoriOrientation::NONE;       break;
        case 1: nPos = text::HoriOrientation::LEFT;       break;
        case 2: nPos = text::HoriOrientation::RIGHT;      break;
        case 3: nPos = text::HoriOrientation::OUTSIDE;    break;
        case 4: nPos = text::HoriOrientation::INSIDE;     break;
    }
    pOpt->SetMarkAlignMode(nPos);

    pOpt->SetMarkAlignColor(aMarkColorLB.GetSelectEntryColor());

    if (!(aInsertedAttr == aOldInsertAttr) ||
        !(aDeletedAttr == aOldDeletedAttr) ||
        !(aChangedAttr == aOldChangedAttr) ||
       nOldMarkColor != pOpt->GetMarkAlignColor().GetColor() ||
       nOldMarkMode != pOpt->GetMarkAlignMode())
    {
        // update all documents
        TypeId aType(TYPE(SwDocShell));
        SwDocShell* pDocShell = (SwDocShell*)SfxObjectShell::GetFirst(&aType);

        while( pDocShell )
        {
            pDocShell->GetWrtShell()->UpdateRedlineAttr();
            pDocShell = (SwDocShell*)SfxObjectShell::GetNext(*pDocShell, &aType);
        }
    }

    return sal_False;
}

void SwRedlineOptionsTabPage::Reset( const SfxItemSet&  )
{
    const SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    const AuthorCharAttr &rInsertAttr = pOpt->GetInsertAuthorAttr();
    const AuthorCharAttr &rDeletedAttr = pOpt->GetDeletedAuthorAttr();
    const AuthorCharAttr &rChangedAttr = pOpt->GetFormatAuthorAttr();

    // initialise preview
    InitFontStyle(aInsertedPreviewWN);
    InitFontStyle(aDeletedPreviewWN);
    InitFontStyle(aChangedPreviewWN);

    // initialise colour list box
    String sColor;
    aInsertColorLB.SetUpdateMode(sal_False);
    aDeletedColorLB.SetUpdateMode(sal_False);
    aChangedColorLB.SetUpdateMode(sal_False);
    aMarkColorLB.SetUpdateMode(sal_False);

    aInsertColorLB.InsertEntry(sNone);
    aDeletedColorLB.InsertEntry(sNone);
    aChangedColorLB.InsertEntry(sNone);

    aInsertColorLB.InsertEntry(sAuthor);
    aDeletedColorLB.InsertEntry(sAuthor);
    aChangedColorLB.InsertEntry(sAuthor);

    XColorListRef pColorLst = XColorList::GetStdColorList();
    sal_uInt16 i;
    for( i = 0; i < pColorLst->Count(); ++i )
    {
        XColorEntry* pEntry = pColorLst->GetColor( i );
        Color aColor = pEntry->GetColor();
        String sName = pEntry->GetName();

        aInsertColorLB.InsertEntry( aColor, sName );
        aDeletedColorLB.InsertEntry( aColor, sName );
        aChangedColorLB.InsertEntry( aColor, sName );
        aMarkColorLB.InsertEntry( aColor, sName );
    }
    aInsertColorLB.SetUpdateMode( sal_True );
    aDeletedColorLB.SetUpdateMode( sal_True );
    aChangedColorLB.SetUpdateMode( sal_True );
    aMarkColorLB.SetUpdateMode( sal_True );

    ColorData nColor = rInsertAttr.nColor;

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

    aInsertLB.SelectEntryPos(0);
    aDeletedLB.SelectEntryPos(0);
    aChangedLB.SelectEntryPos(0);

    lcl_FillRedlineAttrListBox(aInsertLB, rInsertAttr, aInsertAttrMap,
            sizeof(aInsertAttrMap) / sizeof(sal_uInt16));
    lcl_FillRedlineAttrListBox(aDeletedLB, rDeletedAttr, aDeletedAttrMap,
            sizeof(aDeletedAttrMap) / sizeof(sal_uInt16));
    lcl_FillRedlineAttrListBox(aChangedLB, rChangedAttr, aChangedAttrMap,
            sizeof(aChangedAttrMap) / sizeof(sal_uInt16));

    sal_uInt16 nPos = 0;
    switch (pOpt->GetMarkAlignMode())
    {
        case text::HoriOrientation::NONE:     nPos = 0;   break;
        case text::HoriOrientation::LEFT:     nPos = 1;   break;
        case text::HoriOrientation::RIGHT:    nPos = 2;   break;
        case text::HoriOrientation::OUTSIDE:  nPos = 3;   break;
        case text::HoriOrientation::INSIDE:   nPos = 4;   break;
    }
    aMarkPosLB.SelectEntryPos(nPos);

    // show settings in preview
    AttribHdl(&aInsertLB);
    ColorHdl(&aInsertColorLB);
    AttribHdl(&aDeletedLB);
    ColorHdl(&aInsertColorLB);
    AttribHdl(&aChangedLB);
    ColorHdl(&aChangedColorLB);

    ChangedMaskPrevHdl();
}

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

    SvxFont&    rFont = pPrev->GetFont();
    SvxFont&    rCJKFont = pPrev->GetCJKFont();

    rFont.SetWeight(WEIGHT_NORMAL);
    rCJKFont.SetWeight(WEIGHT_NORMAL);
    rFont.SetItalic(ITALIC_NONE);
    rCJKFont.SetItalic(ITALIC_NONE);
    rFont.SetUnderline(UNDERLINE_NONE);
    rCJKFont.SetUnderline(UNDERLINE_NONE);
    rFont.SetStrikeout(STRIKEOUT_NONE);
    rCJKFont.SetStrikeout(STRIKEOUT_NONE);
    rFont.SetCaseMap(SVX_CASEMAP_NOT_MAPPED);
    rCJKFont.SetCaseMap(SVX_CASEMAP_NOT_MAPPED);

    sal_uInt16      nPos = pColorLB->GetSelectEntryPos();

    switch( nPos )
    {
        case 0:
            rFont.SetColor( Color( COL_BLACK ) );
            rCJKFont.SetColor( Color( COL_BLACK ) );
            break;
        case 1:
        case LISTBOX_ENTRY_NOTFOUND:
            rFont.SetColor( Color( COL_RED ) );
            rCJKFont.SetColor( Color( COL_RED ) );
            break;
        default:
            rFont.SetColor( pColorLB->GetEntryColor( nPos ) );
            rCJKFont.SetColor( pColorLB->GetEntryColor( nPos ) );
            break;
    }

    nPos = pLB->GetSelectEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;

    CharAttr*   pAttr = ( CharAttr* ) pLB->GetEntryData( nPos );
    //switch off preview background color
    pPrev->ResetColor();
    switch (pAttr->nItemId)
    {
        case SID_ATTR_CHAR_WEIGHT:
            rFont.SetWeight( ( FontWeight ) pAttr->nAttr );
            rCJKFont.SetWeight( ( FontWeight ) pAttr->nAttr );
            break;

        case SID_ATTR_CHAR_POSTURE:
            rFont.SetItalic( ( FontItalic ) pAttr->nAttr );
            rCJKFont.SetItalic( ( FontItalic ) pAttr->nAttr );
            break;

        case SID_ATTR_CHAR_UNDERLINE:
            rFont.SetUnderline( ( FontUnderline ) pAttr->nAttr );
            rCJKFont.SetUnderline( ( FontUnderline ) pAttr->nAttr );
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            rFont.SetStrikeout( ( FontStrikeout ) pAttr->nAttr );
            rCJKFont.SetStrikeout( ( FontStrikeout ) pAttr->nAttr );
            break;

        case SID_ATTR_CHAR_CASEMAP:
            rFont.SetCaseMap( ( SvxCaseMap ) pAttr->nAttr );
            rCJKFont.SetCaseMap( ( SvxCaseMap ) pAttr->nAttr );
            break;

        case SID_ATTR_BRUSH:
        {
            nPos = pColorLB->GetSelectEntryPos();
            if( nPos )
                pPrev->SetColor( pColorLB->GetSelectEntryColor() );
            else
                pPrev->SetColor( Color( COL_LIGHTGRAY ) );

            rFont.SetColor( Color( COL_BLACK ) );
            rCJKFont.SetColor( Color( COL_BLACK ) );
        }
        break;
    }

    pPrev->Invalidate();

    return 0;
}

IMPL_LINK( SwRedlineOptionsTabPage, ColorHdl, ColorListBox *, pColorLB )
{
    SvxFontPrevWindow *pPrev = 0;
    ListBox* pLB;

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

    SvxFont&    rFont = pPrev->GetFont();
    SvxFont&    rCJKFont = pPrev->GetCJKFont();
    sal_uInt16      nPos = pLB->GetSelectEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;

    CharAttr*   pAttr = ( CharAttr* ) pLB->GetEntryData( nPos );

    if( pAttr->nItemId == SID_ATTR_BRUSH )
    {
        rFont.SetColor( Color( COL_BLACK ) );
        rCJKFont.SetColor( Color( COL_BLACK ) );
        nPos = pColorLB->GetSelectEntryPos();
        if( nPos && nPos != LISTBOX_ENTRY_NOTFOUND )
            pPrev->SetColor( pColorLB->GetSelectEntryColor() );
        else
            pPrev->SetColor( Color( COL_LIGHTGRAY ) );
    }
    else
    {
        nPos = pColorLB->GetSelectEntryPos();

        switch( nPos )
        {
            case 0:
                rFont.SetColor( Color( COL_BLACK ) );
                rCJKFont.SetColor( Color( COL_BLACK ) );
                break;
            case 1:
            case LISTBOX_ENTRY_NOTFOUND:
                rFont.SetColor( Color( COL_RED ) );
                rCJKFont.SetColor( Color( COL_RED ) );
                break;
            default:
                rFont.SetColor( pColorLB->GetEntryColor( nPos ) );
                rCJKFont.SetColor( pColorLB->GetEntryColor( nPos ) );
                break;
        }
    }

    pPrev->Invalidate();

    return 0;
}

IMPL_LINK_NOARG(SwRedlineOptionsTabPage, ChangedMaskPrevHdl)
{
    aMarkPreviewWN.SetMarkPos(aMarkPosLB.GetSelectEntryPos());
    aMarkPreviewWN.SetColor(aMarkColorLB.GetSelectEntryColor().GetColor());

    aMarkPreviewWN.Invalidate();

    return 0;
}

void SwRedlineOptionsTabPage::InitFontStyle(SvxFontPrevWindow& rExampleWin)
{
    const AllSettings&  rAllSettings = Application::GetSettings();
    LanguageType        eLangType = rAllSettings.GetUILanguage();
    Color               aBackCol( rAllSettings.GetStyleSettings().GetWindowColor() );
    SvxFont&            rFont = rExampleWin.GetFont();
    SvxFont&            rCJKFont = rExampleWin.GetCJKFont();
    SvxFont&            rCTLFont = rExampleWin.GetCTLFont();

    Font                aFont( OutputDevice::GetDefaultFont( DEFAULTFONT_SERIF, eLangType,
                                                        DEFAULTFONT_FLAGS_ONLYONE, &rExampleWin ) );
    Font                aCJKFont( OutputDevice::GetDefaultFont( DEFAULTFONT_CJK_TEXT, eLangType,
                                                        DEFAULTFONT_FLAGS_ONLYONE, &rExampleWin ) );
    Font                aCTLFont( OutputDevice::GetDefaultFont( DEFAULTFONT_CTL_TEXT, eLangType,
                                                        DEFAULTFONT_FLAGS_ONLYONE, &rExampleWin ) );
    const Size          aDefSize( 0, 12 );
    aFont.SetSize( aDefSize );
    aCJKFont.SetSize( aDefSize );
    aCTLFont.SetSize( aDefSize );

    aFont.SetFillColor( aBackCol );
    aCJKFont.SetFillColor( aBackCol );
    aCTLFont.SetFillColor( aBackCol );

    aFont.SetWeight( WEIGHT_NORMAL );
    aCJKFont.SetWeight( WEIGHT_NORMAL );
    aCTLFont.SetWeight( WEIGHT_NORMAL );

    rFont = aFont;
    rCJKFont = aCJKFont;
    rCTLFont = aCTLFont;

    const Size          aNewSize( 0, rExampleWin.GetOutputSize().Height() * 2 / 3 );
    rFont.SetSize( aNewSize );
    rCJKFont.SetSize( aNewSize );

    rExampleWin.SetFont( rFont, rCJKFont,rCTLFont );

    rExampleWin.UseResourceText();

    Wallpaper           aWall( aBackCol );
    rExampleWin.SetBackground( aWall );
    rExampleWin.Invalidate();
}


//----------------------------------------------------------
SwCompareOptionsTabPage::SwCompareOptionsTabPage(  Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage( pParent, SW_RES( TP_COMPARISON_OPT ), rSet ),

    aComparisonFL(	this, SW_RES( FL_CMP ) ),
    aAutoRB(		this, SW_RES( RB_AUTO ) ),
    aWordRB(		this, SW_RES( RB_WORD ) ),
    aCharRB(		this, SW_RES( RB_CHAR ) ),
    aSettingsFL(	this, SW_RES( FL_SET ) ),
    aRsidCB(		this, SW_RES( CB_RSID) ),
    aIgnoreCB(		this, SW_RES( CB_IGNORE ) ),
    aLenNF(			this, SW_RES( NF_LEN ) )
{
    FreeResource();
    Link aLnk( LINK( this, SwCompareOptionsTabPage, ComparisonHdl ) );
    aAutoRB.SetClickHdl( aLnk );
    aWordRB.SetClickHdl( aLnk );
    aCharRB.SetClickHdl( aLnk );

    aIgnoreCB.SetClickHdl( LINK( this, SwCompareOptionsTabPage, IgnoreHdl) );
}

SwCompareOptionsTabPage::~SwCompareOptionsTabPage()
{
}

SfxTabPage* SwCompareOptionsTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SwCompareOptionsTabPage( pParent, rAttrSet );
}

sal_Bool SwCompareOptionsTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bRet = sal_False;
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    if( aAutoRB.IsChecked() != aAutoRB.GetSavedValue() ||
        aWordRB.IsChecked() != aWordRB.GetSavedValue() ||
        aCharRB.IsChecked() != aCharRB.GetSavedValue() )
    {
        SvxCompareMode eCmpMode = SVX_CMP_AUTO;

        if ( aAutoRB.IsChecked() ) eCmpMode = SVX_CMP_AUTO;
        if ( aWordRB.IsChecked() ) eCmpMode = SVX_CMP_BY_WORD;
        if ( aCharRB.IsChecked() ) eCmpMode = SVX_CMP_BY_CHAR;

        pOpt->SetCompareMode( eCmpMode );
        bRet = sal_True;
    }

    if( aRsidCB.IsChecked() != aRsidCB.GetSavedValue() )
    {
        pOpt->SetUseRsid( aRsidCB.IsChecked() );
        bRet = sal_True;
    }

    if( aIgnoreCB.IsChecked() != aIgnoreCB.GetSavedValue() )
    {
        pOpt->SetIgnorePieces( aIgnoreCB.IsChecked() );
        bRet = sal_True;
    }

    if( aLenNF.IsModified() )
    {
        pOpt->SetPieceLen( aLenNF.GetValue() );
        bRet = sal_True;
    }

    return bRet;
}

void SwCompareOptionsTabPage::Reset( const SfxItemSet& )
{
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    SvxCompareMode eCmpMode = pOpt->GetCompareMode();
    if( eCmpMode == SVX_CMP_AUTO )
    {
        aAutoRB.Check();
        aSettingsFL.Disable();
        aRsidCB.Disable();
        aIgnoreCB.Disable();
        aLenNF.Disable();
    }
    else if( eCmpMode == SVX_CMP_BY_WORD )
    {
        aWordRB.Check();
        aSettingsFL.Enable();
        aRsidCB.Enable();
        aIgnoreCB.Enable();
        aLenNF.Enable();
    }
    else if( eCmpMode == SVX_CMP_BY_CHAR)
    {
        aCharRB.Check();
        aSettingsFL.Enable();
        aRsidCB.Enable();
        aIgnoreCB.Enable();
        aLenNF.Enable();
    }
    aAutoRB.SaveValue();
    aWordRB.SaveValue();
    aCharRB.SaveValue();

    aRsidCB.Check( pOpt->IsUseRsid() );
    aRsidCB.SaveValue();

    aIgnoreCB.Check( pOpt->IsIgnorePieces() );
    aIgnoreCB.SaveValue();

    aLenNF.Enable( aIgnoreCB.IsChecked() && eCmpMode );

    aLenNF.SetValue( pOpt->GetPieceLen() );
    aLenNF.SaveValue();
}

IMPL_LINK_NOARG(SwCompareOptionsTabPage, ComparisonHdl)
{
    bool bChecked = !aAutoRB.IsChecked();
    aSettingsFL.Enable( bChecked );
    aRsidCB.Enable( bChecked );
    aIgnoreCB.Enable( bChecked );
    aLenNF.Enable( bChecked && aIgnoreCB.IsChecked() );

    return 0;
}

IMPL_LINK_NOARG(SwCompareOptionsTabPage, IgnoreHdl)
{
    aLenNF.Enable( aIgnoreCB.IsChecked() );
    return 0;
}

#ifdef DBG_UTIL

void lcl_SetPosSize(Window& rWin, Point aPos, Size aSize)
{
    aPos = rWin.LogicToPixel(aPos, MAP_APPFONT);
    aSize = rWin.OutputDevice::LogicToPixel(aSize, MAP_APPFONT);
    rWin.SetPosSizePixel(aPos, aSize);
}

SwTestTabPage::SwTestTabPage( Window* pParent,
                                      const SfxItemSet& rCoreSet) :
    SfxTabPage( pParent, WB_HIDE, rCoreSet),
    aTestFL        ( this, WB_GROUP ),
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
    bAttrModified( sal_False )
{
    lcl_SetPosSize(*this,       Point(0,0), Size(260 , 135));
    lcl_SetPosSize(aTestFL,   Point(6,2), Size(209,8));
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

    aTestFL.SetText(rtl::OUString("Settings only for testpurposes"));
    aTest1CBox.SetText(rtl::OUString("unused"));
    aTest2CBox.SetText(rtl::OUString("dynamic"));
    aTest3CBox.SetText(rtl::OUString("No calm"));
    aTest4CBox.SetText(rtl::OUString("WYSIWYG debug"));
    aTest5CBox.SetText(rtl::OUString("No idle format"));
    aTest6CBox.SetText(rtl::OUString("No screen adj"));
    aTest7CBox.SetText(rtl::OUString("win format"));
    aTest8CBox.SetText(rtl::OUString("No Scroll"));
    aTest9CBox.SetText(rtl::OUString("DrawingLayerNotLoading"));
    aTest10CBox.SetText(rtl::OUString("AutoFormat by Input"));
    aTestFL.Show();
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

SfxTabPage* SwTestTabPage::Create( Window* pParent,
                                       const SfxItemSet& rAttrSet )
{
    return ( new SwTestTabPage( pParent, rAttrSet ) );
}

sal_Bool    SwTestTabPage::FillItemSet( SfxItemSet& rCoreSet )
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

void SwTestTabPage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();
    const SwTestItem* pTestAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SWTEST , sal_False,
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

IMPL_LINK_NOARG_INLINE_START(SwTestTabPage, AutoClickHdl)
{
    bAttrModified = sal_True;
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwTestTabPage, AutoClickHdl)


#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
