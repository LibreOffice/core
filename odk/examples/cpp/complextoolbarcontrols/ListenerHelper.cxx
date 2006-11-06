/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ListenerHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 15:04:41 $
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
