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
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
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
#include <unobookmark.hxx>
#include <unodraw.hxx>

namespace writerfilter::dmapper {

using namespace ::com::sun::star;

rtl::Reference<SwFmDrawPage> const & FormControlHelper::getDrawPage()
{
    if (! mxDrawPage.is())
    {
        if (mxTextDocument)
            mxDrawPage = mxTextDocument->getSwDrawPage();
    }

    return mxDrawPage;
}

uno::Reference<form::XForm> const & FormControlHelper::getForm()
{
    if (! mxForm.is())
    {
        rtl::Reference<SwFmDrawPage> xFormsSupplier(getDrawPage());

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

            uno::Reference<uno::XInterface> xForm(mxTextDocument->createInstance(u"com.sun.star.form.component.Form"_ustr));
            if (xForm.is())
            {
                uno::Reference<beans::XPropertySet>
                    xFormProperties(xForm, uno::UNO_QUERY);
                uno::Any aAny(sFormName);
                xFormProperties->setPropertyValue(u"Name"_ustr, aAny);
            }

            mxForm.set(xForm, uno::UNO_QUERY);

            uno::Reference<container::XIndexContainer> xForms(xFormsNamedContainer, uno::UNO_QUERY);
            uno::Any aAny(xForm);
            xForms->insertByIndex(xForms->getCount(), aAny);
        }
    }

    return mxForm;
}

uno::Reference<container::XIndexContainer> FormControlHelper::getFormComps()
{
    uno::Reference<container::XIndexContainer> xIndexContainer(getForm(), uno::UNO_QUERY);

    return xIndexContainer;
}

FormControlHelper::FormControlHelper(FieldId eFieldId,
                                     rtl::Reference<SwXTextDocument> const& xTextDocument,
                                     FFDataHandler::Pointer_t pFFData)
    : m_pFFData(std::move(pFFData))
{
    m_eFieldId = eFieldId;
    mxTextDocument = xTextDocument;
}

FormControlHelper::~FormControlHelper()
{
}

bool FormControlHelper::createCheckbox(uno::Reference<text::XTextRange> const& xTextRange,
                                       const OUString & rControlName)
{
    if ( !m_pFFData )
        return false;
    if (! mxTextDocument)
        return false;

    uno::Reference<uno::XInterface> xInterface = mxTextDocument->createInstance(u"com.sun.star.form.component.CheckBox"_ustr);

    if (!xInterface.is())
        return false;

    mxFormComponent.set(xInterface, uno::UNO_QUERY);
    if (!mxFormComponent.is())
        return false;

    uno::Reference<beans::XPropertySet> xPropSet(xInterface, uno::UNO_QUERY);

    sal_uInt32 nCheckBoxHeight = 16 * m_pFFData->getCheckboxHeight();

    if (m_pFFData->getCheckboxAutoHeight())
    {
        uno::Reference<beans::XPropertySet> xTextRangeProps(xTextRange, uno::UNO_QUERY);

        try
        {
            float fCheckBoxHeight = 0.0;
            xTextRangeProps->getPropertyValue(u"CharHeight"_ustr) >>= fCheckBoxHeight;
            nCheckBoxHeight = static_cast<sal_uInt32>(floor(fCheckBoxHeight * 35.3));
        }
        catch (beans::UnknownPropertyException &)
        {
        }
    }

    maSize.Width = nCheckBoxHeight;
    maSize.Height = maSize.Width;

    if (!m_pFFData->getStatusText().isEmpty())
    {
        xPropSet->setPropertyValue(u"HelpText"_ustr, uno::Any(m_pFFData->getStatusText()));
    }

    xPropSet->setPropertyValue(u"DefaultState"_ustr, uno::Any(m_pFFData->getCheckboxChecked()));

    if (!m_pFFData->getHelpText().isEmpty())
    {
        xPropSet->setPropertyValue(u"HelpF1Text"_ustr, uno::Any(m_pFFData->getHelpText()));
    }

    xPropSet->setPropertyValue(u"Name"_ustr, uno::Any(rControlName));

    return true;
}

