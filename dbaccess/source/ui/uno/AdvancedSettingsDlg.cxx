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

#include <sal/config.h>

#include "uiservices.hxx"
#include "unoadmin.hxx"
#include "dbu_reghelper.hxx"
#include "advancedsettingsdlg.hxx"
#include <comphelper/processfactory.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    // OAdvancedSettingsDialog
    class OAdvancedSettingsDialog
            :public ODatabaseAdministrationDialog
            ,public ::comphelper::OPropertyArrayUsageHelper< OAdvancedSettingsDialog >
    {

    protected:
        explicit OAdvancedSettingsDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

    public:
        // XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_Static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
    protected:
    // OGenericUnoDialog overridables
        virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override;
    };

    OAdvancedSettingsDialog::OAdvancedSettingsDialog(const Reference< XComponentContext >& _rxORB)
        :ODatabaseAdministrationDialog(_rxORB)
    {
    }
    Sequence<sal_Int8> SAL_CALL OAdvancedSettingsDialog::getImplementationId(  ) throw(RuntimeException, std::exception)
    {
        return css::uno::Sequence<sal_Int8>();
    }

    Reference< XInterface > SAL_CALL OAdvancedSettingsDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OAdvancedSettingsDialog( comphelper::getComponentContext(_rxFactory) ));
    }

    OUString SAL_CALL OAdvancedSettingsDialog::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_Static();
    }

    OUString OAdvancedSettingsDialog::getImplementationName_Static() throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.dbu.OAdvancedSettingsDialog");
    }

    css::uno::Sequence<OUString> SAL_CALL OAdvancedSettingsDialog::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_Static();
    }

    css::uno::Sequence<OUString> OAdvancedSettingsDialog::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        css::uno::Sequence<OUString> aSupported(1);
        aSupported[0] = "com.sun.star.sdb.AdvancedDatabaseSettingsDialog";
        return aSupported;
    }

    Reference<XPropertySetInfo>  SAL_CALL OAdvancedSettingsDialog::getPropertySetInfo() throw(RuntimeException, std::exception)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    ::cppu::IPropertyArrayHelper& OAdvancedSettingsDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OAdvancedSettingsDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    VclPtr<Dialog> OAdvancedSettingsDialog::createDialog(vcl::Window* _pParent)
    {
        return VclPtr<AdvancedSettingsDialog>::Create(_pParent, m_pDatasourceItems, m_aContext, m_aInitialSelection);
    }

}   // namespace dbaui

extern "C" void SAL_CALL createRegistryInfo_OAdvancedSettingsDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OAdvancedSettingsDialog > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
