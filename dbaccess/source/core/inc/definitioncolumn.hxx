/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_DEFINITIONCOLUMN_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_DEFINITIONCOLUMN_HXX

#include "apitools.hxx"
#include "column.hxx"
#include "columnsettings.hxx"

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/container/XChild.hpp>

#include <comphelper/IdPropArrayHelper.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase1.hxx>

namespace dbaccess
{

    typedef ::cppu::ImplHelper1< ::com::sun::star::container::XChild > TXChild;
    // OTableColumnDescriptor
    /**
     *  provides the properties for description. A descriptor could be used to create a new table column.
     */
    class OTableColumnDescriptor : public OColumn
                                  ,public OColumnSettings
                                  ,public ::comphelper::OPropertyArrayUsageHelper < OTableColumnDescriptor >
                                  ,public TXChild
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >   m_xParent;
        const bool                                                              m_bActAsDescriptor;

    protected:
    //  <properties>
        OUString           m_aTypeName;
        OUString           m_aDescription;
        OUString           m_aDefaultValue;
        OUString           m_aAutoIncrementValue;
        sal_Int32               m_nType;
        sal_Int32               m_nPrecision;
        sal_Int32               m_nScale;
        sal_Int32               m_nIsNullable;
        bool                m_bAutoIncrement;
        bool                m_bRowVersion;
        bool                m_bCurrency;
    //  </properties>

    public:
        OTableColumnDescriptor( const bool _bActAsDescriptor )
            :OColumn( !_bActAsDescriptor )
            ,m_bActAsDescriptor( _bActAsDescriptor )
            ,m_nType( ::com::sun::star::sdbc::DataType::SQLNULL )
            ,m_nPrecision( 0 )
            ,m_nScale( 0 )
            ,m_nIsNullable( ::com::sun::star::sdbc::ColumnValue::NULLABLE_UNKNOWN )
            ,m_bAutoIncrement( false )
            ,m_bRowVersion( false )
            ,m_bCurrency( false )
        {
            impl_registerProperties();
        }

        DECLARE_XINTERFACE( )

        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // ::comphelper::OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;

        // ::cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    private:
        void    impl_registerProperties();
    };

    // OTableColumn
    class OTableColumn;
    typedef ::comphelper::OPropertyArrayUsageHelper < OTableColumn >    OTableColumn_PBase;
    /** describes a column of a table
     */
    class OTableColumn  :public OTableColumnDescriptor
                        ,public OTableColumn_PBase
    {
    protected:
        virtual ~OTableColumn();

    public:
        OTableColumn(const OUString& _rName);

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;
    };

    // OQueryColumn
    class OQueryColumn;
    typedef ::comphelper::OPropertyArrayUsageHelper< OQueryColumn > OQueryColumn_PBase;
    /** a column of a Query, with additional information obtained from parsing the query statement
    */
    class OQueryColumn  :public OTableColumnDescriptor
                        ,public OQueryColumn_PBase
    {
        // <properties>
        OUString m_sCatalogName;
        OUString m_sSchemaName;
        OUString m_sTableName;
        OUString m_sRealName;
        OUString m_sLabel;
        // </properties>

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xOriginalTableColumn;

    protected:
        virtual ~OQueryColumn();

    public:
        OQueryColumn(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxParserColumn,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const OUString &i_sLabel
        );

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // *Property*
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const SAL_OVERRIDE;
        virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                impl_determineOriginalTableColumn(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection
                );

        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };

    // OColumnWrapper
    /**
     *  describes all properties for a columns of a table. Only the view parts are provided
     *  directly, all the other parts are derived from a driver implementation
     */
    class OColumnWrapper    :public OColumn
    {
    protected:
        // definition which is provided by a driver!
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                m_xAggregate;

        sal_Int32               m_nColTypeID;

    protected:
        OColumnWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rCol, const bool _bNameIsReadOnly );
        virtual ~OColumnWrapper();

    public:
        virtual void SAL_CALL getFastPropertyValue(
                                ::com::sun::star::uno::Any& rValue,
                                sal_Int32 nHandle
                                 ) const SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

        sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    protected:
        OUString impl_getPropertyNameFromHandle( const sal_Int32 _nHandle ) const;

    protected:
        using OColumn::getFastPropertyValue;
    };

    // OTableColumnDescriptorWrapper
    /**
     *  provides the properties for description. A descriptor could be used to create a new table column.
     */
    class OTableColumnDescriptorWrapper :public OColumnWrapper
                                        ,public OColumnSettings
                                        ,public ::comphelper::OIdPropertyArrayUsageHelper < OTableColumnDescriptorWrapper >
    {
        const bool  m_bPureWrap;
        const bool  m_bIsDescriptor;

    public:
        OTableColumnDescriptorWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol,
            const bool _bPureWrap, const bool _bIsDescriptor );

    // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OIdPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const SAL_OVERRIDE;

    // cppu::OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                ::com::sun::star::uno::Any & rConvertedValue,
                                ::com::sun::star::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue )
                                    throw (::com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                                     )
                                                     throw (::com::sun::star::uno::Exception, std::exception) SAL_OVERRIDE;

    protected:
        using OColumnWrapper::getFastPropertyValue;
    };

    // OTableColumnWrapper
    /**
     *  describes all properties for a columns of a table. Only the view parts are provided
     *  directly, all the other parts are derived from a driver implementation
     */
    class OTableColumnWrapper   :public OTableColumnDescriptorWrapper
                                ,public ::comphelper::OIdPropertyArrayUsageHelper < OTableColumnWrapper >
    {
    protected:
        virtual ~OTableColumnWrapper();

    public:
        OTableColumnWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rCol,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& rColDefintion,
                             const bool _bPureWrap );

    // ::com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OIdPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(sal_Int32 nId) const SAL_OVERRIDE;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_DEFINITIONCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
