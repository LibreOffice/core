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
#include <tools/datetime.hxx>
#include <tools/inetmsg.hxx>
#include <comphelper/string.hxx>
#include <rtl/character.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/sprintf.hxx>
#include <o3tl/string_view.hxx>

#include <map>

void INetMIMEMessage::SetHeaderField_Impl (
    const OString &rName,
    const OUString &rValue,
    sal_uInt32 &rnIndex)
{
    SetHeaderField_Impl (
        INetMessageHeader (rName, rValue.toUtf8()), rnIndex);
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

static const char *months[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static sal_uInt16 ParseNumber(std::string_view rStr, size_t& nIndex)
{
    size_t n = nIndex;
    while ((n < rStr.size())
           && rtl::isAsciiDigit(static_cast<unsigned char>(rStr[n])))
        n++;

    std::string_view aNum(rStr.substr(nIndex, (n - nIndex)));
    nIndex = n;

    return static_cast<sal_uInt16>(o3tl::toInt32(aNum));
}

static sal_uInt16 ParseMonth(std::string_view rStr, size_t& nIndex)
{
    size_t n = nIndex;
    while ((n < rStr.size())
           && rtl::isAsciiAlpha(static_cast<unsigned char>(rStr[n])))
        n++;

    std::string_view aMonth(rStr.substr(nIndex, 3));
    nIndex = n;

    sal_uInt16 i;
    for (i = 0; i < 12; i++)
        if (o3tl::equalsIgnoreAsciiCase(aMonth, months[i])) break;
    return (i + 1);
}

bool INetMIMEMessage::ParseDateField (
    std::u16string_view rDateFieldW, DateTime& rDateTime)
{
    OString aDateField(OUStringToOString(rDateFieldW,
        RTL_TEXTENCODING_ASCII_US));

    if (aDateField.isEmpty()) return false;

    if (aDateField.indexOf(':') != -1)
    {
        // Some DateTime format.
        size_t nIndex = 0;

        // Skip over <Wkd> or <Weekday>, leading and trailing space.
        while ((nIndex < o3tl::make_unsigned(aDateField.getLength())) &&
               (aDateField[nIndex] == ' '))
            nIndex++;

        while (
            (nIndex < o3tl::make_unsigned(aDateField.getLength())) &&
            (rtl::isAsciiAlpha (static_cast<unsigned char>(aDateField[nIndex])) ||
             (aDateField[nIndex] == ',')     ))
            nIndex++;

        while ((nIndex < o3tl::make_unsigned(aDateField.getLength())) &&
               (aDateField[nIndex] == ' '))
            nIndex++;

        if (rtl::isAsciiAlpha (static_cast<unsigned char>(aDateField[nIndex])))
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

            const char cPossiblePlusMinus = nIndex < o3tl::make_unsigned(aDateField.getLength()) ? aDateField[nIndex] : 0;
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

const std::map<InetMessageMime, const char*> ImplINetMIMEMessageHeaderData =
{
    { InetMessageMime::VERSION, "MIME-Version"},
    { InetMessageMime::CONTENT_DISPOSITION, "Content-Disposition"},
    { InetMessageMime::CONTENT_TYPE, "Content-Type"},
    { InetMessageMime::CONTENT_TRANSFER_ENCODING, "Content-Transfer-Encoding"}
};

INetMIMEMessage::INetMIMEMessage()
    : pParent(nullptr)
{
    for (sal_uInt16 i = 0; i < static_cast<int>(InetMessageMime::NUMHDR); i++)
        m_nMIMEIndex[static_cast<InetMessageMime>(i)] = SAL_MAX_UINT32;
}

INetMIMEMessage::~INetMIMEMessage()
{
}

void INetMIMEMessage::SetMIMEVersion (const OUString& rVersion)
{
    SetHeaderField_Impl (
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::VERSION), rVersion,
        m_nMIMEIndex[InetMessageMime::VERSION]);
}

void INetMIMEMessage::SetContentDisposition (const OUString& rDisposition)
{
    SetHeaderField_Impl (
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::CONTENT_DISPOSITION), rDisposition,
        m_nMIMEIndex[InetMessageMime::CONTENT_DISPOSITION]);
}

void INetMIMEMessage::SetContentType (const OUString& rType)
{
    SetHeaderField_Impl (
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::CONTENT_TYPE), rType,
        m_nMIMEIndex[InetMessageMime::CONTENT_TYPE]);
}

void INetMIMEMessage::SetContentTransferEncoding (
    const OUString& rEncoding)
{
    SetHeaderField_Impl (
        ImplINetMIMEMessageHeaderData.at(InetMessageMime::CONTENT_TRANSFER_ENCODING), rEncoding,
        m_nMIMEIndex[InetMessageMime::CONTENT_TRANSFER_ENCODING]);
}

OUString INetMIMEMessage::GetDefaultContentType()
{
    if (pParent != nullptr)
    {
        OUString aParentCT (pParent->GetContentType());
        if (aParentCT.isEmpty())
            aParentCT = pParent->GetDefaultContentType();

        if (aParentCT.equalsIgnoreAsciiCase("multipart/digest"))
            return u"message/rfc822"_ustr;
    }
    return u"text/plain; charset=us-ascii"_ustr;
}

void INetMIMEMessage::EnableAttachMultipartFormDataChild()
{
    // Check context.
    if (IsContainer())
        return;

    // Generate a unique boundary from current time.
    char sTail[16 + 1];
    tools::Time aCurTime( tools::Time::SYSTEM );
    sal_uInt64 nThis = reinterpret_cast< sal_uIntPtr >( this ); // we can be on a 64bit architecture
    nThis = ( ( nThis >> 32 ) ^ nThis ) & SAL_MAX_UINT32;
    o3tl::sprintf (sTail, "%08X%08X",
             static_cast< unsigned int >(aCurTime.GetTime()),
             static_cast< unsigned int >(nThis));
    m_aBoundary = "------------_4D48"_ostr;
    m_aBoundary += sTail;

    // Set header fields.
    SetMIMEVersion(u"1.0"_ustr);
    SetContentType(
        "multipart/form-data; boundary=" + OUString::fromUtf8(m_aBoundary));
    SetContentTransferEncoding(u"7bit"_ustr);
}

void INetMIMEMessage::AttachChild(std::unique_ptr<INetMIMEMessage> pChildMsg)
{
    assert(IsContainer());
    if (IsContainer())
    {
        pChildMsg->pParent = this;
        aChildren.push_back( std::move(pChildMsg) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
