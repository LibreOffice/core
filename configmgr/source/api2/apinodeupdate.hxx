/*************************************************************************
 *
 *  $RCSfile: apinodeupdate.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 16:10:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_API_NODEUPDATE_HXX_
#define CONFIGMGR_API_NODEUPDATE_HXX_

#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#include "apinodeaccess.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

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
        class ElementTree;

        class NodeUpdate;
    }
    namespace configapi
    {
        class SetElement;

        typedef uno::XInterface UnoInterface;
        typedef uno::Any UnoAny;

    // API object implementation wrappers
        // these objects just provide the pieces needed to navigate and manipulate trees and nodes

        // Updating access for Group Nodes
        class NodeGroupAccess : public NodeGroupInfoAccess
        {
        public:
            typedef configuration::GroupUpdater     NodeUpdater;
            typedef configuration::GroupDefaulter   NodeDefaulter;
            NodeUpdater     getNodeUpdater(memory::Accessor const& _aAccessor);
            NodeDefaulter   getNodeDefaulter(memory::Accessor const& _aAccessor);

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
            NodeDefaulter     getNodeDefaulter(memory::Accessor const& _aAccessor);
        };

        // Updating access for Set Nodes containing whole trees
        class NodeTreeSetAccess : public NodeSetAccess
        {
        public:
            typedef configuration::TreeSetUpdater     NodeUpdater;
            NodeUpdater     getNodeUpdater(memory::Accessor const& _aAccessor);

            configuration::SetElementFactory    getElementFactory(memory::Accessor const& _aAccessor);
        };

        // Updating access for Set Nodes containing simple values
        class NodeValueSetAccess : public NodeSetAccess
        {
        public:
            typedef configuration::ValueSetUpdater     NodeUpdater;
            NodeUpdater     getNodeUpdater(memory::Accessor const& _aAccessor);
        };

        /// informs a <type>SetElement</type> that it should now link to the given SetElement
        void attachSetElement(NodeTreeSetAccess& aSet, SetElement& aElement);

        /// informs a <type>SetElement</type> that it should now link to the given SetElement
        bool attachSetElement(NodeTreeSetAccess& aSet, configuration::ElementTree const& aElementTree);

        /// informs a <type>SetElement</type> that it should now unlink from its owning SetElement
        void detachSetElement(SetElement& aElement);

        /// informs a <type>SetElement</type> that it should now unlink from its owning SetElement
        bool detachSetElement(Factory& rFactory, configuration::ElementRef const& aElementTree);

    // Guarding and locking implementations
        /// guards a NodeGroupAccess, or NodeSetAccess; provides an object (write)/provider(read) lock; ensures object was not disposed
        class UpdateGuardImpl : Noncopyable
        {
            memory::Accessor                m_aDataAccess;
            osl::MutexGuard             m_aViewLock;
            NodeAccess&                 m_rNode;
        public:
            UpdateGuardImpl(NodeGroupAccess& rNode) throw();
            UpdateGuardImpl(NodeSetAccess& rNode) throw();
            ~UpdateGuardImpl() throw ();
        public:
            NodeAccess& get()        const { return m_rNode; }

            void downgrade() {  }

            memory::Accessor const& accessor() const { return m_aDataAccess; }
        };

    // Thin Wrappers around NodeAccesses: Provide guarding and convenient access
        /// wraps a NodeAccess; provides an object (write)/provider(read) lock, ensures object was not disposed
        template <class Access>
        class GuardedNodeUpdate
        {
            UpdateGuardImpl m_aImpl;
        public:
            GuardedNodeUpdate(Access& rNode) throw() : m_aImpl(rNode) {}
        public:
            Access& get()        const { return static_cast<Access&>(m_aImpl.get()); }

            configuration::Tree     getTree() const;
            configuration::NodeRef  getNode() const;

            typedef typename Access::NodeUpdater    Updater;
            typedef typename Access::NodeDefaulter  Defaulter;

            Updater    getNodeUpdater() const;
            Defaulter  getNodeDefaulter() const;

            memory::Accessor const& getDataAccessor() const { return m_aImpl.accessor(); }

            void clearForBroadcast() { m_aImpl.downgrade(); }
        };

        template <class Access>
        configuration::Tree GuardedNodeUpdate<Access>::getTree() const
        {
            return get().getTree(this->getDataAccessor());
        }

        template <class Access>
        configuration::NodeRef GuardedNodeUpdate<Access>::getNode() const
        {
            return get().getNodeRef();
        }

        template <class Access>
        #if defined(_MSC_VER) && (_MSC_VER > 1300 )
        typename GuardedNodeUpdate<Access>::Updater GuardedNodeUpdate<Access>::getNodeUpdater() const
        #else
        GuardedNodeUpdate<Access>::Updater GuardedNodeUpdate<Access>::getNodeUpdater() const
        #endif
        {
            return get().getNodeUpdater(this->getDataAccessor());
        }

        template <class Access>
        #if defined(_MSC_VER) && (_MSC_VER > 1300 )
        typename GuardedNodeUpdate<Access>::Defaulter GuardedNodeUpdate<Access>::getNodeDefaulter() const
        #else
        GuardedNodeUpdate<Access>::Defaulter GuardedNodeUpdate<Access>::getNodeDefaulter() const
        #endif
        {
            return get().getNodeDefaulter(this->getDataAccessor());
        }

        /// wraps a NodeGroupAccess; provides an object (write) lock, ensures object was not disposed
        typedef GuardedNodeUpdate<NodeGroupAccess>  GuardedGroupUpdateAccess;
        typedef GuardedNodeUpdate<NodeSetAccess>    GuardedSetUpdateAccess;
        typedef GuardedNodeUpdate<NodeTreeSetAccess>    GuardedTreeSetUpdateAccess;
        typedef GuardedNodeUpdate<NodeValueSetAccess>   GuardedValueSetUpdateAccess;
    }
}

#endif // CONFIGMGR_API_NODEUPDATE_HXX_
