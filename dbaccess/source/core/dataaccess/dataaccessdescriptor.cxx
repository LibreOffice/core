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
#include <rtl/ref.hxx>

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

#define REGISTER_PROPERTY( propname, member ) \
    registerProperty( PROPERTY_##propname, PROPERTY_ID_##propname, PropertyAttribute::BOUND, &member, cppu::UnoType<decltype(member)>::get() )

    DataAccessDescriptor::DataAccessDescriptor()
        :DataAccessDescriptor_MutexBase()
        ,DataAccessDescriptor_TypeBase()
        ,DataAccessDescriptor_PropertyBase( m_aBHelper )
        ,m_sDataSourceName()
        ,m_sDatabaseLocation()
        ,m_sConnectionResource()
        ,m_aConnectionInfo()
        ,m_xActiveConnection()
        ,m_sCommand()
        ,m_nCommandType( CommandType::COMMAND )
        ,m_sFilter()
        ,m_sOrder()
        ,m_sHavingClause()
        ,m_sGroupBy()
        ,m_bEscapeProcessing( true )
        ,m_xResultSet()
        ,m_aSelection()
        ,m_bBookmarkSelection( true )
        ,m_sColumnName()
        ,m_xColumn()
    {
        REGISTER_PROPERTY( DATASOURCENAME,      m_sDataSourceName );
        REGISTER_PROPERTY( DATABASE_LOCATION,   m_sDatabaseLocation );
        REGISTER_PROPERTY( CONNECTION_RESOURCE, m_sConnectionResource );
        REGISTER_PROPERTY( CONNECTION_INFO,     m_aConnectionInfo );
        REGISTER_PROPERTY( ACTIVE_CONNECTION,   m_xActiveConnection );
        REGISTER_PROPERTY( COMMAND,             m_sCommand );
        REGISTER_PROPERTY( COMMAND_TYPE,        m_nCommandType );
        REGISTER_PROPERTY( FILTER,              m_sFilter );
        REGISTER_PROPERTY( ORDER,               m_sOrder );
        REGISTER_PROPERTY( HAVING_CLAUSE,       m_sHavingClause );
        REGISTER_PROPERTY( GROUP_BY,            m_sGroupBy );
        REGISTER_PROPERTY( ESCAPE_PROCESSING,   m_bEscapeProcessing );
        REGISTER_PROPERTY( RESULT_SET,          m_xResultSet );
        REGISTER_PROPERTY( SELECTION,           m_aSelection );
        REGISTER_PROPERTY( BOOKMARK_SELECTION,  m_bBookmarkSelection );
        REGISTER_PROPERTY( COLUMN_NAME,         m_sColumnName );
        REGISTER_PROPERTY( COLUMN,              m_xColumn );
    }

    DataAccessDescriptor::~DataAccessDescriptor()
    {
    }

    IMPLEMENT_FORWARD_XINTERFACE2( DataAccessDescriptor, DataAccessDescriptor_TypeBase, DataAccessDescriptor_PropertyBase );

    IMPLEMENT_FORWARD_XTYPEPROVIDER2( DataAccessDescriptor, DataAccessDescriptor_TypeBase, DataAccessDescriptor_PropertyBase );

    OUString SAL_CALL DataAccessDescriptor::getImplementationName()
    {
        return "com.sun.star.comp.dba.DataAccessDescriptor";
    }

    sal_Bool SAL_CALL DataAccessDescriptor::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    Sequence< OUString > SAL_CALL DataAccessDescriptor::getSupportedServiceNames(  )
    {
        return { "com.sun.star.sdb.DataAccessDescriptor" };
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
        return "com.sun.star.comp.dba.DataAccessDescriptorFactory";
    }

    sal_Bool SAL_CALL DataAccessDescriptorFactory::supportsService( const OUString& rServiceName )
    {
        return cppu::supportsService(this, rServiceName);
    }

    Sequence< OUString > SAL_CALL DataAccessDescriptorFactory::getSupportedServiceNames()
    {
        return { "com.sun.star.sdb.DataAccessDescriptorFactory" };
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
