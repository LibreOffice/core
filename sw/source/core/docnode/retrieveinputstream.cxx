/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: retrieveinputstream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:31:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_sw.hxx"

#ifndef _RETRIEVEINPUTSTREAM_HXX
#include <retrieveinputstream.hxx>
#endif
#ifndef _COMPHELPER_MEDIADESCRIPTOR_HXX_
#include <comphelper/mediadescriptor.hxx>
#endif
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
