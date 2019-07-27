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

#include <sal/config.h>

#include <utility>

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
#include <IDocumentRedlineAccess.hxx>

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
#include <strings.hrc>
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
#include <sfx2/viewfrm.hxx>
#include <svl/slstitm.hxx>
#include <svl/ctloptions.hxx>
#include <svl/eitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/unitconv.hxx>
#include <sfx2/htmlmode.hxx>
#include <svx/xtable.hxx>
#include <svx/dlgutil.hxx>
#include <svx/strarray.hxx>
#include <svx/svxids.hrc>

#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/svapp.hxx>

#include <optload.hxx>

using namespace ::com::sun::star;

namespace {

void drawRect(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect, const Color &rFillColor, const Color &rLineColor)
{
    rRenderContext.SetFillColor(rFillColor);
    rRenderContext.SetLineColor(rLineColor);
    rRenderContext.DrawRect(rRect);
}

}

// Tools->Options->Writer->View
// Tools->Options->Writer/Web->View
SwContentOptPage::SwContentOptPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "modules/swriter/ui/viewoptionspage.ui", "ViewOptionsPage", &rCoreSet)
    , m_xCrossCB(m_xBuilder->weld_check_button("helplines"))
    , m_xHMetric(m_xBuilder->weld_combo_box("hrulercombobox"))
    , m_xVRulerCBox(m_xBuilder->weld_check_button("vruler"))
    , m_xVRulerRightCBox(m_xBuilder->weld_check_button("vrulerright"))
    , m_xVMetric(m_xBuilder->weld_combo_box("vrulercombobox"))
    , m_xSmoothCBox(m_xBuilder->weld_check_button("smoothscroll"))
    , m_xGrfCB(m_xBuilder->weld_check_button("graphics"))
    , m_xTableCB(m_xBuilder->weld_check_button("tables"))
    , m_xDrwCB(m_xBuilder->weld_check_button("drawings"))
    , m_xFieldNameCB(m_xBuilder->weld_check_button("fieldcodes"))
    , m_xPostItCB(m_xBuilder->weld_check_button("comments"))
    , m_xSettingsFrame(m_xBuilder->weld_frame("settingsframe"))
    , m_xSettingsLabel(m_xBuilder->weld_label("settingslabel"))
    , m_xMetricLabel(m_xBuilder->weld_label("measureunitlabel"))
    , m_xMetricLB(m_xBuilder->weld_combo_box("measureunit"))
    , m_xShowInlineTooltips(m_xBuilder->weld_check_button("changestooltip"))
    , m_xFieldHiddenCB(m_xBuilder->weld_check_button("hiddentextfield"))
    , m_xFieldHiddenParaCB(m_xBuilder->weld_check_button("hiddenparafield"))
{
    /* This part is visible only with Writer/Web->View dialogue. */
    const SfxPoolItem* pItem;
    if (! (SfxItemState::SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem )
           && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON))
    {
        m_xSettingsFrame->hide();
        m_xSettingsLabel->hide();
        m_xMetricLabel->hide();
        m_xMetricLB->hide();
    }

    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsVerticalTextEnabled() )
        m_xVRulerRightCBox->hide();
    m_xVRulerCBox->connect_toggled(LINK(this, SwContentOptPage, VertRulerHdl ));

    for (size_t i = 0; i < SwFieldUnitTable::Count(); ++i)
    {
        const OUString sMetric = SwFieldUnitTable::GetString(i);
        FieldUnit eFUnit = SwFieldUnitTable::GetValue(i);

        switch ( eFUnit )
        {
            case FieldUnit::MM:
            case FieldUnit::CM:
            case FieldUnit::POINT:
            case FieldUnit::PICA:
            case FieldUnit::INCH:
            case FieldUnit::CHAR:    // add two units , 'character' and 'line' , their ticks are not fixed
            case FieldUnit::LINE:
            {
                // only use these metrics
                // a horizontal ruler has not the 'line' unit
                // there isn't 'line' unit in HTML format
                if (eFUnit != FieldUnit::LINE)
                {
                   m_xMetricLB->append(OUString::number(static_cast<sal_uInt32>(eFUnit)), sMetric);
                   m_xHMetric->append(OUString::number(static_cast<sal_uInt32>(eFUnit)), sMetric);
                }
                // a vertical ruler has not the 'character' unit
                if (eFUnit != FieldUnit::CHAR)
                {
                   m_xVMetric->append(OUString::number(static_cast<sal_uInt32>(eFUnit)), sMetric);
                }
                break;
            }
            default:;//prevent warning
        }
    }
}

SwContentOptPage::~SwContentOptPage()
{
}

VclPtr<SfxTabPage> SwContentOptPage::Create( TabPageParent pParent,
                                             const SfxItemSet* rAttrSet)
{
    return VclPtr<SwContentOptPage>::Create(pParent, *rAttrSet);
}

static void lcl_SelectMetricLB(weld::ComboBox& rMetric, sal_uInt16 nSID, const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if( rSet.GetItemState( nSID, false, &pItem ) >= SfxItemState::DEFAULT )
    {
        FieldUnit eFieldUnit = static_cast<FieldUnit>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
        for (sal_Int32 i = 0, nEntryCount = rMetric.get_count(); i < nEntryCount; ++i)
        {
            if (rMetric.get_id(i).toUInt32() == static_cast<sal_uInt32>(eFieldUnit))
            {
                rMetric.set_active(i);
                break;
            }
        }
    }
    rMetric.save_value();
}

void SwContentOptPage::Reset(const SfxItemSet* rSet)
{
    const SwElemItem* pElemAttr = nullptr;

    rSet->GetItemState( FN_PARAM_ELEM , false,
                                    reinterpret_cast<const SfxPoolItem**>(&pElemAttr) );
    if(pElemAttr)
    {
        m_xTableCB->set_active(pElemAttr->bTable);
        m_xGrfCB->set_active(pElemAttr->bGraphic);
        m_xDrwCB->set_active(pElemAttr->bDrawing);
        m_xFieldNameCB->set_active(pElemAttr->bFieldName);
        m_xPostItCB->set_active(pElemAttr->bNotes);
        m_xCrossCB->set_active(pElemAttr->bCrosshair);
        m_xVRulerCBox->set_active(pElemAttr->bVertRuler);
        m_xVRulerRightCBox->set_active(pElemAttr->bVertRulerRight);
        m_xSmoothCBox->set_active(pElemAttr->bSmoothScroll);
        m_xShowInlineTooltips->set_active(pElemAttr->bShowInlineTooltips);
        m_xFieldHiddenCB->set_active( pElemAttr->bFieldHiddenText );
        m_xFieldHiddenParaCB->set_active( pElemAttr->bShowHiddenPara );
    }
    m_xMetricLB->set_active(-1);
    lcl_SelectMetricLB(*m_xMetricLB, SID_ATTR_METRIC, *rSet);
    lcl_SelectMetricLB(*m_xHMetric, FN_HSCROLL_METRIC, *rSet);
    lcl_SelectMetricLB(*m_xVMetric, FN_VSCROLL_METRIC, *rSet);
}

