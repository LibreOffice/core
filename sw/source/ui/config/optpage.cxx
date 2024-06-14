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

#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <cfgitems.hxx>
#include <poolfmt.hxx>
#include <uiitems.hxx>
#include <printdata.hxx>
#include <modcfg.hxx>
#include <crstate.hxx>
#include <viewopt.hxx>
#include <globals.hrc>
#include <strings.hrc>
#include <swwrtshitem.hxx>

#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxenum.hxx>
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Writer.hxx>
#include <officecfg/Office/WriterWeb.hxx>
#include <sal/macros.h>
#include <sfx2/dialoghelper.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/ctloptions.hxx>
#include <svl/eitem.hxx>
#include <svl/cjkoptions.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/unitconv.hxx>
#include <sfx2/htmlmode.hxx>

#include <vcl/settings.hxx>
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
SwContentOptPage::SwContentOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/viewoptionspage.ui"_ustr, u"ViewOptionsPage"_ustr, &rCoreSet)
    , m_xCrossCB(m_xBuilder->weld_check_button(u"helplines"_ustr))
    , m_xCrossImg(m_xBuilder->weld_widget(u"lockhelplines"_ustr))
    , m_xHMetric(m_xBuilder->weld_combo_box(u"hrulercombobox"_ustr))
    , m_xHMetricImg(m_xBuilder->weld_widget(u"lockhruler"_ustr))
    , m_xVRulerCBox(m_xBuilder->weld_check_button(u"vruler"_ustr))
    , m_xVRulerImg(m_xBuilder->weld_widget(u"lockvruler"_ustr))
    , m_xVRulerRightCBox(m_xBuilder->weld_check_button(u"vrulerright"_ustr))
    , m_xVRulerRightImg(m_xBuilder->weld_widget(u"lockvrulerright"_ustr))
    , m_xVMetric(m_xBuilder->weld_combo_box(u"vrulercombobox"_ustr))
    , m_xSmoothCBox(m_xBuilder->weld_check_button(u"smoothscroll"_ustr))
    , m_xSmoothImg(m_xBuilder->weld_widget(u"locksmoothscroll"_ustr))
    , m_xGrfCB(m_xBuilder->weld_check_button(u"graphics"_ustr))
    , m_xGrfImg(m_xBuilder->weld_widget(u"lockgraphics"_ustr))
    , m_xTableCB(m_xBuilder->weld_check_button(u"tables"_ustr))
    , m_xTableImg(m_xBuilder->weld_widget(u"locktables"_ustr))
    , m_xDrwCB(m_xBuilder->weld_check_button(u"drawings"_ustr))
    , m_xDrwImg(m_xBuilder->weld_widget(u"lockdrawings"_ustr))
    , m_xPostItCB(m_xBuilder->weld_check_button(u"comments"_ustr))
    , m_xSettingsFrame(m_xBuilder->weld_frame(u"settingsframe"_ustr))
    , m_xSettingsLabel(m_xBuilder->weld_label(u"settingslabel"_ustr))
    , m_xMetricLabel(m_xBuilder->weld_label(u"measureunitlabel"_ustr))
    , m_xMetricLB(m_xBuilder->weld_combo_box(u"measureunit"_ustr))
    , m_xMetricImg(m_xBuilder->weld_widget(u"lockmeasureunit"_ustr))
    , m_xShowInlineTooltips(m_xBuilder->weld_check_button(u"changestooltip"_ustr))
    , m_xShowInlineTooltipsImg(m_xBuilder->weld_widget(u"lockchangestooltip"_ustr))
    , m_xShowOutlineContentVisibilityButton(m_xBuilder->weld_check_button(u"outlinecontentvisibilitybutton"_ustr))
    , m_xShowOutlineContentVImg(m_xBuilder->weld_widget(u"lockoutlinecontentvisibility"_ustr))
    , m_xTreatSubOutlineLevelsAsContent(m_xBuilder->weld_check_button(u"suboutlinelevelsascontent"_ustr))
    , m_xTreatSubOutlineLevelsImg(m_xBuilder->weld_widget(u"locksuboutlinelevels"_ustr))
    , m_xShowChangesInMargin(m_xBuilder->weld_check_button(u"changesinmargin"_ustr))
    , m_xShowChangesInMarginImg(m_xBuilder->weld_widget(u"lockchangesinmargin"_ustr))
    , m_xFieldHiddenCB(m_xBuilder->weld_check_button(u"hiddentextfield"_ustr))
    , m_xFieldHiddenImg(m_xBuilder->weld_widget(u"lockhiddentextfield"_ustr))
    , m_xFieldHiddenParaCB(m_xBuilder->weld_check_button(u"hiddenparafield"_ustr))
    , m_xFieldHiddenParaImg(m_xBuilder->weld_widget(u"lockhiddenparafield"_ustr))
{
    m_xShowOutlineContentVisibilityButton->connect_toggled(LINK(this, SwContentOptPage, ShowOutlineContentVisibilityButtonHdl));

    /* This part is visible only with Writer/Web->View dialogue. */
    const SfxUInt16Item* pItem = rCoreSet.GetItemIfSet(SID_HTML_MODE, false );
    if (!pItem || !(pItem->GetValue() & HTMLMODE_ON))
    {
        m_xSettingsFrame->hide();
        m_xSettingsLabel->hide();
        m_xMetricLabel->hide();
        m_xMetricLB->hide();
    }

    if(!SvtCJKOptions::IsVerticalTextEnabled() )
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

std::unique_ptr<SfxTabPage> SwContentOptPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                             const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwContentOptPage>(pPage, pController, *rAttrSet);
}

static void lcl_SelectMetricLB(weld::ComboBox& rMetric, TypedWhichId<SfxUInt16Item> nSID, const SfxItemSet& rSet)
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
    bool bReadOnly = false;
    bool bWebOptionsPage = m_xSettingsFrame->is_visible();
    const SwElemItem* pElemAttr = rSet->GetItemIfSet( FN_PARAM_ELEM , false );
    if(pElemAttr)
    {
        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Content::Display::Table::isReadOnly() :
            officecfg::Office::WriterWeb::Content::Display::Table::isReadOnly();
        m_xTableCB->set_active(pElemAttr->m_bTable);
        m_xTableCB->set_sensitive(!bReadOnly);
        m_xTableImg->set_visible(bReadOnly);

        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Content::Display::GraphicObject::isReadOnly() :
            officecfg::Office::WriterWeb::Content::Display::GraphicObject::isReadOnly();
        m_xGrfCB->set_active(pElemAttr->m_bGraphic);
        m_xGrfCB->set_sensitive(!bReadOnly);
        m_xGrfImg->set_visible(bReadOnly);

        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Content::Display::DrawingControl::isReadOnly() :
            officecfg::Office::WriterWeb::Content::Display::DrawingControl::isReadOnly();
        m_xDrwCB->set_active(pElemAttr->m_bDrawing);
        m_xDrwCB->set_sensitive(!bReadOnly);
        m_xDrwImg->set_visible(bReadOnly);

        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Content::Display::Note::isReadOnly() :
            officecfg::Office::WriterWeb::Content::Display::Note::isReadOnly();
        m_xPostItCB->set_active(pElemAttr->m_bNotes);
        m_xPostItCB->set_sensitive(!bReadOnly);
        m_xPostItCB->set_visible(pElemAttr->m_bNotes);

        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Layout::Line::Guide::isReadOnly() :
            officecfg::Office::WriterWeb::Layout::Line::Guide::isReadOnly();
        m_xCrossCB->set_active(pElemAttr->m_bCrosshair);
        m_xCrossCB->set_sensitive(!bReadOnly);
        m_xCrossImg->set_visible(bReadOnly);

        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Layout::Window::VerticalRuler::isReadOnly() :
            officecfg::Office::WriterWeb::Layout::Window::VerticalRuler::isReadOnly();
        m_xVRulerCBox->set_active(pElemAttr->m_bVertRuler);
        m_xVRulerCBox->set_sensitive(!bReadOnly);
        m_xVRulerImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Layout::Window::IsVerticalRulerRight::isReadOnly();
        m_xVRulerRightCBox->set_active(pElemAttr->m_bVertRulerRight);
        m_xVRulerRightCBox->set_sensitive(!bReadOnly);
        m_xVRulerRightImg->set_visible(bReadOnly);

        bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Layout::Window::SmoothScroll::isReadOnly() :
            officecfg::Office::WriterWeb::Layout::Window::SmoothScroll::isReadOnly();
        m_xSmoothCBox->set_active(pElemAttr->m_bSmoothScroll);
        m_xSmoothCBox->set_sensitive(!bReadOnly);
        m_xSmoothImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::Display::ShowInlineTooltips::isReadOnly();
        m_xShowInlineTooltips->set_active(pElemAttr->m_bShowInlineTooltips);
        m_xShowInlineTooltips->set_sensitive(!bReadOnly);
        m_xShowInlineTooltipsImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::Display::ShowOutlineContentVisibilityButton::isReadOnly();
        m_xShowOutlineContentVisibilityButton->set_active(pElemAttr->m_bShowOutlineContentVisibilityButton);
        m_xShowOutlineContentVisibilityButton->set_sensitive(!bReadOnly);
        m_xShowOutlineContentVImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::Display::TreatSubOutlineLevelsAsContent::isReadOnly();
        m_xTreatSubOutlineLevelsAsContent->set_active(pElemAttr->m_bTreatSubOutlineLevelsAsContent);
        m_xTreatSubOutlineLevelsAsContent->set_sensitive(pElemAttr->m_bShowOutlineContentVisibilityButton && !bReadOnly);
        m_xTreatSubOutlineLevelsImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::Display::ShowChangesInMargin::isReadOnly();
        m_xShowChangesInMargin->set_active(pElemAttr->m_bShowChangesInMargin);
        m_xShowChangesInMargin->set_sensitive(!bReadOnly);
        m_xShowChangesInMarginImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::NonprintingCharacter::HiddenText::isReadOnly();
        m_xFieldHiddenCB->set_active( pElemAttr->m_bFieldHiddenText );
        m_xFieldHiddenCB->set_sensitive(!bReadOnly);
        m_xFieldHiddenImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::NonprintingCharacter::HiddenParagraph::isReadOnly();
        m_xFieldHiddenParaCB->set_active( pElemAttr->m_bShowHiddenPara );
        m_xFieldHiddenParaCB->set_sensitive(!bReadOnly);
        m_xFieldHiddenParaImg->set_visible(bReadOnly);
    }

    bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Layout::Window::HorizontalRulerUnit::isReadOnly() :
        officecfg::Office::WriterWeb::Layout::Window::HorizontalRulerUnit::isReadOnly();
    m_xHMetric->set_sensitive(!bReadOnly);
    m_xHMetricImg->set_visible(bReadOnly);

    bReadOnly = !bWebOptionsPage ? officecfg::Office::Writer::Layout::Window::VerticalRulerUnit::isReadOnly() :
        officecfg::Office::WriterWeb::Layout::Window::VerticalRulerUnit::isReadOnly();
    m_xVMetric->set_sensitive(!bReadOnly);

    m_xMetricLB->set_active(-1);
    if (bWebOptionsPage)
    {
        bReadOnly = officecfg::Office::WriterWeb::Layout::Other::MeasureUnit::isReadOnly();
        m_xMetricLB->set_sensitive(!bReadOnly);
        m_xMetricImg->set_visible(bReadOnly);
    }
    lcl_SelectMetricLB(*m_xMetricLB, SID_ATTR_METRIC, *rSet);
    lcl_SelectMetricLB(*m_xHMetric, FN_HSCROLL_METRIC, *rSet);
    lcl_SelectMetricLB(*m_xVMetric, FN_VSCROLL_METRIC, *rSet);
}

