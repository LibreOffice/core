/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <sal/types.h>
#include <tools/datetime.hxx>
#ifndef _TOOLS_INETMIME_HXX
#include <tools/inetmime.hxx>
#endif
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>
#include <rtl/instance.hxx>

#include <stdio.h>

//=======================================================================

inline sal_Bool ascii_isDigit( sal_Unicode ch )
{
    return ((ch >= 0x0030) && (ch <= 0x0039));
}

inline sal_Bool ascii_isLetter( sal_Unicode ch )
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

/*=======================================================================
 *
 * INetMessage Implementation.
 *
 *=====================================================================*/
#define CONSTASCII_STRINGPARAM(a) (a), RTL_TEXTENCODING_ASCII_US
#define HEADERFIELD INetMessageHeader

/*
 * ~INetMessage.
 */
INetMessage::~INetMessage (void)
{
    ListCleanup_Impl();
}

/*
 * ListCleanup_Impl.
 */
void INetMessage::ListCleanup_Impl (void)
{
    // Cleanup.
    sal_uIntPtr i, n = m_aHeaderList.Count();
    for (i = 0; i < n; i++)
        delete ((HEADERFIELD*)(m_aHeaderList.GetObject(i)));
    m_aHeaderList.Clear();
}

/*
 * ListCopy.
 */
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
            HEADERFIELD *p = (HEADERFIELD*)(rMsg.m_aHeaderList.GetObject(i));
            m_aHeaderList.Insert (new HEADERFIELD(*p), LIST_APPEND);
        }
    }
}

/*
 * SetHeaderField_Impl.
 */
void INetMessage::SetHeaderField_Impl (
    INetMIME::HeaderFieldType  eType,
    const ByteString          &rName,
    const UniString           &rValue,
    sal_uIntPtr                     &rnIndex)
{
    INetMIMEStringOutputSink aSink (0, STRING_MAXLEN);
    INetMIME::writeHeaderFieldBody (
        aSink, eType, rValue, gsl_getSystemTextEncoding(), false);
    SetHeaderField_Impl (
        INetMessageHeader (rName, aSink.takeBuffer()), rnIndex);
}

/*
 * SetHeaderField.
 */
sal_uIntPtr INetMessage::SetHeaderField (
    const UniString& rName, const UniString& rValue, sal_uIntPtr nIndex)
{
    sal_uIntPtr nResult = nIndex;
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        ByteString (rName, RTL_TEXTENCODING_ASCII_US), rValue,
        nResult);
    return nResult;
}

/*
 * SetHeaderField.
 */
sal_uIntPtr INetMessage::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nIndex)
{
    sal_uIntPtr nResult = nIndex;
    SetHeaderField_Impl (rHeader, nResult);
    return nResult;
}


/*
 * operator<<
 */
