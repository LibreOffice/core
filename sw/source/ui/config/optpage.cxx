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
#include <cfgitems.hxx>
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
#include <optdlg.hrc>
#include <swwrtshitem.hxx>
#include <unomid.h>

#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxenum.hxx>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <svl/slstitm.hxx>
#include <svl/ctloptions.hxx>
#include <svl/eitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/xtable.hxx>
#include <svx/dlgutil.hxx>
#include <svx/strarray.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>


using namespace ::com::sun::star;

/*--------------------------------------------------------
 Tools->Options->Writer->View
 Tools->Options->Writer/Web->View
--------------------------------------------------------- */
SwContentOptPage::SwContentOptPage( Window* pParent,
                                      const SfxItemSet& rCoreSet ) :
    SfxTabPage(pParent, "ViewOptionsPage",
               "modules/swriter/ui/viewoptionspage.ui", rCoreSet)
{
    get (m_pCrossCB, "helplines");

    get (m_pHScrollBox, "hscrollbar");
    get (m_pVScrollBox, "vscrollbar");
    get (m_pAnyRulerCB, "ruler");
    get (m_pHRulerCBox, "hruler");
    get (m_pHMetric, "hrulercombobox");
    get (m_pVRulerCBox, "vruler");
    get (m_pVRulerRightCBox, "vrulerright");
    get (m_pVMetric, "vrulercombobox");
    get (m_pSmoothCBox, "smoothscroll");

    get (m_pGrfCB, "graphics");
    get (m_pTblCB, "tables");
    get (m_pDrwCB, "drawings");
    get (m_pFldNameCB, "fieldcodes");
    get (m_pPostItCB, "comments");

    get (m_pSettingsFrame, "settingsframe");
    get (m_pSettingsLabel, "settingslabel");
    get (m_pMetricLabel, "measureunitlabel");
    get (m_pMetricLB, "measureunit");

    /* This part is visible only with Writer/Web->View dialogue. */
    const SfxPoolItem* pItem;
    if (! (SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem )
           && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON))
    {
        m_pSettingsFrame->Hide();
        m_pSettingsLabel->Hide();
        m_pMetricLabel->Hide();
        m_pMetricLB->Hide();
    }

    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsVerticalTextEnabled() )
        m_pVRulerRightCBox->Hide();
    m_pVRulerCBox->SetClickHdl(LINK(this, SwContentOptPage, VertRulerHdl ));
    m_pAnyRulerCB->SetClickHdl(LINK(this, SwContentOptPage, AnyRulerHdl));

    SvxStringArray aMetricArr( SW_RES( STR_ARR_METRIC ) );
    for ( sal_uInt16 i = 0; i < aMetricArr.Count(); ++i )
    {
        OUString sMetric = aMetricArr.GetStringByPos( i );
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
                   sal_uInt16 nPos = m_pMetricLB->InsertEntry( sMetric );
                   m_pMetricLB->SetEntryData( nPos, (void*)(sal_IntPtr)eFUnit );
                   m_pHMetric->InsertEntry( sMetric );
                   m_pHMetric->SetEntryData( nPos, (void*)(sal_IntPtr)eFUnit );
                }
                // a vertical ruler has not the 'character' unit
                if ( eFUnit != FUNIT_CHAR )
                {
                   sal_uInt16 nPos = m_pVMetric->InsertEntry( sMetric );
                   m_pVMetric->SetEntryData( nPos, (void*)(sal_IntPtr)eFUnit );
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

static void lcl_SelectMetricLB(ListBox* rMetric, sal_uInt16 nSID, const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if( rSet.GetItemState( nSID, false, &pItem ) >= SFX_ITEM_AVAILABLE )
    {
        FieldUnit eFieldUnit = (FieldUnit)((SfxUInt16Item*)pItem)->GetValue();
        for ( sal_uInt16 i = 0; i < rMetric->GetEntryCount(); ++i )
        {
            if ( (int)(sal_IntPtr)rMetric->GetEntryData( i ) == (int)eFieldUnit )
            {
                rMetric->SelectEntryPos( i );
                break;
            }
        }
    }
    rMetric->SaveValue();
}

void SwContentOptPage::Reset(const SfxItemSet& rSet)
{
    const SwElemItem* pElemAttr = 0;

    rSet.GetItemState( FN_PARAM_ELEM , false,
                                    (const SfxPoolItem**)&pElemAttr );
    if(pElemAttr)
    {
        m_pTblCB->Check (pElemAttr->bTable);
        m_pGrfCB->Check (pElemAttr->bGraphic);
        m_pDrwCB->Check (pElemAttr->bDrawing);
        m_pFldNameCB->Check (pElemAttr->bFieldName);
        m_pPostItCB->Check (pElemAttr->bNotes);
        m_pCrossCB->Check (pElemAttr->bCrosshair);
        m_pHScrollBox->Check (pElemAttr->bHorzScrollbar);
        m_pVScrollBox->Check (pElemAttr->bVertScrollbar);
        m_pAnyRulerCB->Check (pElemAttr->bAnyRuler);
        m_pHRulerCBox->Check (pElemAttr->bHorzRuler);
        m_pVRulerCBox->Check (pElemAttr->bVertRuler);
        m_pVRulerRightCBox->Check (pElemAttr->bVertRulerRight);
        m_pSmoothCBox->Check (pElemAttr->bSmoothScroll);
    }
    m_pMetricLB->SetNoSelection();
    lcl_SelectMetricLB(m_pMetricLB, SID_ATTR_METRIC, rSet);
    lcl_SelectMetricLB(m_pHMetric, FN_HSCROLL_METRIC, rSet);
    lcl_SelectMetricLB(m_pVMetric, FN_VSCROLL_METRIC, rSet);
    AnyRulerHdl(m_pAnyRulerCB);
}

sal_Bool SwContentOptPage::FillItemSet(SfxItemSet& rSet)
{
    const SwElemItem*   pOldAttr = (const SwElemItem*)
                        GetOldItem(GetItemSet(), FN_PARAM_ELEM);

    SwElemItem aElem;
    if(pOldAttr)
        aElem = *pOldAttr;
    aElem.bTable                = m_pTblCB->IsChecked();
    aElem.bGraphic              = m_pGrfCB->IsChecked();
    aElem.bDrawing              = m_pDrwCB->IsChecked();
    aElem.bFieldName            = m_pFldNameCB->IsChecked();
    aElem.bNotes                = m_pPostItCB->IsChecked();
    aElem.bCrosshair            = m_pCrossCB->IsChecked();
    aElem.bHorzScrollbar        = m_pHScrollBox->IsChecked();
    aElem.bVertScrollbar        = m_pVScrollBox->IsChecked();
    aElem.bAnyRuler             = m_pAnyRulerCB->IsChecked();
    aElem.bHorzRuler            = m_pHRulerCBox->IsChecked();
    aElem.bVertRuler            = m_pVRulerCBox->IsChecked();
    aElem.bVertRulerRight       = m_pVRulerRightCBox->IsChecked();
    aElem.bSmoothScroll         = m_pSmoothCBox->IsChecked();


    sal_Bool bRet = !pOldAttr || aElem != *pOldAttr;
    if(bRet)
        bRet = 0 != rSet.Put(aElem);

    sal_uInt16 nMPos = m_pMetricLB->GetSelectEntryPos();
    sal_uInt16 nGlobalMetricPos = nMPos;
    if ( nMPos != m_pMetricLB->GetSavedValue() )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pMetricLB->GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }

    nMPos = m_pHMetric->GetSelectEntryPos();
    if ( nMPos != m_pHMetric->GetSavedValue() || nMPos != nGlobalMetricPos )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pHMetric->GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( FN_HSCROLL_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }
    nMPos = m_pVMetric->GetSelectEntryPos();
    if ( nMPos != m_pVMetric->GetSavedValue() || nMPos != nGlobalMetricPos )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pVMetric->GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( FN_VSCROLL_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }
    return bRet;
}

IMPL_LINK(SwContentOptPage, VertRulerHdl, CheckBox*, pBox)
{
    m_pVRulerRightCBox->Enable(pBox->IsEnabled() && pBox->IsChecked());
    return 0;
}

