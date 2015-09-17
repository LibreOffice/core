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

#include <sal/types.h>
#include <osl/thread.h>
#include <tools/datetime.hxx>
#include <tools/inetmime.hxx>
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>
#include <tools/contnr.hxx>
#include <rtl/instance.hxx>
#include <comphelper/string.hxx>

#include <stdio.h>
#include <map>

inline bool ascii_isDigit( sal_Unicode ch )
{
    return ((ch >= 0x0030) && (ch <= 0x0039));
}

inline bool ascii_isLetter( sal_Unicode ch )
{
    return (( (ch >= 0x0041) && (ch <= 0x005A)) || ((ch >= 0x0061) && (ch <= 0x007A)));
}

void INetMIMEMessage::SetHeaderField_Impl (
    INetMIME::HeaderFieldType  eType,
    const OString &rName,
    const OUString &rValue,
    sal_uIntPtr &rnIndex)
{
    INetMIMEOutputSink aSink(0, INetMIMEOutputSink::NO_LINE_LENGTH_LIMIT);
    INetMIME::writeHeaderFieldBody (
        aSink, eType, rValue, osl_getThreadTextEncoding(), false);
    SetHeaderField_Impl (
        INetMessageHeader (rName, aSink.takeBuffer()), rnIndex);
}

/* ParseDateField and local helper functions.
 *
 * Parses a String in (implied) GMT format into class Date and tools::Time objects.
 * Four formats are accepted:
 *
 *  [Wkd,] 1*2DIGIT Mon 2*4DIGIT 00:00:00 [GMT]  (rfc1123)
 *  [Wkd,] 00 Mon 0000 00:00:00 [GMT])           (rfc822, rfc1123)
 *   Weekday, 00-Mon-00 00:00:00 [GMT]           (rfc850, rfc1036)
 *   Wkd Mon 00 00:00:00 0000 [GMT]              (ctime)
 *   1*DIGIT                                     (delta seconds)
 */

static const sal_Char *months[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static sal_uInt16 ParseNumber(const OString& rStr, sal_uInt16& nIndex)
{
    sal_uInt16 n = nIndex;
    while ((n < rStr.getLength()) && ascii_isDigit(rStr[n])) n++;

    OString aNum(rStr.copy(nIndex, (n - nIndex)));
    nIndex = n;

    return (sal_uInt16)(aNum.toInt32());
}

static sal_uInt16 ParseMonth(const OString& rStr, sal_uInt16& nIndex)
{
    sal_uInt16 n = nIndex;
    while ((n < rStr.getLength()) && ascii_isLetter(rStr[n])) n++;

    OString aMonth(rStr.copy(nIndex, 3));
    nIndex = n;

    sal_uInt16 i;
    for (i = 0; i < 12; i++)
        if (aMonth.equalsIgnoreAsciiCase(months[i])) break;
    return (i + 1);
}

bool INetMIMEMessage::ParseDateField (
    const OUString& rDateFieldW, DateTime& rDateTime)
{
    OString aDateField(OUStringToOString(rDateFieldW,
        RTL_TEXTENCODING_ASCII_US));

    if (aDateField.isEmpty()) return false;

    if (aDateField.indexOf(':') != -1)
    {
        // Some DateTime format.
        sal_uInt16 nIndex = 0;

        // Skip over <Wkd> or <Weekday>, leading and trailing space.
        while ((nIndex < aDateField.getLength()) &&
               (aDateField[nIndex] == ' '))
            nIndex++;

        while (
            (nIndex < aDateField.getLength()) &&
            (ascii_isLetter (aDateField[nIndex]) ||
             (aDateField[nIndex] == ',')     ))
            nIndex++;

        while ((nIndex < aDateField.getLength()) &&
               (aDateField[nIndex] == ' '))
            nIndex++;

        if (ascii_isLetter (aDateField[nIndex]))
        {
            // Format: ctime().
            if ((aDateField.getLength() - nIndex) < 20) return false;

            rDateTime.SetMonth  (ParseMonth  (aDateField, nIndex)); nIndex++;
            rDateTime.SetDay    (ParseNumber (aDateField, nIndex)); nIndex++;

            rDateTime.SetHour   (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetMin    (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetSec    (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetNanoSec (0);

            sal_uInt16 nYear = ParseNumber (aDateField, nIndex);
            if (nYear < 100) nYear += 1900;
            rDateTime.SetYear   (nYear);
        }
        else
        {
            // Format: RFC1036 or RFC1123.
            if ((aDateField.getLength() - nIndex) < 17) return false;

            rDateTime.SetDay    (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetMonth  (ParseMonth  (aDateField, nIndex)); nIndex++;

            sal_uInt16 nYear  = ParseNumber (aDateField, nIndex);  nIndex++;
            if (nYear < 100) nYear += 1900;
            rDateTime.SetYear   (nYear);

            rDateTime.SetHour   (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetMin    (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetSec    (ParseNumber (aDateField, nIndex)); nIndex++;
            rDateTime.SetNanoSec (0);

            const char cPossiblePlusMinus = nIndex < aDateField.getLength() ? aDateField[nIndex] : 0;
            if (cPossiblePlusMinus == '+' || cPossiblePlusMinus == '-')
            {
                // Offset from GMT: "(+|-)HHMM".
                bool bEast   = (aDateField[nIndex++] == '+');
                sal_uInt16 nOffset = ParseNumber (aDateField, nIndex);
                if (nOffset > 0)
                {
                    tools::Time aDiff( tools::Time::EMPTY );
                    aDiff.SetHour   (nOffset / 100);
                    aDiff.SetMin    (nOffset % 100);
                    aDiff.SetSec    (0);
                    aDiff.SetNanoSec (0);

                    if (bEast)
                        rDateTime -= aDiff;
                    else
                        rDateTime += aDiff;
                }
            }
        }
    }
    else if (comphelper::string::isdigitAsciiString(aDateField))
    {
        // Format: delta seconds.
        tools::Time aDelta (0);
        aDelta.SetTime (aDateField.toInt32() * 100);

        DateTime aNow( DateTime::SYSTEM );
        aNow += aDelta;
        aNow.ConvertToUTC();

        rDateTime.SetDate (aNow.GetDate());
        rDateTime.SetTime (aNow.GetTime());
    }
    else
    {
        // Junk.
        return false;
    }

    return (rDateTime.IsValidAndGregorian() &&
            !((rDateTime.GetSec()  > 59) ||
              (rDateTime.GetMin()  > 59) ||
              (rDateTime.GetHour() > 23)    ));
}

static const std::map<InetMessageMime, const char*> ImplINetMIMEMessageHeaderData =
{
    { InetMessageMime::VERSION, "MIME-Version"},
    { InetMessageMime::CONTENT_DISPOSITION, "Content-Disposition"},
    { InetMessageMime::CONTENT_TYPE, "Content-Type"},
    { InetMessageMime::CONTENT_TRANSFER_ENCODING, "Content-Transfer-Encoding"}
};

INetMIMEMessage::INetMIMEMessage()
    : pParent(NULL)
{
    for (sal_uInt16 i = 0; i < static_cast<int>(InetMessageMime::NUMHDR); i++)
        m_nMIMEIndex[static_cast<InetMessageMime>(i)] = CONTAINER_ENTRY_NOTFOUND;
}

INetMIMEMessage::~INetMIMEMessage()
{
    for (auto i: m_aHeaderList) {
        delete i;
    }
    for (auto i: aChildren) {
        delete i;
    }
}

void INetMIMEMessage::SetMIMEVersion (const OUString& rVersion)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::VERSION), rVersion,
        m_nMIMEIndex[InetMessageMime::VERSION]);
}

void INetMIMEMessage::SetContentDisposition (const OUString& rDisposition)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::CONTENT_DISPOSITION), rDisposition,
        m_nMIMEIndex[InetMessageMime::CONTENT_DISPOSITION]);
}

void INetMIMEMessage::SetContentType (const OUString& rType)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::CONTENT_TYPE), rType,
        m_nMIMEIndex[InetMessageMime::CONTENT_TYPE]);
}

