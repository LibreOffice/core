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

#include "column.hxx"

#include "hintids.hxx"
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
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>

#include <swmodule.hxx>
#include <sal/macros.h>

#include <helpid.h>
#include "globals.hrc"
#include "swtypes.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "docsh.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "viewopt.hxx"
#include "format.hxx"
#include "frmmgr.hxx"
#include "frmdlg.hxx"
#include "colmgr.hxx"
#include "prcntfld.hxx"
#include "paratr.hxx"
#include "frmui.hrc"
#include "strings.hrc"
#include <section.hxx>
#include <docary.hxx>
#include <pagedesc.hxx>

#include "strings.hrc"

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

inline bool IsMarkInSameSection( SwWrtShell& rWrtSh, const SwSection* pSect )
{
    rWrtSh.SwapPam();
    bool bRet = pSect == rWrtSh.GetCurrSection();
    rWrtSh.SwapPam();
    return bRet;
}

SwColumnDlg::SwColumnDlg(vcl::Window* pParent, SwWrtShell& rSh)
    : SfxModalDialog(pParent, "ColumnDialog", "modules/swriter/ui/columndialog.ui")
    , rWrtShell(rSh)
    , pPageSet(nullptr)
    , pSectionSet(nullptr)
    , pSelectionSet(nullptr)
    , pFrameSet(nullptr)
    , nOldSelection(0)
    , nSelectionWidth(0)
    , bPageChanged(false)
    , bSectionChanged(false)
    , bSelSectionChanged(false)
    , bFrameChanged(false)
{
    SwRect aRect;
    rWrtShell.CalcBoundRect(aRect, RndStdIds::FLY_AS_CHAR);

    nSelectionWidth = aRect.Width();

    SfxItemSet* pColPgSet = nullptr;
    static sal_uInt16 const aSectIds[] = { RES_COL, RES_COL,
                                                RES_FRM_SIZE, RES_FRM_SIZE,
                                                RES_COLUMNBALANCE, RES_FRAMEDIR,
                                                0 };

    const SwSection* pCurrSection = rWrtShell.GetCurrSection();
    const sal_uInt16 nFullSectCnt = rWrtShell.GetFullSelectedSectionCount();
    if( pCurrSection && ( !rWrtShell.HasSelection() || 0 != nFullSectCnt ))
    {
        nSelectionWidth = rSh.GetSectionWidth(*pCurrSection->GetFormat());
        if ( !nSelectionWidth )
            nSelectionWidth = USHRT_MAX;
        pSectionSet = new SfxItemSet( rWrtShell.GetAttrPool(), aSectIds );
        pSectionSet->Put( pCurrSection->GetFormat()->GetAttrSet() );
        pColPgSet = pSectionSet;
    }

    if( rWrtShell.HasSelection() && rWrtShell.IsInsRegionAvailable() &&
        ( !pCurrSection || ( 1 != nFullSectCnt &&
            IsMarkInSameSection( rWrtShell, pCurrSection ) )))
    {
        pSelectionSet = new SfxItemSet( rWrtShell.GetAttrPool(), aSectIds );
        pColPgSet = pSelectionSet;
    }

    if( rWrtShell.GetFlyFrameFormat() )
    {
        const SwFrameFormat* pFormat = rSh.GetFlyFrameFormat() ;
        pFrameSet = new SfxItemSet(rWrtShell.GetAttrPool(), aSectIds );
        pFrameSet->Put(pFormat->GetFrameSize());
        pFrameSet->Put(pFormat->GetCol());
        pColPgSet = pFrameSet;
    }

    const SwPageDesc* pPageDesc = rWrtShell.GetSelectedPageDescs();
    if( pPageDesc )
    {
        pPageSet = new SfxItemSet(
            rWrtShell.GetAttrPool(),
            svl::Items<
                RES_FRM_SIZE, RES_FRM_SIZE,
                RES_LR_SPACE, RES_LR_SPACE,
                RES_COL, RES_COL>{});

        const SwFrameFormat &rFormat = pPageDesc->GetMaster();
        nPageWidth = rFormat.GetFrameSize().GetSize().Width();

        const SvxLRSpaceItem& rLRSpace = rFormat.GetLRSpace();
        const SvxBoxItem& rBox = rFormat.GetBox();
        nPageWidth -= rLRSpace.GetLeft() + rLRSpace.GetRight() + rBox.GetSmallestDistance();

        pPageSet->Put(rFormat.GetCol());
        pPageSet->Put(rFormat.GetLRSpace());
        pColPgSet = pPageSet;
    }

    assert(pColPgSet);

    // create TabPage
    pTabPage = static_cast<SwColumnPage*>( SwColumnPage::Create(get_content_area(), pColPgSet).get() );
    pTabPage->get<vcl::Window>("applytoft")->Show();
    pTabPage->get(m_pApplyToLB, "applytolb");
    m_pApplyToLB->Show();

    if (pCurrSection && (!rWrtShell.HasSelection() || 0 != nFullSectCnt))
    {
        m_pApplyToLB->RemoveEntry( m_pApplyToLB->GetEntryPos(
                                        reinterpret_cast<void*>((sal_IntPtr)( 1 >= nFullSectCnt
                                                    ? LISTBOX_SECTIONS
                                                    : LISTBOX_SECTION ))));
    }
    else
    {
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( reinterpret_cast<void*>((sal_IntPtr)LISTBOX_SECTION) ));
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( reinterpret_cast<void*>((sal_IntPtr)LISTBOX_SECTIONS) ));
    }

    if (!( rWrtShell.HasSelection() && rWrtShell.IsInsRegionAvailable() &&
        ( !pCurrSection || ( 1 != nFullSectCnt &&
            IsMarkInSameSection( rWrtShell, pCurrSection ) ))))
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( reinterpret_cast<void*>((sal_IntPtr)LISTBOX_SELECTION) ));

    if (!rWrtShell.GetFlyFrameFormat())
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( reinterpret_cast<void*>(LISTBOX_FRAME) ));

    const sal_Int32 nPagePos = m_pApplyToLB->GetEntryPos( reinterpret_cast<void*>(LISTBOX_PAGE) );
    if (pPageSet && pPageDesc)
    {
        const OUString sPageStr = m_pApplyToLB->GetEntry(nPagePos) + pPageDesc->GetName();
        m_pApplyToLB->RemoveEntry(nPagePos);
        m_pApplyToLB->InsertEntry( sPageStr, nPagePos );
        m_pApplyToLB->SetEntryData( nPagePos, reinterpret_cast<void*>(LISTBOX_PAGE));
    }
    else
        m_pApplyToLB->RemoveEntry( nPagePos );

    m_pApplyToLB->SelectEntryPos(0);
    ObjectHdl(nullptr);

    m_pApplyToLB->SetSelectHdl(LINK(this, SwColumnDlg, ObjectListBoxHdl));
    OKButton *pOK = get<OKButton>("ok");
    pOK->SetClickHdl(LINK(this, SwColumnDlg, OkHdl));
    //#i80458# if no columns can be set then disable OK
    if( !m_pApplyToLB->GetEntryCount() )
        pOK->Enable( false );
    //#i97810# set focus to the TabPage
    pTabPage->ActivateColumnControl();
    pTabPage->Show();
}

