/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valuenodeaccess.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:49:26 $
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

#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#define CONFIGMGR_VALUENODEACCESS_HXX

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class ValueNodeAccess
        {
        public:
            typedef NodeAccess::Name        Name;
            typedef NodeAccess::Attributes  Attributes;
            typedef ValueNodeAddress                    NodeAddressType;
            typedef ValueNodeAddress::AddressType       AddressType;
            typedef ValueNodeAddress::DataType const    DataType;
            typedef DataType * NodePointerType;

            ValueNodeAccess(Accessor const& _aAccessor, NodeAddressType const& _aNodeRef)
            : m_aAccessor(_aAccessor)
            , m_pData(_aNodeRef.m_pData)
            {}

            ValueNodeAccess(Accessor const& _aAccessor, NodePointerType _pNode)
            : m_aAccessor(_aAccessor)
            , m_pData(check(_aAccessor,_pNode))
            {}

            explicit
            ValueNodeAccess(NodeAccess const & _aNode)
            : m_aAccessor(_aNode.accessor())
            , m_pData(check(_aNode))
            {
            }

            explicit
            ValueNodeAccess(NodeAccessRef const & _aNode)
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

            bool isEmpty()     const { return data().isEmpty(); }

            bool isNull()      const { return data().isNull(); }
            bool isDefault()   const;
            bool isLocalized() const;

            bool hasUsableDefault()   const { return data().hasUsableDefault(); }

            uno::Type   getValueType()  const { return data().getValueType(); }
            uno::Any    getValue()      const;
            uno::Any    getUserValue()      const;
            uno::Any    getDefaultValue()   const;

            static void setValue(memory::UpdateAccessor & _aUpdater, NodeAddressType _aValueNode, uno::Any const& _aValue);
            static void setToDefault(memory::UpdateAccessor & _aUpdater, NodeAddressType _aValueNode);
            static void changeDefault(memory::UpdateAccessor & _aUpdater, NodeAddressType _aValueNode, uno::Any const& _aValue);

            NodeAddressType address()   const { return NodeAddressType(m_pData); }
            Accessor const& accessor()  const { return m_aAccessor; }

            DataType& data() const { return *static_cast<NodePointerType>(m_aAccessor.validate(m_pData)); }

            operator NodeAccessRef() const { return NodeAccessRef(&m_aAccessor,NodeAddress(m_pData)); }
        private:
            static AddressType check(Accessor const& _acc, NodePointerType _p) { return _acc.address(_p); }
            static AddressType check(NodeAccessRef const& _aNodeData);

            Accessor    m_aAccessor;
            AddressType m_pData;
        };

        ValueNodeAddress toValueNodeAddress(memory::Accessor const & _aAccess, NodeAddress const & _aNodeAddr);
        ValueNodeAddress toValueNodeAddress(memory::UpdateAccessor & _aAccess, NodeAddress const & _aNodeAddr);
    // -------------------------------------------------------------------------

        inline
        NodeAccess::Name ValueNodeAccess::getName() const
        { return NodeAccess::wrapName( data().info.getName() ); }

        inline
        NodeAccess::Attributes ValueNodeAccess::getAttributes() const
        { return sharable::node(data()).getAttributes(); }

        inline
        bool ValueNodeAccess::isDefault()   const
        { return data().info.isDefault(); }

        inline
        bool ValueNodeAccess::isLocalized()   const
        { return data().info.isLocalized(); }

        inline
        uno::Any    ValueNodeAccess::getValue()      const
        { return data().getValue(m_aAccessor); }

        inline
        uno::Any    ValueNodeAccess::getUserValue()    const
        { return data().getUserValue(m_aAccessor); }

        inline
        uno::Any    ValueNodeAccess::getDefaultValue()    const
        { return data().getDefaultValue(m_aAccessor); }

    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_VALUENODEACCESS_HXX

