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

#include <sal/config.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/wghtitem.hxx>
#include <rtl/ustring.hxx>
#include "DocumentInfoPreview.hxx"
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <svl/itemset.hxx>
#include <tools/datetime.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/localedatawrapper.hxx>

#include <templwin.hrc>
#include "templwin.hxx"

namespace dbaui {

ODocumentInfoPreview::ODocumentInfoPreview()
{
}

void ODocumentInfoPreview::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    WeldEditView::SetDrawingArea(pDrawingArea);
    m_xEditView->HideCursor();
    m_xEditView->SetReadOnly(true);
}

ODocumentInfoPreview::~ODocumentInfoPreview()
{
}

void ODocumentInfoPreview::clear() {
    m_xEditEngine->SetText(OUString());
}

void ODocumentInfoPreview::fill(
    css::uno::Reference< css::document::XDocumentProperties > const & xDocProps)
{
    assert(xDocProps.is());

    insertNonempty(DI_TITLE, xDocProps->getTitle());
    insertNonempty(DI_FROM, xDocProps->getAuthor());
    insertDateTime(DI_DATE, xDocProps->getCreationDate());
    insertNonempty(DI_MODIFIEDBY, xDocProps->getModifiedBy());
    insertDateTime(DI_MODIFIEDDATE, xDocProps->getModificationDate());
    insertNonempty(DI_PRINTBY, xDocProps->getPrintedBy());
    insertDateTime(DI_PRINTDATE, xDocProps->getPrintDate());
    insertNonempty(DI_THEME, xDocProps->getSubject());
    insertNonempty(
        DI_KEYWORDS,
        comphelper::string::convertCommaSeparated(xDocProps->getKeywords()));
    insertNonempty(DI_DESCRIPTION, xDocProps->getDescription());

    // User-defined (custom) properties:
    css::uno::Reference< css::beans::XPropertySet > user(
        xDocProps->getUserDefinedProperties(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::beans::XPropertySetInfo > info(
        user->getPropertySetInfo());
    const css::uno::Sequence< css::beans::Property > props(info->getProperties());
    for (const auto& rProp : props) {
        OUString name(rProp.Name);
        css::uno::Any aAny(user->getPropertyValue(name));
        css::uno::Reference< css::script::XTypeConverter > conv(
            css::script::Converter::create(
                comphelper::getProcessComponentContext()));
        OUString value;
        try {
            value = conv->convertToSimpleType(aAny, css::uno::TypeClass_STRING).
                get< OUString >();
        } catch (css::script::CannotConvertException &) {
            TOOLS_INFO_EXCEPTION("svtools.contnr", "ignored");
        }
        if (!value.isEmpty()) {
            insertEntry(name, value);
        }
    }

    m_xEditView->SetSelection(ESelection(0, 0, 0, 0));
}

namespace
{
    ESelection InsertAtEnd(const EditEngine& rEditEngine)
    {
        const sal_uInt32 nPara = rEditEngine.GetParagraphCount() -1;
        sal_Int32 nLastLen = rEditEngine.GetText(nPara).getLength();
        return ESelection(nPara, nLastLen, nPara, nLastLen);
    }
}

void ODocumentInfoPreview::insertEntry(
    std::u16string_view title, OUString const & value)
{
    if (!m_xEditEngine->GetText().isEmpty()) {
        m_xEditEngine->QuickInsertText("\n\n", InsertAtEnd(*m_xEditEngine));
    }

    OUString caption(OUString::Concat(title) + ":\n");
    m_xEditEngine->QuickInsertText(caption, InsertAtEnd(*m_xEditEngine));

    SfxItemSet aSet(m_xEditEngine->GetEmptyItemSet());
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT_CJK));
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT_CTL));
    int nCaptionPara = m_xEditEngine->GetParagraphCount() - 2;
    m_xEditEngine->QuickSetAttribs(aSet, ESelection(nCaptionPara, 0, nCaptionPara, caption.getLength() - 1));

    m_xEditEngine->QuickInsertText(value, InsertAtEnd(*m_xEditEngine));
}

void ODocumentInfoPreview::insertNonempty(tools::Long id, OUString const & value)
{
    if (!value.isEmpty()) {
        insertEntry(SvtDocInfoTable_Impl::GetString(id), value);
    }
}

void ODocumentInfoPreview::insertDateTime(
    tools::Long id, css::util::DateTime const & value)
{
    DateTime aToolsDT(
        Date(value.Day, value.Month, value.Year),
        tools::Time(
            value.Hours, value.Minutes, value.Seconds, value.NanoSeconds));
    if (aToolsDT.IsValidAndGregorian()) {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        OUStringBuffer buf(rLocaleWrapper.getDate(aToolsDT));
        buf.append(", ");
        buf.append(rLocaleWrapper.getTime(aToolsDT));
        insertEntry(SvtDocInfoTable_Impl::GetString(id), buf.makeStringAndClear());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
