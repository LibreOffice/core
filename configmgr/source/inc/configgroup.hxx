/*************************************************************************
 *
 *  $RCSfile: configgroup.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2001-06-20 20:27:17 $
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

#ifndef CONFIGMGR_CONFIGGROUP_HXX_
#define CONFIGMGR_CONFIGGROUP_HXX_

#include "apitypes.hxx"
#include "configexcept.hxx"
#include "noderef.hxx"

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
    namespace configuration
    {
//-----------------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
        typedef com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter>  UnoTypeConverter;
//-----------------------------------------------------------------------------

        class NodeRef;
        class NodeChange;
        class RelativePath;
//-----------------------------------------------------------------------------

        /// allows to update values of a simple type within a <type>NodeRef</type> that refers to a Group
        class GroupUpdater
        {
            Tree    m_aTree;
            NodeRef m_aNode;
            UnoTypeConverter m_xTypeConverter;
        public:
            GroupUpdater(Tree const& aParentTree, NodeRef const& aGroupNode, UnoTypeConverter const& xConverter);

            NodeChange validateSetDefault(AnyNodeRef const& aNode);

            NodeChange validateSetDefault(ValueRef const& aValueNode);

            NodeChange validateSetValue(ValueRef const& aValueNode, UnoAny const& newValue );

            NodeChange validateSetDeepValue(Tree const& aNestedTree, ValueRef const& aNestedValueNode,
                                            RelativePath const& aRelPath,UnoAny const& newValue);

        private:
            void implValidateTree(Tree const& aTree) const;
            void implValidateGroup(Tree const& aTree, NodeRef const& aNode) const;
            void implValidateNode(Tree const& aTree, ValueRef const& aNode) const;
            UnoAny implValidateValue(Tree const& aTree, ValueRef const& aNode, UnoAny const& aValue) const;
        };
//-----------------------------------------------------------------------------
        bool convertCompatibleValue(UnoTypeConverter const& xConverter, uno::Any& rConverted,
                                    UnoAny const& rNewValue, UnoType const& rTargetType);
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGGROUP_HXX_
