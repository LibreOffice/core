/*************************************************************************
 *
 *  $RCSfile: inetmsg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif

#ifndef _DATETIME_HXX
#include <datetime.hxx>
#endif
#ifndef _TOOLS_INETMIME_HXX
#include <inetmime.hxx>
#endif
#ifndef _TOOLS_INETMSG_HXX
#include <inetmsg.hxx>
#endif
#ifndef _TOOLS_INETSTRM_HXX
#include <inetstrm.hxx>
#endif

#include <stdio.h>

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
    ULONG i, n = m_aHeaderList.Count();
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
        ULONG i, n = rMsg.GetHeaderCount();
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
    ULONG                     &rnIndex)
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
ULONG INetMessage::SetHeaderField (
    const UniString& rName, const UniString& rValue, ULONG nIndex)
{
    ULONG nResult = nIndex;
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        ByteString (rName, RTL_TEXTENCODING_ASCII_US), rValue,
        nResult);
    return nResult;
}

/*
 * SetHeaderField.
 */
ULONG INetMessage::SetHeaderField (
    const INetMessageHeader &rHeader, ULONG nIndex)
{
    ULONG nResult = nIndex;
    SetHeaderField_Impl (rHeader, nResult);
    return nResult;
}


/*
 * operator<<
 */
SvStream& INetMessage::operator<< (SvStream& rStrm) const
{
    rStrm << m_nDocSize;
    rStrm.WriteByteString (m_aDocName, RTL_TEXTENCODING_UTF8);

    ULONG i, n = m_aHeaderList.Count();
    rStrm << n;

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

    // Copy.
    rStrm >> m_nDocSize;
    rStrm.ReadByteString (m_aDocName, RTL_TEXTENCODING_UTF8);

    ULONG i, n = 0;
    rStrm >> n;

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
    ULONG i, n = rMsg.GetHeaderCount();
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
    ULONG i, n = aValueList.Count();
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
 * _ImplINetRFC822MessageHeaderData.
 */
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

#define HDR(n) _ImplINetRFC822MessageHeaderData[(n)]

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
    for (USHORT i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        m_nIndex[i] = LIST_ENTRY_NOTFOUND;
}

INetRFC822Message::INetRFC822Message (const INetRFC822Message& rMsg)
    : INetMessage (rMsg)
{
    for (USHORT i = 0; i < INETMSG_RFC822_NUMHDR; i++)
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

        for (USHORT i = 0; i < INETMSG_RFC822_NUMHDR; i++)
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
BOOL INetRFC822Message::GenerateDateField (
    const DateTime& rDateTime, UniString& rDateFieldW)
{
    // Check arguments.
    if (!rDateTime.IsValid()       ||
        (rDateTime.GetSec()  > 59) ||
        (rDateTime.GetMin()  > 59) ||
        (rDateTime.GetHour() > 23)    ) return FALSE;

    // Prepare output string.
    ByteString rDateField;

    // Insert Date.
    rDateField += wkdays[(USHORT)(rDateTime.GetDayOfWeek())];
    rDateField += ", ";

    USHORT nNum = rDateTime.GetDay();
    if (nNum < 10) rDateField += '0';
    rDateField += nNum;
    rDateField += ' ';

    rDateField += months[(USHORT)(rDateTime.GetMonth() - 1)];
    rDateField += ' ';

    rDateField += rDateTime.GetYear();
    rDateField += ' ';

    // Insert Time.
    nNum = rDateTime.GetHour();
    if (nNum < 10) rDateField += '0';
    rDateField += nNum;
    rDateField += ':';

    nNum = rDateTime.GetMin();
    if (nNum < 10) rDateField += '0';
    rDateField += nNum;
    rDateField += ':';

    nNum = rDateTime.GetSec();
    if (nNum < 10) rDateField += '0';
    rDateField += nNum;
    rDateField += " GMT";

    // Done.
    rDateFieldW = UniString (rDateField, RTL_TEXTENCODING_ASCII_US);
    return TRUE;
}

/*
 * ParseDateField and local helper functions.
 */
static USHORT ParseNumber (const ByteString& rStr, USHORT& nIndex)
{
    USHORT n = nIndex;
    while ((n < rStr.Len()) && rtl_char_isDigit(rStr.GetChar(n))) n++;

    ByteString aNum (rStr.Copy (nIndex, (n - nIndex)));
    nIndex = n;

    return (USHORT)(aNum.ToInt32());
}

static USHORT ParseMonth (const ByteString& rStr, USHORT& nIndex)
{
    USHORT n = nIndex;
    while ((n < rStr.Len()) && rtl_char_isLetter(rStr.GetChar(n))) n++;

    ByteString aMonth (rStr.Copy (nIndex, 3));
    nIndex = n;

    USHORT i;
    for (i = 0; i < 12; i++)
        if (aMonth.CompareIgnoreCaseToAscii (months[i]) == 0) break;
    return (i + 1);
}

BOOL INetRFC822Message::ParseDateField (
    const UniString& rDateFieldW, DateTime& rDateTime)
{
    ByteString rDateField (rDateFieldW, RTL_TEXTENCODING_ASCII_US);
    if (rDateField.Len() == 0) return FALSE;

    if (rDateField.Search (':') != STRING_NOTFOUND)
    {
        // Some DateTime format.
        USHORT nIndex = 0;

        // Skip over <Wkd> or <Weekday>, leading and trailing space.
        while ((nIndex < rDateField.Len()) &&
               (rDateField.GetChar(nIndex) == ' '))
            nIndex++;

        while (
            (nIndex < rDateField.Len()) &&
            (rtl_char_isLetter (rDateField.GetChar(nIndex)) ||
             (rDateField.GetChar(nIndex) == ',')     ))
            nIndex++;

        while ((nIndex < rDateField.Len()) &&
               (rDateField.GetChar(nIndex) == ' '))
            nIndex++;

        if (rtl_char_isLetter (rDateField.GetChar(nIndex)))
        {
            // Format: ctime().
            if ((rDateField.Len() - nIndex) < 20) return FALSE;

            rDateTime.SetMonth  (ParseMonth  (rDateField, nIndex)); nIndex++;
            rDateTime.SetDay    (ParseNumber (rDateField, nIndex)); nIndex++;

            rDateTime.SetHour   (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetMin    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetSec    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.Set100Sec (0);

            USHORT nYear = ParseNumber (rDateField, nIndex);
            if (nYear < 100) nYear += 1900;
            rDateTime.SetYear   (nYear);
        }
        else
        {
            // Format: RFC1036 or RFC1123.
            if ((rDateField.Len() - nIndex) < 17) return FALSE;

            rDateTime.SetDay    (ParseNumber (rDateField, nIndex)); nIndex++;
            rDateTime.SetMonth  (ParseMonth  (rDateField, nIndex)); nIndex++;

            USHORT nYear  = ParseNumber (rDateField, nIndex);  nIndex++;
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
                BOOL   bEast   = (rDateField.GetChar(nIndex++) == '+');
                USHORT nOffset = ParseNumber (rDateField, nIndex);
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
        return FALSE;
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
ULONG INetRFC822Message::SetHeaderField (
    const INetMessageHeader &rHeader, ULONG nNewIndex)
{
    ByteString aName (rHeader.GetName());
    const sal_Char *pData = aName.GetBuffer();
    const sal_Char *pStop = pData + aName.Len() + 1;
    const sal_Char *check = "";

    ULONG       nIdx     = LIST_APPEND;
    int         eState   = INETMSG_RFC822_BEGIN;
    int         eOkState = INETMSG_RFC822_OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case INETMSG_RFC822_BEGIN:
                eState = INETMSG_RFC822_CHECK;
                eOkState = INETMSG_RFC822_OK;

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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
                           (rtl_char_toLowerCase (*pData) == *check))
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

    for (USHORT i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        rStrm << m_nIndex[i];

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetRFC822Message::operator>> (SvStream& rStrm)
{
    INetMessage::operator>> (rStrm);

    for (USHORT i = 0; i < INETMSG_RFC822_NUMHDR; i++)
        rStrm >> m_nIndex[i];

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
static const ByteString _ImplINetMIMEMessageHeaderData[] =
{
    ByteString ("MIME-Version"),
    ByteString ("Content-Description"),
    ByteString ("Content-Disposition"),
    ByteString ("Content-ID"),
    ByteString ("Content-Type"),
    ByteString ("Content-Transfer-Encoding")
};

#define MIMEHDR(n) _ImplINetMIMEMessageHeaderData[(n)]

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
      nNumChildren  (0),
      pParent       (NULL),
      bHeaderParsed (FALSE)
{
    for (USHORT i = 0; i < INETMSG_MIME_NUMHDR; i++)
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

    USHORT i;
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
ULONG INetMIMEMessage::SetHeaderField (
    const INetMessageHeader &rHeader, ULONG nNewIndex)
{
    ByteString aName (rHeader.GetName());
    const sal_Char *pData = aName.GetBuffer();
    const sal_Char *pStop = pData + aName.Len() + 1;
    const sal_Char *check = "";

    ULONG      nIdx     = LIST_APPEND;
    int        eState   = INETMSG_MIME_BEGIN;
    int        eOkState = INETMSG_MIME_OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case INETMSG_MIME_BEGIN:
                eState = INETMSG_MIME_CHECK;
                eOkState = INETMSG_MIME_OK;

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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

                switch (rtl_char_toLowerCase (*pData))
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
                           (rtl_char_toLowerCase (*pData) == *check))
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
BOOL INetMIMEMessage::EnableAttachChild (INetMessageContainerType eType)
{
    // Check context.
    if (IsContainer())
        return FALSE;

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
        sprintf (sTail, "%08X%08X", aCurTime.GetTime(), (ULONG)this);
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
    return TRUE;
}

/*
 * AttachChild.
 */
BOOL INetMIMEMessage::AttachChild (
    INetMIMEMessage& rChildMsg, BOOL bOwner)
{
    if (IsContainer() /*&& rChildMsg.GetContentType().Len() */)
    {
        if (bOwner) rChildMsg.pParent = this;
        aChildren.Insert (&rChildMsg, LIST_APPEND);
        nNumChildren = aChildren.Count();

        return TRUE;
    }
    return FALSE;
}

/*
 * DetachChild.
 */
BOOL INetMIMEMessage::DetachChild (
    ULONG nIndex, INetMIMEMessage& rChildMsg) const
{
    if (IsContainer())
    {
        // Check document stream.
        if (GetDocumentLB() == NULL) return FALSE;
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
                        USHORT nLen = (USHORT)(aLineBuf.Tell() & 0xffff);
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
                    ULONG nRead = pDocStrm->Read (
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
                            return FALSE;
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
                        pMsgBuffer, (pMsgRead - pMsgWrite), NULL);
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
                    ULONG nRead = pDocStrm->Read (
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
        return TRUE;
    }
    return FALSE;
}

/*
 * operator<<
 */
SvStream& INetMIMEMessage::operator<< (SvStream& rStrm) const
{
    INetRFC822Message::operator<< (rStrm);

    for (USHORT i = 0; i < INETMSG_MIME_NUMHDR; i++)
        rStrm << m_nIndex[i];

#ifdef ENABLE_BYTESTRING_STREAM_OPERATORS
    rStrm << m_aBoundary;
#else
    rStrm.WriteByteString (m_aBoundary);
#endif
    rStrm << nNumChildren;

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetMIMEMessage::operator>> (SvStream& rStrm)
{
    INetRFC822Message::operator>> (rStrm);

    for (USHORT i = 0; i < INETMSG_MIME_NUMHDR; i++)
        rStrm >> m_nIndex[i];

#ifdef ENABLE_BYTESTRING_STREAM_OPERATORS
    rStrm >> m_aBoundary;
#else
    rStrm.ReadByteString (m_aBoundary);
#endif
    rStrm >> nNumChildren;

    return rStrm;
}

/*=======================================================================
 *
 * INetNewsMessage Implementation.
 *
 *=====================================================================*/
/*
 * _ImplINetNewsMessageHeaderData.
 */
static const ByteString _ImplINetNewsMessageHeaderData[] =
{
    ByteString ("Approved"),
    ByteString ("Control"),
    ByteString ("Distribution"),
    ByteString ("Expires"),
    ByteString ("Followup-To"),
    ByteString ("Lines"),
    ByteString ("Newsgroups"),
    ByteString ("Organization"),
    ByteString ("Path"),
    ByteString ("Summary"),
    ByteString ("Xref"),
    ByteString ("X-Newsreader")
};

#define NEWSHDR(n) _ImplINetNewsMessageHeaderData[(n)]

/*
 * _ImplINetNewsMessageHeaderState.
 */
enum _ImplINetNewsMessageHeaderState
{
    INETMSG_NEWS_BEGIN,
    INETMSG_NEWS_CHECK,
    INETMSG_NEWS_OK,
    INETMSG_NEWS_JUNK,

    INETMSG_NEWS_LETTER_X
};

/*
 * INetNewsMessage.
 */
INetNewsMessage::INetNewsMessage (void)
    : INetMIMEMessage ()
{
    for (USHORT i = 0; i < INETMSG_NEWS_NUMHDR; i++)
        m_nIndex[i] = LIST_ENTRY_NOTFOUND;
}

INetNewsMessage::INetNewsMessage (const INetNewsMessage& rMsg)
    : INetMIMEMessage (rMsg)
{
    for (USHORT i = 0; i < INETMSG_NEWS_NUMHDR; i++)
        m_nIndex[i] = rMsg.m_nIndex[i];
}

/*
 * operator=
 */
INetNewsMessage& INetNewsMessage::operator= (const INetNewsMessage& rMsg)
{
    if (this != &rMsg)
    {
        // Assign base.
        INetMIMEMessage::operator= (rMsg);

        // Cleanup and copy.
        for (USHORT i = 0; i < INETMSG_NEWS_NUMHDR; i++)
            m_nIndex[i] = rMsg.m_nIndex[i];
    }
    return *this;
}

/*
 * ~INetNewsMessage.
 */
INetNewsMessage::~INetNewsMessage (void)
{
}

/*
 * CreateMessage.
 */
INetNewsMessage *INetNewsMessage::CreateMessage (
    const INetNewsMessage& rMsg) const
{
    return (new INetNewsMessage (rMsg));
}

/*
 * SetHeaderField.
 * (Header Field Parser).
 */
ULONG INetNewsMessage::SetHeaderField (
    const INetMessageHeader &rHeader, ULONG nNewIndex)
{
    ByteString aName (rHeader.GetName());
    const sal_Char *pData = aName.GetBuffer();
    const sal_Char *pStop = pData + aName.Len() + 1;
    const sal_Char *check = "";

    ULONG      nIdx     = LIST_APPEND;
    int        eState   = INETMSG_NEWS_BEGIN;
    int        eOkState = INETMSG_NEWS_OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case INETMSG_NEWS_BEGIN:
                eState = INETMSG_NEWS_CHECK;
                eOkState = INETMSG_NEWS_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'a':
                        check = "pproved";
                        nIdx = INETMSG_NEWS_APPROVED;
                        break;

                    case 'c':
                        check = "ontrol";
                        nIdx = INETMSG_NEWS_CONTROL;
                        break;

                    case 'd':
                        check = "istribution";
                        nIdx = INETMSG_NEWS_DISTRIBUTION;
                        break;

                    case 'e':
                        check = "xpires";
                        nIdx = INETMSG_NEWS_EXPIRES;
                        break;

                    case 'f':
                        check = "ollowup-to";
                        nIdx = INETMSG_NEWS_FOLLOWUP_TO;
                        break;

                    case 'l':
                        check = "ines";
                        nIdx = INETMSG_NEWS_LINES;
                        break;

                    case 'n':
                        check = "ewsgroups";
                        nIdx = INETMSG_NEWS_NEWSGROUPS;
                        break;

                    case 'o':
                        check = "rganization";
                        nIdx = INETMSG_NEWS_ORGANIZATION;
                        break;

                    case 'p':
                        check = "ath";
                        nIdx = INETMSG_NEWS_PATH;
                        break;

                    case 's':
                        check = "ummary";
                        nIdx = INETMSG_NEWS_SUMMARY;
                        break;

                    case 'x':
                        eState = INETMSG_NEWS_LETTER_X;
                        break;

                    default:
                        eState = INETMSG_NEWS_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_NEWS_LETTER_X:
                eState = INETMSG_NEWS_CHECK;
                eOkState = INETMSG_NEWS_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'r':
                        check = "ef";
                        nIdx = INETMSG_NEWS_XREF;
                        break;

                    case '-':
                        check = "newsreader";
                        nIdx = INETMSG_NEWS_X_NEWSREADER;
                        break;

                    default:
                        eState = INETMSG_NEWS_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_NEWS_CHECK:
                if (*check)
                {
                    while (*pData && *check &&
                           (rtl_char_toLowerCase (*pData) == *check))
                    {
                        pData++;
                        check++;
                    }
                }
                else
                {
                    check = pData;
                }
                eState = (*check == '\0') ? eOkState : INETMSG_NEWS_JUNK;
                break;

            case INETMSG_NEWS_OK:
                pData = pStop;
                SetHeaderField_Impl (
                    HEADERFIELD (NEWSHDR(nIdx), rHeader.GetValue()),
                    m_nIndex[nIdx]);
                nNewIndex = m_nIndex[nIdx];
                break;

            default: // INETMSG_NEWS_JUNK
                pData = pStop;
                nNewIndex = INetMIMEMessage::SetHeaderField (
                    rHeader, nNewIndex);
                break;
        }
    }
    return nNewIndex;
}

/*
 * Specific Set-Methods.
 */
void INetNewsMessage::SetApproved (const String& rApproved)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_APPROVED), rApproved,
        m_nIndex[INETMSG_NEWS_APPROVED]);
}

void INetNewsMessage::SetControl (const String& rControl)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_CONTROL), rControl,
        m_nIndex[INETMSG_NEWS_CONTROL]);
}

void INetNewsMessage::SetDistribution (const String& rDistribution)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_DISTRIBUTION), rDistribution,
        m_nIndex[INETMSG_NEWS_DISTRIBUTION]);
}

