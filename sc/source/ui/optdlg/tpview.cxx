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



//------------------------------------------------------------------

#include "scitems.hxx"
#include "tpview.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "docsh.hxx"
#include "sc.hrc"       // -> Slot-IDs
#include "optdlg.hrc"
#include "globstr.hrc"
#include <appoptio.hxx>
#include <scmod.hxx>
#include <svx/dlgutil.hxx>
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>


// STATIC DATA -----------------------------------------------------------



ScTpContentOptions::ScTpContentOptions( Window*         pParent,
                             const SfxItemSet&  rArgSet ) :
    SfxTabPage(pParent, "TpViewPage", "modules/scalc/ui/tpviewpage.ui", rArgSet),
    pLocalOptions(0)
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
    Link aSelObjHdl(LINK( this, ScTpContentOptions, SelLbObjHdl ) );
    pObjGrfLB->  SetSelectHdl(aSelObjHdl);
    pDiagramLB-> SetSelectHdl(aSelObjHdl);
    pDrawLB->    SetSelectHdl(aSelObjHdl);
    pGridLB->    SetSelectHdl( LINK( this, ScTpContentOptions, GridHdl ) );

    Link aCBHdl(LINK( this, ScTpContentOptions, CBHdl ) );
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

}

ScTpContentOptions::~ScTpContentOptions()
{
    delete pLocalOptions;
}

SfxTabPage* ScTpContentOptions::Create( Window*     pParent,
                              const SfxItemSet&     rCoreSet )
{
    return new ScTpContentOptions(pParent, rCoreSet);
}

sal_Bool    ScTpContentOptions::FillItemSet( SfxItemSet& rCoreSet )
{
    sal_Bool bRet = false;
    if( pFormulaCB ->GetSavedValue() != pFormulaCB ->IsChecked() ||
        pNilCB     ->GetSavedValue() != pNilCB     ->IsChecked() ||
        pAnnotCB   ->GetSavedValue() != pAnnotCB   ->IsChecked() ||
        pValueCB   ->GetSavedValue() != pValueCB   ->IsChecked() ||
        pAnchorCB  ->GetSavedValue() != pAnchorCB  ->IsChecked() ||
        pClipMarkCB->GetSavedValue() != pClipMarkCB->IsChecked() ||
        pObjGrfLB  ->GetSavedValue() != pObjGrfLB  ->GetSelectEntryPos() ||
        pDiagramLB ->GetSavedValue() != pDiagramLB ->GetSelectEntryPos() ||
        pDrawLB    ->GetSavedValue() != pDrawLB    ->GetSelectEntryPos() ||
        pGridLB        ->GetSavedValue() != pGridLB->GetSelectEntryPos() ||
        pRowColHeaderCB->GetSavedValue() != pRowColHeaderCB->IsChecked() ||
        pHScrollCB     ->GetSavedValue() != pHScrollCB     ->IsChecked() ||
        pVScrollCB     ->GetSavedValue() != pVScrollCB     ->IsChecked() ||
        pTblRegCB      ->GetSavedValue() != pTblRegCB      ->IsChecked() ||
        pOutlineCB     ->GetSavedValue() != pOutlineCB     ->IsChecked() ||
        pColorLB       ->GetSavedValue() != pColorLB       ->GetSelectEntryPos() ||
        pBreakCB       ->GetSavedValue() != pBreakCB       ->IsChecked() ||
        pGuideLineCB   ->GetSavedValue() != pGuideLineCB   ->IsChecked())
    {
        pLocalOptions->SetGridColor( pColorLB->GetSelectEntryColor(),
                                     pColorLB->GetSelectEntry() );
        rCoreSet.Put(ScTpViewItem(SID_SCVIEWOPTIONS, *pLocalOptions));
        bRet = sal_True;
    }
    if(pRangeFindCB->GetSavedValue() != pRangeFindCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_RANGEFINDER, pRangeFindCB->IsChecked()));
        bRet = sal_True;
    }
    if(pSyncZoomCB->GetSavedValue() != pSyncZoomCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_OPT_SYNCZOOM, pSyncZoomCB->IsChecked()));
        bRet = sal_True;
    }


    return bRet;
}

