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
#include <IDocumentSettingAccess.hxx>
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
#include <swwrtshitem.hxx>
#include <unomid.h>

#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxenum.hxx>
#include <sal/macros.h>
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
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;

namespace {

void drawRect(vcl::RenderContext& rRenderContext, const Rectangle &rRect, const Color &rFillColor, const Color &rLineColor)
{
    rRenderContext.SetFillColor(rFillColor);
    rRenderContext.SetLineColor(rLineColor);
    rRenderContext.DrawRect(rRect);
}

}

// Tools->Options->Writer->View
// Tools->Options->Writer/Web->View
SwContentOptPage::SwContentOptPage( vcl::Window* pParent,
                                      const SfxItemSet& rCoreSet ) :
    SfxTabPage(pParent, "ViewOptionsPage",
               "modules/swriter/ui/viewoptionspage.ui", &rCoreSet)
{
    get (m_pCrossCB, "helplines");

    get (m_pHMetric, "hrulercombobox");
    get (m_pVRulerCBox, "vruler");
    get (m_pVRulerRightCBox, "vrulerright");
    get (m_pVMetric, "vrulercombobox");
    get (m_pSmoothCBox, "smoothscroll");

    get (m_pGrfCB, "graphics");
    get (m_pTableCB, "tables");
    get (m_pDrwCB, "drawings");
    get (m_pFieldNameCB, "fieldcodes");
    get (m_pPostItCB, "comments");

    get (m_pSettingsFrame, "settingsframe");
    get (m_pSettingsLabel, "settingslabel");
    get (m_pMetricLabel, "measureunitlabel");
    get (m_pMetricLB, "measureunit");

    /* This part is visible only with Writer/Web->View dialogue. */
    const SfxPoolItem* pItem;
    if (! (SfxItemState::SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem )
           && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON))
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

    SvxStringArray aMetricArr( SW_RES( STR_ARR_METRIC ) );
    for ( size_t i = 0; i < aMetricArr.Count(); ++i )
    {
        const OUString sMetric = aMetricArr.GetStringByPos( i );
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
                   sal_Int32 nPos = m_pMetricLB->InsertEntry( sMetric );
                   m_pMetricLB->SetEntryData( nPos, reinterpret_cast<void*>((sal_IntPtr)eFUnit) );
                   m_pHMetric->InsertEntry( sMetric );
                   m_pHMetric->SetEntryData( nPos, reinterpret_cast<void*>((sal_IntPtr)eFUnit) );
                }
                // a vertical ruler has not the 'character' unit
                if ( eFUnit != FUNIT_CHAR )
                {
                   sal_Int32 nPos = m_pVMetric->InsertEntry( sMetric );
                   m_pVMetric->SetEntryData( nPos, reinterpret_cast<void*>((sal_IntPtr)eFUnit) );
                }
                break;
            }
            default:;//prevent warning
        }
    }
}

SwContentOptPage::~SwContentOptPage()
{
    disposeOnce();
}

void SwContentOptPage::dispose()
{
    m_pCrossCB.clear();
    m_pHMetric.clear();
    m_pVRulerCBox.clear();
    m_pVRulerRightCBox.clear();
    m_pVMetric.clear();
    m_pSmoothCBox.clear();
    m_pGrfCB.clear();
    m_pTableCB.clear();
    m_pDrwCB.clear();
    m_pFieldNameCB.clear();
    m_pPostItCB.clear();
    m_pSettingsFrame.clear();
    m_pSettingsLabel.clear();
    m_pMetricLabel.clear();
    m_pMetricLB.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SwContentOptPage::Create( vcl::Window* pParent,
                                             const SfxItemSet* rAttrSet)
{
    return VclPtr<SwContentOptPage>::Create(pParent, *rAttrSet);
}

static void lcl_SelectMetricLB(ListBox* rMetric, sal_uInt16 nSID, const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if( rSet.GetItemState( nSID, false, &pItem ) >= SfxItemState::DEFAULT )
    {
        FieldUnit eFieldUnit = (FieldUnit)static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        for ( sal_Int32 i = 0; i < rMetric->GetEntryCount(); ++i )
        {
            if ( (int)reinterpret_cast<sal_IntPtr>(rMetric->GetEntryData( i )) == (int)eFieldUnit )
            {
                rMetric->SelectEntryPos( i );
                break;
            }
        }
    }
    rMetric->SaveValue();
}

void SwContentOptPage::Reset(const SfxItemSet* rSet)
{
    const SwElemItem* pElemAttr = nullptr;

    rSet->GetItemState( FN_PARAM_ELEM , false,
                                    reinterpret_cast<const SfxPoolItem**>(&pElemAttr) );
    if(pElemAttr)
    {
        m_pTableCB->Check (pElemAttr->bTable);
        m_pGrfCB->Check (pElemAttr->bGraphic);
        m_pDrwCB->Check (pElemAttr->bDrawing);
        m_pFieldNameCB->Check (pElemAttr->bFieldName);
        m_pPostItCB->Check (pElemAttr->bNotes);
        m_pCrossCB->Check (pElemAttr->bCrosshair);
        m_pVRulerCBox->Check (pElemAttr->bVertRuler);
        m_pVRulerRightCBox->Check (pElemAttr->bVertRulerRight);
        m_pSmoothCBox->Check (pElemAttr->bSmoothScroll);
    }
    m_pMetricLB->SetNoSelection();
    lcl_SelectMetricLB(m_pMetricLB, SID_ATTR_METRIC, *rSet);
    lcl_SelectMetricLB(m_pHMetric, FN_HSCROLL_METRIC, *rSet);
    lcl_SelectMetricLB(m_pVMetric, FN_VSCROLL_METRIC, *rSet);
}

bool SwContentOptPage::FillItemSet(SfxItemSet* rSet)
{
    const SwElemItem*   pOldAttr = static_cast<const SwElemItem*>(
                        GetOldItem(GetItemSet(), FN_PARAM_ELEM));

    SwElemItem aElem;
    if(pOldAttr)
        aElem = *pOldAttr;
    aElem.bTable                = m_pTableCB->IsChecked();
    aElem.bGraphic              = m_pGrfCB->IsChecked();
    aElem.bDrawing              = m_pDrwCB->IsChecked();
    aElem.bFieldName            = m_pFieldNameCB->IsChecked();
    aElem.bNotes                = m_pPostItCB->IsChecked();
    aElem.bCrosshair            = m_pCrossCB->IsChecked();
    aElem.bVertRuler            = m_pVRulerCBox->IsChecked();
    aElem.bVertRulerRight       = m_pVRulerRightCBox->IsChecked();
    aElem.bSmoothScroll         = m_pSmoothCBox->IsChecked();

    bool bRet = !pOldAttr || aElem != *pOldAttr;
    if(bRet)
        bRet = nullptr != rSet->Put(aElem);

    sal_Int32 nMPos = m_pMetricLB->GetSelectEntryPos();
    sal_Int32 nGlobalMetricPos = nMPos;
    if ( m_pMetricLB->IsValueChangedFromSaved() )
    {
        // Double-Cast for VA3.0
        const sal_uInt16 nFieldUnit = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( nMPos ));
        rSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    nMPos = m_pHMetric->GetSelectEntryPos();
    if ( m_pHMetric->IsValueChangedFromSaved() || nMPos != nGlobalMetricPos )
    {
        // Double-Cast for VA3.0
        const sal_uInt16 nFieldUnit = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pHMetric->GetEntryData( nMPos ));
        rSet->Put( SfxUInt16Item( FN_HSCROLL_METRIC, nFieldUnit ) );
        bRet = true;
    }
    nMPos = m_pVMetric->GetSelectEntryPos();
    if ( m_pVMetric->IsValueChangedFromSaved() || nMPos != nGlobalMetricPos )
    {
        // Double-Cast for VA3.0
        const sal_uInt16 nFieldUnit = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pVMetric->GetEntryData( nMPos ));
        rSet->Put( SfxUInt16Item( FN_VSCROLL_METRIC, nFieldUnit ) );
        bRet = true;
    }
    return bRet;
}

IMPL_LINK_TYPED(SwContentOptPage, VertRulerHdl, Button*, pBox, void)
{
    m_pVRulerRightCBox->Enable(pBox->IsEnabled() && static_cast<CheckBox*>(pBox)->IsChecked());
}

// TabPage Printer additional settings
SwAddPrinterTabPage::SwAddPrinterTabPage(vcl::Window* pParent,
    const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "PrintOptionsPage",
        "modules/swriter/ui/printoptionspage.ui", &rCoreSet)
    , sNone(SW_RESSTR(SW_STR_NONE))
    , bAttrModified(false)
    , bPreview(false)
{
    get(m_pGrfCB, "graphics");
    get(m_pCtrlFieldCB, "formcontrols");
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
    get(m_pInMarginsRB, "inmargins");
    get(m_pPrintEmptyPagesCB, "blankpages");
    get(m_pPaperFromSetupCB, "papertray");
    get(m_pFaxLB, "fax");

    Link<Button*,void> aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
    m_pGrfCB->SetClickHdl( aLk );
    m_pRightPageCB->SetClickHdl( aLk );
    m_pLeftPageCB->SetClickHdl( aLk );
    m_pCtrlFieldCB->SetClickHdl( aLk );
    m_pBackgroundCB->SetClickHdl( aLk );
    m_pBlackFontCB->SetClickHdl( aLk );
    m_pPrintHiddenTextCB->SetClickHdl( aLk );
    m_pPrintTextPlaceholderCB->SetClickHdl( aLk );
    m_pProspectCB->SetClickHdl( aLk );
    m_pProspectCB_RTL->SetClickHdl( aLk );
    m_pPaperFromSetupCB->SetClickHdl( aLk );
    m_pPrintEmptyPagesCB->SetClickHdl( aLk );
    m_pEndPageRB->SetClickHdl( aLk );
    m_pInMarginsRB->SetClickHdl( aLk );
    m_pEndRB->SetClickHdl( aLk );
    m_pOnlyRB->SetClickHdl( aLk );
    m_pNoRB->SetClickHdl( aLk );
    m_pFaxLB->SetSelectHdl( LINK( this, SwAddPrinterTabPage, SelectHdl ) );

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem )
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
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

