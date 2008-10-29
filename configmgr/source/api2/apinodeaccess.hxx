/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apinodeaccess.hxx,v $
 * $Revision: 1.13 $
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

#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#define CONFIGMGR_API_NODEACCESS_HXX_

#include "sal/config.h"

#include "boost/utility.hpp"

#include "datalock.hxx"
#include "utility.hxx"
#include "noderef.hxx"

namespace osl { class Mutex; }

namespace configmgr
{
    namespace configuration
    {
        class AnyNodeRef;
        class NodeRef;
        class Tree;
    }
    namespace configapi
    {
        class Factory;
        class Notifier;
        class SetElement;

        class ApiTreeImpl;

        namespace uno = com::sun::star::uno;

        // API object implementation wrappers
        // these objects just provide the pieces needed to navigate and manipulate trees and nodes

        // The common part of all nodes, provides all you need to read and listen
        class NodeAccess : private boost::noncopyable
        {
        public:
            virtual ~NodeAccess();

        // model access
            configuration::NodeRef  getNodeRef() const;
            rtl::Reference< configuration::Tree > getTreeRef() const;
            rtl::Reference< configuration::Tree > getTree() const;

        // self-locked methods for dispose handling
            void checkAlive() const;
            void disposeNode();

        // api object handling
            uno::XInterface*            getUnoInstance() const
            { return doGetUnoInstance(); }
            Factory&                getFactory() const;
            Notifier                getNotifier() const;

        protected:
            virtual configuration::NodeRef  doGetNode() const = 0;
            virtual uno::XInterface*    doGetUnoInstance() const = 0;
            virtual ApiTreeImpl&    getApiTree() const = 0;
        };

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing node <var>aNode</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        uno::Any    makeElement(configapi::Factory& rFactory, rtl::Reference< configuration::Tree > const& aTree, configuration::AnyNodeRef const& aNode);

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing inner node <var>aNode</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        uno::Any    makeInnerElement(configapi::Factory& rFactory, rtl::Reference< configuration::Tree > const& aTree, configuration::NodeRef const& aNode);

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing set element <var>aElement</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        uno::Any    makeElement(configapi::Factory& rFactory, rtl::Reference< configuration::ElementTree > const& aTree);


        // Info interfaces for Group Nodes
        class NodeGroupInfoAccess : public NodeAccess
        {
        public:
            // currently only used for tagging group nodes
        };

        // Info interfaces for Set Nodes
        class NodeSetInfoAccess : public NodeAccess
        {
            friend class SetElement;
        public:
            rtl::Reference< configuration::Template > getElementInfo() const;
        };

        /** extracts a <type scope='configmgr::configuration'>ElementTree</type> from a <type scope='com::sun::star::uno'>Any</type>
            which must contain an object which wraps an instance of the template available in <var>aTemplate</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to resolve inner nodes (which may suppose that the object was created using the same factory)</p>
            <p> returns an empty tree if <var>aElement</var> is empty.</p>
            <p> May throw exceptions if the type doesn't match the template.</p>
        */
        rtl::Reference< configuration::ElementTree > extractElementTree(Factory& rFactory, uno::Any const& aElement, rtl::Reference< configuration::Template > const& aTemplate );

        /// wraps a NodeAccess; provides both object and provider (read) locks,
        // ensures object was not disposed
        template <class Access>
        class GuardedNodeData
        {
            UnoApiLock              m_aLock;
            Access & m_rNode;
        public:
            GuardedNodeData(Access& rNode);
        public:
            Access& get()        const { return m_rNode; }

            rtl::Reference< configuration::Tree > getTree() const;
            configuration::NodeRef  getNode() const;
        };

        template <class Access>
        GuardedNodeData<Access>::GuardedNodeData(Access& rNode)
        : m_rNode(rNode)
        {
            rNode.checkAlive();
        }

        template <class Access>
        rtl::Reference< configuration::Tree > GuardedNodeData<Access>::getTree() const
        {
            return m_rNode.getTree();
        }

        template <class Access>
        configuration::NodeRef GuardedNodeData<Access>::getNode() const
        {
            return m_rNode.getNodeRef();
        }
    }
}

#endif // CONFIGMGR_API_NODEACCESS_HXX_
