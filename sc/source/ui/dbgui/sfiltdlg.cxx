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

    :   ScAnyRefDlg ( pB, pCW, pParent, "AdvancedFilterDialog", "modules/scalc/ui/advancedfilterdialog.ui" ),
        //
        //
        aStrUndefined   ( SC_RESSTR(SCSTR_UNDEFINED) ),
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
        get(pLbFilterArea,"lbfilterarea");
        get(pEdFilterArea,"edfilterarea");
        get(pRbFilterArea,"rbfilterarea");
        pRbFilterArea->SetReferences(this, pEdFilterArea);
        get(pBtnCase,"case");
        get(pBtnRegExp,"regexp");
        get(pBtnHeader,"header");
        get(pBtnUnique,"unique");
        get(pBtnCopyResult,"copyresult");
        get(pLbCopyArea,"lbcopyarea");
        get(pEdCopyArea,"edcopyarea");
        get(pRbCopyArea,"rbcopyarea");
        pRbCopyArea->SetReferences(this, pEdCopyArea);
        get(pBtnDestPers,"destpers");
        get(pFtDbAreaLabel,"dbarealabel");
        get(pFtDbArea,"dbarea");
        get(pBtnOk,"ok");
        get(pBtnCancel,"cancel");
        get(pExpander,"more");

    Init( rArgSet );
    pEdFilterArea->GrabFocus();

    // Hack: RefInput-Kontrolle
    pTimer = new Timer;
    pTimer->SetTimeout( 50 ); // 50ms warten
    pTimer->SetTimeoutHdl( LINK( this, ScSpecialFilterDlg, TimeOutHdl ) );
    pTimer->Start();

    pLbCopyArea->SetAccessibleName(pBtnCopyResult->GetText());
    pEdCopyArea->SetAccessibleName(pBtnCopyResult->GetText());
}


//----------------------------------------------------------------------------

ScSpecialFilterDlg::~ScSpecialFilterDlg()
{
    sal_uInt16 nEntries = pLbFilterArea->GetEntryCount();
    sal_uInt16 i;

    for ( i=1; i<nEntries; i++ )
        delete (String*)pLbFilterArea->GetEntryData( i );

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

    pBtnOk->SetClickHdl          ( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    pBtnCancel->SetClickHdl      ( LINK( this, ScSpecialFilterDlg, EndDlgHdl ) );
    pLbFilterArea->SetSelectHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaSelHdl ) );
    pEdFilterArea->SetModifyHdl  ( LINK( this, ScSpecialFilterDlg, FilterAreaModHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument()  : NULL;

    pEdFilterArea->SetText( EMPTY_STRING );      // may be overwritten below

    if ( pViewData && pDoc )
    {
        if(pDoc->GetChangeTrack()!=NULL) pBtnCopyResult->Disable();

        ScRangeName* pRangeNames = pDoc->GetRangeName();
        pLbFilterArea->Clear();
        pLbFilterArea->InsertEntry( aStrUndefined, 0 );

        if (!pRangeNames->empty())
        {
            ScRangeName::const_iterator itr = pRangeNames->begin(), itrEnd = pRangeNames->end();
            sal_uInt16 nInsert = 0;
            for (; itr != itrEnd; ++itr)
            {
                if (!itr->second->HasType(RT_CRITERIA))
                    continue;

                nInsert = pLbFilterArea->InsertEntry(itr->second->GetName());
                OUString aSymbol;
                itr->second->GetSymbol(aSymbol);
                pLbFilterArea->SetEntryData(nInsert, new String(aSymbol));
            }
        }

        //  is there a stored source range?

        ScRange aAdvSource;
        if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
        {
            String aRefStr;
            aAdvSource.Format( aRefStr, SCR_ABS_3D, pDoc, pDoc->GetAddressConvention() );
            pEdFilterArea->SetRefString( aRefStr );
        }
    }

    pLbFilterArea->SelectEntryPos( 0 );

    // Optionen initialisieren lassen:

    pOptionsMgr  = new ScNewFilterOptionsMgr(
                            pViewData,
                            theQueryData,
                            pBtnCase,
                            pBtnRegExp,
                            pBtnHeader,
                            pBtnUnique,
                            pBtnCopyResult,
                            pBtnDestPers,
                            pLbCopyArea,
                            pEdCopyArea,
                            pRbCopyArea,
                            pFtDbAreaLabel,
                            pFtDbArea,
                            aStrUndefined );

    //  Spezialfilter braucht immer Spaltenkoepfe
    pBtnHeader->Check(true);
    pBtnHeader->Disable();

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

        if ( pRefInputEdit == pEdCopyArea)
            rRef.aStart.Format( aRefStr, SCA_ABS_3D, pDocP, eConv );
        else if ( pRefInputEdit == pEdFilterArea)
            rRef.Format( aRefStr, SCR_ABS_3D, pDocP, eConv );

        pRefInputEdit->SetRefString( aRefStr );
    }
}


