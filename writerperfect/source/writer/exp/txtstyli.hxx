/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <librevenge/librevenge.h>

#include "xmlictxt.hxx"

namespace writerperfect::exp
{
class XMLStylesContext;

/// Handler for <style:style>.
class XMLStyleContext : public XMLImportContext
{
public:
    XMLStyleContext(XMLImport& rImport, XMLStylesContext& rStyles);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;

    librevenge::RVNGPropertyList& GetTextPropertyList();
    librevenge::RVNGPropertyList& GetParagraphPropertyList();
    librevenge::RVNGPropertyList& GetCellPropertyList();
    librevenge::RVNGPropertyList& GetColumnPropertyList();
    librevenge::RVNGPropertyList& GetRowPropertyList();
    librevenge::RVNGPropertyList& GetTablePropertyList();
    librevenge::RVNGPropertyList& GetGraphicPropertyList();
    librevenge::RVNGPropertyList& GetPageLayoutPropertyList();

private:
    OUString m_aName;
    OUString m_aFamily;
    librevenge::RVNGPropertyList m_aTextPropertyList;
    librevenge::RVNGPropertyList m_aParagraphPropertyList;
    librevenge::RVNGPropertyList m_aCellPropertyList;
    librevenge::RVNGPropertyList m_aColumnPropertyList;
    librevenge::RVNGPropertyList m_aRowPropertyList;
    librevenge::RVNGPropertyList m_aTablePropertyList;
    librevenge::RVNGPropertyList m_aGraphicPropertyList;
    librevenge::RVNGPropertyList m_aPageLayoutPropertyList;
    librevenge::RVNGPropertyList m_aMasterPagePropertyList;
    XMLStylesContext& m_rStyles;
};

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