OUString SwContentOptPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { u"guideslabel"_ustr, u"displaylabel"_ustr,  u"displayfl1"_ustr,       u"changeslabel"_ustr, u"label3"_ustr,
            u"hruler"_ustr,      u"settingslabel"_ustr, u"measureunitlabel"_ustr, u"outlinelabel"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"helplines"_ustr,
                               u"graphics"_ustr,
                               u"tables"_ustr,
                               u"drawings"_ustr,
                               u"comments"_ustr,
                               u"resolvedcomments"_ustr,
                               u"hiddentextfield"_ustr,
                               u"hiddenparafield"_ustr,
                               u"changesinmargin"_ustr,
                               u"changestooltip"_ustr,
                               u"vruler"_ustr,
                               u"vrulerright"_ustr,
                               u"smoothscroll"_ustr,
                               u"outlinecontentvisibilitybutton"_ustr,
                               u"suboutlinelevelsascontent"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SwContentOptPage::FillItemSet(SfxItemSet* rSet)
{
    const SwElemItem* pOldAttr = GetOldItem(GetItemSet(), FN_PARAM_ELEM);

    SwElemItem aElem;
    aElem.m_bTable                = m_xTableCB->get_active();
    aElem.m_bGraphic              = m_xGrfCB->get_active();
    aElem.m_bDrawing              = m_xDrwCB->get_active();
    aElem.m_bNotes                = m_xPostItCB->get_active();
    aElem.m_bCrosshair            = m_xCrossCB->get_active();
    aElem.m_bVertRuler            = m_xVRulerCBox->get_active();
    aElem.m_bVertRulerRight       = m_xVRulerRightCBox->get_active();
    aElem.m_bSmoothScroll         = m_xSmoothCBox->get_active();
    aElem.m_bShowInlineTooltips   = m_xShowInlineTooltips->get_active();
    aElem.m_bShowOutlineContentVisibilityButton = m_xShowOutlineContentVisibilityButton->get_active();
    aElem.m_bTreatSubOutlineLevelsAsContent = m_xTreatSubOutlineLevelsAsContent->get_active();
    aElem.m_bShowChangesInMargin  = m_xShowChangesInMargin->get_active();
    aElem.m_bFieldHiddenText      = m_xFieldHiddenCB->get_active();
    aElem.m_bShowHiddenPara       = m_xFieldHiddenParaCB->get_active();

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

IMPL_LINK(SwContentOptPage, VertRulerHdl, weld::Toggleable&, rBox, void)
{
    m_xVRulerRightCBox->set_sensitive(rBox.get_sensitive() && rBox.get_active() &&
        !officecfg::Office::Writer::Layout::Window::IsVerticalRulerRight::isReadOnly());
}

IMPL_LINK(SwContentOptPage, ShowOutlineContentVisibilityButtonHdl, weld::Toggleable&, rBox, void)
{
    m_xTreatSubOutlineLevelsAsContent->set_sensitive(rBox.get_active());
}

// TabPage Printer additional settings
SwAddPrinterTabPage::SwAddPrinterTabPage(weld::Container* pPage, weld::DialogController* pController,
    const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/printoptionspage.ui"_ustr, u"PrintOptionsPage"_ustr, &rCoreSet)
    , m_sNone(SwResId(SW_STR_NONE))
    , m_bAttrModified(false)
    , m_bPreview(false)
    , m_bHTMLMode(false)
    , m_xGrfCB(m_xBuilder->weld_check_button(u"graphics"_ustr))
    , m_xGrfImg(m_xBuilder->weld_widget(u"lockgraphics"_ustr))
    , m_xCtrlFieldCB(m_xBuilder->weld_check_button(u"formcontrols"_ustr))
    , m_xCtrlFieldImg(m_xBuilder->weld_widget(u"lockformcontrols"_ustr))
    , m_xBackgroundCB(m_xBuilder->weld_check_button(u"background"_ustr))
    , m_xBackgroundImg(m_xBuilder->weld_widget(u"lockbackground"_ustr))
    , m_xBlackFontCB(m_xBuilder->weld_check_button(u"inblack"_ustr))
    , m_xBlackFontImg(m_xBuilder->weld_widget(u"lockinblack"_ustr))
    , m_xPrintHiddenTextCB(m_xBuilder->weld_check_button(u"hiddentext"_ustr))
    , m_xPrintHiddenTextImg(m_xBuilder->weld_widget(u"lockhiddentext"_ustr))
    , m_xPrintTextPlaceholderCB(m_xBuilder->weld_check_button(u"textplaceholder"_ustr))
    , m_xPrintTextPlaceholderImg(m_xBuilder->weld_widget(u"locktextplaceholder"_ustr))
    , m_xPagesFrame(m_xBuilder->weld_widget(u"pagesframe"_ustr))
    , m_xLeftPageCB(m_xBuilder->weld_check_button(u"leftpages"_ustr))
    , m_xLeftPageImg(m_xBuilder->weld_widget(u"lockleftpages"_ustr))
    , m_xRightPageCB(m_xBuilder->weld_check_button(u"rightpages"_ustr))
    , m_xRightPageImg(m_xBuilder->weld_widget(u"lockrightpages"_ustr))
    , m_xProspectCB(m_xBuilder->weld_check_button(u"brochure"_ustr))
    , m_xProspectImg(m_xBuilder->weld_widget(u"lockbrochure"_ustr))
    , m_xProspectCB_RTL(m_xBuilder->weld_check_button(u"rtl"_ustr))
    , m_xProspectImg_RTL(m_xBuilder->weld_widget(u"lockrtl"_ustr))
    , m_xCommentsFrame(m_xBuilder->weld_widget(u"commentsframe"_ustr))
    , m_xNoRB(m_xBuilder->weld_radio_button(u"none"_ustr))
    , m_xOnlyRB(m_xBuilder->weld_radio_button(u"only"_ustr))
    , m_xEndRB(m_xBuilder->weld_radio_button(u"end"_ustr))
    , m_xEndPageRB(m_xBuilder->weld_radio_button(u"endpage"_ustr))
    , m_xInMarginsRB(m_xBuilder->weld_radio_button(u"inmargins"_ustr))
    , m_xMarginsImg(m_xBuilder->weld_widget(u"lockcomments"_ustr))
    , m_xPrintEmptyPagesCB(m_xBuilder->weld_check_button(u"blankpages"_ustr))
    , m_xPrintEmptyPagesImg(m_xBuilder->weld_widget(u"lockblankpages"_ustr))
    , m_xPaperFromSetupCB(m_xBuilder->weld_check_button(u"papertray"_ustr))
    , m_xPaperFromSetupImg(m_xBuilder->weld_widget(u"lockpapertray"_ustr))
    , m_xFaxLB(m_xBuilder->weld_combo_box(u"fax"_ustr))
    , m_xFaxImg(m_xBuilder->weld_widget(u"lockfax"_ustr))
{
    Link<weld::Toggleable&,void> aLk = LINK( this, SwAddPrinterTabPage, AutoClickHdl);
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

    const SfxUInt16Item* pItem = rCoreSet.GetItemIfSet(SID_HTML_MODE, false );
    if(pItem && pItem->GetValue() & HTMLMODE_ON)
    {
        m_bHTMLMode = true;
        m_xLeftPageCB->hide();
        m_xRightPageCB->hide();
        m_xPrintHiddenTextCB->hide();
        m_xPrintTextPlaceholderCB->hide();
        m_xPrintEmptyPagesCB->hide();
    }
    m_xProspectCB_RTL->set_sensitive(false);
    SvtCTLOptions aCTLOptions;
    m_xProspectCB_RTL->set_visible(SvtCTLOptions::IsCTLFontEnabled());
}

SwAddPrinterTabPage::~SwAddPrinterTabPage()
{
}

void SwAddPrinterTabPage::SetPreview(bool bPrev)
{
    m_bPreview = bPrev;
    m_xCommentsFrame->set_sensitive(!m_bPreview);
    m_xPagesFrame->set_sensitive(!m_bPreview);
}

std::unique_ptr<SfxTabPage> SwAddPrinterTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwAddPrinterTabPage>(pPage, pController, *rAttrSet);
}

OUString SwAddPrinterTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label2"_ustr, u"label10"_ustr, u"label1"_ustr, u"label5"_ustr, u"4"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[]
        = { u"graphics"_ustr,  u"formcontrols"_ustr, u"background"_ustr, u"inblack"_ustr, u"hiddentext"_ustr, u"textplaceholder"_ustr,
            u"leftpages"_ustr, u"rightpages"_ustr,   u"brochure"_ustr,   u"rtl"_ustr,     u"blankpages"_ustr, u"papertray"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"none"_ustr, u"only"_ustr, u"end"_ustr, u"endpage"_ustr, u"inmargins"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool    SwAddPrinterTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    if ( m_bAttrModified )
    {
        SwAddPrinterItem aAddPrinterAttr;
        aAddPrinterAttr.m_bPrintGraphic   = m_xGrfCB->get_active();
        aAddPrinterAttr.m_bPrintControl   = m_xCtrlFieldCB->get_active();
        aAddPrinterAttr.m_bPrintPageBackground = m_xBackgroundCB->get_active();
        aAddPrinterAttr.m_bPrintBlackFont = m_xBlackFontCB->get_active();
        aAddPrinterAttr.m_bPrintHiddenText = m_xPrintHiddenTextCB->get_active();
        aAddPrinterAttr.m_bPrintTextPlaceholder = m_xPrintTextPlaceholderCB->get_active();

        aAddPrinterAttr.m_bPrintLeftPages     = m_xLeftPageCB->get_active();
        aAddPrinterAttr.m_bPrintRightPages    = m_xRightPageCB->get_active();
        aAddPrinterAttr.m_bPrintProspect      = m_xProspectCB->get_active();
        aAddPrinterAttr.m_bPrintProspectRTL   = m_xProspectCB_RTL->get_active();
        aAddPrinterAttr.m_bPaperFromSetup     = m_xPaperFromSetupCB->get_active();
        aAddPrinterAttr.m_bPrintEmptyPages    = m_xPrintEmptyPagesCB->get_active();

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
        aAddPrinterAttr.m_sFaxName = m_sNone == sFax ? OUString() : sFax;
        rCoreSet->Put(aAddPrinterAttr);
    }
    return m_bAttrModified;
}

void    SwAddPrinterTabPage::Reset( const SfxItemSet*  )
{
    const   SfxItemSet&         rSet = GetItemSet();
    bool bReadOnly = false;

    if( const SwAddPrinterItem* pAddPrinterAttr = rSet.GetItemIfSet( FN_PARAM_ADDPRINTER , false ) )
    {
        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Graphic::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Graphic::isReadOnly();
        m_xGrfCB->set_active(pAddPrinterAttr->m_bPrintGraphic);
        m_xGrfCB->set_sensitive(!bReadOnly);
        m_xGrfImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Control::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Control::isReadOnly();
        m_xCtrlFieldCB->set_active(       pAddPrinterAttr->m_bPrintControl);
        m_xCtrlFieldCB->set_sensitive(!bReadOnly);
        m_xCtrlFieldImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Background::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Background::isReadOnly();
        m_xBackgroundCB->set_active(    pAddPrinterAttr->m_bPrintPageBackground);
        m_xBackgroundCB->set_sensitive(!bReadOnly);
        m_xBackgroundImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::PrintBlack::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::PrintBlack::isReadOnly();
        m_xBlackFontCB->set_active(     pAddPrinterAttr->m_bPrintBlackFont);
        m_xBlackFontCB->set_sensitive(!bReadOnly);
        m_xBlackFontImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Content::PrintHiddenText::isReadOnly();
        m_xPrintHiddenTextCB->set_active( pAddPrinterAttr->m_bPrintHiddenText);
        m_xPrintHiddenTextCB->set_sensitive(!bReadOnly);
        m_xPrintHiddenTextImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Content::PrintPlaceholders::isReadOnly();
        m_xPrintTextPlaceholderCB->set_active(pAddPrinterAttr->m_bPrintTextPlaceholder);
        m_xPrintTextPlaceholderCB->set_sensitive(!bReadOnly);
        m_xPrintTextPlaceholderImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Page::LeftPage::isReadOnly();
        m_xLeftPageCB->set_active(      pAddPrinterAttr->m_bPrintLeftPages);
        m_xLeftPageCB->set_sensitive(!bReadOnly);
        m_xLeftPageImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::Page::RightPage::isReadOnly();
        m_xRightPageCB->set_active(     pAddPrinterAttr->m_bPrintRightPages);
        m_xRightPageCB->set_sensitive(!bReadOnly);
        m_xRightPageImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Papertray::FromPrinterSetup::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Papertray::FromPrinterSetup::isReadOnly();
        m_xPaperFromSetupCB->set_active(pAddPrinterAttr->m_bPaperFromSetup);
        m_xPaperFromSetupCB->set_sensitive(!bReadOnly);
        m_xPaperFromSetupImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Print::EmptyPages::isReadOnly();
        m_xPrintEmptyPagesCB->set_active(pAddPrinterAttr->m_bPrintEmptyPages);
        m_xPrintEmptyPagesCB->set_sensitive(!bReadOnly);
        m_xPrintEmptyPagesImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Page::Brochure::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Page::Brochure::isReadOnly();
        m_xProspectCB->set_active(      pAddPrinterAttr->m_bPrintProspect);
        m_xProspectCB->set_sensitive(!bReadOnly);
        m_xProspectImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Page::BrochureRightToLeft::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Page::BrochureRightToLeft::isReadOnly();
        m_xProspectCB_RTL->set_active(      pAddPrinterAttr->m_bPrintProspectRTL);
        m_xProspectCB_RTL->set_sensitive(!bReadOnly);
        m_xProspectImg_RTL->set_visible(bReadOnly);

        m_xNoRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::NONE ) ;
        m_xOnlyRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::Only ) ;
        m_xEndRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndDoc ) ;
        m_xEndPageRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::EndPage ) ;
        m_xInMarginsRB->set_active(pAddPrinterAttr->m_nPrintPostIts== SwPostItMode::InMargins ) ;

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Content::Note::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Content::Note::isReadOnly();
        m_xNoRB->set_sensitive(!bReadOnly);
        m_xOnlyRB->set_sensitive(!bReadOnly);
        m_xEndRB->set_sensitive(!bReadOnly);
        m_xEndPageRB->set_sensitive(!bReadOnly);
        m_xInMarginsRB->set_sensitive(!bReadOnly);
        m_xMarginsImg->set_visible(bReadOnly);

        auto nFound = m_xFaxLB->find_text(pAddPrinterAttr->m_sFaxName);
        if (nFound != -1)
            m_xFaxLB->set_active(nFound);
        else if (m_xFaxLB->get_count())
            m_xFaxLB->set_active(0);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Output::Fax::isReadOnly() :
            officecfg::Office::WriterWeb::Print::Output::Fax::isReadOnly();
        m_xFaxLB->set_sensitive(!bReadOnly);
        m_xFaxImg->set_visible(bReadOnly);
    }
    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Print::Page::BrochureRightToLeft::isReadOnly() :
        officecfg::Office::WriterWeb::Print::Page::BrochureRightToLeft::isReadOnly();
    if (m_xProspectCB->get_active())
    {
        m_xProspectCB_RTL->set_sensitive(!bReadOnly);
        m_xNoRB->set_sensitive( false );
        m_xOnlyRB->set_sensitive( false );
        m_xEndRB->set_sensitive( false );
        m_xEndPageRB->set_sensitive( false );
    }
    else
        m_xProspectCB_RTL->set_sensitive( false );

    m_xProspectImg_RTL->set_visible(bReadOnly);
}

