/*************************************************************************
 *
 *  $RCSfile: interpr4.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: er $ $Date: 2001-03-21 12:10:11 $
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


#include <rangelst.hxx>
#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <basic/sbstar.hxx>
#include <svtools/sbx.hxx>
#include <svtools/zforlist.hxx>
#include <tools/urlobj.hxx>
#include <segmentc.hxx>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <float.h>

#include <com/sun/star/table/XCellRange.hpp>

using namespace com::sun::star;

#include "interpre.hxx"
#include "global.hxx"
#include "dbcolect.hxx"
#include "cell.hxx"
#include "callform.hxx"
#include "addincol.hxx"
#include "document.hxx"
#include "dociter.hxx"
#include "docoptio.hxx"
#include "scmatrix.hxx"
#include "adiasync.hxx"
#include "sc.hrc"
#include "scdll.hxx"        // ScLibSignalFunc
#include "cellsuno.hxx"
#include "optuno.hxx"
#include "rangeseq.hxx"
#include "addinlis.hxx"

// Implementiert in ui\miscdlgs\teamdlg.cxx

extern void ShowTheTeam();

extern BOOL bOderSo; // in GLOBAL.CXX

//-----------------------------statische Daten-----------------

USHORT ScInterpreter::nGlobalError = 0; // fuer matherr

#if SOMA_FPSIGNAL_JUMP
jmp_buf* ScInterpreter::pJumpBuf = NULL;    // fuer die mySignal-Funktion
ScLibSignalFunc ScInterpreter::pSignalFunc = NULL;  // signal() Wrapper der App
#endif

#pragma code_seg("SCSTATICS")

TabOpList ScInterpreter::aTableOpList;
#if SC_SPEW_ENABLED
ScSpew ScInterpreter::theSpew;
#endif

#pragma code_seg()

//-------------------------------------------------------

#if SOMA_FPSIGNAL_JUMP

#if ( defined (MSC) && defined (WIN) && ! ( defined (WNT) ) )
extern "C" int _matherr(struct _exception *except);
#elif defined (OS2) && !defined( WTC )
extern "C" int _matherr(struct exception *except);
#elif defined (OS2) && defined( WTC )
extern "C" double _matherr(struct exception *except);
#elif defined (LINUX)
extern "C" int matherr(struct __exception *except);
#elif defined (UNX) && !defined (LINUX)
extern "C" int matherr(struct exception *except);
#endif

#ifndef _UNDERFLOW
#define _UNDERFLOW UNDERFLOW
#endif

#if ( defined (MSC) && defined (WIN) && ! ( defined (WNT) ) )


int _matherr(struct _exception *except)
{
    if( except->type == _UNDERFLOW )
        except->retval = 0.0;
    else
    {
        ScInterpreter::SetError(errIllegalFPOperation);
        except->retval = 1.0;
    }
    return 1;
};

#elif defined (OS2)


#ifndef WTC
int _matherr(struct exception *except)
#else
double _matherr(struct exception *except)
#endif
{
    if( except->type == UNDERFLOW )
        except->retval = 0.0;
    else
    {
        ScInterpreter::SetError(errIllegalFPOperation);
        except->retval = 1.0;
    }
    return 1;
};
#elif ( defined UNX && !defined LINUX )


int matherr(struct exception *except)
{
    if( except->type == UNDERFLOW )
        except->retval = 0.0;
    else
    {
        ScInterpreter::SetError(errIllegalFPOperation);
        except->retval = 1.0;
    }
    return 1;
};
#endif

#endif


void ScInterpreter::MySigFunc(int sig)
{
#if SOMA_FPSIGNAL_JUMP
    SOMA_FPRESET(); // reinitialize math package
    longjmp(*ScInterpreter::pJumpBuf,1);
#endif
}

/*  inline:

void ScInterpreter::SetError(USHORT nError)
{
    if( nError && !nGlobalError )
        nGlobalError = nError;
}
*/

//-------------------------------------------------------------------------
// Funktionen für den Zugriff auf das Document
//-------------------------------------------------------------------------


void ScInterpreter::ReplaceCell( ScAddress& rPos )
{
    USHORT nCol, nRow, nTab;
    nCol = rPos.Col();
    nRow = rPos.Row();
    nTab = rPos.Tab();
    USHORT* pTOp = aTableOpList.First();
    while (pTOp)
    {
        if (nCol == pTOp[0] && nRow == pTOp[1] && nTab == pTOp[2])
        {
            rPos.Set( pTOp[3], pTOp[4], pTOp[5] );
            return ;
        }
        else if (nCol == pTOp[6] && nRow == pTOp[7] && nTab == pTOp[8])
        {
            rPos.Set( pTOp[9], pTOp[10], pTOp[11] );
            return ;
        }
        else
            pTOp = aTableOpList.Next();
    }
}


void ScInterpreter::ReplaceCell( USHORT& rCol, USHORT& rRow, USHORT& rTab )
{
    USHORT* pTOp = aTableOpList.First();
    while (pTOp)
    {
        if (rCol == pTOp[0] && rRow == pTOp[1] && rTab == pTOp[2])
        {
            rCol = pTOp[3];
            rRow = pTOp[4];
            rTab = pTOp[5];
            return ;
        }
        else if (rCol == pTOp[6] && rRow == pTOp[7] && rTab == pTOp[8])
        {
            rCol = pTOp[9];
            rRow = pTOp[10];
            rTab = pTOp[11];
            return ;
        }
        else
            pTOp = aTableOpList.Next();
    }
}


BOOL ScInterpreter::IsTableOpInRange( const ScRange& rRange )
{
    if ( rRange.aStart == rRange.aEnd )
        return FALSE;   // not considered to be a range in TableOp sense

    // we can't replace a single cell in a range
    USHORT* pTOp = aTableOpList.First();
    while (pTOp)
    {
        ScAddress aAdr( pTOp[0], pTOp[1], pTOp[2] );
        if ( rRange.In( aAdr ) )
            return TRUE;
        aAdr.Set( pTOp[6], pTOp[7], pTOp[8] );
        if ( rRange.In( aAdr ) )
            return TRUE;
        pTOp = aTableOpList.Next();
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
double ScInterpreter::GetValueCellValue( const ScAddress& rPos, const ScValueCell* pCell )
{
    double fVal = pCell->GetValue();
    if ( bCalcAsShown && fVal != 0.0 )
    {
        ULONG nFormat = pDok->GetNumberFormat( rPos );
        fVal = pDok->RoundValueAsShown( fVal, nFormat );
    }
    return fVal;
}


double ScInterpreter::GetCellValue( const ScAddress& rPos, const ScBaseCell* pCell )
{
    USHORT nErr = nGlobalError;
    nGlobalError = 0;
    double nVal = GetCellValueOrZero( rPos, pCell );
    if ( !nGlobalError || nGlobalError == errNoValue )
        nGlobalError = nErr;
    return nVal;
}


double ScInterpreter::GetCellValueOrZero( const ScAddress& rPos, const ScBaseCell* pCell )
{
    double fValue;
    if (pCell)
    {
        CellType eType = pCell->GetCellType();
        switch ( eType )
        {
            case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
                USHORT nErr = pFCell->GetErrCode();
                if( !nErr )
                {
                    if (pFCell->IsValue())
                    {
                        fValue = pFCell->GetValue();
                        pDok->GetNumberFormatInfo( nCurFmtType, nCurFmtIndex,
                            rPos, *pFCell );
                    }
                    else
                    {
                        SetError(errNoValue);
                        fValue = 0.0;
                    }
                }
                else
                {
                    fValue = 0.0;
                    SetError(nErr);
                }
            }
            break;
            case CELLTYPE_VALUE:
            {
                fValue = ((ScValueCell*)pCell)->GetValue();
                nCurFmtIndex = pDok->GetNumberFormat( rPos );
                nCurFmtType = pFormatter->GetType( nCurFmtIndex );
                if ( bCalcAsShown && fValue != 0.0 )
                    fValue = pDok->RoundValueAsShown( fValue, nCurFmtIndex );
            }
            break;
            case  CELLTYPE_STRING:
            case  CELLTYPE_EDIT:
#if 0
// Xcl macht es so, aber dann gibt z.B. SUMME(A1:A2) was anderes als A1+A2.
// In dem Fall muesste GetCellValue den errNoValue auch durchreichen.
            {
                String aStr;
                if ( eType == CELLTYPE_STRING )
                    ((ScStringCell*)pCell)->GetString( aStr );
                else
                    ((ScEditCell*)pCell)->GetString( aStr );
                ULONG nFIndex = 0;                  // damit default Land/Spr.
                if ( !pFormatter->IsNumberFormat( aStr, nFIndex, fValue ) )
                {
                    SetError(errNoValue);
                    fValue = 0.0;
                }
            }
            break;
#endif
            default:
                SetError(errNoValue);
                fValue = 0.0;
        }
    }
    else
        fValue = 0.0;
    return fValue;
}


void ScInterpreter::GetCellString( String& rStr, const ScBaseCell* pCell )
{
    USHORT nErr = 0;
    if (pCell)
    {
        switch (pCell->GetCellType())
        {
            case CELLTYPE_STRING:
                ((ScStringCell*) pCell)->GetString(rStr);
            break;
            case CELLTYPE_EDIT:
                ((ScEditCell*) pCell)->GetString(rStr);
            break;
            case CELLTYPE_FORMULA:
            {
                ScFormulaCell* pFCell = (ScFormulaCell*) pCell;
                nErr = pFCell->GetErrCode();
                if (pFCell->IsValue())
                {
                    double fVal = pFCell->GetValue();
                    ULONG nIndex = pFormatter->GetStandardFormat(
                                        NUMBERFORMAT_NUMBER,
                                        ScGlobal::eLnge);
                    pFormatter->GetInputLineString(fVal, nIndex, rStr);
                }
                else
                    pFCell->GetString(rStr);
            }
            break;
            case CELLTYPE_VALUE:
            {
                double fVal = ((ScValueCell*) pCell)->GetValue();
                ULONG nIndex = pFormatter->GetStandardFormat(
                                        NUMBERFORMAT_NUMBER,
                                        ScGlobal::eLnge);
                pFormatter->GetInputLineString(fVal, nIndex, rStr);
            }
            break;
            default:
                rStr = ScGlobal::GetEmptyString();
            break;
        }
    }
    else
        rStr = ScGlobal::GetEmptyString();
    SetError(nErr);
}


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
                    USHORT  nErr = 0;
                    double  nVal = 0.0;
                    BOOL    bOk = TRUE;
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
                    String  aStr;
                    USHORT  nErr = 0;
                    BOOL    bOk = TRUE;
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
                    USHORT  nErr = 0;
                    USHORT  nType = 0; // 0 = Zahl; 1 = String
                    double  nVal = 0.0;
                    String  aStr;
                    BOOL    bOk = TRUE;
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


void ScInterpreter::Push( ScToken& r )
{
    if ( sp >= MAXSTACK )
        SetError( errStackOverflow );
    else
    {
        nCurFmtType = NUMBERFORMAT_UNDEFINED;
        r.IncRef();
        if( sp >= maxsp )
            maxsp = sp + 1;
        else
            pStack[ sp ]->DecRef();
        pStack[ sp ] = (ScToken*) &r;
        pErrorStack[ sp ] = nGlobalError;
        ++sp;
    }
}

// Schlichtes Wegwerfen von TOS


void ScInterpreter::Pop()
{
    if( sp )
        sp--;
    else
        SetError(errUnknownStackVariable);
}

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


void ScInterpreter::PushTempToken( const ScToken& r )
{
    if ( sp >= MAXSTACK )
        SetError( errStackOverflow );
    else
    {
        ScToken* p = r.Clone();
        p->IncRef();
        if( sp >= maxsp )
            maxsp = sp + 1;
        else
            pStack[ sp ]->DecRef();
        pStack[ sp ] = p;
        pErrorStack[ sp ] = nGlobalError;
        ++sp;
    }
}


//! The Token had to be allocated with `new' and must not be used after this
//! call because eventually it gets deleted in case of a errStackOverflow if
//! no RefCount was set!
void ScInterpreter::PushTempToken( ScToken* p )
{
    ((ScToken*)p)->IncRef();
    if ( sp >= MAXSTACK )
    {
        SetError( errStackOverflow );
        ((ScToken*)p)->DecRef();
    }
    else
    {
        if( sp >= maxsp )
            maxsp = sp + 1;
        else
            pStack[ sp ]->DecRef();
        pStack[ sp ] = (ScToken*) p;
        pErrorStack[ sp ] = nGlobalError;
        ++sp;
    }
}


double ScInterpreter::PopDouble()
{
    nCurFmtType = NUMBERFORMAT_NUMBER;
    nCurFmtIndex = 0;
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svDouble )
            return p->GetDouble();
        else if( p->GetType() == svMissing )
            return 0.0;
    }
    SetError(errUnknownStackVariable);
    return 0.0;
}


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