bool SwContentOptPage::FillItemSet(SfxItemSet* rSet)
{
    const SwElemItem*   pOldAttr = static_cast<const SwElemItem*>(
                        GetOldItem(GetItemSet(), FN_PARAM_ELEM));

    SwElemItem aElem;
    aElem.bTable                = m_xTableCB->get_active();
    aElem.bGraphic              = m_xGrfCB->get_active();
    aElem.bDrawing              = m_xDrwCB->get_active();
    aElem.bFieldName            = m_xFieldNameCB->get_active();
    aElem.bNotes                = m_xPostItCB->get_active();
    aElem.bCrosshair            = m_xCrossCB->get_active();
    aElem.bVertRuler            = m_xVRulerCBox->get_active();
    aElem.bVertRulerRight       = m_xVRulerRightCBox->get_active();
    aElem.bSmoothScroll         = m_xSmoothCBox->get_active();
    aElem.bShowInlineTooltips   = m_xShowInlineTooltips->get_active();
    aElem.bFieldHiddenText      = m_xFieldHiddenCB->get_active();
    aElem.bShowHiddenPara       = m_xFieldHiddenParaCB->get_active();

    bool bRet = !pOldAttr || aElem != *pOldAttr;
    if(bRet)
        bRet = nullptr != rSet->Put(aElem);

    sal_Int32 nMPos = m_xMetricLB->get_active();
    sal_Int32 nGlobalMetricPos = nMPos;
    if ( m_xMetricLB->get_value_changed_from_saved() )
    {
        const sal_uInt16 nFieldUnit = m_xMetricLB->get_id(nMPos).toUInt32();
        rSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    nMPos = m_xHMetric->get_active();
    if ( m_xHMetric->get_value_changed_from_saved() || nMPos != nGlobalMetricPos )
    {
        const sal_uInt16 nFieldUnit = m_xHMetric->get_id(nMPos).toUInt32();
        rSet->Put( SfxUInt16Item( FN_HSCROLL_METRIC, nFieldUnit ) );
        bRet = true;
    }
    nMPos = m_xVMetric->get_active();
    if ( m_xVMetric->get_value_changed_from_saved() || nMPos != nGlobalMetricPos )
    {
        const sal_uInt16 nFieldUnit = m_xVMetric->get_id(nMPos).toUInt32();
        rSet->Put( SfxUInt16Item( FN_VSCROLL_METRIC, nFieldUnit ) );
        bRet = true;
    }

    return bRet;
}

IMPL_LINK(SwContentOptPage, VertRulerHdl, weld::ToggleButton&, rBox, void)
{
    m_xVRulerRightCBox->set_sensitive(rBox.get_sensitive() && rBox.get_active());
}

// TabPage Printer additional settings
SwAddPrinterTabPage::SwAddPrinterTabPage(TabPageParent pParent,
    const SfxItemSet& rCoreSet)
    : SfxTabPage(pParent, "modules/swriter/ui/printoptionspage.ui", "PrintOptionsPage", &rCoreSet)
    , sNone(SwResId(SW_STR_NONE))
    , bAttrModified(false)
    , bPreview(false)
    , m_xGrfCB(m_xBuilder->weld_check_button("graphics"))
    , m_xCtrlFieldCB(m_xBuilder->weld_check_button("formcontrols"))
    , m_xBackgroundCB(m_xBuilder->weld_check_button("background"))
    , m_xBlackFontCB(m_xBuilder->weld_check_button("inblack"))
    , m_xPrintHiddenTextCB(m_xBuilder->weld_check_button("hiddentext"))
    , m_xPrintTextPlaceholderCB(m_xBuilder->weld_check_button("textplaceholder"))
    , m_xPagesFrame(m_xBuilder->weld_widget("pagesframe"))
    , m_xLeftPageCB(m_xBuilder->weld_check_button("leftpages"))
    , m_xRightPageCB(m_xBuilder->weld_check_button("rightpages"))
    , m_xProspectCB(m_xBuilder->weld_check_button("brochure"))
    , m_xProspectCB_RTL(m_xBuilder->weld_check_button("rtl"))
    , m_xCommentsFrame(m_xBuilder->weld_widget("commentsframe"))
    , m_xNoRB(m_xBuilder->weld_radio_button("none"))
    , m_xOnlyRB(m_xBuilder->weld_radio_button("only"))
    , m_xEndRB(m_xBuilder->weld_radio_button("end"))
    , m_xEndPageRB(m_xBuilder->weld_radio_button("endpage"))
    , m_xInMarginsRB(m_xBuilder->weld_radio_button("inmargins"))
    , m_xPrintEmptyPagesCB(m_xBuilder->weld_check_button("blankpages"))
    , m_xPaperFromSetupCB(m_xBuilder->weld_check_button("papertray"))
    , m_xFaxLB(m_xBuilder->weld_combo_box("fax"))
{
    Link<weld::ToggleButton&,void> aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
    m_xGrfCB->connect_toggled( aLk );
    m_xRightPageCB->connect_toggled( aLk );
    m_xLeftPageCB->connect_toggled( aLk );
    m_xCtrlFieldCB->connect_toggled( aLk );
    m_xBackgroundCB->connect_toggled( aLk );
    m_xBlackFontCB->connect_toggled( aLk );
    m_xPrintHiddenTextCB->connect_toggled( aLk );
    m_xPrintTextPlaceholderCB->connect_toggled( aLk );
    m_xProspectCB->connect_toggled( aLk );
    m_xProspectCB_RTL->connect_toggled( aLk );
    m_xPaperFromSetupCB->connect_toggled( aLk );
    m_xPrintEmptyPagesCB->connect_toggled( aLk );
    m_xEndPageRB->connect_toggled( aLk );
    m_xInMarginsRB->connect_toggled( aLk );
    m_xEndRB->connect_toggled( aLk );
    m_xOnlyRB->connect_toggled( aLk );
    m_xNoRB->connect_toggled( aLk );
    m_xFaxLB->connect_changed( LINK( this, SwAddPrinterTabPage, SelectHdl ) );

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet.GetItemState(SID_HTML_MODE, false, &pItem )
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
    {
        m_xLeftPageCB->hide();
        m_xRightPageCB->hide();
        m_xPrintHiddenTextCB->hide();
        m_xPrintTextPlaceholderCB->hide();
        m_xPrintEmptyPagesCB->hide();
    }
    m_xProspectCB_RTL->set_sensitive(false);
    SvtCTLOptions aCTLOptions;
    m_xProspectCB_RTL->set_visible(aCTLOptions.IsCTLFontEnabled());
}

SwAddPrinterTabPage::~SwAddPrinterTabPage()
{
}

void SwAddPrinterTabPage::SetPreview(bool bPrev)
{
    bPreview = bPrev;
    m_xCommentsFrame->set_sensitive(!bPreview);
    m_xPagesFrame->set_sensitive(!bPreview);
}

VclPtr<SfxTabPage> SwAddPrinterTabPage::Create( TabPageParent pParent,
                                                const SfxItemSet* rAttrSet )
{
    return VclPtr<SwAddPrinterTabPage>::Create(pParent, *rAttrSet);
}

bool    SwAddPrinterTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( bAttrModified )
    {
        SwAddPrinterItem aAddPrinterAttr;
        aAddPrinterAttr.m_bPrintGraphic   = m_xGrfCB->get_active();
        aAddPrinterAttr.m_bPrintTable     = true; // always enabled since CWS printerpullgpages /*m_xTabCB->get_active();*/
        aAddPrinterAttr.m_bPrintDraw      = m_xGrfCB->get_active(); // UI merged with m_xGrfCB in CWS printerpullgpages
        aAddPrinterAttr.m_bPrintControl   = m_xCtrlFieldCB->get_active();
        aAddPrinterAttr.m_bPrintPageBackground = m_xBackgroundCB->get_active();
        aAddPrinterAttr.m_bPrintBlackFont = m_xBlackFontCB->get_active();
        aAddPrinterAttr.m_bPrintHiddenText = m_xPrintHiddenTextCB->get_active();
        aAddPrinterAttr.m_bPrintTextPlaceholder = m_xPrintTextPlaceholderCB->get_active();

        aAddPrinterAttr.m_bPrintLeftPages     = m_xLeftPageCB->get_active();
        aAddPrinterAttr.m_bPrintRightPages    = m_xRightPageCB->get_active();
        aAddPrinterAttr.m_bPrintReverse       = false; // handled by vcl itself since CWS printerpullpages /*m_xReverseCB->get_active()*/;
        aAddPrinterAttr.m_bPrintProspect      = m_xProspectCB->get_active();
        aAddPrinterAttr.m_bPrintProspectRTL   = m_xProspectCB_RTL->get_active();
        aAddPrinterAttr.m_bPaperFromSetup     = m_xPaperFromSetupCB->get_active();
        aAddPrinterAttr.m_bPrintEmptyPages    = m_xPrintEmptyPagesCB->get_active();
        aAddPrinterAttr.m_bPrintSingleJobs    = true; // handled by vcl in new print dialog since CWS printerpullpages /*m_xSingleJobsCB->get_active()*/;

        if (m_xNoRB->get_active())  aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::NONE;
        if (m_xOnlyRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::Only;
        if (m_xEndRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::EndDoc;
        if (m_xEndPageRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::EndPage;
        if (m_xInMarginsRB->get_active()) aAddPrinterAttr.m_nPrintPostIts =
                                                        SwPostItMode::InMargins;

        const OUString sFax = m_xFaxLB->get_active_text();
        aAddPrinterAttr.m_sFaxName = sNone == sFax ? OUString() : sFax;
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
        m_xGrfCB->set_active(pAddPrinterAttr->m_bPrintGraphic || pAddPrinterAttr->m_bPrintDraw);
        m_xCtrlFieldCB->set_active(       pAddPrinterAttr->m_bPrintControl);
        m_xBackgroundCB->set_active(    pAddPrinterAttr->m_bPrintPageBackground);
        m_xBlackFontCB->set_active(     pAddPrinterAttr->m_bPrintBlackFont);
        m_xPrintHiddenTextCB->set_active( pAddPrinterAttr->m_bPrintHiddenText);
        m_xPrintTextPlaceholderCB->set_active(pAddPrinterAttr->m_bPrintTextPlaceholder);
        m_xLeftPageCB->set_active(      pAddPrinterAttr->m_bPrintLeftPages);
        m_xRightPageCB->set_active(     pAddPrinterAttr->m_bPrintRightPages);
        m_xPaperFromSetupCB->set_active(pAddPrinterAttr->m_bPaperFromSetup);
        m_xPrintEmptyPagesCB->set_active(pAddPrinterAttr->m_bPrintEmptyPages);
        m_xProspectCB->set_active(      pAddPrinterAttr->m_bPrintProspect);
        m_xProspectCB_RTL->set_active(      pAddPrinterAttr->m_bPrintProspectRTL);

        m_xNoRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::NONE ) ;
        m_xOnlyRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::Only ) ;
        m_xEndRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndDoc ) ;
        m_xEndPageRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndPage ) ;
        m_xInMarginsRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::InMargins ) ;
        auto nFound = m_xFaxLB->find_text(pAddPrinterAttr->m_sFaxName);
        if (nFound != -1)
            m_xFaxLB->set_active(nFound);
        else
            m_xFaxLB->set_active(0);
    }
    if (m_xProspectCB->get_active())
    {
        m_xProspectCB_RTL->set_sensitive(true);
        m_xNoRB->set_sensitive( false );
        m_xOnlyRB->set_sensitive( false );
        m_xEndRB->set_sensitive( false );
        m_xEndPageRB->set_sensitive( false );
    }
    else
        m_xProspectCB_RTL->set_sensitive( false );
}

