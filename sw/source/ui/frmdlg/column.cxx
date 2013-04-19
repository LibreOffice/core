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

#include "column.hxx"

#include "hintids.hxx"
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/htmlmode.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/sizeitem.hxx>
#include "editeng/frmdiritem.hxx"
#include <svl/ctloptions.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
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

/*--------------------------------------------------------------------
    Description:  static data
 --------------------------------------------------------------------*/

static const sal_uInt16 nVisCols = 3;

inline bool IsMarkInSameSection( SwWrtShell& rWrtSh, const SwSection* pSect )
{
    rWrtSh.SwapPam();
    bool bRet = pSect == rWrtSh.GetCurrSection();
    rWrtSh.SwapPam();
    return bRet;
}

SwColumnDlg::SwColumnDlg(Window* pParent, SwWrtShell& rSh)
    : SfxModalDialog(pParent, "ColumnDialog", "modules/swriter/ui/columndialog.ui")
    , rWrtShell(rSh)
    , pPageSet(0)
    , pSectionSet(0)
    , pSelectionSet(0)
    , pFrameSet(0)
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

    SfxItemSet* pColPgSet = 0;
    static sal_uInt16 const aSectIds[] = { RES_COL, RES_COL,
                                                RES_FRM_SIZE, RES_FRM_SIZE,
                                                RES_COLUMNBALANCE, RES_FRAMEDIR,
                                                0 };

    const SwSection* pCurrSection = rWrtShell.GetCurrSection();
    sal_uInt16 nFullSectCnt = rWrtShell.GetFullSelectedSectionCount();
    if( pCurrSection && ( !rWrtShell.HasSelection() || 0 != nFullSectCnt ))
    {
        nSelectionWidth = rSh.GetSectionWidth(*pCurrSection->GetFmt());
        if ( !nSelectionWidth )
            nSelectionWidth = USHRT_MAX;
        pSectionSet = new SfxItemSet( rWrtShell.GetAttrPool(), aSectIds );
        pSectionSet->Put( pCurrSection->GetFmt()->GetAttrSet() );
        pColPgSet = pSectionSet;
    }

    if( rWrtShell.HasSelection() && rWrtShell.IsInsRegionAvailable() &&
        ( !pCurrSection || ( 1 != nFullSectCnt &&
            IsMarkInSameSection( rWrtShell, pCurrSection ) )))
    {
        pSelectionSet = new SfxItemSet( rWrtShell.GetAttrPool(), aSectIds );
        pColPgSet = pSelectionSet;
    }

    if( rWrtShell.GetFlyFrmFmt() )
    {
        const SwFrmFmt* pFmt = rSh.GetFlyFrmFmt() ;
        pFrameSet = new SfxItemSet(rWrtShell.GetAttrPool(), aSectIds );
        pFrameSet->Put(pFmt->GetFrmSize());
        pFrameSet->Put(pFmt->GetCol());
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

        const SwFrmFmt &rFmt = pPageDesc->GetMaster();
        nPageWidth = rFmt.GetFrmSize().GetSize().Width();

        const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)rFmt.GetLRSpace();
        const SvxBoxItem& rBox = (const SvxBoxItem&) rFmt.GetBox();
        nPageWidth -= rLRSpace.GetLeft() + rLRSpace.GetRight() + rBox.GetDistance();

        pPageSet->Put(rFmt.GetCol());
        pPageSet->Put(rFmt.GetLRSpace());
        pColPgSet = pPageSet;
    }

    assert(pColPgSet);

    // create TabPage
    pTabPage = (SwColumnPage*) SwColumnPage::Create(get_content_area(), *pColPgSet);
    pTabPage->get<Window>("applytoft")->Show();
    pTabPage->get(m_pApplyToLB, "applytolb");
    m_pApplyToLB->Show();

    if (pCurrSection && (!rWrtShell.HasSelection() || 0 != nFullSectCnt))
    {
        m_pApplyToLB->RemoveEntry( m_pApplyToLB->GetEntryPos(
                                        (void*)(sal_IntPtr)( 1 >= nFullSectCnt
                                                    ? LISTBOX_SECTIONS
                                                    : LISTBOX_SECTION )));
    }
    else
    {
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( (void*)(sal_IntPtr)LISTBOX_SECTION ));
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( (void*)(sal_IntPtr)LISTBOX_SECTIONS ));
    }

    if (!( rWrtShell.HasSelection() && rWrtShell.IsInsRegionAvailable() &&
        ( !pCurrSection || ( 1 != nFullSectCnt &&
            IsMarkInSameSection( rWrtShell, pCurrSection ) ))))
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( (void*)(sal_IntPtr)LISTBOX_SELECTION ));

    if (!rWrtShell.GetFlyFrmFmt())
        m_pApplyToLB->RemoveEntry(m_pApplyToLB->GetEntryPos( (void*) LISTBOX_FRAME ));

    sal_uInt16 nPagePos = m_pApplyToLB->GetEntryPos( (void*) LISTBOX_PAGE );
    if (pPageSet && pPageDesc)
    {
        String sPageStr = m_pApplyToLB->GetEntry(nPagePos);
        m_pApplyToLB->RemoveEntry(nPagePos);
        sPageStr += pPageDesc->GetName();
        m_pApplyToLB->InsertEntry( sPageStr, nPagePos );
        m_pApplyToLB->SetEntryData( nPagePos, (void*) LISTBOX_PAGE);
    }
    else
        m_pApplyToLB->RemoveEntry( nPagePos );

    m_pApplyToLB->SelectEntryPos(0);
    ObjectHdl(0);

    m_pApplyToLB->SetSelectHdl(LINK(this, SwColumnDlg, ObjectHdl));
    OKButton *pOK = get<OKButton>("ok");
    pOK->SetClickHdl(LINK(this, SwColumnDlg, OkHdl));
    //#i80458# if no columns can be set then disable OK
    if( !m_pApplyToLB->GetEntryCount() )
        pOK->Enable( sal_False );
    //#i97810# set focus to the TabPage
    pTabPage->ActivateColumnControl();
    pTabPage->Show();
}

