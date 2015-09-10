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

inline sal_Unicode ascii_toLowerCase( sal_Unicode ch )
{
    if ( (ch >= 0x0041) && (ch <= 0x005A) )
        return ch + 0x20;
    else
        return ch;
}

void INetMIMEMessage::ListCleanup_Impl()
{
    // Cleanup.
    sal_uIntPtr i, n = m_aHeaderList.size();
    for (i = 0; i < n; i++)
        delete m_aHeaderList[ i ];
    m_aHeaderList.clear();
}

void INetMIMEMessage::ListCopy (const INetMIMEMessage &rMsg)
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

void INetMIMEMessage::SetHeaderField_Impl (
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

static const std::map<InetMessageField, const char *> ImplINetRFC822MessageHeaderData =
{
    { InetMessageField::BCC, "BCC" } ,
    { InetMessageField::CC, "CC" } ,
    { InetMessageField::COMMENTS, "Comments" } ,
    { InetMessageField::DATE, "Date" } ,
    { InetMessageField::FROM, "From" } ,
    { InetMessageField::IN_REPLY_TO, "In-Reply-To" } ,
    { InetMessageField::KEYWORDS, "Keywords" } ,
    { InetMessageField::MESSAGE_ID, "Message-ID" } ,
    { InetMessageField::REFERENCES, "References" } ,
    { InetMessageField::REPLY_TO, "Reply-To" } ,
    { InetMessageField::RETURN_PATH, "Return-Path" } ,
    { InetMessageField::SUBJECT, "Subject" } ,
    { InetMessageField::SENDER, "Sender" } ,
    { InetMessageField::TO, "To" } ,
    { InetMessageField::X_MAILER, "X-Mailer" } ,
    { InetMessageField::RETURN_RECEIPT_TO, "Return-Receipt-To" } ,
};

/*
    State of RFC822 header parsing
*/
enum class HeaderState
{
    BEGIN,
    CHECK,
    OK,
    JUNK,

    TOKEN_RE,
    TOKEN_RETURNMINUS,
    TOKEN_XMINUS,
    LETTER_C,
    LETTER_S
};

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

// Header Field Parser
sal_uIntPtr INetMIMEMessage::SetRFC822HeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex)
{
    OString aName (rHeader.GetName());
    const sal_Char *pData = aName.getStr();
    const sal_Char *pStop = pData + aName.getLength() + 1;
    const sal_Char *check = "";

    InetMessageField nIdx   = static_cast<InetMessageField>(CONTAINER_APPEND);
    HeaderState      eState = HeaderState::BEGIN;
    HeaderState      eOkState = HeaderState::OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case HeaderState::BEGIN:
                eState = HeaderState::CHECK;
                eOkState = HeaderState::OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'b':
                        check = "cc";
                        nIdx = InetMessageField::BCC;
                        break;

                    case 'c':
                        eState = HeaderState::LETTER_C;
                        break;

                    case 'd':
                        check = "ate";
                        nIdx = InetMessageField::DATE;
                        break;

                    case 'f':
                        check = "rom";
                        nIdx = InetMessageField::FROM;
                        break;

                    case 'i':
                        check = "n-reply-to";
                        nIdx = InetMessageField::IN_REPLY_TO;
                        break;

                    case 'k':
                        check = "eywords";
                        nIdx = InetMessageField::KEYWORDS;
                        break;

                    case 'm':
                        check = "essage-id";
                        nIdx = InetMessageField::MESSAGE_ID;
                        break;

                    case 'r':
                        check = "e";
                        eOkState = HeaderState::TOKEN_RE;
                        break;

                    case 's':
                        eState = HeaderState::LETTER_S;
                        break;

                    case 't':
                        check = "o";
                        nIdx = InetMessageField::TO;
                        break;

                    case 'x':
                        check = "-";
                        eOkState = HeaderState::TOKEN_XMINUS;
                        break;

                    default:
                        eState = HeaderState::JUNK;
                        break;
                }
                pData++;
                break;

            case HeaderState::TOKEN_RE:
                eState = HeaderState::CHECK;
                eOkState = HeaderState::OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'f':
                        check = "erences";
                        nIdx = InetMessageField::REFERENCES;
                        break;

                    case 'p':
                        check = "ly-to";
                        nIdx = InetMessageField::REPLY_TO;
                        break;

                    case 't':
                        check = "urn-";
                        eOkState = HeaderState::TOKEN_RETURNMINUS;
                        break;

                    default:
                        eState = HeaderState::JUNK;
                        break;
                }
                pData++;
                break;

            case HeaderState::TOKEN_RETURNMINUS:
                eState = HeaderState::CHECK;
                eOkState = HeaderState::OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'p':
                        check = "ath";
                        nIdx = InetMessageField::RETURN_PATH;
                        break;

                    case 'r':
                        check = "eceipt-to";
                        nIdx = InetMessageField::RETURN_RECEIPT_TO;
                        break;

                    default:
                        eState = HeaderState::JUNK;
                        break;
                }
                pData++;
                break;

            case HeaderState::TOKEN_XMINUS:
                eState = HeaderState::CHECK;
                eOkState = HeaderState::OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'm':
                        check = "ailer";
                        nIdx = InetMessageField::X_MAILER;
                        break;

                    default:
                        eState = HeaderState::JUNK;
                        break;
                }
                pData++;
                break;

            case HeaderState::LETTER_C:
                eState = HeaderState::CHECK;
                eOkState = HeaderState::OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'c':
                        check = "";
                        nIdx = InetMessageField::CC;
                        break;

                    case 'o':
                        check = "mments";
                        nIdx = InetMessageField::COMMENTS;
                        break;

                    default:
                        eState = HeaderState::JUNK;
                        break;
                }
                pData++;
                break;

            case HeaderState::LETTER_S:
                eState = HeaderState::CHECK;
                eOkState = HeaderState::OK;

                switch (ascii_toLowerCase (*pData))
                {
                    case 'e':
                        check = "nder";
                        nIdx = InetMessageField::SENDER;
                        break;

                    case 'u':
                        check = "bject";
                        nIdx = InetMessageField::SUBJECT;
                        break;

                    default:
                        eState = HeaderState::JUNK;
                        break;
                }
                pData++;
                break;

            case HeaderState::CHECK:
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
                eState = (*check == '\0') ? eOkState : HeaderState::JUNK;
                break;

            case HeaderState::OK:
                pData = pStop;
                SetHeaderField_Impl (
                    INetMessageHeader( ImplINetRFC822MessageHeaderData.at(nIdx), rHeader.GetValue() ),
                    m_nRFC822Index[nIdx]);
                nNewIndex = m_nRFC822Index[nIdx];
                break;

            default: // INETMSG_RFC822_JUNK
                pData = pStop;
                SetHeaderField_Impl(rHeader, nNewIndex);
                break;
        }
    }
    return nNewIndex;
}

