/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: layerupdatehandler.hxx,v $
 * $Revision: 1.8 $
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

#ifndef CONFIGMGR_BACKEND_LAYERUPDATEHANDLER_HXX
#define CONFIGMGR_BACKEND_LAYERUPDATEHANDLER_HXX

#include "sal/config.h"

#include "boost/utility.hpp"

#include "updatesvc.hxx"
#include "layerupdatebuilder.hxx"
#include "utility.hxx"

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------
        class LayerUpdateBuilder;

        class LayerUpdateHandler: private boost::noncopyable, public UpdateService
        {
        public:
            explicit
            LayerUpdateHandler(uno::Reference< uno::XComponentContext > const & _xContext);

            ~LayerUpdateHandler();

            // XUpdateHandler
            virtual void SAL_CALL
                startUpdate(  )
                    throw ( backenduno::MalformedDataException, lang::IllegalAccessException,
                            lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endUpdate(  )
                    throw ( backenduno::MalformedDataException, lang::IllegalAccessException,
                            lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyNode( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const rtl::OUString& aName, sal_Int16 aAttributes )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const rtl::OUString& aName, sal_Int16 aAttributes, const backenduno::TemplateIdentifier& aTemplate )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                removeNode( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyProperty( const rtl::OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const rtl::OUString& aLocale )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetPropertyValue( )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetPropertyValueForLocale( const rtl::OUString& aLocale )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetProperty( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceProperty( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplacePropertyWithValue( const rtl::OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                removeProperty( const rtl::OUString& aName )
                    throw (backenduno::MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        private:
            LayerUpdateBuilder & getUpdateBuilder();

            void checkBuilder(bool _bForProperty = false);

            void raiseMalformedDataException(sal_Char const * pMsg);
            void raiseNodeChangedBeforeException(sal_Char const * pMsg);
            void raisePropChangedBeforeException(sal_Char const * pMsg);
            void raisePropExistsException(sal_Char const * pMsg);

        private:
            LayerUpdateBuilder                          m_aBuilder;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




