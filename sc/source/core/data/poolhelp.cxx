/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: poolhelp.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:13:30 $
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



// INCLUDE ---------------------------------------------------------------

#include <svtools/zforlist.hxx>
#include <svx/editeng.hxx>

#include "poolhelp.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"

// -----------------------------------------------------------------------

ScPoolHelper::ScPoolHelper( ScDocument* pSourceDoc )
{
    DBG_ASSERT( pSourceDoc, "ScPoolHelper: no document" );

    pDocPool = new ScDocumentPool;
    pDocPool->FreezeIdRanges();

    mxStylePool = new ScStyleSheetPool( *pDocPool, pSourceDoc );

    pFormTable = new SvNumberFormatter( pSourceDoc->GetServiceManager(), ScGlobal::eLnge );
    pFormTable->SetColorLink( LINK( pSourceDoc, ScDocument, GetUserDefinedColor ) );
    pFormTable->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );

    pEditPool = EditEngine::CreatePool();
    pEditPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
    pEditPool->FreezeIdRanges();
    pEditPool->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );   // used in ScGlobal::EETextObjEqual

    pEnginePool = EditEngine::CreatePool();
    pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
    pEnginePool->FreezeIdRanges();
}

ScPoolHelper::~ScPoolHelper()
{
    delete pEnginePool;
    delete pEditPool;
    delete pFormTable;
    mxStylePool.clear();
    delete pDocPool;
}

void ScPoolHelper::SourceDocumentGone()
{
    //  reset all pointers to the source document
    mxStylePool->SetDocument( NULL );
    pFormTable->SetColorLink( Link() );
}

// -----------------------------------------------------------------------


