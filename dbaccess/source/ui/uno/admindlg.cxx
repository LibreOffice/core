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

#include "admindlg.hxx"
#include <dbadmin.hxx>
#include <vcl/svapp.hxx>

using namespace dbaui;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_ODatasourceAdministrationDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ODataSourcePropertyDialog(context));
}

namespace dbaui
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

ODataSourcePropertyDialog::ODataSourcePropertyDialog(const Reference<XComponentContext>& _rxORB)
    : ODatabaseAdministrationDialog(_rxORB)
{
}

Sequence<sal_Int8> SAL_CALL ODataSourcePropertyDialog::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SAL_CALL ODataSourcePropertyDialog::getImplementationName()
{
    return u"org.openoffice.comp.dbu.ODatasourceAdministrationDialog"_ustr;
}

css::uno::Sequence<OUString> SAL_CALL ODataSourcePropertyDialog::getSupportedServiceNames()
{
    return { u"com.sun.star.sdb.DatasourceAdministrationDialog"_ustr };
}

Reference<XPropertySetInfo> SAL_CALL ODataSourcePropertyDialog::getPropertySetInfo()
{
    Reference<XPropertySetInfo> xInfo(createPropertySetInfo(getInfoHelper()));
    return xInfo;
}

::cppu::IPropertyArrayHelper& ODataSourcePropertyDialog::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* ODataSourcePropertyDialog::createArrayHelper() const
{
    Sequence<Property> aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

std::unique_ptr<weld::DialogController>
ODataSourcePropertyDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    std::unique_ptr<ODbAdminDialog> xDialog(new ODbAdminDialog(
        Application::GetFrameWeld(rParent), m_pDatasourceItems.get(), m_aContext));

    // the initial selection
    if (m_aInitialSelection.hasValue())
        xDialog->selectDataSource(m_aInitialSelection);

    return xDialog;
}

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