SwColumnDlg::~SwColumnDlg()
{
    disposeOnce();
}

void SwColumnDlg::dispose()
{
    pTabPage.disposeAndClear();
    delete pPageSet;
    delete pSectionSet;
    delete pSelectionSet;
    m_pApplyToLB.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK(SwColumnDlg, ObjectListBoxHdl, ListBox&, rBox, void)
{
    ObjectHdl(&rBox);
}

void SwColumnDlg::ObjectHdl(ListBox* pBox)
{
    SfxItemSet* pSet = EvalCurrentSelection();

    if(pBox)
    {
        pTabPage->FillItemSet(pSet);
    }
    nOldSelection = reinterpret_cast<sal_IntPtr>(m_pApplyToLB->GetSelectEntryData());
    long nWidth = nSelectionWidth;
    switch(nOldSelection)
    {
        case LISTBOX_SELECTION  :
            pSet = pSelectionSet;
            if( pSelectionSet )
                pSet->Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_SECTION    :
        case LISTBOX_SECTIONS   :
            pSet = pSectionSet;
            pSet->Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_PAGE       :
            nWidth = nPageWidth;
            pSet = pPageSet;
            pSet->Put(SwFormatFrameSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_FRAME:
            pSet = pFrameSet;
        break;
    }

    bool bIsSection = pSet == pSectionSet || pSet == pSelectionSet;
    pTabPage->ShowBalance(bIsSection);
    pTabPage->SetInSection(bIsSection);
    pTabPage->SetFrameMode(true);
    pTabPage->SetPageWidth(nWidth);
    if( pSet )
        pTabPage->Reset(pSet);
}

IMPL_LINK_NOARG(SwColumnDlg, OkHdl, Button*, void)
{
    // evaluate current selection
    SfxItemSet* pSet = EvalCurrentSelection();
    pTabPage->FillItemSet(pSet);

    if(pSelectionSet && SfxItemState::SET == pSelectionSet->GetItemState(RES_COL))
    {
        //insert region with columns
        const SwFormatCol& rColItem = static_cast<const SwFormatCol&>(pSelectionSet->Get(RES_COL));
        //only if there actually are columns!
        if(rColItem.GetNumCols() > 1)
            rWrtShell.GetView().GetViewFrame()->GetDispatcher()->Execute(
                FN_INSERT_REGION, SfxCallMode::ASYNCHRON, *pSelectionSet );
    }

    if(pSectionSet && pSectionSet->Count() && bSectionChanged )
    {
        const SwSection* pCurrSection = rWrtShell.GetCurrSection();
        const SwSectionFormat* pFormat = pCurrSection->GetFormat();
        const size_t nNewPos = rWrtShell.GetSectionFormatPos( *pFormat );
        SwSectionData aData(*pCurrSection);
        rWrtShell.UpdateSection( nNewPos, aData, pSectionSet );
    }

    if(pSectionSet && pSectionSet->Count() && bSelSectionChanged )
    {
        rWrtShell.SetSectionAttr( *pSectionSet );
    }

    if(pPageSet && SfxItemState::SET == pPageSet->GetItemState(RES_COL) && bPageChanged)
    {
        // determine current PageDescriptor and fill the Set with it
        const size_t nCurIdx = rWrtShell.GetCurPageDesc();
        SwPageDesc aPageDesc(rWrtShell.GetPageDesc(nCurIdx));
        SwFrameFormat &rFormat = aPageDesc.GetMaster();
        rFormat.SetFormatAttr(pPageSet->Get(RES_COL));
        rWrtShell.ChgPageDesc(nCurIdx, aPageDesc);
    }
    if(pFrameSet && SfxItemState::SET == pFrameSet->GetItemState(RES_COL) && bFrameChanged)
    {
        SfxItemSet aTmp(*pFrameSet->GetPool(), svl::Items<RES_COL, RES_COL>{});
        aTmp.Put(*pFrameSet);
        rWrtShell.StartAction();
        rWrtShell.Push();
        rWrtShell.SetFlyFrameAttr( aTmp );
        // undo the frame selection again
        if(rWrtShell.IsFrameSelected())
        {
            rWrtShell.UnSelectFrame();
            rWrtShell.LeaveSelFrameMode();
        }
        rWrtShell.Pop();
        rWrtShell.EndAction();
    }
    EndDialog(RET_OK);
}

SfxItemSet* SwColumnDlg::EvalCurrentSelection(void)
{
    SfxItemSet* pSet = nullptr;

    switch(nOldSelection)
    {
        case LISTBOX_SELECTION  :
            pSet = pSelectionSet;
        break;
        case LISTBOX_SECTION    :
            pSet = pSectionSet;
            bSectionChanged = true;
        break;
        case LISTBOX_SECTIONS   :
            pSet = pSectionSet;
            bSelSectionChanged = true;
        break;
        case LISTBOX_PAGE       :
            pSet = pPageSet;
            bPageChanged = true;
        break;
        case LISTBOX_FRAME:
            pSet = pFrameSet;
            bFrameChanged = true;
        break;
    }

    return pSet;
}

#if OSL_DEBUG_LEVEL < 2
inline
#endif
sal_uInt16 GetMaxWidth( SwColMgr* pColMgr, sal_uInt16 nCols )
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
        const sal_uInt16 nWidth = GetMaxWidth( m_pColMgr, m_nCols ) / m_nCols;

        for(sal_uInt16 i = 0; i < m_nCols; ++i)
            m_nColWidth[i] = (long) nWidth;
    }

}

