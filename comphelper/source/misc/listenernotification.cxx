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

#include <comphelper/listenernotification.hxx>

#include <com/sun/star/lang/DisposedException.hpp>


namespace comphelper
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    OListenerContainer::OListenerContainer( ::osl::Mutex& _rMutex )
        :m_aListeners( _rMutex )
    {
    }

    OListenerContainer::~OListenerContainer() {}


    void OListenerContainer::impl_addListener( const Reference< XEventListener >& _rxListener )
    {
        OSL_PRECOND( _rxListener.is(), "OListenerContainer::impl_addListener: a NULL listener?!" );
        if ( _rxListener.is() )
            m_aListeners.addInterface( _rxListener );
    }


    void OListenerContainer::impl_removeListener( const Reference< XEventListener >& _rxListener )
    {
#if OSL_DEBUG_LEVEL > 0
        ::cppu::OInterfaceIteratorHelper aIter( m_aListeners );
        bool bFound = false;
        while ( aIter.hasMoreElements() && !bFound )
        {
            bFound = ( Reference< XInterface >( aIter.next() ) == _rxListener );
        }
        OSL_ENSURE( bFound, "OListenerContainer::impl_removeListener: sure your listener handling is correct? The given listener is not registered!" );
#endif
        m_aListeners.removeInterface( _rxListener );
    }


    void OListenerContainer::disposing( const EventObject& _rEventSource )
    {
        m_aListeners.disposeAndClear( _rEventSource );
    }


    void OListenerContainer::clear()
    {
        m_aListeners.clear();
    }


    bool OListenerContainer::impl_notify( const EventObject& _rEvent )
    {
        ::cppu::OInterfaceIteratorHelper aIter( m_aListeners );
        bool bCancelled = false;
        while ( aIter.hasMoreElements() && !bCancelled )
        {
            Reference< XEventListener > xListener( static_cast< XEventListener* >( aIter.next() ) );
            if ( !xListener.is() )
                continue;

            try
            {
                bCancelled = !implNotify( xListener, _rEvent );
            }
            catch( const DisposedException& e )
            {
                // DisposedExceptions from the listener might indicate a
                // broken connection to a different environment.

                OSL_ENSURE( e.Context.is(), "OListenerContainer::impl_notify: caught dispose exception with empty Context field" );

                // If the exception stems from the listener then remove it
                // from the list of listeners.  If the Context field of the
                // exception is empty this is interpreted to indicate the
                // listener as well.
                if ( e.Context == xListener || !e.Context.is() )
                    aIter.remove();
            }
        }

        return !bCancelled;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