void    ScTpContentOptions::Reset( const SfxItemSet& rCoreSet )
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        pLocalOptions  = new ScViewOptions(
                            ((const ScTpViewItem*)pItem)->GetViewOptions() );
    else
        pLocalOptions = new ScViewOptions;
    pFormulaCB ->Check(pLocalOptions->GetOption(VOPT_FORMULAS));
    pNilCB     ->Check(pLocalOptions->GetOption(VOPT_NULLVALS));
    pAnnotCB   ->Check(pLocalOptions->GetOption(VOPT_NOTES));
    pValueCB   ->Check(pLocalOptions->GetOption(VOPT_SYNTAX));
    pAnchorCB  ->Check(pLocalOptions->GetOption(VOPT_ANCHOR));
    pClipMarkCB->Check(pLocalOptions->GetOption(VOPT_CLIPMARKS));

    pObjGrfLB  ->SelectEntryPos( (sal_uInt16)pLocalOptions->GetObjMode(VOBJ_TYPE_OLE) );
    pDiagramLB ->SelectEntryPos( (sal_uInt16)pLocalOptions->GetObjMode(VOBJ_TYPE_CHART) );
    pDrawLB    ->SelectEntryPos( (sal_uInt16)pLocalOptions->GetObjMode(VOBJ_TYPE_DRAW) );

    pRowColHeaderCB->Check( pLocalOptions->GetOption(VOPT_HEADER) );
    pHScrollCB->Check( pLocalOptions->GetOption(VOPT_HSCROLL) );
    pVScrollCB->Check( pLocalOptions->GetOption(VOPT_VSCROLL) );
    pTblRegCB ->Check( pLocalOptions->GetOption(VOPT_TABCONTROLS) );
    pOutlineCB->Check( pLocalOptions->GetOption(VOPT_OUTLINER) );

    InitGridOpt();

    pBreakCB->Check( pLocalOptions->GetOption(VOPT_PAGEBREAKS) );
    pGuideLineCB->Check( pLocalOptions->GetOption(VOPT_HELPLINES) );

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_RANGEFINDER, false, &pItem))
        pRangeFindCB->Check(((const SfxBoolItem*)pItem)->GetValue());
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_OPT_SYNCZOOM, false, &pItem))
        pSyncZoomCB->Check(((const SfxBoolItem*)pItem)->GetValue());

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
    if(SFX_ITEM_SET == rSet.GetItemState(SID_SCVIEWOPTIONS, false , &pItem))
        *pLocalOptions = ((const ScTpViewItem*)pItem)->GetViewOptions();
}

int ScTpContentOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(*pSetP);
    return SfxTabPage::LEAVE_PAGE;
}

IMPL_LINK( ScTpContentOptions, SelLbObjHdl, ListBox*, pLb )
{
    sal_uInt16      nSelPos = pLb->GetSelectEntryPos();
    ScVObjMode  eMode   = ScVObjMode(nSelPos);
    ScVObjType  eType   = VOBJ_TYPE_OLE;

    if ( pLb == pDiagramLB )
        eType = VOBJ_TYPE_CHART;
    else if ( pLb == pDrawLB )
        eType = VOBJ_TYPE_DRAW;

    pLocalOptions->SetObjMode( eType, eMode );

    return 0;
}

IMPL_LINK( ScTpContentOptions, CBHdl, CheckBox*, pBtn )
{
    ScViewOption eOption = VOPT_FORMULAS;
    sal_Bool         bChecked = pBtn->IsChecked();

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


    return 0;
}

