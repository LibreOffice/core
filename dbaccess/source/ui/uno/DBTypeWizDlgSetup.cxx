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
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/container/XSet.hpp>
#include "DBTypeWizDlgSetup.hxx"
#include <dbwizsetup.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <comphelper/processfactory.hxx>
#include <toolkit/helper/vclunohelper.hxx>

using namespace dbaui;

extern "C" void createRegistryInfo_ODBTypeWizDialogSetup()
{
    static OMultiInstanceAutoRegistration< ODBTypeWizDialogSetup > aAutoRegistration;
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;

ODBTypeWizDialogSetup::ODBTypeWizDialogSetup(const Reference< XComponentContext >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
    ,m_bOpenDatabase(true)
    ,m_bStartTableWizard(false)
{
    registerProperty("OpenDatabase", 3, PropertyAttribute::TRANSIENT,
        &m_bOpenDatabase, cppu::UnoType<bool>::get());

    registerProperty("StartTableWizard", 4, PropertyAttribute::TRANSIENT,
        &m_bStartTableWizard, cppu::UnoType<bool>::get());
}

Sequence<sal_Int8> SAL_CALL ODBTypeWizDialogSetup::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

Reference< XInterface > ODBTypeWizDialogSetup::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    Reference < XInterface > xDBWizard = *(new ODBTypeWizDialogSetup( comphelper::getComponentContext(_rxFactory) ));
    return xDBWizard;
}

OUString SAL_CALL ODBTypeWizDialogSetup::getImplementationName()
{
    return getImplementationName_Static();
}

OUString ODBTypeWizDialogSetup::getImplementationName_Static()
{
    return OUString("org.openoffice.comp.dbu.ODBTypeWizDialogSetup");
}

css::uno::Sequence<OUString> SAL_CALL ODBTypeWizDialogSetup::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence<OUString> ODBTypeWizDialogSetup::getSupportedServiceNames_Static()
{
    css::uno::Sequence<OUString> aSupported { "com.sun.star.sdb.DatabaseWizardDialog" };
    return aSupported;
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

svt::OGenericUnoDialog::Dialog ODBTypeWizDialogSetup::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    return svt::OGenericUnoDialog::Dialog(VclPtr<ODbTypeWizDialogSetup>::Create(VCLUnoHelper::GetWindow(rParent), m_pDatasourceItems.get(), m_aContext, m_aInitialSelection));
}

void ODBTypeWizDialogSetup::executedDialog(sal_Int16 _nExecutionResult)
{
    if ( _nExecutionResult == RET_OK )
    {
        const ODbTypeWizDialogSetup* pDialog = static_cast<ODbTypeWizDialogSetup*>(m_aDialog.m_xVclDialog.get());
        m_bOpenDatabase = pDialog->IsDatabaseDocumentToBeOpened();
        m_bStartTableWizard = pDialog->IsTableWizardToBeStarted();
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