SvStream& INetMessage::operator<< (SvStream& rStrm) const
{
    rStrm << static_cast<sal_uInt32>(m_nDocSize);
    rStrm.WriteByteString (m_aDocName, RTL_TEXTENCODING_UTF8);

    sal_uIntPtr i, n = m_aHeaderList.Count();
    rStrm << static_cast<sal_uInt32>(n);

    for (i = 0; i < n; i++)
        rStrm << *((HEADERFIELD *)(m_aHeaderList.GetObject(i)));

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetMessage::operator>> (SvStream& rStrm)
{
    // Cleanup.
    m_nDocSize = 0;
    m_xDocLB.Clear();
    ListCleanup_Impl();

    sal_uInt32 nTemp;

    // Copy.
    rStrm >> nTemp;
    m_nDocSize = nTemp;
    rStrm.ReadByteString (m_aDocName, RTL_TEXTENCODING_UTF8);

    sal_uIntPtr i, n = 0;
    rStrm >> nTemp;
    n = nTemp;

    for (i = 0; i < n; i++)
    {
        HEADERFIELD *p = new HEADERFIELD();
        rStrm >> *p;
        m_aHeaderList.Insert (p, LIST_APPEND);
    }

    // Done.
    return rStrm;
}

/*=======================================================================
 *
 * INetMessageHeaderIterator Implementation.
 *
 *=====================================================================*/
INetMessageHeaderIterator::INetMessageHeaderIterator (
    const INetMessage& rMsg, const UniString& rHdrName)
{
    sal_uIntPtr i, n = rMsg.GetHeaderCount();
    for (i = 0; i < n; i++)
    {
        if (rHdrName.CompareIgnoreCaseToAscii (rMsg.GetHeaderName(i)) == 0)
        {
            UniString *pValue = new UniString (rMsg.GetHeaderValue(i));
            aValueList.Insert (pValue, LIST_APPEND);
        }
    }
    nValueCount = aValueList.Count();
}

INetMessageHeaderIterator::~INetMessageHeaderIterator (void)
{
    sal_uIntPtr i, n = aValueList.Count();
    for (i = 0; i < n; i++)
        delete ((UniString*)(aValueList.GetObject(i)));
    aValueList.Clear();
}

/*=======================================================================
 *
 * INetRFC822Message Implementation.
 *
 *=====================================================================*/
/*
 * ImplINetRFC822MessageHeaderData.
 */
namespace
{
    struct ImplINetRFC822MessageHeaderDataImpl
    {
        const ByteString* operator()()
        {
            static const ByteString _ImplINetRFC822MessageHeaderData[] =
            {
                ByteString ("BCC"),
                ByteString ("CC"),
                ByteString ("Comments"),
                ByteString ("Date"),
                ByteString ("From"),
                ByteString ("In-Reply-To"),
                ByteString ("Keywords"),
                ByteString ("Message-ID"),
                ByteString ("References"),
                ByteString ("Reply-To"),
                ByteString ("Return-Path"),
                ByteString ("Subject"),
                ByteString ("Sender"),
                ByteString ("To"),
                ByteString ("X-Mailer"),
                ByteString ("Return-Receipt-To")
            };
            return &_ImplINetRFC822MessageHeaderData[0];
        }
    };

    struct ImplINetRFC822MessageHeaderData
        : public rtl::StaticAggregate< const ByteString, ImplINetRFC822MessageHeaderDataImpl > {};
}

#define HDR(n) ImplINetRFC822MessageHeaderData::get()[(n)]

/*
 * _ImplINetRFC822MessageHeaderState.
 */
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

/*
 * INetRFC822Message.
 */
INetRFC822Message::INetRFC822Message (void)
    : INetMessage()
{
    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        m_nIndex[i] = LIST_ENTRY_NOTFOUND;
}

INetRFC822Message::INetRFC822Message (const INetRFC822Message& rMsg)
    : INetMessage (rMsg)
{
    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        m_nIndex[i] = rMsg.m_nIndex[i];
}

/*
 * operator=
 */
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

/*
 * ~INetRFC822Message.
 */
INetRFC822Message::~INetRFC822Message (void)
{
}

/*
 * <Generate|Parse>DateField and local helper functions.
 *
 * GenerateDateField.
 * Generates a String from Date and Time objects in format:
 *   Wkd, 00 Mon 0000 00:00:00 [GMT]            (rfc822, rfc1123)
 *
 * ParseDateField.
 * Parses a String in (implied) GMT format into class Date and Time objects.
 * Four formats are accepted:
 *
 *  [Wkd,] 1*2DIGIT Mon 2*4DIGIT 00:00:00 [GMT]  (rfc1123)
 *  [Wkd,] 00 Mon 0000 00:00:00 [GMT])           (rfc822, rfc1123)
 *   Weekday, 00-Mon-00 00:00:00 [GMT]           (rfc850, rfc1036)
 *   Wkd Mon 00 00:00:00 0000 [GMT]              (ctime)
 *   1*DIGIT                                     (delta seconds)
 *
 */

// Months and Weekdays.
static const sal_Char *months[12] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const sal_Char *wkdays[7] =
{
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

/*
 * GenerateDateField.
 */
sal_Bool INetRFC822Message::GenerateDateField (
    const DateTime& rDateTime, UniString& rDateFieldW)
{
    // Check arguments.
    if (!rDateTime.IsValid()       ||
        (rDateTime.GetSec()  > 59) ||
        (rDateTime.GetMin()  > 59) ||
        (rDateTime.GetHour() > 23)    ) return sal_False;

    // Prepare output string.
    ByteString rDateField;

    // Insert Date.
    rDateField += wkdays[(sal_uInt16)(rDateTime.GetDayOfWeek())];
    rDateField += ", ";

    sal_uInt16 nNum = rDateTime.GetDay();
    if (nNum < 10) rDateField += '0';
    rDateField += ByteString::CreateFromInt32(nNum);
    rDateField += ' ';

    rDateField += months[(sal_uInt16)(rDateTime.GetMonth() - 1)];
    rDateField += ' ';

    rDateField += ByteString::CreateFromInt32(rDateTime.GetYear());
    rDateField += ' ';

    // Insert Time.
    nNum = rDateTime.GetHour();
    if (nNum < 10) rDateField += '0';
    rDateField += ByteString::CreateFromInt32(nNum);
    rDateField += ':';

    nNum = rDateTime.GetMin();
    if (nNum < 10) rDateField += '0';
    rDateField += ByteString::CreateFromInt32(nNum);
    rDateField += ':';

    nNum = rDateTime.GetSec();
    if (nNum < 10) rDateField += '0';
    rDateField += ByteString::CreateFromInt32(nNum);
    rDateField += " GMT";

    // Done.
    rDateFieldW = UniString (rDateField, RTL_TEXTENCODING_ASCII_US);
    return sal_True;
}

/*
 * ParseDateField and local helper functions.
 */
static sal_uInt16 ParseNumber (const ByteString& rStr, sal_uInt16& nIndex)
{
    sal_uInt16 n = nIndex;
    while ((n < rStr.Len()) && ascii_isDigit(rStr.GetChar(n))) n++;

    ByteString aNum (rStr.Copy (nIndex, (n - nIndex)));
    nIndex = n;

    return (sal_uInt16)(aNum.ToInt32());
}

static sal_uInt16 ParseMonth (const ByteString& rStr, sal_uInt16& nIndex)
{
    sal_uInt16 n = nIndex;
    while ((n < rStr.Len()) && ascii_isLetter(rStr.GetChar(n))) n++;

    ByteString aMonth (rStr.Copy (nIndex, 3));
    nIndex = n;

    sal_uInt16 i;
    for (i = 0; i < 12; i++)
        if (aMonth.CompareIgnoreCaseToAscii (months[i]) == 0) break;
    return (i + 1);
}

sal_Bool INetRFC822Message::ParseDateField (
    const UniString& rDateFieldW, DateTime& rDateTime)
{
    ByteString rDateField (rDateFieldW, RTL_TEXTENCODING_ASCII_US);
    if (rDateField.Len() == 0) return sal_False;

    if (rDateField.Search (':') != STRING_NOTFOUND)
    {
        // Some DateTime format.
        sal_uInt16 nIndex = 0;

        // Skip over <Wkd> or <Weekday>, leading and trailing space.
        while ((nIndex < rDateField.Len()) &&
               (rDateField.GetChar(nIndex) == ' '))
            nIndex++;

        while (
            (nIndex < rDateField.Len()) &&
            (ascii_isLetter (rDateField.GetChar(nIndex)) ||
             (rDateField.GetChar(nIndex) == ',')     ))
            nIndex++;

        while ((nIndex < rDateField.Len()) &&
               (rDateField.GetChar(nIndex) == ' '))
            nIndex++;

        if (ascii_isLetter (rDateField.GetChar(nIndex)))
        {
            // Format: ctime().
            if ((rDateField.Len() - nIndex) < 20) return sal_False;

            rDateTime.SetMonth  (ParseMonth  (rDateField, nIndex)); nIndex++;
            rDateTime.SetDay    (ParseNumber (rDateField, nIndex)); nIndex++;

            rDateTime.SetHour   (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetMin    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetSec    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.Set100Sec (0);

            sal_uInt16 nYear = ParseNumber (rDateField, nIndex);
            if (nYear < 100) nYear += 1900;
            rDateTime.SetYear   (nYear);
        }
        else
        {
            // Format: RFC1036 or RFC1123.
            if ((rDateField.Len() - nIndex) < 17) return sal_False;

            rDateTime.SetDay    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetMonth  (ParseMonth  (rDateField, nIndex)); nIndex++;

            sal_uInt16 nYear  = ParseNumber (rDateField, nIndex);  nIndex++;
            if (nYear < 100) nYear += 1900;
            rDateTime.SetYear   (nYear);

            rDateTime.SetHour   (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetMin    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetSec    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.Set100Sec (0);

            if ((rDateField.GetChar(nIndex) == '+') ||
                (rDateField.GetChar(nIndex) == '-')    )
            {
                // Offset from GMT: "(+|-)HHMM".
                sal_Bool   bEast   = (rDateField.GetChar(nIndex++) == '+');
                sal_uInt16 nOffset = ParseNumber (rDateField, nIndex);
                if (nOffset > 0)
                {
                    Time aDiff;
                    aDiff.SetHour   (nOffset / 100);
                    aDiff.SetMin    (nOffset % 100);
                    aDiff.SetSec    (0);
                    aDiff.Set100Sec (0);

                    if (bEast)
                        rDateTime -= aDiff;
                    else
                        rDateTime += aDiff;
                }
            }
        }
    }
    else if (rDateField.IsNumericAscii())
    {
        // Format: delta seconds.
        Time aDelta (0);
        aDelta.SetTime (rDateField.ToInt32() * 100);

        DateTime aNow;
        aNow += aDelta;
        aNow.ConvertToUTC();

        rDateTime.SetDate (aNow.GetDate());
        rDateTime.SetTime (aNow.GetTime());
    }
    else
    {
        // Junk.
        return sal_False;
    }

    return (rDateTime.IsValid() &&
            !((rDateTime.GetSec()  > 59) ||
              (rDateTime.GetMin()  > 59) ||
              (rDateTime.GetHour() > 23)    ));
}

/*
 * SetHeaderField.
 * (Header Field Parser).
 */
sal_uIntPtr INetRFC822Message::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex)
{
    ByteString aName (rHeader.GetName());
    const sal_Char *pData = aName.GetBuffer();
    const sal_Char *pStop = pData + aName.Len() + 1;
    const sal_Char *check = "";

    sal_uIntPtr       nIdx     = LIST_APPEND;
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

#if 0  /* NYI */
                    case 'p':
                        check = "riority";
                        eOkState = INETMSG_RFC822_X_PRIORITY;
                        break;
#endif /* NYI */

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
                    HEADERFIELD (HDR(nIdx), rHeader.GetValue()),
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

/*
 * Specific Set-Methods.
 */
void INetRFC822Message::SetBCC (const UniString& rBCC)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_BCC), rBCC,
        m_nIndex[INETMSG_RFC822_BCC]);
}

void INetRFC822Message::SetCC (const UniString& rCC)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_CC), rCC,
        m_nIndex[INETMSG_RFC822_CC]);
}

