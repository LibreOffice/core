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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include "ftpdirp.hxx"
#include <osl/time.h>


using namespace ftp;

using ::rtl::OUString;
typedef sal_uInt32 ULONG;


inline sal_Bool ascii_isLetter( sal_Unicode ch )
{
    return (( (ch >= 0x0041) && (ch <= 0x005A)) ||
            (( ch >= 0x0061) && (ch <= 0x007A)));
}

inline sal_Bool ascii_isWhitespace( sal_Unicode ch )
{
    return ((ch <= 0x20) && ch);
}



/*========================================================================
 *
 * FTPDirectoryParser implementation.
 *
 *======================================================================*/
/*
 * parseDOS.
 * Accepts one of two styles:
 *
 * 1  *WSP 1*2DIGIT ("." / "-") 1*2DIGIT ("." / "-") 1*4DIGIT 1*WSP
 *    1*2DIGIT ":" 1*2DIGIT [*WSP ("A" / "P") "M"] 1*WSP
 *    ((DIGIT *(DIGIT / "." / ",")) / "<DIR>") 1*WSP 1*OCTET
 *
 *    interpreted as: mm.dd.yy hh:mm (size / <DIR>) name
 *
 * 2  *WSP 1*DIGIT 1*WSP *(1*CHAR *WSP) *1("DIR" 1*WSP) 1*2DIGIT "-" 1*2DIGIT
 *    "-" 1*4DIGIT 1*WSP 1*2DIGIT ":" 1*2DIGIT 1*WSP 1*OCTET
 *
 *    interpreted as: size attribs DIR mm-dd-yy hh:mm name
 */

