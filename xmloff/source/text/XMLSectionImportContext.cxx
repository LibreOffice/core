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

#include "XMLSectionImportContext.hxx"
#include "XMLSectionSourceImportContext.hxx"
#include "XMLSectionSourceDDEImportContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/prstylei.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNamed;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


TYPEINIT1( XMLSectionImportContext, SvXMLImportContext );

const sal_Char sAPI_TextSection[] = "com.sun.star.text.TextSection";
const sal_Char sAPI_IndexHeaderSection[] = "com.sun.star.text.IndexHeaderSection";
const sal_Char sAPI_IsProtected[] = "IsProtected";
const sal_Char sAPI_Condition[] = "Condition";
const sal_Char sAPI_IsVisible[] = "IsVisible";
const sal_Char sAPI_IsCurrentlyVisible[] = "IsCurrentlyVisible";
const sal_Char sAPI_ProtectionKey[] = "ProtectionKey";

enum XMLSectionToken
{
    XML_TOK_SECTION_XMLID,
    XML_TOK_SECTION_STYLE_NAME,
    XML_TOK_SECTION_NAME,
    XML_TOK_SECTION_CONDITION,
    XML_TOK_SECTION_DISPLAY,
    XML_TOK_SECTION_PROTECT,
    XML_TOK_SECTION_PROTECTION_KEY,
    XML_TOK_SECTION_IS_HIDDEN
};

static const SvXMLTokenMapEntry aSectionTokenMap[] =
{
    { XML_NAMESPACE_XML , XML_ID, XML_TOK_SECTION_XMLID },
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_SECTION_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_NAME, XML_TOK_SECTION_NAME },
    { XML_NAMESPACE_TEXT, XML_CONDITION, XML_TOK_SECTION_CONDITION },
    { XML_NAMESPACE_TEXT, XML_DISPLAY, XML_TOK_SECTION_DISPLAY },
    { XML_NAMESPACE_TEXT, XML_PROTECTED, XML_TOK_SECTION_PROTECT },
    { XML_NAMESPACE_TEXT, XML_PROTECTION_KEY, XML_TOK_SECTION_PROTECTION_KEY},
    { XML_NAMESPACE_TEXT, XML_IS_HIDDEN, XML_TOK_SECTION_IS_HIDDEN },
    
    { XML_NAMESPACE_TEXT, XML_PROTECT, XML_TOK_SECTION_PROTECT },
    XML_TOKEN_MAP_END
};







XMLSectionImportContext::XMLSectionImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
,   sTextSection(sAPI_TextSection)
,   sIndexHeaderSection(sAPI_IndexHeaderSection)
,   sCondition(sAPI_Condition)
,   sIsVisible(sAPI_IsVisible)
,   sProtectionKey(sAPI_ProtectionKey)
,   sIsProtected(sAPI_IsProtected)
,   sIsCurrentlyVisible(sAPI_IsCurrentlyVisible)
,   bProtect(sal_False)
,   bCondOK(sal_False)
,   bIsVisible(sal_True)
,   bValid(sal_False)
,   bSequenceOK(sal_False)
,   bIsCurrentlyVisible(sal_True)
,   bIsCurrentlyVisibleOK(sal_False)
,   bHasContent(sal_False)
{
}

XMLSectionImportContext::~XMLSectionImportContext()
{
}

void XMLSectionImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    
    ProcessAttributes(xAttrList);

    
    sal_Bool bIsIndexHeader = IsXMLToken( GetLocalName(), XML_INDEX_TITLE );
    if (bIsIndexHeader)
    {
        bValid = sal_True;
    }

    UniReference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();

    
    if (bValid)
    {
        
        Reference<XMultiServiceFactory> xFactory(
            GetImport().GetModel(),UNO_QUERY);
        if (xFactory.is())
        {
            Reference<XInterface> xIfc =
                xFactory->createInstance( bIsIndexHeader ? sIndexHeaderSection
                                                        : sTextSection );
            if (xIfc.is())
            {
                Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);

                
                xSectionPropertySet = xPropSet;

                
                Reference<XNamed> xNamed(xPropSet, UNO_QUERY);
                xNamed->setName(sName);

                
                if (!sStyleName.isEmpty())
                {
                    XMLPropStyleContext* pStyle = rHelper->
                        FindSectionStyle(sStyleName);

                    if (pStyle != NULL)
                    {
                        pStyle->FillPropertySet( xPropSet );
                    }
                }

                
                if (! bIsIndexHeader)
                {
                    Any aAny;
                    aAny.setValue( &bIsVisible, ::getBooleanCppuType() );
                    xPropSet->setPropertyValue( sIsVisible, aAny );

                    
                    
                    
                    if( bIsCurrentlyVisibleOK )
                    {
                        aAny.setValue( &bIsCurrentlyVisible,
                                       ::getBooleanCppuType() );
                        xPropSet->setPropertyValue( sIsCurrentlyVisible, aAny);
                    }

                    if (bCondOK)
                    {
                        aAny <<= sCond;
                        xPropSet->setPropertyValue( sCondition, aAny );
                    }
                }

                
                if ( bSequenceOK &&
                     IsXMLToken(GetLocalName(), XML_SECTION) )
                {
                    Any aAny;
                    aAny <<= aSequence;
                    xPropSet->setPropertyValue(sProtectionKey, aAny);
                }

                
                Any aAny;
                aAny.setValue( &bProtect, ::getBooleanCppuType() );
                xPropSet->setPropertyValue( sIsProtected, aAny );

                
                
                
                Reference<XTextRange> xStart =
                    rHelper->GetCursor()->getStart();
#ifndef DBG_UTIL
                static const sal_Char sMarker[] = " ";
#else
                static const sal_Char sMarker[] = "X";
#endif
                OUString sMarkerString(sMarker);
                rHelper->InsertString(sMarkerString);
                rHelper->InsertControlCharacter(
                    ControlCharacter::APPEND_PARAGRAPH );
                rHelper->InsertString(sMarkerString);

                
                rHelper->GetCursor()->gotoRange(xStart, sal_False);
                rHelper->GetCursor()->goRight(1, sal_True);

                
                Reference<XTextContent> xTextContent(xSectionPropertySet,
                                                     UNO_QUERY);

                
                rHelper->GetText()->insertTextContent(
                    rHelper->GetCursorAsRange(), xTextContent, sal_True );

                
                rHelper->GetText()->insertString(
                    rHelper->GetCursorAsRange(), sEmpty, sal_True);

                
                
                rHelper->RedlineAdjustStartNodeCursor(sal_True); 

                
                GetImport().SetXmlId(xIfc, sXmlId);
            }
        }
    }
}

