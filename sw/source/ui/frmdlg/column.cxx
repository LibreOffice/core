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

#include <cstdint>

#include <column.hxx>

#include <hintids.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <sfx2/htmlmode.hxx>
#include <svx/colorbox.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svl/ctloptions.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <swmodule.hxx>
#include <sal/macros.h>

#include <globals.hrc>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <viewopt.hxx>
#include <format.hxx>
#include <frmmgr.hxx>
#include <frmdlg.hxx>
#include <colmgr.hxx>
#include <prcntfld.hxx>
#include <paratr.hxx>
#include <strings.hrc>
#include <section.hxx>
#include <docary.hxx>
#include <pagedesc.hxx>

//to match associated data in ColumnPage.ui
#define LISTBOX_SELECTION       0
#define LISTBOX_SECTION         1
#define LISTBOX_SECTIONS        2
#define LISTBOX_PAGE            3
#define LISTBOX_FRAME           4

using namespace ::com::sun::star;

#define FRAME_FORMAT_WIDTH 1000

// static data
static const sal_uInt16 nVisCols = 3;

static bool IsMarkInSameSection( SwWrtShell& rWrtSh, const SwSection* pSect )
{
    rWrtSh.SwapPam();
    bool bRet = pSect == rWrtSh.GetCurrSection();
    rWrtSh.SwapPam();
    return bRet;
}

SwColumnDlg::SwColumnDlg(weld::Window* pParent, SwWrtShell& rSh)
    : SfxDialogController(pParent, "modules/swriter/ui/columndialog.ui", "ColumnDialog")
    , m_rWrtShell(rSh)
    , m_pFrameSet(nullptr)
    , m_nOldSelection(0)
    , m_nSelectionWidth(0)
    , m_bPageChanged(false)
    , m_bSectionChanged(false)
    , m_bSelSectionChanged(false)
    , m_bFrameChanged(false)
    , m_xContentArea(m_xDialog->weld_content_area())
    , m_xOkButton(m_xBuilder->weld_button("ok"))
{
    SwRect aRect;
    m_rWrtShell.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);

    m_nSelectionWidth = aRect.Width();

    SfxItemSet* pColPgSet = nullptr;
    static sal_uInt16 const aSectIds[] = { RES_COL, RES_COL,
                                                RES_FRM_SIZE, RES_FRM_SIZE,
                                                RES_COLUMNBALANCE, RES_FRAMEDIR,
                                                0 };

    const SwSection* pCurrSection = m_rWrtShell.GetCurrSection();
    const sal_uInt16 nFullSectCnt = m_rWrtShell.GetFullSelectedSectionCount();
    if( pCurrSection && ( !m_rWrtShell.HasSelection() || 0 != nFullSectCnt ))
    {
        m_nSelectionWidth = rSh.GetSectionWidth(*pCurrSection->GetFormat());
        if ( !m_nSelectionWidth )
            m_nSelectionWidth = USHRT_MAX;
        m_pSectionSet.reset( new SfxItemSet( m_rWrtShell.GetAttrPool(), aSectIds ) );
        m_pSectionSet->Put( pCurrSection->GetFormat()->GetAttrSet() );
        pColPgSet = m_pSectionSet.get();
    }

    if( m_rWrtShell.HasSelection() && m_rWrtShell.IsInsRegionAvailable() &&
        ( !pCurrSection || ( 1 != nFullSectCnt &&
            IsMarkInSameSection( m_rWrtShell, pCurrSection ) )))
    {
        m_pSelectionSet.reset( new SfxItemSet( m_rWrtShell.GetAttrPool(), aSectIds ) );
        pColPgSet = m_pSelectionSet.get();
    }

    if( m_rWrtShell.GetFlyFrameFormat() )
    {
        const SwFrameFormat* pFormat = rSh.GetFlyFrameFormat() ;
        m_pFrameSet = new SfxItemSet(m_rWrtShell.GetAttrPool(), aSectIds );
        m_pFrameSet->Put(pFormat->GetFrameSize());
        m_pFrameSet->Put(pFormat->GetCol());
        pColPgSet = m_pFrameSet;
    }

    const SwPageDesc* pPageDesc = m_rWrtShell.GetSelectedPageDescs();
    if( pPageDesc )
    {
        m_pPageSet.reset( new SfxItemSet(
            m_rWrtShell.GetAttrPool(),
            svl::Items<
                RES_FRM_SIZE, RES_FRM_SIZE,
                RES_LR_SPACE, RES_LR_SPACE,
                RES_COL, RES_COL>{}) );

        const SwFrameFormat &rFormat = pPageDesc->GetMaster();
        m_nPageWidth = rFormat.GetFrameSize().GetSize().Width();

        const SvxLRSpaceItem& rLRSpace = rFormat.GetLRSpace();
        const SvxBoxItem& rBox = rFormat.GetBox();
        m_nPageWidth -= rLRSpace.GetLeft() + rLRSpace.GetRight() + rBox.GetSmallestDistance();

        m_pPageSet->Put(rFormat.GetCol());
        m_pPageSet->Put(rFormat.GetLRSpace());
        pColPgSet = m_pPageSet.get();
    }

    assert(pColPgSet);

    // create TabPage
    m_pTabPage = static_cast<SwColumnPage*>(SwColumnPage::Create(TabPageParent(m_xContentArea.get(), this), pColPgSet).get());
    m_pTabPage->GetApplyLabel()->show();
    weld::ComboBox* pApplyToLB = m_pTabPage->GetApplyComboBox();
    pApplyToLB->show();

    if (pCurrSection && (!m_rWrtShell.HasSelection() || 0 != nFullSectCnt))
    {
        pApplyToLB->remove_id(1 >= nFullSectCnt ? OUString::number(LISTBOX_SECTIONS) : OUString::number(LISTBOX_SECTION));
    }
    else
    {
        pApplyToLB->remove_id(OUString::number(LISTBOX_SECTION));
        pApplyToLB->remove_id(OUString::number(LISTBOX_SECTIONS));
    }

    if (!( m_rWrtShell.HasSelection() && m_rWrtShell.IsInsRegionAvailable() &&
        ( !pCurrSection || ( 1 != nFullSectCnt &&
            IsMarkInSameSection( m_rWrtShell, pCurrSection ) ))))
        pApplyToLB->remove_id(OUString::number(LISTBOX_SELECTION));

    if (!m_rWrtShell.GetFlyFrameFormat())
        pApplyToLB->remove_id(OUString::number(LISTBOX_FRAME));

    const int nPagePos = pApplyToLB->find_id(OUString::number(LISTBOX_PAGE));
    if (m_pPageSet && pPageDesc)
    {
        const OUString sPageStr = pApplyToLB->get_text(nPagePos) + pPageDesc->GetName();
        pApplyToLB->remove(nPagePos);
        OUString sId(OUString::number(LISTBOX_PAGE));
        pApplyToLB->insert(nPagePos, sPageStr, &sId, nullptr, nullptr);
    }
    else
        pApplyToLB->remove( nPagePos );

    pApplyToLB->set_active(0);
    ObjectHdl(nullptr);

    pApplyToLB->connect_changed(LINK(this, SwColumnDlg, ObjectListBoxHdl));
    m_xOkButton->connect_clicked(LINK(this, SwColumnDlg, OkHdl));
    //#i80458# if no columns can be set then disable OK
    if (!pApplyToLB->get_count())
        m_xOkButton->set_sensitive(false);
    //#i97810# set focus to the TabPage
    m_pTabPage->ActivateColumnControl();
    m_pTabPage->Show();
}

