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
#include <sfx2/htmlmode.hxx>
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
#include "poolfmt.hrc"
#include <section.hxx>
#include <docary.hxx>
#include <pagedesc.hxx>

#include "access.hrc"

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
    rWrtShell.CalcBoundRect(aRect, FLY_AS_CHAR);

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
        pPageSet = new SfxItemSet( rWrtShell.GetAttrPool(),
                                    RES_COL, RES_COL,
                                    RES_FRM_SIZE, RES_FRM_SIZE,
                                    RES_LR_SPACE, RES_LR_SPACE,
                                    0 );

        const SwFrameFormat &rFormat = pPageDesc->GetMaster();
        nPageWidth = rFormat.GetFrameSize().GetSize().Width();

        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)rFormat.GetLRSpace();
        const SvxBoxItem& rBox = (const SvxBoxItem&) rFormat.GetBox();
        nPageWidth -= rLRSpace.GetLeft() + rLRSpace.GetRight() + rBox.GetDistance();

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

IMPL_LINK_TYPED(SwColumnDlg, ObjectListBoxHdl, ListBox&, rBox, void)
{
    ObjectHdl(&rBox);
}
void SwColumnDlg::ObjectHdl(ListBox* pBox)
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

IMPL_LINK_NOARG_TYPED(SwColumnDlg, OkHdl, Button*, void)
{
    // evaluate current selection
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
        SfxItemSet aTmp(*pFrameSet->GetPool(), RES_COL, RES_COL);
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
    if( nCols )
    {
        const sal_uInt16 nWidth = GetMaxWidth( pColMgr, nCols ) / nCols;

        for(sal_uInt16 i = 0; i < nCols; ++i)
            nColWidth[i] = (long) nWidth;
    }

}

