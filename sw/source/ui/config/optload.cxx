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
#include <helpid.h>
#include <uiitems.hxx>
#include <modcfg.hxx>
#include "swmodule.hxx"
#include "usrpref.hxx"
#include "wrtsh.hxx"
#include "linkenum.hxx"
#include <uitool.hxx>
#include <view.hxx>

#include "globals.hrc"
#include "cmdid.h"

#include "optload.hxx"
#include <svx/dlgutil.hxx>
#include <sfx2/htmlmode.hxx>
#include <fldmgr.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <svtools/insdlg.hxx>
#include <svtools/treelistentry.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/settings.hxx>

#include <comphelper/classids.hxx>
#include <unotools/configmgr.hxx>
#include <docsh.hxx>
#include <app.hrc>
#include <config.hrc>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>

#include <doc.hxx>
#include <svl/cjkoptions.hxx>

using namespace ::com::sun::star;

#include <svl/eitem.hxx>

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

    SvxStringArray aMetricArr( SW_RES( STR_ARR_METRIC ) );
    for ( sal_uInt32 i = 0; i < aMetricArr.Count(); ++i )
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
            {
                // use only these metrics
                sal_Int32 nPos = m_pMetricLB->InsertEntry( sMetric );
                m_pMetricLB->SetEntryData( nPos, reinterpret_cast<void*>((sal_IntPtr)eFUnit) );
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


VclPtr<SfxTabPage> SwLoadOptPage::Create( vcl::Window* pParent,
                                          const SfxItemSet* rAttrSet )
{
    return VclPtr<SwLoadOptPage>::Create(pParent, *rAttrSet );
}

IMPL_LINK_NOARG_TYPED(SwLoadOptPage, StandardizedPageCountCheckHdl, Button*, void)
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

    const sal_Int32 nMPos = m_pMetricLB->GetSelectEntryPos();
    if ( m_pMetricLB->IsValueChangedFromSaved() )
    {
        // Double-Cast for VA3.0
        const sal_uInt16 nFieldUnit = (sal_uInt16)reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( nMPos ));
        rSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    if(m_pTabMF->IsVisible() && m_pTabMF->IsValueChangedFromSaved())
    {
        rSet->Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    (sal_uInt16)m_pTabMF->Denormalize(m_pTabMF->GetValue(FUNIT_TWIP))));
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
        const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(rSet->Get( SID_ATTR_METRIC ));
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();

        for ( sal_Int32 i = 0; i < m_pMetricLB->GetEntryCount(); ++i )
        {
            if ( (int)reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( i )) == (int)eFieldUnit )
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
        m_pTabMF->SetValue(m_pTabMF->Normalize(m_nLastTab), FUNIT_TWIP);
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
    m_pWordCountED->SaveValue();
    m_pShowStandardizedPageCount->Check(officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::get());
    m_pShowStandardizedPageCount->SaveValue();
    m_pStandardizedPageSizeNF->SetValue(officecfg::Office::Writer::WordCount::StandardizedPageSize::get());
    m_pStandardizedPageSizeNF->SaveValue();
    m_pStandardizedPageSizeNF->Enable(m_pShowStandardizedPageCount->IsChecked());
}

IMPL_LINK_NOARG_TYPED(SwLoadOptPage, MetricHdl, ListBox&, void)
{
    const sal_Int32 nMPos = m_pMetricLB->GetSelectEntryPos();
    if(nMPos != LISTBOX_ENTRY_NOTFOUND)
    {
        // Double-Cast for VA3.0
        FieldUnit eFieldUnit = (FieldUnit)reinterpret_cast<sal_IntPtr>(m_pMetricLB->GetEntryData( nMPos ));
        bool bModified = m_pTabMF->IsModified();
        long nVal = bModified ?
            sal::static_int_cast<sal_Int32, sal_Int64 >( m_pTabMF->Denormalize( m_pTabMF->GetValue( FUNIT_TWIP ) )) :
                m_nLastTab;
        ::SetFieldUnit( *m_pTabMF, eFieldUnit );
        m_pTabMF->SetValue( m_pTabMF->Normalize( nVal ), FUNIT_TWIP );
        if(!bModified)
            m_pTabMF->ClearModifyFlag();
    }
}

