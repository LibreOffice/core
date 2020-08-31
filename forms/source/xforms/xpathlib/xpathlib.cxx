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


#include <string.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xforms/XModel.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

#include <boost/lexical_cast.hpp>
#include <libxml/xpathInternals.h>

#include "xpathlib.hxx"
#include "extension.hxx"

// C interface

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xforms;
using namespace com::sun::star::lang;

xmlXPathFunction xforms_lookupFunc(void *, const xmlChar *xname, const xmlChar *)
{

    const char *name = reinterpret_cast<char const *>(xname);
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
        return xforms_secondsFunction;
    else if ((strcmp("months", name))==0)
        return xforms_monthsFunction;
    else if ((strcmp("instance", name))==0)
        return xforms_instanceFunction;
    else if ((strcmp("current", name))==0)
        return xforms_currentFunction;
    else
        return nullptr;
}

// boolean functions
void xforms_booleanFromStringFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar *pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString(reinterpret_cast<char*>(pString), strlen(reinterpret_cast<char*>(pString)), RTL_TEXTENCODING_UTF8);
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

// Number Functions
void xforms_avgFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    // use sum(), div() and count()
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);

    // save nodeset
    xmlXPathObjectPtr pObject = valuePop(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    //push back a copy
    valuePush(ctxt, xmlXPathObjectCopy(pObject));
    // get the Sum
    xmlXPathSumFunction(ctxt, 1);
    double nSum = xmlXPathPopNumber(ctxt);
    // push a copy once more
    valuePush(ctxt, xmlXPathObjectCopy(pObject));
    xmlXPathCountFunction(ctxt, 1);
    double nCount = xmlXPathPopNumber(ctxt);
    // push args for div()
    xmlXPathReturnNumber(ctxt, nSum);
    xmlXPathReturnNumber(ctxt, nCount);
    xmlXPathDivValues(ctxt);
    // the result is now on the ctxt stack
    xmlXPathFreeObject(pObject);
}

void xforms_minFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlNodeSetPtr pNodeSet = xmlXPathPopNodeSet(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    double nMinimum = 0;
    for (int i = 0; i <  xmlXPathNodeSetGetLength(pNodeSet); i++)
    {
        double nNumber = xmlXPathCastNodeToNumber(xmlXPathNodeSetItem(pNodeSet, i));
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
    for (int i = 0; i <  xmlXPathNodeSetGetLength(pNodeSet); i++)
    {
        double nNumber = xmlXPathCastNodeToNumber(xmlXPathNodeSetItem(pNodeSet, i));
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
    sal_Int32 nNotEmpty = 0;
    for (int i = 0; i <  xmlXPathNodeSetGetLength(pNodeSet); i++)
    {
        const xmlChar *aString = xmlXPathCastNodeToString(xmlXPathNodeSetItem(pNodeSet, i));
        if (*aString != 0) nNotEmpty++;
    }
    xmlXPathReturnNumber(ctxt, nNotEmpty);
}
void xforms_indexFunction(xmlXPathParserContextPtr /*ctxt*/, int /*nargs*/)
{
    // function index takes a string argument that is the IDREF of a
    // 'repeat' and returns the current 1-based position of the repeat
    // index of the identified repeat -- see xforms/9.3.1

    // doc.getElementByID
    // (...)
}

// String Functions
void xforms_propertyFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString(reinterpret_cast<char*>(pString), strlen(reinterpret_cast<char*>(pString)), RTL_TEXTENCODING_UTF8);
    if (aString.equalsIgnoreAsciiCase("version"))
        xmlXPathReturnString(ctxt, reinterpret_cast<xmlChar *>(const_cast<char *>("1.0")));
    else if (aString.equalsIgnoreAsciiCase("conformance-level"))
        xmlXPathReturnString(ctxt, reinterpret_cast<xmlChar *>(const_cast<char *>("conformance")));
    else
        xmlXPathReturnEmptyString(ctxt);
}

// Date and Time Functions

static OString makeDateTimeString (const DateTime& aDateTime)
{
    OStringBuffer aDateTimeString;
    aDateTimeString.append(static_cast<sal_Int32>(aDateTime.GetYear()));
    aDateTimeString.append('-');
    if (aDateTime.GetMonth()<10) aDateTimeString.append('0');
    aDateTimeString.append(static_cast<sal_Int32>(aDateTime.GetMonth()));
    aDateTimeString.append('-');
    if (aDateTime.GetDay()<10) aDateTimeString.append('0');
    aDateTimeString.append(static_cast<sal_Int32>(aDateTime.GetDay()));
    aDateTimeString.append('T');
    if (aDateTime.GetHour()<10) aDateTimeString.append('0');
    aDateTimeString.append(static_cast<sal_Int32>(aDateTime.GetHour()));
    aDateTimeString.append(':');
    if (aDateTime.GetMin()<10) aDateTimeString.append('0');
    aDateTimeString.append(static_cast<sal_Int32>(aDateTime.GetMin()));
    aDateTimeString.append(':');
    if (aDateTime.GetSec()<10) aDateTimeString.append('0');
    aDateTimeString.append(static_cast<sal_Int32>(aDateTime.GetSec()));
    aDateTimeString.append('Z');

    return aDateTimeString.makeStringAndClear();
}

// returns current system date and time in canonical xsd:dateTime
// format
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
    be omitted or, if present, the time zone must be Coordinated Universal tools::Time (UTC)
    indicated by a "Z".
    */
    DateTime aDateTime( DateTime::SYSTEM );
    OString aDateTimeString = makeDateTimeString(aDateTime);
    xmlChar *pString = static_cast<xmlChar*>(xmlMalloc(aDateTimeString.getLength()+1));
    strncpy(reinterpret_cast<char*>(pString), aDateTimeString.getStr(), aDateTimeString.getLength());
    pString[aDateTimeString.getLength()] = 0;
    xmlXPathReturnString(ctxt, pString);
}

