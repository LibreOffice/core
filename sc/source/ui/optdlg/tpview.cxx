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

#undef SC_DLLIMPLEMENTATION

#include <scitems.hxx>
#include <tpview.hxx>
#include <global.hxx>
#include <viewopti.hxx>
#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <scresid.hxx>
#include <docsh.hxx>
#include <sc.hrc>
#include <strings.hrc>
#include <globstr.hrc>
#include <units.hrc>
#include <appoptio.hxx>
#include <scmod.hxx>
#include <svx/colorbox.hxx>
#include <svx/dlgutil.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>

ScTpContentOptions::ScTpContentOptions( vcl::Window*         pParent,
                             const SfxItemSet&  rArgSet ) :
    SfxTabPage(pParent, "TpViewPage", "modules/scalc/ui/tpviewpage.ui", &rArgSet)
{
    get(pGridLB,"grid");
    get(pColorFT,"color_label");
    get(pColorLB,"color");
    get(pBreakCB,"break");
    get(pGuideLineCB,"guideline");

    get(pFormulaCB,"formula");
    get(pNilCB,"nil");
    get(pAnnotCB,"annot");
    get(pValueCB,"value");
    get(pAnchorCB,"anchor");
    get(pClipMarkCB,"clipmark");
    get(pRangeFindCB,"rangefind");

    get(pObjGrfLB,"objgrf");
    get(pDiagramLB,"diagram");
    get(pDrawLB,"draw");

    get(pSyncZoomCB,"synczoom");

    get(pRowColHeaderCB,"rowcolheader");
    get(pHScrollCB,"hscroll");
    get(pVScrollCB,"vscroll");
    get(pTblRegCB,"tblreg");
    get(pOutlineCB,"outline");

    SetExchangeSupport();
    Link<ListBox&,void> aSelObjHdl(LINK( this, ScTpContentOptions, SelLbObjHdl ) );
    pObjGrfLB->  SetSelectHdl(aSelObjHdl);
    pDiagramLB-> SetSelectHdl(aSelObjHdl);
    pDrawLB->    SetSelectHdl(aSelObjHdl);
    pGridLB->    SetSelectHdl( LINK( this, ScTpContentOptions, GridHdl ) );

    Link<Button*, void> aCBHdl(LINK( this, ScTpContentOptions, CBHdl ) );
    pFormulaCB  ->SetClickHdl(aCBHdl);
    pNilCB      ->SetClickHdl(aCBHdl);
    pAnnotCB    ->SetClickHdl(aCBHdl);
    pValueCB    ->SetClickHdl(aCBHdl);
    pAnchorCB   ->SetClickHdl(aCBHdl);
    pClipMarkCB ->SetClickHdl(aCBHdl);

    pVScrollCB  ->SetClickHdl(aCBHdl);
    pHScrollCB  ->SetClickHdl(aCBHdl);
    pTblRegCB   ->SetClickHdl(aCBHdl);
    pOutlineCB  ->SetClickHdl(aCBHdl);
    pBreakCB    ->SetClickHdl(aCBHdl);
    pGuideLineCB->SetClickHdl(aCBHdl);
    pRowColHeaderCB->SetClickHdl(aCBHdl);

    pColorLB->SetSlotId(SID_ATTR_CHAR_COLOR);
    pColorLB->SetAutoDisplayColor(SC_STD_GRIDCOLOR);
}

ScTpContentOptions::~ScTpContentOptions()
{
    disposeOnce();
}

