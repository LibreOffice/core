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

#include <sal/config.h>

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <tools/inetmime.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XStringWidth.hpp>
#include <o3tl/enumarray.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <rtl/character.hxx>
#include <rtl/string.h>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>

#include <string.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <sax/tools/converter.hxx>
#include <rtl/uri.hxx>

using namespace css;

//  INetURLObject

/* The URI grammar (using RFC 2234 conventions).

   Constructs of the form
       {reference <rule1> using rule2}
   stand for a rule matching the given rule1 specified in the given reference,
   encoded to URI syntax using rule2 (as specified in this URI grammar).


   ; RFC 1738, RFC 2396, RFC 2732, private
   login = [user [":" password] "@"] hostport
   user = *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ";" / "=" / "_" / "~")
   password = *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ";" / "=" / "_" / "~")
   hostport = host [":" port]
   host = incomplete-hostname / hostname / IPv4address / IPv6reference
   incomplete-hostname = *(domainlabel ".") domainlabel
   hostname = *(domainlabel ".") toplabel ["."]
   domainlabel = alphanum [*(alphanum / "-") alphanum]
   toplabel = ALPHA [*(alphanum / "-") alphanum]
   IPv4address = 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT "." 1*3DIGIT
   IPv6reference = "[" hexpart [":" IPv4address] "]"
   hexpart = (hexseq ["::" [hexseq]]) / ("::" [hexseq])
   hexseq = hex4 *(":" hex4)
   hex4 = 1*4HEXDIG
   port = *DIGIT
   escaped = "%" HEXDIG HEXDIG
   reserved = "$" / "&" / "+" / "," / "/" / ":" / ";" / "=" / "?" / "@" / "[" / "]"
   mark = "!" / "'" / "(" / ")" / "*" / "-" / "." / "_" / "~"
   alphanum = ALPHA / DIGIT
   unreserved = alphanum / mark
   uric = escaped / reserved / unreserved
   pchar = escaped / unreserved / "$" / "&" / "+" / "," / ":" / "=" / "@"


   ; RFC 1738, RFC 2396
   ftp-url = "FTP://" login ["/" segment *("/" segment) [";TYPE=" ("A" / "D" / "I")]]
   segment = *pchar


   ; RFC 1738, RFC 2396
   http-url = "HTTP://" hostport ["/" segment *("/" segment) ["?" *uric]]
   segment = *(pchar / ";")


   ; RFC 1738, RFC 2396, <http://support.microsoft.com/default.aspx?scid=KB;EN-US;Q188997&>
   file-url = "FILE://" [host / "LOCALHOST" / netbios-name] ["/" segment *("/" segment)]
   segment = *pchar
   netbios-name = 1*{<alphanum / "!" / "#" / "$" / "%" / "&" / "'" / "(" / ")" / "-" / "." / "@" / "^" / "_" / "{" / "}" / "~"> using (escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "-" / "." / "@" / "_" / "~")}


   ; RFC 2368, RFC 2396
   mailto-url = "MAILTO:" [to] [headers]
   to = {RFC 822 <#mailbox> using *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")}
   headers = "?" header *("&" header)
   header = hname "=" hvalue
   hname = {RFC 822 <field-name> using *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")} / "BODY"
   hvalue = {RFC 822 <field-body> using *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")}


   ; private (see RFC 1738, RFC 2396)
   vnd-sun-star-webdav-url = "VND.SUN.STAR.WEBDAV://" hostport ["/" segment *("/" segment) ["?" *uric]]
   segment = *(pchar / ";")


   ; private
   private-url = "PRIVATE:" path ["?" *uric]
   path = *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   vnd-sun-star-help-url = "VND.SUN.STAR.HELP://" name *("/" segment) ["?" *uric]
   name = *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~")
   segment = *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   https-url = "HTTPS://" hostport ["/" segment *("/" segment) ["?" *uric]]
   segment = *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   slot-url = "SLOT:" path ["?" *uric]
   path = *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   macro-url = "MACRO:" path ["?" *uric]
   path = *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   javascript-url = "JAVASCRIPT:" *uric


   ; RFC 2397
   data-url = "DATA:" [mediatype] [";BASE64"] "," *uric
   mediatype = [type "/" subtype] *(";" attribute "=" value)
   type = {RFC 2045 <type> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}
   subtype = {RFC 2045 <subtype> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}
   attribute = {RFC 2045 <subtype> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}
   value = {RFC 2045 <subtype> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}


   ; RFC 2392, RFC 2396
   cid-url = "CID:" {RFC 822 <addr-spec> using *uric}


   ; private
   vnd-sun-star-hier-url = "VND.SUN.STAR.HIER:" ["//"reg_name] *("/" *pchar)
   reg_name = 1*(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   uno-url = ".UNO:" path ["?" *uric]
   path = *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   component-url = ".COMPONENT:" path ["?" *uric]
   path = *(escaped / alphanum / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   vnd-sun-star-pkg-url = "VND.SUN.STAR.PKG://" reg_name *("/" *pchar) ["?" *uric]
   reg_name = 1*(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~")


   ; RFC 2255
   ldap-url = "LDAP://" [hostport] ["/" [dn ["?" [attrdesct *("," attrdesc)] ["?" ["base" / "one" / "sub"] ["?" [filter] ["?" extension *("," extension)]]]]]]
   dn = {RFC 2253 <distinguishedName> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}
   attrdesc = {RFC 2251 <AttributeDescription> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}
   filter = {RFC 2254 <filter> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}
   extension = ["!"] ["X-"] extoken ["=" exvalue]
   extoken = {RFC 2252 <oid> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")}
   exvalue = {RFC 2251 <LDAPString> using *(escaped / alphanum / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}


   ; private
   db-url = "DB:" *uric


   ; private
   vnd-sun-star-cmd-url = "VND.SUN.STAR.CMD:" opaque_part
   opaque_part = uric_no_slash *uric
   uric_no_slash = unreserved / escaped / ";" / "?" / ":" / "@" / "&" / "=" / "+" / "$" / ","


   ; RFC 1738
   telnet-url = "TELNET://" login ["/"]


   ; private
   vnd-sun-star-expand-url = "VND.SUN.STAR.EXPAND:" opaque_part
   opaque_part = uric_no_slash *uric
   uric_no_slash = unreserved / escaped / ";" / "?" / ":" / "@" / "&" / "=" / "+" / "$" / ","


   ; private
   vnd-sun-star-tdoc-url = "VND.SUN.STAR.TDOC:/" segment *("/" segment)
   segment = *pchar


   ; private
   unknown-url = scheme ":" 1*uric
   scheme = ALPHA *(alphanum / "+" / "-" / ".")


   ; private (http://ubiqx.org/cifs/Appendix-D.html):
   smb-url = "SMB://" login ["/" segment *("/" segment) ["?" *uric]]
   segment = *(pchar / ";")
 */

inline sal_Int32 INetURLObject::SubString::clear()
{
    sal_Int32 nDelta = -m_nLength;
    m_nBegin = -1;
    m_nLength = 0;
    return nDelta;
}

inline sal_Int32 INetURLObject::SubString::set(OUStringBuffer & rString,
                                       OUString const & rSubString)
{
    OUString sTemp(rString.makeStringAndClear());
    sal_Int32 nDelta = set(sTemp, rSubString);
    rString.append(sTemp);
    return nDelta;
}

inline sal_Int32 INetURLObject::SubString::set(OUString & rString,
                                       OUString const & rSubString)
{
    sal_Int32 nDelta = rSubString.getLength() - m_nLength;

    rString = rString.replaceAt(m_nBegin, m_nLength, rSubString);

    m_nLength = rSubString.getLength();
    return nDelta;
}

inline sal_Int32 INetURLObject::SubString::set(OUStringBuffer & rString,
                                       OUString const & rSubString,
                                               sal_Int32 nTheBegin)
{
    m_nBegin = nTheBegin;
    return set(rString, rSubString);
}

inline void INetURLObject::SubString::operator +=(sal_Int32 nDelta)
{
    if (isPresent())
        m_nBegin = m_nBegin + nDelta;
}

int INetURLObject::SubString::compare(SubString const & rOther,
                                      OUStringBuffer const & rThisString,
                                      OUStringBuffer const & rOtherString) const
{
    sal_Int32 len = std::min(m_nLength, rOther.m_nLength);
    sal_Unicode const * p1 = rThisString.getStr() + m_nBegin;
    sal_Unicode const * end = p1 + len;
    sal_Unicode const * p2 = rOtherString.getStr() + rOther.m_nBegin;
    while (p1 != end) {
        if (*p1 < *p2) {
            return -1;
        } else if (*p1 > *p2) {
            return 1;
        }
        ++p1;
        ++p2;
    }
    return m_nLength < rOther.m_nLength ? -1
        : m_nLength > rOther.m_nLength ? 1
        : 0;
}

struct INetURLObject::SchemeInfo
{
    sal_Char const * m_pScheme;
    sal_Char const * m_pPrefix;
    bool m_bAuthority;
    bool m_bUser;
    bool m_bAuth;
    bool m_bPassword;
    bool m_bHost;
    bool m_bPort;
    bool m_bHierarchical;
    bool m_bQuery;
};

struct INetURLObject::PrefixInfo
{
    enum Kind { OFFICIAL, INTERNAL, EXTERNAL, ALIAS }; // order is important!

    sal_Char const * m_pPrefix;
    sal_Char const * m_pTranslatedPrefix;
    INetProtocol m_eScheme;
    Kind m_eKind;
};

// static
inline INetURLObject::SchemeInfo const &
INetURLObject::getSchemeInfo(INetProtocol eTheScheme)
{
    static o3tl::enumarray<INetProtocol, SchemeInfo> const map = {
        SchemeInfo{
            "", "", false, false, false, false, false, false, false, false},
        SchemeInfo{
            "ftp", "ftp://", true, true, false, true, true, true, true,
            false},
        SchemeInfo{
            "http", "http://", true, false, false, false, true, true, true,
            true},
        SchemeInfo{
            "file", "file://", true, false, false, false, true, false, true,
            false},
        SchemeInfo{
            "mailto", "mailto:", false, false, false, false, false, false,
            false, true},
        SchemeInfo{
            "vnd.sun.star.webdav", "vnd.sun.star.webdav://", true, false,
            false, false, true, true, true, true},
        SchemeInfo{
            "private", "private:", false, false, false, false, false, false,
            false, true},
        SchemeInfo{
            "vnd.sun.star.help", "vnd.sun.star.help://", true, false, false,
            false, false, false, true, true},
        SchemeInfo{
            "https", "https://", true, false, false, false, true, true,
            true, true},
        SchemeInfo{
            "slot", "slot:", false, false, false, false, false, false, false,
            true},
        SchemeInfo{
            "macro", "macro:", false, false, false, false, false, false,
            false, true},
        SchemeInfo{
            "javascript", "javascript:", false, false, false, false, false,
            false, false, false},
        SchemeInfo{
            "data", "data:", false, false, false, false, false, false, false,
            false},
        SchemeInfo{
            "cid", "cid:", false, false, false, false, false, false, false,
            false},
        SchemeInfo{
            "vnd.sun.star.hier", "vnd.sun.star.hier:", true, false, false,
            false, false, false, true, false},
        SchemeInfo{
            ".uno", ".uno:", false, false, false, false, false, false, false,
            true},
        SchemeInfo{
            ".component", ".component:", false, false, false, false, false,
            false, false, true},
        SchemeInfo{
            "vnd.sun.star.pkg", "vnd.sun.star.pkg://", true, false, false,
            false, false, false, true, true},
        SchemeInfo{
            "ldap", "ldap://", true, false, false, false, true, true,
            false, true},
        SchemeInfo{
            "db", "db:", false, false, false, false, false, false, false,
            false},
        SchemeInfo{
            "vnd.sun.star.cmd", "vnd.sun.star.cmd:", false, false, false,
            false, false, false, false, false},
        SchemeInfo{
            "telnet", "telnet://", true, true, false, true, true, true,
            true, false},
        SchemeInfo{
            "vnd.sun.star.expand", "vnd.sun.star.expand:", false, false,
            false, false, false, false, false, false},
        SchemeInfo{
            "vnd.sun.star.tdoc", "vnd.sun.star.tdoc:", false, false, false,
            false, false, false, true, false},
        SchemeInfo{
            "", "", false, false, false, false, true, true, true, false },
        SchemeInfo{
            "smb", "smb://", true, true, false, true, true, true, true,
            true},
        SchemeInfo{
            "hid", "hid:", false, false, false, false, false, false, false,
            true},
        SchemeInfo{
            "sftp", "sftp://", true, true, false, true, true, true, true,
            true},
        SchemeInfo{
            "vnd.libreoffice.cmis", "vnd.libreoffice.cmis://", true, true,
            false, false, true, false, true, true} };
    return map[eTheScheme];
};

inline INetURLObject::SchemeInfo const & INetURLObject::getSchemeInfo() const
{
    return getSchemeInfo(m_eScheme);
}

namespace {

sal_Unicode getHexDigit(sal_uInt32 nWeight)
{
    assert(nWeight < 16);
    static const sal_Unicode aDigits[16]
        = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
            'D', 'E', 'F' };
    return aDigits[nWeight];
}

}

// static
inline void INetURLObject::appendEscape(OUStringBuffer & rTheText,
                                        sal_uInt32 nOctet)
{
    rTheText.append( '%' );
    rTheText.append( getHexDigit(nOctet >> 4) );
    rTheText.append( getHexDigit(nOctet & 15) );
}

