/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HConnection.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-24 08:21:37 $
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
#ifndef CONNECTIVITY_HSQLDB_CONNECTION_HXX
#define CONNECTIVITY_HSQLDB_CONNECTION_HXX

#ifndef _CONNECTIVITY_ZCONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#include <memory>

namespace connectivity
{
    namespace hsqldb
    {
        class FlushListeners;

        //==========================================================================
        //= OConnectionWeakWrapper - wraps all methods to the real connection from the driver
        //= but when disposed it doesn't dispose the real connection
        //==========================================================================
        typedef ::cppu::WeakComponentImplHelper1<   ::com::sun::star::util::XFlushable
                                                >   OConnectionWeakWrapper_BASE;

        class OConnectionWeakWrapper    :public ::comphelper::OBaseMutex
                                        ,public OConnectionWeakWrapper_BASE
                                        ,public OConnectionWrapper
        {
        private:
            ::std::auto_ptr< FlushListeners >   m_pFlushListeners;

        protected:
            virtual void SAL_CALL disposing(void);
            virtual ~OConnectionWeakWrapper();

        public:
            OConnectionWeakWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection
                                  ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB);

            // XServiceInfo
            DECLARE_SERVICE_INFO();
            DECLARE_XTYPEPROVIDER()
            DECLARE_XINTERFACE( )

            // XTransactedObject
            virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
        };
    }
}
#endif // CONNECTIVITY_HSQLDB_CONNECTION_HXX