sal_Bool FTPDirectoryParser::parseDOS (
    FTPDirentry &rEntry,
    const sal_Char  *pBuffer)
{
    sal_Bool   bDirectory = false;
    sal_uInt32 nSize = 0;
    sal_uInt16 nYear = 0;
    sal_uInt16 nMonth = 0;
    sal_uInt16 nDay = 0;
    sal_uInt16 nHour = 0;
    sal_uInt16 nMinute = 0;

    enum StateType
    {
        STATE_INIT_LWS,
        STATE_MONTH_OR_SIZE,
        STATE_1_DAY, STATE_1_YEAR, STATE_1_YEAR_LWS, STATE_1_HOUR,
        STATE_1_MINUTE, STATE_1_MINUTE_LWS, STATE_1_AP,
        STATE_1_APM, STATE_1_LESS, STATE_1_D, STATE_1_DI,
        STATE_1_DIR, STATE_1_SIZE,
        STATE_2_SIZE, STATE_2_SIZE_LWS, STATE_2_ATTRIB,
        STATE_2_D, STATE_2_DI, STATE_2_DIR_LWS,
        STATE_2_MONTH, STATE_2_DAY, STATE_2_YEAR, STATE_2_YEAR_LWS,
        STATE_2_HOUR, STATE_2_MINUTE,
        STATE_LWS_NAME,
        STATE_ERROR
    };

    int nDigits = 0;
    enum StateType eState = STATE_INIT_LWS;
    for (const sal_Char *p = pBuffer;
         eState != STATE_ERROR && *p;
         ++p)
    {
        switch (eState)
        {
            case STATE_INIT_LWS:
                if (*p >= '0' && *p <= '9')
                {
                    nMonth = *p - '0';
                    nDigits = 1;
                    eState = STATE_MONTH_OR_SIZE;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_MONTH_OR_SIZE:
                if (*p >= '0' && *p <= '9')
                {
                    nMonth = 10 * nMonth + (*p - '0');
                    if (nDigits < 2)
                        ++nDigits;
                    else
                    {
                        nSize = nMonth;
                        nMonth = 0;
                        eState = STATE_2_SIZE;
                    }
                }
                else if (ascii_isWhitespace(*p))
                {
                    nSize = nMonth;
                    nMonth = 0;
                    eState = STATE_2_SIZE_LWS;
                }
                else if ((*p == '.' || *p == '-') && nMonth && nMonth <= 12)
                {
                    nDigits = 0;
                    eState = STATE_1_DAY;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_1_DAY:
                if (*p >= '0' && *p <= '9')
                    if (nDigits < 2)
                    {
                        nDay = 10 * nDay + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                else if ((*p == '.' || *p == '-') && nDay && nDay <= 31)
                {
                    nDigits = 0;
                    eState = STATE_1_YEAR;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_1_YEAR:
                if (*p >= '0' && *p <= '9')
                {
                    if (nDigits < 4)
                    {
                        nYear = 10 * nYear + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                }
                else
                {
                    if (ascii_isWhitespace(*p))
                        eState = STATE_1_YEAR_LWS;
                    else
                        eState = STATE_ERROR;
                }
                break;

            case STATE_1_YEAR_LWS:
                if (*p >= '0' && *p <= '9')
                {
                    nHour = *p - '0';
                    nDigits = 1;
                    eState = STATE_1_HOUR;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_1_HOUR:
                if (*p >= '0' && *p <= '9')
                    if (nDigits < 2)
                    {
                        nHour = 10 * nHour + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                else if (*p == ':' && nHour < 24)
                {
                    nDigits = 0;
                    eState = STATE_1_MINUTE;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_1_MINUTE:
                if (*p >= '0' && *p <= '9')
                    if (nDigits < 2)
                    {
                        nMinute = 10 * nMinute + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                else if ((*p == 'a' || *p == 'A') && nMinute < 60)
                    if (nHour >= 1 && nHour <= 11)
                        eState = STATE_1_AP;
                    else if (nHour == 12)
                    {
                        nHour = 0;
                        eState = STATE_1_AP;
                    }
                    else
                        eState = STATE_ERROR;
                else if ((*p == 'p' || *p == 'P') && nMinute < 60)
                    if (nHour >= 1 && nHour <= 11)
                    {
                        nHour += 12;
                        eState = STATE_1_AP;
                    }
                    else if (nHour == 12)
                        eState = STATE_1_AP;
                    else
                        eState = STATE_ERROR;
                else if (ascii_isWhitespace(*p) && (nMinute < 60))
                    eState = STATE_1_MINUTE_LWS;
                else
                    eState = STATE_ERROR;
                break;

            case STATE_1_MINUTE_LWS:
                if (*p == 'a' || *p == 'A')
                    if (nHour >= 1 && nHour <= 11)
                        eState = STATE_1_AP;
                    else if (nHour == 12)
                    {
                        nHour = 0;
                        eState = STATE_1_AP;
                    }
                    else
                        eState = STATE_ERROR;
                else if (*p == 'p' || *p == 'P')
                    if (nHour >= 1 && nHour <= 11)
                    {
                        nHour += 12;
                        eState = STATE_1_AP;
                    }
                    else if (nHour == 12)
                        eState = STATE_1_AP;
                    else
                        eState = STATE_ERROR;
                else if (*p == '<')
                    eState = STATE_1_LESS;
                else if (*p >= '0' && *p <= '9')
                {
                    nSize = *p - '0';
                    eState = STATE_1_SIZE;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_1_AP:
                eState = *p == 'm' || *p == 'M' ? STATE_1_APM : STATE_ERROR;
                break;

            case STATE_1_APM:
                if (*p == '<')
                    eState = STATE_1_LESS;
                else if (*p >= '0' && *p <= '9')
                {
                    nSize = *p - '0';
                    eState = STATE_1_SIZE;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_1_LESS:
                eState = *p == 'd' || *p == 'D' ? STATE_1_D : STATE_ERROR;
                break;

            case STATE_1_D:
                eState = *p == 'i' || *p == 'I' ? STATE_1_DI : STATE_ERROR;
                break;

            case STATE_1_DI:
                eState = *p == 'r' || *p == 'R' ? STATE_1_DIR : STATE_ERROR;
                break;

            case STATE_1_DIR:
                if (*p == '>')
                {
                    bDirectory = true;
                    eState = STATE_LWS_NAME;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_1_SIZE:
                if (*p >= '0' && *p <= '9')
                    nSize = 10 * nSize + (*p - '0');
                else if (ascii_isWhitespace(*p))
                    eState = STATE_LWS_NAME;
                else
                    eState = STATE_ERROR;
                break;

            case STATE_2_SIZE:
                if (*p >= '0' && *p <= '9')
                    nSize = 10 * nSize + (*p - '0');
                else if (ascii_isWhitespace(*p))
                    eState = STATE_2_SIZE_LWS;
                else
                    eState = STATE_ERROR;
                break;

            case STATE_2_SIZE_LWS:
                if (*p == 'd' || *p == 'D')
                    eState = STATE_2_D;
                else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
                    eState = STATE_2_ATTRIB;
                else if (*p >= '0' && *p <= '9')
                {
                    nMonth = *p - '0';
                    nDigits = 1;
                    eState = STATE_2_MONTH;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_2_ATTRIB:
                if (ascii_isWhitespace(*p))
                    eState = STATE_2_SIZE_LWS;
                else if ((*p < 'a' || *p > 'z') && (*p < 'A' || *p > 'Z'))
                    eState = STATE_ERROR;
                break;

            case STATE_2_D:
                if (*p == 'i' || *p == 'I')
                    eState = STATE_2_DI;
                else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
                    eState = STATE_2_ATTRIB;
                else if (ascii_isWhitespace(*p))
                    eState = STATE_2_SIZE_LWS;
                else
                    eState = STATE_ERROR;
                break;

            case STATE_2_DI:
                if (*p == 'r' || *p == 'R')
                {
                    bDirectory = true;
                    eState = STATE_2_DIR_LWS;
                }
                else
                {
                    if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
                        eState = STATE_2_ATTRIB;
                    else if (ascii_isWhitespace(*p))
                        eState = STATE_2_SIZE_LWS;
                    else
                        eState = STATE_ERROR;
                }
                break;

            case STATE_2_DIR_LWS:
                if (*p >= '0' && *p <= '9')
                {
                    nMonth = *p - '0';
                    nDigits = 1;
                    eState = STATE_2_MONTH;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_2_MONTH:
                if (*p >= '0' && *p <= '9')
                    if (nDigits < 2)
                    {
                        nMonth = 10 * nMonth + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                else if (*p == '-' && nMonth && nMonth <= 12)
                {
                    nDigits = 0;
                    eState = STATE_2_DAY;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_2_DAY:
                if (*p >= '0' && *p <= '9')
                    if (nDigits < 2)
                    {
                        nDay = 10 * nDay + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                else if (*p == '-' && nDay && nDay <= 31)
                {
                    nDigits = 0;
                    eState = STATE_2_YEAR;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_2_YEAR:
                if (*p >= '0' && *p <= '9')
                {
                    if (nDigits < 4)
                    {
                        nYear = 10 * nYear + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                }
                else
                {
                    if (ascii_isWhitespace(*p))
                        eState = STATE_2_YEAR_LWS;
                    else
                        eState = STATE_ERROR;
                }
                break;

            case STATE_2_YEAR_LWS:
                if (*p >= '0' && *p <= '9')
                {
                    nHour = *p - '0';
                    nDigits = 1;
                    eState = STATE_2_HOUR;
                }
                else if (!ascii_isWhitespace(*p))
                    eState = STATE_ERROR;
                break;

            case STATE_2_HOUR:
                if (*p >= '0' && *p <= '9')
                    if (nDigits < 2)
                    {
                        nHour = 10 * nHour + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                else if (*p == ':' && nHour < 24)
                {
                    nDigits = 0;
                    eState = STATE_2_MINUTE;
                }
                else
                    eState = STATE_ERROR;
                break;

            case STATE_2_MINUTE:
                if (*p >= '0' && *p <= '9')
                {
                    if (nDigits < 2)
                    {
                        nMinute = 10 * nMinute + (*p - '0');
                        ++nDigits;
                    }
                    else
                        eState = STATE_ERROR;
                }
                else
                {
                    if (ascii_isWhitespace(*p) && (nMinute < 60))
                        eState = STATE_LWS_NAME;
                    else
                        eState = STATE_ERROR;
                }
                break;

            case STATE_LWS_NAME:
                if (!ascii_isWhitespace(*p))
                {
                    setPath (rEntry.m_aName, p);
                    if (bDirectory)
                        rEntry.m_nMode |= INETCOREFTP_FILEMODE_ISDIR;
                    rEntry.m_nSize = nSize;

                    setYear (rEntry.m_aDate, nYear);

                    rEntry.m_aDate.SetMonth(nMonth);
                    rEntry.m_aDate.SetDay(nDay);
                    rEntry.m_aDate.SetHour(nHour);
                    rEntry.m_aDate.SetMin(nMinute);

                    return sal_True;
                }
                break;
            case STATE_ERROR:
                break;
        }
    }

    return sal_False;
}

/*
 * parseVMS.
 * Directory entries may span one or two lines:
 *
 *   entry: *lws name *1(*lws <NEWLINE>) 1*lws size 1*lws datetime rest
 *
 *   name: filename "." filetype ";" version
 *   filename: 1*39fchar
 *   filetype: 1*39fchar
 *   version: non0digit *digit
 *
 *   size: "0" / non0digit *digit
 *
 *   datetime: date 1*lwsp time
 *   date: day "-" month "-" year
 *   day: (*1"0" non0digit) / ("1"-"2" digit) / ("3" "0"-"1")
 *   month: "JAN" / "FEB" / "MAR" / "APR" / "MAY" / "JUN" / "JUL" / "AUG"
 *        / "SEP" / "OCT" / "NOV" / "DEC" ; all case insensitive
 *   year: 2digit / 4digit
 *   time: hour ":" minute
 *   hour: ((*1"0" / "1") digit) / ("2" "0"-"3")
 *   minute: "0"-"5" digit
 *
 *   rest: *1(lws *<ANY>)
 *
 *   lws: <TAB> / <SPACE>
 *   non0digit: "1"-"9"
 *   digit: "0" / non0digit
 *   fchar: "A"-"Z" / "a"-"z" / digit / "-" / "_" / "$"
 *
 * For directories, the returned name is the <filename> part; for non-
 * directory files, the returned name is the <filename "." filetype> part.
 * An entry is a directory iff its filetype is "DIR" (ignoring case).
 *
 * The READ, WRITE, and ISLINK mode bits are not supported.
 *
 * The returned size is the <size> part, multiplied by 512, and with the high
 * order bits truncated to fit into a ULONG.
 *
 */
sal_Bool FTPDirectoryParser::parseVMS (
    FTPDirentry &rEntry,
    const sal_Char  *pBuffer)
{
    static OUString aFirstLineName;
    static sal_Bool bFirstLineDir = sal_False;

    for (sal_Bool bFirstLine = sal_True;; bFirstLine = sal_False)
    {
        const sal_Char *p = pBuffer;
        if (bFirstLine)
        {
            // Skip <*lws> part:
            while (*p == '\t' || *p == ' ')
                ++p;

            // Parse <filename "."> part:
            const sal_Char *pFileName = p;
            while ((*p >= 'A' && *p <= 'Z') ||
                   (*p >= 'a' && *p <= 'z') ||
                   (*p >= '0' && *p <= '9') ||
                   *p == '-' || *p == '_' || *p == '$')
                ++p;

            if (*p != '.' || p == pFileName || p - pFileName > 39)
            {
                if (!aFirstLineName.isEmpty())
                    continue;
                else
                    return sal_False;
            }

            // Parse <filetype ";"> part:
            const sal_Char *pFileType = ++p;
            while ((*p >= 'A' && *p <= 'Z') ||
                   (*p >= 'a' && *p <= 'z') ||
                   (*p >= '0' && *p <= '9') ||
                   *p == '-' || *p == '_' || *p == '$')
                ++p;

            if (*p != ';' || p == pFileName || p - pFileName > 39)
            {
                if (!aFirstLineName.isEmpty())
                    continue;
                else
                    return sal_False;
            }
            ++p;

            // Set entry's name and mode (ISDIR flag):
            if ((p - pFileType == 4) &&
                (pFileType[0] == 'D' || pFileType[0] == 'd') &&
                (pFileType[1] == 'I' || pFileType[1] == 'i') &&
                (pFileType[2] == 'R' || pFileType[2] == 'r')    )
            {
                setPath (rEntry.m_aName, pFileName, (pFileType - pFileName));
                rEntry.m_nMode = INETCOREFTP_FILEMODE_ISDIR;
            }
            else
            {
                setPath (rEntry.m_aName, pFileName, (p - pFileName));
                rEntry.m_nMode = 0;
            }

            // Skip <version> part:
            if (*p < '1' || *p > '9')
            {
                if (!aFirstLineName.isEmpty())
                    continue;
                else
                    return sal_False;
            }
            ++p;
            while (*p >= '0' && *p <= '9')
                ++p;

            // Parse <1*lws> or <*lws <NEWLINE>> part:
            sal_Bool bLWS = false;
            while (*p == '\t' || *p == ' ')
            {
                bLWS = true;
                ++p;
            }
            if (*p)
            {
                if (!bLWS)
                {
                    if (!aFirstLineName.isEmpty())
                        continue;
                    else
                        return sal_False;
                }
            }
            else
            {
                /*
                 * First line of entry spanning two lines,
                 * wait for second line.
                 */
                aFirstLineName = rEntry.m_aName;
                bFirstLineDir =
                    ((rEntry.m_nMode & INETCOREFTP_FILEMODE_ISDIR) != 0);
                return sal_False;
            }
        }
        else
        {
            /*
             * Second line of entry spanning two lines,
             * restore entry's name and mode (ISDIR flag).
             */
            rEntry.m_aName = aFirstLineName;
            rEntry.m_nMode = (bFirstLineDir ? INETCOREFTP_FILEMODE_ISDIR : 0);

            // Skip <1*lws> part:
            if (*p != '\t' && *p != ' ')
                return sal_False;
            ++p;
            while (*p == '\t' || *p == ' ')
                ++p;
        }

        // Parse <size> part and set entry's size:
        if (*p < '0' || *p > '9')
            return sal_False;
        ULONG nSize = *p - '0';
        if (*p++ != '0')
            while (*p >= '0' && *p <= '9')
                nSize = 10 * rEntry.m_nSize + (*p++ - '0');
        rEntry.m_nSize = 512 * nSize;

        // Skip <1*lws> part:
        if (*p != '\t' && *p != ' ')
            return sal_False;
        ++p;
        while (*p == '\t' || *p == ' ')
            ++p;

        // Parse <day "-"> part and set entry date's day:
        sal_uInt16 nDay;
        if (*p == '0')
        {
            ++p;
            if (*p < '1' || *p > '9')
                return sal_False;
            nDay = *p++ - '0';
        }
        else if (*p == '1' || *p == '2')
        {
            nDay = *p++ - '0';
            if (*p >= '0' && *p <= '9')
                nDay = 10 * nDay + (*p++ - '0');
        }
        else if (*p == '3')
        {
            ++p;
            nDay = (*p == '0' || *p == '1') ? 30 + (*p++ - '0') : 3;
        }
        else if (*p >= '4' && *p <= '9')
            nDay = *p++ - '0';
        else
            return sal_False;

        rEntry.m_aDate.SetDay(nDay);
        if (*p++ != '-')
            return sal_False;

        // Parse <month "-"> part and set entry date's month:
        sal_Char const * pMonth = p;
        sal_Int32 const monthLen = 3;
        for (int i = 0; i < monthLen; ++i)
        {
            if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z')))
                return sal_False;
            ++p;
        }
        if (rtl_str_compareIgnoreAsciiCase_WithLength(
                pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("JAN")) == 0)
            rEntry.m_aDate.SetMonth(1);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("FEB")) == 0)
            rEntry.m_aDate.SetMonth(2);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("MAR")) == 0)
            rEntry.m_aDate.SetMonth(3);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("APR")) == 0)
            rEntry.m_aDate.SetMonth(4);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("MAY")) == 0)
            rEntry.m_aDate.SetMonth(5);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("JUN")) == 0)
            rEntry.m_aDate.SetMonth(6);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("JUL")) == 0)
            rEntry.m_aDate.SetMonth(7);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("AUG")) == 0)
            rEntry.m_aDate.SetMonth(8);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("SEP")) == 0)
            rEntry.m_aDate.SetMonth(9);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("OCT")) == 0)
            rEntry.m_aDate.SetMonth(10);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("NOV")) == 0)
            rEntry.m_aDate.SetMonth(11);
        else if (rtl_str_compareIgnoreAsciiCase_WithLength(
                     pMonth, monthLen, RTL_CONSTASCII_STRINGPARAM("DEC")) == 0)
            rEntry.m_aDate.SetMonth(12);
        else
            return sal_False;
        if (*p++ != '-')
            return sal_False;

        // Parse <year> part and set entry date's year:
        sal_uInt16 nYear = 0;
        {for (int i = 0; i < 2; ++i)
        {
            if (*p < '0' || *p > '9')
                return sal_False;
            nYear = 10 * nYear + (*p++ - '0');
        }}
        if (*p >= '0' && *p <= '9')
        {
            nYear = 10 * nYear + (*p++ - '0');
            if (*p < '0' || *p > '9')
                return sal_False;
            nYear = 10 * nYear + (*p++ - '0');
        }
        setYear (rEntry.m_aDate, nYear);

        // Skip <1*lws> part:
        if (*p != '\t' && *p != ' ')
            return sal_False;
        ++p;
        while (*p == '\t' || *p == ' ')
            ++p;

        // Parse <hour ":"> part and set entry time's hour:
        sal_uInt16 nHour;
        if (*p == '0' || *p == '1')
        {
            nHour = *p++ - '0';
            if (*p >= '0' && *p <= '9')
                nHour = 10 * nHour + (*p++ - '0');
        }
        else if (*p == '2')
        {
            ++p;
            nHour = (*p >= '0' && *p <= '3') ? 20 + (*p++ - '0') : 2;
        }
        else if (*p >= '3' && *p <= '9')
            nHour = *p++ - '0';
        else
            return sal_False;

        rEntry.m_aDate.SetHour(nHour);
        if (*p++ != ':')
            return sal_False;

        /*
         * Parse <minute> part and set entry time's minutes,
         * seconds (0), and 1/100 seconds (0).
         */
        if (*p < '0' || *p > '5')
            return sal_False;

        sal_uInt16 nMinute = *p++ - '0';
        if (*p < '0' || *p > '9')
            return sal_False;

        nMinute = 10 * nMinute + (*p++ - '0');
        rEntry.m_aDate.SetMin(nMinute);
        rEntry.m_aDate.SetSec(0);
        rEntry.m_aDate.Set100Sec(0);

        // Skip <rest> part:
        if (*p && (*p != '\t' && *p != ' '))
            return sal_False;

        return sal_True;
    }
}

