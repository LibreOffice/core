/*************************************************************************
 *
 *  $RCSfile: setnodeaccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:34:12 $
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

#ifndef CONFIGMGR_SETNODEACCESS_HXX
#define CONFIGMGR_SETNODEACCESS_HXX

#ifndef CONFIGMGR_NODEACCESS_HXX
#include "nodeaccess.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class TreeAddress;
        class TreeAccessor;
    // -------------------------------------------------------------------------
        /** class that mediates access to the data of a Set node
            <p>Is a handle class with reference copy semantics.</p>
        */
        class SetNodeAccess
        {
        public:
            typedef NodeAccess::Name        Name;
            typedef NodeAccess::Attributes  Attributes;
            typedef SetNodeAddress                    NodeAddressType;
            typedef SetNodeAddress::AddressType       AddressType;
            typedef SetNodeAddress::DataType const    DataType;
            typedef DataType * NodePointerType;
            typedef TreeAddress                       ElementAddress;
            typedef TreeAccessor                      ElementAccess;

            SetNodeAccess(Accessor const& _aAccessor, NodeAddressType const& _aNodeRef)
            : m_aAccessor(_aAccessor)
            , m_pData(_aNodeRef.m_pData)
            {}

            SetNodeAccess(Accessor const& _aAccessor, NodePointerType _pNode)
            : m_aAccessor(_aAccessor)
            , m_pData(check(_aAccessor,_pNode))
            {}

            explicit
            SetNodeAccess(NodeAccess const & _aNode)
            : m_aAccessor(_aNode.accessor())
            , m_pData(check(_aNode))
            {
            }

            explicit
            SetNodeAccess(NodeAccessRef const & _aNode)
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

            bool isLocalizedValueSetNode() const;

            Name getElementTemplateName()   const;
            Name getElementTemplateModule() const;

            bool            hasElement      (Name const& _aName) const;
            ElementAccess   getElementTree  (Name const& _aName) const;

            NodeAddressType address()   const { return NodeAddressType(m_pData); }
            Accessor const& accessor()  const { return m_aAccessor; }

            operator NodeAccessRef() const { return NodeAccessRef(&m_aAccessor,NodeAddress(m_pData)); }

            DataType& data() const { return *static_cast<NodePointerType>(m_aAccessor.validate(m_pData)); }

            static void addElement(memory::UpdateAccessor & _aAccessor, SetNodeAddress _aSetAddress, ElementAddress _aNewElement);
            static ElementAddress removeElement(memory::UpdateAccessor & _aAccessor, SetNodeAddress _aSetAddress, Name const & _aName);
        private:
            static AddressType check(NodeAccessRef const&);
            static AddressType check(Accessor const&, NodePointerType);

            ElementAddress implGetElement(Name const& _aName) const;

            Accessor    m_aAccessor;
            AddressType m_pData;
        };

        SetNodeAddress toSetNodeAddress(memory::Accessor const & _aAccess, NodeAddress const & _aNodeAddr);
        SetNodeAddress toSetNodeAddress(memory::UpdateAccessor & _aAccess, NodeAddress const & _aNodeAddr);
    // -------------------------------------------------------------------------
        inline
        NodeAccess::Name SetNodeAccess::getName() const
        { return NodeAccess::wrapName( data().info.getName(m_aAccessor) ); }

        inline
        NodeAccess::Name SetNodeAccess::getElementTemplateName()   const
        { return NodeAccess::wrapName( data().getElementTemplateName(m_aAccessor) ); }

        inline
        NodeAccess::Name SetNodeAccess::getElementTemplateModule() const
        { return NodeAccess::wrapName( data().getElementTemplateModule(m_aAccessor) ); }

        inline
        NodeAccess::Attributes SetNodeAccess::getAttributes() const
        { return sharable::node(data()).getAttributes(); }

        inline
        bool SetNodeAccess::isDefault()   const
        { return data().info.isDefault(); }

        inline
        bool SetNodeAccess::isLocalizedValueSetNode() const
        { return data().isLocalizedValue(); }
    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_SETNODEACCESS_HXX

