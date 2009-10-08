/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: schemabuilder.hxx,v $
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

#ifndef CONFIGMGR_BACKEND_SCHEMABUILDER_HXX
#define CONFIGMGR_BACKEND_SCHEMABUILDER_HXX

#include "mergedcomponentdata.hxx"
#include "componentdatahelper.hxx"
#include "mergeddataprovider.hxx"
#include <com/sun/star/configuration/backend/XSchemaHandler.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase1.hxx>

namespace configmgr
{
// -----------------------------------------------------------------------------
    class OTreeNodeFactory;
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

        class SchemaBuilder
        :   public cppu::WeakImplHelper1<backenduno::XSchemaHandler>
        {
        public:
            SchemaBuilder(uno::Reference< uno::XComponentContext > const & xContext, const rtl::OUString& aExpectedComponentName, MergedComponentData & rData, ITemplateDataProvider* aTemplateProvider = NULL );
            virtual ~SchemaBuilder();

        // checking the result
            bool isDone() const { return m_aContext.isDone(); }

            MergedComponentData &       result();
            MergedComponentData const & result() const;

        // XSchemaHandler
        public:
            virtual void SAL_CALL
                startSchema(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endSchema(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                importComponent( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startComponent( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endComponent(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startGroupTemplate( const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startSetTemplate( const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes, const backenduno::TemplateIdentifier& aItemType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endTemplate(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startGroup( const rtl::OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startSet( const rtl::OUString& aName, sal_Int16 aAttributes, const backenduno::TemplateIdentifier& aItemType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithDefault( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aDefaultValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addInstance( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addItemType( const backenduno::TemplateIdentifier& aItemType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            static node::Attributes getComponentRootAttributes();
            static node::Attributes getTemplateBaseAttributes();
            node::Attributes getNodeAttributes() const;
            node::Attributes makePropertyAttributes(sal_Int16 aSchemaAttributes) const;

            bool isExtensible(sal_Int16 aSchemaAttributes);

            void substituteInstances();
        private:
            MergedComponentData &   m_aData;
            DataBuilderContext      m_aContext;
            ComponentDataFactory    m_aFactory;
        };
// -----------------------------------------------------------------------------

    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