namespace {

enum
{
    PA = INetURLObject::PART_USER_PASSWORD,
    PD = INetURLObject::PART_FPATH,
    PE = INetURLObject::PART_AUTHORITY,
    PF = INetURLObject::PART_REL_SEGMENT_EXTRA,
    PG = INetURLObject::PART_URIC,
    PH = INetURLObject::PART_HTTP_PATH,
    PI = INetURLObject::PART_MESSAGE_ID_PATH,
    PJ = INetURLObject::PART_MAILTO,
    PK = INetURLObject::PART_PATH_BEFORE_QUERY,
    PL = INetURLObject::PART_PCHAR,
    PM = INetURLObject::PART_VISIBLE,
    PN = INetURLObject::PART_VISIBLE_NONSPECIAL,
    PO = INetURLObject::PART_UNO_PARAM_VALUE,
    PP = INetURLObject::PART_UNAMBIGUOUS,
    PQ = INetURLObject::PART_URIC_NO_SLASH,
    PR = INetURLObject::PART_HTTP_QUERY,
};

static sal_uInt32 const aMustEncodeMap[128]
    = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/*   */                                              PP,
/* ! */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* " */                                  PM+PN   +PP,
/* # */                                  PM,
/* $ */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* % */                                  PM,
/* & */ PA   +PD+PE+PF+PG+PH+PI   +PK+PL+PM+PN+PO   +PQ+PR,
/* ' */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* ( */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* ) */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* * */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* + */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO   +PQ+PR,
/* , */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN      +PQ+PR,
/* - */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* . */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* / */      +PD      +PG+PH+PI+PJ+PK   +PM+PN+PO,
/* 0 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 1 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 2 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 3 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 4 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 5 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 6 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 7 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 8 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* 9 */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* : */      +PD+PE   +PG+PH+PI+PJ+PK+PL+PM+PN+PO   +PQ+PR,
/* ; */ PA      +PE+PF+PG+PH+PI+PJ+PK   +PM         +PQ+PR,
/* < */                     +PI         +PM+PN   +PP,
/* = */ PA   +PD+PE+PF+PG+PH      +PK+PL+PM+PN      +PQ+PR,
/* > */                     +PI         +PM+PN   +PP,
/* ? */               +PG               +PM   +PO   +PQ,
/* @ */      +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* A */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* B */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* C */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* D */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* E */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* F */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* G */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* H */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* I */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* J */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* K */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* L */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* M */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* N */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* O */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* P */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* Q */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* R */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* S */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* T */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* U */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* V */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* W */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* X */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* Y */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* Z */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* [ */                PG               +PM+PN+PO,
/* \ */                                 +PM+PN   +PP,
/* ] */                PG               +PM+PN+PO,
/* ^ */                                  PM+PN   +PP,
/* _ */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* ` */                                  PM+PN   +PP,
/* a */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* b */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* c */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* d */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* e */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* f */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* g */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* h */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* i */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* j */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* k */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* l */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* m */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* n */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* o */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* p */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* q */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* r */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* s */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* t */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* u */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* v */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* w */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* x */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* y */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* z */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ+PR,
/* { */                                  PM+PN   +PP,
/* | */                                 +PM+PN   +PP,
/* } */                                  PM+PN   +PP,
/* ~ */ PA   +PD+PE+PF+PG+PH+PI+PJ+PK+PL+PM+PN+PO+PP+PQ,
        0 };

inline bool mustEncode(sal_uInt32 nUTF32, INetURLObject::Part ePart)
{
    return !rtl::isAscii(nUTF32) || !(aMustEncodeMap[nUTF32] & ePart);
}

}

void INetURLObject::setInvalid()
{
    m_aAbsURIRef.setLength(0);
    m_eScheme = INetProtocol::NotValid;
    m_aScheme.clear();
    m_aUser.clear();
    m_aAuth.clear();
    m_aHost.clear();
    m_aPort.clear();
    m_aPath.clear();
    m_aQuery.clear();
    m_aFragment.clear();
}

namespace {

std::unique_ptr<SvMemoryStream> memoryStream(
        void const * data, sal_Int32 length)
{
    std::unique_ptr<char[]> b(
        new char[length]);
    memcpy(b.get(), data, length);
    std::unique_ptr<SvMemoryStream> s(
        new SvMemoryStream(b.get(), length, StreamMode::READ));
    s->ObjectOwnsMemory(true);
    b.release();
    return s;
}

}

std::unique_ptr<SvMemoryStream> INetURLObject::getData()
{
    if( GetProtocol() != INetProtocol::Data )
    {
        return nullptr;
    }

    OUString sURLPath = GetURLPath( DecodeMechanism::WithCharset, RTL_TEXTENCODING_ISO_8859_1 );
    sal_Unicode const * pSkippedMediatype = INetMIME::scanContentType( sURLPath.getStr(), sURLPath.getStr() + sURLPath.getLength() );
    sal_Int32 nCharactersSkipped = pSkippedMediatype == nullptr
        ? 0 : pSkippedMediatype-sURLPath.getStr();
    if (sURLPath.match(",", nCharactersSkipped))
    {
        nCharactersSkipped += strlen(",");
        OString sURLEncodedData(
            sURLPath.getStr() + nCharactersSkipped,
            sURLPath.getLength() - nCharactersSkipped,
            RTL_TEXTENCODING_ISO_8859_1, OUSTRING_TO_OSTRING_CVTFLAGS);
        return memoryStream(
            sURLEncodedData.getStr(), sURLEncodedData.getLength());
    }
    else if (sURLPath.matchIgnoreAsciiCase(";base64,", nCharactersSkipped))
    {
        nCharactersSkipped += strlen(";base64,");
        OUString sBase64Data = sURLPath.copy( nCharactersSkipped );
        css::uno::Sequence< sal_Int8 > aDecodedData;
        if (sax::Converter::decodeBase64SomeChars(aDecodedData, sBase64Data)
            == sBase64Data.getLength())
        {
            return memoryStream(
                aDecodedData.getArray(), aDecodedData.getLength());
        }
    }
    return nullptr;
}

namespace {

FSysStyle guessFSysStyleByCounting(sal_Unicode const * pBegin,
                                                  sal_Unicode const * pEnd,
                                                  FSysStyle eStyle)
{
    DBG_ASSERT(eStyle
                   & (FSysStyle::Unix
                          | FSysStyle::Dos),
               "guessFSysStyleByCounting(): Bad style");
    DBG_ASSERT(std::numeric_limits< sal_Int32 >::min() < pBegin - pEnd
               && pEnd - pBegin <= std::numeric_limits< sal_Int32 >::max(),
               "guessFSysStyleByCounting(): Too big");
    sal_Int32 nSlashCount
        = (eStyle & FSysStyle::Unix) ?
              0 : std::numeric_limits< sal_Int32 >::min();
    sal_Int32 nBackslashCount
        = (eStyle & FSysStyle::Dos) ?
              0 : std::numeric_limits< sal_Int32 >::min();
    while (pBegin != pEnd)
        switch (*pBegin++)
        {
            case '/':
                ++nSlashCount;
                break;

            case '\\':
                ++nBackslashCount;
                break;
        }
    return nSlashCount >= nBackslashCount ?
                   FSysStyle::Unix : FSysStyle::Dos;
}

OUString parseScheme(
    sal_Unicode const ** begin, sal_Unicode const * end,
    sal_uInt32 fragmentDelimiter)
{
    sal_Unicode const * p = *begin;
    if (p != end && rtl::isAsciiAlpha(*p)) {
        do {
            ++p;
        } while (p != end
                 && (rtl::isAsciiAlphanumeric(*p) || *p == '+' || *p == '-'
                     || *p == '.'));
        // #i34835# To avoid problems with Windows file paths like "C:\foo",
        // do not accept generic schemes that are only one character long:
        if (end - p > 1 && p[0] == ':' && p[1] != fragmentDelimiter
            && p - *begin >= 2)
        {
            OUString scheme(
                OUString(*begin, p - *begin).toAsciiLowerCase());
            *begin = p + 1;
            return scheme;
        }
    }
    return OUString();
}

}

bool INetURLObject::setAbsURIRef(OUString const & rTheAbsURIRef,
                                 EncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset,
                                 bool bSmart,
                                 FSysStyle eStyle)
{
    const bool bOctets = false;
    sal_Unicode const * pPos = rTheAbsURIRef.getStr();
    sal_Unicode const * pEnd = pPos + rTheAbsURIRef.getLength();

    setInvalid();

    sal_uInt32 nFragmentDelimiter = '#';

    OUStringBuffer aSynAbsURIRef;

    // Parse <scheme>:
    sal_Unicode const * p = pPos;
    PrefixInfo const * pPrefix = getPrefix(p, pEnd);
    if (pPrefix)
    {
        pPos = p;
        m_eScheme = pPrefix->m_eScheme;

        OUString sTemp(OUString::createFromAscii(pPrefix->m_eKind
                                                 >= PrefixInfo::EXTERNAL ?
                                             pPrefix->m_pTranslatedPrefix :
                                             pPrefix->m_pPrefix));
        aSynAbsURIRef.append(sTemp);
        m_aScheme = SubString( 0, sTemp.indexOf(':') );
    }
    else
    {
        if (bSmart)
        {
            // For scheme detection, the first (if any) of the following
            // productions that matches the input string (and for which the
            // appropriate style bit is set in eStyle, if applicable)
            // determines the scheme. The productions use the auxiliary rules

            //    domain = label *("." label)
            //    label = alphanum [*(alphanum / "-") alphanum]
            //    alphanum = ALPHA / DIGIT
            //    IPv6reference = "[" IPv6address "]"
            //    IPv6address = hexpart [":" IPv4address]
            //    IPv4address = 1*3DIGIT 3("." 1*3DIGIT)
            //    hexpart = (hexseq ["::" [hexseq]]) / ("::" [hexseq])
            //    hexseq = hex4 *(":" hex4)
            //    hex4 = 1*4HEXDIG
            //    UCS4 = <any UCS4 character>

            // 1st Production (known scheme):
            //    <one of the known schemes, ignoring case> ":" *UCS4
            // 2nd Production (mailto):
            //    domain "@" domain
            // 3rd Production (ftp):
            //    "FTP" 2*("." label) ["/" *UCS4]
            // 4th Production (http):
            //    label 2*("." label) ["/" *UCS4]
            // 5th Production (file):
            //    "//" (domain / IPv6reference) ["/" *UCS4]
            // 6th Production (Unix file):
            //    "/" *UCS4
            // 7th Production (UNC file; FSysStyle::Dos only):
            //    "\\" domain ["\" *UCS4]
            // 8th Production (Unix-like DOS file; FSysStyle::Dos only):
            //    ALPHA ":" ["/" *UCS4]
            // 9th Production (DOS file; FSysStyle::Dos only):
            //    ALPHA ":" ["\" *UCS4]

            // For the 'non URL' file productions 6--9, the interpretation of
            // the input as a (degenerate) URI is turned off, i.e., escape
            // sequences and fragments are never detected as such, but are
            // taken as literal characters.

            sal_Unicode const * p1 = pPos;
            if (eStyle & FSysStyle::Dos
                && pEnd - p1 >= 2
                && rtl::isAsciiAlpha(p1[0])
                && p1[1] == ':'
                && (pEnd - p1 == 2 || p1[2] == '/' || p1[2] == '\\'))
            {
                m_eScheme = INetProtocol::File; // 8th, 9th
                eMechanism = EncodeMechanism::All;
                nFragmentDelimiter = 0x80000000;
            }
            else if (pEnd - p1 >= 2 && p1[0] == '/' && p1[1] == '/')
            {
                p1 += 2;
                if ((scanDomain(p1, pEnd) > 0 || scanIPv6reference(p1, pEnd))
                    && (p1 == pEnd || *p1 == '/'))
                    m_eScheme = INetProtocol::File; // 5th
            }
            else if (p1 != pEnd && *p1 == '/')
            {
                m_eScheme = INetProtocol::File; // 6th
                eMechanism = EncodeMechanism::All;
                nFragmentDelimiter = 0x80000000;
            }
            else if (eStyle & FSysStyle::Dos
                     && pEnd - p1 >= 2
                     && p1[0] == '\\'
                     && p1[1] == '\\')
            {
                p1 += 2;
                sal_Int32 n = rtl_ustr_indexOfChar_WithLength(
                    p1, pEnd - p1, '\\');
                sal_Unicode const * pe = n == -1 ? pEnd : p1 + n;
                if (
                    parseHostOrNetBiosName(
                        p1, pe, bOctets, EncodeMechanism::All, RTL_TEXTENCODING_DONTKNOW,
                        true, nullptr) ||
                    (scanDomain(p1, pe) > 0 && p1 == pe)
                   )
                {
                    m_eScheme = INetProtocol::File; // 7th
                    eMechanism = EncodeMechanism::All;
                    nFragmentDelimiter = 0x80000000;
                }
            }
            else
            {
                sal_Unicode const * pDomainEnd = p1;
                sal_uInt32 nLabels = scanDomain(pDomainEnd, pEnd);
                if (nLabels > 0 && pDomainEnd != pEnd && *pDomainEnd == '@')
                {
                    ++pDomainEnd;
                    if (scanDomain(pDomainEnd, pEnd) > 0
                        && pDomainEnd == pEnd)
                        m_eScheme = INetProtocol::Mailto; // 2nd
                }
                else if (nLabels >= 3
                         && (pDomainEnd == pEnd || *pDomainEnd == '/'))
                    m_eScheme
                        = pDomainEnd - p1 >= 4
                          && (p1[0] == 'f' || p1[0] == 'F')
                          && (p1[1] == 't' || p1[1] == 'T')
                          && (p1[2] == 'p' || p1[2] == 'P')
                          && p1[3] == '.' ?
                              INetProtocol::Ftp : INetProtocol::Http; // 3rd, 4th
            }
        }

        OUString aSynScheme;
        if (m_eScheme == INetProtocol::NotValid) {
            sal_Unicode const * p1 = pPos;
            aSynScheme = parseScheme(&p1, pEnd, nFragmentDelimiter);
            if (!aSynScheme.isEmpty())
            {
                m_eScheme = INetProtocol::Generic;
                pPos = p1;
            }
        }

        if (bSmart && m_eScheme == INetProtocol::NotValid && pPos != pEnd
            && *pPos != nFragmentDelimiter)
        {
            m_eScheme = m_eSmartScheme;
        }

        if (m_eScheme == INetProtocol::NotValid)
        {
            setInvalid();
            return false;
        }

        if (m_eScheme != INetProtocol::Generic) {
            aSynScheme = OUString::createFromAscii(getSchemeInfo().m_pScheme);
        }
        m_aScheme.set(aSynAbsURIRef, aSynScheme, aSynAbsURIRef.getLength());
        aSynAbsURIRef.append(':');
    }

    sal_uInt32 nSegmentDelimiter = '/';
    sal_uInt32 nAltSegmentDelimiter = 0x80000000;
    bool bSkippedInitialSlash = false;

    // Parse //<user>;AUTH=<auth>@<host>:<port> or
    // //<user>:<password>@<host>:<port> or
    // //<reg_name>
    if (getSchemeInfo().m_bAuthority)
    {
        sal_Unicode const * pUserInfoBegin = nullptr;
        sal_Unicode const * pUserInfoEnd = nullptr;
        sal_Unicode const * pHostPortBegin = nullptr;
        sal_Unicode const * pHostPortEnd = nullptr;

        switch (m_eScheme)
        {
            case INetProtocol::VndSunStarHelp:
            {
                if (pEnd - pPos < 2 || *pPos++ != '/' || *pPos++ != '/')
                {
                    setInvalid();
                    return false;
                }
                aSynAbsURIRef.append("//");
                OUStringBuffer aSynAuthority;
                while (pPos < pEnd
                       && *pPos != '/' && *pPos != '?'
                       && *pPos != nFragmentDelimiter)
                {
                    EscapeType eEscapeType;
                    sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                                 eMechanism,
                                                 eCharset, eEscapeType);
                    appendUCS4(aSynAuthority, nUTF32, eEscapeType, bOctets,
                               PART_AUTHORITY, eCharset, false);
                }
                m_aHost.set(aSynAbsURIRef,
                            aSynAuthority.makeStringAndClear(),
                            aSynAbsURIRef.getLength());
                    // misusing m_aHost to store the authority
                break;
            }

            case INetProtocol::VndSunStarHier:
            {
                if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
                {
                    pPos += 2;
                    aSynAbsURIRef.append("//");
                    OUStringBuffer aSynAuthority;
                    while (pPos < pEnd
                           && *pPos != '/' && *pPos != '?'
                           && *pPos != nFragmentDelimiter)
                    {
                        EscapeType eEscapeType;
                        sal_uInt32 nUTF32 = getUTF32(pPos,
                                                     pEnd,
                                                     bOctets,
                                                     eMechanism,
                                                     eCharset,
                                                     eEscapeType);
                        appendUCS4(aSynAuthority,
                                   nUTF32,
                                   eEscapeType,
                                   bOctets,
                                   PART_AUTHORITY,
                                   eCharset,
                                   false);
                    }
                    if (aSynAuthority.isEmpty())
                    {
                        setInvalid();
                        return false;
                    }
                    m_aHost.set(aSynAbsURIRef,
                                aSynAuthority.makeStringAndClear(),
                                aSynAbsURIRef.getLength());
                        // misusing m_aHost to store the authority
                }
                break;
            }

            case INetProtocol::VndSunStarPkg:
            case INetProtocol::Cmis:
            {
                if (pEnd - pPos < 2 || *pPos++ != '/' || *pPos++ != '/')
                {
                    setInvalid();
                    return false;
                }
                aSynAbsURIRef.append("//");
                OUStringBuffer aSynUser;

                bool bHasUser = false;
                while (pPos < pEnd && *pPos != '@'
                       && *pPos != '/' && *pPos != '?'
                       && *pPos != nFragmentDelimiter)
                {
                    EscapeType eEscapeType;
                    sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                                 eMechanism,
                                                 eCharset, eEscapeType);
                    appendUCS4(aSynUser, nUTF32, eEscapeType, bOctets,
                               PART_USER_PASSWORD, eCharset, false);

                    bHasUser = *pPos == '@';
                }

                OUStringBuffer aSynAuthority;
                if ( !bHasUser )
                {
                    aSynAuthority = aSynUser;
                }
                else
                {
                    m_aUser.set(aSynAbsURIRef,
                            aSynUser.makeStringAndClear(),
                            aSynAbsURIRef.getLength());
                    aSynAbsURIRef.append("@");
                    ++pPos;

                    while (pPos < pEnd
                           && *pPos != '/' && *pPos != '?'
                           && *pPos != nFragmentDelimiter)
                    {
                        EscapeType eEscapeType;
                        sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                                     eMechanism,
                                                     eCharset, eEscapeType);
                        appendUCS4(aSynAuthority, nUTF32, eEscapeType, bOctets,
                                   PART_AUTHORITY, eCharset, false);
                    }
                }
                if (aSynAuthority.isEmpty())
                {
                    setInvalid();
                    return false;
                }
                m_aHost.set(aSynAbsURIRef,
                            aSynAuthority.makeStringAndClear(),
                            aSynAbsURIRef.getLength());
                    // misusing m_aHost to store the authority
                break;
            }

            case INetProtocol::File:
                if (bSmart)
                {
                    // The first of the following seven productions that
                    // matches the rest of the input string (and for which the
                    // appropriate style bit is set in eStyle, if applicable)
                    // determines the used notation.  The productions use the
                    // auxiliary rules

                    //    domain = label *("." label)
                    //    label = alphanum [*(alphanum / "-") alphanum]
                    //    alphanum = ALPHA / DIGIT
                    //    IPv6reference = "[" IPv6address "]"
                    //    IPv6address = hexpart [":" IPv4address]
                    //    IPv4address = 1*3DIGIT 3("." 1*3DIGIT)
                    //    hexpart = (hexseq ["::" [hexseq]]) / ("::" [hexseq])
                    //    hexseq = hex4 *(":" hex4)
                    //    hex4 = 1*4HEXDIG
                    //    path = <any UCS4 character except "#">
                    //    UCS4 = <any UCS4 character>

                    // 1st Production (URL):
                    //    "//" [domain / IPv6reference] ["/" *path]
                    //        ["#" *UCS4]
                    //  becomes
                    //    "file://" domain "/" *path ["#" *UCS4]
                    if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
                    {
                        sal_Unicode const * p1 = pPos + 2;
                        while (p1 != pEnd && *p1 != '/' &&
                               *p1 != nFragmentDelimiter)
                        {
                            ++p1;
                        }
                        if (parseHostOrNetBiosName(
                                pPos + 2, p1, bOctets, EncodeMechanism::All,
                                RTL_TEXTENCODING_DONTKNOW, true, nullptr))
                        {
                            aSynAbsURIRef.append("//");
                            pHostPortBegin = pPos + 2;
                            pHostPortEnd = p1;
                            pPos = p1;
                            break;
                        }
                    }

                    // 2nd Production (MS IE generated 1; FSysStyle::Dos only):
                    //    "//" ALPHA ":" ["/" *path] ["#" *UCS4]
                    //  becomes
                    //    "file:///" ALPHA ":" ["/" *path] ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    // 3rd Production (MS IE generated 2; FSysStyle::Dos only):
                    //    "//" ALPHA ":" ["\" *path] ["#" *UCS4]
                    //  becomes
                    //    "file:///" ALPHA ":" ["/" *path] ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    // 4th Production (miscounted slashes):
                    //    "//" *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" *path ["#" *UCS4]
                    if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
                    {
                        aSynAbsURIRef.append("//");
                        pPos += 2;
                        bSkippedInitialSlash = true;
                        if ((eStyle & FSysStyle::Dos)
                            && pEnd - pPos >= 2
                            && rtl::isAsciiAlpha(pPos[0])
                            && pPos[1] == ':'
                            && (pEnd - pPos == 2
                                || pPos[2] == '/' || pPos[2] == '\\'))
                            nAltSegmentDelimiter = '\\';
                        break;
                    }

                    // 5th Production (Unix):
                    //    "/" *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" *path ["#" *UCS4]
                    if (pPos < pEnd && *pPos == '/')
                    {
                        aSynAbsURIRef.append("//");
                        break;
                    }

                    // 6th Production (UNC; FSysStyle::Dos only):
                    //    "\\" domain ["\" *path] ["#" *UCS4]
                    //  becomes
                    //    "file://" domain "/" *path ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    if (eStyle & FSysStyle::Dos
                        && pEnd - pPos >= 2
                        && pPos[0] == '\\'
                        && pPos[1] == '\\')
                    {
                        sal_Unicode const * p1 = pPos + 2;
                        sal_Unicode const * pe = p1;
                        while (pe < pEnd && *pe != '\\' &&
                               *pe != nFragmentDelimiter)
                        {
                            ++pe;
                        }
                        if (
                             parseHostOrNetBiosName(
                                p1, pe, bOctets, EncodeMechanism::All,
                                RTL_TEXTENCODING_DONTKNOW, true, nullptr) ||
                             (scanDomain(p1, pe) > 0 && p1 == pe)
                           )
                        {
                            aSynAbsURIRef.append("//");
                            pHostPortBegin = pPos + 2;
                            pHostPortEnd = pe;
                            pPos = pe;
                            nSegmentDelimiter = '\\';
                            break;
                        }
                    }

                    // 7th Production (Unix-like DOS; FSysStyle::Dos only):
                    //    ALPHA ":" ["/" *path] ["#" *UCS4]
                    //  becomes
                    //    "file:///" ALPHA ":" ["/" *path] ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    // 8th Production (DOS; FSysStyle::Dos only):
                    //    ALPHA ":" ["\" *path] ["#" *UCS4]
                    //  becomes
                    //    "file:///" ALPHA ":" ["/" *path] ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    if (eStyle & FSysStyle::Dos
                        && pEnd - pPos >= 2
                        && rtl::isAsciiAlpha(pPos[0])
                        && pPos[1] == ':'
                        && (pEnd - pPos == 2
                            || pPos[2] == '/'
                            || pPos[2] == '\\'))
                    {
                        aSynAbsURIRef.append("//");
                        nAltSegmentDelimiter = '\\';
                        bSkippedInitialSlash = true;
                        break;
                    }

                    // 9th Production (any):
                    //    *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" *path ["#" *UCS4]
                    //  replacing the delimiter by "/" within <*path>.  The
                    //  delimiter is that character from the set { "/", "\"}
                    // which appears most often in <*path> (if FSysStyle::Unix
                    //  is not among the style bits, "/" is removed from the
                    //  set; if FSysStyle::Dos is not among the style bits, "\" is
                    //  removed from the set).  If two or
                    //  more characters appear the same number of times, the
                    //  character mentioned first in that set is chosen.  If
                    //  the first character of <*path> is the delimiter, that
                    //  character is not copied
                    if (eStyle & (FSysStyle::Unix | FSysStyle::Dos))
                    {
                        aSynAbsURIRef.append("//");
                        switch (guessFSysStyleByCounting(pPos, pEnd, eStyle))
                        {
                            case FSysStyle::Unix:
                                nSegmentDelimiter = '/';
                                break;

                            case FSysStyle::Dos:
                                nSegmentDelimiter = '\\';
                                break;

                            default:
                                OSL_FAIL(
                                    "INetURLObject::setAbsURIRef():"
                                        " Bad guessFSysStyleByCounting");
                                break;
                        }
                        bSkippedInitialSlash
                            = pPos != pEnd && *pPos != nSegmentDelimiter;
                        break;
                    }
                }
                SAL_FALLTHROUGH;
            default:
            {
                // For INetProtocol::File, allow an empty authority ("//") to be
                // missing if the following path starts with an explicit "/"
                // (Java is notorious in generating such file URLs, so be
                // liberal here):
                if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
                    pPos += 2;
                else if (!bSmart
                         && !(m_eScheme == INetProtocol::File
                              && pPos != pEnd && *pPos == '/'))
                {
                    setInvalid();
                    return false;
                }
                aSynAbsURIRef.append("//");

                sal_Unicode const * pAuthority = pPos;
                sal_uInt32 c = getSchemeInfo().m_bQuery ? '?' : 0x80000000;
                while (pPos < pEnd && *pPos != '/' && *pPos != c
                       && *pPos != nFragmentDelimiter)
                    ++pPos;
                if (getSchemeInfo().m_bUser)
                    if (getSchemeInfo().m_bHost)
                    {
                        sal_Unicode const * p1 = pAuthority;
                        while (p1 < pPos && *p1 != '@')
                            ++p1;
                        if (p1 == pPos)
                        {
                            pHostPortBegin = pAuthority;
                            pHostPortEnd = pPos;
                        }
                        else
                        {
                            pUserInfoBegin = pAuthority;
                            pUserInfoEnd = p1;
                            pHostPortBegin = p1 + 1;
                            pHostPortEnd = pPos;
                        }
                    }
                    else
                    {
                        pUserInfoBegin = pAuthority;
                        pUserInfoEnd = pPos;
                    }
                else if (getSchemeInfo().m_bHost)
                {
                    pHostPortBegin = pAuthority;
                    pHostPortEnd = pPos;
                }
                else if (pPos != pAuthority)
                {
                    setInvalid();
                    return false;
                }
                break;
            }
        }

        if (pUserInfoBegin)
        {
            Part ePart = PART_USER_PASSWORD;
            bool bSupportsPassword = getSchemeInfo().m_bPassword;
            bool bSupportsAuth
                = !bSupportsPassword && getSchemeInfo().m_bAuth;
            bool bHasAuth = false;
            OUStringBuffer aSynUser;
            sal_Unicode const * p1 = pUserInfoBegin;
            while (p1 < pUserInfoEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(p1, pUserInfoEnd, bOctets,
                                             eMechanism, eCharset, eEscapeType);
                if (eEscapeType == EscapeType::NONE)
                {
                    if (nUTF32 == ':' && bSupportsPassword)
                    {
                        bHasAuth = true;
                        break;
                    }
                    else if (nUTF32 == ';' && bSupportsAuth
                             && pUserInfoEnd - p1
                                    > RTL_CONSTASCII_LENGTH("auth=")
                             && INetMIME::equalIgnoreCase(
                                    p1,
                                    p1 + RTL_CONSTASCII_LENGTH("auth="),
                                    "auth="))
                    {
                        p1 += RTL_CONSTASCII_LENGTH("auth=");
                        bHasAuth = true;
                        break;
                    }
                }
                appendUCS4(aSynUser, nUTF32, eEscapeType, bOctets, ePart,
                           eCharset, false);
            }
            m_aUser.set(aSynAbsURIRef, aSynUser.makeStringAndClear(),
                aSynAbsURIRef.getLength());
            if (bHasAuth)
            {
                if (bSupportsPassword)
                {
                    aSynAbsURIRef.append(':');
                    OUStringBuffer aSynAuth;
                    while (p1 < pUserInfoEnd)
                    {
                        EscapeType eEscapeType;
                        sal_uInt32 nUTF32 = getUTF32(p1, pUserInfoEnd, bOctets,
                                                     eMechanism, eCharset,
                                                     eEscapeType);
                        appendUCS4(aSynAuth, nUTF32, eEscapeType, bOctets,
                                   ePart, eCharset, false);
                    }
                    m_aAuth.set(aSynAbsURIRef, aSynAuth.makeStringAndClear(),
                        aSynAbsURIRef.getLength());
                }
                else
                {
                    aSynAbsURIRef.append(";AUTH=");
                    OUStringBuffer aSynAuth;
                    while (p1 < pUserInfoEnd)
                    {
                        EscapeType eEscapeType;
                        sal_uInt32 nUTF32 = getUTF32(p1, pUserInfoEnd, bOctets,
                                                     eMechanism, eCharset,
                                                     eEscapeType);
                        if (!INetMIME::isIMAPAtomChar(nUTF32))
                        {
                            setInvalid();
                            return false;
                        }
                        appendUCS4(aSynAuth, nUTF32, eEscapeType, bOctets,
                                   ePart, eCharset, false);
                    }
                    m_aAuth.set(aSynAbsURIRef, aSynAuth.makeStringAndClear(),
                        aSynAbsURIRef.getLength());
                }
            }
            if (pHostPortBegin)
                aSynAbsURIRef.append('@');
        }

        if (pHostPortBegin)
        {
            sal_Unicode const * pPort = pHostPortEnd;
            if ( getSchemeInfo().m_bPort && pHostPortBegin < pHostPortEnd )
            {
                sal_Unicode const * p1 = pHostPortEnd - 1;
                while (p1 > pHostPortBegin && rtl::isAsciiDigit(*p1))
                    --p1;
                if (*p1 == ':')
                    pPort = p1;
            }
            bool bNetBiosName = false;
            switch (m_eScheme)
            {
                case INetProtocol::File:
                    // If the host equals "LOCALHOST" (unencoded and ignoring
                    // case), turn it into an empty host:
                    if (INetMIME::equalIgnoreCase(pHostPortBegin, pPort,
                                                  "localhost"))
                        pHostPortBegin = pPort;
                    bNetBiosName = true;
                    break;

                case INetProtocol::Ldap:
                case INetProtocol::Smb:
                    if (pHostPortBegin == pPort && pPort != pHostPortEnd)
                    {
                        setInvalid();
                        return false;
                    }
                    break;
                default:
                    if (pHostPortBegin == pPort)
                    {
                        setInvalid();
                        return false;
                    }
                    break;
            }
            OUStringBuffer aSynHost;
            if (!parseHostOrNetBiosName(
                    pHostPortBegin, pPort, bOctets, eMechanism, eCharset,
                    bNetBiosName, &aSynHost))
            {
                setInvalid();
                return false;
            }
            m_aHost.set(aSynAbsURIRef, aSynHost.makeStringAndClear(),
                aSynAbsURIRef.getLength());
            if (pPort != pHostPortEnd)
            {
                aSynAbsURIRef.append(':');
                m_aPort.set(aSynAbsURIRef,
                    OUString(pPort + 1, pHostPortEnd - (pPort + 1)),
                    aSynAbsURIRef.getLength());
            }
        }
    }

    // Parse <path>
    OUStringBuffer aSynPath;
    if (!parsePath(m_eScheme, &pPos, pEnd, bOctets, eMechanism, eCharset,
                   bSkippedInitialSlash, nSegmentDelimiter,
                   nAltSegmentDelimiter,
                   getSchemeInfo().m_bQuery ? '?' : 0x80000000,
                   nFragmentDelimiter, aSynPath))
    {
        setInvalid();
        return false;
    }
    m_aPath.set(aSynAbsURIRef, aSynPath.makeStringAndClear(),
        aSynAbsURIRef.getLength());

    // Parse ?<query>
    if (getSchemeInfo().m_bQuery && pPos < pEnd && *pPos == '?')
    {
        aSynAbsURIRef.append('?');
        OUStringBuffer aSynQuery;
        for (++pPos; pPos < pEnd && *pPos != nFragmentDelimiter;)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                         eMechanism, eCharset, eEscapeType);
            appendUCS4(aSynQuery, nUTF32, eEscapeType, bOctets,
                       PART_URIC, eCharset, true);
        }
        m_aQuery.set(aSynAbsURIRef, aSynQuery.makeStringAndClear(),
            aSynAbsURIRef.getLength());
    }

    // Parse #<fragment>
    if (pPos < pEnd && *pPos == nFragmentDelimiter)
    {
        aSynAbsURIRef.append(sal_Unicode(nFragmentDelimiter));
        OUStringBuffer aSynFragment;
        for (++pPos; pPos < pEnd;)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                         eMechanism, eCharset, eEscapeType);
            appendUCS4(aSynFragment, nUTF32, eEscapeType, bOctets, PART_URIC,
                       eCharset, true);
        }
        m_aFragment.set(aSynAbsURIRef, aSynFragment.makeStringAndClear(),
            aSynAbsURIRef.getLength());
    }

    if (pPos != pEnd)
    {
        setInvalid();
        return false;
    }

    m_aAbsURIRef = aSynAbsURIRef;

    // At this point references of type "\\server\paths" have
    // been converted to file:://server/path".
#ifdef LINUX
    if (m_eScheme==INetProtocol::File && !m_aHost.isEmpty()) {
        // Change "file:://server/path" URIs to "smb:://server/path" on
        // Linux
        // Leave "file::path" URIs unchanged.
        changeScheme(INetProtocol::Smb);
    }
#endif

#ifdef _WIN32
    if (m_eScheme==INetProtocol::Smb) {
        // Change "smb://server/path" URIs to "file://server/path"
        // URIs on Windows, since Windows doesn't understand the
        // SMB scheme.
        changeScheme(INetProtocol::File);
    }
#endif

    return true;
}

void INetURLObject::changeScheme(INetProtocol eTargetScheme) {
    OUString aTmpStr=m_aAbsURIRef.makeStringAndClear();
    int oldSchemeLen=strlen(getSchemeInfo().m_pScheme);
    m_eScheme=eTargetScheme;
    int newSchemeLen=strlen(getSchemeInfo().m_pScheme);
    m_aAbsURIRef.appendAscii(getSchemeInfo().m_pScheme);
    m_aAbsURIRef.append(aTmpStr.getStr()+oldSchemeLen);
    int delta=newSchemeLen-oldSchemeLen;
    m_aUser+=delta;
    m_aAuth+=delta;
    m_aHost+=delta;
    m_aPort+=delta;
    m_aPath+=delta;
    m_aQuery+=delta;
    m_aFragment+=delta;
}

bool INetURLObject::convertRelToAbs(OUString const & rTheRelURIRef,
                                    INetURLObject & rTheAbsURIRef,
                                    bool & rWasAbsolute,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset,
                                    bool bIgnoreFragment, bool bSmart,
                                    bool bRelativeNonURIs, FSysStyle eStyle)
    const
{
    sal_Unicode const * p = rTheRelURIRef.getStr();
    sal_Unicode const * pEnd = p + rTheRelURIRef.getLength();

    sal_Unicode const * pPrefixBegin = p;
    PrefixInfo const * pPrefix = getPrefix(pPrefixBegin, pEnd);
    bool hasScheme = pPrefix != nullptr;
    if (!hasScheme) {
        pPrefixBegin = p;
        hasScheme = !parseScheme(&pPrefixBegin, pEnd, '#').isEmpty();
    }

    sal_uInt32 nSegmentDelimiter = '/';
    sal_uInt32 nQueryDelimiter
        = !bSmart || getSchemeInfo().m_bQuery ? '?' : 0x80000000;
    sal_uInt32 nFragmentDelimiter = '#';
    Part ePart = PART_VISIBLE;

    if (!hasScheme && bSmart)
    {
        // If the input matches any of the following productions (for which
        // the appropriate style bit is set in eStyle), it is assumed to be an
        // absolute file system path, rather than a relative URI reference.
        // (This is only a subset of the productions used for scheme detection
        // in INetURLObject::setAbsURIRef(), because most of those productions
        // interfere with the syntax of relative URI references.)  The
        // productions use the auxiliary rules

        //    domain = label *("." label)
        //    label = alphanum [*(alphanum / "-") alphanum]
        //    alphanum = ALPHA / DIGIT
        //    UCS4 = <any UCS4 character>

        // 1st Production (UNC file; FSysStyle::Dos only):
        //    "\\" domain ["\" *UCS4]
        // 2nd Production (Unix-like DOS file; FSysStyle::Dos only):
        //    ALPHA ":" ["/" *UCS4]
        // 3rd Production (DOS file; FSysStyle::Dos only):
        //    ALPHA ":" ["\" *UCS4]
        if (eStyle & FSysStyle::Dos)
        {
            bool bFSys = false;
            sal_Unicode const * q = p;
            if (pEnd - q >= 2
                && rtl::isAsciiAlpha(q[0])
                && q[1] == ':'
                && (pEnd - q == 2 || q[2] == '/' || q[2] == '\\'))
                bFSys = true; // 2nd, 3rd
            else if (pEnd - q >= 2 && q[0] == '\\' && q[1] == '\\')
            {
                q += 2;
                sal_Int32 n = rtl_ustr_indexOfChar_WithLength(
                    q, pEnd - q, '\\');
                sal_Unicode const * qe = n == -1 ? pEnd : q + n;
                if (parseHostOrNetBiosName(
                        q, qe, false/*bOctets*/, EncodeMechanism::All, RTL_TEXTENCODING_DONTKNOW,
                        true, nullptr))
                {
                    bFSys = true; // 1st
                }
            }
            if (bFSys)
            {
                INetURLObject aNewURI;
                aNewURI.setAbsURIRef(rTheRelURIRef, eMechanism,
                                     eCharset, true, eStyle);
                if (!aNewURI.HasError())
                {
                    rTheAbsURIRef = aNewURI;
                    rWasAbsolute = true;
                    return true;
                }
            }
        }

        // When the base URL is a file URL, accept relative file system paths
        // using "\" or ":" as delimiter (and ignoring URI conventions for "%"
        // and "#"), as well as relative URIs using "/" as delimiter:
        if (m_eScheme == INetProtocol::File)
            switch (guessFSysStyleByCounting(p, pEnd, eStyle))
            {
                case FSysStyle::Unix:
                    nSegmentDelimiter = '/';
                    break;

                case FSysStyle::Dos:
                    nSegmentDelimiter = '\\';
                    bRelativeNonURIs = true;
                    break;

                default:
                    OSL_FAIL("INetURLObject::convertRelToAbs():"
                                  " Bad guessFSysStyleByCounting");
                    break;
            }

        if (bRelativeNonURIs)
        {
            eMechanism = EncodeMechanism::All;
            nQueryDelimiter = 0x80000000;
            nFragmentDelimiter = 0x80000000;
            ePart = PART_VISIBLE_NONSPECIAL;
        }
    }

    // If the relative URI has the same scheme as the base URI, and that
    // scheme is hierarchical, then ignore its presence in the relative
    // URI in order to be backward compatible (cf. RFC 2396 section 5.2
    // step 3):
    if (pPrefix && pPrefix->m_eScheme == m_eScheme
        && getSchemeInfo().m_bHierarchical)
    {
        hasScheme = false;
        while (p != pEnd && *p++ != ':') ;
    }
    rWasAbsolute = hasScheme;

    // Fast solution for non-relative URIs:
    if (hasScheme)
    {
        INetURLObject aNewURI(rTheRelURIRef, eMechanism, eCharset);
        if (aNewURI.HasError())
        {
            rWasAbsolute = false;
            return false;
        }

        if (bIgnoreFragment)
            aNewURI.clearFragment();
        rTheAbsURIRef = aNewURI;
        return true;
    }

    enum State { STATE_AUTH, STATE_ABS_PATH, STATE_REL_PATH, STATE_FRAGMENT,
                 STATE_DONE };

    OUStringBuffer aSynAbsURIRef;
    // make sure that the scheme is copied for generic schemes: getSchemeInfo().m_pScheme
    // is empty ("") in that case, so take the scheme from m_aAbsURIRef
    if (m_eScheme != INetProtocol::Generic)
    {
        aSynAbsURIRef.appendAscii(getSchemeInfo().m_pScheme);
    }
    else
    {
        sal_Unicode const * pSchemeBegin
            = m_aAbsURIRef.getStr();
        sal_Unicode const * pSchemeEnd = pSchemeBegin;
        while (pSchemeEnd[0] != ':')
        {
            ++pSchemeEnd;
        }
        aSynAbsURIRef.append(pSchemeBegin, pSchemeEnd - pSchemeBegin);
    }
    aSynAbsURIRef.append(':');

    State eState = STATE_AUTH;
    bool bSameDoc = true;

    if (getSchemeInfo().m_bAuthority)
    {
        if (pEnd - p >= 2 && p[0] == '/' && p[1] == '/')
        {
            aSynAbsURIRef.append("//");
            p += 2;
            eState = STATE_ABS_PATH;
            bSameDoc = false;
            while (p != pEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32
                    = getUTF32(p, pEnd, false/*bOctets*/, eMechanism,
                               eCharset, eEscapeType);
                if (eEscapeType == EscapeType::NONE)
                {
                    if (nUTF32 == nSegmentDelimiter)
                        break;
                    else if (nUTF32 == nFragmentDelimiter)
                    {
                        eState = STATE_FRAGMENT;
                        break;
                    }
                }
                appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, false/*bOctets*/,
                           PART_VISIBLE, eCharset, true);
            }
        }
        else
        {
            SubString aAuthority(getAuthority());
            aSynAbsURIRef.append(m_aAbsURIRef.getStr()
                                     + aAuthority.getBegin(),
                                 aAuthority.getLength());
        }
    }

    if (eState == STATE_AUTH)
    {
        if (p == pEnd)
            eState = STATE_DONE;
        else if (*p == nFragmentDelimiter)
        {
            ++p;
            eState = STATE_FRAGMENT;
        }
        else if (*p == nSegmentDelimiter)
        {
            ++p;
            eState = STATE_ABS_PATH;
            bSameDoc = false;
        }
        else
        {
            eState = STATE_REL_PATH;
            bSameDoc = false;
        }
    }

    if (eState == STATE_ABS_PATH)
    {
        aSynAbsURIRef.append('/');
        eState = STATE_DONE;
        while (p != pEnd)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32
                = getUTF32(p, pEnd, false/*bOctets*/, eMechanism, eCharset, eEscapeType);
            if (eEscapeType == EscapeType::NONE)
            {
                if (nUTF32 == nFragmentDelimiter)
                {
                    eState = STATE_FRAGMENT;
                    break;
                }
                else if (nUTF32 == nSegmentDelimiter)
                    nUTF32 = '/';
            }
            appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, false/*bOctets*/, ePart,
                       eCharset, true);
        }
    }
    else if (eState == STATE_REL_PATH)
    {
        if (!getSchemeInfo().m_bHierarchical)
        {
            // Detect cases where a relative input could not be made absolute
            // because the given base URL is broken (most probably because it is
            // empty):
            SAL_WARN_IF(
                HasError(), "tools.urlobj",
                "cannot make <" << rTheRelURIRef
                    << "> absolute against broken base <"
                    << GetMainURL(DecodeMechanism::NONE) << ">");
            rWasAbsolute = false;
            return false;
        }

        sal_Unicode const * pBasePathBegin
            = m_aAbsURIRef.getStr() + m_aPath.getBegin();
        sal_Unicode const * pBasePathEnd
            = pBasePathBegin + m_aPath.getLength();
        while (pBasePathEnd != pBasePathBegin)
            if (*(--pBasePathEnd) == '/')
            {
                ++pBasePathEnd;
                break;
            }

        sal_Int32 nPathBegin = aSynAbsURIRef.getLength();
        aSynAbsURIRef.append(pBasePathBegin, pBasePathEnd - pBasePathBegin);
        DBG_ASSERT(aSynAbsURIRef.getLength() > nPathBegin
                 && aSynAbsURIRef[aSynAbsURIRef.getLength() - 1] == '/',
                 "INetURLObject::convertRelToAbs(): Bad base path");

        while (p != pEnd && *p != nQueryDelimiter && *p != nFragmentDelimiter)
        {
            if (*p == '.')
            {
                if (pEnd - p == 1
                    || p[1] == nSegmentDelimiter
                    || p[1] == nQueryDelimiter
                    || p[1] == nFragmentDelimiter)
                {
                    ++p;
                    if (p != pEnd && *p == nSegmentDelimiter)
                        ++p;
                    continue;
                }
                else if (pEnd - p >= 2
                         && p[1] == '.'
                         && (pEnd - p == 2
                             || p[2] == nSegmentDelimiter
                             || p[2] == nQueryDelimiter
                             || p[2] == nFragmentDelimiter)
                         && aSynAbsURIRef.getLength() - nPathBegin > 1)
                {
                    p += 2;
                    if (p != pEnd && *p == nSegmentDelimiter)
                        ++p;

                    sal_Int32 i = aSynAbsURIRef.getLength() - 2;
                    while (i > nPathBegin && aSynAbsURIRef[i] != '/')
                        --i;
                    aSynAbsURIRef.setLength(i + 1);
                    DBG_ASSERT(
                        aSynAbsURIRef.getLength() > nPathBegin
                        && aSynAbsURIRef[aSynAbsURIRef.getLength() - 1] == '/',
                        "INetURLObject::convertRelToAbs(): Bad base path");
                    continue;
                }
            }

            while (p != pEnd
                   && *p != nSegmentDelimiter
                   && *p != nQueryDelimiter
                   && *p != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32
                    = getUTF32(p, pEnd, false/*bOctets*/, eMechanism,
                               eCharset, eEscapeType);
                appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, false/*bOctets*/, ePart,
                           eCharset, true);
            }
            if (p != pEnd && *p == nSegmentDelimiter)
            {
                aSynAbsURIRef.append('/');
                ++p;
            }
        }

        while (p != pEnd && *p != nFragmentDelimiter)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32
                = getUTF32(p, pEnd, false/*bOctets*/, eMechanism, eCharset, eEscapeType);
            appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, false/*bOctets*/, ePart,
                       eCharset, true);
        }

        if (p == pEnd)
            eState = STATE_DONE;
        else
        {
            ++p;
            eState = STATE_FRAGMENT;
        }
    }
    else if (bSameDoc)
    {
        aSynAbsURIRef.append(m_aAbsURIRef.getStr() + m_aPath.getBegin(),
                             m_aPath.getLength());
        if (m_aQuery.isPresent())
            aSynAbsURIRef.append(m_aAbsURIRef.getStr()
                                     + m_aQuery.getBegin() - 1,
                                 m_aQuery.getLength() + 1);
    }

    if (eState == STATE_FRAGMENT && !bIgnoreFragment)
    {
        aSynAbsURIRef.append('#');
        while (p != pEnd)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32
                = getUTF32(p, pEnd, false/*bOctets*/, eMechanism, eCharset, eEscapeType);
            appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, false/*bOctets*/,
                       PART_VISIBLE, eCharset, true);
        }
    }

    INetURLObject aNewURI(aSynAbsURIRef.makeStringAndClear());
    if (aNewURI.HasError())
    {
        // Detect cases where a relative input could not be made absolute
        // because the given base URL is broken (most probably because it is
        // empty):
        SAL_WARN_IF(
            HasError(), "tools.urlobj",
            "cannot make <" << rTheRelURIRef
                << "> absolute against broken base <" << GetMainURL(DecodeMechanism::NONE)
                << ">");
        rWasAbsolute = false;
        return false;
    }

    rTheAbsURIRef = aNewURI;
    return true;
}

bool INetURLObject::convertAbsToRel(OUString const & rTheAbsURIRef,
                                    OUString & rTheRelURIRef,
                                    EncodeMechanism eEncodeMechanism,
                                    DecodeMechanism eDecodeMechanism,
                                    rtl_TextEncoding eCharset,
                                    FSysStyle eStyle) const
{
    // Check for hierarchical base URL:
    if (!getSchemeInfo().m_bHierarchical)
    {
        rTheRelURIRef = decode(rTheAbsURIRef, eDecodeMechanism, eCharset);
        return false;
    }

    // Convert the input (absolute or relative URI ref) to an absolute URI
    // ref:
    INetURLObject aSubject;
    bool bWasAbsolute;
    if (!convertRelToAbs(rTheAbsURIRef, aSubject, bWasAbsolute,
                         eEncodeMechanism, eCharset, false, false, false,
                         eStyle))
    {
        rTheRelURIRef = decode(rTheAbsURIRef, eDecodeMechanism, eCharset);
        return false;
    }

    // Check for differing scheme or authority parts:
    if ((m_aScheme.compare(
             aSubject.m_aScheme, m_aAbsURIRef, aSubject.m_aAbsURIRef)
         != 0)
        || (m_aUser.compare(
                aSubject.m_aUser, m_aAbsURIRef, aSubject.m_aAbsURIRef)
            != 0)
        || (m_aAuth.compare(
                aSubject.m_aAuth, m_aAbsURIRef, aSubject.m_aAbsURIRef)
            != 0)
        || (m_aHost.compare(
                aSubject.m_aHost, m_aAbsURIRef, aSubject.m_aAbsURIRef)
            != 0)
        || (m_aPort.compare(
                aSubject.m_aPort, m_aAbsURIRef, aSubject.m_aAbsURIRef)
            != 0))
    {
        rTheRelURIRef = aSubject.GetMainURL(eDecodeMechanism, eCharset);
        return false;
    }

    sal_Unicode const * pBasePathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pBasePathEnd = pBasePathBegin + m_aPath.getLength();
    sal_Unicode const * pSubjectPathBegin
        = aSubject.m_aAbsURIRef.getStr() + aSubject.m_aPath.getBegin();
    sal_Unicode const * pSubjectPathEnd
        = pSubjectPathBegin + aSubject.m_aPath.getLength();

    // Make nMatch point past the last matching slash, or past the end of the
    // paths, in case they are equal:
    sal_Unicode const * pSlash = nullptr;
    sal_Unicode const * p1 = pBasePathBegin;
    sal_Unicode const * p2 = pSubjectPathBegin;
    for (;;)
    {
        if (p1 == pBasePathEnd || p2 == pSubjectPathEnd)
        {
            if (p1 == pBasePathEnd && p2 == pSubjectPathEnd)
                pSlash = p1;
            break;
        }

        sal_Unicode c = *p1++;
        if (c != *p2++)
            break;
        if (c == '/')
            pSlash = p1;
    }
    if (!pSlash)
    {
        // One of the paths does not start with '/':
        rTheRelURIRef = aSubject.GetMainURL(eDecodeMechanism, eCharset);
        return false;
    }
    sal_Int32 nMatch = pSlash - pBasePathBegin;

    // If the two URLs are DOS file URLs starting with different volumes
    // (e.g., file:///a:/... and file:///b:/...), the subject is not made
    // relative (it could be, but some people do not like that):
    if (m_eScheme == INetProtocol::File
        && nMatch <= 1
        && hasDosVolume(eStyle)
        && aSubject.hasDosVolume(eStyle)) //TODO! ok to use eStyle for these?
    {
        rTheRelURIRef = aSubject.GetMainURL(eDecodeMechanism, eCharset);
        return false;
    }

    // For every slash in the base path after nMatch, a prefix of "../" is
    // added to the new relative URL (if the common prefix of the two paths is
    // only "/"---but see handling of file URLs above---, the complete subject
    // path could go into the new relative URL instead, but some people don't
    // like that):
    OUStringBuffer aSynRelURIRef;
    for (sal_Unicode const * p = pBasePathBegin + nMatch; p != pBasePathEnd;
         ++p)
    {
        if (*p == '/')
            aSynRelURIRef.append("../");
    }

    // If the new relative URL would start with "//" (i.e., it would be
    // mistaken for a relative URL starting with an authority part), or if the
    // new relative URL would neither be empty nor start with <"/"> nor start
    // with <1*rseg> (i.e., it could be mistaken for an absolute URL starting
    // with a scheme part), then the new relative URL is prefixed with "./":
    if (aSynRelURIRef.isEmpty())
    {
        if (pSubjectPathEnd - pSubjectPathBegin >= nMatch + 2
            && pSubjectPathBegin[nMatch] == '/'
            && pSubjectPathBegin[nMatch + 1] == '/')
        {
            aSynRelURIRef.append("./");
        }
        else
        {
            for (sal_Unicode const * p = pSubjectPathBegin + nMatch;
                 p != pSubjectPathEnd && *p != '/'; ++p)
            {
                if (mustEncode(*p, PART_REL_SEGMENT_EXTRA))
                {
                    aSynRelURIRef.append("./");
                    break;
                }
            }
        }
    }

    // The remainder of the subject path, starting at nMatch, is appended to
    // the new relative URL:
    aSynRelURIRef.append(decode(pSubjectPathBegin + nMatch, pSubjectPathEnd,
                            eDecodeMechanism, eCharset));

    // If the subject has defined query or fragment parts, they are appended
    // to the new relative URL:
    if (aSubject.m_aQuery.isPresent())
    {
        aSynRelURIRef.append('?');
        aSynRelURIRef.append(aSubject.decode(aSubject.m_aQuery,
                                         eDecodeMechanism, eCharset));
    }
    if (aSubject.m_aFragment.isPresent())
    {
        aSynRelURIRef.append('#');
        aSynRelURIRef.append(aSubject.decode(aSubject.m_aFragment,
            eDecodeMechanism, eCharset));
    }

    rTheRelURIRef = aSynRelURIRef.makeStringAndClear();
    return true;
}

// static
bool INetURLObject::convertIntToExt(OUString const & rTheIntURIRef,
                                    bool bOctets, OUString & rTheExtURIRef,
                                    DecodeMechanism eDecodeMechanism,
                                    rtl_TextEncoding eCharset)
{
    OUString aSynExtURIRef(encodeText(rTheIntURIRef, bOctets, PART_VISIBLE,
                                       EncodeMechanism::NotCanonical, eCharset, true));
    sal_Unicode const * pBegin = aSynExtURIRef.getStr();
    sal_Unicode const * pEnd = pBegin + aSynExtURIRef.getLength();
    sal_Unicode const * p = pBegin;
    PrefixInfo const * pPrefix = getPrefix(p, pEnd);
    bool bConvert = pPrefix && pPrefix->m_eKind == PrefixInfo::INTERNAL;
    if (bConvert)
    {
        aSynExtURIRef =
            aSynExtURIRef.replaceAt(0, p - pBegin,
                OUString::createFromAscii(pPrefix->m_pTranslatedPrefix));
    }
    rTheExtURIRef = decode(aSynExtURIRef, eDecodeMechanism, eCharset);
    return bConvert;
}

// static
bool INetURLObject::convertExtToInt(OUString const & rTheExtURIRef,
                                    bool bOctets, OUString & rTheIntURIRef,
                                    DecodeMechanism eDecodeMechanism,
                                    rtl_TextEncoding eCharset)
{
    OUString aSynIntURIRef(encodeText(rTheExtURIRef, bOctets, PART_VISIBLE,
                                       EncodeMechanism::NotCanonical, eCharset, true));
    sal_Unicode const * pBegin = aSynIntURIRef.getStr();
    sal_Unicode const * pEnd = pBegin + aSynIntURIRef.getLength();
    sal_Unicode const * p = pBegin;
    PrefixInfo const * pPrefix = getPrefix(p, pEnd);
    bool bConvert = pPrefix && pPrefix->m_eKind == PrefixInfo::EXTERNAL;
    if (bConvert)
    {
        aSynIntURIRef =
            aSynIntURIRef.replaceAt(0, p - pBegin,
                OUString::createFromAscii(pPrefix->m_pTranslatedPrefix));
    }
    rTheIntURIRef = decode(aSynIntURIRef, eDecodeMechanism, eCharset);
    return bConvert;
}

// static
INetURLObject::PrefixInfo const * INetURLObject::getPrefix(sal_Unicode const *& rBegin,
                                                           sal_Unicode const * pEnd)
{
    static PrefixInfo const aMap[]
        = { // dummy entry at front needed, because pLast may point here:
            { nullptr, nullptr, INetProtocol::NotValid, PrefixInfo::INTERNAL },
            { ".component:", "staroffice.component:", INetProtocol::Component,
              PrefixInfo::INTERNAL },
            { ".uno:", "staroffice.uno:", INetProtocol::Uno,
              PrefixInfo::INTERNAL },
            { "cid:", nullptr, INetProtocol::Cid, PrefixInfo::OFFICIAL },
            { "data:", nullptr, INetProtocol::Data, PrefixInfo::OFFICIAL },
            { "db:", "staroffice.db:", INetProtocol::Db, PrefixInfo::INTERNAL },
            { "file:", nullptr, INetProtocol::File, PrefixInfo::OFFICIAL },
            { "ftp:", nullptr, INetProtocol::Ftp, PrefixInfo::OFFICIAL },
            { "hid:", "staroffice.hid:", INetProtocol::Hid,
              PrefixInfo::INTERNAL },
            { "http:", nullptr, INetProtocol::Http, PrefixInfo::OFFICIAL },
            { "https:", nullptr, INetProtocol::Https, PrefixInfo::OFFICIAL },
            { "javascript:", nullptr, INetProtocol::Javascript, PrefixInfo::OFFICIAL },
            { "ldap:", nullptr, INetProtocol::Ldap, PrefixInfo::OFFICIAL },
            { "macro:", "staroffice.macro:", INetProtocol::Macro,
              PrefixInfo::INTERNAL },
            { "mailto:", nullptr, INetProtocol::Mailto, PrefixInfo::OFFICIAL },
            { "private:", "staroffice.private:", INetProtocol::PrivSoffice,
              PrefixInfo::INTERNAL },
            { "private:factory/", "staroffice.factory:",
              INetProtocol::PrivSoffice, PrefixInfo::INTERNAL },
            { "private:helpid/", "staroffice.helpid:", INetProtocol::PrivSoffice,
              PrefixInfo::INTERNAL },
            { "private:java/", "staroffice.java:", INetProtocol::PrivSoffice,
              PrefixInfo::INTERNAL },
            { "private:searchfolder:", "staroffice.searchfolder:",
              INetProtocol::PrivSoffice, PrefixInfo::INTERNAL },
            { "private:trashcan:", "staroffice.trashcan:",
              INetProtocol::PrivSoffice, PrefixInfo::INTERNAL },
            { "sftp:", nullptr, INetProtocol::Sftp, PrefixInfo::OFFICIAL },
            { "slot:", "staroffice.slot:", INetProtocol::Slot,
              PrefixInfo::INTERNAL },
            { "smb:", nullptr, INetProtocol::Smb, PrefixInfo::OFFICIAL },
            { "staroffice.component:", ".component:", INetProtocol::Component,
              PrefixInfo::EXTERNAL },
            { "staroffice.db:", "db:", INetProtocol::Db, PrefixInfo::EXTERNAL },
            { "staroffice.factory:", "private:factory/",
              INetProtocol::PrivSoffice, PrefixInfo::EXTERNAL },
            { "staroffice.helpid:", "private:helpid/", INetProtocol::PrivSoffice,
              PrefixInfo::EXTERNAL },
            { "staroffice.hid:", "hid:", INetProtocol::Hid,
              PrefixInfo::EXTERNAL },
            { "staroffice.java:", "private:java/", INetProtocol::PrivSoffice,
              PrefixInfo::EXTERNAL },
            { "staroffice.macro:", "macro:", INetProtocol::Macro,
              PrefixInfo::EXTERNAL },
            { "staroffice.private:", "private:", INetProtocol::PrivSoffice,
              PrefixInfo::EXTERNAL },
            { "staroffice.searchfolder:", "private:searchfolder:",
              INetProtocol::PrivSoffice, PrefixInfo::EXTERNAL },
            { "staroffice.slot:", "slot:", INetProtocol::Slot,
              PrefixInfo::EXTERNAL },
            { "staroffice.trashcan:", "private:trashcan:",
              INetProtocol::PrivSoffice, PrefixInfo::EXTERNAL },
            { "staroffice.uno:", ".uno:", INetProtocol::Uno,
              PrefixInfo::EXTERNAL },
            { "staroffice:", "private:", INetProtocol::PrivSoffice,
              PrefixInfo::EXTERNAL },
            { "telnet:", nullptr, INetProtocol::Telnet, PrefixInfo::OFFICIAL },
            { "vnd.libreoffice.cmis:", nullptr, INetProtocol::Cmis, PrefixInfo::INTERNAL },
            { "vnd.sun.star.cmd:", nullptr, INetProtocol::VndSunStarCmd,
              PrefixInfo::OFFICIAL },
            { "vnd.sun.star.expand:", nullptr, INetProtocol::VndSunStarExpand,
              PrefixInfo::OFFICIAL },
            { "vnd.sun.star.help:", nullptr, INetProtocol::VndSunStarHelp,
              PrefixInfo::OFFICIAL },
            { "vnd.sun.star.hier:", nullptr, INetProtocol::VndSunStarHier,
              PrefixInfo::OFFICIAL },
            { "vnd.sun.star.pkg:", nullptr, INetProtocol::VndSunStarPkg,
              PrefixInfo::OFFICIAL },
            { "vnd.sun.star.tdoc:", nullptr, INetProtocol::VndSunStarTdoc,
              PrefixInfo::OFFICIAL },
            { "vnd.sun.star.webdav:", nullptr, INetProtocol::VndSunStarWebdav,
              PrefixInfo::OFFICIAL } };
    /* This list needs to be sorted, or you'll introduce serious bugs */

    PrefixInfo const * pFirst = aMap + 1;
    PrefixInfo const * pLast = aMap + sizeof aMap / sizeof (PrefixInfo) - 1;
    PrefixInfo const * pMatch = nullptr;
    sal_Unicode const * pMatched = rBegin;
    sal_Unicode const * p = rBegin;
    sal_Int32 i = 0;
    for (; pFirst < pLast; ++i)
    {
        if (pFirst->m_pPrefix[i] == '\0')
        {
            pMatch = pFirst++;
            pMatched = p;
        }
        if (p >= pEnd)
            break;
        sal_uInt32 nChar = rtl::toAsciiLowerCase(*p++);
        while (pFirst <= pLast && static_cast<unsigned char>(pFirst->m_pPrefix[i]) < nChar)
            ++pFirst;
        while (pFirst <= pLast && static_cast<unsigned char>(pLast->m_pPrefix[i]) > nChar)
            --pLast;
    }
    if (pFirst == pLast)
    {
        sal_Char const * q = pFirst->m_pPrefix + i;
        while (p < pEnd && *q != '\0'
               && rtl::toAsciiLowerCase(*p) == static_cast<unsigned char>(*q))
        {
            ++p;
            ++q;
        }
        if (*q == '\0')
        {
            rBegin = p;
            return pFirst;
        }
    }
    rBegin = pMatched;
    return pMatch;
}

sal_Int32 INetURLObject::getAuthorityBegin() const
{
    DBG_ASSERT(getSchemeInfo().m_bAuthority,
               "INetURLObject::getAuthority(): Bad scheme");
    sal_Int32 nBegin;
    if (m_aUser.isPresent())
        nBegin = m_aUser.getBegin();
    else if (m_aHost.isPresent())
        nBegin = m_aHost.getBegin();
    else
        nBegin = m_aPath.getBegin();
    nBegin -= RTL_CONSTASCII_LENGTH("//");
    DBG_ASSERT(m_aAbsURIRef[nBegin] == '/' && m_aAbsURIRef[nBegin + 1] == '/',
               "INetURLObject::getAuthority(): Bad authority");
    return nBegin;
}

INetURLObject::SubString INetURLObject::getAuthority() const
{
    sal_Int32 nBegin = getAuthorityBegin();
    sal_Int32 nEnd = m_aPort.isPresent() ? m_aPort.getEnd() :
                      m_aHost.isPresent() ? m_aHost.getEnd() :
                      m_aAuth.isPresent() ? m_aAuth.getEnd() :
                      m_aUser.isPresent() ? m_aUser.getEnd() :
                          nBegin + RTL_CONSTASCII_LENGTH("//");
    return SubString(nBegin, nEnd - nBegin);
}

bool INetURLObject::setUser(OUString const & rTheUser,
                            rtl_TextEncoding eCharset)
{
    if (
         !getSchemeInfo().m_bUser
       )
    {
        return false;
    }

    OUString aNewUser(encodeText(rTheUser, false/*bOctets*/, PART_USER_PASSWORD,
                                  EncodeMechanism::WasEncoded, eCharset, false));
    sal_Int32 nDelta;
    if (m_aUser.isPresent())
        nDelta = m_aUser.set(m_aAbsURIRef, aNewUser);
    else if (m_aHost.isPresent())
    {
        m_aAbsURIRef.insert(m_aHost.getBegin(), u'@');
        nDelta = m_aUser.set(m_aAbsURIRef, aNewUser, m_aHost.getBegin()) + 1;
    }
    else if (getSchemeInfo().m_bHost)
        return false;
    else
        nDelta = m_aUser.set(m_aAbsURIRef, aNewUser, m_aPath.getBegin());
    m_aAuth += nDelta;
    m_aHost += nDelta;
    m_aPort += nDelta;
    m_aPath += nDelta;
    m_aQuery += nDelta;
    m_aFragment += nDelta;
    return true;
}

namespace
{
    void lcl_Erase(OUStringBuffer &rBuf, sal_Int32 index, sal_Int32 count)
    {
        OUString sTemp(rBuf.makeStringAndClear());
        rBuf.append(sTemp.replaceAt(index, count, OUString()));
    }
}

bool INetURLObject::clearPassword()
{
    if (!getSchemeInfo().m_bPassword)
        return false;
    if (m_aAuth.isPresent())
    {
        lcl_Erase(m_aAbsURIRef, m_aAuth.getBegin() - 1,
            m_aAuth.getLength() + 1);
        sal_Int32 nDelta = m_aAuth.clear() - 1;
        m_aHost += nDelta;
        m_aPort += nDelta;
        m_aPath += nDelta;
        m_aQuery += nDelta;
        m_aFragment += nDelta;
    }
    return true;
}

bool INetURLObject::setPassword(OUString const & rThePassword,
                                rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bPassword)
        return false;
    OUString aNewAuth(encodeText(rThePassword, false/*bOctets*/, PART_USER_PASSWORD,
                                  EncodeMechanism::WasEncoded, eCharset, false));
    sal_Int32 nDelta;
    if (m_aAuth.isPresent())
        nDelta = m_aAuth.set(m_aAbsURIRef, aNewAuth);
    else if (m_aUser.isPresent())
    {
        m_aAbsURIRef.insert(m_aUser.getEnd(), u':');
        nDelta
            = m_aAuth.set(m_aAbsURIRef, aNewAuth, m_aUser.getEnd() + 1) + 1;
    }
    else if (m_aHost.isPresent())
    {
        m_aAbsURIRef.insert(m_aHost.getBegin(),
            OUString( ":@" ));
        m_aUser.set(m_aAbsURIRef, OUString(), m_aHost.getBegin());
        nDelta
            = m_aAuth.set(m_aAbsURIRef, aNewAuth, m_aHost.getBegin() + 1) + 2;
    }
    else if (getSchemeInfo().m_bHost)
        return false;
    else
    {
        m_aAbsURIRef.insert(m_aPath.getBegin(), u':');
        m_aUser.set(m_aAbsURIRef, OUString(), m_aPath.getBegin());
        nDelta
            = m_aAuth.set(m_aAbsURIRef, aNewAuth, m_aPath.getBegin() + 1) + 1;
    }
    m_aHost += nDelta;
    m_aPort += nDelta;
    m_aPath += nDelta;
    m_aQuery += nDelta;
    m_aFragment += nDelta;
    return true;
}

// static
bool INetURLObject::parseHost(sal_Unicode const *& rBegin, sal_Unicode const * pEnd,
    OUString & rCanonic)
{
    // RFC 2373 is inconsistent about how to write an IPv6 address in which an
    // IPv4 address directly follows the abbreviating "::".  The ABNF in
    // Appendix B suggests ":::13.1.68.3", while an example in 2.2/3 explicitly
    // mentions "::13:1.68.3".  This algorithm accepts both variants:
    enum State { STATE_INITIAL, STATE_LABEL, STATE_LABEL_HYPHEN,
                 STATE_LABEL_DOT, STATE_TOPLABEL, STATE_TOPLABEL_HYPHEN,
                 STATE_TOPLABEL_DOT, STATE_IP4, STATE_IP4_DOT, STATE_IP6,
                 STATE_IP6_COLON, STATE_IP6_2COLON, STATE_IP6_3COLON,
                 STATE_IP6_HEXSEQ1, STATE_IP6_HEXSEQ1_COLON,
                 STATE_IP6_HEXSEQ1_MAYBE_IP4, STATE_IP6_HEXSEQ2,
                 STATE_IP6_HEXSEQ2_COLON, STATE_IP6_HEXSEQ2_MAYBE_IP4,
                 STATE_IP6_IP4, STATE_IP6_IP4_DOT, STATE_IP6_DONE };
    OUStringBuffer aTheCanonic;
    sal_uInt32 nNumber = 0;
    int nDigits = 0;
    int nOctets = 0;
    State eState = STATE_INITIAL;
    sal_Unicode const * p = rBegin;
    for (; p != pEnd; ++p)
        switch (eState)
        {
            case STATE_INITIAL:
                if (*p == '[')
                {
                    aTheCanonic.append('[');
                    eState = STATE_IP6;
                }
                else if (rtl::isAsciiAlpha(*p) || *p == '_')
                    eState = STATE_TOPLABEL;
                else if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    nOctets = 1;
                    eState = STATE_IP4;
                }
                else
                    goto done;
                break;

            case STATE_LABEL:
                if (*p == '.')
                    eState = STATE_LABEL_DOT;
                else if (*p == '-')
                    eState = STATE_LABEL_HYPHEN;
                else if (!rtl::isAsciiAlphanumeric(*p) && *p != '_')
                    goto done;
                break;

            case STATE_LABEL_HYPHEN:
                if (rtl::isAsciiAlphanumeric(*p) || *p == '_')
                    eState = STATE_LABEL;
                else if (*p != '-')
                    goto done;
                break;

            case STATE_LABEL_DOT:
                if (rtl::isAsciiAlpha(*p) || *p == '_')
                    eState = STATE_TOPLABEL;
                else if (rtl::isAsciiDigit(*p))
                    eState = STATE_LABEL;
                else
                    goto done;
                break;

            case STATE_TOPLABEL:
                if (*p == '.')
                    eState = STATE_TOPLABEL_DOT;
                else if (*p == '-')
                    eState = STATE_TOPLABEL_HYPHEN;
                else if (!rtl::isAsciiAlphanumeric(*p) && *p != '_')
                    goto done;
                break;

            case STATE_TOPLABEL_HYPHEN:
                if (rtl::isAsciiAlphanumeric(*p) || *p == '_')
                    eState = STATE_TOPLABEL;
                else if (*p != '-')
                    goto done;
                break;

            case STATE_TOPLABEL_DOT:
                if (rtl::isAsciiAlpha(*p) || *p == '_')
                    eState = STATE_TOPLABEL;
                else if (rtl::isAsciiDigit(*p))
                    eState = STATE_LABEL;
                else
                    goto done;
                break;

            case STATE_IP4:
                if (*p == '.')
                    if (nOctets < 4)
                    {
                        aTheCanonic.append( OUString::number(nNumber) );
                        aTheCanonic.append( '.' );
                        ++nOctets;
                        eState = STATE_IP4_DOT;
                    }
                    else
                        eState = STATE_LABEL_DOT;
                else if (*p == '-')
                    eState = STATE_LABEL_HYPHEN;
                else if (rtl::isAsciiAlpha(*p) || *p == '_')
                    eState = STATE_LABEL;
                else if (rtl::isAsciiDigit(*p))
                    if (nDigits < 3)
                    {
                        nNumber = 10 * nNumber + INetMIME::getWeight(*p);
                        ++nDigits;
                    }
                    else
                        eState = STATE_LABEL;
                else
                    goto done;
                break;

            case STATE_IP4_DOT:
                if (rtl::isAsciiAlpha(*p) || *p == '_')
                    eState = STATE_TOPLABEL;
                else if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP4;
                }
                else
                    goto done;
                break;

            case STATE_IP6:
                if (*p == ':')
                    eState = STATE_IP6_COLON;
                else if (rtl::isAsciiHexDigit(*p))
                {
                    nNumber = INetMIME::getHexWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ1;
                }
                else
                    goto done;
                break;

            case STATE_IP6_COLON:
                if (*p == ':')
                {
                    aTheCanonic.append("::");
                    eState = STATE_IP6_2COLON;
                }
                else
                    goto done;
                break;

            case STATE_IP6_2COLON:
                if (*p == ']')
                    eState = STATE_IP6_DONE;
                else if (*p == ':')
                {
                    aTheCanonic.append(':');
                    eState = STATE_IP6_3COLON;
                }
                else if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ2_MAYBE_IP4;
                }
                else if (rtl::isAsciiHexDigit(*p))
                {
                    nNumber = INetMIME::getHexWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ2;
                }
                else
                    goto done;
                break;

            case STATE_IP6_3COLON:
                if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    nOctets = 1;
                    eState = STATE_IP6_IP4;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ1:
                if (*p == ']')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    aTheCanonic.append(':');
                    eState = STATE_IP6_HEXSEQ1_COLON;
                }
                else if (rtl::isAsciiHexDigit(*p) && nDigits < 4)
                {
                    nNumber = 16 * nNumber + INetMIME::getHexWeight(*p);
                    ++nDigits;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ1_COLON:
                if (*p == ':')
                {
                    aTheCanonic.append(':');
                    eState = STATE_IP6_2COLON;
                }
                else if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ1_MAYBE_IP4;
                }
                else if (rtl::isAsciiHexDigit(*p))
                {
                    nNumber = INetMIME::getHexWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ1;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ1_MAYBE_IP4:
                if (*p == ']')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    aTheCanonic.append(':');
                    eState = STATE_IP6_HEXSEQ1_COLON;
                }
                else if (*p == '.')
                {
                    nNumber = 100 * (nNumber >> 8) + 10 * (nNumber >> 4 & 15)
                                  + (nNumber & 15);
                    aTheCanonic.append(
                        OUString::number(nNumber));
                    aTheCanonic.append('.');
                    nOctets = 2;
                    eState = STATE_IP6_IP4_DOT;
                }
                else if (rtl::isAsciiDigit(*p) && nDigits < 3)
                {
                    nNumber = 16 * nNumber + INetMIME::getWeight(*p);
                    ++nDigits;
                }
                else if (rtl::isAsciiHexDigit(*p) && nDigits < 4)
                {
                    nNumber = 16 * nNumber + INetMIME::getHexWeight(*p);
                    ++nDigits;
                    eState = STATE_IP6_HEXSEQ1;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ2:
                if (*p == ']')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    aTheCanonic.append(':');
                    eState = STATE_IP6_HEXSEQ2_COLON;
                }
                else if (rtl::isAsciiHexDigit(*p) && nDigits < 4)
                {
                    nNumber = 16 * nNumber + INetMIME::getHexWeight(*p);
                    ++nDigits;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ2_COLON:
                if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ2_MAYBE_IP4;
                }
                else if (rtl::isAsciiHexDigit(*p))
                {
                    nNumber = INetMIME::getHexWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ2;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ2_MAYBE_IP4:
                if (*p == ']')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic.append(
                        OUString::number(nNumber, 16));
                    aTheCanonic.append(':');
                    eState = STATE_IP6_HEXSEQ2_COLON;
                }
                else if (*p == '.')
                {
                    nNumber = 100 * (nNumber >> 8) + 10 * (nNumber >> 4 & 15)
                                  + (nNumber & 15);
                    aTheCanonic.append(
                        OUString::number(nNumber));
                    aTheCanonic.append('.');
                    nOctets = 2;
                    eState = STATE_IP6_IP4_DOT;
                }
                else if (rtl::isAsciiDigit(*p) && nDigits < 3)
                {
                    nNumber = 16 * nNumber + INetMIME::getWeight(*p);
                    ++nDigits;
                }
                else if (rtl::isAsciiHexDigit(*p) && nDigits < 4)
                {
                    nNumber = 16 * nNumber + INetMIME::getHexWeight(*p);
                    ++nDigits;
                    eState = STATE_IP6_HEXSEQ2;
                }
                else
                    goto done;
                break;

            case STATE_IP6_IP4:
                if (*p == ']')
                    if (nOctets == 4)
                    {
                        aTheCanonic.append(
                            OUString::number(nNumber));
                        eState = STATE_IP6_DONE;
                    }
                    else
                        goto done;
                else if (*p == '.')
                    if (nOctets < 4)
                    {
                        aTheCanonic.append(
                            OUString::number(nNumber));
                        aTheCanonic.append('.');
                        ++nOctets;
                        eState = STATE_IP6_IP4_DOT;
                    }
                    else
                        goto done;
                else if (rtl::isAsciiDigit(*p) && nDigits < 3)
                {
                    nNumber = 10 * nNumber + INetMIME::getWeight(*p);
                    ++nDigits;
                }
                else
                    goto done;
                break;

            case STATE_IP6_IP4_DOT:
                if (rtl::isAsciiDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_IP4;
                }
                else
                    goto done;
                break;

            case STATE_IP6_DONE:
                goto done;
        }
 done:
    switch (eState)
    {
        case STATE_LABEL:
        case STATE_TOPLABEL:
        case STATE_TOPLABEL_DOT:
            aTheCanonic.setLength(0);
            aTheCanonic.append(rBegin, p - rBegin);
            rBegin = p;
            rCanonic = aTheCanonic.makeStringAndClear();
            return true;

        case STATE_IP4:
            if (nOctets == 4)
            {
                aTheCanonic.append(
                    OUString::number(nNumber));
                rBegin = p;
                rCanonic = aTheCanonic.makeStringAndClear();
                return true;
            }
            return false;

        case STATE_IP6_DONE:
            aTheCanonic.append(']');
            rBegin = p;
            rCanonic = aTheCanonic.makeStringAndClear();
            return true;

        default:
            return false;
    }
}

// static
bool INetURLObject::parseHostOrNetBiosName(
    sal_Unicode const * pBegin, sal_Unicode const * pEnd, bool bOctets,
    EncodeMechanism eMechanism, rtl_TextEncoding eCharset, bool bNetBiosName,
    OUStringBuffer* pCanonic)
{
    OUString aTheCanonic;
    if (pBegin < pEnd)
    {
        sal_Unicode const * p = pBegin;
        if (!parseHost(p, pEnd, aTheCanonic) || p != pEnd)
        {
            if (bNetBiosName)
            {
                OUStringBuffer buf;
                while (pBegin < pEnd)
                {
                    EscapeType eEscapeType;
                    sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, bOctets,
                                                 eMechanism, eCharset,
                                                 eEscapeType);
                    if (!INetMIME::isVisible(nUTF32))
                        return false;
                    if (!rtl::isAsciiAlphanumeric(nUTF32))
                        switch (nUTF32)
                        {
                        case '"':
                        case '*':
                        case '+':
                        case ',':
                        case '/':
                        case ':':
                        case ';':
                        case '<':
                        case '=':
                        case '>':
                        case '?':
                        case '[':
                        case '\\':
                        case ']':
                        case '`':
                        case '|':
                            return false;
                        }
                    if (pCanonic != nullptr) {
                        appendUCS4(
                            buf, nUTF32, eEscapeType, bOctets, PART_URIC,
                            eCharset, true);
                    }
                }
                aTheCanonic = buf.makeStringAndClear();
            }
            else
                return false;
        }
    }
    if (pCanonic != nullptr) {
        *pCanonic = aTheCanonic;
    }
    return true;
}

