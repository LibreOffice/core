/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nodeaccess.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:48:21 $
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

#ifndef CONFIGMGR_NODEACCESS_HXX
#define CONFIGMGR_NODEACCESS_HXX

#ifndef CONFIGMGR_NODEADDRESS_HXX
#include "nodeaddress.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif

#ifndef _CONFIGMGR_TREE_VALUENODE_HXX
#include "valuenode.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        using memory::Accessor;
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
        class NodeAccess
        {
        public:
            typedef configuration::Name     Name;
            typedef node::Attributes        Attributes;

            typedef NodeAddress                     NodeAddressType;
            typedef NodeAddress::AddressType        AddressType;
            typedef NodeAddress::DataType const     DataType;
            typedef DataType * NodePointerType;

            static NodeAccess emptyNode() { return NodeAccess(); }

            NodeAccess(Accessor const& _aAccessor, NodeAddressType const& _aNodeRef)
            : m_aAccessor(_aAccessor)
            , m_pData(_aNodeRef.m_pData)
            {}

            NodeAccess(Accessor const& _aAccessor, NodePointerType _pNode)
            : m_aAccessor(_aAccessor)
            , m_pData(_aAccessor.address(_pNode))
            {}

            bool isValid() const { return m_pData.is(); }
            bool isLocalRoot() const { return data().isFragmentRoot(); }

            Name getName() const { return wrapName( data().getName() ); }
            Attributes getAttributes() const { return data().getAttributes(); }

            bool isDefault()   const { return data().isDefault(); }
            bool isLocalized() const { return data().isLocalized(); }

            NodeAddressType address() const { return NodeAddressType(m_pData); }
            Accessor const& accessor() const { return m_aAccessor; }

            DataType& data() const { return *static_cast<NodePointerType>(m_aAccessor.validate(m_pData)); }
            NodePointerType getDataPtr() const { return static_cast<NodePointerType>(m_aAccessor.access(m_pData)); }

            AddressType rawAddress() const { return m_pData; }

            static Name wrapName(rtl::OUString const& _aNameString)
            { return configuration::makeName( _aNameString, Name::NoValidate() ); }

            static NodeAddress::DataType* access(NodeAddressType const& _aNodeRef, memory::UpdateAccessor& _rUpdateAccess);
            static NodeAddress::DataType const* access(NodeAddressType const& _aNodeRef, Accessor const& _rReaderAccess)
            { return static_cast<NodePointerType>(_rReaderAccess.access(_aNodeRef.m_pData)); }
        private:
            NodeAccess() : m_aAccessor(NULL), m_pData() {}

            Accessor    m_aAccessor;
            AddressType m_pData;
        };
    // -------------------------------------------------------------------------
        class NodeAccessRef
        {
        public:
            typedef NodeAccess::Name                Name;
            typedef NodeAccess::Attributes          Attributes;

            typedef NodeAccess::NodeAddressType     NodeAddressType;
            typedef NodeAccess::AddressType         AddressType;
            typedef NodeAccess::DataType            DataType;
            typedef NodeAccess::NodePointerType     NodePointerType;

            NodeAccessRef(NodeAccess const& _aNodeAccess)
            : m_pAccessor(&_aNodeAccess.accessor())
            , m_pData(_aNodeAccess.rawAddress())
            {}

            NodeAccessRef(Accessor const * _pAccessor, NodeAddressType const& _aNodeRef)
            : m_pAccessor(_pAccessor)
            , m_pData(_aNodeRef.m_pData)
            {}

            NodeAccessRef(Accessor const * _pAccessor, NodePointerType _pNode)
            : m_pAccessor(_pAccessor)
            , m_pData(_pAccessor->address(_pNode))
            {}

            NodeAccess toNodeAccess () const { return NodeAccess(accessor(),address()); }

            bool isValid() const { return m_pData.is(); }
            bool isLocalRoot() const { return data().isFragmentRoot(); }

            Name getName() const { return NodeAccess::wrapName( data().getName() ); }
            Attributes getAttributes() const { return data().getAttributes(); }

            bool isDefault()   const { return data().isDefault(); }
            bool isLocalized() const { return data().isLocalized(); }

            NodeAddressType address() const { return NodeAddressType(m_pData); }
            Accessor const& accessor() const { return *m_pAccessor; }

            DataType& data() const { return *static_cast<NodePointerType>(m_pAccessor->validate(m_pData)); }
            NodePointerType getDataPtr() const { return static_cast<NodePointerType>(m_pAccessor->access(m_pData)); }

            AddressType rawAddress() const { return m_pData; }

        private:
            Accessor const *   m_pAccessor;
            AddressType m_pData;
        };
    // -------------------------------------------------------------------------
    // helper - finds child or element
        NodeAccess  getSubnode(NodeAccessRef const & _aNode, NodeAccess::Name const & _aName);
        NodeAddress getSubnodeAddress(memory::Accessor const& _aAccess, NodeAddress const & _aNodeAddress, NodeAccess::Name const & _aName);
        NodeAddress getSubnodeAddress(memory::UpdateAccessor& _aAccess, NodeAddress const & _aNodeAddress, NodeAccess::Name const & _aName);
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_NODEACCESS_HXX

