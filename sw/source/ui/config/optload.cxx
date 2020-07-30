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

#include <officecfg/Office/Writer.hxx>
#include <comphelper/string.hxx>
#include <swtypes.hxx>
#include <uiitems.hxx>
#include <modcfg.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>
#include <wrtsh.hxx>
#include <linkenum.hxx>
#include <uitool.hxx>
#include <view.hxx>

#include <strings.hrc>
#include <optload.hrc>
#include <cmdid.h>

#include <optload.hxx>
#include <sfx2/htmlmode.hxx>
#include <fldmgr.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>

#include <svtools/insdlg.hxx>
#include <svtools/unitconv.hxx>
#include <tools/resary.hxx>
#include <vcl/settings.hxx>

#include <comphelper/classids.hxx>
#include <unotools/configmgr.hxx>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>

#include <doc.hxx>
#include <svl/cjkoptions.hxx>

using namespace ::com::sun::star;

#include <svl/eitem.hxx>

sal_uInt32 SwFieldUnitTable::Count()
{
    return SAL_N_ELEMENTS(STR_ARR_METRIC);
}

OUString SwFieldUnitTable::GetString(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return SwResId(STR_ARR_METRIC[nPos].first);
    return OUString();
}

FieldUnit SwFieldUnitTable::GetValue(sal_uInt32 nPos)
{
    if (RESARRAY_INDEX_NOTFOUND != nPos && nPos < Count())
        return STR_ARR_METRIC[nPos].second;
    return FieldUnit::NONE;
}

SwLoadOptPage::SwLoadOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/optgeneralpage.ui", "OptGeneralPage", &rSet)
    , m_pWrtShell(nullptr)
    , m_nLastTab(0)
    , m_nOldLinkMode(MANUAL)
    , m_xAlwaysRB(m_xBuilder->weld_radio_button("always"))
    , m_xRequestRB(m_xBuilder->weld_radio_button("onrequest"))
    , m_xNeverRB(m_xBuilder->weld_radio_button("never"))
    , m_xAutoUpdateFields(m_xBuilder->weld_check_button("updatefields"))
    , m_xAutoUpdateCharts(m_xBuilder->weld_check_button("updatecharts"))
    , m_xMetricLB(m_xBuilder->weld_combo_box("metric"))
    , m_xTabFT(m_xBuilder->weld_label("tablabel"))
    , m_xTabMF(m_xBuilder->weld_metric_spin_button("tab", FieldUnit::CM))
    , m_xUseSquaredPageMode(m_xBuilder->weld_check_button("squaremode"))
    , m_xUseCharUnit(m_xBuilder->weld_check_button("usecharunit"))
    , m_xWordCountED(m_xBuilder->weld_entry("wordcount"))
    , m_xShowStandardizedPageCount(m_xBuilder->weld_check_button("standardizedpageshow"))
    , m_xStandardizedPageSizeNF(m_xBuilder->weld_spin_button("standardpagesize"))
{
    for (sal_uInt32 i = 0; i < SwFieldUnitTable::Count(); ++i)
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
            {
                // use only these metrics
                m_xMetricLB->append(OUString::number(static_cast<sal_uInt32>(eFUnit)), sMetric);
                break;
            }
            default:; //prevent warning
        }
    }
    m_xMetricLB->connect_changed(LINK(this, SwLoadOptPage, MetricHdl));

    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem)
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
    {
        m_xTabFT->hide();
        m_xTabMF->hide();
    }

    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsAsianTypographyEnabled())
    {
        m_xUseSquaredPageMode->hide();
        m_xUseCharUnit->hide();
    }

    Link<weld::Button&,void> aLink = LINK(this, SwLoadOptPage, StandardizedPageCountCheckHdl);
    m_xShowStandardizedPageCount->connect_clicked(aLink);
}

SwLoadOptPage::~SwLoadOptPage()
{
}

std::unique_ptr<SfxTabPage> SwLoadOptPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                          const SfxItemSet* rAttrSet )
{
    return std::make_unique<SwLoadOptPage>(pPage, pController, *rAttrSet );
}

IMPL_LINK_NOARG(SwLoadOptPage, StandardizedPageCountCheckHdl, weld::Button&, void)
{
    m_xStandardizedPageSizeNF->set_sensitive(m_xShowStandardizedPageCount->get_active());
}

