/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TDatabaseMetaDataBase.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:39:10 $
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

#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
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
                                        public ::cppu::WeakImplHelper2< ::com::sun::star::sdbc::XDatabaseMetaData,
                                                                        ::com::sun::star::lang::XEventListener>
        {
        protected:
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >     m_xConnection;
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>   m_xListenerHelper; // forward the calls from the connection to me

            virtual ~ODatabaseMetaDataBase();
        public:

            ODatabaseMetaDataBase(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection);

            // XEventListener
            virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
        };
}
#endif // _CONNECTIVITY_ODATABASEMETADATABASE_HXX_
