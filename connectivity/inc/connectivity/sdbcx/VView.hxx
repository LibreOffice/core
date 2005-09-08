/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VView.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:06:16 $
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

#ifndef _CONNECTIVITY_SDBCX_VIEW_HXX_
#define _CONNECTIVITY_SDBCX_VIEW_HXX_

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include "connectivity/sdbcx/VDescriptor.hxx"
#endif
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#include <comphelper/IdPropArrayHelper.hxx>
#endif

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

