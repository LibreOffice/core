/*************************************************************************
 *
 *  $RCSfile: htmlimp.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 11:37:18 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include "scitems.hxx"
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/lrspitem.hxx>
#include <svx/paperinf.hxx>
#include <svx/sizeitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/boxitem.hxx>
#include <vcl/svapp.hxx>

#include "htmlimp.hxx"
#include "htmlpars.hxx"
#include "filter.hxx"
#include "global.hxx"
#include "document.hxx"
#include "editutil.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "compiler.hxx"
#include "rangenam.hxx"
#include "attrib.hxx"
#include "ftools.hxx"


//------------------------------------------------------------------------

FltError ScImportHTML( SvStream &rStream, ScDocument *pDoc,
        ScRange& rRange, double nOutputFactor, BOOL bCalcWidthHeight )
{
    ScHTMLImport aImp( pDoc, rRange, bCalcWidthHeight );
    FltError nErr = (FltError) aImp.Read( rStream );
    ScRange aR = aImp.GetRange();
    rRange.aEnd = aR.aEnd;
    aImp.WriteToDocument( TRUE, nOutputFactor );
    return nErr;
}


ScHTMLImport::ScHTMLImport( ScDocument* pDocP, const ScRange& rRange, BOOL bCalcWidthHeight ) :
    ScEEImport( pDocP, rRange )
{
    Size aPageSize;
    OutputDevice* pDefaultDev = Application::GetDefaultDevice();
    const String& aPageStyle = pDoc->GetPageStyle( rRange.aStart.Tab() );
    ScStyleSheet* pStyleSheet = (ScStyleSheet*)pDoc->
        GetStyleSheetPool()->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );
    if ( pStyleSheet )
    {
        const SfxItemSet& rSet = pStyleSheet->GetItemSet();
        const SvxLRSpaceItem* pLRItem = (const SvxLRSpaceItem*) &rSet.Get( ATTR_LRSPACE );
        long nLeftMargin   = pLRItem->GetLeft();
        long nRightMargin  = pLRItem->GetRight();
        const SvxULSpaceItem* pULItem = (const SvxULSpaceItem*) &rSet.Get( ATTR_ULSPACE );
        long nTopMargin    = pULItem->GetUpper();
        long nBottomMargin = pULItem->GetLower();
        aPageSize = ((const SvxSizeItem&) rSet.Get(ATTR_PAGE_SIZE)).GetSize();
        if ( !aPageSize.Width() || !aPageSize.Height() )
        {
            DBG_ERRORFILE("PageSize Null ?!?!?");
            aPageSize = SvxPaperInfo::GetPaperSize( SVX_PAPER_A4 );
        }
        aPageSize.Width() -= nLeftMargin + nRightMargin;
        aPageSize.Height() -= nTopMargin + nBottomMargin;
        aPageSize = pDefaultDev->LogicToPixel( aPageSize, MapMode( MAP_TWIP ) );
    }
    else
    {
        DBG_ERRORFILE("kein StyleSheet?!?");
        aPageSize = pDefaultDev->LogicToPixel(
            SvxPaperInfo::GetPaperSize( SVX_PAPER_A4 ), MapMode( MAP_TWIP ) );
    }
    if( bCalcWidthHeight )
        pParser = new ScHTMLLayoutParser( pEngine, aPageSize, pDocP );
    else
        pParser = new ScHTMLQueryParser( pEngine, pDocP );
}


ScHTMLImport::~ScHTMLImport()
{
    // Reihenfolge wichtig, sonst knallt's irgendwann irgendwo in irgendeinem Dtor!
    // Ist gewaehrleistet, da ScEEImport Basisklasse ist
    delete (ScHTMLParser*) pParser;     // vor EditEngine!
}


void ScHTMLImport::InsertRangeName( ScDocument* pDoc, const String& rName, const ScRange& rRange )
{
    ComplRefData aRefData;
    aRefData.InitRange( rRange );
    ScTokenArray aTokArray;
    aTokArray.AddDoubleReference( aRefData );
    ScRangeData* pRangeData = new ScRangeData( pDoc, rName, aTokArray );
    if( !pDoc->GetRangeName()->Insert( pRangeData ) )
        delete pRangeData;
}

void ScHTMLImport::WriteToDocument( BOOL bSizeColsRows, double nOutputFactor )
{
    ScEEImport::WriteToDocument( bSizeColsRows, nOutputFactor );

    const ScHTMLParser* pParser = GetParser();
    const ScHTMLTable* pGlobTable = pParser->GetGlobalTable();
    if( !pGlobTable )
        return;

    // set cell borders for HTML table cells
    pGlobTable->ApplyCellBorders( pDoc, aRange.aStart );

    // correct cell borders for merged cells
    for ( ScEEParseEntry* pEntry = pParser->First(); pEntry; pEntry = pParser->Next() )
    {
        if( (pEntry->nColOverlap > 1) || (pEntry->nRowOverlap > 1) )
        {
            USHORT nTab = aRange.aStart.Tab();
            const ScMergeAttr* pItem = (ScMergeAttr*) pDoc->GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_MERGE );
            if( pItem->IsMerged() )
            {
                USHORT nColMerge = pItem->GetColMerge();
                USHORT nRowMerge = pItem->GetRowMerge();

                const SvxBoxItem* pToItem = (const SvxBoxItem*)
                    pDoc->GetAttr( pEntry->nCol, pEntry->nRow, nTab, ATTR_BORDER );
                SvxBoxItem aNewItem( *pToItem );
                if( nColMerge > 1 )
                {
                    const SvxBoxItem* pFromItem = (const SvxBoxItem*)
                        pDoc->GetAttr( pEntry->nCol + nColMerge - 1, pEntry->nRow, nTab, ATTR_BORDER );
                    aNewItem.SetLine( pFromItem->GetLine( BOX_LINE_RIGHT ), BOX_LINE_RIGHT );
                }
                if( nRowMerge > 1 )
                {
                    const SvxBoxItem* pFromItem = (const SvxBoxItem*)
                        pDoc->GetAttr( pEntry->nCol, pEntry->nRow + nRowMerge - 1, nTab, ATTR_BORDER );
                    aNewItem.SetLine( pFromItem->GetLine( BOX_LINE_BOTTOM ), BOX_LINE_BOTTOM );
                }
                pDoc->ApplyAttr( pEntry->nCol, pEntry->nRow, nTab, aNewItem );
            }
        }
    }

    // create ranges for HTML tables
     // 1 - entire document
    ScRange aNewRange( aRange.aStart );
    aNewRange.aEnd.IncCol( pGlobTable->GetDocSize( tdCol ) - 1 );
    aNewRange.aEnd.IncRow( pGlobTable->GetDocSize( tdRow ) - 1 );
    InsertRangeName( pDoc, ScfTools::GetHTMLDocName(), aNewRange );

    // 2 - all tables
    InsertRangeName( pDoc, ScfTools::GetHTMLTablesName(), ScRange( aRange.aStart ) );

    // 3 - single tables
    short nColDiff = (short)aRange.aStart.Col();
    short nRowDiff = (short)aRange.aStart.Row();
    short nTabDiff = (short)aRange.aStart.Tab();

    ScHTMLTable* pTable = NULL;
    ScHTMLTableId nTableId = SC_HTML_GLOBAL_TABLE;
    while( pTable = pGlobTable->FindNestedTable( ++nTableId ) )
    {
        pTable->GetDocRange( aNewRange );
        aNewRange.Move( nColDiff, nRowDiff, nTabDiff );
        // insert table number as name
        InsertRangeName( pDoc, ScfTools::GetNameFromHTMLIndex( nTableId ), aNewRange );
        // insert table id as name
        if( pTable->GetTableName().Len() )
        {
            String aName( ScfTools::GetNameFromHTMLName( pTable->GetTableName() ) );
            USHORT nPos;
            if( !pDoc->GetRangeName()->SearchName( aName, nPos ) )
                InsertRangeName( pDoc, aName, aNewRange );
        }
    }
}

String ScHTMLImport::GetHTMLRangeNameList( ScDocument* pDoc, const String& rOrigName )
{
    DBG_ASSERT( pDoc, "ScHTMLImport::GetHTMLRangeNameList - missing document" );

    String aNewName;
    ScRangeName* pRangeNames = pDoc->GetRangeName();
    ScRangeList aRangeList;
    xub_StrLen nTokenCnt = rOrigName.GetTokenCount( ';' );
    xub_StrLen nStringIx = 0;
    for( xub_StrLen nToken = 0; nToken < nTokenCnt; nToken++ )
    {
        String aToken( rOrigName.GetToken( 0, ';', nStringIx ) );
        if( pRangeNames && ScfTools::IsHTMLTablesName( aToken ) )
        {   // build list with all HTML tables
            ULONG nIndex = 1;
            USHORT nPos;
            BOOL bLoop = TRUE;
            while( bLoop )
            {
                aToken = ScfTools::GetNameFromHTMLIndex( nIndex++ );
                bLoop = pRangeNames->SearchName( aToken, nPos );
                if( bLoop )
                {
                    const ScRangeData* pRangeData = (*pRangeNames)[ nPos ];
                    ScRange aRange;
                    if( pRangeData && pRangeData->IsReference( aRange ) && !aRangeList.In( aRange ) )
                    {
                        ScGlobal::AddToken( aNewName, aToken, ';' );
                        aRangeList.Append( aRange );
                    }
                }
            }
        }
        else
            ScGlobal::AddToken( aNewName, aToken, ';' );
    }
    return aNewName;
}

