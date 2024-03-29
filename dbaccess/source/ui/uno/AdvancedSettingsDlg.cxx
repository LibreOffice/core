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

#include <unoadmin.hxx>
#include <advancedsettingsdlg.hxx>
#include <comphelper/proparrhlp.hxx>
#include <vcl/svapp.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    namespace {

    // OAdvancedSettingsDialog
    class OAdvancedSettingsDialog
            :public ODatabaseAdministrationDialog
            ,public ::comphelper::OPropertyArrayUsageHelper< OAdvancedSettingsDialog >
    {

    public:
        explicit OAdvancedSettingsDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxORB);

        // XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
    protected:
    // OGenericUnoDialog overridables
        virtual std::unique_ptr<weld::DialogController> createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) override;
    };

    }

    OAdvancedSettingsDialog::OAdvancedSettingsDialog(const Reference< XComponentContext >& _rxORB)
        :ODatabaseAdministrationDialog(_rxORB)
    {
    }
    Sequence<sal_Int8> SAL_CALL OAdvancedSettingsDialog::getImplementationId(  )
    {
        return css::uno::Sequence<sal_Int8>();
    }

    OUString SAL_CALL OAdvancedSettingsDialog::getImplementationName()
    {
        return "org.openoffice.comp.dbu.OAdvancedSettingsDialog";
    }

    css::uno::Sequence<OUString> SAL_CALL OAdvancedSettingsDialog::getSupportedServiceNames()
    {
        return { "com.sun.star.sdb.AdvancedDatabaseSettingsDialog" };
    }

    Reference<XPropertySetInfo>  SAL_CALL OAdvancedSettingsDialog::getPropertySetInfo()
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

    std::unique_ptr<weld::DialogController> OAdvancedSettingsDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
    {
        return std::make_unique<AdvancedSettingsDialog>(Application::GetFrameWeld(rParent), m_pDatasourceItems.get(),
                                                        m_aContext, m_aInitialSelection);
    }

}   // namespace dbaui

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_OAdvancedSettingsDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::OAdvancedSettingsDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