// Now as TabPage
SwColumnPage::SwColumnPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "ColumnPage", "modules/swriter/ui/columnpage.ui", &rSet)
    , m_pColMgr(nullptr)
    , m_nFirstVis(0)
    , m_nMinWidth(MINLAY)
    , m_pModifiedField(nullptr)
    , m_bFormat(false)
    , m_bFrame(false)
    , m_bHtmlMode(false)
    , m_bLockUpdate(false)
{
    get(m_pCLNrEdt, "colsnf");
    get(m_pBalanceColsCB, "balance");
    get(m_pBtnBack, "back");
    get(m_pLbl1, "1");
    get(m_pLbl2, "2");
    get(m_pLbl3, "3");
    get(m_pBtnNext, "next");
    get(m_pAutoWidthBox, "autowidth");
    get(m_pLineTypeLbl, "linestyleft");
    get(m_pLineWidthLbl, "linewidthft");
    get(m_pLineWidthEdit, "linewidthmf");
    get(m_pLineColorLbl, "linecolorft");
    get(m_pLineHeightLbl, "lineheightft");
    get(m_pLineHeightEdit, "lineheightmf");
    get(m_pLinePosLbl, "lineposft");
    get(m_pLinePosDLB, "lineposlb");
    get(m_pTextDirectionFT, "textdirectionft");
    get(m_pTextDirectionLB, "textdirectionlb");
    get(m_pLineColorDLB, "colorlb");
    get(m_pLineTypeDLB, "linestylelb");

    get(m_pDefaultVS, "valueset");
    get(m_pPgeExampleWN, "pageexample");
    get(m_pFrameExampleWN, "frameexample");

    connectPercentField(m_aEd1, "width1mf");
    connectPercentField(m_aEd2, "width2mf");
    connectPercentField(m_aEd3, "width3mf");
    connectPercentField(m_aDistEd1, "spacing1mf");
    connectPercentField(m_aDistEd2, "spacing2mf");

    SetExchangeSupport();

    m_pDefaultVS->SetColCount( 5 );

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
        m_pDefaultVS->InsertItem( i + 1, aItemText, i );
    }

    m_pDefaultVS->SetSelectHdl(LINK(this, SwColumnPage, SetDefaultsHdl));

    Link<Edit&,void> aCLNrLk = LINK(this, SwColumnPage, ColModify);
    m_pCLNrEdt->SetModifyHdl(aCLNrLk);
    Link<Edit&,void> aLk = LINK(this, SwColumnPage, GapModify);
    m_aDistEd1.SetModifyHdl(aLk);
    m_aDistEd2.SetModifyHdl(aLk);

    aLk = LINK(this, SwColumnPage, EdModify);

    m_aEd1.SetModifyHdl(aLk);

    m_aEd2.SetModifyHdl(aLk);

    m_aEd3.SetModifyHdl(aLk);

    m_pBtnBack->SetClickHdl(LINK(this, SwColumnPage, Up));
    m_pBtnNext->SetClickHdl(LINK(this, SwColumnPage, Down));
    m_pAutoWidthBox->SetClickHdl(LINK(this, SwColumnPage, AutoWidthHdl));

    aLk = LINK( this, SwColumnPage, UpdateColMgr );
    m_pLineTypeDLB->SetSelectHdl(LINK(this, SwColumnPage, UpdateColMgrListBox));
    m_pLineWidthEdit->SetModifyHdl( aLk );
    m_pLineColorDLB->SetSelectHdl(LINK( this, SwColumnPage, UpdateColMgrColorBox));
    m_pLineHeightEdit->SetModifyHdl( aLk );
    m_pLinePosDLB->SetSelectHdl(LINK(this, SwColumnPage, UpdateColMgrListBox));

    // Separator line
    m_pLineTypeDLB->SetSourceUnit( FUNIT_TWIP );

    // Fill the line styles listbox
    m_pLineTypeDLB->SetNone( SvxResId( RID_SVXSTR_NONE ) );
    m_pLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::SOLID),
        SvxBorderLineStyle::SOLID );
    m_pLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DOTTED),
        SvxBorderLineStyle::DOTTED );
    m_pLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(SvxBorderLineStyle::DASHED),
        SvxBorderLineStyle::DASHED );

    long nLineWidth = static_cast<long>(MetricField::ConvertDoubleValue(
            m_pLineWidthEdit->GetValue( ),
            m_pLineWidthEdit->GetDecimalDigits( ),
            m_pLineWidthEdit->GetUnit(), MapUnit::MapTwip ));
    m_pLineTypeDLB->SetWidth( nLineWidth );
    m_pLineColorDLB->SelectEntry(COL_BLACK);
}

SwColumnPage::~SwColumnPage()
{
    disposeOnce();
}

void SwColumnPage::dispose()
{
    delete m_pColMgr;
    m_pCLNrEdt.clear();
    m_pDefaultVS.clear();
    m_pBalanceColsCB.clear();
    m_pBtnBack.clear();
    m_pLbl1.clear();
    m_pLbl2.clear();
    m_pLbl3.clear();
    m_pBtnNext.clear();
    m_pAutoWidthBox.clear();
    m_pLineTypeLbl.clear();
    m_pLineTypeDLB.clear();
    m_pLineWidthLbl.clear();
    m_pLineWidthEdit.clear();
    m_pLineColorLbl.clear();
    m_pLineColorDLB.clear();
    m_pLineHeightLbl.clear();
    m_pLineHeightEdit.clear();
    m_pLinePosLbl.clear();
    m_pLinePosDLB.clear();
    m_pTextDirectionFT.clear();
    m_pTextDirectionLB.clear();
    m_pPgeExampleWN.clear();
    m_pFrameExampleWN.clear();
    m_aPercentFieldsMap.clear();
    SfxTabPage::dispose();
}

