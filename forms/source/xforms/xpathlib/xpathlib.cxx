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


#include <string.h>
#include <sal/types.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include "xpathlib.hxx"

#include "extension.hxx"



using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xforms;
using namespace com::sun::star::lang;

xmlXPathFunction xforms_lookupFunc(void *, const xmlChar *xname, const xmlChar *)
{

    const char *name = (char *)xname;
    if (strcmp("boolean-from-string", name)==0)
        return xforms_booleanFromStringFunction;
    else if ((strcmp("if", name))==0)
        return xforms_ifFunction;
    else if ((strcmp("avg", name))==0)
        return xforms_avgFunction;
    else if ((strcmp("min", name))==0)
        return xforms_minFunction;
    else if ((strcmp("max", name))==0)
        return xforms_maxFunction;
    else if ((strcmp("count-non-empty", name))==0)
        return xforms_countNonEmptyFunction;
    else if ((strcmp("index", name))==0)
        return xforms_indexFunction;
    else if ((strcmp("property", name))==0)
        return xforms_propertyFunction;
    else if ((strcmp("now", name))==0)
        return xforms_nowFunction;
    else if ((strcmp("days-from-date", name))==0)
        return xforms_daysFromDateFunction;
    else if ((strcmp("seconds-from-dateTime", name))==0)
        return xforms_secondsFromDateTimeFunction;
    else if ((strcmp("seconds", name))==0)
        return xforms_secondsFuction;
    else if ((strcmp("months", name))==0)
        return xforms_monthsFuction;
    else if ((strcmp("instance", name))==0)
        return xforms_instanceFuction;
    else if ((strcmp("current", name))==0)
        return xforms_currentFunction;
    else
        return NULL;
}


void xforms_booleanFromStringFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar *pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString((char*)pString, strlen((char*)pString), RTL_TEXTENCODING_UTF8);
    if (aString.equalsIgnoreAsciiCase("true") ||
        aString.equalsIgnoreAsciiCase("1"))
        xmlXPathReturnTrue(ctxt);
    else if (aString.equalsIgnoreAsciiCase("false") ||
             aString.equalsIgnoreAsciiCase("0"))
        xmlXPathReturnFalse(ctxt);
    else
        XP_ERROR(XPATH_NUMBER_ERROR);
}

void xforms_ifFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 3) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar *s2 = xmlXPathPopString(ctxt);

    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    xmlChar *s1 = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    bool aBool = xmlXPathPopBoolean(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);

    if (aBool)
        xmlXPathReturnString(ctxt, s1);
    else
        xmlXPathReturnString(ctxt, s2);

}


void xforms_avgFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);

    
    xmlXPathObjectPtr pObject = valuePop(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    
    valuePush(ctxt, xmlXPathObjectCopy(pObject));
    
    xmlXPathSumFunction(ctxt, 1);
    double nSum = xmlXPathPopNumber(ctxt);
    
    valuePush(ctxt, xmlXPathObjectCopy(pObject));
    xmlXPathCountFunction(ctxt, 1);
    double nCount = xmlXPathPopNumber(ctxt);
    
    xmlXPathReturnNumber(ctxt, nSum);
    xmlXPathReturnNumber(ctxt, nCount);
    xmlXPathDivValues(ctxt);
    
    xmlXPathFreeObject(pObject);
}

void xforms_minFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlNodeSetPtr pNodeSet = xmlXPathPopNodeSet(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    double nMinimum = 0;
    double nNumber = 0;
    for (int i = 0; i <  xmlXPathNodeSetGetLength(pNodeSet); i++)
    {
        nNumber = xmlXPathCastNodeToNumber(xmlXPathNodeSetItem(pNodeSet, i));
        if (xmlXPathIsNaN(nNumber))
        {
            xmlXPathReturnNumber(ctxt, xmlXPathNAN);
            return;
        }
        if (i == 0)
            nMinimum = nNumber;
        else if (nNumber < nMinimum)
            nMinimum = nNumber;
    }
    xmlXPathReturnNumber(ctxt, nMinimum);
}

