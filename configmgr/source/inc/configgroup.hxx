/*************************************************************************
 *
 *  $RCSfile: configgroup.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:18:56 $
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

#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_CONFIG_DEFAULTPROVIDER_HXX_
#include "configdefaultprovider.hxx"
#endif

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
        typedef com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter>  UnoTypeConverter;
//-----------------------------------------------------------------------------

        class NodeChange;
        class RelativePath;
//-----------------------------------------------------------------------------

        /// helper for updating a <type>NodeRef</type> that refers to a Group
        class GroupUpdateHelper
        {
            Tree    m_aTree;
            NodeRef m_aNode;
        public:
            GroupUpdateHelper(Tree const& aParentTree, NodeRef const& aGroupNode);
            ~GroupUpdateHelper() {}

            void validateNode(ValueRef const& aNode) const;
            void validateNode(NodeRef const& aNode) const;

            Tree    const& tree() const { return m_aTree; }
            NodeRef const& node() const { return m_aNode; }
        private:
            void implValidateTree(Tree const& aTree) const;
            void implValidateNode(Tree const& aTree, NodeRef const& aNode) const;
            void implValidateNode(Tree const& aTree, ValueRef const& aNode) const;
        };
//-----------------------------------------------------------------------------
        /// allows to update values of a simple type within a <type>NodeRef</type> that refers to a Group
        class GroupUpdater
        {
            GroupUpdateHelper   m_aHelper;
            UnoTypeConverter    m_xTypeConverter;
        public:
            GroupUpdater(Tree const& aParentTree, NodeRef const& aGroupNode, UnoTypeConverter const& xConverter);

            NodeChange validateSetValue(ValueRef const& aValueNode, UnoAny const& newValue );

        private:
            UnoAny implValidateValue(Tree const& aTree, ValueRef const& aNode, UnoAny const& aValue) const;
        };
//-----------------------------------------------------------------------------

        /// allows to reset to default value or state members of a <type>NodeRef</type> that refers to a Group
        class GroupDefaulter
        {
            GroupUpdateHelper   m_aHelper;
            DefaultProvider     m_aDefaultProvider;
            bool                m_bHasDoneSet;
        public:
            GroupDefaulter(Tree const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aProvider);

            bool hasDoneSet() const { return m_bHasDoneSet; }

            bool isDataAvailable();

            NodeChange validateSetToDefaultValue(ValueRef const& aValueNode);

            NodeChange validateSetToDefaultState(NodeRef const& aNode);

            NodeChanges validateSetAllToDefault();

            static bool isDataAvailable(TreeRef const& _aParentTree, NodeRef const& _aGroupNode);
            static bool ensureDataAvailable(TreeRef const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aSource);
        };
//-----------------------------------------------------------------------------
        bool isPossibleValueType(UnoType const& aValueType);
//-----------------------------------------------------------------------------
        bool convertCompatibleValue(UnoTypeConverter const& xConverter, uno::Any& rConverted,
                                    UnoAny const& rNewValue, UnoType const& rTargetType);
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGGROUP_HXX_
