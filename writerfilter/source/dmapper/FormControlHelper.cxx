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

#include <math.h>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
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
    FieldId m_eFieldId;
    awt::Size aSize;
    uno::Reference<drawing::XDrawPage> rDrawPage;
    uno::Reference<form::XForm> rForm;
    uno::Reference<form::XFormComponent> rFormComponent;
    uno::Reference<lang::XMultiServiceFactory> rServiceFactory;
    uno::Reference<text::XTextDocument> rTextDocument;

    uno::Reference<drawing::XDrawPage> getDrawPage();
    uno::Reference<lang::XMultiServiceFactory> getServiceFactory();
    uno::Reference<form::XForm> getForm();
    uno::Reference<container::XIndexContainer> getFormComps();
};

uno::Reference<drawing::XDrawPage> FormControlHelper::FormControlHelper_Impl::getDrawPage()
{
    if (! rDrawPage.is())
    {
        uno::Reference<drawing::XDrawPageSupplier> 
            xDrawPageSupplier(rTextDocument, uno::UNO_QUERY);
        if (xDrawPageSupplier.is())
            rDrawPage = xDrawPageSupplier->getDrawPage();
    }

    return rDrawPage;
}

uno::Reference<lang::XMultiServiceFactory> FormControlHelper::FormControlHelper_Impl::getServiceFactory()
{
    if (! rServiceFactory.is())
        rServiceFactory = uno::Reference<lang::XMultiServiceFactory>(rTextDocument, uno::UNO_QUERY);

    return rServiceFactory;
}

uno::Reference<form::XForm> FormControlHelper::FormControlHelper_Impl::getForm()
{
    if (! rForm.is())
    {
        uno::Reference<form::XFormsSupplier> xFormsSupplier(getDrawPage(), uno::UNO_QUERY);
        
        if (xFormsSupplier.is())
        {
            uno::Reference<container::XNameContainer> xFormsNamedContainer(xFormsSupplier->getForms());
            static ::rtl::OUString sDOCXForm(RTL_CONSTASCII_USTRINGPARAM("DOCX-Standard"));

            ::rtl::OUString sFormName(sDOCXForm);
            sal_uInt16 nUnique = 0;

            while (xFormsNamedContainer->hasByName(sFormName))
            {
                ++nUnique;
                sFormName = sDOCXForm;
                sFormName += ::rtl::OUString::valueOf(nUnique);
            }

            uno::Reference<uno::XInterface> 
                xForm(getServiceFactory()->createInstance
                      (::rtl::OUString
                       (RTL_CONSTASCII_USTRINGPARAM
                        ("com.sun.star.form.component.Form"))));
            if (xForm.is())
            {
                uno::Reference<beans::XPropertySet> 
                    xFormProperties(xForm, uno::UNO_QUERY);
                uno::Any aAny(sFormName);
                static ::rtl::OUString sName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")));
                xFormProperties->setPropertyValue(sName, aAny);
            }

            rForm = uno::Reference<form::XForm>(xForm, uno::UNO_QUERY);

            uno::Reference<container::XIndexContainer> xForms(xFormsNamedContainer, uno::UNO_QUERY);
            uno::Any aAny(xForm);
            xForms->insertByIndex(xForms->getCount(), aAny);
        }
    }

    return rForm;
}

uno::Reference<container::XIndexContainer> FormControlHelper::FormControlHelper_Impl::getFormComps()
{
    uno::Reference<container::XIndexContainer> xIndexContainer(getForm(), uno::UNO_QUERY);

    return xIndexContainer;
}

FormControlHelper::FormControlHelper(FieldId eFieldId,
                                     uno::Reference<text::XTextDocument> rTextDocument,
                                     FFDataHandler::Pointer_t pFFData)
    : m_pFFData(pFFData), m_pImpl(new FormControlHelper_Impl)
{
    m_pImpl->m_eFieldId = eFieldId;
    m_pImpl->rTextDocument = rTextDocument;
}

FormControlHelper::~FormControlHelper()
{  
}