SwColumnDlg::~SwColumnDlg()
{
    delete pTabPage;
    delete pPageSet;
    delete pSectionSet;
    delete pSelectionSet;
}

IMPL_LINK(SwColumnDlg, ObjectHdl, ListBox*, pBox)
{
    SfxItemSet* pSet = 0;
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
        pTabPage->FillItemSet(*pSet);
    }
    nOldSelection = (long)m_pApplyToLB->GetEntryData(m_pApplyToLB->GetSelectEntryPos());
    long nWidth = nSelectionWidth;
    switch(nOldSelection)
    {
        case LISTBOX_SELECTION  :
            pSet = pSelectionSet;
            if( pSelectionSet )
                pSet->Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_SECTION    :
        case LISTBOX_SECTIONS   :
            pSet = pSectionSet;
            pSet->Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_PAGE       :
            nWidth = nPageWidth;
            pSet = pPageSet;
            pSet->Put(SwFmtFrmSize(ATT_VAR_SIZE, nWidth, nWidth));
        break;
        case LISTBOX_FRAME:
            pSet = pFrameSet;
        break;
    }

    sal_Bool bIsSection = pSet == pSectionSet || pSet == pSelectionSet;
    pTabPage->ShowBalance(bIsSection);
    pTabPage->SetInSection(bIsSection);
    pTabPage->SetFrmMode(sal_True);
    pTabPage->SetPageWidth(nWidth);
    if( pSet )
        pTabPage->Reset(*pSet);
    return 0;
}

