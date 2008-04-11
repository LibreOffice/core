/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: setnodeaccess.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_SETNODEACCESS_HXX
#define CONFIGMGR_SETNODEACCESS_HXX

#include "nodeaccess.hxx"
#include "treefragment.hxx"

#include "treeaccessor.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class TreeAccessor;
    // -------------------------------------------------------------------------
        /** class that mediates access to the data of a Set node
            <p>Is a handle class with reference copy semantics.</p>
        */
        class SetNodeAccess
        {
        public:
            typedef TreeAddress    ElementAddress;
            typedef TreeAccessor   ElementAccess;

            SetNodeAccess(const sharable::SetNode *_pNodeRef)
                : m_pData((SetNodeAddress)_pNodeRef)
            {}

            SetNodeAccess(const sharable::Node *_pNodeRef)
                : m_pData(check(_pNodeRef))
            {}

            explicit
            SetNodeAccess(NodeAccess const & _aNode)
                : m_pData(check(_aNode))
            {}

            static bool isInstance(NodeAccess const & _aNode)
                { return check(_aNode) != NULL; }

            bool isValid() const { return m_pData != NULL; }

            configuration::Name getName() const;
            node::Attributes getAttributes() const;

            bool isDefault()   const;

            bool isLocalizedValueSetNode() const;

            configuration::Name getElementTemplateName()   const;
            configuration::Name getElementTemplateModule() const;

            bool            hasElement      (configuration::Name const& _aName) const
                { return SetNodeAccess::implGetElement(_aName) != NULL; }
            ElementAccess   getElementTree  (configuration::Name const& _aName) const
                { return TreeAccessor(implGetElement(_aName)); }

            operator NodeAccess() const { return NodeAccess(NodeAddress(m_pData)); }

            sharable::SetNode & data() const { return *m_pData; }
            operator SetNodeAddress () const { return (SetNodeAddress)m_pData; }
            operator NodeAddress () const { return (NodeAddress)m_pData; }

            static void addElement(SetNodeAddress _aSetAddress, ElementAddress _aNewElement);
            static ElementAddress removeElement(SetNodeAddress _aSetAddress,
                        configuration::Name const & _aName);
        private:
            static SetNodeAddress check(sharable::Node *pNode)
                { return pNode ? const_cast<SetNodeAddress>(pNode->setData()) : NULL; }
            static SetNodeAddress check(NodeAccess const&aRef)
                { return check(static_cast<sharable::Node *>(aRef)); }

            ElementAddress implGetElement(configuration::Name const& _aName) const;

            SetNodeAddress m_pData;
        };

        SetNodeAddress toSetNodeAddress(NodeAddress const & _aNodeAddr);
    // -------------------------------------------------------------------------
        inline
        configuration::Name SetNodeAccess::getName() const
        { return NodeAccess::wrapName( data().info.getName() ); }

        inline
        configuration::Name SetNodeAccess::getElementTemplateName()   const
        { return NodeAccess::wrapName( data().getElementTemplateName() ); }

        inline
        configuration::Name SetNodeAccess::getElementTemplateModule() const
        { return NodeAccess::wrapName( data().getElementTemplateModule() ); }

        inline
        node::Attributes SetNodeAccess::getAttributes() const
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