IMPL_LINK_NOARG(SwAddPrinterTabPage, AutoClickHdl, weld::ToggleButton&, void)
{
    bAttrModified = true;
    bool bIsProspect = m_xProspectCB->get_active();
    if (!bIsProspect)
        m_xProspectCB_RTL->set_active( false );
    m_xProspectCB_RTL->set_sensitive( bIsProspect );
    m_xNoRB->set_sensitive( !bIsProspect );
    m_xOnlyRB->set_sensitive( !bIsProspect );
    m_xEndRB->set_sensitive( !bIsProspect );
    m_xEndPageRB->set_sensitive( !bIsProspect );
    m_xInMarginsRB->set_sensitive( !bIsProspect );
}

void  SwAddPrinterTabPage::SetFax( const std::vector<OUString>& rFaxLst )
{
    m_xFaxLB->append_text(sNone);
    for(const auto & i : rFaxLst)
    {
        m_xFaxLB->append_text(i);
    }
    m_xFaxLB->set_active(0);
}

IMPL_LINK_NOARG(SwAddPrinterTabPage, SelectHdl, weld::ComboBox&, void)
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
    m_bLabelHeightDefault   (false),
    m_bIndexHeightDefault     (false),

    m_nFontGroup(FONT_GROUP_DEFAULT),

    m_sScriptWestern(SwResId(ST_SCRIPT_WESTERN)),
    m_sScriptAsian(SwResId(ST_SCRIPT_ASIAN)),
    m_sScriptComplex(SwResId(ST_SCRIPT_CTL))
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
}

SwStdFontTabPage::~SwStdFontTabPage()
{
    disposeOnce();
}

void SwStdFontTabPage::dispose()
{
    m_pFontList.reset();
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

VclPtr<SfxTabPage> SwStdFontTabPage::Create( TabPageParent pParent,
                                             const SfxItemSet* rAttrSet )
{
    return VclPtr<SwStdFontTabPage>::Create(pParent.pParent, *rAttrSet);
}

static void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    SfxPrinter const * pPrt, const OUString& rStyle,
                    sal_uInt16 nFontWhich)
{
    vcl::Font aFont( rStyle, Size( 0, 10 ) );
    if( pPrt )
        aFont = pPrt->GetFontMetric( aFont );
    SwTextFormatColl *pColl = pWrtShell->GetTextCollFromPool(nType);
    pColl->SetFormatAttr(SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
                OUString(), aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
}

static void lcl_SetColl(SwWrtShell* pWrtShell, sal_uInt16 nType,
                    sal_Int32 nHeight, sal_uInt16 nFontHeightWhich)
{
    float fSize = static_cast<float>(nHeight) / 10;
    nHeight = CalcToUnit( fSize, MapUnit::MapTwip );
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
    bool bListHeightChanged = m_pListHeightLB->IsValueChangedFromSaved() && !m_bListHeightDefault;
    bool bLabelHeightChanged = m_pLabelHeightLB->IsValueChangedFromSaved() && !m_bLabelHeightDefault;
    bool bIndexHeightChanged = m_pIndexHeightLB->IsValueChangedFromSaved() && !m_bIndexHeightDefault;

    m_pFontConfig->SetFontStandard(sStandard, m_nFontGroup);
    m_pFontConfig->SetFontOutline(sTitle, m_nFontGroup);
    m_pFontConfig->SetFontList(sList, m_nFontGroup);
    m_pFontConfig->SetFontCaption(sLabel, m_nFontGroup);
    m_pFontConfig->SetFontIndex(sIdx, m_nFontGroup);
    if(bStandardHeightChanged)
    {
        float fSize = static_cast<float>(m_pStandardHeightLB->GetValue()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_STANDARD, m_nFontGroup );
    }
    if(bTitleHeightChanged)
    {
        float fSize = static_cast<float>(m_pTitleHeightLB->GetValue()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_OUTLINE, m_nFontGroup );
    }
    if(bListHeightChanged)
    {
        float fSize = static_cast<float>(m_pListHeightLB->GetValue()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_LIST, m_nFontGroup );
    }
    if(bLabelHeightChanged)
    {
        float fSize = static_cast<float>(m_pLabelHeightLB->GetValue()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_CAPTION, m_nFontGroup );
    }
    if(bIndexHeightChanged)
    {
        float fSize = static_cast<float>(m_pIndexHeightLB->GetValue()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_INDEX, m_nFontGroup );
    }

    if(m_pWrtShell)
    {
        m_pWrtShell->StartAllAction();
        SfxPrinter* pPrinter = m_pWrtShell->getIDocumentDeviceAccess().getPrinter( false );
        bool bMod = false;
        const sal_uInt16 nFontWhich =
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT;
        const sal_uInt16 nFontHeightWhich =
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE;
        if(sStandard != m_sShellStd)
        {
            vcl::Font aFont( sStandard, Size( 0, 10 ) );
            if( pPrinter )
                aFont = pPrinter->GetFontMetric( aFont );
            m_pWrtShell->SetDefault(SvxFontItem(aFont.GetFamilyType(), aFont.GetFamilyName(),
                                  OUString(), aFont.GetPitch(), aFont.GetCharSet(), nFontWhich));
            SwTextFormatColl *pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_STANDARD);
            pColl->ResetFormatAttr(nFontWhich);
            bMod = true;
        }
        if(bStandardHeightChanged)
        {
            float fSize = static_cast<float>(m_pStandardHeightLB->GetValue()) / 10;
            m_pWrtShell->SetDefault(SvxFontHeightItem( CalcToUnit( fSize, MapUnit::MapTwip ), 100, nFontHeightWhich ) );
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
        auto pPrinterSet = std::make_unique<SfxItemSet>( *rSet->GetPool(),
                    svl::Items<SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC>{} );
        m_pPrt = VclPtr<SfxPrinter>::Create(std::move(pPrinterSet));
    }
    m_pFontList.reset(new FontList( m_pPrt ));
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
        for( const auto& rFontName : aFontNames )
        {
            m_pStandardBox->InsertEntry( rFontName );
            m_pTitleBox->InsertEntry( rFontName );
            m_pListBox->InsertEntry( rFontName );
            m_pLabelBox->InsertEntry( rFontName );
            m_pIdxBox->InsertEntry( rFontName );
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

        const sal_uInt16 nFontHeightWhich =
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE;
        const SvxFontHeightItem& rFontHeightStandard = static_cast<const SvxFontHeightItem& >(pColl->GetFormatAttr(nFontHeightWhich));
        nStandardHeight = static_cast<sal_Int32>(rFontHeightStandard.GetHeight());

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_HEADLINE_BASE);
        const SvxFontItem& rFontHL = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellTitle = sOutBackup = rFontHL.GetFamilyName();

        const SvxFontHeightItem& rFontHeightTitle = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr( nFontHeightWhich ));
        nTitleHeight = static_cast<sal_Int32>(rFontHeightTitle.GetHeight());

        const sal_uInt16 nFontWhich =
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT;
        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_NUMBUL_BASE);
        const SvxFontItem& rFontLS = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_bListDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        m_sShellList = sListBackup = rFontLS.GetFamilyName();

        const SvxFontHeightItem& rFontHeightList = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr(nFontHeightWhich));
        nListHeight = static_cast<sal_Int32>(rFontHeightList.GetHeight());
        m_bListHeightDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_LABEL);
        m_bLabelDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontCP = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellLabel = sCapBackup = rFontCP.GetFamilyName();
        const SvxFontHeightItem& rFontHeightLabel = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr(nFontHeightWhich));
        nLabelHeight = static_cast<sal_Int32>(rFontHeightLabel.GetHeight());
        m_bLabelHeightDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_REGISTER_BASE);
        m_bIdxDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontIDX = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellIndex = sIdxBackup = rFontIDX.GetFamilyName();
        const SvxFontHeightItem& rFontHeightIndex = static_cast<const SvxFontHeightItem&>(pColl->GetFormatAttr(nFontHeightWhich));
        nIndexHeight = static_cast<sal_Int32>(rFontHeightIndex.GetHeight());
        m_bIndexHeightDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
    }
    m_pStandardBox->SetText(sStdBackup );
    m_pTitleBox->SetText(sOutBackup );
    m_pListBox->SetText(sListBackup);
    m_pLabelBox->SetText(sCapBackup );
    m_pIdxBox->SetText(sIdxBackup );

    FontMetric aFontMetric( m_pFontList->Get(sStdBackup, sStdBackup) );
    m_pStandardHeightLB->Fill( &aFontMetric, m_pFontList.get() );
    aFontMetric = m_pFontList->Get(sOutBackup, sOutBackup );
    m_pTitleHeightLB->Fill( &aFontMetric, m_pFontList.get() );
    aFontMetric = m_pFontList->Get(sListBackup,sListBackup);
    m_pListHeightLB->Fill( &aFontMetric, m_pFontList.get() );
    aFontMetric = m_pFontList->Get(sCapBackup, sCapBackup );
    m_pLabelHeightLB->Fill( &aFontMetric, m_pFontList.get() );
    aFontMetric = m_pFontList->Get(sIdxBackup, sIdxBackup );
    m_pIndexHeightLB->Fill( &aFontMetric, m_pFontList.get() );

    m_pStandardHeightLB->SetValue( CalcToPoint( nStandardHeight, MapUnit::MapTwip, 10 ) );
    m_pTitleHeightLB->   SetValue( CalcToPoint( nTitleHeight   , MapUnit::MapTwip, 10 ) );
    m_pListHeightLB->    SetValue( CalcToPoint( nListHeight    , MapUnit::MapTwip, 10 ) );
    m_pLabelHeightLB->   SetValue( CalcToPoint( nLabelHeight   , MapUnit::MapTwip, 10 ));
    m_pIndexHeightLB->   SetValue( CalcToPoint( nIndexHeight   , MapUnit::MapTwip, 10 ));

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