void xforms_maxFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlNodeSetPtr pNodeSet = xmlXPathPopNodeSet(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    double nMaximum = 0;
    double nNumber = 0;
    for (int i = 0; i <  xmlXPathNodeSetGetLength(pNodeSet); i++)
    {
        nNumber = xmlXPathCastNodeToNumber(xmlXPathNodeSetItem(pNodeSet, i));
        if (xmlXPathIsNaN(nNumber))
        {
            xmlXPathReturnNumber(ctxt, xmlXPathNAN);
            return;
        }
        if (i == 0)
            nMaximum = nNumber;
        else if (nNumber > nMaximum)
            nMaximum = nNumber;
    }
    xmlXPathReturnNumber(ctxt, nMaximum);
}
void xforms_countNonEmptyFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlNodeSetPtr pNodeSet = xmlXPathPopNodeSet(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    xmlChar *aString;
    sal_Int32 nNotEmpty = 0;
    for (int i = 0; i <  xmlXPathNodeSetGetLength(pNodeSet); i++)
    {
        aString = xmlXPathCastNodeToString(xmlXPathNodeSetItem(pNodeSet, i));
        if (*aString != 0) nNotEmpty++;
    }
    xmlXPathReturnNumber(ctxt, nNotEmpty);
}
void xforms_indexFunction(xmlXPathParserContextPtr /*ctxt*/, int /*nargs*/)
{
    
    
    

    
    
}


static const char* _version = "1.0";
static const char* _conformance = "conformance";
void xforms_propertyFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString((char*)pString, strlen((char*)pString), RTL_TEXTENCODING_UTF8);
    if (aString.equalsIgnoreAsciiCase("version"))
        xmlXPathReturnString(ctxt, (xmlChar*)_version);
    else if (aString.equalsIgnoreAsciiCase("conformance-level"))
        xmlXPathReturnString(ctxt, (xmlChar*)_conformance);
    else
        xmlXPathReturnEmptyString(ctxt);
}



static OString makeDateTimeString (const DateTime& aDateTime, sal_Bool bUTC = sal_True)
{
    OStringBuffer aDateTimeString;
    aDateTimeString.append((sal_Int32)aDateTime.GetYear());
    aDateTimeString.append("-");
    if (aDateTime.GetMonth()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetMonth());
    aDateTimeString.append("-");
    if (aDateTime.GetDay()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetDay());
    aDateTimeString.append("T");
    if (aDateTime.GetHour()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetHour());
    aDateTimeString.append(":");
    if (aDateTime.GetMin()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetMin());
    aDateTimeString.append(":");
    if (aDateTime.GetSec()<10) aDateTimeString.append("0");
    aDateTimeString.append((sal_Int32)aDateTime.GetSec());
    if (bUTC) aDateTimeString.append("Z");

    return aDateTimeString.makeStringAndClear();
}



void xforms_nowFunction(xmlXPathParserContextPtr ctxt, int /*nargs*/)
{
    /*
    A single lexical representation, which is a subset of the lexical representations
    allowed by [ISO 8601], is allowed for dateTime. This lexical representation is the
    [ISO 8601] extended format CCYY-MM-DDThh:mm:ss where "CC" represents the century,
    "YY" the year, "MM" the month and "DD" the day, preceded by an optional leading "-"
    sign to indicate a negative number. If the sign is omitted, "+" is assumed. The letter
    "T" is the date/time separator and "hh", "mm", "ss" represent hour, minute and second
    respectively.
    */

    /*
    3.2.7.2 Canonical representation
    The canonical representation for dateTime is defined by prohibiting certain options
    from the Lexical representation (par.3.2.7.1). Specifically, either the time zone must
    be omitted or, if present, the time zone must be Coordinated Universal Time (UTC)
    indicated by a "Z".
    */
    DateTime aDateTime( DateTime::SYSTEM );
    OString aDateTimeString = makeDateTimeString(aDateTime);
    xmlChar *pString = static_cast<xmlChar*>(xmlMalloc(aDateTimeString.getLength()+1));
    strncpy((char*)pString, (char*)aDateTimeString.getStr(), aDateTimeString.getLength());
    pString[aDateTimeString.getLength()] = 0;
    xmlXPathReturnString(ctxt, pString);
}

