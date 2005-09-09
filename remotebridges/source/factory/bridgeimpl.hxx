/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bridgeimpl.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:23:33 $
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

namespace remotebridges_factory {
    extern rtl_StandardModuleCount g_moduleCount;

    struct MyMutex
    {
        ::osl::Mutex m_mutex;
    };

    class OBridge :
        public MyMutex,
        public remote_DisposingListener,
        public ::com::sun::star::bridge::XBridge,
        public ::cppu::OComponentHelper
    {
    public:
        OBridge( remote_Context *pContext );
        ~OBridge();

        // XInterface
    public:
        ::com::sun::star::uno::Any      SAL_CALL
               queryInterface( const ::com::sun::star::uno::Type & aType ) throw(com::sun::star::uno::RuntimeException);

        void        SAL_CALL acquire() throw()
            { OComponentHelper::acquire(); }
        void        SAL_CALL release() throw()
            { OComponentHelper::release(); }

    public:
        virtual void SAL_CALL disposing(); // called by OComponentHelper

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
        remote_Context *m_pContext;
        ::rtl::OString m_sName;
        ::rtl::OString m_sDescription;
    };
}

