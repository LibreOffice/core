/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

namespace {

namespace css = com::sun::star;

}

ODocumentInfoPreview::ODocumentInfoPreview(Window * pParent, WinBits nBits):
    Window(pParent, WB_DIALOGCONTROL), m_pEditWin(this, nBits),
    m_pInfoTable(new SvtDocInfoTable_Impl),
    m_aLocale(SvtPathOptions().GetLocale()) // detect application language
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
    m_pEditWin.SetText(rtl::OUString());
}

void ODocumentInfoPreview::fill(
    css::uno::Reference< css::document::XDocumentProperties > const & xDocProps,
    rtl::OUString const & rURL)
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
        insertNonempty(
            DI_MIMETYPE,
            (eTypeID == CONTENT_TYPE_APP_OCTSTREAM
             ? SvFileInformationManager::GetDescription(INetURLObject(rURL))
             : INetContentTypes::GetPresentation(eTypeID, m_aLocale)));
    }

    // User-defined (custom) properties:
    css::uno::Reference< css::beans::XPropertySet > user(
        xDocProps->getUserDefinedProperties(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::beans::XPropertySetInfo > info(
        user->getPropertySetInfo());
    css::uno::Sequence< css::beans::Property > props(info->getProperties());
    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        rtl::OUString name(props[i].Name);
        css::uno::Any aAny(user->getPropertyValue(name));
        css::uno::Reference< css::script::XTypeConverter > conv(
            css::script::Converter::create(
                comphelper::getProcessComponentContext()));
        rtl::OUString value;
        try {
            value = conv->convertToSimpleType(aAny, css::uno::TypeClass_STRING).
                get< rtl::OUString >();
        } catch (css::script::CannotConvertException & e) {
            SAL_INFO("svtools", "ignored CannotConvertException " << e.Message);
        }
        if (!value.isEmpty()) {
            insertEntry(name, value);
        }
    }

    m_pEditWin.SetSelection(Selection(0, 0));
    m_pEditWin.SetAutoScroll(true);
}

void ODocumentInfoPreview::insertEntry(
    rtl::OUString const & title, rtl::OUString const & value)
{
    if (m_pEditWin.GetText().Len() != 0) {
        m_pEditWin.InsertText(rtl::OUString("\n\n"));
    }
    rtl::OUString caption(title + rtl::OUString(":\n"));
    m_pEditWin.InsertText(caption);
    m_pEditWin.SetAttrib(
        TextAttribFontWeight(WEIGHT_BOLD), m_pEditWin.GetParagraphCount() - 2,
        0, caption.getLength() - 1);
    m_pEditWin.InsertText(value);
}

void ODocumentInfoPreview::insertNonempty(long id, rtl::OUString const & value)
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
            value.Hours, value.Minutes, value.Seconds, value.HundredthSeconds));
    if (aToolsDT.IsValidAndGregorian()) {
        LocaleDataWrapper aLocaleWrapper(
            comphelper::getProcessServiceFactory(),
            Application::GetSettings().GetLocale());
        rtl::OUStringBuffer buf(aLocaleWrapper.getDate(aToolsDT));
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        buf.append(aLocaleWrapper.getTime(aToolsDT));
        insertEntry(m_pInfoTable->GetString(id), buf.makeStringAndClear());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
