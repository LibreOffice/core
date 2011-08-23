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

#include <bf_sfx2/objsh.hxx>
#include <vcl/svapp.hxx>


#include "addinlis.hxx"
#include "miscuno.hxx"		// SC_IMPL_SERVICE_INFO
#include "document.hxx"
#include "bf_sc.hrc"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

//SMART_UNO_IMPLEMENTATION( ScAddInListener, UsrObject );

/*N*/ SC_SIMPLE_SERVICE_INFO( ScAddInListener, "ScAddInListener", "stardiv.one.sheet.AddInListener" )

//------------------------------------------------------------------------

/*N*/ List ScAddInListener::aAllListeners;

//------------------------------------------------------------------------

/*N*/ //	static
/*N*/ ScAddInListener* ScAddInListener::CreateListener(
/*N*/ 						uno::Reference<sheet::XVolatileResult> xVR, ScDocument* pDoc )
/*N*/ {
/*N*/ 	ScAddInListener* pNew = new ScAddInListener( xVR, pDoc );
/*N*/ 
/*N*/ 	pNew->acquire();								// for aAllListeners
/*N*/ 	aAllListeners.Insert( pNew, LIST_APPEND );
/*N*/ 
/*N*/ 	if ( xVR.is() )	
/*N*/ 		xVR->addResultListener( pNew );				// after at least 1 ref exists!
/*N*/ 
/*N*/ 	return pNew;
/*N*/ }

/*N*/ ScAddInListener::ScAddInListener( uno::Reference<sheet::XVolatileResult> xVR, ScDocument* pDoc ) :
/*N*/ 	xVolRes( xVR )
/*N*/ {
/*N*/ 	pDocs = new ScAddInDocs( 1, 1 );
/*N*/ 	pDocs->Insert( pDoc );
/*N*/ }

/*N*/ ScAddInListener::~ScAddInListener()
/*N*/ {
/*N*/ 	delete pDocs;
/*N*/ }

/*N*/ // static
/*N*/ ScAddInListener* ScAddInListener::Get( uno::Reference<sheet::XVolatileResult> xVR )
/*N*/ {
/*N*/ 	sheet::XVolatileResult* pComp = xVR.get();
/*N*/ 
/*N*/ 	ULONG nCount = aAllListeners.Count();
/*N*/ 	for (ULONG nPos=0; nPos<nCount; nPos++)
/*N*/ 	{
/*N*/ 		ScAddInListener* pLst = (ScAddInListener*)aAllListeners.GetObject(nPos);
/*N*/ 		if ( pComp == (sheet::XVolatileResult*)pLst->xVolRes.get() )
/*N*/ 			return pLst;
/*N*/ 	}
/*N*/ 	return NULL;		// not found
/*N*/ }

//!	move to some container object?
// static
/*N*/ void ScAddInListener::RemoveDocument( ScDocument* pDocumentP )
/*N*/ {
/*N*/ 	ULONG nPos = aAllListeners.Count();
/*N*/ 	while (nPos)
/*N*/ 	{
/*?*/ 		//	loop backwards because elements are removed
/*?*/ 		--nPos;
/*?*/ 		ScAddInListener* pLst = (ScAddInListener*)aAllListeners.GetObject(nPos);
/*?*/ 		ScAddInDocs* p = pLst->pDocs;
/*?*/ 		USHORT nFoundPos;
/*?*/ 		if ( p->Seek_Entry( pDocumentP, &nFoundPos ) )
/*?*/ 		{
/*?*/ 			p->Remove( nFoundPos );
/*?*/ 			if ( p->Count() == 0 )
/*?*/ 			{
/*?*/ 				// this AddIn is no longer used
/*?*/ 				//	dont delete, just remove the ref for the list
/*?*/ 
/*?*/ 				aAllListeners.Remove( nPos );
/*?*/ 
/*?*/ 				if ( pLst->xVolRes.is() )	
/*?*/ 					pLst->xVolRes->removeResultListener( pLst );
/*?*/ 
/*?*/ 				pLst->release();	// Ref for aAllListeners - pLst may be deleted here
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------------

// XResultListener

/*N*/ void SAL_CALL ScAddInListener::modified( const ::com::sun::star::sheet::ResultEvent& aEvent )
/*N*/ 								throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	SolarMutexGuard aGuard;			//! or generate a UserEvent
/*N*/ 
/*N*/ 	aResult = aEvent.Value;		// store result
/*N*/ 
/*N*/ 	if ( !HasListeners() )
/*N*/ 	{
/*N*/ 		//!	remove from list and removeListener, as in RemoveDocument ???
/*N*/ 
/*N*/ #if 0
/*N*/ 		//!	this will crash if called before first StartListening !!!
/*N*/ 		aAllListeners.Remove( this );
/*N*/ 		if ( xVolRes.is() )	
/*N*/ 			xVolRes->removeResultListener( this );
/*N*/ 		release();	// Ref for aAllListeners - this may be deleted here
/*N*/ 		return;
/*N*/ #endif
/*N*/ 	}
/*N*/ 
/*N*/ 	//	notify document of changes
/*N*/ 
/*N*/ 	Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress( 0 ), NULL ) );
/*N*/ 
/*N*/ 	const ScDocument** ppDoc = (const ScDocument**) pDocs->GetData();
/*N*/ 	USHORT nCount = pDocs->Count();
/*N*/ 	for ( USHORT j=0; j<nCount; j++, ppDoc++ )
/*N*/ 	{
/*N*/ 		ScDocument* pDoc = (ScDocument*)*ppDoc;
/*N*/ 		pDoc->TrackFormulas();
/*N*/ 		pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
/*N*/ 		pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
/*N*/ 	}
/*N*/ }

// XEventListener

/*N*/ void SAL_CALL ScAddInListener::disposing( const ::com::sun::star::lang::EventObject& Source )
/*N*/ 								throw(::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	// hold a ref so this is not deleted at removeResultListener
/*N*/ 	uno::Reference<sheet::XResultListener> xRef( this );
/*N*/ 
/*N*/ 	if ( xVolRes.is() )
/*N*/ 	{
/*N*/ 		xVolRes->removeResultListener( this );
/*N*/ 		xVolRes = NULL;
/*N*/ 	}
/*N*/ }


//------------------------------------------------------------------------



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