void INetRFC822Message::SetComments (const UniString& rComments)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HDR(INETMSG_RFC822_COMMENTS), rComments,
        m_nIndex[INETMSG_RFC822_COMMENTS]);
}

void INetRFC822Message::SetDate (const UniString& rDate)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_STRUCTURED,
        HDR(INETMSG_RFC822_DATE), rDate,
        m_nIndex[INETMSG_RFC822_DATE]);
}

void INetRFC822Message::SetFrom (const UniString& rFrom)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_FROM), rFrom,
        m_nIndex[INETMSG_RFC822_FROM]);
}

void INetRFC822Message::SetInReplyTo (const UniString& rInReplyTo)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS, // ??? MESSAGE_ID ???
        HDR(INETMSG_RFC822_IN_REPLY_TO), rInReplyTo,
        m_nIndex[INETMSG_RFC822_IN_REPLY_TO]);
}

void INetRFC822Message::SetKeywords (const UniString& rKeywords)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_PHRASE,
        HDR(INETMSG_RFC822_KEYWORDS), rKeywords,
        m_nIndex[INETMSG_RFC822_KEYWORDS]);
}

void INetRFC822Message::SetMessageID (const UniString& rMessageID)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_MESSAGE_ID,
        HDR(INETMSG_RFC822_MESSAGE_ID), rMessageID,
        m_nIndex[INETMSG_RFC822_MESSAGE_ID]);
}

