/*************************************************************************
 *
 *  $RCSfile: pfiltdlg.cxx,v $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <vcl/waitobj.hxx>
#include <segmentc.hxx>
#endif

// INCLUDE ---------------------------------------------------------------

#include "viewdata.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "global.hxx"
#include "dbcolect.hxx"
#include "scresid.hxx"

#include "sc.hrc"
#include "filter.hrc"
#include "globstr.hrc"

#define _PFILTDLG_CXX
#include "pfiltdlg.hxx"
#undef _PFILTDLG_CXX

SEG_EOFGLOBALS()

//==================================================================
#pragma SEG_FUNCDEF(pfiltdlg_01)

ScPivotFilterDlg::ScPivotFilterDlg( Window*             pParent,
                                    const SfxItemSet&   rArgSet,
                                    USHORT              nSourceTab )

    :   ModalDialog ( pParent, ScResId( RID_SCDLG_PIVOTFILTER ) ),
        //
        aBtnCase        ( this, ScResId( BTN_CASE ) ),
        aBtnRegExp      ( this, ScResId( BTN_REGEXP ) ),
        aBtnUnique      ( this, ScResId( BTN_UNIQUE ) ),
        aFtDbAreaLabel  ( this, ScResId( FT_DBAREA_LABEL ) ),
        aFtDbArea       ( this, ScResId( FT_DBAREA ) ),
        aGbOptions      ( this, ScResId( GB_OPTIONS ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aBtnMore        ( this, ScResId( BTN_MORE ) ),
        aStrNoName      ( ScGlobal::GetRscString(STR_DB_NONAME) ),
        aStrNone        ( ScResId( SCSTR_NONE ) ),
        aStrUndefined   ( ScResId( SCSTR_UNDEFINED ) ),
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
        pViewData       ( NULL ),
        pDoc            ( NULL ),
        nSrcTab         ( nSourceTab )      // ist nicht im QueryParam
{
    for (USHORT i=0; i<=MAXCOL; i++)
        pEntryLists[i] = NULL;

    Init( rArgSet );
    FreeResource();
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_02)

__EXPORT ScPivotFilterDlg::~ScPivotFilterDlg()
{
    for (USHORT i=0; i<=MAXCOL; i++)
        delete pEntryLists[i];

    if ( pOutItem )
        delete pOutItem;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_03)

void __EXPORT ScPivotFilterDlg::Init( const SfxItemSet& rArgSet )
{
    const ScQueryItem& rQueryItem = (const ScQueryItem&)
                                    rArgSet.Get( nWhichQuery );

    aBtnCase.SetClickHdl    ( LINK( this, ScPivotFilterDlg, CheckBoxHdl ) );

    aLbField1.SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbField2.SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbField3.SetSelectHdl  ( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbConnect1.SetSelectHdl( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );
    aLbConnect2.SetSelectHdl( LINK( this, ScPivotFilterDlg, LbSelectHdl ) );

    aBtnMore.AddWindow( &aBtnCase );
    aBtnMore.AddWindow( &aBtnRegExp );
    aBtnMore.AddWindow( &aBtnUnique );
    aBtnMore.AddWindow( &aFtDbAreaLabel );
    aBtnMore.AddWindow( &aFtDbArea );
    aBtnMore.AddWindow( &aGbOptions );

    aBtnCase    .Check( theQueryData.bCaseSens );
    aBtnRegExp  .Check( theQueryData.bRegExp );
    aBtnUnique  .Check( !theQueryData.bDuplicate );

    pViewData   = rQueryItem.GetViewData();
    pDoc        = pViewData ? pViewData->GetDocument() : NULL;

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

    if ( pViewData && pDoc )
    {
        ScRangeName*    pRangeNames = pDoc->GetRangeName();
        String          theAreaStr;
        ScRange         theCurArea ( ScAddress( theQueryData.nCol1,
                                                theQueryData.nRow1,
                                                nSrcTab ),
                                     ScAddress( theQueryData.nCol2,
                                                theQueryData.nRow2,
                                                nSrcTab ) );
        ScDBCollection* pDBColl     = pDoc->GetDBCollection();
        String          theDbArea;
        String          theDbName   = aStrNoName;

        /*
         * Ueberpruefen, ob es sich bei dem uebergebenen
         * Bereich um einen Datenbankbereich handelt:
         */

        theCurArea.Format( theAreaStr, SCR_ABS_3D, pDoc );

        if ( pDBColl )
        {
            ScAddress&  rStart  = theCurArea.aStart;
            ScAddress&  rEnd    = theCurArea.aEnd;
            ScDBData*   pDBData = pDBColl->GetDBAtArea( rStart.Tab(),
                                                        rStart.Col(), rStart.Row(),
                                                        rEnd.Col(),   rEnd.Row() );
            if ( pDBData )
                pDBData->GetName( theDbName );
        }

        theDbArea.AppendAscii(RTL_CONSTASCII_STRINGPARAM(" ("));
        theDbArea += theDbName;
        theDbArea += ')';
        aFtDbArea.SetText( theDbArea );
    }
    else
    {
        aFtDbArea.SetText( EMPTY_STRING );
    }

    // Feldlisten einlesen und Eintraege selektieren:

    FillFieldLists();

    for ( USHORT i=0; i<3; i++ )
    {
        if ( theQueryData.GetEntry(i).bDoQuery )
        {
            ScQueryEntry& rEntry = theQueryData.GetEntry(i);

            String  aValStr      = *rEntry.pStr;
            USHORT  nCondPos     = (USHORT)rEntry.eOp;
            USHORT  nFieldSelPos = GetFieldSelPos( rEntry.nField );

            aFieldLbArr[i]->SelectEntryPos( nFieldSelPos );
            aCondLbArr [i]->SelectEntryPos( nCondPos );
            UpdateValueList( i+1 );
            aValueEdArr[i]->SetText( aValStr );
        }
        else
        {
            aFieldLbArr[i]->SelectEntryPos( 0 ); // "keiner" selektieren
            aCondLbArr [i]->SelectEntryPos( 0 ); // "=" selektieren
            UpdateValueList( i );
            aValueEdArr[i]->SetText( EMPTY_STRING );
        }
        aValueEdArr[i]->SetModifyHdl( LINK( this, ScPivotFilterDlg, ValModifyHdl ) );
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
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_04)