void FormControlHelper::processField(rtl::Reference<SwXFieldmark> const& xFormField)
{
    // Set field type first before adding parameters.
    if (m_eFieldId == FIELD_FORMTEXT )
    {
        xFormField->setFieldType(ODF_FORMTEXT);
    }
    else if (m_eFieldId == FIELD_FORMCHECKBOX )
    {
        xFormField->setFieldType(ODF_FORMCHECKBOX);
    }
    else if (m_eFieldId == FIELD_FORMDROPDOWN )
    {
        xFormField->setFieldType(ODF_FORMDROPDOWN);
    }

    uno::Reference<container::XNameContainer> xNameCont = xFormField->getParameters();
    if ( !(m_pFFData && xNameCont.is()) )
        return;

    OUString sTmp = m_pFFData->getEntryMacro();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( u"EntryMacro"_ustr, uno::Any(sTmp) );
    sTmp = m_pFFData->getExitMacro();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( u"ExitMacro"_ustr, uno::Any(sTmp) );

    sTmp = m_pFFData->getHelpText();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( u"Help"_ustr, uno::Any(sTmp) );

    sTmp = m_pFFData->getStatusText();
    if ( !sTmp.isEmpty() )
        xNameCont->insertByName( u"Hint"_ustr, uno::Any(sTmp) );

    if (m_eFieldId == FIELD_FORMTEXT )
    {
        sTmp = m_pFFData->getName();
        try
        {
            if ( !sTmp.isEmpty() )
                xFormField->setName( sTmp );
        }
        catch ( uno::Exception& )
        {
            TOOLS_INFO_EXCEPTION("writerfilter", "Set Formfield name failed");
        }

        sTmp = m_pFFData->getTextType();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( u"Type"_ustr, uno::Any(sTmp) );

        const sal_uInt16 nMaxLength = m_pFFData->getTextMaxLength();
        if ( nMaxLength )
        {
            xNameCont->insertByName( u"MaxLength"_ustr, uno::Any(nMaxLength) );
        }

        sTmp = m_pFFData->getTextDefault();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( u"Content"_ustr, uno::Any(sTmp) );

        sTmp = m_pFFData->getTextFormat();
        if ( !sTmp.isEmpty() )
            xNameCont->insertByName( u"Format"_ustr, uno::Any(sTmp) );
    }
    else if (m_eFieldId == FIELD_FORMCHECKBOX )
    {
        uno::Any aAny;
        aAny <<= m_pFFData->getCheckboxChecked();
        xFormField->setPropertyValue(u"Checked"_ustr, aAny);
    }
    else if (m_eFieldId == FIELD_FORMDROPDOWN )
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
    uno::Reference<container::XNameContainer> xFormCompsByName(getForm(), uno::UNO_QUERY);
    uno::Reference<container::XIndexContainer> xFormComps(getFormComps());
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

    switch (m_eFieldId)
    {
    case FIELD_FORMCHECKBOX:
        bCreated = createCheckbox(xTextRange, sControlName);
        break;
    default:
        break;
    }

    if (!bCreated)
        return;

    uno::Any aAny(mxFormComponent);
    xFormComps->insertByIndex(xFormComps->getCount(), aAny);

    if (! mxTextDocument )
        return;

    uno::Reference<uno::XInterface> xInterface = mxTextDocument->createInstance(u"com.sun.star.drawing.ControlShape"_ustr);

    if (! xInterface.is())
        return;

    uno::Reference<drawing::XShape> xShape(xInterface, uno::UNO_QUERY);

    if (! xShape.is())
        return;

    xShape->setSize(maSize);

    uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);

    sal_uInt16 nTmp = sal_uInt16(text::TextContentAnchorType_AS_CHARACTER);
    xShapeProps->setPropertyValue(u"AnchorType"_ustr, uno::Any(sal_uInt16(nTmp)));

    nTmp = text::VertOrientation::CENTER;
    xShapeProps->setPropertyValue(u"VertOrient"_ustr, uno::Any(sal_uInt16(nTmp)));

    xShapeProps->setPropertyValue(u"TextRange"_ustr, uno::Any(xTextRange));

    uno::Reference<drawing::XControlShape> xControlShape(xShape, uno::UNO_QUERY);
    uno::Reference<awt::XControlModel> xControlModel(mxFormComponent, uno::UNO_QUERY);
    xControlShape->setControl(xControlModel);

    getDrawPage()->add(xShape);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
