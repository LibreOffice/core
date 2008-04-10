/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: VView.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#define _CONNECTIVITY_SDBCX_VIEW_HXX_

#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase1.hxx>
#include "connectivity/CommonTools.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/container/XNamed.hpp>
#include "connectivity/sdbcx/VDescriptor.hxx"
#include <comphelper/IdPropArrayHelper.hxx>

namespace connectivity
{
    namespace sdbcx
    {

        typedef ::cppu::OWeakObject OView_BASE;


        class OViewHelper
        {
        protected:
            ::osl::Mutex             m_aMutex;
            ::cppu::OBroadcastHelper rBHelper;

        public:
            OViewHelper(): rBHelper(m_aMutex){}
        };

        class OView :   public OViewHelper,
                        public OView_BASE,
                        public ::com::sun::star::container::XNamed,
                        public ::com::sun::star::lang::XServiceInfo,
                        public ::comphelper::OIdPropertyArrayUsageHelper<OView>,
                        public ODescriptor
        {
        protected:
            ::rtl::OUString m_CatalogName;
            ::rtl::OUString m_SchemaName;
            ::rtl::OUString m_Command;
            sal_Int32       m_CheckOption;
            // need for the getName method
            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >       m_xMetaData;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

        public:
            DECLARE_SERVICE_INFO();

            OView(sal_Bool _bCase,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData);
            OView(  sal_Bool _bCase,
                    const ::rtl::OUString& _rName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _xMetaData,
                    sal_Int32 _nCheckOption = 0,
                    const ::rtl::OUString& _rCommand = ::rtl::OUString(),
                    const ::rtl::OUString& _rSchemaName = ::rtl::OUString(),
                    const ::rtl::OUString& _rCatalogName = ::rtl::OUString());
            virtual ~OView();

            // ODescriptor
            virtual void construct();

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_VIEW_HXX_

