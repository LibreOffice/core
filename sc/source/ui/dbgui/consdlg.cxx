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

#include "tabvwsh.hxx"
#include "uiitems.hxx"
#include "dbdata.hxx"
#include "rangenam.hxx"
#include "rangeutl.hxx"
#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"

#include "globstr.hrc"
#include "sc.hrc" // consdlg.hrc has been deleted

#define _CONSDLG_CXX
#include "consdlg.hxx"
#undef _CONSDLG_CXX
#include <vcl/msgbox.hxx>

#define INFOBOX(id) InfoBox(this, ScGlobal::GetRscString(id)).Execute()

//============================================================================
//  class ScAreaData

class ScAreaData
{
public:
    ScAreaData()  {}
    ~ScAreaData() {}

    void Set( const String& rName, const String& rArea, sal_Bool bDb )
                {
                    aStrName  = rName;
                    aStrArea  = rArea;
                    bIsDbArea = bDb;
                }

    String  aStrName;
    String  aStrArea;
    sal_Bool    bIsDbArea;
};


//============================================================================
//  class ScConsolidateDialog


ScConsolidateDlg::ScConsolidateDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                    const SfxItemSet&   rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, "ConsolidateDialog" , "modules/scalc/ui/consolidatedialog.ui" ),
        //
        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
        //
        theConsData     ( ((const ScConsolidateItem&)
                           rArgSet.Get( rArgSet.GetPool()->
                                            GetWhich( SID_CONSOLIDATE ) )
                                      ).GetData() ),
        pViewData       ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData() ),
        pDoc            ( ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument() ),
        pRangeUtil      ( new ScRangeUtil ),
        pAreaData       ( NULL ),
        nAreaDataCount  ( 0 ),
        nWhichCons      ( rArgSet.GetPool()->GetWhich( SID_CONSOLIDATE ) ),

        pRefInputEdit   ( pEdDataArea )
{
    get(pLbFunc,"func");
    get(pLbConsAreas,"consareas");

    get(pLbDataArea,"lbdataarea");
    get(pEdDataArea,"eddataarea");
    get(pRbDataArea,"rbdataarea");

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


//----------------------------------------------------------------------------

ScConsolidateDlg::~ScConsolidateDlg()
{
    delete [] pAreaData;
    delete pRangeUtil;
}


//----------------------------------------------------------------------------

void ScConsolidateDlg::Init()
{
    OSL_ENSURE( pViewData && pDoc && pRangeUtil, "Error in Ctor" );

    String aStr;
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
    pLbConsAreas->EnableMultiSelection( sal_True );

    pLbConsAreas->set_width_request(pLbConsAreas->approximate_char_width() * 16);
    pLbConsAreas->SetDropDownLineCount(5);

    // Einlesen der Konsolidierungsbereiche
    pLbConsAreas->Clear();
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    for ( i=0; i<theConsData.nDataAreaCount; i++ )
    {
        const ScArea& rArea = *(theConsData.ppDataAreas[i] );
        if ( rArea.nTab < pDoc->GetTableCount() )
        {
            ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                    rArea.nColEnd, rArea.nRowEnd, rArea.nTab ).Format( aStr,
                        SCR_ABS_3D, pDoc, eConv );
            pLbConsAreas->InsertEntry( aStr );
        }
    }

    if ( theConsData.nTab < pDoc->GetTableCount() )
    {
        ScAddress( theConsData.nCol, theConsData.nRow, theConsData.nTab
                ).Format( aStr, SCA_ABS_3D, pDoc, eConv );
        pEdDestArea->SetText( aStr );
    }
    else
        pEdDestArea->SetText( EMPTY_STRING );

    //----------------------------------------------------------

    /*
     * Aus den RangeNames und Datenbankbereichen werden sich
     * in der Hilfsklasse ScAreaData die Bereichsnamen gemerkt,
     * die in den ListBoxen erscheinen.
     */

    ScRangeName*    pRangeNames  = pDoc->GetRangeName();
    ScDBCollection* pDbNames     = pDoc->GetDBCollection();
    size_t nRangeCount = pRangeNames ? pRangeNames->size() : 0;
    size_t nDbCount = pDbNames ? pDbNames->getNamedDBs().size() : 0;

    nAreaDataCount = nRangeCount+nDbCount;
    pAreaData      = NULL;

    if ( nAreaDataCount > 0 )
    {
        pAreaData = new ScAreaData[nAreaDataCount];

        String aStrName;
        String aStrArea;
        sal_uInt16 nAt = 0;
        ScRange aRange;
        ScAreaNameIterator aIter( pDoc );
        while ( aIter.Next( aStrName, aRange ) )
        {
            aRange.Format( aStrArea, SCA_ABS_3D, pDoc, eConv );
            pAreaData[nAt++].Set( aStrName, aStrArea, aIter.WasDBName() );
        }
    }

    FillAreaLists();
    ModifyHdl( pEdDestArea );
    pLbDataArea->SelectEntryPos( 0 );
    pEdDataArea->SetText( EMPTY_STRING );
    pEdDataArea->GrabFocus();

    //aFlSep.SetStyle( aFlSep.GetStyle() | WB_VERT );

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
}


