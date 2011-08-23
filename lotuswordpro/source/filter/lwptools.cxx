/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#include "lwptools.hxx"
#include <rtl/ustrbuf.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <unicode/datefmt.h>
#include <unicode/udat.h>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace icu;
using namespace ::rtl;
using namespace ::osl;

/**
 * @descr		read lwp unicode string from stream to OUString per aEncoding
*/
sal_uInt16 LwpTools::QuickReadUnicode(LwpObjectStream* pObjStrm,
        OUString& str, sal_uInt16 strlen, rtl_TextEncoding aEncoding)
        //strlen: length of bytes
{

    sal_uInt16 readLen = 0;
    OUStringBuffer strBuf(128);
    sal_uInt16 len = 0;

    if( !IsUnicodePacked(pObjStrm, strlen) )
    {
        sal_Char buf[1024];

        while(strlen)
        {
            strlen>1023?len=1023 :len=strlen;
            len = pObjStrm->QuickRead(buf, len);
            buf[len] = '\0';
            strBuf.append( OUString(buf, len, aEncoding) );
            strlen -= len;
            readLen += len;
            if(!len) break;
        }
        str = strBuf.makeStringAndClear();
        return readLen;
    }
    else
    {
        sal_Char buf[1024];
        sal_Unicode unibuf[1024];
        sal_uInt8 readbyte;
        sal_uInt16 readword;

        BOOL flag = sal_False;	//switch if unicode part reached
        sal_uInt16 sublen = 0;

        while(readLen<strlen)
        {
            if(!flag)	//Not unicode string
            {
                len = pObjStrm->QuickRead(&readbyte, sizeof(readbyte));
                if(!len) break;
                readLen+=len;

                if(readbyte == 0x00)
                {
                    flag = sal_True;
                    if(sublen>0)	//add it to the strBuf
                    {
                        strBuf.append( OUString(buf, sublen, aEncoding) ); //try the aEncoding
                        sublen = 0;
                    }
                }
                else
                {
                    buf[sublen++] = readbyte;
                }
                if(sublen>=1023 || readLen==strlen) //add it to the strBuf
                {
                    strBuf.append( OUString(buf, sublen, aEncoding) ); //try the aEncoding
                    sublen = 0;
                }
            }
            else		//unicode string
            {
                len = pObjStrm->QuickRead(&readword, sizeof(readword));
                if(!len) break;
                readLen+=len;

                if(readword == 0x0000)
                {
                    flag = sal_False;
                    if(sublen)
                    {
                        unibuf[sublen] = sal_Unicode('\0');
                        strBuf.append( OUString(unibuf) );
                        sublen = 0;
                    }
                }
                else
                {
                    unibuf[sublen++] = readword;
                }
                if(sublen>=1023 || readLen==strlen)
                {
                    unibuf[sublen] = sal_Unicode('\0');
                    strBuf.append( OUString(unibuf) );
                    sublen = 0;
                }
            }
        }
//		if(sublen)
//		{
//			unibuf[sublen] = sal_Unicode('\0');
//			strBuf.append( OUString(unibuf) );
//			sublen = 0;
//		}
        str = strBuf.makeStringAndClear();
        return readLen;
    }
}

/**
 * @descr		Judge if the data (len) in object stream is lwp unicode packed
*/
BOOL LwpTools::IsUnicodePacked(LwpObjectStream* pObjStrm, sal_uInt16 len)
{
    sal_uInt8 byte;
    sal_uInt16 oldpos = pObjStrm->GetPos();

    for (sal_uInt16 i = 0; i < len; i++)
    {
        pObjStrm->QuickRead(&byte, sizeof(byte));
        if (byte == 0x00)
        {
            pObjStrm->Seek(oldpos);
            return sal_True;
        }
    }
    pObjStrm->Seek(oldpos);
    return sal_False;
}

sal_Bool LwpTools::isFileUrl(const OString &fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OUString LwpTools::convertToFileUrl(const OString &fileName)
{
    if ( isFileUrl(fileName) )
    {
        return OStringToOUString(fileName, osl_getThreadTextEncoding());
    }

    OUString uUrlFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        OSL_VERIFY( osl_getProcessWorkingDir(&uWorkingDir.pData) == osl_Process_E_None );
        OSL_VERIFY( FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName) == FileBase::E_None );
    } else
    {
        OSL_VERIFY( FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName) == FileBase::E_None );
    }

    return uUrlFileName;
}