void ScPivotFilterDlg::FillFieldLists()
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
            if ( aFieldName.Len() == 0 )
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

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_05)

void ScPivotFilterDlg::UpdateValueList( USHORT nList )
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

        if ( pDoc && nFieldSelPos )
        {
            USHORT nColumn = theQueryData.nCol1 + nFieldSelPos - 1;
            if (!pEntryLists[nColumn])
            {
                WaitObject aWaiter( this );

                USHORT  nTab        = nSrcTab;
                USHORT  nFirstRow   = theQueryData.nRow1;
                USHORT  nLastRow    = theQueryData.nRow2;
                nFirstRow++;

                pEntryLists[nColumn] = new TypedStrCollection( 128, 128 );
                pEntryLists[nColumn]->SetCaseSensitive( aBtnCase.IsChecked() );
                pDoc->GetFilterEntriesArea( nColumn, nFirstRow, nLastRow,
                                            nTab, *pEntryLists[nColumn] );
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
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_06)

void ScPivotFilterDlg::ClearValueList( USHORT nList )
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

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_07)

USHORT ScPivotFilterDlg::GetFieldSelPos( USHORT nField )
{
    if ( nField >= theQueryData.nCol1 && nField <= theQueryData.nCol2 )
        return nField - theQueryData.nCol1 + 1;
    else
        return 0;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_08)

const ScQueryItem& ScPivotFilterDlg::GetOutputItem()
{
    ScQueryParam    theParam( theQueryData );
    USHORT          nConnect1 = aLbConnect1.GetSelectEntryPos();
    USHORT          nConnect2 = aLbConnect2.GetSelectEntryPos();

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
                *rEntry.pStr    = EMPTY_STRING;
                rEntry.nVal     = SC_EMPTYFIELDS;
                rEntry.bQueryByString = FALSE;
            }
            else if ( aStrVal == aStrNotEmpty )
            {
                *rEntry.pStr    = EMPTY_STRING;
                rEntry.nVal     = SC_NONEMPTYFIELDS;
                rEntry.bQueryByString = FALSE;
            }
            else
            {
                *rEntry.pStr    = aStrVal;
                rEntry.nVal     = 0;
                rEntry.bQueryByString = TRUE;
            }

            rEntry.nField   = nField ? ( theQueryData.nCol1 + nField - 1 ) : 0;
            rEntry.eOp      = eOp;
        }
    }

    theParam.GetEntry(1).eConnect = (nConnect1 != LISTBOX_ENTRY_NOTFOUND)
                                    ? (ScQueryConnect)nConnect1
                                    : SC_AND;
    theParam.GetEntry(2).eConnect = (nConnect2 != LISTBOX_ENTRY_NOTFOUND)
                                    ? (ScQueryConnect)nConnect2
                                    : SC_AND;

    theParam.bInplace   = FALSE;
    theParam.nDestTab   = 0;    // Woher kommen diese Werte?
    theParam.nDestCol   = 0;
    theParam.nDestRow   = 0;

    theParam.bDuplicate     = !aBtnUnique.IsChecked();
    theParam.bCaseSens      = aBtnCase.IsChecked();
    theParam.bRegExp        = aBtnRegExp.IsChecked();

    if ( pOutItem ) DELETEZ( pOutItem );
    pOutItem = new ScQueryItem( nWhichQuery, &theParam );

    return *pOutItem;
}

