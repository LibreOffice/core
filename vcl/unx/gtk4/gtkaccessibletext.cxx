/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <o3tl/any.hxx>
#include <sal/log.hxx>

#include "a11y.hxx"
#include "gtkaccessibletext.hxx"

#if GTK_CHECK_VERSION(4, 14, 0)

namespace
{
sal_Int16 lcl_GtkTextGranularityToUNOBoundaryType(GtkAccessibleTextGranularity eGranularity)
{
    switch (eGranularity)
    {
        case GTK_ACCESSIBLE_TEXT_GRANULARITY_CHARACTER:
            return com::sun::star::accessibility::AccessibleTextType::CHARACTER;
        case GTK_ACCESSIBLE_TEXT_GRANULARITY_WORD:
            return com::sun::star::accessibility::AccessibleTextType::WORD;
        case GTK_ACCESSIBLE_TEXT_GRANULARITY_SENTENCE:
            return com::sun::star::accessibility::AccessibleTextType::SENTENCE;
        case GTK_ACCESSIBLE_TEXT_GRANULARITY_LINE:
            return com::sun::star::accessibility::AccessibleTextType::LINE;
        case GTK_ACCESSIBLE_TEXT_GRANULARITY_PARAGRAPH:
            return com::sun::star::accessibility::AccessibleTextType::PARAGRAPH;
        default:
            assert(false && "Unhandled GtkAccessibleTextGranularity.");
            return GTK_ACCESSIBLE_TEXT_GRANULARITY_CHARACTER;
    }
}

css::uno::Reference<css::accessibility::XAccessibleText> getXText(GtkAccessibleText* pGtkText)
{
    LoAccessible* pAccessible = LO_ACCESSIBLE(pGtkText);
    if (!pAccessible->uno_accessible)
        return nullptr;

    css::uno::Reference<css::accessibility::XAccessibleContext> xContext(
        pAccessible->uno_accessible->getAccessibleContext());

    css::uno::Reference<css::accessibility::XAccessibleText> xText(xContext, css::uno::UNO_QUERY);
    return xText;
}
}

static GBytes* lo_accessible_text_get_contents(GtkAccessibleText* self, unsigned int start,
                                               unsigned int end)
{
    css::uno::Reference<css::accessibility::XAccessibleText> xText = getXText(self);
    if (!xText.is())
        return nullptr;

    // G_MAXUINT has special meaning: end of the text
    const sal_Int32 nEndIndex = (end == G_MAXUINT) ? xText->getCharacterCount() : end;

    const OString sText
        = rtl::OUStringToOString(xText->getTextRange(start, nEndIndex), RTL_TEXTENCODING_UTF8);
    return g_bytes_new(sText.getStr(), sText.getLength());
}

static GBytes* lo_accessible_text_get_contents_at(GtkAccessibleText* self, unsigned int offset,
                                                  GtkAccessibleTextGranularity eGranularity,
                                                  unsigned int* start, unsigned int* end)
{
    css::uno::Reference<css::accessibility::XAccessibleText> xText = getXText(self);
    if (!xText.is())
        return nullptr;

    if (offset > o3tl::make_unsigned(xText->getCharacterCount()))
    {
        SAL_WARN("vcl.gtk",
                 "lo_accessible_text_get_contents_at called with invalid offset: " << offset);
        return nullptr;
    }

    const sal_Int16 nUnoBoundaryType = lcl_GtkTextGranularityToUNOBoundaryType(eGranularity);
    const css::accessibility::TextSegment aSegment
        = xText->getTextAtIndex(offset, nUnoBoundaryType);
    *start = o3tl::make_unsigned(aSegment.SegmentStart);
    *end = o3tl::make_unsigned(aSegment.SegmentEnd);
    const OString sText = rtl::OUStringToOString(aSegment.SegmentText, RTL_TEXTENCODING_UTF8);
    return g_bytes_new(sText.getStr(), sText.getLength());
}

static unsigned int lo_accessible_text_get_caret_position(GtkAccessibleText* self)
{
    css::uno::Reference<css::accessibility::XAccessibleText> xText = getXText(self);
    if (!xText.is())
        return 0;

    return std::max(0, xText->getCaretPosition());
}

static gboolean lo_accessible_text_get_selection(GtkAccessibleText* self, gsize* n_ranges,
                                                 GtkAccessibleTextRange** ranges)
{
    css::uno::Reference<css::accessibility::XAccessibleText> xText = getXText(self);
    if (!xText.is())
        return 0;

    if (xText->getSelectedText().isEmpty())
        return false;

    const sal_Int32 nSelectionStart = xText->getSelectionStart();
    const sal_Int32 nSelectionEnd = xText->getSelectionEnd();

    *n_ranges = 1;
    *ranges = g_new(GtkAccessibleTextRange, 1);
    (*ranges)[0].start = std::min(nSelectionStart, nSelectionEnd);
    (*ranges)[0].length = std::abs(nSelectionEnd - nSelectionStart);
    return true;
}

