/*************************************************************************
 *
 *  $RCSfile: fltglbls.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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
#ifndef _FLTGLBS_HXX
#define _FLTGLBS_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _LANG_HXX //autogen
#include <tools/lang.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif


class SfxPoolItem;
class SwDoc;
class SwPaM;
class SwTable;

class XF_Buffer;
class ColorBuffer;
class FontBuffer;
class SvNumberFormatter;


class ExcGlob;
extern ExcGlob *pExcGlob;

class LotGlob;
extern LotGlob *pLotGlob;

// ----- Basis-Klasse ----------------------------------------------------
class FilterGlobals
{
protected:
    SvPtrarr        aTblFmts;
    USHORT          nColStart;
    USHORT          nColEnd;
    USHORT          nRowStart;
    USHORT          nRowEnd;
    USHORT          nAnzCols;
    USHORT          nAnzRows;

public:
    FilterGlobals( SwDoc& rDoc, const SwPaM& rPam );
    ~FilterGlobals();

    SwDoc           *pD;
    SwPaM           *pPam;
    const SwTable   *pTable;

    SvNumberFormatter *pNumFormatter;
    LanguageType    eDefLanguage;
    String          aStandard;      // fuer Excel-Standard-Format
    ULONG           nStandard;
    ULONG           nDefFormat;     // = 0xFFFFFFFF

    void SetRange( USHORT nCS, USHORT nCE, USHORT nRS, USHORT nRE );

    BOOL IsInColRange( USHORT nCol )
                {   return ( nCol >= nColStart && nCol <= nColEnd ); }
    BOOL IsInRowRange( USHORT nRow )
                {   return ( nRow >= nRowStart && nRow <= nRowEnd ); }
    BOOL IsInRange( USHORT nCol, USHORT nRow )
                {   return IsInRowRange(nRow) && IsInColRange(nCol); }

    void NormalizeCol( USHORT &rCol )   { rCol -= nColStart; }
    void NormalizeRow( USHORT &rRow )   { rRow -= nRowStart; }
    void Normalize( USHORT &rCol, USHORT &rRow )
                        {   NormalizeCol( rCol ); NormalizeRow( rRow ); }

    USHORT AnzCols() const  { return nAnzCols; }
    USHORT AnzRows() const  { return nAnzRows; }

    BOOL ColRangeLimitter( USHORT &rCS, USHORT &rCE );

    void InsertText( USHORT nCol, USHORT nRow, const String& rStr );
    void CreateTable();
    void InsertAttr( const SfxPoolItem& rItem );

    inline void     ColLimitter( USHORT &rCol );
    inline void     RowLimitter( USHORT &rRow );
#ifdef USED
//      inline BOOL     RowRangeLimitter( USHORT &rRS, USHORT &rRE );
#endif
};





// ----- for Excel-Import only -------------------------------------------

enum ExcelRecordTypes { ERT_Biff2, ERT_Biff3, ERT_Biff4, ERT_Biff5 };

class ExcGlob : public FilterGlobals
{
public:
    ExcGlob( SwDoc& rDoc, const SwPaM& rPam );
    ~ExcGlob();

    XF_Buffer           *pXF_Buff;
    FontBuffer          *pFontBuff;
    ColorBuffer         *pColorBuff;
    ExcelRecordTypes    eHauptDateiTyp;
};

// ----- for Lotus-Import only -------------------------------------------
class LotGlob : public FilterGlobals
{
public:
    LotGlob( SwDoc& rDoc, const SwPaM& rPam )
        : FilterGlobals( rDoc, rPam ) {}
};




inline void FilterGlobals::ColLimitter( USHORT &rCol )
{
    if( rCol < nColStart )
        rCol = nColStart;
    else if( rCol > nColEnd )
        rCol = nColEnd;
}

inline void FilterGlobals::RowLimitter( USHORT &rRow )
{
    if( rRow < nRowStart )
        rRow = nRowStart;
    else if( rRow > nRowEnd )
        rRow = nRowEnd;
}

#ifdef USED

inline BOOL FilterGlobals::RowRangeLimitter( USHORT &rRS, USHORT &rRE )
    {
    //  PREC / POST: analog zu ColRangeLimitter
    BOOL bRet;
    DBG_ASSERT( rRS <= rRE,
        "-FilterGlobals::RowRangeLimitter(): Startzeile > Endzeile!" );
    if( rRS > nRowEnd || rRE < nRowStart )
        bRet = FALSE;
    else
        {
        bRet = TRUE;
        if( rRS < nRowStart )   rRS = nRowStart;
        if( rRE > nRowEnd )     rRE = nRowEnd;
        }
    return bRet;
    }

#endif


#endif

