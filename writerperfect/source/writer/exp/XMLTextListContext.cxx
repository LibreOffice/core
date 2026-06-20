/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLTextListContext.hxx"

#include "XMLTextListItemContext.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect::exp
{
int XMLTextListContext::nLevel = 0;
OUString XMLTextListContext::aStyleName = "";

XMLTextListContext::XMLTextListContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextListContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "text:list-item")
        return new XMLTextListItemContext(GetImport());
    return nullptr;
}

void XMLTextListContext::startElement(
    const OUString& /*rName*/, const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs)
{
    ++nLevel;
    librevenge::RVNGPropertyList aPropertyList;

    for (sal_Int16 i = 0; i < xAttribs->getLength(); ++i)
    {
        const OUString aAttributeName = xAttribs->getNameByIndex(i);
        const OUString aAttributeValue = xAttribs->getValueByIndex(i);

        if (aAttributeName == "text:style-name")
        {
            aStyleName = aAttributeValue;
            break;
        }
    }

    // either we found a style name in the previous for loop
    // or we are in a sublist which hasn't got a defined style-name
    // so we use the last style-name, the one from the parent
    // see XMLListPropertiesContext::startElement for an explanation about use of RVNGPropertyListVector
    if (!aStyleName.isEmpty())
    {
        auto it = GetImport().GetAutomaticListStyles().find(aStyleName);
        OString aKey = OUStringToOString(aStyleName, RTL_TEXTENCODING_UTF8);
        const librevenge::RVNGProperty* propLevel = it->second.child(aKey.getStr());
        if (propLevel)
        {
            const librevenge::RVNGPropertyListVector* pSubStylesVector
                = static_cast<const librevenge::RVNGPropertyListVector*>(propLevel);
            const librevenge::RVNGPropertyList& list = (*pSubStylesVector)[nLevel - 1];
            const librevenge::RVNGProperty* p = list["style:num-format"];
            if (p)
            {
                m_bIsOrderedList = true;
            }
        }
    }

    if (m_bIsOrderedList)
    {
        GetImport().GetGenerator().openOrderedListLevel(librevenge::RVNGPropertyList());
    }
    else
    {
        GetImport().GetGenerator().openUnorderedListLevel(librevenge::RVNGPropertyList());
    }
}

void XMLTextListContext::endElement(const OUString& /*rName*/)
{
    --nLevel;
    aStyleName = "";
    if (m_bIsOrderedList)
    {
        GetImport().GetGenerator().closeOrderedListLevel();
    }
    else
    {
        GetImport().GetGenerator().closeUnorderedListLevel();
    }
}

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
