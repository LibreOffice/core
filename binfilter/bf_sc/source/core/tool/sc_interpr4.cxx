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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifdef RS6000

#pragma options FLTTRAP
#include <fptrap.h>
#include <fpxcp.h>

#elif defined ( MAC )

#include <MAC_START.h>
#include <fp.h>
#include <MAC_END.h>
#include <ctype.h>

#elif defined ( SOLARIS)

#include <ieeefp.h>
#include <ctype.h>

#elif defined ( ICC )

#include <ctype.h>

#endif


#include <bf_sfx2/app.hxx>
#include "bf_basic/sbmeth.hxx"
#include "bf_basic/sbmod.hxx"
#include "bf_basic/sbstar.hxx"
#include "bf_basic/sbx.hxx"
#include <bf_svtools/zforlist.hxx>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <float.h>


using namespace ::com::sun::star;

#include "interpre.hxx"
#include "dbcolect.hxx"
#include "addincol.hxx"
#include "dociter.hxx"
#include "scmatrix.hxx"
#include "bf_sc.hrc"
#include "cellsuno.hxx"
#include "optuno.hxx"
#include "rangeseq.hxx"
#include "addinlis.hxx"
namespace binfilter {

// Implementiert in ui\miscdlgs\teamdlg.cxx

extern void ShowTheTeam();

extern BOOL bOderSo; // in GLOBAL.CXX

//-----------------------------statische Daten-----------------

USHORT ScInterpreter::nGlobalError = 0; // fuer matherr

#if SC_SPEW_ENABLED
ScSpew ScInterpreter::theSpew;
#endif

//-------------------------------------------------------------------------
// Funktionen für den Zugriff auf das Document
//-------------------------------------------------------------------------


void ScInterpreter::ReplaceCell( ScAddress& rPos )
{
    ScInterpreterTableOpParams* pTOp = pDok->aTableOpList.First();
    while (pTOp)
    {
        if ( rPos == pTOp->aOld1 )
        {
            rPos = pTOp->aNew1;
            return ;
        }
        else if ( rPos == pTOp->aOld2 )
        {
            rPos = pTOp->aNew2;
            return ;
        }
        else
            pTOp = pDok->aTableOpList.Next();
    }
}


void ScInterpreter::ReplaceCell( USHORT& rCol, USHORT& rRow, USHORT& rTab )
{
    ScAddress aCellPos( rCol, rRow, rTab );
    ScInterpreterTableOpParams* pTOp = pDok->aTableOpList.First();
    while (pTOp)
    {
        if ( aCellPos == pTOp->aOld1 )
        {
            rCol = pTOp->aNew1.Col();
            rRow = pTOp->aNew1.Row();
            rTab = pTOp->aNew1.Tab();
            return ;
        }
        else if ( aCellPos == pTOp->aOld2 )
        {
            rCol = pTOp->aNew2.Col();
            rRow = pTOp->aNew2.Row();
            rTab = pTOp->aNew2.Tab();
            return ;
        }
        else
            pTOp = pDok->aTableOpList.Next();
    }
}


BOOL ScInterpreter::IsTableOpInRange( const ScRange& rRange )
{
    if ( rRange.aStart == rRange.aEnd )
        return FALSE;	// not considered to be a range in TableOp sense

    // we can't replace a single cell in a range
    ScInterpreterTableOpParams* pTOp = pDok->aTableOpList.First();
    while (pTOp)
    {
        if ( rRange.In( pTOp->aOld1 ) )
            return TRUE;
        if ( rRange.In( pTOp->aOld2 ) )
            return TRUE;
        pTOp = pDok->aTableOpList.Next();
    }
    return FALSE;
}


ULONG ScInterpreter::GetCellNumberFormat( const ScAddress& rPos, const ScBaseCell* pCell)
{
    ULONG nFormat;
    USHORT nErr;
    if ( pCell )
    {
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            nErr = ((ScFormulaCell*)pCell)->GetErrCode();
        else
            nErr = 0;
        nFormat = pDok->GetNumberFormat( rPos );
        if ( pCell->GetCellType() == CELLTYPE_FORMULA
          && ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0) )
            nFormat = ((ScFormulaCell*)pCell)->GetStandardFormat( *pFormatter,
                nFormat );
    }
    else
    {
        nFormat = pDok->GetNumberFormat( rPos );
        nErr = 0;
    }
    SetError(nErr);
    return nFormat;
}


// nur ValueCell, Formelzellen speichern das Ergebnis bereits gerundet
/*N*/ double ScInterpreter::GetValueCellValue( const ScAddress& rPos, const ScValueCell* pCell )
/*N*/ {
/*N*/ 	double fVal = pCell->GetValue();
/*N*/ 	if ( bCalcAsShown && fVal != 0.0 )
/*N*/ 	{
/*?*/ 		ULONG nFormat = pDok->GetNumberFormat( rPos );
/*?*/ 		fVal = pDok->RoundValueAsShown( fVal, nFormat );
/*N*/ 	}
/*N*/ 	return fVal;
/*N*/ }


/*N*/ double ScInterpreter::GetCellValue( const ScAddress& rPos, const ScBaseCell* pCell )
/*N*/ {
/*N*/ 	USHORT nErr = nGlobalError;
/*N*/ 	nGlobalError = 0;
/*N*/ 	double nVal = GetCellValueOrZero( rPos, pCell );
/*N*/     if ( !nGlobalError || nGlobalError == errCellNoValue )
/*N*/ 		nGlobalError = nErr;
/*N*/ 	return nVal;
/*N*/ }


/*N*/ double ScInterpreter::GetCellValueOrZero( const ScAddress& rPos, const ScBaseCell* pCell )
/*N*/ {
/*N*/ 	double fValue;
/*N*/ 	if (pCell)
/*N*/ 	{
/*N*/ 		CellType eType = pCell->GetCellType();
/*N*/ 		switch ( eType )
/*N*/ 		{
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 			{
/*N*/ 				ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
/*N*/ 				USHORT nErr = pFCell->GetErrCode();
/*N*/ 				if( !nErr )
/*N*/ 				{
/*N*/ 					if (pFCell->IsValue())
/*N*/ 					{
/*N*/ 						fValue = pFCell->GetValue();
/*N*/ 						pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex,
/*N*/ 							rPos, *pFCell );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/                         SetError(errCellNoValue);
/*N*/ 						fValue = 0.0;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					fValue = 0.0;
/*N*/ 					SetError(nErr);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case CELLTYPE_VALUE:
/*N*/ 			{
/*N*/ 				fValue = ((ScValueCell*)pCell)->GetValue();
/*N*/ 				nCurFmtIndex = pDok->GetNumberFormat( rPos );
/*N*/ 				nCurFmtType = pFormatter->GetType( nCurFmtIndex );
/*N*/ 				if ( bCalcAsShown && fValue != 0.0 )
/*?*/ 					fValue = pDok->RoundValueAsShown( fValue, nCurFmtIndex );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 			case  CELLTYPE_STRING:
/*N*/ 			case  CELLTYPE_EDIT:
/*?*/ #if 0
/*?*/ // Xcl does it, but SUM(A1:A2) differs from A1+A2. No good.
/*?*/ 			{
/*?*/ 				String aStr;
/*?*/ 				if ( eType == CELLTYPE_STRING )
/*?*/ 					((ScStringCell*)pCell)->GetString( aStr );
/*?*/ 				else
/*?*/ 					((ScEditCell*)pCell)->GetString( aStr );
/*?*/ 				ULONG nFIndex = 0;					// damit default Land/Spr.
/*?*/ 				if ( !pFormatter->IsNumberFormat( aStr, nFIndex, fValue ) )
/*?*/ 				{
/*?*/ 					SetError(errNoValue);
/*?*/ 					fValue = 0.0;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ #endif
/*N*/ 			default:
/*N*/                 SetError(errCellNoValue);
/*N*/ 				fValue = 0.0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		fValue = 0.0;
/*N*/ 	return fValue;
/*N*/ }


/*N*/ void ScInterpreter::GetCellString( String& rStr, const ScBaseCell* pCell )
/*N*/ {
/*N*/ 	USHORT nErr = 0;
/*N*/ 	if (pCell)
/*N*/ 	{
/*N*/ 		switch (pCell->GetCellType())
/*N*/ 		{
/*N*/ 			case CELLTYPE_STRING:
/*N*/ 				((ScStringCell*) pCell)->GetString(rStr);
/*N*/ 			break;
/*?*/ 			case CELLTYPE_EDIT:
/*?*/ 				((ScEditCell*) pCell)->GetString(rStr);
/*?*/ 			break;
/*N*/ 			case CELLTYPE_FORMULA:
/*N*/ 			{
/*N*/ 				ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
/*N*/ 				nErr = pFCell->GetErrCode();
/*N*/ 				if (pFCell->IsValue())
/*N*/ 				{
/*?*/ 					double fVal = pFCell->GetValue();
/*?*/ 					ULONG nIndex = pFormatter->GetStandardFormat(
/*?*/ 										NUMBERFORMAT_NUMBER,
/*?*/ 										ScGlobal::eLnge);
/*?*/ 					pFormatter->GetInputLineString(fVal, nIndex, rStr);
/*N*/ 				}
/*N*/ 				else
/*N*/ 					pFCell->GetString(rStr);
/*N*/ 			}
/*N*/ 			break;
/*?*/ 			case CELLTYPE_VALUE:
/*?*/ 			{
/*?*/ 				double fVal = ((ScValueCell*) pCell)->GetValue();
/*?*/ 				ULONG nIndex = pFormatter->GetStandardFormat(
/*?*/ 										NUMBERFORMAT_NUMBER,
/*?*/ 										ScGlobal::eLnge);
/*?*/ 				pFormatter->GetInputLineString(fVal, nIndex, rStr);
/*?*/ 			}
/*?*/ 			break;
/*N*/ 			default:
/*N*/ 				rStr = ScGlobal::GetEmptyString();
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*?*/ 		rStr = ScGlobal::GetEmptyString();
/*N*/ 	SetError(nErr);
/*N*/ }


USHORT ScInterpreter::GetCellErrCode( const ScBaseCell* pCell )
{
    if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
        return ((ScFormulaCell*)pCell)->GetErrCode();
    return 0;
}


BOOL ScInterpreter::CreateDoubleArr(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2, BYTE* pCellArr)
{
    USHORT nCount = 0;
    USHORT* p = (USHORT*) pCellArr;
    *p++ = nCol1;
    *p++ = nRow1;
    *p++ = nTab1;
    *p++ = nCol2;
    *p++ = nRow2;
    *p++ = nTab2;
    USHORT* pCount = p;
    *p++ = 0;
    USHORT nPos = 14;
    USHORT nTab = nTab1;
    ScAddress aAdr;
    while (nTab <= nTab2)
    {
        aAdr.SetTab( nTab );
        USHORT nRow = nRow1;
        while (nRow <= nRow2)
        {
            aAdr.SetRow( nRow );
            USHORT nCol = nCol1;
            while (nCol <= nCol2)
            {
                aAdr.SetCol( nCol );
                ScBaseCell* pCell = pDok->GetCell( aAdr );
                if (pCell)
                {
                    USHORT	nErr = 0;
                    double	nVal = 0.0;
                    BOOL	bOk = TRUE;
                    switch ( pCell->GetCellType() )
                    {
                        case CELLTYPE_VALUE :
                            nVal = GetValueCellValue( aAdr, (ScValueCell*)pCell );
                            break;
                        case CELLTYPE_FORMULA :
                            if (((ScFormulaCell*)pCell)->IsValue())
                            {
                                nErr = ((ScFormulaCell*)pCell)->GetErrCode();
                                nVal = ((ScFormulaCell*)pCell)->GetValue();
                            }
                            else
                                bOk = FALSE;
                            break;
                        default :
                            bOk = FALSE;
                            break;
                    }
                    if (bOk)
                    {
                        if ((nPos + (4 * sizeof(USHORT)) + sizeof(double)) > MAXARRSIZE)
                            return FALSE;
                        *p++ = nCol;
                        *p++ = nRow;
                        *p++ = nTab;
                        *p++ = nErr;
                        memcpy( p, &nVal, sizeof(double));
                        nPos += 8 + sizeof(double);
                        p = (USHORT*) ( pCellArr + nPos );
                        nCount++;
                    }
                }
                nCol++;
            }
            nRow++;
        }
        nTab++;
    }
    *pCount = nCount;
    return TRUE;
}


BOOL ScInterpreter::CreateStringArr(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    BYTE* pCellArr)
{
    USHORT nCount = 0;
    USHORT* p = (USHORT*) pCellArr;
    *p++ = nCol1;
    *p++ = nRow1;
    *p++ = nTab1;
    *p++ = nCol2;
    *p++ = nRow2;
    *p++ = nTab2;
    USHORT* pCount = p;
    *p++ = 0;
    USHORT nPos = 14;
    USHORT nTab = nTab1;
    while (nTab <= nTab2)
    {
        USHORT nRow = nRow1;
        while (nRow <= nRow2)
        {
            USHORT nCol = nCol1;
            while (nCol <= nCol2)
            {
                ScBaseCell* pCell;
                pDok->GetCell(nCol, nRow, nTab, pCell);
                if (pCell)
                {
                    String	aStr;
                    USHORT	nErr = 0;
                    BOOL	bOk = TRUE;
                    switch ( pCell->GetCellType() )
                    {
                        case CELLTYPE_STRING :
                            ((ScStringCell*)pCell)->GetString(aStr);
                            break;
                        case CELLTYPE_EDIT :
                            ((ScEditCell*)pCell)->GetString(aStr);
                            break;
                        case CELLTYPE_FORMULA :
                            if (!((ScFormulaCell*)pCell)->IsValue())
                            {
                                nErr = ((ScFormulaCell*)pCell)->GetErrCode();
                                ((ScFormulaCell*)pCell)->GetString(aStr);
                            }
                            else
                                bOk = FALSE;
                            break;
                        default :
                            bOk = FALSE;
                            break;
                    }
                    if (bOk)
                    {
                        ByteString aTmp( aStr, osl_getThreadTextEncoding() );
                        // In case the xub_StrLen will be longer than USHORT
                        // one day, and room for pad byte check.
                        if ( aTmp.Len() > ((USHORT)(~0)) - 2 )
                            return FALSE;
                        // Append a 0-pad-byte if string length is not even
                        //! MUST be USHORT and not xub_StrLen
                        USHORT nStrLen = (USHORT) aTmp.Len();
                        USHORT nLen = ( nStrLen + 2 ) & ~1;

                        if (((ULONG)nPos + (5 * sizeof(USHORT)) + nLen) > MAXARRSIZE)
                            return FALSE;
                        *p++ = nCol;
                        *p++ = nRow;
                        *p++ = nTab;
                        *p++ = nErr;
                        *p++ = nLen;
                        memcpy( p, aTmp.GetBuffer(), nStrLen + 1);
                        nPos += 10 + nStrLen + 1;
                        BYTE* q = ( pCellArr + nPos );
                        if( !nStrLen & 1 )
                            *q++ = 0, nPos++;
                        p = (USHORT*) ( pCellArr + nPos );
                        nCount++;
                    }
                }
                nCol++;
            }
            nRow++;
        }
        nTab++;
    }
    *pCount = nCount;
    return TRUE;
}


BOOL ScInterpreter::CreateCellArr(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                  USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                  BYTE* pCellArr)
{
    USHORT nCount = 0;
    USHORT* p = (USHORT*) pCellArr;
    *p++ = nCol1;
    *p++ = nRow1;
    *p++ = nTab1;
    *p++ = nCol2;
    *p++ = nRow2;
    *p++ = nTab2;
    USHORT* pCount = p;
    *p++ = 0;
    USHORT nPos = 14;
    USHORT nTab = nTab1;
    ScAddress aAdr;
    while (nTab <= nTab2)
    {
        aAdr.SetTab( nTab );
        USHORT nRow = nRow1;
        while (nRow <= nRow2)
        {
            aAdr.SetRow( nRow );
            USHORT nCol = nCol1;
            while (nCol <= nCol2)
            {
                aAdr.SetCol( nCol );
                ScBaseCell* pCell = pDok->GetCell( aAdr );
                if (pCell)
                {
                    USHORT	nErr = 0;
                    USHORT	nType = 0; // 0 = Zahl; 1 = String
                    double	nVal = 0.0;
                    String	aStr;
                    BOOL	bOk = TRUE;
                    switch ( pCell->GetCellType() )
                    {
                        case CELLTYPE_STRING :
                            ((ScStringCell*)pCell)->GetString(aStr);
                            nType = 1;
                            break;
                        case CELLTYPE_EDIT :
                            ((ScEditCell*)pCell)->GetString(aStr);
                            nType = 1;
                            break;
                        case CELLTYPE_VALUE :
                            nVal = GetValueCellValue( aAdr, (ScValueCell*)pCell );
                            break;
                        case CELLTYPE_FORMULA :
                            nErr = ((ScFormulaCell*)pCell)->GetErrCode();
                            if (((ScFormulaCell*)pCell)->IsValue())
                                nVal = ((ScFormulaCell*)pCell)->GetValue();
                            else
                                ((ScFormulaCell*)pCell)->GetString(aStr);
                            break;
                        default :
                            bOk = FALSE;
                            break;
                    }
                    if (bOk)
                    {
                        if ((nPos + (5 * sizeof(USHORT))) > MAXARRSIZE)
                            return FALSE;
                        *p++ = nCol;
                        *p++ = nRow;
                        *p++ = nTab;
                        *p++ = nErr;
                        *p++ = nType;
                        nPos += 10;
                        if (nType == 0)
                        {
                            if ((nPos + sizeof(double)) > MAXARRSIZE)
                                return FALSE;
                            memcpy( p, &nVal, sizeof(double));
                            nPos += sizeof(double);
                        }
                        else
                        {
                            ByteString aTmp( aStr, osl_getThreadTextEncoding() );
                            // In case the xub_StrLen will be longer than USHORT
                            // one day, and room for pad byte check.
                            if ( aTmp.Len() > ((USHORT)(~0)) - 2 )
                                return FALSE;
                            // Append a 0-pad-byte if string length is not even
                            //! MUST be USHORT and not xub_StrLen
                            USHORT nStrLen = (USHORT) aTmp.Len();
                            USHORT nLen = ( nStrLen + 2 ) & ~1;
                            if ( ((ULONG)nPos + 2 + nLen) > MAXARRSIZE)
                                return FALSE;
                            *p++ = nLen;
                            memcpy( p, aTmp.GetBuffer(), nStrLen + 1);
                            nPos += 2 + nStrLen + 1;
                            BYTE* q = ( pCellArr + nPos );
                            if( !nStrLen & 1 )
                                *q++ = 0, nPos++;
                        }
                        nCount++;
                        p = (USHORT*) ( pCellArr + nPos );
                    }
                }
                nCol++;
            }
            nRow++;
        }
        nTab++;
    }
    *pCount = nCount;
    return TRUE;
}

//-----------------------------------------------------------------------------
// Stackoperationen
//-----------------------------------------------------------------------------

// vorher wird ggf. ein Temp-Token geloescht.


/*N*/ void ScInterpreter::Push( ScToken& r )
/*N*/ {
/*N*/ 	if ( sp >= MAXSTACK )
/*N*/ 		SetError( errStackOverflow );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nCurFmtType = NUMBERFORMAT_UNDEFINED;
/*N*/ 		r.IncRef();
/*N*/ 		if( sp >= maxsp )
/*N*/ 			maxsp = sp + 1;
/*N*/ 		else
/*N*/ 			pStack[ sp ]->DecRef();
/*N*/ 		pStack[ sp ] = (ScToken*) &r;
/*N*/ 		pErrorStack[ sp ] = nGlobalError;
/*N*/ 		++sp;
/*N*/ 	}
/*N*/ }

// Schlichtes Wegwerfen von TOS


/*N*/ void ScInterpreter::Pop()
/*N*/ {
/*N*/ 	if( sp )
/*N*/ 		sp--;
/*N*/ 	else
/*?*/ 		SetError(errUnknownStackVariable);
/*N*/ }

// Schlichtes Wegwerfen von TOS mit Fehlercode setzen, fuer ocIsError etc.


void ScInterpreter::PopError()
{
    if( sp )
    {
        sp--;
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
    }
    else
        SetError(errUnknownStackVariable);
}


/*N*/ void ScInterpreter::PushTempToken( const ScToken& r )
/*N*/ {
/*N*/ 	if ( sp >= MAXSTACK )
/*?*/ 		SetError( errStackOverflow );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ScToken* p = r.Clone();
/*N*/ 		p->IncRef();
/*N*/ 		if( sp >= maxsp )
/*N*/ 			maxsp = sp + 1;
/*N*/ 		else
/*?*/ 			pStack[ sp ]->DecRef();
/*N*/ 		pStack[ sp ] = p;
/*N*/ 		pErrorStack[ sp ] = nGlobalError;
/*N*/ 		++sp;
/*N*/ 	}
/*N*/ }


//! The Token had to be allocated with `new' and must not be used after this
//! call because eventually it gets deleted in case of a errStackOverflow if
//! no RefCount was set!
/*N*/ void ScInterpreter::PushTempToken( ScToken* p )
/*N*/ {
/*N*/     p->IncRef();
/*N*/ 	if ( sp >= MAXSTACK )
/*N*/ 	{
/*?*/ 		SetError( errStackOverflow );
/*?*/         p->DecRef();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( sp >= maxsp )
/*N*/ 			maxsp = sp + 1;
/*N*/ 		else
/*N*/ 			pStack[ sp ]->DecRef();
/*N*/         pStack[ sp ] = p;
/*N*/ 		pErrorStack[ sp ] = nGlobalError;
/*N*/ 		++sp;
/*N*/ 	}
/*N*/ }


/*N*/ double ScInterpreter::PopDouble()
/*N*/ {
/*N*/ 	nCurFmtType = NUMBERFORMAT_NUMBER;
/*N*/ 	nCurFmtIndex = 0;
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		--sp;
/*N*/ 		ScToken* p = pStack[ sp ];
/*N*/ 		if ( !nGlobalError )
/*N*/ 			nGlobalError = pErrorStack[ sp ];
/*N*/ 		if( p->GetType() == svDouble )
/*N*/ 			return p->GetDouble();
/*N*/ 		else if( p->GetType() == svMissing )
/*N*/ 			return 0.0;
/*N*/ 	}
/*N*/ 	SetError(errUnknownStackVariable);
/*N*/ 	return 0.0;
/*N*/ }


BYTE ScInterpreter::PopByte()
{
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svByte )
            return p->GetByte();
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownStackVariable);
    return 0;
}


/*N*/ const String& ScInterpreter::PopString()
/*N*/ {
/*N*/ 	nCurFmtType = NUMBERFORMAT_TEXT;
/*N*/ 	nCurFmtIndex = 0;
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		--sp;
/*N*/ 		ScToken* p = pStack[ sp ];
/*N*/ 		if ( !nGlobalError )
/*N*/ 			nGlobalError = pErrorStack[ sp ];
/*N*/ 		if( p->GetType() == svString )
/*N*/ 			return p->GetString();
/*N*/ 		else if( p->GetType() == svMissing )
/*N*/ 			return EMPTY_STRING;
/*N*/ 	}
/*N*/ 	SetError(errUnknownStackVariable);
/*N*/ 	return EMPTY_STRING;
/*N*/ }


void ScInterpreter::PopSingleRef(USHORT& rCol, USHORT &rRow, USHORT& rTab)
{
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svSingleRef )
        {
            const SingleRefData& rRef = p->GetSingleRef();
            if ( rRef.IsColRel() )
                rCol = aPos.Col() + rRef.nRelCol;
            else
                rCol = rRef.nCol;
            if ( rRef.IsRowRel() )
                rRow = aPos.Row() + rRef.nRelRow;
            else
                rRow = rRef.nRow;
            if ( rRef.IsTabRel() )
                rTab = aPos.Tab() + rRef.nRelTab;
            else
                rTab = rRef.nTab;
            if( rCol < 0 || rCol > MAXCOL || rRef.IsColDeleted() )
                SetError( errNoRef ), rCol = 0;
            if( rRow < 0 || rRow > MAXROW || rRef.IsRowDeleted() )
                SetError( errNoRef ), rRow = 0;
            if( rTab < 0 || rTab >= pDok->GetTableCount() || rRef.IsTabDeleted() )
                SetError( errNoRef ), rTab = 0;
            if ( pDok->aTableOpList.Count() > 0 )
                ReplaceCell( rCol, rRow, rTab );
            return;
        }
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownStackVariable);
}


/*N*/ void ScInterpreter::PopSingleRef( ScAddress& rAdr )
/*N*/ {
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		--sp;
/*N*/ 		ScToken* p = pStack[ sp ];
/*N*/ 		if ( !nGlobalError )
/*N*/ 			nGlobalError = pErrorStack[ sp ];
/*N*/ 		if( p->GetType() == svSingleRef )
/*N*/ 		{
/*N*/ 			short nCol, nRow, nTab;
/*N*/ 			const SingleRefData& rRef = p->GetSingleRef();
/*N*/ 			if ( rRef.IsColRel() )
/*N*/ 				nCol = aPos.Col() + rRef.nRelCol;
/*N*/ 			else
/*N*/ 				nCol = rRef.nCol;
/*N*/ 			if ( rRef.IsRowRel() )
/*N*/ 				nRow = aPos.Row() + rRef.nRelRow;
/*N*/ 			else
/*N*/ 				nRow = rRef.nRow;
/*N*/ 			if ( rRef.IsTabRel() )
/*N*/ 				nTab = aPos.Tab() + rRef.nRelTab;
/*N*/ 			else
/*N*/ 				nTab = rRef.nTab;
/*N*/ 			if( nCol < 0 || nCol > MAXCOL || rRef.IsColDeleted() )
/*N*/ 				SetError( errNoRef ), nCol = 0;
/*N*/ 			if( nRow < 0 || nRow > MAXROW || rRef.IsRowDeleted() )
/*N*/ 				SetError( errNoRef ), nRow = 0;
/*N*/ 			if( nTab < 0 || nTab >= pDok->GetTableCount() || rRef.IsTabDeleted() )
/*N*/ 				SetError( errNoRef ), nTab = 0;
/*N*/ 			rAdr.Set( (USHORT)nCol, (USHORT)nRow, (USHORT)nTab );
/*N*/ 			if ( pDok->aTableOpList.Count() > 0 )
/*?*/ 				ReplaceCell( rAdr );
/*N*/ 			return;
/*N*/ 		}
/*N*/ 		else if( p->GetType() == svMissing )
/*N*/ 			SetError( errIllegalParameter );
/*N*/ 	}
/*N*/ 	SetError(errUnknownStackVariable);
/*N*/ }


