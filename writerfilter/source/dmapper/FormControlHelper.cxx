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

#include "FormControlHelper.hxx"
#include <xmloff/odffields.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;

struct FormControlHelper::FormControlHelper_Impl : public virtual SvRefBase
{
    FieldId m_eFieldId;
    awt::Size aSize;
    uno::Reference<drawing::XDrawPage> rDrawPage;
    uno::Reference<form::XForm> rForm;
    uno::Reference<form::XFormComponent> rFormComponent;
    uno::Reference<lang::XMultiServiceFactory> rServiceFactory;
    uno::Reference<text::XTextDocument> rTextDocument;

    uno::Reference<drawing::XDrawPage> const & getDrawPage();
    uno::Reference<lang::XMultiServiceFactory> const & getServiceFactory();
    uno::Reference<form::XForm> const & getForm();
    uno::Reference<container::XIndexContainer> getFormComps();
};

uno::Reference<drawing::XDrawPage> const & FormControlHelper::FormControlHelper_Impl::getDrawPage()
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

uno::Reference<lang::XMultiServiceFactory> const & FormControlHelper::FormControlHelper_Impl::getServiceFactory()
{
    if (! rServiceFactory.is())
        rServiceFactory.set(rTextDocument, uno::UNO_QUERY);

    return rServiceFactory;
}

