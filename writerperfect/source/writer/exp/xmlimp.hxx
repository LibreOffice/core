/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLIMP_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLIMP_HXX

#include <map>
#include <stack>
#include <vector>

#include <librevenge/librevenge.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

class Size;

namespace writerperfect
{
namespace exp
{

class XMLImportContext;

/// ODT export feeds this class to make librevenge calls.
class XMLImport : public cppu::WeakImplHelper
    <
    css::xml::sax::XDocumentHandler
    >
{
    librevenge::RVNGTextInterface &mrGenerator;
    std::stack< rtl::Reference<XMLImportContext> > maContexts;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticTextStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maTextStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticParagraphStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maParagraphStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticCellStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maCellStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticColumnStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maColumnStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticRowStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maRowStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticTableStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maTableStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maAutomaticGraphicStyles;
    std::map<OUString, librevenge::RVNGPropertyList> maGraphicStyles;
    librevenge::RVNGPropertyListVector maCoverImages;
    /// Author, date, etc -- overwrites what would be from the document out of the box.
    librevenge::RVNGPropertyList maMetaData;
    const css::uno::Reference<css::uno::XComponentContext> &mxContext;
    css::uno::Reference<css::uri::XUriReferenceFactory> mxUriReferenceFactory;
    OUString maMediaDir;
    const std::vector<std::pair<css::uno::Sequence<sal_Int8>, Size>> &mrPageMetafiles;

public:
    XMLImport(const css::uno::Reference<css::uno::XComponentContext> &xContext, librevenge::RVNGTextInterface &rGenerator, const OUString &rURL, const css::uno::Sequence<css::beans::PropertyValue> &rDescriptor, const std::vector<std::pair<css::uno::Sequence<sal_Int8>, Size>> &rPageMetafiles);

    rtl::Reference<XMLImportContext> CreateContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs);

    librevenge::RVNGTextInterface &GetGenerator() const;
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticTextStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticParagraphStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticCellStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticColumnStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticRowStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticTableStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetAutomaticGraphicStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetTextStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetParagraphStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCellStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetColumnStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetRowStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetTableStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetGraphicStyles();
    const librevenge::RVNGPropertyListVector &GetCoverImages();
    const librevenge::RVNGPropertyList &GetMetaData();
    bool FillPopupData(const OUString &rURL, librevenge::RVNGPropertyList &rPropList);
    const std::vector<std::pair<css::uno::Sequence<sal_Int8>, Size>> &GetPageMetafiles() const;
    const css::uno::Reference<css::uno::XComponentContext> &GetComponentContext() const;

    // XDocumentHandler
    void SAL_CALL startDocument() override;
    void SAL_CALL endDocument() override;
    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;
    void SAL_CALL ignorableWhitespace(const OUString &rWhitespaces) override;
    void SAL_CALL processingInstruction(const OUString &rTarget, const OUString &rData) override;
    void SAL_CALL setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &xLocator) override;
};

} // namespace exp
} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
