/*************************************************************************
 *
 *  $RCSfile: viewnode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:42:55 $
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

#ifndef CONFIGMGR_VIEWNODE_HXX_
#define CONFIGMGR_VIEWNODE_HXX_

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
        class Name;
        struct ElementTreeData;
    }
//-----------------------------------------------------------------------------
    namespace view
    {
    //-------------------------------------------------------------------------
        using configuration::Name;
    //-------------------------------------------------------------------------
        struct Tree        // is a tree fragment
        {
            typedef configuration::TreeImpl TreeData;

            Tree(data::Accessor const& _accessor, TreeData& _ref)
            : m_accessor(_ref.getDataAccessor(_accessor)), m_addr(&_ref)
            {}

        // low-level access
//            TreeData* operator->() const { return m_addr; }

            TreeData* get_impl() const
            { return m_addr; }

            data::Accessor const& accessor() const   // has a Accessor
            { return m_accessor; }

        //    data::TreeAccessor getAccess() const;   // has a TreeAccessor

        private:
            data::Accessor m_accessor;   // has a TreeAccessor
            TreeData *  m_addr;  // has a TreeAddress or NodeAddress
        };
    //-------------------------------------------------------------------------
        struct Node
        {
            typedef configuration::NodeOffset   NodeOffset;
            typedef configuration::NodeData     NodeData;
            typedef configuration::NodeImpl *   NodeAddress;

            Node(Tree const& _tree, NodeData* _addr)
            : m_tree(_tree), m_addr(_addr)
            {}

            Node(data::Accessor const& _accessor, Tree::TreeData& _ref, NodeOffset _offs)
                : m_tree(_accessor,_ref), m_addr( _ref.nodeData(_offs))
            {}

            bool is() const { return m_addr != 0; }

            Node getParent() const;
            Node getNextSibling() const;

            bool isSetNode()    const { return is() && data().isSetNode(this->accessor()); }
            bool isGroupNode()  const { return is() && data().isGroupNode(this->accessor()); }
            bool isValueNode()  const { return is() && data().isValueElementNode(this->accessor()); }

        // low-level access
//            NodeAddress operator->() const { return &data().nodeImpl(); }

            NodeData& data() const { return *m_addr; }

            NodeAddress get_impl() const
            { return is() ? &data().nodeImpl() : NULL; }

            NodeOffset get_offset() const
            { return is() ? m_tree.get_impl()->nodeOffset( this->data() ) : 0; }

            Tree tree() const   // has a Tree
            { return m_tree; }

            data::Accessor const& accessor() const   // has a Accessor
            { return m_tree.accessor(); }

            data::NodeAccessRef getAccessRef() const;   // has a NodeAccess

        private:
            Tree        m_tree;   // has a Tree + Accessor
            NodeData*   m_addr;       // has a NodeAddress
        };
    //-------------------------------------------------------------------------
        struct ValueNode        // has/is a Node
        {
            typedef configuration::ValueElementNodeImpl NodeType;
            Node m_node;

            explicit
            ValueNode(Node const& _node)
            : m_node(_node)
            {}

            bool is() const { return m_node.isValueNode(); }

        // low-level access
 //           NodeType* operator->() const { return &m_node.data().valueElementImpl(accessor()); }

            NodeType* get_impl() const
            { return is() ? &m_node.data().valueElementImpl(accessor()) : NULL; }

            Node node() const   // has a Node
            { return m_node; }

            Tree tree() const   // has a Tree
            { return m_node.tree(); }

            data::Accessor const& accessor() const   // has a TreeAccessor
            { return m_node.accessor(); }

            data::ValueNodeAccess getAccess() const;   // has a NodeAccess
        };
    //-------------------------------------------------------------------------
        struct GroupNode        // has/is a Node
        {
            typedef configuration::GroupNodeImpl NodeType;
            Node m_node;

            explicit
            GroupNode(Node const& _node)
            : m_node(_node)
            {}

            bool is() const { return m_node.isGroupNode(); }

            Node findChild(configuration::Name const& _aName) const;
            Node getFirstChild() const;
            Node getNextChild(Node const& _aAfterNode) const;

//            NodeType* operator->() const { return &m_node.data().groupImpl(accessor()); }

            NodeType* get_impl() const
            { return is() ? &m_node.data().groupImpl(accessor()) : NULL; }

            Node node() const   // has a Node
            { return m_node; }

            Tree tree() const   // has a Tree
            { return m_node.tree(); }

            data::Accessor const& accessor() const   // has a TreeAccessor
            { return m_node.accessor(); }

            data::GroupNodeAccess getAccess() const;   // has a NodeAccess
        };
    //-------------------------------------------------------------------------
        struct SetNode          // has/is a Node
        {
            typedef configuration::SetNodeImpl NodeType;
            typedef configuration::ElementTreeData Element;

            Node m_node;

            explicit
            SetNode(Node const& _node)
            : m_node(_node)
            {}

            bool is() const { return m_node.isSetNode(); }

   //         NodeType* operator->() const  { return &m_node.data().setImpl(accessor()); }

            NodeType* get_impl() const
            { return is() ? &m_node.data().setImpl(accessor()) : 0; }

            Node node() const   // has a Node
            { return m_node; }

            Tree tree() const   // has a Tree
            { return m_node.tree(); }

            data::Accessor const& accessor() const   // has a TreeAccessor
            { return m_node.accessor(); }

            data::SetNodeAccess getAccess() const;   // has a NodeAccess
        };
    //-------------------------------------------------------------------------
        inline
        Node getRootNode(Tree const & _aTree)
        {
            Tree::TreeData* pTreeData = _aTree.get_impl();
            return Node(_aTree,pTreeData->nodeData(pTreeData->root_()));
        }

    //-------------------------------------------------------------------------
        inline
        Name getSimpleRootName(Tree const & _aTree)
        {
            Tree::TreeData* pTreeData = _aTree.get_impl();
            return pTreeData->getSimpleRootName();
        }

    //-------------------------------------------------------------------------
        inline
        bool isValidNode(Node const & _aNode)
        {
            Tree::TreeData* pTreeData = _aNode.tree().get_impl();
            return pTreeData->isValidNode(_aNode.get_offset());
        }

    //-------------------------------------------------------------------------
        inline
        Name getSimpleNodeName(Node const & _aNode)
        {
            Tree::TreeData* pTreeData = _aNode.tree().get_impl();
            return pTreeData->getSimpleNodeName(_aNode.get_offset());
        }

    //-------------------------------------------------------------------------
        extern
        rtl::Reference< view::ViewStrategy > getViewBehavior(Tree const & _aTree);
    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_VIEWNODE_HXX_