static const std::map<InetMessageMime, const char*> ImplINetMIMEMessageHeaderData =
{
    { InetMessageMime::VERSION, "MIME-Version"},
    { InetMessageMime::CONTENT_DESCRIPTION, "Content-Description"},
    { InetMessageMime::CONTENT_DISPOSITION, "Content-Disposition"},
    { InetMessageMime::CONTENT_ID, "Content-ID"},
    { InetMessageMime::CONTENT_TYPE, "Content-Type"},
    { InetMessageMime::CONTENT_TRANSFER_ENCODING, "Content-Transfer-Encoding"}
};

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
    : m_nDocSize(0),
      pParent(NULL),
      bHeaderParsed(false)
{
    for (sal_uInt16 i = 0; i < static_cast<int>(InetMessageField::NUMHDR); i++)
        m_nRFC822Index[static_cast<InetMessageField>(i)] = CONTAINER_ENTRY_NOTFOUND;
    for (sal_uInt16 i = 0; i < static_cast<int>(InetMessageMime::NUMHDR); i++)
        m_nMIMEIndex[static_cast<InetMessageMime>(i)] = CONTAINER_ENTRY_NOTFOUND;
}

INetMIMEMessage::INetMIMEMessage (const INetMIMEMessage& rMsg)
    : m_nDocSize(rMsg.m_nDocSize),
      m_aDocName(rMsg.m_aDocName),
      m_xDocLB(rMsg.m_xDocLB),
      pParent(NULL)
{
    ListCopy (rMsg);
    m_nRFC822Index = rMsg.m_nRFC822Index;
    CopyImp (rMsg);
}

INetMIMEMessage& INetMIMEMessage::operator= (
    const INetMIMEMessage& rMsg)
{
    if (this != &rMsg)
    {
        m_nDocSize = rMsg.m_nDocSize;
        m_aDocName = rMsg.m_aDocName;
        m_xDocLB   = rMsg.m_xDocLB;
        ListCopy (rMsg);
        m_nRFC822Index = rMsg.m_nRFC822Index;
        CleanupImp();
        CopyImp (rMsg);
    }
    return *this;
}

INetMIMEMessage::~INetMIMEMessage()
{
    ListCleanup_Impl();
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
    m_nMIMEIndex = rMsg.m_nMIMEIndex;
    m_aBoundary = rMsg.m_aBoundary;

    for (i = 0; i < rMsg.aChildren.size(); i++)
    {
        INetMIMEMessage *pChild = rMsg.aChildren[ i ];

        if (pChild->pParent == &rMsg)
        {
            pChild = INetMIMEMessage::CreateMessage (*pChild);
            pChild->pParent = this;
        }
        aChildren.push_back( pChild );
    }
}

INetMIMEMessage *INetMIMEMessage::CreateMessage (
    const INetMIMEMessage& rMsg)
{
    return new INetMIMEMessage (rMsg);
}

// Header Field Parser
sal_uIntPtr INetMIMEMessage::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex)
{
    OString aName (rHeader.GetName());
    const sal_Char *pData = aName.getStr();
    const sal_Char *pStop = pData + aName.getLength() + 1;
    const sal_Char *check = "";

    InetMessageMime nIdx = static_cast<InetMessageMime>(CONTAINER_APPEND);
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
                        nIdx = InetMessageMime::VERSION;
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
                        nIdx = InetMessageMime::CONTENT_ID;
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
                        nIdx = InetMessageMime::CONTENT_DESCRIPTION;
                        break;

                    case 'i':
                        check = "sposition";
                        nIdx = InetMessageMime::CONTENT_DISPOSITION;
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
                        nIdx = InetMessageMime::CONTENT_TRANSFER_ENCODING;
                        break;

                    case 'y':
                        check = "pe";
                        nIdx = InetMessageMime::CONTENT_TYPE;
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
                    INetMessageHeader( ImplINetMIMEMessageHeaderData.at(nIdx), rHeader.GetValue()),
                    m_nMIMEIndex[nIdx]);
                nNewIndex = m_nMIMEIndex[nIdx];
                break;

            default: // INETMSG_MIME_JUNK
                pData = pStop;
                nNewIndex = SetRFC822HeaderField(rHeader, nNewIndex);
                break;
        }
    }
    return nNewIndex;
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