// Now as TabPage
SwColumnPage::SwColumnPage(vcl::Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "ColumnPage", "modules/swriter/ui/columnpage.ui", &rSet)
    , pColMgr(nullptr)
    , nFirstVis(0)
    , nMinWidth(MINLAY)
    , pModifiedField(nullptr)
    , bFormat(false)
    , bFrame(false)
    , bHtmlMode(false)
    , bLockUpdate(false)
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

    connectPercentField(aEd1, "width1mf");
    connectPercentField(aEd2, "width2mf");
    connectPercentField(aEd3, "width3mf");
    connectPercentField(aDistEd1, "spacing1mf");
    connectPercentField(aDistEd2, "spacing2mf");

    SetExchangeSupport();

    VclFrame *pSpacing = get<VclFrame>("spacing");
    m_pBtnNext->SetAccessibleRelationMemberOf(pSpacing->get_label_widget());

    m_pDefaultVS->SetColCount( 5 );

    for (int i = 0; i < 5; ++i)
    //Set accessible name one by one
    {
        OUString aItemText;
        switch( i )
        {
            case 0:
                aItemText =  SW_RESSTR( STR_COLUMN_VALUESET_ITEM0 ) ;
                break;
            case 1:
                aItemText =  SW_RESSTR( STR_COLUMN_VALUESET_ITEM1 ) ;
                break;
            case 2:
                aItemText =  SW_RESSTR( STR_COLUMN_VALUESET_ITEM2 ) ;
                break;
            case 3:
                aItemText =  SW_RESSTR( STR_COLUMN_VALUESET_ITEM3 );
                break;
            default:
                aItemText =  SW_RESSTR( STR_COLUMN_VALUESET_ITEM4 );
                break;
        }
        m_pDefaultVS->InsertItem( i + 1, aItemText, i );
    }

    m_pDefaultVS->SetSelectHdl(LINK(this, SwColumnPage, SetDefaultsHdl));

    Link<Edit&,void> aCLNrLk = LINK(this, SwColumnPage, ColModify);
    m_pCLNrEdt->SetModifyHdl(aCLNrLk);
    Link<Edit&,void> aLk = LINK(this, SwColumnPage, GapModify);
    aDistEd1.SetModifyHdl(aLk);
    aDistEd2.SetModifyHdl(aLk);

    aLk = LINK(this, SwColumnPage, EdModify);

    aEd1.SetModifyHdl(aLk);

    aEd2.SetModifyHdl(aLk);

    aEd3.SetModifyHdl(aLk);

    m_pBtnBack->SetClickHdl(LINK(this, SwColumnPage, Up));
    m_pBtnNext->SetClickHdl(LINK(this, SwColumnPage, Down));
    m_pAutoWidthBox->SetClickHdl(LINK(this, SwColumnPage, AutoWidthHdl));

    aLk = LINK( this, SwColumnPage, UpdateColMgr );
    Link<ListBox&,void> aLk2 = LINK( this, SwColumnPage, UpdateColMgrListBox );
    m_pLineTypeDLB->SetSelectHdl( aLk2 );
    m_pLineWidthEdit->SetModifyHdl( aLk );
    m_pLineColorDLB->SetSelectHdl( aLk2 );
    m_pLineHeightEdit->SetModifyHdl( aLk );
    m_pLinePosDLB->SetSelectHdl( aLk2 );

    // Separator line
    m_pLineTypeDLB->SetUnit( FUNIT_POINT );
    m_pLineTypeDLB->SetSourceUnit( FUNIT_TWIP );

    // Fill the line styles listbox
    m_pLineTypeDLB->SetNone( SVX_RESSTR( RID_SVXSTR_NONE ) );
    m_pLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::SOLID),
        table::BorderLineStyle::SOLID );
    m_pLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::DOTTED),
        table::BorderLineStyle::DOTTED );
    m_pLineTypeDLB->InsertEntry(
        ::editeng::SvxBorderLine::getWidthImpl(table::BorderLineStyle::DASHED),
        table::BorderLineStyle::DASHED );

    long nLineWidth = static_cast<long>(MetricField::ConvertDoubleValue(
            m_pLineWidthEdit->GetValue( ),
            m_pLineWidthEdit->GetDecimalDigits( ),
            m_pLineWidthEdit->GetUnit(), MAP_TWIP ));
    m_pLineTypeDLB->SetWidth( nLineWidth );

    // Fill the color listbox
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    XColorListRef pColorList;
    if ( pDocSh )
    {
        const SfxPoolItem*  pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != nullptr )
            pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    }

    if ( pColorList.is() )
    {
        m_pLineColorDLB->SetUpdateMode( false );

        for (long i = 0; i < pColorList->Count(); ++i )
        {
            XColorEntry* pEntry = pColorList->GetColor(i);
            m_pLineColorDLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        m_pLineColorDLB->SetUpdateMode( true );
    }
    m_pLineColorDLB->SelectEntryPos( 0 );
}

SwColumnPage::~SwColumnPage()
{
    disposeOnce();
}

void SwColumnPage::dispose()
{
    delete pColMgr;
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
    SfxTabPage::dispose();
}