void INetNewsMessage::SetExpires (const String& rExpires)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_EXPIRES), rExpires,
        m_nIndex[INETMSG_NEWS_EXPIRES]);
}

void INetNewsMessage::SetFollowupTo (const String& rFollowupTo)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_FOLLOWUP_TO), rFollowupTo,
        m_nIndex[INETMSG_NEWS_FOLLOWUP_TO]);
}

void INetNewsMessage::SetLines (const String& rLines)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_LINES), rLines,
        m_nIndex[INETMSG_NEWS_LINES]);
}

void INetNewsMessage::SetNewsgroups (const String& rNewsgroups)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_NEWSGROUPS), rNewsgroups,
        m_nIndex[INETMSG_NEWS_NEWSGROUPS]);
}

void INetNewsMessage::SetOrganization (const String& rOrganization)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_ORGANIZATION), rOrganization,
        m_nIndex[INETMSG_NEWS_ORGANIZATION]);
}

void INetNewsMessage::SetPath (const String& rPath)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_PATH), rPath,
        m_nIndex[INETMSG_NEWS_PATH]);
}

void INetNewsMessage::SetSummary (const String& rSummary)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_SUMMARY), rSummary,
        m_nIndex[INETMSG_NEWS_SUMMARY]);
}

void INetNewsMessage::SetXref (const String& rXref)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_XREF), rXref,
        m_nIndex[INETMSG_NEWS_XREF]);
}