SwAddPrinterTabPage::~SwAddPrinterTabPage()
{
    disposeOnce();
}

void SwAddPrinterTabPage::dispose()
{
    m_pGrfCB.clear();
    m_pCtrlFieldCB.clear();
    m_pBackgroundCB.clear();
    m_pBlackFontCB.clear();
    m_pPrintHiddenTextCB.clear();
    m_pPrintTextPlaceholderCB.clear();
    m_pPagesFrame.clear();
    m_pLeftPageCB.clear();
    m_pRightPageCB.clear();
    m_pProspectCB.clear();
    m_pProspectCB_RTL.clear();
    m_pCommentsFrame.clear();
    m_pNoRB.clear();
    m_pOnlyRB.clear();
    m_pEndRB.clear();
    m_pEndPageRB.clear();
    m_pInMarginsRB.clear();
    m_pPrintEmptyPagesCB.clear();
    m_pPaperFromSetupCB.clear();
    m_pFaxLB.clear();
    SfxTabPage::dispose();
}

void SwAddPrinterTabPage::SetPreview(bool bPrev)
{
    bPreview = bPrev;
    m_pCommentsFrame->Enable(!bPreview);
    m_pPagesFrame->Enable(!bPreview);
}

VclPtr<SfxTabPage> SwAddPrinterTabPage::Create( vcl::Window* pParent,
                                                const SfxItemSet* rAttrSet )
{
    return VclPtr<SwAddPrinterTabPage>::Create( pParent, *rAttrSet );
}

bool    SwAddPrinterTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( bAttrModified )
    {
        SwAddPrinterItem aAddPrinterAttr (FN_PARAM_ADDPRINTER);
        aAddPrinterAttr.m_bPrintGraphic   = m_pGrfCB->IsChecked();
        aAddPrinterAttr.m_bPrintTable     = true; // always enabled since CWS printerpullgpages /*aTabCB.IsChecked();*/
        aAddPrinterAttr.m_bPrintDraw      = m_pGrfCB->IsChecked(); // UI merged with m_pGrfCB in CWS printerpullgpages
        aAddPrinterAttr.m_bPrintControl   = m_pCtrlFieldCB->IsChecked();
        aAddPrinterAttr.m_bPrintPageBackground = m_pBackgroundCB->IsChecked();
        aAddPrinterAttr.m_bPrintBlackFont = m_pBlackFontCB->IsChecked();
        aAddPrinterAttr.m_bPrintHiddenText = m_pPrintHiddenTextCB->IsChecked();
        aAddPrinterAttr.m_bPrintTextPlaceholder = m_pPrintTextPlaceholderCB->IsChecked();

        aAddPrinterAttr.m_bPrintLeftPages     = m_pLeftPageCB->IsChecked();
        aAddPrinterAttr.m_bPrintRightPages    = m_pRightPageCB->IsChecked();
        aAddPrinterAttr.m_bPrintReverse       = false; // handled by vcl itself since CWS printerpullpages /*aReverseCB.IsChecked()*/;
        aAddPrinterAttr.m_bPrintProspect      = m_pProspectCB->IsChecked();
        aAddPrinterAttr.m_bPrintProspectRTL   = m_pProspectCB_RTL->IsChecked();
        aAddPrinterAttr.m_bPaperFromSetup     = m_pPaperFromSetupCB->IsChecked();
        aAddPrinterAttr.m_bPrintEmptyPages    = m_pPrintEmptyPagesCB->IsChecked();
        aAddPrinterAttr.m_bPrintSingleJobs    = true; // handled by vcl in new print dialog since CWS printerpullpages /*aSingleJobsCB.IsChecked()*/;

        if (m_pNoRB->IsChecked())  aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::NONE;
        if (m_pOnlyRB->IsChecked()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::Only;
        if (m_pEndRB->IsChecked()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::EndDoc;
        if (m_pEndPageRB->IsChecked()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::EndPage;
        if (m_pInMarginsRB->IsChecked()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::InMargins;

        const OUString sFax = m_pFaxLB->GetSelectEntry();
        aAddPrinterAttr.m_sFaxName = sNone == sFax ? aEmptyOUStr : sFax;
        rCoreSet->Put(aAddPrinterAttr);
    }
    return bAttrModified;
}

void    SwAddPrinterTabPage::Reset( const SfxItemSet*  )
{
    const   SfxItemSet&         rSet = GetItemSet();
    const   SwAddPrinterItem*   pAddPrinterAttr = nullptr;

    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_ADDPRINTER , false,
                                    reinterpret_cast<const SfxPoolItem**>(&pAddPrinterAttr) ))
    {
        m_pGrfCB->Check(pAddPrinterAttr->m_bPrintGraphic || pAddPrinterAttr->m_bPrintDraw);
        m_pCtrlFieldCB->Check(       pAddPrinterAttr->m_bPrintControl);
        m_pBackgroundCB->Check(    pAddPrinterAttr->m_bPrintPageBackground);
        m_pBlackFontCB->Check(     pAddPrinterAttr->m_bPrintBlackFont);
        m_pPrintHiddenTextCB->Check( pAddPrinterAttr->m_bPrintHiddenText);
        m_pPrintTextPlaceholderCB->Check(pAddPrinterAttr->m_bPrintTextPlaceholder);
        m_pLeftPageCB->Check(      pAddPrinterAttr->m_bPrintLeftPages);
        m_pRightPageCB->Check(     pAddPrinterAttr->m_bPrintRightPages);
        m_pPaperFromSetupCB->Check(pAddPrinterAttr->m_bPaperFromSetup);
        m_pPrintEmptyPagesCB->Check(pAddPrinterAttr->m_bPrintEmptyPages);
        m_pProspectCB->Check(      pAddPrinterAttr->m_bPrintProspect);
        m_pProspectCB_RTL->Check(      pAddPrinterAttr->m_bPrintProspectRTL);

        m_pNoRB->Check (pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::NONE ) ;
        m_pOnlyRB->Check (pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::Only ) ;
        m_pEndRB->Check (pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndDoc ) ;
        m_pEndPageRB->Check (pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndPage ) ;
        m_pInMarginsRB->Check (pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::InMargins ) ;
        m_pFaxLB->SelectEntry( pAddPrinterAttr->m_sFaxName );
    }
    if (m_pProspectCB->IsChecked())
    {
        m_pProspectCB_RTL->Enable();
        m_pNoRB->Enable( false );
        m_pOnlyRB->Enable( false );
        m_pEndRB->Enable( false );
        m_pEndPageRB->Enable( false );
    }
    else
        m_pProspectCB_RTL->Enable( false );
}

IMPL_LINK_NOARG_TYPED(SwAddPrinterTabPage, AutoClickHdl, Button*, void)
{
    bAttrModified = true;
    bool bIsProspect = m_pProspectCB->IsChecked();
    if (!bIsProspect)
        m_pProspectCB_RTL->Check( false );
    m_pProspectCB_RTL->Enable( bIsProspect );
    m_pNoRB->Enable( !bIsProspect );
    m_pOnlyRB->Enable( !bIsProspect );
    m_pEndRB->Enable( !bIsProspect );
    m_pEndPageRB->Enable( !bIsProspect );
    m_pInMarginsRB->Enable( !bIsProspect );
}

void  SwAddPrinterTabPage::SetFax( const std::vector<OUString>& rFaxLst )
{
    m_pFaxLB->InsertEntry(sNone);
    for(const auto & i : rFaxLst)
    {
        m_pFaxLB->InsertEntry(i);
    }
    m_pFaxLB->SelectEntryPos(0);
}

IMPL_LINK_NOARG_TYPED(SwAddPrinterTabPage, SelectHdl, ListBox&, void)
{
    bAttrModified=true;
}

void SwAddPrinterTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SfxBoolItem* pListItem = aSet.GetItem<SfxBoolItem>(SID_FAX_LIST, false);
    const SfxBoolItem* pPreviewItem = aSet.GetItem<SfxBoolItem>(SID_PREVIEWFLAG_TYPE, false);
    if (pPreviewItem)
    {
        SetPreview(pPreviewItem->GetValue());
        Reset(&aSet);
    }
    if (pListItem && pListItem->GetValue())
    {
        std::vector<OUString> aFaxList;
        const std::vector<OUString>& rPrinters = Printer::GetPrinterQueues();
        for (const auto & rPrinter : rPrinters)
            aFaxList.insert(aFaxList.begin(), rPrinter);
        SetFax( aFaxList );
    }
}

