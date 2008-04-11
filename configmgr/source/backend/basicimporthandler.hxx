/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: basicimporthandler.hxx,v $
 * $Revision: 1.5 $
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

#ifndef CONFIGMGR_BACKEND_BASICIMPORTHANDLER_HXX
#define CONFIGMGR_BACKEND_BASICIMPORTHANDLER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#include <com/sun/star/configuration/backend/XBackend.hpp>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        using rtl::OUString;
        namespace uno        = ::com::sun::star::uno;
        namespace lang       = ::com::sun::star::lang;
        namespace backenduno = ::com::sun::star::configuration::backend;
        using backenduno::MalformedDataException;
        using backenduno::TemplateIdentifier;
// -----------------------------------------------------------------------------

        class BasicImportHandler : public cppu::WeakImplHelper1< backenduno::XLayerHandler >
        {
        public:
            typedef uno::Reference< backenduno::XBackend >          Backend;

            explicit
            BasicImportHandler(){}

            BasicImportHandler(Backend const & xBackend,OUString const & aEntity = OUString(), const sal_Bool& bNofity= sal_False);
            ~BasicImportHandler();

        // XLayerHandler subset - call these implementations from your derived class implementations
        protected:
            virtual void SAL_CALL
                startLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

            virtual void SAL_CALL
                endLayer(  )
                    throw (MalformedDataException, lang::WrappedTargetException, uno::RuntimeException);

        protected:
            bool hasComponent() const { return m_aComponentName.getLength() != 0; }
            bool hasEntity()    const { return m_aEntity.getLength() != 0; }

            OUString getComponent() const { return m_aComponentName; }
            OUString getEntity()    const { return m_aEntity; }
            Backend  getBackend()   const { return m_xBackend; }

            bool startComponent( const OUString& aName );

            void raiseMalformedDataException(sal_Char const * pMsg);
            /** If True, notification should be send to backend
            */
            sal_Bool        m_bSendNotification;
        private:
            Backend const   m_xBackend;
            OUString        m_aComponentName;
            OUString const  m_aEntity;

        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------

} // namespace configmgr
#endif