void INetNewsMessage::SetXNewsreader (const String& rXNewsreader)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        NEWSHDR(INETMSG_NEWS_X_NEWSREADER), rXNewsreader,
        m_nIndex[INETMSG_NEWS_X_NEWSREADER]);
}

/*
 * operator<<
 */
SvStream& INetNewsMessage::operator<< (SvStream& rStrm) const
{
    INetMIMEMessage::operator<< (rStrm);

    for (USHORT i = 0; i < INETMSG_NEWS_NUMHDR; i++)
        rStrm << m_nIndex[i];

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetNewsMessage::operator>> (SvStream& rStrm)
{
    INetMIMEMessage::operator>> (rStrm);

    for (USHORT i = 0; i < INETMSG_NEWS_NUMHDR; i++)
        rStrm >> m_nIndex[i];

    return rStrm;
}

/*=======================================================================
 *
 * INetHTTPMessage Implementation.
 *
 *=====================================================================*/
/*
 * _ImplINetHTTPMessageHeaderData.
 */
static const ByteString _ImplINetHTTPMessageHeaderData[] =
{
    ByteString ("Accept"),
    ByteString ("Accept-Charset"),
    ByteString ("Accept-Encoding"),
    ByteString ("Accept-Language"),
    ByteString ("Allow"),
    ByteString ("Authorization"),
    ByteString ("Cache-Control"),
    ByteString ("Connection"),
    ByteString ("Content-Encoding"),
    ByteString ("Content-Language"),
    ByteString ("Content-Length"),
    ByteString ("Content-MD5"),
    ByteString ("Content-Range"),
    ByteString ("Content-Version"),
    ByteString ("Cookie"),
    ByteString ("Derived-From"),
    ByteString ("Expires"),
    ByteString ("Forwarded"),
    ByteString ("Host"),
    ByteString ("If-Modified-Since"),
    ByteString ("Keep-Alive"),
    ByteString ("Last-Modified"),
    ByteString ("Link"),
    ByteString ("Location"),
    ByteString ("Proxy-Authenticate"),
    ByteString ("Proxy-Authorization"),
    ByteString ("Pragma"),
    ByteString ("Public"),
    ByteString ("Range"),
    ByteString ("Referer"),
    ByteString ("Retry-After"),
    ByteString ("Server"),
    ByteString ("Title"),
    ByteString ("Transfer-Encoding"),
    ByteString ("Unless"),
    ByteString ("Upgrade"),
    ByteString ("URI"),
    ByteString ("User-Agent"),
    ByteString ("WWW-Authenticate")
};

#define HTTPHDR(n) _ImplINetHTTPMessageHeaderData[(n)]

/*
 * _ImplINetHTTPMessageHeaderState.
 */
enum _ImplINetHTTPMessageHeaderState
{
    INETMSG_HTTP_BEGIN,
    INETMSG_HTTP_CHECK,
    INETMSG_HTTP_OK,
    INETMSG_HTTP_JUNK,

    INETMSG_HTTP_LETTER_A,
    INETMSG_HTTP_LETTER_C,
    INETMSG_HTTP_LETTER_L,
    INETMSG_HTTP_LETTER_P,
    INETMSG_HTTP_LETTER_R,
    INETMSG_HTTP_LETTER_T,
    INETMSG_HTTP_LETTER_U,
    INETMSG_HTTP_TOKEN_CON,
    INETMSG_HTTP_CONTENT,
    INETMSG_HTTP_PROXY_AUTH
};

/*
 * INetHTTPMessage.
 */
INetHTTPMessage::INetHTTPMessage (void)
    : INetMIMEMessage ()
{
    for (USHORT i = 0; i < INETMSG_HTTP_NUMHDR; i++)
        m_nIndex[i] = LIST_ENTRY_NOTFOUND;
}

INetHTTPMessage::INetHTTPMessage (const INetHTTPMessage& rMsg)
    : INetMIMEMessage (rMsg)
{
    for (USHORT i = 0; i < INETMSG_HTTP_NUMHDR; i++)
        m_nIndex[i] = rMsg.m_nIndex[i];
}

/*
 * operator=
 */
INetHTTPMessage& INetHTTPMessage::operator= (const INetHTTPMessage& rMsg)
{
    if (this != &rMsg)
    {
        // Assign base.
        INetMIMEMessage::operator= (rMsg);

        // Cleanup and copy.
        for (USHORT i = 0; i < INETMSG_HTTP_NUMHDR; i++)
            m_nIndex[i] = rMsg.m_nIndex[i];
    }
    return *this;
}

/*
 * ~INetHTTPMessage.
 */
INetHTTPMessage::~INetHTTPMessage (void)
{
}

/*
 * CreateMessage.
 */
INetHTTPMessage *INetHTTPMessage::CreateMessage (
    const INetHTTPMessage& rMsg) const
{
    return (new INetHTTPMessage (rMsg));
}

/*
 * SetHeaderField.
 * (Header Field Parser).
 */
ULONG INetHTTPMessage::SetHeaderField (
    const INetMessageHeader &rHeader, ULONG nNewIndex)
{
    ByteString aName (rHeader.GetName());
    const sal_Char  *pData = aName.GetBuffer();
    const sal_Char  *pStop = pData + aName.Len() + 1;
    const sal_Char  *check = "";

    ULONG      nIdx     = LIST_APPEND;
    int        eState   = INETMSG_HTTP_BEGIN;
    int        eOkState = INETMSG_HTTP_OK;

    while (pData < pStop)
    {
        switch (eState)
        {
            case INETMSG_HTTP_BEGIN:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                        case 'a':
                            eState = INETMSG_HTTP_LETTER_A;
                            break;

                        case 'c':
                            eState = INETMSG_HTTP_LETTER_C;
                            break;

                        case 'd':
                            check = "erived-from";
                            nIdx = INETMSG_HTTP_DERIVED_FROM;
                            break;

                        case 'e':
                            check = "xpires";
                            nIdx = INETMSG_HTTP_EXPIRES;
                            break;

                        case 'f':
                            check = "orwarded";
                            nIdx = INETMSG_HTTP_FORWARDED;
                            break;

                        case 'h':
                            check = "ost";
                            nIdx = INETMSG_HTTP_HOST;
                            break;

                        case 'i':
                            check = "f-modified-since";
                            nIdx = INETMSG_HTTP_IF_MODIFIED_SINCE;
                            break;

                        case 'k':
                            check = "eep-alive";
                            nIdx = INETMSG_HTTP_KEEP_ALIVE;
                            break;

                        case 'l':
                            eState = INETMSG_HTTP_LETTER_L;
                            break;

                        case 'p':
                            eState = INETMSG_HTTP_LETTER_P;
                            break;

                        case 'r':
                            eState = INETMSG_HTTP_LETTER_R;
                            break;

                        case 's':
                            check = "erver";
                            nIdx = INETMSG_HTTP_SERVER;
                            break;

                        case 't':
                            eState = INETMSG_HTTP_LETTER_T;
                            break;

                        case 'u':
                            eState = INETMSG_HTTP_LETTER_U;
                            break;

                        case 'w':
                            check = "ww-authenticate";
                            nIdx = INETMSG_HTTP_WWW_AUTHENTICATE;
                            break;

                        default:
                            eState = INETMSG_HTTP_JUNK;
                            break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_A:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'c':
                        if (INetMIME::equalIgnoreCase (
                            pData, pData + 14, "ccept-language"))
                        {
                            nIdx = INETMSG_HTTP_ACCEPT_LANGUAGE;
                            pData += 14;
                        }
                        else if (INetMIME::equalIgnoreCase (
                            pData, pData + 14, "ccept-encoding"))
                        {
                            nIdx = INETMSG_HTTP_ACCEPT_ENCODING;
                            pData += 14;
                        }
                        else if (INetMIME::equalIgnoreCase (
                            pData, pData + 13, "ccept-charset"))
                        {
                            nIdx = INETMSG_HTTP_ACCEPT_CHARSET;
                            pData += 13;
                        }
                        else if (INetMIME::equalIgnoreCase (
                            pData, pData + 5, "ccept"))
                        {
                            nIdx = INETMSG_HTTP_ACCEPT;
                            pData += 5;
                        }
                        else
                        {
                            eState = INETMSG_HTTP_JUNK;
                        }
                        break;

                    case 'l':
                        check = "low";
                        nIdx = INETMSG_HTTP_ALLOW;
                        break;

                    case 'u':
                        check = "thorization";
                        nIdx = INETMSG_HTTP_AUTHORIZATION;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_C:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'a':
                        check = "che-control";
                        nIdx = INETMSG_HTTP_CACHE_CONTROL;
                        break;

                    case 'o':
                        if (rtl_char_toLowerCase (*(pData + 1)) == 'n')
                        {
                            eState = INETMSG_HTTP_TOKEN_CON;
                        }
                        else if (rtl_char_toLowerCase (*(pData + 1)) == 'o')
                        {
                            check = "kie";
                            nIdx = INETMSG_HTTP_COOKIE;
                        }
                        else
                        {
                            eState = INETMSG_HTTP_JUNK;
                        }
                        pData++;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_L:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'a':
                        check = "st-modified";
                        nIdx = INETMSG_HTTP_LAST_MODIFIED;
                        break;

                    case 'i':
                        check = "nk";
                        nIdx = INETMSG_HTTP_LINK;
                        break;

                    case 'o':
                        check = "cation";
                        nIdx = INETMSG_HTTP_LOCATION;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_P:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'r':
                        switch (rtl_char_toLowerCase (*(pData + 1)))
                        {
                            case 'a':
                                check = "gma";
                                nIdx = INETMSG_HTTP_PRAGMA;
                                break;

                            case 'o':
                                check = "xy-auth";
                                eOkState = INETMSG_HTTP_PROXY_AUTH;
                                break;

                            default:
                                eState = INETMSG_HTTP_JUNK;
                                break;
                        }
                        pData++;
                        break;

                    case 'u':
                        check = "blic";
                        nIdx = INETMSG_HTTP_PUBLIC;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_R:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'a':
                        check = "nge";
                        nIdx = INETMSG_HTTP_RANGE;
                        break;

                    case 'e':
                        switch (rtl_char_toLowerCase (*(pData + 1)))
                        {
                            case 'f':
                                check = "erer";
                                nIdx = INETMSG_HTTP_REFERER;
                                break;

                            case 't':
                                check = "ry-after";
                                nIdx = INETMSG_HTTP_RETRY_AFTER;
                                break;

                            default:
                                eState = INETMSG_HTTP_JUNK;
                                break;
                        }
                        pData++;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_T:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'i':
                        check = "tle";
                        nIdx = INETMSG_HTTP_TITLE;
                        break;

                    case 'r':
                        check = "ansfer-encoding";
                        nIdx = INETMSG_HTTP_TRANSFER_ENCODING;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_LETTER_U:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'n':
                        check = "less";
                        nIdx = INETMSG_HTTP_UNLESS;
                        break;

                    case 'p':
                        check = "grade";
                        nIdx = INETMSG_HTTP_UPGRADE;
                        break;

                    case 'r':
                        check = "i";
                        nIdx = INETMSG_HTTP_URI;
                        break;

                    case 's':
                        check = "er-agent";
                        nIdx = INETMSG_HTTP_USER_AGENT;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_TOKEN_CON:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'n':
                        check = "ection";
                        nIdx = INETMSG_HTTP_CONNECTION;
                        break;

                    case 't':
                        check = "ent-";
                        eOkState = INETMSG_HTTP_CONTENT;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_CONTENT:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'e':
                        check = "ncoding";
                        nIdx = INETMSG_HTTP_CONTENT_ENCODING;
                        break;

                    case 'l':
                        switch (rtl_char_toLowerCase (*(pData + 1)))
                        {
                            case 'a':
                                check = "nguage";
                                nIdx = INETMSG_HTTP_CONTENT_LANGUAGE;
                                break;

                            case 'e':
                                check = "ngth";
                                nIdx = INETMSG_HTTP_CONTENT_LENGTH;
                                break;

                            default:
                                eState = INETMSG_HTTP_JUNK;
                                break;
                        }
                        pData++;
                        break;

                    case 'm':
                        check = "d5";
                        nIdx = INETMSG_HTTP_CONTENT_MD5;
                        break;

                    case 'r':
                        check = "ange";
                        nIdx = INETMSG_HTTP_CONTENT_RANGE;
                        break;

                    case 'v':
                        check = "ersion";
                        nIdx = INETMSG_HTTP_CONTENT_VERSION;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_PROXY_AUTH:
                eState = INETMSG_HTTP_CHECK;
                eOkState = INETMSG_HTTP_OK;

                switch (rtl_char_toLowerCase (*pData))
                {
                    case 'e':
                        check = "nticate";
                        nIdx = INETMSG_HTTP_PROXY_AUTHENTICATE;
                        break;

                    case 'o':
                        check = "rization";
                        nIdx = INETMSG_HTTP_PROXY_AUTHORIZATION;
                        break;

                    default:
                        eState = INETMSG_HTTP_JUNK;
                        break;
                }
                pData++;
                break;

            case INETMSG_HTTP_CHECK:
                if (*check)
                {
                    while (*pData && *check &&
                           (rtl_char_toLowerCase (*pData) == *check))
                    {
                        pData++;
                        check++;
                    }
                }
                else
                {
                    check = pData;
                }
                eState = (*check == '\0') ? eOkState : INETMSG_HTTP_JUNK;
                break;

            case INETMSG_HTTP_OK:
                pData = pStop;
                SetHeaderField_Impl (
                    HEADERFIELD (HTTPHDR(nIdx), rHeader.GetValue()),
                    m_nIndex[nIdx]);
                nNewIndex = m_nIndex[nIdx];
                break;

            default: // INETMSG_HTTP_JUNK
                pData = pStop;
                nNewIndex = INetMIMEMessage::SetHeaderField (
                    rHeader, nNewIndex);
                break;
        }
    }
    return nNewIndex;
}

/*
 * Specific Set-Methods.
 */
void INetHTTPMessage::SetAccept (const String& rAccept)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_ACCEPT), rAccept,
        m_nIndex[INETMSG_HTTP_ACCEPT]);
}