IMPL_LINK_NOARG(SwStdFontTabPage, StandardHdl, Button*, void)
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
            MapUnit::MapTwip, 10 ) );
    m_pTitleHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_OUTLINE  +
            nFontOffset, m_eLanguage), MapUnit::MapTwip, 10 ));
    m_pListHeightLB    ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_LIST + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
    m_pLabelHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_CAPTION  + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
    m_pIndexHeightLB   ->SetValue(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_INDEX    + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
}

IMPL_LINK( SwStdFontTabPage, ModifyHdl, Edit&, rBox, void )
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

IMPL_LINK( SwStdFontTabPage, LoseFocusHdl, Control&, rControl, void )
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
    pHeightLB->Fill( &aFontMetric, m_pFontList.get() );
}

void SwStdFontTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pFlagItem = aSet.GetItem<SfxUInt16Item>(SID_FONTMODE_TYPE, false);
    if (pFlagItem)
        m_nFontGroup = sal::static_int_cast< sal_uInt8, sal_uInt16>( pFlagItem->GetValue() );
}

SwTableOptionsTabPage::SwTableOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/opttablepage.ui", "OptTablePage", &rSet)
    , m_pWrtShell(nullptr)
    , m_bHTMLMode(false)
    , m_xHeaderCB(m_xBuilder->weld_check_button("header"))
    , m_xRepeatHeaderCB(m_xBuilder->weld_check_button("repeatheader"))
    , m_xDontSplitCB(m_xBuilder->weld_check_button("dontsplit"))
    , m_xBorderCB(m_xBuilder->weld_check_button("border"))
    , m_xNumFormattingCB(m_xBuilder->weld_check_button("numformatting"))
    , m_xNumFormatFormattingCB(m_xBuilder->weld_check_button("numfmtformatting"))
    , m_xNumAlignmentCB(m_xBuilder->weld_check_button("numalignment"))
    , m_xRowMoveMF(m_xBuilder->weld_metric_spin_button("rowmove", FieldUnit::CM))
    , m_xColMoveMF(m_xBuilder->weld_metric_spin_button("colmove", FieldUnit::CM))
    , m_xRowInsertMF(m_xBuilder->weld_metric_spin_button("rowinsert", FieldUnit::CM))
    , m_xColInsertMF(m_xBuilder->weld_metric_spin_button("colinsert", FieldUnit::CM))
    , m_xFixRB(m_xBuilder->weld_radio_button("fix"))
    , m_xFixPropRB(m_xBuilder->weld_radio_button("fixprop"))
    , m_xVarRB(m_xBuilder->weld_radio_button("var"))
{
    Link<weld::Button&,void> aLnk(LINK(this, SwTableOptionsTabPage, CheckBoxHdl));
    m_xNumFormattingCB->connect_clicked(aLnk);
    m_xNumFormatFormattingCB->connect_clicked(aLnk);
    m_xHeaderCB->connect_clicked(aLnk);
}

SwTableOptionsTabPage::~SwTableOptionsTabPage()
{
}

VclPtr<SfxTabPage> SwTableOptionsTabPage::Create( TabPageParent pParent,
                                                  const SfxItemSet* rAttrSet )
{
    return VclPtr<SwTableOptionsTabPage>::Create(pParent, *rAttrSet);
}

bool SwTableOptionsTabPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    if (m_xRowMoveMF->get_value_changed_from_saved())
        pModOpt->SetTableHMove( static_cast<sal_uInt16>(m_xRowMoveMF->denormalize( m_xRowMoveMF->get_value(FieldUnit::TWIP))));

    if (m_xColMoveMF->get_value_changed_from_saved())
        pModOpt->SetTableVMove( static_cast<sal_uInt16>(m_xColMoveMF->denormalize( m_xColMoveMF->get_value(FieldUnit::TWIP))));

    if (m_xRowInsertMF->get_value_changed_from_saved())
        pModOpt->SetTableHInsert(static_cast<sal_uInt16>(m_xRowInsertMF->denormalize( m_xRowInsertMF->get_value(FieldUnit::TWIP))));

    if (m_xColInsertMF->get_value_changed_from_saved())
        pModOpt->SetTableVInsert(static_cast<sal_uInt16>(m_xColInsertMF->denormalize( m_xColInsertMF->get_value(FieldUnit::TWIP))));

    TableChgMode eMode;
    if (m_xFixRB->get_active())
        eMode = TableChgMode::FixedWidthChangeAbs;
    else if(m_xFixPropRB->get_active())
        eMode = TableChgMode::FixedWidthChangeProp;
    else
        eMode = TableChgMode::VarWidthChangeAbs;
    if(eMode != pModOpt->GetTableMode())
    {
        pModOpt->SetTableMode(eMode);
        // the table-keyboard-mode has changed, now the current
        // table should know about that too.
        if(m_pWrtShell && SelectionType::Table & m_pWrtShell->GetSelectionType())
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

    SwInsertTableOptions aInsOpts( SwInsertTableFlags::NONE, 0 );

    if (m_xHeaderCB->get_active())
        aInsOpts.mnInsMode |= SwInsertTableFlags::Headline;

    if (m_xRepeatHeaderCB->get_sensitive())
        aInsOpts.mnRowsToRepeat = m_xRepeatHeaderCB->get_active() ? 1 : 0;

    if (!m_xDontSplitCB->get_active())
        aInsOpts.mnInsMode |= SwInsertTableFlags::SplitLayout;

    if (m_xBorderCB->get_active())
        aInsOpts.mnInsMode |= SwInsertTableFlags::DefaultBorder;

    if (m_xHeaderCB->get_state_changed_from_saved() ||
        m_xRepeatHeaderCB->get_state_changed_from_saved() ||
        m_xDontSplitCB->get_state_changed_from_saved() ||
        m_xBorderCB->get_state_changed_from_saved())
    {
        pModOpt->SetInsTableFlags(m_bHTMLMode, aInsOpts);
    }

    if (m_xNumFormattingCB->get_state_changed_from_saved())
    {
        pModOpt->SetInsTableFormatNum(m_bHTMLMode, m_xNumFormattingCB->get_active());
        bRet = true;
    }

    if (m_xNumFormatFormattingCB->get_state_changed_from_saved())
    {
        pModOpt->SetInsTableChangeNumFormat(m_bHTMLMode, m_xNumFormatFormattingCB->get_active());
        bRet = true;
    }

    if (m_xNumAlignmentCB->get_state_changed_from_saved())
    {
        pModOpt->SetInsTableAlignNum(m_bHTMLMode, m_xNumAlignmentCB->get_active());
        bRet = true;
    }

    return bRet;
}

void SwTableOptionsTabPage::Reset( const SfxItemSet* rSet)
{
    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    if ( rSet->GetItemState( SID_ATTR_METRIC ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = rSet->Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = static_cast<FieldUnit>(rItem.GetValue());
        ::SetFieldUnit( *m_xRowMoveMF, eFieldUnit );
        ::SetFieldUnit( *m_xColMoveMF, eFieldUnit );
        ::SetFieldUnit( *m_xRowInsertMF, eFieldUnit );
        ::SetFieldUnit( *m_xColInsertMF, eFieldUnit );
    }

    m_xRowMoveMF->set_value(m_xRowMoveMF->normalize(pModOpt->GetTableHMove()), FieldUnit::TWIP);
    m_xColMoveMF->set_value(m_xColMoveMF->normalize(pModOpt->GetTableVMove()), FieldUnit::TWIP);
    m_xRowInsertMF->set_value(m_xRowInsertMF->normalize(pModOpt->GetTableHInsert()), FieldUnit::TWIP);
    m_xColInsertMF->set_value(m_xColInsertMF->normalize(pModOpt->GetTableVInsert()), FieldUnit::TWIP);

    switch(pModOpt->GetTableMode())
    {
        case TableChgMode::FixedWidthChangeAbs:   m_xFixRB->set_active(true);     break;
        case TableChgMode::FixedWidthChangeProp:  m_xFixPropRB->set_active(true); break;
        case TableChgMode::VarWidthChangeAbs:     m_xVarRB->set_active(true); break;
    }
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet->GetItemState(SID_HTML_MODE, false, &pItem))
    {
        m_bHTMLMode = 0 != (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);
    }

    // hide certain controls for html
    if (m_bHTMLMode)
    {
        m_xRepeatHeaderCB->hide();
        m_xDontSplitCB->hide();
    }

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(m_bHTMLMode);
    const SwInsertTableFlags nInsTableFlags = aInsOpts.mnInsMode;

    m_xHeaderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::Headline));
    m_xRepeatHeaderCB->set_active((!m_bHTMLMode) && (aInsOpts.mnRowsToRepeat > 0));
    m_xDontSplitCB->set_active(!(nInsTableFlags & SwInsertTableFlags::SplitLayout));
    m_xBorderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::DefaultBorder));

    m_xNumFormattingCB->set_active(pModOpt->IsInsTableFormatNum(m_bHTMLMode));
    m_xNumFormatFormattingCB->set_active(pModOpt->IsInsTableChangeNumFormat(m_bHTMLMode));
    m_xNumAlignmentCB->set_active(pModOpt->IsInsTableAlignNum(m_bHTMLMode));

    m_xHeaderCB->save_state();
    m_xRepeatHeaderCB->save_state();
    m_xDontSplitCB->save_state();
    m_xBorderCB->save_state();
    m_xNumFormattingCB->save_state();
    m_xNumFormatFormattingCB->save_state();
    m_xNumAlignmentCB->save_state();
    m_xRowMoveMF->save_value();
    m_xColMoveMF->save_value();
    m_xRowInsertMF->save_value();
    m_xColInsertMF->save_value();

    CheckBoxHdl(*m_xHeaderCB);
}