IMPL_LINK( SwContentOptPage, AnyRulerHdl, CheckBox*, pBox)
{
    sal_Bool bChecked = pBox->IsChecked();
    m_pHRulerCBox->Enable(bChecked);
    m_pHMetric->Enable(bChecked);
    m_pVRulerCBox->Enable(bChecked);
    m_pVMetric->Enable(bChecked);
    VertRulerHdl(m_pVRulerCBox);
    return 0;
}
/*------------------------------------------------------
 TabPage Printer additional settings
-------------------------------------------------------*/
SwAddPrinterTabPage::SwAddPrinterTabPage(Window* pParent,
    const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PrintOptionsPage",
        "modules/swriter/ui/printoptionspage.ui", rCoreSet)
    , sNone(SW_RESSTR(SW_STR_NONE))
    , bAttrModified(sal_False)
    , bPreview(sal_False)
{
    get(m_pGrfCB, "graphics");
    get(m_pCtrlFldCB, "formcontrols");
    get(m_pBackgroundCB, "background");
    get(m_pBlackFontCB, "inblack");
    get(m_pPrintHiddenTextCB, "hiddentext");
    get(m_pPrintTextPlaceholderCB, "textplaceholder");

    get(m_pPagesFrame, "pagesframe");
    get(m_pLeftPageCB, "leftpages");
    get(m_pRightPageCB, "rightpages");
    get(m_pProspectCB, "brochure");
    get(m_pProspectCB_RTL, "rtl");

    get(m_pCommentsFrame, "commentsframe");
    get(m_pNoRB, "none");
    get(m_pOnlyRB, "only");
    get(m_pEndRB, "end");
    get(m_pEndPageRB, "endpage");
    get(m_pPrintEmptyPagesCB, "blankpages");
    get(m_pPaperFromSetupCB, "papertray");
    get(m_pFaxLB, "fax");

    Init();

    Link aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
    m_pGrfCB->SetClickHdl( aLk );
    m_pRightPageCB->SetClickHdl( aLk );
    m_pLeftPageCB->SetClickHdl( aLk );
    m_pCtrlFldCB->SetClickHdl( aLk );
    m_pBackgroundCB->SetClickHdl( aLk );
    m_pBlackFontCB->SetClickHdl( aLk );
    m_pPrintHiddenTextCB->SetClickHdl( aLk );
    m_pPrintTextPlaceholderCB->SetClickHdl( aLk );
    m_pProspectCB->SetClickHdl( aLk );
    m_pProspectCB_RTL->SetClickHdl( aLk );
    m_pPaperFromSetupCB->SetClickHdl( aLk );
    m_pPrintEmptyPagesCB->SetClickHdl( aLk );
    m_pEndPageRB->SetClickHdl( aLk );
    m_pEndRB->SetClickHdl( aLk );
    m_pOnlyRB->SetClickHdl( aLk );
    m_pNoRB->SetClickHdl( aLk );
    m_pFaxLB->SetSelectHdl( LINK( this, SwAddPrinterTabPage, SelectHdl ) );

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        m_pLeftPageCB->Hide();
        m_pRightPageCB->Hide();
        m_pPrintHiddenTextCB->Hide();
        m_pPrintTextPlaceholderCB->Hide();

        // hide m_pPrintEmptyPagesCB
        m_pPrintEmptyPagesCB->Hide();
    }
    m_pProspectCB_RTL->Disable();
    SvtCTLOptions aCTLOptions;
    m_pProspectCB_RTL->Show(aCTLOptions.IsCTLFontEnabled());
}

void SwAddPrinterTabPage::SetPreview(sal_Bool bPrev)
{
    bPreview = bPrev;
    m_pCommentsFrame->Enable(!bPreview);
    m_pPagesFrame->Enable(!bPreview);
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
        aAddPrinterAttr.bPrintGraphic   = m_pGrfCB->IsChecked();
        aAddPrinterAttr.bPrintTable     = sal_True; // always enabled since CWS printerpullgpages /*aTabCB.IsChecked();*/
        aAddPrinterAttr.bPrintDraw      = m_pGrfCB->IsChecked(); // UI merged with m_pGrfCB in CWS printerpullgpages
        aAddPrinterAttr.bPrintControl   = m_pCtrlFldCB->IsChecked();
        aAddPrinterAttr.bPrintPageBackground = m_pBackgroundCB->IsChecked();
        aAddPrinterAttr.bPrintBlackFont = m_pBlackFontCB->IsChecked();
        aAddPrinterAttr.bPrintHiddenText = m_pPrintHiddenTextCB->IsChecked();
        aAddPrinterAttr.bPrintTextPlaceholder = m_pPrintTextPlaceholderCB->IsChecked();

        aAddPrinterAttr.bPrintLeftPages     = m_pLeftPageCB->IsChecked();
        aAddPrinterAttr.bPrintRightPages    = m_pRightPageCB->IsChecked();
        aAddPrinterAttr.bPrintReverse       = sal_False; // handled by vcl itself since CWS printerpullpages /*aReverseCB.IsChecked()*/;
        aAddPrinterAttr.bPrintProspect      = m_pProspectCB->IsChecked();
        aAddPrinterAttr.bPrintProspectRTL   = m_pProspectCB_RTL->IsChecked();
        aAddPrinterAttr.bPaperFromSetup     = m_pPaperFromSetupCB->IsChecked();
        aAddPrinterAttr.bPrintEmptyPages    = m_pPrintEmptyPagesCB->IsChecked();
        aAddPrinterAttr.bPrintSingleJobs    = sal_True; // handled by vcl in new print dialog since CWS printerpullpages /*aSingleJobsCB.IsChecked()*/;

        if (m_pNoRB->IsChecked())  aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_NONE;
        if (m_pOnlyRB->IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ONLY;
        if (m_pEndRB->IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ENDDOC;
        if (m_pEndPageRB->IsChecked()) aAddPrinterAttr.nPrintPostIts =
                                                        POSTITS_ENDPAGE;

        OUString sFax = m_pFaxLB->GetSelectEntry();
        aAddPrinterAttr.sFaxName = sNone == sFax ? aEmptyOUStr : sFax;
        rCoreSet.Put(aAddPrinterAttr);
    }
    return bAttrModified;
}

void    SwAddPrinterTabPage::Reset( const SfxItemSet&  )
{
    const   SfxItemSet&         rSet = GetItemSet();
    const   SwAddPrinterItem*   pAddPrinterAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_ADDPRINTER , false,
                                    (const SfxPoolItem**)&pAddPrinterAttr ))
    {
        m_pGrfCB->Check(pAddPrinterAttr->bPrintGraphic || pAddPrinterAttr->bPrintDraw);
        m_pCtrlFldCB->Check(       pAddPrinterAttr->bPrintControl);
        m_pBackgroundCB->Check(    pAddPrinterAttr->bPrintPageBackground);
        m_pBlackFontCB->Check(     pAddPrinterAttr->bPrintBlackFont);
        m_pPrintHiddenTextCB->Check( pAddPrinterAttr->bPrintHiddenText);
        m_pPrintTextPlaceholderCB->Check(pAddPrinterAttr->bPrintTextPlaceholder);
        m_pLeftPageCB->Check(      pAddPrinterAttr->bPrintLeftPages);
        m_pRightPageCB->Check(     pAddPrinterAttr->bPrintRightPages);
        m_pPaperFromSetupCB->Check(pAddPrinterAttr->bPaperFromSetup);
        m_pPrintEmptyPagesCB->Check(pAddPrinterAttr->bPrintEmptyPages);
        m_pProspectCB->Check(      pAddPrinterAttr->bPrintProspect);
        m_pProspectCB_RTL->Check(      pAddPrinterAttr->bPrintProspectRTL);

        m_pNoRB->Check (pAddPrinterAttr->nPrintPostIts== POSTITS_NONE ) ;
        m_pOnlyRB->Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ONLY ) ;
        m_pEndRB->Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ENDDOC ) ;
        m_pEndPageRB->Check (pAddPrinterAttr->nPrintPostIts== POSTITS_ENDPAGE ) ;
        m_pFaxLB->SelectEntry( pAddPrinterAttr->sFaxName );
    }
    if (m_pProspectCB->IsChecked())
    {
        m_pProspectCB_RTL->Enable(true);
        m_pNoRB->Enable( false );
        m_pOnlyRB->Enable( false );
        m_pEndRB->Enable( false );
        m_pEndPageRB->Enable( false );
    }
    else
        m_pProspectCB_RTL->Enable( false );
}

void    SwAddPrinterTabPage::Init()
{

}

IMPL_LINK_NOARG_INLINE_START(SwAddPrinterTabPage, AutoClickHdl)
{
    bAttrModified = sal_True;
    bool bIsProspect = m_pProspectCB->IsChecked();
    if (!bIsProspect)
        m_pProspectCB_RTL->Check( false );
    m_pProspectCB_RTL->Enable( bIsProspect );
    m_pNoRB->Enable( !bIsProspect );
    m_pOnlyRB->Enable( !bIsProspect );
    m_pEndRB->Enable( !bIsProspect );
    m_pEndPageRB->Enable( !bIsProspect );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwAddPrinterTabPage, AutoClickHdl)

