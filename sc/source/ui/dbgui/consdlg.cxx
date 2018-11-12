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

#include <sfx2/dispatch.hxx>

#include <tabvwsh.hxx>
#include <uiitems.hxx>
#include <dbdata.hxx>
#include <rangenam.hxx>
#include <rangeutl.hxx>
#include <reffact.hxx>
#include <document.hxx>
#include <scresid.hxx>

#include <globstr.hrc>
#include <strings.hrc>

#include <consdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

namespace
{
    void INFOBOX(weld::Window* pWindow, const char* id)
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(pWindow,
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(id)));
        xInfoBox->run();
    }
}

class ScAreaData
{
public:
    ScAreaData()
    {
    }

    void Set( const OUString& rName, const OUString& rArea )
    {
        aStrName  = rName;
        aStrArea  = rArea;
    }

    OUString  aStrName;
    OUString  aStrArea;
};

ScConsolidateDlg::ScConsolidateDlg( SfxBindings* pB, SfxChildWindow* pCW, vcl::Window* pParent,
                                    const SfxItemSet&   rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, "ConsolidateDialog" , "modules/scalc/ui/consolidatedialog.ui" ),
        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        theConsData     ( static_cast<const ScConsolidateItem&>(
                           rArgSet.Get( rArgSet.GetPool()->
                                            GetWhich( SID_CONSOLIDATE ) )
                                      ).GetData() ),
        rViewData       ( static_cast<ScTabViewShell*>(SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( static_cast<ScTabViewShell*>(SfxViewShell::Current())->
                                GetViewData().GetDocument() ),
        pRangeUtil      ( new ScRangeUtil ),
        nAreaDataCount  ( 0 ),
        nWhichCons      ( rArgSet.GetPool()->GetWhich( SID_CONSOLIDATE ) ),
        bDlgLostFocus   ( false )
{
    get(pLbFunc,"func");
    get(pLbConsAreas,"consareas");

    get(pLbDataArea,"lbdataarea");
    get(pEdDataArea,"eddataarea");
    get(pRbDataArea,"rbdataarea");

    pRefInputEdit = pEdDataArea;

    get(pLbDestArea,"lbdestarea");
    get(pEdDestArea,"eddestarea");
    get(pRbDestArea,"rbdestarea");

    get(pExpander,"more");
    get(pBtnByRow,"byrow");
    get(pBtnByCol,"bycol");
    get(pBtnRefs,"refs");

    get(pBtnOk,"ok");
    get(pBtnCancel,"cancel");
    get(pBtnAdd,"add");
    get(pBtnRemove,"delete");

    Init();
}

ScConsolidateDlg::~ScConsolidateDlg()
{
    disposeOnce();
}

void ScConsolidateDlg::dispose()
{
    pAreaData.reset();
    pRangeUtil.reset();
    pLbFunc.clear();
    pLbConsAreas.clear();
    pLbDataArea.clear();
    pEdDataArea.clear();
    pRbDataArea.clear();
    pLbDestArea.clear();
    pEdDestArea.clear();
    pRbDestArea.clear();
    pExpander.clear();
    pBtnByRow.clear();
    pBtnByCol.clear();
    pBtnRefs.clear();
    pBtnOk.clear();
    pBtnCancel.clear();
    pBtnAdd.clear();
    pBtnRemove.clear();
    pRefInputEdit.clear();
    ScAnyRefDlg::dispose();
}

void ScConsolidateDlg::Init()
{
    OSL_ENSURE( pDoc && pRangeUtil, "Error in Ctor" );

    OUString aStr;
    sal_uInt16 i=0;

    pRbDataArea->SetReferences(this, pEdDataArea);
    pEdDataArea->SetReferences(this, get<FixedText>("ftdataarea"));
    pRbDestArea->SetReferences(this, pEdDestArea);
    pEdDestArea->SetReferences(this, get<FixedText>("ftdestarea"));

    pEdDataArea ->SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    pEdDestArea ->SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    pLbDataArea ->SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    pLbDestArea ->SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    pEdDataArea ->SetModifyHdl   ( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    pEdDestArea ->SetModifyHdl   ( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    pLbConsAreas->SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    pLbDataArea ->SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    pLbDestArea ->SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    pBtnOk      ->SetClickHdl    ( LINK( this, ScConsolidateDlg, OkHdl ) );
    pBtnCancel  ->SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );
    pBtnAdd     ->SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );
    pBtnRemove  ->SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );

    pBtnAdd->Disable();
    pBtnRemove->Disable();

    pBtnByRow->Check( theConsData.bByRow );
    pBtnByCol->Check( theConsData.bByCol );
    pBtnRefs->Check( theConsData.bReferenceData );

    pLbFunc->SelectEntryPos( FuncToLbPos( theConsData.eFunction ) );

    // Hack: pLbConsAreas used to be MultiLB. We don't have VCL builder equivalent
    // of it yet. So enable selecting multiple items here
    pLbConsAreas->EnableMultiSelection( true );

    pLbConsAreas->set_width_request(pLbConsAreas->approximate_char_width() * 16);
    pLbConsAreas->SetDropDownLineCount(5);

    // read consolidation areas
    pLbConsAreas->Clear();
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    for ( i=0; i<theConsData.nDataAreaCount; i++ )
    {
        const ScArea& rArea = theConsData.pDataAreas[i];
        if ( rArea.nTab < pDoc->GetTableCount() )
        {
            aStr = ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                    rArea.nColEnd, rArea.nRowEnd, rArea.nTab ).Format(
                        ScRefFlags::RANGE_ABS_3D, pDoc, eConv );
            pLbConsAreas->InsertEntry( aStr );
        }
    }

    if ( theConsData.nTab < pDoc->GetTableCount() )
    {
        aStr = ScAddress( theConsData.nCol, theConsData.nRow, theConsData.nTab
                ).Format( ScRefFlags::ADDR_ABS_3D, pDoc, eConv );
        pEdDestArea->SetText( aStr );
    }
    else
        pEdDestArea->SetText(OUString());

    // Use the ScAreaData helper class to save those range names from the
    // RangeNames and database ranges that appear in the ListBoxes.

    ScRangeName*    pRangeNames  = pDoc->GetRangeName();
    ScDBCollection* pDbNames     = pDoc->GetDBCollection();
    size_t nRangeCount = pRangeNames ? pRangeNames->size() : 0;
    size_t nDbCount = pDbNames ? pDbNames->getNamedDBs().size() : 0;

    nAreaDataCount = nRangeCount+nDbCount;
    pAreaData      = nullptr;

    if ( nAreaDataCount > 0 )
    {
        pAreaData.reset( new ScAreaData[nAreaDataCount] );

        OUString aStrName;
        sal_uInt16 nAt = 0;
        ScRange aRange;
        ScAreaNameIterator aIter( pDoc );
        while ( aIter.Next( aStrName, aRange ) )
        {
            OUString aStrArea(aRange.Format(ScRefFlags::ADDR_ABS_3D, pDoc, eConv));
            pAreaData[nAt++].Set( aStrName, aStrArea );
        }
    }

    FillAreaLists();
    ModifyHdl( *pEdDestArea );
    pLbDataArea->SelectEntryPos( 0 );
    pEdDataArea->SetText(OUString());
    pEdDataArea->GrabFocus();

    //aFlSep.SetStyle( aFlSep.GetStyle() | WB_VERT );

    //@BugID 54702 enable/disable only in base class
    //SFX_APPWINDOW->Enable();
}

