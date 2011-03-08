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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include "addinlis.hxx"
#include "miscuno.hxx" // SC_IMPL_SERVICE_INFO
#include "document.hxx"
#include "brdcst.hxx"
#include "sc.hrc"

using namespace com::sun::star;

SC_SIMPLE_SERVICE_INFO( ScAddInListener, "ScAddInListener", "stardiv.one.sheet.AddInListener" )

::std::list<ScAddInListener*> ScAddInListener::aAllListeners;

ScAddInListener* ScAddInListener::CreateListener(
                        uno::Reference<sheet::XVolatileResult> xVR, ScDocument* pDoc )
{
    ScAddInListener* pNew = new ScAddInListener( xVR, pDoc );

    pNew->acquire(); // for aAllListeners
    aAllListeners.push_back( pNew );

    if ( xVR.is() )
        xVR->addResultListener( pNew ); // after at least 1 ref exists!

    return pNew;
}

ScAddInListener::ScAddInListener( uno::Reference<sheet::XVolatileResult> xVR, ScDocument* pDoc ) :
    xVolRes( xVR )
{
    pDocs = new ScAddInDocs( 1, 1 );
    pDocs->Insert( pDoc );
}

ScAddInListener::~ScAddInListener()
{
    delete pDocs;
}

ScAddInListener* ScAddInListener::Get( uno::Reference<sheet::XVolatileResult> xVR )
{
    ScAddInListener* pLst = NULL;
    sheet::XVolatileResult* pComp = xVR.get();

    for(::std::list<ScAddInListener*>::iterator iter = aAllListeners.begin(); iter != aAllListeners.end(); ++iter)
    {
        if ( pComp == (sheet::XVolatileResult*)(*iter)->xVolRes.get() )
        {
            pLst = *iter;
            break;
        }
    }
    return pLst;
}

//! move to some container object?
void ScAddInListener::RemoveDocument( ScDocument* pDocumentP )
{
    ::std::list<ScAddInListener*>::iterator iter = aAllListeners.begin();
    while(iter != aAllListeners.end())
    {
        ScAddInDocs* p = (*iter)->pDocs;
        sal_uInt16 nFoundPos;
        if ( p->Seek_Entry( pDocumentP, &nFoundPos ) )
        {
            p->Remove( nFoundPos );
            if ( p->Count() == 0 )
            {
                if ( (*iter)->xVolRes.is() )
                    (*iter)->xVolRes->removeResultListener( *iter );

                (*iter)->release(); // Ref for aAllListeners - pLst may be deleted here

                // this AddIn is no longer used
                // dont delete, just remove the ref for the list

                iter = aAllListeners.erase( iter );
                continue;
            }
        }
        ++iter;
    }
}

// XResultListener

void SAL_CALL ScAddInListener::modified( const ::com::sun::star::sheet::ResultEvent& aEvent )
                                throw(::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard; //! or generate a UserEvent

    aResult = aEvent.Value; // store result

    // notify document of changes

    Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress(), NULL ) );

    const ScDocument** ppDoc = (const ScDocument**) pDocs->GetData();
    sal_uInt16 nCount = pDocs->Count();
    for ( sal_uInt16 j=0; j<nCount; j++, ppDoc++ )
    {
        ScDocument* pDoc = (ScDocument*)*ppDoc;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
    }
}

// XEventListener

void SAL_CALL ScAddInListener::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
                                throw(::com::sun::star::uno::RuntimeException)
{
    // hold a ref so this is not deleted at removeResultListener
    uno::Reference<sheet::XResultListener> xRef( this );

    if ( xVolRes.is() )
    {
        xVolRes->removeResultListener( this );
        xVolRes = NULL;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
