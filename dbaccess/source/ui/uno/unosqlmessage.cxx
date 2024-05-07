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

#include <sqlmessage.hxx>
#include <unosqlmessage.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
#include <comphelper/propertysequence.hxx>
#include <connectivity/dbexception.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>

using namespace dbaui;
using namespace dbtools;

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_OSQLMessageDialog_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new OSQLMessageDialog(context));
}

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

OSQLMessageDialog::OSQLMessageDialog(const Reference< XComponentContext >& _rxORB)
    :OSQLMessageDialogBase(_rxORB)
{
    registerMayBeVoidProperty(PROPERTY_SQLEXCEPTION, PROPERTY_ID_SQLEXCEPTION, PropertyAttribute::TRANSIENT | PropertyAttribute::MAYBEVOID,
        &m_aException, ::cppu::UnoType<SQLException>::get());
    registerProperty( PROPERTY_HELP_URL, PROPERTY_ID_HELP_URL, PropertyAttribute::TRANSIENT,
        &m_sHelpURL, cppu::UnoType<decltype(m_sHelpURL)>::get() );
}

Sequence<sal_Int8> SAL_CALL OSQLMessageDialog::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

OUString SAL_CALL OSQLMessageDialog::getImplementationName()
{
    return u"org.openoffice.comp.dbu.OSQLMessageDialog"_ustr;
}

css::uno::Sequence<OUString> SAL_CALL OSQLMessageDialog::getSupportedServiceNames()
{
    return { u"com.sun.star.sdb.ErrorMessageDialog"_ustr };
}

void OSQLMessageDialog::initialize(Sequence<Any> const & args)
{
    OUString title;
    Reference< css::awt::XWindow > parentWindow;

    if ((args.getLength() == 3) && (args[0] >>= title) && (args[1] >>= parentWindow)) {
        Sequence<Any> s(comphelper::InitAnyPropertySequence(
        {
            {"Title", Any(title)},
            {"ParentWindow", Any(parentWindow)},
            {"SQLException", args[2]}
        }));
        OGenericUnoDialog::initialize(s);
    } else {
        OGenericUnoDialog::initialize(args);
    }
}

sal_Bool SAL_CALL OSQLMessageDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_SQLEXCEPTION:
        {
            SQLExceptionInfo aInfo(_rValue);
            if (!aInfo.isValid())
                throw IllegalArgumentException();

            _rOldValue = m_aException;
            _rConvertedValue = aInfo.get();

            return true;
                // always assume "modified", don't bother with comparing the two values
        }
        default:
            return OSQLMessageDialogBase::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
}

Reference<XPropertySetInfo>  SAL_CALL OSQLMessageDialog::getPropertySetInfo()
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& OSQLMessageDialog::getInfoHelper()
{
    return *getArrayHelper();
}

::cppu::IPropertyArrayHelper* OSQLMessageDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

std::unique_ptr<weld::DialogController> OSQLMessageDialog::createDialog(const css::uno::Reference<css::awt::XWindow>& rParent)
{
    weld::Window* pParent = Application::GetFrameWeld(rParent);
    if ( m_aException.hasValue() )
        return std::make_unique<OSQLMessageBox>(pParent, SQLExceptionInfo(m_aException), MessBoxStyle::Ok | MessBoxStyle::DefaultOk, m_sHelpURL);

    OSL_FAIL("OSQLMessageDialog::createDialog : You should use the SQLException property to specify the error to display!");
    return std::make_unique<OSQLMessageBox>(pParent, SQLException());
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
