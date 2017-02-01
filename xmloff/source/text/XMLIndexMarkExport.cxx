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

#include "XMLIndexMarkExport.hxx"
#include <o3tl/any.hxx>
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
    bool bAutoStyles)
{
    /// index marks have no styles!
    if (!bAutoStyles)
    {
        const enum XMLTokenEnum * pElements = nullptr;
        sal_Int8 nElementNo = -1;

        // get index mark
        Any aAny;
        aAny = rPropSet->getPropertyValue(sDocumentIndexMark);
        Reference<XPropertySet> xIndexMarkPropSet;
        aAny >>= xIndexMarkPropSet;

        // common: handling of start, end, collapsed entries and
        // alternative text

        // collapsed/alternative text entry?
        aAny = rPropSet->getPropertyValue(sIsCollapsed);
        if (*o3tl::doAccess<bool>(aAny))
        {
            // collapsed entry: needs alternative text
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
            // start and end entries: has ID
            aAny = rPropSet->getPropertyValue(sIsStart);
            nElementNo = *o3tl::doAccess<bool>(aAny) ? 1 : 2;

            // generate ID
            OUStringBuffer sBuf;
            GetID(sBuf, xIndexMarkPropSet);
            rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_ID,
                                 sBuf.makeStringAndClear());
        }

        // distinguish between TOC, user, alphab. index marks by
        // asking for specific properties
        // Export attributes for -mark-start and -mark elements,
        // but not for -mark-end
        Reference<XPropertySetInfo> xPropertySetInfo =
            xIndexMarkPropSet->getPropertySetInfo();
        if (xPropertySetInfo->hasPropertyByName(sUserIndexName))
        {
            // user index mark
            pElements = lcl_pUserIndexMarkName;
            if (nElementNo != 2)
            {
                ExportUserIndexMarkAttributes(xIndexMarkPropSet);
            }
        }
        else if (xPropertySetInfo->hasPropertyByName(sPrimaryKey))
        {
            // alphabetical index mark
            pElements = lcl_pAlphaIndexMarkName;
            if (nElementNo != 2)
            {
                ExportAlphabeticalIndexMarkAttributes(xIndexMarkPropSet);
            }
        }
        else
        {
            // table of content:
            pElements = lcl_pTocMarkNames;
            if (nElementNo != 2)
            {
                ExportTOCMarkAttributes(xIndexMarkPropSet);
            }
        }

        // export element
        DBG_ASSERT(pElements != nullptr, "illegal element array");
        DBG_ASSERT(nElementNo >= 0, "illegal name array index");
        DBG_ASSERT(nElementNo <= 2, "illegal name array index");

        if ((pElements != nullptr) && (nElementNo != -1))
        {
            SvXMLElementExport aElem(rExport,
                                     XML_NAMESPACE_TEXT,
                                     pElements[nElementNo],
                                     false, false);
        }
    }
}

void XMLIndexMarkExport::ExportTOCMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    // outline level
    sal_Int16 nLevel = 0;
    Any aAny = rPropSet->getPropertyValue(sLevel);
    aAny >>= nLevel;
    rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_OUTLINE_LEVEL,
                             OUString::number(nLevel + 1));
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

    bool bValue;
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
    // name of user index
    // (unless it's the default index; then it has no name)
    Any aAny;
    lcl_ExportPropertyString( rExport, rPropSet, sUserIndexName, XML_INDEX_NAME, aAny );

    // additionally export outline level; just reuse ExportTOCMarkAttributes
    ExportTOCMarkAttributes( rPropSet );
}

void XMLIndexMarkExport::ExportAlphabeticalIndexMarkAttributes(
    const Reference<XPropertySet> & rPropSet)
{
    // primary and secondary keys (if available)
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
    // HACK: use address of object to form identifier
    sal_Int64 nId = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(rPropSet.get()));
    sBuf.append("IMark");
    sBuf.append(nId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