bool INetURLObject::setHost(OUString const & rTheHost,
                            rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bHost)
        return false;
    OUStringBuffer aSynHost(rTheHost);
    bool bNetBiosName = false;
    switch (m_eScheme)
    {
        case INetProtocol::File:
            {
                OUString sTemp(aSynHost.toString());
                if (sTemp.equalsIgnoreAsciiCase("localhost"))
                {
                    aSynHost.setLength(0);
                }
                bNetBiosName = true;
            }
            break;
        case INetProtocol::Ldap:
            if (aSynHost.isEmpty() && m_aPort.isPresent())
                return false;
            break;

        default:
            if (aSynHost.isEmpty())
                return false;
            break;
    }
    if (!parseHostOrNetBiosName(
            aSynHost.getStr(), aSynHost.getStr() + aSynHost.getLength(),
            false/*bOctets*/, EncodeMechanism::WasEncoded, eCharset, bNetBiosName, &aSynHost))
        return false;
    sal_Int32 nDelta = m_aHost.set(m_aAbsURIRef, aSynHost.makeStringAndClear());
    m_aPort += nDelta;
    m_aPath += nDelta;
    m_aQuery += nDelta;
    m_aFragment += nDelta;
    return true;
}

// static
bool INetURLObject::parsePath(INetProtocol eScheme,
                              sal_Unicode const ** pBegin,
                              sal_Unicode const * pEnd,
                              bool bOctets,
                              EncodeMechanism eMechanism,
                              rtl_TextEncoding eCharset,
                              bool bSkippedInitialSlash,
                              sal_uInt32 nSegmentDelimiter,
                              sal_uInt32 nAltSegmentDelimiter,
                              sal_uInt32 nQueryDelimiter,
                              sal_uInt32 nFragmentDelimiter,
                              OUStringBuffer &rSynPath)
{
    DBG_ASSERT(pBegin, "INetURLObject::parsePath(): Null output param");

    sal_Unicode const * pPos = *pBegin;
    OUStringBuffer aTheSynPath;

    switch (eScheme)
    {
        case INetProtocol::NotValid:
            return false;

        case INetProtocol::Ftp:
            if (pPos < pEnd && *pPos != '/' && *pPos != nFragmentDelimiter)
                return false;
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_HTTP_PATH, eCharset, true);
            }
            if (aTheSynPath.isEmpty())
                aTheSynPath.append('/');
            break;

        case INetProtocol::Http:
        case INetProtocol::VndSunStarWebdav:
        case INetProtocol::Https:
        case INetProtocol::Smb:
        case INetProtocol::Cmis:
            if (pPos < pEnd && *pPos != '/' && *pPos != nFragmentDelimiter)
                return false;
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_HTTP_PATH, eCharset, true);
            }
            if (aTheSynPath.isEmpty())
                aTheSynPath.append('/');
            break;

        case INetProtocol::File:
        {
            if (bSkippedInitialSlash)
                aTheSynPath.append('/');
            else if (pPos < pEnd
                     && *pPos != nSegmentDelimiter
                     && *pPos != nAltSegmentDelimiter)
                return false;
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                if (eEscapeType == EscapeType::NONE)
                {
                    if (nUTF32 == nSegmentDelimiter
                        || nUTF32 == nAltSegmentDelimiter)
                    {
                        aTheSynPath.append('/');
                        continue;
                    }
                    else if (nUTF32 == '|'
                             && (pPos == pEnd
                                 || *pPos == nFragmentDelimiter
                                 || *pPos == nSegmentDelimiter
                                 || *pPos == nAltSegmentDelimiter)
                             && aTheSynPath.getLength() == 2
                             && rtl::isAsciiAlpha(aTheSynPath[1]))
                    {
                        // A first segment of <ALPHA "|"> is translated to
                        // <ALPHA ":">:
                        aTheSynPath.append(':');
                        continue;
                    }
                }
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_PCHAR, eCharset, true);
            }
            if (aTheSynPath.isEmpty())
                aTheSynPath.append('/');
            break;
        }

        case INetProtocol::Mailto:
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_MAILTO, eCharset, true);
            }
            break;


        case INetProtocol::PrivSoffice:
        case INetProtocol::Slot:
        case INetProtocol::Hid:
        case INetProtocol::Macro:
        case INetProtocol::Uno:
        case INetProtocol::Component:
        case INetProtocol::Ldap:
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_PATH_BEFORE_QUERY, eCharset, true);
            }
            break;

        case INetProtocol::VndSunStarHelp:
            if (pPos == pEnd
                || *pPos == nQueryDelimiter
                || *pPos == nFragmentDelimiter)
                aTheSynPath.append('/');
            else
            {
                if (*pPos != '/')
                    return false;
                while (pPos < pEnd && *pPos != nQueryDelimiter
                       && *pPos != nFragmentDelimiter)
                {
                    EscapeType eEscapeType;
                    sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                                 eMechanism,
                                                 eCharset, eEscapeType);
                    appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                               PART_HTTP_PATH, eCharset, true);
                }
            }
            break;

        case INetProtocol::Javascript:
        case INetProtocol::Data:
        case INetProtocol::Cid:
        case INetProtocol::Db:
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_URIC, eCharset, true);
            }
            break;

        case INetProtocol::VndSunStarHier:
        case INetProtocol::VndSunStarPkg:
            if (pPos < pEnd && *pPos != '/'
                && *pPos != nQueryDelimiter && *pPos != nFragmentDelimiter)
                return false;
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                if (eEscapeType == EscapeType::NONE && nUTF32 == '/')
                    aTheSynPath.append('/');
                else
                    appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                               PART_PCHAR, eCharset, false);
            }
            if (aTheSynPath.isEmpty())
                aTheSynPath.append('/');
            break;

        case INetProtocol::VndSunStarCmd:
        case INetProtocol::VndSunStarExpand:
        {
            if (pPos == pEnd || *pPos == nFragmentDelimiter)
                return false;
            Part ePart = PART_URIC_NO_SLASH;
            while (pPos != pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets, ePart,
                           eCharset, true);
                ePart = PART_URIC;
            }
            break;
        }

        case INetProtocol::Telnet:
            if (pPos < pEnd)
            {
                if (*pPos != '/' || pEnd - pPos > 1)
                    return false;
                ++pPos;
            }
            aTheSynPath.append('/');
            break;

        case INetProtocol::VndSunStarTdoc:
            if (pPos == pEnd || *pPos != '/')
                return false;
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                if (eEscapeType == EscapeType::NONE && nUTF32 == '/')
                    aTheSynPath.append('/');
                else
                    appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                               PART_PCHAR, eCharset, false);
            }
            break;

        case INetProtocol::Generic:
        case INetProtocol::Sftp:
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_URIC, eCharset, true);
            }
            if (aTheSynPath.isEmpty())
                return false;
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    *pBegin = pPos;
    rSynPath = aTheSynPath;
    return true;
}