void  SwAddPrinterTabPage::SetFax( const std::vector<OUString>& rFaxLst )
{
    m_pFaxLB->InsertEntry(sNone);
    for(size_t i = 0; i < rFaxLst.size(); ++i)
    {
        m_pFaxLB->InsertEntry(rFaxLst[i]);
    }
    m_pFaxLB->SelectEntryPos(0);
}

IMPL_LINK_NOARG_INLINE_START(SwAddPrinterTabPage, SelectHdl)
{
    bAttrModified=sal_True;
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwAddPrinterTabPage, SelectHdl)

void SwAddPrinterTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pListItem,SfxBoolItem,SID_FAX_LIST,false);
    SFX_ITEMSET_ARG (&aSet,pPreviewItem,SfxBoolItem,SID_PREVIEWFLAG_TYPE,false);
    if (pPreviewItem)
    {
        SetPreview(pPreviewItem->GetValue());
        Reset(aSet);
    }
    if (pListItem && pListItem->GetValue())
    {
        std::vector<OUString> aFaxList;
        const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
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
    SfxTabPage( pParent, "OptFontTabPage" , "modules/swriter/ui/optfonttabpage.ui" , rSet),
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
    get(pLabelFT,"label1");
    get(pStandardBox,"standardbox");
    pStandardBox->SetStyle(pStandardBox->GetStyle() |  WB_SORT);
    get(pStandardHeightLB,"standardheight");
    get(pTitleBox,"titlebox");
    pTitleBox->SetStyle(pTitleBox->GetStyle() |  WB_SORT);
    get(pTitleHeightLB,"titleheight");
    get(pListBox,"listbox");
    pListBox->SetStyle(pListBox->GetStyle() |  WB_SORT);
    get(pListHeightLB,"listheight");
    get(pLabelBox,"labelbox");
    pLabelBox->SetStyle(pLabelBox->GetStyle() |  WB_SORT);
    get(pLabelHeightLB,"labelheight");
    get(pIdxBox,"idxbox");
    pIdxBox->SetStyle(pIdxBox->GetStyle() |  WB_SORT);
    get(pIndexHeightLB,"indexheight");

    get(pDocOnlyCB,"doconly");
    get(pStandardPB,"standard");

    pStandardPB->SetClickHdl(LINK(this, SwStdFontTabPage, StandardHdl));
    pStandardBox->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    pListBox    ->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    pLabelBox   ->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    pIdxBox     ->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    Link aFocusLink = LINK( this, SwStdFontTabPage, LoseFocusHdl);
    pStandardBox->SetLoseFocusHdl( aFocusLink );
    pTitleBox   ->SetLoseFocusHdl( aFocusLink );
    pListBox    ->SetLoseFocusHdl( aFocusLink );
    pLabelBox   ->SetLoseFocusHdl( aFocusLink );
    pIdxBox     ->SetLoseFocusHdl( aFocusLink );

    Link aModifyHeightLink( LINK( this, SwStdFontTabPage, ModifyHeightHdl));
    pStandardHeightLB->SetModifyHdl( aModifyHeightLink );
    pTitleHeightLB->   SetModifyHdl( aModifyHeightLink );
    pListHeightLB->    SetModifyHdl( aModifyHeightLink );
    pLabelHeightLB->   SetModifyHdl( aModifyHeightLink );
    pIndexHeightLB->   SetModifyHdl( aModifyHeightLink );

    pDocOnlyCB->Check(SW_MOD()->GetModuleConfig()->IsDefaultFontInCurrDocOnly());
}

SwStdFontTabPage::~SwStdFontTabPage()
{
    delete pFontList;
    if (bDeletePrinter)
    {
        delete pPrt;
    }
}

SfxTabPage* SwStdFontTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwStdFontTabPage(pParent, rAttrSet);
}

