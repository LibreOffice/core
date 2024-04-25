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

#include <o3tl/safeint.hxx>

#include "FormControlHelper.hxx"
#include <utility>
#include <xmloff/odffields.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <rtl/ref.hxx>
#include <unotxdoc.hxx>

namespace writerfilter::dmapper {

using namespace ::com::sun::star;

struct FormControlHelper::FormControlHelper_Impl : public virtual SvRefBase
{
    FieldId m_eFieldId;
    awt::Size aSize;
    uno::Reference<drawing::XDrawPage> rDrawPage;
    uno::Reference<form::XForm> rForm;
    uno::Reference<form::XFormComponent> rFormComponent;
    rtl::Reference<SwXTextDocument> mxTextDocument;

    uno::Reference<drawing::XDrawPage> const & getDrawPage();
    uno::Reference<form::XForm> const & getForm();
    uno::Reference<container::XIndexContainer> getFormComps();
};

uno::Reference<drawing::XDrawPage> const & FormControlHelper::FormControlHelper_Impl::getDrawPage()
{
    if (! rDrawPage.is())
    {
        if (mxTextDocument)
            rDrawPage = mxTextDocument->getDrawPage();
    }

    return rDrawPage;
}

uno::Reference<form::XForm> const & FormControlHelper::FormControlHelper_Impl::getForm()
{
    if (! rForm.is())
    {
        uno::Reference<form::XFormsSupplier> xFormsSupplier(getDrawPage(), uno::UNO_QUERY);

        if (xFormsSupplier.is())
        {
            uno::Reference<container::XNameContainer> xFormsNamedContainer(xFormsSupplier->getForms());
            static constexpr OUString sDOCXForm = u"DOCX-Standard"_ustr;

            OUString sFormName(sDOCXForm);
            sal_uInt16 nUnique = 0;

            while (xFormsNamedContainer->hasByName(sFormName))
            {
                ++nUnique;
                sFormName = sDOCXForm + OUString::number(nUnique);
            }

            uno::Reference<uno::XInterface> xForm(mxTextDocument->createInstance("com.sun.star.form.component.Form"));
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
                                     rtl::Reference<SwXTextDocument> const& xTextDocument,
                                     FFDataHandler::Pointer_t pFFData)
    : m_pFFData(std::move(pFFData)), m_pImpl(new FormControlHelper_Impl)
{
    m_pImpl->m_eFieldId = eFieldId;
    m_pImpl->mxTextDocument = xTextDocument;
}

FormControlHelper::~FormControlHelper()
{
}

bool FormControlHelper::createCheckbox(uno::Reference<text::XTextRange> const& xTextRange,
                                       const OUString & rControlName)
{
    if ( !m_pFFData )
        return false;
    if (! m_pImpl->mxTextDocument)
        return false;

    uno::Reference<uno::XInterface> xInterface = m_pImpl->mxTextDocument->createInstance("com.sun.star.form.component.CheckBox");

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
    if ( !(m_pFFData && xNamed.is() && xNameCont.is()) )
        return;

    OUString sTmp = m_pFFData->getEntryMacro();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( "EntryMacro", uno::Any(sTmp) );
    sTmp = m_pFFData->getExitMacro();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( "ExitMacro", uno::Any(sTmp) );

    sTmp = m_pFFData->getHelpText();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( "Help", uno::Any(sTmp) );

    sTmp = m_pFFData->getStatusText();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( "Hint", uno::Any(sTmp) );

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
            TOOLS_INFO_EXCEPTION("writerfilter", "Set Formfield name failed");
        }

        sTmp = m_pFFData->getTextType();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "Type", uno::Any(sTmp) );

        const sal_uInt16 nMaxLength = m_pFFData->getTextMaxLength();
        if ( nMaxLength )
        {
            xNameCont->insertByName( "MaxLength", uno::Any(nMaxLength) );
        }

        sTmp = m_pFFData->getTextDefault();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "Content", uno::Any(sTmp) );

        sTmp = m_pFFData->getTextFormat();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( "Format", uno::Any(sTmp) );
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
                xNameCont->replaceByName(ODF_FORMDROPDOWN_LISTENTRY, uno::Any(comphelper::containerToSequence(rEntries)));
            else
                xNameCont->insertByName(ODF_FORMDROPDOWN_LISTENTRY, uno::Any(comphelper::containerToSequence(rEntries)));

            sal_Int32 nResult = m_pFFData->getDropDownResult().toInt32();
            // 0 is valid, but also how toInt32 reports parse error, but it's a sensible default...
            if (0 <= nResult && o3tl::make_unsigned(nResult) < rEntries.size())
            {
                if ( xNameCont->hasByName(ODF_FORMDROPDOWN_RESULT) )
                    xNameCont->replaceByName(ODF_FORMDROPDOWN_RESULT, uno::Any( nResult ) );
                else
                    xNameCont->insertByName(ODF_FORMDROPDOWN_RESULT, uno::Any( nResult ) );
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

    if (! m_pImpl->mxTextDocument )
        return;

    uno::Reference<uno::XInterface> xInterface = m_pImpl->mxTextDocument->createInstance("com.sun.star.drawing.ControlShape");

    if (! xInterface.is())
        return;

    uno::Reference<drawing::XShape> xShape(xInterface, uno::UNO_QUERY);

    if (! xShape.is())
        return;

    xShape->setSize(m_pImpl->aSize);

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    sal_uInt16 nTmp = sal_uInt16(text::TextContentAnchorType_AS_CHARACTER);
    xShapeProps->setPropertyValue("AnchorType", uno::Any(sal_uInt16(nTmp)));

    nTmp = text::VertOrientation::CENTER;
    xShapeProps->setPropertyValue("VertOrient", uno::Any(sal_uInt16(nTmp)));

    xShapeProps->setPropertyValue("TextRange", uno::Any(xTextRange));

    uno::Reference<drawing::XControlShape> xControlShape(xShape, uno::UNO_QUERY);
    uno::Reference<awt::XControlModel> xControlModel(m_pImpl->rFormComponent, uno::UNO_QUERY);
    xControlShape->setControl(xControlModel);

    m_pImpl->getDrawPage()->add(xShape);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