bool FormControlHelper::createCheckbox(uno::Reference<text::XTextRange> xTextRange,
                                       const ::rtl::OUString & rControlName)
{
    uno::Reference<lang::XMultiServiceFactory> 
        xServiceFactory(m_pImpl->getServiceFactory());

    if (! xServiceFactory.is())
        return false;

    uno::Reference<uno::XInterface> xInterface = 
        xServiceFactory->createInstance
        (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.CheckBox")));
         
    if (!xInterface.is())
        return false;

    m_pImpl->rFormComponent = uno::Reference<form::XFormComponent>(xInterface, uno::UNO_QUERY);
    if (!m_pImpl->rFormComponent.is())
        return false;

    uno::Reference<beans::XPropertySet> xPropSet(xInterface, uno::UNO_QUERY);

    sal_uInt32 nCheckBoxHeight = 16 * m_pFFData->getCheckboxHeight();

    if (m_pFFData->getCheckboxAutoHeight())
    {
        uno::Reference<beans::XPropertySet> xTextRangeProps(xTextRange, uno::UNO_QUERY);

        try 
        {
            static ::rtl::OUString sCharHeight(RTL_CONSTASCII_USTRINGPARAM("CharHeight"));
            float fCheckBoxHeight = 0.0;
            xTextRangeProps->getPropertyValue(sCharHeight) >>= fCheckBoxHeight;
            nCheckBoxHeight = floor(fCheckBoxHeight * 35.3);
        }
        catch (beans::UnknownPropertyException & rException)
        {
        }
    }
    
    m_pImpl->aSize.Width = nCheckBoxHeight;
    m_pImpl->aSize.Height = m_pImpl->aSize.Width;

    uno::Any aAny;
    if (m_pFFData->getStatusText().getLength())
    {
        aAny <<= m_pFFData->getStatusText();
        
        xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpText")), aAny);
    }

    aAny <<= m_pFFData->getCheckboxChecked();
    xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultState")), aAny);

    if (m_pFFData->getHelpText().getLength())
    {
        aAny <<= m_pFFData->getHelpText();
        xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HelpF1Text")), aAny);
    }
    
    aAny <<= rControlName;
    xPropSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Name")), aAny);

    return true;
}

bool FormControlHelper::insertControl(uno::Reference<text::XTextRange> xTextRange)
{
    bool bCreated = false;
    
    uno::Reference<container::XNameContainer> xFormCompsByName(m_pImpl->getForm(), uno::UNO_QUERY);
    uno::Reference<container::XIndexContainer> xFormComps(m_pImpl->getFormComps());
    if (! xFormComps.is())
        return false;
    
    static ::rtl::OUString sControl(RTL_CONSTASCII_USTRINGPARAM("Control"));

    sal_Int32 nControl = 0;
    bool bDone = false;
    ::rtl::OUString sControlName;

    do
    {
        ::rtl::OUString sTmp(sControl);
        sTmp += ::rtl::OUString::valueOf(nControl);

        nControl++;
        if (! xFormCompsByName->hasByName(sTmp))
        {
            sControlName = sTmp;
            bDone = true;
        } 
    }
    while (! bDone);

    switch (m_pImpl->m_eFieldId)
    {
    case FIELD_FORMCHECKBOX:
        bCreated = createCheckbox(xTextRange, sControlName);
        break;
    default:
        break;
    }

    if (!bCreated)
        return false;

    uno::Any aAny(m_pImpl->rFormComponent); 
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
    nTmp = text::VertOrientation::CENTER;
    aAny <<= nTmp;
    xShapeProps->setPropertyValue(sVertOrient, aAny);

    aAny <<= xTextRange;

    static const ::rtl::OUString sTextRange(RTL_CONSTASCII_USTRINGPARAM("TextRange"));
    xShapeProps->setPropertyValue(sTextRange, aAny);

    uno::Reference<drawing::XControlShape> xControlShape(xShape, uno::UNO_QUERY);
    uno::Reference<awt::XControlModel> xControlModel(m_pImpl->rFormComponent, uno::UNO_QUERY);
    xControlShape->setControl(xControlModel);

    m_pImpl->getDrawPage()->add(xShape);

    return true;
}

}}