bool INetURLObject::setPath(OUString const & rThePath,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    OUStringBuffer aSynPath;
    sal_Unicode const * p = rThePath.getStr();
    sal_Unicode const * pEnd = p + rThePath.getLength();
    if (!parsePath(m_eScheme, &p, pEnd, false/*bOctets*/, eMechanism, eCharset, false,
                   '/', 0x80000000, 0x80000000, 0x80000000, aSynPath)
        || p != pEnd)
        return false;
    sal_Int32 nDelta = m_aPath.set(m_aAbsURIRef, aSynPath.makeStringAndClear());
    m_aQuery += nDelta;
    m_aFragment += nDelta;
    return true;
}

bool INetURLObject::checkHierarchical() const {
    if (m_eScheme == INetProtocol::VndSunStarExpand) {
        OSL_FAIL(
            "INetURLObject::checkHierarchical vnd.sun.star.expand");
        return true;
    } else {
        return getSchemeInfo().m_bHierarchical;
    }
}

bool INetURLObject::Append(OUString const & rTheSegment,
                           EncodeMechanism eMechanism,
                           rtl_TextEncoding eCharset)
{
    return insertName(rTheSegment, false, LAST_SEGMENT, eMechanism, eCharset);
}

INetURLObject::SubString INetURLObject::getSegment(sal_Int32 nIndex,
                                                   bool bIgnoreFinalSlash)
    const
{
    DBG_ASSERT(nIndex >= 0 || nIndex == LAST_SEGMENT,
               "INetURLObject::getSegment(): Bad index");

    if (!checkHierarchical())
        return SubString();

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin;
    sal_Unicode const * pSegEnd;
    if (nIndex == LAST_SEGMENT)
    {
        pSegEnd = pPathEnd;
        if (bIgnoreFinalSlash && pSegEnd > pPathBegin && pSegEnd[-1] == '/')
            --pSegEnd;
        if (pSegEnd <= pPathBegin)
            return SubString();
        pSegBegin = pSegEnd - 1;
        while (pSegBegin > pPathBegin && *pSegBegin != '/')
            --pSegBegin;
    }
    else
    {
        pSegBegin = pPathBegin;
        while (nIndex-- > 0)
            do
            {
                ++pSegBegin;
                if (pSegBegin >= pPathEnd)
                    return SubString();
            }
            while (*pSegBegin != '/');
        pSegEnd = pSegBegin + 1;
        while (pSegEnd < pPathEnd && *pSegEnd != '/')
            ++pSegEnd;
    }

    return SubString(pSegBegin - m_aAbsURIRef.getStr(),
                     pSegEnd - pSegBegin);
}

