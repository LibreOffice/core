/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: basicimporthandler.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:24:54 $
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

#ifndef CONFIGMGR_BACKEND_BASICIMPORTHANDLER_HXX
#define CONFIGMGR_BACKEND_BASICIMPORTHANDLER_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERHANDLER_HPP_
#include <com/sun/star/configuration/backend/XLayerHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif

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




