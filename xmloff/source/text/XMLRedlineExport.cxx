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

#include "XMLRedlineExport.hxx"
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <sax/tools/converter.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>


using namespace ::com::sun::star;
using namespace ::xmloff::token;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::document::XRedlinesSupplier;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextSection;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::std::list;


XMLRedlineExport::XMLRedlineExport(SvXMLExport& rExp)
:   sDelete("Delete")
,   sDeletion(GetXMLToken(XML_DELETION))
,   sFormat("Format")
,   sFormatChange(GetXMLToken(XML_FORMAT_CHANGE))
,   sInsert("Insert")
,   sInsertion(GetXMLToken(XML_INSERTION))
,   sIsCollapsed("IsCollapsed")
,   sIsStart("IsStart")
,   sRedlineAuthor("RedlineAuthor")
,   sRedlineComment("RedlineComment")
,   sRedlineDateTime("RedlineDateTime")
,   sRedlineSuccessorData("RedlineSuccessorData")
,   sRedlineText("RedlineText")
,   sRedlineType("RedlineType")
,   sStyle("Style")
,   sTextTable("TextTable")
,   sUnknownChange("UnknownChange")
,   sStartRedline("StartRedline")
,   sEndRedline("EndRedline")
,   sRedlineIdentifier("RedlineIdentifier")
,   sIsInHeaderFooter("IsInHeaderFooter")
,   sRedlineProtectionKey("RedlineProtectionKey")
,   sRecordChanges("RecordChanges")
,   sMergeLastPara("MergeLastPara")
,   sChangePrefix("ct")
,   rExport(rExp)
,   pCurrentChangesList(NULL)
{
}


XMLRedlineExport::~XMLRedlineExport()
{
    
    for( ChangesMapType::iterator aIter = aChangeMap.begin();
         aIter != aChangeMap.end();
         ++aIter )
    {
        delete aIter->second;
    }
    aChangeMap.clear();
}


void XMLRedlineExport::ExportChange(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyle)
{
    if (bAutoStyle)
    {
        
        
        
        
        
        
        
        if (pCurrentChangesList != NULL)
            ExportChangeAutoStyle(rPropSet);
    }
    else
    {
        ExportChangeInline(rPropSet);
    }
}


void XMLRedlineExport::ExportChangesList(sal_Bool bAutoStyles)
{
    if (bAutoStyles)
    {
        ExportChangesListAutoStyles();
    }
    else
    {
        ExportChangesListElements();
    }
}


void XMLRedlineExport::ExportChangesList(
    const Reference<XText> & rText,
    sal_Bool bAutoStyles)
{
    
    
    if (bAutoStyles)
        return;

    
    ChangesMapType::iterator aFind = aChangeMap.find(rText);
    if (aFind != aChangeMap.end())
    {
        ChangesListType* pChangesList = aFind->second;

        
        if (pChangesList->size() > 0)
        {
            
            SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                        XML_TRACKED_CHANGES,
                                        sal_True, sal_True);

            
            for( ChangesListType::iterator aIter = pChangesList->begin();
                 aIter != pChangesList->end();
                 ++aIter )
            {
                ExportChangedRegion( *aIter );
            }
        }
        
    }
    
}

void XMLRedlineExport::SetCurrentXText(
    const Reference<XText> & rText)
{
    if (rText.is())
    {
        
        ChangesMapType::iterator aIter = aChangeMap.find(rText);
        if (aIter == aChangeMap.end())
        {
            ChangesListType* pList = new ChangesListType;
            aChangeMap[rText] = pList;
            pCurrentChangesList = pList;
        }
        else
            pCurrentChangesList = aIter->second;
    }
    else
    {
        
        SetCurrentXText();
    }
}

void XMLRedlineExport::SetCurrentXText()
{
    pCurrentChangesList = NULL;
}


