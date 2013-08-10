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

#include <retrieveinputstreamconsumer.hxx>
#include <ndgrf.hxx>
#include <retrieveinputstream.hxx>
#include <swthreadmanager.hxx>

/* class to provide creation of a thread to retrieve an input stream given by
    an URL and to consume the retrieved input stream.

    #i73788#
*/
SwAsyncRetrieveInputStreamThreadConsumer::SwAsyncRetrieveInputStreamThreadConsumer(
                                                            SwGrfNode& rGrfNode )
    : mrGrfNode( rGrfNode ),
      mnThreadID( 0 )
{
}

SwAsyncRetrieveInputStreamThreadConsumer::~SwAsyncRetrieveInputStreamThreadConsumer()
{
    SwThreadManager::GetThreadManager().RemoveThread( mnThreadID );
}

void SwAsyncRetrieveInputStreamThreadConsumer::CreateThread( const String& rURL )
{
    // Get new data container for input stream data
    SwRetrievedInputStreamDataManager::tDataKey nDataKey =
            SwRetrievedInputStreamDataManager::GetManager().ReserveData(
                                                mrGrfNode.GetThreadConsumer() );

    rtl::Reference< ObservableThread > pNewThread =
        SwAsyncRetrieveInputStreamThread::createThread( nDataKey, rURL );

    // Add thread to thread manager and pass ownership of thread to thread manager.
    mnThreadID = SwThreadManager::GetThreadManager().AddThread( pNewThread );
}

void SwAsyncRetrieveInputStreamThreadConsumer::ApplyInputStream(
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
    const sal_Bool bIsStreamReadOnly )
{
    mrGrfNode.ApplyInputStream( xInputStream, bIsStreamReadOnly );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
