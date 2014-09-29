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
#include <rtl/strbuf.hxx>
#include <comphelper/string.hxx>

#include <stdio.h>

inline bool ascii_isDigit( sal_Unicode ch )
{
    return ((ch >= 0x0030) && (ch <= 0x0039));
}

inline bool ascii_isLetter( sal_Unicode ch )
{
    return (( (ch >= 0x0041) && (ch <= 0x005A)) || ((ch >= 0x0061) && (ch <= 0x007A)));
}

inline sal_Unicode ascii_toLowerCase( sal_Unicode ch )
{
    if ( (ch >= 0x0041) && (ch <= 0x005A) )
        return ch + 0x20;
    else
        return ch;
}

INetMessage::~INetMessage()
{
    ListCleanup_Impl();
}

void INetMessage::ListCleanup_Impl()
{
    // Cleanup.
    sal_uIntPtr i, n = m_aHeaderList.size();
    for (i = 0; i < n; i++)
        delete m_aHeaderList[ i ];
    m_aHeaderList.clear();
}

void INetMessage::ListCopy (const INetMessage &rMsg)
{
    if (!(this == &rMsg))
    {
        // Cleanup.
        ListCleanup_Impl();

        // Copy.
        sal_uIntPtr i, n = rMsg.GetHeaderCount();
        for (i = 0; i < n; i++)
        {
            INetMessageHeader *p = rMsg.m_aHeaderList[ i ];
            m_aHeaderList.push_back( new INetMessageHeader(*p) );
        }
    }
}

void INetMessage::SetHeaderField_Impl (
    INetMIME::HeaderFieldType  eType,
    const OString &rName,
    const OUString &rValue,
    sal_uIntPtr &rnIndex)
{
    INetMIMEStringOutputSink aSink (0, 32767); /* weird the mime standard says that aline MUST not be longeur that 998 */
    INetMIME::writeHeaderFieldBody (
        aSink, eType, rValue, osl_getThreadTextEncoding(), false);
    SetHeaderField_Impl (
        INetMessageHeader (rName, aSink.takeBuffer()), rnIndex);
}

sal_uIntPtr INetMessage::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nIndex)
{
    sal_uIntPtr nResult = nIndex;
    SetHeaderField_Impl (rHeader, nResult);
    return nResult;
}

SvStream& INetMessage::operator<< (SvStream& rStrm) const
{
    rStrm.WriteUInt32( m_nDocSize );
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStrm, m_aDocName, RTL_TEXTENCODING_UTF8);

    sal_uIntPtr i, n = m_aHeaderList.size();
    rStrm.WriteUInt32( n );

    for (i = 0; i < n; i++)
        WriteINetMessageHeader( rStrm, *( m_aHeaderList[ i ] ) );

    return rStrm;
}

SvStream& INetMessage::operator>> (SvStream& rStrm)
{
    // Cleanup.
    m_nDocSize = 0;
    m_xDocLB.Clear();
    ListCleanup_Impl();

    sal_uInt32 nTemp;

    // Copy.
    rStrm.ReadUInt32( nTemp );
    m_nDocSize = nTemp;
    m_aDocName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStrm, RTL_TEXTENCODING_UTF8);

    sal_uIntPtr i, n = 0;
    rStrm.ReadUInt32( nTemp );
    n = nTemp;

    for (i = 0; i < n; i++)
    {
        INetMessageHeader *p = new INetMessageHeader();
        ReadINetMessageHeader( rStrm, *p );
        m_aHeaderList.push_back( p );
    }

    // Done.
    return rStrm;
}

namespace
{
    struct ImplINetRFC822MessageHeaderDataImpl
    {
        const OString* operator()()
        {
            static const OString _ImplINetRFC822MessageHeaderData[] =
            {
                OString("BCC"),
                OString("CC"),
                OString("Comments"),
                OString("Date"),
                OString("From"),
                OString("In-Reply-To"),
                OString("Keywords"),
                OString("Message-ID"),
                OString("References"),
                OString("Reply-To"),
                OString("Return-Path"),
                OString("Subject"),
                OString("Sender"),
                OString("To"),
                OString("X-Mailer"),
                OString("Return-Receipt-To")
            };
            return &_ImplINetRFC822MessageHeaderData[0];
        }
    };