void SwColumnPage::SetPageWidth(long nPageWidth)
{
    long nNewMaxWidth = static_cast< long >(m_aEd1.NormalizePercent(nPageWidth));

    m_aDistEd1.SetMax(nNewMaxWidth, FUNIT_TWIP);
    m_aDistEd2.SetMax(nNewMaxWidth, FUNIT_TWIP);
    m_aEd1.SetMax(nNewMaxWidth, FUNIT_TWIP);
    m_aEd2.SetMax(nNewMaxWidth, FUNIT_TWIP);
    m_aEd3.SetMax(nNewMaxWidth, FUNIT_TWIP);
}

void SwColumnPage::connectPercentField(PercentField &rWrap, const OString &rName)
{
    MetricField *pField = get<MetricField>(rName);
    assert(pField);
    rWrap.set(pField);
    m_aPercentFieldsMap[pField] = &rWrap;
}

void SwColumnPage::Reset(const SfxItemSet *rSet)
{
    const sal_uInt16 nHtmlMode =
        ::GetHtmlMode(static_cast<const SwDocShell*>(SfxObjectShell::Current()));
    if(nHtmlMode & HTMLMODE_ON)
    {
        m_bHtmlMode = true;
        m_pAutoWidthBox->Enable(false);
    }
    FieldUnit aMetric = ::GetDfltMetric(m_bHtmlMode);
    m_aEd1.SetMetric(aMetric);
    m_aEd2.SetMetric(aMetric);
    m_aEd3.SetMetric(aMetric);
    m_aDistEd1.SetMetric(aMetric);
    m_aDistEd2.SetMetric(aMetric);

    delete m_pColMgr;
    m_pColMgr = new SwColMgr(*rSet);
    m_nCols   = m_pColMgr->GetCount() ;
    m_pCLNrEdt->SetMax(std::max((sal_uInt16)m_pCLNrEdt->GetMax(), m_nCols));
    m_pCLNrEdt->SetLast(std::max(m_nCols,(sal_uInt16)m_pCLNrEdt->GetMax()));

    if(m_bFrame)
    {
        if(m_bFormat)                     // there is no size here
            m_pColMgr->SetActualWidth(FRAME_FORMAT_WIDTH);
        else
        {
            const SwFormatFrameSize& rSize = static_cast<const SwFormatFrameSize&>(rSet->Get(RES_FRM_SIZE));
            const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(rSet->Get(RES_BOX));
            m_pColMgr->SetActualWidth((sal_uInt16)rSize.GetSize().Width() - rBox.GetSmallestDistance());
        }
    }
    if(m_pBalanceColsCB->IsVisible())
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == rSet->GetItemState( RES_COLUMNBALANCE, false, &pItem ))
            m_pBalanceColsCB->Check(!static_cast<const SwFormatNoBalancedColumns*>(pItem)->GetValue());
        else
            m_pBalanceColsCB->Check();
    }

    //text direction
    if( SfxItemState::DEFAULT <= rSet->GetItemState( RES_FRAMEDIR ) )
    {
        const SvxFrameDirectionItem& rItem = static_cast<const SvxFrameDirectionItem&>(rSet->Get(RES_FRAMEDIR));
        SvxFrameDirection nVal  = rItem.GetValue();
        const sal_Int32 nPos = m_pTextDirectionLB->GetEntryPos( reinterpret_cast<void*>(nVal) );
        m_pTextDirectionLB->SelectEntryPos( nPos );
        m_pTextDirectionLB->SaveValue();
    }

    Init();
    ActivatePage( *rSet );
}

// create TabPage
VclPtr<SfxTabPage> SwColumnPage::Create(vcl::Window *pParent, const SfxItemSet *rSet)
{
    return VclPtr<SwColumnPage>::Create(pParent, *rSet);
}

// stuff attributes into the Set when OK
bool SwColumnPage::FillItemSet(SfxItemSet *rSet)
{
    if(m_pCLNrEdt->HasChildPathFocus())
        m_pCLNrEdt->GetDownHdl().Call(*m_pCLNrEdt);
    // set in ItemSet setzen
    // the current settings are already present

    const SfxPoolItem* pOldItem;
    const SwFormatCol& rCol = m_pColMgr->GetColumns();
    if(nullptr == (pOldItem = GetOldItem( *rSet, RES_COL )) ||
                rCol != *pOldItem )
        rSet->Put(rCol);

    if(m_pBalanceColsCB->IsVisible() )
    {
        rSet->Put(SwFormatNoBalancedColumns(!m_pBalanceColsCB->IsChecked() ));
    }
    if( m_pTextDirectionLB->IsVisible())
    {
        const sal_Int32 nPos = m_pTextDirectionLB->GetSelectEntryPos();
        if ( m_pTextDirectionLB->IsValueChangedFromSaved() )
        {
            sal_uInt32 nDirection = (sal_uInt32)reinterpret_cast<sal_IntPtr>(m_pTextDirectionLB->GetEntryData( nPos ));
            rSet->Put( SvxFrameDirectionItem( (SvxFrameDirection)nDirection, RES_FRAMEDIR));
        }
    }
    return true;
}

// update ColumnManager
IMPL_LINK_NOARG( SwColumnPage, UpdateColMgrListBox, ListBox&, void )
{
    UpdateColMgr(*m_pLineWidthEdit);
}

IMPL_LINK_NOARG( SwColumnPage, UpdateColMgrColorBox, SvxColorListBox&, void )
{
    UpdateColMgr(*m_pLineWidthEdit);
}