IMPL_LINK_NOARG(SwTableOptionsTabPage, CheckBoxHdl, weld::Button&, void)
{
    m_xNumFormatFormattingCB->set_sensitive(m_xNumFormattingCB->get_active());
    m_xNumAlignmentCB->set_sensitive(m_xNumFormattingCB->get_active());
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active());
}

void SwTableOptionsTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SwWrtShellItem* pWrtSh = aSet.GetItem<SwWrtShellItem>(SID_WRT_SHELL, false);
    if (pWrtSh)
        m_pWrtShell = pWrtSh->GetValue();
}

SwShdwCursorOptionsTabPage::SwShdwCursorOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/optformataidspage.ui", "OptFormatAidsPage", &rSet)
    , m_pWrtShell(nullptr)
    , m_xParaCB(m_xBuilder->weld_check_button("paragraph"))
    , m_xSHyphCB(m_xBuilder->weld_check_button("hyphens"))
    , m_xSpacesCB(m_xBuilder->weld_check_button("spaces"))
    , m_xHSpacesCB(m_xBuilder->weld_check_button("nonbreak"))
    , m_xTabCB(m_xBuilder->weld_check_button("tabs"))
    , m_xBreakCB(m_xBuilder->weld_check_button("break"))
    , m_xCharHiddenCB(m_xBuilder->weld_check_button("hiddentext"))
    , m_xDirectCursorFrame(m_xBuilder->weld_frame("directcrsrframe"))
    , m_xOnOffCB(m_xBuilder->weld_check_button("cursoronoff"))
    , m_xFillMarginRB(m_xBuilder->weld_radio_button("fillmargin"))
    , m_xFillIndentRB(m_xBuilder->weld_radio_button("fillindent"))
    , m_xFillTabRB(m_xBuilder->weld_radio_button("filltab"))
    , m_xFillTabAndSpaceRB(m_xBuilder->weld_radio_button("filltabandspace"))
    , m_xFillSpaceRB(m_xBuilder->weld_radio_button("fillspace"))
    , m_xCursorProtFrame(m_xBuilder->weld_frame("crsrprotframe"))
    , m_xCursorInProtCB(m_xBuilder->weld_check_button("cursorinprot"))
    , m_xMathBaselineAlignmentCB(m_xBuilder->weld_check_button("mathbaseline"))
{
    const SfxPoolItem* pItem = nullptr;
    sal_uInt8 eMode = SwFillMode::FILL_TAB;
    bool bIsOn = false;

    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem ))
    {
        auto& aOpt = *static_cast<const SwShadowCursorItem*>(pItem);
        eMode = aOpt.GetMode();
        bIsOn = aOpt.IsOn();
    }
    m_xOnOffCB->set_active( bIsOn );

    m_xFillIndentRB->set_active( FILL_INDENT == eMode );
    m_xFillMarginRB->set_active( FILL_MARGIN == eMode );
    m_xFillTabRB->set_active( FILL_TAB == eMode );
    m_xFillSpaceRB->set_active( FILL_SPACE == eMode );
    m_xFillTabAndSpaceRB->set_active( FILL_TAB_SPACE == eMode );

    if(SfxItemState::SET != rSet.GetItemState(SID_HTML_MODE, false, &pItem )
        || !(static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON))
        return;

    m_xTabCB->hide();
    m_xCharHiddenCB->hide();

    m_xDirectCursorFrame->hide();
    m_xOnOffCB->hide();
    m_xFillMarginRB->hide();
    m_xFillIndentRB->hide();
    m_xFillTabRB->hide();
    m_xFillSpaceRB->hide();
    m_xFillTabAndSpaceRB->hide();

    m_xCursorProtFrame->hide();
    m_xCursorInProtCB->hide();
}

SwShdwCursorOptionsTabPage::~SwShdwCursorOptionsTabPage()
{
}

VclPtr<SfxTabPage> SwShdwCursorOptionsTabPage::Create( TabPageParent pParent, const SfxItemSet* rSet )
{
    return VclPtr<SwShdwCursorOptionsTabPage>::Create( pParent, *rSet );
}

void SwShdwCursorOptionsTabPage::PageCreated( const SfxAllItemSet& aSet )
{
    const SwWrtShellItem* pWrtSh = aSet.GetItem<SwWrtShellItem>(SID_WRT_SHELL, false);
    if (pWrtSh)
        m_pWrtShell = pWrtSh->GetValue();
}

bool SwShdwCursorOptionsTabPage::FillItemSet( SfxItemSet* rSet )
{
    SwShadowCursorItem aOpt;
    aOpt.SetOn( m_xOnOffCB->get_active() );

    sal_uInt8 eMode;
    if( m_xFillIndentRB->get_active() )
        eMode= FILL_INDENT;
    else if( m_xFillMarginRB->get_active() )
        eMode = FILL_MARGIN;
    else if( m_xFillTabRB->get_active() )
        eMode = FILL_TAB;
    else if ( m_xFillTabAndSpaceRB->get_active() )
        eMode = FILL_TAB_SPACE;
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
                                    m_xMathBaselineAlignmentCB->get_active() );
        bRet |= m_xMathBaselineAlignmentCB->get_state_changed_from_saved();
    }

    if( m_xCursorInProtCB->get_state_changed_from_saved())
    {
        rSet->Put(SfxBoolItem(FN_PARAM_CRSR_IN_PROTECTED, m_xCursorInProtCB->get_active()));
        bRet = true;
    }

    const SwDocDisplayItem* pOldAttr = static_cast<const SwDocDisplayItem*>(
                        GetOldItem(GetItemSet(), FN_PARAM_DOCDISP));

    SwDocDisplayItem aDisp;

    aDisp.bParagraphEnd         = m_xParaCB->get_active();
    aDisp.bTab                  = m_xTabCB->get_active();
    aDisp.bSpace                = m_xSpacesCB->get_active();
    aDisp.bNonbreakingSpace     = m_xHSpacesCB->get_active();
    aDisp.bSoftHyphen           = m_xSHyphCB->get_active();
    aDisp.bCharHiddenText       = m_xCharHiddenCB->get_active();
    aDisp.bManualBreak          = m_xBreakCB->get_active();

    bRet |= (!pOldAttr || aDisp != *pOldAttr);
    if(bRet)
        bRet = nullptr != rSet->Put(aDisp);

    return bRet;
}

void SwShdwCursorOptionsTabPage::Reset( const SfxItemSet* rSet )
{
    const SfxPoolItem* pItem = nullptr;
    sal_uInt8 eMode = SwFillMode::FILL_TAB;
    bool bIsOn = false;

    if( SfxItemState::SET == rSet->GetItemState( FN_PARAM_SHADOWCURSOR, false, &pItem ))
    {
        auto& aOpt = *static_cast<const SwShadowCursorItem*>(pItem);
        eMode = aOpt.GetMode();
        bIsOn = aOpt.IsOn();
    }
    m_xOnOffCB->set_active( bIsOn );

    m_xFillIndentRB->set_active( FILL_INDENT == eMode );
    m_xFillMarginRB->set_active( FILL_MARGIN == eMode );
    m_xFillTabRB->set_active( FILL_TAB == eMode );
    m_xFillSpaceRB->set_active( FILL_SPACE == eMode );
    m_xFillTabAndSpaceRB->set_active( FILL_TAB_SPACE == eMode );

    if (m_pWrtShell) {
        m_xMathBaselineAlignmentCB->set_active( m_pWrtShell->GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT ) );
        m_xMathBaselineAlignmentCB->save_state();
    } else {
        m_xMathBaselineAlignmentCB->hide();
    }

    if( SfxItemState::SET == rSet->GetItemState( FN_PARAM_CRSR_IN_PROTECTED, false, &pItem ))
        m_xCursorInProtCB->set_active(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    m_xCursorInProtCB->save_state();

    const SwDocDisplayItem* pDocDisplayAttr = nullptr;

    rSet->GetItemState( FN_PARAM_DOCDISP, false,
                                    reinterpret_cast<const SfxPoolItem**>(&pDocDisplayAttr) );
    if(pDocDisplayAttr)
    {
        m_xParaCB->set_active( pDocDisplayAttr->bParagraphEnd );
        m_xTabCB->set_active( pDocDisplayAttr->bTab );
        m_xSpacesCB->set_active( pDocDisplayAttr->bSpace );
        m_xHSpacesCB->set_active( pDocDisplayAttr->bNonbreakingSpace );
        m_xSHyphCB->set_active( pDocDisplayAttr->bSoftHyphen );
        m_xCharHiddenCB->set_active( pDocDisplayAttr->bCharHiddenText );
        m_xBreakCB->set_active( pDocDisplayAttr->bManualBreak );
    }
}

// TabPage for Redlining
struct CharAttr
{
    sal_uInt16 const nItemId;
    sal_uInt16 const nAttr;
};

