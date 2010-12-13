/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _VCL_XCONNECTION_HXX
#define _VCL_XCONNECTION_HXX

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include "vcl/displayconnectiondispatch.hxx"

#ifndef _STLP_LIST
#include <list>
#endif

namespace vcl {

    class DisplayConnection :
        public DisplayConnectionDispatch
    {
        ::osl::Mutex                    m_aMutex;
        ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler > >
                                        m_aHandlers;
        ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler > >
                                        m_aErrorHandlers;
        ::com::sun::star::uno::Any      m_aAny;
    public:
        DisplayConnection();
        virtual ~DisplayConnection();

        void start();
        void terminate();

        virtual bool dispatchEvent( void* pData, int nBytes );
        virtual bool dispatchErrorEvent( void* pData, int nBytes );

        // XDisplayConnection
        virtual void SAL_CALL addEventHandler( const ::com::sun::star::uno::Any& window, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler, sal_Int32 eventMask ) throw();
        virtual void SAL_CALL removeEventHandler( const ::com::sun::star::uno::Any& window, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual void SAL_CALL addErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual void SAL_CALL removeErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual ::com::sun::star::uno::Any SAL_CALL getIdentifier() throw();

    };

}

#endif // _VCL_XCONNECTION_HXX