void INetRFC822Message::SetReferences (const UniString& rReferences)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_MESSAGE_ID,
        HDR(INETMSG_RFC822_REFERENCES), rReferences,
        m_nIndex[INETMSG_RFC822_REFERENCES]);
}

void INetRFC822Message::SetReplyTo (const UniString& rReplyTo)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_REPLY_TO), rReplyTo,
        m_nIndex[INETMSG_RFC822_REPLY_TO]);
}

void INetRFC822Message::SetReturnPath (const UniString& rReturnPath)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_RETURN_PATH), rReturnPath,
        m_nIndex[INETMSG_RFC822_RETURN_PATH]);
}

void INetRFC822Message::SetReturnReceiptTo (const UniString& rValue)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_RETURN_RECEIPT_TO), rValue,
        m_nIndex[INETMSG_RFC822_RETURN_RECEIPT_TO]);
}

void INetRFC822Message::SetSender (const UniString& rSender)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_SENDER), rSender,
        m_nIndex[INETMSG_RFC822_SENDER]);
}

void INetRFC822Message::SetSubject (const UniString& rSubject)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HDR(INETMSG_RFC822_SUBJECT), rSubject,
        m_nIndex[INETMSG_RFC822_SUBJECT]);
}

void INetRFC822Message::SetTo (const UniString& rTo)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_ADDRESS,
        HDR(INETMSG_RFC822_TO), rTo,
        m_nIndex[INETMSG_RFC822_TO]);
}