void INetHTTPMessage::SetAcceptCharset (const String& rAcceptCharset)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_ACCEPT_CHARSET), rAcceptCharset,
        m_nIndex[INETMSG_HTTP_ACCEPT_CHARSET]);
}

void INetHTTPMessage::SetAcceptEncoding (const String& rAcceptEncoding)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_ACCEPT_ENCODING), rAcceptEncoding,
        m_nIndex[INETMSG_HTTP_ACCEPT_ENCODING]);
}

void INetHTTPMessage::SetAcceptLanguage (const String& rAcceptLanguage)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_ACCEPT_LANGUAGE), rAcceptLanguage,
        m_nIndex[INETMSG_HTTP_ACCEPT_LANGUAGE]);
}

void INetHTTPMessage::SetAllow (const String& rAllow)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_ALLOW), rAllow,
        m_nIndex[INETMSG_HTTP_ALLOW]);
}

void INetHTTPMessage::SetAuthorization (const String& rAuthorization)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_AUTHORIZATION), rAuthorization,
        m_nIndex[INETMSG_HTTP_AUTHORIZATION]);
}

void INetHTTPMessage::SetCacheControl (const String& rCacheControl)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CACHE_CONTROL), rCacheControl,
        m_nIndex[INETMSG_HTTP_CACHE_CONTROL]);
}

