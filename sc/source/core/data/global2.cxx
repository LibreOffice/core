/*************************************************************************
 *
 *  $RCSfile: global2.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: er $ $Date: 2001-03-12 16:47:50 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/textsearch.hxx>
#include <svtools/pathoptions.hxx>
#include <svtools/useroptions.hxx>
#include <tools/intn.hxx>
#include <tools/urlobj.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>
#include <ctype.h>

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#include "global.hxx"
#include "document.hxx"
#include "rangeutl.hxx"
#include "pivot.hxx"
#include "rechead.hxx"
#include "compiler.hxx"

#include "sc.hrc"
#include "globstr.hrc"


// -----------------------------------------------------------------------



#define MAX_LABELS 256 //!!! aus fieldwnd.hxx, muss noch nach global.hxx ???

//------------------------------------------------------------------------
// struct ScImportParam:

ScImportParam::ScImportParam() :
    nCol1(0),
    nRow1(0),
    nCol2(0),
    nRow2(0),
    bImport(FALSE),
    bNative(FALSE),
    bSql(TRUE),
    nType(ScDbTable)
{
}

ScImportParam::ScImportParam( const ScImportParam& r ) :
    nCol1       (r.nCol1),
    nRow1       (r.nRow1),
    nCol2       (r.nCol2),
    nRow2       (r.nRow2),
    bImport     (r.bImport),
    aDBName     (r.aDBName),
    aStatement  (r.aStatement),
    bNative     (r.bNative),
    bSql        (r.bSql),
    nType       (r.nType)
{
}

ScImportParam::~ScImportParam()
{
}

void ScImportParam::Clear()
{
    nCol1=nRow1=nCol2=nRow2 = 0;
    bImport = FALSE;
    bNative = FALSE;
    bSql = TRUE;
    nType = ScDbTable;
    aDBName.Erase();
    aStatement.Erase();
}

ScImportParam& ScImportParam::operator=( const ScImportParam& r )
{
    nCol1           = r.nCol1;
    nRow1           = r.nRow1;
    nCol2           = r.nCol2;
    nRow2           = r.nRow2;
    bImport         = r.bImport;
    aDBName         = r.aDBName;
    aStatement      = r.aStatement;
    bNative         = r.bNative;
    bSql            = r.bSql;
    nType           = r.nType;

    return *this;
}

BOOL ScImportParam::operator==( const ScImportParam& rOther ) const
{
    return( nCol1       == rOther.nCol1 &&
            nRow1       == rOther.nRow1 &&
            nCol2       == rOther.nCol2 &&
            nRow2       == rOther.nRow2 &&
            bImport     == rOther.bImport &&
            aDBName     == rOther.aDBName &&
            aStatement  == rOther.aStatement &&
            bNative     == rOther.bNative &&
            bSql        == rOther.bSql &&
            nType       == rOther.nType );

    //! nQuerySh und pConnection sind gleich ?
}


//------------------------------------------------------------------------
// struct ScQueryParam:

ScQueryEntry::ScQueryEntry()
{
    bDoQuery        = FALSE;
    bQueryByString  = FALSE;
    eOp             = SC_EQUAL;
    eConnect        = SC_AND;
    nField          = 0;
    nVal            = 0.0;
    pStr            = new String;
    pSearchParam    = NULL;
    pSearchText     = NULL;
}

ScQueryEntry::ScQueryEntry(const ScQueryEntry& r)
{
    bDoQuery        = r.bDoQuery;
    bQueryByString  = r.bQueryByString;
    eOp             = r.eOp;
    eConnect        = r.eConnect;
    nField          = r.nField;
    nVal            = r.nVal;
    pStr            = new String(*r.pStr);
    pSearchParam    = NULL;
    pSearchText     = NULL;
}

ScQueryEntry::~ScQueryEntry()
{
    delete pStr;
    if ( pSearchParam )
    {
        delete pSearchParam;
        delete pSearchText;
    }
}

ScQueryEntry& ScQueryEntry::operator=( const ScQueryEntry& r )
{
    bDoQuery        = r.bDoQuery;
    bQueryByString  = r.bQueryByString;
    eOp             = r.eOp;
    eConnect        = r.eConnect;
    nField          = r.nField;
    nVal            = r.nVal;
    *pStr           = *r.pStr;
    if ( pSearchParam )
    {
        delete pSearchParam;
        delete pSearchText;
    }
    pSearchParam    = NULL;
    pSearchText     = NULL;

    return *this;
}

void ScQueryEntry::Clear()
{
    bDoQuery        = FALSE;
    bQueryByString  = FALSE;
    eOp             = SC_EQUAL;
    eConnect        = SC_AND;
    nField          = 0;
    nVal            = 0.0;
    pStr->Erase();
    if ( pSearchParam )
    {
        delete pSearchParam;
        delete pSearchText;
    }
    pSearchParam    = NULL;
    pSearchText     = NULL;
}

BOOL ScQueryEntry::operator==( const ScQueryEntry& r ) const
{
    return bDoQuery         == r.bDoQuery
        && bQueryByString   == r.bQueryByString
        && eOp              == r.eOp
        && eConnect         == r.eConnect
        && nField           == r.nField
        && nVal             == r.nVal
        && *pStr            == *r.pStr;
    //! pSearchParam und pSearchText nicht vergleichen
}

void ScQueryEntry::Load( SvStream& rStream )
{
    BYTE cOp, cConnect;
    rStream >> bDoQuery
            >> bQueryByString
            >> cOp
            >> cConnect
            >> nField
            >> nVal;
    rStream.ReadByteString( *pStr, rStream.GetStreamCharSet() );
    eOp = (ScQueryOp) cOp;
    eConnect = (ScQueryConnect) cConnect;
}

void ScQueryEntry::Store( SvStream& rStream ) const
{
    rStream << bDoQuery
            << bQueryByString
            << (BYTE) eOp
            << (BYTE) eConnect
            << nField
            << nVal;
    rStream.WriteByteString( *pStr, rStream.GetStreamCharSet() );
}

utl::TextSearch* ScQueryEntry::GetSearchTextPtr( BOOL bCaseSens )
{
    if ( !pSearchParam )
    {
        pSearchParam = new utl::SearchParam( *pStr, utl::SearchParam::SRCH_REGEXP,
            bCaseSens, FALSE, FALSE );
        pSearchText = new utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
    }
    return pSearchText;
}

//------------------------------------------------------------------------

ScQueryParam::ScQueryParam()
{
    nEntryCount = 0;
    Clear();
}

//------------------------------------------------------------------------

ScQueryParam::ScQueryParam( const ScQueryParam& r ) :
        nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),nTab(r.nTab),
        nDestTab(r.nDestTab),nDestCol(r.nDestCol),nDestRow(r.nDestRow),
        bHasHeader(r.bHasHeader),bInplace(r.bInplace),bCaseSens(r.bCaseSens),
        bRegExp(r.bRegExp),bDuplicate(r.bDuplicate),bByRow(r.bByRow),
        bDestPers(r.bDestPers)
{
    nEntryCount = 0;

    Resize( r.nEntryCount );
    for (USHORT i=0; i<nEntryCount; i++)
        pEntries[i] = r.pEntries[i];
}

//------------------------------------------------------------------------

ScQueryParam::~ScQueryParam()
{
    delete[] pEntries;
}

//------------------------------------------------------------------------

void ScQueryParam::Clear()
{
    nCol1=nRow1=nCol2=nRow2=
    nDestTab=nDestCol=nDestRow = 0;
    nTab = USHRT_MAX;
    bHasHeader=bCaseSens=bRegExp = FALSE;
    bInplace=bByRow=bDuplicate=bDestPers = TRUE;

    Resize( MAXQUERY );
    for (USHORT i=0; i<MAXQUERY; i++)
        pEntries[i].Clear();
}

//------------------------------------------------------------------------

ScQueryParam& ScQueryParam::operator=( const ScQueryParam& r )
{
    nCol1       = r.nCol1;
    nRow1       = r.nRow1;
    nCol2       = r.nCol2;
    nRow2       = r.nRow2;
    nTab        = r.nTab;
    nDestTab    = r.nDestTab;
    nDestCol    = r.nDestCol;
    nDestRow    = r.nDestRow;
    bHasHeader  = r.bHasHeader;
    bInplace    = r.bInplace;
    bCaseSens   = r.bCaseSens;
    bRegExp     = r.bRegExp;
    bDuplicate  = r.bDuplicate;
    bByRow      = r.bByRow;
    bDestPers   = r.bDestPers;

    Resize( r.nEntryCount );
    for (USHORT i=0; i<nEntryCount; i++)
        pEntries[i] = r.pEntries[i];

    return *this;
}

//------------------------------------------------------------------------

BOOL ScQueryParam::operator==( const ScQueryParam& rOther ) const
{
    BOOL bEqual = FALSE;

    // Anzahl der Queries gleich?
    USHORT nUsed      = 0;
    USHORT nOtherUsed = 0;
    while ( nUsed<nEntryCount && pEntries[nUsed].bDoQuery ) ++nUsed;
    while ( nOtherUsed<rOther.nEntryCount && rOther.pEntries[nOtherUsed].bDoQuery )
        ++nOtherUsed;

    if (   (nUsed       == nOtherUsed)
        && (nCol1       == rOther.nCol1)
        && (nRow1       == rOther.nRow1)
        && (nCol2       == rOther.nCol2)
        && (nRow2       == rOther.nRow2)
        && (nTab        == rOther.nTab)
        && (bHasHeader  == rOther.bHasHeader)
        && (bByRow      == rOther.bByRow)
        && (bInplace    == rOther.bInplace)
        && (bCaseSens   == rOther.bCaseSens)
        && (bRegExp     == rOther.bRegExp)
        && (bDuplicate  == rOther.bDuplicate)
        && (bDestPers   == rOther.bDestPers)
        && (nDestTab    == rOther.nDestTab)
        && (nDestCol    == rOther.nDestCol)
        && (nDestRow    == rOther.nDestRow) )
    {
        bEqual = TRUE;
        for ( USHORT i=0; i<nUsed && bEqual; i++ )
            bEqual = pEntries[i] == rOther.pEntries[i];
    }
    return bEqual;
}

//------------------------------------------------------------------------

void ScQueryParam::DeleteQuery( USHORT nPos )
{
    if (nPos<nEntryCount)
    {
        for (USHORT i=nPos; i+1<nEntryCount; i++)
            pEntries[i] = pEntries[i+1];

        pEntries[nEntryCount-1].Clear();
    }
    else
        DBG_ERROR("Falscher Parameter bei ScQueryParam::DeleteQuery");
}

//------------------------------------------------------------------------

void ScQueryParam::Resize(USHORT nNew)
{
    if ( nNew < MAXQUERY )
        nNew = MAXQUERY;                // nie weniger als MAXQUERY

    ScQueryEntry* pNewEntries = NULL;
    if ( nNew )
        pNewEntries = new ScQueryEntry[nNew];

    USHORT nCopy = Min( nEntryCount, nNew );
    for (USHORT i=0; i<nCopy; i++)
        pNewEntries[i] = pEntries[i];

    if ( nEntryCount )
        delete[] pEntries;
    nEntryCount = nNew;
    pEntries = pNewEntries;
}

//------------------------------------------------------------------------

void ScQueryParam::MoveToDest()
{
    if (!bInplace)
    {
        short nDifX = ((short) nDestCol) - ((short) nCol1);
        short nDifY = ((short) nDestRow) - ((short) nRow1);
        short nDifZ = ((short) nDestTab) - ((short) nTab);

        nCol1 += nDifX;
        nRow1 += nDifY;
        nCol2 += nDifX;
        nRow2 += nDifY;
        nTab  += nDifZ;
        for (USHORT i=0; i<nEntryCount; i++)
            pEntries[i].nField += nDifX;

        bInplace = TRUE;
    }
    else
        DBG_ERROR("MoveToDest, bInplace == TRUE");
}

//------------------------------------------------------------------------

void ScQueryParam::FillInExcelSyntax(String& aCellStr, USHORT nIndex)
{
    if (aCellStr.Len() > 0)
    {
        if ( nIndex >= nEntryCount )
            Resize( nIndex+1 );

        ScQueryEntry& rEntry = pEntries[nIndex];

        rEntry.bDoQuery = TRUE;
        // Operatoren herausfiltern
        if (aCellStr.GetChar(0) == '<')
        {
            if (aCellStr.GetChar(1) == '>')
            {
                *rEntry.pStr = aCellStr.Copy(2);
                rEntry.eOp   = SC_NOT_EQUAL;
            }
            else if (aCellStr.GetChar(1) == '=')
            {
                *rEntry.pStr = aCellStr.Copy(2);
                rEntry.eOp   = SC_LESS_EQUAL;
            }
            else
            {
                *rEntry.pStr = aCellStr.Copy(1);
                rEntry.eOp   = SC_LESS;
            }
        }
        else if (aCellStr.GetChar(0) == '>')
        {
            if (aCellStr.GetChar(1) == '=')
            {
                *rEntry.pStr = aCellStr.Copy(2);
                rEntry.eOp   = SC_GREATER_EQUAL;
            }
            else
            {
                *rEntry.pStr = aCellStr.Copy(1);
                rEntry.eOp   = SC_GREATER;
            }
        }
        else
        {
            if (aCellStr.GetChar(0) == '=')
                *rEntry.pStr = aCellStr.Copy(1);
            else
                *rEntry.pStr = aCellStr;
            rEntry.eOp = SC_EQUAL;
        }
    }
}

//------------------------------------------------------------------------

void ScQueryParam::Load( SvStream& rStream )        // z.B. fuer Pivot-Tabelle
{
    // bDestPers wird nicht geladen/gespeichert

    ScReadHeader aHdr( rStream );

    rStream >> nCol1
            >> nRow1
            >> nCol2
            >> nRow2
            >> nDestTab
            >> nDestCol
            >> nDestRow
            >> bHasHeader
            >> bInplace
            >> bCaseSens
            >> bRegExp
            >> bDuplicate
            >> bByRow;

    Resize( MAXQUERY );

    for (USHORT i=0; i<MAXQUERY; i++)
        pEntries[i].Load(rStream);
}

//------------------------------------------------------------------------

void ScQueryParam::Store( SvStream& rStream ) const     // z.B. fuer Pivot-Tabelle
{
    // bDestPers wird nicht geladen/gespeichert

    ScWriteHeader aHdr( rStream );

    DBG_ASSERT( nEntryCount <= MAXQUERY || !pEntries[MAXQUERY].bDoQuery,
                    "zuviele Eintraege bei ScQueryParam::Store" );


    if ( nEntryCount < MAXQUERY )
    {
        DBG_ERROR("ScQueryParam::Store - zuwenig Eintraege");
        ((ScQueryParam*)this)->Resize( MAXQUERY );
    }

    rStream << nCol1
            << nRow1
            << nCol2
            << nRow2
            << nDestTab
            << nDestCol
            << nDestRow
            << bHasHeader
            << bInplace
            << bCaseSens
            << bRegExp
            << bDuplicate
            << bByRow;

    for (USHORT i=0; i<MAXQUERY; i++)
        pEntries[i].Store(rStream);
}

//------------------------------------------------------------------------
// struct ScSubTotalParam:

ScSubTotalParam::ScSubTotalParam()
{
    for ( USHORT i=0; i<MAXSUBTOTAL; i++ )
    {
        nSubTotals[i] = 0;
        pSubTotals[i] = NULL;
        pFunctions[i] = NULL;
    }

    Clear();
}

//------------------------------------------------------------------------

ScSubTotalParam::ScSubTotalParam( const ScSubTotalParam& r ) :
        nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),
        bReplace(r.bReplace),bPagebreak(r.bPagebreak),bCaseSens(r.bCaseSens),
        bDoSort(r.bDoSort),bAscending(r.bAscending),bUserDef(r.bUserDef),nUserIndex(r.nUserIndex),
        bIncludePattern(r.bIncludePattern),bRemoveOnly(r.bRemoveOnly)
{
    for (USHORT i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = r.bGroupActive[i];
        nField[i]       = r.nField[i];

        if ( (r.nSubTotals[i] > 0) && r.pSubTotals[i] && r.pFunctions[i] )
        {
            nSubTotals[i] = r.nSubTotals[i];
            pSubTotals[i] = new USHORT          [r.nSubTotals[i]];
            pFunctions[i] = new ScSubTotalFunc  [r.nSubTotals[i]];

            for (USHORT j=0; j<r.nSubTotals[i]; j++)
            {
                pSubTotals[i][j] = r.pSubTotals[i][j];
                pFunctions[i][j] = r.pFunctions[i][j];
            }
        }
        else
        {
            nSubTotals[i] = 0;
            pSubTotals[i] = NULL;
            pFunctions[i] = NULL;
        }
    }
}

//------------------------------------------------------------------------

void ScSubTotalParam::Clear()
{
    nCol1=nRow1=nCol2=nRow2=nUserIndex = 0;
    bPagebreak=bCaseSens=bUserDef=bIncludePattern=bRemoveOnly = FALSE;
    bAscending=bReplace=bDoSort = TRUE;

    for (USHORT i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = FALSE;
        nField[i]       = 0;

        if ( (nSubTotals[i] > 0) && pSubTotals[i] && pFunctions[i] )
        {
            for ( USHORT j=0; j<nSubTotals[i]; j++ )
                pSubTotals[i][j] = 0;
                pFunctions[i][j] = SUBTOTAL_FUNC_NONE;
        }
    }
}

//------------------------------------------------------------------------

ScSubTotalParam& ScSubTotalParam::operator=( const ScSubTotalParam& r )
{
    nCol1           = r.nCol1;
    nRow1           = r.nRow1;
    nCol2           = r.nCol2;
    nRow2           = r.nRow2;
    bRemoveOnly     = r.bRemoveOnly;
    bReplace        = r.bReplace;
    bPagebreak      = r.bPagebreak;
    bCaseSens       = r.bCaseSens;
    bDoSort         = r.bDoSort;
    bAscending      = r.bAscending;
    bUserDef        = r.bUserDef;
    nUserIndex      = r.nUserIndex;
    bIncludePattern = r.bIncludePattern;

    for (USHORT i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = r.bGroupActive[i];
        nField[i]       = r.nField[i];
        nSubTotals[i]   = r.nSubTotals[i];

        if ( pSubTotals[i] ) delete [] pSubTotals[i];
        if ( pFunctions[i] ) delete [] pFunctions[i];

        if ( r.nSubTotals[i] > 0 )
        {
            pSubTotals[i] = new USHORT          [r.nSubTotals[i]];
            pFunctions[i] = new ScSubTotalFunc  [r.nSubTotals[i]];

            for (USHORT j=0; j<r.nSubTotals[i]; j++)
            {
                pSubTotals[i][j] = r.pSubTotals[i][j];
                pFunctions[i][j] = r.pFunctions[i][j];
            }
        }
        else
        {
            nSubTotals[i] = 0;
            pSubTotals[i] = NULL;
            pFunctions[i] = NULL;
        }
    }

    return *this;
}

//------------------------------------------------------------------------

BOOL ScSubTotalParam::operator==( const ScSubTotalParam& rOther ) const
{
    BOOL bEqual =   (nCol1          == rOther.nCol1)
                 && (nRow1          == rOther.nRow1)
                 && (nCol2          == rOther.nCol2)
                 && (nRow2          == rOther.nRow2)
                 && (bRemoveOnly    == rOther.bRemoveOnly)
                 && (bReplace       == rOther.bReplace)
                 && (bPagebreak     == rOther.bPagebreak)
                 && (bDoSort        == rOther.bDoSort)
                 && (bCaseSens      == rOther.bCaseSens)
                 && (bAscending     == rOther.bAscending)
                 && (bUserDef       == rOther.bUserDef)
                 && (nUserIndex     == rOther.nUserIndex)
                 && (bIncludePattern== rOther.bIncludePattern);

    if ( bEqual )
    {
        bEqual = TRUE;
        for ( USHORT i=0; i<MAXSUBTOTAL && bEqual; i++ )
        {
            bEqual =   (bGroupActive[i] == rOther.bGroupActive[i])
                    && (nField[i]       == rOther.nField[i])
                    && (nSubTotals[i]   == rOther.nSubTotals[i]);

            if ( bEqual && (nSubTotals[i] > 0) )
            {
                bEqual = (pSubTotals != NULL) && (pFunctions != NULL);

                for (USHORT j=0; (j<nSubTotals[i]) && bEqual; j++)
                {
                    bEqual =   bEqual
                            && (pSubTotals[i][j] == rOther.pSubTotals[i][j])
                            && (pFunctions[i][j] == rOther.pFunctions[i][j]);
                }
            }
        }
    }

    return bEqual;
}

//------------------------------------------------------------------------

void ScSubTotalParam::SetSubTotals( USHORT                  nGroup,
                                    const USHORT*           ptrSubTotals,
                                    const ScSubTotalFunc*   ptrFunctions,
                                    USHORT                  nCount )
{
    DBG_ASSERT( (nGroup <= MAXSUBTOTAL),
                "ScSubTotalParam::SetSubTotals(): nGroup > MAXSUBTOTAL!" );
    DBG_ASSERT( ptrSubTotals,
                "ScSubTotalParam::SetSubTotals(): ptrSubTotals == NULL!" );
    DBG_ASSERT( ptrFunctions,
                "ScSubTotalParam::SetSubTotals(): ptrFunctions == NULL!" );
    DBG_ASSERT( (nCount > 0),
                "ScSubTotalParam::SetSubTotals(): nCount <= 0!" );

    if ( ptrSubTotals && ptrFunctions && (nCount > 0) && (nGroup <= MAXSUBTOTAL) )
    {
        // 0 wird als 1 aufgefasst, sonst zum Array-Index dekrementieren
        if (nGroup != 0)
            nGroup--;

        delete [] pSubTotals[nGroup];
        delete [] pFunctions[nGroup];

        pSubTotals[nGroup] = new USHORT         [nCount];
        pFunctions[nGroup] = new ScSubTotalFunc [nCount];
        nSubTotals[nGroup] = nCount;

        for ( USHORT i=0; i<nCount; i++ )
        {
            pSubTotals[nGroup][i] = ptrSubTotals[i];
            pFunctions[nGroup][i] = ptrFunctions[i];
        }
    }
}

//------------------------------------------------------------------------
// struct ScConsolidateParam:

ScConsolidateParam::ScConsolidateParam() :
    ppDataAreas( NULL )
{
    Clear();
}

//------------------------------------------------------------------------

ScConsolidateParam::ScConsolidateParam( const ScConsolidateParam& r ) :
        ppDataAreas( NULL ),
        nCol(r.nCol),nRow(r.nRow),nTab(r.nTab),
        bByCol(r.bByCol),bByRow(r.bByRow),bReferenceData(r.bReferenceData),
        nDataAreaCount(0),eFunction(r.eFunction)
{
    if ( r.nDataAreaCount > 0 )
    {
        nDataAreaCount = r.nDataAreaCount;
        ppDataAreas = new ScArea*[nDataAreaCount];
        for ( USHORT i=0; i<nDataAreaCount; i++ )
            ppDataAreas[i] = new ScArea( *(r.ppDataAreas[i]) );
    }
}

//------------------------------------------------------------------------

__EXPORT ScConsolidateParam::~ScConsolidateParam()
{
    ClearDataAreas();
}

//------------------------------------------------------------------------

void __EXPORT ScConsolidateParam::ClearDataAreas()
{
    if ( ppDataAreas )
    {
        for ( USHORT i=0; i<nDataAreaCount; i++ )
            delete ppDataAreas[i];
        delete [] ppDataAreas;
        ppDataAreas = NULL;
    }
    nDataAreaCount = 0;
}

//------------------------------------------------------------------------

void __EXPORT ScConsolidateParam::Clear()
{
    ClearDataAreas();

    nCol = nRow = nTab                  = 0;
    bByCol = bByRow = bReferenceData    = FALSE;
    eFunction                           = SUBTOTAL_FUNC_SUM;
}

//------------------------------------------------------------------------

ScConsolidateParam& __EXPORT ScConsolidateParam::operator=( const ScConsolidateParam& r )
{
    nCol            = r.nCol;
    nRow            = r.nRow;
    nTab            = r.nTab;
    bByCol          = r.bByCol;
    bByRow          = r.bByRow;
    bReferenceData  = r.bReferenceData;
    eFunction       = r.eFunction;
    SetAreas( r.ppDataAreas, r.nDataAreaCount );

    return *this;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScConsolidateParam::operator==( const ScConsolidateParam& r ) const
{
    BOOL bEqual =   (nCol           == r.nCol)
                 && (nRow           == r.nRow)
                 && (nTab           == r.nTab)
                 && (bByCol         == r.bByCol)
                 && (bByRow         == r.bByRow)
                 && (bReferenceData == r.bReferenceData)
                 && (nDataAreaCount == r.nDataAreaCount)
                 && (eFunction      == r.eFunction);

    if ( nDataAreaCount == 0 )
        bEqual = bEqual && (ppDataAreas == NULL) && (r.ppDataAreas == NULL);
    else
        bEqual = bEqual && (ppDataAreas != NULL) && (r.ppDataAreas != NULL);

    if ( bEqual && (nDataAreaCount > 0) )
        for ( USHORT i=0; i<nDataAreaCount && bEqual; i++ )
            bEqual = *(ppDataAreas[i]) == *(r.ppDataAreas[i]);

    return bEqual;
}

//------------------------------------------------------------------------

void __EXPORT ScConsolidateParam::SetAreas( ScArea* const* ppAreas, USHORT nCount )
{
    ClearDataAreas();
    if ( ppAreas && nCount > 0 )
    {
        ppDataAreas = new ScArea*[nCount];
        for ( USHORT i=0; i<nCount; i++ )
            ppDataAreas[i] = new ScArea( *(ppAreas[i]) );
        nDataAreaCount = nCount;
    }
}

void ScConsolidateParam::Load( SvStream& rStream )
{
    ClearDataAreas();

    ScReadHeader aHdr( rStream );

    BYTE nByte;
    rStream >> nCol >> nRow >> nTab
            >> bByCol >> bByRow >> bReferenceData >> nByte;
    eFunction = (ScSubTotalFunc) nByte;

    rStream >> nDataAreaCount;
    if ( nDataAreaCount )
    {
        ppDataAreas = new ScArea*[nDataAreaCount];
        for ( USHORT i=0; i<nDataAreaCount; i++ )
        {
            ppDataAreas[i] = new ScArea();
            rStream >> *ppDataAreas[i];
        }
    }
}

void ScConsolidateParam::Store( SvStream& rStream ) const
{
    ScWriteHeader aHdr( rStream, 12+10*nDataAreaCount );

    rStream << nCol << nRow << nTab
            << bByCol << bByRow << bReferenceData << (BYTE) eFunction;

    rStream << nDataAreaCount;
    for (USHORT i=0; i<nDataAreaCount; i++)
        rStream << *ppDataAreas[i];
}

//------------------------------------------------------------------------
// struct ScPivotParam:

ScPivotParam::ScPivotParam()
    :   nCol(0), nRow(0), nTab(0),
        ppLabelArr( NULL ), nLabels(0),
        nColCount(0), nRowCount(0), nDataCount(0),
        bIgnoreEmptyRows(FALSE), bDetectCategories(FALSE),
        bMakeTotalCol(TRUE), bMakeTotalRow(TRUE)
{
}

//------------------------------------------------------------------------

ScPivotParam::ScPivotParam( const ScPivotParam& r )
    :   nCol( r.nCol ), nRow( r.nRow ), nTab( r.nTab ),
        bIgnoreEmptyRows(r.bIgnoreEmptyRows),
        bDetectCategories(r.bDetectCategories),
        bMakeTotalCol(r.bMakeTotalCol),
        bMakeTotalRow(r.bMakeTotalRow),
        ppLabelArr( NULL ), nLabels(0),
        nColCount(0), nRowCount(0), nDataCount(0)
{
    SetLabelData    ( r.ppLabelArr, r.nLabels );
    SetPivotArrays  ( r.aColArr, r.aRowArr, r.aDataArr,
                      r.nColCount, r.nRowCount, r.nDataCount );
}

//------------------------------------------------------------------------

__EXPORT ScPivotParam::~ScPivotParam()
{
    ClearLabelData();
}

//------------------------------------------------------------------------

void __EXPORT ScPivotParam::Clear()
{
    nCol = nRow = nTab = 0;
    bIgnoreEmptyRows = bDetectCategories = FALSE;
    bMakeTotalCol = bMakeTotalRow = TRUE;
    ClearLabelData();
    ClearPivotArrays();
}

//------------------------------------------------------------------------

void __EXPORT ScPivotParam::ClearLabelData()
{
    if ( (nLabels > 0) && ppLabelArr )
    {
        for ( USHORT i=0; i<nLabels; i++ )
            delete ppLabelArr[i];
        delete [] ppLabelArr;
        ppLabelArr = NULL;
        nLabels = 0;
    }
}

//------------------------------------------------------------------------

void __EXPORT ScPivotParam::ClearPivotArrays()
{
    memset( aColArr, 0, PIVOT_MAXFIELD * sizeof(PivotField) );
    memset( aRowArr, 0, PIVOT_MAXFIELD * sizeof(PivotField) );
    memset( aDataArr, 0, PIVOT_MAXFIELD * sizeof(PivotField) );
    nColCount = nRowCount = nDataCount = 0;
}

//------------------------------------------------------------------------

void __EXPORT ScPivotParam::SetLabelData( LabelData**   pLabArr,
                                          USHORT        nLab )
{
    ClearLabelData();

    if ( (nLab > 0) && pLabArr )
    {
        nLabels = (nLab>MAX_LABELS) ? MAX_LABELS : nLab;
        ppLabelArr = new LabelData*[nLabels];
        for ( USHORT i=0; i<nLabels; i++ )
            ppLabelArr[i] = new LabelData( *(pLabArr[i]) );
    }
}

//------------------------------------------------------------------------

void __EXPORT ScPivotParam::SetPivotArrays  ( const PivotField* pColArr,
                                              const PivotField* pRowArr,
                                              const PivotField* pDataArr,
                                              USHORT            nColCnt,
                                              USHORT            nRowCnt,
                                              USHORT            nDataCnt )
{
    ClearPivotArrays();

    if ( pColArr && pRowArr && pDataArr )
    {
        nColCount   = (nColCnt>PIVOT_MAXFIELD) ? PIVOT_MAXFIELD : nColCnt;
        nRowCount   = (nRowCnt>PIVOT_MAXFIELD) ? PIVOT_MAXFIELD : nRowCnt;
        nDataCount  = (nDataCnt>PIVOT_MAXFIELD) ? PIVOT_MAXFIELD : nDataCnt;

        memcpy( aColArr,  pColArr,  nColCount  * sizeof(PivotField) );
        memcpy( aRowArr,  pRowArr,  nRowCount  * sizeof(PivotField) );
        memcpy( aDataArr, pDataArr, nDataCount * sizeof(PivotField) );
    }
}

//------------------------------------------------------------------------

ScPivotParam& __EXPORT ScPivotParam::operator=( const ScPivotParam& r )
{
    nCol              = r.nCol;
    nRow              = r.nRow;
    nTab              = r.nTab;
    bIgnoreEmptyRows  = r.bIgnoreEmptyRows;
    bDetectCategories = r.bDetectCategories;
    bMakeTotalCol     = r.bMakeTotalCol;
    bMakeTotalRow     = r.bMakeTotalRow;

    SetLabelData    ( r.ppLabelArr, r.nLabels );
    SetPivotArrays  ( r.aColArr, r.aRowArr, r.aDataArr,
                      r.nColCount, r.nRowCount, r.nDataCount );

    return *this;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScPivotParam::operator==( const ScPivotParam& r ) const
{
    BOOL bEqual =   (nCol       == r.nCol)
                 && (nRow       == r.nRow)
                 && (nTab       == r.nTab)
                 && (bIgnoreEmptyRows  == r.bIgnoreEmptyRows)
                 && (bDetectCategories == r.bDetectCategories)
                 && (bMakeTotalCol == r.bMakeTotalCol)
                 && (bMakeTotalRow == r.bMakeTotalRow)
                 && (nLabels    == r.nLabels)
                 && (nColCount  == r.nColCount)
                 && (nRowCount  == r.nRowCount)
                 && (nDataCount == r.nDataCount);

    if ( bEqual )
    {
        USHORT i;

        for ( i=0; i<nColCount && bEqual; i++ )
            bEqual = ( aColArr[i] == r.aColArr[i] );

        for ( i=0; i<nRowCount && bEqual; i++ )
            bEqual = ( aRowArr[i] == r.aRowArr[i] );

        for ( i=0; i<nDataCount && bEqual; i++ )
            bEqual = ( aDataArr[i] == r.aDataArr[i] );
    }

    return bEqual;
}

//------------------------------------------------------------------------
// struct ScSolveParam

ScSolveParam::ScSolveParam()
    :   pStrTargetVal( NULL )
{
}

//------------------------------------------------------------------------

ScSolveParam::ScSolveParam( const ScSolveParam& r )
    :   aRefFormulaCell ( r.aRefFormulaCell ),
        aRefVariableCell( r.aRefVariableCell ),
        pStrTargetVal   ( r.pStrTargetVal
                            ? new String(*r.pStrTargetVal)
                            : NULL )
{
}

//------------------------------------------------------------------------

ScSolveParam::ScSolveParam( const ScAddress& rFormulaCell,
                            const ScAddress& rVariableCell,
                            const String&   rTargetValStr )
    :   aRefFormulaCell ( rFormulaCell ),
        aRefVariableCell( rVariableCell ),
        pStrTargetVal   ( new String(rTargetValStr) )
{
}

//------------------------------------------------------------------------

ScSolveParam::~ScSolveParam()
{
    delete pStrTargetVal;
}

//------------------------------------------------------------------------

ScSolveParam& __EXPORT ScSolveParam::operator=( const ScSolveParam& r )
{
    delete pStrTargetVal;

    aRefFormulaCell  = r.aRefFormulaCell;
    aRefVariableCell = r.aRefVariableCell;
    pStrTargetVal    = r.pStrTargetVal
                            ? new String(*r.pStrTargetVal)
                            : NULL;
    return *this;
}

//------------------------------------------------------------------------

BOOL ScSolveParam::operator==( const ScSolveParam& r ) const
{
    BOOL bEqual =   (aRefFormulaCell  == r.aRefFormulaCell)
                 && (aRefVariableCell == r.aRefVariableCell);

    if ( bEqual )
    {
        if ( !pStrTargetVal && !r.pStrTargetVal )
            bEqual = TRUE;
        else if ( !pStrTargetVal || !r.pStrTargetVal )
            bEqual = FALSE;
        else if ( pStrTargetVal && r.pStrTargetVal )
            bEqual = ( *pStrTargetVal == *(r.pStrTargetVal) );
    }

    return bEqual;
}


//------------------------------------------------------------------------
// struct ScTabOpParam

ScTabOpParam::ScTabOpParam( const ScTabOpParam& r )
    :   aRefFormulaCell ( r.aRefFormulaCell ),
        aRefFormulaEnd  ( r.aRefFormulaEnd ),
        aRefRowCell     ( r.aRefRowCell ),
        aRefColCell     ( r.aRefColCell ),
        nMode           ( r.nMode )
{
}

//------------------------------------------------------------------------

ScTabOpParam::ScTabOpParam( const ScRefTripel& rFormulaCell,
                            const ScRefTripel& rFormulaEnd,
                            const ScRefTripel& rRowCell,
                            const ScRefTripel& rColCell,
                                  BYTE       nMd)
    :   aRefFormulaCell ( rFormulaCell ),
        aRefFormulaEnd  ( rFormulaEnd ),
        aRefRowCell     ( rRowCell ),
        aRefColCell     ( rColCell ),
        nMode           ( nMd )
{
}

//------------------------------------------------------------------------

ScTabOpParam& ScTabOpParam::operator=( const ScTabOpParam& r )
{
    aRefFormulaCell  = r.aRefFormulaCell;
    aRefFormulaEnd   = r.aRefFormulaEnd;
    aRefRowCell      = r.aRefRowCell;
    aRefColCell      = r.aRefColCell;
    nMode            = r.nMode;
    return *this;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScTabOpParam::operator==( const ScTabOpParam& r ) const
{
    return (        (aRefFormulaCell == r.aRefFormulaCell)
                 && (aRefFormulaEnd  == r.aRefFormulaEnd)
                 && (aRefRowCell     == r.aRefRowCell)
                 && (aRefColCell     == r.aRefColCell)
                 && (nMode           == r.nMode) );
}


//========================================================================
// class ScPostIt
//========================================================================

ScPostIt::ScPostIt()
{
    bShown = FALSE;
}

ScPostIt::ScPostIt( const String& rText )
{
    AutoSetText( rText );
    bShown = FALSE;
}

ScPostIt::ScPostIt( const String& rText, const String& rDate, const String& rAuthor )
    :   aStrText    ( rText ),
        aStrDate    ( rDate ),
        aStrAuthor  ( rAuthor )
{
    bShown = FALSE;
}

ScPostIt::ScPostIt( const ScPostIt& rCpy )
    :   aStrText    ( rCpy.aStrText ),
        aStrDate    ( rCpy.aStrDate ),
        aStrAuthor  ( rCpy.aStrAuthor )
{
    bShown = rCpy.bShown;
}

__EXPORT ScPostIt::~ScPostIt()
{
}

void ScPostIt::AutoSetText( const String& rNewText )
{
    aStrText   = rNewText;
    aStrDate   = ScGlobal::pLocaleData->getDate( Date() );

    //  Der Einheitlichkeit halber das Datum immer ohne Uhrzeit (wie im Writer)
//  aStrDate  += ", ";
//  aStrDate  += ScGlobal::pLocaleData->getTime( Time() );

    SvtUserOptions aUserOpt;
    aStrAuthor = aUserOpt.GetID();
}

//========================================================================

String ScGlobal::GetAbsDocName( const String& rFileName,
                                SfxObjectShell* pShell )
{
    String aAbsName;
    if ( !pShell->HasName() )
    {   // maybe relative to document path working directory
        INetURLObject aObj;
        SvtPathOptions aPathOpt;
        aObj.SetSmartURL( aPathOpt.GetWorkPath() );
        aObj.setFinalSlash();       // it IS a path
        bool bWasAbs = true;
        aAbsName = aObj.smartRel2Abs( rFileName, bWasAbs ).GetMainURL();
    }
    else
    {
        const SfxMedium* pMedium = pShell->GetMedium();
        if ( pMedium )
        {
            bool bWasAbs = true;
            aAbsName = pMedium->GetURLObject().smartRel2Abs( rFileName, bWasAbs ).GetMainURL();
        }
        else
        {   // This can't happen, but ...
            // just to be sure to have the same encoding
            INetURLObject aObj;
            aObj.SetSmartURL( aAbsName );
            aAbsName = aObj.GetMainURL();
        }
    }
    return aAbsName;
}


String ScGlobal::GetDocTabName( const String& rFileName,
                                const String& rTabName )
{
    String aDocTab( '\'' );
    aDocTab += rFileName;
    xub_StrLen nPos = 1;
    while( (nPos = aDocTab.Search( '\'', nPos ))
            != STRING_NOTFOUND )
    {   // escape Quotes
        aDocTab.Insert( '\\', nPos );
        nPos += 2;
    }
    aDocTab += '\'';
    aDocTab += SC_COMPILER_FILE_TAB_SEP;
    aDocTab += rTabName;    // "'Doc'#Tab"
    return aDocTab;
}


USHORT lcl_ConvertSingleRef( BOOL& bExternal, const sal_Unicode* p,
            ScDocument* pDoc, ScAddress& rAddr )
{
    if ( !*p )
        return 0;
    USHORT  nRes = 0;
    String  aDocName;       // der pure Dokumentenname
    String  aDocTab;        // zusammengesetzt fuer Table
    String  aTab;
    BOOL    bExtDoc = FALSE;
    BOOL    bNeedExtTab = FALSE;

    if ( *p == '\'' && ScGlobal::UnicodeStrChr( p, SC_COMPILER_FILE_TAB_SEP ) )
    {
        BOOL bQuote = TRUE;         // Dokumentenname ist immer quoted
        aDocTab += *p++;
        while ( bQuote && *p )
        {
            if ( *p == '\'' && *(p-1) != '\\' )
                bQuote = FALSE;
            else if( !(*p == '\\' && *(p+1) == '\'') )
                aDocName += *p;     // falls escaped Quote: nur Quote in den Namen
            aDocTab += *p++;
        }
        aDocTab += *p;              // den SC_COMPILER_FILE_TAB_SEP mitnehmen
        if( *p++ == SC_COMPILER_FILE_TAB_SEP )
            bExtDoc = TRUE;
        else
            return nRes;
    }

    USHORT  nCol = 0;
    USHORT  nRow = 0;
    USHORT  nTab = 0;
    USHORT  nBits = SCA_VALID_TAB;
    const sal_Unicode* q;
    if ( ScGlobal::UnicodeStrChr( p, '.') )
    {
        nRes |= SCA_TAB_3D;
        if ( bExtDoc )
            nRes |= SCA_TAB_ABSOLUTE;
        if (*p == '$')
            nRes |= SCA_TAB_ABSOLUTE, p++;
        BOOL bQuote = FALSE;
        if( *p == '\'' )
            p++, bQuote = TRUE;
        while (*p && (*p != '.'))
        {
            if( bQuote && *p == '\'' )
            {
                p++; break;
            }
            aTab += *p++;
        }
        if( *p++ != '.' )
            nBits = 0;
        if ( pDoc )
        {
            if ( bExtDoc )
            {
                bExternal = TRUE;
                aDocTab += aTab;    // "'Doc'#Tab"
                if ( !pDoc->GetTable( aDocTab, nTab ) )
                {
                    if ( pDoc->ValidTabName( aTab ) )
                    {
                        aDocName = ScGlobal::GetAbsDocName( aDocName,
                            pDoc->GetDocumentShell() );
                        aDocTab = ScGlobal::GetDocTabName( aDocName, aTab );
                        if ( !pDoc->GetTable( aDocTab, nTab ) )
                        {
                            // erst einfuegen, wenn Rest der Ref ok
                            bNeedExtTab = TRUE;
                            nBits = 0;
                        }
                    }
                    else
                        nBits = 0;
                }
            }
            else
            {
                if ( !pDoc->GetTable( aTab, nTab ) )
                    nBits = 0;
            }
        }
        else
            nBits = 0;
    }
    else
    {
        if ( bExtDoc )
            return nRes;        // nach Dokument muss Tabelle folgen
        nTab = rAddr.Tab();
    }
    nRes |= nBits;

    q = p;
    if (*p)
    {
        nBits = SCA_VALID_COL;
        if (*p == '$')
            nBits |= SCA_COL_ABSOLUTE, p++;
        if( CharClass::isAsciiAlpha( *p ) )
            nCol = toupper( char(*p++) ) - 'A';
        else
            nBits = 0;
        if( CharClass::isAsciiAlpha( *p ) )
            nCol = ((nCol + 1) * 26) + (toupper( char(*p++) ) - 'A');
        if( nCol > MAXCOL )
            nBits = 0;
        while( CharClass::isAsciiAlpha( *p ) )
            p++, nBits = 0;
        nRes |= nBits;
        if( !nBits )
            p = q;
    }

    q = p;
    if (*p)
    {
        nBits = SCA_VALID_ROW;
        if (*p == '$')
            nBits |= SCA_ROW_ABSOLUTE, p++;
        if( !CharClass::isAsciiDigit( *p ) )
        {
            nBits = 0;
            nRow = USHORT(-1);
        }
        else
        {
            String aTmp( p );
            long n = aTmp.ToInt32() - 1;
            while (CharClass::isAsciiDigit( *p ))
                p++;
            if( n < 0 || n > MAXROW )
                nBits = 0;
            nRow = (USHORT) n;
        }
        nRes |= nBits;
        if( !nBits )
            p = q;
    }
    if ( bNeedExtTab )
    {
        if ( (nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
          && pDoc->LinkExternalTab( nTab, aDocTab, aDocName, aTab ) )
        {
            nRes |= SCA_VALID_TAB;
        }
        else
            nRes = 0;   // #NAME? statt #REF!, Dateiname bleibt erhalten
    }
    if ( !(nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
            && !( (nRes & SCA_TAB_3D) && (nRes & SCA_VALID_TAB)) )
    {   // keine Row, keine Tab, aber Col => DM (...), B (...) o.ae.
        nRes = 0;
    }
    if( !*p )
    {
        USHORT nMask = nRes & ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
        if( nMask == ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB ) )
            nRes |= SCA_VALID;
    }
    else
        nRes = 0;
    rAddr.Set( nCol, nRow, nTab );
    return nRes;
}


//  ConvertSingleRef mit sal_Unicode* muss vorher stehen!!!

BOOL ConvertSingleRef( ScDocument* pDoc, const String& rRefString,
            USHORT nDefTab, ScRefTripel& rRefTripel )
{
    BOOL bExternal = FALSE;
    ScAddress aAddr( 0, 0, nDefTab );
    USHORT nRes = lcl_ConvertSingleRef( bExternal, rRefString.GetBuffer(), pDoc, aAddr );
    if( nRes & SCA_VALID )
    {
        rRefTripel.Put( aAddr.Col(), aAddr.Row(), aAddr.Tab(),
                        BOOL( ( nRes & SCA_COL_ABSOLUTE ) == 0 ),
                        BOOL( ( nRes & SCA_ROW_ABSOLUTE ) == 0 ),
                        BOOL( ( nRes & SCA_TAB_ABSOLUTE ) == 0 ) );
        return TRUE;
    }
    else
        return FALSE;
}


BOOL ConvertDoubleRef( ScDocument* pDoc, const String& rRefString, USHORT nDefTab,
            ScRefTripel& rStartRefTripel, ScRefTripel& rEndRefTripel )
{
    BOOL bRet = FALSE;
    xub_StrLen nPos = rRefString.Search(':');
    if (nPos != STRING_NOTFOUND)
    {
        String aTmp( rRefString );
        sal_Unicode* p = aTmp.GetBufferAccess();
        p[ nPos ] = 0;
        if ( ConvertSingleRef( pDoc, p, nDefTab, rStartRefTripel ) )
        {
            nDefTab = rStartRefTripel.GetTab();
            bRet = ConvertSingleRef( pDoc, p + nPos + 1, nDefTab, rEndRefTripel );
        }
    }
    return bRet;
}


USHORT ScAddress::Parse( const String& r, ScDocument* pDoc )
{
    BOOL bExternal = FALSE;
    return lcl_ConvertSingleRef( bExternal, r.GetBuffer(), pDoc, *this );
}

BOOL ScRange::Intersects( const ScRange& r ) const
{
    return !(
        Min( aEnd.Col(), r.aEnd.Col() ) < Max( aStart.Col(), r.aStart.Col() )
     || Min( aEnd.Row(), r.aEnd.Row() ) < Max( aStart.Row(), r.aStart.Row() )
     || Min( aEnd.Tab(), r.aEnd.Tab() ) < Max( aStart.Tab(), r.aStart.Tab() )
        );
}

void ScRange::Justify()
{
    USHORT nTemp;
    if ( aEnd.Col() < (nTemp = aStart.Col()) )
    {
        aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTemp);
    }
    if ( aEnd.Row() < (nTemp = aStart.Row()) )
    {
        aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTemp);
    }
    if ( aEnd.Tab() < (nTemp = aStart.Tab()) )
    {
        aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTemp);
    }
}

void ScRange::ExtendOne()
{
    //  Range fuer Rahmen etc. in X und Y Richtung um 1 erweitern

    USHORT nVal;

    if ((nVal = aStart.Col()) > 0)
        aStart.SetCol(nVal-1);
    if ((nVal = aStart.Row()) > 0)
        aStart.SetRow(nVal-1);

    if ((nVal = aEnd.Col()) < MAXCOL)
        aEnd.SetCol(nVal+1);
    if ((nVal = aEnd.Row()) < MAXROW)
        aEnd.SetRow(nVal+1);
}

USHORT ScRange::Parse( const String& r, ScDocument* pDoc )
{
    USHORT nRes1 = 0, nRes2 = 0;
    xub_StrLen nTmp = 0;
    xub_StrLen nPos = STRING_NOTFOUND;
    while ( (nTmp = r.Search( ':', nTmp )) != STRING_NOTFOUND )
        nPos = nTmp++;      // der letzte zaehlt, koennte 'd:\...'!a1:a2 sein
    if (nPos != STRING_NOTFOUND)
    {
        String aTmp( r );
        sal_Unicode* p = aTmp.GetBufferAccess();
        p[ nPos ] = 0;
        BOOL bExternal = FALSE;
        if( nRes1 = lcl_ConvertSingleRef( bExternal, p, pDoc, aStart ) )
        {
            aEnd = aStart;  // die Tab _muss_ gleich sein, so ist`s weniger Code
            if ( nRes2 = lcl_ConvertSingleRef( bExternal, p + nPos+ 1, pDoc, aEnd ) )
            {
                if ( bExternal && aStart.Tab() != aEnd.Tab() )
                    nRes2 &= ~SCA_VALID_TAB;    // #REF!
                else
                {
                    // PutInOrder / Justify
                    USHORT nTemp, nMask, nBits1, nBits2;
                    if ( aEnd.Col() < (nTemp = aStart.Col()) )
                    {
                        aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTemp);
                        nMask = (SCA_VALID_COL | SCA_COL_ABSOLUTE);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    if ( aEnd.Row() < (nTemp = aStart.Row()) )
                    {
                        aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTemp);
                        nMask = (SCA_VALID_ROW | SCA_ROW_ABSOLUTE);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    if ( aEnd.Tab() < (nTemp = aStart.Tab()) )
                    {
                        aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTemp);
                        nMask = (SCA_VALID_TAB | SCA_TAB_ABSOLUTE | SCA_TAB_3D);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    if ( ((nRes1 & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
                            == ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
                            && !(nRes2 & SCA_TAB_3D) )
                        nRes2 |= SCA_TAB_ABSOLUTE;
                }
            }
            else
                nRes1 = 0;      // #38840# keine Tokens aus halben Sachen
        }
    }
    nRes1 = ( ( nRes1 | nRes2 ) & SCA_VALID )
          | nRes1
          | ( ( nRes2 & 0x070F ) << 4 );
    return nRes1;
}

USHORT ScRange::ParseAny( const String& r, ScDocument* pDoc )
{
    USHORT nRet = Parse( r, pDoc );
    const USHORT nValid = SCA_VALID | SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2;
    if ( (nRet & nValid) != nValid )
    {
        ScAddress aAdr;
        nRet = aAdr.Parse( r, pDoc );
        if ( nRet & SCA_VALID )
            aStart = aEnd = aAdr;
    }
    return nRet;
}

void ScAddress::Format( String& r, USHORT nFlags, ScDocument* pDoc ) const
{
    USHORT nTab = Tab();
    USHORT nCol = Col();
    r.Erase();
    if( nFlags & SCA_VALID )
        nFlags |= ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
    if( pDoc && (nFlags & SCA_VALID_TAB ) )
    {
        if ( nTab >= pDoc->GetTableCount() )
        {
            r = ScGlobal::GetRscString( STR_NOREF_STR );
            return;
        }
//      if( nFlags & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ) )
        if( nFlags & SCA_TAB_3D )
        {
            String aTabName;
            pDoc->GetName( nTab, aTabName );

            //  externe Referenzen (wie in ScCompiler::MakeTabStr)
            String aDoc;
            if ( aTabName.GetChar(0) == '\'' )
            {   // "'Doc'#Tab"
                xub_StrLen nPos, nLen = 1;
                while( (nPos = aTabName.Search( '\'', nLen ))
                        != STRING_NOTFOUND )
                    nLen = nPos + 1;
                if ( aTabName.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
                {
                    aDoc = aTabName.Copy( 0, nLen + 1 );
                    aTabName.Erase( 0, nLen + 1 );
                }
            }
            r += aDoc;

            if( nFlags & SCA_TAB_ABSOLUTE )
                r += '$';
            ScCompiler::CheckTabQuotes( aTabName );
            r += aTabName;
            r += '.';
        }
    }
    if( nFlags & SCA_VALID_COL )
    {
        if( nFlags & SCA_COL_ABSOLUTE )
            r += '$';
        if ( nCol < 26 )
            r += (sal_Unicode) ( 'A' + nCol );
        else
        {
            r += (sal_Unicode) ( 'A' + ( nCol / 26 ) - 1 );
            r += (sal_Unicode) ( 'A' + ( nCol % 26 ) );
        }
    }
    if( nFlags & SCA_VALID_ROW )
    {
        if ( nFlags & SCA_ROW_ABSOLUTE )
            r += '$';
        r += String::CreateFromInt32( Row()+1 );
    }
}


void ScRange::Format( String& r, USHORT nFlags, ScDocument* pDoc ) const
{
    if( !( nFlags & SCA_VALID ) )
        r = ScGlobal::GetRscString( STR_NOREF_STR );
    else
    {
        BOOL bOneTab = (aStart.Tab() == aEnd.Tab());
        if ( !bOneTab )
            nFlags |= SCA_TAB_3D;
        aStart.Format( r, nFlags, pDoc );
        if( aStart != aEnd )
        {
            String aName;
            nFlags = ( nFlags & SCA_VALID ) | ( ( nFlags >> 4 ) & 0x070F );
            if ( bOneTab )
                pDoc = NULL;
            else
                nFlags |= SCA_TAB_3D;
            aEnd.Format( aName, nFlags, pDoc );
            r += ':';
            r += aName;
        }
    }
}

BOOL ScAddress::Move( short dx, short dy, short dz, ScDocument* pDoc )
{
    short nMaxTab = pDoc ? pDoc->GetTableCount() : MAXTAB+1;
    dx = Col() + dx;
    dy = Row() + dy;
    dz = Tab() + dz;
    BOOL bValid = TRUE;
    if( dx < 0 )
        dx = 0, bValid = FALSE;
    else if( dx > MAXCOL )
        dx = MAXCOL, bValid =FALSE;
    if( dy < 0 )
        dy = 0, bValid = FALSE;
    else if( dy > MAXROW )
        dy = MAXROW, bValid =FALSE;
    if( dz < 0 )
        dz = 0, bValid = FALSE;
    else if( dz >= nMaxTab )
        dz = nMaxTab-1, bValid =FALSE;
    Set( dx, dy, dz );
    return bValid;
}

BOOL ScRange::Move( short dx, short dy, short dz, ScDocument* pDoc )
{
    // Einfahces &, damit beides ausgefuehrt wird!!
    return aStart.Move( dx, dy, dz, pDoc ) & aEnd.Move( dx, dy, dz, pDoc );
}