OUString LwpTools::DateTimeToOUString(LtTm & dt)
{
    rtl::OUStringBuffer	buf;
    buf.append(dt.tm_year);
    buf.append( A2OUSTR("-") );
    buf.append(dt.tm_mon);
    buf.append( A2OUSTR("-") );
    buf.append(dt.tm_mday);
    buf.append( A2OUSTR("T") );
    buf.append(dt.tm_hour);
    buf.append( A2OUSTR(":") );
    buf.append(dt.tm_min);
    buf.append( A2OUSTR(":") );
    buf.append(dt.tm_sec);

    return buf.makeStringAndClear();
}

/**
 * @descr	get the system date format
*/
XFDateStyle* LwpTools::GetSystemDateStyle(sal_Bool bLongFormat)
{
    icu::DateFormat::EStyle style;
    if (bLongFormat)
        style = icu::DateFormat::FULL;//system full date format
    else
        style = icu::DateFormat::SHORT;//system short date format
/*	::com::sun::star::lang::Locale aLocale=Application::GetSettings().GetLocale();
    rtl::OUString strLang = aLocale.Language;
    rtl::OUString strCountry = aLocale.Country;
    strLang = strLang + A2OUSTR("_");
    rtl::OUString strLocale = strLang + strCountry;

    int32_t nLength = 0;
    int32_t nLengthNeed;
    UErrorCode status = U_ZERO_ERROR;
    UChar* pattern = NULL;

    UDateFormat* fmt= udat_open(UDAT_FULL, UDAT_FULL,
        (char*)(OUStringToOString(strLocale,RTL_TEXTENCODING_MS_1252).getStr()), NULL, 0, NULL, 0, &status);

    nLengthNeed = udat_toPattern(fmt,true,NULL,nLength,&status);
    if (status == U_BUFFER_OVERFLOW_ERROR)
    {
        status = U_ZERO_ERROR;
        nLength = nLengthNeed +1;
        pattern = (UChar*)malloc(sizeof(UChar)*nLength);
        udat_toPattern(fmt,true,pattern,nLength,&status);
    }
*/
    //1 get locale for system
    ::com::sun::star::lang::Locale aLocale=Application::GetSettings().GetLocale();
    rtl::OUString strLang = aLocale.Language;
    rtl::OUString strCountry = aLocale.Country;
    icu::Locale bLocale((char*)(OUStringToOString(strLang,RTL_TEXTENCODING_MS_1252).getStr()),
        (char*)(OUStringToOString(strCountry,RTL_TEXTENCODING_MS_1252).getStr()));
    //2 get icu format pattern by locale
    icu::DateFormat* fmt = icu::DateFormat::createDateInstance(style,bLocale);

    int32_t nLength = 0;
    int32_t nLengthNeed;
    UErrorCode status = U_ZERO_ERROR;
    UChar* pattern = NULL;

    nLengthNeed = udat_toPattern((void *const *)fmt,sal_False,NULL,nLength,&status);
    if (status == U_BUFFER_OVERFLOW_ERROR)
    {
        status = U_ZERO_ERROR;
        nLength = nLengthNeed +1;
        pattern = (UChar*)malloc(sizeof(UChar)*nLength);
        udat_toPattern((void *const *)fmt,sal_False,pattern,nLength,&status);
    }
    if (pattern == NULL)
        return NULL;
    // 3 parse pattern string,per icu date/time format syntax, there are 20 letters reserved
    // as patter letter,each represent a element in date/time and its repeat numbers represent
    // different format: for exampel: M produces '1',MM produces '01', MMM produces 'Jan', MMMM produces 'Januaray'
    // letter other than these letters is regard as text in the format, for example ','in 'Jan,2005'
    // we parse pattern string letter by letter and get the time format.
    UChar cSymbol;
    UChar cTmp;
    XFDateStyle* pDateStyle = new XFDateStyle;

    for (int32_t i=0;i<nLengthNeed;)
    {
        cSymbol = pattern[i];
        int32_t j;
        switch(cSymbol)
        {
            case 'G':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                pDateStyle->AddEra();
                break;
            }
            case 'y':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j <= 2)
                    pDateStyle->AddYear(sal_False);
                else
                    pDateStyle->AddYear();
                break;
            }
            case 'M':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddMonth(sal_False,sal_False);
                else if (j==2)
                    pDateStyle->AddMonth(sal_True,sal_False);
                else if (j==3)
                    pDateStyle->AddMonth(sal_False,sal_True);
                else
                    pDateStyle->AddMonth(sal_True,sal_True);
                break;
            }
            case 'd':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddMonthDay(sal_False);
                else
                    pDateStyle->AddMonthDay();
                break;
            }
            case 'h':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddHour(sal_False);
                else
                    pDateStyle->AddHour();
                break;
            }
            case 'H':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddHour(sal_False);
                else
                    pDateStyle->AddHour();
                break;
            }
            case 'm':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddMinute(sal_False);
                else
                    pDateStyle->AddMinute();
                break;
            }
            case 's':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddSecond(sal_False,0);
                else
                    pDateStyle->AddSecond(sal_True,0);
                break;
            }
            case 'S':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                /*if (j==1)
                    pDateStyle->AddSecond(sal_False);
                else
                    pDateStyle->AddSecond();*/
                break;
            }
            case 'E':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j<=2)
                    pDateStyle->AddWeekDay(sal_False);
                else
                    pDateStyle->AddWeekDay();
                break;
            }
            case 'D':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                /*if (j==1)
                    pDateStyle->AddWeekDay(sal_False);
                else
                    pDateStyle->AddWeekDay();*/
                break;
            }
            case 'F':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                /*if (j==1)
                    pDateStyle->AddWeekDay(sal_False);
                else
                    pDateStyle->AddWeekDay();*/
                break;
            }
            case 'w':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                /*if (j==1)
                    pDateStyle->AddWeekDay(sal_False);
                else
                    pDateStyle->AddWeekDay();*/
                break;
            }
            case 'W':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                /*if (j==1)
                    pDateStyle->AddWeekDay(sal_False);
                else
                    pDateStyle->AddWeekDay();*/
                break;
            }
            case 'a':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                pDateStyle->AddAmPm(sal_True);
                break;
            }
            case 'k':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                break;
            }
            case 'K':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pDateStyle->AddHour(sal_False);
                else
                    pDateStyle->AddHour();
                break;
            }
            case 'Z':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                break;
            }
            case '\''://'
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                break;
            }
            case '"':
            {
                pDateStyle->AddText(OUString(A2OUSTR("'")));
                break;
            }
            default:
            {
                if ((cSymbol>='A' && cSymbol<='Z') || (cSymbol>='a' && cSymbol<='z') )
                    return NULL;
                else//TEXT
                {
                    //UChar buffer[1024];
                    sal_Unicode buffer[1024];
                    buffer[0] = cSymbol;
                    for (j=1;;j++)
                    {
                        cTmp = pattern[i+j];
                        if ((cTmp>='A' && cTmp<='Z') || (cTmp>='a' && cTmp<='z') ||
                            cTmp=='\'' || cTmp=='"' )
                        {
                            i=i+j;
                            buffer[j]= '\0';
                            break;
                        }
                        else
                            buffer[j] = cTmp;
                    }

                    pDateStyle->AddText(OUString(buffer));//keep for all parsed
                }
                break;
            }
        }
    }
