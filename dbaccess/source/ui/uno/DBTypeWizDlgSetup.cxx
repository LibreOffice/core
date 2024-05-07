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

#include "DBTypeWizDlgSetup.hxx"
#include <dbwizsetup.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <vcl/svapp.hxx>

using namespace dbaui;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_ODBTypeWizDialogSetup_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ODBTypeWizDialogSetup(context));
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

ODBTypeWizDialogSetup::ODBTypeWizDialogSetup(const Reference< XComponentContext >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
    ,m_bOpenDatabase(true)
    ,m_bStartTableWizard(false)
{
    registerProperty(u"OpenDatabase"_ustr, 3, PropertyAttribute::TRANSIENT,
        &m_bOpenDatabase, cppu::UnoType<bool>::get());

    registerProperty(u"StartTableWizard"_ustr, 4, PropertyAttribute::TRANSIENT,
        &m_bStartTableWizard, cppu::UnoType<bool>::get());
}

Sequence<sal_Int8> SAL_CALL ODBTypeWizDialogSetup::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SAL_CALL ODBTypeWizDialogSetup::getImplementationName()
{
    return u"org.openoffice.comp.dbu.ODBTypeWizDialogSetup"_ustr;
}

css::uno::Sequence<OUString> SAL_CALL ODBTypeWizDialogSetup::getSupportedServiceNames()
{
    return { u"com.sun.star.sdb.DatabaseWizardDialog"_ustr };
}

Reference<XPropertySetInfo>  SAL_CALL ODBTypeWizDialogSetup::getPropertySetInfo()
{
    return createPropertySetInfo( getInfoHelper() );
}

::cppu::IPropertyArrayHelper& ODBTypeWizDialogSetup::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODBTypeWizDialogSetup::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

std::unique_ptr<weld::DialogController> ODBTypeWizDialogSetup::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    return std::make_unique<ODbTypeWizDialogSetup>(Application::GetFrameWeld(rParent), m_pDatasourceItems.get(), m_aContext, m_aInitialSelection);
}

void ODBTypeWizDialogSetup::executedDialog(sal_Int16 nExecutionResult)
{
    if (nExecutionResult == css::ui::dialogs::ExecutableDialogResults::OK)
    {
        const ODbTypeWizDialogSetup* pDialog = static_cast<ODbTypeWizDialogSetup*>(m_xDialog.get());
        m_bOpenDatabase = pDialog->IsDatabaseDocumentToBeOpened();
        m_bStartTableWizard = pDialog->IsTableWizardToBeStarted();
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
