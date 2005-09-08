/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: schemabuilder.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:34:05 $
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

#ifndef CONFIGMGR_BACKEND_SCHEMABUILDER_HXX
#define CONFIGMGR_BACKEND_SCHEMABUILDER_HXX

#ifndef CONFIGMGR_BACKEND_MERGEDCOMPONENTDATA_HXX
#include "mergedcomponentdata.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_COMPONENTDATAHELPER_HXX
#include "componentdatahelper.hxx"
#endif
#ifndef CONFIGMGR_BACKEND_MERGEDDATAPROVIDER_HXX
#include "mergeddataprovider.hxx"
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMAHANDLER_HPP_
#include <com/sun/star/configuration/backend/XSchemaHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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

        using backenduno::MalformedDataException;
// -----------------------------------------------------------------------------

        typedef ::cppu::WeakImplHelper1<backenduno::XSchemaHandler> SchemaBuilder_Base;

        class SchemaBuilder
        :   public SchemaBuilder_Base
        {
        public:
            typedef uno::Reference< uno::XComponentContext > Context;

            SchemaBuilder(Context const & xContext, const OUString& aExpectedComponentName, MergedComponentData & rData, ITemplateDataProvider* aTemplateProvider = NULL );
            virtual ~SchemaBuilder();

        // checking the result
            bool isDone() const { return m_aContext.isDone(); }

            MergedComponentData &       result();
            MergedComponentData const & result() const;

        // XSchemaHandler
        public:
            virtual void SAL_CALL
                startSchema(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endSchema(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                importComponent( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startComponent( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endComponent(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startGroupTemplate( const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startSetTemplate( const TemplateIdentifier& aTemplate, sal_Int16 aAttributes, const TemplateIdentifier& aItemType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endTemplate(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startGroup( const OUString& aName, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                startSet( const OUString& aName, sal_Int16 aAttributes, const TemplateIdentifier& aItemType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithDefault( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aDefaultValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addInstance( const OUString& aName, const TemplateIdentifier& aTemplate )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addItemType( const TemplateIdentifier& aItemType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

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