IMPL_LINK_NOARG(SwAddPrinterTabPage, AutoClickHdl, weld::Toggleable&, void)
{
    m_bAttrModified = true;
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
    m_xFaxLB->append_text(m_sNone);
    for(const auto & i : rFaxLst)
    {
        m_xFaxLB->append_text(i);
    }
    m_xFaxLB->set_active(0);
}

IMPL_LINK_NOARG(SwAddPrinterTabPage, SelectHdl, weld::ComboBox&, void)
{
    m_bAttrModified=true;
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
SwStdFontTabPage::SwStdFontTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/optfonttabpage.ui"_ustr, u"OptFontTabPage"_ustr, &rSet)
    , m_pPrt(nullptr)
    , m_pFontConfig(nullptr)
    , m_pWrtShell(nullptr)
    , m_eLanguage( GetAppLanguage() )
    , m_bListDefault(false)
    , m_bSetListDefault(true)
    , m_bLabelDefault(false)
    , m_bSetLabelDefault(true)
    , m_bIdxDefault(false)
    , m_bSetIdxDefault(true)
    , m_bDisposePrinter(false)
    , m_nFontGroup(FONT_GROUP_DEFAULT)
    , m_sScriptWestern(SwResId(ST_SCRIPT_WESTERN))
    , m_sScriptAsian(SwResId(ST_SCRIPT_ASIAN))
    , m_sScriptComplex(SwResId(ST_SCRIPT_CTL))
    , m_xLabelFT(m_xBuilder->weld_label(u"label1"_ustr))
    , m_xStandardBox(m_xBuilder->weld_combo_box(u"standardbox"_ustr))
    , m_xStandardBoxImg(m_xBuilder->weld_widget(u"lockstandardbox"_ustr))
    , m_xStandardHeightLB(new FontSizeBox(m_xBuilder->weld_combo_box(u"standardheight"_ustr)))
    , m_xStandardHeightImg(m_xBuilder->weld_widget(u"lockstandardheight"_ustr))
    , m_xTitleBox(m_xBuilder->weld_combo_box(u"titlebox"_ustr))
    , m_xTitleBoxImg(m_xBuilder->weld_widget(u"locktitlebox"_ustr))
    , m_xTitleHeightLB(new FontSizeBox(m_xBuilder->weld_combo_box(u"titleheight"_ustr)))
    , m_xTitleHeightImg(m_xBuilder->weld_widget(u"locktitleheight"_ustr))
    , m_xListBox(m_xBuilder->weld_combo_box(u"listbox"_ustr))
    , m_xListBoxImg(m_xBuilder->weld_widget(u"locklistbox"_ustr))
    , m_xListHeightLB(new FontSizeBox(m_xBuilder->weld_combo_box(u"listheight"_ustr)))
    , m_xListHeightImg(m_xBuilder->weld_widget(u"locklistheight"_ustr))
    , m_xLabelBox(m_xBuilder->weld_combo_box(u"labelbox"_ustr))
    , m_xLabelBoxImg(m_xBuilder->weld_widget(u"locklabelbox"_ustr))
    , m_xLabelHeightLB(new FontSizeBox(m_xBuilder->weld_combo_box(u"labelheight"_ustr)))
    , m_xLabelHeightImg(m_xBuilder->weld_widget(u"locklabelheight"_ustr))
    , m_xIdxBox(m_xBuilder->weld_combo_box(u"idxbox"_ustr))
    , m_xIdxBoxImg(m_xBuilder->weld_widget(u"lockidxbox"_ustr))
    , m_xIndexHeightLB(new FontSizeBox(m_xBuilder->weld_combo_box(u"indexheight"_ustr)))
    , m_xIndexHeightImg(m_xBuilder->weld_widget(u"lockindexheight"_ustr))
    , m_xStandardPB(m_xBuilder->weld_button(u"standard"_ustr))
{
    m_xStandardBox->make_sorted();
    m_xTitleBox->make_sorted();
    m_xListBox->make_sorted();
    m_xLabelBox->make_sorted();
    m_xIdxBox->make_sorted();

    m_xStandardPB->connect_clicked(LINK(this, SwStdFontTabPage, StandardHdl));
    m_xStandardBox->connect_changed( LINK(this, SwStdFontTabPage, ModifyHdl));
    m_xListBox->connect_changed( LINK(this, SwStdFontTabPage, ModifyHdl));
    m_xLabelBox->connect_changed( LINK(this, SwStdFontTabPage, ModifyHdl));
    m_xIdxBox->connect_changed( LINK(this, SwStdFontTabPage, ModifyHdl));
    Link<weld::Widget&,void> aFocusLink = LINK( this, SwStdFontTabPage, LoseFocusHdl);
    m_xStandardBox->connect_focus_out( aFocusLink );
    m_xTitleBox->connect_focus_out( aFocusLink );
    m_xListBox->connect_focus_out( aFocusLink );
    m_xLabelBox->connect_focus_out( aFocusLink );
    m_xIdxBox->connect_focus_out( aFocusLink );
}

SwStdFontTabPage::~SwStdFontTabPage()
{
    m_xIndexHeightLB.reset();
    m_xLabelHeightLB.reset();
    m_xListHeightLB.reset();
    m_xTitleHeightLB.reset();
    m_xStandardHeightLB.reset();
    m_pFontList.reset();
    if (m_bDisposePrinter)
        m_pPrt.disposeAndClear();
    else
        m_pPrt.clear();
}

std::unique_ptr<SfxTabPage> SwStdFontTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                             const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwStdFontTabPage>(pPage, pController, *rAttrSet);
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

OUString SwStdFontTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr,        u"font_label"_ustr, u"size_label"_ustr,    u"default_label"_ustr,
                          u"heading_label"_ustr, u"list_label"_ustr, u"caption_label"_ustr, u"index_label"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xStandardPB->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SwStdFontTabPage::FillItemSet( SfxItemSet* )
{
    SW_MOD()->GetModuleConfig()->SetDefaultFontInCurrDocOnly(false);

    const OUString sStandard    = m_xStandardBox->get_active_text();
    const OUString sTitle       = m_xTitleBox->get_active_text();
    const OUString sList        = m_xListBox->get_active_text();
    const OUString sLabel       = m_xLabelBox->get_active_text();
    const OUString sIdx         = m_xIdxBox->get_active_text();

    bool bStandardHeightChanged = m_xStandardHeightLB->get_value_changed_from_saved();
    bool bTitleHeightChanged = m_xTitleHeightLB->get_value_changed_from_saved();
    bool bListHeightChanged = m_xListHeightLB->get_value_changed_from_saved();
    bool bLabelHeightChanged = m_xLabelHeightLB->get_value_changed_from_saved();
    bool bIndexHeightChanged = m_xIndexHeightLB->get_value_changed_from_saved();

    m_pFontConfig->SetFontStandard(sStandard, m_nFontGroup);
    m_pFontConfig->SetFontOutline(sTitle, m_nFontGroup);
    m_pFontConfig->SetFontList(sList, m_nFontGroup);
    m_pFontConfig->SetFontCaption(sLabel, m_nFontGroup);
    m_pFontConfig->SetFontIndex(sIdx, m_nFontGroup);
    if(bStandardHeightChanged)
    {
        float fSize = static_cast<float>(m_xStandardHeightLB->get_value()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_STANDARD, m_nFontGroup );
    }
    if(bTitleHeightChanged)
    {
        float fSize = static_cast<float>(m_xTitleHeightLB->get_value()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_OUTLINE, m_nFontGroup );
    }
    if(bListHeightChanged)
    {
        float fSize = static_cast<float>(m_xListHeightLB->get_value()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_LIST, m_nFontGroup );
    }
    if(bLabelHeightChanged)
    {
        float fSize = static_cast<float>(m_xLabelHeightLB->get_value()) / 10;
        m_pFontConfig->SetFontHeight( CalcToUnit( fSize, MapUnit::MapTwip ), FONT_CAPTION, m_nFontGroup );
    }
    if(bIndexHeightChanged)
    {
        float fSize = static_cast<float>(m_xIndexHeightLB->get_value()) / 10;
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
            float fSize = static_cast<float>(m_xStandardHeightLB->get_value()) / 10;
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
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_xTitleHeightLB->get_value()), nFontHeightWhich);
            bMod = true;
        }
        if(sList != m_sShellList && (!m_bListDefault || !m_bSetListDefault ))
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_NUMBER_BULLET_BASE, pPrinter, sList, nFontWhich);
            bMod = true;
        }
        if(bListHeightChanged)
        {
            lcl_SetColl(m_pWrtShell, RES_POOLCOLL_NUMBER_BULLET_BASE,
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_xListHeightLB->get_value()), nFontHeightWhich);
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
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_xLabelHeightLB->get_value()), nFontHeightWhich);
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
                sal::static_int_cast< sal_uInt16, sal_Int64 >(m_xIndexHeightLB->get_value()), nFontHeightWhich);
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
    const TypedWhichId<SvxLanguageItem> nLangSlot = m_nFontGroup == FONT_GROUP_DEFAULT  ? SID_ATTR_LANGUAGE :
        FONT_GROUP_CJK == m_nFontGroup ? SID_ATTR_CHAR_CJK_LANGUAGE : SID_ATTR_CHAR_CTL_LANGUAGE;

    if( const SvxLanguageItem* pLang = rSet->GetItemIfSet(nLangSlot, false) )
        m_eLanguage = pLang->GetValue();

    OUString sToReplace = m_sScriptWestern;
    if(FONT_GROUP_CJK == m_nFontGroup )
        sToReplace = m_sScriptAsian;
    else if(FONT_GROUP_CTL == m_nFontGroup )
        sToReplace = m_sScriptComplex;
    m_xLabelFT->set_label(m_xLabelFT->get_label().replaceFirst("%1", sToReplace));

    if (m_bDisposePrinter)
    {
        m_pPrt.disposeAndClear();
        m_bDisposePrinter = false;
    }

    if(const SwPtrItem* pItem = rSet->GetItemIfSet(FN_PARAM_PRINTER, false))
    {
        m_pPrt = static_cast<SfxPrinter*>(pItem->GetValue());
    }
    else
    {
        auto pPrinterSet = std::make_unique<SfxItemSetFixed
                    <SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC>>( *rSet->GetPool() );
        m_pPrt = VclPtr<SfxPrinter>::Create(std::move(pPrinterSet));
        m_bDisposePrinter = true;
    }
    m_pFontList.reset(new FontList( m_pPrt ));
    // #i94536# prevent duplication of font entries when 'reset' button is pressed
    if( !m_xStandardBox->get_count() )
    {
        // get the set of distinct available family names
        std::set< OUString > aFontNames;
        int nFontNames = m_pPrt->GetFontFaceCollectionCount();
        for( int i = 0; i < nFontNames; i++ )
        {
            FontMetric aFontMetric( m_pPrt->GetFontMetricFromCollection( i ) );
            aFontNames.insert( aFontMetric.GetFamilyName() );
        }

        // insert to listboxes
        for( const auto& rFontName : aFontNames )
        {
            m_xStandardBox->append_text( rFontName );
            m_xTitleBox->append_text( rFontName );
            m_xListBox->append_text( rFontName );
            m_xLabelBox->append_text( rFontName );
            m_xIdxBox->append_text( rFontName );
        }
    }
    if(const SwPtrItem* pItem = rSet->GetItemIfSet(FN_PARAM_STDFONTS, false))
    {
         m_pFontConfig = static_cast<SwStdFontConfig*>(pItem->GetValue());
    }

    if(const SwPtrItem* pItem = rSet->GetItemIfSet(FN_PARAM_WRTSHELL, false))
    {
        m_pWrtShell = static_cast<SwWrtShell*>(pItem->GetValue());
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

        const TypedWhichId<SvxFontHeightItem> nFontHeightWhich =
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONTSIZE :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONTSIZE : RES_CHRATR_CTL_FONTSIZE;
        const SvxFontHeightItem& rFontHeightStandard = pColl->GetFormatAttr(nFontHeightWhich);
        nStandardHeight = static_cast<sal_Int32>(rFontHeightStandard.GetHeight());

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_HEADLINE_BASE);
        const SvxFontItem& rFontHL = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellTitle = sOutBackup = rFontHL.GetFamilyName();

        const SvxFontHeightItem& rFontHeightTitle = pColl->GetFormatAttr( nFontHeightWhich );
        nTitleHeight = static_cast<sal_Int32>(rFontHeightTitle.GetHeight());

        const sal_uInt16 nFontWhich =
            m_nFontGroup == FONT_GROUP_DEFAULT  ? RES_CHRATR_FONT :
            FONT_GROUP_CJK == m_nFontGroup ? RES_CHRATR_CJK_FONT : RES_CHRATR_CTL_FONT;
        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_NUMBER_BULLET_BASE);
        const SvxFontItem& rFontLS = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_bListDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        m_sShellList = sListBackup = rFontLS.GetFamilyName();

        const SvxFontHeightItem& rFontHeightList = pColl->GetFormatAttr(nFontHeightWhich);
        nListHeight = static_cast<sal_Int32>(rFontHeightList.GetHeight());

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_LABEL);
        m_bLabelDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontCP = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellLabel = sCapBackup = rFontCP.GetFamilyName();
        const SvxFontHeightItem& rFontHeightLabel = pColl->GetFormatAttr(nFontHeightWhich);
        nLabelHeight = static_cast<sal_Int32>(rFontHeightLabel.GetHeight());

        pColl = m_pWrtShell->GetTextCollFromPool(RES_POOLCOLL_REGISTER_BASE);
        m_bIdxDefault = SfxItemState::DEFAULT == pColl->GetAttrSet().GetItemState(nFontWhich, false);
        const SvxFontItem& rFontIDX = !m_nFontGroup ? pColl->GetFont() :
                FONT_GROUP_CJK == m_nFontGroup ? pColl->GetCJKFont() : pColl->GetCTLFont();
        m_sShellIndex = sIdxBackup = rFontIDX.GetFamilyName();
        const SvxFontHeightItem& rFontHeightIndex = pColl->GetFormatAttr(nFontHeightWhich);
        nIndexHeight = static_cast<sal_Int32>(rFontHeightIndex.GetHeight());
    }
    m_xStandardBox->set_entry_text(sStdBackup );
    m_xTitleBox->set_entry_text(sOutBackup );
    m_xListBox->set_entry_text(sListBackup);
    m_xLabelBox->set_entry_text(sCapBackup );
    m_xIdxBox->set_entry_text(sIdxBackup );

    m_xStandardHeightLB->Fill( m_pFontList.get() );
    m_xTitleHeightLB->Fill( m_pFontList.get() );
    m_xListHeightLB->Fill( m_pFontList.get() );
    m_xLabelHeightLB->Fill( m_pFontList.get() );
    m_xIndexHeightLB->Fill( m_pFontList.get() );

    m_xStandardHeightLB->set_value( CalcToPoint( nStandardHeight, MapUnit::MapTwip, 10 ) );
    m_xTitleHeightLB->set_value( CalcToPoint( nTitleHeight   , MapUnit::MapTwip, 10 ) );
    m_xListHeightLB->set_value( CalcToPoint( nListHeight    , MapUnit::MapTwip, 10 ) );
    m_xLabelHeightLB->set_value( CalcToPoint( nLabelHeight   , MapUnit::MapTwip, 10 ));
    m_xIndexHeightLB->set_value( CalcToPoint( nIndexHeight   , MapUnit::MapTwip, 10 ));


    if (m_nFontGroup == FONT_GROUP_DEFAULT)
    {
        bool bReadonly = officecfg::Office::Writer::DefaultFont::Standard::isReadOnly();
        m_xStandardBox->set_sensitive(!bReadonly);
        m_xStandardBoxImg->set_visible(bReadonly);
        bReadonly = officecfg::Office::Writer::DefaultFont::StandardHeight::isReadOnly();
        m_xStandardHeightLB->set_sensitive(!bReadonly);
        m_xStandardHeightImg->set_visible(bReadonly);

        bReadonly = officecfg::Office::Writer::DefaultFont::Heading::isReadOnly();
        m_xTitleBox->set_sensitive(!bReadonly);
        m_xTitleBoxImg->set_visible(bReadonly);
        bReadonly = officecfg::Office::Writer::DefaultFont::HeadingHeight::isReadOnly();
        m_xTitleHeightLB->set_sensitive(!bReadonly);
        m_xTitleHeightImg->set_visible(bReadonly);

        bReadonly = officecfg::Office::Writer::DefaultFont::List::isReadOnly();
        m_xListBox->set_sensitive(!bReadonly);
        m_xListBoxImg->set_visible(bReadonly);
        bReadonly = officecfg::Office::Writer::DefaultFont::ListHeight::isReadOnly();
        m_xListHeightLB->set_sensitive(!bReadonly);
        m_xListHeightImg->set_visible(bReadonly);

        bReadonly = officecfg::Office::Writer::DefaultFont::Caption::isReadOnly();
        m_xLabelBox->set_sensitive(!bReadonly);
        m_xLabelBoxImg->set_visible(bReadonly);
        bReadonly = officecfg::Office::Writer::DefaultFont::CaptionHeight::isReadOnly();
        m_xLabelHeightLB->set_sensitive(!bReadonly);
        m_xLabelHeightImg->set_visible(bReadonly);

        bReadonly = officecfg::Office::Writer::DefaultFont::Index::isReadOnly();
        m_xIdxBox->set_sensitive(!bReadonly);
        m_xIdxBoxImg->set_visible(bReadonly);
        bReadonly = officecfg::Office::Writer::DefaultFont::IndexHeight::isReadOnly();
        m_xIndexHeightLB->set_sensitive(!bReadonly);
        m_xIndexHeightImg->set_visible(bReadonly);
    }

    m_xStandardBox->save_value();
    m_xTitleBox->save_value();
    m_xListBox->save_value();
    m_xLabelBox->save_value();
    m_xIdxBox->save_value();

    m_xStandardHeightLB->save_value();
    m_xTitleHeightLB->save_value();
    m_xListHeightLB->save_value();
    m_xLabelHeightLB->save_value();
    m_xIndexHeightLB->save_value();
}