    struct ImplINetRFC822MessageHeaderData
        : public rtl::StaticAggregate< const OString, ImplINetRFC822MessageHeaderDataImpl > {};
}

#define HDR(n) ImplINetRFC822MessageHeaderData::get()[(n)]

enum _ImplINetRFC822MessageHeaderState
{
    INETMSG_RFC822_BEGIN,
    INETMSG_RFC822_CHECK,
    INETMSG_RFC822_OK,
    INETMSG_RFC822_JUNK,

    INETMSG_RFC822_TOKEN_RE,
    INETMSG_RFC822_TOKEN_RETURNMINUS,
    INETMSG_RFC822_TOKEN_XMINUS,
    INETMSG_RFC822_LETTER_C,
    INETMSG_RFC822_LETTER_S
};

INetRFC822Message::INetRFC822Message()
    : INetMessage()
{
    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        m_nIndex[i] = CONTAINER_ENTRY_NOTFOUND;
}

INetRFC822Message::INetRFC822Message (const INetRFC822Message& rMsg)
    : INetMessage (rMsg)
{
    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        m_nIndex[i] = rMsg.m_nIndex[i];
}

INetRFC822Message& INetRFC822Message::operator= (const INetRFC822Message& rMsg)
{
    if (this != &rMsg)
    {
        INetMessage::operator= (rMsg);

        for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
            m_nIndex[i] = rMsg.m_nIndex[i];
    }
    return *this;
}

INetRFC822Message::~INetRFC822Message()
{
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

bool INetRFC822Message::ParseDateField (
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

            if ((aDateField[nIndex] == '+') ||
                (aDateField[nIndex] == '-')    )
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

// Header Field Parser
sal_uIntPtr INetRFC822Message::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex)
{
    OString aName (rHeader.GetName());
    const sal_Char *pData = aName.getStr();
    const sal_Char *pStop = pData + aName.getLength() + 1;
    const sal_Char *check = "";

    sal_uIntPtr       nIdx     = CONTAINER_APPEND;
    int         eState   = INETMSG_RFC822_BEGIN;
    int         eOkState = INETMSG_RFC822_OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case INETMSG_RFC822_BEGIN:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'b':
                        check = "cc";
                        nIdx = INETMSG_RFC822_BCC;
                        break;

                    case 'c':
                        eState = INETMSG_RFC822_LETTER_C;
                        break;

                    case 'd':
                        check = "ate";
                        nIdx = INETMSG_RFC822_DATE;
                        break;

                    case 'f':
                        check = "rom";
                        nIdx = INETMSG_RFC822_FROM;
                        break;

                    case 'i':
                        check = "n-reply-to";
                        nIdx = INETMSG_RFC822_IN_REPLY_TO;
                        break;

                    case 'k':
                        check = "eywords";
                        nIdx = INETMSG_RFC822_KEYWORDS;
                        break;

                    case 'm':
                        check = "essage-id";
                        nIdx = INETMSG_RFC822_MESSAGE_ID;
                        break;

                    case 'r':
                        check = "e";
                        eOkState = INETMSG_RFC822_TOKEN_RE;
                        break;

                    case 's':
                        eState = INETMSG_RFC822_LETTER_S;
                        break;

                    case 't':
                        check = "o";
                        nIdx = INETMSG_RFC822_TO;
                        break;

                    case 'x':
                        check = "-";
                        eOkState = INETMSG_RFC822_TOKEN_XMINUS;
                        break;

                    default:
                        eState = INETMSG_RFC822_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_RFC822_TOKEN_RE:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'f':
                        check = "erences";
                        nIdx = INETMSG_RFC822_REFERENCES;
                        break;

                    case 'p':
                        check = "ly-to";
                        nIdx = INETMSG_RFC822_REPLY_TO;
                        break;

                    case 't':
                        check = "urn-";
                        eOkState = INETMSG_RFC822_TOKEN_RETURNMINUS;
                        break;

                    default:
                        eState = INETMSG_RFC822_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_RFC822_TOKEN_RETURNMINUS:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'p':
                        check = "ath";
                        nIdx = INETMSG_RFC822_RETURN_PATH;
                        break;

                    case 'r':
                        check = "eceipt-to";
                        nIdx = INETMSG_RFC822_RETURN_RECEIPT_TO;
                        break;

                    default:
                        eState = INETMSG_RFC822_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_RFC822_TOKEN_XMINUS:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'm':
                        check = "ailer";
                        nIdx = INETMSG_RFC822_X_MAILER;
                        break;

                    default:
                        eState = INETMSG_RFC822_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_RFC822_LETTER_C:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'c':
                        check = "";
                        nIdx = INETMSG_RFC822_CC;
                        break;

                    case 'o':
                        check = "mments";
                        nIdx = INETMSG_RFC822_COMMENTS;
                        break;

                    default:
                        eState = INETMSG_RFC822_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_RFC822_LETTER_S:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'e':
                        check = "nder";
                        nIdx = INETMSG_RFC822_SENDER;
                        break;

                    case 'u':
                        check = "bject";
                        nIdx = INETMSG_RFC822_SUBJECT;
                        break;

                    default:
                        eState = INETMSG_RFC822_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_RFC822_CHECK:
                if (*check)
                {
                    while (*pData && *check &&
                           (ascii_toLowerCase (*pData) == *check))
                    {
                        pData++;
                        check++;
                    }
                }
                else
                {
                    check = pData;
                }
                eState = (*check == '\0') ? eOkState : INETMSG_RFC822_JUNK;
                break;

            case INETMSG_RFC822_OK:
                pData = pStop;
                SetHeaderField_Impl (
                    INetMessageHeader (HDR(nIdx), rHeader.GetValue()),
                    m_nIndex[nIdx]);
                nNewIndex = m_nIndex[nIdx];
                break;

            default: // INETMSG_RFC822_JUNK
                pData = pStop;
                nNewIndex = INetMessage::SetHeaderField (rHeader, nNewIndex);
                break;
        }
    }
    return nNewIndex;
}