void ScTpContentOptions::dispose()
{
    pLocalOptions.reset();
    pGridLB.clear();
    pColorFT.clear();
    pColorLB.clear();
    pBreakCB.clear();
    pGuideLineCB.clear();
    pFormulaCB.clear();
    pNilCB.clear();
    pAnnotCB.clear();
    pValueCB.clear();
    pAnchorCB.clear();
    pClipMarkCB.clear();
    pRangeFindCB.clear();
    pObjGrfLB.clear();
    pDiagramLB.clear();
    pDrawLB.clear();
    pSyncZoomCB.clear();
    pRowColHeaderCB.clear();
    pHScrollCB.clear();
    pVScrollCB.clear();
    pTblRegCB.clear();
    pOutlineCB.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> ScTpContentOptions::Create( TabPageParent pParent,
                                               const SfxItemSet*     rCoreSet )
{
    return VclPtr<ScTpContentOptions>::Create(pParent.pParent, *rCoreSet);
}

bool    ScTpContentOptions::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bRet = false;
    if( pFormulaCB ->IsValueChangedFromSaved() ||
        pNilCB     ->IsValueChangedFromSaved() ||
        pAnnotCB   ->IsValueChangedFromSaved() ||
        pValueCB   ->IsValueChangedFromSaved() ||
        pAnchorCB  ->IsValueChangedFromSaved() ||
        pClipMarkCB->IsValueChangedFromSaved() ||
        pObjGrfLB  ->IsValueChangedFromSaved() ||
        pDiagramLB ->IsValueChangedFromSaved() ||
        pDrawLB    ->IsValueChangedFromSaved() ||
        pGridLB        ->IsValueChangedFromSaved() ||
        pRowColHeaderCB->IsValueChangedFromSaved() ||
        pHScrollCB     ->IsValueChangedFromSaved() ||
        pVScrollCB     ->IsValueChangedFromSaved() ||
        pTblRegCB      ->IsValueChangedFromSaved() ||
        pOutlineCB     ->IsValueChangedFromSaved() ||
        pColorLB       ->IsValueChangedFromSaved() ||
        pBreakCB       ->IsValueChangedFromSaved() ||
        pGuideLineCB   ->IsValueChangedFromSaved())
    {
        NamedColor aNamedColor = pColorLB->GetSelectedEntry();
        if (aNamedColor.first == COL_AUTO)
        {
            aNamedColor.first = SC_STD_GRIDCOLOR;
            aNamedColor.second.clear();
        }
        pLocalOptions->SetGridColor(aNamedColor.first, aNamedColor.second);
        rCoreSet->Put(ScTpViewItem(*pLocalOptions));
        bRet = true;
    }
    if(pRangeFindCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_RANGEFINDER, pRangeFindCB->IsChecked()));
        bRet = true;
    }
    if(pSyncZoomCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_OPT_SYNCZOOM, pSyncZoomCB->IsChecked()));
        bRet = true;
    }

    return bRet;
}

void    ScTpContentOptions::Reset( const SfxItemSet* rCoreSet )
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        pLocalOptions.reset( new ScViewOptions(
                            static_cast<const ScTpViewItem*>(pItem)->GetViewOptions() ) );
    else
        pLocalOptions.reset( new ScViewOptions );
    pFormulaCB ->Check(pLocalOptions->GetOption(VOPT_FORMULAS));
    pNilCB     ->Check(pLocalOptions->GetOption(VOPT_NULLVALS));
    pAnnotCB   ->Check(pLocalOptions->GetOption(VOPT_NOTES));
    pValueCB   ->Check(pLocalOptions->GetOption(VOPT_SYNTAX));
    pAnchorCB  ->Check(pLocalOptions->GetOption(VOPT_ANCHOR));
    pClipMarkCB->Check(pLocalOptions->GetOption(VOPT_CLIPMARKS));

    pObjGrfLB  ->SelectEntryPos( static_cast<sal_uInt16>(pLocalOptions->GetObjMode(VOBJ_TYPE_OLE)) );
    pDiagramLB ->SelectEntryPos( static_cast<sal_uInt16>(pLocalOptions->GetObjMode(VOBJ_TYPE_CHART)) );
    pDrawLB    ->SelectEntryPos( static_cast<sal_uInt16>(pLocalOptions->GetObjMode(VOBJ_TYPE_DRAW)) );

    pRowColHeaderCB->Check( pLocalOptions->GetOption(VOPT_HEADER) );
    pHScrollCB->Check( pLocalOptions->GetOption(VOPT_HSCROLL) );
    pVScrollCB->Check( pLocalOptions->GetOption(VOPT_VSCROLL) );
    pTblRegCB ->Check( pLocalOptions->GetOption(VOPT_TABCONTROLS) );
    pOutlineCB->Check( pLocalOptions->GetOption(VOPT_OUTLINER) );

    InitGridOpt();

    pBreakCB->Check( pLocalOptions->GetOption(VOPT_PAGEBREAKS) );
    pGuideLineCB->Check( pLocalOptions->GetOption(VOPT_HELPLINES) );

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_RANGEFINDER, false, &pItem))
        pRangeFindCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_OPT_SYNCZOOM, false, &pItem))
        pSyncZoomCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    pRangeFindCB->SaveValue();
    pSyncZoomCB->SaveValue();

    pFormulaCB->SaveValue();
    pNilCB->SaveValue();
    pAnnotCB->SaveValue();
    pValueCB->SaveValue();
    pAnchorCB->SaveValue();
    pClipMarkCB->SaveValue();
    pObjGrfLB->SaveValue();
    pDiagramLB->SaveValue();
    pDrawLB->SaveValue();
    pRowColHeaderCB->SaveValue();
    pHScrollCB->SaveValue();
    pVScrollCB->SaveValue();
    pTblRegCB->SaveValue();
    pOutlineCB->SaveValue();
    pGridLB->SaveValue();
    pColorLB->SaveValue();
    pBreakCB->SaveValue();
    pGuideLineCB->SaveValue();
}