void INetRFC822Message::SetXMailer (const UniString& rXMailer)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HDR(INETMSG_RFC822_X_MAILER), rXMailer,
        m_nIndex[INETMSG_RFC822_X_MAILER]);
}

/*
 * operator<<
 */
SvStream& INetRFC822Message::operator<< (SvStream& rStrm) const
{
    INetMessage::operator<< (rStrm);

    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        rStrm << static_cast<sal_uInt32>(m_nIndex[i]);

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetRFC822Message::operator>> (SvStream& rStrm)
{
    INetMessage::operator>> (rStrm);

    sal_uInt32 nTemp;
    for (sal_uInt16 i = 0; i < INETMSG_RFC822_NUMHDR; i++)
    {
        rStrm >> nTemp;
        m_nIndex[i] = nTemp;
    }

    return rStrm;
}

/*=======================================================================
 *
 * INetMIMEMessage Implementation.
 *
 *=====================================================================*/
/*
 * _ImplINetMIMEMessageHeaderData.
 */
namespace
{
    struct ImplINetMIMEMessageHeaderDataImpl
    {
        const ByteString* operator()()
        {
            static const ByteString _ImplINetMIMEMessageHeaderData[] =
            {
                ByteString ("MIME-Version"),
                ByteString ("Content-Description"),
                ByteString ("Content-Disposition"),
                ByteString ("Content-ID"),
                ByteString ("Content-Type"),
                ByteString ("Content-Transfer-Encoding")
            };
            return &_ImplINetMIMEMessageHeaderData[0];
        }
    };

    struct ImplINetMIMEMessageHeaderData
        : public rtl::StaticAggregate< const ByteString, ImplINetMIMEMessageHeaderDataImpl > {};
}

#define MIMEHDR(n) ImplINetMIMEMessageHeaderData::get()[(n)]

/*
 * _ImplINetMIMEMessageHeaderState.
 */
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

/*
 * INetMIMEMessage.
 */
INetMIMEMessage::INetMIMEMessage (void)
    : INetRFC822Message (),
      pParent       (NULL),
      nNumChildren  (0),
      bHeaderParsed (sal_False)
{
    for (sal_uInt16 i = 0; i < INETMSG_MIME_NUMHDR; i++)
        m_nIndex[i] = LIST_ENTRY_NOTFOUND;
}

INetMIMEMessage::INetMIMEMessage (const INetMIMEMessage& rMsg)
    : INetRFC822Message (rMsg)
{
    // Copy.
    CopyImp (rMsg);
}

/*
 * operator=
 */
INetMIMEMessage& INetMIMEMessage::operator= (
    const INetMIMEMessage& rMsg)
{
    if (this != &rMsg)
    {
        // Assign base.
        INetRFC822Message::operator= (rMsg);

        // Cleanup.
        CleanupImp();

        // Copy.
        CopyImp (rMsg);
    }
    return *this;
}

/*
 * ~INetMIMEMessage.
 */
INetMIMEMessage::~INetMIMEMessage (void)
{
    // Cleanup.
    CleanupImp();
}

/*
 * CleanupImp.
 */
void INetMIMEMessage::CleanupImp (void)
{
    INetMIMEMessage *pChild = NULL;
    while ((pChild = (INetMIMEMessage *)(aChildren.Remove())) != NULL)
        if (pChild->pParent == this) delete pChild;
}

/*
 * CopyImp.
 */
void INetMIMEMessage::CopyImp (const INetMIMEMessage& rMsg)
{
    bHeaderParsed = rMsg.bHeaderParsed;

    sal_uInt16 i;
    for (i = 0; i < INETMSG_MIME_NUMHDR; i++)
        m_nIndex[i] = rMsg.m_nIndex[i];

    m_aBoundary = rMsg.m_aBoundary;
    nNumChildren = rMsg.nNumChildren;

    for (i = 0; i < rMsg.aChildren.Count(); i++)
    {
        INetMIMEMessage *pChild =
            (INetMIMEMessage *)(rMsg.aChildren.GetObject (i));

        if (pChild->pParent == &rMsg)
        {
            pChild = pChild->CreateMessage (*pChild);
            pChild->pParent = this;
        }
        aChildren.Insert (pChild, LIST_APPEND);
    }
}

/*
 * CreateMessage.
 */
INetMIMEMessage *INetMIMEMessage::CreateMessage (
    const INetMIMEMessage& rMsg) const
{
    return (new INetMIMEMessage (rMsg));
}

/*
 * SetHeaderField.
 * (Header Field Parser).
 */
sal_uIntPtr INetMIMEMessage::SetHeaderField (
    const INetMessageHeader &rHeader, sal_uIntPtr nNewIndex)
{
    ByteString aName (rHeader.GetName());
    const sal_Char *pData = aName.GetBuffer();
    const sal_Char *pStop = pData + aName.Len() + 1;
    const sal_Char *check = "";

    sal_uIntPtr      nIdx     = LIST_APPEND;
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
                    HEADERFIELD (MIMEHDR(nIdx), rHeader.GetValue()),
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

/*
 * Specific Set-Methods.
 */
void INetMIMEMessage::SetMIMEVersion (const UniString& rVersion)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_VERSION), rVersion,
        m_nIndex[INETMSG_MIME_VERSION]);
}