static bool parseDateTime(const OUString& aString, DateTime& aDateTime)
{
    // take apart a canonical literal xsd:dateTime string
    //CCYY-MM-DDThh:mm:ss(Z)

    OUString aDateTimeString = aString.trim();

    // check length
    if (aDateTimeString.getLength() < 19 || aDateTimeString.getLength() > 20)
        return false;

    sal_Int32 nIndex = 0;
    sal_Int32 nYear = aDateTimeString.getToken(0, '-', nIndex).toInt32();
    sal_Int32 nMonth = aDateTimeString.getToken(0, '-', nIndex).toInt32();
    sal_Int32 nDay = aDateTimeString.getToken(0, 'T', nIndex).toInt32();
    sal_Int32 nHour = aDateTimeString.getToken(0, ':', nIndex).toInt32();
    sal_Int32 nMinute = aDateTimeString.getToken(0, ':', nIndex).toInt32();
    sal_Int32 nSecond = aDateTimeString.getToken(0, 'Z', nIndex).toInt32();

    Date tmpDate(static_cast<sal_uInt16>(nDay), static_cast<sal_uInt16>(nMonth), static_cast<sal_uInt16>(nYear));
    tools::Time tmpTime(nHour, nMinute, nSecond);
    DateTime tmpDateTime(tmpDate, tmpTime);
    if (aString.lastIndexOf('Z') < 0)
        tmpDateTime.ConvertToUTC();

    aDateTime = tmpDateTime;

    return true;
}


void xforms_daysFromDateFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    // number of days from 1970-01-01 to supplied xsd:date(Time)

    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString(reinterpret_cast<char*>(pString), strlen(reinterpret_cast<char*>(pString)), RTL_TEXTENCODING_UTF8);

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
    // number of seconds from 1970-01-01T00:00:00Z to supplied xsd:date(Time)

    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString(reinterpret_cast<char*>(pString), strlen(reinterpret_cast<char*>(pString)), RTL_TEXTENCODING_UTF8);

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

