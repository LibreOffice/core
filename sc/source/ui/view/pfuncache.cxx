/*************************************************************************
 *
 *  $RCSfile: pfuncache.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 12:03:53 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "pfuncache.hxx"
#include "printfun.hxx"
#include "docsh.hxx"
#include "markdata.hxx"

//------------------------------------------------------------------------

ScPrintFuncCache::ScPrintFuncCache( ScDocShell* pD, const ScMarkData& rMark,
                                    const ScPrintSelectionStatus& rStatus ) :
    aSelection( rStatus ),
    pDocSh( pD ),
    nTotalPages( 0 )
{
    //  page count uses the stored cell widths for the printer anyway,
    //  so ScPrintFunc with the document's printer can be used to count

    SfxPrinter* pPrinter = pDocSh->GetPrinter();

    ScRange aRange;
    const ScRange* pSelRange = NULL;
    if ( rMark.IsMarked() )
    {
        rMark.GetMarkArea( aRange );
        pSelRange = &aRange;
    }

    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTab;
    for ( nTab=0; nTab<nTabCount; nTab++ )
    {
        long nAttrPage = nTab > 0 ? nFirstAttr[nTab-1] : 1;

        long nThisTab = 0;
        if ( rMark.GetTableSelect( nTab ) )
        {
            ScPrintFunc aFunc( pDocSh, pPrinter, nTab, nAttrPage, 0, pSelRange );
            nThisTab = aFunc.GetTotalPages();
            nFirstAttr[nTab] = aFunc.GetFirstPageNo();          // from page style or previous sheet
        }
        else
            nFirstAttr[nTab] = nAttrPage;

        nPages[nTab] = nThisTab;
        nTotalPages += nThisTab;
    }
}

ScPrintFuncCache::~ScPrintFuncCache()
{
}

BOOL ScPrintFuncCache::IsSameSelection( const ScPrintSelectionStatus& rStatus ) const
{
    return aSelection == rStatus;
}

SCTAB ScPrintFuncCache::GetTabForPage( long nPage ) const
{
    ScDocument* pDoc = pDocSh->GetDocument();
    SCTAB nTabCount = pDoc->GetTableCount();
    SCTAB nTab = 0;
    while ( nTab < nTabCount && nPage >= nPages[nTab] )
        nPage -= nPages[nTab++];
    return nTab;
}

long ScPrintFuncCache::GetTabStart( SCTAB nTab ) const
{
    long nRet = 0;
    for ( SCTAB i=0; i<nTab; i++ )
        nRet += nPages[i];
    return nRet;
}

long ScPrintFuncCache::GetDisplayStart( SCTAB nTab ) const
{
    //! merge with lcl_GetDisplayStart in preview?

    long nDisplayStart = 0;
    ScDocument* pDoc = pDocSh->GetDocument();
    for (SCTAB i=0; i<nTab; i++)
    {
        if ( pDoc->NeedPageResetAfterTab(i) )
            nDisplayStart = 0;
        else
            nDisplayStart += nPages[i];
    }
    return nDisplayStart;
}