// Tabpage Standardfonts
SwStdFontTabPage::SwStdFontTabPage( vcl::Window* pParent,
                                       const SfxItemSet& rSet ) :
    SfxTabPage( pParent, "OptFontTabPage" , "modules/swriter/ui/optfonttabpage.ui" , &rSet),
    m_pPrt(nullptr),
    m_pFontList(nullptr),
    m_pFontConfig(nullptr),
    m_pWrtShell(nullptr),
    m_eLanguage( GetAppLanguage() ),

    m_bListDefault(false),
    m_bSetListDefault(true),
    m_bLabelDefault(false),
    m_bSetLabelDefault(true),
    m_bIdxDefault(false),
    m_bSetIdxDefault(true),

    m_bListHeightDefault    (false),
    m_bSetListHeightDefault (false),
    m_bLabelHeightDefault   (false),
    m_bSetLabelHeightDefault(false),
    m_bIndexHeightDefault     (false),
    m_bSetIndexHeightDefault  (false),

    m_nFontGroup(FONT_GROUP_DEFAULT),

    m_sScriptWestern(SW_RES(ST_SCRIPT_WESTERN)),
    m_sScriptAsian(SW_RES(ST_SCRIPT_ASIAN)),
    m_sScriptComplex(SW_RES(ST_SCRIPT_CTL))
{
    get(m_pLabelFT,"label1");
    get(m_pStandardBox,"standardbox");
    m_pStandardBox->SetStyle(m_pStandardBox->GetStyle() |  WB_SORT);
    get(m_pStandardHeightLB,"standardheight");
    get(m_pTitleBox,"titlebox");
    m_pTitleBox->SetStyle(m_pTitleBox->GetStyle() |  WB_SORT);
    get(m_pTitleHeightLB,"titleheight");
    get(m_pListBox,"listbox");
    m_pListBox->SetStyle(m_pListBox->GetStyle() |  WB_SORT);
    get(m_pListHeightLB,"listheight");
    get(m_pLabelBox,"labelbox");
    m_pLabelBox->SetStyle(m_pLabelBox->GetStyle() |  WB_SORT);
    get(m_pLabelHeightLB,"labelheight");
    get(m_pIdxBox,"idxbox");
    m_pIdxBox->SetStyle(m_pIdxBox->GetStyle() |  WB_SORT);
    get(m_pIndexHeightLB,"indexheight");

    get(m_pStandardPB,"standard");

    m_pStandardPB->SetClickHdl(LINK(this, SwStdFontTabPage, StandardHdl));
    m_pStandardBox->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    m_pListBox    ->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    m_pLabelBox   ->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    m_pIdxBox     ->SetModifyHdl( LINK(this, SwStdFontTabPage, ModifyHdl));
    Link<Control&,void> aFocusLink = LINK( this, SwStdFontTabPage, LoseFocusHdl);
    m_pStandardBox->SetLoseFocusHdl( aFocusLink );
    m_pTitleBox   ->SetLoseFocusHdl( aFocusLink );
    m_pListBox    ->SetLoseFocusHdl( aFocusLink );
    m_pLabelBox   ->SetLoseFocusHdl( aFocusLink );
    m_pIdxBox     ->SetLoseFocusHdl( aFocusLink );

    Link<Edit&,void> aModifyHeightLink( LINK( this, SwStdFontTabPage, ModifyHeightHdl));
    m_pStandardHeightLB->SetModifyHdl( aModifyHeightLink );
    m_pTitleHeightLB->   SetModifyHdl( aModifyHeightLink );
    m_pListHeightLB->    SetModifyHdl( aModifyHeightLink );
    m_pLabelHeightLB->   SetModifyHdl( aModifyHeightLink );
    m_pIndexHeightLB->   SetModifyHdl( aModifyHeightLink );
}

SwStdFontTabPage::~SwStdFontTabPage()
{
    disposeOnce();
}

void SwStdFontTabPage::dispose()
{
    delete m_pFontList;
    m_pLabelFT.clear();
    m_pStandardBox.clear();
    m_pStandardHeightLB.clear();
    m_pTitleBox.clear();
    m_pTitleHeightLB.clear();
    m_pListBox.clear();
    m_pListHeightLB.clear();
    m_pLabelBox.clear();
    m_pLabelHeightLB.clear();
    m_pIdxBox.clear();
    m_pIndexHeightLB.clear();
    m_pStandardPB.clear();
    m_pPrt.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwStdFontTabPage::Create( vcl::Window* pParent,
                                             const SfxItemSet* rAttrSet )
{
    return VclPtr<SwStdFontTabPage>::Create(pParent, *rAttrSet);
}

static void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    SfxPrinter* pPrt, const OUString& rStyle,
                    sal_uInt16 nFontWhich)
{
    vcl::Font aFont( rStyle, Size( 0, 10 ) );
    if( pPrt )
        aFont = pPrt->GetFontMetric( aFont );
    SwTextFormatColl *pColl = pWrtShell->GetTextCollFromPool(nType);
    pColl->SetFormatAttr(SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
                aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
}

static void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    sal_Int32 nHeight, sal_uInt16 nFontHeightWhich)
{
    float fSize = (float)nHeight / 10;
    nHeight = CalcToUnit( fSize, SFX_MAPUNIT_TWIP );
    SwTextFormatColl *pColl = pWrtShell->GetTextCollFromPool(nType);
    pColl->SetFormatAttr(SvxFontHeightItem(nHeight, 100, nFontHeightWhich));
}

bool SwStdFontTabPage::FillItemSet( SfxItemSet* )
{
    SW_MOD()->GetModuleConfig()->SetDefaultFontInCurrDocOnly(false);

    const OUString sStandard    = m_pStandardBox->GetText();
    const OUString sTitle       = m_pTitleBox->GetText();
    const OUString sList        = m_pListBox->GetText();
    const OUString sLabel       = m_pLabelBox->GetText();
    const OUString sIdx         = m_pIdxBox->GetText();

    bool bStandardHeightChanged = m_pStandardHeightLB->IsValueChangedFromSaved();
    bool bTitleHeightChanged = m_pTitleHeightLB->IsValueChangedFromSaved();
    bool bListHeightChanged = m_pListHeightLB->IsValueChangedFromSaved() && (!m_bListHeightDefault || !m_bSetListHeightDefault );
    bool bLabelHeightChanged = m_pLabelHeightLB->IsValueChangedFromSaved() && (!m_bLabelHeightDefault || !m_bSetLabelHeightDefault );
    bool bIndexHeightChanged = m_pIndexHeightLB->IsValueChangedFromSaved() && (!m_bIndexHeightDefault || !m_bSetIndexHeightDefault );

    m_pFontConfig->SetFontStandard(sStandard, m_nFontGroup);
    m_pFontConfig->SetFontOutline(sTitle, m_nFontGroup);
    m_pFontConfig->SetFontList(sList, m_nFontGroup);
    m_pFontConfig->SetFontCaption(sLabel, m_nFontGroup);
    m_pFontConfig->SetFontIndex(sIdx, m_nFontGroup);
    if(bStandardHeightChanged)
    {
        float fSize = (float)m_pStandardHeightLB->GetValue() / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_STANDARD, m_nFontGroup );
    }
    if(bTitleHeightChanged)
    {
        float fSize = (float)m_pTitleHeightLB->GetValue() / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_OUTLINE, m_nFontGroup );
    }
    if(bListHeightChanged)
    {
        float fSize = (float)m_pListHeightLB->GetValue() / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_LIST, m_nFontGroup );
    }
    if(bLabelHeightChanged)
    {
        float fSize = (float)m_pLabelHeightLB->GetValue() / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_CAPTION, m_nFontGroup );
    }
    if(bIndexHeightChanged)
    {
        float fSize = (float)m_pIndexHeightLB->GetValue() / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), FONT_INDEX, m_nFontGroup );
    }

    if(m_pWrtShell)
    {
        m_pWrtShell->StartAllAction();
        SfxPrinter* pPrinter = m_pWrtShell->getIDocumentDeviceAccess().getPrinter( false );
        bool bMod = false;
        const sal_uInt16 nFontWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT);
        const sal_uInt16 nFontHeightWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE);
        if(sStandard != m_sShellStd)
        {
            vcl::Font aFont( sStandard, Size( 0, 10 ) );
            if( pPrinter )
                aFont = pPrinter->GetFontMetric( aFont );
            m_pWrtShell->SetDefault(SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
                                  aEmptyOUStr, aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
            SwTextFormatColl *pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFormatAttr(nFontWhich);
            bMod = true;
        }
        if(bStandardHeightChanged)
        {
            float fSize = (float)m_pStandardHeightLB->GetValue() / 10;
            m_pWrtShell->SetDefault(SvxFontHeightItem( CalcToUnit( fSize, SFX_MAPUNIT_TWIP ), 100, nFontHeightWhich ) );
            SwTextFormatColl *pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFormatAttr(nFontHeightWhich);
            bMod = true;
        }

        if(sTitle != m_sShellTitle )
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_HEADLINE_BASE, pPrinter, sTitle, nFontWhich);
            bMod = true;
        }
        if(bTitleHeightChanged)
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_HEADLINE_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_pTitleHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if(sList != m_sShellList && (!m_bListDefault || !m_bSetListDefault ))
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_NUMBUL_BASE, pPrinter, sList, nFontWhich);
            bMod = true;
        }
        if(bListHeightChanged)
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_NUMBUL_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_pListHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if(sLabel != m_sShellLabel && (!m_bLabelDefault || !m_bSetLabelDefault))
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_LABEL, pPrinter, sLabel, nFontWhich);
            bMod = true;
        }
        if(bLabelHeightChanged)
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_LABEL,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_pLabelHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if(sIdx != m_sShellIndex && (!m_bIdxDefault || !m_bSetIdxDefault))
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_REGISTER_BASE, pPrinter, sIdx, nFontWhich);
            bMod = true;
        }
        if(bIndexHeightChanged)
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_REGISTER_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_pIndexHeightLB->GetValue()), nFontHeightWhich);
            bMod = true;
        }
        if ( bMod )
            m_pWrtShell->SetModified();
        m_pWrtShell->EndAllAction();
    }
    return false;
}