//----------------------------------------------------------------------------
void ScConsolidateDlg::FillAreaLists()
{
    pLbDataArea->Clear();
    pLbDestArea->Clear();
    pLbDataArea->InsertEntry( aStrUndefined );
    pLbDestArea->InsertEntry( aStrUndefined );

    if ( pRangeUtil && pAreaData && (nAreaDataCount > 0) )
    {
        for ( size_t i=0;
              (i<nAreaDataCount) && (pAreaData[i].aStrName.Len()>0);
              i++ )
        {
            pLbDataArea->InsertEntry( pAreaData[i].aStrName, i+1 );

//          if ( !pAreaData[i].bIsDbArea )
                pLbDestArea->InsertEntry( pAreaData[i].aStrName, i+1 );
        }
    }
}


//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.


void ScConsolidateDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( pRefInputEdit )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        String      aStr;
        sal_uInt16      nFmt = SCR_ABS_3D;       //!!! nCurTab fehlt noch
        const formula::FormulaGrammar::AddressConvention eConv = pDocP->GetAddressConvention();

        if ( rRef.aStart.Tab() != rRef.aEnd.Tab() )
            nFmt |= SCA_TAB2_3D;

        if ( pRefInputEdit == pEdDataArea)
            rRef.Format( aStr, nFmt, pDocP, eConv );
        else if ( pRefInputEdit == pEdDestArea )
            rRef.aStart.Format( aStr, nFmt, pDocP, eConv );

        pRefInputEdit->SetRefString( aStr );
    }

    ModifyHdl( pRefInputEdit );
}


//----------------------------------------------------------------------------

