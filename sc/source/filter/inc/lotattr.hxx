/*************************************************************************
 *
 *  $RCSfile: lotattr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:13 $
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

#ifndef _LOTATTR_HXX
#define _LOTATTR_HXX

#include <tools/solar.h>

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#include <patattr.hxx>
#include <scitems.hxx>

// ----- forwards --------------------------------------------------------
class ScDocument;
class ScDocumentPool;
class SvxBorderLine;
class SvxColorItem;
class Color;

class LotAttrTable;


struct LotAttrWK3
{
    UINT8                   nFont;
    UINT8                   nLineStyle;
    UINT8                   nFontCol;
    UINT8                   nBack;

    inline BOOL             HasStyles( void );
    inline BOOL             IsCentered( void );
};


inline BOOL LotAttrWK3::HasStyles( void )
{
    return ( nFont || nLineStyle || nFontCol || ( nBack & 0x7F ) );
                    // !! ohne Center-Bit!!
}


inline BOOL LotAttrWK3::IsCentered( void )
{
    return ( nBack & 0x80 );
}


class LotAttrCache : private List
{
private:
    friend LotAttrTable;

    struct ENTRY
    {
        ScPatternAttr*  pPattAttr;
        UINT32          nHash0;

        inline          ENTRY( const ScPatternAttr& r )         { pPattAttr = new ScPatternAttr( r ); }

        inline          ENTRY( ScPatternAttr* p )               { pPattAttr = p; }

        inline          ~ENTRY()                                { delete pPattAttr; }

        inline BOOL     operator ==( const ENTRY& r ) const     { return nHash0 == r.nHash0; }

        inline BOOL     operator ==( const UINT32& r ) const    { return nHash0 == r; }
    };

    ScDocumentPool*     pDocPool;
    SvxColorItem*       ppColorItems[ 6 ];      // 0 und 7 fehlen!
    SvxColorItem*       pBlack;
    SvxColorItem*       pWhite;
    Color*              pColTab;

    inline static void  MakeHash( const LotAttrWK3& rAttr, UINT32& rOut )
                        {
                            ( ( UINT8* ) &rOut )[ 0 ] = rAttr.nFont & 0x7F;
                            ( ( UINT8* ) &rOut )[ 1 ] = rAttr.nLineStyle;
                            ( ( UINT8* ) &rOut )[ 2 ] = rAttr.nFontCol;
                            ( ( UINT8* ) &rOut )[ 3 ] = rAttr.nBack;
                        }
    static void         LotusToScBorderLine( UINT8 nLine, SvxBorderLine& );
    const SvxColorItem& GetColorItem( const UINT8 nLotIndex ) const;
    const Color&        GetColor( const UINT8 nLotIndex ) const;
public:
                        LotAttrCache( void );
                        ~LotAttrCache();

    const ScPatternAttr&    GetPattAttr( const LotAttrWK3& );
};


class LotAttrCol : private List
{
private:
    struct ENTRY
    {
        const ScPatternAttr*    pPattAttr;
        UINT16                  nFirstRow;
        UINT16                  nLastRow;
    };

public:
                                ~LotAttrCol( void );
    void                        SetAttr( const UINT16 nRow, const ScPatternAttr& );
    void                        Apply( const UINT16 nCol, const UINT16 nTab, const BOOL bClear = TRUE );
    void                        Clear( void );
};


class LotAttrTable
{
private:
    LotAttrCol          pCols[ MAXCOL + 1 ];
    LotAttrCache        aAttrCache;
public:
                        LotAttrTable( void );
                        ~LotAttrTable();

    void                SetAttr( const UINT8 nColFirst, const UINT8 nColLast, const UINT16 nRow, const LotAttrWK3& );
    void                Apply( const UINT16 nTabNum );
};





#endif

