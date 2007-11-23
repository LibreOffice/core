/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewnode.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:49:36 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "viewnode.hxx"

#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif

#ifndef CONFIGMGR_SETNODEBEHAVIOR_HXX_
#include "setnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif
#ifndef CONFIGMGR_VIEWBEHAVIOR_HXX_
#include "viewstrategy.hxx"
#endif

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

