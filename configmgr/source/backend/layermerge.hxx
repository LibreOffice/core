/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layermerge.hxx,v $
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

#ifndef CONFIGMGR_BACKEND_LAYERMERGE_HXX
#define CONFIGMGR_BACKEND_LAYERMERGE_HXX

#include "mergedcomponentdata.hxx"
#include "componentdatahelper.hxx"
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

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

        class LayerMergeHandler
        :   public cppu::WeakImplHelper1<backenduno::XLayerHandler>
        {
        public:
            explicit
            LayerMergeHandler(uno::Reference< uno::XComponentContext > const & _xContext, MergedComponentData & _rData, ITemplateDataProvider* aTemplateProvider = NULL);
            virtual ~LayerMergeHandler();

        // prepare merging
            void prepareLayer();
            bool prepareSublayer(rtl::OUString const & aLocale);

        // checking the result
            bool isDone() const { return m_aContext.isDone(); }
            bool isInSublayer() const { return m_bSublayer; }

        // XLayerHandler
        public:
            virtual void SAL_CALL
                startLayer(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString & aLocale )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            void propagateAttributes(ISubtree & _rParent);

            node::Attributes makePropertyAttributes(sal_Int16 aSchemaAttributes)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void checkPropertyType(uno::Type const & _aType)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void applyPropertyValue(uno::Any const & _aValue)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void applyPropertyValue(uno::Any const & _aValue, rtl::OUString const & _aLocale)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void applyAttributes(INode * pNode, sal_Int16 aNodeAttributes)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void overrideLayerRoot( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            bool startOverride(INode * pNode, sal_Bool bClear) /* check if writable, mark merged */
                SAL_THROW(());

            void implOverrideNode(
                ISubtree * node, sal_Int16 attributes, bool clear);

            void implAddOrReplaceNode(const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void ensureUnchanged(INode const * pNode) const
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void setLocalizedValue(ISubtree * pProperty, uno::Any const & _aValue, rtl::OUString const & _aLocale)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));

            void setValueAndCheck(ValueNode & _rValueNode, uno::Any const & _aValue)
                SAL_THROW((com::sun::star::configuration::backend::MalformedDataException , com::sun::star::uno::RuntimeException));
       private:
           void skipNode()          { ++m_nSkipping; }
           bool isSkipping() const  { return m_nSkipping != 0; }
           bool leaveSkippedNode()  { return m_nSkipping && m_nSkipping--; }
       private:
            struct Converter;
            MergedComponentData &   m_rData;
            DataBuilderContext      m_aContext;
            ComponentDataFactory    m_aFactory;
            rtl::OUString                m_aLocale;
            INode *                 m_pProperty;
            Converter *             m_pConverter;

            sal_uInt32              m_nSkipping;
            bool                    m_bSublayer;
        };
// -----------------------------------------------------------------------------

        /// change attributes and states of the tree to make it a proper default layer
        void promoteToDefault(MergedComponentData & _rTree);
// -----------------------------------------------------------------------------
    } // namespace backend
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