void INetMIMEMessage::SetContentDescription (const String& rDescription)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_DESCRIPTION), rDescription,
        m_nIndex[INETMSG_MIME_CONTENT_DESCRIPTION]);
}

void INetMIMEMessage::SetContentDisposition (const String& rDisposition)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_DISPOSITION), rDisposition,
        m_nIndex[INETMSG_MIME_CONTENT_DISPOSITION]);
}

void INetMIMEMessage::SetContentID (const String& rID)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_ID), rID,
        m_nIndex[INETMSG_MIME_CONTENT_ID]);
}

void INetMIMEMessage::SetContentType (const String& rType)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_TYPE), rType,
        m_nIndex[INETMSG_MIME_CONTENT_TYPE]);
}

void INetMIMEMessage::SetContentTransferEncoding (
    const String& rEncoding)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        MIMEHDR(INETMSG_MIME_CONTENT_TRANSFER_ENCODING), rEncoding,
        m_nIndex[INETMSG_MIME_CONTENT_TRANSFER_ENCODING]);
}

/*
 * GetDefaultContentType.
 */
void INetMIMEMessage::GetDefaultContentType (String& rContentType)
{
    String aDefaultCT (
        "text/plain; charset=us-ascii", RTL_TEXTENCODING_ASCII_US);
    if (pParent == NULL)
    {
        rContentType = aDefaultCT;
    }
    else
    {
        String aParentCT (pParent->GetContentType());
        if (aParentCT.Len() == 0)
            pParent->GetDefaultContentType (aParentCT);

        if (aParentCT.CompareIgnoreCaseToAscii ("message/", 8) == 0)
        {
            rContentType = aDefaultCT;
        }
        else if (aParentCT.CompareIgnoreCaseToAscii ("multipart/", 10) == 0)
        {
            if (aParentCT.CompareIgnoreCaseToAscii ("multipart/digest") == 0)
                rContentType.AssignAscii ("message/rfc822");
            else
                rContentType = aDefaultCT;
        }
        else
        {
            rContentType = aDefaultCT;
        }
    }
}

/*
 * EnableAttachChild.
 */