/*
 * parseUNIX
 */
sal_Bool FTPDirectoryParser::parseUNIX (
    FTPDirentry &rEntry,
    const sal_Char  *pBuffer)
{
    const sal_Char *p1, *p2;
    p1 = p2 = pBuffer;

    if (!((*p1 == '-') || (*p1 == 'd') || (*p1 == 'l')))
        return sal_False;

    // 1st column: FileMode.
    if (*p1 == 'd')
        rEntry.m_nMode |= INETCOREFTP_FILEMODE_ISDIR;

    if (*p1 == 'l')
        rEntry.m_nMode |= INETCOREFTP_FILEMODE_ISLINK;

    // Skip to end of column and set rights by the way
    while (*p1 && !ascii_isWhitespace(*p1)) {
        if(*p1 == 'r')
            rEntry.m_nMode |= INETCOREFTP_FILEMODE_READ;
        else if(*p1 == 'w')
            rEntry.m_nMode |= INETCOREFTP_FILEMODE_WRITE;
        p1++;
    }

    /*
     * Scan for the sequence of size and date fields:
     *   *LWS 1*DIGIT 1*LWS 3CHAR 1*LWS 1*2DIGIT 1*LWS
     *   (4DIGIT / (1*2DIGIT ":" 2DIGIT)) 1*LWS
     */
    enum Mode
    {
        FOUND_NONE, FOUND_SIZE, FOUND_MONTH, FOUND_DAY, FOUND_YEAR_TIME
    };

    const sal_Char *pDayStart = 0;
    const sal_Char *pDayEnd = 0;
    Mode eMode;
    for (eMode = FOUND_NONE; *p1 && eMode != FOUND_YEAR_TIME; p1 = p2 + 1)
    {
        while (*p1 && ascii_isWhitespace(*p1))
            ++p1;
        p2 = p1;
        while (*p2 && !ascii_isWhitespace(*p2))
            ++p2;

        switch (eMode)
        {
            case FOUND_NONE:
                if (parseUNIX_isSizeField (p1, p2, rEntry.m_nSize))
                    eMode = FOUND_SIZE;
                break;

            case FOUND_SIZE:
                if (parseUNIX_isMonthField (p1, p2, rEntry.m_aDate))
                    eMode = FOUND_MONTH;
                else if (!parseUNIX_isSizeField (p1, p2, rEntry.m_nSize))
                    eMode = FOUND_NONE;
                break;

            case FOUND_MONTH:
                if (parseUNIX_isDayField (p1, p2, rEntry.m_aDate))
                {
                    pDayStart = p1;
                    pDayEnd = p2;
                    eMode = FOUND_DAY;
                }
                else if (parseUNIX_isSizeField (p1, p2, rEntry.m_nSize))
                    eMode = FOUND_SIZE;
                else
                    eMode = FOUND_NONE;
                break;

            case FOUND_DAY:
                if (parseUNIX_isYearTimeField (p1, p2, rEntry.m_aDate))
                    eMode = FOUND_YEAR_TIME;
                else if (
                    parseUNIX_isSizeField (
                        pDayStart, pDayEnd, rEntry.m_nSize) &&
                    parseUNIX_isMonthField (
                        p1, p2, rEntry.m_aDate))
                    eMode = FOUND_MONTH;
                else if (parseUNIX_isSizeField (p1, p2, rEntry.m_nSize))
                    eMode = FOUND_SIZE;
                else
                    eMode = FOUND_NONE;
                break;
            case FOUND_YEAR_TIME:
                break;
        }
    }

    if (eMode == FOUND_YEAR_TIME)
    {
        // 9th column: FileName (rest of line).
        while (*p1 && ascii_isWhitespace(*p1)) p1++;
        setPath (rEntry.m_aName, p1);

        // Done.
        return sal_True;
    }
    return sal_False;
}

