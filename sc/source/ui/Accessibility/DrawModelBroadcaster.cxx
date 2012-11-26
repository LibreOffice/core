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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#include "DrawModelBroadcaster.hxx"
#include <svx/svdmodel.hxx>
#include <svx/unomod.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;

ScDrawModelBroadcaster::ScDrawModelBroadcaster( SdrModel *pDrawModel ) :
    maEventListeners( maListenerMutex ),
    mpDrawModel( pDrawModel )
{
    if (mpDrawModel)
        StartListening( *mpDrawModel );
}

ScDrawModelBroadcaster::~ScDrawModelBroadcaster()
{
    if (mpDrawModel)
        EndListening( *mpDrawModel );
}

void SAL_CALL ScDrawModelBroadcaster::addEventListener( const uno::Reference< document::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    maEventListeners.addInterface( xListener );
}

void SAL_CALL ScDrawModelBroadcaster::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    maEventListeners.removeInterface( xListener );
}

void ScDrawModelBroadcaster::Notify( SfxBroadcaster&,
        const SfxHint& rHint )
{
    const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if( !pSdrHint )
        return;

    document::EventObject aEvent;
    if( !SvxUnoDrawMSFactory::createEvent( mpDrawModel, pSdrHint, aEvent ) )
        return;

    ::cppu::OInterfaceIteratorHelper aIter( maEventListeners );
    while( aIter.hasMoreElements() )
    {
        uno::Reference < document::XEventListener > xListener( aIter.next(), uno::UNO_QUERY );
        try
        {
            xListener->notifyEvent( aEvent );
        }
        catch( uno::RuntimeException& r )
        {
            (void) r;
#if OSL_DEBUG_LEVEL > 1
            ByteString aError( "Runtime exception caught while notifying shape.:\n" );
            aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aError.GetBuffer() );
#endif
        }
    }
}
