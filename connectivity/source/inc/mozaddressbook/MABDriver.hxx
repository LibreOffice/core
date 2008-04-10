/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MABDriver.hxx,v $
 * $Revision: 1.7 $
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

#ifndef _CONNECTIVITY_MAB_DRIVER_HXX_
#define _CONNECTIVITY_MAB_DRIVER_HXX_

#include "file/FDriver.hxx"

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

