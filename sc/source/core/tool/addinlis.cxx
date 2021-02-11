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

#include <addinlis.hxx>
#include <miscuno.hxx>
#include <document.hxx>
#include <brdcst.hxx>

#include <com/sun/star/sheet/XVolatileResult.hpp>

using namespace com::sun::star;

SC_SIMPLE_SERVICE_INFO( ScAddInListener, "ScAddInListener", "stardiv.one.sheet.AddInListener" )

::std::vector<rtl::Reference<ScAddInListener>> ScAddInListener::aAllListeners;

ScAddInListener* ScAddInListener::CreateListener(
                        const uno::Reference<sheet::XVolatileResult>& xVR, ScDocument* pDoc )
{
    rtl::Reference<ScAddInListener> xNew = new ScAddInListener( xVR, pDoc );

    aAllListeners.push_back( xNew );

    if ( xVR.is() )
        xVR->addResultListener( xNew ); // after at least 1 ref exists!

    return xNew.get();
}

ScAddInListener::ScAddInListener( uno::Reference<sheet::XVolatileResult> const & xVR, ScDocument* pDoc ) :
    xVolRes( xVR ),
    pDocs( new ScAddInDocs )
{
    pDocs->insert( pDoc );
}

ScAddInListener::~ScAddInListener()
{
}

ScAddInListener* ScAddInListener::Get( const uno::Reference<sheet::XVolatileResult>& xVR )
{
    ScAddInListener* pLst = nullptr;
    sheet::XVolatileResult* pComp = xVR.get();

    for (auto const& listener : aAllListeners)
    {
        if ( pComp == listener->xVolRes.get() )
        {
            pLst = listener.get();
            break;
        }
    }
    return pLst;
}

//TODO: move to some container object?
void ScAddInListener::RemoveDocument( ScDocument* pDocumentP )
{
    auto iter = aAllListeners.begin();
    while(iter != aAllListeners.end())
    {
        ScAddInDocs* p = (*iter)->pDocs.get();
        ScAddInDocs::iterator iter2 = p->find( pDocumentP );
        if( iter2 != p->end() )
        {
            p->erase( iter2 );
            if ( p->empty() )
            {
                if ( (*iter)->xVolRes.is() )
                    (*iter)->xVolRes->removeResultListener( *iter );

                iter = aAllListeners.erase( iter );
                continue;
            }
        }
        ++iter;
    }
}

// XResultListener

void SAL_CALL ScAddInListener::modified( const css::sheet::ResultEvent& aEvent )
{
    SolarMutexGuard aGuard; //TODO: or generate a UserEvent

    aResult = aEvent.Value; // store result

    // notify document of changes

    Broadcast( ScHint(SfxHintId::ScDataChanged, ScAddress()) );

    for (auto const& pDoc : *pDocs)
    {
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxHint( SfxHintId::ScDataChanged ) );
    }
}

// XEventListener

void SAL_CALL ScAddInListener::disposing( const css::lang::EventObject& /* Source */ )
{
    // hold a ref so this is not deleted at removeResultListener
    uno::Reference<sheet::XResultListener> xKeepAlive( this );

    if ( xVolRes.is() )
    {
        xVolRes->removeResultListener( this );
        xVolRes = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