void ScTpContentOptions::InitGridOpt()
{
    sal_Bool    bGrid = pLocalOptions->GetOption( VOPT_GRID );
    sal_Bool    bGridOnTop = pLocalOptions->GetOption( VOPT_GRID_ONTOP );
    sal_uInt16  nSelPos = 0;

    if ( bGrid || bGridOnTop )
    {
        pColorFT->Enable(), pColorLB->Enable();
        if ( !bGridOnTop )
            nSelPos = 0;
        else
            nSelPos = 1;
    }
    else
    {
        pColorFT->Disable(), pColorLB->Disable();
        nSelPos = 2;
    }

    pGridLB->SelectEntryPos (nSelPos);

    if ( pColorLB->GetEntryCount() == 0 )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        // there might be another DocShell here
        pDocSh = PTR_CAST(ScDocShell, pDocSh);

        XColorListRef pColorList;
        if ( pDocSh  )
        {
            const SfxPoolItem* pItem = pDocSh->GetItem( SID_COLOR_TABLE );
            if ( pItem )
                pColorList = ((SvxColorListItem*)pItem)->GetColorList();
        }
        else
            pColorList = XColorList::GetStdColorList();

        if ( !pColorList.is() )
            return;

        //------------------------------------------------------

        pColorLB->SetUpdateMode( false );

        // items from ColorTable

        long nCount = pColorList->Count();
        for ( long n=0; n<nCount; n++ )
        {
            XColorEntry* pEntry = pColorList->GetColor(n);
            pColorLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        }

        // default GridColor

        Color aStdCol( SC_STD_GRIDCOLOR );          // same default as in ScViewOptions
        if ( LISTBOX_ENTRY_NOTFOUND ==
                pColorLB->GetEntryPos( aStdCol ) )
            pColorLB->InsertEntry( aStdCol, ScGlobal::GetRscString( STR_GRIDCOLOR ) );

        pColorLB->SetUpdateMode( sal_True );

        Invalidate();
    }

    //  also select grid color entry on subsequent calls

    String  aName;
    Color   aCol    = pLocalOptions->GetGridColor( &aName );
    nSelPos = pColorLB->GetEntryPos( aCol );

    if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
        pColorLB->SelectEntryPos( nSelPos );
    else
        pColorLB->SelectEntryPos( pColorLB->InsertEntry( aCol, aName ) );
}

IMPL_LINK( ScTpContentOptions, GridHdl, ListBox*, pLb )
{
    sal_uInt16  nSelPos = pLb->GetSelectEntryPos();
    sal_Bool    bGrid = ( nSelPos <= 1 );
    sal_Bool    bGridOnTop = ( nSelPos == 1 );

    pColorFT->Enable(bGrid);
    pColorLB->Enable(bGrid);
    pLocalOptions->SetOption( VOPT_GRID, bGrid );
    pLocalOptions->SetOption( VOPT_GRID_ONTOP, bGridOnTop );
    return 0;
}