static void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    SfxPrinter* pPrt, const OUString& rStyle,
                    sal_uInt16 nFontWhich)
{
    Font aFont( rStyle, Size( 0, 10 ) );
    if( pPrt )
        aFont = pPrt->GetFontMetric( aFont );
    SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(nType);
    pColl->SetFmtAttr(SvxFontItem(aFont.GetFamily(), aFont.GetName(),
                aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
}

static void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    sal_Int32 nHeight, sal_uInt16 nFontHeightWhich)
{
    float fSize = (float)nHeight / 10;
    nHeight = CalcToUnit( fSize, SFX_MAPUNIT_TWIP );
    SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(nType);
    pColl->SetFmtAttr(SvxFontHeightItem(nHeight, 100, nFontHeightWhich));
}

sal_Bool SwStdFontTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bNotDocOnly = !pDocOnlyCB->IsChecked();
    SW_MOD()->GetModuleConfig()->SetDefaultFontInCurrDocOnly(!bNotDocOnly);

    OUString sStandard    = pStandardBox->GetText();
    OUString sTitle       = pTitleBox->GetText();
    OUString sList        = pListBox->GetText();
    OUString sLabel       = pLabelBox->GetText();
    OUString sIdx         = pIdxBox->GetText();

    bool bStandardHeightChanged = pStandardHeightLB->GetSavedValue() != pStandardHeightLB->GetText();
    bool bTitleHeightChanged = pTitleHeightLB->GetSavedValue() != pTitleHeightLB->GetText();
    bool bListHeightChanged = pListHeightLB->GetSavedValue() != pListHeightLB->GetText() && (!bListHeightDefault || !bSetListHeightDefault );
    bool bLabelHeightChanged = pLabelHeightLB->GetSavedValue() != pLabelHeightLB->GetText() && (!bLabelHeightDefault || !bSetLabelHeightDefault );
    bool bIndexHeightChanged = pIndexHeightLB->GetSavedValue() != pIndexHeightLB->GetText() && (!bIndexHeightDefault || !bSetIndexHeightDefault );
    if(bNotDocOnly)
    {
        pFontConfig->SetFontStandard(sStandard, nFontGroup);
        pFontConfig->SetFontOutline(sTitle, nFontGroup);
        pFontConfig->SetFontList(sList, nFontGroup);
        pFontConfig->SetFontCaption(sLabel, nFontGroup);
        pFontConfig->SetFontIndex(sIdx, nFontGroup);
        if(bStandardHeightChanged)
        {
            float fSize = (float)pStandardHeightLB->GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_STANDARD, nFontGroup );
        }
        if(bTitleHeightChanged)
        {
            float fSize = (float)pTitleHeightLB->GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_OUTLINE, nFontGroup );
        }
        if(bListHeightChanged)
        {
            float fSize = (float)pListHeightLB->GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_LIST, nFontGroup );
        }
        if(bLabelHeightChanged)
        {
            float fSize = (float)pLabelHeightLB->GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_CAPTION, nFontGroup );
        }
        if(bIndexHeightChanged)
        {
            float fSize = (float)pIndexHeightLB->GetValue() / 10;
            pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_INDEX, nFontGroup );
        }
    }
    if(pWrtShell)
    {
        pWrtShell->StartAllAction();
        SfxPrinter* pPrinter = pWrtShell->getIDocumentDeviceAccess()->getPrinter( false );
        bool bMod = false;
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
                                  aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
            SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFmtAttr(nFontWhich);
            bMod = true;
        }
        if(bStandardHeightChanged)
        {
            float fSize = (float)pStandardHeightLB->GetValue() / 10;
            pWrtShell->SetDefault(SvxFontHeightItem( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), 100, nFontHeightWhich ) );
            SwTxtFmtColl *pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFmtAttr(nFontHeightWhich);
            bMod = true;
        }

        if(sTitle != sShellTitle )
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_HEADLINE_BASE, pPrinter, sTitle, nFontWhich);
            bMod = true;
        }
        if(bTitleHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_HEADLINE_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(pTitleHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if(sList != sShellList && (!bListDefault || !bSetListDefault ))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_NUMBUL_BASE, pPrinter, sList, nFontWhich);
            bMod = true;
        }
        if(bListHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_NUMBUL_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(pListHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if(sLabel != sShellLabel && (!bLabelDefault || !bSetLabelDefault))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_LABEL, pPrinter, sLabel, nFontWhich);
            bMod = true;
        }
        if(bLabelHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_LABEL,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(pLabelHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if(sIdx != sShellIndex && (!bIdxDefault || !bSetIdxDefault))
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_REGISTER_BASE, pPrinter, sIdx, nFontWhich);
            bMod = true;
        }
        if(bIndexHeightChanged)
        {
            lcl_SetColl(pWrtShell, RES_POOLCOLL_REGISTER_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(pIndexHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
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

    if( SFX_ITEM_SET == rSet.GetItemState(nLangSlot, false, &pLang))
        eLanguage = ((const SvxLanguageItem*)pLang)->GetValue();

    OUString sTmp = pLabelFT->GetText();
    OUString sToReplace = sScriptWestern;
    if(FONT_GROUP_CJK == nFontGroup )
        sToReplace = sScriptAsian;
    else if(FONT_GROUP_CTL == nFontGroup )
        sToReplace = sScriptComplex;
    pLabelFT->SetText(sTmp.replaceFirst("%1", sToReplace));

    const SfxPoolItem* pItem;

    if (bDeletePrinter)
    {
        delete pPrt;
    }

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_PRINTER, false, &pItem))
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
    }
    delete pFontList;
    pFontList = new FontList( pPrt );
    // #i94536# prevent duplication of font entries when 'reset' button is pressed
    if( !pStandardBox->GetEntryCount() )
    {
        // get the set of disctinct available family names
        std::set< OUString > aFontNames;
        int nFontNames = pPrt->GetDevFontCount();
        for( int i = 0; i < nFontNames; i++ )
        {
            FontInfo aInf( pPrt->GetDevFont( i ) );
            aFontNames.insert( aInf.GetName() );
        }

        // insert to listboxes
        for( std::set< OUString >::const_iterator it = aFontNames.begin();
             it != aFontNames.end(); ++it )
        {
            pStandardBox->InsertEntry( *it );
            pTitleBox->InsertEntry( *it );
            pListBox->InsertEntry( *it );
            pLabelBox->InsertEntry( *it );
            pIdxBox->InsertEntry( *it );
        }
    }
    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_STDFONTS, false, &pItem))
    {
         pFontConfig = (SwStdFontConfig*)((const SwPtrItem*)pItem)->GetValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_WRTSHELL, false, &pItem))
    {
        pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();
    }
    OUString sStdBackup;
    OUString sOutBackup;
    OUString sListBackup;
    OUString sCapBackup;
    OUString sIdxBackup;
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

       pDocOnlyCB->Enable(false);
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
        bListDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        sShellList = sListBackup = rFontLS.GetFamilyName();

        const SvxFontHeightItem& rFontHeightList = (const SvxFontHeightItem&)pColl->GetFmtAttr(nFontHeightWhich, sal_True);
        nListHeight = (sal_Int32)rFontHeightList.GetHeight();
        bListHeightDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);


        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_LABEL);
        bLabelDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontCP = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        sShellLabel = sCapBackup = rFontCP.GetFamilyName();
        const SvxFontHeightItem& rFontHeightLabel = (const SvxFontHeightItem&)pColl->GetFmtAttr(nFontHeightWhich, sal_True);
        nLabelHeight = (sal_Int32)rFontHeightLabel.GetHeight();
        bLabelHeightDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);

        pColl = pWrtShell->GetTxtCollFromPool(RES_POOLCOLL_REGISTER_BASE);
        bIdxDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontIDX = !nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        sShellIndex = sIdxBackup = rFontIDX.GetFamilyName();
        const SvxFontHeightItem& rFontHeightIndex = (const SvxFontHeightItem&)pColl->GetFmtAttr(nFontHeightWhich, sal_True);
        nIndexHeight = (sal_Int32)rFontHeightIndex.GetHeight();
        bIndexHeightDefault = SFX_ITEM_DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
    }
    pStandardBox->SetText(sStdBackup );
    pTitleBox->SetText(sOutBackup );
    pListBox->SetText(sListBackup);
    pLabelBox->SetText(sCapBackup );
    pIdxBox->SetText(sIdxBackup );

    FontInfo aFontInfo( pFontList->Get(sStdBackup, sStdBackup) );
    pStandardHeightLB->Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sOutBackup, sOutBackup );
    pTitleHeightLB->Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sListBackup,sListBackup);
    pListHeightLB->Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sCapBackup, sCapBackup );
    pLabelHeightLB->Fill( &aFontInfo, pFontList );
    aFontInfo = pFontList->Get(sIdxBackup, sIdxBackup );
    pIndexHeightLB->Fill( &aFontInfo, pFontList );

    pStandardHeightLB->SetValue( CalcToPoint( nStandardHeight, SFX_MAPUNIT_TWIP, 10 ) );
    pTitleHeightLB->   SetValue( CalcToPoint( nTitleHeight   , SFX_MAPUNIT_TWIP, 10 ) );
    pListHeightLB->    SetValue( CalcToPoint( nListHeight    , SFX_MAPUNIT_TWIP, 10 ) );
    pLabelHeightLB->   SetValue( CalcToPoint( nLabelHeight   , SFX_MAPUNIT_TWIP, 10 ));
    pIndexHeightLB->   SetValue( CalcToPoint( nIndexHeight   , SFX_MAPUNIT_TWIP, 10 ));

    pStandardBox->SaveValue();
    pTitleBox->SaveValue();
    pListBox->SaveValue();
    pLabelBox->SaveValue();
    pIdxBox->SaveValue();

    pStandardHeightLB->SaveValue();
    pTitleHeightLB->SaveValue();
    pListHeightLB->SaveValue();
    pLabelHeightLB->SaveValue();
    pIndexHeightLB->SaveValue();
}

IMPL_LINK_NOARG(SwStdFontTabPage, StandardHdl)
{
    sal_uInt8 nFontOffset = nFontGroup * FONT_PER_GROUP;
    pStandardBox->SetText(SwStdFontConfig::GetDefaultFor(FONT_STANDARD + nFontOffset, eLanguage));
    pTitleBox   ->SetText(SwStdFontConfig::GetDefaultFor(FONT_OUTLINE  + nFontOffset, eLanguage));
    pListBox    ->SetText(SwStdFontConfig::GetDefaultFor(FONT_LIST     + nFontOffset, eLanguage));
    pLabelBox   ->SetText(SwStdFontConfig::GetDefaultFor(FONT_CAPTION  + nFontOffset, eLanguage));
    pIdxBox     ->SetText(SwStdFontConfig::GetDefaultFor(FONT_INDEX    + nFontOffset, eLanguage));

    pStandardBox->SaveValue();
    pTitleBox->SaveValue();
    pListBox->SaveValue();
    pLabelBox->SaveValue();
    pIdxBox->SaveValue();

    pStandardHeightLB->SetValue( CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_STANDARD + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ) );
    pTitleHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_OUTLINE  +
            nFontOffset, eLanguage), SFX_MAPUNIT_TWIP, 10 ));
    pListHeightLB    ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_LIST + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    pLabelHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_CAPTION  + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    pIndexHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_INDEX    + nFontOffset, eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    return 0;
}

IMPL_LINK( SwStdFontTabPage, ModifyHdl, ComboBox*, pBox )
{
    if(pBox == pStandardBox)
    {
        OUString sEntry = pBox->GetText();
        if(bSetListDefault && bListDefault)
            pListBox->SetText(sEntry);
        if(bSetLabelDefault && bLabelDefault)
            pLabelBox->SetText(sEntry);
        if(bSetIdxDefault && bIdxDefault)
            pIdxBox->SetText(sEntry);
    }
    else if(pBox == pListBox)
    {
        bSetListDefault = sal_False;
    }
    else if(pBox == pLabelBox)
    {
        bSetLabelDefault = sal_False;
    }
    else if(pBox == pIdxBox)
    {
        bSetIdxDefault = sal_False;
    }
    return 0;
}

IMPL_LINK( SwStdFontTabPage, ModifyHeightHdl, FontSizeBox*, pBox )
{
    if(pBox == pStandardHeightLB)
    {
        sal_Int64 nValue = pBox->GetValue(FUNIT_TWIP);
        if(bSetListHeightDefault && bListHeightDefault)
            pListHeightLB->SetValue(nValue, FUNIT_TWIP);
        if(bSetLabelHeightDefault && bLabelHeightDefault)
            pLabelHeightLB->SetValue(nValue, FUNIT_TWIP);
        if(bSetIndexHeightDefault && bIndexHeightDefault)
            pIndexHeightLB->SetValue(nValue, FUNIT_TWIP);
    }
    else if(pBox == pListHeightLB)
    {
        bSetListHeightDefault = sal_False;
    }
    else if(pBox == pLabelHeightLB)
    {
        bSetLabelHeightDefault = sal_False;
    }
    else if(pBox == pIndexHeightLB)
    {
        bSetIndexHeightDefault = sal_False;
    }
    return 0;
}

