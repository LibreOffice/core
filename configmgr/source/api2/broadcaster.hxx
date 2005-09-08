/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: broadcaster.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:10:22 $
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

#ifndef CONFIGMGR_API_BROADCASTER_HXX_
#define CONFIGMGR_API_BROADCASTER_HXX_

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVETOEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyVetoException.hpp>
#endif

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
        using configuration::NodeChange;
        using configuration::NodeChanges;
        using configuration::NodeChangeInformation;
        using configuration::NodeChangesInformation;

        class Notifier;

        namespace css = ::com::sun::star;
// ---------------------------------------------------------------------------------------------------

        /// broadcasts events for changes to a single config node or several sibling nodes
        class Broadcaster
        {
        public:
            /// construct a broadcaster
            Broadcaster(Notifier const& aNotifier, NodeChange const& aChange, bool bLocal);
            Broadcaster(Notifier const& aNotifier, NodeChanges const& aChanges, bool bLocal);
            Broadcaster(Notifier const& aNotifier, NodeChangeInformation const& aChange, bool bLocal);
            Broadcaster(Notifier const& aNotifier, NodeChangesInformation const& aChanges, bool bLocal);
            Broadcaster(Broadcaster const& aOther);
            ~Broadcaster();

            /// give all property veto listeners on the affected node a chance to veto
            void queryConstraints(NodeChange const& aChange) throw(css::beans::PropertyVetoException);
            /// give all property veto listeners on any of the affected nodes a chance to veto
            void queryConstraints(NodeChanges const& aChanges, bool bSingleBase = true) throw(css::beans::PropertyVetoException);

            /// notify all listeners which are affected by this change
            void notifyListeners(NodeChange const& aChange) throw();
            /// notify all listeners which are affected by any of these changes (potentially from many different bases)
            void notifyListeners(NodeChanges const& aChanges, bool bSingleBase) throw();
            /// notify all listeners which are affected by this change
            void notifyListeners(NodeChangeInformation const& aChange) throw();
            /// notify all listeners which are affected by any of these changes (potentially from many different bases)
            void notifyListeners(NodeChangesInformation const& aChanges, bool bSingleBase = false) throw();

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
