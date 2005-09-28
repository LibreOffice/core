/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eeparser.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:55:45 $
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
#ifndef SC_EEPARSER_HXX
#define SC_EEPARSER_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _MyEDITDATA_HXX //autogen
#include <svx/editdata.hxx>
#endif
#ifndef SC_ADDRESS_HXX
#include <address.hxx>
#endif

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
DECLARE_LIST( ScHTMLImageList, ScHTMLImage* );

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
    USHORT              nTab;           // HTML TableInTable
    USHORT              nTwips;         // RTF ColAdjust etc.
    SCCOL               nColOverlap;    // merged cells wenn >1
    SCROW               nRowOverlap;    // merged cells wenn >1
    USHORT              nOffset;        // HTML PixelOffset
    USHORT              nWidth;         // HTML PixelWidth
    BOOL                bHasGraphic;    // HTML any image loaded
    bool                bEntirePara;    // TRUE = use entire paragraph, false = use selection

                        ScEEParseEntry( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ), pImageList( NULL ),
                            nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(FALSE), bEntirePara(true)
                            {}
                        ScEEParseEntry( const SfxItemSet& rItemSet ) :
                            aItemSet( rItemSet ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ), pImageList( NULL ),
                            nCol(SCCOL_MAX), nRow(SCROW_MAX), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(FALSE), bEntirePara(true)
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
DECLARE_LIST( ScEEParseList, ScEEParseEntry* );


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

    virtual ULONG       Read( SvStream&, const String& rBaseURL ) = 0;

    void                GetDimensions( SCCOL& nCols, SCROW& nRows ) const
                            { nCols = nColMax; nRows = nRowMax; }
    ULONG               Count() const   { return pList->Count(); }
    ScEEParseEntry*     First() const   { return pList->First(); }
    ScEEParseEntry*     Next() const    { return pList->Next(); }
    Table*              GetColWidths() const { return pColWidths; }
};



#endif