void SwStdFontTabPage::Reset( const SfxItemSet* rSet)
{
    const SfxPoolItem* pLang;
    const sal_uInt16 nLangSlot = m_nFontGroup == FONT_GROUP_DEFAULT  ? SID_ATTR_LANGUAGE :
        FONT_GROUP_CJK == m_nFontGroup ? SID_ATTR_CHAR_CJK_LANGUAGE : SID_ATTR_CHAR_CTL_LANGUAGE;

    if( SfxItemState::SET == rSet->GetItemState(nLangSlot, false, &pLang))
        m_eLanguage = static_cast<const SvxLanguageItem*>(pLang)->GetValue();

    OUString sToReplace = m_sScriptWestern;
    if(FONT_GROUP_CJK == m_nFontGroup )
        sToReplace = m_sScriptAsian;
    else if(FONT_GROUP_CTL == m_nFontGroup )
        sToReplace = m_sScriptComplex;
    m_pLabelFT->SetText(m_pLabelFT->GetText().replaceFirst("%1", sToReplace));

    const SfxPoolItem* pItem;

    if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_PRINTER, false, &pItem))
    {
        m_pPrt = static_cast<SfxPrinter*>(static_cast<const SwPtrItem*>(pItem)->GetValue());
    }
    else
    {
        SfxItemSet* pPrinterSet = new SfxItemSet( *rSet->GetPool(),
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );
        m_pPrt = VclPtr<SfxPrinter>::Create(pPrinterSet);
    }
    delete m_pFontList;
    m_pFontList = new FontList( m_pPrt );
    // #i94536# prevent duplication of font entries when 'reset' button is pressed
    if( !m_pStandardBox->GetEntryCount() )
    {
        // get the set of disctinct available family names
        std::set< OUString > aFontNames;
        int nFontNames = m_pPrt->GetDevFontCount();
        for( int i = 0; i < nFontNames; i++ )
        {
            FontMetric aFontMetric( m_pPrt->GetDevFont( i ) );
            aFontNames.insert( aFontMetric.GetFamilyName() );
        }

        // insert to listboxes
        for( std::set< OUString >::const_iterator it = aFontNames.begin();
             it != aFontNames.end(); ++it )
        {
            m_pStandardBox->InsertEntry( *it );
            m_pTitleBox->InsertEntry( *it );
            m_pListBox->InsertEntry( *it );
            m_pLabelBox->InsertEntry( *it );
            m_pIdxBox->InsertEntry( *it );
        }
    }
    if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_STDFONTS, false, &pItem))
    {
         m_pFontConfig = static_cast<SwStdFontConfig*>(static_cast<const SwPtrItem*>(pItem)->GetValue());
    }

    if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_WRTSHELL, false, &pItem))
    {
        m_pWrtShell = static_cast<SwWrtShell*>(static_cast<const SwPtrItem*>(pItem)->GetValue());
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

    if(!m_pWrtShell)
    {
        sStdBackup = m_pFontConfig->GetFontStandard(m_nFontGroup);
        sOutBackup = m_pFontConfig->GetFontOutline(m_nFontGroup);
        sListBackup= m_pFontConfig->GetFontList(m_nFontGroup);
        sCapBackup = m_pFontConfig->GetFontCaption(m_nFontGroup);
        sIdxBackup = m_pFontConfig->GetFontIndex(m_nFontGroup);
        nStandardHeight = m_pFontConfig->GetFontHeight( FONT_STANDARD, m_nFontGroup, m_eLanguage );
        nTitleHeight =    m_pFontConfig->GetFontHeight( FONT_OUTLINE , m_nFontGroup, m_eLanguage );
        nListHeight =     m_pFontConfig->GetFontHeight( FONT_LIST    , m_nFontGroup, m_eLanguage );
        nLabelHeight =    m_pFontConfig->GetFontHeight( FONT_CAPTION , m_nFontGroup, m_eLanguage );
        nIndexHeight =    m_pFontConfig->GetFontHeight( FONT_INDEX   , m_nFontGroup, m_eLanguage );
        if( nStandardHeight <= 0)
            nStandardHeight = SwStdFontConfig::GetDefaultHeightFor( FONT_STANDARD + m_nFontGroup * FONT_PER_GROUP, m_eLanguage);
        if( nTitleHeight <= 0)
            nTitleHeight = SwStdFontConfig::GetDefaultHeightFor( FONT_OUTLINE + m_nFontGroup * FONT_PER_GROUP, m_eLanguage);
        if( nListHeight <= 0)
            nListHeight = SwStdFontConfig::GetDefaultHeightFor( FONT_LIST + m_nFontGroup * FONT_PER_GROUP, m_eLanguage);
        if( nLabelHeight <= 0)
            nLabelHeight = SwStdFontConfig::GetDefaultHeightFor( FONT_CAPTION + m_nFontGroup * FONT_PER_GROUP, m_eLanguage);
        if( nIndexHeight <= 0)
            nIndexHeight = SwStdFontConfig::GetDefaultHeightFor( FONT_INDEX + m_nFontGroup * FONT_PER_GROUP, m_eLanguage);
    }
    else
    {
        SwTextFormatColl *pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_STANDARD);
        const SvxFontItem& rFont = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellStd = sStdBackup =  rFont.GetFamilyName();

        const sal_uInt16 nFontHeightWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE );
        const SvxFontHeightItem& rFontHeightStandard = static_cast<const SvxFontHeightItem& >(pColl->GetFormatAttr(nFontHeightWhich));
        nStandardHeight = (sal_Int32)rFontHeightStandard.GetHeight();

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_HEADLINE_BASE);
        const SvxFontItem& rFontHL = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellTitle = sOutBackup = rFontHL.GetFamilyName();

        const SvxFontHeightItem& rFontHeightTitle = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr( nFontHeightWhich ));
        nTitleHeight = (sal_Int32)rFontHeightTitle.GetHeight();

        const sal_uInt16 nFontWhich = sal::static_int_cast< sal_uInt16, RES_CHRATR >(
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT);
        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_NUMBUL_BASE);
        const SvxFontItem& rFontLS = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_bListDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        m_sShellList = sListBackup = rFontLS.GetFamilyName();

        const SvxFontHeightItem& rFontHeightList = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr(nFontHeightWhich));
        nListHeight = (sal_Int32)rFontHeightList.GetHeight();
        m_bListHeightDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_LABEL);
        m_bLabelDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontCP = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellLabel = sCapBackup = rFontCP.GetFamilyName();
        const SvxFontHeightItem& rFontHeightLabel = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr(nFontHeightWhich));
        nLabelHeight = (sal_Int32)rFontHeightLabel.GetHeight();
        m_bLabelHeightDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_REGISTER_BASE);
        m_bIdxDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontIDX = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellIndex = sIdxBackup = rFontIDX.GetFamilyName();
        const SvxFontHeightItem& rFontHeightIndex = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr(nFontHeightWhich));
        nIndexHeight = (sal_Int32)rFontHeightIndex.GetHeight();
        m_bIndexHeightDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
    }
    m_pStandardBox->SetText(sStdBackup );
    m_pTitleBox->SetText(sOutBackup );
    m_pListBox->SetText(sListBackup);
    m_pLabelBox->SetText(sCapBackup );
    m_pIdxBox->SetText(sIdxBackup );

    FontMetric aFontMetric( m_pFontList->Get(sStdBackup, sStdBackup) );
    m_pStandardHeightLB->Fill( &aFontMetric, m_pFontList );
    aFontMetric = m_pFontList->Get(sOutBackup, sOutBackup );
    m_pTitleHeightLB->Fill( &aFontMetric, m_pFontList );
    aFontMetric = m_pFontList->Get(sListBackup,sListBackup);
    m_pListHeightLB->Fill( &aFontMetric, m_pFontList );
    aFontMetric = m_pFontList->Get(sCapBackup, sCapBackup );
    m_pLabelHeightLB->Fill( &aFontMetric, m_pFontList );
    aFontMetric = m_pFontList->Get(sIdxBackup, sIdxBackup );
    m_pIndexHeightLB->Fill( &aFontMetric, m_pFontList );

    m_pStandardHeightLB->SetValue( CalcToPoint( nStandardHeight, SFX_MAPUNIT_TWIP, 10 ) );
    m_pTitleHeightLB->   SetValue( CalcToPoint( nTitleHeight   , SFX_MAPUNIT_TWIP, 10 ) );
    m_pListHeightLB->    SetValue( CalcToPoint( nListHeight    , SFX_MAPUNIT_TWIP, 10 ) );
    m_pLabelHeightLB->   SetValue( CalcToPoint( nLabelHeight   , SFX_MAPUNIT_TWIP, 10 ));
    m_pIndexHeightLB->   SetValue( CalcToPoint( nIndexHeight   , SFX_MAPUNIT_TWIP, 10 ));

    m_pStandardBox->SaveValue();
    m_pTitleBox->SaveValue();
    m_pListBox->SaveValue();
    m_pLabelBox->SaveValue();
    m_pIdxBox->SaveValue();

    m_pStandardHeightLB->SaveValue();
    m_pTitleHeightLB->SaveValue();
    m_pListHeightLB->SaveValue();
    m_pLabelHeightLB->SaveValue();
    m_pIndexHeightLB->SaveValue();
}

IMPL_LINK_NOARG_TYPED(SwStdFontTabPage, StandardHdl, Button*, void)
{
    sal_uInt8 nFontOffset = m_nFontGroup * FONT_PER_GROUP;
    m_pStandardBox->SetText(SwStdFontConfig::GetDefaultFor(FONT_STANDARD + nFontOffset, m_eLanguage));
    m_pTitleBox   ->SetText(SwStdFontConfig::GetDefaultFor(FONT_OUTLINE  + nFontOffset, m_eLanguage));
    m_pListBox    ->SetText(SwStdFontConfig::GetDefaultFor(FONT_LIST     + nFontOffset, m_eLanguage));
    m_pLabelBox   ->SetText(SwStdFontConfig::GetDefaultFor(FONT_CAPTION  + nFontOffset, m_eLanguage));
    m_pIdxBox     ->SetText(SwStdFontConfig::GetDefaultFor(FONT_INDEX    + nFontOffset, m_eLanguage));

    m_pStandardBox->SaveValue();
    m_pTitleBox->SaveValue();
    m_pListBox->SaveValue();
    m_pLabelBox->SaveValue();
    m_pIdxBox->SaveValue();

    m_pStandardHeightLB->SetValue( CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_STANDARD + nFontOffset, m_eLanguage),
            SFX_MAPUNIT_TWIP, 10 ) );
    m_pTitleHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_OUTLINE  +
            nFontOffset, m_eLanguage), SFX_MAPUNIT_TWIP, 10 ));
    m_pListHeightLB    ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_LIST + nFontOffset, m_eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    m_pLabelHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_CAPTION  + nFontOffset, m_eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
    m_pIndexHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_INDEX    + nFontOffset, m_eLanguage),
            SFX_MAPUNIT_TWIP, 10 ));
}

IMPL_LINK_TYPED( SwStdFontTabPage, ModifyHdl, Edit&, rBox, void )
{
    if(&rBox == m_pStandardBox)
    {
        const OUString sEntry = rBox.GetText();
        if(m_bSetListDefault && m_bListDefault)
            m_pListBox->SetText(sEntry);
        if(m_bSetLabelDefault && m_bLabelDefault)
            m_pLabelBox->SetText(sEntry);
        if(m_bSetIdxDefault && m_bIdxDefault)
            m_pIdxBox->SetText(sEntry);
    }
    else if(&rBox == m_pListBox)
    {
        m_bSetListDefault = false;
    }
    else if(&rBox == m_pLabelBox)
    {
        m_bSetLabelDefault = false;
    }
    else if(&rBox == m_pIdxBox)
    {
        m_bSetIdxDefault = false;
    }
}