static bool parseDuration(const xmlChar* aString, bool& bNegative, sal_Int32& nYears, sal_Int32& nMonth, sal_Int32& nDays,
                              sal_Int32& nHours, sal_Int32& nMinutes, sal_Int32& nSeconds)
{
    bool bTime = false; // in part after T
    const xmlChar *pString = aString;

    if (pString[0] == '-') {
        bNegative = true;
        pString++;
    }

    if (pString[0] != 'P')
    {
        return false;
    }

    pString++;
    const xmlChar* pToken = pString;
    while(pToken[0] != 0)
    {
        switch(pToken[0]) {
        case 'Y':
            nYears = boost::lexical_cast<sal_Int32>(pString, pString-pToken);
            pString = ++pToken;
            break;
        case 'M':
            if (!bTime)
                nMonth = boost::lexical_cast<sal_Int32>(pString, pString-pToken);
            else
                nMinutes = boost::lexical_cast<sal_Int32>(pString, pString-pToken);
            pString = ++pToken;
            break;
        case 'D':
            nDays = boost::lexical_cast<sal_Int32>(pString, pString-pToken);
            pString = ++pToken;
            break;
        case 'H':
            nHours = boost::lexical_cast<sal_Int32>(pString, pString-pToken);
            pString = ++pToken;
            break;
        case 'S':
            nSeconds = boost::lexical_cast<sal_Int32>(pString, pString-pToken);
            pString = ++pToken;
            break;
        case 'T':
            bTime = true;
            pString = ++pToken;
            break;
        default:
            pToken++;
        }
    }
    return true;
}

void xforms_secondsFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    // convert a xsd:duration to seconds
    // (-)PnYnMnDTnHnMnS
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);

    bool bNegative = false;
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
        // year and month are ignored according to spec
        if (bNegative)
            nSeconds = 0 - nSeconds;
        xmlXPathReturnNumber(ctxt, nSeconds);
    }
    else
        xmlXPathReturnNumber(ctxt, xmlXPathNAN);
}

void xforms_monthsFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    // convert a xsd:duration to seconds
    // (-)PnYnMnDTnHnMnS
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar* pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);

    bool bNegative = false;
    sal_Int32 nYears = 0;
    sal_Int32 nMonths = 0;
    sal_Int32 nDays = 0;
    sal_Int32 nHours = 0;
    sal_Int32 nMinutes = 0;
    sal_Int32 nSeconds = 0;

    if (parseDuration(pString, bNegative, nYears, nMonths, nDays, nHours, nMinutes, nSeconds))
    {
        nMonths += nYears*12;
        // Days, Hours, Minutes and seconds are ignored, see spec
        if (bNegative)
            nMonths = 0 - nMonths;
        xmlXPathReturnNumber(ctxt, nMonths);
    }
    else
        xmlXPathReturnNumber(ctxt, xmlXPathNAN);

}

// Node-set Functions
void xforms_instanceFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 1) XP_ERROR(XPATH_INVALID_ARITY);
    xmlChar *pString = xmlXPathPopString(ctxt);
    if (xmlXPathCheckError(ctxt)) XP_ERROR(XPATH_INVALID_TYPE);
    OUString aString(reinterpret_cast<char*>(pString), strlen(reinterpret_cast<char*>(pString)), RTL_TEXTENCODING_UTF8);

    Reference< XModel > aModel = static_cast<CLibxml2XFormsExtension*>(ctxt->context->funcLookupData)->getModel();
    if (aModel.is())
    {
        Reference< XDocument > aInstance = aModel->getInstanceDocument(aString);
        if (aInstance.is())
        {
            try {
                // xmlXPathObjectPtr xmlXPathNewNodeSet        (xmlNodePtr val);
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

// Node-set Functions, XForms 1.1
void xforms_currentFunction(xmlXPathParserContextPtr ctxt, int nargs)
{
    if (nargs != 0) XP_ERROR(XPATH_INVALID_ARITY);

    Reference< XNode > aNode = static_cast<CLibxml2XFormsExtension*>(ctxt->context->funcLookupData)->getContextNode();

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