SvStream& INetRFC822Message::operator<< (SvStream& rStrm) const
{
    INetMessage::operator<< (rStrm);

    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        rStrm.WriteUInt32( m_nIndex[i] );

    return rStrm;
}

SvStream& INetRFC822Message::operator>> (SvStream& rStrm)
{
    INetMessage::operator>> (rStrm);

    sal_uInt32 nTemp;
    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
    {
        rStrm.ReadUInt32( nTemp );
        m_nIndex[i] = nTemp;
    }

    return rStrm;
}

namespace
{
    struct ImplINetMIMEMessageHeaderDataImpl
    {
        const OString* operator()()
        {
            static const OString _ImplINetMIMEMessageHeaderData[] =
            {
                OString("MIME-Version"),
                OString("Content-Description"),
                OString("Content-Disposition"),
                OString("Content-ID"),
                OString("Content-Type"),
                OString("Content-Transfer-Encoding")
            };
            return &_ImplINetMIMEMessageHeaderData[0];
        }
    };

    struct ImplINetMIMEMessageHeaderData
        : public rtl::StaticAggregate< const OString, ImplINetMIMEMessageHeaderDataImpl > {};
}

#define MIMEHDR(n) ImplINetMIMEMessageHeaderData::get()[(n)]

enum _ImplINetMIMEMessageHeaderState
{
    INETMSG_MIME_BEGIN,
    INETMSG_MIME_CHECK,
    INETMSG_MIME_OK,
    INETMSG_MIME_JUNK,

    INETMSG_MIME_TOKEN_CONTENT,
    INETMSG_MIME_TOKEN_CONTENT_D,
    INETMSG_MIME_TOKEN_CONTENT_T
};

INetMIMEMessage::INetMIMEMessage()
    : INetRFC822Message (),
      pParent       (NULL),
      bHeaderParsed (false)
{
    for (sal_uInt16 i = 0; i < INETMSG_MIME_NUMHDR; i++)
        m_nIndex[i] = CONTAINER_ENTRY_NOTFOUND;
}

