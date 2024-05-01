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

#include <comphelper/xmlsechelper.hxx>

#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <o3tl/string_view.hxx>

#include <utility>
#include <vector>

namespace comphelper::xmlsec
{
    OUString GetCertificateKind( const css::security::CertificateKind &rKind )
    {
        switch (rKind)
        {
            case css::security::CertificateKind_X509:
                return u"X.509"_ustr;
            case css::security::CertificateKind_OPENPGP:
                return u"OpenPGP"_ustr;
            default:
                return OUString();
        }
    }

    /*
        Creates two strings based on the distinguished name which are displayed in the
        certificate details view. The first string contains only the values of the attribute
        and values pairs, which are separated by commas. All escape characters ('"') are
        removed.
        The second string is for the details view at the bottom. It shows the attribute/value
        pairs on different lines. All escape characters ('"') are removed.
    */
    std::pair< OUString, OUString> GetDNForCertDetailsView( std::u16string_view rRawString)
    {
        std::vector< std::pair< OUString, OUString > > vecAttrValueOfDN = parseDN(rRawString);
        OUStringBuffer s1, s2;
        for (auto i = vecAttrValueOfDN.cbegin(); i < vecAttrValueOfDN.cend(); ++i)
        {
            if (i != vecAttrValueOfDN.cbegin())
            {
                s1.append(',');
                s2.append('\n');
            }
            s1.append(i->second);
            s2.append(i->first + " = " + i->second);
        }
        return std::make_pair(s1.makeStringAndClear(), s2.makeStringAndClear());
    }

/*
    Whenever the attribute value contains special characters, such as '"' or ',' (without '')
    then the value will be enclosed in double quotes by the respective Windows or NSS function
    which we use to retrieve, for example, the subject name. If double quotes appear in the value then
    they are escaped with a double quote. This function removes the escape characters.
*/
#ifdef _WIN32
std::vector< std::pair< OUString, OUString> > parseDN(std::u16string_view rRawString)
{
        std::vector< std::pair<OUString, OUString> > retVal;
        bool bInEscape = false;
        bool bInValue = false;
        bool bInType = true;
        sal_Int32 nTypeNameStart = 0;
        std::u16string_view sType;
        OUStringBuffer sbufValue;
        size_t length = rRawString.size();

        for (size_t i = 0; i < length; i++)
        {
            sal_Unicode c = rRawString[i];

            if (c == '=')
            {
                if (! bInValue)
                {
                    sType = rRawString.substr(nTypeNameStart, i - nTypeNameStart);
                    sType = o3tl::trim(sType);
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
                    OSL_ASSERT(!sType.empty());
                    retVal.emplace_back(OUString(sType), sbufValue.makeStringAndClear());
                    sType = {};
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
            OSL_ASSERT(!sType.empty());
            retVal.emplace_back(OUString(sType), sbufValue.makeStringAndClear());
        }
        return retVal;
    }
#else
std::vector< std::pair< OUString, OUString> > parseDN(std::u16string_view rRawString)
    {
        std::vector< std::pair<OUString, OUString> > retVal;
        //bInEscape == true means that the preceding character is an escape character
        bool bInEscape = false;
        bool bInValue = false;
        bool bInType = true;
        sal_Int32 nTypeNameStart = 0;
        std::u16string_view sType;
        OUStringBuffer sbufValue;
        size_t length = rRawString.size();

        for (size_t i = 0; i < length; i++)
        {
            sal_Unicode c = rRawString[i];

            if (c == '=')
            {
                if (! bInValue)
                {
                    sType = rRawString.substr(nTypeNameStart, i - nTypeNameStart);
                    sType = o3tl::trim(sType);
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
                    OSL_ASSERT(!sType.empty());
                    retVal.emplace_back(sType, sbufValue.makeStringAndClear());
                    sType = {};
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
            OSL_ASSERT(!sType.empty());
            retVal.emplace_back(sType, sbufValue.makeStringAndClear());
        }
        return retVal;
    }

#endif

    OUString GetContentPart( const OUString& _rRawString, const css::security::CertificateKind &rKind )
    {
        static constexpr OUString aIDs[] { u"CN"_ustr, u"OU"_ustr, u"O"_ustr, u"E"_ustr };

        // tdf#131733 Don't process OpenPGP certs, only X509
        if (rKind == css::security::CertificateKind_OPENPGP )
            return _rRawString;

        OUString retVal;
        std::vector< std::pair< OUString, OUString > > vecAttrValueOfDN = parseDN(_rRawString);
        for ( const auto & sPartId : aIDs )
        {
            auto idn = std::find_if(vecAttrValueOfDN.cbegin(), vecAttrValueOfDN.cend(),
                [&sPartId](const std::pair< OUString, OUString >& dn) { return dn.first == sPartId; });
            if (idn != vecAttrValueOfDN.cend())
                retVal = idn->second;
            if (!retVal.isEmpty())
                break;
        }
        return retVal.isEmpty() ? _rRawString : retVal;
    }

    OUString GetHexString( const css::uno::Sequence< sal_Int8 >& _rSeq, const char* _pSep, sal_uInt16 _nLineBreak )
    {
        OUStringBuffer          aStr;
        const char              pHexDigs[ 17 ] = "0123456789ABCDEF";
        char                    pBuffer[ 3 ] = "  ";
        sal_uInt16                  nBreakStart = _nLineBreak? _nLineBreak : 1;
        sal_uInt16                  nBreak = nBreakStart;
        for (sal_Int8 n : _rSeq)
        {
            sal_uInt8 nNum = static_cast<sal_uInt8>(n);
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
