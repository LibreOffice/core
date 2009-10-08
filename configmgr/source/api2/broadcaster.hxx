/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: broadcaster.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_API_BROADCASTER_HXX_
#define CONFIGMGR_API_BROADCASTER_HXX_

#include <com/sun/star/beans/PropertyVetoException.hpp>

#include <vos/ref.hxx>

namespace configmgr
{
    namespace configuration
    {
// ---------------------------------------------------------------------------------------------------
        class NodeChange;
        class NodeChanges;
        class NodeChangeInformation;
        class NodeChangesInformation;
    }
// ---------------------------------------------------------------------------------------------------
    namespace configapi
    {
        class Notifier;

        namespace css = ::com::sun::star;
// ---------------------------------------------------------------------------------------------------

        /// broadcasts events for changes to a single config node or several sibling nodes
        class Broadcaster
        {
        public:
            /// construct a broadcaster
            Broadcaster(Notifier const& aNotifier, configuration::NodeChange const& aChange, bool bLocal);
            Broadcaster(Notifier const& aNotifier, configuration::NodeChanges const& aChanges, bool bLocal);
            Broadcaster(Notifier const& aNotifier, configuration::NodeChangeInformation const& aChange, bool bLocal);
            Broadcaster(Notifier const& aNotifier, configuration::NodeChangesInformation const& aChanges, bool bLocal);
            Broadcaster(Broadcaster const& aOther);
            ~Broadcaster();

            /// give all property veto listeners on the affected node a chance to veto
            void queryConstraints(configuration::NodeChange const& aChange) throw(css::beans::PropertyVetoException);
            /// give all property veto listeners on any of the affected nodes a chance to veto
            void queryConstraints(configuration::NodeChanges const& aChanges, bool bSingleBase = true) throw(css::beans::PropertyVetoException);

            /// notify all listeners which are affected by this change
            void notifyListeners(configuration::NodeChange const& aChange) throw();
            /// notify all listeners which are affected by any of these changes (potentially from many different bases)
            void notifyListeners(configuration::NodeChanges const& aChanges, bool bSingleBase) throw();
            /// notify all listeners which are affected by this change
            void notifyListeners(configuration::NodeChangeInformation const& aChange) throw();
            /// notify all listeners which are affected by any of these changes (potentially from many different bases)
            void notifyListeners(configuration::NodeChangesInformation const& aChanges, bool bSingleBase = false) throw();

            class Impl;
        private:
            vos::ORef<Impl> m_pImpl;
        private:
            void operator=(Broadcaster const& aOther);
        };

// ---------------------------------------------------------------------------------------------------

    }
}

#endif // CONFIGMGR_API_BROADCASTER_HXX_