void INetHTTPMessage::SetConnection (const String& rConnection)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONNECTION), rConnection,
        m_nIndex[INETMSG_HTTP_CONNECTION]);
}

void INetHTTPMessage::SetContentEncoding (const String& rContentEncoding)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONTENT_ENCODING), rContentEncoding,
        m_nIndex[INETMSG_HTTP_CONTENT_ENCODING]);
}

void INetHTTPMessage::SetContentLanguage (const String& rContentLanguage)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONTENT_LANGUAGE), rContentLanguage,
        m_nIndex[INETMSG_HTTP_CONTENT_LANGUAGE]);
}

void INetHTTPMessage::SetContentLength (const String& rContentLength)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONTENT_LENGTH), rContentLength,
        m_nIndex[INETMSG_HTTP_CONTENT_LENGTH]);
}

void INetHTTPMessage::SetContentMD5 (const String& rContentMD5)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONTENT_MD5), rContentMD5,
        m_nIndex[INETMSG_HTTP_CONTENT_MD5]);
}

void INetHTTPMessage::SetContentRange (const String& rContentRange)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONTENT_RANGE), rContentRange,
        m_nIndex[INETMSG_HTTP_CONTENT_RANGE]);
}

void INetHTTPMessage::SetContentVersion (const String& rContentVersion)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_CONTENT_VERSION), rContentVersion,
        m_nIndex[INETMSG_HTTP_CONTENT_VERSION]);
}