void INetMIMEMessage::SetContentTransferEncoding (
    const OUString& rEncoding)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::CONTENT_TRANSFER_ENCODING), rEncoding,
        m_nMIMEIndex[InetMessageMime::CONTENT_TRANSFER_ENCODING]);
}

OUString INetMIMEMessage::GetDefaultContentType()
{
    if (pParent != NULL)
    {
        OUString aParentCT (pParent->GetContentType());
        if (aParentCT.isEmpty())
            aParentCT = pParent->GetDefaultContentType();

        if (aParentCT.equalsIgnoreAsciiCase("multipart/digest"))
            return OUString("message/rfc822");
    }
    return OUString("text/plain; charset=us-ascii");
}

bool INetMIMEMessage::EnableAttachMultipartFormDataChild()
{
    // Check context.
    if (IsContainer())
        return false;

    // Generate a unique boundary from current time.
    sal_Char sTail[16 + 1];
    tools::Time aCurTime( tools::Time::SYSTEM );
    sal_uInt64 nThis = reinterpret_cast< sal_uIntPtr >( this ); // we can be on a 64bit architecture
    nThis = ( ( nThis >> 32 ) ^ nThis ) & SAL_MAX_UINT32;
    sprintf (sTail, "%08X%08X",
             static_cast< unsigned int >(aCurTime.GetTime()),
             static_cast< unsigned int >(nThis));
    m_aBoundary = "------------_4D48";
    m_aBoundary += sTail;

    // Set header fields.
    SetMIMEVersion(OUString("1.0"));
    SetContentType(
        OUString::fromUtf8("multipart/form-data; boundary=" + m_aBoundary));
    SetContentTransferEncoding(OUString("7bit"));

    // Done.
    return true;
}

bool INetMIMEMessage::AttachChild(INetMIMEMessage& rChildMsg, bool bOwner)
{
    if (IsContainer())
    {
        if (bOwner) rChildMsg.pParent = this;
        aChildren.push_back( &rChildMsg );

        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