IMPL_LINK_NOARG(SwColumnDlg, OkHdl)
{
    // evaluate current selection
    SfxItemSet* pSet = 0;
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
    pTabPage->FillItemSet(*pSet);

    if(pSelectionSet && SFX_ITEM_SET == pSelectionSet->GetItemState(RES_COL))
    {
        //insert region with columns
        const SwFmtCol& rColItem = (const SwFmtCol&)pSelectionSet->Get(RES_COL);
        //only if there actually are columns!
        if(rColItem.GetNumCols() > 1)
            rWrtShell.GetView().GetViewFrame()->GetDispatcher()->Execute(
                FN_INSERT_REGION, SFX_CALLMODE_ASYNCHRON, *pSelectionSet );
    }

    if(pSectionSet && pSectionSet->Count() && bSectionChanged )
    {
        const SwSection* pCurrSection = rWrtShell.GetCurrSection();
        const SwSectionFmt* pFmt = pCurrSection->GetFmt();
        sal_uInt16 nNewPos = rWrtShell.GetSectionFmtPos( *pFmt );
        SwSectionData aData(*pCurrSection);
        rWrtShell.UpdateSection( nNewPos, aData, pSectionSet );
    }

    if(pSectionSet && pSectionSet->Count() && bSelSectionChanged )
    {
        rWrtShell.SetSectionAttr( *pSectionSet );
    }

    if(pPageSet && SFX_ITEM_SET == pPageSet->GetItemState(RES_COL) && bPageChanged)
    {
        // deterine current PageDescriptor and fill the Set with it
        const sal_uInt16 nCurIdx = rWrtShell.GetCurPageDesc();
        SwPageDesc aPageDesc(rWrtShell.GetPageDesc(nCurIdx));
        SwFrmFmt &rFmt = aPageDesc.GetMaster();
        rFmt.SetFmtAttr(pPageSet->Get(RES_COL));
        rWrtShell.ChgPageDesc(nCurIdx, aPageDesc);
    }
    if(pFrameSet && SFX_ITEM_SET == pFrameSet->GetItemState(RES_COL) && bFrameChanged)
    {
        SfxItemSet aTmp(*pFrameSet->GetPool(), RES_COL, RES_COL);
        aTmp.Put(*pFrameSet);
        rWrtShell.StartAction();
        rWrtShell.Push();
        rWrtShell.SetFlyFrmAttr( aTmp );
        // undo the frame selction again
        if(rWrtShell.IsFrmSelected())
        {
            rWrtShell.UnSelectFrm();
            rWrtShell.LeaveSelFrmMode();
        }
        rWrtShell.Pop();
        rWrtShell.EndAction();
    }
    EndDialog(RET_OK);
    return 0;
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

static sal_uInt16 aPageRg[] = {
    RES_COL, RES_COL,
    0
};

DBG_NAME(columnhdl)

void SwColumnPage::ResetColWidth()
{
    if( nCols )
    {
        sal_uInt16 nWidth = GetMaxWidth( pColMgr, nCols );
        nWidth = nWidth / nCols;

        for(sal_uInt16 i = 0; i < nCols; ++i)
            nColWidth[i] = (long) nWidth;
    }

}

/*--------------------------------------------------------------------
    Description:    Now as TabPage
 --------------------------------------------------------------------*/
SwColumnPage::SwColumnPage(Window *pParent, const SfxItemSet &rSet)
    : SfxTabPage(pParent, "ColumnPage", "modules/swriter/ui/columnpage.ui", rSet)
    , pColMgr(0)
    , nFirstVis(0)
    , nMinWidth(MINLAY)
    , pModifiedField(0)
    , bFormat(sal_False)
    , bFrm(sal_False)
    , bHtmlMode(sal_False)
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
    get(m_pFrmExampleWN, "frameexample");

    connectPercentFieldWrapper(aEd1, "width1mf");
    connectPercentFieldWrapper(aEd2, "width2mf");
    connectPercentFieldWrapper(aEd3, "width3mf");
    connectPercentFieldWrapper(aDistEd1, "spacing1mf");
    connectPercentFieldWrapper(aDistEd2, "spacing2mf");

    SetExchangeSupport();

    VclFrame *pSpacing = get<VclFrame>("spacing");
    m_pBtnNext->SetAccessibleRelationMemberOf(pSpacing->get_label_widget());

    m_pDefaultVS->SetColCount( 5 );

    for (int i = 0; i < 5; ++i)
        m_pDefaultVS->InsertItem( i + 1, i );

    m_pDefaultVS->SetSelectHdl(LINK(this, SwColumnPage, SetDefaultsHdl));

    // announce Controls for additional region at the MoreButton
    Link aCLNrLk = LINK(this, SwColumnPage, ColModify);
    m_pCLNrEdt->SetLoseFocusHdl(aCLNrLk);
    m_pCLNrEdt->SetUpHdl(aCLNrLk);
    m_pCLNrEdt->SetDownHdl(aCLNrLk);
    Link aLk = LINK(this, SwColumnPage, GapModify);
    aDistEd1.SetUpHdl(aLk);
    aDistEd1.SetDownHdl(aLk);
    aDistEd1.SetLoseFocusHdl(aLk);
    aDistEd2.SetUpHdl(aLk);
    aDistEd2.SetDownHdl(aLk);
    aDistEd2.SetLoseFocusHdl(aLk);

    aLk = LINK(this, SwColumnPage, EdModify);

    aEd1.SetUpHdl(aLk);
    aEd1.SetDownHdl(aLk);
    aEd1.SetLoseFocusHdl(aLk);

    aEd2.SetUpHdl(aLk);
    aEd2.SetDownHdl(aLk);
    aEd2.SetLoseFocusHdl(aLk);

    aEd3.SetUpHdl(aLk);
    aEd3.SetDownHdl(aLk);
    aEd3.SetLoseFocusHdl(aLk);

    m_pBtnBack->SetClickHdl(LINK(this, SwColumnPage, Up));
    m_pBtnNext->SetClickHdl(LINK(this, SwColumnPage, Down));
    m_pAutoWidthBox->SetClickHdl(LINK(this, SwColumnPage, AutoWidthHdl));

    aLk = LINK( this, SwColumnPage, UpdateColMgr );
    m_pLineTypeDLB->SetSelectHdl( aLk );
    m_pLineWidthEdit->SetModifyHdl( aLk );
    m_pLineColorDLB->SetSelectHdl( aLk );
    m_pLineHeightEdit->SetModifyHdl( aLk );
    m_pLinePosDLB->SetSelectHdl( aLk );

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
    const SfxPoolItem*  pItem       = NULL;
    XColorListRef pColorList;
    if ( pDocSh )
    {
        pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem != NULL )
            pColorList = ( (SvxColorListItem*)pItem )->GetColorList();
    }

    if ( pColorList.is() )
    {
        m_pLineColorDLB->SetUpdateMode( sal_False );

        for (long i = 0; i < pColorList->Count(); ++i )
        {
            XColorEntry* pEntry = pColorList->GetColor(i);
            m_pLineColorDLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }
        m_pLineColorDLB->SetUpdateMode( sal_True );
    }
    m_pLineColorDLB->SelectEntryPos( 0 );
}