bool INetURLObject::insertName(OUString const & rTheName,
                               bool bAppendFinalSlash, sal_Int32 nIndex,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset)
{
    DBG_ASSERT(nIndex >= 0 || nIndex == LAST_SEGMENT,
               "INetURLObject::insertName(): Bad index");

    if (!checkHierarchical())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pPrefixEnd;
    bool bInsertSlash;
    sal_Unicode const * pSuffixBegin;
    if (nIndex == LAST_SEGMENT)
    {
        pPrefixEnd = pPathEnd;
        if (pPrefixEnd > pPathBegin &&
            pPrefixEnd[-1] == '/')
        {
            --pPrefixEnd;
        }
        bInsertSlash = bAppendFinalSlash;
        pSuffixBegin = pPathEnd;
    }
    else if (nIndex == 0)
    {
        pPrefixEnd = pPathBegin;
        bInsertSlash =
            (pPathBegin < pPathEnd && *pPathBegin != '/') ||
            (pPathBegin == pPathEnd && bAppendFinalSlash);
        pSuffixBegin =
            (pPathEnd - pPathBegin == 1 && *pPathBegin == '/' &&
             !bAppendFinalSlash)
            ? pPathEnd : pPathBegin;
    }
    else
    {
        pPrefixEnd = pPathBegin;
        sal_Unicode const * pEnd = pPathEnd;
        if (pEnd > pPathBegin && pEnd[-1] == '/')
            --pEnd;
        bool bSkip = pPrefixEnd < pEnd && *pPrefixEnd == '/';
        bInsertSlash = false;
        pSuffixBegin = pPathEnd;
         while (nIndex-- > 0)
            for (;;)
            {
                if (bSkip)
                    ++pPrefixEnd;
                bSkip = true;
                if (pPrefixEnd >= pEnd)
                {
                    if (nIndex == 0)
                    {
                        bInsertSlash = bAppendFinalSlash;
                        break;
                    }
                    else
                        return false;
                }
                if (*pPrefixEnd == '/')
                {
                    pSuffixBegin = pPrefixEnd;
                    break;
                }
            }
    }

    OUStringBuffer aNewPath;
    aNewPath.append(pPathBegin, pPrefixEnd - pPathBegin);
    aNewPath.append('/');
    aNewPath.append(encodeText(rTheName, false/*bOctets*/, PART_PCHAR,
                           eMechanism, eCharset, true));
    if (bInsertSlash) {
        aNewPath.append('/');
    }
    aNewPath.append(pSuffixBegin, pPathEnd - pSuffixBegin);

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

bool INetURLObject::clearQuery()
{
    if (HasError())
        return false;
    if (m_aQuery.isPresent())
    {
        lcl_Erase(m_aAbsURIRef, m_aQuery.getBegin() - 1,
            m_aQuery.getLength() + 1);
        m_aFragment += m_aQuery.clear() - 1;
    }
    return false;
}

bool INetURLObject::setQuery(OUString const & rTheQuery,
                             EncodeMechanism eMechanism,
                             rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bQuery)
        return false;
    OUString aNewQuery(encodeText(rTheQuery, false/*bOctets*/, PART_URIC,
                                   eMechanism, eCharset, true));
    sal_Int32 nDelta;
    if (m_aQuery.isPresent())
        nDelta = m_aQuery.set(m_aAbsURIRef, aNewQuery);
    else
    {
        m_aAbsURIRef.insert(m_aPath.getEnd(), u'?');
        nDelta = m_aQuery.set(m_aAbsURIRef, aNewQuery, m_aPath.getEnd() + 1)
                     + 1;
    }
    m_aFragment += nDelta;
    return true;
}

