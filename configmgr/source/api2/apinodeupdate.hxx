/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinodeupdate.hxx,v $
 * $Revision: 1.11 $
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

#ifndef CONFIGMGR_API_NODEUPDATE_HXX_
#define CONFIGMGR_API_NODEUPDATE_HXX_

#include "sal/config.h"

#include "boost/utility.hpp"

#include "apinodeaccess.hxx"
#include "utility.hxx"

namespace configmgr
{
    namespace configuration
    {
        class GroupUpdater;
        class GroupDefaulter;
        class TreeSetUpdater;
        class ValueSetUpdater;
        class SetDefaulter;
        class SetElementFactory;

        class NodeUpdate;
    }
    namespace configapi
    {
        class SetElement;

    // API object implementation wrappers
        // these objects just provide the pieces needed to navigate and manipulate trees and nodes

        // Updating access for Group Nodes
        class NodeGroupAccess : public NodeGroupInfoAccess
        {
        public:
            typedef configuration::GroupUpdater     NodeUpdater;
            typedef configuration::GroupDefaulter   NodeDefaulter;
            NodeUpdater     getNodeUpdater();
            NodeDefaulter   getNodeDefaulter();

            /** ensures that the default data for a group is loaded (if possible)
                <p>Must be called outside of any locks !</p>
            */
            friend NodeGroupAccess& withDefaultData(NodeGroupAccess& aGroup);
        };

        // Updating access for Set Nodes
        class NodeSetAccess : public NodeSetInfoAccess
        {
        public:
            typedef struct SetUpdater_PlaceHolder   NodeUpdater;
            typedef configuration::SetDefaulter     NodeDefaulter;
            NodeDefaulter     getNodeDefaulter();
        };

        // Updating access for Set Nodes containing whole trees
        class NodeTreeSetAccess : public NodeSetAccess
        {
        public:
            typedef configuration::TreeSetUpdater     NodeUpdater;
            NodeUpdater     getNodeUpdater();

            configuration::SetElementFactory    getElementFactory();
        };

        // Updating access for Set Nodes containing simple values
        class NodeValueSetAccess : public NodeSetAccess
        {
        public:
            typedef configuration::ValueSetUpdater     NodeUpdater;
            NodeUpdater     getNodeUpdater();
        };

        /// informs a <type>SetElement</type> that it should now link to the given SetElement
        void attachSetElement(NodeTreeSetAccess& aSet, SetElement& aElement);

        /// informs a <type>SetElement</type> that it should now link to the given SetElement
        bool attachSetElement(NodeTreeSetAccess& aSet, rtl::Reference< configuration::ElementTree > const& aElementTree);

        /// informs a <type>SetElement</type> that it should now unlink from its owning SetElement
        void detachSetElement(SetElement& aElement);

        /// informs a <type>SetElement</type> that it should now unlink from its owning SetElement
        bool detachSetElement(Factory& rFactory, rtl::Reference< configuration::ElementTree > const& aElementTree);

        /// Guarding and locking implementations
        /// guards a NodeGroupAccess, or NodeSetAccess; provides an object (write)/provider(read) lock; ensures object was not disposed
        class UpdateGuardImpl : private boost::noncopyable
        {
            NodeAccess&      m_rNode;
        public:
            UpdateGuardImpl(NodeGroupAccess& rNode);
            UpdateGuardImpl(NodeSetAccess& rNode);
            ~UpdateGuardImpl() throw ();
        public:
            NodeAccess& get()        const { return m_rNode; }

            void downgrade() {  }
        };

    // Thin Wrappers around NodeAccesses: Provide guarding and convenient access
        /// wraps a NodeAccess; provides an object (write)/provider(read) lock, ensures object was not disposed
        template <class Access>
        class GuardedNodeUpdate
        {
            UnoApiLock      m_aLock;
            UpdateGuardImpl m_aImpl;
        public:
            GuardedNodeUpdate(Access& rNode) : m_aImpl(rNode) {}
        public:
            Access& get()        const { return static_cast<Access&>(m_aImpl.get()); }

            rtl::Reference< configuration::Tree > getTree() const;
            configuration::NodeRef  getNode() const;

            typename Access::NodeUpdater    getNodeUpdater() const;
            typename Access::NodeDefaulter  getNodeDefaulter() const;

            void clearForBroadcast() { m_aImpl.downgrade(); }
        };

        template <class Access>
        rtl::Reference< configuration::Tree > GuardedNodeUpdate<Access>::getTree() const
        {
            return get().getTree();
        }

        template <class Access>
        configuration::NodeRef GuardedNodeUpdate<Access>::getNode() const
        {
            return get().getNodeRef();
        }

        template <class Access>
        typename Access::NodeUpdater GuardedNodeUpdate<Access>::getNodeUpdater() const
        {
            return get().getNodeUpdater();
        }

        template <class Access>
        typename Access::NodeDefaulter GuardedNodeUpdate<Access>::getNodeDefaulter() const
        {
            return get().getNodeDefaulter();
        }
    }
}

#endif // CONFIGMGR_API_NODEUPDATE_HXX_
