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

#include <comphelper/weakeventlistener.hxx>
#include <osl/diagnose.h>


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
            osl_atomic_increment( &m_refCount );
            {
                _rxBroadcaster->addEventListener( this );
            }
            osl_atomic_decrement( &m_refCount );
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