/*
 * parseUNIX_isSizeField.
 */
sal_Bool FTPDirectoryParser::parseUNIX_isSizeField (
    const sal_Char *pStart,
    const sal_Char *pEnd,
    sal_uInt32     &rSize)
{
    if (!*pStart || !*pEnd || pStart == pEnd)
        return sal_False;

    rSize = 0;
    if (*pStart >= '0' && *pStart <= '9')
    {
        for (; pStart < pEnd; ++pStart)
            if ((*pStart >= '0') && (*pStart <= '9'))
                rSize = 10 * rSize + (*pStart - '0');
            else
                return sal_False;
        return sal_True;
    }
    else
    {
        /*
         * For a combination of long group name and large file size,
         * some FTPDs omit LWS between those two columns.
         */
        int nNonDigits = 0;
        int nDigits = 0;

        for (; pStart < pEnd; ++pStart)
            if ((*pStart >= '1') && (*pStart <= '9'))
            {
                ++nDigits;
                rSize = 10 * rSize + (*pStart - '0');
            }
            else if ((*pStart == '0') && nDigits)
            {
                ++nDigits;
                rSize *= 10;
            }
            else if ((*pStart > ' ') && (sal::static_int_cast<sal_uInt8>(*pStart) <= '\x7F'))
            {
                nNonDigits += nDigits + 1;
                nDigits = 0;
                rSize = 0;
            }
            else
                return sal_False;
        return ((nNonDigits >= 9) && (nDigits >= 7));
    }
}

