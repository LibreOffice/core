/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "precompiled_sw.hxx"
#include <retrieveinputstream.hxx>
#include <comphelper/mediadescriptor.hxx>
#ifndef _COM_SUN_STAR_IO_XSTREAM_HXX_
#include <com/sun/star/io/XStream.hpp>
#endif

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
    xProps[0].Name = ::rtl::OUString::createFromAscii( "URL" );
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
