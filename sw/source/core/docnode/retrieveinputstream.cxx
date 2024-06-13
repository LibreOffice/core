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

#include <comphelper/propertyvalue.hxx>
#include <unotools/mediadescriptor.hxx>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <utility>

/* class for a thread to retrieve an input stream given by a URL

    #i73788#
*/
::rtl::Reference< ObservableThread > SwAsyncRetrieveInputStreamThread::createThread(
                        const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                        const OUString& rLinkedURL, const OUString& rReferer )
{
    SwAsyncRetrieveInputStreamThread* pNewThread =
        new SwAsyncRetrieveInputStreamThread( nDataKey, rLinkedURL, rReferer );
    return pNewThread;
}

SwAsyncRetrieveInputStreamThread::SwAsyncRetrieveInputStreamThread(
                            const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                            OUString aLinkedURL,
                            OUString aReferer )
    : mnDataKey( nDataKey ),
      mrLinkedURL(std::move( aLinkedURL )),
      mrReferer(std::move( aReferer ))
{
}

SwAsyncRetrieveInputStreamThread::~SwAsyncRetrieveInputStreamThread()
{
}

void SwAsyncRetrieveInputStreamThread::threadFunction()
{
    osl_setThreadName("SwAsyncRetrieveInputStreamThread");

    css::uno::Sequence < css::beans::PropertyValue > xProps{
        comphelper::makePropertyValue(u"URL"_ustr, mrLinkedURL),
        comphelper::makePropertyValue(u"Referer"_ustr, mrReferer)
    };
    utl::MediaDescriptor aMedium( xProps );

    aMedium.addInputStream();

    css::uno::Reference<css::io::XInputStream> xInputStream;
    aMedium[utl::MediaDescriptor::PROP_INPUTSTREAM] >>= xInputStream;
    if ( !xInputStream.is() )
    {
        css::uno::Reference<css::io::XStream> xStream;
        aMedium[utl::MediaDescriptor::PROP_STREAM] >>= xStream;
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