bool SwLoadOptPage::FillItemSet( SfxItemSet* rSet )
{
    bool bRet = false;
    SwModule* pMod = SW_MOD();

    sal_Int32 nNewLinkMode = AUTOMATIC;
    if (m_xNeverRB->get_active())
        nNewLinkMode = NEVER;
    else if (m_xRequestRB->get_active())
        nNewLinkMode = MANUAL;

    SwFieldUpdateFlags eFieldFlags = m_xAutoUpdateFields->get_active() ?
        m_xAutoUpdateCharts->get_active() ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF;

    if (m_xAutoUpdateFields->get_state_changed_from_saved() ||
        m_xAutoUpdateCharts->get_state_changed_from_saved())
    {
        pMod->ApplyFieldUpdateFlags(eFieldFlags);
        if(m_pWrtShell)
        {
            m_pWrtShell->SetFieldUpdateFlags(eFieldFlags);
            m_pWrtShell->SetModified();
        }
    }

    if (nNewLinkMode != m_nOldLinkMode)
    {
        pMod->ApplyLinkMode(nNewLinkMode);
        if (m_pWrtShell)
        {
            m_pWrtShell->SetLinkUpdMode( nNewLinkMode );
            m_pWrtShell->SetModified();
        }

        bRet = true;
    }

    const sal_Int32 nMPos = m_xMetricLB->get_active();
    if (m_xMetricLB->get_value_changed_from_saved())
    {
        // Double-Cast for VA3.0
        const sal_uInt16 nFieldUnit = m_xMetricLB->get_id(nMPos).toUInt32();
        rSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    if (m_xTabMF->get_visible() && m_xTabMF->get_value_changed_from_saved())
    {
        rSet->Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    static_cast<sal_uInt16>(m_xTabMF->denormalize(m_xTabMF->get_value(FieldUnit::TWIP)))));
        bRet = true;
    }

    bool bIsUseCharUnitFlag = m_xUseCharUnit->get_active();
    SvtCJKOptions aCJKOptions;
    bIsUseCharUnitFlag = bIsUseCharUnitFlag && aCJKOptions.IsAsianTypographyEnabled();
    if( (bIsUseCharUnitFlag ? 1 : 0) != m_xUseCharUnit->get_saved_state())
    {
        rSet->Put(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, bIsUseCharUnitFlag ));
        bRet = true;
    }

    if (m_xWordCountED->get_value_changed_from_saved())
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::AdditionalSeparators::set(m_xWordCountED->get_text(), batch);
        batch->commit();
        bRet = true;
    }

    if (m_xShowStandardizedPageCount->get_state_changed_from_saved())
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::set(
          m_xShowStandardizedPageCount->get_active(),
          batch);
        batch->commit();
        bRet = true;
    }

    if (m_xStandardizedPageSizeNF->get_value_changed_from_saved())
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::StandardizedPageSize::set(
          m_xStandardizedPageSizeNF->get_value(),
          batch);
        batch->commit();
        bRet = true;
    }

    if (m_xUseSquaredPageMode->get_state_changed_from_saved())
    {
        bool bIsSquaredPageModeFlag = m_xUseSquaredPageMode->get_active();
        pMod->ApplyDefaultPageMode( bIsSquaredPageModeFlag );
        if ( m_pWrtShell )
        {
            SwDoc* pDoc = m_pWrtShell->GetDoc();
            pDoc->SetDefaultPageMode( bIsSquaredPageModeFlag );
            m_pWrtShell->SetModified();
        }
        bRet = true;
    }

    return bRet;
}

