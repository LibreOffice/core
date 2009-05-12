/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: emptylayerimpl.hxx,v $
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

#ifndef CONFIGMGR_BACKEND_EMPTYLAYERIMPL_HXX
#define CONFIGMGR_BACKEND_EMPTYLAYERIMPL_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>

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

        class EmptyLayer : public cppu::WeakImplHelper1<backenduno::XLayer>
        {
        public:
            virtual ~EmptyLayer();

            // XLayer
        public:
            virtual void SAL_CALL
                readData( const uno::Reference< backenduno::XLayerHandler >& aHandler )
                    throw (backenduno::MalformedDataException, lang::NullPointerException, lang::WrappedTargetException, uno::RuntimeException);
        };
        // -----------------------------------------------------------------------------

        class RequireEmptyLayer : public cppu::WeakImplHelper1<backenduno::XLayerHandler>
        {
        public:
            RequireEmptyLayer();
            virtual ~RequireEmptyLayer();

            bool wasEmpty()     const { return m_bEmpty; }
            bool wasInvalid()   const { return m_bInvalid; }
            // XLayerHandler
        public:
            virtual void SAL_CALL
                startLayer(  )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Bool bClear )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const rtl::OUString& aName, const backenduno::TemplateIdentifier& aTemplate, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                dropNode( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                overrideProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType, sal_Bool bClear )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                addPropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString& aLocale )
                    throw (backenduno::MalformedDataException, uno::RuntimeException);

        private:
            void failNotEmpty() { fail("layer is not empty"); }
            void fail(sal_Char const * pMsg);

        private:
            bool m_bStarted;
            bool m_bInvalid;
            bool m_bEmpty;
        };
        // -----------------------------------------------------------------------------
    } // namespace xml
    // -----------------------------------------------------------------------------

} // namespace configmgr
#endif