SwColumnPage::~SwColumnPage()
{
    delete pColMgr;
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

void SwColumnPage::connectPercentFieldWrapper(PercentFieldWrap &rWrap, const OString &rName)
{
    MetricField *pFld = get<MetricField>(rName);
    assert(pFld);
    rWrap.set(pFld);
    m_aPercentFieldWrappersMap[pFld] = &rWrap;
}

void SwColumnPage::Reset(const SfxItemSet &rSet)
{
    sal_uInt16 nHtmlMode = ::GetHtmlMode((const SwDocShell*)SfxObjectShell::Current());
    if(nHtmlMode & HTMLMODE_ON)
    {
        bHtmlMode = sal_True;
        m_pAutoWidthBox->Enable(sal_False);
    }
    FieldUnit aMetric = ::GetDfltMetric(bHtmlMode);
    aEd1.SetMetric(aMetric);
    aEd2.SetMetric(aMetric);
    aEd3.SetMetric(aMetric);
    aDistEd1.SetMetric(aMetric);
    aDistEd2.SetMetric(aMetric);

    delete pColMgr;
    pColMgr = new SwColMgr(rSet);
    nCols   = pColMgr->GetCount() ;
    m_pCLNrEdt->SetMax(std::max((sal_uInt16)m_pCLNrEdt->GetMax(), (sal_uInt16)nCols));
    m_pCLNrEdt->SetLast(std::max(nCols,(sal_uInt16)m_pCLNrEdt->GetMax()));

    if(bFrm)
    {
        if(bFormat)                     // there is no size here
            pColMgr->SetActualWidth(FRAME_FORMAT_WIDTH);
        else
        {
            const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);
            const SvxBoxItem& rBox = (const SvxBoxItem&) rSet.Get(RES_BOX);
            pColMgr->SetActualWidth((sal_uInt16)rSize.GetSize().Width() - rBox.GetDistance());
        }
    }
    if(m_pBalanceColsCB->IsVisible())
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_COLUMNBALANCE, sal_False, &pItem ))
            m_pBalanceColsCB->Check(!((const SwFmtNoBalancedColumns*)pItem)->GetValue());
        else
            m_pBalanceColsCB->Check( sal_True );
    }

    //text direction
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState( RES_FRAMEDIR ) )
    {
        const SvxFrameDirectionItem& rItem = (const SvxFrameDirectionItem&)rSet.Get(RES_FRAMEDIR);
        sal_uIntPtr nVal  = rItem.GetValue();
        sal_uInt16 nPos = m_pTextDirectionLB->GetEntryPos( (void*) nVal );
        m_pTextDirectionLB->SelectEntryPos( nPos );
        m_pTextDirectionLB->SaveValue();
    }

    Init();
    ActivatePage( rSet );
}

/*--------------------------------------------------------------------
    Description:    create TabPage
 --------------------------------------------------------------------*/
SfxTabPage* SwColumnPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwColumnPage(pParent, rSet);
}

/*--------------------------------------------------------------------
    Description:    stuff attributes into the Set when OK
 --------------------------------------------------------------------*/
sal_Bool SwColumnPage::FillItemSet(SfxItemSet &rSet)
{
    if(m_pCLNrEdt->HasChildPathFocus())
        m_pCLNrEdt->GetDownHdl().Call(m_pCLNrEdt);
    // set in ItemSet setzen
    // the current settings are already present
    //
    const SfxPoolItem* pOldItem;
    const SwFmtCol& rCol = pColMgr->GetColumns();
    if(0 == (pOldItem = GetOldItem( rSet, RES_COL )) ||
                rCol != *pOldItem )
        rSet.Put(rCol);

    if(m_pBalanceColsCB->IsVisible() )
    {
        rSet.Put(SwFmtNoBalancedColumns(!m_pBalanceColsCB->IsChecked() ));
    }
    sal_uInt16 nPos;
    if( m_pTextDirectionLB->IsVisible() &&
        ( nPos = m_pTextDirectionLB->GetSelectEntryPos() ) !=
                                            m_pTextDirectionLB->GetSavedValue() )
    {
        sal_uInt32 nDirection = (sal_uInt32)(sal_IntPtr)m_pTextDirectionLB->GetEntryData( nPos );
        rSet.Put( SvxFrameDirectionItem( (SvxFrameDirection)nDirection, RES_FRAMEDIR));
    }
    return sal_True;
}

/*--------------------------------------------------------------------
    Description:    update ColumnManager
 --------------------------------------------------------------------*/
