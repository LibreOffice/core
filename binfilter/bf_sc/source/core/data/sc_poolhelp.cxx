/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <bf_svtools/zforlist.hxx>
#include <bf_svx/editeng.hxx>

#include "poolhelp.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "stlpool.hxx"

namespace binfilter {

// -----------------------------------------------------------------------

/*N*/ ScPoolHelper::ScPoolHelper( ScDocument* pSourceDoc )
/*N*/ {
/*N*/ 	DBG_ASSERT( pSourceDoc, "ScPoolHelper: no document" );
/*N*/ 
/*N*/ 	pDocPool = new ScDocumentPool;
/*N*/ 	pDocPool->FreezeIdRanges();
/*N*/ 
/*N*/ 	pStylePool = new ScStyleSheetPool( *pDocPool, pSourceDoc );
/*N*/ 
/*N*/ 	pFormTable = new SvNumberFormatter( pSourceDoc->GetServiceManager(), ScGlobal::eLnge );
/*N*/     pFormTable->SetColorLink( LINK( pSourceDoc, ScDocument, GetUserDefinedColor ) );
/*N*/ 	pFormTable->SetEvalDateFormat( NF_EVALDATEFORMAT_INTL_FORMAT );
/*N*/ 
/*N*/ 	pEditPool = EditEngine::CreatePool();
/*N*/ 	pEditPool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
/*N*/ 	pEditPool->FreezeIdRanges();
/*N*/ 	pEditPool->SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );	// used in ScGlobal::EETextObjEqual
/*N*/ 
/*N*/ 	pEnginePool = EditEngine::CreatePool();
/*N*/ 	pEnginePool->SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
/*N*/ 	pEnginePool->FreezeIdRanges();
/*N*/ }

/*N*/ ScPoolHelper::~ScPoolHelper()
/*N*/ {
/*N*/ 	delete pEnginePool;
/*N*/ 	delete pEditPool;
/*N*/ 	delete pFormTable;
/*N*/ 	delete pStylePool;
/*N*/ 	delete pDocPool;
/*N*/ }

/*N*/ void ScPoolHelper::SourceDocumentGone()
/*N*/ {
/*N*/ 	//	reset all pointers to the source document
/*N*/ 	pStylePool->SetDocument( NULL );
/*N*/     pFormTable->SetColorLink( Link() );
/*N*/ }

// -----------------------------------------------------------------------


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