ScTpLayoutOptions::ScTpLayoutOptions(   Window* pParent,
                                        const SfxItemSet&   rArgSet ) :
    SfxTabPage( pParent, "ScGeneralPage",
                "modules/scalc/ui/scgeneralpage.ui", rArgSet),
    aUnitArr(               ScResId(SCSTR_UNIT           )),
    pDoc(NULL)
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
    get( m_pMarkHdrCB, "markhdrcb");
    get( m_pTextFmtCB, "textfmtcb");
    get( m_pReplWarnCB, "replwarncb");
    get( m_pLegacyCellSelectionCB, "legacy_cell_selection_cb");

    SetExchangeSupport();

    m_pUnitLB->SetSelectHdl( LINK( this, ScTpLayoutOptions, MetricHdl ) );

    m_pAlignCB->SetClickHdl(LINK(this, ScTpLayoutOptions, AlignHdl));


    for ( sal_uInt16 i = 0; i < aUnitArr.Count(); ++i )
    {
        String sMetric = aUnitArr.GetStringByPos( i );
        FieldUnit eFUnit = (FieldUnit)aUnitArr.GetValue( i );

        switch ( eFUnit )
        {
            case FUNIT_MM:
            case FUNIT_CM:
            case FUNIT_POINT:
            case FUNIT_PICA:
            case FUNIT_INCH:
            {
                // nur diese Metriken benutzen
                sal_uInt16 nPos = m_pUnitLB->InsertEntry( sMetric );
                m_pUnitLB->SetEntryData( nPos, (void*)(sal_IntPtr)eFUnit );
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
}

SfxTabPage* ScTpLayoutOptions::Create( Window*          pParent,
                                    const SfxItemSet&   rCoreSet )
{
    ScTpLayoutOptions* pNew = new ScTpLayoutOptions(pParent, rCoreSet);
    ScDocShell* pDocSh = PTR_CAST(ScDocShell,SfxObjectShell::Current());

    if(pDocSh!=NULL)
        pNew->SetDocument(pDocSh->GetDocument());
    return pNew;
}

sal_Bool    ScTpLayoutOptions::FillItemSet( SfxItemSet& rCoreSet )
{
    sal_Bool bRet = sal_True;
    const sal_uInt16 nMPos = m_pUnitLB->GetSelectEntryPos();
    if ( nMPos != m_pUnitLB->GetSavedValue() )
    {
        sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pUnitLB->GetEntryData( nMPos );
        rCoreSet.Put( SfxUInt16Item( SID_ATTR_METRIC,
                                     (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }

    if(m_pTabMF->GetText() != m_pTabMF->GetSavedValue())
    {
        rCoreSet.Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    sal::static_int_cast<sal_uInt16>( m_pTabMF->Denormalize(m_pTabMF->GetValue(FUNIT_TWIP)) )));
        bRet = sal_True;
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

    if(m_pRequestRB->IsChecked() != m_pRequestRB->GetSavedValue() ||
            m_pNeverRB->IsChecked() != m_pNeverRB->GetSavedValue() )
    {
        if(pDoc)
            pDoc->SetLinkMode(nSet);
        ScAppOptions aAppOptions=SC_MOD()->GetAppOptions();
        aAppOptions.SetLinkMode(nSet );
        SC_MOD()->SetAppOptions(aAppOptions);
        bRet = sal_True;
    }
    if(m_pAlignCB->GetSavedValue() != m_pAlignCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_SELECTION, m_pAlignCB->IsChecked()));
        bRet = sal_True;
    }

    if(m_pAlignLB->GetSavedValue() != m_pAlignLB->GetSelectEntryPos())
    {
        rCoreSet.Put(SfxUInt16Item(SID_SC_INPUT_SELECTIONPOS, m_pAlignLB->GetSelectEntryPos()));
        bRet = sal_True;
    }

    if(m_pEditModeCB->GetSavedValue() != m_pEditModeCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_EDITMODE, m_pEditModeCB->IsChecked()));
        bRet = sal_True;
    }

    if(m_pFormatCB->GetSavedValue() != m_pFormatCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_FMT_EXPAND, m_pFormatCB->IsChecked()));
        bRet = sal_True;
    }


    if(m_pExpRefCB->GetSavedValue() != m_pExpRefCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_REF_EXPAND, m_pExpRefCB->IsChecked()));
        bRet = sal_True;
    }

    if(m_pMarkHdrCB->GetSavedValue() != m_pMarkHdrCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_MARK_HEADER, m_pMarkHdrCB->IsChecked()));
        bRet = sal_True;
    }

    if(m_pTextFmtCB->GetSavedValue() != m_pTextFmtCB->IsChecked())
    {
        rCoreSet.Put(SfxBoolItem(SID_SC_INPUT_TEXTWYSIWYG, m_pTextFmtCB->IsChecked()));
        bRet = sal_True;
    }

    if( m_pReplWarnCB->GetSavedValue() != m_pReplWarnCB->IsChecked() )
    {
        rCoreSet.Put( SfxBoolItem( SID_SC_INPUT_REPLCELLSWARN, m_pReplWarnCB->IsChecked() ) );
        bRet = sal_True;
    }

    if( m_pLegacyCellSelectionCB->GetSavedValue() != m_pLegacyCellSelectionCB->IsChecked() )
    {
        rCoreSet.Put( SfxBoolItem( SID_SC_INPUT_LEGACY_CELL_SELECTION, m_pLegacyCellSelectionCB->IsChecked() ) );
        bRet = sal_True;
    }

    return bRet;
}