void SwColumnPage::SetPageWidth(long nPageWidth)
{
    long nNewMaxWidth = static_cast< long >(aEd1.NormalizePercent(nPageWidth));

    aDistEd1.SetMax(nNewMaxWidth, FUNIT_TWIP);
    aDistEd2.SetMax(nNewMaxWidth, FUNIT_TWIP);
    aEd1.SetMax(nNewMaxWidth, FUNIT_TWIP);
    aEd2.SetMax(nNewMaxWidth, FUNIT_TWIP);
    aEd3.SetMax(nNewMaxWidth, FUNIT_TWIP);
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
        bHtmlMode = true;
        m_pAutoWidthBox->Enable(false);
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    aEd1.SetMetric(aMetric);
    aEd2.SetMetric(aMetric);
    aEd3.SetMetric(aMetric);
    aDistEd1.SetMetric(aMetric);
    aDistEd2.SetMetric(aMetric);

    delete pColMgr;
    pColMgr = new SwColMgr(*rSet);
    nCols   = pColMgr->GetCount() ;
    m_pCLNrEdt->SetMax(std::max((sal_uInt16)m_pCLNrEdt->GetMax(), nCols));
    m_pCLNrEdt->SetLast(std::max(nCols,(sal_uInt16)m_pCLNrEdt->GetMax()));

    if(bFrame)
    {
        if(bFormat)                     // there is no size here
            pColMgr->SetActualWidth(FRAME_FORMAT_WIDTH);
        else
        {
            const SwFormatFrameSize& rSize = static_cast<const SwFormatFrameSize&>(rSet->Get(RES_FRM_SIZE));
            const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>(rSet->Get(RES_BOX));
            pColMgr->SetActualWidth((sal_uInt16)rSize.GetSize().Width() - rBox.GetDistance());
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
        sal_uIntPtr nVal  = rItem.GetValue();
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
    const SwFormatCol& rCol = pColMgr->GetColumns();
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
IMPL_LINK_NOARG_TYPED( SwColumnPage, UpdateColMgrListBox, ListBox&, void )
{
    UpdateColMgr(*m_pLineWidthEdit);
}
IMPL_LINK_NOARG_TYPED( SwColumnPage, UpdateColMgr, Edit&, void )
{
    long nGutterWidth = pColMgr->GetGutterWidth();
    if(nCols > 1)
    {
            // Determine whether the most narrow column is too narrow
            // for the adjusted column gap
        long nMin = nColWidth[0];

        for( sal_uInt16 i = 1; i < nCols; ++i )
            nMin = std::min(nMin, nColWidth[i]);

        bool bAutoWidth = m_pAutoWidthBox->IsChecked();
        if(!bAutoWidth)
        {
            pColMgr->SetAutoWidth(false);
                // when the user didn't allocate the whole width,
                // add the missing amount to the last column.
            long nSum = 0;
            for(sal_uInt16 i = 0; i < nCols; ++i)
                nSum += nColWidth[i];
            nGutterWidth = 0;
            for(sal_uInt16 i = 0; i < nCols - 1; ++i)
                nGutterWidth += nColDist[i];
            nSum += nGutterWidth;

            long nMaxW = pColMgr->GetActualSize();

            if( nSum < nMaxW  )
                nColWidth[nCols - 1] += nMaxW - nSum;

            pColMgr->SetColWidth( 0, static_cast< sal_uInt16 >(nColWidth[0] + nColDist[0]/2) );
            for( sal_uInt16 i = 1; i < nCols-1; ++i )
            {
                long nActDist = (nColDist[i] + nColDist[i - 1]) / 2;
                pColMgr->SetColWidth( i, static_cast< sal_uInt16 >(nColWidth[i] + nActDist ));
            }
            pColMgr->SetColWidth( nCols-1, static_cast< sal_uInt16 >(nColWidth[nCols-1] + nColDist[nCols -2]/2) );

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
                m_pLineWidthEdit->GetUnit(), MAP_TWIP ));
        if( !bEnable )
            pColMgr->SetNoLine();
        else
        {
            pColMgr->SetLineWidthAndColor(
                    ::editeng::SvxBorderStyle( m_pLineTypeDLB->GetSelectEntryStyle( ) ),
                    nLineWidth,
                    m_pLineColorDLB->GetSelectEntryColor() );
            pColMgr->SetAdjust( SwColLineAdj(
                                    m_pLinePosDLB->GetSelectEntryPos() + 1) );
            pColMgr->SetLineHeightPercent((short)m_pLineHeightEdit->GetValue());
            bEnable = pColMgr->GetLineHeightPercent() != 100;
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
        pColMgr->NoCols();
        nCols = 0;
    }

    //set maximum values
    m_pCLNrEdt->SetMax(std::max(1L,
        std::min(long(nMaxCols), long( pColMgr->GetActualSize() / (nGutterWidth + MINLAY)) )));
    m_pCLNrEdt->SetLast(m_pCLNrEdt->GetMax());
    m_pCLNrEdt->Reformat();

    //prompt example window
    if(!bLockUpdate)
    {
        if(bFrame)
        {
            m_pFrameExampleWN->SetColumns( pColMgr->GetColumns() );
            m_pFrameExampleWN->Invalidate();
        }
        else
            m_pPgeExampleWN->Invalidate();
    }
}

void SwColumnPage::Init()
{
    m_pCLNrEdt->SetValue(nCols);

    bool bAutoWidth = pColMgr->IsAutoWidth() || bHtmlMode;
    m_pAutoWidthBox->Check( bAutoWidth );

    sal_Int32 nColumnWidthSum = 0;
    // set the widths
    for(sal_uInt16 i = 0; i < nCols; ++i)
    {
        nColWidth[i] = pColMgr->GetColWidth(i);
        nColumnWidthSum += nColWidth[i];
        if(i < nCols - 1)
            nColDist[i] = pColMgr->GetGutterWidth(i);
    }

    if( 1 < nCols )
    {
        // #97495# make sure that the automatic column width's are always equal
        if(bAutoWidth)
        {
            nColumnWidthSum /= nCols;
            for(sal_uInt16 i = 0; i < nCols; ++i)
                nColWidth[i] = nColumnWidthSum;
        }
        SwColLineAdj eAdj = pColMgr->GetAdjust();
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
            m_pLineWidthEdit->SetValue( pColMgr->GetLineWidth() * 100, FUNIT_TWIP );
            m_pLineColorDLB->SelectEntry( pColMgr->GetLineColor() );
            m_pLineTypeDLB->SelectEntry( pColMgr->GetLineStyle() );
            m_pLineTypeDLB->SetWidth( pColMgr->GetLineWidth( ) );
            m_pLineHeightEdit->SetValue( pColMgr->GetLineHeightPercent() );

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
        std::min(long(nMaxCols), long( pColMgr->GetActualSize() / nMinWidth) )));
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
    if ( nCols > nVisCols )
    {
        bEnableBtns = true && !bHtmlMode;
        bEnable12 = bEnable3 = bEdit;
    }
    else if( bEdit )
    {
        // here are purposely hardly any breaks
        switch(nCols)
        {
            case 3: bEnable3 = true;
                SAL_FALLTHROUGH;
            case 2: bEnable12= true; break;
            default: /* do nothing */;
        }
    }
    aEd1.Enable( bEnable12 );
    bool bEnable = nCols > 1;
    aDistEd1.Enable(bEnable);
    m_pAutoWidthBox->Enable( bEnable && !bHtmlMode );
    aEd2.Enable( bEnable12 );
    aDistEd2.Enable(bEnable3);
    aEd3.Enable( bEnable3  );
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
        bEnable = pColMgr->GetLineHeightPercent() != 100;

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

    const OUString sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH ) ;
    aEd1.SetAccessibleName(sColumnWidth.replaceFirst("%1", sLbl1));
    aEd2.SetAccessibleName(sColumnWidth.replaceFirst("%1", sLbl2));
    aEd3.SetAccessibleName(sColumnWidth.replaceFirst("%1", sLbl3));

    const OUString sDist = SW_RESSTR( STR_ACCESS_PAGESETUP_SPACING ) ;
    aDistEd1.SetAccessibleName(
        sDist.replaceFirst("%1", sLbl1).replaceFirst("%2", sLbl2));

    aDistEd2.SetAccessibleName(
        sDist.replaceFirst("%1", sLbl2).replaceFirst("%2", sLbl3));
}

/*
 * Handler that is called at alteration of the column number. An alteration of
 * the column number overwrites potential user's width settings; all columns
 * are equally wide.
 */
IMPL_LINK_TYPED( SwColumnPage, ColModify, Edit&, rEdit, void )
{
    ColModify(static_cast<NumericField*>(&rEdit));
}

void SwColumnPage::ColModify(NumericField* pNF)
{
    nCols = (sal_uInt16)m_pCLNrEdt->GetValue();
    //#107890# the handler is also called from LoseFocus()
    //then no change has been made and thus no action should be taken
    // #i17816# changing the displayed types within the ValueSet
    //from two columns to two columns with different settings doesn't invalidate the
    // example windows in ::ColModify()
    if (!pNF || pColMgr->GetCount() != nCols)
    {
        if(pNF)
            m_pDefaultVS->SetNoSelection();
        long nDist = static_cast< long >(aDistEd1.DenormalizePercent(aDistEd1.GetValue(FUNIT_TWIP)));
        pColMgr->SetCount(nCols, (sal_uInt16)nDist);
        for(sal_uInt16 i = 0; i < nCols; i++)
            nColDist[i] = nDist;
        nFirstVis = 0;
        SetLabels( nFirstVis );
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
IMPL_LINK_TYPED( SwColumnPage, GapModify, Edit&, rEdit, void )
{
    if (nCols < 2)
        return;
    MetricField* pMetricField = static_cast<MetricField*>(&rEdit);
    PercentField *pField = m_aPercentFieldsMap[pMetricField];
    assert(pField);
    long nActValue = static_cast< long >(pField->DenormalizePercent(pField->GetValue(FUNIT_TWIP)));
    if(m_pAutoWidthBox->IsChecked())
    {
        const long nMaxGap = static_cast< long >
            ((pColMgr->GetActualSize() - nCols * MINLAY)/(nCols - 1));
        if(nActValue > nMaxGap)
        {
            nActValue = nMaxGap;
            aDistEd1.SetPrcntValue(aDistEd1.NormalizePercent(nMaxGap), FUNIT_TWIP);
        }
        pColMgr->SetGutterWidth((sal_uInt16)nActValue);
        for(sal_uInt16 i = 0; i < nCols; i++)
            nColDist[i] = nActValue;

        ResetColWidth();
        UpdateCols();
    }
    else
    {
        const sal_uInt16 nVis = nFirstVis + ((pField == &aDistEd2) ? 1 : 0);
        long nDiff = nActValue - nColDist[nVis];
        if(nDiff)
        {
            long nLeft = nColWidth[nVis];
            long nRight = nColWidth[nVis + 1];
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
            nColWidth[nVis] = nLeft;
            nColWidth[nVis + 1] = nRight;
            nColDist[nVis] += nDiff;

            pColMgr->SetColWidth( nVis, sal_uInt16(nLeft) );
            pColMgr->SetColWidth( nVis + 1, sal_uInt16(nRight) );
            pColMgr->SetGutterWidth( sal_uInt16(nColDist[nVis]), nVis );
        }

    }
    Update(pMetricField);
}

IMPL_LINK_TYPED( SwColumnPage, EdModify, Edit&, rEdit, void )
{
    MetricField * pMetricField = static_cast<MetricField*>(&rEdit);
    PercentField *pField = m_aPercentFieldsMap[pMetricField];
    assert(pField);
    pModifiedField = pField;
    Timeout();
}

// Handler behind the Checkbox for automatic width. When the box is checked
// no explicit values for the column width can be entered.
IMPL_LINK_TYPED( SwColumnPage, AutoWidthHdl, Button*, pButton, void )
{
    CheckBox* pBox = static_cast<CheckBox*>(pButton);
    long nDist = static_cast< long >(aDistEd1.DenormalizePercent(aDistEd1.GetValue(FUNIT_TWIP)));
    pColMgr->SetCount(nCols, (sal_uInt16)nDist);
    for(sal_uInt16 i = 0; i < nCols; i++)
        nColDist[i] = nDist;
    if(pBox->IsChecked())
    {
        pColMgr->SetGutterWidth(sal_uInt16(nDist));
        ResetColWidth();
    }
    pColMgr->SetAutoWidth(pBox->IsChecked(), sal_uInt16(nDist));
    UpdateCols();
    Update(nullptr);
}

// scroll up the contents of the edits
IMPL_LINK_NOARG_TYPED(SwColumnPage, Up, Button*, void)
{
    if( nFirstVis )
    {
        --nFirstVis;
        SetLabels( nFirstVis );
        Update(nullptr);
    }
}

// scroll down the contents of the edits.
IMPL_LINK_NOARG_TYPED(SwColumnPage, Down, Button*, void)
{
    if( nFirstVis + nVisCols < nCols )
    {
        ++nFirstVis;
        SetLabels( nFirstVis );
        Update(nullptr);
    }
}

// relict from ancient times - now directly without time handler; triggered by
// an alteration of the column width or the column gap.
void SwColumnPage::Timeout()
{
    PercentField *pField = pModifiedField;
    if (pModifiedField)
    {
            // find the changed column
        sal_uInt16 nChanged = nFirstVis;
        if(pModifiedField == &aEd2)
            ++nChanged;
        else if(pModifiedField == &aEd3)
            nChanged += 2;

        long nNewWidth = static_cast< long >
            (pModifiedField->DenormalizePercent(pModifiedField->GetValue(FUNIT_TWIP)));
        long nDiff = nNewWidth - nColWidth[nChanged];

        // when it's the last column
        if(nChanged == nCols - 1)
        {
            nColWidth[0] -= nDiff;
            if(nColWidth[0] < (long)nMinWidth)
            {
                nNewWidth -= nMinWidth - nColWidth[0];
                nColWidth[0] = nMinWidth;
            }

        }
        else if(nDiff)
        {
            nColWidth[nChanged + 1] -= nDiff;
            if(nColWidth[nChanged + 1] < (long) nMinWidth)
            {
                nNewWidth -= nMinWidth - nColWidth[nChanged + 1];
                nColWidth[nChanged + 1] = nMinWidth;
            }
        }
        nColWidth[nChanged] = nNewWidth;
        pModifiedField = nullptr;
    }

    Update(pField ? pField->get() : nullptr);
}

// Update the view
void SwColumnPage::Update(MetricField *pInteractiveField)
{
    m_pBalanceColsCB->Enable(nCols > 1);
    if(nCols >= 2)
    {
        sal_Int64 nCurrentValue, nNewValue;

        nCurrentValue = aEd1.NormalizePercent(aEd1.DenormalizePercent(aEd1.GetValue(FUNIT_TWIP)));
        nNewValue = aEd1.NormalizePercent(nColWidth[nFirstVis]);

        //fdo#87612 if we're interacting with this widget and the value will be the same
        //then leave it alone (i.e. don't change equivalent values of e.g. .8 -> 0.8)
        if (nNewValue != nCurrentValue || pInteractiveField != aEd1.get())
            aEd1.SetPrcntValue(nNewValue, FUNIT_TWIP);

        nCurrentValue = aDistEd1.NormalizePercent(aDistEd1.DenormalizePercent(aDistEd1.GetValue(FUNIT_TWIP)));
        nNewValue = aDistEd1.NormalizePercent(nColDist[nFirstVis]);
        if (nNewValue != nCurrentValue || pInteractiveField != aDistEd1.get())
            aDistEd1.SetPrcntValue(nNewValue, FUNIT_TWIP);

        nCurrentValue = aEd2.NormalizePercent(aEd2.DenormalizePercent(aEd2.GetValue(FUNIT_TWIP)));
        nNewValue = aEd2.NormalizePercent(nColWidth[nFirstVis+1]);
        if (nNewValue != nCurrentValue || pInteractiveField != aEd2.get())
            aEd2.SetPrcntValue(nNewValue, FUNIT_TWIP);

        if(nCols >= 3)
        {
            nCurrentValue = aDistEd2.NormalizePercent(aDistEd2.DenormalizePercent(aDistEd2.GetValue(FUNIT_TWIP)));
            nNewValue = aDistEd2.NormalizePercent(nColDist[nFirstVis+1]);
            if (nNewValue != nCurrentValue || pInteractiveField != aDistEd2.get())
                aDistEd2.SetPrcntValue(nNewValue, FUNIT_TWIP);

            nCurrentValue = aEd3.NormalizePercent(aEd3.DenormalizePercent(aEd3.GetValue(FUNIT_TWIP)));
            nNewValue = aEd3.NormalizePercent(nColWidth[nFirstVis+2]);
            if (nNewValue != nCurrentValue || pInteractiveField != aEd3.get())
                aEd3.SetPrcntValue(nNewValue, FUNIT_TWIP);
        }
        else
        {
            aEd3.SetText(OUString());
            aDistEd2.SetText(OUString());
        }
    }
    else
    {
        aEd1.SetText(OUString());
        aEd2.SetText(OUString());
        aEd3.SetText(OUString());
        aDistEd1.SetText(OUString());
        aDistEd2.SetText(OUString());
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
        bVertical = rDirItem.GetValue() == FRMDIR_VERT_TOP_RIGHT||
                    rDirItem.GetValue() == FRMDIR_VERT_TOP_LEFT;
    }

    if (!bFrame)
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
                                - rLRSpace.GetLeft() - rLRSpace.GetRight() - rBox.GetDistance();
            }
            else
            {
                const SvxULSpaceItem& rULSpace = static_cast<const SvxULSpaceItem&>(rSet.Get(
                                                                    RES_UL_SPACE ));
                const SvxBoxItem& rBox = static_cast<const SvxBoxItem&>( rSet.Get(RES_BOX));
                nActWidth = rSize.GetSize().Height()
                                - rULSpace.GetUpper() - rULSpace.GetLower() - rBox.GetDistance();

            }

            if( pColMgr->GetActualSize() != nActWidth)
            {
                pColMgr->SetActualWidth(nActWidth);
                ColModify( nullptr );
                UpdateColMgr( *m_pLineWidthEdit );
            }
        }
        m_pFrameExampleWN->Hide();
        m_pPgeExampleWN->UpdateExample( rSet, pColMgr );
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
        if (bFormat)
            nTotalWish = FRAME_FORMAT_WIDTH;
        else
        {
            long nDistance = rBox.GetDistance();
            nTotalWish = (!bVertical ? rSize.GetWidth() : rSize.GetHeight()) - 2 * nDistance;
        }

        // set maximum values of column width
        SetPageWidth(nTotalWish);

        if(pColMgr->GetActualSize() != nTotalWish)
        {
            pColMgr->SetActualWidth(nTotalWish);
            Init();
        }
        bool bPercent;
        // only relative data in frame format
        if ( bFormat || (rSize.GetWidthPercent() && rSize.GetWidthPercent() != SwFormatFrameSize::SYNCED) )
        {
            // set value for 100%
            aEd1.SetRefValue(nTotalWish);
            aEd2.SetRefValue(nTotalWish);
            aEd3.SetRefValue(nTotalWish);
            aDistEd1.SetRefValue(nTotalWish);
            aDistEd2.SetRefValue(nTotalWish);

            // switch to %-view
            bPercent = true;
        }
        else
            bPercent = false;

        aEd1.ShowPercent(bPercent);
        aEd2.ShowPercent(bPercent);
        aEd3.ShowPercent(bPercent);
        aDistEd1.ShowPercent(bPercent);
        aDistEd2.ShowPercent(bPercent);
        aDistEd1.SetMetricFieldMin(0);
        aDistEd2.SetMetricFieldMin(0);
    }
    Update(nullptr);
}