IMPL_LINK( SwStdFontTabPage, LoseFocusHdl, ComboBox*, pBox )
{
    FontSizeBox* pHeightLB = 0;
    OUString sEntry = pBox->GetText();
    if(pBox == pStandardBox)
    {
        pHeightLB = pStandardHeightLB;
    }
    else if(pBox == pTitleBox)
    {
        pHeightLB = pTitleHeightLB;
    }
    else if(pBox == pListBox)
    {
        pHeightLB = pListHeightLB;
    }
    else if(pBox == pLabelBox)
    {
        pHeightLB = pLabelHeightLB;
    }
    else /*if(pBox == pIdxBox)*/
    {
        pHeightLB = pIndexHeightLB;
    }
    FontInfo aFontInfo( pFontList->Get(sEntry, sEntry) );
    pHeightLB->Fill( &aFontInfo, pFontList );
    return 0;
}


void SwStdFontTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt16Item, SID_FONTMODE_TYPE, false);
    if (pFlagItem)
        SetFontMode(sal::static_int_cast< sal_uInt8, sal_uInt16>( pFlagItem->GetValue()));
}

SwTableOptionsTabPage::SwTableOptionsTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "OptTablePage", "modules/swriter/ui/opttablepage.ui", rSet),
    pWrtShell(0),
    bHTMLMode(sal_False)
{
    get(pHeaderCB,"header");
    get(pRepeatHeaderCB,"repeatheader");
    get(pDontSplitCB,"dontsplit");
    get(pBorderCB,"border");
    get(pNumFormattingCB,"numformatting");
    get(pNumFmtFormattingCB,"numfmtformatting");
    get(pNumAlignmentCB,"numalignment");
    get(pRowMoveMF,"rowmove");
    get(pColMoveMF,"colmove");
    get(pRowInsertMF,"rowinsert");
    get(pColInsertMF,"colinsert");
    get(pFixRB,"fix");
    get(pFixPropRB,"fixprop");
    get(pVarRB,"var");

    Link aLnk(LINK(this, SwTableOptionsTabPage, CheckBoxHdl));
    pNumFormattingCB->SetClickHdl(aLnk);
    pNumFmtFormattingCB->SetClickHdl(aLnk);
    pHeaderCB->SetClickHdl(aLnk);
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

    if(pRowMoveMF->IsModified())
        pModOpt->SetTblHMove( (sal_uInt16)pRowMoveMF->Denormalize( pRowMoveMF->GetValue(FUNIT_TWIP)));

    if(pColMoveMF->IsModified())
        pModOpt->SetTblVMove( (sal_uInt16)pColMoveMF->Denormalize( pColMoveMF->GetValue(FUNIT_TWIP)));

    if(pRowInsertMF->IsModified())
        pModOpt->SetTblHInsert((sal_uInt16)pRowInsertMF->Denormalize( pRowInsertMF->GetValue(FUNIT_TWIP)));

    if(pColInsertMF->IsModified())
        pModOpt->SetTblVInsert((sal_uInt16)pColInsertMF->Denormalize( pColInsertMF->GetValue(FUNIT_TWIP)));

    TblChgMode eMode;
    if(pFixRB->IsChecked())
        eMode = TBLFIX_CHGABS;
    else if(pFixPropRB->IsChecked())
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

    if (pHeaderCB->IsChecked())
        aInsOpts.mnInsMode |= tabopts::HEADLINE;

    if (pRepeatHeaderCB->IsEnabled() )
        aInsOpts.mnRowsToRepeat = pRepeatHeaderCB->IsChecked()? 1 : 0;

    if (!pDontSplitCB->IsChecked())
        aInsOpts.mnInsMode |= tabopts::SPLIT_LAYOUT;

    if (pBorderCB->IsChecked())
        aInsOpts.mnInsMode |= tabopts::DEFAULT_BORDER;

    if (pHeaderCB->GetSavedValue() != pHeaderCB->GetState() ||
        pRepeatHeaderCB->GetSavedValue() != pRepeatHeaderCB->GetState() ||
        pDontSplitCB->GetSavedValue() != pDontSplitCB->GetState() ||
        pBorderCB->GetSavedValue() != pBorderCB->GetState())
    {
        pModOpt->SetInsTblFlags(bHTMLMode, aInsOpts);
    }

    if (pNumFormattingCB->GetSavedValue() != pNumFormattingCB->GetState())
    {
        pModOpt->SetInsTblFormatNum(bHTMLMode, pNumFormattingCB->IsChecked());
        bRet = sal_True;
    }

    if (pNumFmtFormattingCB->GetSavedValue() != pNumFmtFormattingCB->GetState())
    {
        pModOpt->SetInsTblChangeNumFormat(bHTMLMode, pNumFmtFormattingCB->IsChecked());
        bRet = sal_True;
    }

    if (pNumAlignmentCB->GetSavedValue() != pNumAlignmentCB->GetState())
    {
        pModOpt->SetInsTblAlignNum(bHTMLMode, pNumAlignmentCB->IsChecked());
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
        ::SetFieldUnit( *pRowMoveMF, eFieldUnit );
        ::SetFieldUnit( *pColMoveMF, eFieldUnit );
        ::SetFieldUnit( *pRowInsertMF, eFieldUnit );
        ::SetFieldUnit( *pColInsertMF, eFieldUnit );
    }

    pRowMoveMF->SetValue(pRowMoveMF->Normalize(pModOpt->GetTblHMove()), FUNIT_TWIP);
    pColMoveMF->SetValue(pColMoveMF->Normalize(pModOpt->GetTblVMove()), FUNIT_TWIP);
    pRowInsertMF->SetValue(pRowInsertMF->Normalize(pModOpt->GetTblHInsert()), FUNIT_TWIP);
    pColInsertMF->SetValue(pColInsertMF->Normalize(pModOpt->GetTblVInsert()), FUNIT_TWIP);

    switch(pModOpt->GetTblMode())
    {
        case TBLFIX_CHGABS:     pFixRB->Check();     break;
        case TBLFIX_CHGPROP:    pFixPropRB->Check(); break;
        case TBLVAR_CHGABS:     pVarRB->Check(); break;
    }
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    // hide certain controls for html
    if(bHTMLMode)
    {
        pRepeatHeaderCB->Hide();
        pDontSplitCB->Hide();
    }

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTblFlags(bHTMLMode);
    sal_uInt16 nInsTblFlags = aInsOpts.mnInsMode;

    pHeaderCB->Check(0 != (nInsTblFlags & tabopts::HEADLINE));
    pRepeatHeaderCB->Check((!bHTMLMode) && (aInsOpts.mnRowsToRepeat > 0));
    pDontSplitCB->Check(!(nInsTblFlags & tabopts::SPLIT_LAYOUT));
    pBorderCB->Check(0 != (nInsTblFlags & tabopts::DEFAULT_BORDER));

    pNumFormattingCB->Check(pModOpt->IsInsTblFormatNum(bHTMLMode));
    pNumFmtFormattingCB->Check(pModOpt->IsInsTblChangeNumFormat(bHTMLMode));
    pNumAlignmentCB->Check(pModOpt->IsInsTblAlignNum(bHTMLMode));

    pHeaderCB->SaveValue();
    pRepeatHeaderCB->SaveValue();
    pDontSplitCB->SaveValue();
    pBorderCB->SaveValue();
    pNumFormattingCB->SaveValue();
    pNumFmtFormattingCB->SaveValue();
    pNumAlignmentCB->SaveValue();

    CheckBoxHdl(0);
}

IMPL_LINK_NOARG(SwTableOptionsTabPage, CheckBoxHdl)
{
    pNumFmtFormattingCB->Enable(pNumFormattingCB->IsChecked());
    pNumAlignmentCB->Enable(pNumFormattingCB->IsChecked());
    pRepeatHeaderCB->Enable(pHeaderCB->IsChecked());
    return 0;
}

void SwTableOptionsTabPage::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pWrtSh,SwWrtShellItem,SID_WRT_SHELL,false);
    if (pWrtSh)
        SetWrtShell(pWrtSh->GetValue());
}

