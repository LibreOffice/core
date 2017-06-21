/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_LOTATTR_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_LOTATTR_HXX

#include <vector>
#include <memory>

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

    bool HasStyles () const
    {
        return ( nFont || nLineStyle || nFontCol || ( nBack & 0x7F ) );
                    // !! without center bit!!
    }

    bool IsCentered () const
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
        std::unique_ptr<ScPatternAttr>  pPattAttr;
        sal_uInt32                      nHash0;

        ENTRY(ScPatternAttr* p);

        ~ENTRY();
    };

    static void  MakeHash( const LotAttrWK3& rAttr, sal_uInt32& rOut )
    {
        reinterpret_cast<sal_uInt8*>(&rOut)[ 0 ] = rAttr.nFont & 0x7F;
        reinterpret_cast<sal_uInt8*>(&rOut)[ 1 ] = rAttr.nLineStyle;
        reinterpret_cast<sal_uInt8*>(&rOut)[ 2 ] = rAttr.nFontCol;
        reinterpret_cast<sal_uInt8*>(&rOut)[ 3 ] = rAttr.nBack;
    }

    static void LotusToScBorderLine( sal_uInt8 nLine, ::editeng::SvxBorderLine& );

    const SvxColorItem& GetColorItem( const sal_uInt8 nLotIndex ) const;

    const Color& GetColor( const sal_uInt8 nLotIndex ) const;

    ScDocumentPool*     pDocPool;
    SvxColorItem*       ppColorItems[6];        // 0 and 7 are missing!
    SvxColorItem*       pWhite;
    Color*              pColTab;
    std::vector< std::unique_ptr<ENTRY> > aEntries;

    LOTUS_ROOT* mpLotusRoot;
};

class LotAttrCol
{
public:
    void SetAttr (const SCROW nRow, const ScPatternAttr&);

    void Apply(LOTUS_ROOT* pLotRoot, const SCCOL nCol, const SCTAB nTab);
private:

    struct ENTRY
    {
        const ScPatternAttr* pPattAttr;
        SCROW nFirstRow;
        SCROW nLastRow;
    };

    std::vector< std::unique_ptr<ENTRY> > aEntries;
};

class LotAttrTable
{
public:
    LotAttrTable(LOTUS_ROOT* pLotRoot);

    void SetAttr( const SCCOL nColFirst, const SCCOL nColLast, const SCROW nRow, const LotAttrWK3& );

    void Apply(LOTUS_ROOT* pLotRoot, const SCTAB nTabNum);

private:

    LotAttrCol pCols[ MAXCOLCOUNT ];
    LotAttrCache aAttrCache;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
