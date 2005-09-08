/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MABDriver.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:29:01 $
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

#ifndef _CONNECTIVITY_MAB_DRIVER_HXX_
#define _CONNECTIVITY_MAB_DRIVER_HXX_

#ifndef _CONNECTIVITY_FILE_ODRIVER_HXX_
#include "file/FDriver.hxx"
#endif

namespace connectivity
{
    namespace mozaddressbook
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            OMozabDriver_CreateInstance(const ::com::sun::star::uno::Reference<
                        ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
                    throw( ::com::sun::star::uno::Exception );

        class OMozabDriver : public file::OFileDriver
        {
        public:
            OMozabDriver(const ::com::sun::star::uno::Reference<
                                ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) :
                file::OFileDriver(_rxFactory), m_xMSFactory(_rxFactory){}

            static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
            ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);

            // XDriver
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL
                    connect( const ::rtl::OUString& url, const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& info )
                        throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL acceptsURL( const ::rtl::OUString& url )
                        throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
            & getMSFactory(void) const
                    { return m_xMSFactory; }
            static sal_Bool acceptsURL_Stat( const ::rtl::OUString& url );
        private:
            const ::com::sun::star::uno::Reference<
                                                ::com::sun::star::lang::XMultiServiceFactory > m_xMSFactory;
        };
    }
}

#endif //_CONNECTIVITY_MAB_DRIVER_HXX_

