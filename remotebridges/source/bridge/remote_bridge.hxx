/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote_bridge.hxx,v $
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

#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <uno/mapping.hxx>
#include <uno/environment.h>

#include <bridges/remote/context.h>
#include <bridges/remote/remote.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/bridge/XInstanceProvider.hpp>


namespace remotebridges_bridge
{
    extern rtl_StandardModuleCount g_moduleCount;

    struct MyMutex
    {
        ::osl::Mutex m_mutex;
    };

    class ORemoteBridge :
        public MyMutex,
        public remote_DisposingListener,
        public ::com::sun::star::lang::XInitialization,
        public ::com::sun::star::bridge::XBridge,
        public ::cppu::OComponentHelper
    {
    public:
        ORemoteBridge();
        ~ORemoteBridge();

        // XInterface
    public:
        ::com::sun::star::uno::Any      SAL_CALL
               queryInterface( const ::com::sun::star::uno::Type & aType ) throw(::com::sun::star::uno::RuntimeException);

        void        SAL_CALL acquire() throw()
            { OComponentHelper::acquire(); }
        void        SAL_CALL release() throw()
            { OComponentHelper::release(); }

    public:
        virtual void SAL_CALL disposing(); // called by OComponentHelper

    public:
        // Methods
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence<
                                          ::com::sun::star::uno::Any >& aArguments )
            throw( ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
               getInstance( const ::rtl::OUString& sInstanceName )
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL getName(  )
            throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getDescription(  )
            throw(::com::sun::star::uno::RuntimeException);

    public:
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
                     getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
                     getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    public:
        static void SAL_CALL thisAcquire( remote_DisposingListener * );
        static void SAL_CALL thisRelease( remote_DisposingListener * );
        static void SAL_CALL thisDisposing( remote_DisposingListener * ,
                                            rtl_uString * pBridgeName  );

    public:
        void objectMappedSuccesfully();

    public:
        remote_Context *m_pContext;
        uno_Environment *m_pEnvRemote;
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sDescription;
        ::rtl::OUString m_sProtocol;
    };

    class OInstanceProviderWrapper :
        public remote_InstanceProvider
    {
    public:
        OInstanceProviderWrapper( const  ::com::sun::star::uno::Reference <
        ::com::sun::star::bridge::XInstanceProvider > & rProvider ,
                                  ORemoteBridge * pBridgeCallback );

        ~OInstanceProviderWrapper();
    public:
        static void SAL_CALL thisAcquire( remote_InstanceProvider * );
        static void SAL_CALL thisRelease( remote_InstanceProvider * );
        static void SAL_CALL thisGetInstance( remote_InstanceProvider * pProvider ,
                                              uno_Environment *pEnvRemote,
                                              remote_Interface **ppRemoteI,
                                              rtl_uString *pInstanceName,
                                              typelib_InterfaceTypeDescription *pType,
                                              uno_Any **ppException );

    private:
        ::com::sun::star::uno::Reference <
        ::com::sun::star::bridge::XInstanceProvider > m_rProvider;
        oslInterlockedCount m_nRef;
        ORemoteBridge *m_pBridgeCallback;
    };

}