IMPL_LINK_NOARG(SwStdFontTabPage, StandardHdl, weld::Button&, void)
{
    sal_uInt8 nFontOffset = m_nFontGroup * FONT_PER_GROUP;
    m_xStandardBox->set_entry_text(SwStdFontConfig::GetDefaultFor(FONT_STANDARD + nFontOffset, m_eLanguage));
    m_xTitleBox->set_entry_text(SwStdFontConfig::GetDefaultFor(FONT_OUTLINE  + nFontOffset, m_eLanguage));
    m_xListBox->set_entry_text(SwStdFontConfig::GetDefaultFor(FONT_LIST     + nFontOffset, m_eLanguage));
    m_xLabelBox->set_entry_text(SwStdFontConfig::GetDefaultFor(FONT_CAPTION  + nFontOffset, m_eLanguage));
    m_xIdxBox->set_entry_text(SwStdFontConfig::GetDefaultFor(FONT_INDEX    + nFontOffset, m_eLanguage));

    m_xStandardBox->save_value();
    m_xTitleBox->save_value();
    m_xListBox->save_value();
    m_xLabelBox->save_value();
    m_xIdxBox->save_value();

    m_xStandardHeightLB->set_value( CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_STANDARD + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
    m_xTitleHeightLB->set_value(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_OUTLINE  +
            nFontOffset, m_eLanguage), MapUnit::MapTwip, 10 ));
    m_xListHeightLB->set_value(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_LIST + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
    m_xLabelHeightLB->set_value(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_CAPTION  + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
    m_xIndexHeightLB->set_value(CalcToPoint(
        SwStdFontConfig::GetDefaultHeightFor(FONT_INDEX    + nFontOffset, m_eLanguage),
            MapUnit::MapTwip, 10 ));
}

IMPL_LINK( SwStdFontTabPage, ModifyHdl, weld::ComboBox&, rBox, void )
{
    if (&rBox == m_xStandardBox.get())
    {
        const OUString sEntry = rBox.get_active_text();
        if(m_bSetListDefault && m_bListDefault)
            m_xListBox->set_entry_text(sEntry);
        if(m_bSetLabelDefault && m_bLabelDefault)
            m_xLabelBox->set_entry_text(sEntry);
        if(m_bSetIdxDefault && m_bIdxDefault)
            m_xIdxBox->set_entry_text(sEntry);
    }
    else if (&rBox == m_xListBox.get())
    {
        m_bSetListDefault = false;
    }
    else if (&rBox == m_xLabelBox.get())
    {
        m_bSetLabelDefault = false;
    }
    else if (&rBox == m_xIdxBox.get())
    {
        m_bSetIdxDefault = false;
    }
}

IMPL_LINK( SwStdFontTabPage, LoseFocusHdl, weld::Widget&, rControl, void )
{
    weld::ComboBox& rBox = dynamic_cast<weld::ComboBox&>(rControl);
    FontSizeBox* pHeightLB = nullptr;

    if (&rBox == m_xStandardBox.get())
    {
        pHeightLB = m_xStandardHeightLB.get();
    }
    else if (&rBox == m_xTitleBox.get())
    {
        pHeightLB = m_xTitleHeightLB.get();
    }
    else if (&rBox == m_xListBox.get())
    {
        pHeightLB = m_xListHeightLB.get();
    }
    else if (&rBox == m_xLabelBox.get())
    {
        pHeightLB = m_xLabelHeightLB.get();
    }
    else /*if (&rBox == m_xIndexHeightLB.get())*/
    {
        pHeightLB = m_xIndexHeightLB.get();
    }

    pHeightLB->Fill( m_pFontList.get() );
}

void SwStdFontTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SfxUInt16Item* pFlagItem = aSet.GetItem<SfxUInt16Item>(SID_FONTMODE_TYPE, false);
    if (pFlagItem)
        m_nFontGroup = sal::static_int_cast< sal_uInt8, sal_uInt16>( pFlagItem->GetValue() );
}

SwTableOptionsTabPage::SwTableOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/opttablepage.ui"_ustr, u"OptTablePage"_ustr, &rSet)
    , m_pWrtShell(nullptr)
    , m_bHTMLMode(false)
    , m_xHeaderCB(m_xBuilder->weld_check_button(u"header"_ustr))
    , m_xHeaderImg(m_xBuilder->weld_widget(u"lockheader"_ustr))
    , m_xRepeatHeaderCB(m_xBuilder->weld_check_button(u"repeatheader"_ustr))
    , m_xRepeatHeaderImg(m_xBuilder->weld_widget(u"lockrepeatheader"_ustr))
    , m_xDontSplitCB(m_xBuilder->weld_check_button(u"dontsplit"_ustr))
    , m_xDontSplitImg(m_xBuilder->weld_widget(u"lockdontsplit"_ustr))
    , m_xBorderCB(m_xBuilder->weld_check_button(u"border"_ustr))
    , m_xBorderImg(m_xBuilder->weld_widget(u"lockborder"_ustr))
    , m_xNumFormattingCB(m_xBuilder->weld_check_button(u"numformatting"_ustr))
    , m_xNumFormattingImg(m_xBuilder->weld_widget(u"locknumformatting"_ustr))
    , m_xNumFormatFormattingCB(m_xBuilder->weld_check_button(u"numfmtformatting"_ustr))
    , m_xNumFormatFormattingImg(m_xBuilder->weld_widget(u"locknumfmtformatting"_ustr))
    , m_xNumAlignmentCB(m_xBuilder->weld_check_button(u"numalignment"_ustr))
    , m_xNumAlignmentImg(m_xBuilder->weld_widget(u"locknumalignment"_ustr))
    , m_xRowMoveMF(m_xBuilder->weld_metric_spin_button(u"rowmove"_ustr, FieldUnit::CM))
    , m_xRowMoveImg(m_xBuilder->weld_widget(u"lockrowmove"_ustr))
    , m_xColMoveMF(m_xBuilder->weld_metric_spin_button(u"colmove"_ustr, FieldUnit::CM))
    , m_xColMoveImg(m_xBuilder->weld_widget(u"lockcolmove"_ustr))
    , m_xRowInsertMF(m_xBuilder->weld_metric_spin_button(u"rowinsert"_ustr, FieldUnit::CM))
    , m_xRowInsertImg(m_xBuilder->weld_widget(u"lockrowinsert"_ustr))
    , m_xColInsertMF(m_xBuilder->weld_metric_spin_button(u"colinsert"_ustr, FieldUnit::CM))
    , m_xColInsertImg(m_xBuilder->weld_widget(u"lockcolinsert"_ustr))
    , m_xFixRB(m_xBuilder->weld_radio_button(u"fix"_ustr))
    , m_xFixPropRB(m_xBuilder->weld_radio_button(u"fixprop"_ustr))
    , m_xVarRB(m_xBuilder->weld_radio_button(u"var"_ustr))
    , m_xBehaviorOfImg(m_xBuilder->weld_widget(u"lockbehaviorof"_ustr))
{
    Link<weld::Toggleable&,void> aLnk(LINK(this, SwTableOptionsTabPage, CheckBoxHdl));
    m_xNumFormattingCB->connect_toggled(aLnk);
    m_xNumFormatFormattingCB->connect_toggled(aLnk);
    m_xHeaderCB->connect_toggled(aLnk);
}

SwTableOptionsTabPage::~SwTableOptionsTabPage()
{
}

std::unique_ptr<SfxTabPage> SwTableOptionsTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                                  const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwTableOptionsTabPage>(pPage, pController, *rAttrSet);
}

