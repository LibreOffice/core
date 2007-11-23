/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valuemembernode.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:48:26 $
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

#ifndef CONFIGMGR_VALUEMEMBERNODE_HXX_
#define CONFIGMGR_VALUEMEMBERNODE_HXX_

#include "nodeimpl.hxx"

#ifndef CONFIGMGR_VALUENODEACCESS_HXX
#include "valuenodeaccess.hxx"
#endif

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;
        typedef com::sun::star::uno::Type UnoType;

        class Name;
//-----------------------------------------------------------------------------
       /// handle class for values that are not nodes themselves, but members of a group
        class ValueMemberNode
        {
            class DeferredImpl;
            typedef rtl::Reference<DeferredImpl> DeferredImplRef;

            data::ValueNodeAccess m_aNodeRef;
            DeferredImplRef m_xDeferredOperation;
        private:
            friend class GroupNodeImpl;
            friend class DeferredGroupNodeImpl;
            friend class ValueMemberUpdate;

            /// create a ValueMemberNode for a given node
            explicit ValueMemberNode(data::ValueNodeAccess const& _aNodeAccess);
            /// create a deferred ValueMemberNode (xOriginal must not be empty)
            ValueMemberNode(DeferredImplRef const& _xDeferred);
        public:
            ValueMemberNode(ValueMemberNode const& rOriginal);
            ValueMemberNode& operator=(ValueMemberNode const& rOriginal);
            ~ValueMemberNode();

            /// does this wrap a valid value ?
            bool isValid() const;

            /// does this wrap a change
            bool hasChange() const;

            /// retrieve the name of the underlying node
            Name getNodeName() const;
            /// retrieve the attributes
            node::Attributes getAttributes()    const;

            /// Does this node assume its default value
            bool isDefault()        const;
            /// is the default value of this node available
            bool canGetDefaultValue() const;
            /// retrieve the current value of this node
            UnoAny  getValue()      const;
            /// retrieve the default value of this node
            UnoAny getDefaultValue() const;

            UnoType getValueType()  const;

        };
    //-------------------------------------------------------------------------

       /// handle class for updating values that are members of a group
        class ValueMemberUpdate
        {
            ValueMemberNode                     m_aMemberNode;
            view::ViewStrategy *                m_pStrategy;
        private:
            typedef ValueMemberNode::DeferredImplRef DeferredImplRef;
            friend class view::ViewStrategy;

            ValueMemberUpdate(ValueMemberNode const& rOriginal, view::ViewStrategy& _rStrategy)
                : m_aMemberNode(rOriginal), m_pStrategy(&_rStrategy)  {}

        public:
            /// does this wrap a valid value ?
            bool isValid() const  { return m_aMemberNode.isValid(); }

            /// get access to the wrapped data
            ValueMemberNode getNode() const { return m_aMemberNode; }

            /// Set this node to a new value
            void    setValue(UnoAny const& aNewValue);

            /// Set this node to assume its default value
            void    setDefault();
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
