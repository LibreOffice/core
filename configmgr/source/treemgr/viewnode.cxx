/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewnode.cxx,v $
 * $Revision: 1.7 $
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
#include "setnodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "valuenodeimpl.hxx"
#include "tree.hxx"
#include "viewstrategy.hxx"

//-----------------------------------------------------------------------------
namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace view
    {
//-----------------------------------------------------------------------------
        rtl::Reference< view::ViewStrategy > getViewBehavior(configuration::Tree * _aTree)
        {
            return _aTree->getViewBehavior();
        }

//-----------------------------------------------------------------------------
        static inline Node makeNode_(configuration::Tree * _aTree, unsigned int nOffset)
        {
            return Node(_aTree, _aTree->nodeData(nOffset));
        }
//-----------------------------------------------------------------------------
        Node Node::getParent() const
        {
            configuration::Tree * pTreeData = this->tree();
            return makeNode_(tree(), pTreeData->parent_(this->get_offset()));
        }

//-----------------------------------------------------------------------------
        Node GroupNode::findChild(rtl::OUString const& _aName) const
        {
            configuration::Tree * pTreeData = this->tree();
            return makeNode_(tree(), pTreeData->findChild_(node().get_offset(), _aName));
        }

//-----------------------------------------------------------------------------
        Node GroupNode::getFirstChild() const
        {
            configuration::Tree * pTreeData = this->tree();
            return makeNode_(tree(), pTreeData->firstChild_(node().get_offset()));
        }

//-----------------------------------------------------------------------------
        Node GroupNode::getNextChild(Node const& _aAfterNode) const
        {
            configuration::Tree * pTreeData = this->tree();
            OSL_ASSERT(pTreeData->parent_(_aAfterNode.get_offset()) == this->node().get_offset());
            return makeNode_(tree(), pTreeData->findNextChild_(node().get_offset(),_aAfterNode.get_offset()));
        }

//-----------------------------------------------------------------------------
        sharable::Node * Node::getAccessRef() const
        {
            return get_impl()->getOriginalNodeAccess();
        }

//-----------------------------------------------------------------------------
        sharable::GroupNode * GroupNode::getAccess() const
        {
            return get_impl()->getDataAccess();
        }

//-----------------------------------------------------------------------------
        sharable::SetNode * SetNode::getAccess() const
        {
            return get_impl()->getDataAccess();
        }

//-----------------------------------------------------------------------------
    }
}