SwColumnDlg::~SwColumnDlg()
{
    m_pTabPage.disposeAndClear();
}

IMPL_LINK(SwColumnDlg, ObjectListBoxHdl, weld::ComboBox&, rBox, void)
{
    ObjectHdl(&rBox);
}

void SwColumnDlg::ObjectHdl(const weld::ComboBox* pBox)
{
    SfxItemSet* pSet = EvalCurrentSelection();

    if (pBox)
    {
        m_pTabPage->FillItemSet(pSet);
    }
    weld::ComboBox* pApplyToLB = m_pTabPage->GetApplyComboBox();
    m_nOldSelection = pApplyToLB->get_active_id().toInt32();
    long nWidth = m_nSelectionWidth;
    switch(m_nOldSelection)
    {
        case LISTBOX_SELECTION  :
            pSet = m_pSelectionSet.get();
            if( m_pSelectionSet )
                pSet->Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_SECTION    :
        case LISTBOX_SECTIONS   :
            pSet = m_pSectionSet.get();
            pSet->Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_PAGE       :
            nWidth = m_nPageWidth;
            pSet = m_pPageSet.get();
            pSet->Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_FRAME:
            pSet = m_pFrameSet;
        break;
    }

    bool bIsSection = pSet == m_pSectionSet.get() || pSet == m_pSelectionSet.get();
    m_pTabPage->ShowBalance(bIsSection);
    m_pTabPage->SetInSection(bIsSection);
    m_pTabPage->SetFrameMode(true);
    m_pTabPage->SetPageWidth(nWidth);
    if( pSet )
        m_pTabPage->Reset(pSet);
}

IMPL_LINK_NOARG(SwColumnDlg, OkHdl, weld::Button&, void)
{
    // evaluate current selection
    SfxItemSet* pSet = EvalCurrentSelection();
    m_pTabPage->FillItemSet(pSet);

    if(m_pSelectionSet && SfxItemState::SET == m_pSelectionSet->GetItemState(RES_COL))
    {
        //insert region with columns
        const SwFormatCol& rColItem = m_pSelectionSet->Get(RES_COL);
        //only if there actually are columns!
        if(rColItem.GetNumCols() > 1)
            m_rWrtShell.GetView().GetViewFrame()->GetDispatcher()->Execute(
                FN_INSERT_REGION, SfxCallMode::ASYNCHRON, *m_pSelectionSet );
    }

    if(m_pSectionSet && m_pSectionSet->Count() && m_bSectionChanged )
    {
        const SwSection* pCurrSection = m_rWrtShell.GetCurrSection();
        const SwSectionFormat* pFormat = pCurrSection->GetFormat();
        const size_t nNewPos = m_rWrtShell.GetSectionFormatPos( *pFormat );
        SwSectionData aData(*pCurrSection);
        m_rWrtShell.UpdateSection( nNewPos, aData, m_pSectionSet.get() );
    }

    if(m_pSectionSet && m_pSectionSet->Count() && m_bSelSectionChanged )
    {
        m_rWrtShell.SetSectionAttr( *m_pSectionSet );
    }

    if(m_pPageSet && SfxItemState::SET == m_pPageSet->GetItemState(RES_COL) && m_bPageChanged)
    {
        // determine current PageDescriptor and fill the Set with it
        const size_t nCurIdx = m_rWrtShell.GetCurPageDesc();
        SwPageDesc aPageDesc(m_rWrtShell.GetPageDesc(nCurIdx));
        SwFrameFormat &rFormat = aPageDesc.GetMaster();
        rFormat.SetFormatAttr(m_pPageSet->Get(RES_COL));
        m_rWrtShell.ChgPageDesc(nCurIdx, aPageDesc);
    }
    if(m_pFrameSet && SfxItemState::SET == m_pFrameSet->GetItemState(RES_COL) && m_bFrameChanged)
    {
        SfxItemSet aTmp(*m_pFrameSet->GetPool(), svl::Items<RES_COL, RES_COL>{});
        aTmp.Put(*m_pFrameSet);
        m_rWrtShell.StartAction();
        m_rWrtShell.Push();
        m_rWrtShell.SetFlyFrameAttr( aTmp );
        // undo the frame selection again
        if(m_rWrtShell.IsFrameSelected())
        {
            m_rWrtShell.UnSelectFrame();
            m_rWrtShell.LeaveSelFrameMode();
        }
        m_rWrtShell.Pop();
        m_rWrtShell.EndAction();
    }
    m_xDialog->response(RET_OK);
}

SfxItemSet* SwColumnDlg::EvalCurrentSelection()
{
    SfxItemSet* pSet = nullptr;

    switch(m_nOldSelection)
    {
        case LISTBOX_SELECTION  :
            pSet = m_pSelectionSet.get();
        break;
        case LISTBOX_SECTION    :
            pSet = m_pSectionSet.get();
            m_bSectionChanged = true;
        break;
        case LISTBOX_SECTIONS   :
            pSet = m_pSectionSet.get();
            m_bSelSectionChanged = true;
        break;
        case LISTBOX_PAGE       :
            pSet = m_pPageSet.get();
            m_bPageChanged = true;
        break;
        case LISTBOX_FRAME:
            pSet = m_pFrameSet;
            m_bFrameChanged = true;
        break;
    }

    return pSet;
}

static
sal_uInt16 GetMaxWidth( SwColMgr const * pColMgr, sal_uInt16 nCols )
{
    sal_uInt16 nMax = pColMgr->GetActualSize();
    if( --nCols )
        nMax -= pColMgr->GetGutterWidth() * nCols;
    return nMax;
}

const sal_uInt16 SwColumnPage::aPageRg[] = {
    RES_COL, RES_COL,
    0
};

void SwColumnPage::ResetColWidth()
{
    if( m_nCols )
    {
        const sal_uInt16 nWidth = GetMaxWidth( m_xColMgr.get(), m_nCols ) / m_nCols;

        for(sal_uInt16 i = 0; i < m_nCols; ++i)
            m_nColWidth[i] = static_cast<long>(nWidth);
    }

}

constexpr sal_uInt16 g_nMinWidth(MINLAY);