OUString SwTableOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { u"label1"_ustr,  u"label3"_ustr,  u"label4"_ustr,  u"label5"_ustr,  u"label6"_ustr,  u"label10"_ustr, u"label2"_ustr,
            u"label14"_ustr, u"label15"_ustr, u"label16"_ustr, u"label11"_ustr, u"label12"_ustr, u"label13"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"header"_ustr,        u"repeatheader"_ustr,     u"dontsplit"_ustr,   u"border"_ustr,
                               u"numformatting"_ustr, u"numfmtformatting"_ustr, u"numalignment"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { u"fix"_ustr, u"fixprop"_ustr, u"var"_ustr };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SwTableOptionsTabPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    if (m_xRowMoveMF->get_value_changed_from_saved())
        pModOpt->SetTableHMove( o3tl::narrowing<sal_uInt16>(m_xRowMoveMF->denormalize( m_xRowMoveMF->get_value(FieldUnit::TWIP))));

    if (m_xColMoveMF->get_value_changed_from_saved())
        pModOpt->SetTableVMove( o3tl::narrowing<sal_uInt16>(m_xColMoveMF->denormalize( m_xColMoveMF->get_value(FieldUnit::TWIP))));

    if (m_xRowInsertMF->get_value_changed_from_saved())
        pModOpt->SetTableHInsert(o3tl::narrowing<sal_uInt16>(m_xRowInsertMF->denormalize( m_xRowInsertMF->get_value(FieldUnit::TWIP))));

    if (m_xColInsertMF->get_value_changed_from_saved())
        pModOpt->SetTableVInsert(o3tl::narrowing<sal_uInt16>(m_xColInsertMF->denormalize( m_xColInsertMF->get_value(FieldUnit::TWIP))));

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
            m_pWrtShell->GetView().GetViewFrame().GetBindings().Invalidate( aInva );
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
    if (const SfxUInt16Item* pItem = rSet->GetItemIfSet(SID_HTML_MODE, false))
    {
        m_bHTMLMode = 0 != (pItem->GetValue() & HTMLMODE_ON);
    }

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

    bool bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Shift::Row::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Shift::Row::isReadOnly();
    m_xRowMoveMF->set_value(m_xRowMoveMF->normalize(pModOpt->GetTableHMove()), FieldUnit::TWIP);
    m_xRowMoveMF->set_sensitive(!bReadOnly);
    m_xRowMoveImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Shift::Column::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Shift::Column::isReadOnly();
    m_xColMoveMF->set_value(m_xColMoveMF->normalize(pModOpt->GetTableVMove()), FieldUnit::TWIP);
    m_xColMoveMF->set_sensitive(!bReadOnly);
    m_xColMoveImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Insert::Row::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Insert::Row::isReadOnly();
    m_xRowInsertMF->set_value(m_xRowInsertMF->normalize(pModOpt->GetTableHInsert()), FieldUnit::TWIP);
    m_xRowInsertMF->set_sensitive(!bReadOnly);
    m_xRowInsertImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Insert::Column::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Insert::Column::isReadOnly();
    m_xColInsertMF->set_value(m_xColInsertMF->normalize(pModOpt->GetTableVInsert()), FieldUnit::TWIP);
    m_xColInsertMF->set_sensitive(!bReadOnly);
    m_xColInsertImg->set_visible(bReadOnly);

    switch(pModOpt->GetTableMode())
    {
        case TableChgMode::FixedWidthChangeAbs:   m_xFixRB->set_active(true);     break;
        case TableChgMode::FixedWidthChangeProp:  m_xFixPropRB->set_active(true); break;
        case TableChgMode::VarWidthChangeAbs:     m_xVarRB->set_active(true); break;
    }

    // hide certain controls for html
    if (m_bHTMLMode)
    {
        m_xRepeatHeaderCB->hide();
        m_xDontSplitCB->hide();
    }

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Change::Effect::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Change::Effect::isReadOnly();
    if (bReadOnly)
    {
        m_xFixRB->set_sensitive(false);
        m_xFixPropRB->set_sensitive(false);
        m_xVarRB->set_sensitive(false);
        m_xBehaviorOfImg->set_visible(true);
    }

    SwInsertTableOptions aInsOpts = pModOpt->GetInsTableFlags(m_bHTMLMode);
    const SwInsertTableFlags nInsTableFlags = aInsOpts.mnInsMode;

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Insert::Table::Header::isReadOnly() :
        officecfg::Office::WriterWeb::Insert::Table::Header::isReadOnly();
    m_xHeaderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::Headline));
    m_xHeaderCB->set_sensitive(!bReadOnly);
    m_xHeaderImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Writer::Insert::Table::RepeatHeader::isReadOnly();
    m_xRepeatHeaderCB->set_active((!m_bHTMLMode) && (aInsOpts.mnRowsToRepeat > 0));
    m_xRepeatHeaderCB->set_sensitive(!bReadOnly);
    m_xRepeatHeaderImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Writer::Insert::Table::Split::isReadOnly();
    m_xDontSplitCB->set_active(!(nInsTableFlags & SwInsertTableFlags::SplitLayout));
    m_xDontSplitCB->set_sensitive(!bReadOnly);
    m_xDontSplitImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Insert::Table::Border::isReadOnly() :
        officecfg::Office::WriterWeb::Insert::Table::Border::isReadOnly();
    m_xBorderCB->set_active(bool(nInsTableFlags & SwInsertTableFlags::DefaultBorder));
    m_xBorderCB->set_sensitive(!bReadOnly);
    m_xBorderImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Input::NumberRecognition::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Input::NumberRecognition::isReadOnly();
    m_xNumFormattingCB->set_active(pModOpt->IsInsTableFormatNum(m_bHTMLMode));
    m_xNumFormattingCB->set_sensitive(!bReadOnly);
    m_xNumFormattingImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Input::NumberFormatRecognition::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Input::NumberFormatRecognition::isReadOnly();
    m_xNumFormatFormattingCB->set_active(pModOpt->IsInsTableChangeNumFormat(m_bHTMLMode));
    m_xNumFormatFormattingCB->set_sensitive(!bReadOnly);
    m_xNumFormatFormattingImg->set_visible(bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Input::Alignment::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Input::Alignment::isReadOnly();
    m_xNumAlignmentCB->set_active(pModOpt->IsInsTableAlignNum(m_bHTMLMode));
    m_xNumAlignmentCB->set_sensitive(!bReadOnly);
    m_xNumAlignmentImg->set_visible(bReadOnly);

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

IMPL_LINK_NOARG(SwTableOptionsTabPage, CheckBoxHdl, weld::Toggleable&, void)
{
    bool bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Input::NumberFormatRecognition::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Input::NumberFormatRecognition::isReadOnly();
    m_xNumFormatFormattingCB->set_sensitive(m_xNumFormattingCB->get_active() && !bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Table::Input::Alignment::isReadOnly() :
        officecfg::Office::WriterWeb::Table::Input::Alignment::isReadOnly();
    m_xNumAlignmentCB->set_sensitive(m_xNumFormattingCB->get_active() && !bReadOnly);

    bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Insert::Table::RepeatHeader::isReadOnly() : false;
    m_xRepeatHeaderCB->set_sensitive(m_xHeaderCB->get_active() && !bReadOnly);
}

void SwTableOptionsTabPage::PageCreated( const SfxAllItemSet& aSet)
{
    const SwWrtShellItem* pWrtSh = aSet.GetItem<SwWrtShellItem>(SID_WRT_SHELL, false);
    if (pWrtSh)
        m_pWrtShell = pWrtSh->GetValue();
}

SwShdwCursorOptionsTabPage::SwShdwCursorOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/optformataidspage.ui"_ustr, u"OptFormatAidsPage"_ustr, &rSet)
    , m_pWrtShell(nullptr)
    , m_bHTMLMode(false)
    , m_xParaCB(m_xBuilder->weld_check_button(u"paragraph"_ustr))
    , m_xParaImg(m_xBuilder->weld_widget(u"lockparagraph"_ustr))
    , m_xSHyphCB(m_xBuilder->weld_check_button(u"hyphens"_ustr))
    , m_xSHyphImg(m_xBuilder->weld_widget(u"lockhyphens"_ustr))
    , m_xSpacesCB(m_xBuilder->weld_check_button(u"spaces"_ustr))
    , m_xSpacesImg(m_xBuilder->weld_widget(u"lockspaces"_ustr))
    , m_xHSpacesCB(m_xBuilder->weld_check_button(u"nonbreak"_ustr))
    , m_xHSpacesImg(m_xBuilder->weld_widget(u"locknonbreak"_ustr))
    , m_xTabCB(m_xBuilder->weld_check_button(u"tabs"_ustr))
    , m_xTabImg(m_xBuilder->weld_widget(u"locktabs"_ustr))
    , m_xTabLabel(m_xBuilder->weld_label(u"tabs_label"_ustr))
    , m_xBreakCB(m_xBuilder->weld_check_button(u"break"_ustr))
    , m_xBreakImg(m_xBuilder->weld_widget(u"lockbreak"_ustr))
    , m_xCharHiddenCB(m_xBuilder->weld_check_button(u"hiddentext"_ustr))
    , m_xCharHiddenImg(m_xBuilder->weld_widget(u"lockhiddentext"_ustr))
    , m_xBookmarkCB(m_xBuilder->weld_check_button(u"bookmarks"_ustr))
    , m_xBookmarkImg(m_xBuilder->weld_widget(u"lockbookmarks"_ustr))
    , m_xBookmarkLabel(m_xBuilder->weld_label(u"bookmarks_label"_ustr))
    , m_xDirectCursorFrame(m_xBuilder->weld_frame(u"directcrsrframe"_ustr))
    , m_xOnOffCB(m_xBuilder->weld_check_button(u"cursoronoff"_ustr))
    , m_xOnOffImg(m_xBuilder->weld_widget(u"lockcursoronoff"_ustr))
    , m_xDirectCursorFillMode(m_xBuilder->weld_combo_box(u"cxDirectCursorFillMode"_ustr))
    , m_xDirectCursorFillModeImg(m_xBuilder->weld_widget(u"lockfillmode"_ustr))
    , m_xCursorProtFrame(m_xBuilder->weld_frame(u"crsrprotframe"_ustr))
    , m_xImageFrame(m_xBuilder->weld_frame(u"frmImage"_ustr))
    , m_xCursorInProtCB(m_xBuilder->weld_check_button(u"cursorinprot"_ustr))
    , m_xCursorInProtImg(m_xBuilder->weld_widget(u"lockcursorinprot"_ustr))
    , m_xDefaultAnchorType(m_xBuilder->weld_combo_box(u"cxDefaultAnchor"_ustr))
    , m_xDefaultAnchorTypeImg(m_xBuilder->weld_widget(u"lockAnchor"_ustr))
    , m_xMathBaselineAlignmentCB(m_xBuilder->weld_check_button(u"mathbaseline"_ustr))
    , m_xMathBaselineAlignmentImg(m_xBuilder->weld_widget(u"lockmathbaseline"_ustr))
    , m_xFmtAidsAutoComplFrame(m_xBuilder->weld_frame(u"fmtaidsautocompleteframe"_ustr))
    , m_xEncloseWithCharactersCB(m_xBuilder->weld_check_button(u"enclosewithcharacters"_ustr))
    , m_xEncloseWithCharactersImg(m_xBuilder->weld_widget(u"lockenclosewithcharacters"_ustr))
{
    SwFillMode eMode = SwFillMode::Tab;
    bool bIsOn = false;

    if( const SwShadowCursorItem* pItem = rSet.GetItemIfSet( FN_PARAM_SHADOWCURSOR, false ))
    {
        eMode = pItem->GetMode();
        bIsOn = pItem->IsOn();
    }
    m_xOnOffCB->set_active( bIsOn );

    bool bIsEncloseWithCharactersOn = false;
    if (const SwFmtAidsAutoComplItem* pItem = rSet.GetItemIfSet(FN_PARAM_FMT_AIDS_AUTOCOMPL, false))
    {
        bIsEncloseWithCharactersOn = pItem->IsEncloseWithCharactersOn();
    }
    m_xEncloseWithCharactersCB->set_active(bIsEncloseWithCharactersOn);

    m_xDirectCursorFillMode->set_active( static_cast<int>(eMode) );
    const SfxUInt16Item* pHtmlModeItem = rSet.GetItemIfSet(SID_HTML_MODE, false);
    if(!pHtmlModeItem || !(pHtmlModeItem->GetValue() & HTMLMODE_ON))
        return;

    m_bHTMLMode = true;

    m_xTabCB->hide();
    m_xTabLabel->hide();
    m_xCharHiddenCB->hide();
    m_xBookmarkCB->hide();
    m_xBookmarkLabel->hide();

    m_xDirectCursorFrame->hide();
    m_xOnOffCB->hide();
    m_xDirectCursorFillMode->hide();
    m_xCursorProtFrame->hide();
    m_xCursorInProtCB->hide();
    m_xImageFrame->hide();

    m_xFmtAidsAutoComplFrame->hide();
    m_xEncloseWithCharactersCB->hide();
}

SwShdwCursorOptionsTabPage::~SwShdwCursorOptionsTabPage()
{
}

std::unique_ptr<SfxTabPage> SwShdwCursorOptionsTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet )
{
    return std::make_unique<SwShdwCursorOptionsTabPage>(pPage, pController, *rSet);
}

void SwShdwCursorOptionsTabPage::PageCreated( const SfxAllItemSet& aSet )
{
    const SwWrtShellItem* pWrtSh = aSet.GetItem<SwWrtShellItem>(SID_WRT_SHELL, false);
    if (pWrtSh)
        m_pWrtShell = pWrtSh->GetValue();
}

OUString SwShdwCursorOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"layoutopt"_ustr, u"displayfl"_ustr, u"cursoropt"_ustr,       u"cursorlabel"_ustr,
                          u"fillmode"_ustr,  u"lbImage"_ustr,   u"lbDefaultAnchor"_ustr, u"autocomplete"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[]
        = { u"mathbaseline"_ustr, u"paragraph"_ustr,    u"hyphens"_ustr,     u"spaces"_ustr,
            u"nonbreak"_ustr,     u"tabs"_ustr,         u"break"_ustr,       u"hiddentext"_ustr,
            u"bookmarks"_ustr,    u"cursorinprot"_ustr, u"cursoronoff"_ustr, u"enclosewithcharacters"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SwShdwCursorOptionsTabPage::FillItemSet( SfxItemSet* rSet )
{
    SwShadowCursorItem aOpt;
    aOpt.SetOn( m_xOnOffCB->get_active() );

    SwFillMode eMode = static_cast<SwFillMode>(m_xDirectCursorFillMode->get_active());
    aOpt.SetMode( eMode );

    bool bRet = false;
    const SwShadowCursorItem* pShadowCursorItem = rSet->GetItemIfSet( FN_PARAM_SHADOWCURSOR, false );
    if( !pShadowCursorItem || *pShadowCursorItem != aOpt )
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

    SwFmtAidsAutoComplItem aFmtAidsAutoComplOpt;
    aFmtAidsAutoComplOpt.SetEncloseWithCharactersOn(m_xEncloseWithCharactersCB->get_active());
    if (const SwFmtAidsAutoComplItem* pFmtAidsAutoComplItem
        = rSet->GetItemIfSet(FN_PARAM_FMT_AIDS_AUTOCOMPL, false);
        !pFmtAidsAutoComplItem || *pFmtAidsAutoComplItem != aFmtAidsAutoComplOpt)
    {
        rSet->Put(aFmtAidsAutoComplOpt);
        bRet = true;
    }

    const SwDocDisplayItem* pOldAttr = GetOldItem(GetItemSet(), FN_PARAM_DOCDISP);

    SwDocDisplayItem aDisp;

    aDisp.m_bParagraphEnd         = m_xParaCB->get_active();
    aDisp.m_bTab                  = m_xTabCB->get_active();
    aDisp.m_bSpace                = m_xSpacesCB->get_active();
    aDisp.m_bNonbreakingSpace     = m_xHSpacesCB->get_active();
    aDisp.m_bSoftHyphen           = m_xSHyphCB->get_active();
    aDisp.m_bCharHiddenText       = m_xCharHiddenCB->get_active();
    aDisp.m_bBookmarks            = m_xBookmarkCB->get_active();
    aDisp.m_bManualBreak          = m_xBreakCB->get_active();
    aDisp.m_xDefaultAnchor        = m_xDefaultAnchorType->get_active();

    bRet |= (!pOldAttr || aDisp != *pOldAttr);
    if(bRet)
        bRet = nullptr != rSet->Put(aDisp);

    return bRet;
}

void SwShdwCursorOptionsTabPage::Reset( const SfxItemSet* rSet )
{
    SwFillMode eMode = SwFillMode::Tab;
    bool bIsOn = false;

    if( const SwShadowCursorItem* pItem = rSet->GetItemIfSet( FN_PARAM_SHADOWCURSOR, false ))
    {
        eMode = pItem->GetMode();
        bIsOn = pItem->IsOn();
    }

    bool bReadOnly = officecfg::Office::Writer::Cursor::DirectCursor::UseDirectCursor::isReadOnly();
    m_xOnOffCB->set_active( bIsOn );
    m_xOnOffCB->set_sensitive(!bReadOnly);
    m_xOnOffImg->set_visible(bReadOnly);

    bReadOnly = officecfg::Office::Writer::Cursor::DirectCursor::Insert::isReadOnly();
    m_xDirectCursorFillMode->set_active( static_cast<int>(eMode) );
    m_xDirectCursorFillMode->set_sensitive(!bReadOnly);
    m_xDirectCursorFillModeImg->set_visible(bReadOnly);

    if (m_pWrtShell) {
        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Layout::Other::IsAlignMathObjectsToBaseline::isReadOnly() :
            officecfg::Office::WriterWeb::Layout::Other::IsAlignMathObjectsToBaseline::isReadOnly();
        m_xMathBaselineAlignmentCB->set_active( m_pWrtShell->GetDoc()->getIDocumentSettingAccess().get( DocumentSettingId::MATH_BASELINE_ALIGNMENT ) );
        m_xMathBaselineAlignmentCB->set_sensitive(!bReadOnly);
        m_xMathBaselineAlignmentImg->set_visible(bReadOnly);
        m_xMathBaselineAlignmentCB->save_state();
    } else {
        m_xMathBaselineAlignmentCB->hide();
    }

    bool bIsEncloseWithCharactersOn = false;
    if (const SwFmtAidsAutoComplItem* pItem
        = rSet->GetItemIfSet(FN_PARAM_FMT_AIDS_AUTOCOMPL, false))
    {
        bIsEncloseWithCharactersOn = pItem->IsEncloseWithCharactersOn();
    }
    bReadOnly = officecfg::Office::Writer::FmtAidsAutocomplete::EncloseWithCharacters::isReadOnly();
    m_xEncloseWithCharactersCB->set_active(bIsEncloseWithCharactersOn);
    m_xEncloseWithCharactersCB->set_sensitive(!bReadOnly);
    m_xEncloseWithCharactersImg->set_visible(bReadOnly);

    if( const SfxBoolItem* pItem = rSet->GetItemIfSet( FN_PARAM_CRSR_IN_PROTECTED, false ) )
        m_xCursorInProtCB->set_active(pItem->GetValue());
    bReadOnly = officecfg::Office::Writer::Cursor::Option::ProtectedArea::isReadOnly();
    m_xCursorInProtCB->set_sensitive(!bReadOnly);
    m_xCursorInProtImg->set_visible(bReadOnly);
    m_xCursorInProtCB->save_state();

    const SwDocDisplayItem* pDocDisplayAttr = rSet->GetItemIfSet( FN_PARAM_DOCDISP, false );
    if(pDocDisplayAttr)
    {
        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Content::NonprintingCharacter::ParagraphEnd::isReadOnly() :
            officecfg::Office::WriterWeb::Content::NonprintingCharacter::ParagraphEnd::isReadOnly();
        m_xParaCB->set_active( pDocDisplayAttr->m_bParagraphEnd );
        m_xParaCB->set_sensitive(!bReadOnly);
        m_xParaImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::NonprintingCharacter::Tab::isReadOnly();
        m_xTabCB->set_active( pDocDisplayAttr->m_bTab );
        m_xTabCB->set_sensitive(!bReadOnly);
        m_xTabImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Content::NonprintingCharacter::Space::isReadOnly() :
            officecfg::Office::WriterWeb::Content::NonprintingCharacter::Space::isReadOnly();
        m_xSpacesCB->set_active( pDocDisplayAttr->m_bSpace );
        m_xSpacesCB->set_sensitive(!bReadOnly);
        m_xSpacesImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Content::NonprintingCharacter::ProtectedSpace::isReadOnly() :
            officecfg::Office::WriterWeb::Content::NonprintingCharacter::ProtectedSpace::isReadOnly();
        m_xHSpacesCB->set_active( pDocDisplayAttr->m_bNonbreakingSpace );
        m_xHSpacesCB->set_sensitive(!bReadOnly);
        m_xHSpacesImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Content::NonprintingCharacter::OptionalHyphen::isReadOnly() :
            officecfg::Office::WriterWeb::Content::NonprintingCharacter::OptionalHyphen::isReadOnly();
        m_xSHyphCB->set_active( pDocDisplayAttr->m_bSoftHyphen );
        m_xSHyphCB->set_sensitive(!bReadOnly);
        m_xSHyphImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::NonprintingCharacter::HiddenCharacter::isReadOnly();
        m_xCharHiddenCB->set_active( pDocDisplayAttr->m_bCharHiddenText );
        m_xCharHiddenCB->set_sensitive(!bReadOnly);
        m_xCharHiddenImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::NonprintingCharacter::Bookmarks::isReadOnly();
        m_xBookmarkCB->set_active(pDocDisplayAttr->m_bBookmarks);
        m_xBookmarkCB->set_sensitive(!bReadOnly);
        m_xBookmarkImg->set_visible(bReadOnly);

        bReadOnly = !m_bHTMLMode ? officecfg::Office::Writer::Content::NonprintingCharacter::Break::isReadOnly() :
            officecfg::Office::WriterWeb::Content::NonprintingCharacter::Break::isReadOnly();
        m_xBreakCB->set_active( pDocDisplayAttr->m_bManualBreak );
        m_xBreakCB->set_sensitive(!bReadOnly);
        m_xBreakImg->set_visible(bReadOnly);

        bReadOnly = officecfg::Office::Writer::Content::Display::DefaultAnchor::isReadOnly();
        m_xDefaultAnchorType->set_active( pDocDisplayAttr->m_xDefaultAnchor );
        m_xDefaultAnchorType->set_sensitive(!bReadOnly);
        m_xDefaultAnchorTypeImg->set_visible(bReadOnly);
    }
}

namespace {

// TabPage for Redlining
struct CharAttr
{
    sal_uInt16 nItemId;
    sal_uInt16 nAttr;
};

}

// Edit corresponds to Paste-attributes
CharAttr const aRedlineAttr[] =
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
SwMarkPreview::SwMarkPreview()
    : m_aTransCol(COL_TRANSPARENT)
    , m_aMarkCol(COL_LIGHTRED)
    , m_nMarkPos(0)

{
    InitColors();
}

SwMarkPreview::~SwMarkPreview()
{
}

void SwMarkPreview::InitColors()
{
    // m_aTransCol and m_aMarkCol are _not_ changed because they are set from outside!

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    m_aBgCol = rSettings.GetWindowColor();

    bool bHC = rSettings.GetHighContrastMode();
    const Color& rFontColor = SwViewOption::GetCurrentViewOptions().GetFontColor();
    m_aLineCol = bHC? rFontColor : COL_BLACK;
    m_aShadowCol = bHC? m_aBgCol : rSettings.GetShadowColor();
    m_aTextCol = bHC? rFontColor : COL_GRAY;
    m_aPrintAreaCol = m_aTextCol;
}

void SwMarkPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle &/*rRect*/)
{
    const Size aSz(GetOutputSizePixel());

    // Page
    m_aPage.SetSize(Size(aSz.Width() - 3, aSz.Height() - 3));

    const tools::Long nOutWPix = m_aPage.GetWidth();
    const tools::Long nOutHPix = m_aPage.GetHeight();

    // PrintArea
    const tools::Long nLBorder = 8;
    const tools::Long nRBorder = 8;
    const tools::Long nTBorder = 4;
    const tools::Long nBBorder = 4;

    m_aLeftPagePrtArea = tools::Rectangle(Point(nLBorder, nTBorder), Point((nOutWPix - 1) - nRBorder, (nOutHPix - 1) - nBBorder));
    const tools::Long nWidth = m_aLeftPagePrtArea.GetWidth();
    const tools::Long nCorr = (nWidth & 1) != 0 ? 0 : 1;
    m_aLeftPagePrtArea.SetSize(Size(nWidth / 2 - (nLBorder + nRBorder) / 2 + nCorr, m_aLeftPagePrtArea.GetHeight()));

    m_aRightPagePrtArea = m_aLeftPagePrtArea;
    m_aRightPagePrtArea.Move(m_aLeftPagePrtArea.GetWidth() + nLBorder + nRBorder + 1, 0);

    // draw shadow
    tools::Rectangle aShadow(m_aPage);
    aShadow += Point(3, 3);
    drawRect(rRenderContext, aShadow, m_aShadowCol, m_aTransCol);

    // draw page
    drawRect(rRenderContext, m_aPage, m_aBgCol, m_aLineCol);

    // draw separator
    tools::Rectangle aPageSeparator(m_aPage);
    aPageSeparator.SetSize(Size(2, aPageSeparator.GetHeight()));
    aPageSeparator.Move(m_aPage.GetWidth() / 2 - 1, 0);
    drawRect(rRenderContext, aPageSeparator, m_aLineCol, m_aTransCol);

    PaintPage(rRenderContext, m_aLeftPagePrtArea);
    PaintPage(rRenderContext, m_aRightPagePrtArea);

    tools::Rectangle aLeftMark(Point(m_aPage.Left() + 2, m_aLeftPagePrtArea.Top() + 4), Size(m_aLeftPagePrtArea.Left() - 4, 2));
    tools::Rectangle aRightMark(Point(m_aRightPagePrtArea.Right() + 2, m_aRightPagePrtArea.Bottom() - 6), Size(m_aLeftPagePrtArea.Left() - 4, 2));

    switch (m_nMarkPos)
    {
        case 1:     // left
            aRightMark.SetPos(Point(m_aRightPagePrtArea.Left() - 2 - aRightMark.GetWidth(), aRightMark.Top()));
            break;

        case 2:     // right
            aLeftMark.SetPos(Point(m_aLeftPagePrtArea.Right() + 2, aLeftMark.Top()));
            break;

        case 3:     // outside
            break;

        case 4:     // inside
            aLeftMark.SetPos(Point(m_aLeftPagePrtArea.Right() + 2, aLeftMark.Top()));
            aRightMark.SetPos(Point(m_aRightPagePrtArea.Left() - 2 - aRightMark.GetWidth(), aRightMark.Top()));
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

    const tools::Long nStep = aTextLine.GetHeight() + 2;
    const tools::Long nLines = rRect.GetHeight() / (aTextLine.GetHeight() + 2) - 1;

    // simulate text
    for (tools::Long i = 0; i < nLines; ++i)
    {
        if (i == (nLines - 1))
            aTextLine.SetSize(Size(aTextLine.GetWidth() / 2, aTextLine.GetHeight()));

        if (m_aPage.Contains(aTextLine))
            drawRect(rRenderContext, aTextLine, m_aTextCol, m_aTransCol);

        aTextLine.Move(0, nStep);
    }
    aTextLine.Move(0, -nStep);
}

void SwMarkPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aInitialSize = getPreviewOptionsSize(pDrawingArea->get_ref_device());
    pDrawingArea->set_size_request(aInitialSize.Width(), aInitialSize.Height());
    weld::CustomWidgetController::SetDrawingArea(pDrawingArea);
    SetOutputSizePixel(aInitialSize);
}

namespace
{
    void lcl_FillRedlineAttrListBox(
            weld::ComboBox& rLB, const AuthorCharAttr& rAttrToSelect,
            const sal_uInt16* pAttrMap, const size_t nAttrMapSize)
    {
        for (size_t i = 0; i != nAttrMapSize; ++i)
        {
            CharAttr const & rAttr(aRedlineAttr[pAttrMap[i]]);
            rLB.set_id(i, weld::toId(&rAttr));
            if (rAttr.nItemId == rAttrToSelect.m_nItemId &&
                rAttr.nAttr == rAttrToSelect.m_nAttr)
                rLB.set_active(i);
        }
    }
}

SwRedlineOptionsTabPage::SwRedlineOptionsTabPage(weld::Container* pPage, weld::DialogController* pController,
                                                 const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/optredlinepage.ui"_ustr, u"OptRedLinePage"_ustr, &rSet)
    , m_xInsertLB(m_xBuilder->weld_combo_box(u"insert"_ustr))
    , m_xInsertImg(m_xBuilder->weld_widget(u"lockinsert"_ustr))
    , m_xInsertColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"insertcolor"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xInsertColorImg(m_xBuilder->weld_widget(u"lockinsertcolor"_ustr))
    , m_xInsertedPreviewWN(new SvxFontPrevWindow)
    , m_xInsertedPreview(new weld::CustomWeld(*m_xBuilder, u"insertedpreview"_ustr, *m_xInsertedPreviewWN))
    , m_xDeletedLB(m_xBuilder->weld_combo_box(u"deleted"_ustr))
    , m_xDeletedImg(m_xBuilder->weld_widget(u"lockdeleted"_ustr))
    , m_xDeletedColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"deletedcolor"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xDeletedColorImg(m_xBuilder->weld_widget(u"lockdeletedcolor"_ustr))
    , m_xDeletedPreviewWN(new SvxFontPrevWindow)
    , m_xDeletedPreview(new weld::CustomWeld(*m_xBuilder, u"deletedpreview"_ustr, *m_xDeletedPreviewWN))
    , m_xChangedLB(m_xBuilder->weld_combo_box(u"changed"_ustr))
    , m_xChangedImg(m_xBuilder->weld_widget(u"lockchanged"_ustr))
    , m_xChangedColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"changedcolor"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xChangedColorImg(m_xBuilder->weld_widget(u"lockchangedcolor"_ustr))
    , m_xChangedPreviewWN(new SvxFontPrevWindow)
    , m_xChangedPreview(new weld::CustomWeld(*m_xBuilder, u"changedpreview"_ustr, *m_xChangedPreviewWN))
    , m_xMarkPosLB(m_xBuilder->weld_combo_box(u"markpos"_ustr))
    , m_xMarkPosImg(m_xBuilder->weld_widget(u"lockmarkpos"_ustr))
    , m_xMarkColorLB(new ColorListBox(m_xBuilder->weld_menu_button(u"markcolor"_ustr),
                [this]{ return GetDialogController()->getDialog(); }))
    , m_xMarkColorImg(m_xBuilder->weld_widget(u"lockmarkcolor"_ustr))
    , m_xMarkPreviewWN(new SwMarkPreview)
    , m_xMarkPreview(new weld::CustomWeld(*m_xBuilder, u"markpreview"_ustr, *m_xMarkPreviewWN))
{
    Size aPreviewSize(getPreviewOptionsSize(m_xMarkPreviewWN->GetDrawingArea()->get_ref_device()));

    m_xInsertColorLB->SetSlotId(SID_AUTHOR_COLOR, true);
    m_xDeletedColorLB->SetSlotId(SID_AUTHOR_COLOR, true);
    m_xChangedColorLB->SetSlotId(SID_AUTHOR_COLOR, true);

    m_xInsertedPreviewWN->set_size_request(aPreviewSize.Width(), aPreviewSize.Height());
    m_xDeletedPreviewWN->set_size_request(aPreviewSize.Width(), aPreviewSize.Height());
    m_xChangedPreviewWN->set_size_request(aPreviewSize.Width(), aPreviewSize.Height());
    m_xMarkPreviewWN->set_size_request(aPreviewSize.Width(), aPreviewSize.Height());

    for (sal_Int32 i = 0, nEntryCount = m_xInsertLB->get_count(); i < nEntryCount; ++i)
    {
        const OUString sEntry(m_xInsertLB->get_text(i));
        m_xDeletedLB->append_text(sEntry);
        m_xChangedLB->append_text(sEntry);
    };

    // remove strikethrough from insert and change and underline + double
    // underline from delete
    m_xInsertLB->remove(5);
    m_xChangedLB->remove(5);
    m_xDeletedLB->remove(4);
    m_xDeletedLB->remove(3);

    Link<weld::ComboBox&,void> aLk = LINK(this, SwRedlineOptionsTabPage, AttribHdl);
    m_xInsertLB->connect_changed( aLk );
    m_xDeletedLB->connect_changed( aLk );
    m_xChangedLB->connect_changed( aLk );

    Link<ColorListBox&,void> aLk2 = LINK(this, SwRedlineOptionsTabPage, ColorHdl);
    m_xInsertColorLB->SetSelectHdl( aLk2 );
    m_xDeletedColorLB->SetSelectHdl( aLk2 );
    m_xChangedColorLB->SetSelectHdl( aLk2 );

    m_xMarkPosLB->connect_changed(LINK(this, SwRedlineOptionsTabPage, ChangedMaskPrevHdl));
    m_xMarkColorLB->SetSelectHdl(LINK(this, SwRedlineOptionsTabPage, ChangedMaskColorPrevHdl));
}

SwRedlineOptionsTabPage::~SwRedlineOptionsTabPage()
{
    m_xInsertColorLB.reset();
    m_xInsertedPreview.reset();
    m_xInsertedPreviewWN.reset();
    m_xDeletedColorLB.reset();
    m_xDeletedPreview.reset();
    m_xDeletedPreviewWN.reset();
    m_xChangedColorLB.reset();
    m_xChangedPreview.reset();
    m_xChangedPreviewWN.reset();
    m_xMarkColorLB.reset();
    m_xMarkPreview.reset();
    m_xMarkPreviewWN.reset();
}

std::unique_ptr<SfxTabPage> SwRedlineOptionsTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwRedlineOptionsTabPage>(pPage, pController, *rSet);
}

OUString SwRedlineOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label2"_ustr, u"insert_label"_ustr,  u"insertcolor_label"_ustr,
                          u"label3"_ustr, u"deleted_label"_ustr, u"deletedcolor_label"_ustr,
                          u"label4"_ustr, u"changed_label"_ustr, u"changedcolor_label"_ustr,
                          u"label5"_ustr, u"markpos_label"_ustr, u"markcolor_label"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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

    sal_Int32 nPos = m_xInsertLB->get_active();
    if (nPos != -1)
    {
        pAttr = weld::fromId<CharAttr*>(m_xInsertLB->get_id(nPos));
        aInsertedAttr.m_nItemId = pAttr->nItemId;
        aInsertedAttr.m_nAttr = pAttr->nAttr;
        aInsertedAttr.m_nColor = m_xInsertColorLB->GetSelectEntryColor();
        pOpt->SetInsertAuthorAttr(aInsertedAttr);
    }

    nPos = m_xDeletedLB->get_active();
    if (nPos != -1)
    {
        pAttr = weld::fromId<CharAttr*>(m_xDeletedLB->get_id(nPos));
        aDeletedAttr.m_nItemId = pAttr->nItemId;
        aDeletedAttr.m_nAttr = pAttr->nAttr;
        aDeletedAttr.m_nColor = m_xDeletedColorLB->GetSelectEntryColor();
        pOpt->SetDeletedAuthorAttr(aDeletedAttr);
    }

    nPos = m_xChangedLB->get_active();
    if (nPos != -1)
    {
        pAttr = weld::fromId<CharAttr*>(m_xChangedLB->get_id(nPos));
        aChangedAttr.m_nItemId = pAttr->nItemId;
        aChangedAttr.m_nAttr = pAttr->nAttr;
        aChangedAttr.m_nColor = m_xChangedColorLB->GetSelectEntryColor();
        pOpt->SetFormatAuthorAttr(aChangedAttr);
    }

    nPos = 0;
    switch (m_xMarkPosLB->get_active())
    {
        case 0: nPos = text::HoriOrientation::NONE;       break;
        case 1: nPos = text::HoriOrientation::LEFT;       break;
        case 2: nPos = text::HoriOrientation::RIGHT;      break;
        case 3: nPos = text::HoriOrientation::OUTSIDE;    break;
        case 4: nPos = text::HoriOrientation::INSIDE;     break;
    }
    pOpt->SetMarkAlignMode(nPos);
    pOpt->SetMarkAlignColor(m_xMarkColorLB->GetSelectEntryColor());

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
    InitFontStyle(*m_xInsertedPreviewWN, SwResId(STR_OPT_PREVIEW_INSERTED));
    InitFontStyle(*m_xDeletedPreviewWN, SwResId(STR_OPT_PREVIEW_DELETED));
    InitFontStyle(*m_xChangedPreviewWN, SwResId(STR_OPT_PREVIEW_CHANGED));

    Color nColor = rInsertAttr.m_nColor;
    m_xInsertColorLB->SelectEntry(nColor);
    m_xInsertColorLB->set_sensitive(!officecfg::Office::Writer::Revision::TextDisplay::Insert::Attribute::isReadOnly());
    m_xInsertColorImg->set_visible(officecfg::Office::Writer::Revision::TextDisplay::Insert::Attribute::isReadOnly());

    nColor = rDeletedAttr.m_nColor;
    m_xDeletedColorLB->SelectEntry(nColor);
    m_xDeletedColorLB->set_sensitive(!officecfg::Office::Writer::Revision::TextDisplay::Delete::Attribute::isReadOnly());
    m_xDeletedColorImg->set_visible(officecfg::Office::Writer::Revision::TextDisplay::Delete::Attribute::isReadOnly());

    nColor = rChangedAttr.m_nColor;
    m_xChangedColorLB->SelectEntry(nColor);
    m_xChangedColorLB->set_sensitive(!officecfg::Office::Writer::Revision::TextDisplay::ChangedAttribute::Attribute::isReadOnly());
    m_xChangedColorImg->set_visible(officecfg::Office::Writer::Revision::TextDisplay::ChangedAttribute::Attribute::isReadOnly());

    m_xMarkColorLB->SelectEntry(pOpt->GetMarkAlignColor());
    m_xMarkColorLB->set_sensitive(!officecfg::Office::Writer::Revision::LinesChanged::Color::isReadOnly());
    m_xMarkColorImg->set_visible(officecfg::Office::Writer::Revision::LinesChanged::Color::isReadOnly());

    m_xInsertLB->set_active(0);
    m_xInsertLB->set_sensitive(!officecfg::Office::Writer::Revision::TextDisplay::Insert::Color::isReadOnly());
    m_xInsertImg->set_visible(officecfg::Office::Writer::Revision::TextDisplay::Insert::Color::isReadOnly());
    m_xDeletedLB->set_active(0);
    m_xDeletedLB->set_sensitive(!officecfg::Office::Writer::Revision::TextDisplay::Delete::Color::isReadOnly());
    m_xDeletedImg->set_visible(officecfg::Office::Writer::Revision::TextDisplay::Delete::Color::isReadOnly());
    m_xChangedLB->set_active(0);
    m_xChangedLB->set_sensitive(!officecfg::Office::Writer::Revision::TextDisplay::ChangedAttribute::Color::isReadOnly());
    m_xChangedImg->set_visible(officecfg::Office::Writer::Revision::TextDisplay::ChangedAttribute::Color::isReadOnly());

    lcl_FillRedlineAttrListBox(*m_xInsertLB, rInsertAttr, aInsertAttrMap, SAL_N_ELEMENTS(aInsertAttrMap));
    lcl_FillRedlineAttrListBox(*m_xDeletedLB, rDeletedAttr, aDeletedAttrMap, SAL_N_ELEMENTS(aDeletedAttrMap));
    lcl_FillRedlineAttrListBox(*m_xChangedLB, rChangedAttr, aChangedAttrMap, SAL_N_ELEMENTS(aChangedAttrMap));

    sal_Int32 nPos = 0;
    switch (pOpt->GetMarkAlignMode())
    {
        case text::HoriOrientation::NONE:     nPos = 0;   break;
        case text::HoriOrientation::LEFT:     nPos = 1;   break;
        case text::HoriOrientation::RIGHT:    nPos = 2;   break;
        case text::HoriOrientation::OUTSIDE:  nPos = 3;   break;
        case text::HoriOrientation::INSIDE:   nPos = 4;   break;
    }
    m_xMarkPosLB->set_active(nPos);
    m_xMarkPosLB->set_sensitive(!officecfg::Office::Writer::Revision::LinesChanged::Mark::isReadOnly());
    m_xMarkPosImg->set_visible(officecfg::Office::Writer::Revision::LinesChanged::Mark::isReadOnly());

    // show settings in preview
    AttribHdl(*m_xInsertLB);
    ColorHdl(*m_xInsertColorLB);
    AttribHdl(*m_xDeletedLB);
    ColorHdl(*m_xInsertColorLB);
    AttribHdl(*m_xChangedLB);
    ColorHdl(*m_xChangedColorLB);

    ChangedMaskPrev();
}