void    ScTpLayoutOptions::Reset( const SfxItemSet& rCoreSet )
{
    m_pUnitLB->SetNoSelection();
    if ( rCoreSet.GetItemState( SID_ATTR_METRIC ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rCoreSet.Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();

        for ( sal_uInt16 i = 0; i < m_pUnitLB->GetEntryCount(); ++i )
        {
            if ( (FieldUnit)(sal_IntPtr)m_pUnitLB->GetEntryData( i ) == eFieldUnit )
            {
                m_pUnitLB->SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(*m_pTabMF, eFieldUnit);
    }
    m_pUnitLB->SaveValue();

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_ATTR_DEFTABSTOP, false, &pItem))
        m_pTabMF->SetValue(m_pTabMF->Normalize(((SfxUInt16Item*)pItem)->GetValue()), FUNIT_TWIP);
    m_pTabMF->SaveValue();

    m_pUnitLB       ->SaveValue();
    m_pTabMF        ->SaveValue();

    ScLkUpdMode nSet=LM_UNKNOWN;

    if(pDoc!=NULL)
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
    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_SELECTION, false, &pItem))
        m_pAlignCB->Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_SELECTIONPOS, false, &pItem))
        m_pAlignLB->SelectEntryPos(((const SfxUInt16Item*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_EDITMODE, false, &pItem))
        m_pEditModeCB->Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_FMT_EXPAND, false, &pItem))
        m_pFormatCB->Check(((const SfxBoolItem*)pItem)->GetValue());


    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_REF_EXPAND, false, &pItem))
        m_pExpRefCB->Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_MARK_HEADER, false, &pItem))
        m_pMarkHdrCB->Check(((const SfxBoolItem*)pItem)->GetValue());

    if(SFX_ITEM_SET == rCoreSet.GetItemState(SID_SC_INPUT_TEXTWYSIWYG, false, &pItem))
        m_pTextFmtCB->Check(((const SfxBoolItem*)pItem)->GetValue());

    if( SFX_ITEM_SET == rCoreSet.GetItemState( SID_SC_INPUT_REPLCELLSWARN, false, &pItem ) )
        m_pReplWarnCB->Check( ( (const SfxBoolItem*)pItem)->GetValue() );

    if( SFX_ITEM_SET == rCoreSet.GetItemState( SID_SC_INPUT_LEGACY_CELL_SELECTION, false, &pItem ) )
        m_pLegacyCellSelectionCB->Check( ( (const SfxBoolItem*)pItem)->GetValue() );


    m_pAlignCB    ->SaveValue();
    m_pAlignLB    ->SaveValue();
    m_pEditModeCB ->SaveValue();
    m_pFormatCB   ->SaveValue();

    m_pExpRefCB   ->SaveValue();
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

int ScTpLayoutOptions::DeactivatePage( SfxItemSet* pSetP )
{
    if(pSetP)
        FillItemSet(*pSetP);
    return SfxTabPage::LEAVE_PAGE;
}

IMPL_LINK_NOARG(ScTpLayoutOptions, MetricHdl)
{
    const sal_uInt16 nMPos = m_pUnitLB->GetSelectEntryPos();
    if(nMPos != USHRT_MAX)
    {
        FieldUnit eFieldUnit = (FieldUnit)(sal_IntPtr)m_pUnitLB->GetEntryData( nMPos );
        sal_Int64 nVal =
            m_pTabMF->Denormalize( m_pTabMF->GetValue( FUNIT_TWIP ) );
        ::SetFieldUnit( *m_pTabMF, eFieldUnit );
        m_pTabMF->SetValue( m_pTabMF->Normalize( nVal ), FUNIT_TWIP );
    }

    return 0;
}

IMPL_LINK( ScTpLayoutOptions, AlignHdl, CheckBox*, pBox )
{
    m_pAlignLB->Enable(pBox->IsChecked());
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