// Edit corresponds to Paste-attributes
static CharAttr const aRedlineAttr[] =
{
    { SID_ATTR_CHAR_CASEMAP,        sal_uInt16(SvxCaseMap::NotMapped) },
    { SID_ATTR_CHAR_WEIGHT,         WEIGHT_BOLD },
    { SID_ATTR_CHAR_POSTURE,        ITALIC_NORMAL },
    { SID_ATTR_CHAR_UNDERLINE,      LINESTYLE_SINGLE },
    { SID_ATTR_CHAR_UNDERLINE,      LINESTYLE_DOUBLE },
    { SID_ATTR_CHAR_STRIKEOUT,      STRIKEOUT_SINGLE },
    { SID_ATTR_CHAR_CASEMAP,        sal_uInt16(SvxCaseMap::Uppercase) },
    { SID_ATTR_CHAR_CASEMAP,        sal_uInt16(SvxCaseMap::Lowercase) },
    { SID_ATTR_CHAR_CASEMAP,        sal_uInt16(SvxCaseMap::SmallCaps) },
    { SID_ATTR_CHAR_CASEMAP,        sal_uInt16(SvxCaseMap::Capitalize) },
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
    SetMapMode(MapMode(MapUnit::MapPixel));
}

VCL_BUILDER_FACTORY_ARGS(SwMarkPreview, 0)

SwMarkPreview::~SwMarkPreview()
{
}

void SwMarkPreview::InitColors()
{
    // m_aTransCol and m_aMarkCol are _not_ changed because they are set from outside!

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    m_aBgCol = rSettings.GetWindowColor();

    bool bHC = rSettings.GetHighContrastMode();
    m_aLineCol = bHC? SwViewOption::GetFontColor() : COL_BLACK;
    m_aShadowCol = bHC? m_aBgCol : rSettings.GetShadowColor();
    m_aTextCol = bHC? SwViewOption::GetFontColor() : COL_GRAY;
    m_aPrintAreaCol = m_aTextCol;
}

void SwMarkPreview::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if( rDCEvt.GetType() == DataChangedEventType::SETTINGS && ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ) )
        InitColors();
}

void SwMarkPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &/*rRect*/)
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

    aLeftPagePrtArea = tools::Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));
    const long nWidth = aLeftPagePrtArea.GetWidth();
    const long nCorr = (nWidth & 1) != 0 ? 0 : 1;
    aLeftPagePrtArea.SetSize(Size(nWidth / 2 - (nLBorder + nRBorder) / 2 + nCorr, aLeftPagePrtArea.GetHeight()));

    aRightPagePrtArea = aLeftPagePrtArea;
    aRightPagePrtArea.Move(aLeftPagePrtArea.GetWidth() + nLBorder + nRBorder + 1, 0);

    // draw shadow
    tools::Rectangle aShadow(aPage);
    aShadow += Point(3, 3);
    drawRect(rRenderContext, aShadow, m_aShadowCol, m_aTransCol);

    // draw page
    drawRect(rRenderContext, aPage, m_aBgCol, m_aLineCol);

    // draw separator
    tools::Rectangle aPageSeparator(aPage);
    aPageSeparator.SetSize(Size(2, aPageSeparator.GetHeight()));
    aPageSeparator.Move(aPage.GetWidth() / 2 - 1, 0);
    drawRect(rRenderContext, aPageSeparator, m_aLineCol, m_aTransCol);

    PaintPage(rRenderContext, aLeftPagePrtArea);
    PaintPage(rRenderContext, aRightPagePrtArea);

    tools::Rectangle aLeftMark(Point(aPage.Left() + 2, aLeftPagePrtArea.Top() + 4), Size(aLeftPagePrtArea.Left() - 4, 2));
    tools::Rectangle aRightMark(Point(aRightPagePrtArea.Right() + 2, aRightPagePrtArea.Bottom() - 6), Size(aLeftPagePrtArea.Left() - 4, 2));

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

void SwMarkPreview::PaintPage(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect)
{
    // draw PrintArea
    drawRect(rRenderContext, rRect, m_aTransCol, m_aPrintAreaCol);

    // draw Testparagraph

    tools::Rectangle aTextLine = rRect;
    aTextLine.SetSize(Size(aTextLine.GetWidth(), 2));
    aTextLine.AdjustLeft(4 );
    aTextLine.AdjustRight( -4 );
    aTextLine.Move(0, 4);

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
            CharAttr const & rAttr(aRedlineAttr[pAttrMap[i]]);
            rLB.SetEntryData(i, const_cast<CharAttr*>(&rAttr));
            if (rAttr.nItemId == rAttrToSelect.m_nItemId &&
                rAttr.nAttr == rAttrToSelect.m_nAttr)
                rLB.SelectEntryPos(i);
        }
    }
}

SwRedlineOptionsTabPage::SwRedlineOptionsTabPage( vcl::Window* pParent,
                                                  const SfxItemSet& rSet )
    : SfxTabPage(pParent, "OptRedLinePage",
        "modules/swriter/ui/optredlinepage.ui" , &rSet)
{
    Size aPreviewSize(getPreviewOptionsSize(this));

    get(m_pInsertLB,"insert");
    get(m_pInsertColorLB,"insertcolor");
    m_pInsertColorLB->SetSlotId(SID_AUTHOR_COLOR, true);
    get(m_pInsertedPreviewWN,"insertedpreview");

    get(m_pDeletedLB,"deleted");
    get(m_pDeletedColorLB,"deletedcolor");
    m_pDeletedColorLB->SetSlotId(SID_AUTHOR_COLOR, true);
    get(m_pDeletedPreviewWN,"deletedpreview");

    get(m_pChangedLB,"changed");
    get(m_pChangedColorLB,"changedcolor");
    m_pChangedColorLB->SetSlotId(SID_AUTHOR_COLOR, true);
    get(m_pChangedPreviewWN,"changedpreview");

    get(m_pMarkPosLB,"markpos");
    get(m_pMarkColorLB,"markcolor");
    get(m_pMarkPreviewWN,"markpreview");

    m_pInsertedPreviewWN->set_height_request(aPreviewSize.Height());
    m_pDeletedPreviewWN->set_height_request(aPreviewSize.Height());
    m_pChangedPreviewWN->set_height_request(aPreviewSize.Height());
    m_pMarkPreviewWN->set_height_request(aPreviewSize.Height());

    m_pInsertedPreviewWN->set_width_request(aPreviewSize.Width());
    m_pDeletedPreviewWN->set_width_request(aPreviewSize.Width());
    m_pChangedPreviewWN->set_width_request(aPreviewSize.Width());
    m_pMarkPreviewWN->set_width_request(aPreviewSize.Width());

    for (sal_Int32 i = 0; i < m_pInsertLB->GetEntryCount(); ++i)
    {
        const OUString sEntry(m_pInsertLB->GetEntry(i));
        m_pDeletedLB->InsertEntry(sEntry);
        m_pChangedLB->InsertEntry(sEntry);
    };

    // remove strikethrough from insert and change and underline + double
    // underline from delete
    m_pInsertLB->RemoveEntry(5);
    m_pChangedLB->RemoveEntry(5);
    m_pDeletedLB->RemoveEntry(4);
    m_pDeletedLB->RemoveEntry(3);

    Link<ListBox&,void> aLk = LINK(this, SwRedlineOptionsTabPage, AttribHdl);
    m_pInsertLB->SetSelectHdl( aLk );
    m_pDeletedLB->SetSelectHdl( aLk );
    m_pChangedLB->SetSelectHdl( aLk );

    Link<SvxColorListBox&,void> aLk2 = LINK(this, SwRedlineOptionsTabPage, ColorHdl);
    m_pInsertColorLB->SetSelectHdl( aLk2 );
    m_pDeletedColorLB->SetSelectHdl( aLk2 );
    m_pChangedColorLB->SetSelectHdl( aLk2 );

    m_pMarkPosLB->SetSelectHdl(LINK(this, SwRedlineOptionsTabPage, ChangedMaskPrevHdl));
    m_pMarkColorLB->SetSelectHdl(LINK(this, SwRedlineOptionsTabPage, ChangedMaskColorPrevHdl));
}

SwRedlineOptionsTabPage::~SwRedlineOptionsTabPage()
{
    disposeOnce();
}

