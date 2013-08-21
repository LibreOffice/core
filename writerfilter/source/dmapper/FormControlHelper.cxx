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

#include <math.h>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
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
#include <xmloff/odffields.hxx>
#include <comphelper/stlunosequence.hxx>

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
            static OUString sDOCXForm("DOCX-Standard");

            OUString sFormName(sDOCXForm);
            sal_uInt16 nUnique = 0;

            while (xFormsNamedContainer->hasByName(sFormName))
            {
                ++nUnique;
                sFormName = sDOCXForm;
                sFormName += OUString(nUnique);
            }

            uno::Reference<uno::XInterface> xForm(getServiceFactory()->createInstance("com.sun.star.form.component.Form"));
            if (xForm.is())
            {
                uno::Reference<beans::XPropertySet>
                    xFormProperties(xForm, uno::UNO_QUERY);
                uno::Any aAny(sFormName);
                xFormProperties->setPropertyValue("Name", aAny);
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
                                       const OUString & rControlName)
{
    if ( !m_pFFData )
        return false;
    uno::Reference<lang::XMultiServiceFactory>
        xServiceFactory(m_pImpl->getServiceFactory());

    if (! xServiceFactory.is())
        return false;

    uno::Reference<uno::XInterface> xInterface = xServiceFactory->createInstance("com.sun.star.form.component.CheckBox");

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
            float fCheckBoxHeight = 0.0;
            xTextRangeProps->getPropertyValue("CharHeight") >>= fCheckBoxHeight;
            nCheckBoxHeight = static_cast<sal_uInt32>(floor(fCheckBoxHeight * 35.3));
        }
        catch (beans::UnknownPropertyException &)
        {
        }
    }

    m_pImpl->aSize.Width = nCheckBoxHeight;
    m_pImpl->aSize.Height = m_pImpl->aSize.Width;

    uno::Any aAny;
    if (!m_pFFData->getStatusText().isEmpty())
    {
        aAny <<= m_pFFData->getStatusText();

        xPropSet->setPropertyValue("HelpText", aAny);
    }

    aAny <<= m_pFFData->getCheckboxChecked();
    xPropSet->setPropertyValue("DefaultState", aAny);

    if (!m_pFFData->getHelpText().isEmpty())
    {
        aAny <<= m_pFFData->getHelpText();
        xPropSet->setPropertyValue("HelpF1Text", aAny);
    }

    aAny <<= rControlName;
    xPropSet->setPropertyValue("Name", aAny);

    return true;
}

bool FormControlHelper::processField(uno::Reference<text::XFormField> xFormField)
{
    bool bRes = true;
    uno::Reference<container::XNameContainer> xNameCont = xFormField->getParameters();
    uno::Reference<container::XNamed> xNamed( xFormField, uno::UNO_QUERY );
    if ( m_pFFData && xNamed.is() && xNameCont.is() )
    {

        if (m_pImpl->m_eFieldId == FIELD_FORMTEXT )
        {
            xFormField->setFieldType(ODF_FORMTEXT);
            if (  !m_pFFData->getName().isEmpty() )
            {
                xNamed->setName( m_pFFData->getName() );
            }
        }
        else if (m_pImpl->m_eFieldId == FIELD_FORMCHECKBOX )
        {
            xFormField->setFieldType(ODF_FORMCHECKBOX);
            uno::Reference<beans::XPropertySet> xPropSet(xFormField, uno::UNO_QUERY);
            uno::Any aAny;
            aAny <<= m_pFFData->getCheckboxChecked();
            if ( xPropSet.is() )
                xPropSet->setPropertyValue("Checked", aAny);
        }
        else if (m_pImpl->m_eFieldId == FIELD_FORMDROPDOWN )
        {
            xFormField->setFieldType(ODF_FORMDROPDOWN);
            uno::Sequence< OUString > sItems;
            sItems.realloc( m_pFFData->getDropDownEntries().size() );
            ::std::copy( m_pFFData->getDropDownEntries().begin(), m_pFFData->getDropDownEntries().end(), ::comphelper::stl_begin(sItems));
            if ( sItems.getLength() )
            {
                if ( xNameCont->hasByName(ODF_FORMDROPDOWN_LISTENTRY) )
                    xNameCont->replaceByName(ODF_FORMDROPDOWN_LISTENTRY, uno::makeAny( sItems ) );
                else
                    xNameCont->insertByName(ODF_FORMDROPDOWN_LISTENTRY, uno::makeAny( sItems ) );

                sal_Int32 nResult = m_pFFData->getDropDownResult().toInt32();
                if ( nResult )
                {
                    if ( xNameCont->hasByName(ODF_FORMDROPDOWN_RESULT) )
                        xNameCont->replaceByName(ODF_FORMDROPDOWN_RESULT, uno::makeAny( nResult ) );
                    else
                        xNameCont->insertByName(ODF_FORMDROPDOWN_RESULT, uno::makeAny( nResult ) );
                }
            }
        }
    }
    else
        bRes = false;
    return bRes;
}

bool FormControlHelper::insertControl(uno::Reference<text::XTextRange> xTextRange)
{
    bool bCreated = false;
    if ( !m_pFFData )
        return false;
    uno::Reference<container::XNameContainer> xFormCompsByName(m_pImpl->getForm(), uno::UNO_QUERY);
    uno::Reference<container::XIndexContainer> xFormComps(m_pImpl->getFormComps());
    if (! xFormComps.is())
        return false;

    static OUString sControl("Control");

    sal_Int32 nControl = 0;
    bool bDone = false;
    OUString sControlName;

    do
    {
        OUString sTmp(sControl);
        sTmp += OUString::number(nControl);

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

    uno::Reference<uno::XInterface> xInterface = m_pImpl->getServiceFactory()->createInstance("com.sun.star.drawing.ControlShape");

    if (! xInterface.is())
        return false;

    uno::Reference<drawing::XShape> xShape(xInterface, uno::UNO_QUERY);

    if (! xShape.is())
        return false;

    xShape->setSize(m_pImpl->aSize);

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    sal_uInt16 nTmp = text::TextContentAnchorType_AS_CHARACTER;
    aAny <<= nTmp;

    xShapeProps->setPropertyValue("AnchorType", aAny);

    nTmp = text::VertOrientation::CENTER;
    aAny <<= nTmp;
    xShapeProps->setPropertyValue("VertOrient", aAny);

    aAny <<= xTextRange;

    xShapeProps->setPropertyValue("TextRange", aAny);

    uno::Reference<drawing::XControlShape> xControlShape(xShape, uno::UNO_QUERY);
    uno::Reference<awt::XControlModel> xControlModel(m_pImpl->rFormComponent, uno::UNO_QUERY);
    xControlShape->setControl(xControlModel);

    m_pImpl->getDrawPage()->add(xShape);

    return true;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
