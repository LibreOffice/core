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



#ifndef SC_EEPARSER_HXX
#define SC_EEPARSER_HXX

#include <tools/string.hxx>
#include <tools/gen.hxx>
#include <vcl/graph.hxx>
#include <tools/table.hxx>
#include <svl/itemset.hxx>
#include <editeng/editdata.hxx>
#include <address.hxx>

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
DECLARE_LIST( ScHTMLImageList, ScHTMLImage* )

struct ScEEParseEntry
{
    SfxItemSet          aItemSet;
    ESelection          aSel;           // Selection in EditEngine
    String*             pValStr;        // HTML evtl. SDVAL String
    String*             pNumStr;        // HTML evtl. SDNUM String
    String*             pName;          // HTML evtl. Anchor/RangeName
    String              aAltText;       // HTML IMG ALT Text
    ScHTMLImageList*    pImageList;     // Grafiken in dieser Zelle
    SCCOL               nCol;           // relativ zum Beginn des Parse
    SCROW               nRow;
    sal_uInt16              nTab;           // HTML TableInTable
    sal_uInt16              nTwips;         // RTF ColAdjust etc.
    SCCOL               nColOverlap;    // merged cells wenn >1
    SCROW               nRowOverlap;    // merged cells wenn >1
    sal_uInt16              nOffset;        // HTML PixelOffset
    sal_uInt16              nWidth;         // HTML PixelWidth
    sal_Bool                bHasGraphic;    // HTML any image loaded
    bool                bEntirePara;    // sal_True = use entire paragraph, false = use selection

                        ScEEParseEntry( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ), pImageList( NULL ),
                            nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(sal_False), bEntirePara(true)
                            {}
                        ScEEParseEntry( const SfxItemSet& rItemSet ) :
                            aItemSet( rItemSet ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ), pImageList( NULL ),
                            nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(sal_False), bEntirePara(true)
                            {}
                        ~ScEEParseEntry()
                            {
                                if ( pValStr )
                                    delete pValStr;
                                if ( pNumStr )
                                    delete pNumStr;
                                if ( pName )
                                    delete pName;
                                if ( pImageList )
                                {
                                    for ( ScHTMLImage* pI = pImageList->First();
                                            pI; pI = pImageList->Next() )
                                    {
                                        delete pI;
                                    }
                                    delete pImageList;
                                }
                            }
};
DECLARE_LIST( ScEEParseList, ScEEParseEntry* )


class EditEngine;

class ScEEParser
{
protected:
    EditEngine*         pEdit;
    SfxItemPool*        pPool;
    SfxItemPool*        pDocPool;
    ScEEParseList*      pList;
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

    virtual sal_uLong       Read( SvStream&, const String& rBaseURL ) = 0;

    void                GetDimensions( SCCOL& nCols, SCROW& nRows ) const
                            { nCols = nColMax; nRows = nRowMax; }
    sal_uLong               Count() const   { return pList->Count(); }
    ScEEParseEntry*     First() const   { return pList->First(); }
    ScEEParseEntry*     Next() const    { return pList->Next(); }
    Table*              GetColWidths() const { return pColWidths; }
};



#endif

