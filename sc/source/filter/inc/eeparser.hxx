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

#ifndef SC_EEPARSER_HXX
#define SC_EEPARSER_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <vcl/graph.hxx>
#include <tools/table.hxx>
#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>
#include <address.hxx>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>

const sal_Char nHorizontal = 1;
const sal_Char nVertical = 2;
const sal_Char nHoriVerti = nHorizontal | nVertical;

struct ScHTMLImage
{
    String              aURL;
    Size                aSize;
    Point               aSpace;
    String              aFilterName;
    Graphic*            pGraphic;       // wird von WriteToDocument uebernommen
    sal_Char            nDir;           // 1==hori, 2==verti, 3==beides

                        ScHTMLImage() :
                            aSize( 0, 0 ), aSpace( 0, 0 ), pGraphic( NULL ),
                            nDir( nHorizontal )
                            {}
                        ~ScHTMLImage()
                            { if ( pGraphic ) delete pGraphic; }
};

struct ScEEParseEntry
{
    SfxItemSet          aItemSet;
    ESelection          aSel;           // Selection in EditEngine
    String*             pValStr;        // HTML evtl. SDVAL String
    String*             pNumStr;        // HTML evtl. SDNUM String
    String*             pName;          // HTML evtl. Anchor/RangeName
    String              aAltText;       // HTML IMG ALT Text
    boost::ptr_vector< ScHTMLImage > maImageList;       // Grafiken in dieser Zelle
    SCCOL               nCol;           // relativ zum Beginn des Parse
    SCROW               nRow;
    sal_uInt16          nTab;           // HTML TableInTable
    sal_uInt16          nTwips;         // RTF ColAdjust etc.
    SCCOL               nColOverlap;    // merged cells wenn >1
    SCROW               nRowOverlap;    // merged cells wenn >1
    sal_uInt16          nOffset;        // HTML PixelOffset
    sal_uInt16          nWidth;         // HTML PixelWidth
    bool                bHasGraphic;    // HTML any image loaded
    bool                bEntirePara;    // TRUE = use entire paragraph, false = use selection

                        ScEEParseEntry( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ),
                            nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(false), bEntirePara(true)
                            {}
                        ScEEParseEntry( const SfxItemSet& rItemSet ) :
                            aItemSet( rItemSet ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ),
                            nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(false), bEntirePara(true)
                            {}
                        ~ScEEParseEntry()
                            {
                                if ( pValStr )
                                    delete pValStr;
                                if ( pNumStr )
                                    delete pNumStr;
                                if ( pName )
                                    delete pName;
                                if ( maImageList.size() )
                                    maImageList.clear();
                            }
};


class EditEngine;

class ScEEParser
{
protected:
    EditEngine*         pEdit;
    SfxItemPool*        pPool;
    SfxItemPool*        pDocPool;
    ::std::vector< ScEEParseEntry* > maList;
    ScEEParseEntry*     pActEntry;
    Table*              pColWidths;
    int                 nLastToken;
    SCCOL               nColCnt;
    SCROW               nRowCnt;
    SCCOL               nColMax;
    SCROW               nRowMax;

    void                NewActEntry( ScEEParseEntry* );

public:
                        ScEEParser( EditEngine* );
    virtual             ~ScEEParser();

    virtual sal_uLong           Read( SvStream&, const String& rBaseURL ) = 0;

    Table*                  GetColWidths() const { return pColWidths; }
    void                    GetDimensions( SCCOL& nCols, SCROW& nRows ) const
                                { nCols = nColMax; nRows = nRowMax; }

    inline size_t           ListSize() const{ return maList.size(); }
    ScEEParseEntry*         ListEntry( size_t index ) { return maList[ index ]; }
    const ScEEParseEntry*   ListEntry( size_t index ) const { return maList[ index ]; }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
