/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xconnection.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:03:18 $
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

#ifndef _VCL_XCONNECTION_HXX
#define _VCL_XCONNECTION_HXX

#ifndef _COM_SUN_STAR_AWT_XDISPLAYCONNECTION_HPP_
#include <com/sun/star/awt/XDisplayConnection.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

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
