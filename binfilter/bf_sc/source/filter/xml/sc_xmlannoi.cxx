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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include "xmlannoi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#ifndef SC_XMLCONTI_HXX
#include "xmlconti.hxx"
#endif

#include <bf_xmloff/nmspmap.hxx>
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLAnnotationContext::ScXMLAnnotationContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                      ScXMLTableRowCellContext* pTempCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nParagraphCount(0),
    bDisplay(sal_False),
    bHasTextP(sal_False)
{
    pCellContext = pTempCellContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAnnotationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR:
            {
                sAuthor = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE:
            {
                sCreateDate = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING:
            {
                if (!sCreateDate.getLength())
                    sCreateDate = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY:
            {
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            }
            break;
        }
    }
}

ScXMLAnnotationContext::~ScXMLAnnotationContext()
{
}

SvXMLImportContext *ScXMLAnnotationContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if ((nPrefix == XML_NAMESPACE_TEXT) && IsXMLToken(rLName, XML_P) )
    {
        if (!bHasTextP)
        {
            bHasTextP = sal_True;
            sOUText.setLength(0);
        }
        if(nParagraphCount)
            sOUText.append(static_cast<sal_Unicode>('\n'));
        nParagraphCount++;
        pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLName, xAttrList, sOUText);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLAnnotationContext::Characters( const ::rtl::OUString& rChars )
{
    if (!bHasTextP)
        sOUText.append(rChars);
}

void ScXMLAnnotationContext::EndElement()
{
    ScMyImportAnnotation* pMyAnnotation = new ScMyImportAnnotation();
    pMyAnnotation->sAuthor = sAuthor;
    pMyAnnotation->sCreateDate = sCreateDate;
    pMyAnnotation->sText = sOUText.makeStringAndClear();
    pMyAnnotation->bDisplay = bDisplay;
    pCellContext->AddAnnotation(pMyAnnotation);
}

}
