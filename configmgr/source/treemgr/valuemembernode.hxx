/*************************************************************************
 *
 *  $RCSfile: valuemembernode.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:40:28 $
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

#ifndef CONFIGMGR_VALUEMEMBERNODE_HXX_
#define CONFIGMGR_VALUEMEMBERNODE_HXX_

#include "nodeimpl.hxx"

namespace configmgr
{
    class ValueNode;

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
            typedef vos::ORef<DeferredImpl> DeferredImplRef;

            ValueNode* m_pOriginal;
            DeferredImplRef m_xDeferredOperation;
        private:
            friend class GroupNodeImpl;
            friend class DeferredGroupNodeImpl;
            friend class ValueMemberUpdate;

            /// create a ValueMemberNode for a given node
            explicit ValueMemberNode(ValueNode* pOriginal = NULL);
            /// create a deferred ValueMemberNode (xOriginal must not be empty)
            explicit ValueMemberNode(DeferredImplRef const& xOriginal);
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
            Attributes getAttributes()  const;

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
            ValueMemberNode m_aMemberNode;
        private:
            typedef ValueMemberNode::DeferredImplRef DeferredImplRef;
            friend class GroupNodeImpl;

            ValueMemberUpdate(ValueMemberNode const& rOriginal)
                : m_aMemberNode(rOriginal) {}

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
