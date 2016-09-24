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
#include "uiservices.hxx"
#include "admindlg.hxx"
#include "dbadmin.hxx"
#include <comphelper/processfactory.hxx>

using namespace dbaui;

extern "C" void SAL_CALL createRegistryInfo_ODataSourcePropertyDialog()
{
    static OMultiInstanceAutoRegistration< ODataSourcePropertyDialog > aAutoRegistration;
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

ODataSourcePropertyDialog::ODataSourcePropertyDialog(const Reference< XComponentContext >& _rxORB)
    :ODatabaseAdministrationDialog(_rxORB)
{
}

Sequence<sal_Int8> SAL_CALL ODataSourcePropertyDialog::getImplementationId(  ) throw(RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

Reference< XInterface > SAL_CALL ODataSourcePropertyDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new ODataSourcePropertyDialog( comphelper::getComponentContext(_rxFactory) ));
}

OUString SAL_CALL ODataSourcePropertyDialog::getImplementationName() throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

OUString ODataSourcePropertyDialog::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.ODatasourceAdministrationDialog");
}

css::uno::Sequence<OUString> SAL_CALL ODataSourcePropertyDialog::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence<OUString> ODataSourcePropertyDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    css::uno::Sequence<OUString> aSupported { "com.sun.star.sdb.DatasourceAdministrationDialog" };
    return aSupported;
}

Reference<XPropertySetInfo>  SAL_CALL ODataSourcePropertyDialog::getPropertySetInfo() throw(RuntimeException, std::exception)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& ODataSourcePropertyDialog::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODataSourcePropertyDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

VclPtr<Dialog> ODataSourcePropertyDialog::createDialog(vcl::Window* _pParent)
{
    VclPtrInstance<ODbAdminDialog> pDialog(_pParent, m_pDatasourceItems, m_aContext);

    // the initial selection
    if ( m_aInitialSelection.hasValue() )
        pDialog->selectDataSource(m_aInitialSelection);

    return pDialog;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
