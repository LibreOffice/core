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




#include "ListenerHelper.h"

using com::sun::star::frame::XFrame;
using com::sun::star::frame::XDispatch;
using com::sun::star::frame::XStatusListener;
using com::sun::star::lang::XEventListener;
using com::sun::star::lang::EventObject;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::frame::FeatureStateEvent;

static AllListeners aListeners;

void ListenerHelper::AddListener(
    const Reference < XFrame >& xFrame,
    const Reference < XStatusListener > xControl,
    const ::rtl::OUString& aCommand )
{
    sal_uInt32 i=0;
    sal_uInt32 nSize = aListeners.size();
    for ( i=0; i<nSize; i++ )
        if ( aListeners[i].xFrame == xFrame )
            break;

    OSL_ENSURE( i<nSize, "No dispatch found for this listener!" );
    aListeners[i].aContainer[aCommand].push_back( xControl );
}

void ListenerHelper::RemoveListener(
    const Reference < XFrame >& xFrame,
    const Reference < XStatusListener > xControl,
    const ::rtl::OUString& aCommand )
{
    sal_uInt32 nSize = aListeners.size();
    for ( sal_uInt32 i=0; i<nSize; i++ )
    {
        if ( aListeners[i].xFrame == xFrame )
        {
            StatusListeners& aL = aListeners[i].aContainer[aCommand];
            StatusListeners::iterator aIter = aL.begin();
            while ( aIter != aL.end() )
            {
                if ( (*aIter) == xControl )
                {
                    aL.erase( aIter );
                    break;
                }

                aIter++;
            }
        }
    }
}

void ListenerHelper::Notify(
        const Reference < XFrame >& xFrame,
        const ::rtl::OUString& aCommand,
        FeatureStateEvent& rEvent )
{
    sal_uInt32 nSize = aListeners.size();
    for ( sal_uInt32 i=0; i<nSize; i++ )
    {
        if ( aListeners[i].xFrame == xFrame )
        {
            rEvent.Source = aListeners[i].xDispatch;
            StatusListeners& aL = aListeners[i].aContainer[aCommand];
            StatusListeners::iterator aIter = aL.begin();
            while ( aIter != aL.end() )
            {
                (*aIter)->statusChanged( rEvent );
                aIter++;
            }
        }
    }
}

com::sun::star::uno::Reference < XDispatch > ListenerHelper::GetDispatch(
        const Reference < XFrame >& xFrame,
        const ::rtl::OUString& aCommand )
{
    sal_uInt32 nSize = aListeners.size();
    for ( sal_uInt32 i=0; i<nSize; i++ )
    {
        if ( aListeners[i].xFrame == xFrame )
            return aListeners[i].xDispatch;
    }

    return Reference < XDispatch >();
}

void ListenerHelper::AddDispatch(
        const Reference < XDispatch > xDispatch,
        const Reference < XFrame >& xFrame,
        const ::rtl::OUString& aCommand )
{
    ListenerItem aItem;
    aItem.xFrame = xFrame;
    aItem.xDispatch = xDispatch;
    aListeners.push_back( aItem );
    xFrame->addEventListener( new ListenerItemEventListener( xFrame ) );
}

void SAL_CALL ListenerItemEventListener::disposing( const EventObject& aEvent) throw (RuntimeException)
{
    AllListeners::iterator aIter = aListeners.begin();
    while ( aIter != aListeners.end() )
    {
        if ( (*aIter).xFrame == mxFrame )
        {
            aListeners.erase( aIter );
            break;
        }

        aIter++;
    }
}