SwShdwCrsrOptionsTabPage::SwShdwCrsrOptionsTabPage( Window* pParent,
                                                    const SfxItemSet& rSet )
   : SfxTabPage(pParent, "OptFormatAidsPage",
                "modules/swriter/ui/optformataidspage.ui", rSet),
    m_pWrtShell( NULL )
{
    get(m_pParaCB, "paragraph");
    get(m_pSHyphCB, "hyphens");
    get(m_pSpacesCB, "spaces");
    get(m_pHSpacesCB, "nonbreak");
    get(m_pTabCB, "tabs");
    get(m_pBreakCB, "break");
    get(m_pCharHiddenCB, "hiddentext");
    get(m_pFldHiddenCB, "hiddentextfield");
    get(m_pFldHiddenParaCB, "hiddenparafield");

    get(m_pDirectCursorFrame, "directcrsrframe");
    get(m_pOnOffCB, "cursoronoff");

    get(m_pFillMarginRB, "fillmargin");
    get(m_pFillIndentRB, "fillindent");
    get(m_pFillTabRB, "filltab");
    get(m_pFillSpaceRB, "fillspace");

    get(m_pCursorProtFrame, "crsrprotframe");
    get(m_pCrsrInProtCB, "cursorinprot");

    get(m_pMathBaselineAlignmentCB, "mathbaseline");

    const SfxPoolItem* pItem = 0;

    SwShadowCursorItem aOpt;
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem ))
        aOpt = *(SwShadowCursorItem*)pItem;
    m_pOnOffCB->Check( aOpt.IsOn() );

    sal_uInt8 eMode = aOpt.GetMode();
    m_pFillIndentRB->Check( FILL_INDENT == eMode );
    m_pFillMarginRB->Check( FILL_MARGIN == eMode );
    m_pFillTabRB->Check( FILL_TAB == eMode );
    m_pFillSpaceRB->Check( FILL_SPACE == eMode );

    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        m_pTabCB->Hide();
        m_pCharHiddenCB->Hide();
        m_pFldHiddenCB->Hide();
        m_pFldHiddenParaCB->Hide();

        m_pDirectCursorFrame->Hide();
        m_pOnOffCB->Hide();
        m_pFillMarginRB->Hide();
        m_pFillIndentRB->Hide();
        m_pFillTabRB->Hide();
        m_pFillSpaceRB->Hide();

        m_pCursorProtFrame->Hide();
        m_pCrsrInProtCB->Hide();
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
    SFX_ITEMSET_ARG (&aSet,pWrtSh,SwWrtShellItem,SID_WRT_SHELL,false);
    if (pWrtSh)
        SetWrtShell(pWrtSh->GetValue());
}


sal_Bool SwShdwCrsrOptionsTabPage::FillItemSet( SfxItemSet& rSet )
{
    SwShadowCursorItem aOpt;
    aOpt.SetOn( m_pOnOffCB->IsChecked() );

    sal_uInt8 eMode;
    if( m_pFillIndentRB->IsChecked() )
        eMode= FILL_INDENT;
    else if( m_pFillMarginRB->IsChecked() )
        eMode = FILL_MARGIN;
    else if( m_pFillTabRB->IsChecked() )
        eMode = FILL_TAB;
    else
        eMode = FILL_SPACE;
    aOpt.SetMode( eMode );

    bool bRet = false;
    const SfxPoolItem* pItem = 0;
    if( SFX_ITEM_SET != rSet.GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem )
        ||  ((SwShadowCursorItem&)*pItem) != aOpt )
    {
        rSet.Put( aOpt );
        bRet = true;
    }

    if (m_pWrtShell) {
        m_pWrtShell->GetDoc()->set( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT,
                                    m_pMathBaselineAlignmentCB->IsChecked() );
        bRet |= TriState(m_pMathBaselineAlignmentCB->IsChecked()) != m_pMathBaselineAlignmentCB->GetSavedValue();
    }

    if( TriState(m_pCrsrInProtCB->IsChecked()) != m_pCrsrInProtCB->GetSavedValue())
    {
        rSet.Put(SfxBoolItem(FN_PARAM_CRSR_IN_PROTECTED, m_pCrsrInProtCB->IsChecked()));
        bRet = true;
    }

    const SwDocDisplayItem* pOldAttr = (const SwDocDisplayItem*)
                        GetOldItem(GetItemSet(), FN_PARAM_DOCDISP);

    SwDocDisplayItem aDisp;
    if(pOldAttr)
        aDisp = *pOldAttr;

    aDisp.bParagraphEnd         = m_pParaCB->IsChecked();
    aDisp.bTab                  = m_pTabCB->IsChecked();
    aDisp.bSpace                = m_pSpacesCB->IsChecked();
    aDisp.bNonbreakingSpace     = m_pHSpacesCB->IsChecked();
    aDisp.bSoftHyphen           = m_pSHyphCB->IsChecked();
    aDisp.bFldHiddenText        = m_pFldHiddenCB->IsChecked();
    aDisp.bCharHiddenText       = m_pCharHiddenCB->IsChecked();
    aDisp.bShowHiddenPara       = m_pFldHiddenParaCB->IsChecked();
    aDisp.bManualBreak          = m_pBreakCB->IsChecked();

    bRet |= (!pOldAttr || aDisp != *pOldAttr);
    if(bRet)
        bRet = 0 != rSet.Put(aDisp);

    return bRet;
}

