/*************************************************************************
 *
 *  $RCSfile: apinodeaccess.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:34:32 $
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

#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#define CONFIGMGR_API_NODEACCESS_HXX_

#include "apitypes.hxx"
#include "synchronize.hxx"

namespace osl { class Mutex; }

namespace configmgr
{
    namespace configuration
    {
        class Name;
        class NodeRef;
        class Tree;

        class SetElementInfo;
        class ElementTree;
    }
    namespace configapi
    {
        class Factory;
        class Notifier;

        class ApiTreeImpl;

        typedef uno::XInterface UnoInterface;
        typedef uno::Any UnoAny;

    // API object implementation wrappers
        // these objects just provide the pieces needed to navigate and manipulate trees and nodes

        // The common part of all nodes, provides all you need to read and listen
        class NodeAccess : NotCopyable
        {
        public:
            virtual ~NodeAccess();

        // model access
            configuration::NodeRef  getNode() const;
            configuration::Tree     getTree() const;

        // self-locked methods for dispose handling
            void checkAlive() const;
            void disposeNode();

        // api object handling
            UnoInterface*           getUnoInstance() const
            { return doGetUnoInstance(); }
            Factory&                getFactory() const;
            Notifier                getNotifier() const;

        // locking support
            ISynchronizedData const*getDataLock() const;
            ISynchronizedData const*getProviderLock() const;
            osl::Mutex&             getApiLock();

        protected:
            virtual configuration::NodeRef  doGetNode() const = 0;
            virtual UnoInterface*   doGetUnoInstance() const = 0;
            virtual ApiTreeImpl&    getApiTree() const = 0;
        };

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing node <var>aNode</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        UnoAny  makeElement(configapi::Factory& rFactory, configuration::Tree const& aTree, configuration::NodeRef const& aNode);

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing set element <var>aElement</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        UnoAny  makeElement(configapi::Factory& rFactory, configuration::ElementTree const& aTree);

        /** extracts a <type scope='configmgr::configuration'>Tree</type> from a <type scope='com::sun::star::uno'>Any</type>
            which must contain an object which wraps an instance of the template available in <var>aElementInfo</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to resolve inner nodes (which may suppose that the object was created using the same factory)</p>
            <p> returns an empty <type scope='configmgr::configuration'>Tree</type> if <var>aElement</var> is empty.</p>
            <p> May throw exceptions if the type doesn't match the template.</p>
        */
        configuration::ElementTree extractElementTree(configapi::Factory& rFactory, UnoAny const& aElement, configuration::SetElementInfo const& aElementInfo );

        // Info interfaces for Group Nodes
        class NodeGroupInfoAccess : public NodeAccess
        {
        public:
            // currently only used for tagging group nodes
        };

        // Info interfaces for Set Nodes
        class NodeSetInfoAccess : public NodeAccess
        {
        public:
            configuration::SetElementInfo getElementInfo() const;
        };

    // Guarding and locking implementations
        /// guards a NodeAccess; provides an object (read) lock, ensures object was not disposed
        class NodeReadGuardImpl : NotCopyable
        {
            OReadSynchronized   m_aLock;
            NodeAccess&         m_rNode;
        public:
            NodeReadGuardImpl(NodeAccess& rNode) throw();
            ~NodeReadGuardImpl() throw ();
        public:
            NodeAccess& get() const { return m_rNode; }
        };

    // Thin Wrappers around NodeAccesses: Provide guarding and convenient access
        /// wraps a NodeAccess; provides an object (read) lock, ensures object was not disposed
        template <class Access>
        class GuardedNode
        {
            NodeReadGuardImpl   m_aImpl;
        public:
            GuardedNode(Access& rNode) : m_aImpl(rNode) {}
        public:
            Access& get()        const { return static_cast<Access&>(m_aImpl.get()); }

            Access& operator *() const  { return  get(); }
            Access* operator->() const  { return &get(); }
        };
        typedef GuardedNode<NodeAccess> GuardedNodeAccess;

        /// wraps a NodeAccess; provides both object and provider (read) locks, ensures object was not disposed
        template <class Access>
        class GuardedNodeData
        {
            OReadSynchronized   m_aProviderLock;
            NodeReadGuardImpl   m_aImpl;
        public:
            GuardedNodeData(Access& rNode);
        public:
            Access& get()        const { return static_cast<Access&>(m_aImpl.get()); }

            Access& operator *() const  { return  get(); }
            Access* operator->() const  { return &get(); }
        };
        typedef GuardedNodeData<NodeAccess> GuardedNodeDataAccess;

        template <class Access>
        GuardedNodeData<Access>::GuardedNodeData(Access& rNode)
        : m_aProviderLock(rNode.getProviderLock())
        , m_aImpl(rNode)
        {
        }

    }
}

#endif // CONFIGMGR_API_NODEACCESS_HXX_