void XMLRedlineExport::ExportChangesListElements()
{
    
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

        
        Reference<XPropertySet> aDocPropertySet( rExport.GetModel(),
                                                 uno::UNO_QUERY );
        
        sal_Bool bEnabled = *(sal_Bool*)aDocPropertySet->getPropertyValue(
                                                sRecordChanges ).getValue();

        
        if ( aEnumAccess->hasElements() || bEnabled )
        {

            
            if ( !bEnabled != !aEnumAccess->hasElements() )
            {
                rExport.AddAttribute(
                    XML_NAMESPACE_TEXT, XML_TRACK_CHANGES,
                    bEnabled ? XML_TRUE : XML_FALSE );
            }

            
            SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                        XML_TRACKED_CHANGES,
                                        sal_True, sal_True);

            
            Reference<XEnumeration> aEnum = aEnumAccess->createEnumeration();
            while (aEnum->hasMoreElements())
            {
                Any aAny = aEnum->nextElement();
                Reference<XPropertySet> xPropSet;
                aAny >>= xPropSet;

                DBG_ASSERT(xPropSet.is(),
                           "can't get XPropertySet; skipping Redline");
                if (xPropSet.is())
                {
                    
                    
                    aAny = xPropSet->getPropertyValue(sIsInHeaderFooter);
                    if (! *(sal_Bool*)aAny.getValue())
                    {
                        
                        ExportChangedRegion(xPropSet);
                    }
                }
                
            }
        }
        
    }
    
}

void XMLRedlineExport::ExportChangeAutoStyle(
    const Reference<XPropertySet> & rPropSet)
{
    
    if (NULL != pCurrentChangesList)
    {
        
        Any aIsStart = rPropSet->getPropertyValue(sIsStart);
        Any aIsCollapsed = rPropSet->getPropertyValue(sIsCollapsed);

        if ( *(sal_Bool*)aIsStart.getValue() ||
             *(sal_Bool*)aIsCollapsed.getValue() )
            pCurrentChangesList->push_back(rPropSet);
    }

    
    Any aAny = rPropSet->getPropertyValue(sRedlineText);
    Reference<XText> xText;
    aAny >>= xText;
    if (xText.is())
    {
        
        rExport.GetTextParagraphExport()->collectTextAutoStyles(xText);
    }
}

void XMLRedlineExport::ExportChangesListAutoStyles()
{
    
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

        
        if (aEnumAccess->hasElements())
        {
            
            Reference<XEnumeration> aEnum = aEnumAccess->createEnumeration();
            while (aEnum->hasMoreElements())
            {
                Any aAny = aEnum->nextElement();
                Reference<XPropertySet> xPropSet;
                aAny >>= xPropSet;

                DBG_ASSERT(xPropSet.is(),
                           "can't get XPropertySet; skipping Redline");
                if (xPropSet.is())
                {

                    
                    
                    aAny = xPropSet->getPropertyValue(sIsInHeaderFooter);
                    if (! *(sal_Bool*)aAny.getValue())
                    {
                        ExportChangeAutoStyle(xPropSet);
                    }
                }
            }
        }
    }
}

void XMLRedlineExport::ExportChangeInline(
    const Reference<XPropertySet> & rPropSet)
{
    
    enum XMLTokenEnum eElement = XML_TOKEN_INVALID;
    Any aAny = rPropSet->getPropertyValue(sIsCollapsed);
    sal_Bool bCollapsed = *(sal_Bool *)aAny.getValue();
    sal_Bool bStart = sal_True; 
    if (bCollapsed)
    {
        eElement = XML_CHANGE;
    }
    else
    {
        aAny = rPropSet->getPropertyValue(sIsStart);
        bStart = *(sal_Bool *)aAny.getValue();
        eElement = bStart ? XML_CHANGE_START : XML_CHANGE_END;
    }

    if (XML_TOKEN_INVALID != eElement)
    {
        
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_CHANGE_ID,
                             GetRedlineID(rPropSet));

        
        SvXMLElementExport aChangeElem(rExport, XML_NAMESPACE_TEXT,
                                       eElement, sal_False, sal_False);
    }
}


