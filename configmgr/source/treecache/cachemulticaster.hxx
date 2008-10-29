/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachemulticaster.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX
#define CONFIGMGR_BACKEND_CACHEMULTICASTER_HXX

#include "sal/config.h"

#include <list>

#include "osl/mutex.hxx"
#include "rtl/ref.hxx"

#include "utility.hxx"

namespace configmgr
{
    class TreeManager;

    namespace backend
    {
        class ComponentRequest;
        class UpdateRequest;
// ---------------------------------------------------------------------------

    /** Interface providing a multicasting service for changes to the cache
        managed by a <type>CacheController</type>
    */
    class CacheChangeMulticaster
    {
    public:
        CacheChangeMulticaster();
        virtual ~CacheChangeMulticaster();

        /** notify a new component to all registered listeners.
            <p> Must be called after the component has been created in the cache.</p>
        */
        void notifyCreated(ComponentRequest const & _aComponentName) SAL_THROW(());

        /** notify changed data to all registered listeners.
            <p> Must be called after the change has been applied to the cache
                and before any subsequent changes to the same component.</p>
        */
        void notifyChanged(UpdateRequest const & _anUpdate) SAL_THROW(());

    // notification support.
        /// register a listener for observing changes to the cached data
        void addListener(rtl::Reference<TreeManager> _xListener) SAL_THROW(());
        /// unregister a listener previously registered
        void removeListener(rtl::Reference<TreeManager> _xListener) SAL_THROW(());
    private:
        std::list< rtl::Reference<TreeManager> > copyListenerList();

        osl::Mutex   m_aMutex;
        std::list< rtl::Reference<TreeManager> > m_aListeners;
    };
// ---------------------------------------------------------------------------
    } // namespace backend

// ---------------------------------------------------------------------------
} // namespace configmgr

#endif