// Now as TabPage
SwColumnPage::SwColumnPage(TabPageParent pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "modules/swriter/ui/columnpage.ui", "ColumnPage", &rSet)
    , m_nFirstVis(0)
    , m_pModifiedField(nullptr)
    , m_bFormat(false)
    , m_bFrame(false)
    , m_bHtmlMode(false)
    , m_bLockUpdate(false)
    , m_xCLNrEdt(m_xBuilder->weld_spin_button("colsnf"))
    , m_xBalanceColsCB(m_xBuilder->weld_check_button("balance"))
    , m_xBtnBack(m_xBuilder->weld_button("back"))
    , m_xLbl1(m_xBuilder->weld_label("1"))
    , m_xLbl2(m_xBuilder->weld_label("2"))
    , m_xLbl3(m_xBuilder->weld_label("3"))
    , m_xBtnNext(m_xBuilder->weld_button("next"))
    , m_xAutoWidthBox(m_xBuilder->weld_check_button("autowidth"))
    , m_xLineTypeLbl(m_xBuilder->weld_label("linestyleft"))
    , m_xLineWidthLbl(m_xBuilder->weld_label("linewidthft"))
    , m_xLineWidthEdit(m_xBuilder->weld_metric_spin_button("linewidthmf", FieldUnit::POINT))
    , m_xLineColorLbl(m_xBuilder->weld_label("linecolorft"))
    , m_xLineHeightLbl(m_xBuilder->weld_label("lineheightft"))
    , m_xLineHeightEdit(m_xBuilder->weld_metric_spin_button("lineheightmf", FieldUnit::PERCENT))
    , m_xLinePosLbl(m_xBuilder->weld_label("lineposft"))
    , m_xLinePosDLB(m_xBuilder->weld_combo_box("lineposlb"))
    , m_xTextDirectionFT(m_xBuilder->weld_label("textdirectionft"))
    , m_xTextDirectionLB(new svx::FrameDirectionListBox(m_xBuilder->weld_combo_box("textdirectionlb")))
    , m_xLineColorDLB(new ColorListBox(m_xBuilder->weld_menu_button("colorlb"), pParent.GetFrameWeld()))
    , m_xLineTypeDLB(new SvtLineListBox(m_xBuilder->weld_menu_button("linestylelb")))
    , m_xEd1(new SwPercentField(m_xBuilder->weld_metric_spin_button("width1mf", FieldUnit::CM)))
    , m_xEd2(new SwPercentField(m_xBuilder->weld_metric_spin_button("width2mf", FieldUnit::CM)))
    , m_xEd3(new SwPercentField(m_xBuilder->weld_metric_spin_button("width3mf", FieldUnit::CM)))
    , m_xDistEd1(new SwPercentField(m_xBuilder->weld_metric_spin_button("spacing1mf", FieldUnit::CM)))
    , m_xDistEd2(new SwPercentField(m_xBuilder->weld_metric_spin_button("spacing2mf", FieldUnit::CM)))
    , m_xDefaultVS(new weld::CustomWeld(*m_xBuilder, "valueset", m_aDefaultVS))
    , m_xPgeExampleWN(new weld::CustomWeld(*m_xBuilder, "pageexample", m_aPgeExampleWN))
    , m_xFrameExampleWN(new weld::CustomWeld(*m_xBuilder, "frameexample", m_aFrameExampleWN))
    , m_xApplyToFT(m_xBuilder->weld_label("applytoft"))
    , m_xApplyToLB(m_xBuilder->weld_combo_box("applytolb"))
{
    connectPercentField(*m_xEd1);
    connectPercentField(*m_xEd2);
    connectPercentField(*m_xEd3);
    connectPercentField(*m_xDistEd1);
    connectPercentField(*m_xDistEd2);

    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_LR_TB, SvxResId(RID_SVXSTR_FRAMEDIR_LTR));
    m_xTextDirectionLB->append(SvxFrameDirection::Horizontal_RL_TB, SvxResId(RID_SVXSTR_FRAMEDIR_RTL));
    m_xTextDirectionLB->append(SvxFrameDirection::Environment, SvxResId(RID_SVXSTR_FRAMEDIR_SUPER));

    SetExchangeSupport();

    m_aDefaultVS.SetColCount(5);

    for (int i = 0; i < 5; ++i)
    //Set accessible name one by one
    {
        OUString aItemText;
        switch( i )
        {
            case 0:
                aItemText =  SwResId( STR_COLUMN_VALUESET_ITEM0 ) ;
                break;
            case 1:
                aItemText =  SwResId( STR_COLUMN_VALUESET_ITEM1 ) ;
                break;
            case 2:
                aItemText =  SwResId( STR_COLUMN_VALUESET_ITEM2 ) ;
                break;
            case 3:
                aItemText =  SwResId( STR_COLUMN_VALUESET_ITEM3 );
                break;
            default:
                aItemText =  SwResId( STR_COLUMN_VALUESET_ITEM4 );
                break;
        }
        m_aDefaultVS.InsertItem( i + 1, aItemText, i );
    }

    m_aDefaultVS.SetSelectHdl(LINK(this, SwColumnPage, SetDefaultsHdl));

    Link<weld::SpinButton&,void> aCLNrLk = LINK(this, SwColumnPage, ColModify);
    m_xCLNrEdt->connect_value_changed(aCLNrLk);
    Link<weld::MetricSpinButton&,void> aLk = LINK(this, SwColumnPage, GapModify);
    m_xDistEd1->connect_value_changed(aLk);
    m_xDistEd2->connect_value_changed(aLk);

    aLk = LINK(this, SwColumnPage, EdModify);

    m_xEd1->connect_value_changed(aLk);
    m_xEd2->connect_value_changed(aLk);
    m_xEd3->connect_value_changed(aLk);

    m_xBtnBack->connect_clicked(LINK(this, SwColumnPage, Up));
    m_xBtnNext->connect_clicked(LINK(this, SwColumnPage, Down));
    m_xAutoWidthBox->connect_toggled(LINK(this, SwColumnPage, AutoWidthHdl));

    Link<weld::MetricSpinButton&,void> aLk2 = LINK( this, SwColumnPage, UpdateColMgr );
    m_xLineTypeDLB->SetSelectHdl(LINK(this, SwColumnPage, UpdateColMgrLineBox));
    m_xLineWidthEdit->connect_value_changed(aLk2);
    m_xLineColorDLB->SetSelectHdl(LINK( this, SwColumnPage, UpdateColMgrColorBox));
    m_xLineHeightEdit->connect_value_changed(aLk2);
    m_xLinePosDLB->connect_changed(LINK(this, SwColumnPage, UpdateColMgrListBox));

    // Separator line
    m_xLineTypeDLB->SetSourceUnit( FieldUnit::TWIP );

    // Fill the line styles listbox
    m_xLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::SOLID),
        SvxBorderLineStyle::SOLID );
    m_xLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DOTTED),
        SvxBorderLineStyle::DOTTED );
    m_xLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DASHED),
        SvxBorderLineStyle::DASHED );

    sal_Int64 nLineWidth = m_xLineWidthEdit->get_value(FieldUnit::POINT);
    nLineWidth = static_cast<long>(MetricField::ConvertDoubleValue(
            nLineWidth,
            m_xLineWidthEdit->get_digits(),
            FieldUnit::POINT, MapUnit::MapTwip ));
    m_xLineTypeDLB->SetWidth(nLineWidth);
    m_xLineColorDLB->SelectEntry(COL_BLACK);
}

SwColumnPage::~SwColumnPage()
{
    disposeOnce();
}

