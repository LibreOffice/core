/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xconnection.hxx,v $
 * $Revision: 1.3 $
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

#include <com/sun/star/awt/XDisplayConnection.hpp>
#include <cppuhelper/implbase1.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Reference.hxx>

#ifndef _STLP_LIST
#include <list>
#endif

namespace vcl {

    class DisplayConnection :
        public ::cppu::WeakImplHelper1< ::com::sun::star::awt::XDisplayConnection >
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

        static bool dispatchEvent( void* pThis, void* pData, int nBytes );
        static bool dispatchErrorEvent( void* pThis, void* pData, int nBytes );
        void dispatchDowningEvent();

        // XDisplayConnection
        virtual void SAL_CALL addEventHandler( const ::com::sun::star::uno::Any& window, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler, sal_Int32 eventMask ) throw();
        virtual void SAL_CALL removeEventHandler( const ::com::sun::star::uno::Any& window, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual void SAL_CALL addErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual void SAL_CALL removeErrorHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XEventHandler >& handler ) throw();
        virtual ::com::sun::star::uno::Any SAL_CALL getIdentifier() throw();

    };

}

#endif // _VCL_XCONNECTION_HXX
