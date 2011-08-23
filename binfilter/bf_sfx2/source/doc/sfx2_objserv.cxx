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

#include <com/sun/star/document/XExporter.hpp>
#include <tools/urlobj.hxx>
#include <bf_svtools/whiter.hxx>
#include <bf_svtools/eitem.hxx>
#include <bf_svtools/itemset.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "request.hxx"
#include "docfile.hxx"
#include "app.hxx"
#include "docinf.hxx"
#include "objshimp.hxx"
#include "interno.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"

#include "bf_so3/staticbaseurl.hxx"

namespace binfilter {

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::task;

//====================================================================

/*N*/ class SfxSaveAsContext_Impl
/*N*/ {
/*N*/ 	String&		_rNewNameVar;
/*N*/ 	String 		_aNewName;
/*N*/
/*N*/ public:
/*N*/ 				SfxSaveAsContext_Impl( String &rNewNameVar,
/*N*/ 									   const String &rNewName )
/*N*/ 				:	_rNewNameVar( rNewNameVar ),
/*N*/ 					_aNewName( rNewName )
/*N*/ 				{ rNewNameVar = rNewName; }
/*N*/ 				~SfxSaveAsContext_Impl()
/*N*/ 				{ _rNewNameVar.Erase(); }
/*N*/ };

//====================================================================

/*N*/ BOOL ShallSetBaseURL_Impl( SfxMedium &rMed );

AsynchronLink* pPendingCloser = 0;

//=========================================================================



/*N*/ long SfxObjectShellClose_Impl( void* pObj, void* pArg )
/*N*/ {
/*N*/ 	SfxObjectShell *pObjSh = (SfxObjectShell*) pArg;
/*N*/ 	if ( pObjSh->Get_Impl()->bHiddenLockedByAPI )
/*N*/ 	{
/*N*/ 		pObjSh->Get_Impl()->bHiddenLockedByAPI = FALSE;
/*N*/ 		pObjSh->OwnerLock(FALSE);
/*N*/ 	}
/*N*/ 	else if ( !pObjSh->Get_Impl()->bClosing )
/*N*/ 		// GCC stuerzt ab, wenn schon im dtor, also vorher Flag abfragen
/*?*/ 		pObjSh->DoClose();
/*N*/ 	return 0;
/*N*/ }

/*N*/ sal_Bool SfxObjectShell::APISaveAs_Impl
/*N*/ (
/*N*/ 	const String& aFileName,
/*N*/ 	SfxItemSet*	  aParams
/*N*/ )
/*N*/ {
/*N*/     BOOL bOk = sal_False;
/*N*/
/*N*/ 	{DBG_CHKTHIS(SfxObjectShell, 0);}
/*N*/
/*N*/ 	pImp->bSetStandardName=FALSE;
/*N*/ 	if ( GetMedium() )
/*N*/ 	{
/*N*/ 		SFX_ITEMSET_ARG( aParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
/*N*/     	sal_Bool bSaveTo = pSaveToItem && pSaveToItem->GetValue();
/*N*/
/*N*/ 		String aFilterName;
/*N*/ 		SFX_ITEMSET_ARG( aParams, pFilterNameItem, SfxStringItem, SID_FILTER_NAME, sal_False );
/*N*/ 		if( pFilterNameItem )
/*N*/ 			aFilterName = pFilterNameItem->GetValue();
/*N*/
/*N*/ 		// in case no filter defined use default one
/*N*/ 		if( !aFilterName.Len() )
/*N*/ 		{
/*N*/ 			sal_uInt16 nActFilt = 0;
/*N*/ 			const SfxFilter* pFilt = GetFactory().GetFilter( 0 ); for( ;
/*N*/ 				 pFilt && ( !pFilt->CanExport()
/*N*/ 				  || !bSaveTo && !pFilt->CanImport() // SaveAs case
/*N*/ 				  || pFilt->IsInternal() );
/*N*/ 			 	 pFilt = GetFactory().GetFilter( ++nActFilt ) );
/*N*/
/*N*/ 			DBG_ASSERT( pFilt, "No default filter!\n" );
/*N*/
/*N*/ 			if( pFilt )
/*N*/         		aFilterName = pFilt->GetFilterName();
/*N*/
/*N*/         	aParams->Put(SfxStringItem( SID_FILTER_NAME, aFilterName));
/*N*/ 		}
/*N*/
/*N*/
/*N*/ 		{
/*N*/ 			SfxObjectShellRef xLock( this ); // ???
/*N*/
/*N*/     		// since saving a document modified its DocumentInfo, the current DocumentInfo must be saved on "SaveTo", because
/*N*/     		// it must be restored after saving
/*N*/ 			SfxDocumentInfo aSavedInfo;
/*N*/ 			sal_Bool bCopyTo =  bSaveTo || GetCreateMode() == SFX_CREATE_MODE_EMBEDDED;
/*N*/ 			if ( bCopyTo )
/*N*/ 				aSavedInfo = GetDocInfo();
/*N*/
/*N*/ 	        bOk = CommonSaveAs_Impl( INetURLObject(aFileName), aFilterName,
/*N*/		 		aParams );
/*N*/
/*N*/ 			if ( bCopyTo )
/*N*/ 			{
/*N*/         		// restore DocumentInfo if only a copy was created
/*N*/ 				SfxDocumentInfo &rDocInfo = GetDocInfo();
/*N*/ 				rDocInfo = aSavedInfo;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		// Picklisten-Eintrag verhindern
/*N*/ 		GetMedium()->SetUpdatePickList( FALSE );
/*N*/ 	}
/*N*/
/*N*/ 	return bOk;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