INetMIMEMessage::INetMIMEMessage (const INetMIMEMessage& rMsg)
    : INetRFC822Message (rMsg)
    , pParent(NULL)
{
    CopyImp (rMsg);
}

INetMIMEMessage& INetMIMEMessage::operator= (
    const INetMIMEMessage& rMsg)
{
    if (this != &rMsg)
    {
        // Assign base.
        INetRFC822Message::operator= (rMsg);

        CleanupImp();
        CopyImp (rMsg);
    }
    return *this;
}

INetMIMEMessage::~INetMIMEMessage()
{
    CleanupImp();
}

void INetMIMEMessage::CleanupImp()
{
    for( size_t i = 0, n = aChildren.size(); i < n; ++i ) {
        delete aChildren[ i ];
    }
    aChildren.clear();
}

void INetMIMEMessage::CopyImp (const INetMIMEMessage& rMsg)
{
    bHeaderParsed = rMsg.bHeaderParsed;

    size_t i;
    for (i = 0; i < INETMSG_MIME_NUMHDR; i++)
        m_nIndex[i] = rMsg.m_nIndex[i];

    m_aBoundary = rMsg.m_aBoundary;

    for (i = 0; i < rMsg.aChildren.size(); i++)
    {
        INetMIMEMessage *pChild = rMsg.aChildren[ i ];

        if (pChild->pParent == &rMsg)
        {
            pChild = pChild->CreateMessage (*pChild);
            pChild->pParent = this;
        }
        aChildren.push_back( pChild );
    }
}

INetMIMEMessage *INetMIMEMessage::CreateMessage (
    const INetMIMEMessage& rMsg) const
{
    return (new INetMIMEMessage (rMsg));
}

// Header Field Parser
sal_uIntPtr INetMIMEMessage::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex)
{
    OString aName (rHeader.GetName());
    const sal_Char *pData = aName.getStr();
    const sal_Char *pStop = pData + aName.getLength() + 1;
    const sal_Char *check = "";

    sal_uIntPtr      nIdx     = CONTAINER_APPEND;
    int        eState   = INETMSG_MIME_BEGIN;
    int        eOkState = INETMSG_MIME_OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case INETMSG_MIME_BEGIN:
                eState = INETMSG_MIME_CHECK;
                eOkState = INETMSG_MIME_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'c':
                        check = "ontent-";
                        eOkState = INETMSG_MIME_TOKEN_CONTENT;
                        break;

                    case 'm':
                        check = "ime-version";
                        nIdx = INETMSG_MIME_VERSION;
                        break;

                    default:
                        eState = INETMSG_MIME_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_MIME_TOKEN_CONTENT:
                eState = INETMSG_MIME_CHECK;
                eOkState = INETMSG_MIME_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'd':
                        eState = INETMSG_MIME_TOKEN_CONTENT_D;
                        break;

                    case 'i':
                        check = "d";
                        nIdx = INETMSG_MIME_CONTENT_ID;
                        break;

                    case 't':
                        eState = INETMSG_MIME_TOKEN_CONTENT_T;
                        break;

                    default:
                        eState = INETMSG_MIME_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_MIME_TOKEN_CONTENT_D:
                eState = INETMSG_MIME_CHECK;
                eOkState = INETMSG_MIME_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'e':
                        check = "scription";
                        nIdx = INETMSG_MIME_CONTENT_DESCRIPTION;
                        break;

                    case 'i':
                        check = "sposition";
                        nIdx = INETMSG_MIME_CONTENT_DISPOSITION;
                        break;

                    default:
                        eState = INETMSG_MIME_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_MIME_TOKEN_CONTENT_T:
                eState = INETMSG_MIME_CHECK;
                eOkState = INETMSG_MIME_OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'r':
                        check = "ansfer-encoding";
                        nIdx = INETMSG_MIME_CONTENT_TRANSFER_ENCODING;
                        break;

                    case 'y':
                        check = "pe";
                        nIdx = INETMSG_MIME_CONTENT_TYPE;
                        break;

                    default:
                        eState = INETMSG_MIME_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_MIME_CHECK:
                if (*check)
                {
                    while (*pData && *check &&
                           (ascii_toLowerCase (*pData) == *check))
                    {
                        pData++;
                        check++;
                    }
                }
                else
                {
                    check = pData;
                }
                eState = (*check == '\0') ? eOkState : INETMSG_MIME_JUNK;
                break;

            case INETMSG_MIME_OK:
                pData = pStop;
                SetHeaderField_Impl (
                    INetMessageHeader (MIMEHDR(nIdx), rHeader.GetValue()),
                    m_nIndex[nIdx]);
                nNewIndex = m_nIndex[nIdx];
                break;

            default: // INETMSG_MIME_JUNK
                pData = pStop;
                nNewIndex = INetRFC822Message::SetHeaderField (
                    rHeader, nNewIndex);
                break;
        }
    }
    return nNewIndex;
}

