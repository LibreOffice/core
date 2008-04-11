/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewnode.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "viewnode.hxx"
#include "valuenodeaccess.hxx"
#include "groupnodeaccess.hxx"
#include "setnodeaccess.hxx"
#include "setnodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "valuenodeimpl.hxx"
#include "treeimpl.hxx"
#include "viewstrategy.hxx"

//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------
        rtl::Reference< view::ViewStrategy > getViewBehavior(Tree const & _aTree)
        {
            Tree::TreeData* pTreeData = _aTree.get_impl();
            return pTreeData->getViewBehavior();
        }

//-----------------------------------------------------------------------------
        static inline Node makeNode_(Tree const & _aTree, configuration::NodeOffset nOffset)
        {
            Tree::TreeData * pTreeData = _aTree.get_impl();
            return Node(_aTree, pTreeData->nodeData(nOffset));
        }
//-----------------------------------------------------------------------------
        Node Node::getParent() const
        {
            Tree::TreeData * pTreeData = this->tree().get_impl();
            return makeNode_(tree(), pTreeData->parent_(this->get_offset()));
        }

//-----------------------------------------------------------------------------
        Node Node::getNextSibling() const
        {
            Tree::TreeData * pTreeData = this->tree().get_impl();
            return makeNode_(tree(), pTreeData->nextSibling_(this->get_offset()));
        }

//-----------------------------------------------------------------------------
        Node GroupNode::findChild(configuration::Name const& _aName) const
        {
            Tree::TreeData * pTreeData = this->tree().get_impl();
            return makeNode_(tree(), pTreeData->findChild_(node().get_offset(), _aName));
        }

//-----------------------------------------------------------------------------
        Node GroupNode::getFirstChild() const
        {
            Tree::TreeData * pTreeData = this->tree().get_impl();
            return makeNode_(tree(), pTreeData->firstChild_(node().get_offset()));
        }

//-----------------------------------------------------------------------------
        Node GroupNode::getNextChild(Node const& _aAfterNode) const
        {
            Tree::TreeData * pTreeData = this->tree().get_impl();
            OSL_ASSERT(pTreeData->parent_(_aAfterNode.get_offset()) == this->node().get_offset());
            return makeNode_(tree(), pTreeData->findNextChild_(node().get_offset(),_aAfterNode.get_offset()));
        }

//-----------------------------------------------------------------------------
        data::NodeAccess Node::getAccessRef() const
        {
            return get_impl()->getOriginalNodeAccess();
        }

//-----------------------------------------------------------------------------
        data::ValueNodeAccess ValueNode::getAccess() const
        {
            return get_impl()->getDataAccess();
        }

//-----------------------------------------------------------------------------
        data::GroupNodeAccess GroupNode::getAccess() const
        {
            return get_impl()->getDataAccess();
        }

//-----------------------------------------------------------------------------
        data::SetNodeAccess SetNode::getAccess() const
        {
            return get_impl()->getDataAccess();
        }

//-----------------------------------------------------------------------------
    }
}

