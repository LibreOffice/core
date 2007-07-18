/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: retrieveinputstreamconsumer.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:31:53 $
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

#ifndef _RETRIEVEINPUTSTREAMCONSUMER_HXX
#include <retrieveinputstreamconsumer.hxx>
#endif

#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _RETRIEVEINPUTSTREAM_HXX
#include <retrieveinputstream.hxx>
#endif
#ifndef _SWTHREADMANAGER_HXX
#include <swthreadmanager.hxx>
#endif

/** class to provide creation of a thread to retrieve an input stream given by
    an URL and to consume the retrieved input stream.

    OD 2007-01-29 #i73788#

    @author OD
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