sal_Bool INetMIMEMessage::EnableAttachChild (INetMessageContainerType eType)
{
    // Check context.
    if (IsContainer())
        return sal_False;

    // Setup Content-Type header field.
    ByteString aContentType;
    switch (eType)
    {
        case INETMSG_MESSAGE_RFC822:
            aContentType = "message/rfc822";
            break;

        case INETMSG_MULTIPART_ALTERNATIVE:
            aContentType = "multipart/alternative";
            break;

        case INETMSG_MULTIPART_DIGEST:
            aContentType = "multipart/digest";
            break;

        case INETMSG_MULTIPART_PARALLEL:
            aContentType = "multipart/parallel";
            break;

        case INETMSG_MULTIPART_RELATED:
            aContentType = "multipart/related";
            break;

        case INETMSG_MULTIPART_FORM_DATA:
            aContentType = "multipart/form-data";
            break;

        default:
            aContentType = "multipart/mixed";
            break;
    }

    // Setup boundary for multipart types.
    if (aContentType.CompareIgnoreCaseToAscii ("multipart/", 10) == 0)
    {
        // Generate a unique boundary from current time.
        sal_Char sTail[16 + 1];
        Time aCurTime;
        sal_uInt64 nThis = reinterpret_cast< sal_uIntPtr >( this ); // we can be on a 64bit architecture
        nThis = ( ( nThis >> 32 ) ^ nThis ) & SAL_MAX_UINT32;
        sprintf (sTail, "%08X%08X",
                 static_cast< unsigned int >(aCurTime.GetTime()),
                 static_cast< unsigned int >(nThis));
        m_aBoundary = "------------_4D48";
        m_aBoundary += sTail;

        // Append boundary as ContentType parameter.
        aContentType += "; boundary=";
        aContentType += m_aBoundary;
    }

    // Set header fields.
    SetMIMEVersion (String (CONSTASCII_STRINGPARAM("1.0")));
    SetContentType (String (aContentType, RTL_TEXTENCODING_ASCII_US));
    SetContentTransferEncoding (String (CONSTASCII_STRINGPARAM("7bit")));

    // Done.
    return sal_True;
}

/*
 * AttachChild.
 */
sal_Bool INetMIMEMessage::AttachChild (
    INetMIMEMessage& rChildMsg, sal_Bool bOwner)
{
    if (IsContainer() /*&& rChildMsg.GetContentType().Len() */)
    {
        if (bOwner) rChildMsg.pParent = this;
        aChildren.Insert (&rChildMsg, LIST_APPEND);
        nNumChildren = aChildren.Count();

        return sal_True;
    }
    return sal_False;
}

/*
 * DetachChild.
 */