void ScTpContentOptions::ActivatePage( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rSet.GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        *pLocalOptions = static_cast<const ScTpViewItem*>(pItem)->GetViewOptions();
}

DeactivateRC ScTpContentOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(pSetP);
    return DeactivateRC::LeavePage;
}

IMPL_LINK( ScTpContentOptions, SelLbObjHdl, ListBox&, rLb, void )
{
    const sal_Int32 nSelPos = rLb.GetSelectedEntryPos();
    ScVObjMode  eMode   = ScVObjMode(nSelPos);
    ScVObjType  eType   = VOBJ_TYPE_OLE;

    if ( &rLb == pDiagramLB )
        eType = VOBJ_TYPE_CHART;
    else if ( &rLb == pDrawLB )
        eType = VOBJ_TYPE_DRAW;

    pLocalOptions->SetObjMode( eType, eMode );
}

IMPL_LINK( ScTpContentOptions, CBHdl, Button*, pBtn, void )
{
    ScViewOption eOption = VOPT_FORMULAS;
    bool         bChecked = static_cast<CheckBox*>(pBtn)->IsChecked();

    if (      pFormulaCB   == pBtn )   eOption = VOPT_FORMULAS;
    else if ( pNilCB       == pBtn )   eOption = VOPT_NULLVALS;
    else if ( pAnnotCB     == pBtn )   eOption = VOPT_NOTES;
    else if ( pValueCB     == pBtn )   eOption = VOPT_SYNTAX;
    else if ( pAnchorCB    == pBtn )   eOption = VOPT_ANCHOR;
    else if ( pClipMarkCB  == pBtn )   eOption = VOPT_CLIPMARKS;
    else if ( pVScrollCB       == pBtn )   eOption = VOPT_VSCROLL;
    else if ( pHScrollCB       == pBtn )   eOption = VOPT_HSCROLL;
    else if ( pTblRegCB        == pBtn )   eOption = VOPT_TABCONTROLS;
    else if ( pOutlineCB       == pBtn )   eOption = VOPT_OUTLINER;
    else if ( pBreakCB         == pBtn )   eOption = VOPT_PAGEBREAKS;
    else if ( pGuideLineCB     == pBtn )   eOption = VOPT_HELPLINES;
    else if ( pRowColHeaderCB  == pBtn )   eOption = VOPT_HEADER;

    pLocalOptions->SetOption( eOption, bChecked );
}

void ScTpContentOptions::InitGridOpt()
{
    bool    bGrid = pLocalOptions->GetOption( VOPT_GRID );
    bool    bGridOnTop = pLocalOptions->GetOption( VOPT_GRID_ONTOP );
    sal_Int32   nSelPos = 0;

    if ( bGrid || bGridOnTop )
    {
        pColorFT->Enable();
        pColorLB->Enable();
        if ( !bGridOnTop )
            nSelPos = 0;
        else
            nSelPos = 1;
    }
    else
    {
        pColorFT->Disable();
        pColorLB->Disable();
        nSelPos = 2;
    }

    pGridLB->SelectEntryPos (nSelPos);

    //  select grid color entry
    OUString  aName;
    Color     aCol    = pLocalOptions->GetGridColor( &aName );

    if (aName.trim().isEmpty() && aCol == SC_STD_GRIDCOLOR)
        aCol = COL_AUTO;

    pColorLB->SelectEntry(std::make_pair(aCol, aName));
}

