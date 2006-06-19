/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodevisitor.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:29:58 $
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

#include "nodevisitor.hxx"

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_SETNODEACCESS_HXX
#include "setnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
// -------------------------------------------------------------------------
typedef NodeVisitor::Result Result;

// -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
    struct NodeVisitor::Dispatcher
    {
        NodeVisitor& m_target;
        Accessor const & m_accessor;
        Result       m_result;

        Dispatcher(NodeVisitor& _rTarget, Accessor const * _pAccessor)
        : m_target(_rTarget)
        , m_accessor(*_pAccessor)
        , m_result(NodeVisitor::CONTINUE)
        {}

        void applyToNode(sharable::Node const & _aNode);
        void applyToChildren(sharable::GroupNode const & _aNode);

        Result dispatch(sharable::Node const& _aNode);
    };

    // -------------------------------------------------------------------------
    struct SetVisitor::Dispatcher
    {
        SetVisitor&  m_target;
        Accessor const & m_accessor;
        Result       m_result;

        Dispatcher(SetVisitor& _rTarget, Accessor const * _pAccessor)
        : m_target(_rTarget)
        , m_accessor(*_pAccessor)
        , m_result(NodeVisitor::CONTINUE)
        {}

        void applyToTree(sharable::TreeFragment const & _aElement);
        void applyToElements(sharable::SetNode const & _aNode);

        Result dispatch(sharable::TreeFragment const& _aElement);
    };

    // -------------------------------------------------------------------------

    inline
    Result NodeVisitor::Dispatcher::dispatch(sharable::Node const& _aNode)
    {
        using namespace sharable::Type;
        switch (_aNode.node.info.type & mask_nodetype)
        {
        case nodetype_value:
            return m_target.handle( ValueNodeAccess(m_accessor, &_aNode.value) );

        case nodetype_group:
            return m_target.handle( GroupNodeAccess(m_accessor, &_aNode.group) );

        case nodetype_set:
            return m_target.handle( SetNodeAccess(m_accessor, &_aNode.set) );

        default:
            OSL_ENSURE(false,"NodeVisitor: invalid node type detected"); // invalid node
            return m_target.handle( NodeAccessRef(&m_accessor, &_aNode) );
        }
    }
    // -------------------------------------------------------------------------

    inline
    Result SetVisitor::Dispatcher::dispatch(sharable::TreeFragment const& _aElement)
    {
        return m_target.handle( TreeAccessor(m_accessor, &_aElement) );
    }
    // -------------------------------------------------------------------------

    void NodeVisitor::Dispatcher::applyToNode(sharable::Node const & _aNode)
    {
        if (m_result != NodeVisitor::DONE)
            m_result = dispatch(_aNode);
    }
    // -------------------------------------------------------------------------

    void SetVisitor::Dispatcher::applyToTree(sharable::TreeFragment const & _aElement)
    {
        if (m_result != NodeVisitor::DONE)
            m_result = dispatch(_aElement);
    }
    // -------------------------------------------------------------------------

    void NodeVisitor::Dispatcher::applyToChildren(sharable::GroupNode const & _aNode)
    {
        using sharable::Node;
        for (Node const * pChild = _aNode.getFirstChild();
                pChild != NULL && m_result != NodeVisitor::DONE;
                pChild = _aNode.getNextChild(pChild) )
            m_result = dispatch(*pChild);
    }
    // -------------------------------------------------------------------------

    void SetVisitor::Dispatcher::applyToElements(sharable::SetNode const & _aNode)
    {
        using sharable::TreeFragment;
        for (TreeFragment const * pElement = _aNode.getFirstElement(m_accessor);
                pElement != NULL && m_result != NodeVisitor::DONE;
                pElement = _aNode.getNextElement(m_accessor,pElement) )
            m_result = dispatch(*pElement);

    }
    // -------------------------------------------------------------------------
// -------------------------------------------------------------------------

Result NodeVisitor::visitNode(NodeAccessRef const& _aNode)
{
    Dispatcher aDispatcher(*this, &_aNode.accessor());

    aDispatcher.applyToNode(_aNode.data());

    return aDispatcher.m_result;
}
// -------------------------------------------------------------------------

Result SetVisitor::visitTree(TreeAccessor const& _aNode)
{
    Dispatcher aDispatcher(*this, &_aNode.accessor());

    aDispatcher.applyToTree(_aNode.data());

    return aDispatcher.m_result;
}
// -------------------------------------------------------------------------

Result NodeVisitor::visitChildren(GroupNodeAccess const& _aNode)
{
    Dispatcher aDispatcher(*this, &_aNode.accessor());

    aDispatcher.applyToChildren(_aNode.data());

    return aDispatcher.m_result;
}
// -------------------------------------------------------------------------

Result SetVisitor::visitElements(SetNodeAccess const& _aNode)
{
    Dispatcher aDispatcher(*this, &_aNode.accessor());

    aDispatcher.applyToElements(_aNode.data());

    return aDispatcher.m_result;
}
// -------------------------------------------------------------------------

Result NodeVisitor::handle(NodeAccessRef const& /*_aNode*/)
{
    return CONTINUE;
}
// -------------------------------------------------------------------------

Result NodeVisitor::handle(ValueNodeAccess const& _aNode)
{
    return handle(static_cast<NodeAccessRef>(_aNode));
}
// -------------------------------------------------------------------------

Result NodeVisitor::handle(GroupNodeAccess const& _aNode)
{
    return handle(static_cast<NodeAccessRef>(_aNode));
}
// -------------------------------------------------------------------------

Result NodeVisitor::handle(SetNodeAccess const& _aNode)
{
    return handle(static_cast<NodeAccessRef>(_aNode));
}
// -------------------------------------------------------------------------

Result SetVisitor::handle(TreeAccessor const& _aTree)
{
    return NodeVisitor::visitNode(_aTree.getRootNode());
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr


