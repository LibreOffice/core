/*************************************************************************
 *
 *  $RCSfile: sfiltdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 18:48:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE -------------------------------------------------------------------

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#include "uiitems.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "reffact.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"

#include "foptmgr.hxx"

#include "globstr.hrc"
#include "filter.hrc"

#define _SFILTDLG_CXX
#include "filtdlg.hxx"
#undef _SFILTDLG_CXX

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

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
        _INIT_COMMON_FILTER_RSCOBJS
        //
        aLbFilterArea   ( this, ScResId( LB_CRITERIA_AREA ) ),
        aFtFilterArea   ( this, ScResId( FT_CRITERIA_AREA ) ),
        aEdFilterArea   ( this, ScResId( ED_CRITERIA_AREA ) ),
        aRbFilterArea   ( this, ScResId( RB_CRITERIA_AREA ), &aEdFilterArea ),
        //
        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( ((const ScQueryItem&)
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        pOutItem        ( NULL ),
        pOptionsMgr     ( NULL ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        pRefInputEdit   ( NULL ),
        bRefInputMode   ( FALSE ),
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
}


//----------------------------------------------------------------------------

__EXPORT ScSpecialFilterDlg::~ScSpecialFilterDlg()
{
    USHORT nEntries = aLbFilterArea.GetEntryCount();
    USHORT i;

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

void __EXPORT ScSpecialFilterDlg::Init( const SfxItemSet& rArgSet )
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

        ScRangeName*    pRangeNames = pDoc->GetRangeName();
        const USHORT    nCount      = pRangeNames ? pRangeNames->GetCount() : 0;

        /*
         * Aus den RangeNames des Dokumentes werden nun die
         * gemerkt, bei denen es sich um Filter-Bereiche handelt
         */

        aLbFilterArea.Clear();
        aLbFilterArea.InsertEntry( aStrUndefined, 0 );

        if ( nCount > 0 )
        {
            String       aString;
            ScRangeData* pData = NULL;
            USHORT       nInsert = 0;

            for ( USHORT i=0; i<nCount; i++ )
            {
                pData = (ScRangeData*)(pRangeNames->At( i ));
                if ( pData )
                {
                    if ( pData->HasType( RT_CRITERIA ) )
                    {
                        pData->GetName( aString );
                        nInsert = aLbFilterArea.InsertEntry( aString );
                        pData->GetSymbol( aString );
                        aLbFilterArea.SetEntryData( nInsert,
                                                    new String( aString ) );
                    }
                }
            }
        }

        //  is there a stored source range?

        ScRange aAdvSource;
        if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
        {
            String aRefStr;
            aAdvSource.Format( aRefStr, SCR_ABS_3D, pDoc );
            aEdFilterArea.SetRefString( aRefStr );
        }
    }

    aLbFilterArea.SelectEntryPos( 0 );

    // Optionen initialisieren lassen:

    pOptionsMgr  = new ScFilterOptionsMgr(
                            this,
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
                            aGbOptions,
                            aStrNoName,
                            aStrUndefined );

    //  #35206# Spezialfilter braucht immer Spaltenkoepfe
    aBtnHeader.Check(TRUE);
    aBtnHeader.Disable();

    // Modal-Modus einschalten
