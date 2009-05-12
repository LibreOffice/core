/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configgroup.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_CONFIGGROUP_HXX_
#define CONFIGMGR_CONFIGGROUP_HXX_

#include "configexcept.hxx"
#include "noderef.hxx"
#include "configdefaultprovider.hxx"

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configuration
    {
//-----------------------------------------------------------------------------

        class NodeChange;
        class RelativePath;
//-----------------------------------------------------------------------------

        /// helper for updating a <type>NodeRef</type> that refers to a Group
        class GroupUpdateHelper
        {
            rtl::Reference< Tree > m_aTree;
            NodeRef m_aNode;
        public:
            GroupUpdateHelper(rtl::Reference< Tree > const& aParentTree, NodeRef const& aGroupNode);
            ~GroupUpdateHelper() {}

            void validateNode(ValueRef const& aNode) const;
            void validateNode(NodeRef const& aNode) const;

            rtl::Reference< Tree > const& tree() const { return m_aTree; }
            NodeRef const& node() const { return m_aNode; }
        private:
            void implValidateTree(rtl::Reference< Tree > const& aTree) const;
            void implValidateNode(rtl::Reference< Tree > const& aTree, NodeRef const& aNode) const;
            void implValidateNode(rtl::Reference< Tree > const& aTree, ValueRef const& aNode) const;
        };
//-----------------------------------------------------------------------------
        /// allows to update values of a simple type within a <type>NodeRef</type> that refers to a Group
        class GroupUpdater
        {
            GroupUpdateHelper   m_aHelper;
            com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter>    m_xTypeConverter;
        public:
            GroupUpdater(rtl::Reference< Tree > const& aParentTree, NodeRef const& aGroupNode, com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> const& xConverter);

            NodeChange validateSetValue(ValueRef const& aValueNode, com::sun::star::uno::Any const& newValue );

        private:
            com::sun::star::uno::Any implValidateValue(rtl::Reference< Tree > const& aTree, ValueRef const& aNode, com::sun::star::uno::Any const& aValue) const;
        };
//-----------------------------------------------------------------------------

        /// allows to reset to default value or state members of a <type>NodeRef</type> that refers to a Group
        class GroupDefaulter
        {
            GroupUpdateHelper   m_aHelper;
            DefaultProvider     m_aDefaultProvider;
            bool                m_bHasDoneSet;
        public:
            GroupDefaulter(rtl::Reference< Tree > const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aProvider);

            bool hasDoneSet() const { return m_bHasDoneSet; }

            NodeChange validateSetToDefaultValue(ValueRef const& aValueNode);

            NodeChange validateSetToDefaultState(NodeRef const& aNode);

            NodeChanges validateSetAllToDefault();

            static bool isDataAvailable(rtl::Reference< Tree > const& _aParentTree, NodeRef const& _aGroupNode);
            static bool ensureDataAvailable(rtl::Reference< Tree > const& _aParentTree, NodeRef const& _aGroupNode, DefaultProvider const& _aSource);
        };
//-----------------------------------------------------------------------------
        bool isPossibleValueType(com::sun::star::uno::Type const& aValueType);
//-----------------------------------------------------------------------------
        bool convertCompatibleValue(com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> const& xConverter, uno::Any& rConverted,
                                    com::sun::star::uno::Any const& rNewValue, com::sun::star::uno::Type const& rTargetType);
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGGROUP_HXX_
