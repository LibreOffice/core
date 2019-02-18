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

#include <globals.hrc>
#include <strings.hrc>
#include <optload.hrc>
#include <cmdid.h>

#include <optload.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/htmlmode.hxx>
#include <fldmgr.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <svtools/insdlg.hxx>
#include <vcl/treelistentry.hxx>
#include <tools/resary.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>

#include <comphelper/classids.hxx>
#include <unotools/configmgr.hxx>
#include <docsh.hxx>
#include <app.hrc>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>

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

SwLoadOptPage::SwLoadOptPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptGeneralPage", "modules/swriter/ui/optgeneralpage.ui", &rSet)
    , m_pWrtShell(nullptr)
    , m_nLastTab(0)
    , m_nOldLinkMode(MANUAL)
{
    get(m_pAlwaysRB, "always");
    get(m_pRequestRB, "onrequest");
    get(m_pNeverRB, "never");
    get(m_pAutoUpdateFields, "updatefields");
    get(m_pAutoUpdateCharts, "updatecharts");
    get(m_pMetricLB, "metric");
    get(m_pTabFT, "tablabel");
    get(m_pTabMF, "tab");
    get(m_pUseSquaredPageMode, "squaremode");
    get(m_pUseCharUnit, "usecharunit");
    get(m_pWordCountED, "wordcount");
    get(m_pShowStandardizedPageCount, "standardizedpageshow");
    get(m_pStandardizedPageSizeNF, "standardpagesize");

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
                sal_Int32 nPos = m_pMetricLB->InsertEntry( sMetric );
                m_pMetricLB->SetEntryData( nPos, reinterpret_cast<void*>(static_cast<sal_IntPtr>(eFUnit)) );
                break;
            }
            default:; //prevent warning
        }
    }
    m_pMetricLB->SetSelectHdl(LINK(this, SwLoadOptPage, MetricHdl));

    const SfxPoolItem* pItem;
    if (SfxItemState::SET == rSet.GetItemState(SID_HTML_MODE, false, &pItem)
        && static_cast<const SfxUInt16Item*>(pItem)->GetValue() & HTMLMODE_ON)
    {
        m_pTabFT->Hide();
        m_pTabMF->Hide();
    }

    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsAsianTypographyEnabled())
    {
        m_pUseSquaredPageMode->Hide();
        m_pUseCharUnit->Hide();
    }

    Link<Button*,void> aLink = LINK(this, SwLoadOptPage, StandardizedPageCountCheckHdl);
    m_pShowStandardizedPageCount->SetClickHdl(aLink);
}

SwLoadOptPage::~SwLoadOptPage()
{
    disposeOnce();
}