/*
 * parseUNIX_isMonthField.
 */
sal_Bool FTPDirectoryParser::parseUNIX_isMonthField (
    const sal_Char *pStart,
    const sal_Char *pEnd,
    DateTime       &rDateTime)
{
    if (!*pStart || !*pEnd || pStart + 3 != pEnd)
        return sal_False;

    if ((pStart[0] == 'j' || pStart[0] == 'J') &&
        (pStart[1] == 'a' || pStart[1] == 'A') &&
        (pStart[2] == 'n' || pStart[2] == 'N')    )
    {
        rDateTime.SetMonth(1);
        return sal_True;
    }
    if ((pStart[0] == 'f' || pStart[0] == 'F') &&
        (pStart[1] == 'e' || pStart[1] == 'E') &&
        (pStart[2] == 'b' || pStart[2] == 'B')    )
    {
        rDateTime.SetMonth(2);
        return sal_True;
    }
    if ((pStart[0] == 'm' || pStart[0] == 'M') &&
        (pStart[1] == 'a' || pStart[1] == 'A') &&
        (pStart[2] == 'r' || pStart[2] == 'R')    )
    {
        rDateTime.SetMonth(3);
        return sal_True;
    }
    if ((pStart[0] == 'a' || pStart[0] == 'A') &&
        (pStart[1] == 'p' || pStart[1] == 'P') &&
        (pStart[2] == 'r' || pStart[2] == 'R')    )
    {
        rDateTime.SetMonth(4);
        return sal_True;
    }
    if ((pStart[0] == 'm' || pStart[0] == 'M') &&
        (pStart[1] == 'a' || pStart[1] == 'A') &&
        (pStart[2] == 'y' || pStart[2] == 'Y')    )
    {
        rDateTime.SetMonth(5);
        return sal_True;
    }
    if ((pStart[0] == 'j' || pStart[0] == 'J') &&
        (pStart[1] == 'u' || pStart[1] == 'U') &&
        (pStart[2] == 'n' || pStart[2] == 'N')    )
    {
        rDateTime.SetMonth(6);
        return sal_True;
    }
    if ((pStart[0] == 'j' || pStart[0] == 'J') &&
        (pStart[1] == 'u' || pStart[1] == 'U') &&
        (pStart[2] == 'l' || pStart[2] == 'L')    )
    {
        rDateTime.SetMonth(7);
        return sal_True;
    }
    if ((pStart[0] == 'a' || pStart[0] == 'A') &&
        (pStart[1] == 'u' || pStart[1] == 'U') &&
        (pStart[2] == 'g' || pStart[2] == 'G')    )
    {
        rDateTime.SetMonth(8);
        return sal_True;
    }
    if ((pStart[0] == 's' || pStart[0] == 'S') &&
        (pStart[1] == 'e' || pStart[1] == 'E') &&
        (pStart[2] == 'p' || pStart[2] == 'P')    )
    {
        rDateTime.SetMonth(9);
        return sal_True;
    }
    if ((pStart[0] == 'o' || pStart[0] == 'O') &&
        (pStart[1] == 'c' || pStart[1] == 'C') &&
        (pStart[2] == 't' || pStart[2] == 'T')    )
    {
        rDateTime.SetMonth(10);
        return sal_True;
    }
    if ((pStart[0] == 'n' || pStart[0] == 'N') &&
        (pStart[1] == 'o' || pStart[1] == 'O') &&
        (pStart[2] == 'v' || pStart[2] == 'V')    )
    {
        rDateTime.SetMonth(11);
        return sal_True;
    }
    if ((pStart[0] == 'd' || pStart[0] == 'D') &&
        (pStart[1] == 'e' || pStart[1] == 'E') &&
        (pStart[2] == 'c' || pStart[2] == 'C')    )
    {
        rDateTime.SetMonth(12);
        return sal_True;
    }
    return sal_False;
}