IMPL_LINK_NOARG( SwColumnPage, UpdateColMgr, Edit&, void )
{
    if (!m_pColMgr)
        return;
    long nGutterWidth = m_pColMgr->GetGutterWidth();
    if (m_nCols > 1)
    {
            // Determine whether the most narrow column is too narrow
            // for the adjusted column gap
        long nMin = m_nColWidth[0];

        for( sal_uInt16 i = 1; i < m_nCols; ++i )
            nMin = std::min(nMin, m_nColWidth[i]);

        bool bAutoWidth = m_pAutoWidthBox->IsChecked();
        if(!bAutoWidth)
        {
            m_pColMgr->SetAutoWidth(false);
                // when the user didn't allocate the whole width,
                // add the missing amount to the last column.
            long nSum = 0;
            for(sal_uInt16 i = 0; i < m_nCols; ++i)
                nSum += m_nColWidth[i];
            nGutterWidth = 0;
            for(sal_uInt16 i = 0; i < m_nCols - 1; ++i)
                nGutterWidth += m_nColDist[i];
            nSum += nGutterWidth;

            long nMaxW = m_pColMgr->GetActualSize();

            if( nSum < nMaxW  )
                m_nColWidth[m_nCols - 1] += nMaxW - nSum;

            m_pColMgr->SetColWidth( 0, static_cast< sal_uInt16 >(m_nColWidth[0] + m_nColDist[0]/2) );
            for( sal_uInt16 i = 1; i < m_nCols-1; ++i )
            {
                long nActDist = (m_nColDist[i] + m_nColDist[i - 1]) / 2;
                m_pColMgr->SetColWidth( i, static_cast< sal_uInt16 >(m_nColWidth[i] + nActDist ));
            }
            m_pColMgr->SetColWidth( m_nCols-1, static_cast< sal_uInt16 >(m_nColWidth[m_nCols-1] + m_nColDist[m_nCols -2]/2) );

        }

        bool bEnable = isLineNotNone();
        m_pLineHeightEdit->Enable( bEnable );
        m_pLineHeightLbl->Enable( bEnable );
        m_pLineWidthLbl->Enable( bEnable );
        m_pLineWidthEdit->Enable( bEnable );
        m_pLineColorDLB->Enable( bEnable );
        m_pLineColorLbl->Enable( bEnable );

        long nLineWidth = static_cast<long>(MetricField::ConvertDoubleValue(
                m_pLineWidthEdit->GetValue( ),
                m_pLineWidthEdit->GetDecimalDigits( ),
                m_pLineWidthEdit->GetUnit(), MapUnit::MapTwip ));
        if( !bEnable )
            m_pColMgr->SetNoLine();
        else
        {
            m_pColMgr->SetLineWidthAndColor(
                    m_pLineTypeDLB->GetSelectEntryStyle(),
                    nLineWidth,
                    m_pLineColorDLB->GetSelectEntryColor() );
            m_pColMgr->SetAdjust( SwColLineAdj(
                                    m_pLinePosDLB->GetSelectEntryPos() + 1) );
            m_pColMgr->SetLineHeightPercent((short)m_pLineHeightEdit->GetValue());
            bEnable = m_pColMgr->GetLineHeightPercent() != 100;
        }
        m_pLinePosLbl->Enable( bEnable );
        m_pLinePosDLB->Enable( bEnable );

        //fdo#66815 if the values are going to be the same, don't update
        //them to avoid the listbox selection resetting
        if (nLineWidth != m_pLineTypeDLB->GetWidth())
            m_pLineTypeDLB->SetWidth(nLineWidth);
        Color aColor(m_pLineColorDLB->GetSelectEntryColor());
        if (aColor != m_pLineTypeDLB->GetColor())
            m_pLineTypeDLB->SetColor(aColor);
    }
    else
    {
        m_pColMgr->NoCols();
        m_nCols = 0;
    }

    //set maximum values
    m_pCLNrEdt->SetMax(std::max(1L,
        std::min(long(nMaxCols), long( m_pColMgr->GetActualSize() / (nGutterWidth + MINLAY)) )));
    m_pCLNrEdt->SetLast(m_pCLNrEdt->GetMax());
    m_pCLNrEdt->Reformat();

    //prompt example window
    if(!m_bLockUpdate)
    {
        if(m_bFrame)
        {
            m_pFrameExampleWN->SetColumns( m_pColMgr->GetColumns() );
            m_pFrameExampleWN->Invalidate();
        }
        else
            m_pPgeExampleWN->Invalidate();
    }
}

void SwColumnPage::Init()
{
    m_pCLNrEdt->SetValue(m_nCols);

    bool bAutoWidth = m_pColMgr->IsAutoWidth() || m_bHtmlMode;
    m_pAutoWidthBox->Check( bAutoWidth );

    sal_Int32 nColumnWidthSum = 0;
    // set the widths
    for(sal_uInt16 i = 0; i < m_nCols; ++i)
    {
        m_nColWidth[i] = m_pColMgr->GetColWidth(i);
        nColumnWidthSum += m_nColWidth[i];
        if(i < m_nCols - 1)
            m_nColDist[i] = m_pColMgr->GetGutterWidth(i);
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
        SwColLineAdj eAdj = m_pColMgr->GetAdjust();
        if( COLADJ_NONE == eAdj )       // the dialog doesn't know a NONE!
        {
            eAdj = COLADJ_TOP;
            //without Adjust no line type
            m_pLineTypeDLB->SelectEntryPos( 0 );
            m_pLineHeightEdit->SetValue( 100 );
        }
        else
        {
            // Need to multiply by 100 because of the 2 decimals
            m_pLineWidthEdit->SetValue( m_pColMgr->GetLineWidth() * 100, FUNIT_TWIP );
            m_pLineColorDLB->SelectEntry( m_pColMgr->GetLineColor() );
            m_pLineTypeDLB->SelectEntry( m_pColMgr->GetLineStyle() );
            m_pLineTypeDLB->SetWidth( m_pColMgr->GetLineWidth( ) );
            m_pLineHeightEdit->SetValue( m_pColMgr->GetLineHeightPercent() );

        }
        m_pLinePosDLB->SelectEntryPos( static_cast< sal_Int32 >(eAdj - 1) );
    }
    else
    {
        m_pLinePosDLB->SelectEntryPos( 0 );
        m_pLineTypeDLB->SelectEntryPos( 0 );
        m_pLineHeightEdit->SetValue( 100 );
    }

    UpdateCols();
    Update(nullptr);

        // set maximum number of columns
        // values below 1 are not allowed
    m_pCLNrEdt->SetMax(std::max(1L,
        std::min(long(nMaxCols), long( m_pColMgr->GetActualSize() / m_nMinWidth) )));
}