IMPL_LINK_TYPED( SwStdFontTabPage, ModifyHeightHdl, Edit&, rBox, void )
{
    if(&rBox == m_pStandardHeightLB)
    {
        sal_Int64 nValue = static_cast<FontSizeBox&>(rBox).GetValue(FUNIT_TWIP);
        if(m_bSetListHeightDefault && m_bListHeightDefault)
            m_pListHeightLB->SetValue(nValue, FUNIT_TWIP);
        if(m_bSetLabelHeightDefault && m_bLabelHeightDefault)
            m_pLabelHeightLB->SetValue(nValue, FUNIT_TWIP);
        if(m_bSetIndexHeightDefault && m_bIndexHeightDefault)
            m_pIndexHeightLB->SetValue(nValue, FUNIT_TWIP);
    }
    else if(&rBox == m_pListHeightLB)
    {
        m_bSetListHeightDefault = false;
    }
    else if(&rBox == m_pLabelHeightLB)
    {
        m_bSetLabelHeightDefault = false;
    }
    else if(&rBox == m_pIndexHeightLB)
    {
        m_bSetIndexHeightDefault = false;
    }
}

IMPL_LINK_TYPED( SwStdFontTabPage, LoseFocusHdl, Control&, rControl, void )
{
    ComboBox* pBox = static_cast<ComboBox*>(&rControl);
    FontSizeBox* pHeightLB = nullptr;
    const OUString sEntry = pBox->GetText();
    if(pBox == m_pStandardBox)
    {
        pHeightLB = m_pStandardHeightLB;
    }
    else if(pBox == m_pTitleBox)
    {
        pHeightLB = m_pTitleHeightLB;
    }
    else if(pBox == m_pListBox)
    {
        pHeightLB = m_pListHeightLB;
    }
    else if(pBox == m_pLabelBox)
    {
        pHeightLB = m_pLabelHeightLB;
    }
    else /*if(pBox == pIdxBox)*/
    {
        pHeightLB = m_pIndexHeightLB;
    }
    FontMetric aFontMetric( m_pFontList->Get(sEntry, sEntry) );
    pHeightLB->Fill( &aFontMetric, m_pFontList );
}

void SwStdFontTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pFlagItem = aSet.GetItem<SfxUInt16Item>(SID_FONTMODE_TYPE, false);
    if (pFlagItem)
        SetFontMode(sal::static_int_cast< sal_uInt8, sal_uInt16>( pFlagItem->GetValue()));
}

SwTableOptionsTabPage::SwTableOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "OptTablePage", "modules/swriter/ui/opttablepage.ui", &rSet),
    m_pWrtShell(nullptr),
    m_bHTMLMode(false)
{
    get(m_pHeaderCB,"header");
    get(m_pRepeatHeaderCB,"repeatheader");
    get(m_pDontSplitCB,"dontsplit");
    get(m_pBorderCB,"border");
    get(m_pNumFormattingCB,"numformatting");
    get(m_pNumFormatFormattingCB,"numfmtformatting");
    get(m_pNumAlignmentCB,"numalignment");
    get(m_pRowMoveMF,"rowmove");
    get(m_pColMoveMF,"colmove");
    get(m_pRowInsertMF,"rowinsert");
    get(m_pColInsertMF,"colinsert");
    get(m_pFixRB,"fix");
    get(m_pFixPropRB,"fixprop");
    get(m_pVarRB,"var");

    Link<Button*,void> aLnk(LINK(this, SwTableOptionsTabPage, CheckBoxHdl));
    m_pNumFormattingCB->SetClickHdl(aLnk);
    m_pNumFormatFormattingCB->SetClickHdl(aLnk);
    m_pHeaderCB->SetClickHdl(aLnk);
}

SwTableOptionsTabPage::~SwTableOptionsTabPage()
{
    disposeOnce();
}

void SwTableOptionsTabPage::dispose()
{
    m_pHeaderCB.clear();
    m_pRepeatHeaderCB.clear();
    m_pDontSplitCB.clear();
    m_pBorderCB.clear();
    m_pNumFormattingCB.clear();
    m_pNumFormatFormattingCB.clear();
    m_pNumAlignmentCB.clear();
    m_pRowMoveMF.clear();
    m_pColMoveMF.clear();
    m_pRowInsertMF.clear();
    m_pColInsertMF.clear();
    m_pFixRB.clear();
    m_pFixPropRB.clear();
    m_pVarRB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwTableOptionsTabPage::Create( vcl::Window* pParent,
                                                  const SfxItemSet* rAttrSet )
{
    return VclPtr<SwTableOptionsTabPage>::Create(pParent, *rAttrSet);
}

bool SwTableOptionsTabPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    if(m_pRowMoveMF->IsModified())
        pModOpt->SetTableHMove( (sal_uInt16)m_pRowMoveMF->Denormalize( m_pRowMoveMF->GetValue(FUNIT_TWIP)));

    if(m_pColMoveMF->IsModified())
        pModOpt->SetTableVMove( (sal_uInt16)m_pColMoveMF->Denormalize( m_pColMoveMF->GetValue(FUNIT_TWIP)));

    if(m_pRowInsertMF->IsModified())
        pModOpt->SetTableHInsert((sal_uInt16)m_pRowInsertMF->Denormalize( m_pRowInsertMF->GetValue(FUNIT_TWIP)));

    if(m_pColInsertMF->IsModified())
        pModOpt->SetTableVInsert((sal_uInt16)m_pColInsertMF->Denormalize( m_pColInsertMF->GetValue(FUNIT_TWIP)));

    TableChgMode eMode;
    if(m_pFixRB->IsChecked())
        eMode = TBLFIX_CHGABS;
    else if(m_pFixPropRB->IsChecked())
        eMode = TBLFIX_CHGPROP;
    else
        eMode = TBLVAR_CHGABS;
    if(eMode != pModOpt->GetTableMode())
    {
        pModOpt->SetTableMode(eMode);
        // the table-keyboard-mode has changed, now the current
        // table should know about that too.
        if(m_pWrtShell && nsSelectionType::SEL_TBL & m_pWrtShell->GetSelectionType())
        {
            m_pWrtShell->SetTableChgMode(eMode);
            static sal_uInt16 aInva[] =
                                {   FN_TABLE_MODE_FIX,
                                    FN_TABLE_MODE_FIX_PROP,
                                    FN_TABLE_MODE_VARIABLE,
                                    0
                                };
            m_pWrtShell->GetView().GetViewFrame()->GetBindings().Invalidate( aInva );
        }

        bRet = true;
    }

    SwInsertTableOptions aInsOpts( 0, 0 );

    if (m_pHeaderCB->IsChecked())
        aInsOpts.mnInsMode |= tabopts::HEADLINE;

    if (m_pRepeatHeaderCB->IsEnabled() )
        aInsOpts.mnRowsToRepeat = m_pRepeatHeaderCB->IsChecked()? 1 : 0;

    if (!m_pDontSplitCB->IsChecked())
        aInsOpts.mnInsMode |= tabopts::SPLIT_LAYOUT;

    if (m_pBorderCB->IsChecked())
        aInsOpts.mnInsMode |= tabopts::DEFAULT_BORDER;

    if (m_pHeaderCB->IsValueChangedFromSaved() ||
        m_pRepeatHeaderCB->IsValueChangedFromSaved() ||
        m_pDontSplitCB->IsValueChangedFromSaved() ||
        m_pBorderCB->IsValueChangedFromSaved())
    {
        pModOpt->SetInsTableFlags(m_bHTMLMode, aInsOpts);
    }

    if (m_pNumFormattingCB->IsValueChangedFromSaved())
    {
        pModOpt->SetInsTableFormatNum(m_bHTMLMode, m_pNumFormattingCB->IsChecked());
        bRet = true;
    }

    if (m_pNumFormatFormattingCB->IsValueChangedFromSaved())
    {
        pModOpt->SetInsTableChangeNumFormat(m_bHTMLMode, m_pNumFormatFormattingCB->IsChecked());
        bRet = true;
    }

    if (m_pNumAlignmentCB->IsValueChangedFromSaved())
    {
        pModOpt->SetInsTableAlignNum(m_bHTMLMode, m_pNumAlignmentCB->IsChecked());
        bRet = true;
    }

    return bRet;
}

void SwTableOptionsTabPage::Reset( const SfxItemSet* rSet)
{
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    if ( rSet->GetItemState( SID_ATTR_METRIC ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(rSet->Get( SID_ATTR_METRIC ));
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();
        ::SetFieldUnit( *m_pRowMoveMF, eFieldUnit );
        ::SetFieldUnit( *m_pColMoveMF, eFieldUnit );
        ::SetFieldUnit( *m_pRowInsertMF, eFieldUnit );
        ::SetFieldUnit( *m_pColInsertMF, eFieldUnit );
    }

    m_pRowMoveMF->SetValue(m_pRowMoveMF->Normalize(pModOpt->GetTableHMove()), FUNIT_TWIP);
    m_pColMoveMF->SetValue(m_pColMoveMF->Normalize(pModOpt->GetTableVMove()), FUNIT_TWIP);
    m_pRowInsertMF->SetValue(m_pRowInsertMF->Normalize(pModOpt->GetTableHInsert()), FUNIT_TWIP);
    m_pColInsertMF->SetValue(m_pColInsertMF->Normalize(pModOpt->GetTableVInsert()), FUNIT_TWIP);

    switch(pModOpt->GetTableMode())
    {
        case TBLFIX_CHGABS:     m_pFixRB->Check();     break;
        case TBLFIX_CHGPROP:    m_pFixPropRB->Check(); break;
        case TBLVAR_CHGABS:     m_pVarRB->Check(); break;
    }
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet->GetItemState(SID_HTML_MODE, false, &pItem))
    {
        m_bHTMLMode = 0 != (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);
    }

    // hide certain controls for html
    if(m_bHTMLMode)
    {
        m_pRepeatHeaderCB->Hide();
        m_pDontSplitCB->Hide();
    }

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(m_bHTMLMode);
    const sal_uInt16 nInsTableFlags = aInsOpts.mnInsMode;

    m_pHeaderCB->Check(0 != (nInsTableFlags & tabopts::HEADLINE));
    m_pRepeatHeaderCB->Check((!m_bHTMLMode) && (aInsOpts.mnRowsToRepeat > 0));
    m_pDontSplitCB->Check(!(nInsTableFlags & tabopts::SPLIT_LAYOUT));
    m_pBorderCB->Check(0 != (nInsTableFlags & tabopts::DEFAULT_BORDER));

    m_pNumFormattingCB->Check(pModOpt->IsInsTableFormatNum(m_bHTMLMode));
    m_pNumFormatFormattingCB->Check(pModOpt->IsInsTableChangeNumFormat(m_bHTMLMode));
    m_pNumAlignmentCB->Check(pModOpt->IsInsTableAlignNum(m_bHTMLMode));

    m_pHeaderCB->SaveValue();
    m_pRepeatHeaderCB->SaveValue();
    m_pDontSplitCB->SaveValue();
    m_pBorderCB->SaveValue();
    m_pNumFormattingCB->SaveValue();
    m_pNumFormatFormattingCB->SaveValue();
    m_pNumAlignmentCB->SaveValue();

    CheckBoxHdl(nullptr);
}

