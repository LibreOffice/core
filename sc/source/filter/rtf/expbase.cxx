/*************************************************************************
 *
 *  $RCSfile: expbase.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:14 $
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


BOOL ScExportBase::GetDataArea( USHORT nTab, USHORT& nStartCol,
            USHORT& nStartRow, USHORT& nEndCol, USHORT& nEndRow ) const
{
    pDoc->GetDataStart( nTab, nStartCol, nStartRow );
    pDoc->GetPrintArea( nTab, nEndCol, nEndRow, TRUE );
    return TrimDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}


BOOL ScExportBase::TrimDataArea( USHORT nTab, USHORT& nStartCol,
            USHORT& nStartRow, USHORT& nEndCol, USHORT& nEndRow ) const
{
    while ( nStartCol <= nEndCol &&
            pDoc->GetColFlags( nStartCol, nTab ) & CR_HIDDEN )
        ++nStartCol;
    while ( nStartCol <= nEndCol &&
            pDoc->GetColFlags( nEndCol, nTab ) & CR_HIDDEN )
        --nEndCol;
    while ( nStartRow <= nEndRow &&
            pDoc->GetRowFlags( nStartRow, nTab ) & CR_HIDDEN )
        ++nStartRow;
    while ( nStartRow <= nEndRow &&
            pDoc->GetRowFlags( nEndRow, nTab ) & CR_HIDDEN )
        --nEndRow;
    return nStartCol <= nEndCol && nStartRow <= nEndRow;
}


BOOL ScExportBase::IsEmptyTable( USHORT nTab ) const
{
    if ( !pDoc->HasTable( nTab ) )
        return TRUE;
    USHORT nStartCol, nStartRow, nEndCol, nEndRow;
    return !GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}


ScFieldEditEngine& ScExportBase::GetEditEngine() const
{
    if ( !pEditEngine )
        ((ScExportBase*)this)->pEditEngine = new ScFieldEditEngine( pDoc->GetEditPool() );
    return *pEditEngine;
}


