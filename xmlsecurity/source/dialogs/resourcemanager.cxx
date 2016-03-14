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


#include "resourcemanager.hxx"

#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>
#include <svl/solar.hrc>
#include <unotools/syslocale.hxx>
#include <rtl/ustring.h>
#include <rtl/ustrbuf.h>
#include <vector>

using namespace std;

namespace XmlSec
{
    static ResMgr*          pResMgr = nullptr;
    static SvtSysLocale*    pSysLocale = nullptr;

    ResMgr* GetResMgr()
    {
        if (!pResMgr)
            pResMgr = ResMgr::CreateResMgr("xmlsec");
        return pResMgr;
    }

    const LocaleDataWrapper&    GetLocaleData()
    {
        if (!pSysLocale)
            pSysLocale = new SvtSysLocale;
        return pSysLocale->GetLocaleData();
    }

    DateTime GetDateTime( const css::util::DateTime& _rDT )
    {
        return DateTime(_rDT);
    }

    OUString GetDateTimeString( const css::util::DateTime& _rDT )
    {
        // String with date and time information (#i20172#)
        DateTime aDT( GetDateTime( _rDT ) );
        const LocaleDataWrapper& rLoDa = GetLocaleData();

        return  rLoDa.getDate( aDT ) + " " + rLoDa.getTime( aDT );
    }

    OUString GetDateString( const css::util::DateTime& _rDT )
    {
        return GetLocaleData().getDate( GetDateTime( _rDT ) );
    }

    /*
        Creates two strings based on the distinguished name which are displayed in the
        certificate details view. The first string contains only the values of the attribute
        and values pairs, which are separated by commas. All escape characters ('"') are
        removed.
        The second string is for the details view at the bottom. It shows the attribute/value
        pairs on different lines. All escape characters ('"') are removed.
    */
    pair< OUString, OUString> GetDNForCertDetailsView( const OUString & rRawString)
    {
        vector< pair< OUString, OUString > > vecAttrValueOfDN = parseDN(rRawString);
        OUStringBuffer s1, s2;
        typedef vector< pair < OUString, OUString > >::const_iterator CIT;
        for (CIT i = vecAttrValueOfDN.begin(); i < vecAttrValueOfDN.end(); ++i)
        {
            if (i != vecAttrValueOfDN.begin())
            {
                s1.append(',');
                s2.append('\n');
            }
            s1.append(i->second);
            s2.append(i->first + " = " + i->second);
        }
        return make_pair(s1.makeStringAndClear(), s2.makeStringAndClear());
    }

/*
    Whenever the attribute value contains special characters, such as '"' or ',' (without '')
    then the value will be enclosed in double quotes by the respective Windows or NSS function
    which we use to retrieve, for example, the subject name. If double quotes appear in the value then
    they are escaped with a double quote. This function removes the escape characters.
*/
#ifdef _WIN32
vector< pair< OUString, OUString> > parseDN(const OUString& rRawString)
{
        vector< pair<OUString, OUString> > retVal;
        bool bInEscape = false;
        bool bInValue = false;
        bool bInType = true;
        sal_Int32 nTypeNameStart = 0;
        OUString sType;
        OUStringBuffer sbufValue;
        sal_Int32 length = rRawString.getLength();

        for (sal_Int32 i = 0; i < length; i++)
        {
            sal_Unicode c = rRawString[i];

            if (c == '=')
            {
                if (! bInValue)
                {
                    sType = rRawString.copy(nTypeNameStart, i - nTypeNameStart);
                    sType = sType.trim();
                    bInType = false;
                }
                else
                {
                    sbufValue.append(c);
                }
            }
            else if (c == '"')
            {
                if (!bInEscape)
                {
                    //If this is the quote is the first of the couple which enclose the
                    //whole value, because the value contains special characters
                    //then we just drop it. That is, this character must be followed by
                    //a character which is not '"'.
                    if ( i + 1 < length && rRawString[i+1] == '"')
                        bInEscape = true;
                    else
                        bInValue = !bInValue; //value is enclosed in " "
                }
                else
                {
                    //This quote is escaped by a preceding quote and therefore is
                    //part of the value
                    sbufValue.append(c);
                    bInEscape = false;
                }
            }
            else if (c == ',' || c == '+')
            {
                //The comma separate the attribute value pairs.
                //If the comma is not part of a value (the value would then be enclosed in '"'),
                //then we have reached the end of the value
                if (!bInValue)
                {
                    OSL_ASSERT(!sType.isEmpty());
                    retVal.push_back(make_pair(sType, sbufValue.makeStringAndClear()));
                    sType.clear();
                    //The next char is the start of the new type
                    nTypeNameStart = i + 1;
                    bInType = true;
                }
                else
                {
                    //The whole string is enclosed because it contains special characters.
                    //The enclosing '"' are not part of certificate but will be added by
                    //the function (Windows or NSS) which retrieves DN
                    sbufValue.append(c);
                }
            }
            else
            {
                if (!bInType)
                    sbufValue.append(c);
            }
        }
        if (sbufValue.getLength())
        {
            OSL_ASSERT(!sType.isEmpty());
            retVal.push_back(make_pair(sType, sbufValue.makeStringAndClear()));
        }
        return retVal;
    }
#else
vector< pair< OUString, OUString> > parseDN(const OUString& rRawString)
    {
        vector< pair<OUString, OUString> > retVal;
        //bInEscape == true means that the preceding character is an escape character
        bool bInEscape = false;
        bool bInValue = false;
        bool bInType = true;
        sal_Int32 nTypeNameStart = 0;
        OUString sType;
        OUStringBuffer sbufValue;
        sal_Int32 length = rRawString.getLength();

        for (sal_Int32 i = 0; i < length; i++)
        {
            sal_Unicode c = rRawString[i];

            if (c == '=')
            {
                if (! bInValue)
                {
                    sType = rRawString.copy(nTypeNameStart, i - nTypeNameStart);
                    sType = sType.trim();
                    bInType = false;
                }
                else
                {
                    sbufValue.append(c);
                }
            }
            else if (c == '\\')
            {
                if (!bInEscape)
                {
                    bInEscape = true;
                }
                else
                { // bInEscape is true
                    sbufValue.append(c);
                    bInEscape = false;
                }
            }
            else if (c == '"')
            {
                //an unescaped '"' is either at the beginning or end of the value
                if (!bInEscape)
                {
                    if ( !bInValue)
                        bInValue = true;
                    else if (bInValue)
                        bInValue = false;
                }
                else
                {
                    //This quote is escaped by a preceding quote and therefore is
                    //part of the value
                    sbufValue.append(c);
                    bInEscape = false;
                }
            }
            else if (c == ',' || c == '+')
            {
                //The comma separate the attribute value pairs.
                //If the comma is not part of a value (the value would then be enclosed in '"'),
                //then we have reached the end of the value
                if (!bInValue)
                {
                    OSL_ASSERT(!sType.isEmpty());
                    retVal.push_back(make_pair(sType, sbufValue.makeStringAndClear()));
                    sType.clear();
                    //The next char is the start of the new type
                    nTypeNameStart = i + 1;
                    bInType = true;
                }
                else
                {
                    //The whole string is enclosed because it contains special characters.
                    //The enclosing '"' are not part of certificate but will be added by
                    //the function (Windows or NSS) which retrieves DN
                    sbufValue.append(c);
                }
            }
            else
            {
                if (!bInType)
                {
                    sbufValue.append(c);
                    bInEscape = false;
                }
            }
        }
        if (!sbufValue.isEmpty())
        {
            OSL_ASSERT(!sType.isEmpty());
            retVal.push_back(make_pair(sType, sbufValue.makeStringAndClear()));
        }
        return retVal;
    }

#endif

