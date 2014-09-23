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

#include <svtools/addresstemplate.hxx>
#include <svtools/genericunodialog.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <rtl/ref.hxx>

using namespace svt;

namespace {

#define UNODIALOG_PROPERTY_ID_ALIASES       100
#define UNODIALOG_PROPERTY_ALIASES          "FieldMapping"

    using namespace com::sun::star::uno;
    using namespace com::sun::star::lang;
    using namespace com::sun::star::util;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::sdbc;

    typedef OGenericUnoDialog OAddressBookSourceDialogUnoBase;
    class OAddressBookSourceDialogUno
            :public OAddressBookSourceDialogUnoBase
            ,public ::comphelper::OPropertyArrayUsageHelper< OAddressBookSourceDialogUno >
    {
    private:
        Sequence< AliasProgrammaticPair >   m_aAliases;
        Reference< XDataSource >            m_xDataSource;
        OUString                     m_sDataSourceName;
        OUString                     m_sTable;

    public:
        OAddressBookSourceDialogUno(const Reference< XComponentContext >& _rxORB);

        // XTypeProvider
        virtual Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertySet
        virtual Reference< XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

        virtual void SAL_CALL initialize(const Sequence< Any >& aArguments) throw(Exception, RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(vcl::Window* _pParent) SAL_OVERRIDE;

        virtual void implInitialize(const com::sun::star::uno::Any& _rValue) SAL_OVERRIDE;

        virtual void executedDialog(sal_Int16 _nExecutionResult) SAL_OVERRIDE;
    };


    OAddressBookSourceDialogUno::OAddressBookSourceDialogUno(const Reference< XComponentContext >& _rxORB)
        :OGenericUnoDialog(_rxORB)
    {
        registerProperty(OUString(UNODIALOG_PROPERTY_ALIASES), UNODIALOG_PROPERTY_ID_ALIASES, PropertyAttribute::READONLY,
            &m_aAliases, getCppuType(&m_aAliases));
    }


    Sequence<sal_Int8> SAL_CALL OAddressBookSourceDialogUno::getImplementationId(  ) throw(RuntimeException, std::exception)
    {
        return css::uno::Sequence<sal_Int8>();
    }


    OUString SAL_CALL OAddressBookSourceDialogUno::getImplementationName() throw(RuntimeException, std::exception)
    {
        return OUString( "com.sun.star.comp.svtools.OAddressBookSourceDialogUno" );
    }


    ::comphelper::StringSequence SAL_CALL OAddressBookSourceDialogUno::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = "com.sun.star.ui.AddressBookSourceDialog";
        return aSupported;
    }


    Reference<XPropertySetInfo>  SAL_CALL OAddressBookSourceDialogUno::getPropertySetInfo() throw(RuntimeException, std::exception)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    ::cppu::IPropertyArrayHelper& OAddressBookSourceDialogUno::getInfoHelper()
    {
        return *getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* OAddressBookSourceDialogUno::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }


    void OAddressBookSourceDialogUno::executedDialog(sal_Int16 _nExecutionResult)
    {
        OAddressBookSourceDialogUnoBase::executedDialog(_nExecutionResult);

        if ( _nExecutionResult )
            if ( m_pDialog )
                static_cast< AddressBookSourceDialog* >( m_pDialog )->getFieldMapping( m_aAliases );
    }

    void SAL_CALL OAddressBookSourceDialogUno::initialize(const Sequence< Any >& rArguments) throw(Exception, RuntimeException, std::exception)
    {
        if( rArguments.getLength() == 5 )
        {
            Reference<com::sun::star::awt::XWindow> xParentWindow;
            Reference<com::sun::star::beans::XPropertySet> xDataSource;
            OUString sDataSourceName;
            OUString sCommand;
            OUString sTitle;
            if ( (rArguments[0] >>= xParentWindow)
               && (rArguments[1] >>= xDataSource)
               && (rArguments[2] >>= sDataSourceName)
               && (rArguments[3] >>= sCommand)
               && (rArguments[4] >>= sTitle) )
            {

                // convert the parameters for creating the dialog to PropertyValues
                Sequence< Any > aArguments(5);
                Any* pArguments = aArguments.getArray();
                // the parent window
                *pArguments++ <<= PropertyValue(OUString( "ParentWindow" ), -1, makeAny( xParentWindow ), PropertyState_DIRECT_VALUE);
                // the data source to use
                *pArguments++ <<= PropertyValue(OUString( "DataSource" ), -1, makeAny( xDataSource ), PropertyState_DIRECT_VALUE);
                *pArguments++ <<= PropertyValue(OUString( "DataSourceName" ), -1, makeAny( sDataSourceName ), PropertyState_DIRECT_VALUE);
                // the table to use
                *pArguments++ <<= PropertyValue(OUString( "Command" ), -1, makeAny( sCommand ), PropertyState_DIRECT_VALUE);
                // the title
                *pArguments++ <<= PropertyValue(OUString( "Title" ), -1, makeAny( sTitle ), PropertyState_DIRECT_VALUE);
                OGenericUnoDialog::initialize(aArguments);
                return;
            }
        }
        OGenericUnoDialog::initialize(rArguments);
    }

    void OAddressBookSourceDialogUno::implInitialize(const com::sun::star::uno::Any& _rValue)
    {
        PropertyValue aVal;
        if (_rValue >>= aVal)
        {
            if (aVal.Name.equalsAscii("DataSource"))
            {
#if OSL_DEBUG_LEVEL > 0
                bool bSuccess =
#endif
                aVal.Value >>= m_xDataSource;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSource!" );
                return;
            }

            if (aVal.Name.equalsAscii("DataSourceName"))
            {
#if OSL_DEBUG_LEVEL > 0
                bool bSuccess =
#endif
                aVal.Value >>= m_sDataSourceName;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSourceName!" );
                return;
            }

            if (aVal.Name.equalsAscii("Command"))
            {
#if OSL_DEBUG_LEVEL > 0
                bool bSuccess =
#endif
                aVal.Value >>= m_sTable;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for Command!" );
                return;
            }
        }

        OAddressBookSourceDialogUnoBase::implInitialize( _rValue );
    }


    Dialog* OAddressBookSourceDialogUno::createDialog(vcl::Window* _pParent)
    {
        if ( m_xDataSource.is() && !m_sTable.isEmpty() )
            return new AddressBookSourceDialog(_pParent, m_aContext, m_xDataSource, m_sDataSourceName, m_sTable, m_aAliases );
        else
            return new AddressBookSourceDialog( _pParent, m_aContext );
    }

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_svtools_OAddressBookSourceDialogUno_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OAddressBookSourceDialogUno(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