void XMLRedlineExport::ExportChangedRegion(
    const Reference<XPropertySet> & rPropSet)
{
    
    rExport.AddAttributeIdLegacy(XML_NAMESPACE_TEXT, GetRedlineID(rPropSet));

    
    Any aAny = rPropSet->getPropertyValue(sMergeLastPara);
    if( ! *(sal_Bool*)aAny.getValue() )
        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_MERGE_LAST_PARAGRAPH,
                             XML_FALSE);

    
    SvXMLElementExport aChangedRegion(rExport, XML_NAMESPACE_TEXT,
                                      XML_CHANGED_REGION, sal_True, sal_True);


    
    {
        aAny = rPropSet->getPropertyValue(sRedlineType);
        OUString sType;
        aAny >>= sType;
        SvXMLElementExport aChange(rExport, XML_NAMESPACE_TEXT,
                                   ConvertTypeName(sType), sal_True, sal_True);

        ExportChangeInfo(rPropSet);

        
        aAny = rPropSet->getPropertyValue(sRedlineText);
        Reference<XText> xText;
        aAny >>= xText;
        if (xText.is())
        {
            rExport.GetTextParagraphExport()->exportText(xText);
            
        }
        
        
    }

    
    
    aAny = rPropSet->getPropertyValue(sRedlineSuccessorData);
    Sequence<PropertyValue> aSuccessorData;
    aAny >>= aSuccessorData;

    
    
    if (aSuccessorData.getLength() > 0)
    {
        
        
        
        
        SvXMLElementExport aSecondChangeElem(
            rExport, XML_NAMESPACE_TEXT, XML_INSERTION,
            sal_True, sal_True);

        ExportChangeInfo(aSuccessorData);
    }
    
}


const OUString XMLRedlineExport::ConvertTypeName(
    const OUString& sApiName)
{
    if (sApiName == sDelete)
    {
        return sDeletion;
    }
    else if (sApiName == sInsert)
    {
        return sInsertion;
    }
    else if (sApiName == sFormat)
    {
        return sFormatChange;
    }
    else
    {
        OSL_FAIL("unknown redline type");
        return sUnknownChange;
    }
}


/** Create a Redline-ID */
const OUString XMLRedlineExport::GetRedlineID(
    const Reference<XPropertySet> & rPropSet)
{
    Any aAny = rPropSet->getPropertyValue(sRedlineIdentifier);
    OUString sTmp;
    aAny >>= sTmp;

    OUStringBuffer sBuf(sChangePrefix);
    sBuf.append(sTmp);
    return sBuf.makeStringAndClear();
}


void XMLRedlineExport::ExportChangeInfo(
    const Reference<XPropertySet> & rPropSet)
{

    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   XML_CHANGE_INFO, sal_True, sal_True);

    Any aAny = rPropSet->getPropertyValue(sRedlineAuthor);
    OUString sTmp;
    aAny >>= sTmp;
    if (!sTmp.isEmpty())
    {
        SvXMLElementExport aCreatorElem( rExport, XML_NAMESPACE_DC,
                                          XML_CREATOR, sal_True,
                                          sal_False );
        rExport.Characters(sTmp);
    }

    aAny = rPropSet->getPropertyValue(sRedlineDateTime);
    util::DateTime aDateTime;
    aAny >>= aDateTime;
    {
        OUStringBuffer sBuf;
        ::sax::Converter::convertDateTime(sBuf, aDateTime, 0);
        SvXMLElementExport aDateElem( rExport, XML_NAMESPACE_DC,
                                          XML_DATE, sal_True,
                                          sal_False );
        rExport.Characters(sBuf.makeStringAndClear());
    }

    
    aAny = rPropSet->getPropertyValue(sRedlineComment);
    aAny >>= sTmp;
    WriteComment( sTmp );
}