bool SwColumnPage::isLineNotNone() const
{
    // nothing is turned off
    const sal_Int32 nPos = m_pLineTypeDLB->GetSelectEntryPos();
    return nPos != LISTBOX_ENTRY_NOTFOUND && nPos != 0;
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
    const bool bEdit = !m_pAutoWidthBox->IsChecked();
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
                SAL_FALLTHROUGH;
            case 2: bEnable12= true; break;
            default: /* do nothing */;
        }
    }
    m_aEd1.Enable( bEnable12 );
    bool bEnable = m_nCols > 1;
    m_aDistEd1.Enable(bEnable);
    m_pAutoWidthBox->Enable( bEnable && !m_bHtmlMode );
    m_aEd2.Enable( bEnable12 );
    m_aDistEd2.Enable(bEnable3);
    m_aEd3.Enable( bEnable3  );
    m_pLbl1->Enable(bEnable12 );
    m_pLbl2->Enable(bEnable12 );
    m_pLbl3->Enable(bEnable3  );
    m_pBtnBack->Enable( bEnableBtns );
    m_pBtnNext->Enable( bEnableBtns );

    m_pLineTypeDLB->Enable( bEnable );
    m_pLineTypeLbl->Enable( bEnable );

    if (bEnable)
    {
        bEnable = isLineNotNone();
    }

    //all these depend on > 1 column and line style != none
    m_pLineHeightEdit->Enable( bEnable );
    m_pLineHeightLbl->Enable( bEnable );
    m_pLineWidthLbl->Enable( bEnable );
    m_pLineWidthEdit->Enable( bEnable );
    m_pLineColorDLB->Enable( bEnable );
    m_pLineColorLbl->Enable( bEnable );

    if (bEnable)
        bEnable = m_pColMgr->GetLineHeightPercent() != 100;

    //and these additionally depend on line height != 100%
    m_pLinePosDLB->Enable( bEnable );
    m_pLinePosLbl->Enable( bEnable );
}

void SwColumnPage::SetLabels( sal_uInt16 nVis )
{
    //insert ~ before the last character, e.g. 1 -> ~1, 10 -> 1~0
    const OUString sLbl( '~' );

    const OUString sLbl1(OUString::number( nVis + 1 ));
    m_pLbl1->SetText(sLbl1.replaceAt(sLbl1.getLength()-1, 0, sLbl));

    const OUString sLbl2(OUString::number( nVis + 2 ));
    m_pLbl2->SetText(sLbl2.replaceAt(sLbl2.getLength()-1, 0, sLbl));

    const OUString sLbl3(OUString::number( nVis + 3 ));
    m_pLbl3->SetText(sLbl3.replaceAt(sLbl3.getLength()-1, 0, sLbl));

    const OUString sColumnWidth = SwResId( STR_ACCESS_COLUMN_WIDTH ) ;
    m_aEd1.SetAccessibleName(sColumnWidth.replaceFirst("%1", sLbl1));
    m_aEd2.SetAccessibleName(sColumnWidth.replaceFirst("%1", sLbl2));
    m_aEd3.SetAccessibleName(sColumnWidth.replaceFirst("%1", sLbl3));

    const OUString sDist = SwResId( STR_ACCESS_PAGESETUP_SPACING ) ;
    m_aDistEd1.SetAccessibleName(
        sDist.replaceFirst("%1", sLbl1).replaceFirst("%2", sLbl2));

    m_aDistEd2.SetAccessibleName(
        sDist.replaceFirst("%1", sLbl2).replaceFirst("%2", sLbl3));
}

/*
 * Handler that is called at alteration of the column number. An alteration of
 * the column number overwrites potential user's width settings; all columns
 * are equally wide.
 */
IMPL_LINK( SwColumnPage, ColModify, Edit&, rEdit, void )
{
    ColModify(static_cast<NumericField*>(&rEdit));
}

