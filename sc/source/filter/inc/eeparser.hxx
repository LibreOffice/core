/*************************************************************************
 *
 *  $RCSfile: eeparser.hxx,v $
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
                            pGraphic( NULL ),
                            aSize( 0, 0 ), aSpace( 0, 0 ), nDir( nHorizontal )
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
    USHORT              nCol;           // relativ zum Beginn des Parse
    USHORT              nRow;
    USHORT              nTab;           // HTML TableInTable
    USHORT              nTwips;         // RTF ColAdjust etc.
    USHORT              nColOverlap;    // merged cells wenn >1
    USHORT              nRowOverlap;    // merged cells wenn >1
    USHORT              nOffset;        // HTML PixelOffset
    USHORT              nWidth;         // HTML PixelWidth
    BOOL                bHasGraphic;    // HTML any image loaded

                        ScEEParseEntry( SfxItemPool* pPool ) :
                            aItemSet( *pPool ), pValStr( NULL ),
                            pNumStr( NULL ), pName( NULL ), pImageList( NULL ),
                            nCol((USHORT)~0), nRow((USHORT)~0), nTab(0),
                            nColOverlap(1), nRowOverlap(1),
                            nOffset(0), nWidth(0), bHasGraphic(FALSE)
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
    USHORT              nColCnt;
    USHORT              nRowCnt;
    USHORT              nColMax;
    USHORT              nRowMax;

    void                NewActEntry( ScEEParseEntry* );

public:
                        ScEEParser( EditEngine* );
    virtual             ~ScEEParser();

    virtual ULONG       Read( SvStream& ) = 0;

    void                GetDimensions( USHORT& nCols, USHORT& nRows ) const
                            { nCols = nColMax; nRows = nRowMax; }
    ULONG               Count() const   { return pList->Count(); }
    ScEEParseEntry*     First() const   { return pList->First(); }
    ScEEParseEntry*     Next() const    { return pList->Next(); }
    Table*              GetColWidths() const { return pColWidths; }
};



#endif