//  SFX_APP()->LockDispatcher( TRUE );
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
    //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScSpecialFilterDlg::Close()
{
    return DoClose( ScSpecialFilterDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
// neue Selektion im Referenz-Edit angezeigt wird.

void ScSpecialFilterDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( bRefInputMode && pRefInputEdit )       // Nur moeglich, wenn im Referenz-Editmodus
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( pRefInputEdit );

        String aRefStr;

        if ( pRefInputEdit == &aEdCopyArea)
            rRef.aStart.Format( aRefStr, SCA_ABS_3D, pDoc );
        else if ( pRefInputEdit == &aEdFilterArea)
            rRef.Format( aRefStr, SCR_ABS_3D, pDoc );

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

BOOL ScSpecialFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}


//----------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScSpecialFilterDlg, EndDlgHdl, Button*, pBtn )
{
    DBG_ASSERT( pDoc && pViewData, "Document or ViewData not found. :-/" );

    if ( (pBtn == &aBtnOk) && pDoc && pViewData )
    {
        String          theCopyStr( aEdCopyArea.GetText() );
        String          theAreaStr( aEdFilterArea.GetText() );
        ScQueryParam    theOutParam( theQueryData );
        ScAddress       theAdrCopy;
        BOOL            bEditInputOk    = TRUE;
        BOOL            bQueryOk        = FALSE;
        USHORT          nCurTab         = pViewData->GetTabNo();
        ScRange         theFilterArea;

        if ( aBtnCopyResult.IsChecked() )
        {
            xub_StrLen nColonPos = theCopyStr.Search( ':' );

            if ( STRING_NOTFOUND != nColonPos )
                theCopyStr.Erase( nColonPos );

            USHORT nResult = theAdrCopy.Parse( theCopyStr, pDoc );

            if ( SCA_VALID != (nResult & SCA_VALID) )
            {
                if ( !aBtnMore.GetState() )
                    aBtnMore.SetState( TRUE );

                ERRORBOX( STR_INVALID_TABREF );
                aEdCopyArea.GrabFocus();
                bEditInputOk = FALSE;
            }
        }

        if ( bEditInputOk )
        {
            USHORT nResult = ScRange().Parse( theAreaStr, pDoc );

            if ( SCA_VALID != (nResult & SCA_VALID) )
            {
                ERRORBOX( STR_INVALID_TABREF );
                aEdFilterArea.GrabFocus();
                bEditInputOk = FALSE;
            }
        }

        if ( bEditInputOk )
        {
            /*
             * Alle Edit-Felder enthalten gueltige Bereiche.
             * Nun wird versucht aus dem Filterbereich
             * ein ScQueryParam zu erzeugen:
             */

            USHORT  nResult = theFilterArea.Parse( theAreaStr, pDoc );

            if ( SCA_VALID == (nResult & SCA_VALID) )
            {
                ScAddress& rStart = theFilterArea.aStart;
                ScAddress& rEnd   = theFilterArea.aEnd;

                if ( aBtnCopyResult.IsChecked() )
                {
                    theOutParam.bInplace    = FALSE;
                    theOutParam.nDestTab    = theAdrCopy.Tab();
                    theOutParam.nDestCol    = theAdrCopy.Col();
                    theOutParam.nDestRow    = theAdrCopy.Row();
                }
                else
                {
                    theOutParam.bInplace    = TRUE;
                    theOutParam.nDestTab    =
                    theOutParam.nDestCol    =
                    theOutParam.nDestRow    = 0;
                }

                theOutParam.bHasHeader = aBtnHeader.IsChecked();
                theOutParam.bByRow     = TRUE;
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

                //  an der DB-Collection koennen nur MAXQUERY Filter-Eintraege
                //  gespeichert werden

                if ( bQueryOk && theOutParam.GetEntryCount() > MAXQUERY &&
                     theOutParam.GetEntry(MAXQUERY).bDoQuery )
                {
                    bQueryOk = FALSE;       // zu viele
                                            //! andere Fehlermeldung ??
                }
            }
        }

        if ( bQueryOk )
        {
            SFX_APP()->LockDispatcher( FALSE );
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

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, TimeOutHdl, Timer*, _pTimer )
{
    // alle 50ms nachschauen, ob RefInputMode noch stimmt

    if ( _pTimer == pTimer && IsActive() )
    {
        if ( aEdCopyArea.HasFocus() || aEdFilterArea.HasFocus() )
        {
            pRefInputEdit = aEdCopyArea.HasFocus()
                             ? &aEdCopyArea
                             : &aEdFilterArea;

            if ( !bRefInputMode )
            {

                bRefInputMode = TRUE;
                //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
                //SFX_APPWINDOW->Enable(); // Mauseingabe im Tabellenfenster zulassen
            }
        }
        else
        {
            if ( bRefInputMode )
            {
                pRefInputEdit = NULL;
                bRefInputMode = FALSE;
                //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
                //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
            }
        }
    }

    pTimer->Start();

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, FilterAreaSelHdl, ListBox*, pLb )
{
    if ( pLb == &aLbFilterArea )
    {
        String  aString;
        USHORT  nSelPos = aLbFilterArea.GetSelectEntryPos();

        if ( nSelPos > 0 )
            aString = *(String*)aLbFilterArea.GetEntryData( nSelPos );

        aEdFilterArea.SetText( aString );
    }

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScSpecialFilterDlg, FilterAreaModHdl, ScRefEdit*, pEd )
{
    if ( pEd == &aEdFilterArea )
    {
        if ( pDoc && pViewData )
        {
            String  theCurAreaStr = pEd->GetText();
            USHORT  nResult = ScRange().Parse( theCurAreaStr, pDoc );

            if ( SCA_VALID == (nResult & SCA_VALID) )
            {
                String* pStr    = NULL;
                BOOL    bFound  = FALSE;
                USHORT  i       = 0;
                USHORT  nCount  = aLbFilterArea.GetEntryCount();

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

    return NULL;
}