void SwColumnPage::dispose()
{
    m_xFrameExampleWN.reset();
    m_xPgeExampleWN.reset();
    m_xDefaultVS.reset();
    m_xDistEd2.reset();
    m_xDistEd1.reset();
    m_xEd3.reset();
    m_xEd2.reset();
    m_xEd1.reset();
    m_xLineTypeDLB.reset();
    m_xLineColorDLB.reset();
    m_xTextDirectionLB.reset();
    SfxTabPage::dispose();
}

void SwColumnPage::SetPageWidth(long nPageWidth)
{
    long nNewMaxWidth = static_cast< long >(m_xEd1->NormalizePercent(nPageWidth));

    m_xDistEd1->set_max(nNewMaxWidth, FieldUnit::TWIP);
    m_xDistEd2->set_max(nNewMaxWidth, FieldUnit::TWIP);
    m_xEd1->set_max(nNewMaxWidth, FieldUnit::TWIP);
    m_xEd2->set_max(nNewMaxWidth, FieldUnit::TWIP);
    m_xEd3->set_max(nNewMaxWidth, FieldUnit::TWIP);
}

void SwColumnPage::connectPercentField(SwPercentField &rWrap)
{
    weld::MetricSpinButton *pField = rWrap.get();
    assert(pField);
    m_aPercentFieldsMap[pField] = &rWrap;
}

void SwColumnPage::Reset(const SfxItemSet *rSet)
{
    const sal_uInt16 nHtmlMode =
        ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_bHtmlMode = true;
        m_xAutoWidthBox->set_sensitive(false);
    }
    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    m_xEd1->SetMetric(aMetric);
    m_xEd2->SetMetric(aMetric);
    m_xEd3->SetMetric(aMetric);
    m_xDistEd1->SetMetric(aMetric);
    m_xDistEd2->SetMetric(aMetric);
    //default spacing between cols = 0.5cm
    m_xDistEd1->set_value(50, FieldUnit::CM);
    m_xDistEd2->set_value(50, FieldUnit::CM);

    m_xColMgr.reset(new SwColMgr(*rSet));
    m_nCols = m_xColMgr->GetCount() ;
    m_xCLNrEdt->set_max(std::max(static_cast<sal_uInt16>(m_xCLNrEdt->get_max()), m_nCols));

    if(m_bFrame)
    {
        if(m_bFormat)                     // there is no size here
            m_xColMgr->SetActualWidth(FRAME_FORMAT_WIDTH);
        else
        {
            const SwFormatFrameSize& rSize = rSet->Get(RES_FRM_SIZE);
            const SvxBoxItem& rBox = rSet->Get(RES_BOX);
            m_xColMgr->SetActualWidth(static_cast<sal_uInt16>(rSize.GetSize().Width()) - rBox.GetSmallestDistance());
        }
    }
    if (m_xBalanceColsCB->get_visible())
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == rSet->GetItemState( RES_COLUMNBALANCE, false, &pItem ))
            m_xBalanceColsCB->set_active(!static_cast<const SwFormatNoBalancedColumns*>(pItem)->GetValue());
        else
            m_xBalanceColsCB->set_active(true);
    }

    //text direction
    if( SfxItemState::DEFAULT <= rSet->GetItemState( RES_FRAMEDIR ) )
    {
        const SvxFrameDirectionItem& rItem = rSet->Get(RES_FRAMEDIR);
        SvxFrameDirection nVal  = rItem.GetValue();
        m_xTextDirectionLB->set_active_id(nVal);
        m_xTextDirectionLB->save_value();
    }

    Init();
    ActivatePage( *rSet );
}

// create TabPage
VclPtr<SfxTabPage> SwColumnPage::Create(TabPageParent pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwColumnPage>::Create(pParent, *rSet);
}

// stuff attributes into the Set when OK
bool SwColumnPage::FillItemSet(SfxItemSet *rSet)
{
    // set in ItemSet;
    // the current settings are already present

    const SfxPoolItem* pOldItem;
    const SwFormatCol& rCol = m_xColMgr->GetColumns();
    if(nullptr == (pOldItem = GetOldItem( *rSet, RES_COL )) ||
                rCol != *pOldItem )
        rSet->Put(rCol);

    if (m_xBalanceColsCB->get_visible())
    {
        rSet->Put(SwFormatNoBalancedColumns(!m_xBalanceColsCB->get_active()));
    }
    if (m_xTextDirectionLB->get_visible())
    {
        if (m_xTextDirectionLB->get_value_changed_from_saved())
        {
            rSet->Put(SvxFrameDirectionItem(m_xTextDirectionLB->get_active_id(), RES_FRAMEDIR) );
        }
    }
    return true;
}

// update ColumnManager
IMPL_LINK_NOARG( SwColumnPage, UpdateColMgrListBox, weld::ComboBox&, void )
{
    UpdateColMgr(*m_xLineWidthEdit);
}

IMPL_LINK_NOARG( SwColumnPage, UpdateColMgrLineBox, SvtLineListBox&, void )
{
    UpdateColMgr(*m_xLineWidthEdit);
}

IMPL_LINK_NOARG( SwColumnPage, UpdateColMgrColorBox, ColorListBox&, void )
{
    UpdateColMgr(*m_xLineWidthEdit);
}

