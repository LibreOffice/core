/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_LOTATTR_HXX
#define SC_LOTATTR_HXX

#include <tools/solar.h>
#include <tools/list.hxx>
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
    sal_uInt8                   nFont;
    sal_uInt8                   nLineStyle;
    sal_uInt8                   nFontCol;
    sal_uInt8                   nBack;

    inline sal_Bool             HasStyles( void );
    inline sal_Bool             IsCentered( void );
};


inline sal_Bool LotAttrWK3::HasStyles( void )
{
    return ( nFont || nLineStyle || nFontCol || ( nBack & 0x7F ) );
                    // !! ohne Center-Bit!!
}


inline sal_Bool LotAttrWK3::IsCentered( void )
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
        sal_uInt32          nHash0;

        inline          ENTRY( const ScPatternAttr& r )         { pPattAttr = new ScPatternAttr( r ); }

        inline          ENTRY( ScPatternAttr* p )               { pPattAttr = p; }

        inline          ~ENTRY()                                { delete pPattAttr; }

        inline sal_Bool     operator ==( const ENTRY& r ) const     { return nHash0 == r.nHash0; }

        inline sal_Bool     operator ==( const sal_uInt32& r ) const    { return nHash0 == r; }
    };

    ScDocumentPool*     pDocPool;
    SvxColorItem*       ppColorItems[ 6 ];      // 0 und 7 fehlen!
    SvxColorItem*       pBlack;
    SvxColorItem*       pWhite;
    Color*              pColTab;

    inline static void  MakeHash( const LotAttrWK3& rAttr, sal_uInt32& rOut )
                        {
                            ( ( sal_uInt8* ) &rOut )[ 0 ] = rAttr.nFont & 0x7F;
                            ( ( sal_uInt8* ) &rOut )[ 1 ] = rAttr.nLineStyle;
                            ( ( sal_uInt8* ) &rOut )[ 2 ] = rAttr.nFontCol;
                            ( ( sal_uInt8* ) &rOut )[ 3 ] = rAttr.nBack;
                        }
    static void         LotusToScBorderLine( sal_uInt8 nLine, SvxBorderLine& );
    const SvxColorItem& GetColorItem( const sal_uInt8 nLotIndex ) const;
    const Color&        GetColor( const sal_uInt8 nLotIndex ) const;
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
    void                        Apply( const SCCOL nCol, const SCTAB nTab, const sal_Bool bClear = sal_True );
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