IMPL_LINK( SwRedlineOptionsTabPage, AttribHdl, weld::ComboBox&, rLB, void )
{
    SvxFontPrevWindow *pPrev = nullptr;
    ColorListBox *pColorLB;

    if (&rLB == m_xInsertLB.get())
    {
        pColorLB = m_xInsertColorLB.get();
        pPrev = m_xInsertedPreviewWN.get();
    }
    else if (&rLB == m_xDeletedLB.get())
    {
        pColorLB = m_xDeletedColorLB.get();
        pPrev = m_xDeletedPreviewWN.get();
    }
    else
    {
        pColorLB = m_xChangedColorLB.get();
        pPrev = m_xChangedPreviewWN.get();
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

    sal_Int32 nPos = rLB.get_active();
    if( nPos == -1)
        nPos = 0;

    CharAttr* pAttr = weld::fromId<CharAttr*>(rLB.get_id(nPos));
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

IMPL_LINK(SwRedlineOptionsTabPage, ColorHdl, ColorListBox&, rListBox, void)
{
    ColorListBox* pColorLB = &rListBox;
    SvxFontPrevWindow *pPrev = nullptr;
    weld::ComboBox* pLB;

    if (pColorLB == m_xInsertColorLB.get())
    {
        pLB = m_xInsertLB.get();
        pPrev = m_xInsertedPreviewWN.get();
    }
    else if (pColorLB == m_xDeletedColorLB.get())
    {
        pLB = m_xDeletedLB.get();
        pPrev = m_xDeletedPreviewWN.get();
    }
    else
    {
        pLB = m_xChangedLB.get();
        pPrev = m_xChangedPreviewWN.get();
    }

    SvxFont&    rFont = pPrev->GetFont();
    SvxFont&    rCJKFont = pPrev->GetCJKFont();
    sal_Int32      nPos = pLB->get_active();
    if( nPos == -1)
        nPos = 0;

    CharAttr* pAttr = weld::fromId<CharAttr*>(pLB->get_id(nPos));

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
    m_xMarkPreviewWN->SetMarkPos(m_xMarkPosLB->get_active());
    m_xMarkPreviewWN->SetColor(m_xMarkColorLB->GetSelectEntryColor());

    m_xMarkPreviewWN->Invalidate();
}

IMPL_LINK_NOARG(SwRedlineOptionsTabPage, ChangedMaskPrevHdl, weld::ComboBox&, void)
{
    ChangedMaskPrev();
}

IMPL_LINK_NOARG(SwRedlineOptionsTabPage, ChangedMaskColorPrevHdl, ColorListBox&, void)
{
    ChangedMaskPrev();
}

void SwRedlineOptionsTabPage::InitFontStyle(SvxFontPrevWindow& rExampleWin, const OUString& rText)
{
    const AllSettings&  rAllSettings = Application::GetSettings();
    LanguageType        eLangType = rAllSettings.GetUILanguageTag().getLanguageType();
    Color               aBackCol( rAllSettings.GetStyleSettings().GetWindowColor() );
    SvxFont&            rFont = rExampleWin.GetFont();
    SvxFont&            rCJKFont = rExampleWin.GetCJKFont();
    SvxFont&            rCTLFont = rExampleWin.GetCTLFont();

    OutputDevice& rDevice = rExampleWin.GetDrawingArea()->get_ref_device();

    vcl::Font           aFont( OutputDevice::GetDefaultFont( DefaultFontType::SERIF, eLangType,
                                                        GetDefaultFontFlags::OnlyOne, &rDevice ) );
    vcl::Font           aCJKFont( OutputDevice::GetDefaultFont( DefaultFontType::CJK_TEXT, eLangType,
                                                        GetDefaultFontFlags::OnlyOne, &rDevice ) );
    vcl::Font           aCTLFont( OutputDevice::GetDefaultFont( DefaultFontType::CTL_TEXT, eLangType,
                                                        GetDefaultFontFlags::OnlyOne, &rDevice ) );
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

    const Size aNewSize( 0, rExampleWin.GetOutputSizePixel().Height() * 2 / 3 );
    rFont.SetFontSize( aNewSize );
    rCJKFont.SetFontSize( aNewSize );

    rExampleWin.SetFont( rFont, rCJKFont,rCTLFont );
    rExampleWin.SetPreviewText(rText);
}

SwCompareOptionsTabPage::SwCompareOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/optcomparison.ui", "OptComparison", &rSet)
    , m_xAutoRB(m_xBuilder->weld_radio_button("auto"))
    , m_xWordRB(m_xBuilder->weld_radio_button("byword"))
    , m_xCharRB(m_xBuilder->weld_radio_button("bycharacter"))
    , m_xCompareModeImg(m_xBuilder->weld_widget("lockcomparemode"))
    , m_xRsidCB(m_xBuilder->weld_check_button("useRSID"))
    , m_xRsidImg(m_xBuilder->weld_widget("lockuseRSID"))
    , m_xStoreRsidCB(m_xBuilder->weld_check_button("storeRSID"))
    , m_xStoreRsidImg(m_xBuilder->weld_widget("lockstoreRSID"))
{
    Link<weld::Toggleable&,void> aLnk( LINK( this, SwCompareOptionsTabPage, ComparisonHdl ) );
    m_xAutoRB->connect_toggled( aLnk );
    m_xWordRB->connect_toggled( aLnk );
    m_xCharRB->connect_toggled( aLnk );
}

SwCompareOptionsTabPage::~SwCompareOptionsTabPage()
{
}

std::unique_ptr<SfxTabPage> SwCompareOptionsTabPage::Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwCompareOptionsTabPage>(pPage, pController, *rAttrSet);
}

OUString SwCompareOptionsTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr, u"setting"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString radioButton[] = { "auto", "byword", "bycharacter" };

    for (const auto& radio : radioButton)
    {
        if (const auto& pString = m_xBuilder->weld_radio_button(radio))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
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
    }
    else if( eCmpMode == SwCompareMode::ByWord )
    {
        m_xWordRB->set_active(true);
        m_xRsidCB->set_sensitive(true);
    }
    else if( eCmpMode == SwCompareMode::ByChar)
    {
        m_xCharRB->set_active(true);
        m_xRsidCB->set_sensitive(true);
    }

    if (officecfg::Office::Writer::Comparison::Mode::isReadOnly())
    {
        m_xAutoRB->set_sensitive(false);
        m_xWordRB->set_sensitive(false);
        m_xCharRB->set_sensitive(false);
        m_xCompareModeImg->set_visible(true);
    }

    m_xAutoRB->save_state();
    m_xWordRB->save_state();
    m_xCharRB->save_state();

    m_xRsidCB->set_active( pOpt->IsUseRsid() );
    if (officecfg::Office::Writer::Comparison::UseRSID::isReadOnly())
    {
        m_xRsidCB->set_sensitive(false);
        m_xRsidImg->set_visible(true);
    }
    m_xRsidCB->save_state();

    m_xStoreRsidCB->set_active(pOpt->IsStoreRsid());
    m_xStoreRsidCB->set_sensitive(!officecfg::Office::Writer::Comparison::StoreRSID::isReadOnly());
    m_xStoreRsidImg->set_visible(officecfg::Office::Writer::Comparison::StoreRSID::isReadOnly());
    m_xStoreRsidCB->save_state();
}

