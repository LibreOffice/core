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
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertysequence.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <vcl/svapp.hxx>
#include <rtl/ref.hxx>

using namespace svt;

namespace {

#define UNODIALOG_PROPERTY_ID_ALIASES       100
#define UNODIALOG_PROPERTY_ALIASES          "FieldMapping"

    using namespace css::uno;
    using namespace css::lang;
    using namespace css::util;
    using namespace css::beans;
    using namespace css::sdbc;

    class OAddressBookSourceDialogUno
            :public OGenericUnoDialog
            ,public ::comphelper::OPropertyArrayUsageHelper< OAddressBookSourceDialogUno >
    {
    private:
        Sequence< AliasProgrammaticPair >   m_aAliases;
        Reference< XDataSource >            m_xDataSource;
        OUString                     m_sDataSourceName;
        OUString                     m_sTable;

    public:
        explicit OAddressBookSourceDialogUno(const Reference< XComponentContext >& _rxORB);

        // XTypeProvider
        virtual Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        // XPropertySet
        virtual Reference< XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        virtual void SAL_CALL initialize(const Sequence< Any >& aArguments) override;

    protected:
    // OGenericUnoDialog overridables
        virtual svt::OGenericUnoDialog::Dialog createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;

        virtual void implInitialize(const css::uno::Any& _rValue) override;

        virtual void executedDialog(sal_Int16 _nExecutionResult) override;
    };


    OAddressBookSourceDialogUno::OAddressBookSourceDialogUno(const Reference< XComponentContext >& _rxORB)
        :OGenericUnoDialog(_rxORB)
    {
        registerProperty(UNODIALOG_PROPERTY_ALIASES, UNODIALOG_PROPERTY_ID_ALIASES, PropertyAttribute::READONLY,
            &m_aAliases, cppu::UnoType<decltype(m_aAliases)>::get());
    }


    Sequence<sal_Int8> SAL_CALL OAddressBookSourceDialogUno::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }


    OUString SAL_CALL OAddressBookSourceDialogUno::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.OAddressBookSourceDialogUno" );
    }


    css::uno::Sequence<OUString> SAL_CALL OAddressBookSourceDialogUno::getSupportedServiceNames()
    {
        css::uno::Sequence<OUString> aSupported { "com.sun.star.ui.AddressBookSourceDialog" };
        return aSupported;
    }


    Reference<XPropertySetInfo>  SAL_CALL OAddressBookSourceDialogUno::getPropertySetInfo()
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
        OGenericUnoDialog::executedDialog(_nExecutionResult);

        if ( _nExecutionResult && m_aDialog )
            static_cast<AddressBookSourceDialog*>(m_aDialog.m_xWeldDialog.get())->getFieldMapping(m_aAliases);
    }

    void SAL_CALL OAddressBookSourceDialogUno::initialize(const Sequence< Any >& rArguments)
    {
        if( rArguments.getLength() == 5 )
        {
            Reference<css::awt::XWindow> xParentWindow;
            Reference<css::beans::XPropertySet> xDataSource;
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
                Sequence<Any> aArguments(comphelper::InitAnyPropertySequence(
                {
                    {"ParentWindow", Any(xParentWindow)},
                    {"DataSource", Any(xDataSource)},
                    {"DataSourceName", Any(sDataSourceName)},
                    {"Command", Any(sCommand)}, // the table to use
                    {"Title", Any(sTitle)}
                }));
                OGenericUnoDialog::initialize(aArguments);
                return;
            }
        }
        OGenericUnoDialog::initialize(rArguments);
    }

    void OAddressBookSourceDialogUno::implInitialize(const css::uno::Any& _rValue)
    {
        PropertyValue aVal;
        if (_rValue >>= aVal)
        {
            if (aVal.Name == "DataSource")
            {
                bool bSuccess = aVal.Value >>= m_xDataSource;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSource!" );
                return;
            }

            if (aVal.Name == "DataSourceName")
            {
                bool bSuccess = aVal.Value >>= m_sDataSourceName;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for DataSourceName!" );
                return;
            }

            if (aVal.Name == "Command")
            {
                bool bSuccess = aVal.Value >>= m_sTable;
                OSL_ENSURE( bSuccess, "OAddressBookSourceDialogUno::implInitialize: invalid type for Command!" );
                return;
            }
        }

        OGenericUnoDialog::implInitialize( _rValue );
    }

    svt::OGenericUnoDialog::Dialog OAddressBookSourceDialogUno::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        weld::Window* pParent = Application::GetFrameWeld(rParent);
        if ( m_xDataSource.is() && !m_sTable.isEmpty() )
            return svt::OGenericUnoDialog::Dialog(std::make_unique<AddressBookSourceDialog>(pParent, m_aContext, m_xDataSource, m_sDataSourceName, m_sTable, m_aAliases));
        else
            return svt::OGenericUnoDialog::Dialog(std::make_unique<AddressBookSourceDialog>(pParent, m_aContext));
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_svtools_OAddressBookSourceDialogUno_get_implementation(
    css::uno::XComponentContext * context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new OAddressBookSourceDialogUno(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
