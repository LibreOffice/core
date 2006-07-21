/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expbase.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:32:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"




#include "expbase.hxx"
#include "document.hxx"
#include "editutil.hxx"


//------------------------------------------------------------------

#if defined(MAC)
const sal_Char __FAR_DATA ScExportBase::sNewLine = '\015';
#elif defined(UNX)
const sal_Char __FAR_DATA ScExportBase::sNewLine = '\012';
#else
const sal_Char __FAR_DATA ScExportBase::sNewLine[] = "\015\012";
#endif


ScExportBase::ScExportBase( SvStream& rStrmP, ScDocument* pDocP,
                const ScRange& rRangeP )
            :
            rStrm( rStrmP ),
            aRange( rRangeP ),
            pDoc( pDocP ),
            pFormatter( pDocP->GetFormatTable() ),
            pEditEngine( NULL )
{
}


ScExportBase::~ScExportBase()
{
    delete pEditEngine;
}


BOOL ScExportBase::GetDataArea( SCTAB nTab, SCCOL& nStartCol,
            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const
{
    pDoc->GetDataStart( nTab, nStartCol, nStartRow );
    pDoc->GetPrintArea( nTab, nEndCol, nEndRow, TRUE );
    return TrimDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}


BOOL ScExportBase::TrimDataArea( SCTAB nTab, SCCOL& nStartCol,
            SCROW& nStartRow, SCCOL& nEndCol, SCROW& nEndRow ) const
{
    while ( nStartCol <= nEndCol &&
            pDoc->GetColFlags( nStartCol, nTab ) & CR_HIDDEN )
        ++nStartCol;
    while ( nStartCol <= nEndCol &&
            pDoc->GetColFlags( nEndCol, nTab ) & CR_HIDDEN )
        --nEndCol;
    nStartRow = pDoc->GetRowFlagsArray( nTab).GetFirstForCondition( nStartRow,
            nEndRow, CR_HIDDEN, 0);
    nEndRow = pDoc->GetRowFlagsArray( nTab).GetLastForCondition( nStartRow,
            nEndRow, CR_HIDDEN, 0);
    return nStartCol <= nEndCol && nStartRow <= nEndRow && nEndRow !=
        ::std::numeric_limits<SCROW>::max();
}


BOOL ScExportBase::IsEmptyTable( SCTAB nTab ) const
{
    if ( !pDoc->HasTable( nTab ) || !pDoc->IsVisible( nTab ) )
        return TRUE;
    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    return !GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}


ScFieldEditEngine& ScExportBase::GetEditEngine() const
{
    if ( !pEditEngine )
        ((ScExportBase*)this)->pEditEngine = new ScFieldEditEngine( pDoc->GetEditPool() );
    return *pEditEngine;
}