IMPL_LINK( SwColumnPage, UpdateColMgr, void *, /*pField*/ )
{
    long nGutterWidth = pColMgr->GetGutterWidth();
    if(nCols > 1)
    {
            // Determine whether the most narrow column is too narrow
            // for the adjusted column gap
        long nMin = nColWidth[0];
        sal_uInt16 i;

        for( i = 1; i < nCols; ++i)
            nMin = std::min(nMin, nColWidth[i]);

        sal_Bool bAutoWidth = m_pAutoWidthBox->IsChecked();
        if(!bAutoWidth)
        {
            pColMgr->SetAutoWidth(sal_False);
                // when the user didn't allocate the whole width,
                // add the missing amount to the last column.
            long nSum = 0;
            for(i = 0; i < nCols; ++i)
                nSum += nColWidth[i];
            nGutterWidth = 0;
            for(i = 0; i < nCols - 1; ++i)
                nGutterWidth += nColDist[i];
            nSum += nGutterWidth;

            long nMaxW = pColMgr->GetActualSize();

            if( nSum < nMaxW  )
                nColWidth[nCols - 1] += nMaxW - nSum;

            pColMgr->SetColWidth( 0, static_cast< sal_uInt16 >(nColWidth[0] + (sal_uInt16)nColDist[0]/2) );
            for( i = 1; i < nCols-1; ++i )
            {
                long nActDist = (nColDist[i] + nColDist[i - 1]) / 2;
                pColMgr->SetColWidth( i, (sal_uInt16)nColWidth[i] + (sal_uInt16)nActDist );
            }
            pColMgr->SetColWidth( nCols-1, static_cast< sal_uInt16 >(nColWidth[nCols-1] + nColDist[nCols -2]/2) );

        }

            // nothing is turned off
        const sal_uInt16 nPos = m_pLineTypeDLB->GetSelectEntryPos();
        sal_Bool bEnable = 0 != nPos;
        m_pLineHeightEdit->Enable( bEnable );
        m_pLineHeightLbl->Enable( bEnable );
        long nLineWidth = static_cast<long>(MetricField::ConvertDoubleValue(
                m_pLineWidthEdit->GetValue( ),
                m_pLineWidthEdit->GetDecimalDigits( ),
                m_pLineWidthEdit->GetUnit(), MAP_TWIP ));
        if( !bEnable )
            pColMgr->SetNoLine();
        else if( LISTBOX_ENTRY_NOTFOUND != nPos )
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

        m_pLineTypeDLB->SetWidth( nLineWidth );
        m_pLineTypeDLB->SetColor( m_pLineColorDLB->GetSelectEntryColor( ) );
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
        if(bFrm)
        {
            m_pFrmExampleWN->SetColumns( pColMgr->GetColumns() );
            m_pFrmExampleWN->Invalidate();
        }
        else
            m_pPgeExampleWN->Invalidate();
    }

    return 0;
}

