/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "XMLIndexTableSourceContext.hxx"



#include <com/sun/star/text/ReferenceFieldPart.hpp>

#include "XMLIndexTemplateContext.hxx"






#include "xmlnmspe.hxx"



#include "xmluconv.hxx"


namespace binfilter {


using namespace ::com::sun::star::text;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::xml::sax::XAttributeList;

const sal_Char sAPI_CreateFromLabels[] = "CreateFromLabels";
const sal_Char sAPI_LabelCategory[] = "LabelCategory";
const sal_Char sAPI_LabelDisplayType[] = "LabelDisplayType";


TYPEINIT1(XMLIndexTableSourceContext, XMLIndexSourceBaseContext);


XMLIndexTableSourceContext::XMLIndexTableSourceContext(
    SvXMLImport& rImport, 
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    Reference<XPropertySet> & rPropSet) :
        XMLIndexSourceBaseContext(rImport, nPrfx, rLocalName, 
                                  rPropSet, sal_False),
        sCreateFromLabels(RTL_CONSTASCII_USTRINGPARAM(sAPI_CreateFromLabels)),
        sLabelCategory(RTL_CONSTASCII_USTRINGPARAM(sAPI_LabelCategory)),
        sLabelDisplayType(RTL_CONSTASCII_USTRINGPARAM(sAPI_LabelDisplayType)),
        bSequenceOK(sal_False),
        bDisplayFormatOK(sal_False),
        bUseCaption(sal_True)
{
}

XMLIndexTableSourceContext::~XMLIndexTableSourceContext()
{
}

static SvXMLEnumMapEntry __READONLY_DATA lcl_aReferenceTypeTokenMap[] =
{
    
    { XML_TEXT,			        ReferenceFieldPart::TEXT },
    { XML_CATEGORY_AND_VALUE,	ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_CAPTION, 		        ReferenceFieldPart::ONLY_CAPTION },

    // wrong values that previous versions wrote:
    { XML_CHAPTER, 		        ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_PAGE,			        ReferenceFieldPart::ONLY_CAPTION },

    { XML_TOKEN_INVALID, 		0 }
};

void XMLIndexTableSourceContext::ProcessAttribute(
    enum IndexSourceParamEnum eParam, 
    const OUString& rValue)
{
    sal_Bool bTmp;

    switch (eParam)
    {
        case XML_TOK_INDEXSOURCE_USE_CAPTION:
            if (SvXMLUnitConverter::convertBool(bTmp, rValue))
            {
                bUseCaption = bTmp;
            }
            break;

        case XML_TOK_INDEXSOURCE_SEQUENCE_NAME:
            sSequence = rValue;
            bSequenceOK = sal_True;
            break;

        case XML_TOK_INDEXSOURCE_SEQUENCE_FORMAT:
        {
             sal_uInt16 nTmp;
             if (SvXMLUnitConverter::convertEnum(nTmp, rValue, 
                                                 lcl_aReferenceTypeTokenMap))
            {
                 nDisplayFormat = nTmp;
                 bDisplayFormatOK = sal_True;
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

    aAny.setValue(&bUseCaption, ::getBooleanCppuType());
    rIndexPropertySet->setPropertyValue(sCreateFromLabels, aAny);

    if (bSequenceOK)
    {
        aAny <<= sSequence;
        rIndexPropertySet->setPropertyValue(sLabelCategory, aAny);
    }

    if (bDisplayFormatOK)
    {
        aAny <<= nDisplayFormat;
        rIndexPropertySet->setPropertyValue(sLabelDisplayType, aAny);
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
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