void SwLoadOptPage::Reset( const SfxItemSet* rSet)
{
    const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref(false);
    const SfxPoolItem* pItem;

    if(SfxItemState::SET == rSet->GetItemState(FN_PARAM_WRTSHELL, false, &pItem))
        m_pWrtShell = static_cast<SwWrtShell*>(static_cast<const SwPtrItem*>(pItem)->GetValue());

    SwFieldUpdateFlags eFieldFlags = AUTOUPD_GLOBALSETTING;
    m_nOldLinkMode = GLOBALSETTING;
    if (m_pWrtShell)
    {
        eFieldFlags = m_pWrtShell->GetFieldUpdateFlags();
        m_nOldLinkMode = m_pWrtShell->GetLinkUpdMode();
    }
    if(GLOBALSETTING == m_nOldLinkMode)
        m_nOldLinkMode = pUsrPref->GetUpdateLinkMode();
    if(AUTOUPD_GLOBALSETTING == eFieldFlags)
        eFieldFlags = pUsrPref->GetFieldUpdateFlags();

    m_xAutoUpdateFields->set_active(eFieldFlags != AUTOUPD_OFF);
    m_xAutoUpdateCharts->set_active(eFieldFlags == AUTOUPD_FIELD_AND_CHARTS);

    switch (m_nOldLinkMode)
    {
        case NEVER:     m_xNeverRB->set_active(true);   break;
        case MANUAL:    m_xRequestRB->set_active(true); break;
        case AUTOMATIC: m_xAlwaysRB->set_active(true);  break;
    }

    m_xAutoUpdateFields->save_state();
    m_xAutoUpdateCharts->save_state();
    m_xMetricLB->set_active(-1);
    if ( rSet->GetItemState( SID_ATTR_METRIC ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = rSet->Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = static_cast<FieldUnit>(rItem.GetValue());

        for (sal_Int32 i = 0, nEntryCount = m_xMetricLB->get_count(); i < nEntryCount; ++i)
        {
            if (m_xMetricLB->get_id(i).toUInt32() == static_cast<sal_uInt32>(eFieldUnit))
            {
                m_xMetricLB->set_active(i);
                break;
            }
        }
        ::SetFieldUnit(*m_xTabMF, eFieldUnit);
    }
    m_xMetricLB->save_value();
    if(SfxItemState::SET == rSet->GetItemState(SID_ATTR_DEFTABSTOP, false, &pItem))
    {
        m_nLastTab = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        m_xTabMF->set_value(m_xTabMF->normalize(m_nLastTab), FieldUnit::TWIP);
    }
    m_xTabMF->save_value();

    //default page mode loading
    if(m_pWrtShell)
    {
        bool bSquaredPageMode = m_pWrtShell->GetDoc()->IsSquaredPageMode();
        m_xUseSquaredPageMode->set_active( bSquaredPageMode );
        m_xUseSquaredPageMode->save_state();
    }

    if(SfxItemState::SET == rSet->GetItemState(SID_ATTR_APPLYCHARUNIT, false, &pItem))
    {
        bool bUseCharUnit = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_xUseCharUnit->set_active(bUseCharUnit);
    }
    else
    {
        m_xUseCharUnit->set_active(pUsrPref->IsApplyCharUnit());
    }
    m_xUseCharUnit->save_state();

    m_xWordCountED->set_text(officecfg::Office::Writer::WordCount::AdditionalSeparators::get());
    m_xWordCountED->set_sensitive(!officecfg::Office::Writer::WordCount::AdditionalSeparators::isReadOnly());
    m_xWordCountED->save_value();
    m_xShowStandardizedPageCount->set_active(officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::get());
    m_xShowStandardizedPageCount->set_sensitive(!officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::isReadOnly());
    m_xShowStandardizedPageCount->save_state();
    m_xStandardizedPageSizeNF->set_value(officecfg::Office::Writer::WordCount::StandardizedPageSize::get());
    m_xStandardizedPageSizeNF->set_sensitive(!officecfg::Office::Writer::WordCount::StandardizedPageSize::isReadOnly());
    m_xStandardizedPageSizeNF->save_value();
    m_xStandardizedPageSizeNF->set_sensitive(m_xShowStandardizedPageCount->get_active());
}

IMPL_LINK_NOARG(SwLoadOptPage, MetricHdl, weld::ComboBox&, void)
{
    const sal_Int32 nMPos = m_xMetricLB->get_active();
    if(nMPos == -1)
        return;

    // Double-Cast for VA3.0
    FieldUnit eFieldUnit = static_cast<FieldUnit>(m_xMetricLB->get_id(nMPos).toUInt32());
    bool bModified = m_xTabMF->get_value_changed_from_saved();
    long nVal = bModified ?
        sal::static_int_cast<sal_Int32, sal_Int64 >( m_xTabMF->denormalize( m_xTabMF->get_value( FieldUnit::TWIP ) )) :
            m_nLastTab;
    ::SetFieldUnit( *m_xTabMF, eFieldUnit );
    m_xTabMF->set_value( m_xTabMF->normalize( nVal ), FieldUnit::TWIP );
    if (!bModified)
        m_xTabMF->save_value();
}

SwCaptionOptDlg::SwCaptionOptDlg(weld::Window* pParent, const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, &rSet, "modules/swriter/ui/captiondialog.ui",
                                   "CaptionDialog")
{
    // create TabPage
    SetTabPage(SwCaptionOptPage::Create(get_content_area(), this, &rSet));
}

SwCaptionPreview::SwCaptionPreview()
    : mbFontInitialized(false)
{
}

void SwCaptionPreview::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Wallpaper aBack(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.SetBackground(aBack);
    rRenderContext.SetFillColor(aBack.GetColor());
    rRenderContext.SetLineColor(aBack.GetColor());

    if (!mbFontInitialized)
    {
        maFont = rRenderContext.GetFont();
        maFont.SetFontHeight(maFont.GetFontHeight() * 120 / 100);
        mbFontInitialized = true;
    }
    rRenderContext.SetFont(maFont);
}

void SwCaptionPreview::SetPreviewText(const OUString& rText)
{
    if (rText != maText)
    {
        maText = rText;
        Invalidate();
    }
}

void SwCaptionPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(106 , 20), MapMode(MapUnit::MapAppFont)));
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
}

void SwCaptionPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    ApplySettings(rRenderContext);

    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0), GetOutputSizePixel()));
    rRenderContext.DrawText(Point(4, 6), maText);
}

IMPL_LINK(SwCaptionOptPage, TextFilterHdl, OUString&, rTest, bool)
{
    rTest = m_aTextFilter.filter(rTest);
    return true;
}

SwCaptionOptPage::SwCaptionOptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "modules/swriter/ui/optcaptionpage.ui", "OptCaptionPage", &rSet)
    , m_sSWTable(SwResId(STR_CAPTION_TABLE))
    , m_sSWFrame(SwResId(STR_CAPTION_FRAME))
    , m_sSWGraphic(SwResId(STR_CAPTION_GRAPHIC))
    , m_sOLE(SwResId(STR_CAPTION_OLE))
    , m_sBegin(SwResId(STR_CAPTION_BEGINNING))
    , m_sEnd(SwResId(STR_CAPTION_END))
    , m_sAbove(SwResId(STR_CAPTION_ABOVE))
    , m_sBelow(SwResId(STR_CAPTION_BELOW))
    , m_sNone(SwResId(SW_STR_NONE))
    , m_nPrevSelectedEntry(-1)
    , pMgr(new SwFieldMgr)
    , bHTMLMode(false)
    , m_aTextFilter(m_sNone)
    , m_xCheckLB(m_xBuilder->weld_tree_view("objects"))
    , m_xLbCaptionOrder(m_xBuilder->weld_combo_box("captionorder"))
    , m_xSettingsGroup(m_xBuilder->weld_widget("settings"))
    , m_xCategoryBox(m_xBuilder->weld_combo_box("category"))
    , m_xFormatText(m_xBuilder->weld_label("numberingft"))
    , m_xFormatBox(m_xBuilder->weld_combo_box("numbering"))
    , m_xNumberingSeparatorFT(m_xBuilder->weld_label("numseparatorft"))
    , m_xNumberingSeparatorED(m_xBuilder->weld_entry("numseparator"))
    , m_xTextText(m_xBuilder->weld_label("separatorft"))
    , m_xTextEdit(m_xBuilder->weld_entry("separator"))
    , m_xPosBox(m_xBuilder->weld_combo_box("position"))
    , m_xNumCapt(m_xBuilder->weld_widget("numcaption"))
    , m_xLbLevel(m_xBuilder->weld_combo_box("level"))
    , m_xEdDelim(m_xBuilder->weld_entry("chapseparator"))
    , m_xCategory(m_xBuilder->weld_widget("categoryformat"))
    , m_xCharStyleLB(m_xBuilder->weld_combo_box("charstyle"))
    , m_xApplyBorderCB(m_xBuilder->weld_check_button("applyborder"))
    , m_xPreview(new weld::CustomWeld(*m_xBuilder, "preview", m_aPreview))
{
    m_xCategoryBox->connect_entry_insert_text(LINK(this, SwCaptionOptPage, TextFilterHdl));

    m_xCheckLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_ABB, m_sIllustration);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_TABLE, m_sTable);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_FRAME, m_sText);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_DRAWING, m_sDrawing);

    SwWrtShell* pSh = ::GetActiveWrtShell();

    // m_xFormatBox
    sal_uInt16 nSelFormat = SVX_NUM_ARABIC;
    if (pSh)
    {
        for ( auto i = pMgr->GetFieldTypeCount(); i; )
        {
            SwFieldType* pFieldType = pMgr->GetFieldType(SwFieldIds::Unknown, --i);
            if (!pFieldType->GetName().isEmpty()
                && pFieldType->GetName() == m_xCategoryBox->get_active_text())
            {
                nSelFormat = static_cast<sal_uInt16>(static_cast<SwSetExpFieldType*>(pFieldType)->GetSeqFormat());
                break;
            }
        }

        ::FillCharStyleListBox( *m_xCharStyleLB, pSh->GetView().GetDocShell(), true, true );
    }

    const sal_uInt16 nCount = pMgr->GetFormatCount(SwFieldTypesEnum::Sequence, false);
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const sal_uInt16 nFormatId = pMgr->GetFormatId(SwFieldTypesEnum::Sequence, i);
        m_xFormatBox->append(OUString::number(nFormatId), pMgr->GetFormatStr(SwFieldTypesEnum::Sequence, i));
        if (nFormatId == nSelFormat)
            m_xFormatBox->set_active(i);
    }

    for (int i = 0; i < MAXLEVEL; ++i)
        m_xLbLevel->append_text(OUString::number(i + 1));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(": ");

    if (pSh)
    {
        SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(pMgr->GetFieldType(
                                            SwFieldIds::SetExp, m_xCategoryBox->get_active_text() ));
        if( pFieldType )
        {
            sDelim = pFieldType->GetDelimiter();
            nLvl = pFieldType->GetOutlineLvl();
        }
    }

    m_xLbLevel->set_active(nLvl < MAXLEVEL ? nLvl + 1 : 0);
    m_xEdDelim->set_text(sDelim);

    m_xCategoryBox->connect_changed(LINK(this, SwCaptionOptPage, ModifyComboHdl));
    Link<weld::Entry&,void> aLk = LINK(this, SwCaptionOptPage, ModifyEntryHdl);
    m_xNumberingSeparatorED->connect_changed(aLk);
    m_xTextEdit->connect_changed(aLk);

    m_xCategoryBox->connect_changed(LINK(this, SwCaptionOptPage, SelectHdl));
    m_xFormatBox->connect_changed(LINK(this, SwCaptionOptPage, SelectListBoxHdl));

    m_xLbCaptionOrder->connect_changed(LINK(this, SwCaptionOptPage, OrderHdl));

    m_xCheckLB->connect_changed(LINK(this, SwCaptionOptPage, ShowEntryHdl));
    m_xCheckLB->connect_toggled(LINK(this, SwCaptionOptPage, ToggleEntryHdl));
}