/*------------------------------------------------------------------------
 Description:   Initialisation
------------------------------------------------------------------------*/
void SwColumnPage::Init()
{
    m_pCLNrEdt->SetValue(nCols);

    sal_Bool bAutoWidth = pColMgr->IsAutoWidth() || bHtmlMode;
    m_pAutoWidthBox->Check( bAutoWidth );

    sal_Int32 nColumnWidthSum = 0;
    // set the widths
    sal_uInt16 i;
    for(i = 0; i < nCols; ++i)
    {
        nColWidth[i] = pColMgr->GetColWidth(i);
        nColumnWidthSum += nColWidth[i];
        if(i < nCols - 1)
            nColDist[i] = pColMgr->GetGutterWidth(i);
    }

    if( 1 < nCols )
    {
        // #97495# make sure that the automatic column widht's are always equal
        if(bAutoWidth)
        {
            nColumnWidthSum /= nCols;
            for(i = 0; i < nCols; ++i)
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
        m_pLinePosDLB->SelectEntryPos( static_cast< sal_uInt16 >(eAdj - 1) );
    }
    else
    {
        m_pLinePosDLB->SelectEntryPos( 0 );
        m_pLineTypeDLB->SelectEntryPos( 0 );
        m_pLineHeightEdit->SetValue( 100 );
    }

    UpdateCols();
    Update();

        // set maximum number of columns
        // values below 1 are not allowed
    m_pCLNrEdt->SetMax(std::max(1L,
        std::min(long(nMaxCols), long( pColMgr->GetActualSize() / nMinWidth) )));
}

/*------------------------------------------------------------------------
 Description:   The number of columns has changed -- here the controls for
                editing of the columns are en- or disabled according to the
                column number.
                In case there are more than nVisCols (=3) all Edit are being
                enabled and the buttons for scrolling too.
                Otherwise Edits are being enabled according to the column
                numbers; one column can not be edited.
------------------------------------------------------------------------*/
void SwColumnPage::UpdateCols()
{
    sal_Bool bEnableBtns= sal_False;
    sal_Bool bEnable12  = sal_False;
    sal_Bool bEnable3   = sal_False;
    const sal_Bool bEdit = !m_pAutoWidthBox->IsChecked();
    if ( nCols > nVisCols )
    {
        bEnableBtns = sal_True && !bHtmlMode;
        bEnable12 = bEnable3 = bEdit;
    }
    else if( bEdit )
    {
        // here are purposely hardly any breaks
        switch(nCols)
        {
            case 3: bEnable3 = sal_True;
            case 2: bEnable12= sal_True; break;
            default: /* do nothing */;
        }
    }
    aEd1.Enable( bEnable12 );
    aDistEd1.Enable(nCols > 1);
    aEd2.Enable( bEnable12 );
    aDistEd2.Enable(bEnable3);
    aEd3.Enable( bEnable3  );
    m_pLbl1->Enable(bEnable12 );
    m_pLbl2->Enable(bEnable12 );
    m_pLbl3->Enable(bEnable3  );
    m_pBtnBack->Enable( bEnableBtns );
    m_pBtnNext->Enable( bEnableBtns );

    const sal_Bool bEnable = nCols > 1;
    if( !bEnable )
    {
        m_pLinePosDLB->Enable( sal_False );
        m_pLinePosLbl->Enable( sal_False );
    }
    m_pLineHeightEdit->Enable( bEnable );
    m_pLineHeightLbl->Enable( bEnable );
    m_pLineTypeDLB->Enable( bEnable );
    m_pLineTypeLbl->Enable( bEnable );
    m_pLineWidthLbl->Enable( bEnable );
    m_pLineWidthEdit->Enable( bEnable );
    m_pLineColorDLB->Enable( bEnable );
    m_pLineColorLbl->Enable( bEnable );
    m_pAutoWidthBox->Enable( bEnable && !bHtmlMode );
}

void SwColumnPage::SetLabels( sal_uInt16 nVis )
{
    OUString sLbl( '~' );

    String sLbl2( OUString::number( nVis + 1 ));
    String tmp1(sLbl2);
    sLbl2.Insert(sLbl, sLbl2.Len() - 1);
    m_pLbl1->SetText(sLbl2);

    sLbl2 = OUString::number( nVis + 2 );
    String tmp2(sLbl2);
    sLbl2.Insert(sLbl, sLbl2.Len() - 1);
    m_pLbl2->SetText(sLbl2);

    sLbl2 = OUString::number( nVis + 3 );
    String tmp3(sLbl2);
    sLbl2.Insert(sLbl, sLbl2.Len() - 1);
    m_pLbl3->SetText(sLbl2);
    String sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH ) ;
    sColumnWidth.SearchAndReplaceAscii("%1", tmp1);
    aEd1.SetAccessibleName(sColumnWidth);

    sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH ) ;
    sColumnWidth.SearchAndReplaceAscii("%1", tmp2);
    aEd2.SetAccessibleName(sColumnWidth);

    sColumnWidth = SW_RESSTR( STR_ACCESS_COLUMN_WIDTH ) ;
    sColumnWidth.SearchAndReplaceAscii("%1", tmp3);
    aEd3.SetAccessibleName(sColumnWidth);

    String sDist = SW_RESSTR( STR_ACCESS_PAGESETUP_SPACING ) ;
    String sDist1 = sDist;
    sDist1.SearchAndReplaceAscii("%1", tmp1);
    sDist1.SearchAndReplaceAscii("%2", tmp2);
    aDistEd1.SetAccessibleName(sDist1);

    String sDist2 = sDist;
    sDist2.SearchAndReplaceAscii("%1", tmp2);
    sDist2.SearchAndReplaceAscii("%2", tmp3);
    aDistEd2.SetAccessibleName(sDist2);
}

/*------------------------------------------------------------------------
 Description:   Handler that is called at alteration of the column number.
                An alteration of the column number overwrites potential
                user's width settings; all columns are equally wide.
------------------------------------------------------------------------*/
IMPL_LINK( SwColumnPage, ColModify, NumericField *, pNF )
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
        Update();
    }

    return 0;
}