void SwLoadOptPage::dispose()
{
    m_pAlwaysRB.clear();
    m_pRequestRB.clear();
    m_pNeverRB.clear();
    m_pAutoUpdateFields.clear();
    m_pAutoUpdateCharts.clear();
    m_pMetricLB.clear();
    m_pTabFT.clear();
    m_pTabMF.clear();
    m_pUseSquaredPageMode.clear();
    m_pUseCharUnit.clear();
    m_pWordCountED.clear();
    m_pShowStandardizedPageCount.clear();
    m_pStandardizedPageSizeNF.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> SwLoadOptPage::Create( TabPageParent pParent,
                                          const SfxItemSet* rAttrSet )
{
    return VclPtr<SwLoadOptPage>::Create(pParent.pParent, *rAttrSet );
}

IMPL_LINK_NOARG(SwLoadOptPage, StandardizedPageCountCheckHdl, Button*, void)
{
    m_pStandardizedPageSizeNF->Enable(m_pShowStandardizedPageCount->IsChecked());
}

bool SwLoadOptPage::FillItemSet( SfxItemSet* rSet )
{
    bool bRet = false;
    SwModule* pMod = SW_MOD();

    sal_Int32 nNewLinkMode = AUTOMATIC;
    if (m_pNeverRB->IsChecked())
        nNewLinkMode = NEVER;
    else if (m_pRequestRB->IsChecked())
        nNewLinkMode = MANUAL;

    SwFieldUpdateFlags eFieldFlags = m_pAutoUpdateFields->IsChecked() ?
        m_pAutoUpdateCharts->IsChecked() ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF;

    if(m_pAutoUpdateFields->IsValueChangedFromSaved() ||
       m_pAutoUpdateCharts->IsValueChangedFromSaved())
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

    const sal_Int32 nMPos = m_pMetricLB->GetSelectedEntryPos();
    if ( m_pMetricLB->IsValueChangedFromSaved() )
    {
        // Double-Cast for VA3.0
        const sal_uInt16 nFieldUnit = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( nMPos )));
        rSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    if(m_pTabMF->IsVisible() && m_pTabMF->IsValueChangedFromSaved())
    {
        rSet->Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    static_cast<sal_uInt16>(m_pTabMF->Denormalize(m_pTabMF->GetValue(FieldUnit::TWIP)))));
        bRet = true;
    }

    bool bIsUseCharUnitFlag = m_pUseCharUnit->IsChecked();
    SvtCJKOptions aCJKOptions;
    bIsUseCharUnitFlag = bIsUseCharUnitFlag && aCJKOptions.IsAsianTypographyEnabled();
    if( (bIsUseCharUnitFlag ? 1 : 0) != m_pUseCharUnit->GetSavedValue())
    {
        rSet->Put(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, bIsUseCharUnitFlag ));
        bRet = true;
    }

    if( m_pWordCountED->IsValueChangedFromSaved() )
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::AdditionalSeparators::set(m_pWordCountED->GetText(), batch);
        batch->commit();
        bRet = true;
    }

    if (m_pShowStandardizedPageCount->GetState() != m_pShowStandardizedPageCount->GetSavedValue())
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::set(
          m_pShowStandardizedPageCount->IsChecked(),
          batch);
        batch->commit();
        bRet = true;
    }

    if (m_pStandardizedPageSizeNF->GetText() != m_pStandardizedPageSizeNF->GetSavedValue())
    {
        std::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::StandardizedPageSize::set(
          m_pStandardizedPageSizeNF->GetValue(),
          batch);
        batch->commit();
        bRet = true;
    }

    bool bIsSquaredPageModeFlag = m_pUseSquaredPageMode->IsChecked();
    if ( bIsSquaredPageModeFlag != static_cast<bool>(m_pUseSquaredPageMode->GetSavedValue()) )
    {
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

    m_pAutoUpdateFields->Check(eFieldFlags != AUTOUPD_OFF);
    m_pAutoUpdateCharts->Check(eFieldFlags == AUTOUPD_FIELD_AND_CHARTS);

    switch (m_nOldLinkMode)
    {
        case NEVER:     m_pNeverRB->Check();   break;
        case MANUAL:    m_pRequestRB->Check(); break;
        case AUTOMATIC: m_pAlwaysRB->Check();  break;
    }

    m_pAutoUpdateFields->SaveValue();
    m_pAutoUpdateCharts->SaveValue();
    m_pMetricLB->SetNoSelection();
    if ( rSet->GetItemState( SID_ATTR_METRIC ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = rSet->Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = static_cast<FieldUnit>(rItem.GetValue());

        for ( sal_Int32 i = 0; i < m_pMetricLB->GetEntryCount(); ++i )
        {
            if ( static_cast<int>(reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( i ))) == static_cast<int>(eFieldUnit) )
            {
                m_pMetricLB->SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(*m_pTabMF, eFieldUnit);
    }
    m_pMetricLB->SaveValue();
    if(SfxItemState::SET == rSet->GetItemState(SID_ATTR_DEFTABSTOP, false, &pItem))
    {
        m_nLastTab = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
        m_pTabMF->SetValue(m_pTabMF->Normalize(m_nLastTab), FieldUnit::TWIP);
    }
    m_pTabMF->SaveValue();

    //default page mode loading
    if(m_pWrtShell)
    {
        bool bSquaredPageMode = m_pWrtShell->GetDoc()->IsSquaredPageMode();
        m_pUseSquaredPageMode->Check( bSquaredPageMode );
        m_pUseSquaredPageMode->SaveValue();
    }

    if(SfxItemState::SET == rSet->GetItemState(SID_ATTR_APPLYCHARUNIT, false, &pItem))
    {
        bool bUseCharUnit = static_cast<const SfxBoolItem*>(pItem)->GetValue();
        m_pUseCharUnit->Check(bUseCharUnit);
    }
    else
    {
        m_pUseCharUnit->Check(pUsrPref->IsApplyCharUnit());
    }
    m_pUseCharUnit->SaveValue();

    m_pWordCountED->SetText(officecfg::Office::Writer::WordCount::AdditionalSeparators::get());
    m_pWordCountED->Enable(!officecfg::Office::Writer::WordCount::AdditionalSeparators::isReadOnly());
    m_pWordCountED->SaveValue();
    m_pShowStandardizedPageCount->Check(officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::get());
    m_pShowStandardizedPageCount->Enable(!officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::isReadOnly());
    m_pShowStandardizedPageCount->SaveValue();
    m_pStandardizedPageSizeNF->SetValue(officecfg::Office::Writer::WordCount::StandardizedPageSize::get());
    m_pStandardizedPageSizeNF->Enable(!officecfg::Office::Writer::WordCount::StandardizedPageSize::isReadOnly());
    m_pStandardizedPageSizeNF->SaveValue();
    m_pStandardizedPageSizeNF->Enable(m_pShowStandardizedPageCount->IsChecked());
}

IMPL_LINK_NOARG(SwLoadOptPage, MetricHdl, ListBox&, void)
{
    const sal_Int32 nMPos = m_pMetricLB->GetSelectedEntryPos();
    if(nMPos != LISTBOX_ENTRY_NOTFOUND)
    {
        // Double-Cast for VA3.0
        FieldUnit eFieldUnit = static_cast<FieldUnit>(reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( nMPos )));
        bool bModified = m_pTabMF->IsModified();
        long nVal = bModified ?
            sal::static_int_cast<sal_Int32, sal_Int64 >( m_pTabMF->Denormalize( m_pTabMF->GetValue( FieldUnit::TWIP ) )) :
                m_nLastTab;
        ::SetFieldUnit( *m_pTabMF, eFieldUnit );
        m_pTabMF->SetValue( m_pTabMF->Normalize( nVal ), FieldUnit::TWIP );
        if(!bModified)
            m_pTabMF->ClearModifyFlag();
    }
}

SwCaptionOptDlg::SwCaptionOptDlg(weld::Window* pParent, const SfxItemSet& rSet)
    : SfxSingleTabDialogController(pParent, rSet, "modules/swriter/ui/captiondialog.ui",
                                   "CaptionDialog")
{
    // create TabPage
    TabPageParent aParent(get_content_area(), this);
    SetTabPage(SwCaptionOptPage::Create(aParent, &rSet));
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

SwCaptionOptPage::SwCaptionOptPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/optcaptionpage.ui", "OptCaptionPage", &rSet)
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

    std::vector<int> aWidths;
    aWidths.push_back(m_xCheckLB->get_checkbox_column_width());
    m_xCheckLB->set_column_fixed_widths(aWidths);

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

    const sal_uInt16 nCount = pMgr->GetFormatCount(TYP_SEQFLD, false);
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const sal_uInt16 nFormatId = pMgr->GetFormatId(TYP_SEQFLD, i);
        m_xFormatBox->append(OUString::number(nFormatId), pMgr->GetFormatStr(TYP_SEQFLD, i));
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
    disposeOnce();
}

void SwCaptionOptPage::dispose()
{
    DelUserData();
    pMgr.reset();
    m_xPreview.reset();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwCaptionOptPage::Create(TabPageParent pParent,
                                            const SfxItemSet* rAttrSet)
{
    return VclPtr<SwCaptionOptPage>::Create(pParent, *rAttrSet);
}

bool SwCaptionOptPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SaveEntry(m_xCheckLB->get_selected_index());    // apply current entry

    int nCheckCount = 0;
    for (int i = 0, nCount = m_xCheckLB->n_children(); i < nCount; ++i)
    {
        if (m_xCheckLB->get_toggle(i, 0))
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
    m_xCheckLB->set_toggle(nPos, false, 0);
    m_xCheckLB->set_text(nPos, m_sSWTable, 1);
    SetOptions(nPos++, TABLE_CAP);
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, false, 0);
    m_xCheckLB->set_text(nPos, m_sSWFrame, 1);
    SetOptions(nPos++, FRAME_CAP);
    m_xCheckLB->append();
    m_xCheckLB->set_toggle(nPos, false, 0);
    m_xCheckLB->set_text(nPos, m_sSWGraphic, 1);
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
        m_xCheckLB->set_toggle(nPos, false, 0);
        m_xCheckLB->set_text(nPos, sClass, 1);
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
        m_xCheckLB->set_toggle(nPos, pOpt->UseCaption(), 0);
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
        bool bChecked = m_xCheckLB->get_toggle(nSelEntry, 0);

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

IMPL_LINK(SwCaptionOptPage, ToggleEntryHdl, const row_col&, rRowCol, void)
{
    UpdateEntry(rRowCol.first);
}

void SwCaptionOptPage::SaveEntry(int nEntry)
{
    if (nEntry == -1)
        return;

    InsCaptionOpt* pOpt = reinterpret_cast<InsCaptionOpt*>(m_xCheckLB->get_id(nEntry).toInt64());

    pOpt->UseCaption() = m_xCheckLB->get_toggle(nEntry, 0);
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
        bChecked = m_xCheckLB->get_toggle(nSelEntry, 0);
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
