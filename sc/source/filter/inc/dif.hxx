/*************************************************************************
 *
 *  $RCSfile: dif.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
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


#ifndef _DIF_HXX
#define _DIF_HXX


#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif


class SvStream;
class SvNumberFormatter;
class ScDocument;
class ScPatternAttr;

extern const sal_Char*  pKeyTABLE;
extern const sal_Char*  pKeyVECTORS;
extern const sal_Char*  pKeyTUPLES;
extern const sal_Char*  pKeyDATA;
extern const sal_Char*  pKeyBOT;
extern const sal_Char*  pKeyEOD;
extern const sal_Char*  pKeyTRUE;
extern const sal_Char*  pKeyFALSE;
extern const sal_Char*  pKeyNA;


enum TOPIC
{
    T_UNKNOWN,
    T_TABLE, T_VECTORS, T_TUPLES, T_DATA, T_LABEL, T_COMMENT, T_SIZE,
    T_PERIODICITY, T_MAJORSTART, T_MINORSTART, T_TRUELENGTH, T_UINITS,
    T_DISPLAYUNITS,
    T_END
};

enum DATASET { D_BOT, D_EOD, D_NUMERIC, D_STRING, D_UNKNOWN, D_SYNT_ERROR };


class DifParser
{
public:
    ByteString          aData;
    double              fVal;
    UINT32              nVector;
    UINT32              nVal;
    UINT32              nNumFormat;
    CharSet             eCharSet;
private:
    SvNumberFormatter*  pNumFormatter;
    SvStream&           rIn;
    BOOL                bPlain;
#if __ALIGNMENT4 == 1
    static UINT32       nBOT;
    static UINT32       nEOD;
    static UINT32       n1_0;
    static UINT16       nV;
#else
    static sal_Char     cBOT_0;
    static sal_Char     cEOD_0;
    static sal_Char     c1_0_0;
    static sal_Char     cBOT_1;
    static sal_Char     cEOD_1;
    static sal_Char     c1_0_1;
    static sal_Char     cBOT_2;
    static sal_Char     cEOD_2;
    static sal_Char     c1_0_2;
    static sal_Char     cBOT_3;
    static sal_Char     cEOD_3;
    static sal_Char     c1_0_3;
    static sal_Char     cV_0;
    static sal_Char     cV_1;
#endif

    static inline BOOL  IsBOT( const sal_Char* pRef );
    static inline BOOL  IsEOD( const sal_Char* pRef );
    static inline BOOL  Is1_0( const sal_Char* pRef );
public:
                        DifParser( SvStream&, const UINT32 nOption, ScDocument&, CharSet );

    TOPIC               GetNextTopic( void );

    DATASET             GetNextDataset( void );

    const sal_Char*     ScanIntVal( const sal_Char* pStart, UINT32& rRet );
    BOOL                ScanFloatVal( const sal_Char* pStart );

    inline BOOL         IsNumber( const sal_Char cChar );
    inline BOOL         IsNumberEnding( const sal_Char cChar );

    static inline BOOL  IsV( const sal_Char* pRef );

    inline BOOL         IsPlain( void ) const;
};


inline BOOL DifParser::IsBOT( const sal_Char* pRef )
{
#if __ALIGNMENT4 == 1
    return ( nBOT == *( ( UINT32* ) pRef ) );
#else
    return  (   pRef[ 0 ] == cBOT_0 &&
                pRef[ 1 ] == cBOT_1 &&
                pRef[ 2 ] == cBOT_2 &&
                pRef[ 3 ] == cBOT_3 );
#endif
}


inline BOOL DifParser::IsEOD( const sal_Char* pRef )
{
#if __ALIGNMENT4 == 1
    return ( nEOD == *( ( UINT32* ) pRef ) );
#else
    return  (   pRef[ 0 ] == cEOD_0 &&
                pRef[ 1 ] == cEOD_1 &&
                pRef[ 2 ] == cEOD_2 &&
                pRef[ 3 ] == cEOD_3 );
#endif
}


inline BOOL DifParser::Is1_0( const sal_Char* pRef )
{
#if __ALIGNMENT4 == 1
    return ( n1_0 == *( ( UINT32* ) pRef ) );
#else
    return  (   pRef[ 0 ] == c1_0_0 &&
                pRef[ 1 ] == c1_0_1 &&
                pRef[ 2 ] == c1_0_2 &&
                pRef[ 3 ] == c1_0_3 );
#endif
}


inline BOOL DifParser::IsV( const sal_Char* pRef )
{
#if __ALIGNMENT4 == 1
    return ( nV == *( ( UINT16 * ) pRef ) );
#else
    return  (   pRef[ 0 ] == cV_0 &&
                pRef[ 1 ] == cV_1   );
#endif
}


inline BOOL DifParser::IsNumber( const sal_Char cChar )
{
    return ( cChar >= '0' && cChar <= '9' );
}


inline BOOL DifParser::IsNumberEnding( const sal_Char cChar )
{
    return ( cChar == 0x00 );
}


inline BOOL DifParser::IsPlain( void ) const
{
    return bPlain;
}




class DifAttrCache;
class ScPatternAttr;


class DifColumn : private List
{
private:
    friend DifAttrCache;
    struct ENTRY
    {
        UINT32          nNumFormat;

        UINT16          nStart;
        UINT16          nEnd;
    };

    ENTRY*              pAkt;

    inline              DifColumn( void );
                        ~DifColumn();
    void                SetLogical( UINT16 nRow );
    void                SetNumFormat( UINT16 nRow, const UINT32 nNumFormat );
    void                NewEntry( const UINT16 nPos, const UINT32 nNumFormat );
    void                Apply( ScDocument&, const UINT16 nCol, const UINT16 nTab, const ScPatternAttr& );
    void                Apply( ScDocument &rDoc, const UINT16 nCol, const UINT16 nTab );
public:     // geht niemanden etwas an...
};


inline DifColumn::DifColumn( void )
{
    pAkt = NULL;
}




class DifAttrCache
{
private:
    DifColumn**         ppCols;
    BOOL                bPlain;
public:
                        DifAttrCache( const BOOL bPlain );
                        ~DifAttrCache();
    inline void         SetLogical( const UINT16 nCol, const UINT16 nRow );
    void                SetNumFormat( const UINT16 nCol, const UINT16 nRow, const UINT32 nNumFormat );
    void                Apply( ScDocument&, UINT16 nTab );
};


inline void DifAttrCache::SetLogical( const UINT16 nCol, const UINT16 nRow )
{
    DBG_ASSERT( nCol <= MAXCOL, "-DifAttrCache::SetLogical(): Col zu gross!" );
    DBG_ASSERT( bPlain, "*DifAttrCache::SetLogical(): muss Plain sein!" );

    if( !ppCols[ nCol ] )
        ppCols[ nCol ] = new DifColumn;
    ppCols[ nCol ]->SetLogical( nRow );
}


#endif