bool INetURLObject::clearFragment()
{
    if (HasError())
        return false;
    if (m_aFragment.isPresent())
    {
        m_aAbsURIRef.setLength(m_aFragment.getBegin() - 1);
        m_aFragment.clear();
    }
    return true;
}

bool INetURLObject::setFragment(OUString const & rTheFragment,
                                EncodeMechanism eMechanism,
                                rtl_TextEncoding eCharset)
{
    if (HasError())
        return false;
    OUString aNewFragment(encodeText(rTheFragment, false/*bOctets*/, PART_URIC,
                                      eMechanism, eCharset, true));
    if (m_aFragment.isPresent())
        m_aFragment.set(m_aAbsURIRef, aNewFragment);
    else
    {
        m_aAbsURIRef.append('#');
        m_aFragment.set(m_aAbsURIRef, aNewFragment, m_aAbsURIRef.getLength());
    }
    return true;
}

bool INetURLObject::hasDosVolume(FSysStyle eStyle) const
{
    sal_Unicode const * p = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    return (eStyle & FSysStyle::Dos)
           && m_aPath.getLength() >= 3
           && p[0] == '/'
           && rtl::isAsciiAlpha(p[1])
           && p[2] == ':'
           && (m_aPath.getLength() == 3 || p[3] == '/');
}

// static
OUString INetURLObject::encodeText(sal_Unicode const * pBegin,
                                    sal_Unicode const * pEnd, bool bOctets,
                                    Part ePart, EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset,
                                    bool bKeepVisibleEscapes)
{
    OUStringBuffer aResult;
    while (pBegin < pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, bOctets,
                                     eMechanism, eCharset, eEscapeType);
        appendUCS4(aResult, nUTF32, eEscapeType, bOctets, ePart,
                   eCharset, bKeepVisibleEscapes);
    }
    return aResult.makeStringAndClear();
}

// static
OUString INetURLObject::decode(sal_Unicode const * pBegin,
                                sal_Unicode const * pEnd,
                                DecodeMechanism eMechanism,
                                rtl_TextEncoding eCharset)
{
    switch (eMechanism)
    {
        case DecodeMechanism::NONE:
            return OUString(pBegin, pEnd - pBegin);

        case DecodeMechanism::ToIUri:
            eCharset = RTL_TEXTENCODING_UTF8;
            break;

        default:
            break;
    }
    OUStringBuffer aResult;
    while (pBegin < pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, false,
                                     EncodeMechanism::WasEncoded, eCharset, eEscapeType);
        switch (eEscapeType)
        {
            case EscapeType::NONE:
                aResult.appendUtf32(nUTF32);
                break;

            case EscapeType::Octet:
                appendEscape(aResult, nUTF32);
                break;

            case EscapeType::Utf32:
                if (
                     rtl::isAscii(nUTF32) &&
                     (
                       eMechanism == DecodeMechanism::ToIUri ||
                       (
                         eMechanism == DecodeMechanism::Unambiguous &&
                         mustEncode(nUTF32, PART_UNAMBIGUOUS)
                       )
                     )
                   )
                {
                    appendEscape(aResult, nUTF32);
                }
                else
                    aResult.appendUtf32(nUTF32);
                break;
        }
    }
    return aResult.makeStringAndClear();
}

OUString INetURLObject::GetURLNoPass(DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset) const
{
    INetURLObject aTemp(*this);
    aTemp.clearPassword();
    return aTemp.GetMainURL(eMechanism, eCharset);
}

OUString INetURLObject::GetURLNoMark(DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset) const
{
    INetURLObject aTemp(*this);
    aTemp.clearFragment();
    return aTemp.GetMainURL(eMechanism, eCharset);
}

OUString
INetURLObject::getAbbreviated(
    uno::Reference< util::XStringWidth > const & rStringWidth,
    sal_Int32 nWidth,
    DecodeMechanism eMechanism,
    rtl_TextEncoding eCharset)
    const
{
    OSL_ENSURE(rStringWidth.is(), "specification violation");
    OUStringBuffer aBuffer;
    // make sure that the scheme is copied for generic schemes: getSchemeInfo().m_pScheme
    // is empty ("") in that case, so take the scheme from m_aAbsURIRef
    if (m_eScheme != INetProtocol::Generic)
    {
        aBuffer.appendAscii(getSchemeInfo().m_pScheme);
    }
    else
    {
        if (!m_aAbsURIRef.isEmpty())
        {
            sal_Unicode const * pSchemeBegin
                = m_aAbsURIRef.getStr();
            sal_Unicode const * pSchemeEnd = pSchemeBegin;

            while (pSchemeEnd[0] != ':')
            {
                ++pSchemeEnd;
            }
            aBuffer.append(pSchemeBegin, pSchemeEnd - pSchemeBegin);
        }
    }
    aBuffer.append(':');
    bool bAuthority = getSchemeInfo().m_bAuthority;
    sal_Unicode const * pCoreBegin
        = m_aAbsURIRef.getStr() + (bAuthority ? getAuthorityBegin() :
                                                   m_aPath.getBegin());
    sal_Unicode const * pCoreEnd
        = m_aAbsURIRef.getStr() + m_aPath.getBegin() + m_aPath.getLength();
    bool bSegment = false;
    if (getSchemeInfo().m_bHierarchical)
    {
        OUString aRest;
        if (m_aQuery.isPresent())
            aRest = "?...";
        else if (m_aFragment.isPresent())
            aRest = "#...";
        OUStringBuffer aTrailer;
        sal_Unicode const * pBegin = pCoreBegin;
        sal_Unicode const * pEnd = pCoreEnd;
        sal_Unicode const * pPrefixBegin = pBegin;
        sal_Unicode const * pSuffixEnd = pEnd;
        bool bPrefix = true;
        bool bSuffix = true;
        do
        {
            if (bSuffix)
            {
                sal_Unicode const * p = pSuffixEnd - 1;
                if (pSuffixEnd == pCoreEnd && *p == '/')
                    --p;
                while (*p != '/')
                    --p;
                if (bAuthority && p == pCoreBegin + 1)
                    --p;
                OUString
                    aSegment(decode(p + (p == pBegin && pBegin != pCoreBegin ?
                                             1 : 0),
                                    pSuffixEnd,
                                    eMechanism,
                                    eCharset));
                pSuffixEnd = p;
                OUStringBuffer aResult(aBuffer);
                if (pSuffixEnd != pBegin)
                    aResult.append("...");
                aResult.append(aSegment);
                aResult.append(aTrailer.toString());
                aResult.append(aRest);
                if (rStringWidth->
                            queryStringWidth(aResult.makeStringAndClear())
                        <= nWidth)
                {
                    aTrailer.insert(0, aSegment);
                    bSegment = true;
                    pEnd = pSuffixEnd;
                }
                else
                    bSuffix = false;
                if (pPrefixBegin > pSuffixEnd)
                    pPrefixBegin = pSuffixEnd;
                if (pBegin == pEnd)
                    break;
            }
            if (bPrefix)
            {
                sal_Unicode const * p
                    = pPrefixBegin
                          + (bAuthority && pPrefixBegin == pCoreBegin ? 2 :
                                                                        1);
                OSL_ASSERT(p <= pEnd);
                while (p < pEnd && *p != '/')
                    ++p;
                if (p == pCoreEnd - 1 && *p == '/')
                    ++p;
                OUString
                    aSegment(decode(pPrefixBegin
                                        + (pPrefixBegin == pCoreBegin ? 0 :
                                                                        1),
                                    p == pEnd ? p : p + 1,
                                    eMechanism,
                                    eCharset));
                pPrefixBegin = p;
                OUStringBuffer aResult(aBuffer);
                aResult.append(aSegment);
                if (pPrefixBegin != pEnd)
                    aResult.append("...");
                aResult.append(aTrailer.toString());
                aResult.append(aRest);
                if (rStringWidth->
                            queryStringWidth(aResult.makeStringAndClear())
                        <= nWidth)
                {
                    aBuffer.append(aSegment);
                    bSegment = true;
                    pBegin = pPrefixBegin;
                }
                else
                    bPrefix = false;
                if (pPrefixBegin > pSuffixEnd)
                    pSuffixEnd = pPrefixBegin;
                if (pBegin == pEnd)
                    break;
            }
        }
        while (bPrefix || bSuffix);
        if (bSegment)
        {
            if (pPrefixBegin != pBegin || pSuffixEnd != pEnd)
                aBuffer.append("...");
            aBuffer.append(aTrailer.toString());
        }
    }
    if (!bSegment)
        aBuffer.append(decode(pCoreBegin,
                              pCoreEnd,
                              eMechanism,
                              eCharset));
    if (m_aQuery.isPresent())
    {
        aBuffer.append('?');
        aBuffer.append(decode(m_aQuery, eMechanism, eCharset));
    }
    if (m_aFragment.isPresent())
    {
        aBuffer.append('#');
        aBuffer.append(decode(m_aFragment, eMechanism, eCharset));
    }
    if (!aBuffer.isEmpty())
    {
        OUStringBuffer aResult(aBuffer);
        if (rStringWidth->queryStringWidth(aResult.makeStringAndClear())
                > nWidth)
            for (sal_Int32 i = aBuffer.getLength();;)
            {
                if (i == 0)
                {
                    aBuffer.setLength(aBuffer.getLength() - 1);
                    if (aBuffer.isEmpty())
                        break;
                }
                else
                {
                    aBuffer.setLength(--i);
                    aBuffer.append("...");
                }
                aResult = aBuffer;
                if (rStringWidth->
                            queryStringWidth(aResult.makeStringAndClear())
                        <= nWidth)
                    break;
            }
    }
    return aBuffer.makeStringAndClear();
}