void SwColumnPage::ColModify(NumericField* pNF)
{
    m_nCols = (sal_uInt16)m_pCLNrEdt->GetValue();
    //#107890# the handler is also called from LoseFocus()
    //then no change has been made and thus no action should be taken
    // #i17816# changing the displayed types within the ValueSet
    //from two columns to two columns with different settings doesn't invalidate the
    // example windows in ::ColModify()
    if (!pNF || m_pColMgr->GetCount() != m_nCols)
    {
        if(pNF)
            m_pDefaultVS->SetNoSelection();
        long nDist = static_cast< long >(m_aDistEd1.DenormalizePercent(m_aDistEd1.GetValue(FUNIT_TWIP)));
        m_pColMgr->SetCount(m_nCols, (sal_uInt16)nDist);
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
IMPL_LINK( SwColumnPage, GapModify, Edit&, rEdit, void )
{
    if (m_nCols < 2)
        return;
    MetricField* pMetricField = static_cast<MetricField*>(&rEdit);
    PercentField *pField = m_aPercentFieldsMap[pMetricField];
    assert(pField);
    long nActValue = static_cast< long >(pField->DenormalizePercent(pField->GetValue(FUNIT_TWIP)));
    if(m_pAutoWidthBox->IsChecked())
    {
        const long nMaxGap = static_cast< long >
            ((m_pColMgr->GetActualSize() - m_nCols * MINLAY)/(m_nCols - 1));
        if(nActValue > nMaxGap)
        {
            nActValue = nMaxGap;
            m_aDistEd1.SetPrcntValue(m_aDistEd1.NormalizePercent(nMaxGap), FUNIT_TWIP);
        }
        m_pColMgr->SetGutterWidth((sal_uInt16)nActValue);
        for(sal_uInt16 i = 0; i < m_nCols; i++)
            m_nColDist[i] = nActValue;

        ResetColWidth();
        UpdateCols();
    }
    else
    {
        const sal_uInt16 nVis = m_nFirstVis + ((pField == &m_aDistEd2) ? 1 : 0);
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

            m_pColMgr->SetColWidth( nVis, sal_uInt16(nLeft) );
            m_pColMgr->SetColWidth( nVis + 1, sal_uInt16(nRight) );
            m_pColMgr->SetGutterWidth( sal_uInt16(m_nColDist[nVis]), nVis );
        }

    }
    Update(pMetricField);
}

IMPL_LINK( SwColumnPage, EdModify, Edit&, rEdit, void )
{
    MetricField * pMetricField = static_cast<MetricField*>(&rEdit);
    PercentField *pField = m_aPercentFieldsMap[pMetricField];
    assert(pField);
    m_pModifiedField = pField;
    Timeout();
}

// Handler behind the Checkbox for automatic width. When the box is checked
// no explicit values for the column width can be entered.
IMPL_LINK( SwColumnPage, AutoWidthHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    long nDist = static_cast< long >(m_aDistEd1.DenormalizePercent(m_aDistEd1.GetValue(FUNIT_TWIP)));
    m_pColMgr->SetCount(m_nCols, (sal_uInt16)nDist);
    for(sal_uInt16 i = 0; i < m_nCols; i++)
        m_nColDist[i] = nDist;
    if(pBox->IsChecked())
    {
        m_pColMgr->SetGutterWidth(sal_uInt16(nDist));
        ResetColWidth();
    }
    m_pColMgr->SetAutoWidth(pBox->IsChecked(), sal_uInt16(nDist));
    UpdateCols();
    Update(nullptr);
}

// scroll up the contents of the edits
IMPL_LINK_NOARG(SwColumnPage, Up, Button*, void)
{
    if( m_nFirstVis )
    {
        --m_nFirstVis;
        SetLabels( m_nFirstVis );
        Update(nullptr);
    }
}

// scroll down the contents of the edits.
IMPL_LINK_NOARG(SwColumnPage, Down, Button*, void)
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
    PercentField *pField = m_pModifiedField;
    if (m_pModifiedField)
    {
            // find the changed column
        sal_uInt16 nChanged = m_nFirstVis;
        if(m_pModifiedField == &m_aEd2)
            ++nChanged;
        else if(m_pModifiedField == &m_aEd3)
            nChanged += 2;

        long nNewWidth = static_cast< long >
            (m_pModifiedField->DenormalizePercent(m_pModifiedField->GetValue(FUNIT_TWIP)));
        long nDiff = nNewWidth - m_nColWidth[nChanged];

        // when it's the last column
        if(nChanged == m_nCols - 1)
        {
            m_nColWidth[0] -= nDiff;
            if(m_nColWidth[0] < (long)m_nMinWidth)
            {
                nNewWidth -= m_nMinWidth - m_nColWidth[0];
                m_nColWidth[0] = m_nMinWidth;
            }

        }
        else if(nDiff)
        {
            m_nColWidth[nChanged + 1] -= nDiff;
            if(m_nColWidth[nChanged + 1] < (long) m_nMinWidth)
            {
                nNewWidth -= m_nMinWidth - m_nColWidth[nChanged + 1];
                m_nColWidth[nChanged + 1] = m_nMinWidth;
            }
        }
        m_nColWidth[nChanged] = nNewWidth;
        m_pModifiedField = nullptr;
    }

    Update(pField ? pField->get() : nullptr);
}

// Update the view
void SwColumnPage::Update(MetricField *pInteractiveField)
{
    m_pBalanceColsCB->Enable(m_nCols > 1);
    if(m_nCols >= 2)
    {
        sal_Int64 nCurrentValue, nNewValue;

        nCurrentValue = m_aEd1.NormalizePercent(m_aEd1.DenormalizePercent(m_aEd1.GetValue(FUNIT_TWIP)));
        nNewValue = m_aEd1.NormalizePercent(m_nColWidth[m_nFirstVis]);

        //fdo#87612 if we're interacting with this widget and the value will be the same
        //then leave it alone (i.e. don't change equivalent values of e.g. .8 -> 0.8)
        if (nNewValue != nCurrentValue || pInteractiveField != m_aEd1.get())
            m_aEd1.SetPrcntValue(nNewValue, FUNIT_TWIP);

        nCurrentValue = m_aDistEd1.NormalizePercent(m_aDistEd1.DenormalizePercent(m_aDistEd1.GetValue(FUNIT_TWIP)));
        nNewValue = m_aDistEd1.NormalizePercent(m_nColDist[m_nFirstVis]);
        if (nNewValue != nCurrentValue || pInteractiveField != m_aDistEd1.get())
            m_aDistEd1.SetPrcntValue(nNewValue, FUNIT_TWIP);

        nCurrentValue = m_aEd2.NormalizePercent(m_aEd2.DenormalizePercent(m_aEd2.GetValue(FUNIT_TWIP)));
        nNewValue = m_aEd2.NormalizePercent(m_nColWidth[m_nFirstVis+1]);
        if (nNewValue != nCurrentValue || pInteractiveField != m_aEd2.get())
            m_aEd2.SetPrcntValue(nNewValue, FUNIT_TWIP);

        if(m_nCols >= 3)
        {
            nCurrentValue = m_aDistEd2.NormalizePercent(m_aDistEd2.DenormalizePercent(m_aDistEd2.GetValue(FUNIT_TWIP)));
            nNewValue = m_aDistEd2.NormalizePercent(m_nColDist[m_nFirstVis+1]);
            if (nNewValue != nCurrentValue || pInteractiveField != m_aDistEd2.get())
                m_aDistEd2.SetPrcntValue(nNewValue, FUNIT_TWIP);

            nCurrentValue = m_aEd3.NormalizePercent(m_aEd3.DenormalizePercent(m_aEd3.GetValue(FUNIT_TWIP)));
            nNewValue = m_aEd3.NormalizePercent(m_nColWidth[m_nFirstVis+2]);
            if (nNewValue != nCurrentValue || pInteractiveField != m_aEd3.get())
                m_aEd3.SetPrcntValue(nNewValue, FUNIT_TWIP);
        }
        else
        {
            m_aEd3.SetText(OUString());
            m_aDistEd2.SetText(OUString());
        }
    }
    else
    {
        m_aEd1.SetText(OUString());
        m_aEd2.SetText(OUString());
        m_aEd3.SetText(OUString());
        m_aDistEd1.SetText(OUString());
        m_aDistEd2.SetText(OUString());
    }
    UpdateColMgr(*m_pLineWidthEdit);
}