void SwShdwCrsrOptionsTabPage::Reset( const SfxItemSet& rSet )
{
    const SfxPoolItem* pItem = 0;

    SwShadowCursorItem aOpt;
    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem ))
        aOpt = *(SwShadowCursorItem*)pItem;
    m_pOnOffCB->Check( aOpt.IsOn() );

    sal_uInt8 eMode = aOpt.GetMode();
    m_pFillIndentRB->Check( FILL_INDENT == eMode );
    m_pFillMarginRB->Check( FILL_MARGIN == eMode );
    m_pFillTabRB->Check( FILL_TAB == eMode );
    m_pFillSpaceRB->Check( FILL_SPACE == eMode );

    if (m_pWrtShell) {
       m_pMathBaselineAlignmentCB->Check( m_pWrtShell->GetDoc()->get( IDocumentSettingAccess::MATH_BASELINE_ALIGNMENT ) );
       m_pMathBaselineAlignmentCB->SaveValue();
    } else {
        m_pMathBaselineAlignmentCB->Hide();
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_CRSR_IN_PROTECTED, false, &pItem ))
        m_pCrsrInProtCB->Check(((const SfxBoolItem*)pItem)->GetValue());
    m_pCrsrInProtCB->SaveValue();

    const SwDocDisplayItem* pDocDisplayAttr = 0;

    rSet.GetItemState( FN_PARAM_DOCDISP, false,
                                    (const SfxPoolItem**)&pDocDisplayAttr );
    if(pDocDisplayAttr)
    {
        m_pParaCB->Check  ( pDocDisplayAttr->bParagraphEnd );
        m_pTabCB->Check  ( pDocDisplayAttr->bTab );
        m_pSpacesCB->Check  ( pDocDisplayAttr->bSpace );
        m_pHSpacesCB->Check  ( pDocDisplayAttr->bNonbreakingSpace );
        m_pSHyphCB->Check  ( pDocDisplayAttr->bSoftHyphen );
        m_pCharHiddenCB->Check ( pDocDisplayAttr->bCharHiddenText );
        m_pFldHiddenCB->Check  ( pDocDisplayAttr->bFldHiddenText );
        m_pFldHiddenParaCB->Check ( pDocDisplayAttr->bShowHiddenPara );
        m_pBreakCB->Check  ( pDocDisplayAttr->bManualBreak );
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
SwMarkPreview::SwMarkPreview( Window *pParent, WinBits nWinBits ) :

    Window(pParent, nWinBits),
    m_aTransCol( COL_TRANSPARENT ),
    m_aMarkCol( COL_LIGHTRED ),
    nMarkPos(0)

{
    m_aInitialSize = getPreviewOptionsSize(this);
    InitColors();
    SetMapMode(MAP_PIXEL);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwMarkPreview(Window *pParent, VclBuilder::stringmap &)
{
    return new SwMarkPreview(pParent, 0);
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

Size SwMarkPreview::GetOptimalSize() const
{
    return m_aInitialSize;
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
    : SfxTabPage(pParent, "OptRedLinePage",
        "modules/swriter/ui/optredlinepage.ui" , rSet)
    , sNone(SW_RESSTR(SW_STR_NONE))
{
    Size aPreviewSize(getPreviewOptionsSize(this));

    get(pInsertLB,"insert");
    get(pInsertColorLB,"insertcolor");
    get(pInsertedPreviewWN,"insertedpreview");

    get(pDeletedLB,"deleted");
    get(pDeletedColorLB,"deletedcolor");
    get(pDeletedPreviewWN,"deletedpreview");

    get(pChangedLB,"changed");
    get(pChangedColorLB,"changedcolor");
    get(pChangedPreviewWN,"changedpreview");

    get(pMarkPosLB,"markpos");
    get(pMarkColorLB,"markcolor");
    get(pMarkPreviewWN,"markpreview");

    pInsertedPreviewWN->set_height_request(aPreviewSize.Height());
    pDeletedPreviewWN->set_height_request(aPreviewSize.Height());
    pChangedPreviewWN->set_height_request(aPreviewSize.Height());
    pMarkPreviewWN->set_height_request(aPreviewSize.Height());

    pInsertedPreviewWN->set_width_request(aPreviewSize.Width());
    pDeletedPreviewWN->set_width_request(aPreviewSize.Width());
    pChangedPreviewWN->set_width_request(aPreviewSize.Width());
    pMarkPreviewWN->set_width_request(aPreviewSize.Width());

    sAuthor = get<Window>("byauthor")->GetText();

    for (sal_uInt16 i = 0; i < pInsertLB->GetEntryCount(); ++i)
    {
        OUString sEntry(pInsertLB->GetEntry(i));
        pDeletedLB->InsertEntry(sEntry);
        pChangedLB->InsertEntry(sEntry);
    };

    // remove strikethrough from insert and change and underline + double
    // underline from delete
    pInsertLB->RemoveEntry(5);
    pChangedLB->RemoveEntry(5);
    pDeletedLB->RemoveEntry(4);
    pDeletedLB->RemoveEntry(3);

    Link aLk = LINK(this, SwRedlineOptionsTabPage, AttribHdl);
    pInsertLB->SetSelectHdl( aLk );
    pDeletedLB->SetSelectHdl( aLk );
    pChangedLB->SetSelectHdl( aLk );

    aLk = LINK(this, SwRedlineOptionsTabPage, ColorHdl);
    pInsertColorLB->SetSelectHdl( aLk );
    pDeletedColorLB->SetSelectHdl( aLk );
    pChangedColorLB->SetSelectHdl( aLk );

    aLk = LINK(this, SwRedlineOptionsTabPage, ChangedMaskPrevHdl);
    pMarkPosLB->SetSelectHdl( aLk );
    pMarkColorLB->SetSelectHdl( aLk );
/*
    //solution: set different accessible name of four color box
    pInsertColorLB->SetAccessibleName(OUString( aInsertFT.GetDisplayText()) + OUString(aInsertColorFT.GetDisplayText()));
    pDeletedColorLB->SetAccessibleName(OUString( aDeletedFT.GetDisplayText()) + OUString( aDeletedColorFT.GetDisplayText()));
    pChangedColorLB->SetAccessibleName(OUString( aChangedFT.GetDisplayText()) + OUString( aChangedColorFT.GetDisplayText()));
    pMarkColorLB->SetAccessibleName(OUString( aMarkPosFT.GetDisplayText()) + OUString( aMarkColorFT.GetDisplayText()));*/
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

    sal_uInt16 nPos = pInsertLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)pInsertLB->GetEntryData(nPos);
        aInsertedAttr.nItemId = pAttr->nItemId;
        aInsertedAttr.nAttr = pAttr->nAttr;

        nPos = pInsertColorLB->GetSelectEntryPos();

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
                aInsertedAttr.nColor = pInsertColorLB->GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetInsertAuthorAttr(aInsertedAttr);
    }

    nPos = pDeletedLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)pDeletedLB->GetEntryData(nPos);
        aDeletedAttr.nItemId = pAttr->nItemId;
        aDeletedAttr.nAttr = pAttr->nAttr;

        nPos = pDeletedColorLB->GetSelectEntryPos();

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
                aDeletedAttr.nColor = pDeletedColorLB->GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetDeletedAuthorAttr(aDeletedAttr);
    }

    nPos = pChangedLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = (CharAttr *)pChangedLB->GetEntryData(nPos);
        aChangedAttr.nItemId = pAttr->nItemId;
        aChangedAttr.nAttr = pAttr->nAttr;

        nPos = pChangedColorLB->GetSelectEntryPos();

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
                aChangedAttr.nColor = pChangedColorLB->GetEntryColor(nPos).GetColor();
                break;
        }

        pOpt->SetFormatAuthorAttr(aChangedAttr);
    }

    nPos = 0;
    switch (pMarkPosLB->GetSelectEntryPos())
    {
        case 0: nPos = text::HoriOrientation::NONE;       break;
        case 1: nPos = text::HoriOrientation::LEFT;       break;
        case 2: nPos = text::HoriOrientation::RIGHT;      break;
        case 3: nPos = text::HoriOrientation::OUTSIDE;    break;
        case 4: nPos = text::HoriOrientation::INSIDE;     break;
    }
    pOpt->SetMarkAlignMode(nPos);

    pOpt->SetMarkAlignColor(pMarkColorLB->GetSelectEntryColor());

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
    InitFontStyle(*pInsertedPreviewWN);
    InitFontStyle(*pDeletedPreviewWN);
    InitFontStyle(*pChangedPreviewWN);

    // initialise colour list box
    pInsertColorLB->SetUpdateMode(false);
    pDeletedColorLB->SetUpdateMode(false);
    pChangedColorLB->SetUpdateMode(false);
    pMarkColorLB->SetUpdateMode(false);
    pInsertColorLB->InsertEntry(sNone);
    pDeletedColorLB->InsertEntry(sNone);
    pChangedColorLB->InsertEntry(sNone);

    pInsertColorLB->InsertEntry(sAuthor);
    pDeletedColorLB->InsertEntry(sAuthor);
    pChangedColorLB->InsertEntry(sAuthor);

    XColorListRef pColorLst = XColorList::GetStdColorList();
    sal_uInt16 i;
    for( i = 0; i < pColorLst->Count(); ++i )
    {
        XColorEntry* pEntry = pColorLst->GetColor( i );
        Color aColor = pEntry->GetColor();
        OUString sName = pEntry->GetName();

        pInsertColorLB->InsertEntry( aColor, sName );
        pDeletedColorLB->InsertEntry( aColor, sName );
        pChangedColorLB->InsertEntry( aColor, sName );
        pMarkColorLB->InsertEntry( aColor, sName );
    }
    pInsertColorLB->SetUpdateMode( true );
    pDeletedColorLB->SetUpdateMode( true );
    pChangedColorLB->SetUpdateMode( true );
    pMarkColorLB->SetUpdateMode( true );

    ColorData nColor = rInsertAttr.nColor;

    switch (nColor)
    {
        case COL_TRANSPARENT:
            pInsertColorLB->SelectEntryPos(1);
            break;
        case COL_NONE:
            pInsertColorLB->SelectEntryPos(0);
            break;
        default:
            pInsertColorLB->SelectEntry(Color(nColor));
    }

    nColor = rDeletedAttr.nColor;

    switch (nColor)
    {
        case COL_TRANSPARENT:
            pDeletedColorLB->SelectEntryPos(1);
            break;
        case COL_NONE:
            pDeletedColorLB->SelectEntryPos(0);
            break;
        default:
            pDeletedColorLB->SelectEntry(Color(nColor));
    }

    nColor = rChangedAttr.nColor;

    switch (nColor)
    {
        case COL_TRANSPARENT:
            pChangedColorLB->SelectEntryPos(1);
            break;
        case COL_NONE:
            pChangedColorLB->SelectEntryPos(0);
            break;
        default:
            pChangedColorLB->SelectEntry(Color(nColor));
    }

    pMarkColorLB->SelectEntry(pOpt->GetMarkAlignColor());

    pInsertLB->SelectEntryPos(0);
    pDeletedLB->SelectEntryPos(0);
    pChangedLB->SelectEntryPos(0);

    lcl_FillRedlineAttrListBox(*pInsertLB, rInsertAttr, aInsertAttrMap,
            sizeof(aInsertAttrMap) / sizeof(sal_uInt16));
    lcl_FillRedlineAttrListBox(*pDeletedLB, rDeletedAttr, aDeletedAttrMap,
            sizeof(aDeletedAttrMap) / sizeof(sal_uInt16));
    lcl_FillRedlineAttrListBox(*pChangedLB, rChangedAttr, aChangedAttrMap,
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
    pMarkPosLB->SelectEntryPos(nPos);

    // show settings in preview
    AttribHdl(pInsertLB);
    ColorHdl(pInsertColorLB);
    AttribHdl(pDeletedLB);
    ColorHdl(pInsertColorLB);
    AttribHdl(pChangedLB);
    ColorHdl(pChangedColorLB);

    ChangedMaskPrevHdl();
}

