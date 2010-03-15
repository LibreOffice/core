/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "FormControlHelper.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

struct FormControlHelper::FormControlHelper_Impl
{
    uno::Reference<lang::XMultiServiceFactory> rServiceFactory;
    uno::Reference<form::XFormComponent> rFormComponent;
    awt::Size aSize;
};

FormControlHelper::FormControlHelper(FFDataHandler::Pointer_t pFFData)
    : m_pFFData(pFFData), m_pImpl(new FormControlHelper_Impl)
{
}

FormControlHelper::~FormControlHelper()
{
}

bool FormControlHelper::insertCheckBox()
{
    return true;
}

bool FormControlHelper::createCheckbox()
{
    uno::Reference<uno::XInterface> xInterface =
        m_pImpl->rServiceFactory->createInstance
        (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.CheckBox")));

    if (!xInterface.is())
        return false;

    m_pImpl->rFormComponent = uno::Reference<form::XFormComponent>(xInterface, uno::UNO_QUERY);
    if (!m_pImpl->rFormComponent.is())
        return false;

    uno::Reference<beans::XPropertySet> xPropSet(xInterface, uno::UNO_QUERY);

    m_pImpl->aSize.Width = 16 * m_pFFData->getCheckboxHeight();
    m_pImpl->aSize.Height = m_pImpl->aSize.Width;

    uno::Any aAny;
    if (m_pFFData->getStatusText().getLength())
    {
        aAny <<= m_pFFData->getStatusText();

        xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")), aAny);
        xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpText")), aAny);
    }

    aAny <<= m_pFFData->getCheckboxChecked();
    xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultState")), aAny);

    if (m_pFFData->getHelpText().getLength())
    {
        aAny <<= m_pFFData->getHelpText();
        xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpF1Text")), aAny);
    }

    return true;
}

bool FormControlHelper::insertControl()
{
    return true;
}

}}
