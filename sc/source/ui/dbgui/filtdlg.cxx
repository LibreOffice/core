/*************************************************************************
 *
 *  $RCSfile: filtdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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

#ifndef PCH
#include <segmentc.hxx>
#endif

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

SEG_EOFGLOBALS()


//============================================================================
//  class ScFilterDlg

//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(filtdlg_01)

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
#pragma SEG_FUNCDEF(filtdlg_02)

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
#pragma SEG_FUNCDEF(filtdlg_03)

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
#pragma SEG_FUNCDEF(filtdlg_04)

BOOL __EXPORT ScFilterDlg::Close()
{
    return DoClose( ScFilterDlgWrapper::GetChildWindowId() );
}


//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
// neue Selektion im Referenz-Edit angezeigt wird.

#pragma SEG_FUNCDEF(filtdlg_05)

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
#pragma SEG_FUNCDEF(filtdlg_06)

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

#pragma SEG_FUNCDEF(filtdlg_07)

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
#pragma SEG_FUNCDEF(filtdlg_08)

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

#pragma SEG_FUNCDEF(filtdlg_13)

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
#pragma SEG_FUNCDEF(filtdlg_10)

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
#pragma SEG_FUNCDEF(filtdlg_09)

USHORT ScFilterDlg::GetFieldSelPos( USHORT nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return nField - theQueryData.nCol1 + 1;
    else
        return 0;
}

//----------------------------------------------------------------------------
#pragma SEG_FUNCDEF(filtdlg_0a)

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
#pragma SEG_FUNCDEF(filtdlg_11)

BOOL ScFilterDlg::IsRefInputMode() const
{
    return bRefInputMode;
}


//----------------------------------------------------------------------------
// Handler:
// ========
#pragma SEG_FUNCDEF(filtdlg_0b)

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
            SFX_DISPATCHER().Execute( FID_FILTER_OK, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD,
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
#pragma SEG_FUNCDEF(filtdlg_0c)

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
#pragma SEG_FUNCDEF(filtdlg_0d)

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
#pragma SEG_FUNCDEF(filtdlg_0e)

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
#pragma SEG_FUNCDEF(filtdlg_0f)

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
#pragma SEG_FUNCDEF(filtdlg_12)

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


/*----------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.65  2000/09/17 14:08:56  willem.vandorp
    OpenOffice header added.

    Revision 1.64  2000/08/31 16:38:20  willem.vandorp
    Header and footer replaced

    Revision 1.63  2000/05/25 10:20:06  er
    NOOLDSV

    Revision 1.62  2000/04/14 17:38:02  nn
    unicode changes

    Revision 1.61  2000/02/11 12:23:25  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.60  2000/01/17 19:21:28  nn
    #67336# SwitchToDocument

    Revision 1.59  1999/06/09 16:09:04  NN
    Sfx-appwin removed


      Rev 1.58   09 Jun 1999 18:09:04   NN
   Sfx-appwin removed

      Rev 1.57   10 Dec 1998 20:09:18   ANK
   #51738# Umstellung auf ScRefEdit zum Anzeigen von Referenzen

      Rev 1.56   14 Sep 1998 20:58:06   ANK
   #55978# Ausgabebereich disablen bei Redlining

      Rev 1.55   02 Sep 1998 12:48:32   TJ
   include

      Rev 1.54   12 Aug 1998 19:47:32   ANK
   #54702# IsRefInputMode ueberarbeitet

      Rev 1.53   06 Aug 1998 21:18:30   ANK
   #54702# Enablen/Disablen der Applikation nur noch in ScAnyRefDlg

      Rev 1.52   15 Mar 1998 14:22:18   NN
   #48339# App-Fenster disablen mit bChild=FALSE

      Rev 1.51   02 Dec 1997 13:08:12   TJ
   include

      Rev 1.50   30 Sep 1997 13:07:42   TJ
   include

      Rev 1.49   03 Sep 1997 15:18:20   RG
   change header

      Rev 1.48   15 Feb 1997 17:36:26   NN
   Checkbox 'persistent' fuer Ausgabe-Bereich

      Rev 1.47   14 Feb 1997 19:14:28   ER
   aktuelle Spalte nur im ersten Eintrag

      Rev 1.46   14 Feb 1997 12:59:30   ER
   selektierte Spalte statt ersetm leeren Feldnamen

      Rev 1.45   06 Dec 1996 14:11:46   NN
   #33824# Header umschalten: nur betroffenen Eintrag loeschen/einfuegen

      Rev 1.44   29 Nov 1996 18:57:30   NN
   Gross-/Kleinschreibung auch in Werte-Liste

      Rev 1.43   27 Nov 1996 14:18:08   NN
   #31076# Entry-Listen pro Spalte nur einmal holen

      Rev 1.42   15 Nov 1996 17:22:44   NN
   #33345# Spaltenkoepfe-Button auswerten

      Rev 1.41   13 Nov 1996 20:11:16   NN
   #33143# AutoHide nur bei Bereichsreferenz

      Rev 1.40   13 Nov 1996 11:33:20   NN
   ScQueryParam mit dynamischen Eintraegen

      Rev 1.39   05 Nov 1996 14:49:58   NN
   ScApplication gibts nicht mehr

      Rev 1.38   29 Oct 1996 14:03:36   NN
   ueberall ScResId statt ResId

      Rev 1.37   22 Oct 1996 15:32:02   RJ
   Buttons fuer Referenzeingabe

      Rev 1.36   04 Oct 1996 17:22:46   RJ
   Einklappen des Fensters bei Referenzeingabe

      Rev 1.35   27 Jun 1996 11:49:28   NN
   Dispatcher::Execute Umstellung

      Rev 1.34   05 Jun 1996 19:19:10   NN
   kein Show im ctor (Position wird von aussen gesetzt)

      Rev 1.33   26 Apr 1996 12:02:40   NN
   SfxModelessDialog statt ModelessDialog

      Rev 1.32   25 Apr 1996 18:08:36   NN
   SetReference aufgeteilt in SetReference und AddRefEntry

      Rev 1.31   18 Jan 1996 11:44:42   MO
   #24232# leer/nicht-leer-Behandlung, neuer Link

      Rev 1.30   27 Nov 1995 13:42:12   MO
   RangeUtil/Area/Tripel gegen Address/Range ersetzt

      Rev 1.29   09 Nov 1995 10:20:12   JN
   weitere Verbesseungen Modeless Dialoge

      Rev 1.28   08 Nov 1995 13:05:48   MO
   301-Aenderungen

      Rev 1.27   08 Nov 1995 10:45:48   JN
   Umstellung auf SfxChildWindow

      Rev 1.26   26 Oct 1995 11:34:24   MO
   Default-Button-Bit bei Error/MessBoxen

      Rev 1.25   18 Sep 1995 10:07:48   MO
   bRefInput-Kontrolle ueber Timer

      Rev 1.24   14 Jul 1995 13:55:18   MO
   3. ValueList im Init fuellen (BugId: 15364)

      Rev 1.23   12 Jul 1995 16:57:48   MO
   TypedStrColloction fuer Wertklisten

      Rev 1.22   07 Jul 1995 17:36:36   MO
   Bugfix: Auswertung Connect-Parameter

      Rev 1.21   08 Jun 1995 16:22:06   MO
   leer/nicht-leer Filter

      Rev 1.20   25 Apr 1995 11:40:08   MO
   leer/nicht-leer Feldwerte

      Rev 1.19   24 Apr 1995 12:49:46   MO
   Dispatcher Unlock vor Execute

      Rev 1.18   18 Apr 1995 10:02:34   MO
   Referenzeingabe mit Maus ueberarbeitet

      Rev 1.17   02 Mar 1995 16:20:20   MO
   InfoBoxen mit Rsc-Strings

      Rev 1.16   26 Feb 1995 10:26:58   TRI
   basicide.hxx included

      Rev 1.15   22 Feb 1995 17:07:20   MO
   * Fehler bei der Rueckgabe der Verknuefungsoperationen behoben


      Rev 1.14   08 Feb 1995 12:55:02   MO
   * Dispatcher-Execute: 0L angehaengt (sonst uneindeutig)

      Rev 1.13   03 Feb 1995 12:43:02   MO
   * Ctor: Show()


      Rev 1.12   27 Jan 1995 16:14:44   MO
   * Umstellung aus Slot-IDs

      Rev 1.11   26 Jan 1995 19:00:18   TRI
   __EXPORT bei virtuellen Methoden eingebaut

      Rev 1.10   25 Jan 1995 18:53:56   MO
   * Auswertung der Connect-ListBoxen korrigiert

      Rev 1.9   25 Jan 1995 12:39:00   MO
   * Einzelne Zellposition als Kopierziel (SetReferenz und Ueberpruefungen)

      Rev 1.8   19 Jan 1995 16:47:48   TRI
   __EXPORT vor verschiedene LinkHandler gesetzt

      Rev 1.7   18 Jan 1995 13:56:26   TRI
   Pragmas zur Segementierung eingebaut

      Rev 1.6   17 Jan 1995 11:57:34   MO
   CheckBoxHdl fuer aBtnHeader wieder aktiviert

      Rev 1.5   16 Jan 1995 14:24:54   MO
   Fehlerbehandlung bei ungueltigen Bereichsnamen im EndDlg-Handler

      Rev 1.4   13 Jan 1995 17:30:34   MO
   Special-Dialog ausgegliedert

      Rev 1.3   13 Jan 1995 10:22:14   MO
   fuer Spezialdialog verwendbare Resource-Handler nach foptmgr verschoben

      Rev 1.2   12 Jan 1995 14:49:52   MO
   * Erkennung von DB-Bereichen
   * Gegenseitige Aktualisierung der CopyArea-ListBox/Edit
   * Einlesen von RangeNames mit RT_ABSAREA


      Rev 1.1   09 Jan 1995 11:14:52   MO
   Parameter werden jetzt per SfxItem (ScQueryItem) uebergeben,
   bzw. mit GetOutputItemSet() zurueckgegeben.

      Rev 1.0   05 Jan 1995 12:44:12   MO
   Initial revision.

----------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