IMPL_LINK(SwCompareOptionsTabPage, ComparisonHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    bool bChecked = !m_xAutoRB->get_active();
    m_xRsidCB->set_sensitive( bChecked );
}

#ifdef DBG_UTIL

SwTestTabPage::SwTestTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rCoreSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/opttestpage.ui"_ustr, u"OptTestPage"_ustr, &rCoreSet)
    , m_bAttrModified( false )
    , m_xTest1CBox(m_xBuilder->weld_check_button(u"unused"_ustr))
    , m_xTest2CBox(m_xBuilder->weld_check_button(u"dynamic"_ustr))
    , m_xTest3CBox(m_xBuilder->weld_check_button(u"nocalm"_ustr))
    , m_xTest4CBox(m_xBuilder->weld_check_button(u"wysiwygdbg"_ustr))
    , m_xTest5CBox(m_xBuilder->weld_check_button(u"noidle"_ustr))
    , m_xTest6CBox(m_xBuilder->weld_check_button(u"noscreenadj"_ustr))
    , m_xTest7CBox(m_xBuilder->weld_check_button(u"winformat"_ustr))
    , m_xTest8CBox(m_xBuilder->weld_check_button(u"noscroll"_ustr))
    , m_xTest9CBox(m_xBuilder->weld_check_button(u"DrawingLayerNotLoading"_ustr))
    , m_xTest10CBox(m_xBuilder->weld_check_button(u"AutoFormatByInput"_ustr))
{
    Init();
}