bool INetURLObject::operator ==(INetURLObject const & rObject) const
{
    if (m_eScheme != rObject.m_eScheme)
        return false;
    if (m_eScheme == INetProtocol::NotValid)
        return m_aAbsURIRef.toString() == rObject.m_aAbsURIRef.toString();
    if ((m_aScheme.compare(
             rObject.m_aScheme, m_aAbsURIRef, rObject.m_aAbsURIRef)
         != 0)
        || GetUser(DecodeMechanism::NONE) != rObject.GetUser(DecodeMechanism::NONE)
        || GetPass(DecodeMechanism::NONE) != rObject.GetPass(DecodeMechanism::NONE)
        || !GetHost(DecodeMechanism::NONE).equalsIgnoreAsciiCase(
            rObject.GetHost(DecodeMechanism::NONE))
        || GetPort() != rObject.GetPort()
        || HasParam() != rObject.HasParam()
        || GetParam() != rObject.GetParam())
        return false;
    OUString aPath1(GetURLPath(DecodeMechanism::NONE));
    OUString aPath2(rObject.GetURLPath(DecodeMechanism::NONE));
    switch (m_eScheme)
    {
        case INetProtocol::File:
        {
            // If the URL paths of two file URLs only differ in that one has a
            // final '/' and the other has not, take the two paths as
            // equivalent (this could be useful for other schemes, too):
            sal_Int32 nLength = aPath1.getLength();
            switch (nLength - aPath2.getLength())
            {
                case -1:
                    if (aPath2[nLength] != '/')
                        return false;
                    break;

                case 0:
                    break;

                case 1:
                    if (aPath1[--nLength] != '/')
                        return false;
                    break;

                default:
                    return false;
            }
            return aPath1.compareTo(aPath2, nLength) == 0;
        }

        default:
            return aPath1 == aPath2;
    }
}

bool INetURLObject::ConcatData(INetProtocol eTheScheme,
                               OUString const & rTheUser,
                               OUString const & rThePassword,
                               OUString const & rTheHost,
                               sal_uInt32 nThePort,
                               OUString const & rThePath)
{
    setInvalid();
    m_eScheme = eTheScheme;
    if (HasError() || m_eScheme == INetProtocol::Generic)
        return false;
    m_aAbsURIRef.setLength(0);
    m_aAbsURIRef.appendAscii(getSchemeInfo().m_pScheme);
    m_aAbsURIRef.append(':');
    if (getSchemeInfo().m_bAuthority)
    {
        m_aAbsURIRef.append("//");
        bool bUserInfo = false;
        if (getSchemeInfo().m_bUser)
        {
            if (!rTheUser.isEmpty())
            {
                m_aUser.set(m_aAbsURIRef,
                            encodeText(rTheUser, false, PART_USER_PASSWORD,
                                       EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, false),
                            m_aAbsURIRef.getLength());
                bUserInfo = true;
            }
        }
        else if (!rTheUser.isEmpty())
        {
            setInvalid();
            return false;
        }
        if (!rThePassword.isEmpty())
        {
            if (getSchemeInfo().m_bPassword)
            {
                m_aAbsURIRef.append(':');
                m_aAuth.set(m_aAbsURIRef,
                            encodeText(rThePassword, false, PART_USER_PASSWORD,
                                       EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, false),
                            m_aAbsURIRef.getLength());
                bUserInfo = true;
            }
            else
            {
                setInvalid();
                return false;
            }
        }
        if (bUserInfo && getSchemeInfo().m_bHost)
            m_aAbsURIRef.append('@');
        if (getSchemeInfo().m_bHost)
        {
            OUStringBuffer aSynHost(rTheHost);
            bool bNetBiosName = false;
            switch (m_eScheme)
            {
                case INetProtocol::File:
                    {
                        OUString sTemp(aSynHost.toString());
                        if (sTemp.equalsIgnoreAsciiCase( "localhost" ))
                        {
                            aSynHost.setLength(0);
                        }
                        bNetBiosName = true;
                    }
                    break;

                case INetProtocol::Ldap:
                    if (aSynHost.isEmpty() && nThePort != 0)
                    {
                        setInvalid();
                        return false;
                    }
                    break;

                default:
                    if (aSynHost.isEmpty())
                    {
                        setInvalid();
                        return false;
                    }
                    break;
            }
            if (!parseHostOrNetBiosName(
                    aSynHost.getStr(), aSynHost.getStr() + aSynHost.getLength(),
                    false, EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, bNetBiosName, &aSynHost))
            {
                setInvalid();
                return false;
            }
            m_aHost.set(m_aAbsURIRef, aSynHost.makeStringAndClear(),
                m_aAbsURIRef.getLength());
            if (nThePort != 0)
            {
                if (getSchemeInfo().m_bPort)
                {
                    m_aAbsURIRef.append(':');
                    m_aPort.set(m_aAbsURIRef,
                                OUString::number(nThePort),
                                m_aAbsURIRef.getLength());
                }
                else
                {
                    setInvalid();
                    return false;
                }
            }
        }
        else if (!rTheHost.isEmpty() || nThePort != 0)
        {
            setInvalid();
            return false;
        }
    }
    OUStringBuffer aSynPath;
    sal_Unicode const * p = rThePath.getStr();
    sal_Unicode const * pEnd = p + rThePath.getLength();
    if (!parsePath(m_eScheme, &p, pEnd, false, EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, false, '/',
                   0x80000000, 0x80000000, 0x80000000, aSynPath)
        || p != pEnd)
    {
        setInvalid();
        return false;
    }
    m_aPath.set(m_aAbsURIRef, aSynPath.makeStringAndClear(),
        m_aAbsURIRef.getLength());
    return true;
}

// static
OUString INetURLObject::GetAbsURL(OUString const & rTheBaseURIRef,
                                       OUString const & rTheRelURIRef,
                                       bool bIgnoreFragment,
                                       EncodeMechanism eEncodeMechanism,
                                       DecodeMechanism eDecodeMechanism,
                                       rtl_TextEncoding eCharset)
{
    // Backwards compatibility:
    if (rTheRelURIRef.isEmpty() || rTheRelURIRef[0] == '#')
        return rTheRelURIRef;

    INetURLObject aTheAbsURIRef;
    bool bWasAbsolute;
    return INetURLObject(rTheBaseURIRef, eEncodeMechanism, eCharset).
            convertRelToAbs(rTheRelURIRef, aTheAbsURIRef,
                            bWasAbsolute, eEncodeMechanism,
                            eCharset, bIgnoreFragment, false,
                            false, FSysStyle::Detect)
           || eEncodeMechanism != EncodeMechanism::WasEncoded
           || eDecodeMechanism != DecodeMechanism::ToIUri
           || eCharset != RTL_TEXTENCODING_UTF8 ?
               aTheAbsURIRef.GetMainURL(eDecodeMechanism, eCharset) :
               rTheRelURIRef;
}

OUString INetURLObject::getExternalURL() const
{
    OUString aTheExtURIRef;
    translateToExternal(
        m_aAbsURIRef.toString(), aTheExtURIRef);
    return aTheExtURIRef;
}

bool INetURLObject::isSchemeEqualTo(OUString const & scheme) const {
    return m_aScheme.isPresent()
        && (rtl_ustr_compareIgnoreAsciiCase_WithLength(
                scheme.getStr(), scheme.getLength(),
                m_aAbsURIRef.getStr() + m_aScheme.getBegin(),
                m_aScheme.getLength())
            == 0);
}

bool INetURLObject::isAnyKnownWebDAVScheme() const {
    return ( isSchemeEqualTo( INetProtocol::Http ) ||
             isSchemeEqualTo( INetProtocol::Https ) ||
             isSchemeEqualTo( INetProtocol::VndSunStarWebdav ) ||
             isSchemeEqualTo( "vnd.sun.star.webdavs" ) );
}

// static
OUString INetURLObject::GetScheme(INetProtocol eTheScheme)
{
    return OUString::createFromAscii(getSchemeInfo(eTheScheme).m_pPrefix);
}

// static
OUString INetURLObject::GetSchemeName(INetProtocol eTheScheme)
{
    return OUString::createFromAscii(getSchemeInfo(eTheScheme).m_pScheme);
}

// static
INetProtocol INetURLObject::CompareProtocolScheme(OUString const &
                                                      rTheAbsURIRef)
{
    sal_Unicode const * p = rTheAbsURIRef.getStr();
    PrefixInfo const * pPrefix = getPrefix(p, p + rTheAbsURIRef.getLength());
    return pPrefix ? pPrefix->m_eScheme : INetProtocol::NotValid;
}

OUString INetURLObject::GetHostPort(DecodeMechanism eMechanism,
                                     rtl_TextEncoding eCharset)
{
    // Check because PROT_VND_SUN_STAR_HELP, PROT_VND_SUN_STAR_HIER, and
    // PROT_VND_SUN_STAR_PKG misuse m_aHost:
    if (!getSchemeInfo().m_bHost)
        return OUString();
    OUStringBuffer aHostPort(decode(m_aHost, eMechanism, eCharset));
    if (m_aPort.isPresent())
    {
        aHostPort.append(':');
        aHostPort.append(decode(m_aPort, eMechanism, eCharset));
    }
    return aHostPort.makeStringAndClear();
}

sal_uInt32 INetURLObject::GetPort() const
{
    if (m_aPort.isPresent())
    {
        sal_Unicode const * p = m_aAbsURIRef.getStr() + m_aPort.getBegin();
        sal_Unicode const * pEnd = p + m_aPort.getLength();
        sal_uInt32 nThePort;
        if (INetMIME::scanUnsigned(p, pEnd, true, nThePort) && p == pEnd)
            return nThePort;
    }
    return 0;
}

bool INetURLObject::SetPort(sal_uInt32 nThePort)
{
    if (getSchemeInfo().m_bPort && m_aHost.isPresent())
    {
        OUString aNewPort(OUString::number(nThePort));
        sal_Int32 nDelta;
        if (m_aPort.isPresent())
            nDelta = m_aPort.set(m_aAbsURIRef, aNewPort);
        else
        {
            m_aAbsURIRef.insert(m_aHost.getEnd(), u':');
            nDelta = m_aPort.set(m_aAbsURIRef, aNewPort, m_aHost.getEnd() + 1)
                         + 1;
        }
        m_aPath += nDelta;
        m_aQuery += nDelta;
        m_aFragment += nDelta;
        return true;
    }
    return false;
}

sal_Int32 INetURLObject::getSegmentCount(bool bIgnoreFinalSlash) const
{
    if (!checkHierarchical())
        return 0;

    sal_Unicode const * p = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pEnd = p + m_aPath.getLength();
    if (bIgnoreFinalSlash && pEnd > p && pEnd[-1] == '/')
        --pEnd;
    sal_Int32 n = p == pEnd || *p == '/' ? 0 : 1;
    while (p != pEnd)
        if (*p++ == '/')
            ++n;
    return n;
}

bool INetURLObject::removeSegment(sal_Int32 nIndex, bool bIgnoreFinalSlash)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    OUStringBuffer aNewPath;
    aNewPath.append(m_aAbsURIRef.getStr() + m_aPath.getBegin(),
                       aSegment.getBegin() - m_aPath.getBegin());
    if (bIgnoreFinalSlash && aSegment.getEnd() == m_aPath.getEnd())
        aNewPath.append('/');
    else
        aNewPath.append(m_aAbsURIRef.getStr() + aSegment.getEnd(),
                        m_aPath.getEnd() - aSegment.getEnd());
    if (aNewPath.isEmpty() && !aSegment.isEmpty() &&
        m_aAbsURIRef[aSegment.getBegin()] == '/')
    {
        aNewPath.append('/');
    }

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

OUString INetURLObject::getName(sal_Int32 nIndex, bool bIgnoreFinalSlash,
                                 DecodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset) const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return OUString();

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * p = pSegBegin;
    while (p != pSegEnd && *p != ';')
        ++p;

    return decode(pSegBegin, p, eMechanism, eCharset);
}

bool INetURLObject::setName(OUString const & rTheName)
{
    SubString aSegment(getSegment(LAST_SEGMENT, true));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * p = pSegBegin;
    while (p != pSegEnd && *p != ';')
        ++p;

    OUStringBuffer aNewPath;
    aNewPath.append(pPathBegin, pSegBegin - pPathBegin);
    aNewPath.append(encodeText(rTheName, false, PART_PCHAR,
        EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, true));
    aNewPath.append(p, pPathEnd - p);

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

bool INetURLObject::hasExtension()
    const
{
    SubString aSegment(getSegment(LAST_SEGMENT, true/*bIgnoreFinalSlash*/));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    for (sal_Unicode const * p = pSegBegin; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            return true;
    return false;
}

OUString INetURLObject::getBase(sal_Int32 nIndex, bool bIgnoreFinalSlash,
                                 DecodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset) const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return OUString();

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * pExtension = nullptr;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        pExtension = p;

    return decode(pSegBegin, pExtension, eMechanism, eCharset);
}

bool INetURLObject::setBase(OUString const & rTheBase, sal_Int32 nIndex,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    SubString aSegment(getSegment(nIndex, true/*bIgnoreFinalSlash*/));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * pExtension = nullptr;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        pExtension = p;

    OUStringBuffer aNewPath;
    aNewPath.append(pPathBegin, pSegBegin - pPathBegin);
    aNewPath.append(encodeText(rTheBase, false, PART_PCHAR,
        eMechanism, eCharset, true));
    aNewPath.append(pExtension, pPathEnd - pExtension);

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

OUString INetURLObject::getExtension(sal_Int32 nIndex,
                                      bool bIgnoreFinalSlash,
                                      DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset) const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return OUString();

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * pExtension = nullptr;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;

    if (!pExtension)
        return OUString();

    return decode(pExtension + 1, p, eMechanism, eCharset);
}

bool INetURLObject::setExtension(OUString const & rTheExtension,
                                 sal_Int32 nIndex, bool bIgnoreFinalSlash,
                                 rtl_TextEncoding eCharset)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * pExtension = nullptr;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        pExtension = p;

    OUStringBuffer aNewPath;
    aNewPath.append(pPathBegin, pExtension - pPathBegin);
    aNewPath.append('.');
    aNewPath.append(encodeText(rTheExtension, false, PART_PCHAR,
        EncodeMechanism::WasEncoded, eCharset, true));
    aNewPath.append(p, pPathEnd - p);

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

bool INetURLObject::removeExtension(sal_Int32 nIndex, bool bIgnoreFinalSlash)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.getStr() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    if (pSegBegin < pSegEnd && *pSegBegin == '/')
        ++pSegBegin;
    sal_Unicode const * pExtension = nullptr;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        return true;

    OUStringBuffer aNewPath;
    aNewPath.append(pPathBegin, pExtension - pPathBegin);
    aNewPath.append(p, pPathEnd - p);

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

bool INetURLObject::hasFinalSlash() const
{
    if (!checkHierarchical())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    return pPathEnd > pPathBegin && pPathEnd[-1] == '/';
}

bool INetURLObject::setFinalSlash()
{
    if (!checkHierarchical())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathEnd > pPathBegin && pPathEnd[-1] == '/')
        return true;

    OUStringBuffer aNewPath;
    aNewPath.append(pPathBegin, pPathEnd - pPathBegin);
    aNewPath.append('/');

    return setPath(aNewPath.makeStringAndClear(), EncodeMechanism::NotCanonical,
        RTL_TEXTENCODING_UTF8);
}

bool INetURLObject::removeFinalSlash()
{
    if (!checkHierarchical())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.getStr() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathEnd <= pPathBegin || pPathEnd[-1] != '/')
        return true;

    --pPathEnd;
    if (pPathEnd == pPathBegin && *pPathBegin == '/')
        return false;
    OUString aNewPath(pPathBegin, pPathEnd - pPathBegin);

    return setPath(aNewPath, EncodeMechanism::NotCanonical, RTL_TEXTENCODING_UTF8);
}