static sal_Bool parseDateTime(const OUString& aString, DateTime& aDateTime)
{
    
    

    OUString aDateTimeString = aString.trim();

    
    if (aDateTimeString.getLength() < 19 || aDateTimeString.getLength() > 20)
        return sal_False;

    sal_Int32 nDateLength = 10;
    sal_Int32 nTimeLength = 8;

    OUString aUTCString("Z");

    OUString aDateString = aDateTimeString.copy(0, nDateLength);
    OUString aTimeString = aDateTimeString.copy(nDateLength+1, nTimeLength);

    sal_Int32 nIndex = 0;
    sal_Int32 nYear = aDateString.getToken(0, '-', nIndex).toInt32();
    sal_Int32 nMonth = aDateString.getToken(0, '-', nIndex).toInt32();
    sal_Int32 nDay = aDateString.getToken(0, '-', nIndex).toInt32();
    nIndex = 0;
    sal_Int32 nHour = aTimeString.getToken(0, ':', nIndex).toInt32();
    sal_Int32 nMinute = aTimeString.getToken(0, ':', nIndex).toInt32();
    sal_Int32 nSecond = aTimeString.getToken(0, ':', nIndex).toInt32();

    Date tmpDate((sal_uInt16)nDay, (sal_uInt16)nMonth, (sal_uInt16)nYear);
    Time tmpTime(nHour, nMinute, nSecond);
    DateTime tmpDateTime(tmpDate, tmpTime);
    if (aString.indexOf(aUTCString) < 0)
        tmpDateTime.ConvertToUTC();

    aDateTime = tmpDateTime;

    return sal_True;
}


void xforms_daysFromDateFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    

    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString((char*)pString, strlen((char*)pString), RTL_TEXTENCODING_UTF8);

    DateTime aDateTime( DateTime::EMPTY );
    if (parseDateTime(aString, aDateTime))
    {
        Date aReferenceDate(1, 1, 1970);
        sal_Int32 nDays = aDateTime - aReferenceDate;
        xmlXPathReturnNumber(ctxt, nDays);
    }
    else
        xmlXPathReturnNumber(ctxt, xmlXPathNAN);


}


void xforms_secondsFromDateTimeFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    

    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString((char*)pString, strlen((char*)pString), RTL_TEXTENCODING_UTF8);

    DateTime aDateTime( DateTime::EMPTY );

    if (parseDateTime(aString, aDateTime))
    {
        Date aReferenceDate(1, 1, 1970);
        sal_Int32 nDays = aDateTime - aReferenceDate;
        sal_Int32 nSeconds = nDays * 24 * 60 * 60;
        nSeconds += aDateTime.GetHour() * 60 * 60;
        nSeconds += aDateTime.GetMin() * 60;
        nSeconds += aDateTime.GetSec();
        xmlXPathReturnNumber(ctxt, nSeconds);
    }
    else
        xmlXPathReturnNumber(ctxt, xmlXPathNAN);

}

static sal_Bool parseDuration(const xmlChar* aString, sal_Bool& bNegative, sal_Int32& nYears, sal_Int32& nMonth, sal_Int32& nDays,
                              sal_Int32& nHours, sal_Int32& nMinutes, sal_Int32& nSeconds)
{
    sal_Bool bTime = sal_False; 
    sal_Int32 nLength = strlen((char*)aString)+1;
    char *pString = (char*)rtl_allocateMemory(nLength);
    char *pString0 = pString;
    strncpy(pString, (char*)aString, nLength);

    if (pString[0] == '-') {
        bNegative = sal_True;
        pString++;
    }

    if (pString[0] != 'P')
    {
        rtl_freeMemory(pString0);
        return sal_False;
    }

    pString++;
    char* pToken = pString;
    while(pToken[0] != 0)
    {
        switch(pToken[0]) {
        case 'Y':
            pToken[0] = 0;
            nYears = atoi(pString);
            pString = ++pToken;
            break;
        case 'M':
            pToken[0] = 0;
            if (!bTime)
                nMonth = atoi(pString);
            else
                nMinutes = atoi(pString);
            pString = ++pToken;
            break;
        case 'D':
            pToken[0] = 0;
            nDays = atoi(pString);
            pString = ++pToken;
            break;
        case 'H':
            pToken[0] = 0;
            nHours = atoi(pString);
            pString = ++pToken;
            break;
        case 'S':
            pToken[0] = 0;
            nSeconds = atoi(pString);
            pString = ++pToken;
            break;
        case 'T':
            bTime = sal_True;
            pString = ++pToken;
            break;
        default:
            pToken++;
        }
    }
    rtl_freeMemory(pString0);
    return sal_True;
}