SwTestTabPage::~SwTestTabPage()
{
}

std::unique_ptr<SfxTabPage> SwTestTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwTestTabPage>(pPage, pController, *rAttrSet);
}

OUString SwTestTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString checkButton[] = { u"unused"_ustr,           u"dynamic"_ustr,  u"nocalm"_ustr,
                               u"wysiwygdbg"_ustr,       u"noidle"_ustr,   u"noscreenadj"_ustr,
                               u"winformat"_ustr,        u"noscroll"_ustr, u"DrawingLayerNotLoading"_ustr,
                               u"AutoFormatByInput"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    if (const auto& pString = m_xBuilder->weld_label(u"label1"_ustr))
        sAllStrings += pString->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool    SwTestTabPage::FillItemSet( SfxItemSet* rCoreSet )
{

    if ( m_bAttrModified )
    {
        SwTestItem aTestItem;
        aTestItem.m_bTest1=m_xTest1CBox->get_active();
        aTestItem.m_bTest2=m_xTest2CBox->get_active();
        aTestItem.m_bTest3=m_xTest3CBox->get_active();
        aTestItem.m_bTest4=m_xTest4CBox->get_active();
        aTestItem.m_bTest5=m_xTest5CBox->get_active();
        aTestItem.m_bTest6=m_xTest6CBox->get_active();
        aTestItem.m_bTest7=m_xTest7CBox->get_active();
        aTestItem.m_bTest8=m_xTest8CBox->get_active();
        aTestItem.m_bTest9=m_xTest9CBox->get_active();
        aTestItem.m_bTest10=m_xTest10CBox->get_active();
        rCoreSet->Put(aTestItem);
    }
    return m_bAttrModified;
}

void SwTestTabPage::Reset( const SfxItemSet* )
{
    const SfxItemSet& rSet = GetItemSet();
    const SwTestItem* pTestAttr = rSet.GetItemIfSet( FN_PARAM_SWTEST, false );
    if(!pTestAttr)
        return;

    m_xTest1CBox->set_active(pTestAttr->m_bTest1);
    m_xTest2CBox->set_active(pTestAttr->m_bTest2);
    m_xTest3CBox->set_active(pTestAttr->m_bTest3);
    m_xTest4CBox->set_active(pTestAttr->m_bTest4);
    m_xTest5CBox->set_active(pTestAttr->m_bTest5);
    m_xTest6CBox->set_active(pTestAttr->m_bTest6);
    m_xTest7CBox->set_active(pTestAttr->m_bTest7);
    m_xTest8CBox->set_active(pTestAttr->m_bTest8);
    m_xTest9CBox->set_active(pTestAttr->m_bTest9);
    m_xTest10CBox->set_active(pTestAttr->m_bTest10);
}

void SwTestTabPage::Init()
{
    // handler
    Link<weld::Toggleable&,void> aLk = LINK( this, SwTestTabPage, AutoClickHdl );
    m_xTest1CBox->connect_toggled( aLk );
    m_xTest2CBox->connect_toggled( aLk );
    m_xTest3CBox->connect_toggled( aLk );
    m_xTest4CBox->connect_toggled( aLk );
    m_xTest5CBox->connect_toggled( aLk );
    m_xTest6CBox->connect_toggled( aLk );
    m_xTest7CBox->connect_toggled( aLk );
    m_xTest8CBox->connect_toggled( aLk );
    m_xTest9CBox->connect_toggled( aLk );
    m_xTest10CBox->connect_toggled( aLk );
}

IMPL_LINK_NOARG(SwTestTabPage, AutoClickHdl, weld::Toggleable&, void)
{
    m_bAttrModified = true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