sal_Bool ScConsolidateDlg::Close()
{
    return DoClose( ScConsolidateDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------

void ScConsolidateDlg::SetActive()
{
    if ( bDlgLostFocus )
    {
        bDlgLostFocus = false;

        if ( pRefInputEdit )
        {
            pRefInputEdit->GrabFocus();
            ModifyHdl( pRefInputEdit );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}


//----------------------------------------------------------------------------

void ScConsolidateDlg::Deactivate()
{
    bDlgLostFocus = sal_True;
}


//----------------------------------------------------------------------------

sal_Bool ScConsolidateDlg::VerifyEdit( formula::RefEdit* pEd )
{
    if ( !pRangeUtil || !pDoc || !pViewData ||
         ((pEd != pEdDataArea) && (pEd != pEdDestArea)) )
        return false;

    SCTAB   nTab    = pViewData->GetTabNo();
    sal_Bool    bEditOk = false;
    String  theCompleteStr;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

    if ( pEd == pEdDataArea )
    {
        bEditOk = pRangeUtil->IsAbsArea( pEd->GetText(), pDoc,
                                         nTab, &theCompleteStr, NULL, NULL, eConv );
    }
    else if ( pEd == pEdDestArea )
    {
        String aPosStr;

        pRangeUtil->CutPosString( pEd->GetText(), aPosStr );
        bEditOk = pRangeUtil->IsAbsPos( aPosStr, pDoc,
                                        nTab, &theCompleteStr, NULL, eConv );
    }

    if ( bEditOk )
        pEd->SetText( theCompleteStr );

    return bEditOk;
}


//----------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScConsolidateDlg, GetFocusHdl, Control*, pCtr )
{
    if ( pCtr ==(Control*)pEdDataArea ||
         pCtr ==(Control*)pEdDestArea)
    {
        pRefInputEdit = (formula::RefEdit*)pCtr;
    }
    else if(pCtr ==(Control*)pLbDataArea )
    {
        pRefInputEdit = pEdDataArea;
    }
    else if(pCtr ==(Control*)pLbDestArea )
    {
        pRefInputEdit = pEdDestArea;
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScConsolidateDlg, OkHdl)
{
    sal_uInt16 nDataAreaCount = pLbConsAreas->GetEntryCount();

    if ( nDataAreaCount > 0 )
    {
        ScRefAddress aDestAddress;
        SCTAB       nTab = pViewData->GetTabNo();
        String      aDestPosStr( pEdDestArea->GetText() );
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( pRangeUtil->IsAbsPos( aDestPosStr, pDoc, nTab, NULL, &aDestAddress, eConv ) )
        {
            ScConsolidateParam  theOutParam( theConsData );
            ScArea**            ppDataAreas = new ScArea*[nDataAreaCount];
            ScArea*             pArea;
            sal_uInt16              i=0;

            for ( i=0; i<nDataAreaCount; i++ )
            {
                pArea = new ScArea;
                pRangeUtil->MakeArea( pLbConsAreas->GetEntry( i ),
                                      *pArea, pDoc, nTab, eConv );
                ppDataAreas[i] = pArea;
            }

            theOutParam.nCol            = aDestAddress.Col();
            theOutParam.nRow            = aDestAddress.Row();
            theOutParam.nTab            = aDestAddress.Tab();
            theOutParam.eFunction       = LbPosToFunc( pLbFunc->GetSelectEntryPos() );
            theOutParam.bByCol          = pBtnByCol->IsChecked();
            theOutParam.bByRow          = pBtnByRow->IsChecked();
            theOutParam.bReferenceData  = pBtnRefs->IsChecked();
            theOutParam.SetAreas( ppDataAreas, nDataAreaCount );

            for ( i=0; i<nDataAreaCount; i++ )
                delete ppDataAreas[i];
            delete [] ppDataAreas;

            ScConsolidateItem aOutItem( nWhichCons, &theOutParam );

            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->Execute( SID_CONSOLIDATE,
                                      SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      &aOutItem, 0L, 0L );
            Close();
        }
        else
        {
            INFOBOX( STR_INVALID_TABREF );
            pEdDestArea->GrabFocus();
        }
    }
    else
        Close(); // keine Datenbereiche definiert -> Cancel
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, ClickHdl, PushButton*, pBtn )
{
    if ( pBtn == pBtnCancel )
        Close();
    else if ( pBtn == pBtnAdd )
    {
        if ( !pEdDataArea->GetText().isEmpty() )
        {
            String      aNewEntry( pEdDataArea->GetText() );
            ScArea**    ppAreas = NULL;
            sal_uInt16      nAreaCount = 0;
            const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

            if ( pRangeUtil->IsAbsTabArea( aNewEntry, pDoc, &ppAreas, &nAreaCount, sal_True, eConv ) )
            {
                // IsAbsTabArea() legt ein Array von ScArea-Zeigern an,
                // welche ebenfalls dynamisch erzeugt wurden.
                // Diese Objekte muessen hier abgeraeumt werden.

                for ( sal_uInt16 i=0; i<nAreaCount; i++ )
                {
                    String aNewArea;

                    if ( ppAreas[i] )
                    {
                        const ScArea& rArea = *(ppAreas[i]);
                        ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                                rArea.nColEnd, rArea.nRowEnd, rArea.nTab
                                ).Format( aNewArea, SCR_ABS_3D, pDoc, eConv );

                        if ( pLbConsAreas->GetEntryPos( aNewArea )
                             == LISTBOX_ENTRY_NOTFOUND )
                        {
                            pLbConsAreas->InsertEntry( aNewArea );
                        }
                        delete ppAreas[i];
                    }
                }
                delete [] ppAreas;
            }
            else if ( VerifyEdit( pEdDataArea ) )
            {
                String aNewArea( pEdDataArea->GetText() );

                if ( pLbConsAreas->GetEntryPos( aNewArea ) == LISTBOX_ENTRY_NOTFOUND )
                    pLbConsAreas->InsertEntry( aNewArea );
                else
                    INFOBOX( STR_AREA_ALREADY_INSERTED );
            }
            else
            {
                INFOBOX( STR_INVALID_TABREF );
                pEdDataArea->GrabFocus();
            }
        }
    }
    else if ( pBtn == pBtnRemove )
    {
        while ( pLbConsAreas->GetSelectEntryCount() )
            pLbConsAreas->RemoveEntry( pLbConsAreas->GetSelectEntryPos() );
        pBtnRemove->Disable();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, SelectHdl, ListBox*, pLb )
{
    if ( pLb == pLbConsAreas )
    {
        if ( pLbConsAreas->GetSelectEntryCount() > 0 )
            pBtnRemove->Enable();
        else
            pBtnRemove->Disable();
    }
    else if ( (pLb == pLbDataArea) || (pLb == pLbDestArea) )
    {
        Edit*   pEd = (pLb == pLbDataArea) ? pEdDataArea : pEdDestArea;
        sal_uInt16  nSelPos = pLb->GetSelectEntryPos();

        if (    pRangeUtil
            && (nSelPos > 0)
            && (nAreaDataCount > 0)
            && (pAreaData != NULL) )
        {
            if ( static_cast<size_t>(nSelPos) <= nAreaDataCount )
            {
                String aString( pAreaData[nSelPos-1].aStrArea );

                if ( pLb == pLbDestArea )
                    pRangeUtil->CutPosString( aString, aString );

                pEd->SetText( aString );

                if ( pEd == pEdDataArea )
                    pBtnAdd->Enable();
            }
        }
        else
        {
            pEd->SetText( EMPTY_STRING );
            if ( pEd == pEdDataArea )
                pBtnAdd->Enable();
        }
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, ModifyHdl, formula::RefEdit*, pEd )
{
    if ( pEd == pEdDataArea )
    {
        String aAreaStr( pEd->GetText() );
        if ( aAreaStr.Len() > 0 )
        {
            pBtnAdd->Enable();
        }
        else
            pBtnAdd->Disable();
    }
    else if ( pEd == pEdDestArea )
    {
        pLbDestArea->SelectEntryPos(0);
    }
    return 0;
}


//----------------------------------------------------------------------------
// Verallgemeinern!!! :
// Resource der ListBox und diese beiden Umrechnungsmethoden gibt es
// auch noch in tpsubt bzw. ueberall, wo StarCalc-Funktionen
// auswaehlbar sind.

ScSubTotalFunc ScConsolidateDlg::LbPosToFunc( sal_uInt16 nPos )
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


//----------------------------------------------------------------------------

sal_uInt16 ScConsolidateDlg::FuncToLbPos( ScSubTotalFunc eFunc )
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