/*
 * parseUNIX_isDayField.
 */
sal_Bool FTPDirectoryParser::parseUNIX_isDayField (
    const sal_Char *pStart,
    const sal_Char *pEnd,
    DateTime       &rDateTime)
{
    if (!*pStart || !*pEnd || pStart == pEnd)
        return sal_False;
    if (*pStart < '0' || *pStart > '9')
        return sal_False;

    sal_uInt16 nDay = *pStart - '0';
    if (pStart + 1 < pEnd)
    {
        if (pStart + 2 != pEnd || pStart[1] < '0' || pStart[1] > '9')
            return sal_False;
        nDay = 10 * nDay + (pStart[1] - '0');
    }
    if (!nDay || nDay > 31)
        return sal_False;

    rDateTime.SetDay(nDay);
    return sal_True;
}

/*
 * parseUNIX_isYearTimeField.
 */
sal_Bool FTPDirectoryParser::parseUNIX_isYearTimeField (
    const sal_Char *pStart,
    const sal_Char *pEnd,
    DateTime       &rDateTime)
{
    if (!*pStart || !*pEnd || pStart == pEnd ||
        *pStart < '0' || *pStart > '9')
        return sal_False;

    sal_uInt16 nNumber = *pStart - '0';
    ++pStart;

    if (pStart == pEnd)
        return sal_False;
    if (*pStart == ':')
        return parseUNIX_isTime (pStart, pEnd, nNumber, rDateTime);
    if (*pStart < '0' || *pStart > '9')
        return sal_False;

    nNumber = 10 * nNumber + (*pStart - '0');
    ++pStart;

    if (pStart == pEnd)
        return sal_False;
    if (*pStart == ':')
        return parseUNIX_isTime (pStart, pEnd, nNumber, rDateTime);
    if (*pStart < '0' || *pStart > '9')
        return sal_False;

    nNumber = 10 * nNumber + (*pStart - '0');
    ++pStart;

    if (pStart == pEnd || *pStart < '0' || *pStart > '9')
        return sal_False;

    nNumber = 10 * nNumber + (*pStart - '0');
    if (pStart + 1 != pEnd || nNumber < 1970)
        return sal_False;

    rDateTime.SetYear(nNumber);
    rDateTime.SetTime(0);
    return sal_True;
}

