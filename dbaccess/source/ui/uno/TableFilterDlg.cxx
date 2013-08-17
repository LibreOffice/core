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
#include "TableFilterDlg.hxx"
#include "TablesSingleDlg.hxx"
#include <comphelper/processfactory.hxx>

using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_OTableFilterDialog()
{
    static OMultiInstanceAutoRegistration< OTableFilterDialog > aAutoRegistration;
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

OTableFilterDialog::OTableFilterDialog(const Reference< XComponentContext >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
{
}

Sequence<sal_Int8> SAL_CALL OTableFilterDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

Reference< XInterface > SAL_CALL OTableFilterDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OTableFilterDialog( comphelper::getComponentContext(_rxFactory) ));
}

OUString SAL_CALL OTableFilterDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

OUString OTableFilterDialog::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.OTableFilterDialog");
}

::comphelper::StringSequence SAL_CALL OTableFilterDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

::comphelper::StringSequence OTableFilterDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = OUString("com.sun.star.sdb.TableFilterDialog");
    return aSupported;
}

Reference<XPropertySetInfo>  SAL_CALL OTableFilterDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& OTableFilterDialog::getInfoHelper()
{
    return *const_cast<OTableFilterDialog*>(this)->getArrayHelper();
}

::cppu::IPropertyArrayHelper* OTableFilterDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

Dialog* OTableFilterDialog::createDialog(Window* _pParent)
{
    OTableSubscriptionDialog* pDlg = new OTableSubscriptionDialog(_pParent, m_pDatasourceItems, m_aContext, m_aInitialSelection);
    return pDlg;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
