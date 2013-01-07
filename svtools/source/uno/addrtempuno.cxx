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

#include "svtools/genericunodialog.hxx"
#include <svtools/addresstemplate.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>

// .......................................................................
namespace svt
{
// .......................................................................

#define UNODIALOG_PROPERTY_ID_ALIASES       100
#define UNODIALOG_PROPERTY_ALIASES          "FieldMapping"

    using namespace com::sun::star::uno;
    using namespace com::sun::star::lang;
    using namespace com::sun::star::util;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::sdbc;

    //=========================================================================
    //= OAddressBookSourceDialogUno
    //=========================================================================
    typedef OGenericUnoDialog OAddressBookSourceDialogUnoBase;
    class OAddressBookSourceDialogUno
            :public OAddressBookSourceDialogUnoBase
            ,public ::comphelper::OPropertyArrayUsageHelper< OAddressBookSourceDialogUno >
    {
    protected:
        Sequence< AliasProgrammaticPair >   m_aAliases;
        Reference< XDataSource >            m_xDataSource;
        ::rtl::OUString                     m_sDataSourceName;
        ::rtl::OUString                     m_sTable;

    protected:
        OAddressBookSourceDialogUno(const Reference< XMultiServiceFactory >& _rxORB);

    public:
        // XTypeProvider
        virtual Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(RuntimeException);

        // XServiceInfo - static methods
        static Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( RuntimeException );
        static Reference< XInterface >
                SAL_CALL Create(const Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual Reference< XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        void initialize(const Sequence< Any >& aArguments) throw(RuntimeException);

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);

        virtual void implInitialize(const com::sun::star::uno::Any& _rValue);

        virtual void executedDialog(sal_Int16 _nExecutionResult);
    };


    //=========================================================================
    //= OAddressBookSourceDialogUno
    //=========================================================================
    Reference< XInterface > SAL_CALL OAddressBookSourceDialogUno_CreateInstance( const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return OAddressBookSourceDialogUno::Create(_rxFactory);
    }

    //-------------------------------------------------------------------------
    OAddressBookSourceDialogUno::OAddressBookSourceDialogUno(const Reference< XMultiServiceFactory >& _rxORB)
        :OGenericUnoDialog(_rxORB)
    {
        registerProperty(::rtl::OUString(UNODIALOG_PROPERTY_ALIASES), UNODIALOG_PROPERTY_ID_ALIASES, PropertyAttribute::READONLY,
            &m_aAliases, getCppuType(&m_aAliases));
    }

    //-------------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OAddressBookSourceDialogUno::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //-------------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OAddressBookSourceDialogUno::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OAddressBookSourceDialogUno(_rxFactory));
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OAddressBookSourceDialogUno::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //-------------------------------------------------------------------------
    ::rtl::OUString OAddressBookSourceDialogUno::getImplementationName_Static() throw(RuntimeException)
    {
        return ::rtl::OUString( "com.sun.star.comp.svtools.OAddressBookSourceDialogUno" );
    }

    //-------------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OAddressBookSourceDialogUno::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //-------------------------------------------------------------------------
    ::comphelper::StringSequence OAddressBookSourceDialogUno::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString( "com.sun.star.ui.AddressBookSourceDialog" );
        return aSupported;
    }

    //-------------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OAddressBookSourceDialogUno::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //-------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OAddressBookSourceDialogUno::getInfoHelper()
    {
        return *const_cast<OAddressBookSourceDialogUno*>(this)->getArrayHelper();
    }

    //------------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OAddressBookSourceDialogUno::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //------------------------------------------------------------------------------
    void OAddressBookSourceDialogUno::executedDialog(sal_Int16 _nExecutionResult)
    {
        OAddressBookSourceDialogUnoBase::executedDialog(_nExecutionResult);

        if ( _nExecutionResult )
            if ( m_pDialog )
                static_cast< AddressBookSourceDialog* >( m_pDialog )->getFieldMapping( m_aAliases );
    }
    //------------------------------------------------------------------------------
    void OAddressBookSourceDialogUno::initialize(const Sequence< Any >& aArguments) throw(RuntimeException)
    {
        if( aArguments.getLength() == 5 )
        {
            Reference<com::sun::star::awt::XWindow> xParentWindow;
            Reference<com::sun::star::beans::XPropertySet> xDataSource;
            rtl::OUString sDataSourceName;
            rtl::OUString sCommand;
            rtl::OUString sTitle;
            if ( (aArguments[0] >>= xParentWindow)
               && (aArguments[1] >>= xDataSource)
               && (aArguments[2] >>= sDataSourceName)
               && (aArguments[3] >>= sCommand)
               && (aArguments[4] >>= sTitle) )
            {
                setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ) ), makeAny( xParentWindow ) );
                setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataSource" ) ), makeAny( xDataSource ) );
                setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DataSourceName" ) ), makeAny( sDataSourceName ) );
                setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Command" ) ), makeAny( sCommand ) );
                setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) ), makeAny( sTitle ) );
                return;
            }
        }
        OGenericUnoDialog::initialize(aArguments);
    }
    //------------------------------------------------------------------------------
    void OAddressBookSourceDialogUno::implInitialize(const com::sun::star::uno::Any& _rValue)
    {
        PropertyValue aVal;
        if (_rValue >>= aVal)
        {
            if (0 == aVal.Name.compareToAscii("DataSource"))
            {
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                aVal.Value >>= m_xDataSource;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSource!" );
                return;
            }

            if (0 == aVal.Name.compareToAscii("DataSourceName"))
            {
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                aVal.Value >>= m_sDataSourceName;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSourceName!" );
                return;
            }

            if (0 == aVal.Name.compareToAscii("Command"))
            {
#if OSL_DEBUG_LEVEL > 0
                sal_Bool bSuccess =
#endif
                aVal.Value >>= m_sTable;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for Command!" );
                return;
            }
        }

        OAddressBookSourceDialogUnoBase::implInitialize( _rValue );
    }

    //------------------------------------------------------------------------------
    Dialog* OAddressBookSourceDialogUno::createDialog(Window* _pParent)
    {
        if ( m_xDataSource.is() && !m_sTable.isEmpty() )
            return new AddressBookSourceDialog(_pParent, m_aContext.getLegacyServiceFactory(), m_xDataSource, m_sDataSourceName, m_sTable, m_aAliases );
        else
            return new AddressBookSourceDialog( _pParent, m_aContext.getLegacyServiceFactory() );
    }

// .......................................................................
}   // namespace svt
// .......................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