void INetMIMEMessage::SetMIMEVersion (const OUString& rVersion)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_VERSION), rVersion,
        m_nIndex[INETMSG_MIME_VERSION]);
}

void INetMIMEMessage::SetContentDisposition (const OUString& rDisposition)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_DISPOSITION), rDisposition,
        m_nIndex[INETMSG_MIME_CONTENT_DISPOSITION]);
}

void INetMIMEMessage::SetContentType (const OUString& rType)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_TYPE), rType,
        m_nIndex[INETMSG_MIME_CONTENT_TYPE]);
}

void INetMIMEMessage::SetContentTransferEncoding (
    const OUString& rEncoding)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_TRANSFER_ENCODING), rEncoding,
        m_nIndex[INETMSG_MIME_CONTENT_TRANSFER_ENCODING]);
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

bool INetMIMEMessage::EnableAttachChild (INetMessageContainerType eType)
{
    // Check context.
    if (IsContainer())
        return false;

    // Setup Content-Type header field.
    OStringBuffer aContentType;
    switch (eType)
    {
        case INETMSG_MESSAGE_RFC822:
            aContentType.append("message/rfc822");
            break;

        case INETMSG_MULTIPART_ALTERNATIVE:
            aContentType.append("multipart/alternative");
            break;

        case INETMSG_MULTIPART_DIGEST:
            aContentType.append("multipart/digest");
            break;

        case INETMSG_MULTIPART_PARALLEL:
            aContentType.append("multipart/parallel");
            break;

        case INETMSG_MULTIPART_RELATED:
            aContentType.append("multipart/related");
            break;

        case INETMSG_MULTIPART_FORM_DATA:
            aContentType.append("multipart/form-data");
            break;

        default:
            aContentType.append("multipart/mixed");
            break;
    }

    // Setup boundary for multipart types.
    if (aContentType.toString().equalsIgnoreAsciiCase("multipart/"))
    {
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

        // Append boundary as ContentType parameter.
        aContentType.append("; boundary=");
        aContentType.append(m_aBoundary);
    }

    // Set header fields.
    SetMIMEVersion(OUString("1.0"));
    SetContentType(OStringToOUString(aContentType.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US));
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

SvStream& INetMIMEMessage::operator<< (SvStream& rStrm) const
{
    INetRFC822Message::operator<< (rStrm);

    for (sal_uInt16 i = 0; i < INETMSG_MIME_NUMHDR; i++)
        rStrm.WriteUInt32( m_nIndex[i] );

    write_uInt16_lenPrefixed_uInt8s_FromOString(rStrm, m_aBoundary);
    rStrm.WriteUInt32( aChildren.size() );

    return rStrm;
}

SvStream& INetMIMEMessage::operator>> (SvStream& rStrm)
{
    INetRFC822Message::operator>> (rStrm);

    sal_uInt32 nTemp;
    for (sal_uInt16 i = 0; i < INETMSG_MIME_NUMHDR; i++)
    {
        rStrm.ReadUInt32( nTemp );
        m_nIndex[i] = nTemp;
    }

    m_aBoundary = read_uInt16_lenPrefixed_uInt8s_ToOString(rStrm);

    rStrm.ReadUInt32( nTemp );

    return rStrm;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