SwCaptionOptDlg::SwCaptionOptDlg(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxSingleTabDialog(pParent, rSet, "CaptionDialog",
        "modules/swriter/ui/captiondialog.ui")
{
    // create TabPage
    SetTabPage(SwCaptionOptPage::Create(get_content_area(), &rSet));
}

SwCaptionPreview::SwCaptionPreview(vcl::Window* pParent, WinBits nStyle)
    : Window(pParent, nStyle)
    , mbFontInitialized(false)
{
    Init();
}

VCL_BUILDER_DECL_FACTORY(SwCaptionPreview)
{
    WinBits nBits = 0;
    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nBits |= WB_BORDER;
    rRet = VclPtr<SwCaptionPreview>::Create(pParent, nBits);
}

void SwCaptionPreview::Init()
{
    maDrawPos = Point(4, 6);
}

void SwCaptionPreview::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Wallpaper aBack(rRenderContext.GetSettings().GetStyleSettings().GetWindowColor());
    rRenderContext.SetBackground(aBack);
    rRenderContext.SetFillColor(aBack.GetColor());
    rRenderContext.SetLineColor(aBack.GetColor());

    if (!mbFontInitialized)
    {
        maFont = vcl::Font(rRenderContext.GetFont());
        maFont.SetFontHeight(maFont.GetFontHeight() * 120 / 100);
        mbFontInitialized = true;
    }
    rRenderContext.SetFont(maFont);

    SetBorderStyle(WindowBorderStyle::MONO);
}

void SwCaptionPreview::SetPreviewText(const OUString& rText)
{
    if (rText != maText)
    {
        maText = rText;
        Invalidate();
    }
}

Size SwCaptionPreview::GetOptimalSize() const
{
    return LogicToPixel(Size(106 , 20), MapMode(MAP_APPFONT));
}

void SwCaptionPreview::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    Window::Paint(rRenderContext, rRect);

    rRenderContext.DrawRect(Rectangle(Point(0, 0), GetSizePixel()));
    rRenderContext.DrawText(Point(4, 6), maText);
}

