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

#include <boost/ptr_container/ptr_vector.hpp>

#include <tools/solar.h>

#include "address.hxx"
#include "scitems.hxx"

class ScDocumentPool;
class ScPatternAttr;
class SvxColorItem;
class Color;
class LotAttrTable;
struct LOTUS_ROOT;

namespace editeng { class SvxBorderLine; }

struct LotAttrWK3
{
    sal_uInt8 nFont;
    sal_uInt8 nLineStyle;
    sal_uInt8 nFontCol;
    sal_uInt8 nBack;

    inline bool HasStyles () const
    {
        return ( nFont || nLineStyle || nFontCol || ( nBack & 0x7F ) );
                    // !! ohne Center-Bit!!
    }

    inline bool IsCentered () const
    {
        return ( nBack & 0x80 );
    }
};

class LotAttrCache
{
public:

    LotAttrCache(LOTUS_ROOT* pLotRoot);

    ~LotAttrCache();

    const ScPatternAttr& GetPattAttr( const LotAttrWK3& );

private:

    friend class LotAttrTable;

    struct ENTRY
    {
        ScPatternAttr*  pPattAttr;
        sal_uInt32          nHash0;

        ENTRY (ScPatternAttr* p);

        ~ENTRY ();

        inline bool operator == (const ENTRY &r) const { return nHash0 == r.nHash0; }

        inline bool operator == (const sal_uInt32 &r) const { return nHash0 == r; }
    };

    inline static void  MakeHash( const LotAttrWK3& rAttr, sal_uInt32& rOut )
    {
        ( ( sal_uInt8* ) &rOut )[ 0 ] = rAttr.nFont & 0x7F;
        ( ( sal_uInt8* ) &rOut )[ 1 ] = rAttr.nLineStyle;
        ( ( sal_uInt8* ) &rOut )[ 2 ] = rAttr.nFontCol;
        ( ( sal_uInt8* ) &rOut )[ 3 ] = rAttr.nBack;
    }

    static void LotusToScBorderLine( sal_uInt8 nLine, ::editeng::SvxBorderLine& );

    const SvxColorItem& GetColorItem( const sal_uInt8 nLotIndex ) const;

    const Color& GetColor( const sal_uInt8 nLotIndex ) const;

    ScDocumentPool*     pDocPool;
    SvxColorItem*       ppColorItems[6];        // 0 und 7 fehlen!
    SvxColorItem*       pBlack;
    SvxColorItem*       pWhite;
    Color*              pColTab;
    boost::ptr_vector<ENTRY> aEntries;

    LOTUS_ROOT* mpLotusRoot;
};


class LotAttrCol
{
public:
    void SetAttr (const SCROW nRow, const ScPatternAttr&);

    void Apply (const SCCOL nCol, const SCTAB nTab );
private:

    struct ENTRY
    {
        const ScPatternAttr* pPattAttr;
        SCROW nFirstRow;
        SCROW nLastRow;
    };

    boost::ptr_vector<ENTRY> aEntries;
};


class LotAttrTable
{
public:
    LotAttrTable(LOTUS_ROOT* pLotRoot);

    void SetAttr( const SCCOL nColFirst, const SCCOL nColLast, const SCROW nRow, const LotAttrWK3& );

    void Apply( const SCTAB nTabNum );

private:

    LotAttrCol pCols[ MAXCOLCOUNT ];
    LotAttrCache aAttrCache;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