//------------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_09)

IMPL_LINK( ScPivotFilterDlg, LbSelectHdl, ListBox*, pLb )
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
#pragma SEG_FUNCDEF(pfiltdlg_0b)

IMPL_LINK( ScPivotFilterDlg, CheckBoxHdl, CheckBox*, pBox )
{
    //  bei Gross-/Kleinschreibung die Werte-Listen aktualisieren

    if ( pBox == &aBtnCase )                    // Wertlisten
    {
        for (USHORT i=0; i<=MAXCOL; i++)
            DELETEZ( pEntryLists[i] );

        String aCurVal1 = aEdVal1.GetText();
        String aCurVal2 = aEdVal2.GetText();
        String aCurVal3 = aEdVal3.GetText();
        UpdateValueList( 1 );
        UpdateValueList( 2 );
        UpdateValueList( 3 );
        aEdVal1.SetText( aCurVal1 );
        aEdVal2.SetText( aCurVal2 );
        aEdVal3.SetText( aCurVal3 );
    }

    return NULL;
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(pfiltdlg_0a)

IMPL_LINK( ScPivotFilterDlg, ValModifyHdl, ComboBox*, pEd )
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

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.20  2000/09/17 14:08:56  willem.vandorp
    OpenOffice header added.

    Revision 1.19  2000/08/31 16:38:20  willem.vandorp
    Header and footer replaced

    Revision 1.18  2000/05/25 10:20:06  er
    NOOLDSV

    Revision 1.17  2000/04/14 17:38:03  nn
    unicode changes

    Revision 1.16  2000/02/11 12:23:42  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.15  1997/12/05 18:56:52  ANK
    Includes geaendert


      Rev 1.14   05 Dec 1997 19:56:52   ANK
   Includes geaendert

      Rev 1.13   12 Jun 1997 13:16:54   NN
   #40646# STR_DB_NONAME statt SCSTR_NONAME fuer DB-Bereiche

      Rev 1.12   29 Nov 1996 18:57:42   NN
   Gross-/Kleinschreibung auch in Werte-Liste

      Rev 1.11   27 Nov 1996 14:18:10   NN
   #31076# Entry-Listen pro Spalte nur einmal holen

      Rev 1.10   13 Nov 1996 11:33:20   NN
   ScQueryParam mit dynamischen Eintraegen

      Rev 1.9   29 Oct 1996 14:03:36   NN
   ueberall ScResId statt ResId

      Rev 1.8   18 Jul 1996 11:21:08   NN
   Quell-Tabelle merken

      Rev 1.7   18 Jan 1996 11:44:30   MO
   #24232# leer/nicht-leer-Behandlung, neuer Link

      Rev 1.6   27 Nov 1995 13:42:12   MO
   RangeUtil/Area/Tripel gegen Address/Range ersetzt

      Rev 1.5   08 Nov 1995 13:05:48   MO
   301-Aenderungen

      Rev 1.4   14 Jul 1995 13:55:28   MO
   3. ValueList im Init fuellen (BugId: 15364)

      Rev 1.3   12 Jul 1995 17:40:30   MO
   GetFilterEntries: mit TypedStrCollection

      Rev 1.2   25 Apr 1995 11:39:56   MO
   leer/nicht-leer Feldwerte

      Rev 1.1   24 Mar 1995 13:19:06   TRI
   Segmentierung

      Rev 1.0   22 Feb 1995 19:13:08   MO
   Initial revision.

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

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