IMPL_LINK_NOARG_TYPED(SwTableOptionsTabPage, CheckBoxHdl, Button*, void)
{
    m_pNumFormatFormattingCB->Enable(m_pNumFormattingCB->IsChecked());
    m_pNumAlignmentCB->Enable(m_pNumFormattingCB->IsChecked());
    m_pRepeatHeaderCB->Enable(m_pHeaderCB->IsChecked());
}

void SwTableOptionsTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SwWrtShellItem* pWrtSh = aSet.GetItem<SwWrtShellItem>(SID_WRT_SHELL, false);
    if (pWrtSh)
        SetWrtShell(pWrtSh->GetValue());
}

SwShdwCursorOptionsTabPage::SwShdwCursorOptionsTabPage( vcl::Window* pParent,
                                                    const SfxItemSet& rSet )
   : SfxTabPage(pParent, "OptFormatAidsPage",
                "modules/swriter/ui/optformataidspage.ui", &rSet),
    m_pWrtShell( nullptr )
{
    get(m_pParaCB, "paragraph");
    get(m_pSHyphCB, "hyphens");
    get(m_pSpacesCB, "spaces");
    get(m_pHSpacesCB, "nonbreak");
    get(m_pTabCB, "tabs");
    get(m_pBreakCB, "break");
    get(m_pCharHiddenCB, "hiddentext");
    get(m_pFieldHiddenCB, "hiddentextfield");
    get(m_pFieldHiddenParaCB, "hiddenparafield");

    get(m_pDirectCursorFrame, "directcrsrframe");
    get(m_pOnOffCB, "cursoronoff");

    get(m_pFillMarginRB, "fillmargin");
    get(m_pFillIndentRB, "fillindent");
    get(m_pFillTabRB, "filltab");
    get(m_pFillSpaceRB, "fillspace");

    get(m_pCursorProtFrame, "crsrprotframe");
    get(m_pCursorInProtCB, "cursorinprot");
    get(m_pIgnoreProtCB, "ignoreprot");

    get(m_pMathBaselineAlignmentCB, "mathbaseline");

    const SfxPoolItem* pItem = nullptr;

    SwShadowCursorItem aOpt;
    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem ))
        aOpt = *static_cast<const SwShadowCursorItem*>(pItem);
    m_pOnOffCB->Check( aOpt.IsOn() );

    sal_uInt8 eMode = aOpt.GetMode();
    m_pFillIndentRB->Check( FILL_INDENT == eMode );
    m_pFillMarginRB->Check( FILL_MARGIN == eMode );
    m_pFillTabRB->Check( FILL_TAB == eMode );
    m_pFillSpaceRB->Check( FILL_SPACE == eMode );

    if(SfxItemState::SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem )
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
    {
        m_pTabCB->Hide();
        m_pCharHiddenCB->Hide();
        m_pFieldHiddenCB->Hide();
        m_pFieldHiddenParaCB->Hide();

        m_pDirectCursorFrame->Hide();
        m_pOnOffCB->Hide();
        m_pFillMarginRB->Hide();
        m_pFillIndentRB->Hide();
        m_pFillTabRB->Hide();
        m_pFillSpaceRB->Hide();

        m_pCursorProtFrame->Hide();
        m_pCursorInProtCB->Hide();
        m_pIgnoreProtCB->Hide();
    }
}

SwShdwCursorOptionsTabPage::~SwShdwCursorOptionsTabPage()
{
    disposeOnce();
}

void SwShdwCursorOptionsTabPage::dispose()
{
    m_pParaCB.clear();
    m_pSHyphCB.clear();
    m_pSpacesCB.clear();
    m_pHSpacesCB.clear();
    m_pTabCB.clear();
    m_pBreakCB.clear();
    m_pCharHiddenCB.clear();
    m_pFieldHiddenCB.clear();
    m_pFieldHiddenParaCB.clear();
    m_pDirectCursorFrame.clear();
    m_pOnOffCB.clear();
    m_pFillMarginRB.clear();
    m_pFillIndentRB.clear();
    m_pFillTabRB.clear();
    m_pFillSpaceRB.clear();
    m_pCursorProtFrame.clear();
    m_pCursorInProtCB.clear();
    m_pIgnoreProtCB.clear();
    m_pMathBaselineAlignmentCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwShdwCursorOptionsTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet )
{
    return VclPtr<SwShdwCursorOptionsTabPage>::Create( pParent, *rSet );
}

void SwShdwCursorOptionsTabPage::PageCreated( const SfxAllItemSet& aSet )
{
    const SwWrtShellItem* pWrtSh = aSet.GetItem<SwWrtShellItem>(SID_WRT_SHELL, false);
    if (pWrtSh)
        SetWrtShell(pWrtSh->GetValue());
}

bool SwShdwCursorOptionsTabPage::FillItemSet( SfxItemSet* rSet )
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
    const SfxPoolItem* pItem = nullptr;
    if( SfxItemState::SET != rSet->GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem )
        ||  static_cast<const SwShadowCursorItem&>(*pItem) != aOpt )
    {
        rSet->Put( aOpt );
        bRet = true;
    }

    if (m_pWrtShell) {
        m_pWrtShell->GetDoc()->getIDocumentSettingAccess().set( DocumentSettingId::MATH_BASELINE_ALIGNMENT,
                                    m_pMathBaselineAlignmentCB->IsChecked() );
        bRet |= m_pMathBaselineAlignmentCB->IsValueChangedFromSaved();
    }

    if( m_pCursorInProtCB->IsValueChangedFromSaved())
    {
        rSet->Put(SfxBoolItem(FN_PARAM_CRSR_IN_PROTECTED, m_pCursorInProtCB->IsChecked()));
        bRet = true;
    }

    if (m_pIgnoreProtCB->IsValueChangedFromSaved())
    {
        rSet->Put(SfxBoolItem(FN_PARAM_IGNORE_PROTECTED, m_pIgnoreProtCB->IsChecked()));
        bRet = true;
    }

    const SwDocDisplayItem* pOldAttr = static_cast<const SwDocDisplayItem*>(
                        GetOldItem(GetItemSet(), FN_PARAM_DOCDISP));

    SwDocDisplayItem aDisp;
    if(pOldAttr)
        aDisp = *pOldAttr;

    aDisp.bParagraphEnd         = m_pParaCB->IsChecked();
    aDisp.bTab                  = m_pTabCB->IsChecked();
    aDisp.bSpace                = m_pSpacesCB->IsChecked();
    aDisp.bNonbreakingSpace     = m_pHSpacesCB->IsChecked();
    aDisp.bSoftHyphen           = m_pSHyphCB->IsChecked();
    aDisp.bFieldHiddenText        = m_pFieldHiddenCB->IsChecked();
    aDisp.bCharHiddenText       = m_pCharHiddenCB->IsChecked();
    aDisp.bShowHiddenPara       = m_pFieldHiddenParaCB->IsChecked();
    aDisp.bManualBreak          = m_pBreakCB->IsChecked();

    bRet |= (!pOldAttr || aDisp != *pOldAttr);
    if(bRet)
        bRet = nullptr != rSet->Put(aDisp);

    return bRet;
}

void SwShdwCursorOptionsTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pItem = nullptr;

    SwShadowCursorItem aOpt;
    if( SfxItemState::SET == rSet->GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem ))
        aOpt = *static_cast<const SwShadowCursorItem*>(pItem);
    m_pOnOffCB->Check( aOpt.IsOn() );

    sal_uInt8 eMode = aOpt.GetMode();
    m_pFillIndentRB->Check( FILL_INDENT == eMode );
    m_pFillMarginRB->Check( FILL_MARGIN == eMode );
    m_pFillTabRB->Check( FILL_TAB == eMode );
    m_pFillSpaceRB->Check( FILL_SPACE == eMode );

    if (m_pWrtShell) {
       m_pMathBaselineAlignmentCB->Check( m_pWrtShell->GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT ) );
       m_pMathBaselineAlignmentCB->SaveValue();
    } else {
        m_pMathBaselineAlignmentCB->Hide();
    }

    if( SfxItemState::SET == rSet->GetItemState( FN_PARAM_CRSR_IN_PROTECTED, false, &pItem ))
        m_pCursorInProtCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    m_pCursorInProtCB->SaveValue();

    if (rSet->GetItemState(FN_PARAM_IGNORE_PROTECTED, false, &pItem) == SfxItemState::SET)
        m_pIgnoreProtCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    m_pIgnoreProtCB->SaveValue();

    const SwDocDisplayItem* pDocDisplayAttr = nullptr;

    rSet->GetItemState( FN_PARAM_DOCDISP, false,
                                    reinterpret_cast<const SfxPoolItem**>(&pDocDisplayAttr) );
    if(pDocDisplayAttr)
    {
        m_pParaCB->Check  ( pDocDisplayAttr->bParagraphEnd );
        m_pTabCB->Check  ( pDocDisplayAttr->bTab );
        m_pSpacesCB->Check  ( pDocDisplayAttr->bSpace );
        m_pHSpacesCB->Check  ( pDocDisplayAttr->bNonbreakingSpace );
        m_pSHyphCB->Check  ( pDocDisplayAttr->bSoftHyphen );
        m_pCharHiddenCB->Check ( pDocDisplayAttr->bCharHiddenText );
        m_pFieldHiddenCB->Check  ( pDocDisplayAttr->bFieldHiddenText );
        m_pFieldHiddenParaCB->Check ( pDocDisplayAttr->bShowHiddenPara );
        m_pBreakCB->Check  ( pDocDisplayAttr->bManualBreak );
    }
}

