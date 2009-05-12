/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: configset.hxx,v $
 * $Revision: 1.18 $
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

#ifndef CONFIGMGR_CONFIGSET_HXX_
#define CONFIGMGR_CONFIGSET_HXX_

#include "configexcept.hxx"
#include "configdefaultprovider.hxx"
#include "template.hxx"
#include "noderef.hxx"
#include <rtl/ref.hxx>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace com { namespace sun { namespace star {
    namespace script { class XTypeConverter; }
} } }

namespace configmgr
{
    namespace data { class TreeSegment; }

    namespace configuration
    {
        namespace Path { class Component; }
        //---------------------------------------------------------------------

        class ElementTree;

        class NodeChange;
        class Template;

        class SetElementFactory
        {
            TemplateProvider m_aProvider;
        public:
            SetElementFactory(TemplateProvider const& aProvider);
            SetElementFactory(SetElementFactory const& aOther);
            SetElementFactory& operator=(SetElementFactory const& aOther);
            ~SetElementFactory();

            rtl::Reference< ElementTree > instantiateTemplate(rtl::Reference<Template> const& aTemplate);
            rtl::Reference< ElementTree > instantiateOnDefault(rtl::Reference< data::TreeSegment > const& _aTree, rtl::Reference<Template> const& aDummyTemplate);

            static TemplateProvider findTemplateProvider(rtl::Reference< Tree > const& aTree, NodeRef const& aNode);
        };

//-----------------------------------------------------------------------------

        /// allows to insert,remove and replace an element of a <type>Node</type> that is a Container ("set") of full-fledged trees.
        class TreeSetUpdater
        {
            rtl::Reference< Tree > m_aParentTree;
            NodeRef m_aSetNode;
            rtl::Reference<Template>    m_aTemplate;
        public:
            TreeSetUpdater(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode, rtl::Reference< Template > const& aTemplate);

            NodeChange validateInsertElement (rtl::OUString const& aName, rtl::Reference< ElementTree > const& aNewElement);

            NodeChange validateReplaceElement(rtl::Reference< ElementTree > const& aElement, rtl::Reference< ElementTree > const& aNewElement);

            NodeChange validateRemoveElement (rtl::Reference< ElementTree > const& aElement);
        private:
            void                implValidateSet();
            Path::Component     implValidateElement(rtl::Reference< ElementTree > const& aElement, bool bReqRemovable);
            void                implValidateTree(rtl::Reference< ElementTree > const& aElementTree);
        };
//-----------------------------------------------------------------------------
        /// allows to insert,remove and replace an element of a <type>Node</type> that is a Container ("set") of simple values.
        class ValueSetUpdater
        {
            rtl::Reference< Tree > m_aParentTree;
            NodeRef m_aSetNode;
            rtl::Reference<Template>    m_aTemplate;
            com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> m_xTypeConverter;
        public:
            ValueSetUpdater(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode,
                            rtl::Reference< Template > const& aTemplate, com::sun::star::uno::Reference<com::sun::star::script::XTypeConverter> const& xConverter);

            NodeChange validateInsertElement (rtl::OUString const& aName, com::sun::star::uno::Any const& aNewValue);

            NodeChange validateReplaceElement(rtl::Reference< ElementTree > const& aElement, com::sun::star::uno::Any const& aNewValue);

            NodeChange validateRemoveElement (rtl::Reference< ElementTree > const& aElement);
        private:
            void implValidateSet();
            Path::Component     implValidateElement(rtl::Reference< ElementTree > const& aElement, bool bReqRemovable);
            com::sun::star::uno::Any implValidateValue(rtl::Reference< Tree > const& aElementTree, com::sun::star::uno::Any const& aValue);
            com::sun::star::uno::Any implValidateValue(com::sun::star::uno::Any const& aValue);

            rtl::Reference<ElementTree> makeValueElement(rtl::OUString const& aName, rtl::Reference< Tree > const& aElementTree, com::sun::star::uno::Any const& aValue, bool bInserting);
            rtl::Reference<ElementTree> makeValueElement(rtl::OUString const& aName, com::sun::star::uno::Any const& aValue, bool bInserting);
            rtl::Reference< Tree > extractElementNode(rtl::Reference< ElementTree > const& aElement);
        };
//-----------------------------------------------------------------------------

        /// allows to restore to its default state a <type>Node</type> that is a Container ("set") of full-fledged trees.
        class SetDefaulter
        {
            rtl::Reference< Tree > m_aParentTree;
            NodeRef m_aSetNode;
            DefaultProvider m_aDefaultProvider;
        public:
            SetDefaulter(rtl::Reference< Tree > const& aParentTree, NodeRef const& aSetNode, DefaultProvider const& aDefaultProvider);

            NodeChange validateSetToDefaultState();

        private:
            void implValidateSet();
        };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGSET_HXX_
