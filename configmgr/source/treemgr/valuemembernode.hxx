/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valuemembernode.hxx,v $
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

#ifndef CONFIGMGR_VALUEMEMBERNODE_HXX_
#define CONFIGMGR_VALUEMEMBERNODE_HXX_

#include "nodeimpl.hxx"

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------
       /// handle class for values that are not nodes themselves, but members of a group
        class ValueMemberNode
        {
        public:
            class DeferredImpl;
        private:
            sharable::ValueNode * m_node;
            rtl::Reference<DeferredImpl> m_xDeferredOperation;
        private:
            friend class GroupNodeImpl;
            friend class DeferredGroupNodeImpl;
            friend class ValueMemberUpdate;

            /// create a ValueMemberNode for a given node
            explicit ValueMemberNode(sharable::ValueNode * node);
            /// create a deferred ValueMemberNode (xOriginal must not be empty)
            ValueMemberNode(rtl::Reference<DeferredImpl> const& _xDeferred);
        public:
            ValueMemberNode(ValueMemberNode const& rOriginal);
            ValueMemberNode& operator=(ValueMemberNode const& rOriginal);
            ~ValueMemberNode();

            /// does this wrap a valid value ?
            bool isValid() const;

            /// does this wrap a change
            bool hasChange() const;

            /// retrieve the name of the underlying node
            rtl::OUString getNodeName() const;
            /// retrieve the attributes
            node::Attributes getAttributes()    const;

            /// Does this node assume its default value
            bool isDefault()        const;
            /// is the default value of this node available
            bool canGetDefaultValue() const;
            /// retrieve the current value of this node
            com::sun::star::uno::Any    getValue()      const;
            /// retrieve the default value of this node
            com::sun::star::uno::Any getDefaultValue() const;

            com::sun::star::uno::Type   getValueType()  const;

        };
    //-------------------------------------------------------------------------

       /// handle class for updating values that are members of a group
        class ValueMemberUpdate
        {
            ValueMemberNode                     m_aMemberNode;
            view::ViewStrategy *                m_pStrategy;
        private:
            friend class view::ViewStrategy;

            ValueMemberUpdate(ValueMemberNode const& rOriginal, view::ViewStrategy& _rStrategy)
                : m_aMemberNode(rOriginal), m_pStrategy(&_rStrategy)  {}

        public:
            /// does this wrap a valid value ?
            bool isValid() const  { return m_aMemberNode.isValid(); }

            /// get access to the wrapped data
            ValueMemberNode getNode() const { return m_aMemberNode; }

            /// Set this node to a new value
            void    setValue(com::sun::star::uno::Any const& aNewValue);

            /// Set this node to assume its default value
            void    setDefault();
        };

//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_GROUPNODEBEHAVIOR_HXX_
