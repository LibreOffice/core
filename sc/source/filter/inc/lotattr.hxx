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

#ifndef SC_LOTATTR_HXX
#define SC_LOTATTR_HXX

#include <tools/solar.h>
#include "patattr.hxx"
#include "scitems.hxx"
#include "address.hxx"

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
    friend class LotAttrTable;

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
        SCROW                   nFirstRow;
        SCROW                   nLastRow;
    };

public:
                                ~LotAttrCol( void );
    void                        SetAttr( const SCROW nRow, const ScPatternAttr& );
    void                        Apply( const SCCOL nCol, const SCTAB nTab, const BOOL bClear = TRUE );
    void                        Clear( void );
};


class LotAttrTable
{
private:
    LotAttrCol          pCols[ MAXCOLCOUNT ];
    LotAttrCache        aAttrCache;
public:
                        LotAttrTable( void );
                        ~LotAttrTable();

    void                SetAttr( const SCCOL nColFirst, const SCCOL nColLast, const SCROW nRow, const LotAttrWK3& );
    void                Apply( const SCTAB nTabNum );
};





#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