SwCaptionOptPage::SwCaptionOptPage(vcl::Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptCaptionPage", "modules/swriter/ui/optcaptionpage.ui", &rSet)
    , m_sSWTable(SW_RESSTR(STR_CAPTION_TABLE))
    , m_sSWFrame(SW_RESSTR(STR_CAPTION_FRAME))
    , m_sSWGraphic(SW_RESSTR(STR_CAPTION_GRAPHIC))
    , m_sOLE(SW_RESSTR(STR_CAPTION_OLE))
    , m_sBegin(SW_RESSTR(STR_CAPTION_BEGINNING))
    , m_sEnd(SW_RESSTR(STR_CAPTION_END))
    , m_sAbove(SW_RESSTR(STR_CAPTION_ABOVE))
    , m_sBelow(SW_RESSTR(STR_CAPTION_BELOW))
    , m_sNone(SW_RESSTR(SW_STR_NONE))
    , pMgr(new SwFieldMgr())
    , bHTMLMode(false)
{
    get(m_pCheckLB, "objects");
    get(m_pLbCaptionOrder, "captionorder");
    get(m_pPreview, "preview");
    get(m_pSettingsGroup, "settings");
    get(m_pCategoryBox, "category");
    get(m_pFormatText, "numberingft");
    get(m_pFormatBox, "numbering");
    get(m_pNumberingSeparatorFT, "numseparatorft");
    get(m_pNumberingSeparatorED, "numseparator");
    get(m_pTextText, "separatorft");
    get(m_pTextEdit, "separator");
    get(m_pPosBox, "position");
    get(m_pNumCapt, "numcaption");
    get(m_pLbLevel, "level");
    get(m_pEdDelim, "chapseparator");
    get(m_pCategory, "categoryformat");
    get(m_pCharStyleLB, "charstyle");
    get(m_pApplyBorderCB, "applyborder");

    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_ABB, m_sIllustration);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_TABLE, m_sTable);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_FRAME, m_sText);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_DRAWING, m_sDrawing);

    SwWrtShell* pSh = ::GetActiveWrtShell();

    // m_pFormatBox
    sal_uInt16 nSelFormat = SVX_NUM_ARABIC;
    if (pSh)
    {
        for ( auto i = pMgr->GetFieldTypeCount(); i; )
        {
            SwFieldType* pFieldType = pMgr->GetFieldType(USHRT_MAX, --i);
            if (pFieldType->GetName().equals(m_pCategoryBox->GetText()))
            {
                nSelFormat = (sal_uInt16)static_cast<SwSetExpFieldType*>(pFieldType)->GetSeqFormat();
                break;
            }
        }

        ::FillCharStyleListBox( *m_pCharStyleLB, pSh->GetView().GetDocShell(), true, true );
    }

    const sal_uInt16 nCount = pMgr->GetFormatCount(TYP_SEQFLD, false);
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        m_pFormatBox->InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        const sal_uInt16 nFormatId = pMgr->GetFormatId(TYP_SEQFLD, i);
        m_pFormatBox->SetEntryData( i, reinterpret_cast<void*>(nFormatId) );
        if( nFormatId == nSelFormat )
            m_pFormatBox->SelectEntryPos( i );
    }

    for (int i = 0; i < MAXLEVEL; ++i)
        m_pLbLevel->InsertEntry(OUString::number(i + 1));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(": ");

    if (pSh)
    {
        SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(pMgr->GetFieldType(
                                            RES_SETEXPFLD, m_pCategoryBox->GetText() ));
        if( pFieldType )
        {
            sDelim = pFieldType->GetDelimiter();
            nLvl = pFieldType->GetOutlineLvl();
        }
    }

    m_pLbLevel->SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    m_pEdDelim->SetText( sDelim );

    Link<Edit&,void> aLk = LINK( this, SwCaptionOptPage, ModifyHdl );
    m_pCategoryBox->SetModifyHdl( aLk );
    m_pNumberingSeparatorED->SetModifyHdl( aLk );
    m_pTextEdit->SetModifyHdl( aLk );

    m_pCategoryBox->SetSelectHdl( LINK(this, SwCaptionOptPage, SelectHdl) );
    m_pFormatBox->SetSelectHdl( LINK(this, SwCaptionOptPage, SelectListBoxHdl) );

    m_pLbCaptionOrder->SetSelectHdl( LINK(this, SwCaptionOptPage, OrderHdl));

    m_pCheckLB->SetSelectHdl( LINK(this, SwCaptionOptPage, ShowEntryHdl) );
    m_pCheckLB->SetCheckButtonHdl( LINK(this, SwCaptionOptPage, ShowEntryHdl) );
    m_pCheckLB->SetDeselectHdl( LINK(this, SwCaptionOptPage, SaveEntryHdl) );
}

SwCaptionOptPage::~SwCaptionOptPage()
{
    disposeOnce();
}

void SwCaptionOptPage::dispose()
{
    DelUserData();
    delete pMgr;
    m_pCheckLB.clear();
    m_pLbCaptionOrder.clear();
    m_pPreview.clear();
    m_pSettingsGroup.clear();
    m_pCategoryBox.clear();
    m_pFormatText.clear();
    m_pFormatBox.clear();
    m_pNumberingSeparatorFT.clear();
    m_pNumberingSeparatorED.clear();
    m_pTextText.clear();
    m_pTextEdit.clear();
    m_pPosBox.clear();
    m_pNumCapt.clear();
    m_pLbLevel.clear();
    m_pEdDelim.clear();
    m_pCategory.clear();
    m_pCharStyleLB.clear();
    m_pApplyBorderCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SwCaptionOptPage::Create( vcl::Window* pParent,
                                             const SfxItemSet* rAttrSet )
{
    return VclPtr<SwCaptionOptPage>::Create( pParent, *rAttrSet );
}

bool SwCaptionOptPage::FillItemSet( SfxItemSet* )
{
    bool bRet = false;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SaveEntry(m_pCheckLB->FirstSelected());    // apply current entry

    SvTreeListEntry* pEntry = m_pCheckLB->First();

    while (pEntry)
    {
        InsCaptionOpt* pData = static_cast<InsCaptionOpt*>(pEntry->GetUserData());
        bRet |= pModOpt->SetCapOption(bHTMLMode, pData);
        pEntry = m_pCheckLB->Next(pEntry);
    }

    sal_uLong nCheckCount = m_pCheckLB->GetCheckedEntryCount();
    pModOpt->SetInsWithCaption( bHTMLMode, nCheckCount > 0 );

    sal_Int32 nPos = m_pLbCaptionOrder->GetSelectEntryPos();
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
    m_pCheckLB->GetModel()->Clear();   // remove all entries

    // Writer objects
    sal_uLong nPos = 0;
    m_pCheckLB->InsertEntry(m_sSWTable);
    SetOptions(nPos++, TABLE_CAP);
    m_pCheckLB->InsertEntry(m_sSWFrame);
    SetOptions(nPos++, FRAME_CAP);
    m_pCheckLB->InsertEntry(m_sSWGraphic);
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
        m_pCheckLB->InsertEntry( sClass );
        SetOptions( nPos++, OLE_CAP, &rOleId );
    }
    m_pLbCaptionOrder->SelectEntryPos(
        SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst() ? 1 : 0);
    ModifyHdl(*m_pCategoryBox);
}

