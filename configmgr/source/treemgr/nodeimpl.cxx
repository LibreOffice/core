/*************************************************************************
 *
 *  $RCSfile: nodeimpl.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:40:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc.,October,2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems,Inc.
 *  901 San Antonio Road,Palo Alto,CA 94303,USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1,as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not,write to the Free Software
 *  Foundation,Inc.,59 Temple Place,Suite 330,Boston,
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
 *  WITHOUT WARRANTY OF ANY KIND,EITHER EXPRESSED OR IMPLIED,INCLUDING,
 *  WITHOUT LIMITATION,WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE,FIT FOR A PARTICULAR PURPOSE,OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems,Inc.
 *
 *  Copyright: 2000 by Sun Microsystems,Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>
#include "nodeimpl.hxx"

#ifndef CONFIGMGR_VALUENODEBEHAVIOR_HXX_
#include "valuenodeimpl.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#include "groupnodeimpl.hxx"
#endif

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif
#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif
#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#include "groupnodeaccess.hxx"
#endif
#ifndef CONFIGMGR_NODEVISITOR_HXX
#include "nodevisitor.hxx"
#endif

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#include "treeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGE_HXX_
#include "nodechange.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEIMPL_HXX_
#include "nodechangeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGCHANGEINFO_HXX_
#include "nodechangeinfo.hxx"
#endif
#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

namespace configmgr
{
    namespace configuration
    {


//-----------------------------------------------------------------------------
// class NodeImpl
//-----------------------------------------------------------------------------

/// provide access to the data of the underlying node
data::NodeAccessRef NodeImpl::getOriginalNodeAccessRef(data::Accessor const * _pAccessor) const
{
    return data::NodeAccessRef(_pAccessor,m_aNodeRef_);
}
//-----------------------------------------------------------------------------


// Specific types of nodes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// class GroupNodeImpl
//-----------------------------------------------------------------------------

data::GroupNodeAccess GroupNodeImpl::getDataAccess(data::Accessor const& _aAccessor) const
{
    using namespace data;

    NodeAccessRef aNodeAccess = getOriginalNodeAccessRef(&_aAccessor);
    OSL_ASSERT(GroupNodeAccess::isInstance(aNodeAccess));

    GroupNodeAccess aGroupAccess(aNodeAccess);
    OSL_ASSERT(aGroupAccess.isValid());

    return aGroupAccess;
}
//-----------------------------------------------------------------------------

GroupNodeImpl::GroupNodeImpl(data::GroupNodeAddress const& _aNodeRef)
: NodeImpl(_aNodeRef)
{
}
//-----------------------------------------------------------------------------

bool GroupNodeImpl::areValueDefaultsAvailable(data::Accessor const& _aAccessor) const
{
    data::GroupNodeAccess aGroupAccess = getDataAccess(_aAccessor);

    return aGroupAccess.data().hasDefaultsAvailable();
}
//-----------------------------------------------------------------------------

ValueMemberNode GroupNodeImpl::makeValueMember(data::ValueNodeAccess const& _aNodeAccess)
{
    return ValueMemberNode(_aNodeAccess);
}
//-----------------------------------------------------------------------------

data::ValueNodeAccess GroupNodeImpl::getOriginalValueNode(data::Accessor const& _aAccessor, Name const& _aName) const
{
    OSL_ENSURE( !_aName.isEmpty(), "Cannot get nameless child value");

    using namespace data;

    NodeAccessRef aChild = this->getDataAccess(_aAccessor).getChildNode(_aName);

    return ValueNodeAccess(aChild);
}

//-----------------------------------------------------------------------------
// class ValueElementNodeImpl
//-----------------------------------------------------------------------------

data::ValueNodeAccess ValueElementNodeImpl::getDataAccess(data::Accessor const& _aAccessor) const
{
    using namespace data;

    NodeAccessRef aNodeAccess = getOriginalNodeAccessRef(&_aAccessor);
    OSL_ASSERT(ValueNodeAccess::isInstance(aNodeAccess));

    ValueNodeAccess aValueAccess(aNodeAccess);
    OSL_ASSERT(aValueAccess.isValid());

    return aValueAccess;
}
//-----------------------------------------------------------------------------

ValueElementNodeImpl::ValueElementNodeImpl(data::ValueNodeAddress const& _aNodeRef)
: NodeImpl(_aNodeRef)
{
}
//-----------------------------------------------------------------------------

UnoAny  ValueElementNodeImpl::getValue(data::Accessor const& _aAccessor) const
{
    return getDataAccess(_aAccessor).getValue();
}
//-----------------------------------------------------------------------------

UnoType ValueElementNodeImpl::getValueType(data::Accessor const& _aAccessor) const
{
    return getDataAccess(_aAccessor).getValueType();
}
//-----------------------------------------------------------------------------

namespace
{
    struct AbstractNodeCast : data::NodeVisitor
    {
        virtual Result handle( data::ValueNodeAccess const& rNode)
        {
            throw Exception( "INTERNAL ERROR: Node is not a value node. Cast failing." );
            return CONTINUE;
        }
        virtual Result handle( data::GroupNodeAccess const& rNode)
        {
            throw Exception( "INTERNAL ERROR: Node is not a group node. Cast failing." );
            return CONTINUE;
        }
        virtual Result handle( data::SetNodeAccess const& rNode)
        {
            return CONTINUE;
        }
    };

    template <class NodeType>
    class NodeCast : AbstractNodeCast
    {
    public:
        typedef typename NodeType::DataAccess DataNodeType;

        NodeCast(NodeImpl& rOriginalNode, data::Accessor const& _aAccessor)
        : m_pNode(0)
        {
            if (this->visitNode(rOriginalNode.getOriginalNodeAccessRef(&_aAccessor)) == DONE)
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
