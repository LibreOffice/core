/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: table5.cxx,v $
 * $Revision: 1.14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
#include "tabprotection.hxx"
#include "globstr.hrc"

using ::com::sun::star::uno::Sequence;

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
            pRowFlags->AndValue( 0, MAXROW-1, sal::static_int_cast<BYTE>(~CR_PAGEBREAK) );

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
    pRowFlags->AndValue( 0, nStartRow-1, sal::static_int_cast<BYTE>(~CR_PAGEBREAK) );

    if (nStartCol > 0)
        pColFlags[nStartCol] |= CR_PAGEBREAK;           //! AREABREAK
    if (nStartRow > 0)
        pRowFlags->OrValue( nStartRow, CR_PAGEBREAK);           //! AREABREAK

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

    // Remove all page breaks in range.
    pRowFlags->AndValue( nStartRow+1, nEndRow, sal::static_int_cast<BYTE>(~CR_PAGEBREAK) );
    // And set new page breaks.
    BOOL bRepeatRow = ( nRepeatStartY != SCROW_REPEAT_NONE );
    BOOL bRowFound = FALSE;
    long nSizeY = 0;
    ScCompressedArrayIterator< SCROW, BYTE> aFlagsIter( *pRowFlags, nStartRow, nEndRow);
    ScCompressedArrayIterator< SCROW, USHORT> aHeightIter( *pRowHeight, nStartRow, nEndRow);
    for ( ; aFlagsIter; ++aFlagsIter, ++aHeightIter)
    {
        nY = aFlagsIter.GetPos();
        BOOL bStartOfPage = FALSE;
        BYTE nFlags = *aFlagsIter;
        long nThisY = (nFlags & CR_HIDDEN) ? 0 : *aHeightIter;
        if ( (nSizeY+nThisY > nPageSizeY) || ((nFlags & CR_MANUALBREAK) && !bSkipBreaks) )
        {
            pRowFlags->SetValue( nY, nFlags | CR_PAGEBREAK);
            aFlagsIter.Resync( nY);
            nSizeY = 0;
            bStartOfPage = TRUE;
        }
        else if (nY != nStartRow)
            ; // page break already removed
        else
            bStartOfPage = TRUE;

        if ( bStartOfPage && bRepeatRow && nY>nRepeatStartY && !bRowFound )
        {
            // subtract size of repeat rows from page size
            unsigned long nHeights = pRowFlags->SumCoupledArrayForCondition(
                    nRepeatStartY, nRepeatEndY, CR_HIDDEN, 0, *pRowHeight);
#ifdef DBG_UTIL
            if (nHeights == ::std::numeric_limits<unsigned long>::max())
                DBG_ERRORFILE("ScTable::UpdatePageBreaks: row heights overflow");
#endif
            nPageSizeY -= nHeights;
            if (nY <= nRepeatEndY)
            {
                pRowFlags->AndValue( nY, nRepeatEndY, sal::static_int_cast<BYTE>(~CR_PAGEBREAK) );
                nY = nRepeatEndY + 1;
                aFlagsIter.Resync( nY);
                aHeightIter.Resync( nY);
            }
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
        pRowFlags->OrValue( nEndRow+1, CR_PAGEBREAK);           //! AREABREAK
        if (nEndRow+2 <= MAXROW)
            pRowFlags->AndValue( nEndRow+2, MAXROW, sal::static_int_cast<BYTE>(~CR_PAGEBREAK) );
    }
}

void ScTable::RemoveManualBreaks()
{
    if (pColFlags)
        for (SCCOL nCol = 0; nCol <= MAXCOL; nCol++)
            pColFlags[nCol] &= ~CR_MANUALBREAK;

    if (pRowFlags)
        pRowFlags->AndValue( 0, MAXROW, sal::static_int_cast<BYTE>(~CR_MANUALBREAK) );
}

BOOL ScTable::HasManualBreaks() const
{
    if (pColFlags)
        for (SCCOL nCol = 0; nCol <= MAXCOL; nCol++)
            if ( pColFlags[nCol] & CR_MANUALBREAK )
                return TRUE;

    if (pRowFlags)
        if (ValidRow( pRowFlags->GetLastAnyBitAccess( 0, CR_MANUALBREAK)))
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

BOOL ScTable::IsProtected() const
{
    return pTabProtection.get() && pTabProtection->isProtected();
}

void ScTable::SetProtection(const ScTableProtection* pProtect)
{
    if (pProtect)
        pTabProtection.reset(new ScTableProtection(*pProtect));
    else
        pTabProtection.reset(NULL);
}

ScTableProtection* ScTable::GetProtection()
{
    return pTabProtection.get();
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
                    InvalidateTextWidth(NULL, NULL, FALSE, FALSE);
            }

            if ( pNewStyle )            // auch ohne den alten (fuer UpdateStdNames)
                aPageStyle = aStrNew;

            if (IsStreamValid())
                SetStreamValid(FALSE);
        }
    }
}

void ScTable::PageStyleModified( const String& rNewName )
{
    aPageStyle = rNewName;
    InvalidateTextWidth(NULL, NULL, FALSE, FALSE);      // don't know what was in the style before
}

void ScTable::InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                   BOOL bNumFormatChanged, BOOL bBroadcast )
{
    if ( pAdrFrom && !pAdrTo )
    {
        ScBaseCell* pCell = aCol[pAdrFrom->Col()].GetCell( pAdrFrom->Row() );
        if ( pCell )
        {
            pCell->SetTextWidth( TEXTWIDTH_DIRTY );
            if ( bNumFormatChanged )
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
                    default:
                    {
                        // added to avoid warnings
                    }
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
                if ( bNumFormatChanged )
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
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
        }
    }
}





