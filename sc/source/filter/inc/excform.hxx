/*************************************************************************
 *
 *  $RCSfile: excform.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-28 10:59:50 $
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


#ifndef _EXCFORM_HXX
#define _EXCFORM_HXX

#ifndef _FORMEL_HXX
#include "formel.hxx"
#endif


class ScRangeList;
class UINT16List;


#ifdef MWERKS
class ExcelToSc : public ConverterBase, public ExcRoot
#else
class ExcelToSc : public ConverterBase, protected ExcRoot
#endif
{
protected:
    BOOL                bExternName;    // wenn External Name gefunden wurde
    static const UINT16 nRowMask;
    static const UINT16 nLastInd;       // letzter Index fuer Excel->SC-
                                        // Token Umsetzung
    // ---------------------------------------------------------------
    inline void         DoDefArgs( UINT16 );

    void                DoMulArgs( DefTokenId, BYTE );

    static DefTokenId   IndexToToken( UINT16 );

    static BYTE         IndexToAnzahl( UINT16 );

    void                ExcRelToScRel( UINT16 nRow, UINT8 nCol, SingleRefData&, const BOOL bName );

public:
                        ExcelToSc( RootData* pRD, SvStream& aStr, const UINT16& rOrgTab );
    virtual             ~ExcelToSc();
    virtual ConvErr     Convert( const ScTokenArray*&, INT32& nRest, const FORMULA_TYPE eFT = FT_CellFormula );

    virtual ConvErr     Convert( _ScRangeListTabs&, INT32& nRest, const FORMULA_TYPE eFT = FT_CellFormula );

    void                GetDummy( const ScTokenArray*& );
    const ScTokenArray* GetBoolErr( BoolError );
    BOOL                GetShrFmla( const ScTokenArray*&, INT32& rRestbytes );

    static BOOL         SetCurVal( ScFormulaCell& rCell, double& rCurVal );
                            // return = TRUE -> String-Record folgt!
    static void         SetError( ScFormulaCell& rCell, const ConvErr eErr );

    static inline BOOL  IsComplColRange( const UINT16 nCol1, const UINT16 nCol2 );
    static inline BOOL  IsComplRowRange( const UINT16 nRow1, const UINT16 nRow2 );

    void                SetComplCol( ComplRefData& );
    void                SetComplRow( ComplRefData& );
};


inline void ExcelToSc::DoDefArgs( UINT16 nIndex )
{
    DoMulArgs( IndexToToken( nIndex ), IndexToAnzahl( nIndex ) );
}


inline BOOL ExcelToSc::IsComplColRange( const UINT16 nCol1, const UINT16 nCol2 )
{
    return ( nCol1 == 0x00 ) && ( nCol2 == 0xFF );
}


inline BOOL ExcelToSc::IsComplRowRange( const UINT16 nRow1, const UINT16 nRow2 )
{
    return ( ( nRow1 & 0x3FFF ) == 0x0000 ) && ( ( nRow2 & 0x3FFF ) == 0x3FFF );
}




class ExcelToSc8 : public ExcelToSc
{
    void                ExcRelToScRel( UINT16 nRow, UINT16 nCol, SingleRefData&,
                            const BOOL bName );

    virtual ConvErr     Convert( const ScTokenArray*& rpTokArray, INT32& rLeft, const FORMULA_TYPE eFT, UINT16List* pChTrackList );

public:
                        ExcelToSc8( RootData* pRD, SvStream& aStr, const UINT16& rOrgTab );
    virtual             ~ExcelToSc8();

    virtual ConvErr     Convert( const ScTokenArray*& rpTokArray, INT32& rLeft, const FORMULA_TYPE eFT = FT_CellFormula );
    virtual ConvErr     Convert( const ScTokenArray*& rpTokArray, INT32& rLeft, UINT16List& rChTrackList );

    virtual ConvErr     Convert( _ScRangeListTabs&, INT32& nRest, const FORMULA_TYPE eFT = FT_CellFormula );

    static inline BOOL  IsComplRowRange( const UINT16 nRow1, const UINT16 nRow2 );

    BOOL                GetAbsRefs( SvStream& rStream, INT32& rRest, ScRangeList& rRangeList );
};


inline BOOL ExcelToSc8::IsComplRowRange( const UINT16 nRow1, const UINT16 nRow2 )
{
    return ( nRow1 == 0x0000 ) && ( nRow2 == 0xFFFF );
}





#endif
