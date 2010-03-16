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

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>

#include "FormControlHelper.hxx"

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

struct FormControlHelper::FormControlHelper_Impl
{
    awt::Size aSize;
    uno::Reference<form::XForm> rForm;
    uno::Reference<form::XFormComponent> rFormComponent;
    uno::Reference<lang::XMultiServiceFactory> rServiceFactory;
    uno::Reference<text::XTextDocument> rTextDocument;
    uno::Reference<text::XTextRange> rTextRange;

    uno::Reference<lang::XMultiServiceFactory> getServiceFactory();
    uno::Reference<form::XForm> getForm();
    uno::Reference<container::XIndexContainer> getFormComps();
};

uno::Reference<lang::XMultiServiceFactory> FormControlHelper::FormControlHelper_Impl::getServiceFactory()
{
    uno::Reference<lang::XMultiServiceFactory> xFactory(rTextDocument, uno::UNO_QUERY);

    return xFactory;
}

uno::Reference<form::XForm> FormControlHelper::FormControlHelper_Impl::getForm()
{
    if (! rForm.is())
    {
        uno::Reference<uno::XInterface>
            xRef(rServiceFactory->createInstance
                 (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.Form"))));
        rForm = uno::Reference<form::XForm>(xRef, uno::UNO_QUERY);
    }

    return rForm;
}

uno::Reference<container::XIndexContainer> FormControlHelper::FormControlHelper_Impl::getFormComps()
{
    uno::Reference<container::XIndexContainer> xIndexContainer(getForm(), uno::UNO_QUERY);

    return xIndexContainer;
}

FormControlHelper::FormControlHelper(uno::Reference<text::XTextDocument> rTextDocument,
                                     FFDataHandler::Pointer_t pFFData)
    : m_pFFData(pFFData), m_pImpl(new FormControlHelper_Impl)
{
    m_pImpl->rTextDocument = rTextDocument;
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
    uno::Reference<container::XIndexContainer> xFormComps(m_pImpl->getFormComps());
    if (! xFormComps.is())
        return false;

    uno::Any aAny;
    aAny <<= m_pImpl->rFormComponent;
    xFormComps->insertByIndex(xFormComps->getCount(), aAny);

    if (! m_pImpl->getServiceFactory().is())
        return false;

    uno::Reference<uno::XInterface> xInterface =
        m_pImpl->getServiceFactory()->createInstance
        (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ControlShape")));

    if (! xInterface.is())
        return false;

    uno::Reference<drawing::XShape> xShape(xInterface, uno::UNO_QUERY);

    if (! xShape.is())
        return false;

    xShape->setSize(m_pImpl->aSize);

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    sal_uInt16 nTmp = text::TextContentAnchorType_AS_CHARACTER;
    aAny <<= nTmp;

    static const ::rtl::OUString sAnchorType(RTL_CONSTASCII_USTRINGPARAM("AnchorType"));
    xShapeProps->setPropertyValue(sAnchorType, aAny);

    static const ::rtl::OUString sVertOrient(RTL_CONSTASCII_USTRINGPARAM("VertOrient"));
    nTmp = text::VertOrientation::TOP;
    aAny <<= nTmp;
    xShapeProps->setPropertyValue(sVertOrient, aAny);

    aAny <<= m_pImpl->rTextRange;

    static const ::rtl::OUString sTextRange(RTL_CONSTASCII_USTRINGPARAM("TextRange"));
    xShapeProps->setPropertyValue(sTextRange, aAny);

    uno::Reference<drawing::XControlShape> xControlShape(xShape, uno::UNO_QUERY);
    uno::Reference<awt::XControlModel> xControlModel(m_pImpl->rFormComponent, uno::UNO_QUERY);
    xControlShape->setControl(xControlModel);

    return true;
}

}}
