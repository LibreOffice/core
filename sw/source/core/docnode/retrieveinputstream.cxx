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

#include <retrieveinputstream.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/io/XStream.hpp>

/* class for a thread to retrieve an input stream given by an URL

    #i73788#
*/
::rtl::Reference< ObservableThread > SwAsyncRetrieveInputStreamThread::createThread(
                        const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                        const String& rLinkedURL )
{
    SwAsyncRetrieveInputStreamThread* pNewThread =
            new SwAsyncRetrieveInputStreamThread( nDataKey, rLinkedURL );
    return pNewThread;
}

SwAsyncRetrieveInputStreamThread::SwAsyncRetrieveInputStreamThread(
                            const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                            const String& rLinkedURL )
    : ObservableThread(),
      mnDataKey( nDataKey ),
      mrLinkedURL( rLinkedURL )
{
}

SwAsyncRetrieveInputStreamThread::~SwAsyncRetrieveInputStreamThread()
{
}

void SwAsyncRetrieveInputStreamThread::threadFunction()
{
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > xProps( 1 );
    xProps[0].Name = OUString("URL");
    xProps[0].Value <<= OUString( mrLinkedURL );
    comphelper::MediaDescriptor aMedium( xProps );

    aMedium.addInputStream();

    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream;
    aMedium[comphelper::MediaDescriptor::PROP_INPUTSTREAM()] >>= xInputStream;
    if ( !xInputStream.is() )
    {
        com::sun::star::uno::Reference<com::sun::star::io::XStream> xStream;
        aMedium[comphelper::MediaDescriptor::PROP_STREAM()] >>= xStream;
        if ( xStream.is() )
        {
            xInputStream = xStream->getInputStream();
        }
    }

    SwRetrievedInputStreamDataManager::GetManager().PushData( mnDataKey,
                                                              xInputStream,
                                                              aMedium.isStreamReadOnly() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
