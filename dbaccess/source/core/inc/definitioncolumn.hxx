/*************************************************************************
 *
 *  $RCSfile: definitioncolumn.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:14:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#define _DBACORE_DEFINITIONCOLUMN_HXX_

#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_COREAPI_COLUMN_HXX_
#include "column.hxx"
#endif
#ifndef COMPHELPER_IDPROPERTYARRAYUSAGEHELPER_HXX
#include <comphelper/IdPropArrayHelper.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif


namespace dbaccess
{

    typedef ::cppu::ImplHelper1< ::com::sun::star::container::XChild > TXChild;
    /**
     *  provides the properties for description. A descriptor could be used to create a new table column.
     */
    class OTableColumnDescriptor : public OColumn
                                  ,public OColumnSettings
                                  ,public ::comphelper::OPropertyArrayUsageHelper < OTableColumnDescriptor >
                                  ,public TXChild
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >           m_xParent;
    protected:
    //  <properties>
        rtl::OUString           m_aTypeName;
        rtl::OUString           m_aDescription;
        rtl::OUString           m_aDefaultValue;
        rtl::OUString           m_aAutoIncrementValue;
        sal_Int32               m_nType;
        sal_Int32               m_nPrecision;
        sal_Int32               m_nScale;
        sal_Int32               m_nIsNullable;
        sal_Bool                m_bAutoIncrement : 1;
        sal_Bool                m_bRowVersion : 1;
        sal_Bool                m_bCurrency : 1;
    //  </properties>
    public:
        OTableColumnDescriptor():m_nType(::com::sun::star::sdbc::DataType::SQLNULL)
                           ,m_nPrecision(0)
                           ,m_nScale(0)
                           ,m_nIsNullable(::com::sun::star::sdbc::ColumnValue::NULLABLE_UNKNOWN)
                           ,m_bAutoIncrement(sal_False)
                           ,m_bRowVersion(sal_False)
                           ,m_bCurrency(sal_False){}

        DECLARE_XINTERFACE( )
    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

    // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

    // ::cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);
    };

    /**
     *  describes a column of an existing table. The informations for the column are
     *  retrieved in a generic way using the meta data of a connection.
     */
    class OTableColumn : public OTableColumnDescriptor,
                         public ::comphelper::OPropertyArrayUsageHelper < OTableColumn >
    {
        friend class ODBTable;

    protected:
        virtual ~OTableColumn();
    public:
        OTableColumn(const ::rtl::OUString& _rName);
        OTableColumn(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn);

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    };

    /**
     *  describes all properties for a columns of a table. Only the view parts are provided
     *  directly, all the other parts are derived from a driver implementation
     */
    class OColumnWrapper : public OColumn
    {
    protected:
        // definition which is provided by a driver!
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                m_xAggregate;

        sal_Int32               m_nColTypeID;

    protected:
        OColumnWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol);
        virtual ~OColumnWrapper();

    public:
        virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                sal_Int32 nHandle
                                 ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);

        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
    //  Helper
//      sal_Int32   getColumnTypeID() const {return m_nColTypeID;}
    };

    /**
     *  provides the properties for description. A descriptor could be used to create a new table column.
     */
    class OTableColumnDescriptorWrapper :   public OColumnWrapper,
                                            public OColumnSettings,
                                            public ::comphelper::OIdPropertyArrayUsageHelper < OTableColumnDescriptorWrapper >
    {
        sal_Bool m_bPureWrap;
    public:
        OTableColumnDescriptorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol,sal_Bool _bPureWrap = sal_False)
                            :OColumnWrapper(rCol),m_bPureWrap(_bPureWrap){}

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // OIdPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception);
    };

    /**
     *  describes all properties for a columns of a table. Only the view parts are provided
     *  directly, all the other parts are derived from a driver implementation
     */
    class OTableColumnWrapper : public OTableColumnDescriptorWrapper,
                                public ::comphelper::OIdPropertyArrayUsageHelper < OTableColumnWrapper >
    {
    public:
        OTableColumnWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rColDefintion
                            ,sal_Bool _bPureWrap = sal_False);

    // ::com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // OIdPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;
    };

    /**
     *  describes all properties for a columns of an index.
     */
    class OIndexColumnWrapper : public OColumnWrapper,
                                public ::comphelper::OIdPropertyArrayUsageHelper < OIndexColumnWrapper >
    {
    protected:
    //  <properties>
        sal_Bool                m_bAscending;
    //  </properties>

    public:
        OIndexColumnWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol )
                            :OColumnWrapper(rCol) {}

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // OIdPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;

        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
    };

    /**
     *  describes all properties for a columns of an key column.
     */
    class OKeyColumnWrapper : public OColumnWrapper,
                              public ::comphelper::OIdPropertyArrayUsageHelper < OKeyColumnWrapper >
    {
    protected:
    //  <properties>
        rtl::OUString           m_aRelatedColumn;
    //  </properties>

    public:
        OKeyColumnWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol)
                        :OColumnWrapper(rCol) {}

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // OIdPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const;

        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
    };
}
#endif // _DBACORE_DEFINITIONCOLUMN_HXX_

