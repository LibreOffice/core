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

#include "sal/config.h"

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/document/XDocumentProperties.hpp"
#include "com/sun/star/script/Converter.hpp"
#include "com/sun/star/script/XTypeConverter.hpp"
#include "comphelper/processfactory.hxx"
#include "comphelper/string.hxx"
#include "rtl/ustring.hxx"
#include "svl/inettype.hxx"
#include "svtools/DocumentInfoPreview.hxx"
#include "svtools/imagemgr.hxx"
#include "vcl/txtattr.hxx"
#include "tools/datetime.hxx"
#include "tools/urlobj.hxx"
#include "unotools/pathoptions.hxx"
#include "unotools/ucbhelper.hxx"

#include "fileview.hxx"
#include "templwin.hrc"
#include "templwin.hxx"

namespace svtools {

ODocumentInfoPreview::ODocumentInfoPreview(Window * pParent, WinBits nBits):
    Window(pParent, WB_DIALOGCONTROL), m_pEditWin(this, nBits),
    m_pInfoTable(new SvtDocInfoTable_Impl),
    m_aLanguageTag(SvtPathOptions().GetLanguageTag()) // detect application language
{
    m_pEditWin.SetLeftMargin(10);
    m_pEditWin.Show();
    m_pEditWin.EnableCursor(false);
}

ODocumentInfoPreview::~ODocumentInfoPreview() {}

void ODocumentInfoPreview::Resize() {
    m_pEditWin.SetPosSizePixel(Point(0, 0), GetOutputSize());
}

void ODocumentInfoPreview::clear() {
    m_pEditWin.SetText(OUString());
}

void ODocumentInfoPreview::fill(
    css::uno::Reference< css::document::XDocumentProperties > const & xDocProps,
    OUString const & rURL)
{
    assert(xDocProps.is());

    m_pEditWin.SetAutoScroll(false);

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
    if (!rURL.isEmpty()) {
        insertNonempty(
            DI_SIZE, CreateExactSizeText(utl::UCBContentHelper::GetSize(rURL)));
        INetContentType eTypeID = INetContentTypes::GetContentTypeFromURL(rURL);
        if(eTypeID == CONTENT_TYPE_APP_OCTSTREAM)
        {
            insertNonempty( DI_MIMETYPE, SvFileInformationManager::GetDescription(INetURLObject(rURL)));
        }
        else
        {
            insertNonempty( DI_MIMETYPE, INetContentTypes::GetPresentation(eTypeID, m_aLanguageTag));
        }
    }

    // User-defined (custom) properties:
    css::uno::Reference< css::beans::XPropertySet > user(
        xDocProps->getUserDefinedProperties(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::beans::XPropertySetInfo > info(
        user->getPropertySetInfo());
    css::uno::Sequence< css::beans::Property > props(info->getProperties());
    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        OUString name(props[i].Name);
        css::uno::Any aAny(user->getPropertyValue(name));
        css::uno::Reference< css::script::XTypeConverter > conv(
            css::script::Converter::create(
                comphelper::getProcessComponentContext()));
        OUString value;
        try {
            value = conv->convertToSimpleType(aAny, css::uno::TypeClass_STRING).
                get< OUString >();
        } catch (css::script::CannotConvertException & e) {
            SAL_INFO("svtools.contnr", "ignored CannotConvertException " << e.Message);
        }
        if (!value.isEmpty()) {
            insertEntry(name, value);
        }
    }

    m_pEditWin.SetSelection(Selection(0, 0));
    m_pEditWin.SetAutoScroll(true);
}

void ODocumentInfoPreview::insertEntry(
    OUString const & title, OUString const & value)
{
    if (!m_pEditWin.GetText().isEmpty()) {
        m_pEditWin.InsertText(OUString("\n\n"));
    }
    OUString caption(title + OUString(":\n"));
    m_pEditWin.InsertText(caption);
    m_pEditWin.SetAttrib(
        TextAttribFontWeight(WEIGHT_BOLD), m_pEditWin.GetParagraphCount() - 2,
        0, caption.getLength() - 1);
    m_pEditWin.InsertText(value);
}

void ODocumentInfoPreview::insertNonempty(long id, OUString const & value)
{
    if (!value.isEmpty()) {
        insertEntry(m_pInfoTable->GetString(id), value);
    }
}

void ODocumentInfoPreview::insertDateTime(
    long id, css::util::DateTime const & value)
{
    DateTime aToolsDT(
        Date(value.Day, value.Month, value.Year),
        Time(
            value.Hours, value.Minutes, value.Seconds, value.NanoSeconds));
    if (aToolsDT.IsValidAndGregorian()) {
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
        OUStringBuffer buf(rLocaleWrapper.getDate(aToolsDT));
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        buf.append(rLocaleWrapper.getTime(aToolsDT));
        insertEntry(m_pInfoTable->GetString(id), buf.makeStringAndClear());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
