/*************************************************************************
 *
 *  $RCSfile: apinodeaccess.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 15:07:20 $
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

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#if defined(_MSC_VER) && (_MSC_VER >=1310)
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#endif

#ifndef CONFIGMGR_ACCESSOR_HXX
#include <accessor.hxx>
#endif

namespace osl { class Mutex; }

namespace configmgr
{
    namespace memory
    {
        class Accessor;
        class Segment;
    }
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
            configuration::Tree     getTree(memory::Accessor const& _aAccessor) const;

        // self-locked methods for dispose handling
            void checkAlive() const;
            void disposeNode();

        // api object handling
            UnoInterface*           getUnoInstance() const
            { return doGetUnoInstance(); }
            Factory&                getFactory() const;
            Notifier                getNotifier() const;

        // locking support
            osl::Mutex&             getDataLock() const;
            memory::Segment const*  getSourceData() const;
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
            configuration::SetElementInfo getElementInfo(memory::Accessor const& _aAccessor) const;
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
        /// guards a NodeAccess; provides an object (read) lock, ensures object was not disposed
        class NodeReadGuardImpl : Noncopyable
        {
            osl::MutexGuard     m_aLock;
            NodeAccess&         m_rNode;
        public:
            NodeReadGuardImpl(NodeAccess& rNode) throw();
            ~NodeReadGuardImpl() throw ();
        public:
            NodeAccess& get() const { return m_rNode; }

            configuration::Tree     getTree(memory::Accessor const& _aAccessor) const;
            configuration::NodeRef  getNode() const;

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

        /// wraps a NodeAccess; provides both object and provider (read) locks, ensures object was not disposed
        template <class Access>
        class GuardedNodeData
        {
            memory::Accessor        m_aDataAccess;
            NodeReadGuardImpl   m_aViewLock;
        public:
            GuardedNodeData(Access& rNode);
        public:
            Access& get()        const { return static_cast<Access&>(m_aViewLock.get()); }

            configuration::Tree     getTree() const;
            configuration::NodeRef  getNode() const;

            memory::Accessor const & getDataAccessor() const { return m_aDataAccess; }
        };
        typedef GuardedNodeData<NodeAccess> GuardedNodeDataAccess;

        template <class Access>
        GuardedNodeData<Access>::GuardedNodeData(Access& rNode)
        : m_aDataAccess(rNode.getSourceData())
        , m_aViewLock(rNode)
        {
        }

        template <class Access>
        configuration::Tree GuardedNodeData<Access>::getTree() const
        {
            return (configuration::Tree) m_aViewLock.getTree(m_aDataAccess);
        }

        template <class Access>
        configuration::NodeRef GuardedNodeData<Access>::getNode() const
        {
            return m_aViewLock.getNode();
        }
    }
}

#endif // CONFIGMGR_API_NODEACCESS_HXX_
