/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupnodeaccess.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:47:55 $
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

#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#define CONFIGMGR_GROUPNODEACCESS_HXX

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class GroupNodeAccess
        {
        public:
            typedef NodeAccess::Name        Name;
            typedef NodeAccess::Attributes  Attributes;
            typedef GroupNodeAddress                    NodeAddressType;
            typedef GroupNodeAddress::AddressType       AddressType;
            typedef GroupNodeAddress::DataType const    DataType;
            typedef DataType * NodePointerType;
            typedef NodeAddress                         ChildAddressType;
            typedef NodeAccessRef                       ChildAccessType;

            GroupNodeAccess(Accessor const& _aAccessor, NodeAddressType const& _aNodeRef)
            : m_aAccessor(_aAccessor)
            , m_pData(_aNodeRef.m_pData)
            {}

            GroupNodeAccess(Accessor const& _aAccessor, NodePointerType _pNode)
            : m_aAccessor(_aAccessor)
            , m_pData(check(_aAccessor,_pNode))
            {}

            explicit
            GroupNodeAccess(NodeAccess const & _aNode)
            : m_aAccessor(_aNode.accessor())
            , m_pData(check(_aNode))
            {
            }

            explicit
            GroupNodeAccess(NodeAccessRef const & _aNode)
            : m_aAccessor(_aNode.accessor())
            , m_pData(check(_aNode))
            {
            }

            static bool isInstance(NodeAccessRef const & _aNode)
            {
                return check(_aNode) != NULL;
            }

            bool isValid() const { return m_pData != NULL; }

            Name getName() const;
            Attributes getAttributes() const;

            bool isDefault()   const;

            bool hasChild(Name const& _aName) const
            { return implGetChild(_aName).is(); }

            bool hasChildren() const;

            ChildAccessType getChildNode(Name const& _aName) const
            { return NodeAccessRef(&m_aAccessor, implGetChild(_aName)); }

            NodeAddressType address()   const { return NodeAddressType(m_pData); }
            Accessor const& accessor()  const { return m_aAccessor; }

            operator NodeAccessRef()    const { return NodeAccessRef(&m_aAccessor,NodeAddress(m_pData)); }

            DataType& data() const { return *static_cast<NodePointerType>(m_aAccessor.validate(m_pData)); }

        private:
            static AddressType check(NodeAccessRef const&);
            static AddressType check(Accessor const&, NodePointerType);

            ChildAddressType implGetChild(Name const& _aName) const;

            Accessor    m_aAccessor;
            AddressType m_pData;
        };

        GroupNodeAddress toGroupNodeAddress(memory::Accessor const & _aAccess, NodeAddress const & _aNodeAddr);
        GroupNodeAddress toGroupNodeAddress(memory::UpdateAccessor & _aAccess, NodeAddress const & _aNodeAddr);
    // -------------------------------------------------------------------------
        inline
        NodeAccess::Name GroupNodeAccess::getName() const
        { return NodeAccess::wrapName( data().info.getName() ); }

        inline
        NodeAccess::Attributes GroupNodeAccess::getAttributes() const
        { return sharable::node(data()).getAttributes(); }

        inline
        bool GroupNodeAccess::isDefault()   const
        { return data().info.isDefault(); }

    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_GROUPNODEACCESS_HXX