IMPL_LINK( ScTpContentOptions, GridHdl, ListBox&, rLb, void )
{
    sal_Int32   nSelPos = rLb.GetSelectedEntryPos();
    bool    bGrid = ( nSelPos <= 1 );
    bool    bGridOnTop = ( nSelPos == 1 );

    pColorFT->Enable(bGrid);
    pColorLB->Enable(bGrid);
    pLocalOptions->SetOption( VOPT_GRID, bGrid );
    pLocalOptions->SetOption( VOPT_GRID_ONTOP, bGridOnTop );
}

ScTpLayoutOptions::ScTpLayoutOptions(   vcl::Window* pParent,
                                        const SfxItemSet&   rArgSet ) :
    SfxTabPage( pParent, "ScGeneralPage",
                "modules/scalc/ui/scgeneralpage.ui", &rArgSet),
    pDoc(nullptr)
{
    get( m_pUnitLB, "unitlb");
    get( m_pTabMF, "tabmf");

    get( m_pAlwaysRB, "alwaysrb");
    get( m_pRequestRB, "requestrb");
    get( m_pNeverRB, "neverrb");

    get( m_pAlignCB, "aligncb");
    get( m_pAlignLB, "alignlb");
    get( m_pEditModeCB, "editmodecb");
    get( m_pFormatCB, "formatcb");
    get( m_pExpRefCB, "exprefcb");
    get( m_pSortRefUpdateCB, "sortrefupdatecb");
    get( m_pMarkHdrCB, "markhdrcb");
    get( m_pTextFmtCB, "textfmtcb");
    get( m_pReplWarnCB, "replwarncb");
    get( m_pLegacyCellSelectionCB, "legacy_cell_selection_cb");

    SetExchangeSupport();

    m_pUnitLB->SetSelectHdl( LINK( this, ScTpLayoutOptions, MetricHdl ) );

    m_pAlignCB->SetClickHdl(LINK(this, ScTpLayoutOptions, AlignHdl));

    for (size_t i = 0; i < SAL_N_ELEMENTS(SCSTR_UNIT); ++i)
    {
        OUString sMetric = ScResId(SCSTR_UNIT[i].first);
        FieldUnit eFUnit = SCSTR_UNIT[i].second;

        switch ( eFUnit )
        {
            case FUNIT_MM:
            case FUNIT_CM:
            case FUNIT_POINT:
            case FUNIT_PICA:
            case FUNIT_INCH:
            {
                // only use these metrics
                sal_Int32 nPos = m_pUnitLB->InsertEntry( sMetric );
                m_pUnitLB->SetEntryData( nPos, reinterpret_cast<void*>(static_cast<sal_IntPtr>(eFUnit)) );
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

ScTpLayoutOptions::~ScTpLayoutOptions()
{
    disposeOnce();
}

void ScTpLayoutOptions::dispose()
{
    m_pUnitLB.clear();
    m_pTabMF.clear();
    m_pAlwaysRB.clear();
    m_pRequestRB.clear();
    m_pNeverRB.clear();
    m_pAlignCB.clear();
    m_pAlignLB.clear();
    m_pEditModeCB.clear();
    m_pFormatCB.clear();
    m_pExpRefCB.clear();
    m_pSortRefUpdateCB.clear();
    m_pMarkHdrCB.clear();
    m_pTextFmtCB.clear();
    m_pReplWarnCB.clear();
    m_pLegacyCellSelectionCB.clear();
    SfxTabPage::dispose();
}


VclPtr<SfxTabPage> ScTpLayoutOptions::Create( TabPageParent pParent,
                                              const SfxItemSet*   rCoreSet )
{
    VclPtrInstance<ScTpLayoutOptions> pNew( pParent.pParent, *rCoreSet );
    ScDocShell* pDocSh = dynamic_cast< ScDocShell *>( SfxObjectShell::Current() );

    if(pDocSh!=nullptr)
        pNew->pDoc = &pDocSh->GetDocument();
    return pNew;
}

bool    ScTpLayoutOptions::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bRet = true;
    const sal_Int32 nMPos = m_pUnitLB->GetSelectedEntryPos();
    if ( m_pUnitLB->IsValueChangedFromSaved() )
    {
        sal_uInt16 nFieldUnit = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(m_pUnitLB->GetEntryData( nMPos )));
        rCoreSet->Put( SfxUInt16Item( SID_ATTR_METRIC, nFieldUnit ) );
        bRet = true;
    }

    if(m_pTabMF->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    sal::static_int_cast<sal_uInt16>( m_pTabMF->Denormalize(m_pTabMF->GetValue(FUNIT_TWIP)) )));
        bRet = true;
    }

    ScLkUpdMode nSet=LM_ALWAYS;

    if(m_pRequestRB->IsChecked())
    {
        nSet=LM_ON_DEMAND;
    }
    else if(m_pNeverRB->IsChecked())
    {
        nSet=LM_NEVER;
    }

    if(m_pRequestRB->IsValueChangedFromSaved() ||
       m_pNeverRB->IsValueChangedFromSaved() )
    {
        if(pDoc)
            pDoc->SetLinkMode(nSet);
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        aAppOptions.SetLinkMode(nSet );
        SC_MOD()->SetAppOptions(aAppOptions);
        bRet = true;
    }
    if(m_pAlignCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_SELECTION, m_pAlignCB->IsChecked()));
        bRet = true;
    }

    if(m_pAlignLB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxUInt16Item(SID_SC_INPUT_SELECTIONPOS, m_pAlignLB->GetSelectedEntryPos()));
        bRet = true;
    }

    if(m_pEditModeCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_EDITMODE, m_pEditModeCB->IsChecked()));
        bRet = true;
    }

    if(m_pFormatCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_FMT_EXPAND, m_pFormatCB->IsChecked()));
        bRet = true;
    }

    if(m_pExpRefCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_REF_EXPAND, m_pExpRefCB->IsChecked()));
        bRet = true;
    }

    if (m_pSortRefUpdateCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_OPT_SORT_REF_UPDATE, m_pSortRefUpdateCB->IsChecked()));
        bRet = true;
    }

    if(m_pMarkHdrCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_MARK_HEADER, m_pMarkHdrCB->IsChecked()));
        bRet = true;
    }

    if(m_pTextFmtCB->IsValueChangedFromSaved())
    {
        rCoreSet->Put(SfxBoolItem(SID_SC_INPUT_TEXTWYSIWYG, m_pTextFmtCB->IsChecked()));
        bRet = true;
    }

    if( m_pReplWarnCB->IsValueChangedFromSaved() )
    {
        rCoreSet->Put( SfxBoolItem( SID_SC_INPUT_REPLCELLSWARN, m_pReplWarnCB->IsChecked() ) );
        bRet = true;
    }

    if( m_pLegacyCellSelectionCB->IsValueChangedFromSaved() )
    {
        rCoreSet->Put( SfxBoolItem( SID_SC_INPUT_LEGACY_CELL_SELECTION, m_pLegacyCellSelectionCB->IsChecked() ) );
        bRet = true;
    }

    return bRet;
}