//----------------------------------------------------------------------------

void ScSpecialFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        if ( pRefInputEdit == pEdCopyArea )
        {
            pEdCopyArea->GrabFocus();
            if ( pEdCopyArea->GetModifyHdl().IsSet() )
                ((Link&)pEdCopyArea->GetModifyHdl()).Call( pEdCopyArea );
        }
        else if ( pRefInputEdit == pEdFilterArea )
        {
            pEdFilterArea->GrabFocus();
            FilterAreaModHdl( pEdFilterArea );
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

    if ( (pBtn == pBtnOk) && pDoc && pViewData )
    {
        String          theCopyStr( pEdCopyArea->GetText() );
        String          theAreaStr( pEdFilterArea->GetText() );
        ScQueryParam    theOutParam( theQueryData );
        ScAddress       theAdrCopy;
        sal_Bool            bEditInputOk    = true;
        sal_Bool            bQueryOk        = false;
        ScRange         theFilterArea;
        const formula::FormulaGrammar::AddressConvention eConv = pDoc->GetAddressConvention();

        if ( pBtnCopyResult->IsChecked() )
        {
            xub_StrLen nColonPos = theCopyStr.Search( ':' );

            if ( STRING_NOTFOUND != nColonPos )
                theCopyStr.Erase( nColonPos );

            sal_uInt16 nResult = theAdrCopy.Parse( theCopyStr, pDoc, eConv );

            if ( SCA_VALID != (nResult & SCA_VALID) )
            {
                /*if (!pExpander->get_expanded())
                    pExpander->set_expanded(true);*/

                ERRORBOX( STR_INVALID_TABREF );
                pEdCopyArea->GrabFocus();
                bEditInputOk = false;
            }
        }

        if ( bEditInputOk )
        {
            sal_uInt16 nResult = ScRange().Parse( theAreaStr, pDoc, eConv );

            if ( SCA_VALID != (nResult & SCA_VALID) )
            {
                ERRORBOX( STR_INVALID_TABREF );
                pEdFilterArea->GrabFocus();
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

                if ( pBtnCopyResult->IsChecked() )
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

                theOutParam.bHasHeader = pBtnHeader->IsChecked();
                theOutParam.bByRow     = true;
                theOutParam.bCaseSens  = pBtnCase->IsChecked();
                theOutParam.bRegExp    = pBtnRegExp->IsChecked();
                theOutParam.bDuplicate = !pBtnUnique->IsChecked();
                theOutParam.bDestPers  = pBtnDestPers->IsChecked();

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
            pEdFilterArea->GrabFocus();
        }
    }
    else if ( pBtn == pBtnCancel )
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
        if( pEdCopyArea->HasFocus() || pRbCopyArea->HasFocus() )
        {
            pRefInputEdit = pEdCopyArea;
            bRefInputMode = true;
        }
        else if( pEdFilterArea->HasFocus() || pRbFilterArea->HasFocus() )
        {
            pRefInputEdit = pEdFilterArea;
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
    if ( pLb == pLbFilterArea )
    {
        String  aString;
        sal_uInt16  nSelPos = pLbFilterArea->GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)pLbFilterArea->GetEntryData( nSelPos );

        pEdFilterArea->SetText( aString );
    }

    return 0;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, FilterAreaModHdl, formula::RefEdit*, pEd )
{
    if ( pEd == pEdFilterArea )
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
                sal_uInt16  nCount  = pLbFilterArea->GetEntryCount();

                for ( i=1; i<nCount && !bFound; i++ )
                {
                    pStr = (String*)pLbFilterArea->GetEntryData( i );
                    bFound = (theCurAreaStr == *pStr);
                }

                if ( bFound )
                    pLbFilterArea->SelectEntryPos( --i );
                else
                    pLbFilterArea->SelectEntryPos( 0 );
            }
        }
        else
            pLbFilterArea->SelectEntryPos( 0 );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
