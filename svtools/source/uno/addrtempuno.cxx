/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include "svtools/genericunodialog.hxx"
#include <svtools/addresstemplate.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

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
        registerProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNODIALOG_PROPERTY_ALIASES)), UNODIALOG_PROPERTY_ID_ALIASES, PropertyAttribute::READONLY,
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
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.svtools.OAddressBookSourceDialogUno" ));
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
        aSupported.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.AddressBookSourceDialog" ));
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
        if ( m_xDataSource.is() && m_sTable.getLength() )
            return new AddressBookSourceDialog(_pParent, m_aContext.getLegacyServiceFactory(), m_xDataSource, m_sDataSourceName, m_sTable, m_aAliases );
        else
            return new AddressBookSourceDialog( _pParent, m_aContext.getLegacyServiceFactory() );
    }

// .......................................................................
}   // namespace svt
// .......................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
