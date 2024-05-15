/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#include "XMLIndexUserSourceContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sax/tools/converter.hxx>
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/ustring.hxx>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using namespace ::xmloff::token;

XMLIndexUserSourceContext::XMLIndexUserSourceContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet)
    :   XMLIndexSourceBaseContext(rImport, rPropSet, UseStyles::Level),
        bUseObjects(false),
        bUseGraphic(false),
        bUseMarks(false),
        bUseTables(false),
        bUseFrames(false),
        bUseLevelFromSource(false),
        bUseLevelParagraphStyles(false)
{
}

XMLIndexUserSourceContext::~XMLIndexUserSourceContext()
{
}

void XMLIndexUserSourceContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    bool bTmp(false);

    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_USE_INDEX_MARKS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseMarks = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_OBJECTS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseObjects = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_GRAPHICS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseGraphic = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_TABLES):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseTables = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_FLOATING_FRAMES):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseFrames = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_COPY_OUTLINE_LEVELS):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseLevelFromSource = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_INDEX_SOURCE_STYLES):
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseLevelParagraphStyles = bTmp;
            }
            break;

        case XML_ELEMENT(TEXT, XML_INDEX_NAME):
            sIndexName = aIter.toString();
            break;

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(aIter);
            break;
    }
}

void XMLIndexUserSourceContext::endFastElement(sal_Int32 nElement)
{
    rIndexPropertySet->setPropertyValue(u"CreateFromEmbeddedObjects"_ustr, css::uno::Any(bUseObjects));
    rIndexPropertySet->setPropertyValue(u"CreateFromGraphicObjects"_ustr, css::uno::Any(bUseGraphic));
    rIndexPropertySet->setPropertyValue(u"UseLevelFromSource"_ustr, css::uno::Any(bUseLevelFromSource));
    rIndexPropertySet->setPropertyValue(u"CreateFromMarks"_ustr, css::uno::Any(bUseMarks));
    rIndexPropertySet->setPropertyValue(u"CreateFromTables"_ustr, css::uno::Any(bUseTables));
    rIndexPropertySet->setPropertyValue(u"CreateFromTextFrames"_ustr, css::uno::Any(bUseFrames));
    rIndexPropertySet->setPropertyValue(u"CreateFromLevelParagraphStyles"_ustr, css::uno::Any(bUseLevelParagraphStyles));

    if( !sIndexName.isEmpty() )
    {
        rIndexPropertySet->setPropertyValue(u"UserIndexName"_ustr, css::uno::Any(sIndexName));
    }

    XMLIndexSourceBaseContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexUserSourceContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_USER_INDEX_ENTRY_TEMPLATE) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           aSvLevelNameTOCMap,
                                           XML_OUTLINE_LEVEL,
                                           aLevelStylePropNameTOCMap,
                                           aAllowedTokenTypesUser);
    }
    else
    {
        return XMLIndexSourceBaseContext::createFastChildContext(nElement,
                                                             xAttrList);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
