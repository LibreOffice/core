/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fltglbls.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-08 13:33:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _FLTGLBS_HXX
#define _FLTGLBS_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
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

#endif
