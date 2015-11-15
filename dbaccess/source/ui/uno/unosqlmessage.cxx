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

#include "sqlmessage.hxx"
#include "uiservices.hxx"
#include "unosqlmessage.hxx"
#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>

using namespace dbaui;
using namespace dbtools;

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;

extern "C" void SAL_CALL createRegistryInfo_OSQLMessageDialog()
{
    static OMultiInstanceAutoRegistration< OSQLMessageDialog > aAutoRegistration;
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

Sequence<sal_Int8> SAL_CALL OSQLMessageDialog::getImplementationId(  ) throw(RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

Reference< XInterface > SAL_CALL OSQLMessageDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OSQLMessageDialog( comphelper::getComponentContext(_rxFactory) ));
}

OUString SAL_CALL OSQLMessageDialog::getImplementationName() throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

OUString OSQLMessageDialog::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.OSQLMessageDialog");
}

css::uno::Sequence<OUString> SAL_CALL OSQLMessageDialog::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

css::uno::Sequence<OUString> OSQLMessageDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    css::uno::Sequence<OUString> aSupported { "com.sun.star.sdb.ErrorMessageDialog" };
    return aSupported;
}

void OSQLMessageDialog::initialize(Sequence<Any> const & args) throw (css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    OUString title;
    Reference< css::awt::XWindow > parentWindow;
    css::uno::Any sqlException;

    if ((args.getLength() == 3) && (args[0] >>= title) && (args[1] >>= parentWindow) && (args[2] >>= sqlException)) {
        Sequence<Any> s(3);
        s[0] <<= PropertyValue( "Title", -1, makeAny(title), PropertyState_DIRECT_VALUE);
        s[1] <<= PropertyValue( "ParentWindow", -1, makeAny(parentWindow), PropertyState_DIRECT_VALUE);
        s[2] <<= PropertyValue( "SQLException", -1, sqlException, PropertyState_DIRECT_VALUE);
        OGenericUnoDialog::initialize(s);
    } else {
        OGenericUnoDialog::initialize(args);
    }
}

sal_Bool SAL_CALL OSQLMessageDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue) throw(IllegalArgumentException)
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

            return sal_True;
                // always assume "modified", don't bother with comparing the two values
        }
        default:
            return OSQLMessageDialogBase::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
}

Reference<XPropertySetInfo>  SAL_CALL OSQLMessageDialog::getPropertySetInfo() throw(RuntimeException, std::exception)
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

VclPtr<Dialog> OSQLMessageDialog::createDialog(vcl::Window* _pParent)
{
    if ( m_aException.hasValue() )
        return VclPtr<OSQLMessageBox>::Create( _pParent, SQLExceptionInfo( m_aException ), WB_OK | WB_DEF_OK, m_sHelpURL );

    OSL_FAIL("OSQLMessageDialog::createDialog : You should use the SQLException property to specify the error to display!");
    return VclPtr<OSQLMessageBox>::Create(_pParent, SQLException());
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
