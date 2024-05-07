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

#include <stringconstants.hxx>
#include <strings.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdb/XDataAccessDescriptorFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/broadcasthelper.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::PropertyValue;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;
    namespace CommandType = ::com::sun::star::sdb::CommandType;

    // DataAccessDescriptor
    typedef ::comphelper::OMutexAndBroadcastHelper  DataAccessDescriptor_MutexBase;

    typedef ::cppu::WeakImplHelper<   XServiceInfo
                                  >   DataAccessDescriptor_TypeBase;

    typedef ::comphelper::OPropertyContainer        DataAccessDescriptor_PropertyBase;

    class DataAccessDescriptor  :public DataAccessDescriptor_MutexBase
                                ,public DataAccessDescriptor_TypeBase
                                ,public DataAccessDescriptor_PropertyBase
                                ,public ::comphelper::OPropertyArrayUsageHelper< DataAccessDescriptor >
    {
    public:
        DataAccessDescriptor();

        // UNO
        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    protected:
        virtual ~DataAccessDescriptor() override;

    protected:
        // XPropertySet
        virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

    private:
        // </properties>
        OUString             m_sDataSourceName;
        OUString             m_sDatabaseLocation;
        OUString             m_sConnectionResource;
        Sequence< PropertyValue >   m_aConnectionInfo;
        Reference< XConnection >    m_xActiveConnection;
        OUString             m_sCommand;
        sal_Int32                   m_nCommandType;
        OUString             m_sFilter;
        OUString             m_sOrder;
        OUString             m_sHavingClause;
        OUString             m_sGroupBy;
        bool                    m_bEscapeProcessing;
        Reference< XResultSet >     m_xResultSet;
        Sequence< Any >             m_aSelection;
        bool                    m_bBookmarkSelection;
        OUString             m_sColumnName;
        Reference< XPropertySet >   m_xColumn;
        // </properties>
    };

    DataAccessDescriptor::DataAccessDescriptor()
        :DataAccessDescriptor_PropertyBase( m_aBHelper )
        ,m_nCommandType( CommandType::COMMAND )
        ,m_bEscapeProcessing( true )
        ,m_bBookmarkSelection( true )
    {
        registerProperty(PROPERTY_DATASOURCENAME, PROPERTY_ID_DATASOURCENAME, PropertyAttribute::BOUND, &m_sDataSourceName , cppu::UnoType<decltype(m_sDataSourceName )>::get());
        registerProperty(PROPERTY_DATABASE_LOCATION, PROPERTY_ID_DATABASE_LOCATION, PropertyAttribute::BOUND, &m_sDatabaseLocation , cppu::UnoType<decltype(m_sDatabaseLocation )>::get());
        registerProperty(PROPERTY_CONNECTION_RESOURCE, PROPERTY_ID_CONNECTION_RESOURCE, PropertyAttribute::BOUND, &m_sConnectionResource , cppu::UnoType<decltype(m_sConnectionResource )>::get());
        registerProperty(PROPERTY_CONNECTION_INFO, PROPERTY_ID_CONNECTION_INFO, PropertyAttribute::BOUND, &m_aConnectionInfo , cppu::UnoType<decltype(m_aConnectionInfo )>::get());
        registerProperty(PROPERTY_ACTIVE_CONNECTION, PROPERTY_ID_ACTIVE_CONNECTION, PropertyAttribute::BOUND, &m_xActiveConnection , cppu::UnoType<decltype(m_xActiveConnection )>::get());
        registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND, &m_sCommand , cppu::UnoType<decltype(m_sCommand )>::get());
        registerProperty(PROPERTY_COMMAND_TYPE, PROPERTY_ID_COMMAND_TYPE, PropertyAttribute::BOUND, &m_nCommandType , cppu::UnoType<decltype(m_nCommandType )>::get());
        registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND, &m_sFilter , cppu::UnoType<decltype(m_sFilter )>::get());
        registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND, &m_sOrder , cppu::UnoType<decltype(m_sOrder )>::get());
        registerProperty(PROPERTY_HAVING_CLAUSE, PROPERTY_ID_HAVING_CLAUSE, PropertyAttribute::BOUND, &m_sHavingClause , cppu::UnoType<decltype(m_sHavingClause )>::get());
        registerProperty(PROPERTY_GROUP_BY, PROPERTY_ID_GROUP_BY, PropertyAttribute::BOUND, &m_sGroupBy , cppu::UnoType<decltype(m_sGroupBy )>::get());
        registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND, &m_bEscapeProcessing , cppu::UnoType<decltype(m_bEscapeProcessing )>::get());
        registerProperty(PROPERTY_RESULT_SET, PROPERTY_ID_RESULT_SET, PropertyAttribute::BOUND, &m_xResultSet , cppu::UnoType<decltype(m_xResultSet )>::get());
        registerProperty(PROPERTY_SELECTION, PROPERTY_ID_SELECTION, PropertyAttribute::BOUND, &m_aSelection , cppu::UnoType<decltype(m_aSelection )>::get());
        registerProperty(PROPERTY_BOOKMARK_SELECTION, PROPERTY_ID_BOOKMARK_SELECTION, PropertyAttribute::BOUND, &m_bBookmarkSelection , cppu::UnoType<decltype(m_bBookmarkSelection )>::get());
        registerProperty(PROPERTY_COLUMN_NAME, PROPERTY_ID_COLUMN_NAME, PropertyAttribute::BOUND, &m_sColumnName , cppu::UnoType<decltype(m_sColumnName )>::get());
        registerProperty(PROPERTY_COLUMN, PROPERTY_ID_COLUMN, PropertyAttribute::BOUND, &m_xColumn , cppu::UnoType<decltype(m_xColumn )>::get());
    }

    DataAccessDescriptor::~DataAccessDescriptor()
    {
    }

    IMPLEMENT_FORWARD_XINTERFACE2( DataAccessDescriptor, DataAccessDescriptor_TypeBase, DataAccessDescriptor_PropertyBase );

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataAccessDescriptor, DataAccessDescriptor_TypeBase, DataAccessDescriptor_PropertyBase );

    OUString SAL_CALL DataAccessDescriptor::getImplementationName()
    {
        return u"com.sun.star.comp.dba.DataAccessDescriptor"_ustr;
    }

    sal_Bool SAL_CALL DataAccessDescriptor::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    Sequence< OUString > SAL_CALL DataAccessDescriptor::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.sdb.DataAccessDescriptor"_ustr };
    }

    Reference< XPropertySetInfo > SAL_CALL DataAccessDescriptor::getPropertySetInfo()
    {
        Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    ::cppu::IPropertyArrayHelper& DataAccessDescriptor::getInfoHelper()
    {
        return *getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* DataAccessDescriptor::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    // DataAccessDescriptorFactory
    class DataAccessDescriptorFactory: public ::cppu::WeakImplHelper<XServiceInfo, css::sdb::XDataAccessDescriptorFactory>
    {
    public:
        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

        // XDataAccessDescriptorFactory
        virtual Reference< XPropertySet > SAL_CALL createDataAccessDescriptor(  ) override;

        DataAccessDescriptorFactory();
    };

    DataAccessDescriptorFactory::DataAccessDescriptorFactory()
    {
    }

    OUString SAL_CALL DataAccessDescriptorFactory::getImplementationName()
    {
        return u"com.sun.star.comp.dba.DataAccessDescriptorFactory"_ustr;
    }

    sal_Bool SAL_CALL DataAccessDescriptorFactory::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    Sequence< OUString > SAL_CALL DataAccessDescriptorFactory::getSupportedServiceNames()
    {
        return { u"com.sun.star.sdb.DataAccessDescriptorFactory"_ustr };
    }

    Reference< XPropertySet > SAL_CALL DataAccessDescriptorFactory::createDataAccessDescriptor(  )
    {
        return new DataAccessDescriptor();
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_dba_DataAccessDescriptorFactory(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new DataAccessDescriptorFactory());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
