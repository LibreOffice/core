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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
