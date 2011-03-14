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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>
#include <ctype.h>
#include <unotools/syslocale.hxx>

#include "global.hxx"
#include "rangeutl.hxx"
#include "rechead.hxx"
#include "compiler.hxx"
#include "paramisc.hxx"
#include "dpglobal.hxx"
#include "pivot.hxx"

#include "sc.hrc"
#include "globstr.hrc"

using ::std::vector;

// -----------------------------------------------------------------------

//------------------------------------------------------------------------
// struct ScImportParam:

ScImportParam::ScImportParam() :
    nCol1(0),
    nRow1(0),
    nCol2(0),
    nRow2(0),
    bImport(false),
    bNative(false),
    bSql(sal_True),
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

sal_Bool ScImportParam::operator==( const ScImportParam& rOther ) const
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

ScQueryEntry::ScQueryEntry() :
    bDoQuery(false),
    bQueryByString(false),
    bQueryByDate(false),
    nField(0),
    eOp(SC_EQUAL),
    eConnect(SC_AND),
    pStr(new String),
    nVal(0.0),
    pSearchParam(NULL),
    pSearchText(NULL)
{
}

ScQueryEntry::ScQueryEntry(const ScQueryEntry& r) :
    bDoQuery(r.bDoQuery),
    bQueryByString(r.bQueryByString),
    bQueryByDate(r.bQueryByDate),
    nField(r.nField),
    eOp(r.eOp),
    eConnect(r.eConnect),
    pStr(new String(*r.pStr)),
    nVal(r.nVal),
    pSearchParam(NULL),
    pSearchText(NULL)
{
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
    bQueryByDate    = r.bQueryByDate;
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
    bDoQuery        = false;
    bQueryByString  = false;
    bQueryByDate    = false;
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

sal_Bool ScQueryEntry::operator==( const ScQueryEntry& r ) const
{
    return bDoQuery         == r.bDoQuery
        && bQueryByString   == r.bQueryByString
        && bQueryByDate     == r.bQueryByDate
        && eOp              == r.eOp
        && eConnect         == r.eConnect
        && nField           == r.nField
        && nVal             == r.nVal
        && *pStr            == *r.pStr;
    //! pSearchParam und pSearchText nicht vergleichen
}

utl::TextSearch* ScQueryEntry::GetSearchTextPtr( sal_Bool bCaseSens )
{
    if ( !pSearchParam )
    {
        pSearchParam = new utl::SearchParam( *pStr, utl::SearchParam::SRCH_REGEXP,
            bCaseSens, false, false );
        pSearchText = new utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
    }
    return pSearchText;
}

//------------------------------------------------------------------------
// struct ScSubTotalParam:

ScSubTotalParam::ScSubTotalParam()
{
    for ( sal_uInt16 i=0; i<MAXSUBTOTAL; i++ )
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
        bRemoveOnly(r.bRemoveOnly),bReplace(r.bReplace),bPagebreak(r.bPagebreak),bCaseSens(r.bCaseSens),
        bDoSort(r.bDoSort),bAscending(r.bAscending),bUserDef(r.bUserDef),nUserIndex(r.nUserIndex),
        bIncludePattern(r.bIncludePattern)
{
    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = r.bGroupActive[i];
        nField[i]       = r.nField[i];

        if ( (r.nSubTotals[i] > 0) && r.pSubTotals[i] && r.pFunctions[i] )
        {
            nSubTotals[i] = r.nSubTotals[i];
            pSubTotals[i] = new SCCOL   [r.nSubTotals[i]];
            pFunctions[i] = new ScSubTotalFunc  [r.nSubTotals[i]];

            for (SCCOL j=0; j<r.nSubTotals[i]; j++)
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
    nCol1=nCol2= 0;
    nRow1=nRow2 = 0;
    nUserIndex = 0;
    bPagebreak=bCaseSens=bUserDef=bIncludePattern=bRemoveOnly = false;
    bAscending=bReplace=bDoSort = sal_True;

    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = false;
        nField[i]       = 0;

        if ( (nSubTotals[i] > 0) && pSubTotals[i] && pFunctions[i] )
        {
            for ( SCCOL j=0; j<nSubTotals[i]; j++ ) {
                pSubTotals[i][j] = 0;
                pFunctions[i][j] = SUBTOTAL_FUNC_NONE;
            }
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

    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        bGroupActive[i] = r.bGroupActive[i];
        nField[i]       = r.nField[i];
        nSubTotals[i]   = r.nSubTotals[i];

        if ( pSubTotals[i] ) delete [] pSubTotals[i];
        if ( pFunctions[i] ) delete [] pFunctions[i];

        if ( r.nSubTotals[i] > 0 )
        {
            pSubTotals[i] = new SCCOL   [r.nSubTotals[i]];
            pFunctions[i] = new ScSubTotalFunc  [r.nSubTotals[i]];

            for (SCCOL j=0; j<r.nSubTotals[i]; j++)
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

sal_Bool ScSubTotalParam::operator==( const ScSubTotalParam& rOther ) const
{
    sal_Bool bEqual =   (nCol1          == rOther.nCol1)
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
        bEqual = sal_True;
        for ( sal_uInt16 i=0; i<MAXSUBTOTAL && bEqual; i++ )
        {
            bEqual =   (bGroupActive[i] == rOther.bGroupActive[i])
                    && (nField[i]       == rOther.nField[i])
                    && (nSubTotals[i]   == rOther.nSubTotals[i]);

            if ( bEqual && (nSubTotals[i] > 0) )
            {
                bEqual = (pSubTotals != NULL) && (pFunctions != NULL);

                for (SCCOL j=0; (j<nSubTotals[i]) && bEqual; j++)
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

void ScSubTotalParam::SetSubTotals( sal_uInt16                  nGroup,
                                    const SCCOL*            ptrSubTotals,
                                    const ScSubTotalFunc*   ptrFunctions,
                                    sal_uInt16                  nCount )
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

        pSubTotals[nGroup] = new SCCOL      [nCount];
        pFunctions[nGroup] = new ScSubTotalFunc [nCount];
        nSubTotals[nGroup] = static_cast<SCCOL>(nCount);

        for ( sal_uInt16 i=0; i<nCount; i++ )
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
        nCol(r.nCol),nRow(r.nRow),nTab(r.nTab),
        eFunction(r.eFunction),nDataAreaCount(0),
        ppDataAreas( NULL ),
        bByCol(r.bByCol),bByRow(r.bByRow),bReferenceData(r.bReferenceData)
{
    if ( r.nDataAreaCount > 0 )
    {
        nDataAreaCount = r.nDataAreaCount;
        ppDataAreas = new ScArea*[nDataAreaCount];
        for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
            ppDataAreas[i] = new ScArea( *(r.ppDataAreas[i]) );
    }
}

//------------------------------------------------------------------------

ScConsolidateParam::~ScConsolidateParam()
{
    ClearDataAreas();
}

//------------------------------------------------------------------------

void ScConsolidateParam::ClearDataAreas()
{
    if ( ppDataAreas )
    {
        for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
            delete ppDataAreas[i];
        delete [] ppDataAreas;
        ppDataAreas = NULL;
    }
    nDataAreaCount = 0;
}

//------------------------------------------------------------------------

void ScConsolidateParam::Clear()
{
    ClearDataAreas();

    nCol = 0;
    nRow = 0;
    nTab = 0;
    bByCol = bByRow = bReferenceData    = false;
    eFunction                           = SUBTOTAL_FUNC_SUM;
}

//------------------------------------------------------------------------

ScConsolidateParam& ScConsolidateParam::operator=( const ScConsolidateParam& r )
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

sal_Bool ScConsolidateParam::operator==( const ScConsolidateParam& r ) const
{
    sal_Bool bEqual =   (nCol           == r.nCol)
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
        for ( sal_uInt16 i=0; i<nDataAreaCount && bEqual; i++ )
            bEqual = *(ppDataAreas[i]) == *(r.ppDataAreas[i]);

    return bEqual;
}

//------------------------------------------------------------------------

void ScConsolidateParam::SetAreas( ScArea* const* ppAreas, sal_uInt16 nCount )
{
    ClearDataAreas();
    if ( ppAreas && nCount > 0 )
    {
        ppDataAreas = new ScArea*[nCount];
        for ( sal_uInt16 i=0; i<nCount; i++ )
            ppDataAreas[i] = new ScArea( *(ppAreas[i]) );
        nDataAreaCount = nCount;
    }
}

// -----------------------------------------------------------------------

PivotField::PivotField( SCsCOL nNewCol, sal_uInt16 nNewFuncMask ) :
    nCol( nNewCol ),
    nFuncMask( nNewFuncMask ),
    nFuncCount( 0 )
{
}

PivotField::PivotField( const PivotField& r ) :
    nCol(r.nCol), nFuncMask(r.nFuncMask), nFuncCount(r.nFuncCount), maFieldRef(r.maFieldRef)
{
}

bool PivotField::operator==( const PivotField& r ) const
{
    return (nCol                            == r.nCol)
        && (nFuncMask                       == r.nFuncMask)
        && (nFuncCount                      == r.nFuncCount)
        && (maFieldRef.ReferenceType        == r.maFieldRef.ReferenceType)
        && (maFieldRef.ReferenceField       == r.maFieldRef.ReferenceField)
        && (maFieldRef.ReferenceItemType    == r.maFieldRef.ReferenceItemType)
        && (maFieldRef.ReferenceItemName    == r.maFieldRef.ReferenceItemName);
}

//------------------------------------------------------------------------
// struct ScPivotParam:

ScPivotParam::ScPivotParam()
    :   nCol(0), nRow(0), nTab(0),
        bIgnoreEmptyRows(false), bDetectCategories(false),
        bMakeTotalCol(true), bMakeTotalRow(true)
{
}

//------------------------------------------------------------------------

ScPivotParam::ScPivotParam( const ScPivotParam& r )
    :   nCol( r.nCol ), nRow( r.nRow ), nTab( r.nTab ),
        maPageFields(r.maPageFields),
        maColFields(r.maColFields),
        maRowFields(r.maRowFields),
        maDataFields(r.maDataFields),
        bIgnoreEmptyRows(r.bIgnoreEmptyRows),
        bDetectCategories(r.bDetectCategories),
        bMakeTotalCol(r.bMakeTotalCol),
        bMakeTotalRow(r.bMakeTotalRow)
{
    SetLabelData(r.maLabelArray);
}

//------------------------------------------------------------------------

ScPivotParam::~ScPivotParam()
{
}

//------------------------------------------------------------------------

void ScPivotParam::ClearPivotArrays()
{
    maPageFields.clear();
    maColFields.clear();
    maRowFields.clear();
    maDataFields.clear();
}

void ScPivotParam::SetLabelData(const vector<ScDPLabelDataRef>& r)
{
    vector<ScDPLabelDataRef> aNewArray;
    aNewArray.reserve(r.size());
    for (vector<ScDPLabelDataRef>::const_iterator itr = r.begin(), itrEnd = r.end();
          itr != itrEnd; ++itr)
    {
        ScDPLabelDataRef p(new ScDPLabelData(**itr));
        aNewArray.push_back(p);
    }
    maLabelArray.swap(aNewArray);
}

//------------------------------------------------------------------------

ScPivotParam& ScPivotParam::operator=( const ScPivotParam& r )
{
    nCol              = r.nCol;
    nRow              = r.nRow;
    nTab              = r.nTab;
    bIgnoreEmptyRows  = r.bIgnoreEmptyRows;
    bDetectCategories = r.bDetectCategories;
    bMakeTotalCol     = r.bMakeTotalCol;
    bMakeTotalRow     = r.bMakeTotalRow;

    maPageFields = r.maPageFields;
    maColFields  = r.maColFields;
    maRowFields  = r.maRowFields;
    maDataFields = r.maDataFields;
    SetLabelData(r.maLabelArray);
    return *this;
}

//------------------------------------------------------------------------

bool ScPivotParam::operator==( const ScPivotParam& r ) const
{
    sal_Bool bEqual =   (nCol       == r.nCol)
                 && (nRow       == r.nRow)
                 && (nTab       == r.nTab)
                 && (bIgnoreEmptyRows  == r.bIgnoreEmptyRows)
                 && (bDetectCategories == r.bDetectCategories)
                 && (bMakeTotalCol == r.bMakeTotalCol)
                 && (bMakeTotalRow == r.bMakeTotalRow)
                 && (maLabelArray.size() == r.maLabelArray.size())
                 && maPageFields == r.maPageFields
                 && maColFields == r.maColFields
                 && maRowFields == r.maRowFields
                 && maDataFields == r.maDataFields;

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

ScSolveParam& ScSolveParam::operator=( const ScSolveParam& r )
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

sal_Bool ScSolveParam::operator==( const ScSolveParam& r ) const
{
    sal_Bool bEqual =   (aRefFormulaCell  == r.aRefFormulaCell)
                 && (aRefVariableCell == r.aRefVariableCell);

    if ( bEqual )
    {
        if ( !pStrTargetVal && !r.pStrTargetVal )
            bEqual = sal_True;
        else if ( !pStrTargetVal || !r.pStrTargetVal )
            bEqual = false;
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

ScTabOpParam::ScTabOpParam( const ScRefAddress& rFormulaCell,
                            const ScRefAddress& rFormulaEnd,
                            const ScRefAddress& rRowCell,
                            const ScRefAddress& rColCell,
                                  sal_uInt8      nMd)
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

sal_Bool ScTabOpParam::operator==( const ScTabOpParam& r ) const
{
    return (        (aRefFormulaCell == r.aRefFormulaCell)
                 && (aRefFormulaEnd  == r.aRefFormulaEnd)
                 && (aRefRowCell     == r.aRefRowCell)
                 && (aRefColCell     == r.aRefColCell)
                 && (nMode           == r.nMode) );
}

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
        aAbsName = aObj.smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
        //  returned string must be encoded because it's used directly to create SfxMedium
    }
    else
    {
        const SfxMedium* pMedium = pShell->GetMedium();
        if ( pMedium )
        {
            bool bWasAbs = true;
            aAbsName = pMedium->GetURLObject().smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
        }
        else
        {   // This can't happen, but ...
            // just to be sure to have the same encoding
            INetURLObject aObj;
            aObj.SetSmartURL( aAbsName );
            aAbsName = aObj.GetMainURL(INetURLObject::NO_DECODE);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