void SwRedlineOptionsTabPage::dispose()
{
    m_pInsertLB.clear();
    m_pInsertColorLB.clear();
    m_pInsertedPreviewWN.clear();
    m_pDeletedLB.clear();
    m_pDeletedColorLB.clear();
    m_pDeletedPreviewWN.clear();
    m_pChangedLB.clear();
    m_pChangedColorLB.clear();
    m_pChangedPreviewWN.clear();
    m_pMarkPosLB.clear();
    m_pMarkColorLB.clear();
    m_pMarkPreviewWN.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwRedlineOptionsTabPage::Create( TabPageParent pParent, const SfxItemSet* rSet)
{
    return VclPtr<SwRedlineOptionsTabPage>::Create( pParent.pParent, *rSet );
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

    Color nOldMarkColor = pOpt->GetMarkAlignColor();
    sal_uInt16 nOldMarkMode = pOpt->GetMarkAlignMode();

    sal_Int32 nPos = m_pInsertLB->GetSelectedEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = static_cast<CharAttr *>(m_pInsertLB->GetEntryData(nPos));
        aInsertedAttr.m_nItemId = pAttr->nItemId;
        aInsertedAttr.m_nAttr = pAttr->nAttr;
        aInsertedAttr.m_nColor = m_pInsertColorLB->GetSelectEntryColor();
        pOpt->SetInsertAuthorAttr(aInsertedAttr);
    }

    nPos = m_pDeletedLB->GetSelectedEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = static_cast<CharAttr *>(m_pDeletedLB->GetEntryData(nPos));
        aDeletedAttr.m_nItemId = pAttr->nItemId;
        aDeletedAttr.m_nAttr = pAttr->nAttr;
        aDeletedAttr.m_nColor = m_pDeletedColorLB->GetSelectEntryColor();
        pOpt->SetDeletedAuthorAttr(aDeletedAttr);
    }

    nPos = m_pChangedLB->GetSelectedEntryPos();
    if (nPos != LISTBOX_ENTRY_NOTFOUND)
    {
        pAttr = static_cast<CharAttr *>(m_pChangedLB->GetEntryData(nPos));
        aChangedAttr.m_nItemId = pAttr->nItemId;
        aChangedAttr.m_nAttr = pAttr->nAttr;
        aChangedAttr.m_nColor = m_pChangedColorLB->GetSelectEntryColor();
        pOpt->SetFormatAuthorAttr(aChangedAttr);
    }

    nPos = 0;
    switch (m_pMarkPosLB->GetSelectedEntryPos())
    {
        case 0: nPos = text::HoriOrientation::NONE;       break;
        case 1: nPos = text::HoriOrientation::LEFT;       break;
        case 2: nPos = text::HoriOrientation::RIGHT;      break;
        case 3: nPos = text::HoriOrientation::OUTSIDE;    break;
        case 4: nPos = text::HoriOrientation::INSIDE;     break;
    }
    pOpt->SetMarkAlignMode(nPos);
    pOpt->SetMarkAlignColor(m_pMarkColorLB->GetSelectEntryColor());

    if (!(aInsertedAttr == aOldInsertAttr) ||
        !(aDeletedAttr == aOldDeletedAttr) ||
        !(aChangedAttr == aOldChangedAttr) ||
       nOldMarkColor != pOpt->GetMarkAlignColor() ||
       nOldMarkMode != pOpt->GetMarkAlignMode() )
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
    InitFontStyle(*m_pInsertedPreviewWN);
    InitFontStyle(*m_pDeletedPreviewWN);
    InitFontStyle(*m_pChangedPreviewWN);

    Color nColor = rInsertAttr.m_nColor;
    m_pInsertColorLB->SelectEntry(nColor);

    nColor = rDeletedAttr.m_nColor;
    m_pDeletedColorLB->SelectEntry(nColor);

    nColor = rChangedAttr.m_nColor;
    m_pChangedColorLB->SelectEntry(nColor);

    m_pMarkColorLB->SelectEntry(pOpt->GetMarkAlignColor());

    m_pInsertLB->SelectEntryPos(0);
    m_pDeletedLB->SelectEntryPos(0);
    m_pChangedLB->SelectEntryPos(0);

    lcl_FillRedlineAttrListBox(*m_pInsertLB, rInsertAttr, aInsertAttrMap, SAL_N_ELEMENTS(aInsertAttrMap));
    lcl_FillRedlineAttrListBox(*m_pDeletedLB, rDeletedAttr, aDeletedAttrMap, SAL_N_ELEMENTS(aDeletedAttrMap));
    lcl_FillRedlineAttrListBox(*m_pChangedLB, rChangedAttr, aChangedAttrMap, SAL_N_ELEMENTS(aChangedAttrMap));

    sal_Int32 nPos = 0;
    switch (pOpt->GetMarkAlignMode())
    {
        case text::HoriOrientation::NONE:     nPos = 0;   break;
        case text::HoriOrientation::LEFT:     nPos = 1;   break;
        case text::HoriOrientation::RIGHT:    nPos = 2;   break;
        case text::HoriOrientation::OUTSIDE:  nPos = 3;   break;
        case text::HoriOrientation::INSIDE:   nPos = 4;   break;
    }
    m_pMarkPosLB->SelectEntryPos(nPos);

    // show settings in preview
    AttribHdl(*m_pInsertLB);
    ColorHdl(*m_pInsertColorLB);
    AttribHdl(*m_pDeletedLB);
    ColorHdl(*m_pInsertColorLB);
    AttribHdl(*m_pChangedLB);
    ColorHdl(*m_pChangedColorLB);

    ChangedMaskPrev();
}