/*------------------------------------------------------------------------
 Description:   Modify handler for an alteration of the column width or
                the column gap.
                These changes take effect time-displaced. With an
                alteration of the column width the automatic calculation
                of the column width is overruled; only an alteration
                of the column number leads back to that default.
------------------------------------------------------------------------*/
IMPL_LINK( SwColumnPage, GapModify, MetricField*, pMetricFld )
{
    PercentFieldWrap *pFld = m_aPercentFieldWrappersMap[pMetricFld];
    assert(pFld);
    long nActValue = static_cast< long >(pFld->DenormalizePercent(pFld->GetValue(FUNIT_TWIP)));
    if(nCols < 2)
        return 0;
    if(m_pAutoWidthBox->IsChecked())
    {
        sal_uInt16 nMaxGap = pColMgr->GetActualSize() - nCols * MINLAY;
        OSL_ENSURE(nCols, "Abstand kann nicht ohne Spalten eingestellt werden");
        nMaxGap /= nCols - 1;
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
        sal_uInt16 nOffset = 0;
        if(pFld == &aDistEd2)
        {
            nOffset = 1;
        }
        long nDiff = nActValue - nColDist[nFirstVis + nOffset];
        if(nDiff)
        {
            long nLeft = nColWidth[nFirstVis + nOffset];
            long nRight = nColWidth[nFirstVis + nOffset + 1];
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
            nColWidth[nFirstVis + nOffset] = nLeft;
            nColWidth[nFirstVis + nOffset + 1] = nRight;
            nColDist[nFirstVis + nOffset] += nDiff;

            pColMgr->SetColWidth( nFirstVis + nOffset, sal_uInt16(nLeft) );
            pColMgr->SetColWidth( nFirstVis + nOffset + 1, sal_uInt16(nRight) );
            pColMgr->SetGutterWidth( sal_uInt16(nColDist[nFirstVis + nOffset]), nFirstVis + nOffset );
        }

    }
    Update();
    return 0;
}

IMPL_LINK( SwColumnPage, EdModify, MetricField *, pMetricFld )
{
    PercentFieldWrap *pField = m_aPercentFieldWrappersMap[pMetricFld];
    assert(pField);
    pModifiedField = pField;
    Timeout();
    return 0;
}

/*------------------------------------------------------------------------
 Description:   Handler behind the Checkbox for automatic width.
                When the box is checked no expicit values for the column
                width can be entered.
------------------------------------------------------------------------*/
IMPL_LINK( SwColumnPage, AutoWidthHdl, CheckBox *, pBox )
{
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
    Update();
    return 0;
}

/*------------------------------------------------------------------------
 Description:   scroll up the contents of the edits
------------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwColumnPage, Up)
{
    if( nFirstVis )
    {
        --nFirstVis;
        SetLabels( nFirstVis );
        Update();
    }
    return 0;
}

/*------------------------------------------------------------------------
 Description:   scroll down the contents of the edits.
------------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwColumnPage, Down)
{
    if( nFirstVis + nVisCols < nCols )
    {
        ++nFirstVis;
        SetLabels( nFirstVis );
        Update();
    }
    return 0;
}

/*------------------------------------------------------------------------
 Description:   relict from ancient times - now directly without time
                handler; triggered by an alteration of the column width
                or the column gap.
------------------------------------------------------------------------*/
void SwColumnPage::Timeout()
{
    DBG_PROFSTART(columnhdl) ;
    if(pModifiedField)
    {
            // find the changed column
        sal_uInt16 nChanged = nFirstVis;
        if(pModifiedField == &aEd2)
            ++nChanged;
        else if(pModifiedField == &aEd3)
            nChanged += 2;

        long nNewWidth = (sal_uInt16)
            pModifiedField->DenormalizePercent(pModifiedField->GetValue(FUNIT_TWIP));
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
        pModifiedField = 0;
    }
    Update();
    DBG_PROFSTOP(columnhdl) ;
}

/*------------------------------------------------------------------------
 Description:   Update the view
------------------------------------------------------------------------*/
void SwColumnPage::Update()
{
    m_pBalanceColsCB->Enable(nCols > 1);
    if(nCols >= 2)
    {
        aEd1.SetPrcntValue(aEd1.NormalizePercent(nColWidth[nFirstVis]), FUNIT_TWIP);
        aDistEd1.SetPrcntValue(aDistEd1.NormalizePercent(nColDist[nFirstVis]), FUNIT_TWIP);
        aEd2.SetPrcntValue(aEd2.NormalizePercent(nColWidth[nFirstVis + 1]), FUNIT_TWIP);
        if(nCols >= 3)
        {
            aDistEd2.SetPrcntValue(aDistEd2.NormalizePercent(nColDist[nFirstVis + 1]), FUNIT_TWIP);
            aEd3.SetPrcntValue(aEd3.NormalizePercent(nColWidth[nFirstVis + 2]), FUNIT_TWIP);
        }
        else
        {
            aEd3.SetText(aEmptyStr);
            aDistEd2.SetText(aEmptyStr);
        }
    }
    else
    {
        aEd1.SetText(aEmptyStr);
        aEd2.SetText(aEmptyStr);
        aEd3.SetText(aEmptyStr);
        aDistEd1.SetText(aEmptyStr);
        aDistEd2.SetText(aEmptyStr);
    }
    UpdateColMgr(0);
}

/*--------------------------------------------------------------------
    Description:    Update Bsp
 --------------------------------------------------------------------*/