void    ScTpLayoutOptions::Reset( const SfxItemSet* rCoreSet )
{
    m_pUnitLB->SetNoSelection();
    if ( rCoreSet->GetItemState( SID_ATTR_METRIC ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = rCoreSet->Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = static_cast<FieldUnit>(rItem.GetValue());

        for ( sal_Int32 i = 0; i < m_pUnitLB->GetEntryCount(); ++i )
        {
            if ( static_cast<FieldUnit>(reinterpret_cast<sal_IntPtr>(m_pUnitLB->GetEntryData( i ))) == eFieldUnit )
            {
                m_pUnitLB->SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(*m_pTabMF, eFieldUnit);
    }
    m_pUnitLB->SaveValue();

    const SfxPoolItem* pItem;
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_ATTR_DEFTABSTOP, false, &pItem))
        m_pTabMF->SetValue(m_pTabMF->Normalize(static_cast<const SfxUInt16Item*>(pItem)->GetValue()), FUNIT_TWIP);
    m_pTabMF->SaveValue();

    m_pUnitLB       ->SaveValue();
    m_pTabMF        ->SaveValue();

    ScLkUpdMode nSet=LM_UNKNOWN;

    if(pDoc!=nullptr)
    {
        nSet=pDoc->GetLinkMode();
    }

    if(nSet==LM_UNKNOWN)
    {
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        nSet=aAppOptions.GetLinkMode();
    }

    switch(nSet)
    {
        case LM_ALWAYS:     m_pAlwaysRB->  Check();    break;
        case LM_NEVER:      m_pNeverRB->   Check();    break;
        case LM_ON_DEMAND:  m_pRequestRB-> Check();    break;
        default:
        {
            // added to avoid warnings
        }
    }
    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_SELECTION, false, &pItem))
        m_pAlignCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_SELECTIONPOS, false, &pItem))
        m_pAlignLB->SelectEntryPos(static_cast<const SfxUInt16Item*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_EDITMODE, false, &pItem))
        m_pEditModeCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_FMT_EXPAND, false, &pItem))
        m_pFormatCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_REF_EXPAND, false, &pItem))
        m_pExpRefCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if (rCoreSet->HasItem(SID_SC_OPT_SORT_REF_UPDATE, &pItem))
        m_pSortRefUpdateCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_MARK_HEADER, false, &pItem))
        m_pMarkHdrCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if(SfxItemState::SET == rCoreSet->GetItemState(SID_SC_INPUT_TEXTWYSIWYG, false, &pItem))
        m_pTextFmtCB->Check(static_cast<const SfxBoolItem*>(pItem)->GetValue());

    if( SfxItemState::SET == rCoreSet->GetItemState( SID_SC_INPUT_REPLCELLSWARN, false, &pItem ) )
        m_pReplWarnCB->Check( static_cast<const SfxBoolItem*>(pItem)->GetValue() );

    if( SfxItemState::SET == rCoreSet->GetItemState( SID_SC_INPUT_LEGACY_CELL_SELECTION, false, &pItem ) )
        m_pLegacyCellSelectionCB->Check( static_cast<const SfxBoolItem*>(pItem)->GetValue() );

    m_pAlignCB    ->SaveValue();
    m_pAlignLB    ->SaveValue();
    m_pEditModeCB ->SaveValue();
    m_pFormatCB   ->SaveValue();

    m_pExpRefCB   ->SaveValue();
    m_pSortRefUpdateCB->SaveValue();
    m_pMarkHdrCB  ->SaveValue();
    m_pTextFmtCB  ->SaveValue();
    m_pReplWarnCB ->SaveValue();

    m_pLegacyCellSelectionCB->SaveValue();

    AlignHdl(m_pAlignCB);

    m_pAlwaysRB->SaveValue();
    m_pNeverRB->SaveValue();
    m_pRequestRB->SaveValue();
}

void    ScTpLayoutOptions::ActivatePage( const SfxItemSet& /* rCoreSet */ )
{
}

DeactivateRC ScTpLayoutOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(pSetP);
    return DeactivateRC::LeavePage;
}

IMPL_LINK_NOARG(ScTpLayoutOptions, MetricHdl, ListBox&, void)
{
    const sal_Int32 nMPos = m_pUnitLB->GetSelectedEntryPos();
    if(nMPos != LISTBOX_ENTRY_NOTFOUND)
    {
        FieldUnit eFieldUnit = static_cast<FieldUnit>(reinterpret_cast<sal_IntPtr>(m_pUnitLB->GetEntryData( nMPos )));
        sal_Int64 nVal =
            m_pTabMF->Denormalize( m_pTabMF->GetValue( FUNIT_TWIP ) );
        ::SetFieldUnit( *m_pTabMF, eFieldUnit );
        m_pTabMF->SetValue( m_pTabMF->Normalize( nVal ), FUNIT_TWIP );
    }
}

IMPL_LINK( ScTpLayoutOptions, AlignHdl, Button*, pBox, void )
{
    m_pAlignLB->Enable(static_cast<CheckBox*>(pBox)->IsChecked());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