const String& ScInterpreter::PopString()
{
    nCurFmtType = NUMBERFORMAT_TEXT;
    nCurFmtIndex = 0;
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svString )
            return p->GetString();
        else if( p->GetType() == svMissing )
            return EMPTY_STRING;
    }
    SetError(errUnknownStackVariable);
    return EMPTY_STRING;
}


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
            if ( aTableOpList.Count() > 0 )
                ReplaceCell( rCol, rRow, rTab );
            return;
        }
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownStackVariable);
}


void ScInterpreter::PopSingleRef( ScAddress& rAdr )
{
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svSingleRef )
        {
            short nCol, nRow, nTab;
            const SingleRefData& rRef = p->GetSingleRef();
            if ( rRef.IsColRel() )
                nCol = aPos.Col() + rRef.nRelCol;
            else
                nCol = rRef.nCol;
            if ( rRef.IsRowRel() )
                nRow = aPos.Row() + rRef.nRelRow;
            else
                nRow = rRef.nRow;
            if ( rRef.IsTabRel() )
                nTab = aPos.Tab() + rRef.nRelTab;
            else
                nTab = rRef.nTab;
            if( nCol < 0 || nCol > MAXCOL || rRef.IsColDeleted() )
                SetError( errNoRef ), nCol = 0;
            if( nRow < 0 || nRow > MAXROW || rRef.IsRowDeleted() )
                SetError( errNoRef ), nRow = 0;
            if( nTab < 0 || nTab >= pDok->GetTableCount() || rRef.IsTabDeleted() )
                SetError( errNoRef ), nTab = 0;
            rAdr.Set( (USHORT)nCol, (USHORT)nRow, (USHORT)nTab );
            if ( aTableOpList.Count() > 0 )
                ReplaceCell( rAdr );
            return;
        }
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownStackVariable);
}


void ScInterpreter::PopDoubleRef(USHORT& rCol1, USHORT &rRow1, USHORT& rTab1,
                                 USHORT& rCol2, USHORT &rRow2, USHORT& rTab2,
                                 BOOL bDontCheckForTableOp )
{
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svDoubleRef )
        {
            const ComplRefData& rCRef = p->GetDoubleRef();
            USHORT nMaxTab = pDok->GetTableCount();
            {
                const SingleRefData& rRef = rCRef.Ref1;
                if ( rRef.IsColRel() )
                    rCol1 = aPos.Col() + rRef.nRelCol;
                else
                    rCol1 = rRef.nCol;
                if ( rRef.IsRowRel() )
                    rRow1 = aPos.Row() + rRef.nRelRow;
                else
                    rRow1 = rRef.nRow;
                if ( rRef.IsTabRel() )
                    rTab1 = aPos.Tab() + rRef.nRelTab;
                else
                    rTab1 = rRef.nTab;
                if( rCol1 < 0 || rCol1 > MAXCOL || rRef.IsColDeleted() )
                    SetError( errNoRef ), rCol1 = 0;
                if( rRow1 < 0 || rRow1 > MAXROW || rRef.IsRowDeleted() )
                    SetError( errNoRef ), rRow1 = 0;
                if( rTab1 < 0 || rTab1 >= nMaxTab || rRef.IsTabDeleted() )
                    SetError( errNoRef ), rTab1 = 0;
            }
            {
                const SingleRefData& rRef = rCRef.Ref2;
                if ( rRef.IsColRel() )
                    rCol2 = aPos.Col() + rRef.nRelCol;
                else
                    rCol2 = rRef.nCol;
                if ( rRef.IsRowRel() )
                    rRow2 = aPos.Row() + rRef.nRelRow;
                else
                    rRow2 = rRef.nRow;
                if ( rRef.IsTabRel() )
                    rTab2 = aPos.Tab() + rRef.nRelTab;
                else
                    rTab2 = rRef.nTab;
                if( rCol2 < 0 || rCol2 > MAXCOL || rRef.IsColDeleted() )
                    SetError( errNoRef ), rCol2 = 0;
                if( rRow2 < 0 || rRow2 > MAXROW || rRef.IsRowDeleted() )
                    SetError( errNoRef ), rRow2 = 0;
                if( rTab2 < 0 || rTab2 >= nMaxTab || rRef.IsTabDeleted() )
                    SetError( errNoRef ), rTab2 = 0;
            }
            if ( aTableOpList.Count() > 0 && !bDontCheckForTableOp )
            {
                ScRange aRange( rCol1, rRow1, rTab1, rCol2, rRow2, rTab2 );
                if ( IsTableOpInRange( aRange ) )
                    SetError( errIllegalParameter );
            }
            return;
        }
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownStackVariable);
}


void ScInterpreter::PopDoubleRef( ScRange& rRange, BOOL bDontCheckForTableOp )
{
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svDoubleRef )
        {
            const ComplRefData& rCRef = p->GetDoubleRef();
            short nCol, nRow, nTab;
            USHORT nMaxTab = pDok->GetTableCount();
            {
                const SingleRefData& rRef = rCRef.Ref1;
                if ( rRef.IsColRel() )
                    nCol = aPos.Col() + rRef.nRelCol;
                else
                    nCol = rRef.nCol;
                if ( rRef.IsRowRel() )
                    nRow = aPos.Row() + rRef.nRelRow;
                else
                    nRow = rRef.nRow;
                if ( rRef.IsTabRel() )
                    nTab = aPos.Tab() + rRef.nRelTab;
                else
                    nTab = rRef.nTab;
                if( nCol < 0 || nCol > MAXCOL || rRef.IsColDeleted() )
                    SetError( errNoRef ), nCol = 0;
                if( nRow < 0 || nRow > MAXROW || rRef.IsRowDeleted() )
                    SetError( errNoRef ), nRow = 0;
                if( nTab < 0 || nTab >= nMaxTab || rRef.IsTabDeleted() )
                    SetError( errNoRef ), nTab = 0;
                rRange.aStart.Set( (USHORT)nCol, (USHORT)nRow, (USHORT)nTab );
            }
            {
                const SingleRefData& rRef = rCRef.Ref2;
                if ( rRef.IsColRel() )
                    nCol = aPos.Col() + rRef.nRelCol;
                else
                    nCol = rRef.nCol;
                if ( rRef.IsRowRel() )
                    nRow = aPos.Row() + rRef.nRelRow;
                else
                    nRow = rRef.nRow;
                if ( rRef.IsTabRel() )
                    nTab = aPos.Tab() + rRef.nRelTab;
                else
                    nTab = rRef.nTab;
                if( nCol < 0 || nCol > MAXCOL || rRef.IsColDeleted() )
                    SetError( errNoRef ), nCol = 0;
                if( nRow < 0 || nRow > MAXROW || rRef.IsRowDeleted() )
                    SetError( errNoRef ), nRow = 0;
                if( nTab < 0 || nTab >= nMaxTab || rRef.IsTabDeleted() )
                    SetError( errNoRef ), nTab = 0;
                rRange.aEnd.Set( (USHORT)nCol, (USHORT)nRow, (USHORT)nTab );
            }
            if ( aTableOpList.Count() > 0 && !bDontCheckForTableOp )
            {
                if ( IsTableOpInRange( rRange ) )
                    SetError( errIllegalParameter );
            }
            return;
        }
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownStackVariable);
}


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
        break;
        case svSingleRef :
        {
            PopSingleRef( rAdr );
            return TRUE;
        }
        break;
        default:
            Pop();
            SetError( errNoRef );
    }
    return FALSE;
}