void SwCaptionOptPage::SetOptions(const sal_uLong nPos,
        const SwCapObjType eObjType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    const InsCaptionOpt* pOpt = pModOpt->GetCapOption(bHTMLMode, eObjType, pOleId);

    if (pOpt)
    {
        m_pCheckLB->SetEntryData(nPos, new InsCaptionOpt(*pOpt));
        m_pCheckLB->CheckEntryPos(nPos, pOpt->UseCaption());
    }
    else
        m_pCheckLB->SetEntryData(nPos, new InsCaptionOpt(eObjType, pOleId));
}

void SwCaptionOptPage::DelUserData()
{
    SvTreeListEntry* pEntry = m_pCheckLB->First();

    while (pEntry)
    {
        delete static_cast<InsCaptionOpt*>(pEntry->GetUserData());
        pEntry->SetUserData(nullptr);
        pEntry = m_pCheckLB->Next(pEntry);
    }
}

IMPL_LINK_NOARG_TYPED(SwCaptionOptPage, ShowEntryHdl, SvTreeListBox*, void)
{
    SvTreeListEntry* pSelEntry = m_pCheckLB->FirstSelected();

    if (pSelEntry)
    {
        bool bChecked = m_pCheckLB->IsChecked(m_pCheckLB->GetModel()->GetAbsPos(pSelEntry));

        m_pSettingsGroup->Enable(bChecked);
        bool bNumSep = bChecked && m_pLbCaptionOrder->GetSelectEntryPos() == 1;
        m_pNumberingSeparatorED->Enable( bNumSep );
        m_pNumberingSeparatorFT->Enable( bNumSep );

        m_pNumCapt->Enable(bChecked);
        m_pCategory->Enable(bChecked);
        m_pPreview->Enable(bChecked);

        SwWrtShell *pSh = ::GetActiveWrtShell();

        InsCaptionOpt* pOpt = static_cast<InsCaptionOpt*>(pSelEntry->GetUserData());

        m_pCategoryBox->Clear();
        m_pCategoryBox->InsertEntry(m_sNone, 0);
        if (pSh)
        {
            const size_t nCount = pMgr->GetFieldTypeCount();

            for (size_t i = 0; i < nCount; ++i)
            {
                SwFieldType *pType = pMgr->GetFieldType( USHRT_MAX, i );
                if( pType->Which() == RES_SETEXPFLD &&
                    static_cast<SwSetExpFieldType *>( pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
                    m_pCategoryBox->InsertSwEntry(SwBoxEntry(pType->GetName()));
            }
        }
        else
        {
            m_pCategoryBox->InsertSwEntry(SwBoxEntry(m_sIllustration));
            m_pCategoryBox->InsertSwEntry(SwBoxEntry(m_sTable));
            m_pCategoryBox->InsertSwEntry(SwBoxEntry(m_sText));
            m_pCategoryBox->InsertSwEntry(SwBoxEntry(m_sDrawing));
        }

        if(!pOpt->GetCategory().isEmpty())
            m_pCategoryBox->SetText(pOpt->GetCategory());
        else
            m_pCategoryBox->SetText(m_sNone);
        if (!pOpt->GetCategory().isEmpty() &&
            m_pCategoryBox->GetEntryPos(pOpt->GetCategory()) == COMBOBOX_ENTRY_NOTFOUND)
            m_pCategoryBox->InsertEntry(pOpt->GetCategory(), 0);
        if (m_pCategoryBox->GetText().isEmpty())
        {
            sal_Int32 nPos = 0;
            switch(pOpt->GetObjType())
            {
                case OLE_CAP:
                case GRAPHIC_CAP:       nPos = 1;   break;
                case TABLE_CAP:         nPos = 2;   break;
                case FRAME_CAP:         nPos = 3;   break;
            }
            m_pCategoryBox->SetText(m_pCategoryBox->GetSwEntry(nPos).GetName());
        }

        for (sal_Int32 i = 0; i < m_pFormatBox->GetEntryCount(); i++)
        {
            if (pOpt->GetNumType() == (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFormatBox->GetEntryData(i)))
            {
                m_pFormatBox->SelectEntryPos(i);
                break;
            }
        }
        m_pTextEdit->SetText(pOpt->GetCaption());

        m_pPosBox->Clear();
        switch (pOpt->GetObjType())
        {
            case GRAPHIC_CAP:
            case TABLE_CAP:
            case OLE_CAP:
                m_pPosBox->InsertEntry(m_sAbove);
                m_pPosBox->InsertEntry(m_sBelow);
                break;
            case FRAME_CAP:
                m_pPosBox->InsertEntry(m_sBegin);
                m_pPosBox->InsertEntry(m_sEnd);
                break;
        }
        m_pPosBox->SelectEntryPos(pOpt->GetPos());
        m_pPosBox->Enable( m_pPosBox->IsEnabled() );
        m_pPosBox->SelectEntryPos(pOpt->GetPos());

        sal_Int32 nLevelPos = ( pOpt->GetLevel() < MAXLEVEL ) ? pOpt->GetLevel() + 1 : 0;
        m_pLbLevel->SelectEntryPos( nLevelPos );
        m_pEdDelim->SetText(pOpt->GetSeparator());
        m_pNumberingSeparatorED->SetText( pOpt->GetNumSeparator() );
        if(!pOpt->GetCharacterStyle().isEmpty())
            m_pCharStyleLB->SelectEntry( pOpt->GetCharacterStyle() );
        else
            m_pCharStyleLB->SelectEntryPos( 0 );
        m_pApplyBorderCB->Enable( m_pCategoryBox->IsEnabled() &&
                pOpt->GetObjType() != TABLE_CAP && pOpt->GetObjType() != FRAME_CAP );
        m_pApplyBorderCB->Check( pOpt->CopyAttributes() );
    }

    ModifyHdl(*m_pCategoryBox);
}

IMPL_LINK_NOARG_TYPED(SwCaptionOptPage, SaveEntryHdl, SvTreeListBox*, void)
{
    SvTreeListEntry* pEntry = m_pCheckLB->GetHdlEntry();

    if (pEntry) // save all
        SaveEntry(pEntry);
}

void SwCaptionOptPage::SaveEntry(SvTreeListEntry* pEntry)
{
    if (pEntry)
    {
        InsCaptionOpt* pOpt = static_cast<InsCaptionOpt*>(pEntry->GetUserData());

        pOpt->UseCaption() = m_pCheckLB->IsChecked(m_pCheckLB->GetModel()->GetAbsPos(pEntry));
        const OUString aName( m_pCategoryBox->GetText() );
        if (aName == m_sNone)
            pOpt->SetCategory("");
        else
            pOpt->SetCategory(comphelper::string::strip(aName, ' '));
        pOpt->SetNumType((sal_uInt16)reinterpret_cast<sal_uLong>(m_pFormatBox->GetSelectEntryData()));
        pOpt->SetCaption(m_pTextEdit->IsEnabled() ? m_pTextEdit->GetText() : OUString() );
        pOpt->SetPos(m_pPosBox->GetSelectEntryPos());
        sal_Int32 nPos = m_pLbLevel->GetSelectEntryPos();
        sal_Int32 nLevel = ( nPos > 0 && nPos != LISTBOX_ENTRY_NOTFOUND ) ? nPos - 1 : MAXLEVEL;
        pOpt->SetLevel(nLevel);
        pOpt->SetSeparator(m_pEdDelim->GetText());
        pOpt->SetNumSeparator( m_pNumberingSeparatorED->GetText());
        if(!m_pCharStyleLB->GetSelectEntryPos())
            pOpt->SetCharacterStyle("");
        else
            pOpt->SetCharacterStyle(m_pCharStyleLB->GetSelectEntry());
        pOpt->CopyAttributes() = m_pApplyBorderCB->IsChecked();
    }
}

IMPL_LINK_NOARG_TYPED(SwCaptionOptPage, ModifyHdl, Edit&, void)
{
    const OUString sFieldTypeName = m_pCategoryBox->GetText();

    SfxSingleTabDialog *pDlg = dynamic_cast<SfxSingleTabDialog*>(GetParentDialog());
    PushButton *pBtn = pDlg ? pDlg->GetOKButton() : nullptr;
    if (pBtn)
        pBtn->Enable(!sFieldTypeName.isEmpty());
    bool bEnable = m_pCategoryBox->IsEnabled() && sFieldTypeName != m_sNone;

    m_pFormatText->Enable(bEnable);
    m_pFormatBox->Enable(bEnable);
    m_pTextText->Enable(bEnable);
    m_pTextEdit->Enable(bEnable);

    InvalidatePreview();
}

IMPL_LINK_NOARG_TYPED(SwCaptionOptPage, SelectHdl, ComboBox&, void)
{
    InvalidatePreview();
}

IMPL_LINK_NOARG_TYPED(SwCaptionOptPage, SelectListBoxHdl, ListBox&, void)
{
    InvalidatePreview();
}

IMPL_LINK_TYPED( SwCaptionOptPage, OrderHdl, ListBox&, rBox, void )
{
    InvalidatePreview();

    SvTreeListEntry* pSelEntry = m_pCheckLB->FirstSelected();
    bool bChecked = false;
    if (pSelEntry)
    {
        bChecked = m_pCheckLB->IsChecked(m_pCheckLB->GetModel()->GetAbsPos(pSelEntry));
    }

    sal_Int32 nPos = rBox.GetSelectEntryPos();
    m_pNumberingSeparatorFT->Enable( bChecked && nPos == 1 );
    m_pNumberingSeparatorED->Enable( bChecked && nPos == 1 );
}

void SwCaptionOptPage::InvalidatePreview()
{
    OUString aStr;

    if( m_pCategoryBox->GetText() != m_sNone)
    {
        //#i61007# order of captions
        bool bOrderNumberingFirst = m_pLbCaptionOrder->GetSelectEntryPos() == 1;
        // number
        const sal_uInt16 nNumFormat = (sal_uInt16)reinterpret_cast<sal_uLong>(m_pFormatBox->GetEntryData(
                                        m_pFormatBox->GetSelectEntryPos() ));
        if( SVX_NUM_NUMBER_NONE != nNumFormat )
        {
            //#i61007# order of captions
            if( !bOrderNumberingFirst )
            {
                // category
                aStr += m_pCategoryBox->GetText() + " ";
            }

            SwWrtShell *pSh = ::GetActiveWrtShell();
            if (pSh)
            {
                SwSetExpFieldType* pFieldType = static_cast<SwSetExpFieldType*>(pMgr->GetFieldType(
                                                RES_SETEXPFLD, m_pCategoryBox->GetText() ));
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
            aStr += m_pNumberingSeparatorED->GetText() + m_pCategoryBox->GetText();
        }
        aStr += m_pTextEdit->GetText();
    }
    m_pPreview->SetPreviewText(aStr);
}

// Description: ComboBox without Spaces
void CaptionComboBox::KeyInput(const KeyEvent& rEvt)
{
    if (rEvt.GetKeyCode().GetCode() != KEY_SPACE)
        SwComboBox::KeyInput(rEvt);
}

VCL_BUILDER_DECL_FACTORY(CaptionComboBox)
{
    WinBits nBits = WB_CLIPCHILDREN|WB_LEFT|WB_VCENTER|WB_3DLOOK;
    bool bDropdown = VclBuilder::extractDropdown(rMap);
    if (bDropdown)
        nBits |= WB_DROPDOWN;
    VclPtrInstance<CaptionComboBox> pComboBox(pParent, nBits);
    pComboBox->EnableAutoSize(true);
    rRet = pComboBox;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
