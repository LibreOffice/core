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

#include "XMLIndexMarkExport.hxx"
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlexp.hxx>


using namespace ::xmloff::token;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::XPropertySetInfo;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;



XMLIndexMarkExport::XMLIndexMarkExport(
    SvXMLExport& rExp)
:   sLevel("Level")
,   sUserIndexName("UserIndexName")
,   sPrimaryKey("PrimaryKey")
,   sSecondaryKey("SecondaryKey")
,   sDocumentIndexMark("DocumentIndexMark")
,   sIsStart("IsStart")
,   sIsCollapsed("IsCollapsed")
,   sAlternativeText("AlternativeText")
,   sTextReading("TextReading")
,   sPrimaryKeyReading("PrimaryKeyReading")
,   sSecondaryKeyReading("SecondaryKeyReading")
,   sMainEntry("IsMainEntry")
,   rExport(rExp)
{
}

const enum XMLTokenEnum lcl_pTocMarkNames[] =
    { XML_TOC_MARK, XML_TOC_MARK_START, XML_TOC_MARK_END };
const enum XMLTokenEnum lcl_pUserIndexMarkName[] =
    { XML_USER_INDEX_MARK,
          XML_USER_INDEX_MARK_START, XML_USER_INDEX_MARK_END };
const enum XMLTokenEnum lcl_pAlphaIndexMarkName[] =
    { XML_ALPHABETICAL_INDEX_MARK,
          XML_ALPHABETICAL_INDEX_MARK_START,
          XML_ALPHABETICAL_INDEX_MARK_END };


XMLIndexMarkExport::~XMLIndexMarkExport()
{
}

void XMLIndexMarkExport::ExportIndexMark(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyles)
{
    
    if (!bAutoStyles)
    {
        const enum XMLTokenEnum * pElements = NULL;
        sal_Int8 nElementNo = -1;

        
        Any aAny;
        aAny = rPropSet->getPropertyValue(sDocumentIndexMark);
        Reference<XPropertySet> xIndexMarkPropSet;
        aAny >>= xIndexMarkPropSet;

        
        

        
        aAny = rPropSet->getPropertyValue(sIsCollapsed);
        if (*(sal_Bool *)aAny.getValue())
        {
            
            nElementNo = 0;

            aAny = xIndexMarkPropSet->getPropertyValue(sAlternativeText);
            OUString sTmp;
            aAny >>= sTmp;
            DBG_ASSERT(!sTmp.isEmpty(),
                       "collapsed index mark without alternative text");
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_STRING_VALUE, sTmp);
        }
        else
        {
            
            aAny = rPropSet->getPropertyValue(sIsStart);
            nElementNo = *(sal_Bool *)aAny.getValue() ? 1 : 2;

            
            OUStringBuffer sBuf;
            GetID(sBuf, xIndexMarkPropSet);
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_ID,
                                 sBuf.makeStringAndClear());
        }

        
        
        
        
        Reference<XPropertySetInfo> xPropertySetInfo =
            xIndexMarkPropSet->getPropertySetInfo();
        if (xPropertySetInfo->hasPropertyByName(sUserIndexName))
        {
            
            pElements = lcl_pUserIndexMarkName;
            if (nElementNo != 2)
            {
                ExportUserIndexMarkAttributes(xIndexMarkPropSet);
            }
        }
        else if (xPropertySetInfo->hasPropertyByName(sPrimaryKey))
        {
            
            pElements = lcl_pAlphaIndexMarkName;
            if (nElementNo != 2)
            {
                ExportAlphabeticalIndexMarkAttributes(xIndexMarkPropSet);
            }
        }
        else
        {
            
            pElements = lcl_pTocMarkNames;
            if (nElementNo != 2)
            {
                ExportTOCMarkAttributes(xIndexMarkPropSet);
            }
        }

        
        DBG_ASSERT(pElements != NULL, "illegal element array");
        DBG_ASSERT(nElementNo >= 0, "illegal name array index");
        DBG_ASSERT(nElementNo <= 2, "illegal name array index");

        if ((pElements != NULL) && (nElementNo != -1))
        {
            SvXMLElementExport aElem(rExport,
                                     XML_NAMESPACE_TEXT,
                                     pElements[nElementNo],
                                     sal_False, sal_False);
        }
    }
}

void XMLIndexMarkExport::ExportTOCMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    
    sal_Int16 nLevel = 0;
    Any aAny = rPropSet->getPropertyValue(sLevel);
    aAny >>= nLevel;
    OUStringBuffer sBuf;
    ::sax::Converter::convertNumber(sBuf, static_cast<sal_Int32>(nLevel + 1));
    rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL,
                             sBuf.makeStringAndClear());
}

static void lcl_ExportPropertyString( SvXMLExport& rExport,
                               const Reference<XPropertySet> & rPropSet,
                               const OUString & sProperty,
                               XMLTokenEnum eToken,
                               Any& rAny )
{
    rAny = rPropSet->getPropertyValue( sProperty );

    OUString sValue;
    if( rAny >>= sValue )
    {
        if( !sValue.isEmpty() )
        {
            rExport.AddAttribute( XML_NAMESPACE_TEXT, eToken, sValue );
        }
    }
}

static void lcl_ExportPropertyBool( SvXMLExport& rExport,
                             const Reference<XPropertySet> & rPropSet,
                             const OUString & sProperty,
                             XMLTokenEnum eToken,
                             Any& rAny )
{
    rAny = rPropSet->getPropertyValue( sProperty );

    sal_Bool bValue = sal_Bool();
    if( rAny >>= bValue )
    {
        if( bValue )
        {
            rExport.AddAttribute( XML_NAMESPACE_TEXT, eToken, XML_TRUE );
        }
    }
}

void XMLIndexMarkExport::ExportUserIndexMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    
    
    Any aAny;
    lcl_ExportPropertyString( rExport, rPropSet, sUserIndexName, XML_INDEX_NAME, aAny );

    
    ExportTOCMarkAttributes( rPropSet );
}

void XMLIndexMarkExport::ExportAlphabeticalIndexMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    
    Any aAny;
    lcl_ExportPropertyString( rExport, rPropSet, sTextReading, XML_STRING_VALUE_PHONETIC, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sPrimaryKey, XML_KEY1, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sPrimaryKeyReading, XML_KEY1_PHONETIC, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sSecondaryKey, XML_KEY2, aAny );
    lcl_ExportPropertyString( rExport, rPropSet, sSecondaryKeyReading, XML_KEY2_PHONETIC, aAny );
    lcl_ExportPropertyBool( rExport, rPropSet, sMainEntry, XML_MAIN_ENTRY, aAny );
}

void XMLIndexMarkExport::GetID(
    OUStringBuffer& sBuf,
    const Reference<XPropertySet> & rPropSet)
{
    static const sal_Char sPrefix[] = "IMark";

    
    sal_Int64 nId = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(rPropSet.get()));
    sBuf.appendAscii(sPrefix, sizeof(sPrefix)-1);
    sBuf.append(nId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