/*
 * parseUNIX_isTime.
 */
sal_Bool FTPDirectoryParser::parseUNIX_isTime (
    const sal_Char *pStart,
    const sal_Char *pEnd,
    sal_uInt16      nHour,
    DateTime       &rDateTime)
{
    if ((nHour     > 23 ) || (pStart + 3 != pEnd) ||
        (pStart[1] < '0') || (pStart[1] > '5')    ||
        (pStart[2] < '0') || (pStart[2] > '9')       )
        return sal_False;

    sal_uInt16 nMin = 10 * (pStart[1] - '0') + (pStart[2] - '0');

    rDateTime.SetHour (nHour);
    rDateTime.SetMin (nMin);
    rDateTime.SetSec (0);
    rDateTime.Set100Sec (0);

//      Date aCurDate;
//      if (rDateTime.GetMonth() > aCurDate.GetMonth())
//          rDateTime.SetYear(aCurDate.GetYear() - 1);
//      else
//          rDateTime.SetYear(aCurDate.GetYear());
//      return sal_True;

    TimeValue aTimeVal;
    osl_getSystemTime(&aTimeVal);
    oslDateTime aCurrDateTime;
    osl_getDateTimeFromTimeValue(&aTimeVal,&aCurrDateTime);

    if (rDateTime.GetMonth() > aCurrDateTime.Month)
        rDateTime.SetYear(aCurrDateTime.Year - 1);
    else
        rDateTime.SetYear(aCurrDateTime.Year);
    return sal_True;
}

