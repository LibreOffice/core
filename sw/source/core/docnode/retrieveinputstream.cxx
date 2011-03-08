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
#include "precompiled_sw.hxx"
#include <retrieveinputstream.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <com/sun/star/io/XStream.hpp>

/** class for a thread to retrieve an input stream given by an URL

    OD 2007-01-29 #i73788#

    @author OD
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
    xProps[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"));
    xProps[0].Value <<= ::rtl::OUString( mrLinkedURL );
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