/*N*/ void ScInterpreter::PopDoubleRef(USHORT& rCol1, USHORT &rRow1, USHORT& rTab1,
/*N*/ 								 USHORT& rCol2, USHORT &rRow2, USHORT& rTab2,
/*N*/ 								 BOOL bDontCheckForTableOp )
/*N*/ {
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		--sp;
/*N*/ 		ScToken* p = pStack[ sp ];
/*N*/ 		if ( !nGlobalError )
/*N*/ 			nGlobalError = pErrorStack[ sp ];
/*N*/ 		if( p->GetType() == svDoubleRef )
/*N*/ 		{
/*N*/ 			const ComplRefData& rCRef = p->GetDoubleRef();
/*N*/ 			USHORT nMaxTab = pDok->GetTableCount();
/*N*/ 			{
/*N*/ 				const SingleRefData& rRef = rCRef.Ref1;
/*N*/ 				if ( rRef.IsColRel() )
/*N*/ 					rCol1 = aPos.Col() + rRef.nRelCol;
/*N*/ 				else
/*N*/ 					rCol1 = rRef.nCol;
/*N*/ 				if ( rRef.IsRowRel() )
/*N*/ 					rRow1 = aPos.Row() + rRef.nRelRow;
/*N*/ 				else
/*N*/ 					rRow1 = rRef.nRow;
/*N*/ 				if ( rRef.IsTabRel() )
/*N*/ 					rTab1 = aPos.Tab() + rRef.nRelTab;
/*N*/ 				else
/*N*/ 					rTab1 = rRef.nTab;
/*N*/ 				if( rCol1 < 0 || rCol1 > MAXCOL || rRef.IsColDeleted() )
/*N*/ 					SetError( errNoRef ), rCol1 = 0;
/*N*/ 				if( rRow1 < 0 || rRow1 > MAXROW || rRef.IsRowDeleted() )
/*N*/ 					SetError( errNoRef ), rRow1 = 0;
/*N*/ 				if( rTab1 < 0 || rTab1 >= nMaxTab || rRef.IsTabDeleted() )
/*N*/ 					SetError( errNoRef ), rTab1 = 0;
/*N*/ 			}
/*N*/ 			{
/*N*/ 				const SingleRefData& rRef = rCRef.Ref2;
/*N*/ 				if ( rRef.IsColRel() )
/*N*/ 					rCol2 = aPos.Col() + rRef.nRelCol;
/*N*/ 				else
/*N*/ 					rCol2 = rRef.nCol;
/*N*/ 				if ( rRef.IsRowRel() )
/*N*/ 					rRow2 = aPos.Row() + rRef.nRelRow;
/*N*/ 				else
/*N*/ 					rRow2 = rRef.nRow;
/*N*/ 				if ( rRef.IsTabRel() )
/*N*/ 					rTab2 = aPos.Tab() + rRef.nRelTab;
/*N*/ 				else
/*N*/ 					rTab2 = rRef.nTab;
/*N*/ 				if( rCol2 < 0 || rCol2 > MAXCOL || rRef.IsColDeleted() )
/*N*/ 					SetError( errNoRef ), rCol2 = 0;
/*N*/ 				if( rRow2 < 0 || rRow2 > MAXROW || rRef.IsRowDeleted() )
/*N*/ 					SetError( errNoRef ), rRow2 = 0;
/*N*/ 				if( rTab2 < 0 || rTab2 >= nMaxTab || rRef.IsTabDeleted() )
/*N*/ 					SetError( errNoRef ), rTab2 = 0;
/*N*/ 			}
/*N*/ 			if ( pDok->aTableOpList.Count() > 0 && !bDontCheckForTableOp )
/*N*/ 			{
/*?*/ 				ScRange aRange( rCol1, rRow1, rTab1, rCol2, rRow2, rTab2 );
/*?*/ 				if ( IsTableOpInRange( aRange ) )
/*?*/ 					SetError( errIllegalParameter );
/*N*/ 			}
/*N*/ 			return;
/*N*/ 		}
/*N*/ 		else if( p->GetType() == svMissing )
/*N*/ 			SetError( errIllegalParameter );
/*N*/ 	}
/*N*/ 	SetError(errUnknownStackVariable);
/*N*/ }