SwCaptionOptPage::~SwCaptionOptPage()
{
    DelUserData();
    pMgr.reset();
    m_xPreview.reset();
}

std::unique_ptr<SfxTabPage> SwCaptionOptPage::Create(weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rAttrSet)
{
    return std::make_unique<SwCaptionOptPage>(pPage, pController, *rAttrSet);
}

bool SwCaptionOptPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SaveEntry(m_xCheckLB->get_selected_index());    // apply current entry

    int nCheckCount = 0;
    for (int i = 0, nCount = m_xCheckLB->n_children(); i < nCount; ++i)
    {
        if (m_xCheckLB->get_toggle(i) == TRISTATE_TRUE)
            ++nCheckCount;
        InsCaptionOpt* pData = reinterpret_cast<InsCaptionOpt*>(m_xCheckLB->get_id(i).toInt64());
        bRet |= pModOpt->SetCapOption(bHTMLMode, pData);
    }

    pModOpt->SetInsWithCaption(bHTMLMode, nCheckCount > 0);

    int nPos = m_xLbCaptionOrder->get_active();
    pModOpt->SetCaptionOrderNumberingFirst(nPos == 1);

    return bRet;
}

void SwCaptionOptPage::Reset( const SfxItemSet* rSet)
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet->GetItemState(SID_HTML_MODE, false, &pItem))
    {
        bHTMLMode = 0 != (static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON);
    }

    DelUserData();
    m_xCheckLB->clear();   // remove all entries

    // Writer objects
    int nPos = 0;
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
    m_xCheckLB->set_text(nPos, m_sSWTable, 0);
    SetOptions(nPos++, TABLE_CAP);
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
    m_xCheckLB->set_text(nPos, m_sSWFrame, 0);
    SetOptions(nPos++, FRAME_CAP);
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
    m_xCheckLB->set_text(nPos, m_sSWGraphic, 0);
    SetOptions(nPos++, GRAPHIC_CAP);

    // get Productname and -version
    const OUString sWithoutVersion( utl::ConfigManager::getProductName() );
    const OUString sComplete(
        sWithoutVersion + " " +
        utl::ConfigManager::getProductVersion() );

    SvObjectServerList aObjS;
    aObjS.FillInsertObjects();
    aObjS.Remove( SvGlobalName( SO3_SW_CLASSID ) ); // remove Writer-ID

    for ( sal_uLong i = 0; i < aObjS.Count(); ++i )
    {
        const SvGlobalName &rOleId = aObjS[i].GetClassName();
        OUString sClass;
        if (rOleId == SvGlobalName(SO3_OUT_CLASSID))
            sClass = m_sOLE;
        else
            sClass = aObjS[i].GetHumanName();
        // don't show product version
        sClass = sClass.replaceFirst( sComplete, sWithoutVersion );
        m_xCheckLB->append();
        m_xCheckLB->set_toggle(nPos, TRISTATE_FALSE);
        m_xCheckLB->set_text(nPos, sClass, 0);
        SetOptions( nPos++, OLE_CAP, &rOleId );
    }
    m_xLbCaptionOrder->set_active(
        SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst() ? 1 : 0);
    m_xCheckLB->select(0);
    ShowEntryHdl(*m_xCheckLB);
}

