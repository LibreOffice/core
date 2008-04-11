/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layermerge.hxx,v $
 * $Revision: 1.10 $
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
        using backenduno::MalformedDataException;
// -----------------------------------------------------------------------------

        typedef ::cppu::WeakImplHelper1<backenduno::XLayerHandler> LayerMergeHandler_Base;

        class LayerMergeHandler
        :   public LayerMergeHandler_Base
        {
        public:
            typedef uno::Reference< uno::XComponentContext > Context;

            explicit
            LayerMergeHandler(Context const & _xContext, MergedComponentData & _rData, ITemplateDataProvider* aTemplateProvider = NULL);
            virtual ~LayerMergeHandler();

        // prepare merging
            void prepareLayer();
            bool prepareSublayer(OUString const & aLocale);

        // checking the result
            bool isDone() const { return m_aContext.isDone(); }
            bool isInSublayer() const { return m_bSublayer; }

            MergedComponentData &       result();
            MergedComponentData const & result() const;

        // XLayerHandler
        public:
            virtual void SAL_CALL
                startLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString & aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            void propagateAttributes(ISubtree & _rParent);

            node::Attributes makePropertyAttributes(sal_Int16 aSchemaAttributes)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void checkPropertyType(uno::Type const & _aType)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void applyPropertyValue(uno::Any const & _aValue)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void applyPropertyValue(uno::Any const & _aValue, OUString const & _aLocale)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void applyAttributes(INode * pNode, sal_Int16 aNodeAttributes)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void overrideLayerRoot( const OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            bool startOverride(INode * pNode, sal_Bool bClear) /* check if writable, mark merged */
                CFG_NOTHROW( );

            void implOverrideNode(
                ISubtree * node, sal_Int16 attributes, bool clear);

            void implAddOrReplaceNode(const OUString& aName, const TemplateIdentifier& aTemplate, sal_Int16 aAttributes)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void ensureUnchanged(INode const * pNode) const
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void setLocalizedValue(ISubtree * pProperty, uno::Any const & _aValue, OUString const & _aLocale)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );

            void setValueAndCheck(ValueNode & _rValueNode, uno::Any const & _aValue)
                CFG_UNO_THROW1( configuration::backend::MalformedDataException );
       private:
           void skipNode()          { ++m_nSkipping; }
           bool isSkipping() const  { return m_nSkipping != 0; }
           bool leaveSkippedNode()  { return m_nSkipping && m_nSkipping--; }
       private:
            struct Converter;
            MergedComponentData &   m_rData;
            DataBuilderContext      m_aContext;
            ComponentDataFactory    m_aFactory;
            OUString                m_aLocale;
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