void INetHTTPMessage::SetCookie (const String& rCookie)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_COOKIE), rCookie,
        m_nIndex[INETMSG_HTTP_COOKIE]);
}

void INetHTTPMessage::SetDerivedFrom (const String& rDerivedFrom)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_DERIVED_FROM), rDerivedFrom,
        m_nIndex[INETMSG_HTTP_DERIVED_FROM]);
}

void INetHTTPMessage::SetExpires (const String& rExpires)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_EXPIRES), rExpires,
        m_nIndex[INETMSG_HTTP_EXPIRES]);
}

void INetHTTPMessage::SetForwarded (const String& rForwarded)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_FORWARDED), rForwarded,
        m_nIndex[INETMSG_HTTP_FORWARDED]);
}

void INetHTTPMessage::SetHost (const String& rHost)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_HOST), rHost,
        m_nIndex[INETMSG_HTTP_HOST]);
}

void INetHTTPMessage::SetIfModifiedSince (const String& rIfModifiedSince)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_IF_MODIFIED_SINCE), rIfModifiedSince,
        m_nIndex[INETMSG_HTTP_IF_MODIFIED_SINCE]);
}

void INetHTTPMessage::SetKeepAlive (const String& rKeepAlive)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_KEEP_ALIVE), rKeepAlive,
        m_nIndex[INETMSG_HTTP_KEEP_ALIVE]);
}

