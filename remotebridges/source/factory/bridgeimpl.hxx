/*************************************************************************
 *
 *  $RCSfile: bridgeimpl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
               queryInterface( const ::com::sun::star::uno::Type & aType );

        void        SAL_CALL acquire()
            { OComponentHelper::acquire(); }
        void        SAL_CALL release()
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

