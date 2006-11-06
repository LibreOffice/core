/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeaccessor.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:48:46 $
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

#ifndef CONFIGMGR_TREEACCESSOR_HXX
#define CONFIGMGR_TREEACCESSOR_HXX

#ifndef CONFIGMGR_TREEADDRESS_HXX
#include "treeaddress.hxx"
#endif
#ifndef CONFIGMGR_ACCESSOR_HXX
#include "accessor.hxx"
#endif
#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif

#ifndef INCLUDED_CSTDDEF
#include <cstddef>
#define INCLUDED_CSTDDEF
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------

    namespace memory { class UpdateAccessor; }
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        using memory::Accessor;
    // -------------------------------------------------------------------------
        class ValueNodeAccess;
        class GroupNodeAccess;
        class SetNodeAccess;
    // -------------------------------------------------------------------------
        /** class that mediates access to the data of a tree fragment
        */
        class TreeAccessor
        {
        public:
            typedef configuration::Name     Name;
            typedef node::Attributes        Attributes;
            typedef TreeAddress                 DataAddressType;
            typedef TreeAddress::DataType const DataType;
            typedef DataType  * DataPointerType;

            static TreeAccessor emptyTree() { return TreeAccessor(); }

            TreeAccessor(Accessor const& _aAccessor, DataAddressType const& _aTreeRef)
            : m_aAccessor(_aAccessor)
            , m_pBase(_aTreeRef.m_pData)
            {}

            TreeAccessor(Accessor const& _aAccessor, DataPointerType _pTree)
            : m_aAccessor(_aAccessor)
            , m_pBase(_aAccessor.address(_pTree))
            {}

            bool isValid() const { return m_pBase.is(); }

            bool isDefault() const { return data().isDefault(); }

            Attributes getAttributes() const { return data().getAttributes(); }
            Name getName() const;

            NodeAccessRef getRootNode() const { return NodeAccessRef(&m_aAccessor,rootAddress(m_pBase)); }

            DataAddressType address() const { return m_pBase; }
            Accessor const& accessor() const { return m_aAccessor; }

            DataType& data() const { return *static_cast<DataPointerType>(m_aAccessor.validate(m_pBase.m_pData)); }
            DataPointerType getDataPtr() const { return access(m_pBase,m_aAccessor); }

            TreeAddress copyTree(memory::UpdateAccessor & _aTargetSpace) const;
            static void freeTree(memory::UpdateAccessor & _aTargetSpace, TreeAddress _aTree);

            static Name wrapName(rtl::OUString const& _aNameString)
            { return configuration::makeName( _aNameString, Name::NoValidate() ); }

            static TreeAddress::DataType* access(DataAddressType const& _aTreeRef, memory::UpdateAccessor& _rUpdateAccess);
            static TreeAddress::DataType const* access(DataAddressType const& _aTreeRef, Accessor const& _rReaderAccess)
            { return static_cast<DataPointerType>(_rReaderAccess.access(_aTreeRef.m_pData)); }
        private:
            NodeAddress rootAddress(DataAddressType const& p) const;

            TreeAccessor();
        private:
            Accessor            m_aAccessor;
            DataAddressType     m_pBase;
        };
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------
        inline
        TreeAccessor::Name TreeAccessor::getName() const
        {
            return wrapName( data().getName() );
        }
    // -------------------------------------------------------------------------
        inline
        NodeAddress TreeAccessor::rootAddress(DataAddressType const& p) const
        {
            sharable::Address aAddr = p.addressValue();

            if (aAddr) aAddr += offsetof(TreeAddress::DataType,nodes);

            return NodeAddress( memory::Pointer(aAddr) );
        }
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_TREEACCESSOR_HXX