uno::Reference<form::XForm> const & FormControlHelper::FormControlHelper_Impl::getForm()
{
    if (! rForm.is())
    {
        uno::Reference<form::XFormsSupplier> xFormsSupplier(getDrawPage(), uno::UNO_QUERY);

        if (xFormsSupplier.is())
        {
            uno::Reference<container::XNameContainer> xFormsNamedContainer(xFormsSupplier->getForms());
            static const char sDOCXForm[] = "DOCX-Standard";

            OUString sFormName(sDOCXForm);
            sal_uInt16 nUnique = 0;

            while (xFormsNamedContainer->hasByName(sFormName))
            {
                ++nUnique;
                sFormName = sDOCXForm + OUString::number(nUnique);
            }

            uno::Reference<uno::XInterface> xForm(getServiceFactory()->createInstance("com.sun.star.form.component.Form"));
            if (xForm.is())
            {
                uno::Reference<beans::XPropertySet>
                    xFormProperties(xForm, uno::UNO_QUERY);
                uno::Any aAny(sFormName);
                xFormProperties->setPropertyValue("Name", aAny);
            }

            rForm.set(xForm, uno::UNO_QUERY);

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
                                     uno::Reference<text::XTextDocument> const& xTextDocument,
                                     FFDataHandler::Pointer_t const & pFFData)
    : m_pFFData(pFFData), m_pImpl(new FormControlHelper_Impl)
{
    m_pImpl->m_eFieldId = eFieldId;
    m_pImpl->rTextDocument = xTextDocument;
}

FormControlHelper::~FormControlHelper()
{
}

bool FormControlHelper::createCheckbox(uno::Reference<text::XTextRange> const& xTextRange,
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

    m_pImpl->rFormComponent.set(xInterface, uno::UNO_QUERY);
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

    if (!m_pFFData->getStatusText().isEmpty())
    {
        xPropSet->setPropertyValue("HelpText", uno::Any(m_pFFData->getStatusText()));
    }

    xPropSet->setPropertyValue("DefaultState", uno::Any(m_pFFData->getCheckboxChecked()));

    if (!m_pFFData->getHelpText().isEmpty())
    {
        xPropSet->setPropertyValue("HelpF1Text", uno::Any(m_pFFData->getHelpText()));
    }

    xPropSet->setPropertyValue("Name", uno::Any(rControlName));

    return true;
}

void FormControlHelper::processField(uno::Reference<text::XFormField> const& xFormField)
{
    // Set field type first before adding parameters.
    if (m_pImpl->m_eFieldId == FIELD_FORMTEXT )
    {
        xFormField->setFieldType(ODF_FORMTEXT);
    }
    else if (m_pImpl->m_eFieldId == FIELD_FORMCHECKBOX )
    {
        xFormField->setFieldType(ODF_FORMCHECKBOX);
    }
    else if (m_pImpl->m_eFieldId == FIELD_FORMDROPDOWN )
    {
        xFormField->setFieldType(ODF_FORMDROPDOWN);
    }

    uno::Reference<container::XNameContainer> xNameCont = xFormField->getParameters();
    uno::Reference<container::XNamed> xNamed( xFormField, uno::UNO_QUERY );
    if ( m_pFFData && xNamed.is() && xNameCont.is() )
    {
        OUString sTmp = m_pFFData->getEntryMacro();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "EntryMacro", uno::makeAny(sTmp) );
        sTmp = m_pFFData->getExitMacro();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "ExitMacro", uno::makeAny(sTmp) );

        sTmp = m_pFFData->getHelpText();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "Help", uno::makeAny(sTmp) );

        sTmp = m_pFFData->getStatusText();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "Hint", uno::makeAny(sTmp) );

        if (m_pImpl->m_eFieldId == FIELD_FORMTEXT )
        {
            sTmp = m_pFFData->getName();
            try
            {
                if ( !sTmp.isEmpty() )
                    xNamed->setName( sTmp );
            }
            catch ( uno::Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("writerfilter","Set Formfield name failed");
            }

            sTmp = m_pFFData->getTextType();
            if ( !sTmp.isEmpty() )
                xNameCont->insertByName( "Type", uno::makeAny(sTmp) );

            const sal_uInt16 nMaxLength = m_pFFData->getTextMaxLength();
            if ( nMaxLength )
            {
                xNameCont->insertByName( "MaxLength", uno::makeAny(nMaxLength) );
            }

            sTmp = m_pFFData->getTextDefault();
            if ( !sTmp.isEmpty() )
                xNameCont->insertByName( "Content", uno::makeAny(sTmp) );

            sTmp = m_pFFData->getTextFormat();
            if ( !sTmp.isEmpty() )
                xNameCont->insertByName( "Format", uno::makeAny(sTmp) );
        }
        else if (m_pImpl->m_eFieldId == FIELD_FORMCHECKBOX )
        {
            uno::Reference<beans::XPropertySet> xPropSet(xFormField, uno::UNO_QUERY);
            uno::Any aAny;
            aAny <<= m_pFFData->getCheckboxChecked();
            if ( xPropSet.is() )
                xPropSet->setPropertyValue("Checked", aAny);
        }
        else if (m_pImpl->m_eFieldId == FIELD_FORMDROPDOWN )
        {
            const FFDataHandler::DropDownEntries_t& rEntries = m_pFFData->getDropDownEntries();
            if (!rEntries.empty())
            {
                if ( xNameCont->hasByName(ODF_FORMDROPDOWN_LISTENTRY) )
                    xNameCont->replaceByName(ODF_FORMDROPDOWN_LISTENTRY, uno::makeAny(comphelper::containerToSequence(rEntries)));
                else
                    xNameCont->insertByName(ODF_FORMDROPDOWN_LISTENTRY, uno::makeAny(comphelper::containerToSequence(rEntries)));

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
}

void FormControlHelper::insertControl(uno::Reference<text::XTextRange> const& xTextRange)
{
    bool bCreated = false;
    if ( !m_pFFData )
        return;
    uno::Reference<container::XNameContainer> xFormCompsByName(m_pImpl->getForm(), uno::UNO_QUERY);
    uno::Reference<container::XIndexContainer> xFormComps(m_pImpl->getFormComps());
    if (! xFormComps.is())
        return;

    sal_Int32 nControl = 0;
    bool bDone = false;
    OUString sControlName;

    do
    {
        OUString sTmp = "Control" + OUString::number(nControl);

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
        return;

    uno::Any aAny(m_pImpl->rFormComponent);
    xFormComps->insertByIndex(xFormComps->getCount(), aAny);

    if (! m_pImpl->getServiceFactory().is())
        return;

    uno::Reference<uno::XInterface> xInterface = m_pImpl->getServiceFactory()->createInstance("com.sun.star.drawing.ControlShape");

    if (! xInterface.is())
        return;

    uno::Reference<drawing::XShape> xShape(xInterface, uno::UNO_QUERY);

    if (! xShape.is())
        return;

    xShape->setSize(m_pImpl->aSize);

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    sal_uInt16 nTmp = sal_uInt16(text::TextContentAnchorType_AS_CHARACTER);
    xShapeProps->setPropertyValue("AnchorType", uno::makeAny<sal_uInt16>(nTmp));

    nTmp = text::VertOrientation::CENTER;
    xShapeProps->setPropertyValue("VertOrient", uno::makeAny<sal_uInt16>(nTmp));

    xShapeProps->setPropertyValue("TextRange", uno::Any(xTextRange));

    uno::Reference<drawing::XControlShape> xControlShape(xShape, uno::UNO_QUERY);
    uno::Reference<awt::XControlModel> xControlModel(m_pImpl->rFormComponent, uno::UNO_QUERY);
    xControlShape->setControl(xControlModel);

    m_pImpl->getDrawPage()->add(xShape);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