void SwCaptionOptPage::SetOptions(const sal_uLong nPos,
        const SwCapObjType eObjType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    const InsCaptionOpt* pOpt = pModOpt->GetCapOption(bHTMLMode, eObjType, pOleId);

    if (pOpt)
    {
        InsCaptionOpt* pIns = new InsCaptionOpt(*pOpt);
        m_xCheckLB->set_id(nPos, OUString::number(reinterpret_cast<sal_Int64>(pIns)));
        m_xCheckLB->set_toggle(nPos, pOpt->UseCaption() ? TRISTATE_TRUE : TRISTATE_FALSE);
    }
    else
    {
        InsCaptionOpt* pIns = new InsCaptionOpt(eObjType, pOleId);
        m_xCheckLB->set_id(nPos, OUString::number(reinterpret_cast<sal_Int64>(pIns)));
    }
}

void SwCaptionOptPage::DelUserData()
{
    for (int i = 0, nCount = m_xCheckLB->n_children(); i < nCount; ++i)
    {
        delete reinterpret_cast<InsCaptionOpt*>(m_xCheckLB->get_id(i).toInt64());
        m_xCheckLB->set_id(i, "0");
    }
}

void SwCaptionOptPage::UpdateEntry(int nSelEntry)
{
    if (nSelEntry != -1)
    {
        bool bChecked = m_xCheckLB->get_toggle(nSelEntry) == TRISTATE_TRUE;

        m_xSettingsGroup->set_sensitive(bChecked);
        bool bNumSep = bChecked && m_xLbCaptionOrder->get_active() == 1;
        m_xNumberingSeparatorED->set_sensitive( bNumSep );
        m_xNumberingSeparatorFT->set_sensitive( bNumSep );

        m_xNumCapt->set_sensitive(bChecked);
        m_xCategory->set_sensitive(bChecked);
        m_xPreview->set_sensitive(bChecked);

        SwWrtShell *pSh = ::GetActiveWrtShell();

        InsCaptionOpt* pOpt = reinterpret_cast<InsCaptionOpt*>(m_xCheckLB->get_id(nSelEntry).toInt64());

        m_xCategoryBox->clear();
        m_xCategoryBox->append_text(m_sNone);
        if (pSh)
        {
            const size_t nCount = pMgr->GetFieldTypeCount();

            for (size_t i = 0; i < nCount; ++i)
            {
                SwFieldType *pType = pMgr->GetFieldType( SwFieldIds::Unknown, i );
                if( pType->Which() == SwFieldIds::SetExp &&
                    static_cast<SwSetExpFieldType *>( pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
                {
                    m_xCategoryBox->append_text(pType->GetName());
                }
            }
        }
        else
        {
            m_xCategoryBox->append_text(m_sIllustration);
            m_xCategoryBox->append_text(m_sTable);
            m_xCategoryBox->append_text(m_sText);
            m_xCategoryBox->append_text(m_sDrawing);
        }

        if (!pOpt->GetCategory().isEmpty())
        {
            if (m_xCategoryBox->find_text(pOpt->GetCategory()) == -1)
                m_xCategoryBox->insert_text(0, pOpt->GetCategory());
            m_xCategoryBox->set_active_text(pOpt->GetCategory());
        }
        else
            m_xCategoryBox->set_active_text(m_sNone);

        if (m_xCategoryBox->get_active_text().isEmpty())
        {
            sal_Int32 nPos = 0;
            switch(pOpt->GetObjType())
            {
                case OLE_CAP:
                case GRAPHIC_CAP:       nPos = 1;   break;
                case TABLE_CAP:         nPos = 2;   break;
                case FRAME_CAP:         nPos = 3;   break;
            }
            m_xCategoryBox->set_active(nPos);
        }

        for (sal_Int32 i = 0; i < m_xFormatBox->get_count(); i++)
        {
            if (pOpt->GetNumType() == m_xFormatBox->get_id(i).toUInt32())
            {
                m_xFormatBox->set_active(i);
                break;
            }
        }
        m_xTextEdit->set_text(pOpt->GetCaption());

        m_xPosBox->clear();
        switch (pOpt->GetObjType())
        {
            case GRAPHIC_CAP:
            case TABLE_CAP:
            case OLE_CAP:
                m_xPosBox->append_text(m_sAbove);
                m_xPosBox->append_text(m_sBelow);
                break;
            case FRAME_CAP:
                m_xPosBox->append_text(m_sBegin);
                m_xPosBox->append_text(m_sEnd);
                break;
        }
        m_xPosBox->set_active(pOpt->GetPos());

        sal_Int32 nLevelPos = ( pOpt->GetLevel() < MAXLEVEL ) ? pOpt->GetLevel() + 1 : 0;
        m_xLbLevel->set_active(nLevelPos);
        m_xEdDelim->set_text(pOpt->GetSeparator());
        m_xNumberingSeparatorED->set_text(pOpt->GetNumSeparator());
        if (!pOpt->GetCharacterStyle().isEmpty())
            m_xCharStyleLB->set_active_text(pOpt->GetCharacterStyle());
        else
            m_xCharStyleLB->set_active(0);
        m_xApplyBorderCB->set_sensitive(m_xCategoryBox->get_sensitive() &&
                pOpt->GetObjType() != TABLE_CAP && pOpt->GetObjType() != FRAME_CAP );
        m_xApplyBorderCB->set_active(pOpt->CopyAttributes());
    }

    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionOptPage, ShowEntryHdl, weld::TreeView&, void)
{
    if (m_nPrevSelectedEntry != -1)
        SaveEntry(m_nPrevSelectedEntry);
    UpdateEntry(m_xCheckLB->get_selected_index());
    m_nPrevSelectedEntry = m_xCheckLB->get_selected_index();
}

IMPL_LINK(SwCaptionOptPage, ToggleEntryHdl, const weld::TreeView::iter_col&, rRowCol, void)
{
    UpdateEntry(m_xCheckLB->get_iter_index_in_parent(rRowCol.first));
}

void SwCaptionOptPage::SaveEntry(int nEntry)
{
    if (nEntry == -1)
        return;

    InsCaptionOpt* pOpt = reinterpret_cast<InsCaptionOpt*>(m_xCheckLB->get_id(nEntry).toInt64());

    pOpt->UseCaption() = m_xCheckLB->get_toggle(nEntry) == TRISTATE_TRUE;
    const OUString aName(m_xCategoryBox->get_active_text());
    if (aName == m_sNone)
        pOpt->SetCategory("");
    else
        pOpt->SetCategory(comphelper::string::strip(aName, ' '));
    pOpt->SetNumType(m_xFormatBox->get_active_id().toUInt32());
    pOpt->SetCaption(m_xTextEdit->get_sensitive() ? m_xTextEdit->get_text() : OUString() );
    pOpt->SetPos(m_xPosBox->get_active());
    int nPos = m_xLbLevel->get_active();
    sal_Int32 nLevel = (nPos > 0) ? nPos - 1 : MAXLEVEL;
    pOpt->SetLevel(nLevel);
    pOpt->SetSeparator(m_xEdDelim->get_text());
    pOpt->SetNumSeparator(m_xNumberingSeparatorED->get_text());
    if (m_xCharStyleLB->get_active() == -1)
        pOpt->SetCharacterStyle("");
    else
        pOpt->SetCharacterStyle(m_xCharStyleLB->get_active_text());
    pOpt->CopyAttributes() = m_xApplyBorderCB->get_active();
}

void SwCaptionOptPage::ModifyHdl()
{
    const OUString sFieldTypeName = m_xCategoryBox->get_active_text();

    if (SfxSingleTabDialogController* pDlg = dynamic_cast<SfxSingleTabDialogController*>(GetDialogController()))
        pDlg->GetOKButton().set_sensitive(!sFieldTypeName.isEmpty());
    bool bEnable = m_xCategoryBox->get_sensitive() && sFieldTypeName != m_sNone;

    m_xFormatText->set_sensitive(bEnable);
    m_xFormatBox->set_sensitive(bEnable);
    m_xTextText->set_sensitive(bEnable);
    m_xTextEdit->set_sensitive(bEnable);

    InvalidatePreview();
}

IMPL_LINK_NOARG(SwCaptionOptPage, ModifyEntryHdl, weld::Entry&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionOptPage, ModifyComboHdl, weld::ComboBox&, void)
{
    ModifyHdl();
}

IMPL_LINK_NOARG(SwCaptionOptPage, SelectHdl, weld::ComboBox&, void)
{
    InvalidatePreview();
}

IMPL_LINK_NOARG(SwCaptionOptPage, SelectListBoxHdl, weld::ComboBox&, void)
{
    InvalidatePreview();
}

IMPL_LINK(SwCaptionOptPage, OrderHdl, weld::ComboBox&, rBox, void)
{
    InvalidatePreview();

    int nSelEntry = m_xCheckLB->get_selected_index();
    bool bChecked = false;
    if (nSelEntry != -1)
    {
        bChecked = m_xCheckLB->get_toggle(nSelEntry) == TRISTATE_TRUE;
    }

    int nPos = rBox.get_active();
    m_xNumberingSeparatorFT->set_sensitive(bChecked && nPos == 1);
    m_xNumberingSeparatorED->set_sensitive(bChecked && nPos == 1);
}

void SwCaptionOptPage::InvalidatePreview()
{
    OUString aStr;

    if (m_xCategoryBox->get_active_text() != m_sNone)
    {
        //#i61007# order of captions
        bool bOrderNumberingFirst = m_xLbCaptionOrder->get_active() == 1;
        // number
        const sal_uInt16 nNumFormat = m_xFormatBox->get_active_id().toUInt32();
        if (SVX_NUM_NUMBER_NONE != nNumFormat)
        {
            //#i61007# order of captions
            if( !bOrderNumberingFirst )
            {
                // category
                aStr += m_xCategoryBox->get_active_text() + " ";
            }

            SwWrtShell *pSh = ::GetActiveWrtShell();
            if (pSh)
            {
                SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(pMgr->GetFieldType(
                                                SwFieldIds::SetExp, m_xCategoryBox->get_active_text() ));
                if( pFieldType && pFieldType->GetOutlineLvl() < MAXLEVEL )
                {
                    sal_uInt8 nLvl = pFieldType->GetOutlineLvl();
                    SwNumberTree::tNumberVector aNumVector;
                    for( sal_uInt8 i = 0; i <= nLvl; ++i )
                        aNumVector.push_back(1);

                    const OUString sNumber( pSh->GetOutlineNumRule()->MakeNumString(
                                                            aNumVector, false ));
                    if( !sNumber.isEmpty() )
                        aStr += sNumber + pFieldType->GetDelimiter();
                }
            }

            switch( nNumFormat )
            {
                case SVX_NUM_CHARS_UPPER_LETTER:    aStr += "A"; break;
                case SVX_NUM_CHARS_UPPER_LETTER_N:  aStr += "A"; break;
                case SVX_NUM_CHARS_LOWER_LETTER:    aStr += "a"; break;
                case SVX_NUM_CHARS_LOWER_LETTER_N:  aStr += "a"; break;
                case SVX_NUM_ROMAN_UPPER:           aStr += "I"; break;
                case SVX_NUM_ROMAN_LOWER:           aStr += "i"; break;
                //case ARABIC:
                default:                    aStr += "1"; break;
            }
        }
        //#i61007# order of captions
        if( bOrderNumberingFirst )
        {
            aStr += m_xNumberingSeparatorED->get_text() + m_xCategoryBox->get_active_text();
        }
        aStr += m_xTextEdit->get_text();
    }
    m_aPreview.SetPreviewText(aStr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