IMPL_LINK_NOARG( SwColumnPage, UpdateColMgr, weld::MetricSpinButton&, void )
{
    if (!m_xColMgr)
        return;
    long nGutterWidth = m_xColMgr->GetGutterWidth();
    if (m_nCols > 1)
    {
            // Determine whether the most narrow column is too narrow
            // for the adjusted column gap
        long nMin = m_nColWidth[0];

        for( sal_uInt16 i = 1; i < m_nCols; ++i )
            nMin = std::min(nMin, m_nColWidth[i]);

        bool bAutoWidth = m_xAutoWidthBox->get_active();
        if(!bAutoWidth)
        {
            m_xColMgr->SetAutoWidth(false);
                // when the user didn't allocate the whole width,
                // add the missing amount to the last column.
            long nSum = 0;
            for(sal_uInt16 i = 0; i < m_nCols; ++i)
                nSum += m_nColWidth[i];
            nGutterWidth = 0;
            for(sal_uInt16 i = 0; i < m_nCols - 1; ++i)
                nGutterWidth += m_nColDist[i];
            nSum += nGutterWidth;

            long nMaxW = m_xColMgr->GetActualSize();

            if( nSum < nMaxW  )
                m_nColWidth[m_nCols - 1] += nMaxW - nSum;

            m_xColMgr->SetColWidth( 0, static_cast< sal_uInt16 >(m_nColWidth[0] + m_nColDist[0]/2) );
            for( sal_uInt16 i = 1; i < m_nCols-1; ++i )
            {
                long nActDist = (m_nColDist[i] + m_nColDist[i - 1]) / 2;
                m_xColMgr->SetColWidth( i, static_cast< sal_uInt16 >(m_nColWidth[i] + nActDist ));
            }
            m_xColMgr->SetColWidth( m_nCols-1, static_cast< sal_uInt16 >(m_nColWidth[m_nCols-1] + m_nColDist[m_nCols -2]/2) );

        }

        bool bEnable = isLineNotNone();
        m_xLineHeightEdit->set_sensitive(bEnable);
        m_xLineHeightLbl->set_sensitive(bEnable);
        m_xLineWidthLbl->set_sensitive(bEnable);
        m_xLineWidthEdit->set_sensitive(bEnable);
        m_xLineColorDLB->set_sensitive(bEnable);
        m_xLineColorLbl->set_sensitive(bEnable);

        sal_Int64 nLineWidth = m_xLineWidthEdit->get_value(FieldUnit::PERCENT);
        nLineWidth = static_cast<long>(MetricField::ConvertDoubleValue(
                nLineWidth,
                m_xLineWidthEdit->get_digits(),
                m_xLineWidthEdit->get_unit(), MapUnit::MapTwip ));
        if( !bEnable )
            m_xColMgr->SetNoLine();
        else
        {
            m_xColMgr->SetLineWidthAndColor(
                    m_xLineTypeDLB->GetSelectEntryStyle(),
                    nLineWidth,
                    m_xLineColorDLB->GetSelectEntryColor() );
            m_xColMgr->SetAdjust(SwColLineAdj(m_xLinePosDLB->get_active() + 1));
            m_xColMgr->SetLineHeightPercent(static_cast<short>(m_xLineHeightEdit->get_value(FieldUnit::PERCENT)));
            bEnable = m_xColMgr->GetLineHeightPercent() != 100;
        }
        m_xLinePosLbl->set_sensitive(bEnable);
        m_xLinePosDLB->set_sensitive(bEnable);

        //fdo#66815 if the values are going to be the same, don't update
        //them to avoid the listbox selection resetting
        if (nLineWidth != m_xLineTypeDLB->GetWidth())
            m_xLineTypeDLB->SetWidth(nLineWidth);
        Color aColor(m_xLineColorDLB->GetSelectEntryColor());
        if (aColor != m_xLineTypeDLB->GetColor())
            m_xLineTypeDLB->SetColor(aColor);
    }
    else
    {
        m_xColMgr->NoCols();
        m_nCols = 0;
    }

    //set maximum values
    m_xCLNrEdt->set_max(std::max(1L,
        std::min(long(nMaxCols), long( m_xColMgr->GetActualSize() / (nGutterWidth + MINLAY)) )));

    //prompt example window
    if(!m_bLockUpdate)
    {
        if(m_bFrame)
        {
            m_aFrameExampleWN.SetColumns(m_xColMgr->GetColumns());
            m_aFrameExampleWN.Invalidate();
        }
        else
            m_aPgeExampleWN.Invalidate();
    }
}

void SwColumnPage::Init()
{
    m_xCLNrEdt->set_value(m_nCols);

    bool bAutoWidth = m_xColMgr->IsAutoWidth() || m_bHtmlMode;
    m_xAutoWidthBox->set_active(bAutoWidth);

    sal_Int32 nColumnWidthSum = 0;
    // set the widths
    for(sal_uInt16 i = 0; i < m_nCols; ++i)
    {
        m_nColWidth[i] = m_xColMgr->GetColWidth(i);
        nColumnWidthSum += m_nColWidth[i];
        if(i < m_nCols - 1)
            m_nColDist[i] = m_xColMgr->GetGutterWidth(i);
    }

    if( 1 < m_nCols )
    {
        // #97495# make sure that the automatic column width's are always equal
        if(bAutoWidth)
        {
            nColumnWidthSum /= m_nCols;
            for(sal_uInt16 i = 0; i < m_nCols; ++i)
                m_nColWidth[i] = nColumnWidthSum;
        }
        SwColLineAdj eAdj = m_xColMgr->GetAdjust();
        if( COLADJ_NONE == eAdj )       // the dialog doesn't know a NONE!
        {
            eAdj = COLADJ_TOP;
            //without Adjust no line type
            m_xLineTypeDLB->SelectEntry(SvxBorderLineStyle::NONE);
            m_xLineHeightEdit->set_value(100, FieldUnit::PERCENT);
        }
        else
        {
            // Need to multiply by 100 because of the 2 decimals
            m_xLineWidthEdit->set_value( m_xColMgr->GetLineWidth() * 100, FieldUnit::TWIP);
            m_xLineColorDLB->SelectEntry( m_xColMgr->GetLineColor() );
            m_xLineTypeDLB->SelectEntry( m_xColMgr->GetLineStyle() );
            m_xLineTypeDLB->SetWidth( m_xColMgr->GetLineWidth( ) );
            m_xLineHeightEdit->set_value(m_xColMgr->GetLineHeightPercent(), FieldUnit::PERCENT);

        }
        m_xLinePosDLB->set_active( static_cast< sal_Int32 >(eAdj - 1) );
    }
    else
    {
        m_xLinePosDLB->set_active(0);
        m_xLineTypeDLB->SelectEntry(SvxBorderLineStyle::NONE);
        m_xLineHeightEdit->set_value(100, FieldUnit::PERCENT);
    }

    UpdateCols();
    Update(nullptr);

        // set maximum number of columns
        // values below 1 are not allowed
    m_xCLNrEdt->set_max(std::max(1L,
        std::min(long(nMaxCols), long( m_xColMgr->GetActualSize() / g_nMinWidth) )));
}

bool SwColumnPage::isLineNotNone() const
{
    // nothing is turned off
    return m_xLineTypeDLB->GetSelectEntryStyle() != SvxBorderLineStyle::NONE;
}

/*
 * The number of columns has changed -- here the controls for editing of the
 * columns are en- or disabled according to the column number. In case there are
 * more than nVisCols (=3) all Edit are being enabled and the buttons for
 * scrolling too. Otherwise Edits are being enabled according to the column
 * numbers; one column can not be edited.
 */
void SwColumnPage::UpdateCols()
{
    bool bEnableBtns= false;
    bool bEnable12  = false;
    bool bEnable3   = false;
    const bool bEdit = !m_xAutoWidthBox->get_active();
    if ( m_nCols > nVisCols )
    {
        bEnableBtns = !m_bHtmlMode;
        bEnable12 = bEnable3 = bEdit;
    }
    else if( bEdit )
    {
        // here are purposely hardly any breaks
        switch(m_nCols)
        {
            case 3: bEnable3 = true;
                [[fallthrough]];
            case 2: bEnable12= true; break;
            default: /* do nothing */;
        }
    }
    m_xEd1->set_sensitive(bEnable12);
    bool bEnable = m_nCols > 1;
    m_xDistEd1->set_sensitive(bEnable);
    m_xAutoWidthBox->set_sensitive(bEnable && !m_bHtmlMode);
    m_xEd2->set_sensitive(bEnable12);
    m_xDistEd2->set_sensitive(bEnable3);
    m_xEd3->set_sensitive(bEnable3);
    m_xLbl1->set_sensitive(bEnable12);
    m_xLbl2->set_sensitive(bEnable12);
    m_xLbl3->set_sensitive(bEnable3);
    m_xBtnBack->set_sensitive(bEnableBtns);
    m_xBtnNext->set_sensitive(bEnableBtns);

    m_xLineTypeDLB->set_sensitive( bEnable );
    m_xLineTypeLbl->set_sensitive( bEnable );

    if (bEnable)
    {
        bEnable = isLineNotNone();
    }

    //all these depend on > 1 column and line style != none
    m_xLineHeightEdit->set_sensitive(bEnable);
    m_xLineHeightLbl->set_sensitive(bEnable);
    m_xLineWidthLbl->set_sensitive(bEnable);
    m_xLineWidthEdit->set_sensitive(bEnable);
    m_xLineColorDLB->set_sensitive(bEnable);
    m_xLineColorLbl->set_sensitive(bEnable);

    if (bEnable)
        bEnable = m_xColMgr->GetLineHeightPercent() != 100;

    //and these additionally depend on line height != 100%
    m_xLinePosDLB->set_sensitive(bEnable);
    m_xLinePosLbl->set_sensitive(bEnable);
}

