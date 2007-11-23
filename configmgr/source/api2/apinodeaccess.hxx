/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apinodeaccess.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:03:08 $
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

#ifndef CONFIGMGR_API_NODEACCESS_HXX_
#define CONFIGMGR_API_NODEACCESS_HXX_

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif

namespace osl { class Mutex; }

namespace configmgr
{
    namespace configuration
    {
        class Name;
        class AnyNodeRef;
        class NodeRef;
        class TreeRef;
        class Tree;

        class SetElementInfo;
        class TemplateInfo;
        class ElementRef;
        class ElementTree;
    }
    namespace configapi
    {
        class Factory;
        class Notifier;
        class SetElement;

        class ApiTreeImpl;

        namespace uno = com::sun::star::uno;
        typedef uno::XInterface UnoInterface;
        typedef uno::Any UnoAny;

        // API object implementation wrappers
        // these objects just provide the pieces needed to navigate and manipulate trees and nodes

        // The common part of all nodes, provides all you need to read and listen
        class NodeAccess : Noncopyable
        {
        public:
            virtual ~NodeAccess();

        // model access
            configuration::NodeRef  getNodeRef() const;
            configuration::TreeRef  getTreeRef() const;
            configuration::Tree     getTree() const;

        // self-locked methods for dispose handling
            void checkAlive() const;
            void disposeNode();

        // api object handling
            UnoInterface*           getUnoInstance() const
            { return doGetUnoInstance(); }
            Factory&                getFactory() const;
            Notifier                getNotifier() const;

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
        UnoAny  makeElement(configapi::Factory& rFactory, configuration::Tree const& aTree, configuration::AnyNodeRef const& aNode);

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing inner node <var>aNode</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        UnoAny  makeInnerElement(configapi::Factory& rFactory, configuration::Tree const& aTree, configuration::NodeRef const& aNode);

        /** builds a Uno <type scope='com::sun::star::uno'>Any</type> representing set element <var>aElement</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to create service implementations wrapping inner nodes</p>
            <p> returns VOID if <var>aNode</var> is empty.</p>
        */
        UnoAny  makeElement(configapi::Factory& rFactory, configuration::ElementTree const& aTree);


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
            configuration::SetElementInfo getElementInfo() const;
        };

        /** extracts a <type scope='configmgr::configuration'>ElementTree</type> from a <type scope='com::sun::star::uno'>Any</type>
            which must contain an object which wraps an instance of the template available in <var>aElementInfo</var>.
            <p> Uses the <type scope='configmgr::configapi'>Factory</type> provided
                to resolve inner nodes (which may suppose that the object was created using the same factory)</p>
            <p> returns an empty <type scope='configmgr::configuration'>Tree</type> if <var>aElement</var> is empty.</p>
            <p> May throw exceptions if the type doesn't match the template.</p>
        */
        configuration::ElementRef  extractElementRef (Factory& rFactory, UnoAny const& aElement, configuration::TemplateInfo   const& aElementInfo );
        configuration::ElementTree extractElementTree(Factory& rFactory, UnoAny const& aElement, configuration::SetElementInfo const& aElementInfo );

        /// finds a existing <type>SetElement</type> for a given <type scope='configmgr::configuration'>ElementTree</type>
        SetElement* findSetElement(Factory& rFactory, configuration::ElementRef const& aElementTree);

        // Guarding and locking implementations
        /// guards a NodeAccess; provides an object (read) lock,
        /// ensures object was not disposed

// FIXME: can evaporate this class [ I think ]
        class NodeReadGuardImpl : Noncopyable
        {
            NodeAccess &m_rNode;
        public:
            NodeReadGuardImpl(NodeAccess& rNode)
                : m_rNode(rNode)
                { rNode.checkAlive(); }
            ~NodeReadGuardImpl()
                {}
        public:
            NodeAccess& get() const { return m_rNode; }

            configuration::Tree     getTree() const
                { return m_rNode.getTree(); }
            configuration::NodeRef  getNode() const
                { return m_rNode.getNodeRef(); }
        };

    // Thin Wrappers around NodeAccesses: Provide guarding and convenient access
        /// wraps a NodeAccess; provides an object (read) lock, ensures object was not disposed
        template <class Access>
        class GuardedNode
        {
            NodeReadGuardImpl   m_aViewLock;
        public:
            GuardedNode(Access& rNode) : m_aViewLock(rNode) {}
        public:
            Access& get()        const { return static_cast<Access&>(m_aViewLock.get()); }
        };
        typedef GuardedNode<NodeAccess> GuardedNodeAccess;

        /// wraps a NodeAccess; provides both object and provider (read) locks,
        // ensures object was not disposed
        template <class Access>
        class GuardedNodeData
        {
            UnoApiLock              m_aLock;
            NodeReadGuardImpl       m_aViewLock;
        public:
            GuardedNodeData(Access& rNode);
        public:
            Access& get()        const { return static_cast<Access&>(m_aViewLock.get()); }

            configuration::Tree     getTree() const;
            configuration::NodeRef  getNode() const;
        };
        typedef GuardedNodeData<NodeAccess> GuardedNodeDataAccess;

        template <class Access>
        GuardedNodeData<Access>::GuardedNodeData(Access& rNode)
        : m_aViewLock(rNode)
        {
        }

        template <class Access>
        configuration::Tree GuardedNodeData<Access>::getTree() const
        {
            return (configuration::Tree) m_aViewLock.getTree();
        }

        template <class Access>
        configuration::NodeRef GuardedNodeData<Access>::getNode() const
        {
            return m_aViewLock.getNode();
        }
    }
}

#endif // CONFIGMGR_API_NODEACCESS_HXX_