IMPL_LINK( SwRedlineOptionsTabPage, AttribHdl, ListBox&, rLB, void )
{
    SvxFontPrevWindow *pPrev = nullptr;
    SvxColorListBox *pColorLB;

    if (&rLB == m_pInsertLB)
    {
        pColorLB = m_pInsertColorLB;
        pPrev = m_pInsertedPreviewWN;
    }
    else if (&rLB == m_pDeletedLB)
    {
        pColorLB = m_pDeletedColorLB;
        pPrev = m_pDeletedPreviewWN;
    }
    else
    {
        pColorLB = m_pChangedColorLB;
        pPrev = m_pChangedPreviewWN;
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
    rFont.SetCaseMap(SvxCaseMap::NotMapped);
    rCJKFont.SetCaseMap(SvxCaseMap::NotMapped);

    Color aColor = pColorLB->GetSelectEntryColor();

    if (aColor == COL_NONE_COLOR)
    {
        rFont.SetColor( COL_BLACK );
        rCJKFont.SetColor( COL_BLACK );
    }
    else if (aColor == COL_TRANSPARENT)
    {
        rFont.SetColor( COL_RED );
        rCJKFont.SetColor( COL_RED );
    }
    else
    {
        rFont.SetColor(aColor);
        rCJKFont.SetColor(aColor);
    }

    sal_Int32 nPos = rLB.GetSelectedEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;

    CharAttr*   pAttr = static_cast<CharAttr*>(rLB.GetEntryData( nPos ));
    //switch off preview background color
    pPrev->ResetColor();
    switch (pAttr->nItemId)
    {
        case SID_ATTR_CHAR_WEIGHT:
            rFont.SetWeight( static_cast<FontWeight>(pAttr->nAttr) );
            rCJKFont.SetWeight( static_cast<FontWeight>(pAttr->nAttr) );
            break;

        case SID_ATTR_CHAR_POSTURE:
            rFont.SetItalic( static_cast<FontItalic>(pAttr->nAttr) );
            rCJKFont.SetItalic( static_cast<FontItalic>(pAttr->nAttr) );
            break;

        case SID_ATTR_CHAR_UNDERLINE:
            rFont.SetUnderline( static_cast<FontLineStyle>(pAttr->nAttr) );
            rCJKFont.SetUnderline( static_cast<FontLineStyle>(pAttr->nAttr) );
            break;

        case SID_ATTR_CHAR_STRIKEOUT:
            rFont.SetStrikeout( static_cast<FontStrikeout>(pAttr->nAttr) );
            rCJKFont.SetStrikeout( static_cast<FontStrikeout>(pAttr->nAttr) );
            break;

        case SID_ATTR_CHAR_CASEMAP:
            rFont.SetCaseMap( static_cast<SvxCaseMap>(pAttr->nAttr) );
            rCJKFont.SetCaseMap( static_cast<SvxCaseMap>(pAttr->nAttr) );
            break;

        case SID_ATTR_BRUSH:
        {
            Color aBgColor = pColorLB->GetSelectEntryColor();
            if (aBgColor != COL_NONE_COLOR)
                pPrev->SetColor(aBgColor);
            else
                pPrev->SetColor(COL_LIGHTGRAY);
            rFont.SetColor( COL_BLACK );
            rCJKFont.SetColor( COL_BLACK );
        }
        break;
    }

    pPrev->Invalidate();
}

IMPL_LINK( SwRedlineOptionsTabPage, ColorHdl, SvxColorListBox&, rListBox, void )
{
    SvxColorListBox* pColorLB = &rListBox;
    SvxFontPrevWindow *pPrev = nullptr;
    ListBox* pLB;

    if (pColorLB == m_pInsertColorLB)
    {
        pLB = m_pInsertLB;
        pPrev = m_pInsertedPreviewWN;
    }
    else if (pColorLB == m_pDeletedColorLB)
    {
        pLB = m_pDeletedLB;
        pPrev = m_pDeletedPreviewWN;
    }
    else
    {
        pLB = m_pChangedLB;
        pPrev = m_pChangedPreviewWN;
    }

    SvxFont&    rFont = pPrev->GetFont();
    SvxFont&    rCJKFont = pPrev->GetCJKFont();
    sal_Int32      nPos = pLB->GetSelectedEntryPos();
    if( nPos == LISTBOX_ENTRY_NOTFOUND )
        nPos = 0;

    CharAttr*   pAttr = static_cast<CharAttr*>(pLB->GetEntryData( nPos ));

    if( pAttr->nItemId == SID_ATTR_BRUSH )
    {
        rFont.SetColor( COL_BLACK );
        rCJKFont.SetColor( COL_BLACK );

        Color aBgColor = pColorLB->GetSelectEntryColor();
        if (aBgColor != COL_NONE_COLOR)
            pPrev->SetColor(aBgColor);
        else
            pPrev->SetColor(COL_LIGHTGRAY);
    }
    else
    {
        Color aColor = pColorLB->GetSelectEntryColor();

        if (aColor == COL_NONE_COLOR)
        {
            rFont.SetColor( COL_BLACK );
            rCJKFont.SetColor( COL_BLACK );
        }
        else if (aColor == COL_TRANSPARENT)
        {
            rFont.SetColor( COL_RED );
            rCJKFont.SetColor( COL_RED );
        }
        else
        {
            rFont.SetColor(aColor);
            rCJKFont.SetColor(aColor);
        }
    }

    pPrev->Invalidate();
}

void SwRedlineOptionsTabPage::ChangedMaskPrev()
{
    m_pMarkPreviewWN->SetMarkPos(m_pMarkPosLB->GetSelectedEntryPos());
    m_pMarkPreviewWN->SetColor(m_pMarkColorLB->GetSelectEntryColor());

    m_pMarkPreviewWN->Invalidate();
}

IMPL_LINK_NOARG(SwRedlineOptionsTabPage, ChangedMaskPrevHdl, ListBox&, void)
{
    ChangedMaskPrev();
}

IMPL_LINK_NOARG(SwRedlineOptionsTabPage, ChangedMaskColorPrevHdl, SvxColorListBox&, void)
{
    ChangedMaskPrev();
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

SwCompareOptionsTabPage::SwCompareOptionsTabPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/optcomparison.ui", "OptComparison", &rSet)
    , m_xAutoRB(m_xBuilder->weld_radio_button("auto"))
    , m_xWordRB(m_xBuilder->weld_radio_button("byword"))
    , m_xCharRB(m_xBuilder->weld_radio_button("bycharacter"))
    , m_xRsidCB(m_xBuilder->weld_check_button("useRSID"))
    , m_xIgnoreCB(m_xBuilder->weld_check_button("ignore"))
    , m_xLenNF(m_xBuilder->weld_spin_button("ignorelen"))
    , m_xStoreRsidCB(m_xBuilder->weld_check_button("storeRSID"))
{
    Link<weld::Button&,void> aLnk( LINK( this, SwCompareOptionsTabPage, ComparisonHdl ) );
    m_xAutoRB->connect_clicked( aLnk );
    m_xWordRB->connect_clicked( aLnk );
    m_xCharRB->connect_clicked( aLnk );

    m_xIgnoreCB->connect_clicked( LINK( this, SwCompareOptionsTabPage, IgnoreHdl) );
}

SwCompareOptionsTabPage::~SwCompareOptionsTabPage()
{
}

VclPtr<SfxTabPage> SwCompareOptionsTabPage::Create( TabPageParent pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<SwCompareOptionsTabPage>::Create( pParent, *rAttrSet );
}

bool SwCompareOptionsTabPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    if( m_xAutoRB->get_state_changed_from_saved() ||
        m_xWordRB->get_state_changed_from_saved() ||
        m_xCharRB->get_state_changed_from_saved() )
    {
        SwCompareMode eCmpMode = SwCompareMode::Auto;

        if ( m_xAutoRB->get_active() ) eCmpMode = SwCompareMode::Auto;
        if ( m_xWordRB->get_active() ) eCmpMode = SwCompareMode::ByWord;
        if ( m_xCharRB->get_active() ) eCmpMode = SwCompareMode::ByChar;

        pOpt->SetCompareMode( eCmpMode );
        bRet = true;
    }

    if( m_xRsidCB->get_state_changed_from_saved() )
    {
        pOpt->SetUseRsid( m_xRsidCB->get_active() );
        bRet = true;
    }

    if( m_xIgnoreCB->get_state_changed_from_saved() )
    {
        pOpt->SetIgnorePieces( m_xIgnoreCB->get_active() );
        bRet = true;
    }

    if( m_xLenNF->get_value_changed_from_saved() )
    {
        pOpt->SetPieceLen( m_xLenNF->get_value() );
        bRet = true;
    }

    if (m_xStoreRsidCB->get_state_changed_from_saved())
    {
        pOpt->SetStoreRsid(m_xStoreRsidCB->get_active());
        bRet = true;
    }

    return bRet;
}

void SwCompareOptionsTabPage::Reset( const SfxItemSet* )
{
    SwModuleOptions *pOpt = SW_MOD()->GetModuleConfig();

    SwCompareMode eCmpMode = pOpt->GetCompareMode();
    if( eCmpMode == SwCompareMode::Auto )
    {
        m_xAutoRB->set_active(true);
        m_xRsidCB->set_sensitive(false);
        m_xIgnoreCB->set_sensitive(false);
        m_xLenNF->set_sensitive(false);
    }
    else if( eCmpMode == SwCompareMode::ByWord )
    {
        m_xWordRB->set_active(true);
        m_xRsidCB->set_sensitive(true);
        m_xIgnoreCB->set_sensitive(true);
        m_xLenNF->set_sensitive(true);
    }
    else if( eCmpMode == SwCompareMode::ByChar)
    {
        m_xCharRB->set_active(true);
        m_xRsidCB->set_sensitive(true);
        m_xIgnoreCB->set_sensitive(true);
        m_xLenNF->set_sensitive(true);
    }
    m_xAutoRB->save_state();
    m_xWordRB->save_state();
    m_xCharRB->save_state();

    m_xRsidCB->set_active( pOpt->IsUseRsid() );
    m_xRsidCB->save_state();

    m_xIgnoreCB->set_active( pOpt->IsIgnorePieces() );
    m_xIgnoreCB->save_state();

    m_xLenNF->set_sensitive( m_xIgnoreCB->get_active() && eCmpMode != SwCompareMode::Auto );

    m_xLenNF->set_value( pOpt->GetPieceLen() );
    m_xLenNF->save_value();

    m_xStoreRsidCB->set_active(pOpt->IsStoreRsid());
    m_xStoreRsidCB->save_state();
}

IMPL_LINK_NOARG(SwCompareOptionsTabPage, ComparisonHdl, weld::Button&, void)
{
    bool bChecked = !m_xAutoRB->get_active();
    m_xRsidCB->set_sensitive( bChecked );
    m_xIgnoreCB->set_sensitive( bChecked );
    m_xLenNF->set_sensitive( bChecked && m_xIgnoreCB->get_active() );
}

IMPL_LINK_NOARG(SwCompareOptionsTabPage, IgnoreHdl, weld::Button&, void)
{
    m_xLenNF->set_sensitive(m_xIgnoreCB->get_active());
}

#ifdef DBG_UTIL

SwTestTabPage::SwTestTabPage(TabPageParent pParent, const SfxItemSet& rCoreSet)
    : SfxTabPage( pParent, "modules/swriter/ui/opttestpage.ui", "OptTestPage", &rCoreSet)
    , bAttrModified( false )
    , m_xTest1CBox(m_xBuilder->weld_check_button("unused"))
    , m_xTest2CBox(m_xBuilder->weld_check_button("dynamic"))
    , m_xTest3CBox(m_xBuilder->weld_check_button("nocalm"))
    , m_xTest4CBox(m_xBuilder->weld_check_button("wysiwygdbg"))
    , m_xTest5CBox(m_xBuilder->weld_check_button("noidle"))
    , m_xTest6CBox(m_xBuilder->weld_check_button("noscreenadj"))
    , m_xTest7CBox(m_xBuilder->weld_check_button("winformat"))
    , m_xTest8CBox(m_xBuilder->weld_check_button("noscroll"))
    , m_xTest9CBox(m_xBuilder->weld_check_button("DrawingLayerNotLoading"))
    , m_xTest10CBox(m_xBuilder->weld_check_button("AutoFormatByInput"))
{
    Init();
}

SwTestTabPage::~SwTestTabPage()
{
}

VclPtr<SfxTabPage> SwTestTabPage::Create( TabPageParent pParent,
                                          const SfxItemSet* rAttrSet )
{
    return VclPtr<SwTestTabPage>::Create(pParent, *rAttrSet);
}

bool    SwTestTabPage::FillItemSet( SfxItemSet* rCoreSet )
{

    if ( bAttrModified )
    {
        SwTestItem aTestItem;
        aTestItem.bTest1=m_xTest1CBox->get_active();
        aTestItem.bTest2=m_xTest2CBox->get_active();
        aTestItem.bTest3=m_xTest3CBox->get_active();
        aTestItem.bTest4=m_xTest4CBox->get_active();
        aTestItem.bTest5=m_xTest5CBox->get_active();
        aTestItem.bTest6=m_xTest6CBox->get_active();
        aTestItem.bTest7=m_xTest7CBox->get_active();
        aTestItem.bTest8=m_xTest8CBox->get_active();
        aTestItem.bTest9=m_xTest9CBox->get_active();
        aTestItem.bTest10=m_xTest10CBox->get_active();
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
        m_xTest1CBox->set_active(pTestAttr->bTest1);
        m_xTest2CBox->set_active(pTestAttr->bTest2);
        m_xTest3CBox->set_active(pTestAttr->bTest3);
        m_xTest4CBox->set_active(pTestAttr->bTest4);
        m_xTest5CBox->set_active(pTestAttr->bTest5);
        m_xTest6CBox->set_active(pTestAttr->bTest6);
        m_xTest7CBox->set_active(pTestAttr->bTest7);
        m_xTest8CBox->set_active(pTestAttr->bTest8);
        m_xTest9CBox->set_active(pTestAttr->bTest9);
        m_xTest10CBox->set_active(pTestAttr->bTest10);
    }
}

void SwTestTabPage::Init()
{
    // handler
    Link<weld::Button&,void> aLk = LINK( this, SwTestTabPage, AutoClickHdl );
    m_xTest1CBox->connect_clicked( aLk );
    m_xTest2CBox->connect_clicked( aLk );
    m_xTest3CBox->connect_clicked( aLk );
    m_xTest4CBox->connect_clicked( aLk );
    m_xTest5CBox->connect_clicked( aLk );
    m_xTest6CBox->connect_clicked( aLk );
    m_xTest7CBox->connect_clicked( aLk );
    m_xTest8CBox->connect_clicked( aLk );
    m_xTest9CBox->connect_clicked( aLk );
    m_xTest10CBox->connect_clicked( aLk );
}

IMPL_LINK_NOARG(SwTestTabPage, AutoClickHdl, weld::Button&, void)
{
    bAttrModified = true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
