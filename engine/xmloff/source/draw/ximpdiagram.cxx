/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/log.hxx>
#include <utility>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "ximpdiagram.hxx"
#include <xmloff/xmltoken.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SdXMLDiagramContext::SdXMLDiagramContext(
    SvXMLImport& rImport, const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/,
    uno::Reference<drawing::XShapes> const& rShapes)
    : SvXMLShapeContext(rImport, false)
    , rxTarget(rShapes)
{
}

uno::Reference<xml::sax::XFastContextHandler> SdXMLDiagramContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    // use small local context to import the text data
    class TextImportHelperContext final : public SvXMLImportContext
    {
        OUString& mrText;

    public:
        TextImportHelperContext(SvXMLImport& rImport, OUString& rText)
            : SvXMLImportContext(rImport)
            , mrText(rText)
        {
        }
        virtual void SAL_CALL characters(const OUString& rChars) override { mrText += rChars; }
    };

    switch (nElement)
    {
        case XML_ELEMENT(CO_EXT, XML_DIAGRAM_LAYOUT):
            return new TextImportHelperContext(GetImport(), maLayout);
        case XML_ELEMENT(CO_EXT, XML_DATA):
            return new TextImportHelperContext(GetImport(), maData);
        case XML_ELEMENT(CO_EXT, XML_DIAGRAM_COLORS):
            return new TextImportHelperContext(GetImport(), maColors);
        case XML_ELEMENT(CO_EXT, XML_DIAGRAM_QUICKSTYLE):
            return new TextImportHelperContext(GetImport(), maQuickstyle);
        default:
            XMLOFF_INFO_UNKNOWN_ELEMENT("xmloff", nElement);
            return nullptr;
    }
}

void SdXMLDiagramContext::endFastElement(sal_Int32)
{
    const bool bContentIsValid(!maLayout.isEmpty() && !maData.isEmpty() && !maColors.isEmpty()
                               && !maQuickstyle.isEmpty());
    SAL_WARN_IF(
        !bContentIsValid, "xmloff",
        "xmloff::SdXMLDiagramContext::EndElement(), content of <loext:diagram> not complete!");
    if (!bContentIsValid)
        return;

    try
    {
        // collect & deliver data to XShape
        cpo::uno::Sequence<OUString> aAllDiagramData(4);
        aAllDiagramData.getArray()[0] = maLayout;
        aAllDiagramData.getArray()[1] = maData;
        aAllDiagramData.getArray()[2] = maColors;
        aAllDiagramData.getArray()[3] = maQuickstyle;

        uno::Reference<beans::XPropertySet> xProps(rxTarget, uno::UNO_QUERY_THROW);
        xProps->setPropertyValue(u"DiagramData"_ustr, cpo::uno::Any(aAllDiagramData));
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