void ScInterpreter::PopDoubleRefPushMatrix()
{
    if ( GetStackType() == svDoubleRef )
    {
        USHORT nMatInd;
        ScMatrix* pMat = GetMatrix( nMatInd );
        if ( pMat )
        {
            PushMatrix( pMat );
            nRetMat = nMatInd;
        }
        else
            SetIllegalParameter();
    }
    else
        SetError( errNoRef );
}


ScMatrix* ScInterpreter::PopMatrix()
{
    if( sp )
    {
        --sp;
        ScToken* p = pStack[ sp ];
        if ( !nGlobalError )
            nGlobalError = pErrorStack[ sp ];
        if( p->GetType() == svMatrix )
            return p->GetMatrix();
        else if( p->GetType() == svMissing )
            SetError( errIllegalParameter );
    }
    SetError(errUnknownVariable);
    return NULL;
}



void ScInterpreter::PushDouble(double nVal)
{
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(nVal))
    {
        nVal = 0.0;
        SetError(errIllegalFPOperation);
    }
#endif
    PushTempToken( new ScDoubleToken( nVal ) );
}


void ScInterpreter::PushInt(int nVal)
{
    PushTempToken( new ScDoubleToken( nVal ) );
}


void ScInterpreter::PushStringBuffer( const sal_Unicode* pString )
{
    if ( pString )
        PushString( String( pString ) );
    else
        PushString( EMPTY_STRING );
}


void ScInterpreter::PushString( const String& rString )
{
    PushTempToken( new ScStringToken( rString ) );
}


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


void ScInterpreter::PushMatrix(ScMatrix* pMat)
{
    PushTempToken( new ScMatrixToken( pMat ) );
}


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


void ScInterpreter::SetIllegalArgument()
{
    SetError(errIllegalArgument);
    PushInt(0);
}


void ScInterpreter::SetNV()
{
    SetError(NOVALUE);
    PushInt(0);
}


void ScInterpreter::SetNoValue()
{
    SetError(errNoValue);
    PushInt(0);
}


StackVar ScInterpreter::GetStackType()
{
    StackVar eRes;
    if( sp )
    {
        eRes = pStack[sp - 1]->GetType();
        if( eRes == svMissing )
            eRes = svDouble;    // default!
    }
    else
    {
        SetError(errUnknownStackVariable);
        eRes = svErr;
    }
    return eRes;
}


StackVar ScInterpreter::GetStackType( BYTE nParam )
{
    StackVar eRes;
    if( sp > nParam-1 )
    {
        eRes = pStack[sp - nParam]->GetType();
        if( eRes == svMissing )
            eRes = svDouble;    // default!
    }
    else
        eRes = svErr;
    return eRes;
}


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
            ;   // all done
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


double ScInterpreter::GetDouble()
{
    double nVal;
    switch( GetStackType() )
    {
        case svDouble:
            nVal = PopDouble(); break;
        case svString:
        {
            String aStr(PopString());
            ULONG nFIndex = 0;                  // damit default Land/Spr.
            if(!pFormatter->IsNumberFormat( aStr, nFIndex, nVal ) )
            {
                SetError(errIllegalArgument);
                nVal = 0.0;
            }
        }
        break;
        case svSingleRef:
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            ScBaseCell* pCell = GetCell( aAdr );
            nVal = GetCellValue( aAdr, pCell );
        }
        break;
        case svDoubleRef:
        {   // positionsabhaengige SingleRef generieren
            ScRange aRange;
            PopDoubleRef( aRange );
            ScAddress aAdr;
            if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
            {
                ScBaseCell* pCell = GetCell( aAdr );
                nVal = GetCellValue( aAdr, pCell );
            }
            else
                nVal = 0.0;
        }
        break;
        default:
            Pop();
            SetError(errIllegalParameter);
            nVal = 0.0;
    }
    if ( nFuncFmtType == nCurFmtType )
        nFuncFmtIndex = nCurFmtIndex;
    return nVal;
}


const String& ScInterpreter::GetString()
{
    StackVar eRes = (StackVar) GetStackType();
    if( eRes == svDouble && pStack[ sp-1 ]->GetType() == svMissing )
        eRes = svString;
    switch( eRes )
    {
        case svDouble:
        {
            double fVal = PopDouble();
            ULONG nIndex = pFormatter->GetStandardFormat(
                                    NUMBERFORMAT_NUMBER,
                                    ScGlobal::eLnge);
            pFormatter->GetInputLineString(fVal, nIndex, aTempStr);
            return aTempStr;
        }
        break;
        case svString:
            return PopString();
        break;
        case svSingleRef:
        {
            ScAddress aAdr;
            PopSingleRef( aAdr );
            if (nGlobalError == 0)
            {
                ScBaseCell* pCell = GetCell( aAdr );
                GetCellString( aTempStr, pCell );
                return aTempStr;
            }
            else
                return EMPTY_STRING;
        }
        break;
        case svDoubleRef:
        {   // positionsabhaengige SingleRef generieren
            ScRange aRange;
            PopDoubleRef( aRange );
            ScAddress aAdr;
            if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
            {
                ScBaseCell* pCell = GetCell( aAdr );
                GetCellString( aTempStr, pCell );
                return aTempStr;
            }
            else
                return EMPTY_STRING;
        }
        break;
        default:
            Pop();
            SetError(errIllegalParameter);
    }
    return EMPTY_STRING;
}


void ScInterpreter::ScDBGet()
{
    USHORT nTab;
    ScQueryParam aQueryParam;
    if (GetDBParams(nTab, aQueryParam))
    {
        ScBaseCell* pCell;
        ScQueryCellIterator aCellIter(pDok, nTab, aQueryParam);
        if (pCell = aCellIter.GetFirst())
        {
            if (aCellIter.GetNext())
                SetIllegalArgument();
            else
            {
                switch (pCell->GetCellType())
                {
                    case CELLTYPE_VALUE:
                    {
                        double rValue = ((ScValueCell*)pCell)->GetValue();
                        if ( bCalcAsShown )
                        {
                            ULONG nFormat;
                            nFormat = aCellIter.GetNumberFormat();
                            rValue = pDok->RoundValueAsShown( rValue, nFormat );
                        }
                        PushDouble(rValue);
                    }
                    break;
                    case CELLTYPE_STRING:
                    {
                        String rString;
                        ((ScStringCell*)pCell)->GetString(rString);
                        PushString(rString);
                    }
                    break;
                    case CELLTYPE_EDIT:
                    {
                        String rString;
                        ((ScEditCell*)pCell)->GetString(rString);
                        PushString(rString);
                    }
                    break;
                    case CELLTYPE_FORMULA:
                    {
                        USHORT rErr = ((ScFormulaCell*)pCell)->GetErrCode();
                        if (rErr)
                        {
                            SetError(rErr);
                            PushInt(0);
                        }
                        else if (((ScFormulaCell*)pCell)->IsValue())
                        {
                            double rValue = ((ScFormulaCell*)pCell)->GetValue();
                            PushDouble(rValue);
                        }
                        else
                        {
                            String rString;
                            ((ScFormulaCell*)pCell)->GetString(rString);
                            PushString(rString);
                        }
                    }
                    break;
                    case CELLTYPE_NONE:
                    case CELLTYPE_NOTE:
                    default:
                        SetIllegalArgument();
                    break;
                }
            }
        }
        else
            SetNoValue();
    }
    else
        SetIllegalParameter();
}