void ScConsolidateDlg::FillAreaLists()
{
    pLbDataArea->Clear();
    pLbDestArea->Clear();
    pLbDataArea->InsertEntry( aStrUndefined );
    pLbDestArea->InsertEntry( aStrUndefined );

    if ( pRangeUtil && pAreaData && (nAreaDataCount > 0) )
    {
        for ( size_t i=0;
              (i<nAreaDataCount) && (!pAreaData[i].aStrName.isEmpty());
              i++ )
        {
            pLbDataArea->InsertEntry( pAreaData[i].aStrName, i+1 );

//          if ( !pAreaData[i].bIsDbArea )
            pLbDestArea->InsertEntry( pAreaData[i].aStrName, i+1 );
        }
    }
}

// Handover of a range within a table that has been selected by the mouse.
// This range is then shown in the reference window as new selection.

void ScConsolidateDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( pRefInputEdit )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        OUString      aStr;
        ScRefFlags      nFmt = ScRefFlags::RANGE_ABS_3D;       //!!! nCurTab is still missing
        const formula::FormulaGrammar::AddressConvention eConv = pDocP->GetAddressConvention();

        if ( rRef.aStart.Tab() != rRef.aEnd.Tab() )
            nFmt |= ScRefFlags::TAB2_3D;

        if ( pRefInputEdit == pEdDataArea)
            aStr = rRef.Format(nFmt, pDocP, eConv);
        else if ( pRefInputEdit == pEdDestArea )
            aStr = rRef.aStart.Format(nFmt, pDocP, eConv);

        pRefInputEdit->SetRefString( aStr );
        ModifyHdl( *pRefInputEdit );
    }
}

bool ScConsolidateDlg::Close()
{
    return DoClose( ScConsolidateDlgWrapper::GetChildWindowId() );
}

void ScConsolidateDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;

        if ( pRefInputEdit )
        {
            pRefInputEdit->GrabFocus();
            ModifyHdl( *pRefInputEdit );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}

void ScConsolidateDlg::Deactivate()
{
    bDlgLostFocus = true;
}