void SwColumnPage::SetLabels( sal_uInt16 nVis )
{
    //insert ~ before the last character, e.g. 1 -> ~1, 10 -> 1~0
    const OUString sLbl( '~' );

    const OUString sLbl1(OUString::number( nVis + 1 ));
    m_xLbl1->set_label(sLbl1.replaceAt(sLbl1.getLength()-1, 0, sLbl));

    const OUString sLbl2(OUString::number( nVis + 2 ));
    m_xLbl2->set_label(sLbl2.replaceAt(sLbl2.getLength()-1, 0, sLbl));

    const OUString sLbl3(OUString::number( nVis + 3 ));
    m_xLbl3->set_label(sLbl3.replaceAt(sLbl3.getLength()-1, 0, sLbl));

    const OUString sColumnWidth = SwResId( STR_ACCESS_COLUMN_WIDTH ) ;
    m_xEd1->set_accessible_name(sColumnWidth.replaceFirst("%1", sLbl1));
    m_xEd2->set_accessible_name(sColumnWidth.replaceFirst("%1", sLbl2));
    m_xEd3->set_accessible_name(sColumnWidth.replaceFirst("%1", sLbl3));

    const OUString sDist = SwResId( STR_ACCESS_PAGESETUP_SPACING ) ;
    m_xDistEd1->set_accessible_name(
        sDist.replaceFirst("%1", sLbl1).replaceFirst("%2", sLbl2));

    m_xDistEd2->set_accessible_name(
        sDist.replaceFirst("%1", sLbl2).replaceFirst("%2", sLbl3));
}

/*
 * Handler that is called at alteration of the column number. An alteration of
 * the column number overwrites potential user's width settings; all columns
 * are equally wide.
 */
IMPL_LINK(SwColumnPage, ColModify, weld::SpinButton&, rEdit, void)
{
    ColModify(&rEdit);
}

void SwColumnPage::ColModify(const weld::SpinButton* pNF)
{
    m_nCols = static_cast<sal_uInt16>(m_xCLNrEdt->get_value());
    //#107890# the handler is also called from LoseFocus()
    //then no change has been made and thus no action should be taken
    // #i17816# changing the displayed types within the ValueSet
    //from two columns to two columns with different settings doesn't invalidate the
    // example windows in ::ColModify()
    if (!pNF || m_xColMgr->GetCount() != m_nCols)
    {
        if (pNF)
            m_aDefaultVS.SetNoSelection();
        long nDist = static_cast< long >(m_xDistEd1->DenormalizePercent(m_xDistEd1->get_value(FieldUnit::TWIP)));
        m_xColMgr->SetCount(m_nCols, static_cast<sal_uInt16>(nDist));
        for(sal_uInt16 i = 0; i < m_nCols; i++)
            m_nColDist[i] = nDist;
        m_nFirstVis = 0;
        SetLabels( m_nFirstVis );
        UpdateCols();
        ResetColWidth();
        Update(nullptr);
    }
}

/*
 * Modify handler for an alteration of the column width or the column gap.
 * These changes take effect time-displaced. With an alteration of the column
 * width the automatic calculation of the column width is overruled; only an
 * alteration of the column number leads back to that default.
 */
IMPL_LINK(SwColumnPage, GapModify, weld::MetricSpinButton&, rMetricField, void)
{
    if (m_nCols < 2)
        return;
    SwPercentField *pField = m_aPercentFieldsMap[&rMetricField];
    assert(pField);
    long nActValue = static_cast< long >(pField->DenormalizePercent(pField->get_value(FieldUnit::TWIP)));
    if (m_xAutoWidthBox->get_active())
    {
        const long nMaxGap = static_cast< long >
            ((m_xColMgr->GetActualSize() - m_nCols * MINLAY)/(m_nCols - 1));
        if(nActValue > nMaxGap)
        {
            nActValue = nMaxGap;
            m_xDistEd1->set_value(m_xDistEd1->NormalizePercent(nMaxGap), FieldUnit::TWIP);
        }
        m_xColMgr->SetGutterWidth(static_cast<sal_uInt16>(nActValue));
        for(sal_uInt16 i = 0; i < m_nCols; i++)
            m_nColDist[i] = nActValue;

        ResetColWidth();
        UpdateCols();
    }
    else
    {
        const sal_uInt16 nVis = m_nFirstVis + ((pField == m_xDistEd2.get()) ? 1 : 0);
        long nDiff = nActValue - m_nColDist[nVis];
        if(nDiff)
        {
            long nLeft = m_nColWidth[nVis];
            long nRight = m_nColWidth[nVis + 1];
            if(nLeft + nRight + 2 * MINLAY < nDiff)
                nDiff = nLeft + nRight - 2 * MINLAY;
            if(nDiff < nRight - MINLAY)
            {
                nRight -= nDiff;
            }
            else
            {
                long nTemp = nDiff - nRight + MINLAY;
                nRight = MINLAY;
                if(nLeft > nTemp - MINLAY)
                {
                    nLeft -= nTemp;
                    nTemp = 0;
                }
                else
                {
                    nTemp -= nLeft + MINLAY;
                    nLeft = MINLAY;
                }
                nDiff = nTemp;
            }
            m_nColWidth[nVis] = nLeft;
            m_nColWidth[nVis + 1] = nRight;
            m_nColDist[nVis] += nDiff;

            m_xColMgr->SetColWidth( nVis, sal_uInt16(nLeft) );
            m_xColMgr->SetColWidth( nVis + 1, sal_uInt16(nRight) );
            m_xColMgr->SetGutterWidth( sal_uInt16(m_nColDist[nVis]), nVis );
        }

    }
    Update(&rMetricField);
}

IMPL_LINK(SwColumnPage, EdModify, weld::MetricSpinButton&, rEdit, void)
{
    SwPercentField *pField = m_aPercentFieldsMap[&rEdit];
    assert(pField);
    m_pModifiedField = pField;
    Timeout();
}

