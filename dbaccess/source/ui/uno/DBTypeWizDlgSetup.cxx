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

#include "dbu_reghelper.hxx"
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/container/XSet.hpp>
#include "DBTypeWizDlgSetup.hxx"
#include "dbwizsetup.hxx"
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <vcl/msgbox.hxx>
#include <comphelper/processfactory.hxx>

using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_ODBTypeWizDialogSetup()
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
    ,m_bOpenDatabase(sal_True)
    ,m_bStartTableWizard(sal_False)
{
    registerProperty(OUString("OpenDatabase"), 3, PropertyAttribute::TRANSIENT,
        &m_bOpenDatabase, getBooleanCppuType());

    registerProperty(OUString("StartTableWizard"), 4, PropertyAttribute::TRANSIENT,
        &m_bStartTableWizard, getBooleanCppuType());
}

Sequence<sal_Int8> SAL_CALL ODBTypeWizDialogSetup::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

Reference< XInterface > SAL_CALL ODBTypeWizDialogSetup::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    Reference < XInterface > xDBWizard = *(new ODBTypeWizDialogSetup( comphelper::getComponentContext(_rxFactory) ));
    return xDBWizard;
}

OUString SAL_CALL ODBTypeWizDialogSetup::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

OUString ODBTypeWizDialogSetup::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.ODBTypeWizDialogSetup");
}

::comphelper::StringSequence SAL_CALL ODBTypeWizDialogSetup::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

::comphelper::StringSequence ODBTypeWizDialogSetup::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = OUString("com.sun.star.sdb.DatabaseWizardDialog");
    return aSupported;
}

Reference<XPropertySetInfo>  SAL_CALL ODBTypeWizDialogSetup::getPropertySetInfo() throw(RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() );
}

::cppu::IPropertyArrayHelper& ODBTypeWizDialogSetup::getInfoHelper()
{
    return *const_cast<ODBTypeWizDialogSetup*>(this)->getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODBTypeWizDialogSetup::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

Dialog* ODBTypeWizDialogSetup::createDialog(Window* _pParent)
{
    return new ODbTypeWizDialogSetup(_pParent, m_pDatasourceItems, m_aContext, m_aInitialSelection);
}

void ODBTypeWizDialogSetup::executedDialog(sal_Int16 _nExecutionResult)
{
    if ( _nExecutionResult == RET_OK )
    {
        const ODbTypeWizDialogSetup* pDialog = static_cast< ODbTypeWizDialogSetup* >( m_pDialog );
        m_bOpenDatabase = pDialog->IsDatabaseDocumentToBeOpened();
        m_bStartTableWizard = pDialog->IsTableWizardToBeStarted();
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