IMPL_LINK( SwRedlineOptionsTabPage, AttribHdl, ListBox *, pLB )
{
    SvxFontPrevWindow *pPrev = 0;
    ColorListBox *pColorLB;

    if (pLB == pInsertLB)
    {
        pColorLB = pInsertColorLB;
        pPrev = pInsertedPreviewWN;
    }
    else if (pLB == pDeletedLB)
    {
        pColorLB = pDeletedColorLB;
        pPrev = pDeletedPreviewWN;
    }
    else
    {
        pColorLB = pChangedColorLB;
        pPrev = pChangedPreviewWN;
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

    if (pColorLB == pInsertColorLB)
    {
        pLB = pInsertLB;
        pPrev = pInsertedPreviewWN;
    }
    else if (pColorLB == pDeletedColorLB)
    {
        pLB = pDeletedLB;
        pPrev = pDeletedPreviewWN;
    }
    else
    {
        pLB = pChangedLB;
        pPrev = pChangedPreviewWN;
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
    pMarkPreviewWN->SetMarkPos(pMarkPosLB->GetSelectEntryPos());
    pMarkPreviewWN->SetColor(pMarkColorLB->GetSelectEntryColor().GetColor());

    pMarkPreviewWN->Invalidate();

    return 0;
}

void SwRedlineOptionsTabPage::InitFontStyle(SvxFontPrevWindow& rExampleWin)
{
    const AllSettings&  rAllSettings = Application::GetSettings();
    LanguageType        eLangType = rAllSettings.GetUILanguageTag().getLanguageType();
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



SwCompareOptionsTabPage::SwCompareOptionsTabPage(  Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage( pParent,"OptComparison","modules/swriter/ui/optcomparison.ui", rSet )
{
    get(m_pAutoRB,"auto");
    get(m_pWordRB, "byword");
    get(m_pCharRB, "bycharacter");

    get(m_pRsidCB, "useRSID");
    get(m_pIgnoreCB, "ignore");
    get(m_pLenNF, "ignorelen");

    Link aLnk( LINK( this, SwCompareOptionsTabPage, ComparisonHdl ) );
    m_pAutoRB->SetClickHdl( aLnk );
    m_pWordRB->SetClickHdl( aLnk );
    m_pCharRB->SetClickHdl( aLnk );

    m_pIgnoreCB->SetClickHdl( LINK( this, SwCompareOptionsTabPage, IgnoreHdl) );
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

    if( m_pAutoRB->IsChecked() != m_pAutoRB->GetSavedValue() ||
        m_pWordRB->IsChecked() != m_pWordRB->GetSavedValue() ||
        m_pCharRB->IsChecked() != m_pCharRB->GetSavedValue() )
    {
        SvxCompareMode eCmpMode = SVX_CMP_AUTO;

        if ( m_pAutoRB->IsChecked() ) eCmpMode = SVX_CMP_AUTO;
        if ( m_pWordRB->IsChecked() ) eCmpMode = SVX_CMP_BY_WORD;
        if ( m_pCharRB->IsChecked() ) eCmpMode = SVX_CMP_BY_CHAR;

        pOpt->SetCompareMode( eCmpMode );
        bRet = sal_True;
    }

    if( TriState(m_pRsidCB->IsChecked()) != m_pRsidCB->GetSavedValue() )
    {
        pOpt->SetUseRsid( m_pRsidCB->IsChecked() );
        bRet = sal_True;
    }

    if( TriState(m_pIgnoreCB->IsChecked()) != m_pIgnoreCB->GetSavedValue() )
    {
        pOpt->SetIgnorePieces( m_pIgnoreCB->IsChecked() );
        bRet = sal_True;
    }

    if( m_pLenNF->IsModified() )
    {
        pOpt->SetPieceLen( m_pLenNF->GetValue() );
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
        m_pAutoRB->Check();
        m_pRsidCB->Disable();
        m_pIgnoreCB->Disable();
        m_pLenNF->Disable();
    }
    else if( eCmpMode == SVX_CMP_BY_WORD )
    {
        m_pWordRB->Check();
        m_pRsidCB->Enable();
        m_pIgnoreCB->Enable();
        m_pLenNF->Enable();
    }
    else if( eCmpMode == SVX_CMP_BY_CHAR)
    {
        m_pCharRB->Check();
        m_pRsidCB->Enable();
        m_pIgnoreCB->Enable();
        m_pLenNF->Enable();
    }
    m_pAutoRB->SaveValue();
    m_pWordRB->SaveValue();
    m_pCharRB->SaveValue();

    m_pRsidCB->Check( pOpt->IsUseRsid() );
    m_pRsidCB->SaveValue();

    m_pIgnoreCB->Check( pOpt->IsIgnorePieces() );
    m_pIgnoreCB->SaveValue();

    m_pLenNF->Enable( m_pIgnoreCB->IsChecked() && eCmpMode );

    m_pLenNF->SetValue( pOpt->GetPieceLen() );
    m_pLenNF->SaveValue();
}

IMPL_LINK_NOARG(SwCompareOptionsTabPage, ComparisonHdl)
{
    bool bChecked = !m_pAutoRB->IsChecked();
    m_pRsidCB->Enable( bChecked );
    m_pIgnoreCB->Enable( bChecked );
    m_pLenNF->Enable( bChecked && m_pIgnoreCB->IsChecked() );

    return 0;
}

IMPL_LINK_NOARG(SwCompareOptionsTabPage, IgnoreHdl)
{
    m_pLenNF->Enable( m_pIgnoreCB->IsChecked() );
    return 0;
}

#ifdef DBG_UTIL

SwTestTabPage::SwTestTabPage(Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage( pParent, "OptTestPage", "modules/swriter/ui/opttestpage.ui", rCoreSet)
    , bAttrModified( sal_False )
{
    get(m_pTest1CBox, "unused");
    get(m_pTest2CBox, "dynamic");
    get(m_pTest3CBox, "nocalm");
    get(m_pTest4CBox, "wysiwygdbg");
    get(m_pTest5CBox, "noidle");
    get(m_pTest6CBox, "noscreenadj");
    get(m_pTest7CBox, "winformat");
    get(m_pTest8CBox, "noscroll");
    get(m_pTest9CBox, "DrawingLayerNotLoading");
    get(m_pTest10CBox, "AutoFormatByInput");

    Init();

}

SfxTabPage* SwTestTabPage::Create( Window* pParent,
                                       const SfxItemSet& rAttrSet )
{
    return new SwTestTabPage(pParent, rAttrSet);
}

sal_Bool    SwTestTabPage::FillItemSet( SfxItemSet& rCoreSet )
{

    if ( bAttrModified )
    {
        SwTestItem aTestItem(FN_PARAM_SWTEST);
        aTestItem.bTest1=m_pTest1CBox->IsChecked();
        aTestItem.bTest2=m_pTest2CBox->IsChecked();
        aTestItem.bTest3=m_pTest3CBox->IsChecked();
        aTestItem.bTest4=m_pTest4CBox->IsChecked();
        aTestItem.bTest5=m_pTest5CBox->IsChecked();
        aTestItem.bTest6=m_pTest6CBox->IsChecked();
        aTestItem.bTest7=m_pTest7CBox->IsChecked();
        aTestItem.bTest8=m_pTest8CBox->IsChecked();
        aTestItem.bTest9=m_pTest9CBox->IsChecked();
        aTestItem.bTest10=m_pTest10CBox->IsChecked();
        rCoreSet.Put(aTestItem);
    }
    return bAttrModified;
}

void SwTestTabPage::Reset( const SfxItemSet& )
{
    const SfxItemSet& rSet = GetItemSet();
    const SwTestItem* pTestAttr = 0;

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_SWTEST , false,
                                    (const SfxPoolItem**)&pTestAttr ))
    {
        m_pTest1CBox->Check(pTestAttr->bTest1);
        m_pTest2CBox->Check(pTestAttr->bTest2);
        m_pTest3CBox->Check(pTestAttr->bTest3);
        m_pTest4CBox->Check(pTestAttr->bTest4);
        m_pTest5CBox->Check(pTestAttr->bTest5);
        m_pTest6CBox->Check(pTestAttr->bTest6);
        m_pTest7CBox->Check(pTestAttr->bTest7);
        m_pTest8CBox->Check(pTestAttr->bTest8);
        m_pTest9CBox->Check(pTestAttr->bTest9);
        m_pTest10CBox->Check(pTestAttr->bTest10);
    }
}

void SwTestTabPage::Init()
{
    // handler
    Link aLk = LINK( this, SwTestTabPage, AutoClickHdl );
    m_pTest1CBox->SetClickHdl( aLk );
    m_pTest2CBox->SetClickHdl( aLk );
    m_pTest3CBox->SetClickHdl( aLk );
    m_pTest4CBox->SetClickHdl( aLk );
    m_pTest5CBox->SetClickHdl( aLk );
    m_pTest6CBox->SetClickHdl( aLk );
    m_pTest7CBox->SetClickHdl( aLk );
    m_pTest8CBox->SetClickHdl( aLk );
    m_pTest9CBox->SetClickHdl( aLk );
    m_pTest10CBox->SetClickHdl( aLk );
}

IMPL_LINK_NOARG_INLINE_START(SwTestTabPage, AutoClickHdl)
{
    bAttrModified = sal_True;
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwTestTabPage, AutoClickHdl)


#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