void INetHTTPMessage::SetLastModified (const String& rLastModified)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_LAST_MODIFIED), rLastModified,
        m_nIndex[INETMSG_HTTP_LAST_MODIFIED]);
}

void INetHTTPMessage::SetLink (const String& rLink)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_LINK), rLink,
        m_nIndex[INETMSG_HTTP_LINK]);
}

void INetHTTPMessage::SetLocation (const String& rLocation)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_LOCATION), rLocation,
        m_nIndex[INETMSG_HTTP_LOCATION]);
}

void INetHTTPMessage::SetProxyAuthenticate (
    const String& rProxyAuthenticate)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_PROXY_AUTHENTICATE), rProxyAuthenticate,
        m_nIndex[INETMSG_HTTP_PROXY_AUTHENTICATE]);
}

void INetHTTPMessage::SetProxyAuthorization (
    const String& rProxyAuthorization)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_PROXY_AUTHORIZATION), rProxyAuthorization,
        m_nIndex[INETMSG_HTTP_PROXY_AUTHORIZATION]);
}

void INetHTTPMessage::SetPragma (const String& rPragma)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_PRAGMA), rPragma,
        m_nIndex[INETMSG_HTTP_PRAGMA]);
}

void INetHTTPMessage::SetPublic (const String& rPublic)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_PUBLIC), rPublic,
        m_nIndex[INETMSG_HTTP_PUBLIC]);
}

