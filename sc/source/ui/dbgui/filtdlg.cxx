/*************************************************************************
 *
 *  $RCSfile: filtdlg.cxx,v $
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
#include <rangelst.hxx>
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SV_WAITOBJ_HXX //autogen
#include <vcl/waitobj.hxx>
#endif

#include "uiitems.hxx"
#include "dbcolect.hxx"
#include "reffact.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"

#include "foptmgr.hxx"

#include "globstr.hrc"
#include "filter.hrc"

#define _FILTDLG_CXX
#include "filtdlg.hxx"
#undef _FILTDLG_CXX

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

// DEFINE --------------------------------------------------------------------

#define ERRORBOX(rid)   ErrorBox( this, WinBits( WB_OK|WB_DEF_OK), \
                                   ScGlobal::GetRscString(rid) ).Execute()


//============================================================================
//  class ScFilterDlg

//----------------------------------------------------------------------------

ScFilterDlg::ScFilterDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          const SfxItemSet& rArgSet )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_FILTER ),
        //
        _INIT_COMMON_FILTER_RSCOBJS
        aFtConnect      ( this, ScResId( FT_OP ) ),
        aFtField        ( this, ScResId( FT_FIELD ) ),
        aFtCond         ( this, ScResId( FT_COND ) ),
        aFtVal          ( this, ScResId( FT_VAL ) ),
        aLbField1       ( this, ScResId( LB_FIELD1 ) ),
        aLbField2       ( this, ScResId( LB_FIELD2 ) ),
        aLbField3       ( this, ScResId( LB_FIELD3 ) ),
        aLbConnect1     ( this, ScResId( LB_OP1 ) ),
        aLbConnect2     ( this, ScResId( LB_OP2 ) ),
        aLbCond1        ( this, ScResId( LB_COND1 ) ),
        aLbCond2        ( this, ScResId( LB_COND2 ) ),
        aLbCond3        ( this, ScResId( LB_COND3 ) ),
        aEdVal1         ( this, ScResId( ED_VAL1 ) ),
        aEdVal2         ( this, ScResId( ED_VAL2 ) ),
        aEdVal3         ( this, ScResId( ED_VAL3 ) ),
        aGbCriteria     ( this, ScResId( GB_CRITERIA ) ),
        aStrEmpty       ( ScResId( SCSTR_EMPTY ) ),
        aStrNotEmpty    ( ScResId( SCSTR_NOTEMPTY ) ),
        aStrRow         ( ScResId( SCSTR_ROW ) ),
        aStrColumn      ( ScResId( SCSTR_COLUMN ) ),
        //
        nWhichQuery     ( rArgSet.GetPool()->GetWhich( SID_QUERY ) ),
        theQueryData    ( ((const ScQueryItem&)
                           rArgSet.Get( nWhichQuery )).GetQueryData() ),
        nFieldCount     ( 0 ),
        pOutItem        ( NULL ),
        pOptionsMgr     ( NULL ),
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        pTimer          ( NULL ),
        bRefInputMode   ( FALSE ),
        nSrcTab         ( 0 )
{
    for (USHORT i=0; i<=MAXCOL; i++)
        pEntryLists[i] = NULL;

    Init( rArgSet );
    FreeResource();

    // Hack: RefInput-Kontrolle
    pTimer = new Timer;
    pTimer->SetTimeout( 50 ); // 50ms warten
    pTimer->SetTimeoutHdl( LINK( this, ScFilterDlg, TimeOutHdl ) );
}


//----------------------------------------------------------------------------

__EXPORT ScFilterDlg::~ScFilterDlg()
{
    for (USHORT i=0; i<=MAXCOL; i++)
        delete pEntryLists[i];

    delete pOptionsMgr;
    delete pOutItem;

    // Hack: RefInput-Kontrolle
    pTimer->Stop();
    delete pTimer;
}


//----------------------------------------------------------------------------

void __EXPORT ScFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = (const ScQueryItem&)
                                    rArgSet.Get( nWhichQuery );

    aBtnOk.SetClickHdl      ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScFilterDlg, EndDlgHdl ) );
    aBtnMore.SetClickHdl    ( LINK( this, ScFilterDlg, MoreClickHdl ) );
    aBtnHeader.SetClickHdl  ( LINK( this, ScFilterDlg, CheckBoxHdl ) );
    aBtnCase.SetClickHdl    ( LINK( this, ScFilterDlg, CheckBoxHdl ) );
    //
    aLbField1.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbField2.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbField3.SetSelectHdl  ( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbConnect1.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );
    aLbConnect2.SetSelectHdl( LINK( this, ScFilterDlg, LbSelectHdl ) );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : NULL;
    nSrcTab     = pViewData ? pViewData->GetTabNo() : NULL;

    // fuer leichteren Zugriff:
    aFieldLbArr  [0] = &aLbField1;
    aFieldLbArr  [1] = &aLbField2;
    aFieldLbArr  [2] = &aLbField3;
    aValueEdArr  [0] = &aEdVal1;
    aValueEdArr  [1] = &aEdVal2;
    aValueEdArr  [2] = &aEdVal3;
    aCondLbArr   [0] = &aLbCond1;
    aCondLbArr   [1] = &aLbCond2;
    aCondLbArr   [2] = &aLbCond3;

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

    // Feldlisten einlesen und Eintraege selektieren:

    FillFieldLists();

    for ( USHORT i=0; i<3; i++ )
    {
        String  aValStr;
        USHORT  nCondPos     = 0;
        USHORT  nFieldSelPos = 0;

        ScQueryEntry& rEntry = theQueryData.GetEntry(i);
        if ( rEntry.bDoQuery )
        {
            nCondPos     = (USHORT)rEntry.eOp;
            nFieldSelPos = GetFieldSelPos( rEntry.nField );

            if ( rEntry.nVal == SC_EMPTYFIELDS )
            {
                aValStr = aStrEmpty;
                aCondLbArr[i]->Disable();
            }
            else if ( rEntry.nVal == SC_NONEMPTYFIELDS )
            {
                aValStr = aStrNotEmpty;
                aCondLbArr[i]->Disable();
            }
            else
                aValStr = *rEntry.pStr;
        }
        else if ( i == 0 )
            nFieldSelPos = GetFieldSelPos( pViewData->GetCurX() );

        aFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
        aCondLbArr [i]->SelectEntryPos( nCondPos );
        aValueEdArr[i]->SetText( aValStr );
        aValueEdArr[i]->SetModifyHdl( LINK( this, ScFilterDlg, ValModifyHdl ) );
        UpdateValueList( i+1 );
    }

    // Disable/Enable Logik:

       (aLbField1.GetSelectEntryPos() != 0)
    && (aLbField2.GetSelectEntryPos() != 0)
        ? aLbConnect1.SelectEntryPos( (USHORT)theQueryData.GetEntry(1).eConnect )
        : aLbConnect1.SetNoSelection();

       (aLbField2.GetSelectEntryPos() != 0)
    && (aLbField3.GetSelectEntryPos() != 0)
        ? aLbConnect2.SelectEntryPos( (USHORT)theQueryData.GetEntry(2).eConnect )
        : aLbConnect2.SetNoSelection();

    if ( aLbField1.GetSelectEntryPos() == 0 )
    {
        aLbConnect1.Disable();
        aLbField2.Disable();
        aLbCond2.Disable();
        aEdVal2.Disable();
    }
    else if ( aLbConnect1.GetSelectEntryCount() == 0 )
    {
        aLbField2.Disable();
        aLbCond2.Disable();
        aEdVal2.Disable();
    }

    if ( aLbField2.GetSelectEntryPos() == 0 )
    {
        aLbConnect2.Disable();
        aLbField3.Disable();
        aLbCond3.Disable();
        aEdVal3.Disable();
    }
    else if ( aLbConnect2.GetSelectEntryCount() == 0 )
    {
        aLbField3.Disable();
        aLbCond3.Disable();
        aEdVal3.Disable();
    }

    if(pDoc!=NULL &&
        pDoc->GetChangeTrack()!=NULL) aBtnCopyResult.Disable();
    // Modal-Modus einschalten
//  SFX_APP()->LockDispatcher( TRUE );
    //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
//  SFX_APPWINDOW->Disable(FALSE);      //! allgemeine Methode im ScAnyRefDlg
}


//----------------------------------------------------------------------------

BOOL __EXPORT ScFilterDlg::Close()
{
    return DoClose( ScFilterDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
// neue Selektion im Referenz-Edit angezeigt wird.

void ScFilterDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( bRefInputMode )    // Nur moeglich, wenn im Referenz-Editmodus
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdCopyArea );
        String aRefStr;
        rRef.aStart.Format( aRefStr, SCA_ABS_3D, pDoc );
        aEdCopyArea.SetRefString( aRefStr );
    }
}


//----------------------------------------------------------------------------

void ScFilterDlg::SetActive()
{
    if ( bRefInputMode )
    {
        aEdCopyArea.GrabFocus();
        if ( aEdCopyArea.GetModifyHdl().IsSet() )
            ((Link&)aEdCopyArea.GetModifyHdl()).Call( &aEdCopyArea );
    }
    else
        GrabFocus();

    RefInputDone();
}

//----------------------------------------------------------------------------

void ScFilterDlg::FillFieldLists()
{
    aLbField1.Clear();
    aLbField2.Clear();
    aLbField3.Clear();
    aLbField1.InsertEntry( aStrNone, 0 );
    aLbField2.InsertEntry( aStrNone, 0 );
    aLbField3.InsertEntry( aStrNone, 0 );

    if ( pDoc )
    {
        String  aFieldName;
        USHORT  nTab        = nSrcTab;
        USHORT  nFirstCol   = theQueryData.nCol1;
        USHORT  nFirstRow   = theQueryData.nRow1;
        USHORT  nMaxCol     = theQueryData.nCol2;
        USHORT  col = 0;
        USHORT  i=1;

        for ( col=nFirstCol; col<=nMaxCol; col++ )
        {
            pDoc->GetString( col, nFirstRow, nTab, aFieldName );
            if ( !aBtnHeader.IsChecked() || (aFieldName.Len() == 0) )
            {
                aFieldName  = aStrColumn;
                aFieldName += ' ';
                if ( col < 26 )
                    aFieldName += (sal_Unicode)( 'A' + col );
                else
                {
                    aFieldName += (sal_Unicode)( 'A' + ( col / 26 ) - 1 );
                    aFieldName += (sal_Unicode)( 'A' + ( col % 26 ) );
                }
            }
            aLbField1.InsertEntry( aFieldName, i );
            aLbField2.InsertEntry( aFieldName, i );
            aLbField3.InsertEntry( aFieldName, i );
            i++;
        }
        nFieldCount = i;
    }
}


//----------------------------------------------------------------------------

void ScFilterDlg::UpdateValueList( USHORT nList )
{
    if ( pDoc && nList>0 && nList<=3 )
    {
        ComboBox*   pValList        = aValueEdArr[nList-1];
        USHORT      nFieldSelPos    = aFieldLbArr[nList-1]->GetSelectEntryPos();
        USHORT      nListPos        = 0;
        String      aCurValue       = pValList->GetText();

        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        nListPos = 2;

        if ( nFieldSelPos )
        {
            WaitObject aWaiter( this );     // auch wenn nur die ListBox gefuellt wird

            USHORT nColumn = theQueryData.nCol1 + nFieldSelPos - 1;
            if (!pEntryLists[nColumn])
            {
                USHORT nTab      = nSrcTab;
                USHORT nFirstRow = theQueryData.nRow1;
                USHORT nLastRow  = theQueryData.nRow2;

                //  erstmal ohne die erste Zeile

                pEntryLists[nColumn] = new TypedStrCollection( 128, 128 );
                pEntryLists[nColumn]->SetCaseSensitive( aBtnCase.IsChecked() );
                pDoc->GetFilterEntriesArea( nColumn, nFirstRow+1, nLastRow,
                                            nTab, *pEntryLists[nColumn] );

                //  Eintrag fuer die erste Zeile
                //! Eintrag (pHdrEntry) ohne Collection erzeugen?

                nHeaderPos[nColumn] = USHRT_MAX;
                TypedStrCollection aHdrColl( 1, 1 );
                pDoc->GetFilterEntriesArea( nColumn, nFirstRow, nFirstRow,
                                            nTab, aHdrColl );
                TypedStrData* pHdrEntry = aHdrColl[0];
                if ( pHdrEntry )
                {
                    TypedStrData* pNewEntry = new TypedStrData(*pHdrEntry);
                    if ( pEntryLists[nColumn]->Insert( pNewEntry ) )
                    {
                        nHeaderPos[nColumn] = pEntryLists[nColumn]->IndexOf( pNewEntry );
                        DBG_ASSERT( nHeaderPos[nColumn] != USHRT_MAX,
                                    "Header-Eintrag nicht wiedergefunden" );
                    }
                    else
                        delete pNewEntry;           // war schon drin
                }
            }

            TypedStrCollection* pColl = pEntryLists[nColumn];
            USHORT nValueCount = pColl->GetCount();
            if ( nValueCount > 0 )
            {
                for ( USHORT i=0; i<nValueCount; i++ )
                {
                    pValList->InsertEntry( (*pColl)[i]->GetString(), nListPos );
                    nListPos++;
                }
            }
        }
        pValList->SetText( aCurValue );
    }

    UpdateHdrInValueList( nList );
}

void ScFilterDlg::UpdateHdrInValueList( USHORT nList )
{
    //! GetText / SetText ??

    if ( pDoc && nList>0 && nList<=3 )
    {
        USHORT nFieldSelPos = aFieldLbArr[nList-1]->GetSelectEntryPos();
        if ( nFieldSelPos )
        {
            USHORT nColumn = theQueryData.nCol1 + nFieldSelPos - 1;
            if ( pEntryLists[nColumn] )
            {
                USHORT nPos = nHeaderPos[nColumn];
                if ( nPos != USHRT_MAX )
                {
                    ComboBox* pValList = aValueEdArr[nList-1];
                    USHORT nListPos = nPos + 2;                 // nach "leer" und "nicht leer"

                    TypedStrData* pHdrEntry = (*pEntryLists[nColumn])[nPos];
                    if ( pHdrEntry )
                    {
                        String aHdrStr = pHdrEntry->GetString();
                        BOOL bWasThere = ( pValList->GetEntry(nListPos) == aHdrStr );
                        BOOL bInclude = !aBtnHeader.IsChecked();

                        if (bInclude)           // Eintrag aufnehmen
                        {
                            if (!bWasThere)
                                pValList->InsertEntry(aHdrStr, nListPos);
                        }
                        else                    // Eintrag weglassen
                        {
                            if (bWasThere)
                                pValList->RemoveEntry(nListPos);
                        }
                    }
                    else
                        DBG_ERROR("Eintag in Liste nicht gefunden");
                }
            }
            else
                DBG_ERROR("Spalte noch nicht initialisiert");
        }
    }
}

//----------------------------------------------------------------------------

void ScFilterDlg::ClearValueList( USHORT nList )
{
    if ( nList>0 && nList<=3 )
    {
        ComboBox* pValList = aValueEdArr[nList-1];
        pValList->Clear();
        pValList->InsertEntry( aStrNotEmpty, 0 );
        pValList->InsertEntry( aStrEmpty, 1 );
        pValList->SetText( EMPTY_STRING );
    }
}


//----------------------------------------------------------------------------

USHORT ScFilterDlg::GetFieldSelPos( USHORT nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return nField - theQueryData.nCol1 + 1;
    else
        return 0;
}

//----------------------------------------------------------------------------

ScQueryItem* ScFilterDlg::GetOutputItem()
{
    ScAddress       theCopyPos;
    ScQueryParam    theParam( theQueryData );
    USHORT          nConnect1 = aLbConnect1.GetSelectEntryPos();
    USHORT          nConnect2 = aLbConnect2.GetSelectEntryPos();
    BOOL            bCopyPosOk;

    if ( aBtnCopyResult.IsChecked() )
    {
        String theCopyStr( aEdCopyArea.GetText() );
        xub_StrLen nColonPos = theCopyStr.Search( ':' );

        if ( STRING_NOTFOUND != nColonPos )
            theCopyStr.Erase( nColonPos );

        USHORT nResult = theCopyPos.Parse( theCopyStr, pDoc );
        bCopyPosOk = ( SCA_VALID == (nResult & SCA_VALID) );
    }

    for ( USHORT i=0; i<3; i++ )
    {
        USHORT      nField  = aFieldLbArr[i]->GetSelectEntryPos();
        ScQueryOp   eOp     = (ScQueryOp)aCondLbArr[i]->GetSelectEntryPos();

        BOOL bDoThis = (aFieldLbArr[i]->GetSelectEntryPos() != 0);
        theParam.GetEntry(i).bDoQuery = bDoThis;

        if ( bDoThis )
        {
            ScQueryEntry& rEntry = theParam.GetEntry(i);

            String aStrVal( aValueEdArr[i]->GetText() );

            /*
             * Dialog liefert die ausgezeichneten Feldwerte "leer"/"nicht leer"
             * als Konstanten in nVal in Verbindung mit dem Schalter
             * bQueryByString auf FALSE.
             */
            if ( aStrVal == aStrEmpty )
            {
                rEntry.pStr->Erase();
                rEntry.nVal = SC_EMPTYFIELDS;
                rEntry.bQueryByString = FALSE;
            }
            else if ( aStrVal == aStrNotEmpty )
            {
                rEntry.pStr->Erase();
                rEntry.nVal = SC_NONEMPTYFIELDS;
                rEntry.bQueryByString = FALSE;
            }
            else
            {
                *rEntry.pStr          = aStrVal;
                rEntry.nVal           = 0;
                rEntry.bQueryByString = TRUE;
            }

            rEntry.nField = nField ? ( theQueryData.nCol1 + nField - 1 ) : 0;
            rEntry.eOp    = eOp;
        }
    }

    theParam.GetEntry(1).eConnect = (nConnect1 != LISTBOX_ENTRY_NOTFOUND)
                                    ? (ScQueryConnect)nConnect1
                                    : SC_AND;
    theParam.GetEntry(2).eConnect = (nConnect2 != LISTBOX_ENTRY_NOTFOUND)
                                    ? (ScQueryConnect)nConnect2
                                    : SC_AND;

    if ( aBtnCopyResult.IsChecked() && bCopyPosOk )
    {
        theParam.bInplace   = FALSE;
        theParam.nDestTab   = theCopyPos.Tab();
        theParam.nDestCol   = theCopyPos.Col();
        theParam.nDestRow   = theCopyPos.Row();
    }
    else
    {
        theParam.bInplace   = TRUE;
        theParam.nDestTab   = 0;
        theParam.nDestCol   = 0;
        theParam.nDestRow   = 0;
    }

    theParam.bHasHeader     = aBtnHeader.IsChecked();
    theParam.bByRow         = TRUE;
    theParam.bDuplicate     = !aBtnUnique.IsChecked();
    theParam.bCaseSens      = aBtnCase.IsChecked();
    theParam.bRegExp        = aBtnRegExp.IsChecked();
    theParam.bDestPers      = aBtnDestPers.IsChecked();

    //  nur die drei eingestellten - alles andere zuruecksetzen

    USHORT nEC = theParam.GetEntryCount();
    for (i=3; i<nEC; i++)                               // alles ueber 3
        theParam.GetEntry(i).bDoQuery = FALSE;          // zuruecksetzen

    DELETEZ( pOutItem );
    pOutItem = new ScQueryItem( nWhichQuery, &theParam );

    return pOutItem;
}