void XMLSectionImportContext::ProcessAttributes(
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLTokenMap aTokenMap(aSectionTokenMap);

    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nNamePrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );
        OUString sAttr = xAttrList->getValueByIndex(nAttr);

        switch (aTokenMap.Get(nNamePrefix, sLocalName))
        {
            case XML_TOK_SECTION_XMLID:
                sXmlId = sAttr;
                break;
            case XML_TOK_SECTION_STYLE_NAME:
                sStyleName = sAttr;
                break;
            case XML_TOK_SECTION_NAME:
                sName = sAttr;
                bValid = sal_True;
                break;
            case XML_TOK_SECTION_CONDITION:
                {
                    OUString sTmp;
                    sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                                    _GetKeyByAttrName( sAttr, &sTmp, sal_False );
                    if( XML_NAMESPACE_OOOW == nPrefix )
                    {
                        sCond = sTmp;
                        bCondOK = sal_True;
                    }
                    else
                        sCond = sAttr;
                }
                break;
            case XML_TOK_SECTION_DISPLAY:
                if (IsXMLToken(sAttr, XML_TRUE))
                {
                    bIsVisible = sal_True;
                }
                else if ( IsXMLToken(sAttr, XML_NONE) ||
                          IsXMLToken(sAttr, XML_CONDITION) )
                {
                    bIsVisible = sal_False;
                }
                
                break;
            case XML_TOK_SECTION_IS_HIDDEN:
                {
                    bool bTmp(false);
                    if (::sax::Converter::convertBool(bTmp, sAttr))
                    {
                        bIsCurrentlyVisible = !bTmp;
                        bIsCurrentlyVisibleOK = sal_True;
                    }
                }
                break;
            case XML_TOK_SECTION_PROTECTION_KEY:
                ::sax::Converter::decodeBase64(aSequence, sAttr);
                bSequenceOK = sal_True;
                break;
            case XML_TOK_SECTION_PROTECT:
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, sAttr))
                {
                    bProtect = bTmp;
                }
                break;
            }
            default:
                ; 
                break;
        }
    }
}

void XMLSectionImportContext::EndElement()
{
    
    
    UniReference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();
    rHelper->GetCursor()->goRight(1, sal_False);
    if (bHasContent)
    {
        rHelper->GetCursor()->goLeft(1, sal_True);
        rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                         sEmpty, sal_True);
    }

    
    rHelper->GetCursor()->goRight(1, sal_True);
    rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                     sEmpty, sal_True);

    
    rHelper->RedlineAdjustStartNodeCursor(sal_False);
}

SvXMLImportContext* XMLSectionImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken(rLocalName, XML_SECTION_SOURCE) )
    {
        pContext = new XMLSectionSourceImportContext(GetImport(),
                                                     nPrefix, rLocalName,
                                                     xSectionPropertySet);
    }
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken(rLocalName, XML_DDE_SOURCE) )
    {
        pContext = new XMLSectionSourceDDEImportContext(GetImport(),
                                                        nPrefix, rLocalName,
                                                        xSectionPropertySet);
    }
    else
    {
        
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList,
            XML_TEXT_TYPE_SECTION );

        
        if (NULL == pContext)
        {
            pContext = new SvXMLImportContext( GetImport(),
                                               nPrefix, rLocalName );
        }
        else
            bHasContent = sal_True;
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
