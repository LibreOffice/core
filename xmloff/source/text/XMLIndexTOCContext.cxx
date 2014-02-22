/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "XMLIndexTOCContext.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/XRelativeTextContentInsert.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sax/tools/converter.hxx>
#include "XMLIndexTOCSourceContext.hxx"
#include "XMLIndexObjectSourceContext.hxx"
#include "XMLIndexAlphabeticalSourceContext.hxx"
#include "XMLIndexUserSourceContext.hxx"
#include "XMLIndexBibliographySourceContext.hxx"
#include "XMLIndexTableSourceContext.hxx"
#include "XMLIndexIllustrationSourceContext.hxx"
#include "XMLIndexBodyContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustring.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::lang::IllegalArgumentException;


TYPEINIT1(XMLIndexTOCContext, SvXMLImportContext);

static const sal_Char* aIndexServiceMap[] =
{
    "com.sun.star.text.ContentIndex",
    "com.sun.star.text.DocumentIndex",
    "com.sun.star.text.TableIndex",
    "com.sun.star.text.ObjectIndex",
    "com.sun.star.text.Bibliography",
    "com.sun.star.text.UserIndex",
    "com.sun.star.text.IllustrationsIndex"
};

static const XMLTokenEnum aIndexSourceElementMap[] =
{
    XML_TABLE_OF_CONTENT_SOURCE,
    XML_ALPHABETICAL_INDEX_SOURCE,
    XML_TABLE_INDEX_SOURCE,
    XML_OBJECT_INDEX_SOURCE,
    XML_BIBLIOGRAPHY_SOURCE,
    XML_USER_INDEX_SOURCE,
    XML_ILLUSTRATION_INDEX_SOURCE
};

SvXMLEnumMapEntry const aIndexTypeMap[] =
{
    { XML_TABLE_OF_CONTENT,     TEXT_INDEX_TOC },
    { XML_ALPHABETICAL_INDEX,   TEXT_INDEX_ALPHABETICAL },
    { XML_TABLE_INDEX,          TEXT_INDEX_TABLE },
    { XML_OBJECT_INDEX,         TEXT_INDEX_OBJECT },
    { XML_BIBLIOGRAPHY,         TEXT_INDEX_BIBLIOGRAPHY },
    { XML_USER_INDEX,           TEXT_INDEX_USER },
    { XML_ILLUSTRATION_INDEX,   TEXT_INDEX_ILLUSTRATION },
    { XML_TOKEN_INVALID,        0 }
};


XMLIndexTOCContext::XMLIndexTOCContext(SvXMLImport& rImport,
    sal_uInt16 nPrfx, const OUString& rLocalName)
    : SvXMLImportContext(rImport, nPrfx, rLocalName)
    , sTitle("Title")
    , sIsProtected("IsProtected")
    , sName("Name")
    , eIndexType(TEXT_INDEX_UNKNOWN)
    , bValid(sal_False)
{
    if (XML_NAMESPACE_TEXT == nPrfx)
    {
        sal_uInt16 nTmp;
        if (SvXMLUnitConverter::convertEnum(nTmp, rLocalName, aIndexTypeMap))
        {
            
            OSL_ENSURE(nTmp < (SAL_N_ELEMENTS(aIndexServiceMap)), "index out of range");
            OSL_ENSURE(SAL_N_ELEMENTS(aIndexServiceMap) ==
                       SAL_N_ELEMENTS(aIndexSourceElementMap),
                       "service and source element maps must be same size");

            eIndexType = static_cast<IndexTypeEnum>(nTmp);
            bValid = sal_True;
        }
    }
}

XMLIndexTOCContext::~XMLIndexTOCContext()
{
}

void XMLIndexTOCContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    if (bValid)
    {
        
        
        
        sal_Int16 nCount = xAttrList->getLength();
        sal_Bool bProtected = sal_False;
        OUString sIndexName;
        OUString sXmlId;
        XMLPropStyleContext* pStyle(NULL);
        for(sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
        {
            OUString sLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                                  &sLocalName );
            if ( XML_NAMESPACE_TEXT == nPrefix)
            {
                if ( IsXMLToken( sLocalName, XML_STYLE_NAME ) )
                {
                    pStyle = GetImport().GetTextImport()->FindSectionStyle(
                                xAttrList->getValueByIndex(nAttr));
                }
                else if ( IsXMLToken( sLocalName, XML_PROTECTED ) )
                {
                    bool bTmp(false);
                    if (::sax::Converter::convertBool(
                         bTmp, xAttrList->getValueByIndex(nAttr)))
                    {
                        bProtected = bTmp;
                    }
                }
                else if ( IsXMLToken( sLocalName, XML_NAME ) )
                {
                    sIndexName = xAttrList->getValueByIndex(nAttr);
                }
            }
            else if ( XML_NAMESPACE_XML == nPrefix)
            {
                if ( IsXMLToken( sLocalName, XML_ID ) )
                {
                    sXmlId = xAttrList->getValueByIndex(nAttr);
                }
            }
        }

        
        Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                                 UNO_QUERY);
        if( xFactory.is() )
        {
            Reference<XInterface> xIfc =
                xFactory->createInstance(
                    OUString::createFromAscii(aIndexServiceMap[eIndexType]));
            if( xIfc.is() )
            {
                
                Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);
                xTOCPropertySet = xPropSet;

                
                
                
                
                
                

                
#ifndef DBG_UTIL
                OUString sMarker(" ");
#else
                OUString sMarker("Y");
#endif
                UniReference<XMLTextImportHelper> rImport =
                    GetImport().GetTextImport();

                
                Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
                try
                {
                    GetImport().GetTextImport()->InsertTextContent(
                        xTextContent);
                }
                catch(const IllegalArgumentException& e)
                {
                    
                    Sequence<OUString> aSeq(1);
                    aSeq[0] = GetLocalName();
                    GetImport().SetError(
                        XMLERROR_FLAG_ERROR | XMLERROR_NO_INDEX_ALLOWED_HERE,
                        aSeq, e.Message, NULL );

                    
                    bValid = false;
                    return;
                }

                
                GetImport().SetXmlId(xIfc, sXmlId);

                
                rImport->InsertString(sMarker);
                rImport->GetCursor()->goLeft(2, sal_False);
            }
        }

        
        
        if( bValid )
            GetImport().GetTextImport()->
                RedlineAdjustStartNodeCursor(sal_True);

        if (pStyle != NULL)
        {
            pStyle->FillPropertySet( xTOCPropertySet );
        }

        Any aAny;
        aAny.setValue( &bProtected, ::getBooleanCppuType() );
        xTOCPropertySet->setPropertyValue( sIsProtected, aAny );

        if (!sIndexName.isEmpty())
        {
            aAny <<= sIndexName;
            xTOCPropertySet->setPropertyValue( sName, aAny );
        }
    }
}

void XMLIndexTOCContext::EndElement()
{
    
    
    if( bValid )
    {
        
        OUString sEmpty;
        UniReference<XMLTextImportHelper> rHelper= GetImport().GetTextImport();

        
        rHelper->GetCursor()->goRight(1, sal_False);
        if( xBodyContextRef.Is() &&
            ((XMLIndexBodyContext*)&xBodyContextRef)->HasContent() )
        {
            rHelper->GetCursor()->goLeft(1, sal_True);
            rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                             sEmpty, sal_True);
        }

        
        rHelper->GetCursor()->goRight(1, sal_True);
        rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                         sEmpty, sal_True);

        
        GetImport().GetTextImport()->RedlineAdjustStartNodeCursor(sal_False);
    }
}

SvXMLImportContext* XMLIndexTOCContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if (bValid)
    {
        if (XML_NAMESPACE_TEXT == nPrefix)
        {
            if ( IsXMLToken( rLocalName, XML_INDEX_BODY ) )
            {
                pContext = new XMLIndexBodyContext(GetImport(), nPrefix,
                                                   rLocalName);
                if ( !xBodyContextRef.Is() ||
                     !((XMLIndexBodyContext*)&xBodyContextRef)->HasContent() )
                {
                    xBodyContextRef = pContext;
                }
            }
            else if (IsXMLToken(rLocalName, aIndexSourceElementMap[eIndexType]))
            {
                
                switch (eIndexType)
                {
                    case TEXT_INDEX_TOC:
                        pContext = new XMLIndexTOCSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_OBJECT:
                        pContext = new XMLIndexObjectSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ALPHABETICAL:
                        pContext = new XMLIndexAlphabeticalSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_USER:
                        pContext = new XMLIndexUserSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_BIBLIOGRAPHY:
                        pContext = new XMLIndexBibliographySourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_TABLE:
                        pContext = new XMLIndexTableSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    case TEXT_INDEX_ILLUSTRATION:
                        pContext = new XMLIndexIllustrationSourceContext(
                            GetImport(), nPrefix, rLocalName, xTOCPropertySet);
                        break;

                    default:
                        OSL_FAIL("index type not implemented");
                        break;
                }
            }
            
        }
        
    }
    

    
    if (pContext == NULL)
    {
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName,
                                                          xAttrList);
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
