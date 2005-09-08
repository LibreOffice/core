/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: weakeventlistener.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:53:42 $
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

#ifndef COMPHELPER_WEAKEVENTLISTENER_HXX
#include <comphelper/weakeventlistener.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


//.........................................................................
namespace comphelper
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    //=====================================================================
    //= OWeakListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OWeakListenerAdapterBase::~OWeakListenerAdapterBase()
    {
    }

    //=====================================================================
    //= OWeakEventListenerAdapter
    //=====================================================================
    //---------------------------------------------------------------------
    OWeakEventListenerAdapter::OWeakEventListenerAdapter( Reference< XWeak > _rxListener, Reference< XComponent > _rxBroadcaster )
        :OWeakEventListenerAdapter_Base( _rxListener, _rxBroadcaster )
    {
        // add ourself as listener to the broadcaster
        OSL_ENSURE( _rxBroadcaster.is(), "OWeakEventListenerAdapter::OWeakEventListenerAdapter: invalid broadcaster!" );
        if ( _rxBroadcaster.is() )
        {
            osl_incrementInterlockedCount( &m_refCount );
            {
                _rxBroadcaster->addEventListener( this );
            }
            osl_decrementInterlockedCount( &m_refCount );
            OSL_ENSURE( m_refCount > 0, "OWeakEventListenerAdapter::OWeakEventListenerAdapter: oops - not to be used with implementations which hold their listeners weak!" );
                // the one and only reason for this adapter class (A) is to add as listener to a component (C) which
                // holds its listeners hard, and forward all calls then to another listener (L) which is
                // held weak by A.
                // Now if C holds listeners weak, then we do not need A, we can add L directly to C.
        }

        OSL_ENSURE( getListener().is(), "OWeakEventListenerAdapter::OWeakEventListenerAdapter: invalid listener (does not support the XEventListener interface)!" );
    }

    //---------------------------------------------------------------------
    void SAL_CALL OWeakEventListenerAdapter::disposing( )
    {
        Reference< XComponent > xBroadcaster( getBroadcaster( ), UNO_QUERY );
        OSL_ENSURE( xBroadcaster.is(), "OWeakEventListenerAdapter::disposing: broadcaster is invalid in the meantime! How this?" );
        if ( xBroadcaster.is() )
        {
            xBroadcaster->removeEventListener( this );
        }

        resetListener();
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