void XMLRedlineExport::ExportChangeInfo(
    const Sequence<PropertyValue> & rPropertyValues)
{
    OUString sComment;

    sal_Int32 nCount = rPropertyValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        const PropertyValue& rVal = rPropertyValues[i];

        if( rVal.Name.equals(sRedlineAuthor) )
        {
            OUString sTmp;
            rVal.Value >>= sTmp;
            if (!sTmp.isEmpty())
            {
                rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CHG_AUTHOR, sTmp);
            }
        }
        else if( rVal.Name.equals(sRedlineComment) )
        {
            rVal.Value >>= sComment;
        }
        else if( rVal.Name.equals(sRedlineDateTime) )
        {
            util::DateTime aDateTime;
            rVal.Value >>= aDateTime;
            OUStringBuffer sBuf;
            ::sax::Converter::convertDateTime(sBuf, aDateTime, 0);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_CHG_DATE_TIME,
                                 sBuf.makeStringAndClear());
        }
        else if( rVal.Name.equals(sRedlineType) )
        {
            
            OUString sTmp;
            rVal.Value >>= sTmp;
            DBG_ASSERT(sTmp.equals(sInsert),
                       "hierarchical change must be insertion");
        }
        
    }

    
    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   XML_CHANGE_INFO, sal_True, sal_True);

    WriteComment( sComment );
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bStart)
{
    if( ! rPropSet.is() )
        return;

    
    Any aAny;
    try
    {
        aAny = rPropSet->getPropertyValue(bStart ? sStartRedline : sEndRedline);
    }
    catch(const UnknownPropertyException&)
    {
        
        return;
    }

    Sequence<PropertyValue> aValues;
    aAny >>= aValues;
    const PropertyValue* pValues = aValues.getConstArray();

    
    sal_Bool bIsCollapsed = sal_False;
    sal_Bool bIsStart = sal_True;
    OUString sId;
    sal_Bool bIdOK = sal_False; 
    sal_Int32 nLength = aValues.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
        if (sRedlineIdentifier.equals(pValues[i].Name))
        {
            pValues[i].Value >>= sId;
            bIdOK = sal_True;
        }
        else if (sIsCollapsed.equals(pValues[i].Name))
        {
            bIsCollapsed = *(sal_Bool*)pValues[i].Value.getValue();
        }
        else if (sIsStart.equals(pValues[i].Name))
        {
            bIsStart = *(sal_Bool*)pValues[i].Value.getValue();
        }
    }

    if( bIdOK )
    {
        DBG_ASSERT( !sId.isEmpty(), "Redlines must have IDs" );

        
        OUStringBuffer sBuffer(sChangePrefix);
        sBuffer.append(sId);

        rExport.AddAttribute(XML_NAMESPACE_TEXT, XML_CHANGE_ID,
                             sBuffer.makeStringAndClear());

        
        
        SvXMLElementExport aChangeElem(
            rExport, XML_NAMESPACE_TEXT,
            bIsCollapsed ? XML_CHANGE :
                ( bIsStart ? XML_CHANGE_START : XML_CHANGE_END ),
            sal_True, sal_True);
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextContent> & rContent,
    sal_Bool bStart)
{
    Reference<XPropertySet> xPropSet(rContent, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        ExportStartOrEndRedline(xPropSet, bStart);
    }
    else
    {
        OSL_FAIL("XPropertySet expected");
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextSection> & rSection,
    sal_Bool bStart)
{
    Reference<XPropertySet> xPropSet(rSection, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        ExportStartOrEndRedline(xPropSet, bStart);
    }
    else
    {
        OSL_FAIL("XPropertySet expected");
    }
}

void XMLRedlineExport::WriteComment(const OUString& rComment)
{
    if (!rComment.isEmpty())
    {
        
        
        SvXMLTokenEnumerator aEnumerator(rComment, sal_Char(0x0a));
        OUString aSubString;
        while (aEnumerator.getNextToken(aSubString))
        {
            SvXMLElementExport aParagraph(
                rExport, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
            rExport.Characters(aSubString);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