static int
convertUnoTextAttributesToGtk(const css::uno::Sequence<css::beans::PropertyValue>& rAttribs,
                              char*** attribute_names, char*** attribute_values)
{
    std::vector<std::pair<OString, OUString>> aNameValuePairs;
    for (const css::beans::PropertyValue& rAttribute : rAttribs)
    {
        if (rAttribute.Name == "CharFontName")
        {
            const OUString sValue = *o3tl::doAccess<OUString>(rAttribute.Value);
            aNameValuePairs.emplace_back(GTK_ACCESSIBLE_ATTRIBUTE_FAMILY, sValue);
        }
    }

    if (aNameValuePairs.empty())
        return 0;

    const int nCount = aNameValuePairs.size();
    *attribute_names = g_new(char*, nCount + 1);
    *attribute_values = g_new(char*, nCount + 1);
    for (int i = 0; i < nCount; i++)
    {
        (*attribute_names)[i] = g_strdup(aNameValuePairs[i].first.getStr());
        (*attribute_values)[i] = g_strdup(
            OUStringToOString(aNameValuePairs[i].second, RTL_TEXTENCODING_UTF8).getStr());
    }
    (*attribute_names)[nCount] = nullptr;
    (*attribute_values)[nCount] = nullptr;

    return nCount;
}

static gboolean lo_accessible_text_get_attributes(GtkAccessibleText* self, unsigned int offset,
                                                  gsize* n_ranges, GtkAccessibleTextRange** ranges,
                                                  char*** attribute_names, char*** attribute_values)
{
    css::uno::Reference<css::accessibility::XAccessibleText> xText = getXText(self);
    if (!xText.is())
        return false;

    const unsigned int nTextLength = o3tl::make_unsigned(xText->getCharacterCount());
    if (offset == nTextLength)
        offset = nTextLength - 1;

    if (offset >= nTextLength)
    {
        SAL_WARN("vcl.gtk",
                 "lo_accessible_text_get_attributes called with invalid offset: " << offset);
        return false;
    }

    css::uno::Sequence<css::beans::PropertyValue> aAttribs;
    css::uno::Reference<css::accessibility::XAccessibleTextAttributes> xAttributes(
        xText, com::sun::star::uno::UNO_QUERY);
    if (xAttributes.is())
        aAttribs = xAttributes->getRunAttributes(offset, css::uno::Sequence<OUString>());
    else
        aAttribs = xText->getCharacterAttributes(offset, css::uno::Sequence<OUString>());

    const int nCount = convertUnoTextAttributesToGtk(aAttribs, attribute_names, attribute_values);
    if (nCount == 0)
        return false;

    *n_ranges = nCount;
    *ranges = g_new(GtkAccessibleTextRange, nCount);
    // just use start and end of attribute run for each single attribute
    const css::accessibility::TextSegment aAttributeRun
        = xText->getTextAtIndex(offset, css::accessibility::AccessibleTextType::ATTRIBUTE_RUN);
    for (int i = 0; i < nCount; i++)
    {
        ((*ranges)[i]).start = aAttributeRun.SegmentStart;
        ((*ranges)[i]).length = aAttributeRun.SegmentEnd - aAttributeRun.SegmentStart;
    }

    return true;
}

static void lo_accessible_text_get_default_attributes(GtkAccessibleText* self,
                                                      char*** attribute_names,
                                                      char*** attribute_values)
{
    css::uno::Reference<css::accessibility::XAccessibleText> xText = getXText(self);
    if (!xText.is())
        return;

    css::uno::Reference<css::accessibility::XAccessibleTextAttributes> xAttributes(
        xText, com::sun::star::uno::UNO_QUERY);
    if (!xAttributes.is())
        return;

    css::uno::Sequence<css::beans::PropertyValue> aAttribs
        = xAttributes->getDefaultAttributes(css::uno::Sequence<OUString>());

    convertUnoTextAttributesToGtk(aAttribs, attribute_names, attribute_values);
}

void lo_accessible_text_init(gpointer iface_, gpointer)
{
    auto const iface = static_cast<GtkAccessibleTextInterface*>(iface_);
    iface->get_contents = lo_accessible_text_get_contents;
    iface->get_contents_at = lo_accessible_text_get_contents_at;
    iface->get_caret_position = lo_accessible_text_get_caret_position;
    iface->get_selection = lo_accessible_text_get_selection;
    iface->get_attributes = lo_accessible_text_get_attributes;
    iface->get_default_attributes = lo_accessible_text_get_default_attributes;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
