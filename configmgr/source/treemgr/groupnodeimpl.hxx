/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: groupnodeimpl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:42:29 $
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

#ifndef CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
#define CONFIGMGR_GROUPNODEBEHAVIOR_HXX_

#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif

#ifndef CONFIGMGR_VALUEMEMBERNODE_HXX_
#include "valuemembernode.hxx"
#endif

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    class SubtreeChange;
    class ValueChange;
//-----------------------------------------------------------------------------

    namespace data
    {
        class GroupNodeAccess;
        class ValueNodeAccess;
    }
//-----------------------------------------------------------------------------

    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;
        typedef com::sun::star::uno::Type UnoType;

        class Name;
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
            explicit GroupNodeImpl(data::GroupNodeAddress _pNodeRef);

            data::GroupNodeAccess getDataAccess() const;

            bool areValueDefaultsAvailable() const;

            data::ValueNodeAccess getOriginalValueNode(Name const& aName) const;

            ValueMemberNode makeValueMember(data::ValueNodeAccess const& _aValueNode);
        };

//-----------------------------------------------------------------------------

        // domain-specific 'dynamic_cast' replacements
        GroupNodeImpl&  AsGroupNode(NodeImpl& rNode);

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
