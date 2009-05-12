/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: componentdatahelper.hxx,v $
 * $Revision: 1.11 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX
#define CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX

#include "valuenode.hxx"
#include "utility.hxx"
#include "stack.hxx"
#include <com/sun/star/configuration/backend/TemplateIdentifier.hpp>
#include <com/sun/star/configuration/backend/MalformedDataException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#include "mergeddataprovider.hxx"
#ifndef CONFIGMGR_LOGGER_HXX_
#include "logger.hxx"
#endif
#include "request.hxx"

namespace configmgr
{
// -----------------------------------------------------------------------------
    class OTreeNodeFactory;
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno        = ::com::sun::star::uno;
        namespace backenduno = ::com::sun::star::configuration::backend;

// -----------------------------------------------------------------------------

        class DataBuilderContext
        {
            Logger                      m_aLogger;
            Stack< ISubtree * >         m_aParentStack;
            rtl::OUString                    m_aActiveComponent;
            uno::XInterface *           m_pContext;
            rtl::OUString                   m_aExpectedComponentName;
            ITemplateDataProvider *     m_aTemplateProvider;
        public:
            explicit DataBuilderContext(uno::Reference< uno::XComponentContext > const & xContext);
            DataBuilderContext(uno::Reference< uno::XComponentContext > const & xContext, uno::XInterface * _pContext , ITemplateDataProvider*  aTemplateProvider = NULL);
            DataBuilderContext(uno::Reference< uno::XComponentContext > const & xContext, uno::XInterface * _pContext, const rtl::OUString& aExpectedComponentName,ITemplateDataProvider*  aTemplateProvider = NULL );
            DataBuilderContext(DataBuilderContext const & aBaseContext, uno::XInterface * _pContext);
            ~DataBuilderContext();

            bool        isDone() const;

            bool        hasActiveComponent()    const { return m_aActiveComponent.getLength() != 0; }
            rtl::OUString    getActiveComponent()    const { return m_aActiveComponent; }

            ISubtree &          getCurrentParent()
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
                { return implGetCurrentParent(); }

            ISubtree const &    getCurrentParent() const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
                { return implGetCurrentParent(); }

            node::Attributes    getCurrentAttributes() const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException))
                { return implGetCurrentParent().getAttributes(); }

            ITemplateDataProvider * getTemplateProvider() const
            { return  m_aTemplateProvider; }

            rtl::OUString getTemplateComponent(backenduno::TemplateIdentifier const & aItemType ) const;

            backenduno::TemplateIdentifier completeComponent(backenduno::TemplateIdentifier const & aItemType ) const;

            backenduno::TemplateIdentifier getCurrentItemType() const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            backenduno::TemplateIdentifier getValidItemType(backenduno::TemplateIdentifier const & aItemType) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void startActiveComponent(rtl::OUString const & _aComponent)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            void endActiveComponent()
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            bool isProperty(INode * pProp) const
                SAL_THROW((com::sun::star::uno::RuntimeException));

            bool isNode(INode * pNode) const SAL_THROW((com::sun::star::uno::RuntimeException))
            { return !isProperty(pNode); }

            void pushNode(ISubtree * pTree)
                SAL_THROW((com::sun::star::uno::RuntimeException));
            void popNode()
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            INode    * findProperty(rtl::OUString const & _aName)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            ISubtree * findNode(rtl::OUString const & _aName)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            bool isWritable(INode const * pNode) const
                SAL_THROW(());
            bool isRemovable(ISubtree const * pItem) const
                SAL_THROW(());

            ISubtree  * addNodeToCurrent(std::auto_ptr<ISubtree>  _aNode)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            ISubtree  * addLocalizedToCurrent(std::auto_ptr<ISubtree>  _aNode)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            ValueNode * addPropertyToCurrent(std::auto_ptr<ValueNode> _aNode, bool _bMayReplace = false)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void markCurrentMerged();

            // Logging support
            Logger const & getLogger() const { return m_aLogger; }

            rtl::OUString getNodeParentagePath() const;
            rtl::OUString getNodePath(rtl::OUString const & aNodeName) const;

            // Exception support
            void raiseMalformedDataException    (sal_Char const * _pText) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            void raiseIllegalArgumentException  (sal_Char const * _pText, sal_Int16 _nPos = 0) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            void raiseElementExistException     (sal_Char const * _pText, rtl::OUString const & _sElement) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            void raiseNoSuchElementException    (sal_Char const * _pText, rtl::OUString const & _sElement) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            void raisePropertyExistException    (sal_Char const * _pText, rtl::OUString const & _sElement) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            void raiseIllegalTypeException  (sal_Char const * _pText) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
            ResultHolder< TemplateInstance > getTemplateData (TemplateRequest const & _aRequest  );
        private:
            INode * findChild(rtl::OUString const & _aName)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            rtl::OUString makeMessageWithPath(sal_Char const * _pText) const
                SAL_THROW((com::sun::star::uno::RuntimeException));

            rtl::OUString makeMessageWithName(sal_Char const * _pText, rtl::OUString const & _aName) const
                SAL_THROW((com::sun::star::uno::RuntimeException));

            ISubtree & implGetCurrentParent() const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

        };
// -----------------------------------------------------------------------------

        class ComponentDataFactory
        {
            OTreeNodeFactory &          m_rNodeFactory;
        public:
            ComponentDataFactory();

            ComponentDataFactory(OTreeNodeFactory & _rNodeFactory)
            : m_rNodeFactory(_rNodeFactory)
            {}

        public:
            OTreeNodeFactory&   getNodeFactory() const { return m_rNodeFactory; }

            std::auto_ptr<ISubtree> createGroup(rtl::OUString const & _aName,
                                                bool _bExtensible,
                                                node::Attributes const & _aAttributes) const;

            std::auto_ptr<ISubtree> createSet( rtl::OUString const & _aName,
                                                backenduno::TemplateIdentifier const & aItemType,
                                                bool _bExtensible,
                                                node::Attributes const & _aAttributes) const;

            std::auto_ptr<ISubtree> createLocalizedContainer(rtl::OUString const & _aName,
                                                             uno::Type const & _aValueType,
                                                             node::Attributes const & _aAttributes) const;

            std::auto_ptr<ISubtree> createPlaceHolder(rtl::OUString const & _aName,
                                                        backenduno::TemplateIdentifier const & _aInstanceType) const;

            static bool isInstancePlaceHolder(ISubtree const & _aInstanceTree);
            static backenduno::TemplateIdentifier getInstanceType(ISubtree const & _aInstanceTree);
        };
// -----------------------------------------------------------------------------
    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