// TabPage for Redlining
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
    { SID_ATTR_CHAR_UNDERLINE,      LINESTYLE_SINGLE },
    { SID_ATTR_CHAR_UNDERLINE,      LINESTYLE_DOUBLE },
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

// Preview of selection
SwMarkPreview::SwMarkPreview( vcl::Window *pParent, WinBits nWinBits ) :

    Window(pParent, nWinBits),
    m_aTransCol( COL_TRANSPARENT ),
    m_aMarkCol( COL_LIGHTRED ),
    nMarkPos(0)

{
    m_aInitialSize = getPreviewOptionsSize(this);
    InitColors();
    SetMapMode(MAP_PIXEL);
}

VCL_BUILDER_FACTORY_ARGS(SwMarkPreview, 0)

SwMarkPreview::~SwMarkPreview()
{
}

void SwMarkPreview::InitColors()
{
    // m_aTransCol and m_aMarkCol are _not_ changed because they are set from outside!

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    m_aBgCol = Color( rSettings.GetWindowColor() );

    bool bHC = rSettings.GetHighContrastMode();
    m_aLineCol = bHC? SwViewOption::GetFontColor() : Color( COL_BLACK );
    m_aShadowCol = bHC? m_aBgCol : rSettings.GetShadowColor();
    m_aTextCol = bHC? SwViewOption::GetFontColor() : Color( COL_GRAY );
    m_aPrintAreaCol = m_aTextCol;
}

void SwMarkPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if( rDCEvt.GetType() == DataChangedEventType::SETTINGS && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        InitColors();
}

void SwMarkPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle &/*rRect*/)
{
    const Size aSz(GetOutputSizePixel());

    // Page
    aPage.SetSize(Size(aSz.Width() - 3, aSz.Height() - 3));

    const long nOutWPix = aPage.GetWidth();
    const long nOutHPix = aPage.GetHeight();

    // PrintArea
    const long nLBorder = 8;
    const long nRBorder = 8;
    const long nTBorder = 4;
    const long nBBorder = 4;

    aLeftPagePrtArea = Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));
    const long nWidth = aLeftPagePrtArea.GetWidth();
    const long nKorr = (nWidth & 1) != 0 ? 0 : 1;
    aLeftPagePrtArea.SetSize(Size(nWidth / 2 - (nLBorder + nRBorder) / 2 + nKorr, aLeftPagePrtArea.GetHeight()));

    aRightPagePrtArea = aLeftPagePrtArea;
    aRightPagePrtArea.Move(aLeftPagePrtArea.GetWidth() + nLBorder + nRBorder + 1, 0);

    // draw shadow
    Rectangle aShadow(aPage);
    aShadow += Point(3, 3);
    drawRect(rRenderContext, aShadow, m_aShadowCol, m_aTransCol);

    // draw page
    drawRect(rRenderContext, aPage, m_aBgCol, m_aLineCol);

    // draw separator
    Rectangle aPageSeparator(aPage);
    aPageSeparator.SetSize(Size(2, aPageSeparator.GetHeight()));
    aPageSeparator.Move(aPage.GetWidth() / 2 - 1, 0);
    drawRect(rRenderContext, aPageSeparator, m_aLineCol, m_aTransCol);

    PaintPage(rRenderContext, aLeftPagePrtArea);
    PaintPage(rRenderContext, aRightPagePrtArea);

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
    drawRect(rRenderContext, aLeftMark, m_aMarkCol, m_aTransCol);
    drawRect(rRenderContext, aRightMark, m_aMarkCol, m_aTransCol);
}

void SwMarkPreview::PaintPage(vcl::RenderContext& rRenderContext, const Rectangle &rRect)
{
    // draw PrintArea
    drawRect(rRenderContext, rRect, m_aTransCol, m_aPrintAreaCol);

    // draw Testparagraph
    sal_uLong nLTextBorder = 4;
    sal_uLong nRTextBorder = 4;
    sal_uLong nTTextBorder = 4;

    Rectangle aTextLine = rRect;
    aTextLine.SetSize(Size(aTextLine.GetWidth(), 2));
    aTextLine.Left()    += nLTextBorder;
    aTextLine.Right()   -= nRTextBorder;
    aTextLine.Move(0, nTTextBorder);

    const long nStep = aTextLine.GetHeight() + 2;
    const long nLines = rRect.GetHeight() / (aTextLine.GetHeight() + 2) - 1;

    // simulate text
    for (long i = 0; i < nLines; ++i)
    {
        if (i == (nLines - 1))
            aTextLine.SetSize(Size(aTextLine.GetWidth() / 2, aTextLine.GetHeight()));

        if (aPage.IsInside(aTextLine))
            drawRect(rRenderContext, aTextLine, m_aTextCol, m_aTransCol);

        aTextLine.Move(0, nStep);
    }
    aTextLine.Move(0, -nStep);
}

Size SwMarkPreview::GetOptimalSize() const
{
    return m_aInitialSize;
}

namespace
{
    void lcl_FillRedlineAttrListBox(
            ListBox& rLB, const AuthorCharAttr& rAttrToSelect,
            const sal_uInt16* pAttrMap, const size_t nAttrMapSize)
    {
        for (size_t i = 0; i != nAttrMapSize; ++i)
        {
            CharAttr& rAttr(aRedlineAttr[pAttrMap[i]]);
            rLB.SetEntryData(i, &rAttr);
            if (rAttr.nItemId == rAttrToSelect.nItemId &&
                rAttr.nAttr == rAttrToSelect.nAttr)
                rLB.SelectEntryPos(i);
        }
    }
}

SwRedlineOptionsTabPage::SwRedlineOptionsTabPage( vcl::Window* pParent,
                                                  const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptRedLinePage",
        "modules/swriter/ui/optredlinepage.ui" , &rSet)
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

    sAuthor = get<vcl::Window>("byauthor")->GetText();

    for (sal_Int32 i = 0; i < pInsertLB->GetEntryCount(); ++i)
    {
        const OUString sEntry(pInsertLB->GetEntry(i));
        pDeletedLB->InsertEntry(sEntry);
        pChangedLB->InsertEntry(sEntry);
    };

    // remove strikethrough from insert and change and underline + double
    // underline from delete
    pInsertLB->RemoveEntry(5);
    pChangedLB->RemoveEntry(5);
    pDeletedLB->RemoveEntry(4);
    pDeletedLB->RemoveEntry(3);

    Link<ListBox&,void> aLk = LINK(this, SwRedlineOptionsTabPage, AttribHdl);
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
}

SwRedlineOptionsTabPage::~SwRedlineOptionsTabPage()
{
    disposeOnce();
}

void SwRedlineOptionsTabPage::dispose()
{
    pInsertLB.clear();
    pInsertColorLB.clear();
    pInsertedPreviewWN.clear();
    pDeletedLB.clear();
    pDeletedColorLB.clear();
    pDeletedPreviewWN.clear();
    pChangedLB.clear();
    pChangedColorLB.clear();
    pChangedPreviewWN.clear();
    pMarkPosLB.clear();
    pMarkColorLB.clear();
    pMarkPreviewWN.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwRedlineOptionsTabPage::Create( vcl::Window* pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwRedlineOptionsTabPage>::Create( pParent, *rSet );
}

bool SwRedlineOptionsTabPage::FillItemSet( SfxItemSet* )
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

    sal_Int32 nPos = pInsertLB->GetSelectEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = static_cast<CharAttr *>(pInsertLB->GetEntryData(nPos));
        aInsertedAttr.nItemId = pAttr->nItemId;
        aInsertedAttr.nAttr = pAttr->nAttr;

        nPos = pInsertColorLB->GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aInsertedAttr.nColor = COL_NONE_COLOR;
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
        pAttr = static_cast<CharAttr *>(pDeletedLB->GetEntryData(nPos));
        aDeletedAttr.nItemId = pAttr->nItemId;
        aDeletedAttr.nAttr = pAttr->nAttr;

        nPos = pDeletedColorLB->GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aDeletedAttr.nColor = COL_NONE_COLOR;
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
        pAttr = static_cast<CharAttr *>(pChangedLB->GetEntryData(nPos));
        aChangedAttr.nItemId = pAttr->nItemId;
        aChangedAttr.nAttr = pAttr->nAttr;

        nPos = pChangedColorLB->GetSelectEntryPos();

        switch (nPos)
        {
            case 0:
                aChangedAttr.nColor = COL_NONE_COLOR;
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
        SwDocShell* pDocShell = static_cast<SwDocShell*>(SfxObjectShell::GetFirst(checkSfxObjectShell<SwDocShell>));

        while( pDocShell )
        {
            pDocShell->GetWrtShell()->UpdateRedlineAttr();
            pDocShell = static_cast<SwDocShell*>(SfxObjectShell::GetNext(*pDocShell, checkSfxObjectShell<SwDocShell>));
        }
    }

    return false;
}

