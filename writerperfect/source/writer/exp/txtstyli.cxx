/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "txtstyli.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <style:paragraph-properties>.
class XMLParagraphPropertiesContext : public XMLImportContext
{
public:
    XMLParagraphPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle);

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

private:
    XMLStyleContext &mrStyle;
};

XMLParagraphPropertiesContext::XMLParagraphPropertiesContext(XMLImport &rImport, XMLStyleContext &rStyle)
    : XMLImportContext(rImport)
    , mrStyle(rStyle)
{
}

void XMLParagraphPropertiesContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        OString sName = OUStringToOString(xAttribs->getNameByIndex(i), RTL_TEXTENCODING_UTF8);
        OString sValue = OUStringToOString(xAttribs->getValueByIndex(i), RTL_TEXTENCODING_UTF8);
        mrStyle.GetPropertyList().insert(sName.getStr(), sValue.getStr());
    }
}

XMLStyleContext::XMLStyleContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLStyleContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "style:paragraph-properties")
        return new XMLParagraphPropertiesContext(mrImport, *this);
    return nullptr;
}

void XMLStyleContext::startElement(const OUString &/*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString &rAttributeName = xAttribs->getNameByIndex(i);
        if (rAttributeName == "style:name")
        {
            m_aName = xAttribs->getValueByIndex(i);
            break;
        }
    }
}

void XMLStyleContext::endElement(const OUString &/*rName*/)
{
    if (m_aName.isEmpty())
        return;

    mrImport.GetAutomaticStyles()[m_aName] = m_aPropertyList;
}

librevenge::RVNGPropertyList &XMLStyleContext::GetPropertyList()
{
    return m_aPropertyList;
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
