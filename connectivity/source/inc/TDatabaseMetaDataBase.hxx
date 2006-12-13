/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDatabaseMetaDataBase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:22:52 $
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

#ifndef _CONNECTIVITY_ODATABASEMETADATABASE_HXX_
#define _CONNECTIVITY_ODATABASEMETADATABASE_HXX_

#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA2_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData2.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

namespace connectivity
{
        class ODatabaseMetaDataBase :   public  comphelper::OBaseMutex,
                                        public ::cppu::WeakImplHelper2< ::com::sun::star::sdbc::XDatabaseMetaData2,
                                                                        ::com::sun::star::lang::XEventListener>
        {
        private:
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aConnectionInfo;

        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >     m_xConnection;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>   m_xListenerHelper; // forward the calls from the connection to me

            virtual ~ODatabaseMetaDataBase();

        protected:
            inline  void    setConnectionInfo( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rInfo )
            {
                m_aConnectionInfo = _rInfo;
            }

        public:

            ODatabaseMetaDataBase(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection);

            // XDatabaseMetaData2
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getConnectionInfo(  ) throw (::com::sun::star::uno::RuntimeException);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
        };
}
#endif // _CONNECTIVITY_ODATABASEMETADATABASE_HXX_
