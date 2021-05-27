/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "XMLTextColumnsContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextColumns.hpp>

#include <sax/fastattribs.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

SvxXMLTextColumnsContext::SvxXMLTextColumnsContext(
    SvXMLImport& rImport, sal_Int32 nElement,
    const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList,
    const XMLPropertyState& rProp, ::std::vector<XMLPropertyState>& rProps)
    : XMLElementPropertyContext(rImport, nElement, rProp, rProps)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        sal_Int32 nVal;
        switch (aIter.getToken())
        {
            XML_NAMESPACE_FO;
            case XML_ELEMENT(FO, xmloff::token::XML_COLUMN_COUNT):
            case XML_ELEMENT(FO_COMPAT, xmloff::token::XML_COLUMN_COUNT):
                if (sax::Converter::convertNumber(nVal, aIter.toView(), 1, SHRT_MAX))
                    mnColumns = static_cast<sal_Int16>(nVal);
                break;
            case XML_ELEMENT(FO, xmloff::token::XML_COLUMN_GAP):
            case XML_ELEMENT(FO_COMPAT, xmloff::token::XML_COLUMN_GAP):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(mnSpacing, aIter.toView());
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
}

css::uno::Reference<css::xml::sax::XFastContextHandler>
SvxXMLTextColumnsContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    // We only support equally distributed columns in EditEngine; ignore style:column for now
    if (nElement != XML_ELEMENT(STYLE, xmloff::token::XML_COLUMN))
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return {};
}

void SvxXMLTextColumnsContext::endFastElement(sal_Int32 nElement)
{
    css::uno::Reference<css::lang::XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                                                  css::uno::UNO_QUERY);
    if (!xFactory)
        return;

    css::uno::Reference<css::text::XTextColumns> xColumns(
        xFactory->createInstance("com.sun.star.text.TextColumns"), css::uno::UNO_QUERY);
    if (!xColumns)
        return;

    xColumns->setColumnCount(mnColumns);

    if (css::uno::Reference<css::beans::XPropertySet> xPropSet{ xColumns, css::uno::UNO_QUERY })
    {
        xPropSet->setPropertyValue("AutomaticDistance", css::uno::Any(mnSpacing));
    }

    aProp.maValue <<= xColumns;

    SetInsert(true);
    XMLElementPropertyContext::endFastElement(nElement);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