bool ScConsolidateDlg::VerifyEdit( formula::RefEdit* pEd )
{
    if ( !pRangeUtil || !pDoc ||
         ((pEd != pEdDataArea) && (pEd != pEdDestArea)) )
        return false;

    SCTAB    nTab    = rViewData.GetTabNo();
    bool bEditOk = false;
    OUString theCompleteStr;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

    if ( pEd == pEdDataArea )
    {
        bEditOk = ScRangeUtil::IsAbsArea( pEd->GetText(), pDoc,
                                         nTab, &theCompleteStr, nullptr, nullptr, eConv );
    }
    else if ( pEd == pEdDestArea )
    {
        OUString aPosStr;

        ScRangeUtil::CutPosString( pEd->GetText(), aPosStr );
        bEditOk = ScRangeUtil::IsAbsPos( aPosStr, pDoc,
                                        nTab, &theCompleteStr, nullptr, eConv );
    }

    if ( bEditOk )
        pEd->SetText( theCompleteStr );

    return bEditOk;
}

// Handler:

IMPL_LINK( ScConsolidateDlg, GetFocusHdl, Control&, rControl, void )
{
    if ( &rControl ==static_cast<Control*>(pEdDataArea) ||
         &rControl ==static_cast<Control*>(pEdDestArea))
    {
        pRefInputEdit = static_cast<formula::RefEdit*>(&rControl);
    }
    else if(&rControl ==static_cast<Control*>(pLbDataArea) )
    {
        pRefInputEdit = pEdDataArea;
    }
    else if(&rControl ==static_cast<Control*>(pLbDestArea) )
    {
        pRefInputEdit = pEdDestArea;
    }
}

IMPL_LINK_NOARG(ScConsolidateDlg, OkHdl, Button*, void)
{
    const sal_Int32 nDataAreaCount = pLbConsAreas->GetEntryCount();

    if ( nDataAreaCount > 0 )
    {
        ScRefAddress aDestAddress;
        SCTAB       nTab = rViewData.GetTabNo();
        OUString    aDestPosStr( pEdDestArea->GetText() );
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( ScRangeUtil::IsAbsPos( aDestPosStr, pDoc, nTab, nullptr, &aDestAddress, eConv ) )
        {
            ScConsolidateParam  theOutParam( theConsData );
            std::unique_ptr<ScArea[]> pDataAreas(new ScArea[nDataAreaCount]);

            for ( sal_Int32 i=0; i<nDataAreaCount; ++i )
            {
                ScRangeUtil::MakeArea( pLbConsAreas->GetEntry( i ),
                                      pDataAreas[i], pDoc, nTab, eConv );
            }

            theOutParam.nCol            = aDestAddress.Col();
            theOutParam.nRow            = aDestAddress.Row();
            theOutParam.nTab            = aDestAddress.Tab();
            theOutParam.eFunction       = LbPosToFunc( pLbFunc->GetSelectedEntryPos() );
            theOutParam.bByCol          = pBtnByCol->IsChecked();
            theOutParam.bByRow          = pBtnByRow->IsChecked();
            theOutParam.bReferenceData  = pBtnRefs->IsChecked();
            theOutParam.SetAreas( std::move(pDataAreas), nDataAreaCount );

            ScConsolidateItem aOutItem( nWhichCons, &theOutParam );

            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->ExecuteList(SID_CONSOLIDATE,
                                      SfxCallMode::SLOT | SfxCallMode::RECORD,
                                      { &aOutItem });
            Close();
        }
        else
        {
            INFOBOX(GetFrameWeld(), STR_INVALID_TABREF);
            pEdDestArea->GrabFocus();
        }
    }
    else
        Close(); // no area defined -> Cancel
}

IMPL_LINK( ScConsolidateDlg, ClickHdl, Button*, pBtn, void )
{
    if ( pBtn == pBtnCancel )
        Close();
    else if ( pBtn == pBtnAdd )
    {
        if ( !pEdDataArea->GetText().isEmpty() )
        {
            OUString    aNewEntry( pEdDataArea->GetText() );
            std::unique_ptr<ScArea[]> ppAreas;
            sal_uInt16      nAreaCount = 0;
            const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

            if ( ScRangeUtil::IsAbsTabArea( aNewEntry, pDoc, &ppAreas, &nAreaCount, true, eConv ) )
            {
                // IsAbsTabArea() creates an array of ScArea pointers,
                // which have been created dynamically as well.
                // These objects need to be deleted here.

                for ( sal_uInt16 i=0; i<nAreaCount; i++ )
                {
                    const ScArea& rArea = ppAreas[i];
                    OUString aNewArea = ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                            rArea.nColEnd, rArea.nRowEnd, rArea.nTab
                            ).Format(ScRefFlags::RANGE_ABS_3D, pDoc, eConv);

                    if ( pLbConsAreas->GetEntryPos( aNewArea )
                         == LISTBOX_ENTRY_NOTFOUND )
                    {
                        pLbConsAreas->InsertEntry( aNewArea );
                    }
                }
            }
            else if ( VerifyEdit( pEdDataArea ) )
            {
                OUString aNewArea( pEdDataArea->GetText() );

                if ( pLbConsAreas->GetEntryPos( aNewArea ) == LISTBOX_ENTRY_NOTFOUND )
                    pLbConsAreas->InsertEntry( aNewArea );
                else
                    INFOBOX(GetFrameWeld(), STR_AREA_ALREADY_INSERTED);
            }
            else
            {
                INFOBOX(GetFrameWeld(), STR_INVALID_TABREF);
                pEdDataArea->GrabFocus();
            }
        }
    }
    else if ( pBtn == pBtnRemove )
    {
        while ( pLbConsAreas->GetSelectedEntryCount() )
            pLbConsAreas->RemoveEntry( pLbConsAreas->GetSelectedEntryPos() );
        pBtnRemove->Disable();
    }
}

