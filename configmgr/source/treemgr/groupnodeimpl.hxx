/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: groupnodeimpl.hxx,v $
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

#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#define CONFIGMGR_GROUPNODEBEHAVIOR_HXX_

#include "nodeimpl.hxx"
#include "valuemembernode.hxx"

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class SubtreeChange;
    class ValueChange;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        class ValueChangeImpl;
//-----------------------------------------------------------------------------
// a visitor
//-----------------------------------------------------------------------------
        struct GroupMemberVisitor
        {
            enum Result { DONE, CONTINUE };
            virtual Result visit(ValueMemberNode const& aValue) = 0;
        protected:
            virtual ~GroupMemberVisitor() {}
        };


// Specific type of nodes
//-----------------------------------------------------------------------------

        class GroupNodeImpl : public NodeImpl
        {
            mutable sharable::Node *m_pCache;
        public:
            explicit GroupNodeImpl(sharable::GroupNode * _pNodeRef);

            sharable::GroupNode * getDataAccess() const;

            bool areValueDefaultsAvailable() const;

            sharable::ValueNode * getOriginalValueNode(rtl::OUString const& aName) const;

            ValueMemberNode makeValueMember(sharable::ValueNode * node);
        };

//-----------------------------------------------------------------------------

        // domain-specific 'dynamic_cast' replacements
        GroupNodeImpl&  AsGroupNode(NodeImpl& rNode);

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
