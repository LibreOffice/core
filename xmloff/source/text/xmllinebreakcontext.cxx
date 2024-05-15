/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmllinebreakcontext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextContent.hpp>

#include <xmloff/xmlement.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

namespace
{
const SvXMLEnumMapEntry<sal_Int16> pXML_LineBreakClear_Enum[] = {
    { XML_NONE, 0 }, { XML_LEFT, 1 }, { XML_RIGHT, 2 }, { XML_ALL, 3 }, { XML_TOKEN_INVALID, 0 }
};
}

SvXMLLineBreakContext::SvXMLLineBreakContext(SvXMLImport& rImport, XMLTextImportHelper& rHelper)
    : SvXMLImportContext(rImport)
    , m_rHelper(rHelper)
{
}

void SvXMLLineBreakContext::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    const uno::Reference<frame::XModel>& xModel = GetImport().GetModel();
    uno::Reference<lang::XMultiServiceFactory> xFactory(xModel, uno::UNO_QUERY);
    if (!xFactory.is())
        return;

    uno::Reference<text::XTextContent> xLineBreak(
        xFactory->createInstance(u"com.sun.star.text.LineBreak"_ustr), uno::UNO_QUERY);

    sal_Int16 eClear = 0;
    OUString aClear = xAttrList->getValue(XML_ELEMENT(LO_EXT, XML_CLEAR));
    if (SvXMLUnitConverter::convertEnum(eClear, aClear, pXML_LineBreakClear_Enum))
    {
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->setPropertyValue(u"Clear"_ustr, uno::Any(eClear));
    }

    m_rHelper.InsertTextContent(xLineBreak);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
