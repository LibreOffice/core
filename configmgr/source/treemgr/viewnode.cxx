/*************************************************************************
 *
 *  $RCSfile: viewnode.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:42:43 $
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
/*        data::TreeAccessor Tree::getAccess() const   // has a TreeAccessor
        {
            return get_impl()->getOriginalTreeAccess(m_accessor);
        }
*/
//-----------------------------------------------------------------------------
        data::NodeAccessRef Node::getAccessRef() const
        {
            return get_impl()->getOriginalNodeAccessRef(&accessor());
        }

//-----------------------------------------------------------------------------
        data::ValueNodeAccess ValueNode::getAccess() const
        {
            return get_impl()->getDataAccess(accessor());
        }

//-----------------------------------------------------------------------------
        data::GroupNodeAccess GroupNode::getAccess() const
        {
            return get_impl()->getDataAccess(accessor());
        }

//-----------------------------------------------------------------------------
        data::SetNodeAccess SetNode::getAccess() const
        {
            return get_impl()->getDataAccess(accessor());
        }

//-----------------------------------------------------------------------------
    }
}