void ScInterpreter::ScExternal()
{
    USHORT nIndex;
    BYTE nParamCount = GetByte();
    String aUnoName;
    String aFuncName( pCur->GetExternal() );
    if (ScGlobal::GetFuncCollection()->SearchFunc(aFuncName, nIndex))
    {
        FuncData* pFuncData = (FuncData*)ScGlobal::GetFuncCollection()->At(nIndex);
        if (nParamCount == pFuncData->GetParamCount() - 1)
        {
            ParamType   eParamType[MAXFUNCPARAM];
            void*       ppParam[MAXFUNCPARAM];
            double      nVal[MAXFUNCPARAM];
            sal_Char*   pStr[MAXFUNCPARAM];
            BYTE*       pCellArr[MAXFUNCPARAM];
            short       i;

            for (i = 0; i < MAXFUNCPARAM; i++)
            {
                eParamType[i] = pFuncData->GetParamType(i);
                ppParam[i] = NULL;
                nVal[i] = 0.0;
                pStr[i] = NULL;
                pCellArr[i] = NULL;
            }

            for (i = nParamCount; (i > 0) && (nGlobalError == 0); i--)
            {
                BYTE nStackType = GetStackType();
                switch (eParamType[i])
                {
                    case PTR_DOUBLE :
                        {
                            nVal[i-1] = GetDouble();
                            ppParam[i] = &nVal[i-1];
                        }
                        break;
                    case PTR_STRING :
                        {
                            ByteString aStr( GetString(), osl_getThreadTextEncoding() );
                            if ( aStr.Len() >= MAXSTRLEN )
                                SetError( errStringOverflow );
                            else
                            {
                                pStr[i-1] = new sal_Char[MAXSTRLEN];
                                strcpy( pStr[i-1], aStr.GetBuffer() );
                                ppParam[i] = pStr[i-1];
                            }
                        }
                        break;
                    case PTR_DOUBLE_ARR :
                        {
                            USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                            pCellArr[i-1] = new BYTE[MAXARRSIZE];
                            if (!CreateDoubleArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
                                SetError(errCodeOverflow);
                            else
                                ppParam[i] = pCellArr[i-1];
                        }
                        break;
                    case PTR_STRING_ARR :
                        {
                            USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                            pCellArr[i-1] = new BYTE[MAXARRSIZE];
                            if (!CreateStringArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
                                SetError(errCodeOverflow);
                            else
                                ppParam[i] = pCellArr[i-1];
                        }
                        break;
                    case PTR_CELL_ARR :
                        {
                            USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                            PopDoubleRef(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2);
                            pCellArr[i-1] = new BYTE[MAXARRSIZE];
                            if (!CreateCellArr(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, pCellArr[i-1]))
                                SetError(errCodeOverflow);
                            else
                                ppParam[i] = pCellArr[i-1];
                        }
                        break;
                    default :
                        SetError(errIllegalParameter);
                        break;
                }
            }
            while ( i-- )
                Pop();      // im Fehlerfall (sonst ist i==0) Parameter wegpoppen

            if (nGlobalError == 0)
            {
                if ( pFuncData->GetAsyncType() == NONE )
                {
                    switch ( eParamType[0] )
                    {
                        case PTR_DOUBLE :
                        {
                            double nErg = 0.0;
                            ppParam[0] = &nErg;
                            pFuncData->Call(ppParam);
                            PushDouble(nErg);
                        }
                        break;
                        case PTR_STRING :
                        {
                            sal_Char* pcErg = new sal_Char[MAXSTRLEN];
                            ppParam[0] = pcErg;
                            pFuncData->Call(ppParam);
                            String aUni( pcErg, osl_getThreadTextEncoding() );
                            PushString( aUni );
                            delete[] pcErg;
                        }
                        break;
                        default:
                            SetError( errUnknownState );
                            PushInt(0);
                    }
                }
                else
                {
                    // nach dem Laden Asyncs wieder anwerfen
                    if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
                        pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();
                    // garantiert identischer Handle bei identischem Aufruf?!?
                    // sonst schei*e ...
                    double nErg = 0.0;
                    ppParam[0] = &nErg;
                    pFuncData->Call(ppParam);
                    ULONG nHandle = ULONG( nErg );
                    if ( nHandle >= 65536 )
                    {
                        ScAddInAsync* pAs = ScAddInAsync::Get( nHandle );
                        if ( !pAs )
                        {
                            pAs = new ScAddInAsync( nHandle, nIndex, pDok );
                            pMyFormulaCell->StartListening( *pAs, TRUE );
                        }
                        else
                        {
                            // falls per cut/copy/paste
                            if ( !pMyFormulaCell->IsListening( *pAs ) )
                                pMyFormulaCell->StartListening( *pAs, TRUE );
                            // in anderes Dokument?
                            if ( !pAs->HasDocument( pDok ) )
                                pAs->AddDocument( pDok );
                        }
                        if ( pAs->IsValid() )
                        {
                            switch ( pAs->GetType() )
                            {
                                case PTR_DOUBLE :
                                    PushDouble( pAs->GetValue() );
                                    break;
                                case PTR_STRING :
                                    PushString( pAs->GetString() );
                                    break;
                                default:
                                    SetError( errUnknownState );
                                    PushInt(0);
                            }
                        }
                        else
                            SetNV();
                    }
                    else
                        SetNoValue();
                }
            }

            for (i = 0; i < MAXFUNCPARAM; i++)
            {
                delete[] pStr[i];
                delete[] pCellArr[i];
            }
        }
        else
        {
            while( nParamCount-- )
                Pop();
            SetError(errIllegalParameter);
            PushInt(0);
        }
    }
    else if ( ( aUnoName = ScGlobal::GetAddInCollection()->FindFunction(aFuncName, FALSE) ).Len()  )
    {
        //  bLocalFirst=FALSE in FindFunction, cFunc should be the stored internal name

        ScUnoAddInCall aCall( *ScGlobal::GetAddInCollection(), aUnoName, nParamCount );

        if ( !aCall.ValidParamCount() )
            SetError( errIllegalParameter );

        if ( aCall.NeedsCaller() && !GetError() )
        {
            SfxObjectShell* pShell = pDok->GetDocumentShell();
            if (pShell)
                aCall.SetCallerFromObjectShell( pShell );
            else
            {
                // use temporary model object (without document) to supply options
                aCall.SetCaller( static_cast<beans::XPropertySet*>(
                                    new ScDocOptionsObj( pDok->GetDocOptions() ) ) );
            }
        }

        short nPar = nParamCount;
        while ( nPar && !GetError() )
        {
            --nPar;     // 0 .. (nParamCount-1)

            ScAddInArgumentType eType = aCall.GetArgType( nPar );
            BYTE nStackType = GetStackType();

            uno::Any aParam;
            switch (eType)
            {
                case SC_ADDINARG_INTEGER:
                    {
                        double fVal = GetDouble();
                        double fInt = (fVal >= 0.0) ? SolarMath::ApproxFloor( fVal ) :
                                                      SolarMath::ApproxCeil( fVal );
                        if ( fInt >= LONG_MIN && fInt <= LONG_MAX )
                            aParam <<= (INT32)fInt;
                        else
                            SetError(errIllegalArgument);
                    }
                    break;

                case SC_ADDINARG_DOUBLE:
                    aParam <<= (double) GetDouble();
                    break;

                case SC_ADDINARG_STRING:
                    aParam <<= rtl::OUString( GetString() );
                    break;

                case SC_ADDINARG_INTEGER_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                double fVal = GetDouble();
                                double fInt = (fVal >= 0.0) ? SolarMath::ApproxFloor( fVal ) :
                                                              SolarMath::ApproxCeil( fVal );
                                if ( fInt >= LONG_MIN && fInt <= LONG_MAX )
                                {
                                    INT32 nIntVal = (long)fInt;
                                    uno::Sequence<INT32> aInner( &nIntVal, 1 );
                                    uno::Sequence< uno::Sequence<INT32> > aOuter( &aInner, 1 );
                                    aParam <<= aOuter;
                                }
                                else
                                    SetError(errIllegalArgument);
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillLongArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillLongArray( aParam, PopMatrix() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            Pop();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_DOUBLE_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                double fVal = GetDouble();
                                uno::Sequence<double> aInner( &fVal, 1 );
                                uno::Sequence< uno::Sequence<double> > aOuter( &aInner, 1 );
                                aParam <<= aOuter;
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillDoubleArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillDoubleArray( aParam, PopMatrix() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            Pop();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_STRING_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                rtl::OUString aString = rtl::OUString( GetString() );
                                uno::Sequence<rtl::OUString> aInner( &aString, 1 );
                                uno::Sequence< uno::Sequence<rtl::OUString> > aOuter( &aInner, 1 );
                                aParam <<= aOuter;
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillStringArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillStringArray( aParam, PopMatrix(), pFormatter ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            Pop();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_MIXED_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                        case svString:
                        case svSingleRef:
                            {
                                uno::Any aElem;
                                if ( nStackType == svDouble )
                                    aElem <<= (double) GetDouble();
                                else if ( nStackType == svString )
                                    aElem <<= rtl::OUString( GetString() );
                                else
                                {
                                    ScAddress aAdr;
                                    if ( PopDoubleRefOrSingleRef( aAdr ) )
                                    {
                                        ScBaseCell* pCell = GetCell( aAdr );
                                        if ( pCell && pCell->HasStringData() )
                                        {
                                            String aStr;
                                            GetCellString( aStr, pCell );
                                            aElem <<= rtl::OUString( aStr );
                                        }
                                        else
                                            aElem <<= (double) GetCellValue( aAdr, pCell );
                                    }
                                }
                                uno::Sequence<uno::Any> aInner( &aElem, 1 );
                                uno::Sequence< uno::Sequence<uno::Any> > aOuter( &aInner, 1 );
                                aParam <<= aOuter;
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillMixedArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillMixedArray( aParam, PopMatrix() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            Pop();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_VALUE_OR_ARRAY:
                    switch( nStackType )
                    {
                        case svDouble:
                            aParam <<= (double) GetDouble();
                            break;
                        case svString:
                            aParam <<= rtl::OUString( GetString() );
                            break;
                        case svSingleRef:
                            {
                                ScAddress aAdr;
                                if ( PopDoubleRefOrSingleRef( aAdr ) )
                                {
                                    ScBaseCell* pCell = GetCell( aAdr );
                                    if ( pCell && pCell->HasStringData() )
                                    {
                                        String aStr;
                                        GetCellString( aStr, pCell );
                                        aParam <<= rtl::OUString( aStr );
                                    }
                                    else
                                        aParam <<= (double) GetCellValue( aAdr, pCell );
                                }
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                if (!ScRangeToSequence::FillMixedArray( aParam, pDok, aRange ))
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svMatrix:
                            if (!ScRangeToSequence::FillMixedArray( aParam, PopMatrix() ))
                                SetError(errIllegalParameter);
                            break;
                        default:
                            Pop();
                            SetError(errIllegalParameter);
                    }
                    break;

                case SC_ADDINARG_CELLRANGE:
                    switch( nStackType )
                    {
                        case svSingleRef:
                            {
                                ScAddress aAdr;
                                PopSingleRef( aAdr );
                                ScRange aRange( aAdr );
                                uno::Reference<table::XCellRange> xObj =
                                        ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
                                if (xObj.is())
                                    aParam <<= xObj;
                                else
                                    SetError(errIllegalParameter);
                            }
                            break;
                        case svDoubleRef:
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                uno::Reference<table::XCellRange> xObj =
                                        ScCellRangeObj::CreateRangeFromDoc( pDok, aRange );
                                if (xObj.is())
                                    aParam <<= xObj;
                                else
                                    SetError(errIllegalParameter);
                            }
                            break;
                        default:
                            Pop();
                            SetError(errIllegalParameter);
                    }
                    break;

                default:
                    Pop();
                    SetError(errIllegalParameter);
            }
            aCall.SetParam( nPar, aParam );
        }

        while (nPar--)
            Pop();                  // in case of error, remove remaining args

        if ( !GetError() )
        {
            aCall.ExecuteCall();

            if ( aCall.HasVarRes() )                        // handle async functions
            {
                if ( pMyFormulaCell->GetCode()->IsRecalcModeNormal() )
                    pMyFormulaCell->GetCode()->SetRecalcModeOnLoad();

                uno::Reference<sheet::XVolatileResult> xResult = aCall.GetVarRes();
                ScAddInListener* pLis = ScAddInListener::Get( xResult );
                if ( !pLis )
                {
                    pLis = ScAddInListener::CreateListener( xResult, pDok );
                    pMyFormulaCell->StartListening( *pLis, TRUE );
                }
                else
                {
                    if ( !pMyFormulaCell->IsListening( *pLis ) )
                        pMyFormulaCell->StartListening( *pLis, TRUE );
                    if ( !pLis->HasDocument( pDok ) )
                        pLis->AddDocument( pDok );
                }

                aCall.SetResult( pLis->GetResult() );       // use result from async
            }

            if ( aCall.GetErrCode() )
            {
                SetError( aCall.GetErrCode() );
                PushInt(0);
            }
            else if ( aCall.HasMatrix() )
            {
                const ScMatrix* pLinkMat = aCall.GetMatrix();       // not NULL

                USHORT nC, nR, nMatInd;                             // copy matrix result
                pLinkMat->GetDimensions(nC, nR);
                ScMatrix* pNewMat = GetNewMat( nC, nR, nMatInd );
                if (pNewMat)
                {
                    pLinkMat->MatCopy(*pNewMat);
                    PushMatrix( pNewMat );
                    nRetMat = nMatInd;
                }                               // otherwise error code has been set in GetNewMat
            }
            else if ( aCall.HasString() )
                PushString( aCall.GetString() );
            else
                PushDouble( aCall.GetValue() );
        }
        else                // error...
            PushInt(0);
    }
    else
    {
        while( nParamCount-- )
            Pop();
        SetError(errNoName);
        PushInt(0);
    }
}


void ScInterpreter::ScMissing()
{
    PushTempToken( new ScMissingToken );
}


void ScInterpreter::ScMacro()
{
    SbxBase::ResetError();

    BYTE nParamCount = GetByte();
    String aMacro( pCur->GetExternal() );

    SfxObjectShell* pDocSh = pDok->GetDocumentShell();
    if ( !pDocSh || !pDok->CheckMacroWarn() )
    {
        SetError( errNoValue );     // ohne DocShell kein CallBasic
        return;
    }

    //  keine Sicherheitsabfrage mehr vorneweg (nur CheckMacroWarn), das passiert im CallBasic

    SfxApplication* pSfxApp = SFX_APP();
    pSfxApp->EnterBasicCall();              // Dok-Basic anlegen etc.

    //  Wenn das Dok waehrend eines Basic-Calls geladen wurde,
    //  ist das Sbx-Objekt evtl. nicht angelegt (?)
//  pDocSh->GetSbxObject();

    //  Funktion ueber den einfachen Namen suchen,
    //  dann aBasicStr, aMacroStr fuer SfxObjectShell::CallBasic zusammenbauen

    StarBASIC* pRoot = pDocSh->GetBasic();
    SbxVariable* pVar = pRoot->Find( aMacro, SbxCLASS_METHOD );
    if( !pVar || pVar->GetType() == SbxVOID || !pVar->ISA(SbMethod) )
    {
        SetError( errNoName );
        pSfxApp->LeaveBasicCall();
        return;
    }

    SbMethod* pMethod = (SbMethod*)pVar;
    SbModule* pModule = pMethod->GetModule();
    SbxObject* pObject = pModule->GetParent();
    DBG_ASSERT(pObject->IsA(TYPE(StarBASIC)), "Kein Basic gefunden!");
    String aMacroStr = pObject->GetName();
    aMacroStr += '.';
    aMacroStr += pModule->GetName();
    aMacroStr += '.';
    aMacroStr += pMethod->GetName();
    String aBasicStr;
    if (pObject->GetParent())
        aBasicStr = pObject->GetParent()->GetName();    // Dokumentenbasic
    else
        aBasicStr = SFX_APP()->GetName();               // Applikationsbasic

    //  Parameter-Array zusammenbauen

    SbxArrayRef refPar = new SbxArray;
    BOOL bOk = TRUE;
    for( short i = nParamCount; i && bOk ; i-- )
    {
        SbxVariable* pPar = refPar->Get( (USHORT) i );
        BYTE nStackType = GetStackType();
        switch( nStackType )
        {
            case svDouble:
                pPar->PutDouble( GetDouble() );
            break;
            case svString:
                pPar->PutString( GetString() );
            break;
            case svSingleRef:
            {
                ScAddress aAdr;
                PopSingleRef( aAdr );
                bOk = SetSbxVariable( pPar, aAdr );
            }
            break;
            case svDoubleRef:
            {
                USHORT nCol1, nRow1, nTab1, nCol2, nRow2, nTab2;
                PopDoubleRef( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );
                if( nTab1 != nTab2 )
                {
                    SetError( errIllegalParameter );
                    bOk = FALSE;
                }
                else
                {
                    SbxDimArrayRef refArray = new SbxDimArray;
                    refArray->AddDim( 1, nRow2 - nRow1 + 1 );
                    refArray->AddDim( 1, nCol2 - nCol1 + 1 );
                    ScAddress aAdr( nCol1, nRow1, nTab1 );
                    for( USHORT nRow = nRow1; bOk && nRow <= nRow2; nRow++ )
                    {
                        aAdr.SetRow( nRow );
                        short nIdx[ 2 ];
                        nIdx[ 0 ] = nRow-nRow1+1;
                        for( USHORT nCol = nCol1; bOk && nCol <= nCol2; nCol++ )
                        {
                            aAdr.SetCol( nCol );
                            nIdx[ 1 ] = nCol-nCol1+1;
                            SbxVariable* p = refArray->Get( nIdx );
                            bOk = SetSbxVariable( p, aAdr );
                        }
                    }
                    pPar->PutObject( refArray );
                }
            }
            break;
            case svMatrix:
            {
                ScMatrix* pMat = PopMatrix();
                USHORT nC, nR;
                if (pMat)
                {
                    pMat->GetDimensions(nC, nR);
                    SbxDimArrayRef refArray = new SbxDimArray;
                    refArray->AddDim( 1, nR );
                    refArray->AddDim( 1, nC );
                    for( USHORT j = 0; j < nR; j++ )
                    {
                        short nIdx[ 2 ];
                        nIdx[ 0 ] = j+1;
                        for( USHORT i = 0; i < nC; i++ )
                        {
                            nIdx[ 1 ] = i+1;
                            SbxVariable* p = refArray->Get( nIdx );
                            if (pMat->IsString(i, j))
                                p->PutString( pMat->GetString(i, j) );
                            else
                                p->PutDouble( pMat->GetDouble(i, j) );
                        }
                    }
                    pPar->PutObject( refArray );
                }
                else
                    SetError( errIllegalParameter );
            }
            break;
            default:
                SetError( errIllegalParameter );
                bOk = FALSE;
        }
    }
    if( bOk )
    {
        pDok->LockTable( aPos.Tab() );
        SbxVariableRef refRes = new SbxVariable;
        pDok->IncMacroInterpretLevel();
        ErrCode eRet = pDocSh->CallBasic( aMacroStr, aBasicStr, NULL, refPar, refRes );
        pDok->DecMacroInterpretLevel();
        pDok->UnlockTable( aPos.Tab() );

        SbxDataType eResType = refRes->GetType();
        if ( eRet != ERRCODE_NONE )
            SetNoValue();
        else if( eResType >= SbxINTEGER && eResType <= SbxDOUBLE )
            PushDouble( refRes->GetDouble() );
        else if ( eResType & SbxARRAY )
        {
            SbxBase* pElemObj = refRes->GetObject();
            SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,pElemObj);
            short nDim = pDimArray->GetDims();
            if ( 1 <= nDim && nDim <= 2 )
            {
                short nCs, nCe, nRs, nRe;
                USHORT nC, nR, nMatInd;
                USHORT nColIdx, nRowIdx;
                if ( nDim == 1 )
                {   // array( cols )  eine Zeile, mehrere Spalten
                    pDimArray->GetDim( 1, nCs, nCe );
                    nC = USHORT(nCe - nCs + 1);
                    nRs = nRe = 0;
                    nR = 1;
                    nColIdx = 0;
                    nRowIdx = 1;
                }
                else
                {   // array( rows, cols )
                    pDimArray->GetDim( 1, nRs, nRe );
                    nR = USHORT(nRe - nRs + 1);
                    pDimArray->GetDim( 2, nCs, nCe );
                    nC = USHORT(nCe - nCs + 1);
                    nColIdx = 1;
                    nRowIdx = 0;
                }
                ScMatrix* pMat = GetNewMat( nC, nR, nMatInd );
                if ( pMat )
                {
                    SbxVariable* pV;
                    SbxDataType eType;
                    for ( USHORT j=0; j < nR; j++ )
                    {
                        short nIdx[ 2 ];
                        // bei eindimensionalem array( cols ) wird nIdx[1]
                        // von SbxDimArray::Get ignoriert
                        nIdx[ nRowIdx ] = nRs + j;
                        for ( USHORT i=0; i < nC; i++ )
                        {
                            nIdx[ nColIdx ] = nCs + i;
                            pV = pDimArray->Get( nIdx );
                            eType = pV->GetType();
                            if ( eType >= SbxINTEGER && eType <= SbxDOUBLE )
                                pMat->PutDouble( pV->GetDouble(), i, j );
                            else
                                pMat->PutString( pV->GetString(), i, j );
                        }
                    }
                    PushMatrix( pMat );
                    nRetMat = nMatInd;
                }
            }
            else
                SetNoValue();
        }
        else
            PushString( refRes->GetString() );
        if( pVar->GetError() )
            SetNoValue();
    }

    pSfxApp->LeaveBasicCall();
}


BOOL ScInterpreter::SetSbxVariable( SbxVariable* pVar, const ScAddress& rPos )
{
    BOOL bOk = TRUE;
    ScBaseCell* pCell = pDok->GetCell( rPos );
    if (pCell)
    {
        USHORT nErr;
        double nVal;
        switch( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE :
                nVal = GetValueCellValue( rPos, (ScValueCell*)pCell );
                pVar->PutDouble( nVal );
                break;
            case CELLTYPE_STRING :
            {
                String aVal;
                ((ScStringCell*)pCell)->GetString( aVal );
                pVar->PutString( aVal );
                break;
            }
            case CELLTYPE_EDIT :
            {
                String aVal;
                ((ScEditCell*) pCell)->GetString( aVal );
                pVar->PutString( aVal );
                break;
            }
            case CELLTYPE_FORMULA :
                nErr = ((ScFormulaCell*)pCell)->GetErrCode();
                if( !nErr )
                {
                    if( ((ScFormulaCell*)pCell)->IsValue() )
                    {
                        nVal = ((ScFormulaCell*)pCell)->GetValue();
                        pVar->PutDouble( nVal );
                    }
                    else
                    {
                        String aVal;
                        ((ScFormulaCell*)pCell)->GetString( aVal );
                        pVar->PutString( aVal );
                    }
                }
                else
                    SetError( nErr ), bOk = FALSE;
                break;
            default :
                pVar->PutDouble( 0.0 );
        }
    }
    else
        pVar->PutDouble( 0.0 );
    return bOk;
}


void ScInterpreter::ScTableOp()
{
    BYTE nParamCount = GetByte();
    if (nParamCount != 3 && nParamCount != 5)
    {
        SetIllegalParameter();
        return;
    }
    ScAddress aFAdr;
    USHORT nOCol1, nORow1, nOTab1, nNCol1, nNRow1, nNTab1;
    USHORT nOCol2, nORow2, nOTab2, nNCol2, nNRow2, nNTab2;
    ScRange aRange2;
    if (nParamCount == 5)
    {
        PopSingleRef(nNCol2, nNRow2, nNTab2);
        PopSingleRef(nOCol2, nORow2, nOTab2);
        aRange2.aStart.Set( nOCol2, nORow2, nOTab2 );
        aRange2.aEnd = aRange2.aStart;
    }
    else
        nOCol2 = MAXCOL+1;
    PopSingleRef(nNCol1, nNRow1, nNTab1);
    PopSingleRef(nOCol1, nORow1, nOTab1);
    ScRange aRange1( nOCol1, nORow1, nOTab1 );
    PopSingleRef( aFAdr );
    pTableOp = new USHORT[12];
    pTableOp[0]  = nOCol1;
    pTableOp[1]  = nORow1;
    pTableOp[2]  = nOTab1;
    pTableOp[3]  = nNCol1;
    pTableOp[4]  = nNRow1;
    pTableOp[5]  = nNTab1;
    pTableOp[6]  = nOCol2;
    pTableOp[7]  = nORow2;
    pTableOp[8]  = nOTab2;
    pTableOp[9]  = nNCol2;
    pTableOp[10] = nNRow2;
    pTableOp[11] = nNTab2;
    aTableOpList.Insert(pTableOp);

    pDok->IncInterpreterTableOpLevel();

    pDok->SetTableOpDirty( aRange1 );
    if ( nOCol2 != MAXCOL+1 )
        pDok->SetTableOpDirty( aRange2 );
    ScBaseCell* pFCell = pDok->GetCell( aFAdr );
    if (pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA)
        ((ScFormulaCell*)pFCell)->SetDirtyVar();
    if (HasCellValueData(pFCell))
        PushDouble(GetCellValue( aFAdr, pFCell ));
    else
    {
        String aCellString;
        GetCellString(aCellString, pFCell);
        PushString(aCellString);
    }
    if (pTableOp)
    {
        aTableOpList.Remove(pTableOp);
        delete [] pTableOp;
        pTableOp = NULL;
    }
    pDok->SetTableOpDirty( aRange1 );
    if ( nOCol2 != MAXCOL+1 )
        pDok->SetTableOpDirty( aRange2 );
    if (pFCell && pFCell->GetCellType() == CELLTYPE_FORMULA)
    {
        ((ScFormulaCell*)pFCell)->SetDirtyVar();
        ((ScFormulaCell*)pFCell)->GetErrCode();     // recalculate original
    }

    pDok->DecInterpreterTableOpLevel();
}

/*

void ScInterpreter::ScErrCell()
{
    double fErrNum = GetDouble();
    SetError((USHORT) fErrNum);
    PushInt(0);
}
*/


void ScInterpreter::ScDefPar()
{
    PushInt(0);
}


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
        pDok->GetDataArea(  (USHORT) aRefData.Ref1.nTab,
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
        {   // ColName
            aRefData.Ref2.nCol = nStartCol;
            // evtl. vorherige Begrenzung durch definierte ColRowNameRanges erhalten
            if ( aRefData.Ref2.nRow > nRow2 )
                aRefData.Ref2.nRow = nRow2;
            USHORT nMyRow;
            if ( aPos.Col() == nStartCol
              && nStartRow <= (nMyRow = aPos.Row()) && nMyRow <= aRefData.Ref2.nRow )
            {   // Formel in gleicher Spalte und innerhalb des Range
                if ( nMyRow == nStartRow )
                {   // direkt unter dem Namen den Rest nehmen
                    nStartRow++;
                    if ( nStartRow > MAXROW )
                        nStartRow = MAXROW;
                    aRefData.Ref1.nRow = nStartRow;
                }
                else
                {   // weiter unten vom Namen bis zur Formelzelle
                    aRefData.Ref2.nRow = nMyRow - 1;
                }
            }
        }
        else
        {   // RowName
            aRefData.Ref2.nRow = nStartRow;
            // evtl. vorherige Begrenzung durch definierte ColRowNameRanges erhalten
            if ( aRefData.Ref2.nCol > nCol2 )
                aRefData.Ref2.nCol = nCol2;
            USHORT nMyCol;
            if ( aPos.Row() == nStartRow
              && nStartCol <= (nMyCol = aPos.Col()) && nMyCol <= aRefData.Ref2.nCol )
            {   // Formel in gleicher Zeile und innerhalb des Range
                if ( nMyCol == nStartCol )
                {   // direkt neben dem Namen den Rest nehmen
                    nStartCol++;
                    if ( nStartCol > MAXCOL )
                        nStartCol = MAXCOL;
                    aRefData.Ref1.nCol = nStartCol;
                }
                else
                {   // weiter rechts vom Namen bis zur Formelzelle
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
        ShowTheTeam();
        String aTeam( RTL_CONSTASCII_USTRINGPARAM( "Timm, Ballach, Rathke, Rentz, Nebel" ) );
        if ( (GetByte() == 1) && SolarMath::ApproxEqual( GetDouble(), 1996) )
            aTeam.AppendAscii( "   (a word with 'B': -Olk, -Nietsch, -Daeumling)" );
        PushString( aTeam );
        bShown = TRUE;
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
        theSpew.Clear();        // GetSpew liest SpewRulesFile neu
    theSpew.GetSpew( aStr );
    if ( bClear )
        theSpew.Clear();        // release Memory
    xub_StrLen nPos = 0;
    while ( (nPos = aStr.SearchAndReplace( '\n', ' ', nPos )) != STRING_NOTFOUND )
        nPos++;
#else
    aStr.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "spitted out all spew :-(" ) );
#endif
    PushString( aStr );
}


#include "sctictac.hxx"

//#define SC_INVADER_GPF        // GPF wollen wir nicht :-(
// zum testen Environment-Variable SC_INVADER_GPF=xxx setzen
// 08.10.98: wenn PB optpath.cxx gefixt hat geht's wieder

extern void StartInvader( Window* pParent );    // StarWars, Wrapper in SVX options/optpath.cxx
extern void Game();                     // Froggie
void ScInterpreter::ScGame()
{
    enum GameType {
        SC_GAME_NONE,
        SC_GAME_ONCE,
        SC_GAME_START,
        SC_GAME_TICTACTOE = SC_GAME_START,
        SC_GAME_STARWARS,
        SC_GAME_FROGGER,
        SC_GAME_COUNT
    };
    // ein grep im binary laeuft ins leere
    static sal_Char sGameNone[]         = "\14\36\6\137\10\27\36\13\100";
    static sal_Char sGameOnce[]         = "\20\27\137\21\20\123\137\21\20\13\137\36\30\36\26\21\136";
    static sal_Char sGameTicTacToe[]    = "\53\26\34\53\36\34\53\20\32";
    static sal_Char sGameStarWars[]     = "\54\13\36\15\50\36\15\14";
    static sal_Char sGameFrogger[]      = "\71\15\20\30\30\26\32";
    sal_Char* const pGames[SC_GAME_COUNT] = {
        sGameNone,
        sGameOnce,
        sGameTicTacToe,
        sGameStarWars,
        sGameFrogger
    };
#if 0
say what?
oh no, not again!
TicTacToe
StarWars
Froggie
// Routine um Datenblock zu erzeugen:
#include <stdio.h>
int main()
{
    int b = 1;
    int c;
    while ( (c = getchar()) != EOF )
    {
        if ( b == 1 )
        {
            printf( "\"" );
            b = 0;
        }
        if ( c != 10 )
        {
            c ^= 0x7F;
            printf( "\\%o", c );

        }
        else
        {
            printf( "\";\n" );
            b = 1;
        }
    }
    return 0;
}
#endif
    static BOOL bRun[SC_GAME_COUNT] = { FALSE };
    static BOOL bFirst = TRUE;
    if ( bFirst )
    {
        bFirst = FALSE;
        for ( int j = SC_GAME_NONE; j < SC_GAME_COUNT; j++ )
        {
            sal_Char* p = pGames[j];
            while ( *p )
                *p++ ^= 0x7F;
        }
    }
    String aResult;
    GameType eGame = SC_GAME_NONE;
    BYTE nParamCount = GetByte();
    if ( nParamCount >= 1 )
    {
        String aStr( GetString() );
        nParamCount--;
        for ( int j = SC_GAME_START; j < SC_GAME_COUNT; j++ )
        {
            if ( aStr.EqualsAscii( pGames[j] ) )
            {
                eGame = (GameType) j;
                break;  // for
            }
        }
        if ( eGame != SC_GAME_NONE )
        {
            // jedes Game nur ein einziges Mal starten, um nicht durch Recalc
            // o.ae. mehrere Instanzen zu haben, ideal waere eine Abfrage an den
            // Games, ob sie bereits laufen ...
            if ( bRun[ eGame ] && eGame != SC_GAME_TICTACTOE )
                eGame = SC_GAME_ONCE;
            else
            {
                bRun[ eGame ] = TRUE;
                switch ( eGame )
                {
                    case SC_GAME_TICTACTOE :
                    {
                        static ScTicTacToe* pTicTacToe = NULL;
                        static ScRange aTTTrange;
                        static BOOL bHumanFirst = FALSE;
                        if ( nParamCount >= 1 )
                        {
                            if ( GetStackType() == svDoubleRef )
                            {
                                ScRange aRange;
                                PopDoubleRef( aRange );
                                nParamCount--;
                                if ( aRange.aEnd.Col() - aRange.aStart.Col() == 2
                                  && aRange.aEnd.Row() - aRange.aStart.Row() == 2 )
                                {
                                    BOOL bOk;
                                    if ( pTicTacToe )
                                        bOk = (aRange == aTTTrange);
                                    else
                                    {
                                        bOk =TRUE;
                                        aTTTrange = aRange;
                                        pTicTacToe = new ScTicTacToe( pDok,
                                            aRange.aStart );
                                        pTicTacToe->Initialize( bHumanFirst );
                                    }
                                    // nur einmal und das auf dem gleichen Range
                                    if ( !bOk )
                                        eGame = SC_GAME_ONCE;
                                    else
                                    {
                                        Square_Type aWinner = pTicTacToe->CalcMove();
                                        pTicTacToe->GetOutput( aResult );
                                        if ( aWinner != pTicTacToe->GetEmpty() )
                                        {
                                            delete pTicTacToe;
                                            pTicTacToe = NULL;
                                            bRun[ eGame ] = FALSE;
                                            bHumanFirst = !bHumanFirst;
                                        }
                                        pDok->GetDocumentShell()->Broadcast(
                                            SfxSimpleHint( FID_DATACHANGED ) );
                                        pDok->ResetChanged( aRange );
                                    }
                                }
                                else
                                    SetError( errIllegalArgument );
                            }
                            else
                                SetError( errIllegalParameter );
                        }
                        else
                            SetError( errIllegalParameter );
                    }
                    break;
                    case SC_GAME_STARWARS :
#ifdef SC_INVADER_GPF
                        if ( getenv( "SC_INVADER_GPF" ) )
#endif
//                          StartInvader( Application::GetDefDialogParent() );
                    break;
                    case SC_GAME_FROGGER :
                        //Game();
                    break;
                }
            }
        }
    }
    // Stack aufraeumen
    while ( nParamCount-- )
        Pop();
    if ( !aResult.Len() )
        PushString( String( pGames[ eGame ], RTL_TEXTENCODING_ASCII_US ) );
    else
        PushString( aResult );
}

void ScInterpreter::ScTTT()
{   // Temporaerer Test-Tanz, zum auspropieren von Funktionen etc.
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
    static const sal_Unicode __FAR_DATA sEyes[]     = { ',',';',':','|','8','B', 0 };
    static const sal_Unicode __FAR_DATA sGoods[]    = { ')',']','}', 0 };
    static const sal_Unicode __FAR_DATA sBads[]     = { '(','[','{','/', 0 };
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


ScInterpreter::ScInterpreter( ScFormulaCell* pCell, ScDocument* pDoc,
        const ScAddress& rPos, ScTokenArray& r ) :
    pMyFormulaCell( pCell ),
    pDok( pDoc ),
    aCode( r ),
    aPos( rPos ),
    rArr( r ),
    bCalcAsShown( pDoc->GetDocOptions().IsCalcAsShown() ),
    pFormatter( pDoc->GetFormatTable() )
{
//  pStack = new ScToken*[ MAXSTACK ];

    BYTE cMatFlag = pMyFormulaCell->GetMatrixFlag();
    bMatrixFormula = ( cMatFlag == MM_FORMULA || cMatFlag == MM_FAKE );
    if (!bGlobalStackInUse)
    {
        bGlobalStackInUse = TRUE;
        if (!pGlobalStack)
            pGlobalStack = new ScTokenStack;
        if (!pGlobalErrorStack)
            pGlobalErrorStack = new ScErrorStack;
        pStackObj = pGlobalStack;
        pErrorStackObj = pGlobalErrorStack;
    }
    else
    {
        pStackObj = new ScTokenStack;
        pErrorStackObj = new ScErrorStack;
    }
    pStack = pStackObj->pPointer;
    pErrorStack = pErrorStackObj->pPointer;
}


ScInterpreter::~ScInterpreter()
{
//  delete pStack;

    if ( pStackObj == pGlobalStack )
        bGlobalStackInUse = FALSE;
    else
    {
        delete pStackObj;
        delete pErrorStackObj;
    }
}


void ScInterpreter::GlobalExit()        // static
{
    DBG_ASSERT(!bGlobalStackInUse, "wer benutzt noch den TokenStack?");
    DELETEZ(pGlobalStack);
    DELETEZ(pGlobalErrorStack);
}


StackVar ScInterpreter::Interpret()
{
    short nRetTypeExpr = NUMBERFORMAT_UNDEFINED;
    ULONG nRetIndexExpr = 0;
    USHORT nErrorFunction = 0;
    USHORT nErrorFunctionCount = 0;
    USHORT nStackBase;

#if SOMA_FPSIGNAL_JUMP
    BOOL bIllegalFPOperationSignal = FALSE;
    pLocalJumpBuf = pJumpBuf;
    pJumpBuf = (jmp_buf*) new BYTE[sizeof(jmp_buf)];
#endif
    nGlobError = nGlobalError;
    nGlobalError = 0;
    nMatCount = 0;
    bMatDel = FALSE;
    ppGlobSortArray = NULL;
    pTableOp = NULL;
    nStackBase = sp = maxsp = 0;
    nRetFmtType = NUMBERFORMAT_UNDEFINED;
    nFuncFmtType = NUMBERFORMAT_UNDEFINED;
    nFuncFmtIndex = nCurFmtIndex = nRetFmtIndex = 0;
    nResult = 0;
    pResult = NULL;
    eResult = svDouble;
    glSubTotal = FALSE;
    UINT16 nOldOpCode = ocStop;

    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
#if SOMA_FPSIGNAL_JUMP
    ScSigCatchFunc pOldSigFunc = (pSignalFunc)( SC_SIGFPE, MySigFunc );
#endif  // SOMA_FPSIGNAL_JUMP
    aCode.Reset();
    while( ( pCur = aCode.Next() ) != NULL
            && (!nGlobalError || nErrorFunction <= nErrorFunctionCount) )
    {
        OpCode eOp = pCur->GetOpCode();
        cPar = pCur->GetByte();
        if ( eOp == ocPush )
        {
            Push( (ScToken&) *pCur );
            if ( sp <= MAXSTACK )
                pErrorStack[ sp - 1 ] = 0;      // RPN-Code Push ohne Fehler
        }
        else
        {
            // bisheriger Ausdruck bestimmt das aktuelle Format
            nCurFmtType = nRetTypeExpr;
            nCurFmtIndex = nRetIndexExpr;
            // default Funtionsformat, andere werden bei Bedarf gesetzt
            nFuncFmtType = NUMBERFORMAT_NUMBER;
            nFuncFmtIndex = 0;

            if ( eOp == ocIf || eOp == ocChose )
                nStackBase = sp;        // nicht die Jumps vertueddeln
            else
                nStackBase = sp - pCur->GetParamCount();
            if ( nStackBase > sp )
                nStackBase = sp;        // underflow?!?

#if SOMA_FPSIGNAL_JUMP
            if (setjmp(*pJumpBuf))
            {
                bIllegalFPOperationSignal = TRUE;
                SetError( errIllegalFPOperation );
            }
            else
#endif
            {
                switch( eOp )
                {
                    case ocSep:
                    case ocClose:           // vom Compiler gepusht
                    case ocMissing          : ScMissing();                  break;
                    case ocMacro            : ScMacro();                    break;
                    case ocDBArea           : ScDBArea();                   break;
                    case ocColRowNameAuto   : ScColRowNameAuto();           break;
// gesondert        case ocPush             : Push( (ScToken&) *pCur );     break;
                    case ocIf               : ScIfJump();                   break;
                    case ocChose            : ScChoseJump();                break;
                    case ocAdd              : ScAdd();                      break;
                    case ocSub              : ScSub();                      break;
                    case ocMul              : ScMul();                      break;
                    case ocDiv              : ScDiv();                      break;
                    case ocAmpersand        : ScAmpersand();                break;
                    case ocPow              : ScPow();                      break;
                    case ocEqual            : ScEqual();                    break;
                    case ocNotEqual         : ScNotEqual();                 break;
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
                    case ocPi               : ScPi();                       break;
//                  case ocDefPar           : ScDefPar();                   break;
                    case ocRandom           : ScRandom();                   break;
                    case ocTrue             : ScTrue();                     break;
                    case ocFalse            : ScFalse();                    break;
                    case ocGetActDate       : ScGetActDate();               break;
                    case ocGetActTime       : ScGetActTime();               break;
                    case ocNoValue          : nGlobalError = NOVALUE;
                                                PushInt(0);     break;
                    case ocDeg              : ScDeg();                      break;
                    case ocRad              : ScRad();                      break;
                    case ocSin              : ScSin();                      break;
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
                    case ocGetDay           : ScGetDay();                   break;
                    case ocGetDayOfWeek     : ScGetDayOfWeek();             break;
                    case ocWeek             : ScGetWeekOfYear();            break;
                    case ocEasterSunday     : ScEasterSunday();             break;
                    case ocGetHour          : ScGetHour();                  break;
                    case ocGetMin           : ScGetMin();                   break;
                    case ocGetSec           : ScGetSec();                   break;
                    case ocPlusMinus        : ScPlusMinus();                break;
                    case ocAbs              : ScAbs();                      break;
                    case ocInt              : ScInt();                      break;
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
                    case ocMod              : ScMod();                      break;
                    case ocPower            : ScPower();                    break;
                    case ocRound            : ScRound();                    break;
                    case ocRoundUp          : ScRoundUp();                  break;
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
                    case ocMin              : ScMin( FALSE );               break;
                    case ocMinA             : ScMin( TRUE );                break;
                    case ocMax              : ScMax( FALSE );               break;
                    case ocMaxA             : ScMax( TRUE );                break;
                    case ocSum              : ScSum();                      break;
                    case ocProduct          : ScProduct();                  break;
                    case ocNBW              : ScNBW();                      break;
                    case ocIKV              : ScIKV();                      break;
                    case ocMIRR             : ScMIRR();                     break;
                    case ocISPMT            : ScISPMT();                    break;
                    case ocAverage          : ScAverage( FALSE );           break;
                    case ocAverageA         : ScAverage( TRUE );            break;
                    case ocCount            : ScCount();                    break;
                    case ocCount2           : ScCount2();                   break;
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
                    case ocSubTotal         : ScSubTotal();                 break;
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
                    case ocVLookup          : ScVLookup();                  break;
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
                    case ocText             : ScText();                     break;
                    case ocSubstitute       : ScSubstitute();               break;
                    case ocRept             : ScRept();                     break;
                    case ocConcat           : ScConcat();                   break;
                    case ocMatValue         : ScMatValue();                 break;
                    case ocMatrixUnit       : ScEMat();                     break;
                    case ocMatDet           : ScMatDet();                   break;
                    case ocMatInv           : ScMatInv();                   break;
                    case ocMatMult          : ScMatMult();                  break;
                    case ocMatTrans         : ScMatTrans();                 break;
                    case ocMatRef           : ScMatRef();                   break;
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
                    case ocExternal         : ScExternal();                 break;
                    case ocTableOp          : ScTableOp();                  break;
//                  case ocErrCell          : ScErrCell();                  break;
                    case ocStop :                                           break;
                    case ocErrorType        : ScErrorType();                break;
                    case ocCurrent          : ScCurrent();                  break;
                    case ocStyle            : ScStyle();                    break;
                    case ocDde              : ScDde();                      break;
                    case ocBase             : ScBase();                     break;
                    case ocDecimal          : ScDecimal();                  break;
                    case ocConvert          : ScConvert();                  break;
                    case ocRoman            : ScRoman();                    break;
                    case ocArabic           : ScArabic();                   break;
                    case ocAnswer           : ScAnswer();                   break;
                    case ocTeam             : ScCalcTeam();                 break;
                    case ocTTT              : ScTTT();                      break;
                    case ocSpew             : ScSpewFunc();                 break;
                    case ocGame             : ScGame();                     break;
                    default : SetError(errUnknownOpCode); PushInt(0);       break;
                }
            }   // of else ( !setjmp )

            // aeussere Funktion bestimmt das Format eines Ausdrucks
            if ( nFuncFmtType != NUMBERFORMAT_UNDEFINED )
            {
                nRetTypeExpr = nFuncFmtType;
                // nur fuer Waehrungsformate den FormatIndex uebernehmen
                nRetIndexExpr = ( nFuncFmtType == NUMBERFORMAT_CURRENCY ?
                    nFuncFmtIndex : 0 );
            }
        }

// Funktionen, die einen Fehlercode auswerten und nGlobalError direkt auf 0 setzen
// usage: switch( OpCode ) { OCERRFUNCCASE( ++n ) }
#define CASEOCERRFUNC( statement ) \
    case ocErrorType : \
    case ocIsEmpty : \
    case ocIsErr : \
    case ocIsError : \
    case ocIsFormula : \
    case ocIsLogical : \
    case ocIsNV : \
    case ocIsNonString : \
    case ocIsRef : \
    case ocIsString : \
    case ocIsValue : \
    case ocN : \
    case ocType : \
        statement;

        switch ( eOp )
        {
            CASEOCERRFUNC( ++nErrorFunction )
        }
        if ( nGlobalError )
        {
            if ( !nErrorFunctionCount )
            {   // Anzahl der Fehlercode-Funktionen in Formel
                for ( ScToken* t = rArr.FirstRPN(); t; t = rArr.NextRPN() )
                {
                    switch ( t->GetOpCode() )
                    {
                        CASEOCERRFUNC( ++nErrorFunctionCount )
                    }
                }
            }
#if SOMA_FPSIGNAL_JUMP
            if ( bIllegalFPOperationSignal )
            {
                if ( sp )
                {   // eigentlich kann kein Ergebnis gepusht worden sein
                    ScToken* pRes = pStack[ sp - 1 ];
                    if ( pRes->GetOpCode() == ocPush
                        && pRes->GetType() == svDouble
                        && !SOMA_FINITE( pRes->nValue ) )
                    {
                        pRes->nValue = 0.0;
                    }
                    else
                        PushInt(0);
                }
                else
                    PushInt(0);
            }
#endif
            if ( nErrorFunction >= nErrorFunctionCount )
                ++nErrorFunction;   // das war's, Fehler => Abbruch
            else
            {
                if ( eOp != ocPush && sp > nStackBase + 1 )
                {   // Stack abraeumen, geht davon aus, dass jede Funktion
                    // prinzipiell ein Ergebnis pusht, im Fehlerfall kann dies
                    // ein zufaelliger Wert sein
                    const ScToken* pResult = pStack[ sp - 1 ];
                    while ( sp > nStackBase )
                        Pop();
                    PushTempToken( *pResult );
                }
#if SOMA_FPSIGNAL_JUMP
                if ( bIllegalFPOperationSignal )
                {
                    bIllegalFPOperationSignal = FALSE;
                    SOMA_FPCONTROL();       // FP-Exceptions ein-/ausschalten
                    (pSignalFunc)( SC_SIGFPE, MySigFunc );  // reestablish handler
                }
#endif
            }
        }

        nOldOpCode = eOp;
    }

    // Ende: Returnwert holen

    if( sp )
    {
        pCur = pStack[ sp-1 ];
        if( pCur->GetOpCode() == ocPush )
        {
            if ( !nGlobalError )
                nGlobalError = pErrorStack[ sp-1 ];
            switch( eResult = pCur->GetType() )
            {
                case svDouble :
                    nResult = pCur->GetDouble();
                    if ( nFuncFmtType == NUMBERFORMAT_UNDEFINED )
                    {
                        nRetTypeExpr = NUMBERFORMAT_NUMBER;
                        nRetIndexExpr = 0;
                    }
                break;
                case svString :
                    nRetTypeExpr = NUMBERFORMAT_TEXT;
                    nRetIndexExpr = 0;
                    aResult = PopString();
                break;
                case svSingleRef :
                {
                    ScAddress aAdr;
                    PopSingleRef( aAdr );
                    if( !nGlobalError )
                    {
                        ScBaseCell* pCell = pDok->GetCell( aAdr );
                        if( pCell && pCell->HasStringData() )
                        {
                            GetCellString( aResult, pCell );
                            nRetTypeExpr = NUMBERFORMAT_TEXT;
                            nRetIndexExpr = 0;
                            eResult = svString;
                        }
                        else
                        {
                            nResult = GetCellValue( aAdr, pCell );
                            nRetTypeExpr = nCurFmtType;
                            nRetIndexExpr = nCurFmtIndex;
                            eResult = svDouble;
                        }
                    }
                }
                break;
                case svDoubleRef :
                {
                    if ( bMatrixFormula )
                    {   // Matrix erzeugen fuer {=A1:A5}
                        PopDoubleRefPushMatrix();
                        //  kein break, weiter mit svMatrix
                    }
                    else
                    {
                        ScRange aRange;
                        PopDoubleRef( aRange );
                        ScAddress aAdr;
                        if ( !nGlobalError && DoubleRefToPosSingleRef( aRange, aAdr ) )
                        {
                            ScBaseCell* pCell = pDok->GetCell( aAdr );
                            if( pCell && pCell->HasStringData() )
                            {
                                GetCellString( aResult, pCell );
                                nRetTypeExpr = NUMBERFORMAT_TEXT;
                                nRetIndexExpr = 0;
                                eResult = svString;
                            }
                            else
                            {
                                nResult = GetCellValue( aAdr, pCell );
                                nRetTypeExpr = nCurFmtType;
                                nRetIndexExpr = nCurFmtIndex;
                                eResult = svDouble;
                            }
                        }
                        break;
                    }
                }
                //  kein break
                case svMatrix :
                    pResult = PopMatrix();
                    if (pResult)
                    {
                        BOOL bIsString;
                        const MatValue* pMatVal = pResult->Get(0, 0, bIsString);
                        if ( pMatVal )
                        {
                            if (bIsString)
                            {
                                aResult = pMatVal->GetString();
                                eResult = svString;
                                nRetTypeExpr = NUMBERFORMAT_TEXT;
                                nRetIndexExpr = 0;
                            }
                            else
                            {
                                nResult = pMatVal->fVal;
                                eResult = svDouble;
                                if ( nRetTypeExpr != NUMBERFORMAT_LOGICAL )
                                    nRetTypeExpr = NUMBERFORMAT_NUMBER;
                                nRetIndexExpr = 0;
                            }
                        }
                        else
                            SetError(errUnknownStackVariable);
                        DBG_ASSERT(nRetMat <= MAX_ANZ_MAT,
                                        "ScInterpreter::nRetMat falsch");
                        ResetNewMat(nRetMat);           // Matrix nicht loeschen
                    }
                    else
                        eResult = svDouble;
                    break;
                default :
                    SetError(errUnknownStackVariable);
            }
        }
        else
            SetError(errUnknownStackVariable);
    }
    else
        SetError(errNoCode);
#if !SOMA_FPSIGNAL_JUMP
    if (!SOMA_FINITE(nResult))
    {
        nResult = 0.0;
        SetError(errIllegalFPOperation);
    }
#else
    delete [] pJumpBuf;
    pJumpBuf = (jmp_buf*) pLocalJumpBuf;
    (pSignalFunc)(SC_SIGFPE, pOldSigFunc);
#endif

    if( nRetTypeExpr != NUMBERFORMAT_UNDEFINED )
    {
        nRetFmtType = nRetTypeExpr;
        nRetFmtIndex = nRetIndexExpr;
    }
    else if( nFuncFmtType != NUMBERFORMAT_UNDEFINED )
    {
        nRetFmtType = nFuncFmtType;
        nRetFmtIndex = nFuncFmtIndex;
    }
    else
        nRetFmtType = NUMBERFORMAT_NUMBER;
    // nur fuer Waehrungsformate den FormatIndex uebernehmen
    if ( nRetFmtType != NUMBERFORMAT_CURRENCY )
        nRetFmtIndex = 0;

    // grrr.. EiterZirkel!
    // Fehler nur zuruecksetzen wenn nicht errCircularReference ohne Iterationen
    if ( nGlobalError || !(rArr.GetError() == errCircularReference && !pDok->GetDocOptions().IsIter()) )
        rArr.SetError( nGlobalError );

    if (ppGlobSortArray)
#ifdef WIN
        SvMemFree(*ppGlobSortArray);
#else
        delete [] (*ppGlobSortArray);
#endif
    if (bMatDel)
    {
        for (USHORT i = 0; i < MAX_ANZ_MAT; i++)
            delete ppTempMatArray[i];
        delete [] ppTempMatArray;
    }
    // Tokens im ExprStack freigeben
    ScToken** p = pStack;
    while( maxsp-- )
        (*p++)->DecRef();
    nGlobalError = nGlobError;
    if (pTableOp)
    {
        aTableOpList.Remove(pTableOp);
        delete [] pTableOp;
    }
    return eResult;
}

