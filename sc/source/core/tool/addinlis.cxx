/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sfx2/objsh.hxx>
#include <vcl/svapp.hxx>

#include "addinlis.hxx"
#include "miscuno.hxx"
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
    pDocs = new ScAddInDocs();
    pDocs->insert( pDoc );
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
        if ( pComp == (*iter)->xVolRes.get() )
        {
            pLst = *iter;
            break;
        }
    }
    return pLst;
}

//TODO: move to some container object?
void ScAddInListener::RemoveDocument( ScDocument* pDocumentP )
{
    ::std::list<ScAddInListener*>::iterator iter = aAllListeners.begin();
    while(iter != aAllListeners.end())
    {
        ScAddInDocs* p = (*iter)->pDocs;
        ScAddInDocs::iterator iter2 = p->find( pDocumentP );
        if( iter2 != p->end() )
        {
            p->erase( iter2 );
            if ( p->empty() )
            {
                if ( (*iter)->xVolRes.is() )
                    (*iter)->xVolRes->removeResultListener( *iter );

                (*iter)->release(); // Ref for aAllListeners - pLst may be deleted here

                // this AddIn is no longer used
                // don't delete, just remove the ref for the list

                iter = aAllListeners.erase( iter );
                continue;
            }
        }
        ++iter;
    }
}

// XResultListener

void SAL_CALL ScAddInListener::modified( const ::com::sun::star::sheet::ResultEvent& aEvent )
                                throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard; //TODO: or generate a UserEvent

    aResult = aEvent.Value; // store result

    // notify document of changes

    Broadcast( ScHint(SC_HINT_DATACHANGED, ScAddress()) );

    for ( ScAddInDocs::iterator it = pDocs->begin(); it != pDocs->end(); ++it )
    {
        ScDocument* pDoc = *it;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
    }
}

// XEventListener

void SAL_CALL ScAddInListener::disposing( const ::com::sun::star::lang::EventObject& /* Source */ )
                                throw(::com::sun::star::uno::RuntimeException, std::exception)
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
