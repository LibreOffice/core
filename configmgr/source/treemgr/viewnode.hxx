/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewnode.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_VIEWNODE_HXX_
#define CONFIGMGR_VIEWNODE_HXX_

#include "tree.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
        struct ElementTreeData;
    }
    namespace sharable { struct GroupNode; }
//-----------------------------------------------------------------------------
    namespace view
    {
    //-------------------------------------------------------------------------
        struct Node
        {
            Node(configuration::Tree * _tree, configuration::NodeData* _addr)
            : m_tree(_tree), m_addr(_addr)
            {}

            Node(configuration::Tree * _tree, unsigned int _offs)
                : m_tree(_tree), m_addr( _tree->nodeData(_offs))
            {}

            bool is() const { return m_addr != 0; }

            Node getParent() const;

            bool isSetNode()    const { return is() && data().isSetNode(); }
            bool isGroupNode()  const { return is() && data().isGroupNode(); }
            bool isValueNode()  const { return is() && data().isValueElementNode(); }

        // low-level access
//            configuration::NodeImpl * operator->() const { return &data().nodeImpl(); }

            configuration::NodeData& data() const { return *m_addr; }

            configuration::NodeImpl * get_impl() const
            { return is() ? &data().nodeImpl() : NULL; }

            unsigned int get_offset() const
            { return is() ? m_tree->nodeOffset( this->data() ) : 0; }

            configuration::Tree * tree() const   // has a tree
            { return m_tree; }

            sharable::Node * getAccessRef() const;   // has a Node

        private:
            configuration::Tree *        m_tree;   // has a Tree + Accessor
            configuration::NodeData*   m_addr;       // has a configuration::NodeImpl *
        };
    //-------------------------------------------------------------------------
        struct ValueNode        // has/is a Node
        {
            Node m_node;

            explicit
            ValueNode(Node const& _node)
            : m_node(_node)
            {}

            bool is() const { return m_node.isValueNode(); }

        // low-level access
 //           configuration::ValueElementNodeImpl* operator->() const { return &m_node.data().valueElementImpl(); }

            configuration::ValueElementNodeImpl* get_impl() const
            { return is() ? &m_node.data().valueElementImpl() : NULL; }

            Node node() const   // has a Node
            { return m_node; }

            configuration::Tree * tree() const   // has a tree
            { return m_node.tree(); }
        };
    //-------------------------------------------------------------------------
        struct GroupNode        // has/is a Node
        {
            Node m_node;

            explicit
            GroupNode(Node const& _node)
            : m_node(_node)
            {}

            bool is() const { return m_node.isGroupNode(); }

            Node findChild(rtl::OUString const& _aName) const;
            Node getFirstChild() const;
            Node getNextChild(Node const& _aAfterNode) const;

//            configuration::GroupNodeImpl* operator->() const { return &m_node.data().groupImpl(); }

            configuration::GroupNodeImpl* get_impl() const
            { return is() ? &m_node.data().groupImpl() : NULL; }

            Node node() const   // has a Node
            { return m_node; }

            configuration::Tree * tree() const   // has a tree
            { return m_node.tree(); }

            sharable::GroupNode * getAccess() const;   // has a GroupNode
        };
    //-------------------------------------------------------------------------
        struct SetNode          // has/is a Node
        {
            Node m_node;

            explicit
            SetNode(Node const& _node)
            : m_node(_node)
            {}

            bool is() const { return m_node.isSetNode(); }

   //         configuration::SetNodeImpl* operator->() const  { return &m_node.data().setImpl(); }

            configuration::SetNodeImpl* get_impl() const
            { return is() ? &m_node.data().setImpl() : 0; }

            Node node() const   // has a Node
            { return m_node; }

            configuration::Tree * tree() const   // has a tree
            { return m_node.tree(); }

            sharable::SetNode * getAccess() const;   // has a SetNode
        };
    //-------------------------------------------------------------------------
        inline
        Node getRootNode(configuration::Tree * _aTree)
        {
            return Node(_aTree,_aTree->nodeData(configuration::Tree::ROOT));
        }

    //-------------------------------------------------------------------------
        inline
        rtl::OUString getSimpleRootName(configuration::Tree * _aTree)
        {
            return _aTree->getSimpleRootName();
        }

    //-------------------------------------------------------------------------
        inline
        bool isValidNode(Node const & _aNode)
        {
            configuration::Tree* pTreeData = _aNode.tree();
            return pTreeData->isValidNode(_aNode.get_offset());
        }

    //-------------------------------------------------------------------------
        inline
        rtl::OUString getSimpleNodeName(Node const & _aNode)
        {
            configuration::Tree* pTreeData = _aNode.tree();
            return pTreeData->getSimpleNodeName(_aNode.get_offset());
        }

    //-------------------------------------------------------------------------
        extern
        rtl::Reference< view::ViewStrategy > getViewBehavior(configuration::Tree * _aTree);
    //-------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_VIEWNODE_HXX_