void xforms_secondsFuction(xmlXPathParserContextPtr ctxt, int nargs)
{
    
    
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);

    sal_Bool bNegative = sal_False;
    sal_Int32 nYears = 0;
    sal_Int32 nMonths = 0;
    sal_Int32 nDays = 0;
    sal_Int32 nHours = 0;
    sal_Int32 nMinutes = 0;
    sal_Int32 nSeconds = 0;

    if (parseDuration(pString, bNegative, nYears, nMonths, nDays, nHours, nMinutes, nSeconds))
    {
        nSeconds += nMinutes*60;
        nSeconds += nHours*60*60;
        nSeconds += nDays*24*60*60;
        
        if (bNegative)
            nSeconds = 0 - nSeconds;
        xmlXPathReturnNumber(ctxt, nSeconds);
    }
    else
        xmlXPathReturnNumber(ctxt, xmlXPathNAN);
}

void xforms_monthsFuction(xmlXPathParserContextPtr ctxt, int nargs)
{
    
    
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);

    sal_Bool bNegative = sal_False;
    sal_Int32 nYears = 0;
    sal_Int32 nMonths = 0;
    sal_Int32 nDays = 0;
    sal_Int32 nHours = 0;
    sal_Int32 nMinutes = 0;
    sal_Int32 nSeconds = 0;

    if (parseDuration(pString, bNegative, nYears, nMonths, nDays, nHours, nMinutes, nSeconds))
    {
        nMonths += nYears*12;
        
        if (bNegative)
            nMonths = 0 - nMonths;
        xmlXPathReturnNumber(ctxt, nMonths);
    }
    else
        xmlXPathReturnNumber(ctxt, xmlXPathNAN);

}


void xforms_instanceFuction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar *pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString((char*)pString, strlen((char*)pString), RTL_TEXTENCODING_UTF8);

    Reference< XModel > aModel = ((CLibxml2XFormsExtension*)ctxt->context->funcLookupData)->getModel();
    if (aModel.is())
    {
        Reference< XDocument > aInstance = aModel->getInstanceDocument(aString);
        if (aInstance.is())
        {
            try {
                
                Reference< XUnoTunnel > aTunnel(aInstance, UNO_QUERY_THROW);
                xmlNodePtr pNode = reinterpret_cast< xmlNodePtr >( aTunnel->getSomething(Sequence< sal_Int8 >()) );
                xmlXPathObjectPtr pObject = xmlXPathNewNodeSet(pNode);
                xmlXPathReturnNodeSet(ctxt, pObject->nodesetval);
            } catch (const RuntimeException&)
            {
                xmlXPathReturnEmptyNodeSet(ctxt);
            }
        }
        else
            xmlXPathReturnEmptyNodeSet(ctxt);
    }
    else
        xmlXPathReturnEmptyNodeSet(ctxt);

}


void xforms_currentFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 0) XP_ERROR(XPATH_INVALID_ARITY);

    Reference< XNode > aNode = ((CLibxml2XFormsExtension*)ctxt->context->funcLookupData)->getContextNode();

    if (aNode.is())
    {
        try {
            Reference< XUnoTunnel > aTunnel(aNode, UNO_QUERY_THROW);
            xmlNodePtr pNode = reinterpret_cast< xmlNodePtr >( aTunnel->getSomething(Sequence< sal_Int8 >()) );
            xmlXPathObjectPtr pObject = xmlXPathNewNodeSet(pNode);
            xmlXPathReturnNodeSet(ctxt, pObject->nodesetval);
        }
        catch (const RuntimeException&)
        {
            xmlXPathReturnEmptyNodeSet(ctxt);
        }
    }
    else
        xmlXPathReturnEmptyNodeSet(ctxt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
