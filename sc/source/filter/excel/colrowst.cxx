/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colrowst.cxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-28 11:40:25 $
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
#ifdef PCH
#include "filt_pch.hxx"
#endif


#pragma hdrstop

#include "colrowst.hxx"

#include <string.h>

#include "document.hxx"
#include "root.hxx"

#ifndef SC_FTOOLS_HXX
#include "ftools.hxx"
#endif
#ifndef SC_XLTABLE_HXX
#include "xltable.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif

// for filter manager
#include "excimp8.hxx"


XclImpColRowSettings::XclImpColRowSettings( RootData& rRootData ) :
    ExcRoot( &rRootData )
{
    nDefWidth = nDefHeight = 0;

    pWidth = new INT32 [ MAXCOL + 1 ];
    pColHidden = new BOOL [ MAXCOL + 1 ];

    pHeight = new UINT16 [ MAXROW + 1 ];
    pRowFlags = new INT8[ MAXROW + 1 ];

    Reset();
}


XclImpColRowSettings::~XclImpColRowSettings()
{
    delete[] pRowFlags;
    delete[] pHeight;
    delete[] pColHidden;
    delete[] pWidth;
}


void XclImpColRowSettings::Reset( void )
{
    SCCOL   nC;
    for( nC = 0 ; nC <= MAXCOL ; nC++ )
    {
        pColHidden[ nC ] = FALSE;
        pWidth[ nC ] = -1;
    }

    memset( pRowFlags, 0x00, sizeof( INT8 ) * ( MAXROW + 1 ) );

    bDirty = TRUE;
    nMaxRow = -1;

    bSetByStandard = FALSE;
}


void XclImpColRowSettings::Apply( SCTAB nScTab )
{
    if( !bDirty )
        return;

    SCCOLROW                nC;
    SCROW                   nStart = 0;
    UINT16                  nWidth;
    UINT16                  nLastWidth = ( pWidth[ 0 ] >= 0 )? ( UINT16 ) pWidth[ 0 ] : nDefWidth;
    ScDocument&             rD = pExcRoot->pIR->GetDoc();

    rD.IncSizeRecalcLevel( nScTab );

    // Column-Bemachung
    for( nC = 0 ; nC <= MAXCOL ; nC++ )
    {
        if( pWidth[ nC ] >= 0 )
            // eingestellte Width
            nWidth = ( UINT16 ) pWidth[ nC ];
        else
            // Default-Width
            nWidth = nDefWidth;

        if( nWidth == 0 )
        {
            pColHidden[ nC ] = TRUE;
            // Column hidden: remember original column width and set width 0.
            // Needed for #i11776#, no HIDDEN flags in the document, until
            // filters and outlines are inserted.
            pWidth[ nC ] = rD.GetColWidth( static_cast<SCCOL>( nC ), nScTab );
        }
        rD.SetColWidth( static_cast<SCCOL>( nC ), nScTab, nWidth );
    }

    // Row-Bemachung

    INT8                    nFlags;
    nStart = 0;
    UINT16                  nHeight;

    UINT16                  nLastHeight;
    nFlags = pRowFlags[ 0 ];
    if( nFlags & ROWFLAG_USED )
    {
        if( nFlags & ROWFLAG_DEFAULT )
            nLastHeight = nDefHeight;
        else
        {
            nLastHeight = pHeight[ 0 ];
            if( !nLastHeight )
                nLastHeight = nDefHeight;
        }
    }
    else
        nLastHeight = nDefHeight;

    for( nC = 0 ; nC <= nMaxRow ; nC++ )
    {
        nFlags = pRowFlags[ nC ];

        if( nFlags & ROWFLAG_USED )
        {
            if( nFlags & ROWFLAG_DEFAULT )
                nHeight = nDefHeight;
            else
            {
                nHeight = pHeight[ nC ];
                if( !nHeight )
                    nHeight = nDefHeight;
            }

            if( nFlags & ( ROWFLAG_HIDDEN | ROWFLAG_MAN ) )
            {
                BYTE        nSCFlags = rD.GetRowFlags( nC , nScTab );

                if( nFlags & ROWFLAG_MAN )
                    nSCFlags |= CR_MANUALSIZE;

                rD.SetRowFlags( nC, nScTab, nSCFlags );
            }
        }
        else
            nHeight = nDefHeight;

        if( !nHeight )
        {
            pRowFlags[ nC ] |= ROWFLAG_HIDDEN;
            // Row hidden: remember original row height and set height 0.
            // Needed for #i11776#, no HIDDEN flags in the document, until
            // filters and outlines are inserted.
            pHeight[ nC ] = rD.GetRowHeight( nC, nScTab );
        }

        if( nLastHeight != nHeight )
        {
            DBG_ASSERT( nC > 0, "XclImpColRowSettings::Apply(): Algorithmus-Fehler!" );

            if( nLastHeight )
                rD.SetRowHeightRange( nStart, nC - 1, nScTab, nLastHeight );

            nStart = nC;
            nLastHeight = nHeight;
        }
    }

    if( nLastHeight && nMaxRow >= 0 )
        rD.SetRowHeightRange( nStart, static_cast<SCROW>( nMaxRow ), nScTab, nLastHeight );

    bDirty = FALSE; // jetzt stimmt Tabelle im ScDocument

    rD.DecSizeRecalcLevel( nScTab );
}