sal_Bool INetMIMEMessage::DetachChild (
    sal_uIntPtr nIndex, INetMIMEMessage& rChildMsg) const
{
    if (IsContainer())
    {
        // Check document stream.
        if (GetDocumentLB() == NULL) return sal_False;
        SvStream *pDocStrm = new SvStream (GetDocumentLB());

        // Initialize message buffer.
        char pMsgBuffer[1024];
        char *pMsgRead, *pMsgWrite;
        pMsgRead = pMsgWrite = pMsgBuffer;

        // Initialize message parser stream.
        INetMIMEMessageStream *pMsgStrm = NULL;

        // Check for "multipart/uvw" or "message/xyz".
        if (IsMultipart())
        {
            // Multipart message body. Initialize multipart delimiters.
            ByteString aDelim ("--");
            aDelim += GetMultipartBoundary();
            ByteString aClose = aDelim;
            aClose += "--";

            // Initialize line buffer.
            SvMemoryStream aLineBuf;

            // Initialize control variables.
            INetMessageStreamState eState = INETMSG_EOL_SCR;
            int nCurIndex = -1;

            // Go!
            while (nCurIndex < (int)(nIndex + 1))
            {
                if ((pMsgRead - pMsgWrite) > 0)
                {
                    // Bytes still in buffer.
                    if (eState == INETMSG_EOL_FCR)
                    {
                        // Check for 2nd line break character.
                        if ((*pMsgWrite == '\r') || (*pMsgWrite == '\n'))
                            aLineBuf << *pMsgWrite++;

                        // Check current index.
                        if (nCurIndex == (int)nIndex)
                        {
                            // Found requested part.
                            if (pMsgStrm == NULL)
                            {
                                // Create message parser stream.
                                pMsgStrm = new INetMIMEMessageStream;
                                pMsgStrm->SetTargetMessage (&rChildMsg);
                            }

                            // Put message down-stream.
                            int status = pMsgStrm->Write (
                                (const sal_Char *) aLineBuf.GetData(), aLineBuf.Tell());
                            if (status != INETSTREAM_STATUS_OK)
                            {
                                // Cleanup.
                                delete pDocStrm;
                                delete pMsgStrm;

                                // Finish.
                                return (!(status == INETSTREAM_STATUS_OK));
                            }
                        }

                        // Reset to <Begin-of-Line>.
                        aLineBuf.Seek (STREAM_SEEK_TO_BEGIN);
                        eState = INETMSG_EOL_SCR;
                    }
                    else if ((*pMsgWrite == '\r') || (*pMsgWrite == '\n'))
                    {
                        /*
                         * Found any line break character.
                         * Compare buffered line with part/close delimiter.
                         * Increment current part index upon match.
                         */
                        sal_uInt16 nLen = (sal_uInt16)(aLineBuf.Tell() & 0xffff);
                        if (nLen == aDelim.Len())
                        {
                            if (aDelim.CompareTo ((const sal_Char *) aLineBuf.GetData(), nLen)
                                == COMPARE_EQUAL) nCurIndex++;
                        }
                        else if (nLen == aClose.Len())
                        {
                            if (aClose.CompareTo ((const sal_Char *) aLineBuf.GetData(), nLen)
                                == COMPARE_EQUAL) nCurIndex++;
                        }
                        aLineBuf << *pMsgWrite++;
                        eState = INETMSG_EOL_FCR;
                    }
                    else
                    {
                        // Insert into line buffer.
                        aLineBuf << *pMsgWrite;
                    }
                }
                else
                {
                    // Buffer empty. Reset to <Begin-of-Buffer>.
                    pMsgRead = pMsgWrite = pMsgBuffer;

                    // Read document stream.
                    sal_uIntPtr nRead = pDocStrm->Read (
                        pMsgBuffer, sizeof (pMsgBuffer));
                    if (nRead > 0)
                    {
                        // Set read pointer.
                        pMsgRead += nRead;
                    }
                    else
                    {
                        // Premature end.
                        if (pMsgStrm)
                        {
                            // Assume end of requested part.
                            nCurIndex++;
                        }
                        else
                        {
                            // Requested part not found.
                            delete pDocStrm;
                            return sal_False;
                        }
                    }
                }
            } // while (nCurIndex < (nIndex + 1))
        }
        else
        {
            // Encapsulated message body. Create message parser stream.
            pMsgStrm = new INetMIMEMessageStream;
            pMsgStrm->SetTargetMessage (&rChildMsg);

            // Initialize control variables.
            INetMessageStreamState eState = INETMSG_EOL_BEGIN;

            // Go.
            while (eState == INETMSG_EOL_BEGIN)
            {
                if ((pMsgRead - pMsgWrite) > 0)
                {
                    // Bytes still in buffer. Put message down-stream.
                    int status = pMsgStrm->Write (
                        pMsgBuffer, (pMsgRead - pMsgWrite));
                    if (status != INETSTREAM_STATUS_OK)
                    {
                        // Cleanup.
                        delete pDocStrm;
                        delete pMsgStrm;

                        // Finish.
                        return (!(status == INETSTREAM_STATUS_ERROR));
                    }
                    pMsgWrite = pMsgBuffer + (pMsgRead - pMsgWrite);
                }
                else
                {
                    // Buffer empty. Reset to <Begin-of-Buffer>.
                    pMsgRead = pMsgWrite = pMsgBuffer;

                    // Read document stream.
                    sal_uIntPtr nRead = pDocStrm->Read (
                        pMsgBuffer, sizeof (pMsgBuffer));
                    if (nRead > 0)
                    {
                        // Set read pointer.
                        pMsgRead += nRead;
                    }
                    else
                    {
                        // Mark we're done.
                        eState = INETMSG_EOL_DONE;
                    }
                }
            } // while (eState == INETMSG_EOL_BEGIN)
        }

        // Done.
        if (pDocStrm) delete pDocStrm;
        if (pMsgStrm) delete pMsgStrm;
        return sal_True;
    }
    return sal_False;
}

/*
 * operator<<
 */
SvStream& INetMIMEMessage::operator<< (SvStream& rStrm) const
{
    INetRFC822Message::operator<< (rStrm);

    for (sal_uInt16 i = 0; i < INETMSG_MIME_NUMHDR; i++)
        rStrm << static_cast<sal_uInt32>(m_nIndex[i]);

#ifdef ENABLE_BYTESTRING_STREAM_OPERATORS
    rStrm << m_aBoundary;
#else
    rStrm.WriteByteString (m_aBoundary);
#endif
    rStrm << static_cast<sal_uInt32>(nNumChildren);

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetMIMEMessage::operator>> (SvStream& rStrm)
{
    INetRFC822Message::operator>> (rStrm);

    sal_uInt32 nTemp;
    for (sal_uInt16 i = 0; i < INETMSG_MIME_NUMHDR; i++)
    {
        rStrm >> nTemp;
        m_nIndex[i] = nTemp;
    }

#ifdef ENABLE_BYTESTRING_STREAM_OPERATORS
    rStrm >> m_aBoundary;
#else
    rStrm.ReadByteString (m_aBoundary);
#endif
    rStrm >> nTemp;
    nNumChildren = nTemp;

    return rStrm;
}


