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

#include "XMLIndexTOCSourceContext.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <sax/tools/converter.hxx>
#include "XMLIndexTemplateContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;

XMLIndexTOCSourceContext::XMLIndexTOCSourceContext(
    SvXMLImport& rImport,
    Reference<XPropertySet> & rPropSet)
:   XMLIndexSourceBaseContext(rImport, rPropSet, UseStyles::Level)
    // use all chapters by default
,   nOutlineLevel(rImport.GetTextImport()->GetChapterNumbering()->getCount())
,   bUseOutline(true)
,   bUseMarks(true)
,   bUseParagraphStyles(false)
{
}

XMLIndexTOCSourceContext::~XMLIndexTOCSourceContext()
{
}

void XMLIndexTOCSourceContext::ProcessAttribute(const sax_fastparser::FastAttributeList::FastAttributeIter & aIter)
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL):
            if ( IsXMLToken( aIter, XML_NONE ) )
            {
                // #104651# use OUTLINE_LEVEL and USE_OUTLINE_LEVEL instead of
                // OUTLINE_LEVEL with values none|1..10. For backwards
                // compatibility, 'none' must still be read.
                bUseOutline = false;
            }
            else
            {
                sal_Int32 nTmp;
                if (::sax::Converter::convertNumber(
                    nTmp, aIter.toView(), 1, GetImport().GetTextImport()->
                    GetChapterNumbering()->getCount()))
                {
                    bUseOutline = true;
                    nOutlineLevel = nTmp;
                }
            }
            break;

        case XML_ELEMENT(TEXT, XML_USE_OUTLINE_LEVEL):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseOutline = bTmp;
            }
            break;
        }


        case XML_ELEMENT(TEXT, XML_USE_INDEX_MARKS):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseMarks = bTmp;
            }
            break;
        }

        case XML_ELEMENT(TEXT, XML_USE_INDEX_SOURCE_STYLES):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, aIter.toView()))
            {
                bUseParagraphStyles = bTmp;
            }
            break;
        }

        default:
            // default: ask superclass
            XMLIndexSourceBaseContext::ProcessAttribute(aIter);
            break;
    }
}

void XMLIndexTOCSourceContext::endFastElement(sal_Int32 nElement)
{
    rIndexPropertySet->setPropertyValue("CreateFromMarks", css::uno::Any(bUseMarks));
    rIndexPropertySet->setPropertyValue("CreateFromOutline", css::uno::Any(bUseOutline));
    rIndexPropertySet->setPropertyValue("CreateFromLevelParagraphStyles", css::uno::Any(bUseParagraphStyles));

    rIndexPropertySet->setPropertyValue("Level", css::uno::Any(static_cast<sal_Int16>(nOutlineLevel)));

    // process common attributes
    XMLIndexSourceBaseContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLIndexTOCSourceContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_TABLE_OF_CONTENT_ENTRY_TEMPLATE) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           aSvLevelNameTOCMap,
                                           XML_OUTLINE_LEVEL,
                                           aLevelStylePropNameTOCMap,
                                           aAllowedTokenTypesTOC, true );
    }
    else
    {
        return XMLIndexSourceBaseContext::createFastChildContext(nElement,
                                                             xAttrList);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