// Handler behind the Checkbox for automatic width. When the box is checked
// no explicit values for the column width can be entered.
IMPL_LINK(SwColumnPage, AutoWidthHdl, weld::ToggleButton&, rBox, void)
{
    long nDist = static_cast< long >(m_xDistEd1->DenormalizePercent(m_xDistEd1->get_value(FieldUnit::TWIP)));
    m_xColMgr->SetCount(m_nCols, static_cast<sal_uInt16>(nDist));
    for(sal_uInt16 i = 0; i < m_nCols; i++)
        m_nColDist[i] = nDist;
    if (rBox.get_active())
    {
        m_xColMgr->SetGutterWidth(sal_uInt16(nDist));
        ResetColWidth();
    }
    m_xColMgr->SetAutoWidth(rBox.get_active(), sal_uInt16(nDist));
    UpdateCols();
    Update(nullptr);
}

// scroll up the contents of the edits
IMPL_LINK_NOARG(SwColumnPage, Up, weld::Button&, void)
{
    if( m_nFirstVis )
    {
        --m_nFirstVis;
        SetLabels( m_nFirstVis );
        Update(nullptr);
    }
}

// scroll down the contents of the edits.
IMPL_LINK_NOARG(SwColumnPage, Down, weld::Button&, void)
{
    if( m_nFirstVis + nVisCols < m_nCols )
    {
        ++m_nFirstVis;
        SetLabels( m_nFirstVis );
        Update(nullptr);
    }
}

// relict from ancient times - now directly without time handler; triggered by
// an alteration of the column width or the column gap.
void SwColumnPage::Timeout()
{
    SwPercentField *pField = m_pModifiedField;
    if (m_pModifiedField)
    {
            // find the changed column
        sal_uInt16 nChanged = m_nFirstVis;
        if (m_pModifiedField == m_xEd2.get())
            ++nChanged;
        else if (m_pModifiedField == m_xEd3.get())
            nChanged += 2;

        long nNewWidth = static_cast< long >
            (m_pModifiedField->DenormalizePercent(m_pModifiedField->get_value(FieldUnit::TWIP)));
        long nDiff = nNewWidth - m_nColWidth[nChanged];

        // when it's the last column
        if(nChanged == m_nCols - 1)
        {
            m_nColWidth[0] -= nDiff;
            if(m_nColWidth[0] < static_cast<long>(g_nMinWidth))
            {
                nNewWidth -= g_nMinWidth - m_nColWidth[0];
                m_nColWidth[0] = g_nMinWidth;
            }

        }
        else if(nDiff)
        {
            m_nColWidth[nChanged + 1] -= nDiff;
            if(m_nColWidth[nChanged + 1] < static_cast<long>(g_nMinWidth))
            {
                nNewWidth -= g_nMinWidth - m_nColWidth[nChanged + 1];
                m_nColWidth[nChanged + 1] = g_nMinWidth;
            }
        }
        m_nColWidth[nChanged] = nNewWidth;
        m_pModifiedField = nullptr;
    }

    Update(pField ? pField->get() : nullptr);
}

// Update the view
void SwColumnPage::Update(const weld::MetricSpinButton* pInteractiveField)
{
    m_xBalanceColsCB->set_sensitive(m_nCols > 1);
    if(m_nCols >= 2)
    {
        sal_Int64 nCurrentValue, nNewValue;

        nCurrentValue = m_xEd1->NormalizePercent(m_xEd1->DenormalizePercent(m_xEd1->get_value(FieldUnit::TWIP)));
        nNewValue = m_xEd1->NormalizePercent(m_nColWidth[m_nFirstVis]);

        //fdo#87612 if we're interacting with this widget and the value will be the same
        //then leave it alone (i.e. don't change equivalent values of e.g. .8 -> 0.8)
        if (nNewValue != nCurrentValue || pInteractiveField != m_xEd1->get())
            m_xEd1->set_value(nNewValue, FieldUnit::TWIP);

        nCurrentValue = m_xDistEd1->NormalizePercent(m_xDistEd1->DenormalizePercent(m_xDistEd1->get_value(FieldUnit::TWIP)));
        nNewValue = m_xDistEd1->NormalizePercent(m_nColDist[m_nFirstVis]);
        if (nNewValue != nCurrentValue || pInteractiveField != m_xDistEd1->get())
            m_xDistEd1->set_value(nNewValue, FieldUnit::TWIP);

        nCurrentValue = m_xEd2->NormalizePercent(m_xEd2->DenormalizePercent(m_xEd2->get_value(FieldUnit::TWIP)));
        nNewValue = m_xEd2->NormalizePercent(m_nColWidth[m_nFirstVis+1]);
        if (nNewValue != nCurrentValue || pInteractiveField != m_xEd2->get())
            m_xEd2->set_value(nNewValue, FieldUnit::TWIP);

        if(m_nCols >= 3)
        {
            nCurrentValue = m_xDistEd2->NormalizePercent(m_xDistEd2->DenormalizePercent(m_xDistEd2->get_value(FieldUnit::TWIP)));
            nNewValue = m_xDistEd2->NormalizePercent(m_nColDist[m_nFirstVis+1]);
            if (nNewValue != nCurrentValue || pInteractiveField != m_xDistEd2->get())
                m_xDistEd2->set_value(nNewValue, FieldUnit::TWIP);

            nCurrentValue = m_xEd3->NormalizePercent(m_xEd3->DenormalizePercent(m_xEd3->get_value(FieldUnit::TWIP)));
            nNewValue = m_xEd3->NormalizePercent(m_nColWidth[m_nFirstVis+2]);
            if (nNewValue != nCurrentValue || pInteractiveField != m_xEd3->get())
                m_xEd3->set_value(nNewValue, FieldUnit::TWIP);
        }
        else
        {
            m_xEd3->set_text(OUString());
            m_xDistEd2->set_text(OUString());
        }
    }
    else
    {
        m_xEd1->set_text(OUString());
        m_xEd2->set_text(OUString());
        m_xEd3->set_text(OUString());
        m_xDistEd1->set_text(OUString());
        m_xDistEd2->set_text(OUString());
    }
    UpdateColMgr(*m_xLineWidthEdit);
}