//	udat_close(fmt);
    return pDateStyle;
}
/**
 * @descr	get the system time format
*/
XFTimeStyle* LwpTools::GetSystemTimeStyle()
{
/*	::com::sun::star::lang::Locale aLocale=Application::GetSettings().GetLocale();
    rtl::OUString strLang = aLocale.Language;
    rtl::OUString strCountry = aLocale.Country;
    strLang = strLang + A2OUSTR("_");
    rtl::OUString strLocale = strLang + strCountry;

    int32_t nLength = 0;
    int32_t nLengthNeed;
    UErrorCode status = U_ZERO_ERROR;
    UChar* pattern = NULL;

    UDateFormat* fmt= udat_open(UDAT_FULL, UDAT_FULL,
        (char*)(OUStringToOString(strLocale,RTL_TEXTENCODING_MS_1252).getStr()), NULL, 0, NULL, 0, &status);

    nLengthNeed = udat_toPattern(fmt,true,NULL,nLength,&status);
    if (status == U_BUFFER_OVERFLOW_ERROR)
    {
        status = U_ZERO_ERROR;
        nLength = nLengthNeed +1;
        pattern = (UChar*)malloc(sizeof(UChar)*nLength);
        udat_toPattern(fmt,true,pattern,nLength,&status);
    }
*/
    //1 get locale for system
    ::com::sun::star::lang::Locale aLocale=Application::GetSettings().GetLocale();
    rtl::OUString strLang = aLocale.Language;
    rtl::OUString strCountry = aLocale.Country;
    icu::Locale bLocale((char*)(OUStringToOString(strLang,RTL_TEXTENCODING_MS_1252).getStr()),
        (char*)(OUStringToOString(strCountry,RTL_TEXTENCODING_MS_1252).getStr()));

    icu::DateFormat* fmt = icu::DateFormat::createTimeInstance(icu::DateFormat::DEFAULT,bLocale);
    //2 get icu format pattern by locale
    int32_t nLength = 0;
    int32_t nLengthNeed;
    UErrorCode status = U_ZERO_ERROR;
    UChar* pattern = NULL;
    nLengthNeed = udat_toPattern((void *const *)fmt,false,NULL,nLength,&status);
    if (status == U_BUFFER_OVERFLOW_ERROR)
    {
        status = U_ZERO_ERROR;
        nLength = nLengthNeed +1;
        pattern = (UChar*)malloc(sizeof(UChar)*nLength);
        udat_toPattern((void *const *)fmt,false,pattern,nLength,&status);
    }

    if (pattern == NULL)
        return NULL;
    // 3 parse pattern string,per icu date/time format syntax, there are 20 letters reserved
    // as patter letter,each represent a element in date/time and its repeat numbers represent
    // different format: for exampel: M produces '1',MM produces '01', MMM produces 'Jan', MMMM produces 'Januaray'
    // letter other than these letters is regard as text in the format, for example ','in 'Jan,2005'
    // we parse pattern string letter by letter and get the time format.
    // for time format ,for there is not date info,we can only parse the letter representing time.
    UChar cSymbol;
    UChar cTmp;
    XFTimeStyle* pTimeStyle = new XFTimeStyle;

    for (int32_t i=0;i<nLengthNeed;)
    {
        cSymbol = pattern[i];
        int32_t j;
        switch(cSymbol)
        {
            case 'h':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pTimeStyle->AddHour(sal_False);
                else
                    pTimeStyle->AddHour();
                break;
            }
            case 'H':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pTimeStyle->AddHour(sal_False);
                else
                    pTimeStyle->AddHour();
                break;
            }
            case 'm':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pTimeStyle->AddMinute(sal_False);
                else
                    pTimeStyle->AddMinute();
                break;
            }
            case 's':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pTimeStyle->AddSecond(sal_False,0);
                else
                    pTimeStyle->AddSecond(sal_True,0);
                break;
            }
            case 'S':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                /*if (j==1)
                    pDateStyle->AddSecond(sal_False);
                else
                    pDateStyle->AddSecond();*/
                break;
            }
            case 'a':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                pTimeStyle->SetAmPm(sal_True);
                break;
            }
            case 'k':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                break;
            }
            case 'K':
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                if (j==1)
                    pTimeStyle->AddHour(sal_False);
                else
                    pTimeStyle->AddHour();
                break;
            }
            case '\''://'
            {
                for (j=1;;j++)
                {
                    cTmp = pattern[i+j];
                    if (cTmp != cSymbol)
                    {
                        i=i+j;
                        break;
                    }
                }
                break;
            }
            case '"':
            {
                pTimeStyle->AddText(OUString(A2OUSTR("'")));
                break;
            }
            default:
            {
                if ((cSymbol>='A' && cSymbol<='Z') || (cSymbol>='a' && cSymbol<='z') )
                    return NULL;
                else//TEXT
                {
                    sal_Unicode buffer[1024];
                    buffer[0] = cSymbol;
                    //strBuffer.append(cSymbol);
                    for (j=1;;j++)
                    {
                        cTmp = pattern[i+j];
                        if ((cTmp>='A' && cTmp<='Z') || (cTmp>='a' && cTmp<='z') ||
                            cTmp=='\'' || cTmp=='"' )
                        {
                            i=i+j;
                            buffer[j]= '\0';
                            break;
                        }
                        else
                            buffer[j] = cTmp;
                    }
                    pTimeStyle->AddText(OUString(buffer));//keep for all parsed
                }
                break;
            }
        }
    }
//	udat_close(fmt);
    return pTimeStyle;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