IMPL_LINK( ScConsolidateDlg, SelectHdl, ListBox&, rLb, void )
{
    if ( &rLb == pLbConsAreas )
    {
        if ( pLbConsAreas->GetSelectedEntryCount() > 0 )
            pBtnRemove->Enable();
        else
            pBtnRemove->Disable();
    }
    else if ( (&rLb == pLbDataArea) || (&rLb == pLbDestArea) )
    {
        Edit*   pEd = (&rLb == pLbDataArea) ? pEdDataArea : pEdDestArea;
        const sal_Int32 nSelPos = rLb.GetSelectedEntryPos();

        if (    pRangeUtil
            && (nSelPos > 0)
            && (nAreaDataCount > 0)
            && (pAreaData != nullptr) )
        {
            if ( static_cast<size_t>(nSelPos) <= nAreaDataCount )
            {
                OUString aString( pAreaData[nSelPos-1].aStrArea );

                if ( &rLb == pLbDestArea )
                    ScRangeUtil::CutPosString( aString, aString );

                pEd->SetText( aString );

                if ( pEd == pEdDataArea )
                    pBtnAdd->Enable();
            }
        }
        else
        {
            pEd->SetText( EMPTY_OUSTRING );
            if ( pEd == pEdDataArea )
                pBtnAdd->Enable();
        }
    }
}

IMPL_LINK( ScConsolidateDlg, ModifyHdl, Edit&, rEd, void )
{
    if ( &rEd == pEdDataArea )
    {
        OUString aAreaStr( rEd.GetText() );
        if ( !aAreaStr.isEmpty() )
        {
            pBtnAdd->Enable();
        }
        else
            pBtnAdd->Disable();
    }
    else if ( &rEd == pEdDestArea )
    {
        pLbDestArea->SelectEntryPos(0);
    }
}

// TODO: generalize!
// Resource of the ListBox and these two conversion methods are also in
// tpsubt and everywhere, where StarCalc functions are selectable.

ScSubTotalFunc ScConsolidateDlg::LbPosToFunc( sal_Int32 nPos )
{
    switch ( nPos )
    {
        case  2:    return SUBTOTAL_FUNC_AVE;
        case  6:    return SUBTOTAL_FUNC_CNT;
        case  1:    return SUBTOTAL_FUNC_CNT2;
        case  3:    return SUBTOTAL_FUNC_MAX;
        case  4:    return SUBTOTAL_FUNC_MIN;
        case  5:    return SUBTOTAL_FUNC_PROD;
        case  7:    return SUBTOTAL_FUNC_STD;
        case  8:    return SUBTOTAL_FUNC_STDP;
        case  9:    return SUBTOTAL_FUNC_VAR;
        case 10:    return SUBTOTAL_FUNC_VARP;
        case  0:
        default:
            return SUBTOTAL_FUNC_SUM;
    }
}

sal_Int32 ScConsolidateDlg::FuncToLbPos( ScSubTotalFunc eFunc )
{
    switch ( eFunc )
    {
        case SUBTOTAL_FUNC_AVE:     return 2;
        case SUBTOTAL_FUNC_CNT:     return 6;
        case SUBTOTAL_FUNC_CNT2:    return 1;
        case SUBTOTAL_FUNC_MAX:     return 3;
        case SUBTOTAL_FUNC_MIN:     return 4;
        case SUBTOTAL_FUNC_PROD:    return 5;
        case SUBTOTAL_FUNC_STD:     return 7;
        case SUBTOTAL_FUNC_STDP:    return 8;
        case SUBTOTAL_FUNC_VAR:     return 9;
        case SUBTOTAL_FUNC_VARP:    return 10;
        case SUBTOTAL_FUNC_NONE:
        case SUBTOTAL_FUNC_SUM:
        default:
            return 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
