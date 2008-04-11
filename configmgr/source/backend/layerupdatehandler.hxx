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
        using rtl::OUString;
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;
        namespace backenduno = ::com::sun::star::configuration::backend;

        using backenduno::TemplateIdentifier;
        using backenduno::MalformedDataException;
// -----------------------------------------------------------------------------
        class LayerUpdateBuilder;

        class LayerUpdateHandler : public UpdateService , Noncopyable
        {
        public:
            explicit
            LayerUpdateHandler(CreationArg _xContext);

            ~LayerUpdateHandler();

            // XUpdateHandler
            virtual void SAL_CALL
                startUpdate(  )
                    throw ( MalformedDataException, lang::IllegalAccessException,
                            lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endUpdate(  )
                    throw ( MalformedDataException, lang::IllegalAccessException,
                            lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyNode( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, sal_Bool bReset )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNode( const OUString& aName, sal_Int16 aAttributes )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceNodeFromTemplate( const OUString& aName, sal_Int16 aAttributes, const TemplateIdentifier& aTemplate )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endNode(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                removeNode( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                modifyProperty( const OUString& aName, sal_Int16 aAttributes, sal_Int16 aAttributeMask, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValue( const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                setPropertyValueForLocale( const uno::Any& aValue, const OUString& aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetPropertyValue( )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetPropertyValueForLocale( const OUString& aLocale )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endProperty(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                resetProperty( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplaceProperty( const OUString& aName, sal_Int16 aAttributes, const uno::Type& aType )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                addOrReplacePropertyWithValue( const OUString& aName, sal_Int16 aAttributes, const uno::Any& aValue )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                removeProperty( const OUString& aName )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

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




