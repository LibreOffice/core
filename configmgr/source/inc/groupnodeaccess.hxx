/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupnodeaccess.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:18:41 $
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

