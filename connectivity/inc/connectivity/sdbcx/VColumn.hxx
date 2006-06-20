/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VColumn.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:01:07 $
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
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#define _CONNECTIVITY_SDBCX_COLUMN_HXX_

#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#include <comphelper/IdPropArrayHelper.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "connectivity/CommonTools.hxx"
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include "connectivity/sdbcx/VDescriptor.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace connectivity
{
    namespace sdbcx
    {
        class OColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OColumn> OColumn_PROP;

        typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::container::XNamed,
                                                  ::com::sun::star::lang::XServiceInfo> OColumnDescriptor_BASE;
        typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XDataDescriptorFactory > OColumn_BASE;


        class OColumn            :  public comphelper::OBaseMutex,
                                    public OColumn_BASE,
                                    public OColumnDescriptor_BASE,
                                    public OColumn_PROP,
                                    public ODescriptor
        {
        protected:
            ::rtl::OUString m_TypeName;
            ::rtl::OUString m_Description;
            ::rtl::OUString m_DefaultValue;

            sal_Int32       m_IsNullable;
            sal_Int32       m_Precision;
            sal_Int32       m_Scale;
            sal_Int32       m_Type;

            sal_Bool        m_IsAutoIncrement;
            sal_Bool        m_IsRowVersion;
            sal_Bool        m_IsCurrency;

            using OColumnDescriptor_BASE::rBHelper;
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

            virtual ~OColumn();
        public:
            virtual void    SAL_CALL acquire() throw();
            virtual void    SAL_CALL release() throw();

            OColumn(    sal_Bool _bCase);
            OColumn(    const ::rtl::OUString& _Name,
                                const ::rtl::OUString& _TypeName,
                                const ::rtl::OUString& _DefaultValue,
                                sal_Int32       _IsNullable,
                                sal_Int32       _Precision,
                                sal_Int32       _Scale,
                                sal_Int32       _Type,
                                sal_Bool        _IsAutoIncrement,
                                sal_Bool        _IsRowVersion,
                                sal_Bool        _IsCurrency,
                                sal_Bool        _bCase);

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // ODescriptor
            virtual void construct();
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_COLUMN_HXX_