void SwColumnPage::ActivatePage(const SfxItemSet& rSet)
{
    if(!bFrm)
    {
        if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_PAGE_SIZE ))
        {
            const SvxSizeItem& rSize = (const SvxSizeItem&)rSet.Get(
                                                SID_ATTR_PAGE_SIZE);
            const SvxLRSpaceItem& rLRSpace = (const SvxLRSpaceItem&)rSet.Get(
                                                                RES_LR_SPACE );
            const SvxBoxItem& rBox = (const SvxBoxItem&) rSet.Get(RES_BOX);
            sal_uInt16 nActWidth = static_cast< sal_uInt16 >(rSize.GetSize().Width()
                            - rLRSpace.GetLeft() - rLRSpace.GetRight() - rBox.GetDistance());

            if( pColMgr->GetActualSize() != nActWidth)
            {
                pColMgr->SetActualWidth(nActWidth);
                ColModify( 0 );
                UpdateColMgr( 0 );
            }
        }
        m_pFrmExampleWN->Hide();
        m_pPgeExampleWN->UpdateExample( rSet, pColMgr );
        m_pPgeExampleWN->Show();

    }
    else
    {
        m_pPgeExampleWN->Hide();
        m_pFrmExampleWN->Show();

        // Size
        const SwFmtFrmSize& rSize = (const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE);
        const SvxBoxItem& rBox = (const SvxBoxItem&) rSet.Get(RES_BOX);

        long nDistance = rBox.GetDistance();
        const sal_uInt16 nTotalWish = bFormat ? FRAME_FORMAT_WIDTH : sal_uInt16(rSize.GetWidth() - 2 * nDistance);

        // set maximum values of column width
        SetPageWidth(nTotalWish);

        if(pColMgr->GetActualSize() != nTotalWish)
        {
            pColMgr->SetActualWidth(nTotalWish);
            Init();
        }
        sal_Bool bPercent;
        // only relative data in frame format
        if ( bFormat || (rSize.GetWidthPercent() && rSize.GetWidthPercent() != 0xff) )
        {
            // set value for 100%
            aEd1.SetRefValue(nTotalWish);
            aEd2.SetRefValue(nTotalWish);
            aEd3.SetRefValue(nTotalWish);
            aDistEd1.SetRefValue(nTotalWish);
            aDistEd2.SetRefValue(nTotalWish);

            // switch to %-view
            bPercent = sal_True;
        }
        else
            bPercent = sal_False;

        aEd1.ShowPercent(bPercent);
        aEd2.ShowPercent(bPercent);
        aEd3.ShowPercent(bPercent);
        aDistEd1.ShowPercent(bPercent);
        aDistEd2.ShowPercent(bPercent);
        aDistEd1.SetMetricFieldMin(0);
        aDistEd2.SetMetricFieldMin(0);
    }
    Update();
}

int SwColumnPage::DeactivatePage(SfxItemSet *_pSet)
{
    if(_pSet)
        FillItemSet(*_pSet);

    return sal_True;
}

sal_uInt16* SwColumnPage::GetRanges()
{
    return aPageRg;
}

IMPL_LINK( SwColumnPage, SetDefaultsHdl, ValueSet *, pVS )
{
    sal_uInt16 nItem = pVS->GetSelectItemId();
    if( nItem < 4 )
    {
        m_pCLNrEdt->SetValue( nItem );
        m_pAutoWidthBox->Check();
        aDistEd1.SetPrcntValue(0);
        ColModify(0);
    }
    else
    {
        bLockUpdate = true;
        m_pCLNrEdt->SetValue( 2 );
        m_pAutoWidthBox->Check(sal_False);
        aDistEd1.SetPrcntValue(0);
        ColModify(0);
        // now set the width ratio to 2 : 1 or 1 : 2 respectively
        sal_uInt16 nSmall = pColMgr->GetActualSize()  / 3;
        if(nItem == 4)
        {
            aEd2.SetPrcntValue(aEd2.NormalizePercent(long(nSmall)), FUNIT_TWIP);
            pModifiedField = &aEd2;
        }
        else
        {
            aEd1.SetPrcntValue(aEd1.NormalizePercent(long(nSmall)), FUNIT_TWIP);
            pModifiedField = &aEd1;
        }
        bLockUpdate = false;
        Timeout();

    }
    return 0;
}

void SwColumnPage::SetFrmMode(sal_Bool bMod)
{
    bFrm = bMod;
}

void SwColumnPage::SetInSection(sal_Bool bSet)
{
    if(!SW_MOD()->GetCTLOptions().IsCTLFontEnabled())
        return;

    m_pTextDirectionFT->Show(bSet);
    m_pTextDirectionLB->Show(bSet);
}

void ColumnValueSet::UserDraw( const UserDrawEvent& rUDEvt )
{
    OutputDevice*  pDev = rUDEvt.GetDevice();
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    Rectangle aRect = rUDEvt.GetRect();
    sal_uInt16  nItemId = rUDEvt.GetItemId();
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
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        Format();
    }
    ValueSet::DataChanged( rDCEvt );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeColumnValueSet(Window *pParent, VclBuilder::stringmap &)
{
    return new ColumnValueSet(pParent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