void INetHTTPMessage::SetRange (const String& rRange)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_RANGE), rRange,
        m_nIndex[INETMSG_HTTP_RANGE]);
}

void INetHTTPMessage::SetReferer (const String& rReferer)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_REFERER), rReferer,
        m_nIndex[INETMSG_HTTP_REFERER]);
}

void INetHTTPMessage::SetRetryAfter (const String& rRetryAfter)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_RETRY_AFTER), rRetryAfter,
        m_nIndex[INETMSG_HTTP_RETRY_AFTER]);
}

void INetHTTPMessage::SetServer (const String& rServer)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_SERVER), rServer,
        m_nIndex[INETMSG_HTTP_SERVER]);
}

void INetHTTPMessage::SetTitle (const String& rTitle)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_TITLE), rTitle,
        m_nIndex[INETMSG_HTTP_TITLE]);
}

void INetHTTPMessage::SetTransferEncoding (
    const String& rTransferEncoding)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_TRANSFER_ENCODING), rTransferEncoding,
        m_nIndex[INETMSG_HTTP_TRANSFER_ENCODING]);
}

void INetHTTPMessage::SetUnless (const String& rUnless)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_UNLESS), rUnless,
        m_nIndex[INETMSG_HTTP_UNLESS]);
}

void INetHTTPMessage::SetUpgrade (const String& rUpgrade)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_UPGRADE), rUpgrade,
        m_nIndex[INETMSG_HTTP_UPGRADE]);
}

void INetHTTPMessage::SetURI (const String& rURI)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_URI), rURI,
        m_nIndex[INETMSG_HTTP_URI]);
}

void INetHTTPMessage::SetUserAgent (const String& rUserAgent)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_USER_AGENT), rUserAgent,
        m_nIndex[INETMSG_HTTP_USER_AGENT]);
}

void INetHTTPMessage::SetWWWAuthenticate (const String& rWWWAuthenticate)
{
    SetHeaderField_Impl (
        INetMIME::HEADER_FIELD_TEXT,
        HTTPHDR(INETMSG_HTTP_WWW_AUTHENTICATE), rWWWAuthenticate,
        m_nIndex[INETMSG_HTTP_WWW_AUTHENTICATE]);
}

/*
 * operator<<
 */
SvStream& INetHTTPMessage::operator<< (SvStream& rStrm) const
{
    INetMIMEMessage::operator<< (rStrm);

    for (USHORT i = 0; i < INETMSG_HTTP_NUMHDR; i++)
        rStrm << m_nIndex[i];

    return rStrm;
}

/*
 * operator>>
 */
SvStream& INetHTTPMessage::operator>> (SvStream& rStrm)
{
    INetMIMEMessage::operator>> (rStrm);

    for (USHORT i = 0; i < INETMSG_HTTP_NUMHDR; i++)
        rStrm >> m_nIndex[i];

    return rStrm;
}

