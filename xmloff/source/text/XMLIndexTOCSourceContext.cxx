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
#include "XMLIndexTitleTemplateContext.hxx"
#include "XMLIndexTOCStylesContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <rtl/ustring.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;

const sal_Char sAPI_CreateFromOutline[] = "CreateFromOutline";
const sal_Char sAPI_CreateFromMarks[] = "CreateFromMarks";
const sal_Char sAPI_Level[] = "Level";
const sal_Char sAPI_CreateFromLevelParagraphStyles[] = "CreateFromLevelParagraphStyles";


XMLIndexTOCSourceContext::XMLIndexTOCSourceContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet)
:   XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName, rPropSet, true)
,   sCreateFromMarks(sAPI_CreateFromMarks)
,   sLevel(sAPI_Level)
,   sCreateFromOutline(sAPI_CreateFromOutline)
,   sCreateFromLevelParagraphStyles(sAPI_CreateFromLevelParagraphStyles)
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

void XMLIndexTOCSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam,
    const OUString& rValue)
{
    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_OUTLINE_LEVEL:
            if ( IsXMLToken( rValue, XML_NONE ) )
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
                    nTmp, rValue, 1, GetImport().GetTextImport()->
                    GetChapterNumbering()->getCount()))
                {
                    bUseOutline = true;
                    nOutlineLevel = nTmp;
                }
            }
            break;

        case XML_TOK_INDEXSOURCE_USE_OUTLINE_LEVEL:
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseOutline = bTmp;
            }
            break;
        }


        case XML_TOK_INDEXSOURCE_USE_INDEX_MARKS:
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseMarks = bTmp;
            }
            break;
        }

        case XML_TOK_INDEXSOURCE_USE_INDEX_SOURCE_STYLES:
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseParagraphStyles = bTmp;
            }
            break;
        }

        default:
            // default: ask superclass
            XMLIndexSourceBaseContext::ProcessAttribute(eParam, rValue);
            break;
    }
}

void XMLIndexTOCSourceContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bUseMarks, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromMarks, aAny);

    aAny.setValue(&bUseOutline, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromOutline, aAny);

    aAny.setValue(&bUseParagraphStyles, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromLevelParagraphStyles, aAny);

    rIndexPropertySet->setPropertyValue(sLevel, css::uno::Any((sal_Int16)nOutlineLevel));

    // process common attributes
    XMLIndexSourceBaseContext::EndElement();
}


SvXMLImportContext* XMLIndexTOCSourceContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken(rLocalName, XML_TABLE_OF_CONTENT_ENTRY_TEMPLATE) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           nPrefix, rLocalName,
                                           aSvLevelNameTOCMap,
                                           XML_OUTLINE_LEVEL,
                                           aLevelStylePropNameTOCMap,
                                           aAllowedTokenTypesTOC, true );
    }
    else
    {
        return XMLIndexSourceBaseContext::CreateChildContext(nPrefix,
                                                             rLocalName,
                                                             xAttrList);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