    OUString GetContentPart( const OUString& _rRawString )
    {
        char const * aIDs[] = { "CN", "OU", "O", "E", nullptr };
        OUString retVal;
        int i = 0;
        vector< pair< OUString, OUString > > vecAttrValueOfDN = parseDN(_rRawString);
        while ( aIDs[i] )
        {
            OUString sPartId = OUString::createFromAscii( aIDs[i++] );
            typedef vector< pair < OUString, OUString > >::const_iterator CIT;
            for (CIT idn = vecAttrValueOfDN.begin(); idn != vecAttrValueOfDN.end(); ++idn)
            {
                if (idn->first.equals(sPartId))
                {
                    retVal = idn->second;
                    break;
                }
            }
            if (!retVal.isEmpty())
                break;
        }
        return retVal;
    }

    OUString GetHexString( const css::uno::Sequence< sal_Int8 >& _rSeq, const char* _pSep, sal_uInt16 _nLineBreak )
    {
        const sal_Int8*         pSerNumSeq = _rSeq.getConstArray();
        int                     nCnt = _rSeq.getLength();
        OUStringBuffer          aStr;
        const char              pHexDigs[ 17 ] = "0123456789ABCDEF";
        char                    pBuffer[ 3 ] = "  ";
        sal_uInt8                   nNum;
        sal_uInt16                  nBreakStart = _nLineBreak? _nLineBreak : 1;
        sal_uInt16                  nBreak = nBreakStart;
        for( int i = 0 ; i < nCnt ; ++i )
        {
            nNum = sal_uInt8( pSerNumSeq[ i ] );

            // exchange the buffer[0] and buffer[1], which make it consistent with Mozilla and Windows
            pBuffer[ 1 ] = pHexDigs[ nNum & 0x0F ];
            nNum >>= 4;
            pBuffer[ 0 ] = pHexDigs[ nNum ];
            aStr.appendAscii( pBuffer );

            --nBreak;
            if( nBreak )
                aStr.appendAscii( _pSep );
            else
            {
                nBreak = nBreakStart;
                aStr.append( '\n' );
            }
        }

        return aStr.makeStringAndClear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