// Update Bsp
void SwColumnPage::ActivatePage(const SfxItemSet& rSet)
{
    bool bVertical = false;
    if (SfxItemState::DEFAULT <= rSet.GetItemState(RES_FRAMEDIR))
    {
        const SvxFrameDirectionItem& rDirItem =
                    rSet.Get(RES_FRAMEDIR);
        bVertical = rDirItem.GetValue() == SvxFrameDirection::Vertical_RL_TB||
                    rDirItem.GetValue() == SvxFrameDirection::Vertical_LR_TB;
    }

    if (!m_bFrame)
    {
        if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
        {
            const SvxSizeItem& rSize = rSet.Get(SID_ATTR_PAGE_SIZE);

            sal_uInt16 nActWidth;

            if (!bVertical)
            {
                const SvxLRSpaceItem& rLRSpace = rSet.Get(RES_LR_SPACE);
                const SvxBoxItem& rBox = rSet.Get(RES_BOX);
                nActWidth = rSize.GetSize().Width()
                                - rLRSpace.GetLeft() - rLRSpace.GetRight() - rBox.GetSmallestDistance();
            }
            else
            {
                const SvxULSpaceItem& rULSpace = rSet.Get( RES_UL_SPACE );
                const SvxBoxItem& rBox = rSet.Get(RES_BOX);
                nActWidth = rSize.GetSize().Height()
                                - rULSpace.GetUpper() - rULSpace.GetLower() - rBox.GetSmallestDistance();

            }

            if( m_xColMgr->GetActualSize() != nActWidth)
            {
                m_xColMgr->SetActualWidth(nActWidth);
                ColModify( nullptr );
                UpdateColMgr( *m_xLineWidthEdit );
            }
        }
        m_xFrameExampleWN->hide();
        m_aPgeExampleWN.UpdateExample(rSet, m_xColMgr.get());
        m_xPgeExampleWN->show();

    }
    else
    {
        m_xPgeExampleWN->hide();
        m_xFrameExampleWN->show();

        // Size
        const SwFormatFrameSize& rSize = rSet.Get(RES_FRM_SIZE);
        const SvxBoxItem& rBox = rSet.Get(RES_BOX);

        sal_uInt16 nTotalWish;
        if (m_bFormat)
            nTotalWish = FRAME_FORMAT_WIDTH;
        else
        {
            long const nDistance = rBox.GetSmallestDistance();
            nTotalWish = (!bVertical ? rSize.GetWidth() : rSize.GetHeight()) - 2 * nDistance;
        }

        // set maximum values of column width
        SetPageWidth(nTotalWish);

        if(m_xColMgr->GetActualSize() != nTotalWish)
        {
            m_xColMgr->SetActualWidth(nTotalWish);
            Init();
        }
        bool bPercent;
        // only relative data in frame format
        if ( m_bFormat || (rSize.GetWidthPercent() && rSize.GetWidthPercent() != SwFormatFrameSize::SYNCED) )
        {
            // set value for 100%
            m_xEd1->SetRefValue(nTotalWish);
            m_xEd2->SetRefValue(nTotalWish);
            m_xEd3->SetRefValue(nTotalWish);
            m_xDistEd1->SetRefValue(nTotalWish);
            m_xDistEd2->SetRefValue(nTotalWish);

            // switch to %-view
            bPercent = true;
        }
        else
            bPercent = false;

        m_xEd1->ShowPercent(bPercent);
        m_xEd2->ShowPercent(bPercent);
        m_xEd3->ShowPercent(bPercent);
        m_xDistEd1->ShowPercent(bPercent);
        m_xDistEd2->ShowPercent(bPercent);
        m_xDistEd1->SetMetricFieldMin(0);
        m_xDistEd2->SetMetricFieldMin(0);
    }
    Update(nullptr);
}

DeactivateRC SwColumnPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

IMPL_LINK(SwColumnPage, SetDefaultsHdl, SvtValueSet *, pVS, void)
{
    const sal_uInt16 nItem = pVS->GetSelectedItemId();
    if( nItem < 4 )
    {
        m_xCLNrEdt->set_value(nItem);
        m_xAutoWidthBox->set_active(true);
        m_xDistEd1->set_value(50, FieldUnit::CM);
        ColModify(nullptr);
    }
    else
    {
        m_bLockUpdate = true;
        m_xCLNrEdt->set_value(2);
        m_xAutoWidthBox->set_active(false);
        m_xDistEd1->set_value(50, FieldUnit::CM);
        ColModify(nullptr);
        // now set the width ratio to 2 : 1 or 1 : 2 respectively
        const long nSmall = static_cast< long >(m_xColMgr->GetActualSize() / 3);
        if(nItem == 4)
        {
            m_xEd2->set_value(m_xEd2->NormalizePercent(nSmall), FieldUnit::TWIP);
            m_pModifiedField = m_xEd2.get();
        }
        else
        {
            m_xEd1->set_value(m_xEd1->NormalizePercent(nSmall), FieldUnit::TWIP);
            m_pModifiedField = m_xEd1.get();
        }
        m_bLockUpdate = false;
        Timeout();

    }
}

void SwColumnPage::SetFrameMode(bool bMod)
{
    m_bFrame = bMod;
}

void SwColumnPage::SetInSection(bool bSet)
{
    if(!SW_MOD()->GetCTLOptions().IsCTLFontEnabled())
        return;

    m_xTextDirectionFT->set_visible(bSet);
    m_xTextDirectionLB->set_visible(bSet);
}

void ColumnValueSet::UserDraw(const UserDrawEvent& rUDEvt)
{
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    tools::Rectangle aRect = rUDEvt.GetRect();
    const sal_uInt16 nItemId = rUDEvt.GetItemId();
    long nRectWidth = aRect.GetWidth();
    long nRectHeight = aRect.GetHeight();

    Point aBLPos = aRect.TopLeft();
    Color aFillColor(pDev->GetFillColor());
    Color aLineColor(pDev->GetLineColor());
    pDev->SetFillColor(rStyleSettings.GetFieldColor());
    pDev->SetLineColor(SwViewOption::GetFontColor());

    long nStep = std::abs(std::abs(nRectHeight * 95 /100) / 11);
    long nTop = (nRectHeight - 11 * nStep ) / 2;
    sal_uInt16 nCols = 0;
    long nStarts[3];
    long nEnds[3];
    nStarts[0] = nRectWidth * 10 / 100;
    switch( nItemId )
    {
        case 1:
            nEnds[0] = nRectWidth * 9 / 10;
            nCols = 1;
        break;
        case 2: nCols = 2;
            nEnds[0] = nRectWidth * 45 / 100;
            nStarts[1] = nEnds[0] + nStep;
            nEnds[1] = nRectWidth * 9 / 10;
        break;
        case 3: nCols = 3;
            nEnds[0]    = nRectWidth * 30 / 100;
            nStarts[1]  = nEnds[0] + nStep;
            nEnds[1]    = nRectWidth * 63 / 100;
            nStarts[2]  = nEnds[1] + nStep;
            nEnds[2]    = nRectWidth * 9 / 10;
        break;
        case 4: nCols = 2;
            nEnds[0] = nRectWidth * 63 / 100;
            nStarts[1] = nEnds[0] + nStep;
            nEnds[1] = nRectWidth * 9 / 10;
        break;
        case 5: nCols = 2;
            nEnds[0] = nRectWidth * 30 / 100;
            nStarts[1] = nEnds[0] + nStep;
            nEnds[1] = nRectWidth * 9 / 10;
        break;
    }
    for(sal_uInt16 j = 0; j < nCols; j++ )
    {
        Point aStart(aBLPos.X() + nStarts[j], 0);
        Point aEnd(aBLPos.X() + nEnds[j], 0);
        for( sal_uInt16 i = 0; i < 12; i ++)
        {
            aStart.setY( aBLPos.Y() + nTop + i * nStep);
            aEnd.setY( aStart.Y() );
            pDev->DrawLine(aStart, aEnd);
        }
    }
    pDev->SetFillColor(aFillColor);
    pDev->SetLineColor(aLineColor);
}

void ColumnValueSet::StyleUpdated()
{
    SetFormat();
    Invalidate();
    SvtValueSet::StyleUpdated();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