bool INetURLObject::setFSysPath(OUString const & rFSysPath,
    FSysStyle eStyle)
{
    sal_Unicode const * pFSysBegin = rFSysPath.getStr();
    sal_Unicode const * pFSysEnd = pFSysBegin + rFSysPath.getLength();

    switch (((eStyle & FSysStyle::Vos) ? 1 : 0)
                + ((eStyle & FSysStyle::Unix) ? 1 : 0)
                + ((eStyle & FSysStyle::Dos) ? 1 : 0))
    {
        case 0:
            return false;

        case 1:
            break;

        default:
            if (eStyle & FSysStyle::Vos
                && pFSysEnd - pFSysBegin >= 2
                && pFSysBegin[0] == '/'
                && pFSysBegin[1] == '/')
            {
                if (pFSysEnd - pFSysBegin >= 3
                    && pFSysBegin[2] == '.'
                    && (pFSysEnd - pFSysBegin == 3 || pFSysBegin[3] == '/'))
                {
                    eStyle = FSysStyle::Vos; // Production T1
                    break;
                }

                sal_Unicode const * p = pFSysBegin + 2;
                OUString aHost;
                if (parseHost(p, pFSysEnd, aHost)
                    && (p == pFSysEnd || *p == '/'))
                {
                    eStyle = FSysStyle::Vos; // Production T2
                    break;
                }
            }

            if (eStyle & FSysStyle::Dos
                && pFSysEnd - pFSysBegin >= 2
                && pFSysBegin[0] == '\\'
                && pFSysBegin[1] == '\\')
            {
                sal_Unicode const * p = pFSysBegin + 2;
                OUString aHost;
                if (parseHost(p, pFSysEnd, aHost)
                    && (p == pFSysEnd || *p == '\\'))
                {
                    eStyle = FSysStyle::Dos; // Production T3
                    break;
                }
            }

            if (eStyle & FSysStyle::Dos
                && pFSysEnd - pFSysBegin >= 2
                && rtl::isAsciiAlpha(pFSysBegin[0])
                && pFSysBegin[1] == ':'
                && (pFSysEnd - pFSysBegin == 2
                    || pFSysBegin[2] == '/'
                    || pFSysBegin[2] == '\\'))
            {
                eStyle = FSysStyle::Dos; // Productions T4, T5
                break;
            }

            if (!(eStyle & (FSysStyle::Unix | FSysStyle::Dos)))
                return false;

            eStyle = guessFSysStyleByCounting(pFSysBegin, pFSysEnd, eStyle);
                // Production T6
            break;
    }

    OUStringBuffer aSynAbsURIRef("file://");

    switch (eStyle)
    {
        case FSysStyle::Vos:
        {
            sal_Unicode const * p = pFSysBegin;
            if (pFSysEnd - p < 2 || *p++ != '/' || *p++ != '/')
                return false;
            if (p != pFSysEnd && *p == '.'
                && (pFSysEnd - p == 1 || p[1] == '/'))
                ++p;
            for (; p != pFSysEnd; ++p)
                switch (*p)
                {
                    case '#':
                    case '%':
                        appendEscape(aSynAbsURIRef, *p);
                        break;

                    default:
                        aSynAbsURIRef.append(*p);
                        break;
                }
            break;
        }

        case FSysStyle::Unix:
        {
            sal_Unicode const * p = pFSysBegin;
            if (p != pFSysEnd && *p != '/')
                return false;
            for (; p != pFSysEnd; ++p)
                switch (*p)
                {
                    case '|':
                    case '#':
                    case '%':
                        appendEscape(aSynAbsURIRef, *p);
                        break;

                    default:
                        aSynAbsURIRef.append(*p);
                        break;
                }
            break;
        }

        case FSysStyle::Dos:
        {
            sal_uInt32 nAltDelimiter = 0x80000000;
            sal_Unicode const * p = pFSysBegin;
            if (pFSysEnd - p >= 3 && p[0] == '\\' && p[1] == '\\')
                p += 2;
            else
            {
                aSynAbsURIRef.append('/');
                if (pFSysEnd - p >= 2
                    && rtl::isAsciiAlpha(p[0])
                    && p[1] == ':'
                    && (pFSysEnd - p == 2 || p[2] == '\\' || p[2] == '/'))
                    nAltDelimiter = '/';
            }
            for (; p != pFSysEnd; ++p)
                if (*p == '\\' || *p == nAltDelimiter)
                    aSynAbsURIRef.append('/');
                else
                    switch (*p)
                    {
                        case '/':
                        case '#':
                        case '%':
                            appendEscape(aSynAbsURIRef, *p);
                            break;

                        default:
                            aSynAbsURIRef.append(*p);
                            break;
                    }
            break;
        }

        default:
            OSL_ASSERT(false);
            break;
    }

    INetURLObject aTemp(aSynAbsURIRef.makeStringAndClear(), EncodeMechanism::WasEncoded,
        RTL_TEXTENCODING_UTF8);
    if (aTemp.HasError())
        return false;

    *this = aTemp;
    return true;
}

OUString INetURLObject::getFSysPath(FSysStyle eStyle,
                                     sal_Unicode * pDelimiter) const
{
    if (m_eScheme != INetProtocol::File)
        return OUString();

    if (((eStyle & FSysStyle::Vos) ? 1 : 0)
                + ((eStyle & FSysStyle::Unix) ? 1 : 0)
                + ((eStyle & FSysStyle::Dos) ? 1 : 0)
            > 1)
    {
        if(eStyle & FSysStyle::Vos && m_aHost.isPresent() && m_aHost.getLength() > 0)
        {
            eStyle= FSysStyle::Vos;
        }
        else
        {
            if(hasDosVolume(eStyle) || ((eStyle & FSysStyle::Dos) && m_aHost.isPresent() && m_aHost.getLength() > 0))
            {
                eStyle = FSysStyle::Dos;
            }
            else
            {
                if(eStyle & FSysStyle::Unix && (!m_aHost.isPresent() || m_aHost.getLength() == 0))
                {
                    eStyle = FSysStyle::Unix;
                }
                else
                {
                    eStyle= FSysStyle(0);
                }
            }
        }
    }

    switch (eStyle)
    {
        case FSysStyle::Vos:
        {
            if (pDelimiter)
                *pDelimiter = '/';

            OUStringBuffer aSynFSysPath;
            aSynFSysPath.append("//");
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
                aSynFSysPath.append(decode(m_aHost, DecodeMechanism::WithCharset,
                                       RTL_TEXTENCODING_UTF8));
            else
                aSynFSysPath.append('.');
            aSynFSysPath.append(decode(m_aPath, DecodeMechanism::WithCharset,
                                   RTL_TEXTENCODING_UTF8));
            return aSynFSysPath.makeStringAndClear();
        }

        case FSysStyle::Unix:
        {
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
                return OUString();

            if (pDelimiter)
                *pDelimiter = '/';

            return decode(m_aPath, DecodeMechanism::WithCharset, RTL_TEXTENCODING_UTF8);
        }

        case FSysStyle::Dos:
        {
            if (pDelimiter)
                *pDelimiter = '\\';

            OUStringBuffer aSynFSysPath;
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
            {
                aSynFSysPath.append("\\\\");
                aSynFSysPath.append(decode(m_aHost, DecodeMechanism::WithCharset,
                                       RTL_TEXTENCODING_UTF8));
                aSynFSysPath.append('\\');
            }
            sal_Unicode const * p
                = m_aAbsURIRef.getStr() + m_aPath.getBegin();
            sal_Unicode const * pEnd = p + m_aPath.getLength();
            DBG_ASSERT(p < pEnd && *p == '/',
                       "INetURLObject::getFSysPath(): Bad path");
            ++p;
            while (p < pEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(p, pEnd, false, EncodeMechanism::WasEncoded,
                                             RTL_TEXTENCODING_UTF8,
                                             eEscapeType);
                if (eEscapeType == EscapeType::NONE && nUTF32 == '/')
                    aSynFSysPath.append('\\');
                else
                    aSynFSysPath.appendUtf32(nUTF32);
            }
            return aSynFSysPath.makeStringAndClear();
        }

        default:
            return OUString();
    }
}

// static
void INetURLObject::appendUCS4Escape(OUStringBuffer & rTheText,
                                     sal_uInt32 nUCS4)
{
    DBG_ASSERT(nUCS4 < 0x80000000,
               "INetURLObject::appendUCS4Escape(): Bad char");
    if (nUCS4 < 0x80)
        appendEscape(rTheText, nUCS4);
    else if (nUCS4 < 0x800)
    {
        appendEscape(rTheText, nUCS4 >> 6 | 0xC0);
        appendEscape(rTheText, (nUCS4 & 0x3F) | 0x80);
    }
    else if (nUCS4 < 0x10000)
    {
        appendEscape(rTheText, nUCS4 >> 12 | 0xE0);
        appendEscape(rTheText, (nUCS4 >> 6 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 & 0x3F) | 0x80);
    }
    else if (nUCS4 < 0x200000)
    {
        appendEscape(rTheText, nUCS4 >> 18 | 0xF0);
        appendEscape(rTheText, (nUCS4 >> 12 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 >> 6 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 & 0x3F) | 0x80);
    }
    else if (nUCS4 < 0x4000000)
    {
        appendEscape(rTheText, nUCS4 >> 24 | 0xF8);
        appendEscape(rTheText, (nUCS4 >> 18 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 >> 12 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 >> 6 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 & 0x3F) | 0x80);
    }
    else
    {
        appendEscape(rTheText, nUCS4 >> 30 | 0xFC);
        appendEscape(rTheText, (nUCS4 >> 24 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 >> 18 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 >> 12 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 >> 6 & 0x3F) | 0x80);
        appendEscape(rTheText, (nUCS4 & 0x3F) | 0x80);
    }
}

// static
void INetURLObject::appendUCS4(OUStringBuffer& rTheText, sal_uInt32 nUCS4,
                               EscapeType eEscapeType, bool bOctets,
                               Part ePart, rtl_TextEncoding eCharset,
                               bool bKeepVisibleEscapes)
{
    bool bEscape;
    rtl_TextEncoding eTargetCharset = RTL_TEXTENCODING_DONTKNOW;
    switch (eEscapeType)
    {
        case EscapeType::NONE:
            if (mustEncode(nUCS4, ePart))
            {
                bEscape = true;
                eTargetCharset = bOctets ? RTL_TEXTENCODING_ISO_8859_1 :
                                           RTL_TEXTENCODING_UTF8;
            }
            else
                bEscape = false;
            break;

        case EscapeType::Octet:
            bEscape = true;
            eTargetCharset = RTL_TEXTENCODING_ISO_8859_1;
            break;

        case EscapeType::Utf32:
            if (mustEncode(nUCS4, ePart))
            {
                bEscape = true;
                eTargetCharset = eCharset;
            }
            else if (bKeepVisibleEscapes && INetMIME::isVisible(nUCS4))
            {
                bEscape = true;
                eTargetCharset = RTL_TEXTENCODING_ASCII_US;
            }
            else
                bEscape = false;
            break;
        default:
            bEscape = false;
    }

    if (bEscape)
    {
        switch (eTargetCharset)
        {
            default:
                OSL_FAIL("INetURLObject::appendUCS4(): Unsupported charset");
                SAL_FALLTHROUGH;
            case RTL_TEXTENCODING_ASCII_US:
            case RTL_TEXTENCODING_ISO_8859_1:
                appendEscape(rTheText, nUCS4);
                break;
            case RTL_TEXTENCODING_UTF8:
                appendUCS4Escape(rTheText, nUCS4);
                break;
        }
    }
    else
        rTheText.append(sal_Unicode(nUCS4));
}

// static
sal_uInt32 INetURLObject::getUTF32(sal_Unicode const *& rBegin,
                                   sal_Unicode const * pEnd, bool bOctets,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset,
                                   EscapeType & rEscapeType)
{
    DBG_ASSERT(rBegin < pEnd, "INetURLObject::getUTF32(): Bad sequence");
    sal_uInt32 nUTF32 = bOctets ? *rBegin++ :
                                  INetMIME::getUTF32Character(rBegin, pEnd);
    switch (eMechanism)
    {
        case EncodeMechanism::All:
            rEscapeType = EscapeType::NONE;
            break;

        case EncodeMechanism::WasEncoded:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == static_cast<unsigned char>('%') && rBegin + 1 < pEnd
                && (nWeight1 = INetMIME::getHexWeight(rBegin[0])) >= 0
                && (nWeight2 = INetMIME::getHexWeight(rBegin[1])) >= 0)
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                switch (eCharset)
                {
                    default:
                        OSL_FAIL(
                            "INetURLObject::getUTF32(): Unsupported charset");
                        SAL_FALLTHROUGH;
                    case RTL_TEXTENCODING_ASCII_US:
                        rEscapeType = rtl::isAscii(nUTF32) ?
                                          EscapeType::Utf32 : EscapeType::Octet;
                        break;

                    case RTL_TEXTENCODING_ISO_8859_1:
                        rEscapeType = EscapeType::Utf32;
                        break;

                    case RTL_TEXTENCODING_UTF8:
                        if (rtl::isAscii(nUTF32))
                            rEscapeType = EscapeType::Utf32;
                        else
                        {
                            if (nUTF32 >= 0xC0 && nUTF32 <= 0xF4)
                            {
                                sal_uInt32 nEncoded;
                                int nShift;
                                sal_uInt32 nMin;
                                if (nUTF32 <= 0xDF)
                                {
                                    nEncoded = (nUTF32 & 0x1F) << 6;
                                    nShift = 0;
                                    nMin = 0x80;
                                }
                                else if (nUTF32 <= 0xEF)
                                {
                                    nEncoded = (nUTF32 & 0x0F) << 12;
                                    nShift = 6;
                                    nMin = 0x800;
                                }
                                else
                                {
                                    nEncoded = (nUTF32 & 0x07) << 18;
                                    nShift = 12;
                                    nMin = 0x10000;
                                }
                                sal_Unicode const * p = rBegin;
                                bool bUTF8 = true;
                                for (;;)
                                {
                                    if (pEnd - p < 3
                                        || p[0] != '%'
                                        || (nWeight1
                                               = INetMIME::getHexWeight(p[1]))
                                               < 8
                                        || nWeight1 > 11
                                        || (nWeight2
                                               = INetMIME::getHexWeight(p[2]))
                                               < 0)
                                    {
                                        bUTF8 = false;
                                        break;
                                    }
                                    p += 3;
                                    nEncoded
                                        |= ((nWeight1 & 3) << 4 | nWeight2)
                                               << nShift;
                                    if (nShift == 0)
                                        break;
                                    nShift -= 6;
                                }
                                if (bUTF8 && rtl::isUnicodeCodePoint(nEncoded)
                                    && nEncoded >= nMin
                                    && !rtl::isHighSurrogate(nEncoded)
                                    && !rtl::isLowSurrogate(nEncoded))
                                {
                                    rBegin = p;
                                    nUTF32 = nEncoded;
                                    rEscapeType = EscapeType::Utf32;
                                    break;
                                }
                            }
                            rEscapeType = EscapeType::Octet;
                        }
                        break;
                }
            }
            else
                rEscapeType = EscapeType::NONE;
            break;
        }

        case EncodeMechanism::NotCanonical:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == static_cast<unsigned char>('%') && rBegin + 1 < pEnd
                && ((nWeight1 = INetMIME::getHexWeight(rBegin[0])) >= 0)
                && ((nWeight2 = INetMIME::getHexWeight(rBegin[1])) >= 0))
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                rEscapeType = EscapeType::Octet;
            }
            else
                rEscapeType = EscapeType::NONE;
            break;
        }
    }
    return nUTF32;
}

// static
sal_uInt32 INetURLObject::scanDomain(sal_Unicode const *& rBegin,
                                     sal_Unicode const * pEnd,
                                     bool bEager)
{
    enum State { STATE_DOT, STATE_LABEL, STATE_HYPHEN };
    State eState = STATE_DOT;
    sal_Int32 nLabels = 0;
    sal_Unicode const * pLastAlphanumeric = nullptr;
    for (sal_Unicode const * p = rBegin;; ++p)
        switch (eState)
        {
            case STATE_DOT:
                if (p != pEnd && (rtl::isAsciiAlphanumeric(*p) || *p == '_'))
                {
                    ++nLabels;
                    eState = STATE_LABEL;
                    break;
                }
                if (bEager || nLabels == 0)
                    return 0;
                rBegin = p - 1;
                return nLabels;

            case STATE_LABEL:
                if (p != pEnd)
                {
                    if (rtl::isAsciiAlphanumeric(*p) || *p == '_')
                        break;
                    else if (*p == '.')
                    {
                        eState = STATE_DOT;
                        break;
                    }
                    else if (*p == '-')
                    {
                        pLastAlphanumeric = p;
                        eState = STATE_HYPHEN;
                        break;
                    }
                }
                rBegin = p;
                return nLabels;

            case STATE_HYPHEN:
                if (p != pEnd)
                {
                    if (rtl::isAsciiAlphanumeric(*p) || *p == '_')
                    {
                        eState = STATE_LABEL;
                        break;
                    }
                    else if (*p == '-')
                        break;
                }
                if (bEager)
                    return 0;
                rBegin = pLastAlphanumeric;
                return nLabels;
        }
}

// static
bool INetURLObject::scanIPv6reference(sal_Unicode const *& rBegin,
                                      sal_Unicode const * pEnd)
{
    if (rBegin != pEnd && *rBegin == '[') {
        sal_Unicode const * p = rBegin + 1;
        //TODO: check for valid IPv6address (RFC 2373):
        while (p != pEnd && (rtl::isAsciiHexDigit(*p) || *p == ':' || *p == '.'))
        {
            ++p;
        }
        if (p != pEnd && *p == ']') {
            rBegin = p + 1;
            return true;
        }
    }
    return false;
}

OUString INetURLObject::GetPartBeforeLastName()
    const
{
    if (!checkHierarchical())
        return OUString();
    INetURLObject aTemp(*this);
    aTemp.clearFragment();
    aTemp.clearQuery();
    aTemp.removeSegment(LAST_SEGMENT, false);
    aTemp.setFinalSlash();
    return aTemp.GetMainURL(DecodeMechanism::ToIUri);
}

OUString INetURLObject::GetLastName(DecodeMechanism eMechanism,
                                     rtl_TextEncoding eCharset) const
{
    return getName(LAST_SEGMENT, true, eMechanism, eCharset);
}

OUString INetURLObject::GetFileExtension() const
{
    return getExtension(LAST_SEGMENT, false);
}

void INetURLObject::CutLastName()
{
    INetURLObject aTemp(*this);
    aTemp.clearFragment();
    aTemp.clearQuery();
    if (!aTemp.removeSegment(LAST_SEGMENT, false))
        return;
    *this = aTemp;
}

OUString INetURLObject::PathToFileName() const
{
    if (m_eScheme != INetProtocol::File)
        return OUString();
    OUString aSystemPath;
    if (osl::FileBase::getSystemPathFromFileURL(
                decode(m_aAbsURIRef.getStr(),
                       m_aAbsURIRef.getStr() + m_aPath.getEnd(),
                       DecodeMechanism::NONE, RTL_TEXTENCODING_UTF8),
                aSystemPath)
            != osl::FileBase::E_None)
        return OUString();
    return aSystemPath;
}

OUString INetURLObject::GetFull() const
{
    INetURLObject aTemp(*this);
    aTemp.removeFinalSlash();
    return aTemp.PathToFileName();
}

OUString INetURLObject::GetPath() const
{
    INetURLObject aTemp(*this);
    aTemp.removeSegment();
    aTemp.removeFinalSlash();
    return aTemp.PathToFileName();
}

void INetURLObject::SetBase(OUString const & rTheBase)
{
    setBase(rTheBase, LAST_SEGMENT, EncodeMechanism::All);
}

OUString INetURLObject::GetBase() const
{
    return getBase(LAST_SEGMENT, true, DecodeMechanism::WithCharset);
}

void INetURLObject::SetName(OUString const & rTheName,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    INetURLObject aTemp(*this);
    if (aTemp.removeSegment()
        && aTemp.insertName(rTheName, false, LAST_SEGMENT, eMechanism,
                            eCharset))
        *this = aTemp;
}

void INetURLObject::SetExtension(OUString const & rTheExtension)
{
    setExtension(rTheExtension, LAST_SEGMENT, false);
}

OUString INetURLObject::CutExtension()
{
    OUString aTheExtension(getExtension(LAST_SEGMENT, false));
    return removeExtension(LAST_SEGMENT, false)
        ? aTheExtension : OUString();
}

bool INetURLObject::IsExoticProtocol() const
{
    return m_eScheme == INetProtocol::Slot ||
           m_eScheme == INetProtocol::Macro ||
           m_eScheme == INetProtocol::Uno ||
           m_eScheme == INetProtocol::VndSunStarExpand ||
           isSchemeEqualTo(u"vnd.sun.star.script") ||
           isSchemeEqualTo(u"service");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
