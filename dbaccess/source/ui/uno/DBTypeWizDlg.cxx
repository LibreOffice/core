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
#include "DBTypeWizDlg.hxx"
#include "dbwiz.hxx"
#include <comphelper/processfactory.hxx>

using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_ODBTypeWizDialog()
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

Sequence<sal_Int8> SAL_CALL ODBTypeWizDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

Reference< XInterface > SAL_CALL ODBTypeWizDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODBTypeWizDialog( comphelper::getComponentContext(_rxFactory) ));
}

OUString SAL_CALL ODBTypeWizDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

OUString ODBTypeWizDialog::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.ODBTypeWizDialog");
}

::comphelper::StringSequence SAL_CALL ODBTypeWizDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

::comphelper::StringSequence ODBTypeWizDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = OUString("com.sun.star.sdb.DataSourceTypeChangeDialog");
    return aSupported;
}

Reference<XPropertySetInfo>  SAL_CALL ODBTypeWizDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& ODBTypeWizDialog::getInfoHelper()
{
    return *const_cast<ODBTypeWizDialog*>(this)->getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODBTypeWizDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

Dialog* ODBTypeWizDialog::createDialog(Window* _pParent)
{
    ODbTypeWizDialog* pDlg = new ODbTypeWizDialog(_pParent, m_pDatasourceItems, m_aContext, m_aInitialSelection);
    return pDlg;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
