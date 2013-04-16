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
#include "consdlg.hrc"

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

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_CONSOLIDATE ),
        //
        aFtFunc         ( this, ScResId( FT_FUNC ) ),
        aLbFunc         ( this, ScResId( LB_FUNC ) ),

        aFtConsAreas    ( this, ScResId( FT_CONSAREAS ) ),
        aLbConsAreas    ( this, ScResId( LB_CONSAREAS ) ),

        aLbDataArea     ( this, ScResId( LB_DATA_AREA ) ),
        aFtDataArea     ( this, ScResId( FT_DATA_AREA ) ),
        aEdDataArea     ( this, this, &aFtDataArea, ScResId( ED_DATA_AREA ) ),
        aRbDataArea     ( this, ScResId( RB_DATA_AREA ), &aEdDataArea, this ),

        aLbDestArea     ( this, ScResId( LB_DEST_AREA ) ),
        aFtDestArea     ( this, ScResId( FT_DEST_AREA ) ),
        aEdDestArea     ( this, this, &aFtDestArea, ScResId( ED_DEST_AREA ) ),
        aRbDestArea     ( this, ScResId( RB_DEST_AREA ), &aEdDestArea, this),

        aFlConsBy       ( this, ScResId( FL_CONSBY ) ),
        aBtnByRow       ( this, ScResId( BTN_BYROW ) ),
        aBtnByCol       ( this, ScResId( BTN_BYCOL) ),

        aFlSep          ( this, ScResId( FL_SEP ) ),
        aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
        aBtnRefs        ( this, ScResId( BTN_REFS ) ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnAdd         ( this, ScResId( BTN_ADD ) ),
        aBtnRemove      ( this, ScResId( BTN_REMOVE ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),

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

        pRefInputEdit   ( &aEdDataArea )
{
    Init();
    FreeResource();
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

    aEdDataArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aEdDestArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aLbDataArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aLbDestArea .SetGetFocusHdl( LINK( this, ScConsolidateDlg, GetFocusHdl ) );
    aEdDataArea .SetModifyHdl   ( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    aEdDestArea .SetModifyHdl   ( LINK( this, ScConsolidateDlg, ModifyHdl ) );
    aLbConsAreas.SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    aLbDataArea .SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    aLbDestArea .SetSelectHdl   ( LINK( this, ScConsolidateDlg, SelectHdl ) );
    aBtnOk      .SetClickHdl    ( LINK( this, ScConsolidateDlg, OkHdl ) );
    aBtnCancel  .SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );
    aBtnAdd     .SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );
    aBtnRemove  .SetClickHdl    ( LINK( this, ScConsolidateDlg, ClickHdl ) );

    aBtnMore.AddWindow( &aFlConsBy );
    aBtnMore.AddWindow( &aBtnByRow );
    aBtnMore.AddWindow( &aBtnByCol );
    aBtnMore.AddWindow( &aFlSep );
    aBtnMore.AddWindow( &aFlOptions );
    aBtnMore.AddWindow( &aBtnRefs );

    aBtnAdd.Disable();
    aBtnRemove.Disable();

    aBtnByRow.Check( theConsData.bByRow );
    aBtnByCol.Check( theConsData.bByCol );
    aBtnRefs .Check( theConsData.bReferenceData );

    aLbFunc.SelectEntryPos( FuncToLbPos( theConsData.eFunction ) );

    // Einlesen der Konsolidierungsbereiche
    aLbConsAreas.Clear();
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();
    for ( i=0; i<theConsData.nDataAreaCount; i++ )
    {
        const ScArea& rArea = *(theConsData.ppDataAreas[i] );
        if ( rArea.nTab < pDoc->GetTableCount() )
        {
            ScRange( rArea.nColStart, rArea.nRowStart, rArea.nTab,
                    rArea.nColEnd, rArea.nRowEnd, rArea.nTab ).Format( aStr,
                        SCR_ABS_3D, pDoc, eConv );
            aLbConsAreas.InsertEntry( aStr );
        }
    }

    if ( theConsData.nTab < pDoc->GetTableCount() )
    {
        ScAddress( theConsData.nCol, theConsData.nRow, theConsData.nTab
                ).Format( aStr, SCA_ABS_3D, pDoc, eConv );
        aEdDestArea.SetText( aStr );
    }
    else
        aEdDestArea.SetText( EMPTY_STRING );

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
    ModifyHdl( &aEdDestArea );
    aLbDataArea.SelectEntryPos( 0 );
    aEdDataArea.SetText( EMPTY_STRING );
    aEdDataArea.GrabFocus();

    aFlSep.SetStyle( aFlSep.GetStyle() | WB_VERT );

    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Enable();
}


//----------------------------------------------------------------------------
void ScConsolidateDlg::FillAreaLists()
{
    aLbDataArea.Clear();
    aLbDestArea.Clear();
    aLbDataArea.InsertEntry( aStrUndefined );
    aLbDestArea.InsertEntry( aStrUndefined );

    if ( pRangeUtil && pAreaData && (nAreaDataCount > 0) )
    {
        for ( size_t i=0;
              (i<nAreaDataCount) && (pAreaData[i].aStrName.Len()>0);
              i++ )
        {
            aLbDataArea.InsertEntry( pAreaData[i].aStrName, i+1 );

//          if ( !pAreaData[i].bIsDbArea )
                aLbDestArea.InsertEntry( pAreaData[i].aStrName, i+1 );
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

        if ( pRefInputEdit == &aEdDataArea)
            rRef.Format( aStr, nFmt, pDocP, eConv );
        else if ( pRefInputEdit == &aEdDestArea )
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
         ((pEd != &aEdDataArea) && (pEd != &aEdDestArea)) )
        return false;

    SCTAB   nTab    = pViewData->GetTabNo();
    sal_Bool    bEditOk = false;
    String  theCompleteStr;
    const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

    if ( pEd == &aEdDataArea )
    {
        bEditOk = pRangeUtil->IsAbsArea( pEd->GetText(), pDoc,
                                         nTab, &theCompleteStr, NULL, NULL, eConv );
    }
    else if ( pEd == &aEdDestArea )
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
    if ( pCtr ==(Control*)&aEdDataArea ||
         pCtr ==(Control*)&aEdDestArea)
    {
        pRefInputEdit = (formula::RefEdit*)pCtr;
    }
    else if(pCtr ==(Control*)&aLbDataArea )
    {
        pRefInputEdit = &aEdDataArea;
    }
    else if(pCtr ==(Control*)&aLbDestArea )
    {
        pRefInputEdit = &aEdDestArea;
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK_NOARG(ScConsolidateDlg, OkHdl)
{
    sal_uInt16 nDataAreaCount = aLbConsAreas.GetEntryCount();

    if ( nDataAreaCount > 0 )
    {
        ScRefAddress aDestAddress;
        SCTAB       nTab = pViewData->GetTabNo();
        String      aDestPosStr( aEdDestArea.GetText() );
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
                pRangeUtil->MakeArea( aLbConsAreas.GetEntry( i ),
                                      *pArea, pDoc, nTab, eConv );
                ppDataAreas[i] = pArea;
            }

            theOutParam.nCol            = aDestAddress.Col();
            theOutParam.nRow            = aDestAddress.Row();
            theOutParam.nTab            = aDestAddress.Tab();
            theOutParam.eFunction       = LbPosToFunc( aLbFunc.GetSelectEntryPos() );
            theOutParam.bByCol          = aBtnByCol.IsChecked();
            theOutParam.bByRow          = aBtnByRow.IsChecked();
            theOutParam.bReferenceData  = aBtnRefs.IsChecked();
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
            aEdDestArea.GrabFocus();
        }
    }
    else
        Close(); // keine Datenbereiche definiert -> Cancel
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, ClickHdl, PushButton*, pBtn )
{
    if ( pBtn == &aBtnCancel )
        Close();
    else if ( pBtn == &aBtnAdd )
    {
        if ( !aEdDataArea.GetText().isEmpty() )
        {
            String      aNewEntry( aEdDataArea.GetText() );
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

                        if ( aLbConsAreas.GetEntryPos( aNewArea )
                             == LISTBOX_ENTRY_NOTFOUND )
                        {
                            aLbConsAreas.InsertEntry( aNewArea );
                        }
                        delete ppAreas[i];
                    }
                }
                delete [] ppAreas;
            }
            else if ( VerifyEdit( &aEdDataArea ) )
            {
                String aNewArea( aEdDataArea.GetText() );

                if ( aLbConsAreas.GetEntryPos( aNewArea ) == LISTBOX_ENTRY_NOTFOUND )
                    aLbConsAreas.InsertEntry( aNewArea );
                else
                    INFOBOX( STR_AREA_ALREADY_INSERTED );
            }
            else
            {
                INFOBOX( STR_INVALID_TABREF );
                aEdDataArea.GrabFocus();
            }
        }
    }
    else if ( pBtn == &aBtnRemove )
    {
        while ( aLbConsAreas.GetSelectEntryCount() )
            aLbConsAreas.RemoveEntry( aLbConsAreas.GetSelectEntryPos() );
        aBtnRemove.Disable();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, SelectHdl, ListBox*, pLb )
{
    if ( pLb == &aLbConsAreas )
    {
        if ( aLbConsAreas.GetSelectEntryCount() > 0 )
            aBtnRemove.Enable();
        else
            aBtnRemove.Disable();
    }
    else if ( (pLb == &aLbDataArea) || (pLb == &aLbDestArea) )
    {
        Edit*   pEd = (pLb == &aLbDataArea) ? &aEdDataArea : &aEdDestArea;
        sal_uInt16  nSelPos = pLb->GetSelectEntryPos();

        if (    pRangeUtil
            && (nSelPos > 0)
            && (nAreaDataCount > 0)
            && (pAreaData != NULL) )
        {
            if ( static_cast<size_t>(nSelPos) <= nAreaDataCount )
            {
                String aString( pAreaData[nSelPos-1].aStrArea );

                if ( pLb == &aLbDestArea )
                    pRangeUtil->CutPosString( aString, aString );

                pEd->SetText( aString );

                if ( pEd == &aEdDataArea )
                    aBtnAdd.Enable();
            }
        }
        else
        {
            pEd->SetText( EMPTY_STRING );
            if ( pEd == &aEdDataArea )
                aBtnAdd.Enable();
        }
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScConsolidateDlg, ModifyHdl, formula::RefEdit*, pEd )
{
    if ( pEd == &aEdDataArea )
    {
        String aAreaStr( pEd->GetText() );
        if ( aAreaStr.Len() > 0 )
        {
            aBtnAdd.Enable();
        }
        else
            aBtnAdd.Disable();
    }
    else if ( pEd == &aEdDestArea )
    {
        aLbDestArea.SelectEntryPos(0);
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