/*
 * setYear.
 *
 * Two-digit years are taken as within 50 years back and 49 years forward
 * (both ends inclusive) from the current year. The returned date is not
 * checked for validity of the given day in the given month and year.
 *
 */
sal_Bool FTPDirectoryParser::setYear (
    DateTime &rDateTime, sal_uInt16 nYear)
{
    if (nYear < 100)
    {
        TimeValue aTimeVal;
        osl_getSystemTime(&aTimeVal);
        oslDateTime aCurrDateTime;
        osl_getDateTimeFromTimeValue(&aTimeVal,&aCurrDateTime);
        sal_uInt16 nCurrentYear = aCurrDateTime.Year;
//        sal_uInt16 nCurrentYear = Date().GetYear();
        sal_uInt16 nCurrentCentury = nCurrentYear / 100;
        nCurrentYear %= 100;
        if (nCurrentYear < 50)
            if (nYear <= nCurrentYear)
                nYear += nCurrentCentury * 100;
            else if (nYear < nCurrentYear + 50)
                nYear += nCurrentCentury * 100;
            else
                nYear += (nCurrentCentury - 1) * 100;
        else
            if (nYear >= nCurrentYear)
                nYear += nCurrentCentury * 100;
            else if (nYear >= nCurrentYear - 50)
                nYear += nCurrentCentury * 100;
            else
                nYear += (nCurrentCentury + 1) * 100;
    }

    rDateTime.SetYear(nYear);
    return sal_True;
}

/*
 * setPath.
 */
sal_Bool FTPDirectoryParser::setPath (
    OUString &rPath, const sal_Char *value, sal_Int32 length)
{
    if (value)
    {
        if (length < 0)
            length = rtl_str_getLength (value);
        rPath = OUString (value, length, RTL_TEXTENCODING_UTF8);
    }
    return (!!value);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