void SwRedlineOptionsTabPage::Reset( const SfxItemSet*  )
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
    for( long i = 0; i < pColorLst->Count(); ++i )
    {
        XColorEntry* pEntry = pColorLst->GetColor( i );
        Color aColor = pEntry->GetColor();
        const OUString sName = pEntry->GetName();

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
        case COL_NONE_COLOR:
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
        case COL_NONE_COLOR:
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
        case COL_NONE_COLOR:
            pChangedColorLB->SelectEntryPos(0);
            break;
        default:
            pChangedColorLB->SelectEntry(Color(nColor));
    }

    pMarkColorLB->SelectEntry(pOpt->GetMarkAlignColor());

    pInsertLB->SelectEntryPos(0);
    pDeletedLB->SelectEntryPos(0);
    pChangedLB->SelectEntryPos(0);

    lcl_FillRedlineAttrListBox(*pInsertLB, rInsertAttr, aInsertAttrMap, SAL_N_ELEMENTS(aInsertAttrMap));
    lcl_FillRedlineAttrListBox(*pDeletedLB, rDeletedAttr, aDeletedAttrMap, SAL_N_ELEMENTS(aDeletedAttrMap));
    lcl_FillRedlineAttrListBox(*pChangedLB, rChangedAttr, aChangedAttrMap, SAL_N_ELEMENTS(aChangedAttrMap));

    sal_Int32 nPos = 0;
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
    AttribHdl(*pInsertLB);
    ColorHdl(*pInsertColorLB);
    AttribHdl(*pDeletedLB);
    ColorHdl(*pInsertColorLB);
    AttribHdl(*pChangedLB);
    ColorHdl(*pChangedColorLB);

    ChangedMaskPrevHdl(*pMarkPosLB);
}

IMPL_LINK_TYPED( SwRedlineOptionsTabPage, AttribHdl, ListBox&, rLB, void )
{
    SvxFontPrevWindow *pPrev = nullptr;
    ColorListBox *pColorLB;

    if (&rLB == pInsertLB)
    {
        pColorLB = pInsertColorLB;
        pPrev = pInsertedPreviewWN;
    }
    else if (&rLB == pDeletedLB)
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
    rFont.SetUnderline(LINESTYLE_NONE);
    rCJKFont.SetUnderline(LINESTYLE_NONE);
    rFont.SetStrikeout(STRIKEOUT_NONE);
    rCJKFont.SetStrikeout(STRIKEOUT_NONE);
    rFont.SetCaseMap(SVX_CASEMAP_NOT_MAPPED);
    rCJKFont.SetCaseMap(SVX_CASEMAP_NOT_MAPPED);

    sal_Int32      nPos = pColorLB->GetSelectEntryPos();

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

    nPos = rLB.GetSelectEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;

    CharAttr*   pAttr = static_cast<CharAttr*>(rLB.GetEntryData( nPos ));
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
            rFont.SetUnderline( ( FontLineStyle ) pAttr->nAttr );
            rCJKFont.SetUnderline( ( FontLineStyle ) pAttr->nAttr );
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
}

IMPL_LINK_TYPED( SwRedlineOptionsTabPage, ColorHdl, ListBox&, rListBox, void )
{
    ColorListBox* pColorLB = static_cast<ColorListBox*>(&rListBox);
    SvxFontPrevWindow *pPrev = nullptr;
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
    sal_Int32      nPos = pLB->GetSelectEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;

    CharAttr*   pAttr = static_cast<CharAttr*>(pLB->GetEntryData( nPos ));

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
}

IMPL_LINK_NOARG_TYPED(SwRedlineOptionsTabPage, ChangedMaskPrevHdl, ListBox&, void)
{
    pMarkPreviewWN->SetMarkPos(pMarkPosLB->GetSelectEntryPos());
    pMarkPreviewWN->SetColor(pMarkColorLB->GetSelectEntryColor().GetColor());

    pMarkPreviewWN->Invalidate();
}

void SwRedlineOptionsTabPage::InitFontStyle(SvxFontPrevWindow& rExampleWin)
{
    const AllSettings&  rAllSettings = Application::GetSettings();
    LanguageType        eLangType = rAllSettings.GetUILanguageTag().getLanguageType();
    Color               aBackCol( rAllSettings.GetStyleSettings().GetWindowColor() );
    SvxFont&            rFont = rExampleWin.GetFont();
    SvxFont&            rCJKFont = rExampleWin.GetCJKFont();
    SvxFont&            rCTLFont = rExampleWin.GetCTLFont();

    vcl::Font           aFont( OutputDevice::GetDefaultFont( DefaultFontType::SERIF, eLangType,
                                                        GetDefaultFontFlags::OnlyOne, &rExampleWin ) );
    vcl::Font           aCJKFont( OutputDevice::GetDefaultFont( DefaultFontType::CJK_TEXT, eLangType,
                                                        GetDefaultFontFlags::OnlyOne, &rExampleWin ) );
    vcl::Font           aCTLFont( OutputDevice::GetDefaultFont( DefaultFontType::CTL_TEXT, eLangType,
                                                        GetDefaultFontFlags::OnlyOne, &rExampleWin ) );
    const Size          aDefSize( 0, 12 );
    aFont.SetFontSize( aDefSize );
    aCJKFont.SetFontSize( aDefSize );
    aCTLFont.SetFontSize( aDefSize );

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
    rFont.SetFontSize( aNewSize );
    rCJKFont.SetFontSize( aNewSize );

    rExampleWin.SetFont( rFont, rCJKFont,rCTLFont );

    rExampleWin.UseResourceText();

    Wallpaper           aWall( aBackCol );
    rExampleWin.SetBackground( aWall );
    rExampleWin.Invalidate();
}

SwCompareOptionsTabPage::SwCompareOptionsTabPage(  vcl::Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage( pParent,"OptComparison","modules/swriter/ui/optcomparison.ui", &rSet )
{
    get(m_pAutoRB,"auto");
    get(m_pWordRB, "byword");
    get(m_pCharRB, "bycharacter");

    get(m_pRsidCB, "useRSID");
    get(m_pIgnoreCB, "ignore");
    get(m_pLenNF, "ignorelen");
    get(m_pStoreRsidCB, "storeRSID");

    Link<Button*,void> aLnk( LINK( this, SwCompareOptionsTabPage, ComparisonHdl ) );
    m_pAutoRB->SetClickHdl( aLnk );
    m_pWordRB->SetClickHdl( aLnk );
    m_pCharRB->SetClickHdl( aLnk );

    m_pIgnoreCB->SetClickHdl( LINK( this, SwCompareOptionsTabPage, IgnoreHdl) );
}

SwCompareOptionsTabPage::~SwCompareOptionsTabPage()
{
    disposeOnce();
}

void SwCompareOptionsTabPage::dispose()
{
    m_pAutoRB.clear();
    m_pWordRB.clear();
    m_pCharRB.clear();
    m_pRsidCB.clear();
    m_pIgnoreCB.clear();
    m_pLenNF.clear();
    m_pStoreRsidCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwCompareOptionsTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SwCompareOptionsTabPage>::Create( pParent, *rAttrSet );
}

bool SwCompareOptionsTabPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    if( m_pAutoRB->IsValueChangedFromSaved() ||
        m_pWordRB->IsValueChangedFromSaved() ||
        m_pCharRB->IsValueChangedFromSaved() )
    {
        SvxCompareMode eCmpMode = SVX_CMP_AUTO;

        if ( m_pAutoRB->IsChecked() ) eCmpMode = SVX_CMP_AUTO;
        if ( m_pWordRB->IsChecked() ) eCmpMode = SVX_CMP_BY_WORD;
        if ( m_pCharRB->IsChecked() ) eCmpMode = SVX_CMP_BY_CHAR;

        pOpt->SetCompareMode( eCmpMode );
        bRet = true;
    }

    if( m_pRsidCB->IsValueChangedFromSaved() )
    {
        pOpt->SetUseRsid( m_pRsidCB->IsChecked() );
        bRet = true;
    }

    if( m_pIgnoreCB->IsValueChangedFromSaved() )
    {
        pOpt->SetIgnorePieces( m_pIgnoreCB->IsChecked() );
        bRet = true;
    }

    if( m_pLenNF->IsModified() )
    {
        pOpt->SetPieceLen( m_pLenNF->GetValue() );
        bRet = true;
    }

    if (m_pStoreRsidCB->IsValueChangedFromSaved())
    {
        pOpt->SetStoreRsid(m_pStoreRsidCB->IsChecked());
        bRet = true;
    }

    return bRet;
}

void SwCompareOptionsTabPage::Reset( const SfxItemSet* )
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

    m_pStoreRsidCB->Check(pOpt->IsStoreRsid());
    m_pStoreRsidCB->SaveValue();
}

IMPL_LINK_NOARG_TYPED(SwCompareOptionsTabPage, ComparisonHdl, Button*, void)
{
    bool bChecked = !m_pAutoRB->IsChecked();
    m_pRsidCB->Enable( bChecked );
    m_pIgnoreCB->Enable( bChecked );
    m_pLenNF->Enable( bChecked && m_pIgnoreCB->IsChecked() );
}

IMPL_LINK_NOARG_TYPED(SwCompareOptionsTabPage, IgnoreHdl, Button*, void)
{
    m_pLenNF->Enable( m_pIgnoreCB->IsChecked() );
}

#ifdef DBG_UTIL

SwTestTabPage::SwTestTabPage(vcl::Window* pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage( pParent, "OptTestPage", "modules/swriter/ui/opttestpage.ui", &rCoreSet)
    , bAttrModified( false )
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

SwTestTabPage::~SwTestTabPage()
{
    disposeOnce();
}

void SwTestTabPage::dispose()
{
    m_pTest1CBox.clear();
    m_pTest2CBox.clear();
    m_pTest3CBox.clear();
    m_pTest4CBox.clear();
    m_pTest5CBox.clear();
    m_pTest6CBox.clear();
    m_pTest7CBox.clear();
    m_pTest8CBox.clear();
    m_pTest9CBox.clear();
    m_pTest10CBox.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwTestTabPage::Create( vcl::Window* pParent,
                                          const SfxItemSet* rAttrSet )
{
    return VclPtr<SwTestTabPage>::Create(pParent, *rAttrSet);
}

bool    SwTestTabPage::FillItemSet( SfxItemSet* rCoreSet )
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
        rCoreSet->Put(aTestItem);
    }
    return bAttrModified;
}

void SwTestTabPage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();
    const SwTestItem* pTestAttr = nullptr;

    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_SWTEST , false,
                                    reinterpret_cast<const SfxPoolItem**>(&pTestAttr) ))
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
    Link<Button*,void> aLk = LINK( this, SwTestTabPage, AutoClickHdl );
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

IMPL_LINK_NOARG_TYPED(SwTestTabPage, AutoClickHdl, Button*, void)
{
    bAttrModified = true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