void XclImpColRowSettings::SetHiddenFlags( SCTAB nScTab )
{
    ScDocument& rDoc = pExcRoot->pIR->GetDoc();

    for( SCCOL nScCol = 0; nScCol <= MAXCOL; ++nScCol )
    {
        if( pColHidden[ nScCol ] )
        {
            // set original width, needed to unhide the column
            if( pWidth[ nScCol ] > 0 )
                rDoc.SetColWidth( nScCol, nScTab, static_cast< sal_uInt16 >( pWidth[ nScCol ] ) );
            // really hide the column
            rDoc.ShowCol( nScCol, nScTab, FALSE );
        }
    }

    // #i38093# rows hidden by filter need extra flag
    SCROW nFirstFilterScRow = SCROW_MAX;
    SCROW nLastFilterScRow = SCROW_MAX;
    if( pExcRoot->pIR->GetBiff() == EXC_BIFF8 )
    {
        const XclImpAutoFilterData* pFilter = pExcRoot->pIR->GetFilterManager().GetByTab( nScTab );
        if( pFilter && pFilter->IsActive() )
        {
            nFirstFilterScRow = pFilter->StartRow();
            nLastFilterScRow = pFilter->EndRow();
        }
    }

    for( SCROW nScRow = 0; nScRow <= nMaxRow; ++nScRow )
    {
        if( pRowFlags[ nScRow ] & ROWFLAG_HIDDEN )
        {
            // set original height, needed to unhide the row
            if( pHeight[ nScRow ] > 0 )
                rDoc.SetRowHeight( nScRow, nScTab, pHeight[ nScRow ] );
            // really hide the row
            rDoc.ShowRow( nScRow, nScTab, FALSE );
            // #i38093# rows hidden by filter need extra flag
            if( (nFirstFilterScRow <= nScRow) && (nScRow <= nLastFilterScRow) )
                rDoc.SetRowFlags( nScRow, nScTab, rDoc.GetRowFlags( nScRow, nScTab ) | CR_FILTERED );
        }
    }

    // #i47438# if default row format is hidden, hide remaining rows
    if( (nDefHeight == 0) && (nMaxRow < MAXROW) )
        rDoc.ShowRows( nMaxRow + 1, MAXROW, nScTab, FALSE );
}


void XclImpColRowSettings::HideColRange( SCCOL nColFirst, SCCOL nColLast )
{
    DBG_ASSERT( nColFirst <= nColLast, "+XclImpColRowSettings::HideColRange(): First > Last?!" );
    DBG_ASSERT( ValidCol(nColLast), "+XclImpColRowSettings::HideColRange(): ungueltige Column" );

    if( !ValidCol(nColLast) )
        nColLast = MAXCOL;

    BOOL*   pHidden;
    BOOL*   pFinish;
    pHidden = &pColHidden[ nColFirst ];
    pFinish = &pColHidden[ nColLast ];
    while( pHidden <= pFinish )
        *( pHidden++ ) = TRUE;
}


void XclImpColRowSettings::SetWidthRange( SCCOL nColFirst, SCCOL nColLast, UINT16 nNew )
{
    DBG_ASSERT( nColFirst <= nColLast, "+XclImpColRowSettings::SetColWidthRange(): First > Last?!" );
    DBG_ASSERT( ValidCol(nColLast), "+XclImpColRowSettings::SetColWidthRange(): ungueltige Column" );

    if( !ValidCol(nColLast) )
        nColLast = MAXCOL;

    INT32*  pWidthCount;
    INT32*  pFinish;
    pWidthCount = &pWidth[ nColFirst ];
    pFinish = &pWidth[ nColLast ];

    while( pWidthCount <= pFinish )
        *( pWidthCount++ ) = nNew;
}


void XclImpColRowSettings::SetDefaultXF( SCCOL nColFirst, SCCOL nColLast, UINT16 nXF )
{
    DBG_ASSERT( nColFirst <= nColLast, "+XclImpColRowSettings::SetDefaultXF(): First > Last?!" );
    DBG_ASSERT( ValidCol(nColLast), "+XclImpColRowSettings::SetDefaultXF(): ungueltige Column" );

    if( !ValidCol(nColLast) )
        nColLast = MAXCOL;

    const XclImpRoot& rRoot = *pExcRoot->pIR;

    // #109555# assign the default column formatting here to ensure
    // that explicit cell formatting is not overwritten.
    for( SCCOL nScCol = nColFirst; nScCol <= nColLast; ++nScCol )
        rRoot.GetXFRangeBuffer().SetColumnDefXF( nScCol, nXF );
}


void XclImpColRowSettings::SetDefaults( UINT16 nWidth, UINT16 nHeight )
{
    nDefWidth = nWidth;
    nDefHeight = nHeight;
}


void XclImpColRowSettings::_SetRowSettings( const SCROW nRow, const UINT16 nExcelHeight, const UINT16 nGrbit )
{
    pHeight[ nRow ] = nExcelHeight & 0x7FFF;

    INT8    nFlags = ROWFLAG_USED;

    if( nExcelHeight & 0x8000 )
        nFlags |= ROWFLAG_DEFAULT;

    if( nGrbit & EXC_ROW_UNSYNCED )
        nFlags |= ROWFLAG_MAN;

    if( nGrbit &  EXC_ROW_HIDDEN )
        nFlags |= ROWFLAG_HIDDEN;

    pRowFlags[ nRow ] = nFlags;

    if( nRow > nMaxRow )
        nMaxRow = nRow;
}

