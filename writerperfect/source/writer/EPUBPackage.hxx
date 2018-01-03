/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EPUBPACKAGE_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EPUBPACKAGE_HXX

#include <libepubgen/EPUBPackage.h>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>

namespace writerperfect
{

/// The epub package has direct access to the resulting ZIP file.
class EPUBPackage : public libepubgen::EPUBPackage
{
    css::uno::Reference<css::uno::XComponentContext> mxContext;
    css::uno::Reference<css::embed::XHierarchicalStorageAccess> mxStorage;
    css::uno::Reference<css::io::XOutputStream> mxOutputStream;
    css::uno::Reference<css::xml::sax::XWriter> mxOutputWriter;

public:
    explicit EPUBPackage(const css::uno::Reference<css::uno::XComponentContext> &xContext, const css::uno::Sequence<css::beans::PropertyValue> &rDescriptor);

    ~EPUBPackage() override;

    void openXMLFile(const char *pName) override;

    void openElement(const char *pName, const librevenge::RVNGPropertyList &rAttributes) override;
    void closeElement(const char *pName) override;

    void insertCharacters(const librevenge::RVNGString &rCharacters) override;

    void closeXMLFile() override;

    void openCSSFile(const char *pName) override;

    void insertRule(const librevenge::RVNGString &rSelector, const librevenge::RVNGPropertyList &rProperties) override;

    void closeCSSFile() override;

    void openBinaryFile(const char *pName) override;

    void insertBinaryData(const librevenge::RVNGBinaryData &rData) override;

    void closeBinaryFile() override;

    void openTextFile(const char *pName) override;

    void insertText(const librevenge::RVNGString &rCharacters) override;

    void insertLineBreak() override;

    void closeTextFile() override;
};

} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
