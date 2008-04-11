/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: groupnodeaccess.hxx,v $
 * $Revision: 1.8 $
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

#ifndef CONFIGMGR_GROUPNODEACCESS_HXX
#define CONFIGMGR_GROUPNODEACCESS_HXX

#include "nodeaccess.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        class GroupNodeAccess
        {
        public:
            GroupNodeAccess(const sharable::GroupNode *_pNodeRef)
            : m_pData((GroupNodeAddress)_pNodeRef)
            {}

            explicit
            GroupNodeAccess(NodeAccess const & _aNode)
            : m_pData(check(_aNode))
            {}

            ~GroupNodeAccess() {}

            static bool isInstance(NodeAccess const & _aNode)
                { return check(_aNode) != NULL; }

            bool isValid() const { return m_pData != NULL; }

            configuration::Name getName() const;
            node::Attributes getAttributes() const;

            bool isDefault()   const;

            bool hasChild(configuration::Name const& _aName) const
                { return implGetChild(_aName) != NULL; }

            bool hasChildren() const
                { return m_pData->numDescendants > 0 ? true : false; }

            NodeAccess getChildNode(configuration::Name const& _aName) const
                { return NodeAccess(implGetChild(_aName)); }

            operator NodeAccess() const { return NodeAccess(NodeAddress(m_pData)); }

            sharable::GroupNode& data() const { return *m_pData; }
            operator GroupNodeAddress () const { return (GroupNodeAddress)m_pData; }

        private:
            NodeAddress implGetChild(configuration::Name const& _aName) const;
            static GroupNodeAddress check(sharable::Node *pNode)
                { return pNode ? const_cast<GroupNodeAddress>(pNode->groupData()) : NULL; }
            static GroupNodeAddress check(NodeAccess const&aRef)
                { return check(static_cast<sharable::Node *>(aRef)); }

            GroupNodeAddress m_pData;
        };

        GroupNodeAddress toGroupNodeAddress(NodeAddress const & _aNodeAddr);
    // -------------------------------------------------------------------------
        inline
        configuration::Name GroupNodeAccess::getName() const
        { return NodeAccess::wrapName( data().info.getName() ); }

        inline
        node::Attributes GroupNodeAccess::getAttributes() const
        { return sharable::node(data()).getAttributes(); }

        inline
        bool GroupNodeAccess::isDefault()   const
        { return data().info.isDefault(); }

    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_GROUPNODEACCESS_HXX

