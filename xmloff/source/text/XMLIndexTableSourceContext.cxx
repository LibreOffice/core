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


#include "XMLIndexTableSourceContext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>

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
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>


using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;

const sal_Char sAPI_CreateFromLabels[] = "CreateFromLabels";
const sal_Char sAPI_LabelCategory[] = "LabelCategory";
const sal_Char sAPI_LabelDisplayType[] = "LabelDisplayType";


XMLIndexTableSourceContext::XMLIndexTableSourceContext(
    SvXMLImport& rImport, sal_uInt16 nPrfx,
    const OUString& rLocalName, Reference<XPropertySet> & rPropSet)
    : XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName,
                                  rPropSet, false)
    , sCreateFromLabels(sAPI_CreateFromLabels)
    , sLabelCategory(sAPI_LabelCategory)
    , sLabelDisplayType(sAPI_LabelDisplayType)
    , nDisplayFormat(0)
    , bSequenceOK(false)
    , bDisplayFormatOK(false)
    , bUseCaption(true)
{
}

XMLIndexTableSourceContext::~XMLIndexTableSourceContext()
{
}

static SvXMLEnumMapEntry const lcl_aReferenceTypeTokenMap[] =
{

    { XML_TEXT,                 ReferenceFieldPart::TEXT },
    { XML_CATEGORY_AND_VALUE,   ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_CAPTION,              ReferenceFieldPart::ONLY_CAPTION },

    // wrong values that previous versions wrote:
    { XML_CHAPTER,              ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_PAGE,                 ReferenceFieldPart::ONLY_CAPTION },

    { XML_TOKEN_INVALID,        0 }
};

void XMLIndexTableSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam,
    const OUString& rValue)
{
    bool bTmp(false);

    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_USE_CAPTION:
            if (::sax::Converter::convertBool(bTmp, rValue))
            {
                bUseCaption = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_SEQUENCE_NAME:
            sSequence = rValue;
            bSequenceOK = true;
            break;

        case XML_TOK_INDEXSOURCE_SEQUENCE_FORMAT:
        {
             sal_uInt16 nTmp;
             if (SvXMLUnitConverter::convertEnum(nTmp, rValue,
                                                 lcl_aReferenceTypeTokenMap))
            {
                 nDisplayFormat = nTmp;
                 bDisplayFormatOK = true;
             }
            break;
        }

        default:
            XMLIndexSourceBaseContext::ProcessAttribute(eParam, rValue);
            break;
    }
}


void XMLIndexTableSourceContext::EndElement()
{
    Any aAny;

    aAny.setValue(&bUseCaption, cppu::UnoType<bool>::get());
    rIndexPropertySet->setPropertyValue(sCreateFromLabels, aAny);

    if (bSequenceOK)
    {
        rIndexPropertySet->setPropertyValue(sLabelCategory, css::uno::Any(sSequence));
    }

    if (bDisplayFormatOK)
    {
        rIndexPropertySet->setPropertyValue(sLabelDisplayType, css::uno::Any(nDisplayFormat));
    }

    XMLIndexSourceBaseContext::EndElement();
}


SvXMLImportContext* XMLIndexTableSourceContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    if ( ( XML_NAMESPACE_TEXT == nPrefix ) &&
         ( IsXMLToken( rLocalName, XML_TABLE_INDEX_ENTRY_TEMPLATE ) ) )
    {
        return new XMLIndexTemplateContext(GetImport(), rIndexPropertySet,
                                           nPrefix, rLocalName,
                                           aLevelNameTableMap,
                                           XML_TOKEN_INVALID, // no outline-level attr
                                           aLevelStylePropNameTableMap,
                                           aAllowedTokenTypesTable);
    }
    else
    {
        return XMLIndexSourceBaseContext::CreateChildContext(nPrefix,
                                                             rLocalName,
                                                             xAttrList);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