/*N*/ void ScInterpreter::PopDoubleRef( ScRange& rRange, BOOL bDontCheckForTableOp )
/*N*/ {
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		--sp;
/*N*/ 		ScToken* p = pStack[ sp ];
/*N*/ 		if ( !nGlobalError )
/*N*/ 			nGlobalError = pErrorStack[ sp ];
/*N*/ 		if( p->GetType() == svDoubleRef )
/*N*/ 		{
/*N*/ 			const ComplRefData& rCRef = p->GetDoubleRef();
/*N*/ 			short nCol, nRow, nTab;
/*N*/ 			USHORT nMaxTab = pDok->GetTableCount();
/*N*/ 			{
/*N*/ 				const SingleRefData& rRef = rCRef.Ref1;
/*N*/ 				if ( rRef.IsColRel() )
/*N*/ 					nCol = aPos.Col() + rRef.nRelCol;
/*N*/ 				else
/*N*/ 					nCol = rRef.nCol;
/*N*/ 				if ( rRef.IsRowRel() )
/*N*/ 					nRow = aPos.Row() + rRef.nRelRow;
/*N*/ 				else
/*N*/ 					nRow = rRef.nRow;
/*N*/ 				if ( rRef.IsTabRel() )
/*N*/ 					nTab = aPos.Tab() + rRef.nRelTab;
/*N*/ 				else
/*N*/ 					nTab = rRef.nTab;
/*N*/ 				if( nCol < 0 || nCol > MAXCOL || rRef.IsColDeleted() )
/*N*/ 					SetError( errNoRef ), nCol = 0;
/*N*/ 				if( nRow < 0 || nRow > MAXROW || rRef.IsRowDeleted() )
/*N*/ 					SetError( errNoRef ), nRow = 0;
/*N*/ 				if( nTab < 0 || nTab >= nMaxTab || rRef.IsTabDeleted() )
/*N*/ 					SetError( errNoRef ), nTab = 0;
/*N*/ 				rRange.aStart.Set( (USHORT)nCol, (USHORT)nRow, (USHORT)nTab );
/*N*/ 			}
/*N*/ 			{
/*N*/ 				const SingleRefData& rRef = rCRef.Ref2;
/*N*/ 				if ( rRef.IsColRel() )
/*N*/ 					nCol = aPos.Col() + rRef.nRelCol;
/*N*/ 				else
/*N*/ 					nCol = rRef.nCol;
/*N*/ 				if ( rRef.IsRowRel() )
/*N*/ 					nRow = aPos.Row() + rRef.nRelRow;
/*N*/ 				else
/*N*/ 					nRow = rRef.nRow;
/*N*/ 				if ( rRef.IsTabRel() )
/*N*/ 					nTab = aPos.Tab() + rRef.nRelTab;
/*N*/ 				else
/*N*/ 					nTab = rRef.nTab;
/*N*/ 				if( nCol < 0 || nCol > MAXCOL || rRef.IsColDeleted() )
/*N*/ 					SetError( errNoRef ), nCol = 0;
/*N*/ 				if( nRow < 0 || nRow > MAXROW || rRef.IsRowDeleted() )
/*N*/ 					SetError( errNoRef ), nRow = 0;
/*N*/ 				if( nTab < 0 || nTab >= nMaxTab || rRef.IsTabDeleted() )
/*N*/ 					SetError( errNoRef ), nTab = 0;
/*N*/ 				rRange.aEnd.Set( (USHORT)nCol, (USHORT)nRow, (USHORT)nTab );
/*N*/ 			}
/*N*/ 			if ( pDok->aTableOpList.Count() > 0 && !bDontCheckForTableOp )
/*N*/ 			{
/*?*/ 				if ( IsTableOpInRange( rRange ) )
/*?*/ 					SetError( errIllegalParameter );
/*N*/ 			}
/*N*/ 			return;
/*N*/ 		}
/*?*/ 		else if( p->GetType() == svMissing )
/*?*/ 			SetError( errIllegalParameter );
/*?*/ 	}
/*?*/ 	SetError(errUnknownStackVariable);
/*N*/ }


BOOL ScInterpreter::PopDoubleRefOrSingleRef( ScAddress& rAdr )
{
    switch ( GetStackType() )
    {
        case svDoubleRef :
        {
            ScRange aRange;
            PopDoubleRef( aRange, TRUE );
            return DoubleRefToPosSingleRef( aRange, rAdr );
        }
        //break;
        case svSingleRef :
        {
            PopSingleRef( rAdr );
            return TRUE;
        }
        //break;
        default:
            Pop();
            SetError( errNoRef );
    }
    return FALSE;
}


/*N*/ void ScInterpreter::PopDoubleRefPushMatrix()
/*N*/ {
/*N*/ 	if ( GetStackType() == svDoubleRef )
/*N*/ 	{
/*N*/ 		USHORT nMatInd;
/*N*/ 		ScMatrix* pMat = GetMatrix( nMatInd );
/*N*/ 		if ( pMat )
/*N*/ 		{
/*N*/ 			PushMatrix( pMat );
/*N*/ 			nRetMat = nMatInd;
/*N*/ 		}
/*N*/ 		else
/*?*/ 			SetIllegalParameter();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SetError( errNoRef );
/*N*/ }


/*N*/ ScMatrix* ScInterpreter::PopMatrix()
/*N*/ {
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		--sp;
/*N*/ 		ScToken* p = pStack[ sp ];
/*N*/ 		if ( !nGlobalError )
/*N*/ 			nGlobalError = pErrorStack[ sp ];
/*N*/ 		if( p->GetType() == svMatrix )
/*N*/ 			return p->GetMatrix();
/*N*/ 		else if( p->GetType() == svMissing )
/*N*/ 			SetError( errIllegalParameter );
/*N*/ 	}
/*N*/ 	SetError(errUnknownVariable);
/*N*/ 	return NULL;
/*N*/ }



/*N*/ void ScInterpreter::PushDouble(double nVal)
/*N*/ {
/*N*/ 	if (!::rtl::math::isFinite(nVal))
/*N*/ 	{
/*N*/         if ( ::rtl::math::isNan( nVal ) )
/*N*/             SetError(errNoValue);
/*N*/         else
/*N*/             SetError(errIllegalFPOperation);
/*N*/ 		nVal = 0.0;
/*N*/ 	}
/*N*/ 	PushTempToken( new ScDoubleToken( nVal ) );
/*N*/ }


/*N*/ void ScInterpreter::PushInt(int nVal)
/*N*/ {
/*N*/ 	PushTempToken( new ScDoubleToken( nVal ) );
/*N*/ }


void ScInterpreter::PushStringBuffer( const sal_Unicode* pString )
{
    if ( pString )
        PushString( String( pString ) );
    else
        PushString( EMPTY_STRING );
}


/*N*/ void ScInterpreter::PushString( const String& rString )
/*N*/ {
/*N*/ 	PushTempToken( new ScStringToken( rString ) );
/*N*/ }


void ScInterpreter::PushSingleRef(USHORT nCol, USHORT nRow, USHORT nTab)
{
    SingleRefData aRef;
    aRef.InitFlags();
    aRef.nCol = nCol;
    aRef.nRow = nRow;
    aRef.nTab = nTab;
    PushTempToken( new ScSingleRefToken( aRef ) );
}


void ScInterpreter::PushDoubleRef(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                  USHORT nCol2, USHORT nRow2, USHORT nTab2)
{
    ComplRefData aRef;
    aRef.InitFlags();
    aRef.Ref1.nCol = nCol1;
    aRef.Ref1.nRow = nRow1;
    aRef.Ref1.nTab = nTab1;
    aRef.Ref2.nCol = nCol2;
    aRef.Ref2.nRow = nRow2;
    aRef.Ref2.nTab = nTab2;
    PushTempToken( new ScDoubleRefToken( aRef ) );
}


/*N*/ void ScInterpreter::PushMatrix(ScMatrix* pMat)
/*N*/ {
/*N*/ 	PushTempToken( new ScMatrixToken( pMat ) );
/*N*/ }


void ScInterpreter::SetParameterExpected()
{
    SetError(errParameterExpected);
    PushInt(0);
}


void ScInterpreter::SetIllegalParameter()
{
    SetError(errIllegalParameter);
    PushInt(0);
}


/*N*/ void ScInterpreter::SetIllegalArgument()
/*N*/ {
/*N*/ 	SetError(errIllegalArgument);
/*N*/ 	PushInt(0);
/*N*/ }


void ScInterpreter::SetNV()
{
    SetError(NOVALUE);
    PushInt(0);
}


/*N*/ void ScInterpreter::SetNoValue()
/*N*/ {
/*N*/ 	SetError(errNoValue);
/*N*/ 	PushInt(0);
/*N*/ }


/*N*/ StackVar ScInterpreter::GetStackType()
/*N*/ {
/*N*/ 	StackVar eRes;
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		eRes = pStack[sp - 1]->GetType();
/*N*/ 		if( eRes == svMissing )
/*N*/ 			eRes = svDouble; 	// default!
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SetError(errUnknownStackVariable);
/*N*/ 		eRes = svErr;
/*N*/ 	}
/*N*/ 	return eRes;
/*N*/ }


/*N*/ StackVar ScInterpreter::GetStackType( BYTE nParam )
/*N*/ {
/*N*/ 	StackVar eRes;
/*N*/ 	if( sp > nParam-1 )
/*N*/ 	{
/*N*/ 		eRes = pStack[sp - nParam]->GetType();
/*N*/ 		if( eRes == svMissing )
/*N*/ 			eRes = svDouble; 	// default!
/*N*/ 	}
/*N*/ 	else
/*N*/ 		eRes = svErr;
/*N*/ 	return eRes;
/*N*/ }


BOOL ScInterpreter::DoubleRefToPosSingleRef( const ScRange& rRange, ScAddress& rAdr )
{
    USHORT nMyCol = aPos.Col();
    USHORT nMyRow = aPos.Row();
    USHORT nMyTab = aPos.Tab();
    USHORT nCol, nRow, nTab;
    nTab = rRange.aStart.Tab();
    BOOL bOk = FALSE;
    if ( rRange.aStart.Col() <= nMyCol && nMyCol <= rRange.aEnd.Col() )
    {
        nRow = rRange.aStart.Row();
        if ( nRow == rRange.aEnd.Row() )
        {
            bOk = TRUE;
            nCol = nMyCol;
        }
        else if ( nTab != nMyTab && nTab == rRange.aEnd.Tab()
                && rRange.aStart.Row() <= nMyRow && nMyRow <= rRange.aEnd.Row() )
        {
            bOk = TRUE;
            nCol = nMyCol;
            nRow = nMyRow;
        }
    }
    else if ( rRange.aStart.Row() <= nMyRow && nMyRow <= rRange.aEnd.Row() )
    {
        nCol = rRange.aStart.Col();
        if ( nCol == rRange.aEnd.Col() )
        {
            bOk = TRUE;
            nRow = nMyRow;
        }
        else if ( nTab != nMyTab && nTab == rRange.aEnd.Tab()
                && rRange.aStart.Col() <= nMyCol && nMyCol <= rRange.aEnd.Col() )
        {
            bOk = TRUE;
            nCol = nMyCol;
            nRow = nMyRow;
        }
    }
    if ( bOk )
    {
        if ( nTab == rRange.aEnd.Tab() )
            ;	// all done
        else if ( nTab <= nMyTab && nMyTab <= rRange.aEnd.Tab() )
            nTab = nMyTab;
        else
            bOk = FALSE;
        if ( bOk )
            rAdr.Set( nCol, nRow, nTab );
    }
    if ( !bOk )
        SetError( errNoValue );
    return bOk;
}


