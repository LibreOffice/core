/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerdefaultremover.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_XML_LAYERDECORATOR_HXX
#define CONFIGMGR_XML_LAYERDECORATOR_HXX

#include <cppuhelper/implbase1.hxx>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#include <com/sun/star/configuration/backend/XLayerHandler.hpp>

// -----------------------------------------------------------------------------
namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        // -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace backenduno    = ::com::sun::star::configuration::backend;
        // -----------------------------------------------------------------------------

        class LayerDefaultRemover : public cppu::WeakImplHelper1<backenduno::XLayerHandler>
        {
        public:
            explicit
                LayerDefaultRemover(uno::Reference< backenduno::XLayerHandler > const & _xResultHandler);
            virtual ~LayerDefaultRemover();

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
                setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString& aLocale )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            void playBackNodeStack( bool bPlayProperty=false);
            void raiseMalformedDataException(sal_Char const * pMsg);
            inline bool hasPendingProperty();
            inline void clearPendingProperty();
        private:
            uno::Reference< backenduno::XLayerHandler > m_xResultHandler;
            std::vector<rtl::OUString> m_aNodeStack;
            struct PropertyStruct
            {
                rtl::OUString Name;
                uno::Type Type;
            }m_aPropName;
        };
        // -----------------------------------------------------------------------------
    } // namespace xml
    // -----------------------------------------------------------------------------

} // namespace configmgr
#endif
