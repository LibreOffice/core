/*************************************************************************
 *
 *  $RCSfile: table5.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:29:06 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include "collect.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "column.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "userlist.hxx"
#include "stlsheet.hxx"
#include "global.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "brdcst.hxx"
#include "globstr.hrc"

// STATIC DATA -----------------------------------------------------------

#define GET_SCALEVALUE(set,id)  ((const SfxUInt16Item&)(set.Get( id ))).GetValue()


void ScTable::UpdatePageBreaks( const ScRange* pUserArea )
{
    if ( pDocument->IsImportingXML() )
        return;
    if ( !pUserArea && !bPageSizeValid )
        return;

    SfxStyleSheetBase* pStyle = pDocument->GetStyleSheetPool()->
                                    Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );
    if ( !pStyle )
    {
        DBG_ERROR("UpdatePageBreaks: Style nicht gefunden");
        return;
    }
    SfxItemSet* pStyleSet = &pStyle->GetItemSet();
    const SfxPoolItem* pItem;

    SCCOL nX;
    SCROW nY;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = MAXCOL;
    SCROW nEndRow = MAXROW;
    if (pUserArea)
    {
        nStartCol = pUserArea->aStart.Col();
        nStartRow = pUserArea->aStart.Row();
        nEndCol = pUserArea->aEnd.Col();
        nEndRow = pUserArea->aEnd.Row();
    }
    else
    {
        USHORT nAreaCount = GetPrintRangeCount();
        if ( nAreaCount > 1 )
        {
            //  bei mehreren Bereichen nichts anzeigen:

            for (nX=0; nX<MAXCOL; nX++)
                pColFlags[nX] &= ~CR_PAGEBREAK;
            for (nY=0; nY<MAXROW; nY++)
                pRowFlags[nY] &= ~CR_PAGEBREAK;

            return;
        }
        else if ( nAreaCount == 1 )
        {
            const ScRange* pArea = GetPrintRange( 0 );
            if (pArea)
            {
                nStartCol = pArea->aStart.Col();
                nStartRow = pArea->aStart.Row();
                nEndCol = pArea->aEnd.Col();
                nEndRow = pArea->aEnd.Row();
            }
        }           // sonst alles
    }

    // bSkipBreaks holen:

    BOOL bSkipBreaks = FALSE;

    if ( pStyleSet->GetItemState( ATTR_PAGE_SCALETOPAGES, FALSE, &pItem ) == SFX_ITEM_SET )
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "falsches Item" );
        bSkipBreaks = ( ((const SfxUInt16Item*)pItem)->GetValue() > 0 );
    }

    //--------------------------------------------------------------------------

    long nPageSizeX = aPageSizeTwips.Width();
    long nPageSizeY = aPageSizeTwips.Height();

        //  Anfang: Breaks loeschen

    for (nX=0; nX<nStartCol; nX++)
        pColFlags[nX] &= ~CR_PAGEBREAK;
    for (nY=0; nY<nStartRow; nY++)
        pRowFlags[nY] &= ~CR_PAGEBREAK;

    if (nStartCol > 0)
        pColFlags[nStartCol] |= CR_PAGEBREAK;           //! AREABREAK
    if (nStartRow > 0)
        pRowFlags[nStartRow] |= CR_PAGEBREAK;           //! AREABREAK

        //  Mittelteil: Breaks verteilen

    BOOL bRepeatCol = ( nRepeatStartX != SCCOL_REPEAT_NONE );
    BOOL bColFound = FALSE;
    long nSizeX = 0;
    for (nX=nStartCol; nX<=nEndCol; nX++)
    {
        BOOL bStartOfPage = FALSE;
        long nThisX = ( pColFlags[nX] & CR_HIDDEN ) ? 0 : pColWidth[nX];
        if ( (nSizeX+nThisX > nPageSizeX) || ((pColFlags[nX] & CR_MANUALBREAK) && !bSkipBreaks) )
        {
            pColFlags[nX] |= CR_PAGEBREAK;
            nSizeX = 0;
            bStartOfPage = TRUE;
        }
        else if (nX != nStartCol)
            pColFlags[nX] &= ~CR_PAGEBREAK;
        else
            bStartOfPage = TRUE;

        if ( bStartOfPage && bRepeatCol && nX>nRepeatStartX && !bColFound )
        {
            // subtract size of repeat columns from page size
            for (SCCOL i=nRepeatStartX; i<=nRepeatEndX; i++)
                nPageSizeX -= ( pColFlags[i] & CR_HIDDEN ) ? 0 : pColWidth[i];
            while (nX<=nRepeatEndX)
                pColFlags[++nX] &= ~CR_PAGEBREAK;
            bColFound = TRUE;
        }

        nSizeX += nThisX;
    }

    BOOL bRepeatRow = ( nRepeatStartY != SCROW_REPEAT_NONE );
    BOOL bRowFound = FALSE;
    long nSizeY = 0;
    for (nY=nStartRow; nY<=nEndRow; nY++)
    {
        BOOL bStartOfPage = FALSE;
        long nThisY = ( pRowFlags[nY] & CR_HIDDEN ) ? 0 : pRowHeight[nY];
        if ( (nSizeY+nThisY > nPageSizeY) || ((pRowFlags[nY] & CR_MANUALBREAK) && !bSkipBreaks) )
        {
            pRowFlags[nY] |= CR_PAGEBREAK;
            nSizeY = 0;
            bStartOfPage = TRUE;
        }
        else if (nY != nStartRow)
            pRowFlags[nY] &= ~CR_PAGEBREAK;
        else
            bStartOfPage = TRUE;

        if ( bStartOfPage && bRepeatRow && nY>nRepeatStartY && !bRowFound )
        {
            // subtract size of repeat rows from page size
            for (SCROW i=nRepeatStartY; i<=nRepeatEndY; i++)
                nPageSizeY -= ( pRowFlags[i] & CR_HIDDEN ) ? 0 : pRowHeight[i];
            while (nY<=nRepeatEndY)
                pRowFlags[++nY] &= ~CR_PAGEBREAK;
            bRowFound = TRUE;
        }

        nSizeY += nThisY;
    }

        //  Ende: Breaks loeschen

    if (nEndCol < MAXCOL)
    {
        pColFlags[nEndCol+1] |= CR_PAGEBREAK;           //! AREABREAK
        for (nX=nEndCol+2; nX<=MAXCOL; nX++)
            pColFlags[nX] &= ~CR_PAGEBREAK;
    }
    if (nEndRow < MAXROW)
    {
        pRowFlags[nEndRow+1] |= CR_PAGEBREAK;           //! AREABREAK
        for (nY=nEndRow+2; nY<=MAXROW; nY++)
            pRowFlags[nY] &= ~CR_PAGEBREAK;
    }
}

void ScTable::RemoveManualBreaks()
{
    if (pColFlags)
        for (SCCOL nCol = 0; nCol <= MAXCOL; nCol++)
            pColFlags[nCol] &= ~CR_MANUALBREAK;

    if (pRowFlags)
        for (SCROW nRow = 0; nRow <= MAXROW; nRow++)
            pRowFlags[nRow] &= ~CR_MANUALBREAK;
}

BOOL ScTable::HasManualBreaks() const
{
    if (pColFlags)
        for (SCCOL nCol = 0; nCol <= MAXCOL; nCol++)
            if ( pColFlags[nCol] & CR_MANUALBREAK )
                return TRUE;

    if (pRowFlags)
        for (SCROW nRow = 0; nRow <= MAXROW; nRow++)
            if ( pRowFlags[nRow] & CR_MANUALBREAK )
                return TRUE;

    return FALSE;
}

void ScTable::SetPageSize( const Size& rSize )
{
    if ( rSize.Width() != 0 && rSize.Height() != 0 )
    {
        bPageSizeValid = TRUE;
        aPageSizeTwips = rSize;
    }
    else
        bPageSizeValid = FALSE;
}

Size ScTable::GetPageSize() const
{
    if ( bPageSizeValid )
        return aPageSizeTwips;
    else
        return Size();  // leer
}

void ScTable::SetRepeatArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow )
{
    nRepeatStartX = nStartCol;
    nRepeatEndX   = nEndCol;
    nRepeatStartY = nStartRow;
    nRepeatEndY   = nEndRow;
}

void ScTable::StartListening( const ScAddress& rAddress, SvtListener* pListener )
{
    aCol[rAddress.Col()].StartListening( *pListener, rAddress.Row() );
}

void ScTable::EndListening( const ScAddress& rAddress, SvtListener* pListener )
{
    aCol[rAddress.Col()].EndListening( *pListener, rAddress.Row() );
}

void ScTable::SetPageStyle( const String& rName )
{
    if ( aPageStyle != rName )
    {
        String                  aStrNew    = rName;
        SfxStyleSheetBasePool*  pStylePool = pDocument->GetStyleSheetPool();
        SfxStyleSheetBase*      pNewStyle  = pStylePool->Find( aStrNew, SFX_STYLE_FAMILY_PAGE );

        if ( !pNewStyle )
        {
            aStrNew = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
            pNewStyle = pStylePool->Find( aStrNew, SFX_STYLE_FAMILY_PAGE );
        }

        if ( aPageStyle != aStrNew )
        {
            SfxStyleSheetBase* pOldStyle = pStylePool->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );

            if ( pOldStyle && pNewStyle )
            {
                SfxItemSet&  rOldSet          = pOldStyle->GetItemSet();
                SfxItemSet&  rNewSet          = pNewStyle->GetItemSet();
                const USHORT nOldScale        = GET_SCALEVALUE(rOldSet,ATTR_PAGE_SCALE);
                const USHORT nOldScaleToPages = GET_SCALEVALUE(rOldSet,ATTR_PAGE_SCALETOPAGES);
                const USHORT nNewScale        = GET_SCALEVALUE(rNewSet,ATTR_PAGE_SCALE);
                const USHORT nNewScaleToPages = GET_SCALEVALUE(rNewSet,ATTR_PAGE_SCALETOPAGES);

                if ( (nOldScale != nNewScale) || (nOldScaleToPages != nNewScaleToPages) )
                    InvalidateTextWidth();
            }

            if ( pNewStyle )            // auch ohne den alten (fuer UpdateStdNames)
                aPageStyle = aStrNew;
        }
    }
}

void ScTable::PageStyleModified( const String& rNewName )
{
    aPageStyle = rNewName;
    InvalidateTextWidth();          // man weiss nicht mehr, was vorher drinstand...
}

void ScTable::InvalidateTextWidth( const ScAddress* pAdrFrom,
                                   const ScAddress* pAdrTo,
                                   BOOL bBroadcast )
{
    if ( pAdrFrom && !pAdrTo )
    {
        ScBaseCell* pCell = aCol[pAdrFrom->Col()].GetCell( pAdrFrom->Row() );
        if ( pCell )
        {
            pCell->SetTextWidth( TEXTWIDTH_DIRTY );
            pCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
            if ( bBroadcast )
            {   // nur bei CalcAsShown
                switch ( pCell->GetCellType() )
                {
                    case CELLTYPE_VALUE :
                        pDocument->Broadcast( SC_HINT_DATACHANGED,
                            ScAddress( pAdrFrom->Col(), pAdrFrom->Row(), nTab ),
                            pCell );
                        break;
                    case CELLTYPE_FORMULA :
                        ((ScFormulaCell*)pCell)->SetDirty();
                        break;
                }
            }
        }
    }
    else
    {
        const SCCOL nColStart = pAdrFrom ? pAdrFrom->Col() : 0;
        const SCROW nRowStart = pAdrFrom ? pAdrFrom->Row() : 0;
        const SCCOL nColEnd   = pAdrTo   ? pAdrTo->Col()   : MAXCOL;
        const SCROW nRowEnd   = pAdrTo   ? pAdrTo->Row()   : MAXROW;

        for ( SCCOL nCol=nColStart; nCol<=nColEnd; nCol++ )
        {
            ScColumnIterator aIter( &aCol[nCol], nRowStart, nRowEnd );
            ScBaseCell*      pCell = NULL;
            SCROW            nRow  = nRowStart;

            while ( aIter.Next( nRow, pCell ) )
            {
                pCell->SetTextWidth( TEXTWIDTH_DIRTY );
                pCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
                if ( bBroadcast )
                {   // nur bei CalcAsShown
                    switch ( pCell->GetCellType() )
                    {
                        case CELLTYPE_VALUE :
                            pDocument->Broadcast( SC_HINT_DATACHANGED,
                                ScAddress( nCol, nRow, nTab ), pCell );
                            break;
                        case CELLTYPE_FORMULA :
                            ((ScFormulaCell*)pCell)->SetDirty();
                            break;
                    }
                }
            }
        }
    }
}





