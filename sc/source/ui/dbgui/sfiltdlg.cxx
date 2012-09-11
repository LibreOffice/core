/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sfx2/dispatch.hxx>

#include "uiitems.hxx"
#include "rangenam.hxx"
#include "dbdata.hxx"
#include "reffact.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "scresid.hxx"

#include "foptmgr.hxx"

#include "globstr.hrc"
#include "filter.hrc"

#define _SFILTDLG_CXX
#include "filtdlg.hxx"
#undef _SFILTDLG_CXX
#include <vcl/msgbox.hxx>

// DEFINE --------------------------------------------------------------------

#define ERRORBOX(rid) ErrorBox( this, WinBits( WB_OK|WB_DEF_OK),\
                                    ScGlobal::GetRscString(rid) ).Execute()


//============================================================================
//  class ScSpecialFilterDialog

//----------------------------------------------------------------------------

ScSpecialFilterDlg::ScSpecialFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                        const SfxItemSet&   rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_SPEC_FILTER ),
        //
        aFtFilterArea   ( this, ScResId( FT_CRITERIA_AREA ) ),
        aLbFilterArea   ( this, ScResId( LB_CRITERIA_AREA ) ),
        aEdFilterArea   ( this, this, ScResId( ED_CRITERIA_AREA ) ),
        aRbFilterArea   ( this, ScResId( RB_CRITERIA_AREA ), &aEdFilterArea, this ),
        //
        aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
        aBtnCase        ( this, ScResId( BTN_CASE ) ),
        aBtnRegExp      ( this, ScResId( BTN_REGEXP ) ),
        aBtnHeader      ( this, ScResId( BTN_HEADER ) ),
        aBtnUnique      ( this, ScResId( BTN_UNIQUE ) ),
        aBtnCopyResult  ( this, ScResId( BTN_COPY_RESULT ) ),
        aLbCopyArea     ( this, ScResId( LB_COPY_AREA ) ),
        aEdCopyArea     ( this, this, ScResId( ED_COPY_AREA ) ),
        aRbCopyArea     ( this, ScResId( RB_COPY_AREA ), &aEdCopyArea, this ),
        aBtnDestPers    ( this, ScResId( BTN_DEST_PERS ) ),
        aFtDbAreaLabel  ( this, ScResId( FT_DBAREA_LABEL ) ),
        aFtDbArea       ( this, ScResId( FT_DBAREA ) ),
        aStrUndefined   ( SC_RESSTR(SCSTR_UNDEFINED) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),
        //
        pOptionsMgr     ( NULL ),
        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( ((const ScQueryItem&)
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        pOutItem        ( NULL ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        pRefInputEdit   ( NULL ),
        bRefInputMode   ( false ),
        pTimer          ( NULL )
{
    Init( rArgSet );
    aEdFilterArea.GrabFocus();

    FreeResource();

    // Hack: RefInput-Kontrolle
    pTimer = new Timer;
    pTimer->SetTimeout( 50 ); // 50ms warten
    pTimer->SetTimeoutHdl( LINK( this, ScSpecialFilterDlg, TimeOutHdl ) );
    pTimer->Start();

    aLbCopyArea.SetAccessibleName(aBtnCopyResult.GetText());
    aEdCopyArea.SetAccessibleName(aBtnCopyResult.GetText());
    aLbCopyArea.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
    aEdCopyArea.SetAccessibleRelationLabeledBy(&aBtnCopyResult);
}


//----------------------------------------------------------------------------

ScSpecialFilterDlg::~ScSpecialFilterDlg()
{
    sal_uInt16 nEntries = aLbFilterArea.GetEntryCount();
    sal_uInt16 i;

    for ( i=1; i<nEntries; i++ )
        delete (String*)aLbFilterArea.GetEntryData( i );

    delete pOptionsMgr;

    if ( pOutItem )
        delete pOutItem;

    // Hack: RefInput-Kontrolle
    pTimer->Stop();
    delete pTimer;
}


//----------------------------------------------------------------------------

void ScSpecialFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = (const ScQueryItem&)
                                    rArgSet.Get( nWhichQuery );

    aBtnOk.SetClickHdl          ( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    aBtnCancel.SetClickHdl      ( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    aLbFilterArea.SetSelectHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaSelHdl ) );
    aEdFilterArea.SetModifyHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaModHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument()  : NULL;

    aEdFilterArea.SetText( EMPTY_STRING );      // may be overwritten below

    if ( pViewData && pDoc )
    {
        if(pDoc->GetChangeTrack()!=NULL) aBtnCopyResult.Disable();

        ScRangeName* pRangeNames = pDoc->GetRangeName();
        aLbFilterArea.Clear();
        aLbFilterArea.InsertEntry( aStrUndefined, 0 );

        if (!pRangeNames->empty())
        {
            ScRangeName::const_iterator itr = pRangeNames->begin(), itrEnd = pRangeNames->end();
            sal_uInt16 nInsert = 0;
            for (; itr != itrEnd; ++itr)
            {
                if (!itr->second->HasType(RT_CRITERIA))
                    continue;

                nInsert = aLbFilterArea.InsertEntry(itr->second->GetName());
                rtl::OUString aSymbol;
                itr->second->GetSymbol(aSymbol);
                aLbFilterArea.SetEntryData(nInsert, new String(aSymbol));
            }
        }

        //  is there a stored source range?

        ScRange aAdvSource;
        if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
        {
            String aRefStr;
            aAdvSource.Format( aRefStr, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
            aEdFilterArea.SetRefString( aRefStr );
        }
    }

    aLbFilterArea.SelectEntryPos( 0 );

    // Optionen initialisieren lassen:

    pOptionsMgr  = new ScFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            aBtnMore,
                            aBtnCase,
                            aBtnRegExp,
                            aBtnHeader,
                            aBtnUnique,
                            aBtnCopyResult,
                            aBtnDestPers,
                            aLbCopyArea,
                            aEdCopyArea,
                            aRbCopyArea,
                            aFtDbAreaLabel,
                            aFtDbArea,
                            aFlOptions,
                            aStrUndefined );

    //  Spezialfilter braucht immer Spaltenkoepfe
    aBtnHeader.Check(true);
    aBtnHeader.Disable();

    // Modal-Modus einschalten
//  SetDispatcherLock( true );
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Disable(false);        //! allgemeine Methode im ScAnyRefDlg
}


//----------------------------------------------------------------------------

sal_Bool ScSpecialFilterDlg::Close()
{
    if (pViewData)
        pViewData->GetDocShell()->CancelAutoDBRange();

    return DoClose( ScSpecialFilterDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
// neue Selektion im Referenz-Edit angezeigt wird.

void ScSpecialFilterDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( bRefInputMode && pRefInputEdit )       // Nur moeglich, wenn im Referenz-Editmodus
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        String aRefStr;
        const formula::FormulaGrammar::AddressConvention eConv = pDocP->GetAddressConvention();

        if ( pRefInputEdit == &aEdCopyArea)
            rRef.aStart.Format( aRefStr, SCA_ABS_3D, pDocP, eConv );
        else if ( pRefInputEdit == &aEdFilterArea)
            rRef.Format( aRefStr, SCR_ABS_3D, pDocP, eConv );

        pRefInputEdit->SetRefString( aRefStr );
    }
}


//----------------------------------------------------------------------------

void ScSpecialFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        if ( pRefInputEdit == &aEdCopyArea )
        {
            aEdCopyArea.GrabFocus();
            if ( aEdCopyArea.GetModifyHdl().IsSet() )
                ((Link&)aEdCopyArea.GetModifyHdl()).Call( &aEdCopyArea );
        }
        else if ( pRefInputEdit == &aEdFilterArea )
        {
            aEdFilterArea.GrabFocus();
            FilterAreaModHdl( &aEdFilterArea );
        }
    }
    else
        GrabFocus();

    RefInputDone();
}