// Update Bsp
void SwColumnPage::ActivatePage(const SfxItemSet& rSet)
{
    bool bVertical = false;
    if (SfxItemState::DEFAULT <= rSet.GetItemState(RES_FRAMEDIR))
    {
        const SvxFrameDirectionItem& rDirItem =
                    static_cast<const SvxFrameDirectionItem&>(rSet.Get(RES_FRAMEDIR));
        bVertical = rDirItem.GetValue() == SvxFrameDirection::Vertical_RL_TB||
                    rDirItem.GetValue() == SvxFrameDirection::Vertical_LR_TB;
    }

    if (!m_bFrame)
    {
        if( SfxItemState::SET == rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
        {
            const SvxSizeItem& rSize = static_cast<const SvxSizeItem&>(rSet.Get(
                                                SID_ATTR_PAGE_SIZE));

            sal_uInt16 nActWidth;

            if (!bVertical)
            {
                const SvxLRSpaceItem& rLRSpace = static_cast<const SvxLRSpaceItem&>(rSet.Get(
                                                                    RES_LR_SPACE ));
                const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>( rSet.Get(RES_BOX));
                nActWidth = rSize.GetSize().Width()
                                - rLRSpace.GetLeft() - rLRSpace.GetRight() - rBox.GetSmallestDistance();
            }
            else
            {
                const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(rSet.Get(
                                                                    RES_UL_SPACE ));
                const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>( rSet.Get(RES_BOX));
                nActWidth = rSize.GetSize().Height()
                                - rULSpace.GetUpper() - rULSpace.GetLower() - rBox.GetSmallestDistance();

            }

            if( m_pColMgr->GetActualSize() != nActWidth)
            {
                m_pColMgr->SetActualWidth(nActWidth);
                ColModify( nullptr );
                UpdateColMgr( *m_pLineWidthEdit );
            }
        }
        m_pFrameExampleWN->Hide();
        m_pPgeExampleWN->UpdateExample( rSet, m_pColMgr );
        m_pPgeExampleWN->Show();

    }
    else
    {
        m_pPgeExampleWN->Hide();
        m_pFrameExampleWN->Show();

        // Size
        const SwFormatFrameSize& rSize = static_cast<const SwFormatFrameSize&>(rSet.Get(RES_FRM_SIZE));
        const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>( rSet.Get(RES_BOX));

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

        if(m_pColMgr->GetActualSize() != nTotalWish)
        {
            m_pColMgr->SetActualWidth(nTotalWish);
            Init();
        }
        bool bPercent;
        // only relative data in frame format
        if ( m_bFormat || (rSize.GetWidthPercent() && rSize.GetWidthPercent() != SwFormatFrameSize::SYNCED) )
        {
            // set value for 100%
            m_aEd1.SetRefValue(nTotalWish);
            m_aEd2.SetRefValue(nTotalWish);
            m_aEd3.SetRefValue(nTotalWish);
            m_aDistEd1.SetRefValue(nTotalWish);
            m_aDistEd2.SetRefValue(nTotalWish);

            // switch to %-view
            bPercent = true;
        }
        else
            bPercent = false;

        m_aEd1.ShowPercent(bPercent);
        m_aEd2.ShowPercent(bPercent);
        m_aEd3.ShowPercent(bPercent);
        m_aDistEd1.ShowPercent(bPercent);
        m_aDistEd2.ShowPercent(bPercent);
        m_aDistEd1.SetMetricFieldMin(0);
        m_aDistEd2.SetMetricFieldMin(0);
    }
    Update(nullptr);
}

DeactivateRC SwColumnPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

IMPL_LINK( SwColumnPage, SetDefaultsHdl, ValueSet *, pVS, void )
{
    const sal_uInt16 nItem = pVS->GetSelectItemId();
    if( nItem < 4 )
    {
        m_pCLNrEdt->SetValue( nItem );
        m_pAutoWidthBox->Check();
        m_aDistEd1.SetPrcntValue(0);
        ColModify(nullptr);
    }
    else
    {
        m_bLockUpdate = true;
        m_pCLNrEdt->SetValue( 2 );
        m_pAutoWidthBox->Check(false);
        m_aDistEd1.SetPrcntValue(0);
        ColModify(nullptr);
        // now set the width ratio to 2 : 1 or 1 : 2 respectively
        const long nSmall = static_cast< long >(m_pColMgr->GetActualSize() / 3);
        if(nItem == 4)
        {
            m_aEd2.SetPrcntValue(m_aEd2.NormalizePercent(nSmall), FUNIT_TWIP);
            m_pModifiedField = &m_aEd2;
        }
        else
        {
            m_aEd1.SetPrcntValue(m_aEd1.NormalizePercent(nSmall), FUNIT_TWIP);
            m_pModifiedField = &m_aEd1;
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

    m_pTextDirectionFT->Show(bSet);
    m_pTextDirectionLB->Show(bSet);
}

void ColumnValueSet::UserDraw(const UserDrawEvent& rUDEvt)
{
    vcl::RenderContext* pDev = rUDEvt.GetRenderContext();
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

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
            aStart.Y() = aEnd.Y() = aBLPos.Y() + nTop + i * nStep;
            pDev->DrawLine(aStart, aEnd);
        }
    }
    pDev->SetFillColor(aFillColor);
    pDev->SetLineColor(aLineColor);
}

void ColumnValueSet::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        SetFormat();
        Invalidate();
    }
    ValueSet::DataChanged( rDCEvt );
}

VCL_BUILDER_FACTORY(ColumnValueSet)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
