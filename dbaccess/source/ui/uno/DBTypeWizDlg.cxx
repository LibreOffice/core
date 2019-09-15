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

#include <dbu_reghelper.hxx>
#include <uiservices.hxx>
#include "DBTypeWizDlg.hxx"
#include <dbwiz.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>

using namespace dbaui;

extern "C" void createRegistryInfo_ODBTypeWizDialog()
{
    static OMultiInstanceAutoRegistration< ODBTypeWizDialog > aAutoRegistration;
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

ODBTypeWizDialog::ODBTypeWizDialog(const Reference< XComponentContext >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
{
}

Sequence<sal_Int8> SAL_CALL ODBTypeWizDialog::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

Reference< XInterface > ODBTypeWizDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODBTypeWizDialog( comphelper::getComponentContext(_rxFactory) ));
}

OUString SAL_CALL ODBTypeWizDialog::getImplementationName()
{
    return getImplementationName_Static();
}

OUString ODBTypeWizDialog::getImplementationName_Static()
{
    return "org.openoffice.comp.dbu.ODBTypeWizDialog";
}

css::uno::Sequence<OUString> SAL_CALL ODBTypeWizDialog::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence<OUString> ODBTypeWizDialog::getSupportedServiceNames_Static()
{
    css::uno::Sequence<OUString> aSupported { "com.sun.star.sdb.DataSourceTypeChangeDialog" };
    return aSupported;
}

Reference<XPropertySetInfo>  SAL_CALL ODBTypeWizDialog::getPropertySetInfo()
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& ODBTypeWizDialog::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODBTypeWizDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

std::unique_ptr<weld::DialogController> ODBTypeWizDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    return std::make_unique<ODbTypeWizDialog>(Application::GetFrameWeld(rParent), m_pDatasourceItems.get(), m_aContext, m_aInitialSelection);
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