//----------------------------------------------------------------------------

ScQueryItem* ScSpecialFilterDlg::GetOutputItem( const ScQueryParam& rParam,
                                                const ScRange& rSource )
{
    if ( pOutItem ) DELETEZ( pOutItem );
    pOutItem = new ScQueryItem( nWhichQuery, &rParam );
    pOutItem->SetAdvancedQuerySource( &rSource );

    return pOutItem;
}


//----------------------------------------------------------------------------

sal_Bool ScSpecialFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}


//----------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScSpecialFilterDlg, EndDlgHdl, Button*, pBtn )
{
    OSL_ENSURE( pDoc && pViewData, "Document or ViewData not found. :-/" );

    if ( (pBtn == &aBtnOk) && pDoc && pViewData )
    {
        String          theCopyStr( aEdCopyArea.GetText() );
        String          theAreaStr( aEdFilterArea.GetText() );
        ScQueryParam    theOutParam( theQueryData );
        ScAddress       theAdrCopy;
        sal_Bool            bEditInputOk    = true;
        sal_Bool            bQueryOk        = false;
        ScRange         theFilterArea;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( aBtnCopyResult.IsChecked() )
        {
            xub_StrLen nColonPos = theCopyStr.Search( ':' );

            if ( STRING_NOTFOUND != nColonPos )
                theCopyStr.Erase( nColonPos );

            sal_uInt16 nResult = theAdrCopy.Parse( theCopyStr, pDoc, eConv );

            if ( SCA_VALID != (nResult & SCA_VALID) )
            {
                if ( !aBtnMore.GetState() )
                    aBtnMore.SetState( true );

                ERRORBOX( STR_INVALID_TABREF );
                aEdCopyArea.GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            sal_uInt16 nResult = ScRange().Parse( theAreaStr, pDoc, eConv );

            if ( SCA_VALID != (nResult & SCA_VALID) )
            {
                ERRORBOX( STR_INVALID_TABREF );
                aEdFilterArea.GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            /*
             * Alle Edit-Felder enthalten gueltige Bereiche.
             * Nun wird versucht aus dem Filterbereich
             * ein ScQueryParam zu erzeugen:
             */

            sal_uInt16  nResult = theFilterArea.Parse( theAreaStr, pDoc, eConv );

            if ( SCA_VALID == (nResult & SCA_VALID) )
            {
                ScAddress& rStart = theFilterArea.aStart;
                ScAddress& rEnd   = theFilterArea.aEnd;

                if ( aBtnCopyResult.IsChecked() )
                {
                    theOutParam.bInplace    = false;
                    theOutParam.nDestTab    = theAdrCopy.Tab();
                    theOutParam.nDestCol    = theAdrCopy.Col();
                    theOutParam.nDestRow    = theAdrCopy.Row();
                }
                else
                {
                    theOutParam.bInplace    = true;
                    theOutParam.nDestTab    = 0;
                    theOutParam.nDestCol    = 0;
                    theOutParam.nDestRow    = 0;
                }

                theOutParam.bHasHeader = aBtnHeader.IsChecked();
                theOutParam.bByRow     = true;
                theOutParam.bCaseSens  = aBtnCase.IsChecked();
                theOutParam.bRegExp    = aBtnRegExp.IsChecked();
                theOutParam.bDuplicate = !aBtnUnique.IsChecked();
                theOutParam.bDestPers  = aBtnDestPers.IsChecked();

                bQueryOk =
                    pDoc->CreateQueryParam( rStart.Col(),
                                            rStart.Row(),
                                            rEnd.Col(),
                                            rEnd.Row(),
                                            rStart.Tab(),
                                            theOutParam );
            }
        }

        if ( bQueryOk )
        {
            SetDispatcherLock( false );
            SwitchToDocument();
            GetBindings().GetDispatcher()->Execute( FID_FILTER_OK,
                                      SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      GetOutputItem( theOutParam, theFilterArea ), 0L, 0L );
            Close();
        }
        else
        {
            ERRORBOX( STR_INVALID_QUERYAREA );
            aEdFilterArea.GrabFocus();
        }
    }
    else if ( pBtn == &aBtnCancel )
    {
        Close();
    }
    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, TimeOutHdl, Timer*, _pTimer )
{
    // alle 50ms nachschauen, ob RefInputMode noch stimmt

    if( (_pTimer == pTimer) && IsActive() )
    {
        if( aEdCopyArea.HasFocus() || aRbCopyArea.HasFocus() )
        {
            pRefInputEdit = &aEdCopyArea;
            bRefInputMode = true;
        }
        else if( aEdFilterArea.HasFocus() || aRbFilterArea.HasFocus() )
        {
            pRefInputEdit = &aEdFilterArea;
            bRefInputMode = true;
        }
        else if( bRefInputMode )
        {
            pRefInputEdit = NULL;
            bRefInputMode = false;
        }
    }

    pTimer->Start();

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, FilterAreaSelHdl, ListBox*, pLb )
{
    if ( pLb == &aLbFilterArea )
    {
        String  aString;
        sal_uInt16  nSelPos = aLbFilterArea.GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)aLbFilterArea.GetEntryData( nSelPos );

        aEdFilterArea.SetText( aString );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, FilterAreaModHdl, formula::RefEdit*, pEd )
{
    if ( pEd == &aEdFilterArea )
    {
        if ( pDoc && pViewData )
        {
            String  theCurAreaStr = pEd->GetText();
            sal_uInt16  nResult = ScRange().Parse( theCurAreaStr, pDoc );

            if ( SCA_VALID == (nResult & SCA_VALID) )
            {
                String* pStr    = NULL;
                sal_Bool    bFound  = false;
                sal_uInt16  i       = 0;
                sal_uInt16  nCount  = aLbFilterArea.GetEntryCount();

                for ( i=1; i<nCount && !bFound; i++ )
                {
                    pStr = (String*)aLbFilterArea.GetEntryData( i );
                    bFound = (theCurAreaStr == *pStr);
                }

                if ( bFound )
                    aLbFilterArea.SelectEntryPos( --i );
                else
                    aLbFilterArea.SelectEntryPos( 0 );
            }
        }
        else
            aLbFilterArea.SelectEntryPos( 0 );
    }

    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