//----------------------------------------------------------------------------

BOOL ScFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}


//----------------------------------------------------------------------------
// Handler:
// ========

IMPL_LINK( ScFilterDlg, EndDlgHdl, Button*, pBtn )
{
    if ( pBtn == &aBtnOk )
    {
        BOOL bAreaInputOk = TRUE;

        if ( aBtnCopyResult.IsChecked() )
        {
            if ( !pOptionsMgr->VerifyPosStr( aEdCopyArea.GetText() ) )
            {
                if ( !aBtnMore.GetState() )
                    aBtnMore.SetState( TRUE );

                ERRORBOX( STR_INVALID_TABREF );
                aEdCopyArea.GrabFocus();
                bAreaInputOk = FALSE;
            }
        }

        if ( bAreaInputOk )
        {
            SFX_APP()->LockDispatcher( FALSE );
            SwitchToDocument();
            GetBindings().GetDispatcher()->Execute( FID_FILTER_OK,
                                      SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
                                      GetOutputItem(), 0L, 0L );
            Close();
        }
    }
    else if ( pBtn == &aBtnCancel )
    {
        Close();
    }

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, MoreClickHdl, MoreButton*, pBtn )
{
    if ( aBtnMore.GetState() )
        pTimer->Start();
    else
    {
        pTimer->Stop();
        bRefInputMode = FALSE;
        //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
        //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
    }
    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, TimeOutHdl, Timer*, _pTimer )
{
    // alle 50ms nachschauen, ob RefInputMode noch stimmt

    if ( _pTimer == pTimer && IsActive() )
    {
        if ( aEdCopyArea.HasFocus() )
        {
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
                bRefInputMode = FALSE;
                //@BugID 54702 Enablen/Disablen nur noch in Basisklasse
                //SFX_APPWINDOW->Disable(FALSE);        //! allgemeine Methode im ScAnyRefDlg
            }
        }
    }

    if ( aBtnMore.GetState() )
        pTimer->Start();

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, LbSelectHdl, ListBox*, pLb )
{
    /*
     * Behandlung der Enable/Disable-Logik,
     * abhaengig davon, welche ListBox angefasst wurde:
     */

    if ( pLb == &aLbConnect1 )
    {
        if ( !aLbField2.IsEnabled() )
        {
            aLbField2.Enable();
            aLbCond2.Enable();
            aEdVal2.Enable();
        }
    }
    else if ( pLb == &aLbConnect2 )
    {
        if ( !aLbField3.IsEnabled() )
        {
            aLbField3.Enable();
            aLbCond3.Enable();
            aEdVal3.Enable();
        }
    }
    else if ( pLb == &aLbField1 )
    {
        if ( aLbField1.GetSelectEntryPos() == 0 )
        {
            aLbConnect1.SetNoSelection();
            aLbConnect2.SetNoSelection();
            aLbField2.SelectEntryPos( 0 );
            aLbField3.SelectEntryPos( 0 );
            aLbCond2.SelectEntryPos( 0 );
            aLbCond3.SelectEntryPos( 0 );
            ClearValueList( 1 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            aLbConnect1.Disable();
            aLbConnect2.Disable();
            aLbField2.Disable();
            aLbField3.Disable();
            aLbCond2.Disable();
            aLbCond3.Disable();
            aEdVal2.Disable();
            aEdVal3.Disable();
        }
        else
        {
            UpdateValueList( 1 );
            if ( !aLbConnect1.IsEnabled() )
            {
                aLbConnect1.Enable();
            }
        }
    }
    else if ( pLb == &aLbField2 )
    {
        if ( aLbField2.GetSelectEntryPos() == 0 )
        {
            aLbConnect2.SetNoSelection();
            aLbField3.SelectEntryPos( 0 );
            aLbCond3.SelectEntryPos( 0 );
            ClearValueList( 2 );
            ClearValueList( 3 );

            aLbConnect2.Disable();
            aLbField3.Disable();
            aLbCond3.Disable();
            aEdVal3.Disable();
        }
        else
        {
            UpdateValueList( 2 );
            if ( !aLbConnect2.IsEnabled() )
            {
                aLbConnect2.Enable();
            }
        }
    }
    else if ( pLb == &aLbField3 )
    {
        ( aLbField3.GetSelectEntryPos() == 0 )
            ? ClearValueList( 3 )
            : UpdateValueList( 3 );
    }

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, CheckBoxHdl, CheckBox*, pBox )
{
    //  Spaltenkoepfe:
    //      FeldListen: Spaltexx <-> Spaltenkopf-String
    //      WertListen: Spaltenkopf-Wert entfaellt.
    //  Gross-/Kleinschreibung:
    //      WertListen: komplett neu

    if ( pBox == &aBtnHeader )              // Feldlisten und Wertlisten
    {
        USHORT nCurSel1 = aLbField1.GetSelectEntryPos();
        USHORT nCurSel2 = aLbField2.GetSelectEntryPos();
        USHORT nCurSel3 = aLbField3.GetSelectEntryPos();
        FillFieldLists();
        aLbField1.SelectEntryPos( nCurSel1 );
        aLbField2.SelectEntryPos( nCurSel2 );
        aLbField3.SelectEntryPos( nCurSel3 );

        UpdateHdrInValueList( 1 );
        UpdateHdrInValueList( 2 );
        UpdateHdrInValueList( 3 );
    }

    if ( pBox == &aBtnCase )            // Wertlisten komplett
    {
        for (USHORT i=0; i<=MAXCOL; i++)
            DELETEZ( pEntryLists[i] );

        UpdateValueList( 1 );       // aktueller Text wird gemerkt
        UpdateValueList( 2 );
        UpdateValueList( 3 );
    }

    return NULL;
}


//----------------------------------------------------------------------------

IMPL_LINK( ScFilterDlg, ValModifyHdl, ComboBox*, pEd )
{
    if ( pEd )
    {
        String aStrVal  = pEd->GetText();
        ListBox* pLb    = &aLbCond1;

             if ( pEd == &aEdVal2 ) pLb = &aLbCond2;
        else if ( pEd == &aEdVal3 ) pLb = &aLbCond3;

        // wenn einer der Sonderwerte leer/nicht-leer
        // gewaehlt wird, so macht nur der =-Operator Sinn:

        if ( aStrEmpty == aStrVal || aStrNotEmpty == aStrVal )
        {
            pLb->SelectEntry( '=' );
            pLb->Disable();
        }
        else
            pLb->Enable();
    }

    return NULL;
}