/*N*/ double ScInterpreter::GetDouble()
/*N*/ {
/*N*/ 	double nVal;
/*N*/ 	switch( GetStackType() )
/*N*/ 	{
/*N*/ 		case svDouble:
/*N*/ 			nVal = PopDouble(); break;
/*N*/ 		case svString:
/*N*/ 		{
/*N*/ 			String aStr(PopString());
/*N*/ 			sal_uInt32 nFIndex = 0;					// damit default Land/Spr.
/*N*/ 			if(!pFormatter->IsNumberFormat( aStr, nFIndex, nVal ) )
/*N*/ 			{
/*N*/ 				SetError(errIllegalArgument);
/*N*/ 				nVal = 0.0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/ 		case svSingleRef:
/*N*/ 		{
/*N*/ 			ScAddress aAdr;
/*N*/ 			PopSingleRef( aAdr );
/*N*/ 			ScBaseCell* pCell = GetCell( aAdr );
/*N*/ 			nVal = GetCellValue( aAdr, pCell );
/*N*/ 		}
/*N*/ 		break;
/*?*/ 		case svDoubleRef:
/*?*/ 		{	// positionsabhaengige SingleRef generieren
/*?*/ 			ScRange aRange;
/*?*/ 			PopDoubleRef( aRange );
/*?*/ 			ScAddress aAdr;
/*?*/ 			if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
/*?*/ 			{
/*?*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 				nVal = GetCellValue( aAdr, pCell );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				nVal = 0.0;
/*?*/ 		}
/*?*/ 		break;
/*?*/ 		default:
/*?*/ 			Pop();
/*?*/ 			SetError(errIllegalParameter);
/*?*/ 			nVal = 0.0;
/*N*/ 	}
/*N*/ 	if ( nFuncFmtType == nCurFmtType )
/*N*/ 		nFuncFmtIndex = nCurFmtIndex;
/*N*/ 	return nVal;
/*N*/ }


/*N*/ const String& ScInterpreter::GetString()
/*N*/ {
/*N*/ 	StackVar eRes = (StackVar) GetStackType();
/*N*/ 	if( eRes == svDouble && pStack[ sp-1 ]->GetType() == svMissing )
/*N*/ 		eRes = svString;
/*N*/ 	switch( eRes )
/*N*/ 	{
/*?*/ 		case svDouble:
/*?*/ 		{
/*?*/ 			double fVal = PopDouble();
/*?*/ 			ULONG nIndex = pFormatter->GetStandardFormat(
/*?*/ 									NUMBERFORMAT_NUMBER,
/*?*/ 									ScGlobal::eLnge);
/*?*/ 			pFormatter->GetInputLineString(fVal, nIndex, aTempStr);
/*?*/ 			return aTempStr;
/*?*/ 		}
/*?*/ 		//break;
/*N*/ 		case svString:
/*N*/ 			return PopString();
/*N*/ 		//break;
/*N*/ 		case svSingleRef:
/*N*/ 		{
/*N*/ 			ScAddress aAdr;
/*N*/ 			PopSingleRef( aAdr );
/*N*/ 			if (nGlobalError == 0)
/*N*/ 			{
/*N*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*N*/ 				GetCellString( aTempStr, pCell );
/*N*/ 				return aTempStr;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				return EMPTY_STRING;
/*N*/ 		}
/*N*/ 		//break;
/*?*/ 		case svDoubleRef:
/*?*/ 		{	// positionsabhaengige SingleRef generieren
/*?*/ 			ScRange aRange;
/*?*/ 			PopDoubleRef( aRange );
/*?*/ 			ScAddress aAdr;
/*?*/ 			if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
/*?*/ 			{
/*?*/ 				ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 				GetCellString( aTempStr, pCell );
/*?*/ 				return aTempStr;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				return EMPTY_STRING;
/*?*/ 		}
/*?*/ 		//break;
/*?*/ 		default:
/*?*/ 			Pop();
/*?*/ 			SetError(errIllegalParameter);
/*N*/ 	}
/*N*/ 	return EMPTY_STRING;
/*N*/ }


/*N*/  void ScInterpreter::ScDBGet()
/*N*/  {
/*N*/  	USHORT nTab;
/*N*/  	ScQueryParam aQueryParam;
/*N*/      BOOL bMissingField = FALSE;
/*N*/  	if (GetDBParams( nTab, aQueryParam, bMissingField))
/*N*/  	{
/*N*/  		ScBaseCell* pCell;
/*N*/  		ScQueryCellIterator aCellIter(pDok, nTab, aQueryParam);
/*N*/  		if (pCell = aCellIter.GetFirst())
/*N*/  		{
/*N*/  			if (aCellIter.GetNext())
/*N*/  				SetIllegalArgument();
/*N*/  			else
/*N*/  			{
/*N*/  				switch (pCell->GetCellType())
/*N*/  				{
/*N*/  					case CELLTYPE_VALUE:
/*N*/  					{
/*N*/  						double rValue = ((ScValueCell*)pCell)->GetValue();
/*N*/  						if ( bCalcAsShown )
/*N*/  						{
/*N*/  							ULONG nFormat;
/*N*/  							nFormat = aCellIter.GetNumberFormat();
/*N*/  							rValue = pDok->RoundValueAsShown( rValue, nFormat );
/*N*/  						}
/*N*/  						PushDouble(rValue);
/*N*/  					}
/*N*/  					break;
/*N*/  					case CELLTYPE_STRING:
/*N*/  					{
/*N*/  						String rString;
/*N*/  						((ScStringCell*)pCell)->GetString(rString);
/*N*/  						PushString(rString);
/*N*/  					}
/*N*/  					break;
/*N*/  					case CELLTYPE_EDIT:
/*N*/  					{
/*N*/  						String rString;
/*N*/  						((ScEditCell*)pCell)->GetString(rString);
/*N*/  						PushString(rString);
/*N*/  					}
/*N*/  					break;
/*N*/  					case CELLTYPE_FORMULA:
/*N*/  					{
/*N*/  						USHORT rErr = ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/  						if (rErr)
/*N*/  						{
/*N*/  							SetError(rErr);
/*N*/  							PushInt(0);
/*N*/  						}
/*N*/  						else if (((ScFormulaCell*)pCell)->IsValue())
/*N*/  						{
/*N*/  							double rValue = ((ScFormulaCell*)pCell)->GetValue();
/*N*/  							PushDouble(rValue);
/*N*/  						}
/*N*/  						else
/*N*/  						{
/*N*/  							String rString;
/*N*/  							((ScFormulaCell*)pCell)->GetString(rString);
/*N*/  							PushString(rString);
/*N*/  						}
/*N*/  					}
/*N*/  					break;
/*N*/  					case CELLTYPE_NONE:
/*N*/  					case CELLTYPE_NOTE:
/*N*/  					default:
/*N*/  						SetIllegalArgument();
/*N*/  					break;
/*N*/  				}
/*N*/  			}
/*N*/  		}
/*N*/  		else
/*N*/  			SetNoValue();
/*N*/  	}
/*N*/  	else
/*N*/  		SetIllegalParameter();
/*N*/  }


/*N*/ void ScInterpreter::ScExternal()
/*N*/ {
/*N*/ 	USHORT nIndex;
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	String aUnoName;
/*N*/ 	String aFuncName( ScGlobal::pCharClass->upper( pCur->GetExternal() ) );
/*N*/ 	if (ScGlobal::GetFuncCollection()->SearchFunc(aFuncName, nIndex))
/*N*/ 	{
/*?*/ 		FuncData* pFuncData = (FuncData*)ScGlobal::GetFuncCollection()->At(nIndex);
/*?*/ 		if (nParamCount == pFuncData->GetParamCount() - 1)
/*?*/ 		{
/*?*/ 			ParamType	eParamType[MAXFUNCPARAM];
/*?*/ 			void*		ppParam[MAXFUNCPARAM];
/*?*/ 			double		nVal[MAXFUNCPARAM];
/*?*/ 			sal_Char*	pStr[MAXFUNCPARAM];
/*?*/ 			BYTE*		pCellArr[MAXFUNCPARAM];
/*?*/ 			short		i;
/*?*/ 
/*?*/ 			for (i = 0; i < MAXFUNCPARAM; i++)
/*?*/ 			{
/*?*/ 				eParamType[i] = pFuncData->GetParamType(i);
/*?*/ 				ppParam[i] = NULL;
/*?*/ 				nVal[i] = 0.0;
/*?*/ 				pStr[i] = NULL;
/*?*/ 				pCellArr[i] = NULL;
/*?*/ 			}
/*?*/ 
/*?*/ 			for (i = nParamCount; (i > 0) && (nGlobalError == 0); i--)
/*?*/ 			{
/*?*/ 				BYTE nStackType = GetStackType();
/*?*/ 				switch (eParamType[i])
/*?*/ 				{
/*?*/ 					case PTR_DOUBLE :
/*?*/ 						{
/*?*/ 							nVal[i-1] = GetDouble();
/*?*/ 							ppParam[i] = &nVal[i-1];
/*?*/ 						}
/*?*/ 						break;
/*?*/ 					case PTR_STRING :
/*?*/ 						{
/*?*/ 							ByteString aStr( GetString(), osl_getThreadTextEncoding() );
/*?*/ 							if ( aStr.Len() >= MAXSTRLEN )
/*?*/ 								SetError( errStringOverflow );
/*?*/ 							else
/*?*/ 							{
/*?*/ 								pStr[i-1] = new sal_Char[MAXSTRLEN];
/*?*/ 								strncpy( pStr[i-1], aStr.GetBuffer(), MAXSTRLEN );
/*?*/                                 pStr[i-1][MAXSTRLEN-1] = 0;
/*?*/ 								ppParam[i] = pStr[i-1];
/*?*/ 							}
/*?*/ 						}
/*?*/ 						break;
/*?*/ 					case PTR_DOUBLE_ARR :
/*?*/ 						{
/*?*/ 							USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*?*/ 							PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
/*?*/ 							pCellArr[i-1] = new BYTE[MAXARRSIZE];
/*?*/ 							if (!CreateDoubleArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
/*?*/ 								SetError(errCodeOverflow);
/*?*/ 							else
/*?*/ 								ppParam[i] = pCellArr[i-1];
/*?*/ 						}
/*?*/ 						break;
/*?*/ 					case PTR_STRING_ARR :
/*?*/ 						{
/*?*/ 							USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*?*/ 							PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
/*?*/ 							pCellArr[i-1] = new BYTE[MAXARRSIZE];
/*?*/ 							if (!CreateStringArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
/*?*/ 								SetError(errCodeOverflow);
/*?*/ 							else
/*?*/ 								ppParam[i] = pCellArr[i-1];
/*?*/ 						}
/*?*/ 						break;
/*?*/ 					case PTR_CELL_ARR :
/*?*/ 						{
/*?*/ 							USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*?*/ 							PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
/*?*/ 							pCellArr[i-1] = new BYTE[MAXARRSIZE];
/*?*/ 							if (!CreateCellArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
/*?*/ 								SetError(errCodeOverflow);
/*?*/ 							else
/*?*/ 								ppParam[i] = pCellArr[i-1];
/*?*/ 						}
/*?*/ 						break;
/*?*/ 					default :
/*?*/ 						SetError(errIllegalParameter);
/*?*/ 						break;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			while ( i-- )
/*?*/ 				Pop();		// im Fehlerfall (sonst ist i==0) Parameter wegpoppen
/*?*/ 
/*?*/ 			if (nGlobalError == 0)
/*?*/ 			{
/*?*/ 				if ( pFuncData->GetAsyncType() == NONE )
/*?*/ 				{
/*?*/ 					switch ( eParamType[0] )
/*?*/ 					{
/*?*/ 						case PTR_DOUBLE :
/*?*/ 						{
/*?*/ 							double nErg = 0.0;
/*?*/ 							ppParam[0] = &nErg;
/*?*/ 							pFuncData->Call(ppParam);
/*?*/ 							PushDouble(nErg);
/*?*/ 						}
/*?*/ 						break;
/*?*/ 						case PTR_STRING :
/*?*/ 						{
/*?*/ 							sal_Char* pcErg = new sal_Char[MAXSTRLEN];
/*?*/ 							ppParam[0] = pcErg;
/*?*/ 							pFuncData->Call(ppParam);
/*?*/ 							String aUni( pcErg, osl_getThreadTextEncoding() );
/*?*/ 							PushString( aUni );
/*?*/ 							delete[] pcErg;
/*?*/ 						}
/*?*/ 						break;
/*?*/ 						default:
/*?*/ 							SetError( errUnknownState );
/*?*/ 							PushInt(0);
/*?*/ 					}
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// nach dem Laden Asyncs wieder anwerfen
/*?*/ 					if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
/*?*/ 						pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();
/*?*/ 					// garantiert identischer Handle bei identischem Aufruf?!?
/*?*/ 					// sonst schei*e ...
/*?*/ 					double nErg = 0.0;
/*?*/ 					ppParam[0] = &nErg;
/*?*/ 					pFuncData->Call(ppParam);
/*?*/ 					ULONG nHandle = ULONG( nErg );
/*?*/ 					if ( nHandle >= 65536 )
/*?*/ 					{
/*?*/ 						ScAddInAsync* pAs = ScAddInAsync::Get( nHandle );
/*?*/ 						if ( !pAs )
/*?*/ 						{
/*?*/ 							pAs = new ScAddInAsync( nHandle, nIndex, pDok );
/*?*/ 							pMyFormulaCell->StartListening( *pAs, TRUE );
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							// falls per cut/copy/paste
/*?*/ 							if ( !pMyFormulaCell->IsListening( *pAs ) )
/*?*/ 								pMyFormulaCell->StartListening( *pAs, TRUE );
/*?*/ 							// in anderes Dokument?
/*?*/ 							if ( !pAs->HasDocument( pDok ) )
/*?*/ 								pAs->AddDocument( pDok );
/*?*/ 						}
/*?*/ 						if ( pAs->IsValid() )
/*?*/ 						{
/*?*/ 							switch ( pAs->GetType() )
/*?*/ 							{
/*?*/ 								case PTR_DOUBLE :
/*?*/ 									PushDouble( pAs->GetValue() );
/*?*/ 									break;
/*?*/ 								case PTR_STRING :
/*?*/ 									PushString( pAs->GetString() );
/*?*/ 									break;
/*?*/ 								default:
/*?*/ 									SetError( errUnknownState );
/*?*/ 									PushInt(0);
/*?*/ 							}
/*?*/ 						}
/*?*/ 						else
/*?*/ 							SetNV();
/*?*/ 					}
/*?*/ 					else
/*?*/ 						SetNoValue();
/*?*/ 				}
/*?*/ 			}
/*?*/ 
/*?*/ 			for (i = 0; i < MAXFUNCPARAM; i++)
/*?*/ 			{
/*?*/ 				delete[] pStr[i];
/*?*/ 				delete[] pCellArr[i];
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			while( nParamCount-- )
/*?*/ 				Pop();
/*?*/ 			SetError(errIllegalParameter);
/*?*/ 			PushInt(0);
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else if ( ( aUnoName = ScGlobal::GetAddInCollection()->FindFunction(aFuncName, FALSE) ).Len()  )
/*N*/ 	{
/*N*/ 		//	bLocalFirst=FALSE in FindFunction, cFunc should be the stored internal name
/*N*/ 
/*N*/ 		ScUnoAddInCall aCall( *ScGlobal::GetAddInCollection(), aUnoName, nParamCount );
/*N*/ 
/*N*/ 		if ( !aCall.ValidParamCount() )
/*N*/ 			SetError( errIllegalParameter );
/*N*/ 
/*N*/ 		if ( aCall.NeedsCaller() && !GetError() )
/*N*/ 		{
/*N*/ 			SfxObjectShell* pShell = pDok->GetDocumentShell();
/*N*/ 			if (pShell)
/*N*/ 				aCall.SetCallerFromObjectShell( pShell );
/*N*/ 			else
/*N*/ 			{
/*?*/ 				// use temporary model object (without document) to supply options
/*?*/ 				aCall.SetCaller( static_cast<beans::XPropertySet*>(
/*?*/ 									new ScDocOptionsObj( pDok->GetDocOptions() ) ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		short nPar = nParamCount;
/*N*/ 		while ( nPar && !GetError() )
/*N*/ 		{
/*N*/ 			--nPar;		// 0 .. (nParamCount-1)
/*N*/ 
/*N*/ 			ScAddInArgumentType eType = aCall.GetArgType( nPar );
/*N*/ 			BYTE nStackType = GetStackType();
/*N*/ 
/*N*/ 			uno::Any aParam;
/*N*/ 			switch (eType)
/*N*/ 			{
/*N*/ 				case SC_ADDINARG_INTEGER:
/*N*/ 					{
/*N*/ 						double fVal = GetDouble();
/*N*/ 						double fInt = (fVal >= 0.0) ? ::rtl::math::approxFloor( fVal ) :
/*N*/ 													  ::rtl::math::approxCeil( fVal );
/*N*/ 						if ( fInt >= LONG_MIN && fInt <= LONG_MAX )
/*N*/ 							aParam <<= (INT32)fInt;
/*N*/ 						else
/*N*/ 							SetError(errIllegalArgument);
/*N*/ 					}
/*N*/ 					break;
/*N*/ 
/*?*/ 				case SC_ADDINARG_DOUBLE:
/*?*/ 					aParam <<= (double) GetDouble();
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_STRING:
/*?*/ 					aParam <<= ::rtl::OUString( GetString() );
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_INTEGER_ARRAY:
/*?*/ 					switch( nStackType )
/*?*/ 					{
/*?*/ 						case svDouble:
/*?*/ 						case svString:
/*?*/ 						case svSingleRef:
/*?*/ 							{
/*?*/ 								double fVal = GetDouble();
/*?*/ 								double fInt = (fVal >= 0.0) ? ::rtl::math::approxFloor( fVal ) :
/*?*/ 															  ::rtl::math::approxCeil( fVal );
/*?*/ 								if ( fInt >= LONG_MIN && fInt <= LONG_MAX )
/*?*/ 								{
/*?*/ 									INT32 nIntVal = (long)fInt;
/*?*/ 									uno::Sequence<INT32> aInner( &nIntVal, 1 );
/*?*/ 									uno::Sequence< uno::Sequence<INT32> > aOuter( &aInner, 1 );
/*?*/ 									aParam <<= aOuter;
/*?*/ 								}
/*?*/ 								else
/*?*/ 									SetError(errIllegalArgument);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svDoubleRef:
/*?*/ 							{
/*?*/ 								ScRange aRange;
/*?*/ 								PopDoubleRef( aRange );
/*?*/ 								if (!ScRangeToSequence::FillLongArray( aParam, pDok, aRange ))
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svMatrix:
/*?*/ 							if (!ScRangeToSequence::FillLongArray( aParam, PopMatrix() ))
/*?*/ 								SetError(errIllegalParameter);
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							SetError(errIllegalParameter);
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_DOUBLE_ARRAY:
/*?*/ 					switch( nStackType )
/*?*/ 					{
/*?*/ 						case svDouble:
/*?*/ 						case svString:
/*?*/ 						case svSingleRef:
/*?*/ 							{
/*?*/ 								double fVal = GetDouble();
/*?*/ 								uno::Sequence<double> aInner( &fVal, 1 );
/*?*/ 								uno::Sequence< uno::Sequence<double> > aOuter( &aInner, 1 );
/*?*/ 								aParam <<= aOuter;
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svDoubleRef:
/*?*/ 							{
/*?*/ 								ScRange aRange;
/*?*/ 								PopDoubleRef( aRange );
/*?*/ 								if (!ScRangeToSequence::FillDoubleArray( aParam, pDok, aRange ))
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svMatrix:
/*?*/ 							if (!ScRangeToSequence::FillDoubleArray( aParam, PopMatrix() ))
/*?*/ 								SetError(errIllegalParameter);
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							SetError(errIllegalParameter);
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_STRING_ARRAY:
/*?*/ 					switch( nStackType )
/*?*/ 					{
/*?*/ 						case svDouble:
/*?*/ 						case svString:
/*?*/ 						case svSingleRef:
/*?*/ 							{
/*?*/ 								::rtl::OUString aString = ::rtl::OUString( GetString() );
/*?*/ 								uno::Sequence<rtl::OUString> aInner( &aString, 1 );
/*?*/ 								uno::Sequence< uno::Sequence<rtl::OUString> > aOuter( &aInner, 1 );
/*?*/ 								aParam <<= aOuter;
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svDoubleRef:
/*?*/ 							{
/*?*/ 								ScRange aRange;
/*?*/ 								PopDoubleRef( aRange );
/*?*/ 								if (!ScRangeToSequence::FillStringArray( aParam, pDok, aRange ))
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svMatrix:
/*?*/ 							if (!ScRangeToSequence::FillStringArray( aParam, PopMatrix(), pFormatter ))
/*?*/ 								SetError(errIllegalParameter);
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							SetError(errIllegalParameter);
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_MIXED_ARRAY:
/*?*/ 					switch( nStackType )
/*?*/ 					{
/*?*/ 						case svDouble:
/*?*/ 						case svString:
/*?*/ 						case svSingleRef:
/*?*/ 							{
/*?*/ 								uno::Any aElem;
/*?*/ 								if ( nStackType == svDouble )
/*?*/ 									aElem <<= (double) GetDouble();
/*?*/ 								else if ( nStackType == svString )
/*?*/ 									aElem <<= ::rtl::OUString( GetString() );
/*?*/ 								else
/*?*/ 								{
/*?*/ 									ScAddress aAdr;
/*?*/ 									if ( PopDoubleRefOrSingleRef( aAdr ) )
/*?*/ 									{
/*?*/ 										ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 										if ( pCell && pCell->HasStringData() )
/*?*/ 										{
/*?*/ 											String aStr;
/*?*/ 											GetCellString( aStr, pCell );
/*?*/ 											aElem <<= ::rtl::OUString( aStr );
/*?*/ 										}
/*?*/ 										else
/*?*/ 											aElem <<= (double) GetCellValue( aAdr, pCell );
/*?*/ 									}
/*?*/ 								}
/*?*/ 								uno::Sequence<uno::Any> aInner( &aElem, 1 );
/*?*/ 								uno::Sequence< uno::Sequence<uno::Any> > aOuter( &aInner, 1 );
/*?*/ 								aParam <<= aOuter;
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svDoubleRef:
/*?*/ 							{
/*?*/ 								ScRange aRange;
/*?*/ 								PopDoubleRef( aRange );
/*?*/ 								if (!ScRangeToSequence::FillMixedArray( aParam, pDok, aRange ))
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svMatrix:
/*?*/ 							if (!ScRangeToSequence::FillMixedArray( aParam, PopMatrix() ))
/*?*/ 								SetError(errIllegalParameter);
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							SetError(errIllegalParameter);
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_VALUE_OR_ARRAY:
/*?*/ 					switch( nStackType )
/*?*/ 					{
/*?*/ 						case svDouble:
/*?*/ 							aParam <<= (double) GetDouble();
/*?*/ 							break;
/*?*/ 						case svString:
/*?*/ 							aParam <<= ::rtl::OUString( GetString() );
/*?*/ 							break;
/*?*/ 						case svSingleRef:
/*?*/ 							{
/*?*/ 								ScAddress aAdr;
/*?*/ 								if ( PopDoubleRefOrSingleRef( aAdr ) )
/*?*/ 								{
/*?*/ 									ScBaseCell* pCell = GetCell( aAdr );
/*?*/ 									if ( pCell && pCell->HasStringData() )
/*?*/ 									{
/*?*/ 										String aStr;
/*?*/ 										GetCellString( aStr, pCell );
/*?*/ 										aParam <<= ::rtl::OUString( aStr );
/*?*/ 									}
/*?*/ 									else
/*?*/ 										aParam <<= (double) GetCellValue( aAdr, pCell );
/*?*/ 								}
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svDoubleRef:
/*?*/ 							{
/*?*/ 								ScRange aRange;
/*?*/ 								PopDoubleRef( aRange );
/*?*/ 								if (!ScRangeToSequence::FillMixedArray( aParam, pDok, aRange ))
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svMatrix:
/*?*/ 							if (!ScRangeToSequence::FillMixedArray( aParam, PopMatrix() ))
/*?*/ 								SetError(errIllegalParameter);
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							SetError(errIllegalParameter);
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				case SC_ADDINARG_CELLRANGE:
/*?*/ 					switch( nStackType )
/*?*/ 					{
/*?*/ 						case svSingleRef:
/*?*/ 							{
/*?*/ 								ScAddress aAdr;
/*?*/ 								PopSingleRef( aAdr );
/*?*/ 								ScRange aRange( aAdr );
/*?*/ 								uno::Reference<table::XCellRange> xObj =
/*?*/ 										ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
/*?*/ 								if (xObj.is())
/*?*/ 									aParam <<= xObj;
/*?*/ 								else
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						case svDoubleRef:
/*?*/ 							{
/*?*/ 								ScRange aRange;
/*?*/ 								PopDoubleRef( aRange );
/*?*/ 								uno::Reference<table::XCellRange> xObj =
/*?*/ 										ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
/*?*/ 								if (xObj.is())
/*?*/ 									aParam <<= xObj;
/*?*/ 								else
/*?*/ 									SetError(errIllegalParameter);
/*?*/ 							}
/*?*/ 							break;
/*?*/ 						default:
/*?*/ 							Pop();
/*?*/ 							SetError(errIllegalParameter);
/*?*/ 					}
/*?*/ 					break;
/*?*/ 
/*?*/ 				default:
/*?*/ 					Pop();
/*?*/ 					SetError(errIllegalParameter);
/*N*/ 			}
/*N*/ 			aCall.SetParam( nPar, aParam );
/*N*/ 		}
/*N*/ 
/*N*/ 		while (nPar--)
/*?*/ 			Pop();					// in case of error, remove remaining args
/*N*/ 
/*N*/ 		if ( !GetError() )
/*N*/ 		{
/*N*/ 			aCall.ExecuteCall();
/*N*/ 
/*N*/ 			if ( aCall.HasVarRes() )						// handle async functions
/*N*/ 			{
/*?*/ 				if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
/*?*/ 					pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();
/*?*/ 
/*?*/ 				uno::Reference<sheet::XVolatileResult> xResult = aCall.GetVarRes();
/*?*/ 				ScAddInListener* pLis = ScAddInListener::Get( xResult );
/*?*/ 				if ( !pLis )
/*?*/ 				{
/*?*/ 					pLis = ScAddInListener::CreateListener( xResult, pDok );
/*?*/ 					pMyFormulaCell->StartListening( *pLis, TRUE );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					if ( !pMyFormulaCell->IsListening( *pLis ) )
/*?*/ 						pMyFormulaCell->StartListening( *pLis, TRUE );
/*?*/ 					if ( !pLis->HasDocument( pDok ) )
/*?*/ 						pLis->AddDocument( pDok );
/*?*/ 				}
/*?*/ 
/*?*/ 				aCall.SetResult( pLis->GetResult() );		// use result from async
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( aCall.GetErrCode() )
/*N*/ 			{
/*?*/ 				SetError( aCall.GetErrCode() );
/*?*/ 				PushInt(0);
/*N*/ 			}
/*N*/ 			else if ( aCall.HasMatrix() )
/*N*/ 			{
/*?*/ 				const ScMatrix*	pLinkMat = aCall.GetMatrix();		// not NULL
/*?*/ 
/*?*/ 				USHORT nC, nR, nMatInd;								// copy matrix result
/*?*/ 				pLinkMat->GetDimensions(nC, nR);
/*?*/ 				ScMatrix* pNewMat = GetNewMat( nC, nR, nMatInd );
/*?*/ 				if (pNewMat)
/*?*/ 				{
/*?*/ 					pLinkMat->MatCopy(*pNewMat);
/*?*/ 					PushMatrix( pNewMat );
/*?*/ 					nRetMat = nMatInd;
/*?*/ 				}								// otherwise error code has been set in GetNewMat
/*N*/ 			}
/*N*/ 			else if ( aCall.HasString() )
/*?*/ 				PushString( aCall.GetString() );
/*N*/ 			else
/*N*/ 				PushDouble( aCall.GetValue() );
/*N*/ 		}
/*N*/ 		else				// error...
/*?*/ 			PushInt(0);
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		while( nParamCount-- )
/*N*/ 			Pop();
/*N*/         SetError( errNoAddin );
/*N*/ 		PushInt(0);
/*N*/ 	}
/*N*/ }


void ScInterpreter::ScMissing()
{
    PushTempToken( new ScMissingToken );
}


/*N*/ void ScInterpreter::ScMacro()
/*N*/ {
/*N*/ 	SbxBase::ResetError();
/*N*/ 
/*N*/ 	BYTE nParamCount = GetByte();
/*N*/ 	String aMacro( pCur->GetExternal() );
/*N*/ 
/*N*/ 	SfxObjectShell* pDocSh = pDok->GetDocumentShell();
/*N*/ 	if ( !pDocSh || !pDok->CheckMacroWarn() )
/*N*/ 	{
/*N*/ 		SetError( errNoValue );		// ohne DocShell kein CallBasic
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	//	keine Sicherheitsabfrage mehr vorneweg (nur CheckMacroWarn), das passiert im CallBasic
/*N*/ 
/*N*/ 	SfxApplication* pSfxApp = SFX_APP();
/*N*/ 	pSfxApp->EnterBasicCall();				// Dok-Basic anlegen etc.
/*N*/ 
/*N*/ 	//	Wenn das Dok waehrend eines Basic-Calls geladen wurde,
/*N*/ 	//	ist das Sbx-Objekt evtl. nicht angelegt (?)
/*N*/ //	pDocSh->GetSbxObject();
/*N*/ 
/*N*/ 	//	Funktion ueber den einfachen Namen suchen,
/*N*/ 	//	dann aBasicStr, aMacroStr fuer SfxObjectShell::CallBasic zusammenbauen
/*N*/ 
/*N*/ 	StarBASIC* pRoot = pDocSh->GetBasic();
/*N*/ 	SbxVariable* pVar = pRoot->Find( aMacro, SbxCLASS_METHOD );
/*N*/ 	if( !pVar || pVar->GetType() == SbxVOID || !pVar->ISA(SbMethod) )
/*N*/ 	{
/*?*/         SetError( errNoMacro );
/*?*/ 		pSfxApp->LeaveBasicCall();
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	SbMethod* pMethod = (SbMethod*)pVar;
/*N*/ 	SbModule* pModule = pMethod->GetModule();
/*N*/ 	SbxObject* pObject = pModule->GetParent();
/*N*/ 	DBG_ASSERT(pObject->IsA(TYPE(StarBASIC)), "Kein Basic gefunden!");
/*N*/ 	String aMacroStr = pObject->GetName();
/*N*/ 	aMacroStr += '.';
/*N*/ 	aMacroStr += pModule->GetName();
/*N*/ 	aMacroStr += '.';
/*N*/ 	aMacroStr += pMethod->GetName();
/*N*/ 	String aBasicStr;
/*N*/ 	if (pObject->GetParent())
/*N*/ 		aBasicStr = pObject->GetParent()->GetName();	// Dokumentenbasic
/*N*/ 	else
/*?*/ 		aBasicStr = SFX_APP()->GetName();				// Applikationsbasic
/*N*/ 
/*N*/ 	//	Parameter-Array zusammenbauen
/*N*/ 
/*N*/ 	SbxArrayRef refPar = new SbxArray;
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	for( short i = nParamCount; i && bOk ; i-- )
/*N*/ 	{
/*N*/ 		SbxVariable* pPar = refPar->Get( (USHORT) i );
/*N*/ 		BYTE nStackType = GetStackType();
/*N*/ 		switch( nStackType )
/*N*/ 		{
/*?*/ 			case svDouble:
/*?*/ 				pPar->PutDouble( GetDouble() );
/*?*/ 			break;
/*?*/ 			case svString:
/*?*/ 				pPar->PutString( GetString() );
/*?*/ 			break;
/*N*/ 			case svSingleRef:
/*N*/ 			{
/*N*/ 				ScAddress aAdr;
/*N*/ 				PopSingleRef( aAdr );
/*N*/ 				bOk = SetSbxVariable( pPar, aAdr );
/*N*/ 			}
/*N*/ 			break;
/*?*/ 			case svDoubleRef:
/*?*/ 			{
/*?*/ 				USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
/*?*/ 				PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
/*?*/ 				if( nTab1 != nTab2 )
/*?*/ 				{
/*?*/ 					SetError( errIllegalParameter );
/*?*/ 					bOk = FALSE;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					SbxDimArrayRef refArray = new SbxDimArray;
/*?*/ 					refArray->AddDim( 1, nRow2 - nRow1 + 1 );
/*?*/ 					refArray->AddDim( 1, nCol2 - nCol1 + 1 );
/*?*/ 					ScAddress aAdr( nCol1, nRow1, nTab1 );
/*?*/ 					for( USHORT nRow = nRow1; bOk && nRow <= nRow2; nRow++ )
/*?*/ 					{
/*?*/ 						aAdr.SetRow( nRow );
/*?*/ 						short nIdx[ 2 ];
/*?*/ 						nIdx[ 0 ] = nRow-nRow1+1;
/*?*/ 						for( USHORT nCol = nCol1; bOk && nCol <= nCol2; nCol++ )
/*?*/ 						{
/*?*/ 							aAdr.SetCol( nCol );
/*?*/ 							nIdx[ 1 ] = nCol-nCol1+1;
/*?*/ 							SbxVariable* p = refArray->Get( nIdx );
/*?*/ 							bOk = SetSbxVariable( p, aAdr );
/*?*/ 						}
/*?*/ 					}
/*?*/ 					pPar->PutObject( refArray );
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			case svMatrix:
/*?*/ 			{
/*?*/ 				ScMatrix* pMat = PopMatrix();
/*?*/ 				USHORT nC, nR;
/*?*/ 				if (pMat)
/*?*/ 				{
/*?*/ 					pMat->GetDimensions(nC, nR);
/*?*/ 					SbxDimArrayRef refArray = new SbxDimArray;
/*?*/ 					refArray->AddDim( 1, nR );
/*?*/ 					refArray->AddDim( 1, nC );
/*?*/ 					for( USHORT j = 0; j < nR; j++ )
/*?*/ 					{
/*?*/ 						short nIdx[ 2 ];
/*?*/ 						nIdx[ 0 ] = j+1;
/*?*/ 						for( USHORT i = 0; i < nC; i++ )
/*?*/ 						{
/*?*/ 							nIdx[ 1 ] = i+1;
/*?*/ 							SbxVariable* p = refArray->Get( nIdx );
/*?*/ 							if (pMat->IsString(i, j))
/*?*/ 								p->PutString( pMat->GetString(i, j) );
/*?*/ 							else
/*?*/ 								p->PutDouble( pMat->GetDouble(i, j) );
/*?*/ 						}
/*?*/ 					}
/*?*/ 					pPar->PutObject( refArray );
/*?*/ 				}
/*?*/ 				else
/*?*/ 					SetError( errIllegalParameter );
/*?*/ 			}
/*?*/ 			break;
/*?*/ 			default:
/*N*/ 				SetError( errIllegalParameter );
/*N*/ 				bOk = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( bOk )
/*N*/ 	{
/*N*/ 		pDok->LockTable( aPos.Tab() );
/*N*/ 		SbxVariableRef refRes = new SbxVariable;
/*N*/ 		pDok->IncMacroInterpretLevel();
/*N*/ 		ErrCode eRet = pDocSh->CallBasic( aMacroStr, aBasicStr, NULL, refPar, refRes );
/*N*/ 		pDok->DecMacroInterpretLevel();
/*N*/ 		pDok->UnlockTable( aPos.Tab() );
/*N*/ 
/*N*/ 		SbxDataType eResType = refRes->GetType();
/*N*/ 		if ( eRet != ERRCODE_NONE )
/*N*/ 			SetNoValue();
/*N*/ 		else if( eResType >= SbxINTEGER && eResType <= SbxDOUBLE )
/*?*/ 			PushDouble( refRes->GetDouble() );
/*N*/ 		else if ( eResType & SbxARRAY )
/*N*/ 		{
/*?*/ 			SbxBase* pElemObj = refRes->GetObject();
/*?*/ 			SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
/*?*/ 			short nDim = pDimArray->GetDims();
/*?*/ 			if ( 1 <= nDim && nDim <= 2 )
/*?*/ 			{
/*?*/ 				short nCs, nCe, nRs, nRe;
/*?*/ 				USHORT nC, nR, nMatInd;
/*?*/ 				USHORT nColIdx, nRowIdx;
/*?*/ 				if ( nDim == 1 )
/*?*/ 				{	// array( cols )  eine Zeile, mehrere Spalten
/*?*/ 					pDimArray->GetDim( 1, nCs, nCe );
/*?*/ 					nC = USHORT(nCe - nCs + 1);
/*?*/ 					nRs = nRe = 0;
/*?*/ 					nR = 1;
/*?*/ 					nColIdx = 0;
/*?*/ 					nRowIdx = 1;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{	// array( rows, cols )
/*?*/ 					pDimArray->GetDim( 1, nRs, nRe );
/*?*/ 					nR = USHORT(nRe - nRs + 1);
/*?*/ 					pDimArray->GetDim( 2, nCs, nCe );
/*?*/ 					nC = USHORT(nCe - nCs + 1);
/*?*/ 					nColIdx = 1;
/*?*/ 					nRowIdx = 0;
/*?*/ 				}
/*?*/ 				ScMatrix* pMat = GetNewMat( nC, nR, nMatInd );
/*?*/ 				if ( pMat )
/*?*/ 				{
/*?*/ 					SbxVariable* pV;
/*?*/ 					SbxDataType eType;
/*?*/ 					for ( USHORT j=0; j < nR; j++ )
/*?*/ 					{
/*?*/ 						short nIdx[ 2 ];
/*?*/ 						// bei eindimensionalem array( cols ) wird nIdx[1]
/*?*/ 						// von SbxDimArray::Get ignoriert
/*?*/ 						nIdx[ nRowIdx ] = nRs + j;
/*?*/ 						for ( USHORT i=0; i < nC; i++ )
/*?*/ 						{
/*?*/ 							nIdx[ nColIdx ] = nCs + i;
/*?*/ 							pV = pDimArray->Get( nIdx );
/*?*/ 							eType = pV->GetType();
/*?*/ 							if ( eType >= SbxINTEGER && eType <= SbxDOUBLE )
/*?*/ 								pMat->PutDouble( pV->GetDouble(), i, j );
/*?*/ 							else
/*?*/ 								pMat->PutString( pV->GetString(), i, j );
/*?*/ 						}
/*?*/ 					}
/*?*/ 					PushMatrix( pMat );
/*?*/ 					nRetMat = nMatInd;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 				SetNoValue();
/*N*/ 		}
/*N*/ 		else
/*?*/ 			PushString( refRes->GetString() );
/*N*/ 		if( pVar->GetError() )
/*?*/ 			SetNoValue();
/*N*/ 	}
/*N*/ 
/*N*/ 	pSfxApp->LeaveBasicCall();
/*N*/ }


/*N*/ BOOL ScInterpreter::SetSbxVariable( SbxVariable* pVar, const ScAddress& rPos )
/*N*/ {
/*N*/ 	BOOL bOk = TRUE;
/*N*/ 	ScBaseCell* pCell = pDok->GetCell( rPos );
/*N*/ 	if (pCell)
/*N*/ 	{
/*N*/ 		USHORT nErr;
/*N*/ 		double nVal;
/*N*/ 		switch( pCell->GetCellType() )
/*N*/ 		{
/*N*/ 			case CELLTYPE_VALUE :
/*N*/ 				nVal = GetValueCellValue( rPos, (ScValueCell*)pCell );
/*N*/ 				pVar->PutDouble( nVal );
/*N*/ 				break;
/*N*/ 			case CELLTYPE_STRING :
/*N*/ 			{
/*N*/ 				String aVal;
/*N*/ 				((ScStringCell*)pCell)->GetString( aVal );
/*N*/ 				pVar->PutString( aVal );
/*N*/ 				break;
/*N*/ 			}
/*?*/ 			case CELLTYPE_EDIT :
/*?*/ 			{
/*?*/ 				String aVal;
/*?*/ 				((ScEditCell*) pCell)->GetString( aVal );
/*?*/ 				pVar->PutString( aVal );
/*?*/ 				break;
/*?*/ 			}
/*N*/ 			case CELLTYPE_FORMULA :
/*N*/ 				nErr = ((ScFormulaCell*)pCell)->GetErrCode();
/*N*/ 				if( !nErr )
/*N*/ 				{
/*N*/ 					if( ((ScFormulaCell*)pCell)->IsValue() )
/*N*/ 					{
/*N*/ 						nVal = ((ScFormulaCell*)pCell)->GetValue();
/*N*/ 						pVar->PutDouble( nVal );
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						String aVal;
/*?*/ 						((ScFormulaCell*)pCell)->GetString( aVal );
/*?*/ 						pVar->PutString( aVal );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					SetError( nErr ), bOk = FALSE;
/*N*/ 				break;
/*?*/ 			default :
/*?*/ 				pVar->PutDouble( 0.0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pVar->PutDouble( 0.0 );
/*N*/ 	return bOk;
/*N*/ }


/*N*/  void ScInterpreter::ScTableOp()
/*N*/  {
/*N*/      BYTE nParamCount = GetByte();
/*N*/      if (nParamCount != 3 && nParamCount != 5)
/*N*/      {
/*N*/          SetIllegalParameter();
/*N*/          return;
/*N*/      }
/*N*/      ScInterpreterTableOpParams* pTableOp = new ScInterpreterTableOpParams;
/*N*/      if (nParamCount == 5)
/*N*/      {
/*N*/          PopSingleRef( pTableOp->aNew2 );
/*N*/          PopSingleRef( pTableOp->aOld2 );
/*N*/      }
/*N*/      PopSingleRef( pTableOp->aNew1 );
/*N*/      PopSingleRef( pTableOp->aOld1 );
/*N*/      PopSingleRef( pTableOp->aFormulaPos );
/*N*/  
/*N*/      pTableOp->bValid = TRUE;
/*N*/      pDok->aTableOpList.Insert( pTableOp );
/*N*/      pDok->IncInterpreterTableOpLevel();
/*N*/  
/*N*/      BOOL bReuseLastParams = (pDok->aLastTableOpParams == *pTableOp);
/*N*/      if ( bReuseLastParams )
/*N*/      {
/*N*/          pTableOp->aNotifiedFormulaPos = pDok->aLastTableOpParams.aNotifiedFormulaPos;
/*N*/          pTableOp->bRefresh = TRUE;
/*N*/          for ( ::std::vector< ScAddress >::const_iterator iBroadcast(
/*N*/                      pTableOp->aNotifiedFormulaPos.begin() );
/*N*/                  iBroadcast != pTableOp->aNotifiedFormulaPos.end();
/*N*/                  ++iBroadcast )
/*N*/          {   // emulate broadcast and indirectly collect cell pointers
/*N*/              ScBaseCell* pCell = pDok->GetCell( *iBroadcast );
/*N*/              if ( pCell && pCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/                  ((ScFormulaCell*)pCell)->SetTableOpDirty();
/*N*/          }
/*N*/      }
/*N*/      else
/*N*/      {   // broadcast and indirectly collect cell pointers and positions
/*N*/          pDok->SetTableOpDirty( pTableOp->aOld1 );
/*N*/          if ( nParamCount == 5 )
/*N*/              pDok->SetTableOpDirty( pTableOp->aOld2 );
/*N*/      }
/*N*/      pTableOp->bCollectNotifications = FALSE;
/*N*/  
/*N*/      ScBaseCell* pFCell = pDok->GetCell( pTableOp->aFormulaPos );
/*N*/      if ( pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/          ((ScFormulaCell*)pFCell)->SetDirtyVar();
/*N*/      if ( HasCellValueData( pFCell ) )
/*N*/          PushDouble( GetCellValue( pTableOp->aFormulaPos, pFCell ));
/*N*/      else
/*N*/      {
/*N*/          String aCellString;
/*N*/          GetCellString( aCellString, pFCell );
/*N*/          PushString( aCellString );
/*N*/      }
/*N*/  
/*N*/      pDok->aTableOpList.Remove( pTableOp );
/*N*/      // set dirty again once more to be able to recalculate original
/*N*/      for ( ::std::vector< ScFormulaCell* >::const_iterator iBroadcast(
/*N*/                  pTableOp->aNotifiedFormulaCells.begin() );
/*N*/              iBroadcast != pTableOp->aNotifiedFormulaCells.end();
/*N*/              ++iBroadcast )
/*N*/      {
/*N*/          (*iBroadcast)->SetTableOpDirty();
/*N*/      }
/*N*/  
/*N*/      // save these params for next incarnation
/*N*/      if ( !bReuseLastParams )
/*N*/          pDok->aLastTableOpParams = *pTableOp;
/*N*/  
/*N*/      if ( pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA )
/*N*/      {
/*N*/          ((ScFormulaCell*)pFCell)->SetDirtyVar();
/*N*/          ((ScFormulaCell*)pFCell)->GetErrCode();     // recalculate original
/*N*/      }
/*N*/  
/*N*/      // Reset all dirty flags so next incarnation does really collect all cell
/*N*/      // pointers during notifications and not just non-dirty ones, which may
/*N*/      // happen if a formula cell is used by more than one TableOp block.
/*N*/      for ( ::std::vector< ScFormulaCell* >::const_iterator iBroadcast2(
/*N*/                  pTableOp->aNotifiedFormulaCells.begin() );
/*N*/              iBroadcast2 != pTableOp->aNotifiedFormulaCells.end();
/*N*/              ++iBroadcast2 )
/*N*/      {
/*N*/          (*iBroadcast2)->ResetTableOpDirtyVar();
/*N*/      }
/*N*/      delete pTableOp;
/*N*/  
/*N*/      pDok->DecInterpreterTableOpLevel();
/*N*/  }

void ScInterpreter::ScDBArea()
{
    ScDBData* pDBData = pDok->GetDBCollection()->FindIndex( pCur->GetIndex());
    if (pDBData)
    {
        ComplRefData aRefData;
        aRefData.InitFlags();
        pDBData->GetArea( (USHORT&) aRefData.Ref1.nTab,
                          (USHORT&) aRefData.Ref1.nCol,
                          (USHORT&) aRefData.Ref1.nRow,
                          (USHORT&) aRefData.Ref2.nCol,
                          (USHORT&) aRefData.Ref2.nRow);
        aRefData.Ref2.nTab    = aRefData.Ref1.nTab;
        aRefData.CalcRelFromAbs( aPos );
        PushTempToken( new ScDoubleRefToken( aRefData ) );
    }
    else
        SetError(errNoName);
}


void ScInterpreter::ScColRowNameAuto()
{
    ComplRefData aRefData( pCur->GetDoubleRef() );
    aRefData.CalcAbsIfRel( aPos );
    if ( aRefData.Valid() )
    {
        INT16 nStartCol, nStartRow, nCol2, nRow2;
        // evtl. Begrenzung durch definierte ColRowNameRanges merken
        nCol2 = aRefData.Ref2.nCol;
        nRow2 = aRefData.Ref2.nRow;
        // DataArea der ersten Zelle
        nStartCol = aRefData.Ref2.nCol = aRefData.Ref1.nCol;
        nStartRow = aRefData.Ref2.nRow = aRefData.Ref1.nRow;
        aRefData.Ref2.nTab = aRefData.Ref1.nTab;
        pDok->GetDataArea(	(USHORT) aRefData.Ref1.nTab,
                            (USHORT&) aRefData.Ref1.nCol,
                            (USHORT&) aRefData.Ref1.nRow,
                            (USHORT&) aRefData.Ref2.nCol,
                            (USHORT&) aRefData.Ref2.nRow,
                            TRUE );
        // DataArea im Ursprung begrenzen
        aRefData.Ref1.nCol = nStartCol;
        aRefData.Ref1.nRow = nStartRow;

        //! korrespondiert mit ScCompiler::GetToken
        if ( aRefData.Ref1.IsColRel() )
        {	// ColName
            aRefData.Ref2.nCol = nStartCol;
            // evtl. vorherige Begrenzung durch definierte ColRowNameRanges erhalten
            if ( aRefData.Ref2.nRow > nRow2 )
                aRefData.Ref2.nRow = nRow2;
            USHORT nMyRow;
            if ( aPos.Col() == nStartCol
              && nStartRow <= (nMyRow = aPos.Row()) && nMyRow <= aRefData.Ref2.nRow )
            {	// Formel in gleicher Spalte und innerhalb des Range
                if ( nMyRow == nStartRow )
                {	// direkt unter dem Namen den Rest nehmen
                    nStartRow++;
                    if ( nStartRow > MAXROW )
                        nStartRow = MAXROW;
                    aRefData.Ref1.nRow = nStartRow;
                }
                else
                {	// weiter unten vom Namen bis zur Formelzelle
                    aRefData.Ref2.nRow = nMyRow - 1;
                }
            }
        }
        else
        {	// RowName
            aRefData.Ref2.nRow = nStartRow;
            // evtl. vorherige Begrenzung durch definierte ColRowNameRanges erhalten
            if ( aRefData.Ref2.nCol > nCol2 )
                aRefData.Ref2.nCol = nCol2;
            USHORT nMyCol;
            if ( aPos.Row() == nStartRow
              && nStartCol <= (nMyCol = aPos.Col()) && nMyCol <= aRefData.Ref2.nCol )
            {	// Formel in gleicher Zeile und innerhalb des Range
                if ( nMyCol == nStartCol )
                {	// direkt neben dem Namen den Rest nehmen
                    nStartCol++;
                    if ( nStartCol > MAXCOL )
                        nStartCol = MAXCOL;
                    aRefData.Ref1.nCol = nStartCol;
                }
                else
                {	// weiter rechts vom Namen bis zur Formelzelle
                    aRefData.Ref2.nCol = nMyCol - 1;
                }
            }
        }
        aRefData.CalcRelFromAbs( aPos );
    }
    else
        SetError( errNoRef );
    PushTempToken( new ScDoubleRefToken( aRefData ) );
}

// --- internals ------------------------------------------------------------


void ScInterpreter::ScAnswer()
{
    String aStr( GetString() );
    if( aStr.EqualsIgnoreCaseAscii( "Das Leben, das Universum und der ganze Rest" ) )
    {
        PushInt( 42 );
        bOderSo = TRUE;
    }
    else
        SetNoValue();
}


void ScInterpreter::ScCalcTeam()
{
    static BOOL bShown = FALSE;
    if( !bShown )
    {
        DBG_BF_ASSERT(0, "STRIP"); //STRIP001  ShowTheTeam();
/*N*/          String aTeam( RTL_CONSTASCII_USTRINGPARAM( "Ballach, Nebel, Rentz, Rathke, Marmion" ) );
/*N*/  		if ( (GetByte() == 1) && ::rtl::math::approxEqual( GetDouble(), 1996) )
/*N*/  /*?*/ 			aTeam.AppendAscii( "   (a word with 'B': -Olk, -Nietsch, -Daeumling)" );
/*N*/  /*?*/ 		PushString( aTeam );
/*N*/  /*?*/ 		bShown = TRUE;
    }
    else
        PushInt( 42 );
}


void ScInterpreter::ScSpewFunc()
{
    BOOL bRefresh = FALSE;
    BOOL bClear = FALSE;
    // Stack aufraeumen
    BYTE nParamCount = GetByte();
    while ( nParamCount-- )
    {
        switch ( GetStackType() )
        {
            case svString:
            case svSingleRef:
            case svDoubleRef:
            {
                const sal_Unicode ch = GetString().GetChar(0);
                if ( !bRefresh && ch < 256 )
                    bRefresh = (tolower( (sal_uChar) ch ) == 'r');
                if ( !bClear && ch < 256 )
                    bClear = (tolower( (sal_uChar) ch ) == 'c');
            }
            break;
            default:
                Pop();
        }
    }
    String aStr;
#if SC_SPEW_ENABLED
    if ( bRefresh )
        theSpew.Clear();		// GetSpew liest SpewRulesFile neu
    theSpew.GetSpew( aStr );
    if ( bClear )
        theSpew.Clear();		// release Memory
    xub_StrLen nPos = 0;
    while ( (nPos = aStr.SearchAndReplace( '\n', ' ', nPos )) != STRING_NOTFOUND )
        nPos++;
#else
    aStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "spitted out all spew :-(" ) );
#endif
    PushString( aStr );
}


/*N*/  
/*N*/  //#define SC_INVADER_GPF		// GPF wollen wir nicht :-(
/*N*/  // zum testen Environment-Variable SC_INVADER_GPF=xxx setzen
/*N*/  // 08.10.98: wenn PB optpath.cxx gefixt hat geht's wieder
/*N*/  
/*N*/  extern void StartInvader( Window* pParent );	// StarWars, Wrapper in SVX options/optpath.cxx
/*N*/  extern void Game();						// Froggie

void ScInterpreter::ScTTT()
{	// Temporaerer Test-Tanz, zum auspropieren von Funktionen etc.
    BOOL bOk = TRUE;
    BYTE nParamCount = GetByte();
    // do something, nParamCount bei Pops runterzaehlen!

    if ( bOk && nParamCount )
    {
        bOk = GetBool();
        --nParamCount;
    }
    // Stack aufraeumen
    while ( nParamCount-- )
        Pop();
    static const sal_Unicode __FAR_DATA sEyes[]		= { ',',';',':','|','8','B', 0 };
    static const sal_Unicode __FAR_DATA sGoods[]	= { ')',']','}', 0 };
    static const sal_Unicode __FAR_DATA sBads[]		= { '(','[','{','/', 0 };
    sal_Unicode aFace[4];
    if ( bOk )
    {
        aFace[0] = sEyes[ rand() % ((sizeof( sEyes )/sizeof(sal_Unicode)) - 1) ];
        aFace[1] = '-';
        aFace[2] = sGoods[ rand() % ((sizeof( sGoods )/sizeof(sal_Unicode)) - 1) ];
    }
    else
    {
        aFace[0] = ':';
        aFace[1] = '-';
        aFace[2] = sBads[ rand() % ((sizeof( sBads )/sizeof(sal_Unicode)) - 1) ];
    }
    aFace[3] = 0;
    PushStringBuffer( aFace );
}

// -------------------------------------------------------------------------


/*N*/ ScInterpreter::ScInterpreter( ScFormulaCell* pCell, ScDocument* pDoc,
/*N*/ 		const ScAddress& rPos, ScTokenArray& r ) :
/*N*/ 	pMyFormulaCell( pCell ),
/*N*/ 	pDok( pDoc ),
/*N*/ 	aCode( r ),
/*N*/ 	aPos( rPos ),
/*N*/ 	rArr( r ),
/*N*/ 	bCalcAsShown( pDoc->GetDocOptions().IsCalcAsShown() ),
/*N*/ 	pFormatter( pDoc->GetFormatTable() )
/*N*/ {
/*N*/ //	pStack = new ScToken*[ MAXSTACK ];
/*N*/ 
/*N*/ 	BYTE cMatFlag = pMyFormulaCell->GetMatrixFlag();
/*N*/ 	bMatrixFormula = ( cMatFlag == MM_FORMULA || cMatFlag == MM_FAKE );
/*N*/ 	if (!bGlobalStackInUse)
/*N*/ 	{
/*N*/ 		bGlobalStackInUse = TRUE;
/*N*/ 		if (!pGlobalStack)
/*N*/ 			pGlobalStack = new ScTokenStack;
/*N*/ 		if (!pGlobalErrorStack)
/*N*/ 			pGlobalErrorStack = new ScErrorStack;
/*N*/ 		pStackObj = pGlobalStack;
/*N*/ 		pErrorStackObj = pGlobalErrorStack;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pStackObj = new ScTokenStack;
/*N*/ 		pErrorStackObj = new ScErrorStack;
/*N*/ 	}
/*N*/ 	pStack = pStackObj->pPointer;
/*N*/ 	pErrorStack = pErrorStackObj->pPointer;
/*N*/ }


/*N*/ ScInterpreter::~ScInterpreter()
/*N*/ {
/*N*/ //	delete pStack;
/*N*/ 
/*N*/ 	if ( pStackObj == pGlobalStack )
/*N*/ 		bGlobalStackInUse = FALSE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		delete pStackObj;
/*N*/ 		delete pErrorStackObj;
/*N*/ 	}
/*N*/ }


/*N*/ void ScInterpreter::GlobalExit()		// static
/*N*/ {
/*N*/ 	DBG_ASSERT(!bGlobalStackInUse, "wer benutzt noch den TokenStack?");
/*N*/ 	DELETEZ(pGlobalStack);
/*N*/ 	DELETEZ(pGlobalErrorStack);
/*N*/ }


/*N*/ StackVar ScInterpreter::Interpret()
/*N*/ {
/*N*/ 	short nRetTypeExpr = NUMBERFORMAT_UNDEFINED;
/*N*/ 	ULONG nRetIndexExpr = 0;
/*N*/ 	USHORT nErrorFunction = 0;
/*N*/ 	USHORT nErrorFunctionCount = 0;
/*N*/ 	USHORT nStackBase;
/*N*/ 
/*N*/ 	nGlobError = nGlobalError;
/*N*/ 	nGlobalError = 0;
/*N*/ 	nMatCount = 0;
/*N*/ 	bMatDel = FALSE;
/*N*/ 	ppGlobSortArray = NULL;
/*N*/ 	nStackBase = sp = maxsp = 0;
/*N*/ 	nRetFmtType = NUMBERFORMAT_UNDEFINED;
/*N*/ 	nFuncFmtType = NUMBERFORMAT_UNDEFINED;
/*N*/ 	nFuncFmtIndex = nCurFmtIndex = nRetFmtIndex = 0;
/*N*/ 	nResult = 0;
/*N*/ 	pResult = NULL;
/*N*/ 	eResult = svDouble;
/*N*/ 	glSubTotal = FALSE;
/*N*/ 	UINT16 nOldOpCode = ocStop;
/*N*/ 
    // Once upon a time we used to have FP exceptions on, and there was a
    // Windows printer driver that kept switching off exceptions, so we had to
    // switch them back on again every time. Who knows if there isn't a driver
    // that keeps switching exceptions on, now that we run with exceptions off,
    // so reassure exceptions are really off.
/*N*/     SAL_MATH_FPEXCEPTIONS_OFF();

/*N*/ 	aCode.Reset();
/*N*/ 	while( ( pCur = aCode.Next() ) != NULL
/*N*/ 			&& (!nGlobalError || nErrorFunction <= nErrorFunctionCount) )
/*N*/ 	{
/*N*/ 		OpCode eOp = pCur->GetOpCode();
/*N*/ 		cPar = pCur->GetByte();
/*N*/ 		if ( eOp == ocPush )
/*N*/ 		{
/*N*/ 			Push( (ScToken&) *pCur );
/*N*/ 			if ( sp <= MAXSTACK )
/*N*/ 				pErrorStack[ sp - 1 ] = 0;		// RPN-Code Push ohne Fehler
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// bisheriger Ausdruck bestimmt das aktuelle Format
/*N*/ 			nCurFmtType = nRetTypeExpr;
/*N*/ 			nCurFmtIndex = nRetIndexExpr;
/*N*/ 			// default Funtionsformat, andere werden bei Bedarf gesetzt
/*N*/ 			nFuncFmtType = NUMBERFORMAT_NUMBER;
/*N*/ 			nFuncFmtIndex = 0;
/*N*/ 
/*N*/ 			if ( eOp == ocIf || eOp == ocChose )
/*N*/ 				nStackBase = sp;		// nicht die Jumps vertueddeln
/*N*/ 			else
/*N*/ 				nStackBase = sp - pCur->GetParamCount();
/*N*/ 			if ( nStackBase > sp )
/*N*/ 				nStackBase = sp;		// underflow?!?
/*N*/ 
/*N*/             switch( eOp )
/*N*/             {
                case ocSep:
                case ocClose:           // vom Compiler gepusht
                case ocMissing          : ScMissing();                  break;
/*N*/                 case ocMacro            : ScMacro();                    break;
                case ocDBArea           : ScDBArea();                   break;
                case ocColRowNameAuto   : ScColRowNameAuto();           break;
// gesondert    case ocPush             : Push( (ScToken&) *pCur );     break;
/*N*/                 case ocIf               : ScIfJump();                   break;
                case ocChose            : ScChoseJump();                break;
/*N*/                 case ocAdd              : ScAdd();                      break;
/*N*/                 case ocSub              : ScSub();                      break;
/*N*/                 case ocMul              : ScMul();                      break;
/*N*/                 case ocDiv              : ScDiv();                      break;
/*N*/                 case ocAmpersand        : ScAmpersand();                break;
                case ocPow              : ScPow();                      break;
/*N*/                 case ocEqual            : ScEqual();                    break;
/*N*/                 case ocNotEqual         : ScNotEqual();                 break;
                case ocLess             : ScLess();                     break;
                case ocGreater          : ScGreater();                  break;
                case ocLessEqual        : ScLessEqual();                break;
                case ocGreaterEqual     : ScGreaterEqual();             break;
                case ocAnd              : ScAnd();                      break;
                case ocOr               : ScOr();                       break;
                case ocIntersect        : ScIntersect();                break;
                case ocNot              : ScNot();                      break;
                case ocNegSub           :
                case ocNeg              : ScNeg();                      break;
                case ocPercentSign      : ScPercentSign();              break;
/*N*/                 case ocPi               : ScPi();                       break;
                case ocRandom           : ScRandom();                   break;
                case ocTrue             : ScTrue();                     break;
                case ocFalse            : ScFalse();                    break;
/*N*/                 case ocGetActDate       : ScGetActDate();               break;
                case ocGetActTime       : ScGetActTime();               break;
                case ocNoValue          : nGlobalError = NOVALUE;
                                            PushInt(0);                 break;
                case ocDeg              : ScDeg();                      break;
                case ocRad              : ScRad();                      break;
/*N*/                 case ocSin              : ScSin();                      break;
                case ocCos              : ScCos();                      break;
                case ocTan              : ScTan();                      break;
                case ocCot              : ScCot();                      break;
                case ocArcSin           : ScArcSin();                   break;
                case ocArcCos           : ScArcCos();                   break;
                case ocArcTan           : ScArcTan();                   break;
                case ocArcCot           : ScArcCot();                   break;
                case ocSinHyp           : ScSinHyp();                   break;
                case ocCosHyp           : ScCosHyp();                   break;
                case ocTanHyp           : ScTanHyp();                   break;
                case ocCotHyp           : ScCotHyp();                   break;
                case ocArcSinHyp        : ScArcSinHyp();                break;
                case ocArcCosHyp        : ScArcCosHyp();                break;
                case ocArcTanHyp        : ScArcTanHyp();                break;
                case ocArcCotHyp        : ScArcCotHyp();                break;
                case ocExp              : ScExp();                      break;
                case ocLn               : ScLn();                       break;
                case ocLog10            : ScLog10();                    break;
                case ocSqrt             : ScSqrt();                     break;
                case ocFact             : ScFact();                     break;
                case ocGetYear          : ScGetYear();                  break;
                case ocGetMonth         : ScGetMonth();                 break;
/*N*/                 case ocGetDay           : ScGetDay();                   break;
/*N*/                 case ocGetDayOfWeek     : ScGetDayOfWeek();             break;
/*N*/                 case ocWeek             : ScGetWeekOfYear();            break;
/*N*/                 case ocEasterSunday     : ScEasterSunday();             break;
                case ocGetHour          : ScGetHour();                  break;
                case ocGetMin           : ScGetMin();                   break;
                case ocGetSec           : ScGetSec();                   break;
/*N*/                 case ocPlusMinus        : ScPlusMinus();                break;
                case ocAbs              : ScAbs();                      break;
/*N*/                 case ocInt              : ScInt();                      break;
                case ocEven             : ScEven();                     break;
                case ocOdd              : ScOdd();                      break;
                case ocPhi              : ScPhi();                      break;
                case ocGauss            : ScGauss();                    break;
                case ocStdNormDist      : ScStdNormDist();              break;
                case ocFisher           : ScFisher();                   break;
                case ocFisherInv        : ScFisherInv();                break;
                case ocIsEmpty          : ScIsEmpty();                  break;
                case ocIsString         : ScIsString();                 break;
                case ocIsNonString      : ScIsNonString();              break;
                case ocIsLogical        : ScIsLogical(nOldOpCode);      break;
                case ocType             : ScType();                     break;
                case ocCell             : ScCell();                     break;
                case ocIsRef            : ScIsRef();                    break;
                case ocIsValue          : ScIsValue();                  break;
                case ocIsFormula        : ScIsFormula();                break;
                case ocFormula          : ScFormula();                  break;
                case ocIsNV             : ScIsNV();                     break;
                case ocIsErr            : ScIsErr();                    break;
                case ocIsError          : ScIsError();                  break;
                case ocIsEven           : ScIsEven();                   break;
                case ocIsOdd            : ScIsOdd();                    break;
                case ocN                : ScN();                        break;
                case ocGetDateValue     : ScGetDateValue();             break;
                case ocGetTimeValue     : ScGetTimeValue();             break;
                case ocCode             : ScCode();                     break;
                case ocTrim             : ScTrim();                     break;
                case ocUpper            : ScUpper();                    break;
                case ocPropper          : ScPropper();                  break;
                case ocLower            : ScLower();                    break;
                case ocLen              : ScLen();                      break;
                case ocT                : ScT();                        break;
                case ocClean            : ScClean();                    break;
                case ocValue            : ScValue();                    break;
                case ocChar             : ScChar();                     break;
                case ocArcTan2          : ScArcTan2();                  break;
/*N*/                 case ocMod              : ScMod();                      break;
                case ocPower            : ScPower();                    break;
                case ocRound            : ScRound();                    break;
/*N*/                 case ocRoundUp          : ScRoundUp();                  break;
                case ocTrunc            :
                case ocRoundDown        : ScRoundDown();                break;
                case ocCeil             : ScCeil();                     break;
                case ocFloor            : ScFloor();                    break;
                case ocSumProduct       : ScSumProduct();               break;
                case ocSumSQ            : ScSumSQ();                    break;
                case ocSumX2MY2         : ScSumX2MY2();                 break;
                case ocSumX2DY2         : ScSumX2DY2();                 break;
                case ocSumXMY2          : ScSumXMY2();                  break;
                case ocLog              : ScLog();                      break;
                case ocGGT              : ScGGT();                      break;
                case ocKGV              : ScKGV();                      break;
                case ocGetDate          : ScGetDate();                  break;
                case ocGetTime          : ScGetTime();                  break;
                case ocGetDiffDate      : ScGetDiffDate();              break;
                case ocGetDiffDate360   : ScGetDiffDate360();           break;
/*N*/                 case ocMin              : ScMin( FALSE );               break;
                case ocMinA             : ScMin( TRUE );                break;
/*N*/                 case ocMax              : ScMax( FALSE );               break;
                case ocMaxA             : ScMax( TRUE );                break;
/*N*/                 case ocSum              : ScSum();                      break;
                case ocProduct          : ScProduct();                  break;
                case ocNBW              : ScNBW();                      break;
                case ocIKV              : ScIKV();                      break;
                case ocMIRR             : ScMIRR();                     break;
                case ocISPMT            : ScISPMT();                    break;
/*N*/                 case ocAverage          : ScAverage( FALSE );           break;
                case ocAverageA         : ScAverage( TRUE );            break;
                case ocCount            : ScCount();                    break;
/*N*/                 case ocCount2           : ScCount2();                   break;
                case ocVar              : ScVar( FALSE );               break;
                case ocVarA             : ScVar( TRUE );                break;
                case ocVarP             : ScVarP( FALSE );              break;
                case ocVarPA            : ScVarP( TRUE );               break;
                case ocStDev            : ScStDev( FALSE );             break;
                case ocStDevA           : ScStDev( TRUE );              break;
                case ocStDevP           : ScStDevP( FALSE );            break;
                case ocStDevPA          : ScStDevP( TRUE );             break;
                case ocBW               : ScBW();                       break;
                case ocDIA              : ScDIA();                      break;
                case ocGDA              : ScGDA();                      break;
                case ocGDA2             : ScGDA2();                     break;
                case ocVBD              : ScVDB();                      break;
                case ocLaufz            : ScLaufz();                    break;
                case ocLIA              : ScLIA();                      break;
                case ocRMZ              : ScRMZ();                      break;
                case ocColumns          : ScColumns();                  break;
                case ocRows             : ScRows();                     break;
                case ocTables           : ScTables();                   break;
                case ocColumn           : ScColumn();                   break;
                case ocRow              : ScRow();                      break;
                case ocTable            : ScTable();                    break;
                case ocZGZ              : ScZGZ();                      break;
                case ocZW               : ScZW();                       break;
                case ocZZR              : ScZZR();                      break;
                case ocZins             : ScZins();                     break;
                case ocZinsZ            : ScZinsZ();                    break;
                case ocKapz             : ScKapz();                     break;
                case ocKumZinsZ         : ScKumZinsZ();                 break;
                case ocKumKapZ          : ScKumKapZ();                  break;
                case ocEffektiv         : ScEffektiv();                 break;
                case ocNominal          : ScNominal();                  break;
/*N*/                 case ocSubTotal         : ScSubTotal();                 break;
                case ocDBSum            : ScDBSum();                    break;
                case ocDBCount          : ScDBCount();                  break;
                case ocDBCount2         : ScDBCount2();                 break;
                case ocDBAverage        : ScDBAverage();                break;
                case ocDBGet            : ScDBGet();                    break;
                case ocDBMax            : ScDBMax();                    break;
                case ocDBMin            : ScDBMin();                    break;
                case ocDBProduct        : ScDBProduct();                break;
                case ocDBStdDev         : ScDBStdDev();                 break;
                case ocDBStdDevP        : ScDBStdDevP();                break;
                case ocDBVar            : ScDBVar();                    break;
                case ocDBVarP           : ScDBVarP();                   break;
                case ocIndirect         : ScIndirect();                 break;
                case ocAdress           : ScAdress();                   break;
                case ocMatch            : ScMatch();                    break;
                case ocCountEmptyCells  : ScCountEmptyCells();          break;
                case ocCountIf          : ScCountIf();                  break;
                case ocSumIf            : ScSumIf();                    break;
                case ocLookup           : ScLookup();                   break;
/*N*/                 case ocVLookup          : ScVLookup();                  break;
                case ocHLookup          : ScHLookup();                  break;
                case ocIndex            : ScIndex();                    break;
                case ocMultiArea        : ScMultiArea();                break;
                case ocOffset           : ScOffset();                   break;
                case ocAreas            : ScAreas();                    break;
                case ocCurrency         : ScCurrency();                 break;
                case ocReplace          : ScReplace();                  break;
                case ocFixed            : ScFixed();                    break;
                case ocFind             : ScFind();                     break;
                case ocExact            : ScExact();                    break;
                case ocLeft             : ScLeft();                     break;
                case ocRight            : ScRight();                    break;
                case ocSearch           : ScSearch();                   break;
                case ocMid              : ScMid();                      break;
/*N*/                 case ocText             : ScText();                     break;
                case ocSubstitute       : ScSubstitute();               break;
                case ocRept             : ScRept();                     break;
                case ocConcat           : ScConcat();                   break;
                case ocMatValue         : ScMatValue();                 break;
                case ocMatrixUnit       : ScEMat();                     break;
                case ocMatDet           : ScMatDet();                   break;
                case ocMatInv           : ScMatInv();                   break;
                case ocMatMult          : ScMatMult();                  break;
                case ocMatTrans         : ScMatTrans();                 break;
/*N*/                 case ocMatRef           : ScMatRef();                   break;
                case ocBackSolver       : ScBackSolver();               break;
                case ocB                : ScB();                        break;
                case ocNormDist         : ScNormDist();                 break;
                case ocExpDist          : ScExpDist();                  break;
                case ocBinomDist        : ScBinomDist();                break;
                case ocPoissonDist      : ScPoissonDist();              break;
                case ocKombin           : ScKombin();                   break;
                case ocKombin2          : ScKombin2();                  break;
                case ocVariationen      : ScVariationen();              break;
                case ocVariationen2     : ScVariationen2();             break;
                case ocHypGeomDist      : ScHypGeomDist();              break;
                case ocLogNormDist      : ScLogNormDist();              break;
                case ocTDist            : ScTDist();                    break;
                case ocFDist            : ScFDist();                    break;
                case ocChiDist          : ScChiDist();                  break;
                case ocStandard         : ScStandard();                 break;
                case ocAveDev           : ScAveDev();                   break;
                case ocDevSq            : ScDevSq();                    break;
                case ocKurt             : ScKurt();                     break;
                case ocSchiefe          : ScSkew();                     break;
                case ocModalValue       : ScModalValue();               break;
                case ocMedian           : ScMedian();                   break;
                case ocGeoMean          : ScGeoMean();                  break;
                case ocHarMean          : ScHarMean();                  break;
                case ocWeibull          : ScWeibull();                  break;
                case ocKritBinom        : ScCritBinom();                break;
                case ocNegBinomVert     : ScNegBinomDist();             break;
                case ocNoName           : ScNoName();                   break;
                case ocZTest            : ScZTest();                    break;
                case ocTTest            : ScTTest();                    break;
                case ocFTest            : ScFTest();                    break;
                case ocRank             : ScRank();                     break;
                case ocPercentile       : ScPercentile();               break;
                case ocPercentrank      : ScPercentrank();              break;
                case ocLarge            : ScLarge();                    break;
                case ocSmall            : ScSmall();                    break;
                case ocFrequency        : ScFrequency();                break;
                case ocQuartile         : ScQuartile();                 break;
                case ocNormInv          : ScNormInv();                  break;
                case ocSNormInv         : ScSNormInv();                 break;
                case ocConfidence       : ScConfidence();               break;
                case ocTrimMean         : ScTrimMean();                 break;
                case ocProb             : ScProbability();              break;
                case ocCorrel           : ScCorrel();                   break;
                case ocCovar            : ScCovar();                    break;
                case ocPearson          : ScPearson();                  break;
                case ocRSQ              : ScRSQ();                      break;
                case ocSTEYX            : ScSTEXY();                    break;
                case ocSlope            : ScSlope();                    break;
                case ocIntercept        : ScIntercept();                break;
                case ocTrend            : ScTrend();                    break;
                case ocGrowth           : ScGrowth();                   break;
                case ocRGP              : ScRGP();                      break;
                case ocRKP              : ScRKP();                      break;
                case ocForecast         : ScForecast();                 break;
                case ocGammaLn          : ScLogGamma();                 break;
                case ocGammaDist        : ScGammaDist();                break;
                case ocGammaInv         : ScGammaInv();                 break;
                case ocChiTest          : ScChiTest();                  break;
                case ocChiInv           : ScChiInv();                   break;
                case ocTInv             : ScTInv();                     break;
                case ocFInv             : ScFInv();                     break;
                case ocLogInv           : ScLogNormInv();               break;
                case ocBetaDist         : ScBetaDist();                 break;
                case ocBetaInv          : ScBetaInv();                  break;
/*N*/                 case ocExternal         : ScExternal();                 break;
                case ocTableOp          : ScTableOp();                  break;
//              case ocErrCell          : ScErrCell();                  break;
                case ocStop :                                           break;
                case ocErrorType        : ScErrorType();                break;
                case ocCurrent          : ScCurrent();                  break;
                case ocStyle            : ScStyle();                    break;
                case ocDde              : ScDde();                      break;
                case ocBase             : ScBase();                     break;
/*N*/                 case ocDecimal          : ScDecimal();                  break;
/*N*/                 case ocConvert          : ScConvert();                  break;
                case ocRoman            : ScRoman();                    break;
                case ocArabic           : ScArabic();                   break;
                case ocAnswer           : ScAnswer();                   break;
                case ocTeam             : ScCalcTeam();                 break;
                case ocTTT              : ScTTT();                      break;
                case ocSpew             : ScSpewFunc();                 break;
                case ocGame             : ScGame();                     break;
/*N*/                 default : SetError(errUnknownOpCode); PushInt(0);       break;
/*N*/             }
/*N*/ 
/*N*/ 			// aeussere Funktion bestimmt das Format eines Ausdrucks
/*N*/ 			if ( nFuncFmtType != NUMBERFORMAT_UNDEFINED )
/*N*/ 			{
/*N*/ 				nRetTypeExpr = nFuncFmtType;
/*N*/ 				// nur fuer Waehrungsformate den FormatIndex uebernehmen
/*N*/ 				nRetIndexExpr = ( nFuncFmtType == NUMBERFORMAT_CURRENCY ?
/*N*/ 					nFuncFmtIndex : 0 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ // Funktionen, die einen Fehlercode auswerten und nGlobalError direkt auf 0 setzen
/*N*/ // usage: switch( OpCode ) { OCERRFUNCCASE( ++n ) }
/*N*/ #define CASEOCERRFUNC( statement ) \
/*N*/ 	case ocErrorType : \
/*N*/ 	case ocIsEmpty : \
/*N*/ 	case ocIsErr : \
/*N*/ 	case ocIsError : \
/*N*/ 	case ocIsFormula : \
/*N*/ 	case ocIsLogical : \
/*N*/ 	case ocIsNV : \
/*N*/ 	case ocIsNonString : \
/*N*/ 	case ocIsRef : \
/*N*/ 	case ocIsString : \
/*N*/ 	case ocIsValue : \
/*N*/ 	case ocN : \
/*N*/ 	case ocType : \
/*N*/ 		statement;
/*N*/ 
/*N*/ 		switch ( eOp )
/*N*/ 		{
/*N*/ 			CASEOCERRFUNC( ++nErrorFunction )
/*N*/ 		}
/*N*/ 		if ( nGlobalError )
/*N*/ 		{
/*N*/ 			if ( !nErrorFunctionCount )
/*N*/ 			{	// Anzahl der Fehlercode-Funktionen in Formel
/*N*/ 				for ( ScToken* t = rArr.FirstRPN();	t; t = rArr.NextRPN() )
/*N*/ 				{
/*N*/ 					switch ( t->GetOpCode() )
/*N*/ 					{
/*N*/ 						CASEOCERRFUNC( ++nErrorFunctionCount )
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( nErrorFunction >= nErrorFunctionCount )
/*N*/ 				++nErrorFunction;	// das war's, Fehler => Abbruch
/*N*/ 			else
/*N*/ 			{
/*?*/ 				if ( eOp != ocPush && sp > nStackBase + 1 )
/*?*/ 				{	// Stack abraeumen, geht davon aus, dass jede Funktion
/*?*/ 					// prinzipiell ein Ergebnis pusht, im Fehlerfall kann dies
/*?*/ 					// ein zufaelliger Wert sein
/*?*/ 					const ScToken* pResult = pStack[ sp - 1 ];
/*?*/ 					while ( sp > nStackBase )
/*?*/ 						Pop();
/*?*/ 					PushTempToken( *pResult );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		nOldOpCode = eOp;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ende: Returnwert holen
/*N*/ 
/*N*/ 	if( sp )
/*N*/ 	{
/*N*/ 		pCur = pStack[ sp-1 ];
/*N*/ 		if( pCur->GetOpCode() == ocPush )
/*N*/ 		{
/*N*/ 			if ( !nGlobalError )
/*N*/ 				nGlobalError = pErrorStack[ sp-1 ];
/*N*/ 			switch( eResult = pCur->GetType() )
/*N*/ 			{
/*N*/ 				case svDouble :
/*N*/ 					nResult = pCur->GetDouble();
/*N*/ 					if ( nFuncFmtType == NUMBERFORMAT_UNDEFINED )
/*N*/ 					{
/*N*/ 						nRetTypeExpr = NUMBERFORMAT_NUMBER;
/*N*/ 						nRetIndexExpr = 0;
/*N*/ 					}
/*N*/ 				break;
/*N*/ 				case svString :
/*N*/ 					nRetTypeExpr = NUMBERFORMAT_TEXT;
/*N*/ 					nRetIndexExpr = 0;
/*N*/ 					aResult = PopString();
/*N*/ 				break;
/*N*/ 				case svSingleRef :
/*N*/ 				{
/*N*/ 					ScAddress aAdr;
/*N*/ 					PopSingleRef( aAdr );
/*N*/ 					if( !nGlobalError )
/*N*/ 					{
/*N*/ 						ScBaseCell* pCell = pDok->GetCell( aAdr );
/*N*/ 						if( pCell && pCell->HasStringData() )
/*N*/ 						{
/*N*/ 							GetCellString( aResult, pCell );
/*N*/ 							nRetTypeExpr = NUMBERFORMAT_TEXT;
/*N*/ 							nRetIndexExpr = 0;
/*N*/ 							eResult = svString;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							nResult = GetCellValue( aAdr, pCell );
/*N*/ 							nRetTypeExpr = nCurFmtType;
/*N*/ 							nRetIndexExpr = nCurFmtIndex;
/*N*/ 							eResult = svDouble;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 				case svDoubleRef :
/*N*/ 				{
/*?*/ 					if ( bMatrixFormula )
/*?*/ 					{	// Matrix erzeugen fuer {=A1:A5}
/*?*/ 						PopDoubleRefPushMatrix();
/*?*/ 						//	kein break, weiter mit svMatrix
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						ScRange aRange;
/*?*/ 						PopDoubleRef( aRange );
/*?*/ 						ScAddress aAdr;
/*?*/ 						if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
/*?*/ 						{
/*?*/ 							ScBaseCell* pCell = pDok->GetCell( aAdr );
/*?*/ 							if( pCell && pCell->HasStringData() )
/*?*/ 							{
/*?*/ 								GetCellString( aResult, pCell );
/*?*/ 								nRetTypeExpr = NUMBERFORMAT_TEXT;
/*?*/ 								nRetIndexExpr = 0;
/*?*/ 								eResult = svString;
/*?*/ 							}
/*?*/ 							else
/*?*/ 							{
/*?*/ 								nResult = GetCellValue( aAdr, pCell );
/*?*/ 								nRetTypeExpr = nCurFmtType;
/*?*/ 								nRetIndexExpr = nCurFmtIndex;
/*?*/ 								eResult = svDouble;
/*?*/ 							}
/*?*/ 						}
/*?*/ 						break;
/*?*/ 					}
/*?*/ 				}
/*?*/ 				//	kein break
/*N*/ 				case svMatrix :
/*N*/ 					pResult = PopMatrix();
/*N*/ 					if (pResult)
/*N*/ 					{
/*N*/ 						BOOL bIsString;
/*N*/ 						const MatValue* pMatVal = pResult->Get(0, 0, bIsString);
/*N*/ 						if ( pMatVal )
/*N*/ 						{
/*N*/ 							if (bIsString)
/*N*/ 							{
/*?*/ 								aResult = pMatVal->GetString();
/*?*/ 								eResult = svString;
/*?*/ 								nRetTypeExpr = NUMBERFORMAT_TEXT;
/*?*/ 								nRetIndexExpr = 0;
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								nResult = pMatVal->fVal;
/*N*/ 								eResult = svDouble;
/*N*/ 								if ( nRetTypeExpr != NUMBERFORMAT_LOGICAL )
/*N*/ 									nRetTypeExpr = NUMBERFORMAT_NUMBER;
/*N*/ 								nRetIndexExpr = 0;
/*N*/ 							}
/*N*/ 						}
/*N*/ 						else
/*N*/ 							SetError(errUnknownStackVariable);
/*N*/ 						DBG_ASSERT(nRetMat <= MAX_ANZ_MAT,
/*N*/ 										"ScInterpreter::nRetMat falsch");
/*N*/ 						ResetNewMat(nRetMat);			// Matrix nicht loeschen
/*N*/ 					}
/*N*/ 					else
/*N*/ 						eResult = svDouble;
/*N*/ 					break;
/*N*/ 				default :
/*N*/ 					SetError(errUnknownStackVariable);
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			SetError(errUnknownStackVariable);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SetError(errNoCode);
/*N*/ 
/*N*/ 	if (!::rtl::math::isFinite(nResult))
/*N*/ 	{
/*N*/         if ( ::rtl::math::isNan( nResult ) )
/*N*/             SetError(errNoValue);
/*N*/         else
/*N*/             SetError(errIllegalFPOperation);
/*N*/ 		nResult = 0.0;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nRetTypeExpr != NUMBERFORMAT_UNDEFINED )
/*N*/ 	{
/*N*/ 		nRetFmtType = nRetTypeExpr;
/*N*/ 		nRetFmtIndex = nRetIndexExpr;
/*N*/ 	}
/*N*/ 	else if( nFuncFmtType != NUMBERFORMAT_UNDEFINED )
/*N*/ 	{
/*N*/ 		nRetFmtType = nFuncFmtType;
/*N*/ 		nRetFmtIndex = nFuncFmtIndex;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nRetFmtType = NUMBERFORMAT_NUMBER;
/*N*/ 	// nur fuer Waehrungsformate den FormatIndex uebernehmen
/*N*/ 	if ( nRetFmtType != NUMBERFORMAT_CURRENCY )
/*N*/ 		nRetFmtIndex = 0;
/*N*/ 
/*N*/ 	// grrr.. EiterZirkel!
/*N*/ 	// Fehler nur zuruecksetzen wenn nicht errCircularReference ohne Iterationen
/*N*/ 	if ( nGlobalError || !(rArr.GetError() == errCircularReference && !pDok->GetDocOptions().IsIter()) )
/*N*/ 		rArr.SetError( nGlobalError );
/*N*/ 
/*N*/ 	if (ppGlobSortArray)
/*?*/ #ifdef WIN
/*?*/ 		SvMemFree(*ppGlobSortArray);
/*?*/ #else
/*?*/ 		delete [] (*ppGlobSortArray);
/*?*/ #endif
/*N*/ 	if (bMatDel)
/*N*/ 	{
/*N*/ 		for (USHORT i = 0; i < MAX_ANZ_MAT; i++)
/*N*/ 			delete ppTempMatArray[i];
/*N*/ 		delete [] ppTempMatArray;
/*N*/ 	}
/*N*/ 	// Tokens im ExprStack freigeben
/*N*/ 	ScToken** p = pStack;
/*N*/ 	while( maxsp-- )
/*N*/ 		(*p++)->DecRef();
/*N*/ 	nGlobalError = nGlobError;
/*N*/ 	return eResult;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