SfxTabPage::sfxpg SwColumnPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(_pSet);

    return LEAVE_PAGE;
}

IMPL_LINK_TYPED( SwColumnPage, SetDefaultsHdl, ValueSet *, pVS, void )
{
    const sal_uInt16 nItem = pVS->GetSelectItemId();
    if( nItem < 4 )
    {
        m_pCLNrEdt->SetValue( nItem );
        m_pAutoWidthBox->Check();
        aDistEd1.SetPrcntValue(0);
        ColModify(nullptr);
    }
    else
    {
        bLockUpdate = true;
        m_pCLNrEdt->SetValue( 2 );
        m_pAutoWidthBox->Check(false);
        aDistEd1.SetPrcntValue(0);
        ColModify(nullptr);
        // now set the width ratio to 2 : 1 or 1 : 2 respectively
        const long nSmall = static_cast< long >(pColMgr->GetActualSize() / 3);
        if(nItem == 4)
        {
            aEd2.SetPrcntValue(aEd2.NormalizePercent(nSmall), FUNIT_TWIP);
            pModifiedField = &aEd2;
        }
        else
        {
            aEd1.SetPrcntValue(aEd1.NormalizePercent(nSmall), FUNIT_TWIP);
            pModifiedField = &aEd1;
        }
        bLockUpdate = false;
        Timeout();

    }
}

void SwColumnPage::SetFrameMode(bool bMod)
{
    bFrame = bMod;
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

    Rectangle aRect = rUDEvt.GetRect();
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
