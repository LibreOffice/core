/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: nodeimpl.cxx,v $
 * $Revision: 1.26 $
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
#include <stdio.h>
#include "nodeimpl.hxx"
#include "valuenodeimpl.hxx"
#include "groupnodeimpl.hxx"
#include "nodeaccess.hxx"
#include "valuenodeaccess.hxx"
#include "groupnodeaccess.hxx"
#include "nodevisitor.hxx"
#include "treeimpl.hxx"
#include "nodechange.hxx"
#include "nodechangeimpl.hxx"
#include "nodechangeinfo.hxx"
#include "change.hxx"
#include <osl/diagnose.h>

namespace configmgr
{
    namespace configuration
    {

// Specific types of nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupNodeImpl
//-----------------------------------------------------------------------------

data::GroupNodeAccess GroupNodeImpl::getDataAccess() const
{
    using namespace data;

    NodeAccess aNodeAccess = getOriginalNodeAccess();
    OSL_ASSERT(GroupNodeAccess::isInstance(aNodeAccess));

    GroupNodeAccess aGroupAccess(aNodeAccess);
    OSL_ASSERT(aGroupAccess.isValid());

    return aGroupAccess;
}
//-----------------------------------------------------------------------------

GroupNodeImpl::GroupNodeImpl(data::GroupNodeAddress _pNodeRef)
    : NodeImpl(reinterpret_cast<data::NodeAddress>(_pNodeRef))
    , m_pCache( NULL )
{
}
//-----------------------------------------------------------------------------

bool GroupNodeImpl::areValueDefaultsAvailable() const
{
    data::GroupNodeAccess aGroupAccess = getDataAccess();

    return aGroupAccess.data().hasDefaultsAvailable();
}
//-----------------------------------------------------------------------------

ValueMemberNode GroupNodeImpl::makeValueMember(data::ValueNodeAccess const& _aNodeAccess)
{
    return ValueMemberNode(_aNodeAccess);
}
//-----------------------------------------------------------------------------

data::ValueNodeAccess GroupNodeImpl::getOriginalValueNode(Name const& _aName) const
{
    OSL_ENSURE( !_aName.isEmpty(), "Cannot get nameless child value");

    using namespace data;

    data::GroupNodeAccess aAccess = this->getDataAccess();
    const rtl::OUString &rName = _aName.toString();

/*
    fprintf (stderr, "GroupNodeImpl::GetOriginalValueNode %p '%s' ", this,
             rtl::OUStringToOString(rName, RTL_TEXTENCODING_UTF8).getStr());
    fprintf (stderr, "cache '%s'\n",
             m_pCache ? rtl::OUStringToOString(m_pCache->getName(),
                                               RTL_TEXTENCODING_UTF8).getStr()
             : "<null>");
*/

    if (m_pCache)
    {
        if (m_pCache->isNamed(rName))
            return ValueNodeAccess( (ValueNodeAddress) m_pCache );

        sharable::GroupNode & aNode = aAccess.data();
        m_pCache = aNode.getNextChild(m_pCache);

        if (m_pCache && m_pCache->isNamed(rName))
            return ValueNodeAccess( (ValueNodeAddress) m_pCache );
        m_pCache = NULL;
    }

    NodeAccess aChild = aAccess.getChildNode(_aName);
    m_pCache = aChild;

    // to do: investigate cache lifecycle more deeply.

    return ValueNodeAccess(aChild);
}

//-----------------------------------------------------------------------------
// class ValueElementNodeImpl
//-----------------------------------------------------------------------------

data::ValueNodeAccess ValueElementNodeImpl::getDataAccess() const
{
    using namespace data;

    NodeAccess aNodeAccess = getOriginalNodeAccess();
    OSL_ASSERT(ValueNodeAccess::isInstance(aNodeAccess));

    ValueNodeAccess aValueAccess(aNodeAccess);
    OSL_ASSERT(aValueAccess.isValid());

    return aValueAccess;
}
//-----------------------------------------------------------------------------

ValueElementNodeImpl::ValueElementNodeImpl(data::ValueNodeAddress const& _aNodeRef)
    : NodeImpl(reinterpret_cast<data::NodeAddress>(_aNodeRef))
{
}
//-----------------------------------------------------------------------------

UnoAny  ValueElementNodeImpl::getValue() const
{
    return getDataAccess().getValue();
}
//-----------------------------------------------------------------------------

UnoType ValueElementNodeImpl::getValueType() const
{
    return getDataAccess().getValueType();
}
//-----------------------------------------------------------------------------

namespace
{
    struct AbstractNodeCast : data::NodeVisitor
    {
           virtual Result handle( data::ValueNodeAccess const& /*rNode*/)
           {
               throw Exception( "INTERNAL ERROR: Node is not a value node. Cast failing." );
           }
           virtual Result handle( data::GroupNodeAccess const& /*rNode*/)
           {
               throw Exception( "INTERNAL ERROR: Node is not a group node. Cast failing." );
           }
           virtual Result handle( data::SetNodeAccess const& /*rNode*/)
           {
               return CONTINUE;
           }
        protected:
            using NodeVisitor::handle;
    };

    template <class NodeType>
    class NodeCast : AbstractNodeCast
    {
    public:
        typedef typename NodeType::DataAccess DataNodeType;

        NodeCast(NodeImpl& rOriginalNode)
        : m_pNode(0)
        {
            if (this->visitNode(rOriginalNode.getOriginalNodeAccess()) == DONE)
                m_pNode = static_cast<NodeType*>(&rOriginalNode);
        }

        NodeType& get() const
        {
            OSL_ENSURE(m_pNode,"INTERNAL ERROR: Node not set after Cast." );
            return *m_pNode;
        }

        operator NodeType& () const { return get(); }
        private:
        virtual Result handle( DataNodeType& ) { return DONE; }

        NodeType* m_pNode;
    };
}
//-----------------------------------------------------------------------------
    }
}
