/*************************************************************************
 *
 *  $RCSfile: urlobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sb $ $Date: 2000-10-16 06:56:05 $
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

#define min min // fool <tools/solar.h>...
#define max max // fool <tools/solar.h>...

#ifndef _URLOBJ_HXX
#include <urlobj.hxx>
#endif

#include <limits>

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef TOOLS_INETMIME_HXX
#include <inetmime.hxx>
#endif

namespace unnamed_tools_urlobj {} using namespace unnamed_tools_urlobj;
    // unnamed namespaces don't work well yet...

//============================================================================
//
//  INetURLObject
//
//============================================================================

/* The URI grammar (using RFC 2234 conventions).

   Constructs of the form
       {reference <rule1> using rule2}
   stand for a rule matching the given rule1 specified in the given reference,
   encoded to URI syntax using rule2 (as specified in this URI grammar).


   ; RFC 1738, RFC 2396, RFC 2732, private
   login = [user [":" password] "@"] hostport
   user = *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ";" / "=" / "_" / "~")
   password = *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ";" / "=" / "_" / "~")
   hostport = host [":" port]
   host = incomplete-hostname / hostname / IPv4address / IPv6reference
   incomplete-hostname = *(domainlabel ".") domainlabel
   hostname = *(domainlabel ".") toplabel ["."]
   domainlabel = alphadigit [*(alphadigit / "-") alphadigit]
   toplabel = ALPHA [*(alphadigit / "-") alphadigit]
   alphadigit = ALPHA / DIGIT
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


   ; RFC 1738, RFC 2396
   file-url = "FILE://" [host / "LOCALHOST"] ["/" segment *("/" segment)]
   segment = *pchar


   ; RFC 2368, RFC 2396
   mailto-url = "MAILTO:" [to] [headers]
   to = {RFC 822 <#mailbox> using *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")}
   headers = "?" header *("&" header)
   header = hname "=" hvalue
   hname = {RFC 822 <field-name> using *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")} / "BODY"
   hvalue = {RFC 822 <field-body> using *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")}


   ; private (see RFC 1738, RFC 2396)
   vnd-sun-star-webdav-url = "VND.SUN.STAR.WEBDAV://" hostport ["/" segment *("/" segment) ["?" *uric]]
   segment = *(pchar / ";")


   ; private (see RFC 1738)
   news-url = "NEWS://" login ["/" (group / message)]
   group = *uric
   message = "<" *uric ">"


   ; private
   private-url = "PRIVATE:" path ["?" *uric]
   path = *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~"


   ; private
   https-url = "HTTPS://" hostport ["/" segment *("/" segment) ["?" *uric]]
   segment = *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "@" / "_" / "~")


   ; private
   slot-url = "SLOT:" path ["?" *uric]
   path = *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~"


   ; private
   macro-url = "MACRO:" path ["?" *uric]
   path = *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~"


   ; private
   javascript-url = "JAVASCRIPT:" *uric


   ; private (see RFC 2192)
   imap-url = "IMAP://" user [";AUTH=" auth] "@" hostport "/" segment *("/" segment) ["/;UID=" nz_number]
   user = 1*{RFC 2060 <CHAR8> using (escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "=" / "_" / "~")}
   auth = {RFC 2060 <atom> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "+" / "," / "-" / "." / "=" / "_" / "~")}
   segment = *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / "=" / "@" / "_" / "~")
   nz_number = {RFC 2060 <nz_number> using *DIGIT}


   ; private
   pop3-url = "POP3://" login ["/" ["<" *uric ">"]]


   ; RFC 2397
   data-url = "DATA:" [mediatype] [";BASE64"] "," *uric
   mediatype = [type "/" subtype] *(";" attribute "=" value)
   type = {RFC 2045 <type> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}
   subtype = {RFC 2045 <subtype> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}
   attribute = {RFC 2045 <subtype> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}
   value = {RFC 2045 <subtype> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / ":" / "?" / "@" / "_" / "~")}


   ; RFC 2392, RFC 2396
   cid-url = "CID:" {RFC 822 <addr-spec> using *uric}


   ; private
   out-url = "OUT:///~" name ["/" *uric]
   name = *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / ":" / ";" / "=" / "?" / "@" / "_" / "~"


   ; private
   vim-url = "VIM://" +vimc [":" *vimc] ["/" [("INBOX" message) / ("NEWSGROUPS" ["/" [+vimc message]])]]
   message = ["/" [+vimc [":" +DIGIT "." +DIGIT "." +DIGIT]]]
   vimc = ("=" HEXDIG HEXDIG) / ALPHA / DIGIT


   ; private
   uno-url = ".UNO:" *uric


   ; private
   component-url = ".COMPONENT:" path ["?" *uric]
   path = *(escaped / ALPHA / DIGIT / "!" / "$" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~"


   ; RFC 2255
   ldap-url = "LDAP://" [hostport] ["/" [dn ["?" [attrdesct *("," attrdesc)] ["?" ["base" / "one" / "sub"] ["?" [filter] ["?" extension *("," extension)]]]]]]
   dn = {RFC 2253 <distinguishedName> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}
   attrdesc = {RFC 2251 <AttributeDescription> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}
   filter = {RFC 2254 <filter> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}
   extension = ["!"] ["X-"] extoken ["=" exvalue]
   extoken = {RFC 2252 <oid> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / "/" / ":" / ";" / "@" / "_" / "~")}
   exvalue = {RFC 2251 <LDAPString> using *(escaped / ALPHA / DIGIT / "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "-" / "." / "/" / ":" / ";" / "=" / "@" / "_" / "~")}


   ; private
   db-url = "DB:" *uric
 */

//============================================================================
inline sal_Int32 INetURLObject::SubString::clear()
{
    sal_Int32 nDelta = -m_nLength;
    m_nBegin = STRING_NOTFOUND;
    m_nLength = 0;
    return nDelta;
}

//============================================================================
inline sal_Int32 INetURLObject::SubString::set(UniString & rString,
                                               UniString const & rSubString)
{
    DBG_ASSERT(isPresent(), "INetURLObject::SubString::set(): Not present");
    sal_Int32 nDelta = rSubString.Len() - m_nLength;
    rString.Replace(m_nBegin, m_nLength, rSubString);
    m_nLength = rSubString.Len();
    return nDelta;
}

//============================================================================
inline sal_Int32 INetURLObject::SubString::set(UniString & rString,
                                               UniString const & rSubString,
                                               xub_StrLen nTheBegin)
{
    m_nBegin = nTheBegin;
    return set(rString, rSubString);
}

//============================================================================
inline void INetURLObject::SubString::operator +=(sal_Int32 nDelta)
{
    if (isPresent())
        m_nBegin += nDelta;
}

//============================================================================
bool INetURLObject::SubString::equals(SubString const & rOther,
                                      UniString const & rThisString,
                                      UniString const & rOtherString) const
{
    if (m_nLength != rOther.m_nLength)
        return true;
    sal_Unicode const * p1 = rThisString.GetBuffer() + m_nBegin;
    sal_Unicode const * p1End = p1 + m_nLength;
    sal_Unicode const * p2 = rOtherString.GetBuffer() + rOther.m_nBegin;
    while (p1 != p1End)
        if (*p1++ != *p2++)
            return false;
    return true;
}

//============================================================================
struct INetURLObject::SchemeInfo
{
    sal_Char const * m_pScheme;
    sal_Char const * m_pPrefix;
    sal_uInt16 m_nDefaultPort;
    bool m_bAuthority;
    bool m_bUser;
    bool m_bAuth;
    bool m_bPassword;
    bool m_bHost;
    bool m_bPort;
    bool m_bHierarchical;
    bool m_bQuery;
};

//============================================================================
struct INetURLObject::PrefixInfo
{
    enum Kind { OFFICIAL, EXTERNAL, INTERNAL };

    sal_Char const * m_pPrefix;
    sal_Char const * m_pTranslatedPrefix;
    INetProtocol m_eScheme;
    Kind m_eKind;
};

//============================================================================
// static

static INetURLObject::SchemeInfo const aSchemeInfoMap[INET_PROT_END]
    = { { "", "", 0, false, false, false, false, false, false, false,
          false },
        { "ftp", "ftp://", 21, true, true, false, true, true, true, true,
          false },
        { "http", "http://", 80, true, false, false, false, true, true,
          true, true },
        { "file", "file://", 0, true, false, false, false, true, false,
          true, false },
        { "mailto", "mailto:", 0, false, false, false, false, false,
          false, false, true },
        { "vnd.sun.star.webdav", "vnd.sun.star.webdav://", 80, true, false,
          false, false, true, true, true, true },
        { "news", "news:", 119, true, true, false, true, true, true,
          false, false },
        { "private", "private:", 0, false, false, false, false, false,
          false, false, true },
        { 0, 0, 0, false, false, false, false, false, false, false,
          false },
        { "https", "https://", 443, true, false, false, false, true, true,
          true, true },
        { "slot", "slot:", 0, false, false, false, false, false, false,
          false, true },
        { "macro", "macro:", 0, false, false, false, false, false, false,
          false, true },
        { "javascript", "javascript:", 0, false, false, false, false,
          false, false, false, false },
        { "imap", "imap://", 143, true, true, true, false, true, true,
          true, false },
        { "pop3", "pop3://", 110, true, true, false, true, true, true,
          false, false },
        { "data", "data:", 0, false, false, false, false, false, false,
          false, false },
        { "cid", "cid:", 0, false, false, false, false, false, false,
          false, false },
        { "out", "out://", 0, true, false, false, false, false, false,
          false, false },
        { 0, 0, 0, false, false, false, false, false, false, false,
          false },
        { 0, 0, 0, false, false, false, false, false, false, false,
          false },
        { "vim", "vim://", 0, true, true, false, true, false, false, true,
          false },
        { ".uno", ".uno:", 0, false, false, false, false, false, false,
          false, false },
        { ".component", ".component:", 0, false, false, false, false,
          false, false, false, true },
        { 0, 0, 0, false, false, false, false, false, false, false,
          false },
        { "ldap", "ldap://", 389, true, false, false, false, true, true,
          false, true },
        { "db", "db:", 0, false, false, false, false, false, false, false,
          false },
        { 0, 0, 0, false, false, false, false, false, false, false,
          false } };

inline INetURLObject::SchemeInfo const &
INetURLObject::getSchemeInfo(INetProtocol eTheScheme)
{
    return aSchemeInfoMap[eTheScheme];
};

//============================================================================
inline INetURLObject::SchemeInfo const & INetURLObject::getSchemeInfo() const
{
    return getSchemeInfo(m_eScheme);
}

//============================================================================
// static
inline void INetURLObject::appendEscape(UniString & rTheText,
                                        sal_Char cEscapePrefix,
                                        sal_uInt32 nOctet)
{
    rTheText += cEscapePrefix;
    rTheText += sal_Unicode(INetMIME::getHexDigit(int(nOctet >> 4)));
    rTheText += sal_Unicode(INetMIME::getHexDigit(int(nOctet & 15)));
}

//============================================================================
// static
inline void INetURLObject::appendUTF32(UniString & rTheText,
                                       sal_uInt32 nUTF32)
{
    DBG_ASSERT(nUTF32 <= 0x10FFFF, "INetURLObject::appendUTF32(): Bad char");
    if (nUTF32 < 0x10000)
        rTheText += sal_Unicode(nUTF32);
    else
    {
        nUTF32 -= 0x10000;
        rTheText += sal_Unicode(0xD800 | nUTF32 >> 10);
        rTheText += sal_Unicode(0xDC00 | nUTF32 & 0x3FF);
    }
}

//============================================================================
namespace unnamed_tools_urlobj {

enum
{
    pA = INetURLObject::PART_OBSOLETE_NORMAL,
    pB = INetURLObject::PART_OBSOLETE_FILE,
    pC = INetURLObject::PART_OBSOLETE_PARAM,
    pD = INetURLObject::PART_USER_PASSWORD,
    pE = INetURLObject::PART_IMAP_ACHAR,
    pF = INetURLObject::PART_VIM,
    pG = INetURLObject::PART_HOST_EXTRA,
    pH = INetURLObject::PART_FPATH,
    pI = INetURLObject::PART_AUTHORITY,
    pJ = INetURLObject::PART_PATH_SEGMENTS_EXTRA,
    pK = INetURLObject::PART_REL_SEGMENT_EXTRA,
    pL = INetURLObject::PART_URIC,
    pM = INetURLObject::PART_HTTP_PATH,
    pN = INetURLObject::PART_FILE_SEGMENT_EXTRA,
    pO = INetURLObject::PART_MESSAGE_ID,
    pP = INetURLObject::PART_MESSAGE_ID_PATH,
    pQ = INetURLObject::PART_MAILTO,
    pR = INetURLObject::PART_PATH_BEFORE_QUERY,
    pS = INetURLObject::PART_PCHAR,
    pT = INetURLObject::PART_FRAGMENT,
    pU = INetURLObject::PART_VISIBLE,
    pV = INetURLObject::PART_VISIBLE_NONSPECIAL,
    pW = INetURLObject::PART_CREATEFRAGMENT,
    pX = INetURLObject::PART_UNO_PARAM_VALUE,
    pY = INetURLObject::PART_UNAMBIGUOUS
};
static sal_uInt32 const aMustEncodeMap[128]
    = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* */   pY,
/* ! */ pC+pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* " */ pU+pV+pY,
/* # */ pU,
/* $ */ pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* % */ pU,
/* & */ pA+pB+pC+pD+pE+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pR+pS+pT+pU+pV+pW+pX,
/* ' */ pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* ( */ pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* ) */ pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* * */ pA+pB+pC+pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* + */ pA+pB+pC+pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX,
/* , */ pA+pB+pC+pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW,
/* - */ pA+pB+pC+pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* . */ pA+pB+pC+pD+pE+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* / */ pA+pB+pC+pH+pJ+pL+pM+pP+pQ+pR+pT+pU+pV+pX,
/* 0 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 1 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 2 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 3 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 4 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 5 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 6 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 7 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 8 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* 9 */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* : */ pB+pC+pH+pI+pJ+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX,
/* ; */ pC+pD+pI+pJ+pK+pL+pM+pO+pP+pQ+pR+pT+pU+pW,
/* < */ pC+pO+pP+pU+pV+pY,
/* = */ pA+pB+pC+pD+pE+pH+pI+pJ+pK+pL+pM+pN+pR+pS+pT+pU+pV+pW,
/* > */ pC+pO+pP+pU+pV+pY,
/* ? */ pC+pL+pT+pU+pW+pX,
/* @ */ pC+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* A */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* B */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* C */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* D */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* E */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* F */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* G */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* H */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* I */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* J */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* K */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* L */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* M */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* N */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* O */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* P */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* Q */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* R */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* S */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* T */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* U */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* V */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* W */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* X */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* Y */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* Z */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* [ */ pL+pU+pV+pX,
/* \ */ pB+pU+pV+pY,
/* ] */ pL+pU+pV+pX,
/* ^ */ pU+pV+pY,
/* _ */ pA+pB+pC+pD+pE +pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* ` */ +pU+pV+pY,
/* a */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* b */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* c */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* d */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* e */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* f */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* g */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* h */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* i */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* j */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* k */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* l */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* m */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* n */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* o */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* p */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* q */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* r */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* s */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* t */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* u */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* v */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* w */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* x */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* y */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* z */ pA+pB+pC+pD+pE+pF+pG+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* { */ +pU+pV+pY,
/* | */ pB+pC+pN+pT+pU+pV+pY,
/* } */ +pU+pV+pY,
/* ~ */ pA+pB+pC+pD+pE+pH+pI+pJ+pK+pL+pM+pN+pO+pP+pQ+pR+pS+pT+pU+pV+pW+pX+pY,
/* */   0 };

inline bool mustEncode(sal_uInt32 nUTF32, INetURLObject::Part ePart)
{
    return !INetMIME::isUSASCII(nUTF32) || !(aMustEncodeMap[nUTF32] & ePart);
}

}

//============================================================================
void INetURLObject::setInvalid()
{
    m_aAbsURIRef.Erase();
    m_eScheme = INET_PROT_NOT_VALID;
    m_aUser.clear();
    m_aAuth.clear();
    m_aHost.clear();
    m_aPort.clear();
    m_aPath.clear();
    m_aQuery.clear();
    m_aFragment.clear();
}

//============================================================================

namespace unnamed_tools_urlobj {

INetURLObject::FSysStyle
guessFSysStyleByCounting(sal_Unicode const * pBegin,
                         sal_Unicode const * pEnd,
                         INetURLObject::FSysStyle eStyle)
{
    DBG_ASSERT(eStyle
                   & (INetURLObject::FSYS_UNX
                          | INetURLObject::FSYS_DOS
                          | INetURLObject::FSYS_MAC),
               "guessFSysStyleByCounting(): Bad style");
    DBG_ASSERT(std::numeric_limits< sal_Int32 >::min() < pBegin - pEnd
               && pEnd - pBegin <= std::numeric_limits< sal_Int32 >::max(),
               "guessFSysStyleByCounting(): Too big");
    sal_Int32 nSlashCount
        = eStyle & INetURLObject::FSYS_UNX ?
              0 : std::numeric_limits< sal_Int32 >::min();
    sal_Int32 nBackslashCount
        = eStyle & INetURLObject::FSYS_DOS ?
              0 : std::numeric_limits< sal_Int32 >::min();
    sal_Int32 nColonCount
        = eStyle & INetURLObject::FSYS_MAC ?
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

            case ':':
                ++nColonCount;
                break;
        }
    return nSlashCount >= nBackslashCount ?
               nSlashCount >= nColonCount ?
                   INetURLObject::FSYS_UNX : INetURLObject::FSYS_MAC :
               nBackslashCount >= nColonCount ?
                   INetURLObject::FSYS_DOS : INetURLObject::FSYS_MAC;
}

}

bool INetURLObject::setAbsURIRef(UniString const & rTheAbsURIRef,
                                 bool bOctets,
                                 EncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset,
                                 bool bSmart,
                                 FSysStyle eStyle)
{
    sal_Unicode const * pPos = rTheAbsURIRef.GetBuffer();
    sal_Unicode const * pEnd = pPos + rTheAbsURIRef.Len();

    setInvalid();

    sal_uInt32 nFragmentDelimiter = '#';

    UniString aSynAbsURIRef;

    // Parse <scheme>:
    sal_Unicode const * p = pPos;
    PrefixInfo const * pPrefix = getPrefix(p, pEnd);
    if (pPrefix)
    {
        pPos = p;
        m_eScheme = pPrefix->m_eScheme;
        aSynAbsURIRef
            = UniString::CreateFromAscii(pPrefix->m_eKind
                                                 == PrefixInfo::EXTERNAL ?
                                             pPrefix->m_pTranslatedPrefix :
                                             pPrefix->m_pPrefix);
    }
    else
    {
        if (bSmart)
        {
            // For scheme detection, the first (if any) of the following
            // productions that matches the input string (and for which the
            // appropriate style bit is set in eStyle, if applicable)
            // determines the scheme. The productions use the auxiliary rules
            //
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
            //
            // 1st Production (known scheme):
            //    <one of the known schemes, ignoring case> ":" *UCS4
            //
            // 2nd Production (mailto):
            //    domain "@" domain
            //
            // 3rd Production (ftp):
            //    "ftp" 2*("." label) ["/" *UCS4]
            //
            // 4th Production (http):
            //    label 2*("." label) ["/" *UCS4]
            //
            // 5th Production (file):
            //    "//" (domain / IPv6reference) ["/" *UCS4]
            //
            // 6th Production (Unix file):
            //    "/" *UCS4
            //
            // 7th Production (UNC file; FSYS_DOS only):
            //    "\\" domain ["\" *UCS4]
            //
            // 8th Production (Unix-like DOS file; FSYS_DOS only):
            //    ALPHA ":/" *UCS4
            //
            // 9th Production (DOS file; FSYS_DOS only):
            //    ALPHA ":\" *UCS4
            //
            // For the 'non URL' file productions 6--9, the interpretation of
            // the input as a (degenerate) URI is turned off, i.e., escape
            // sequences and fragments are never detected as such, but are
            // taken as literal characters.

            sal_Unicode const * p = pPos;
            if (eStyle & FSYS_DOS
                && pEnd - p >= 3
                && INetMIME::isAlpha(p[0])
                && p[1] == ':'
                && (p[2] == '/' || p[2] == '\\'))
            {
                m_eScheme = INET_PROT_FILE; // 8th, 9th
                eMechanism = ENCODE_ALL;
                nFragmentDelimiter = 0x80000000;
            }
            else if (pEnd - p >= 2 && p[0] == '/' && p[1] == '/')
            {
                p += 2;
                if ((scanDomain(p, pEnd) > 0 || scanIPv6reference(p, pEnd))
                    && (p == pEnd || *p == '/'))
                    m_eScheme = INET_PROT_FILE; // 5th
            }
            else if (p != pEnd && *p == '/')
            {
                m_eScheme = INET_PROT_FILE; // 6th
                eMechanism = ENCODE_ALL;
                nFragmentDelimiter = 0x80000000;
            }
            else if (eStyle & FSYS_DOS
                     && pEnd - p >= 2
                     && p[0] == '\\'
                     && p[1] == '\\')
            {
                p += 2;
                if (scanDomain(p, pEnd) > 0 && (p == pEnd || *p == '\\'))
                {
                    m_eScheme = INET_PROT_FILE; // 7th
                    eMechanism = ENCODE_ALL;
                    nFragmentDelimiter = 0x80000000;
                }
            }
            else
            {
                sal_Unicode const * pDomainEnd = p;
                sal_uInt32 nLabels = scanDomain(pDomainEnd, pEnd);
                if (nLabels > 0 && pDomainEnd != pEnd && *pDomainEnd == '@')
                {
                    ++pDomainEnd;
                    if (scanDomain(pDomainEnd, pEnd) > 0
                        && pDomainEnd == pEnd)
                        m_eScheme = INET_PROT_MAILTO; // 2nd
                }
                else if (nLabels >= 3
                         && (pDomainEnd == pEnd || *pDomainEnd == '/'))
                    m_eScheme
                        = pDomainEnd - p >= 4
                          && p[0] == 'f'
                          && p[1] == 't'
                          && p[2] == 'p'
                          && p[3] == '.' ?
                              INET_PROT_FTP : INET_PROT_HTTP; // 3rd, 4th
            }

            if (m_eScheme == INET_PROT_NOT_VALID && pPos != pEnd
                && *pPos != nFragmentDelimiter)
                m_eScheme = m_eSmartScheme;
        }

        if (m_eScheme == INET_PROT_NOT_VALID)
        {
            setInvalid();
            return false;
        }

        aSynAbsURIRef = UniString::CreateFromAscii(getSchemeInfo().m_pScheme);
        aSynAbsURIRef += ':';
    }

    sal_Char cEscapePrefix = getEscapePrefix();
    sal_uInt32 nSegmentDelimiter = '/';
    sal_uInt32 nAltSegmentDelimiter = 0x80000000;
    bool bSkippedInitialSlash = false;

    // Parse //<user>;AUTH=<auth>@<host>:<port> or
    // //<user>:<password>@<host>:<port>
    if (getSchemeInfo().m_bAuthority)
    {
        aSynAbsURIRef.AppendAscii(RTL_CONSTASCII_STRINGPARAM("//"));

        sal_Unicode const * pUserInfoBegin = 0;
        sal_Unicode const * pUserInfoEnd = 0;
        sal_Unicode const * pHostPortBegin = 0;
        sal_Unicode const * pHostPortEnd = 0;

        switch (m_eScheme)
        {
            case INET_PROT_FILE:
                if (bSmart)
                {
                    // The first of the following seven productions that
                    // matches the rest of the input string (and for which the
                    // appropriate style bit is set in eStyle, if applicable)
                    // determines the used notation.  The productions use the
                    // auxiliary rules
                    //
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
                        sal_Unicode const * p = pPos + 2;
                        if (p == pEnd || *p == nFragmentDelimiter || *p == '/'
                            || (scanDomain(p, pEnd) > 0
                                    || scanIPv6reference(p, pEnd))
                               && (p == pEnd || *p == nFragmentDelimiter
                                   || *p == '/'))
                        {
                            pHostPortBegin = pPos + 2;
                            pHostPortEnd = p;
                            pPos = p;
                            break;
                        }
                    }

                    // 2nd Production (misscounted slashes):
                    //    "//" *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" *path ["#" *UCS4]
                    if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
                    {
                        pPos += 2;
                        bSkippedInitialSlash = true;
                        break;
                    }

                    // 3rd Production (Unix):
                    //    "/" *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" *path ["#" *UCS4]
                    if (pPos < pEnd && *pPos == '/')
                        break;

                    // 4th Production (UNC; FSYS_DOS only):
                    //    "\\" domain ["\" *path] ["#" *UCS4]
                    //  becomes
                    //    "file://" domain "/" *path ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    if (eStyle & FSYS_DOS
                        && pEnd - pPos >= 2
                        && pPos[0] == '\\'
                        && pPos[1] == '\\')
                    {
                        sal_Unicode const * p = pPos + 2;
                        if (scanDomain(p, pEnd) > 0
                            && (p == pEnd || *p == nFragmentDelimiter
                                || *p == '\\'))
                        {
                            pHostPortBegin = pPos + 2;
                            pHostPortEnd = p;
                            pPos = p;
                            nSegmentDelimiter = '\\';
                            break;
                        }
                    }

                    // 5th Production (Unix-like DOS; FSYS_DOS only):
                    //    ALPHA ":/" *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" ALPHA ":/" *path ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    //
                    // 6th Production (DOS; FSYS_DOS only):
                    //    ALPHA ":\" *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" ALPHA ":/" *path ["#" *UCS4]
                    //  replacing "\" by "/" within <*path>
                    if (eStyle & FSYS_DOS
                        && pEnd - pPos >= 3
                        && INetMIME::isAlpha(pPos[0])
                        && pPos[1] == ':'
                        && (pPos[2] == '/' || pPos[2] == '\\'))
                    {
                        nAltSegmentDelimiter = '\\';
                        bSkippedInitialSlash = true;
                        break;
                    }

                    // 7th Production (any):
                    //    *path ["#" *UCS4]
                    //  becomes
                    //    "file:///" *path ["#" *UCS4]
                    //  replacing the delimiter by "/" within <*path>.  The
                    //  delimiter is that character from the set { "/", "\",
                    //  ":" } which appears most often in <*path> (if FSYS_UNX
                    //  is not among the style bits, "/" is removed from the
                    //  set; if FSYS_DOS is not among the style bits, "\" is
                    //  removed from the set; if FSYS_MAC is not among the
                    //  style bits, ":" is removed from the set).  If two or
                    //  more characters appear the same number of times, the
                    //  character mentioned first in that set is chosen.  If
                    //  the first character of <*path> is the delimiter, that
                    //  character is not copied.
                    if (eStyle & (FSYS_UNX | FSYS_DOS | FSYS_MAC))
                    {
                        switch (guessFSysStyleByCounting(pPos, pEnd, eStyle))
                        {
                            case FSYS_UNX:
                                nSegmentDelimiter = '/';
                                break;

                            case FSYS_DOS:
                                nSegmentDelimiter = '\\';
                                break;

                            case FSYS_MAC:
                                nSegmentDelimiter = ':';
                                break;

                            default:
                                DBG_ERROR(
                                    "INetURLObject::setAbsURIRef():"
                                        " Bad guessFSysStyleByCounting");
                                break;
                        }
                        bSkippedInitialSlash
                            = pPos != pEnd && *pPos != nSegmentDelimiter;
                        break;
                    }
                }
            default:
            {
                if (pEnd - pPos >= 2 && pPos[0] == '/' && pPos[1] == '/')
                    pPos += 2;
                else if (!bSmart)
                {
                    setInvalid();
                    return false;
                }

                sal_Unicode const * pAuthority = pPos;
                sal_uInt32 c = getSchemeInfo().m_bQuery ? '?' : 0x80000000;
                while (pPos < pEnd && *pPos != '/' && *pPos != c
                       && *pPos != nFragmentDelimiter)
                    ++pPos;
                if (getSchemeInfo().m_bUser)
                    if (getSchemeInfo().m_bHost)
                    {
                        sal_Unicode const * p = pAuthority;
                        while (p < pPos && *p != '@')
                            ++p;
                        if (p == pPos)
                        {
                            pHostPortBegin = pAuthority;
                            pHostPortEnd = pPos;
                        }
                        else
                        {
                            pUserInfoBegin = pAuthority;
                            pUserInfoEnd = p;
                            pHostPortBegin = p + 1;
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
            Part ePart = m_eScheme == INET_PROT_IMAP ?
                             PART_IMAP_ACHAR :
                         m_eScheme == INET_PROT_VIM ?
                             PART_VIM :
                             PART_USER_PASSWORD;
            bool bSupportsPassword = getSchemeInfo().m_bPassword;
            bool bSupportsAuth
                = !bSupportsPassword && getSchemeInfo().m_bAuth;
            bool bHasAuth = false;
            UniString aSynUser;
            sal_Unicode const * p = pUserInfoBegin;
            while (p < pUserInfoEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(p, pUserInfoEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                if (eEscapeType == ESCAPE_NO)
                    if (nUTF32 == ':' && bSupportsPassword)
                    {
                        bHasAuth = true;
                        break;
                    }
                    else if (nUTF32 == ';' && bSupportsAuth
                             && pUserInfoEnd - p
                                    > RTL_CONSTASCII_LENGTH("auth=")
                             && INetMIME::equalIgnoreCase(
                                    p,
                                    p + RTL_CONSTASCII_LENGTH("auth="),
                                    "auth="))
                    {
                        p += RTL_CONSTASCII_LENGTH("auth=");
                        bHasAuth = true;
                        break;
                    }
                appendUCS4(aSynUser, nUTF32, eEscapeType, bOctets, ePart,
                           cEscapePrefix, eCharset, false);
            }
            m_aUser.set(aSynAbsURIRef, aSynUser, aSynAbsURIRef.Len());
            if (bHasAuth)
                if (bSupportsPassword)
                {
                    aSynAbsURIRef += ':';
                    UniString aSynAuth;
                    while (p < pUserInfoEnd)
                    {
                        EscapeType eEscapeType;
                        sal_uInt32 nUTF32 = getUTF32(p, pUserInfoEnd, bOctets,
                                                     cEscapePrefix,
                                                     eMechanism, eCharset,
                                                     eEscapeType);
                        appendUCS4(aSynAuth, nUTF32, eEscapeType, bOctets,
                                   ePart, cEscapePrefix, eCharset, false);
                    }
                    m_aAuth.set(aSynAbsURIRef, aSynAuth, aSynAbsURIRef.Len());
                }
                else
                {
                    aSynAbsURIRef.
                        AppendAscii(RTL_CONSTASCII_STRINGPARAM(";AUTH="));
                    UniString aSynAuth;
                    while (p < pUserInfoEnd)
                    {
                        EscapeType eEscapeType;
                        sal_uInt32 nUTF32 = getUTF32(p, pUserInfoEnd, bOctets,
                                                     cEscapePrefix,
                                                     eMechanism, eCharset,
                                                     eEscapeType);
                        if (!INetMIME::isIMAPAtomChar(nUTF32))
                        {
                            setInvalid();
                            return false;
                        }
                        appendUCS4(aSynAuth, nUTF32, eEscapeType, bOctets,
                                   ePart, cEscapePrefix, eCharset, false);
                    }
                    m_aAuth.set(aSynAbsURIRef, aSynAuth, aSynAbsURIRef.Len());
                }
            if (pHostPortBegin)
                aSynAbsURIRef += '@';
        }

        if (pHostPortBegin)
        {
            sal_Unicode const * pPort = pHostPortEnd;
            if (getSchemeInfo().m_bPort && pHostPortBegin < pHostPortEnd)
            {
                sal_Unicode const * p = pHostPortEnd - 1;
                while (p > pHostPortBegin && INetMIME::isDigit(*p))
                    --p;
                if (*p == ':')
                    pPort = p;
            }
            switch (m_eScheme)
            {
                case INET_PROT_FILE:
                    // If the host equals "LOCALHOST" (unencoded and ignoring
                    // case), turn it into an empty host:
                    if (INetMIME::equalIgnoreCase(pHostPortBegin, pPort,
                                                  "localhost"))
                        pHostPortBegin = pPort;
                    break;

                case INET_PROT_LDAP:
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
            UniString aSynHost;
            if (pHostPortBegin != pPort
                && (!parseHost(pHostPortBegin, pPort, bOctets, eMechanism,
                               eCharset, aSynHost)
                    || pHostPortBegin != pPort))
            {
                setInvalid();
                return false;
            }
            m_aHost.set(aSynAbsURIRef, aSynHost, aSynAbsURIRef.Len());
            if (pPort != pHostPortEnd)
            {
                aSynAbsURIRef += ':';
                m_aPort.set(aSynAbsURIRef,
                            UniString(pPort + 1, pHostPortEnd - (pPort + 1)),
                            aSynAbsURIRef.Len());
            }
        }
    }

    // Parse <path>
    UniString aSynPath;
    if (!parsePath(&pPos, pEnd, bOctets, eMechanism, eCharset,
                   bSkippedInitialSlash, nSegmentDelimiter,
                   nAltSegmentDelimiter,
                   getSchemeInfo().m_bQuery ? '?' : 0x80000000,
                   nFragmentDelimiter, &aSynPath))
    {
        setInvalid();
        return false;
    }
    m_aPath.set(aSynAbsURIRef, aSynPath, aSynAbsURIRef.Len());

    // Parse ?<query>
    if (getSchemeInfo().m_bQuery && pPos < pEnd && *pPos == '?')
    {
        aSynAbsURIRef += '?';
        UniString aSynQuery;
        for (++pPos; pPos < pEnd && *pPos != nFragmentDelimiter;)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, cEscapePrefix,
                                         eMechanism, eCharset, eEscapeType);
            appendUCS4(aSynQuery, nUTF32, eEscapeType, bOctets,
                       PART_URIC, cEscapePrefix, eCharset, true);
        }
        m_aQuery.set(aSynAbsURIRef, aSynQuery, aSynAbsURIRef.Len());
    }

    // Parse #<fragment>
    if (pPos < pEnd && *pPos == nFragmentDelimiter)
    {
        aSynAbsURIRef += nFragmentDelimiter;
        UniString aSynFragment;
        for (++pPos; pPos < pEnd;)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets, cEscapePrefix,
                                         eMechanism, eCharset, eEscapeType);
            appendUCS4(aSynFragment, nUTF32, eEscapeType, bOctets, PART_URIC,
                       cEscapePrefix, eCharset, true);
        }
        m_aFragment.set(aSynAbsURIRef, aSynFragment, aSynAbsURIRef.Len());
    }

    if (pPos != pEnd)
    {
        setInvalid();
        return false;
    }

    m_aAbsURIRef = aSynAbsURIRef;

    return true;
}

//============================================================================
// static
INetURLObject INetURLObject::m_aBaseURIRef;

//============================================================================
bool INetURLObject::convertRelToAbs(UniString const & rTheRelURIRef,
                                    bool bOctets,
                                    INetURLObject & rTheAbsURIRef,
                                    bool & rWasAbsolute,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset,
                                    bool bIgnoreFragment, bool bSmart,
                                    bool bRelativeNonURIs, FSysStyle eStyle)
    const
{
    sal_Unicode const * p = rTheRelURIRef.GetBuffer();
    sal_Unicode const * pEnd = p + rTheRelURIRef.Len();

    sal_Unicode const * pPrefixBegin = p;
    PrefixInfo const * pPrefix = getPrefix(pPrefixBegin, pEnd);

    sal_uInt32 nSegmentDelimiter = '/';
    sal_uInt32 nQueryDelimiter
        = !bSmart || getSchemeInfo().m_bQuery ? '?' : 0x80000000;
    sal_uInt32 nFragmentDelimiter = '#';
    Part ePart = PART_VISIBLE;

    if (!pPrefix && bSmart)
    {
        // If the input matches any of the following productions (for which
        // the appropriate style bit is set in eStyle), it is assumed to be an
        // absolute file system path, rather than a relative URI reference.
        // (This is only a subset of the productions used for scheme detection
        // in INetURLObject::setAbsURIRef(), because most of those productions
        // interfere with the syntax of relative URI references.)  The
        // productions use the auxiliary rules
        //
        //    domain = label *("." label)
        //    label = alphanum [*(alphanum / "-") alphanum]
        //    alphanum = ALPHA / DIGIT
        //    UCS4 = <any UCS4 character>
        //
        // 1st Production (UNC file; FSYS_DOS only):
        //    "\\" domain ["\" *UCS4]
        //
        // 2nd Production (Unix-like DOS file; FSYS_DOS only):
        //    ALPHA ":/" *UCS4
        //
        // 3rd Production (DOS file; FSYS_DOS only):
        //    ALPHA ":\" *UCS4
        if (eStyle & FSYS_DOS)
        {
            bool bFSys = false;
            sal_Unicode const * q = p;
            if (pEnd - q >= 3
                && INetMIME::isAlpha(q[0])
                && q[1] == ':'
                && (q[2] == '/' || q[2] == '\\'))
                bFSys = true; // 2nd, 3rd
            else if (pEnd - q >= 2 && q[0] == '\\' && q[1] == '\\')
            {
                q += 2;
                if (scanDomain(q, pEnd) > 0 && (q == pEnd || *q == '\\'))
                    bFSys = true; // 1st
            }
            if (bFSys)
            {
                INetURLObject aNewURI;
                aNewURI.setAbsURIRef(rTheRelURIRef, bOctets, eMechanism,
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
        if (m_eScheme == INET_PROT_FILE)
            switch (guessFSysStyleByCounting(p, pEnd, eStyle))
            {
                case FSYS_UNX:
                    nSegmentDelimiter = '/';
                    break;

                case FSYS_DOS:
                    nSegmentDelimiter = '\\';
                    bRelativeNonURIs = true;
                    break;

                case FSYS_MAC:
                    nSegmentDelimiter = ':';
                    bRelativeNonURIs = true;
                    break;

                default:
                    DBG_ERROR("INetURLObject::convertRelToAbs():"
                                  " Bad guessFSysStyleByCounting");
                    break;
            }

        if (bRelativeNonURIs)
        {
            eMechanism = ENCODE_ALL;
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
        pPrefix = 0;
        while (p != pEnd && *p++ != ':');
    }
    rWasAbsolute = pPrefix != 0;

    // Fast solution for non-relative URIs:
    if (pPrefix)
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

    UniString aSynAbsURIRef(UniString::CreateFromAscii(getSchemeInfo().
                                                           m_pScheme));
    aSynAbsURIRef += ':';

    sal_Char cEscapePrefix = getEscapePrefix();

    enum State { STATE_AUTH, STATE_ABS_PATH, STATE_REL_PATH, STATE_FRAGMENT,
                 STATE_DONE };
    State eState = STATE_AUTH;
    bool bSameDoc = true;

    if (getSchemeInfo().m_bAuthority)
        if (pEnd - p >= 2 && p[0] == '/' && p[1] == '/')
        {
            aSynAbsURIRef.AppendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
            p += 2;
            eState = STATE_ABS_PATH;
            bSameDoc = false;
            while (p != pEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32
                    = getUTF32(p, pEnd, bOctets, cEscapePrefix, eMechanism,
                               eCharset, eEscapeType);
                if (eEscapeType == ESCAPE_NO)
                    if (nUTF32 == nSegmentDelimiter)
                        break;
                    else if (nUTF32 == nFragmentDelimiter)
                    {
                        eState = STATE_FRAGMENT;
                        break;
                    }
                appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, bOctets,
                           PART_VISIBLE, cEscapePrefix, eCharset, true);
            }
        }
        else
        {
            SubString aAuthority(getAuthority());
            aSynAbsURIRef.Append(m_aAbsURIRef.GetBuffer()
                                     + aAuthority.getBegin(),
                                 aAuthority.getLength());
        }

    if (eState == STATE_AUTH)
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

    if (eState == STATE_ABS_PATH)
    {
        aSynAbsURIRef += '/';
        eState = STATE_DONE;
        while (p != pEnd)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32
                = getUTF32(p, pEnd, bOctets, cEscapePrefix, eMechanism,
                           eCharset, eEscapeType);
            if (eEscapeType == ESCAPE_NO)
                if (nUTF32 == nFragmentDelimiter)
                {
                    eState = STATE_FRAGMENT;
                    break;
                }
                else if (nUTF32 == nSegmentDelimiter)
                    nUTF32 = '/';
            appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, bOctets, ePart,
                       cEscapePrefix, eCharset, true);
        }
    }
    else if (eState == STATE_REL_PATH)
    {
        if (!getSchemeInfo().m_bHierarchical)
        {
            rWasAbsolute = false;
            return false;
        }

        sal_Unicode const * pBasePathBegin
            = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
        sal_Unicode const * pBasePathEnd
            = pBasePathBegin + m_aPath.getLength();
        while (pBasePathEnd != pBasePathBegin)
            if (*(--pBasePathEnd) == '/')
            {
                ++pBasePathEnd;
                break;
            }

        xub_StrLen nPathBegin = aSynAbsURIRef.Len();
        aSynAbsURIRef.Append(pBasePathBegin, pBasePathEnd - pBasePathBegin);
        DBG_ASSERT(aSynAbsURIRef.Len() > nPathBegin
                   && aSynAbsURIRef.GetChar(aSynAbsURIRef.Len() - 1) == '/',
                   "INetURLObject::convertRelToAbs(): Bad base path");

        while (p != pEnd && *p != nQueryDelimiter && *p != nFragmentDelimiter)
        {
            if (*p == '.')
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
                         && aSynAbsURIRef.Len() - nPathBegin > 1)
                {
                    p += 2;
                    if (p != pEnd && *p == nSegmentDelimiter)
                        ++p;

                    xub_StrLen i = aSynAbsURIRef.Len() - 2;
                    while (i > nPathBegin && aSynAbsURIRef.GetChar(i) != '/')
                        --i;
                    aSynAbsURIRef.Erase(i + 1);
                    DBG_ASSERT(
                        aSynAbsURIRef.Len() > nPathBegin
                        && aSynAbsURIRef.GetChar(aSynAbsURIRef.Len() - 1)
                               == '/',
                        "INetURLObject::convertRelToAbs(): Bad base path");
                    continue;
                }

            while (p != pEnd
                   && *p != nSegmentDelimiter
                   && *p != nQueryDelimiter
                   && *p != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32
                    = getUTF32(p, pEnd, bOctets, cEscapePrefix, eMechanism,
                               eCharset, eEscapeType);
                appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, bOctets, ePart,
                           cEscapePrefix, eCharset, true);
            }
            if (p != pEnd && *p == nSegmentDelimiter)
            {
                aSynAbsURIRef += '/';
                ++p;
            }
        }

        while (p != pEnd && *p != nFragmentDelimiter)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32
                = getUTF32(p, pEnd, bOctets, cEscapePrefix, eMechanism,
                           eCharset, eEscapeType);
            appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, bOctets, ePart,
                       cEscapePrefix, eCharset, true);
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
        aSynAbsURIRef.Append(m_aAbsURIRef.GetBuffer() + m_aPath.getBegin(),
                             m_aPath.getLength());
        if (m_aQuery.isPresent())
            aSynAbsURIRef.Append(m_aAbsURIRef.GetBuffer()
                                     + m_aQuery.getBegin() - 1,
                                 m_aQuery.getLength() + 1);
    }

    if (eState == STATE_FRAGMENT && !bIgnoreFragment)
    {
        aSynAbsURIRef += '#';
        while (p != pEnd)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32
                = getUTF32(p, pEnd, bOctets, cEscapePrefix, eMechanism,
                           eCharset, eEscapeType);
            appendUCS4(aSynAbsURIRef, nUTF32, eEscapeType, bOctets,
                       PART_VISIBLE, cEscapePrefix, eCharset, true);
        }
    }

    INetURLObject aNewURI(aSynAbsURIRef);
    if (aNewURI.HasError())
    {
        rWasAbsolute = false;
        return false;
    }

    rTheAbsURIRef = aNewURI;
    return true;
}

//============================================================================
bool INetURLObject::convertAbsToRel(UniString const & rTheAbsURIRef,
                                    bool bOctets, UniString & rTheRelURIRef,
                                    EncodeMechanism eEncodeMechanism,
                                    DecodeMechanism eDecodeMechanism,
                                    rtl_TextEncoding eCharset,
                                    FSysStyle eStyle) const
{
    // Check for hierarchical base URL:
    if (!getSchemeInfo().m_bHierarchical)
    {
        rTheRelURIRef
            = decode(rTheAbsURIRef,
                     getEscapePrefix(CompareProtocolScheme(rTheAbsURIRef)),
                     eDecodeMechanism, eCharset);
        return false;
    }

    // Convert the input (absolute URI ref, relative URI ref, file path) to an
    // absolute URI ref:
    INetURLObject aSubject;
    bool bWasAbsolute;
    if (!convertRelToAbs(rTheAbsURIRef, bOctets, aSubject, bWasAbsolute,
                         eEncodeMechanism, eCharset, false, true, false,
                         eStyle))
    {
        rTheRelURIRef
            = decode(rTheAbsURIRef,
                     getEscapePrefix(CompareProtocolScheme(rTheAbsURIRef)),
                     eDecodeMechanism, eCharset);
        return false;
    }

    // Check for differing scheme or authority parts:
    if (m_eScheme != aSubject.m_eScheme
        || !m_aUser.equals(aSubject.m_aUser, m_aAbsURIRef,
                           aSubject.m_aAbsURIRef)
        || !m_aAuth.equals(aSubject.m_aAuth, m_aAbsURIRef,
                           aSubject.m_aAbsURIRef)
        || !m_aHost.equals(aSubject.m_aHost, m_aAbsURIRef,
                           aSubject.m_aAbsURIRef)
        || !m_aPort.equals(aSubject.m_aPort, m_aAbsURIRef,
                           aSubject.m_aAbsURIRef))
    {
        rTheRelURIRef = aSubject.GetMainURL(eDecodeMechanism, eCharset);
        return false;
    }

    sal_Unicode const * pBasePathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pBasePathEnd = pBasePathBegin + m_aPath.getLength();
    sal_Unicode const * pSubjectPathBegin
        = aSubject.m_aAbsURIRef.GetBuffer() + aSubject.m_aPath.getBegin();
    sal_Unicode const * pSubjectPathEnd
        = pSubjectPathBegin + aSubject.m_aPath.getLength();

    // Make nMatch point past the last matching slash, or past the end of the
    // paths, in case they are equal:
    sal_Unicode const * pSlash = 0;
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
    sal_uInt32 nMatch = pSlash - pBasePathBegin;

    // For file URLs, if the common prefix of the two paths is only "/" (which
    // covers different DOS volumes like "/a:" and "/b:"), the subject is not
    // made relative (it could be, but some people don't like that):
    if (m_eScheme == INET_PROT_FILE && nMatch <= 1)
    {
        rTheRelURIRef = aSubject.GetMainURL(eDecodeMechanism, eCharset);
        return false;
    }

    // For every slash in the base path after nMatch, a prefix of "../" is
    // added to the new relative URL (if the common prefix of the two paths is
    // only "/"---but see handling of file URLs above---, the complete subject
    // path could go into the new relative URL instead, but some people don't
    // like that):
    UniString aSynRelURIRef;
//  if (nMatch <= 1) nMatch = 0; else // see comment above
    for (sal_Unicode const * p = pBasePathBegin + nMatch; p != pBasePathEnd;
         ++p)
    {
        if (*p == '/')
            aSynRelURIRef.AppendAscii(RTL_CONSTASCII_STRINGPARAM("../"));
    }

    // If the new relative URL would start with "//" (i.e., it would be
    // mistaken for a relative URL starting with an authority part), or if the
    // new relative URL would neither be empty nor start with <"/"> nor start
    // with <1*rseg> (i.e., it could be mistaken for an absolute URL starting
    // with a scheme part), then the new relative URL is prefixed with "./":
    if (aSynRelURIRef.Len() == 0)
        if (pSubjectPathEnd - pSubjectPathBegin >= nMatch + 2
            && pSubjectPathBegin[nMatch] == '/'
            && pSubjectPathBegin[nMatch + 1] == '/')
            aSynRelURIRef.AppendAscii(RTL_CONSTASCII_STRINGPARAM("./"));
        else
            for (sal_Unicode const * p = pSubjectPathBegin + nMatch;
                 p != pSubjectPathEnd && *p != '/'; ++p)
                if (mustEncode(*p, PART_REL_SEGMENT_EXTRA))
                {
                    aSynRelURIRef.
                        AppendAscii(RTL_CONSTASCII_STRINGPARAM("./"));
                    break;
                }

    // The remainder of the subject path, starting at nMatch, is appended to
    // the new relative URL:
    sal_Char cEscapePrefix = getEscapePrefix();
    aSynRelURIRef += decode(pSubjectPathBegin + nMatch, pSubjectPathEnd,
                            cEscapePrefix, eDecodeMechanism, eCharset);

    // If the subject has defined query or fragment parts, they are appended
    // to the new relative URL:
    if (aSubject.m_aQuery.isPresent())
    {
        aSynRelURIRef += '?';
        aSynRelURIRef += aSubject.decode(aSubject.m_aQuery, cEscapePrefix,
                                         eDecodeMechanism, eCharset);
    }
    if (aSubject.m_aFragment.isPresent())
    {
        aSynRelURIRef += '#';
        aSynRelURIRef += aSubject.decode(aSubject.m_aFragment, cEscapePrefix,
                                         eDecodeMechanism, eCharset);
    }

    rTheRelURIRef = aSynRelURIRef;
    return true;
}

//============================================================================
// static
bool INetURLObject::convertIntToExt(UniString const & rTheIntURIRef,
                                    bool bOctets, UniString & rTheExtURIRef,
                                    EncodeMechanism eEncodeMechanism,
                                    DecodeMechanism eDecodeMechanism,
                                    rtl_TextEncoding eCharset)
{
    sal_Char cEscapePrefix
        = getEscapePrefix(CompareProtocolScheme(rTheIntURIRef));
    UniString aSynExtURIRef(encodeText(rTheIntURIRef, bOctets, PART_VISIBLE,
                                       cEscapePrefix, NOT_CANONIC, eCharset,
                                       true));
    sal_Unicode const * pBegin = aSynExtURIRef.GetBuffer();
    sal_Unicode const * pEnd = pBegin + aSynExtURIRef.Len();
    sal_Unicode const * p = pBegin;
    PrefixInfo const * pPrefix = getPrefix(p, pEnd);
    bool bConvert = pPrefix && pPrefix->m_eKind == PrefixInfo::INTERNAL;
    if (bConvert)
        aSynExtURIRef.
            Replace(0, p - pBegin,
                    UniString::CreateFromAscii(pPrefix->m_pTranslatedPrefix));
    rTheExtURIRef = decode(aSynExtURIRef, cEscapePrefix, eDecodeMechanism,
                           eCharset);
    return bConvert;
}

//============================================================================
// static
bool INetURLObject::convertExtToInt(UniString const & rTheExtURIRef,
                                    bool bOctets, UniString & rTheIntURIRef,
                                    EncodeMechanism eEncodeMechanism,
                                    DecodeMechanism eDecodeMechanism,
                                    rtl_TextEncoding eCharset)
{
    sal_Char cEscapePrefix
        = getEscapePrefix(CompareProtocolScheme(rTheExtURIRef));
    UniString aSynIntURIRef(encodeText(rTheExtURIRef, bOctets, PART_VISIBLE,
                                       cEscapePrefix, NOT_CANONIC, eCharset,
                                       true));
    sal_Unicode const * pBegin = aSynIntURIRef.GetBuffer();
    sal_Unicode const * pEnd = pBegin + aSynIntURIRef.Len();
    sal_Unicode const * p = pBegin;
    PrefixInfo const * pPrefix = getPrefix(p, pEnd);
    bool bConvert = pPrefix && pPrefix->m_eKind == PrefixInfo::EXTERNAL;
    if (bConvert)
        aSynIntURIRef.
            Replace(0, p - pBegin,
                    UniString::CreateFromAscii(pPrefix->m_pTranslatedPrefix));
    rTheIntURIRef = decode(aSynIntURIRef, cEscapePrefix, eDecodeMechanism,
                           eCharset);
    return bConvert;
}

//============================================================================
// static
INetURLObject::PrefixInfo const *
INetURLObject::getPrefix(sal_Unicode const *& rBegin,
                         sal_Unicode const * pEnd)
{
    static PrefixInfo const aMap[]
        = { // dummy entry at front needed, because pLast may point here:
            { 0, 0, INET_PROT_NOT_VALID, PrefixInfo::INTERNAL },
            { ".component:", "staroffice.component:", INET_PROT_COMPONENT,
              PrefixInfo::INTERNAL },
            { ".uno:", "staroffice.uno:", INET_PROT_UNO,
              PrefixInfo::INTERNAL },
            { "cid:", 0, INET_PROT_CID, PrefixInfo::OFFICIAL },
            { "data:", 0, INET_PROT_DATA, PrefixInfo::OFFICIAL },
            { "db:", "staroffice.db:", INET_PROT_DB, PrefixInfo::INTERNAL },
            { "file:", 0, INET_PROT_FILE, PrefixInfo::OFFICIAL },
            { "ftp:", 0, INET_PROT_FTP, PrefixInfo::OFFICIAL },
            { "http:", 0, INET_PROT_HTTP, PrefixInfo::OFFICIAL },
            { "https:", 0, INET_PROT_HTTPS, PrefixInfo::OFFICIAL },
            { "imap:", 0, INET_PROT_IMAP, PrefixInfo::OFFICIAL },
            { "javascript:", 0, INET_PROT_JAVASCRIPT, PrefixInfo::OFFICIAL },
            { "ldap:", 0, INET_PROT_LDAP, PrefixInfo::OFFICIAL },
            { "macro:", "staroffice.macro:", INET_PROT_MACRO,
              PrefixInfo::INTERNAL },
            { "mailto:", 0, INET_PROT_MAILTO, PrefixInfo::OFFICIAL },
            { "news:", 0, INET_PROT_NEWS, PrefixInfo::OFFICIAL },
            { "out:", "staroffice.out:", INET_PROT_OUT,
              PrefixInfo::INTERNAL },
            { "pop3:", "staroffice.pop3:", INET_PROT_POP3,
              PrefixInfo::INTERNAL },
            { "private:", "staroffice.private:", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::INTERNAL },
            { "private:factory/", "staroffice.factory:",
              INET_PROT_PRIV_SOFFICE, PrefixInfo::INTERNAL },
            { "private:helpid/", "staroffice.helpid:", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::INTERNAL },
            { "private:java/", "staroffice.java:", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::INTERNAL },
            { "private:searchfolder:", "staroffice.searchfolder:",
              INET_PROT_PRIV_SOFFICE, PrefixInfo::INTERNAL },
            { "private:trashcan:", "staroffice.trashcan:",
              INET_PROT_PRIV_SOFFICE, PrefixInfo::INTERNAL },
            { "slot:", "staroffice.slot:", INET_PROT_SLOT,
              PrefixInfo::INTERNAL },
            { "staroffice.component:", ".component:", INET_PROT_COMPONENT,
              PrefixInfo::EXTERNAL },
            { "staroffice.db:", "db:", INET_PROT_DB, PrefixInfo::EXTERNAL },
            { "staroffice.factory:", "private:factory/",
              INET_PROT_PRIV_SOFFICE, PrefixInfo::EXTERNAL },
            { "staroffice.helpid:", "private:helpid/", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::EXTERNAL },
            { "staroffice.java:", "private:java/", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::EXTERNAL },
            { "staroffice.macro:", "macro:", INET_PROT_MACRO,
              PrefixInfo::EXTERNAL },
            { "staroffice.out:", "out:", INET_PROT_OUT,
              PrefixInfo::EXTERNAL },
            { "staroffice.pop3:", "pop3:", INET_PROT_POP3,
              PrefixInfo::EXTERNAL },
            { "staroffice.private:", "private:", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::EXTERNAL },
            { "staroffice.searchfolder:", "private:searchfolder:",
              INET_PROT_PRIV_SOFFICE, PrefixInfo::EXTERNAL },
            { "staroffice.slot:", "slot:", INET_PROT_SLOT,
              PrefixInfo::EXTERNAL },
            { "staroffice.trashcan:", "private:trashcan:",
              INET_PROT_PRIV_SOFFICE, PrefixInfo::EXTERNAL },
            { "staroffice.uno:", ".uno:", INET_PROT_UNO,
              PrefixInfo::EXTERNAL },
            { "staroffice.vim:", "vim:", INET_PROT_VIM,
              PrefixInfo::EXTERNAL },
            { "staroffice:", "private:", INET_PROT_PRIV_SOFFICE,
              PrefixInfo::EXTERNAL },
            { "vim:", "staroffice.vim:", INET_PROT_VIM,
              PrefixInfo::INTERNAL },
            { "vnd.sun.star.webdav:", 0, INET_PROT_VND_SUN_STAR_WEBDAV,
              PrefixInfo::OFFICIAL } };
    PrefixInfo const * pFirst = aMap + 1;
    PrefixInfo const * pLast = aMap + sizeof aMap / sizeof (PrefixInfo) - 1;
    PrefixInfo const * pMatch = 0;
    sal_Unicode const * pMatched = rBegin;
    sal_Unicode const * p = rBegin;
    xub_StrLen i = 0;
    for (; pFirst < pLast; ++i)
    {
        if (pFirst->m_pPrefix[i] == '\0')
        {
            pMatch = pFirst++;
            pMatched = p;
        }
        if (p >= pEnd)
            break;
        sal_uInt32 nChar = INetMIME::toLowerCase(*p++);
        while (pFirst <= pLast && pFirst->m_pPrefix[i] < nChar)
            ++pFirst;
        while (pFirst <= pLast && pLast->m_pPrefix[i] > nChar)
            --pLast;
    }
    if (pFirst == pLast)
    {
        sal_Char const * q = pFirst->m_pPrefix + i;
        while (p < pEnd && *q != '\0' && INetMIME::toLowerCase(*p) == *q)
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

//============================================================================
INetURLObject::SubString INetURLObject::getAuthority() const
{
    DBG_ASSERT(getSchemeInfo().m_bAuthority,
               "INetURLObject::getAuthority(): Bad scheme");

    xub_StrLen nBegin;
    if (m_aUser.isPresent())
        nBegin = m_aUser.getBegin();
    else if (m_aHost.isPresent())
        nBegin = m_aHost.getBegin();
    else
        nBegin = m_aPath.getBegin();
    nBegin -= RTL_CONSTASCII_LENGTH("//");
    DBG_ASSERT(m_aAbsURIRef.GetChar(nBegin) == '/'
               && m_aAbsURIRef.GetChar(nBegin + 1) == '/',
               "INetURLObject::getAuthority(): Bad authority");

    xub_StrLen nEnd = m_aPort.isPresent() ? m_aPort.getEnd() :
                      m_aHost.isPresent() ? m_aHost.getEnd() :
                      m_aAuth.isPresent() ? m_aAuth.getEnd() :
                      m_aUser.isPresent() ? m_aUser.getEnd() :
                          nBegin + RTL_CONSTASCII_LENGTH("//");
    return SubString(nBegin, nEnd - nBegin);
}

//============================================================================
bool INetURLObject::setUser(UniString const & rTheUser,
                            bool bOctets, EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bUser
        || m_eScheme == INET_PROT_IMAP && rTheUser.Len() == 0)
        return false;
    UniString aNewUser(encodeText(rTheUser, bOctets,
                                  m_eScheme == INET_PROT_IMAP ?
                                      PART_IMAP_ACHAR :
                                  m_eScheme == INET_PROT_VIM ?
                                      PART_VIM :
                                      PART_USER_PASSWORD,
                                  getEscapePrefix(), eMechanism, eCharset,
                                  false));
    sal_Int32 nDelta;
    if (m_aUser.isPresent())
        nDelta = m_aUser.set(m_aAbsURIRef, aNewUser);
    else if (m_aHost.isPresent())
    {
        m_aAbsURIRef.Insert('@', m_aHost.getBegin());
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

//============================================================================
bool INetURLObject::clearPassword()
{
    if (!getSchemeInfo().m_bPassword)
        return false;
    if (m_aAuth.isPresent())
    {
        m_aAbsURIRef.Erase(m_aAuth.getBegin() - 1, m_aAuth.getLength() + 1);
        sal_Int32 nDelta = m_aAuth.clear() - 1;
        m_aHost += nDelta;
        m_aPort += nDelta;
        m_aPath += nDelta;
        m_aQuery += nDelta;
        m_aFragment += nDelta;
    }
    return true;
}

//============================================================================
bool INetURLObject::setPassword(UniString const & rThePassword, bool bOctets,
                                EncodeMechanism eMechanism,
                                rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bPassword)
        return false;
    UniString aNewAuth(encodeText(rThePassword, bOctets,
                                  m_eScheme == INET_PROT_VIM ?
                                      PART_VIM : PART_USER_PASSWORD,
                                  getEscapePrefix(), eMechanism, eCharset,
                                  false));
    sal_Int32 nDelta;
    if (m_aAuth.isPresent())
        nDelta = m_aAuth.set(m_aAbsURIRef, aNewAuth);
    else if (m_aUser.isPresent())
    {
        m_aAbsURIRef.Insert(':', m_aUser.getEnd());
        nDelta
            = m_aAuth.set(m_aAbsURIRef, aNewAuth, m_aUser.getEnd() + 1) + 1;
    }
    else if (m_aHost.isPresent())
    {
        m_aAbsURIRef.InsertAscii(":@", m_aHost.getBegin());
        m_aUser.set(m_aAbsURIRef, UniString(), m_aHost.getBegin());
        nDelta
            = m_aAuth.set(m_aAbsURIRef, aNewAuth, m_aHost.getBegin() + 1) + 2;
    }
    else if (getSchemeInfo().m_bHost)
        return false;
    else
    {
        m_aAbsURIRef.Insert(':', m_aPath.getBegin());
        m_aUser.set(m_aAbsURIRef, UniString(), m_aPath.getBegin());
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

//============================================================================
// static
UniString INetURLObject::encodeHostPort(UniString const & rTheHostPort,
                                        bool bOctets,
                                        EncodeMechanism eMechanism,
                                        rtl_TextEncoding eCharset)
{
    xub_StrLen nPort = rTheHostPort.Len();
    if (rTheHostPort.Len() != 0)
    {
        xub_StrLen i = rTheHostPort.Len() - 1;
        while (i != 0 && INetMIME::isDigit(rTheHostPort.GetChar(i)))
            --i;
        if (rTheHostPort.GetChar(i) == ':')
            nPort = i;
    }
    UniString aResult(encodeText(rTheHostPort.Copy(0, nPort), bOctets,
                                 PART_HOST_EXTRA, '%', eMechanism, eCharset,
                                 true));
    aResult += rTheHostPort.Copy(nPort);
    return aResult;
}

//============================================================================
bool INetURLObject::setHost(UniString const & rTheHost, bool bOctets,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bHost)
        return false;
    UniString aSynHost(rTheHost);
    switch (m_eScheme)
    {
        case INET_PROT_FILE:
            if (aSynHost.EqualsIgnoreCaseAscii("localhost"))
                aSynHost.Erase();
            break;

        case INET_PROT_LDAP:
            if (aSynHost.Len() == 0 && m_aPort.isPresent())
                return false;
            break;

        default:
            if (aSynHost.Len() == 0)
                return false;
            break;
    }
    if (aSynHost.Len() != 0)
    {
        sal_Unicode const * p = aSynHost.GetBuffer();
        sal_Unicode const * pEnd = p + aSynHost.Len();
        if (!parseHost(p, pEnd, bOctets, eMechanism, eCharset, aSynHost)
            || p != pEnd)
            return false;
    }
    sal_Int32 nDelta = m_aHost.set(m_aAbsURIRef, aSynHost);
    m_aPort += nDelta;
    m_aPath += nDelta;
    m_aQuery += nDelta;
    m_aFragment += nDelta;
    return true;
}

//============================================================================
bool INetURLObject::parsePath(sal_Unicode const ** pBegin,
                              sal_Unicode const * pEnd,
                              bool bOctets,
                              EncodeMechanism eMechanism,
                              rtl_TextEncoding eCharset,
                              bool bSkippedInitialSlash,
                              sal_uInt32 nSegmentDelimiter,
                              sal_uInt32 nAltSegmentDelimiter,
                              sal_uInt32 nQueryDelimiter,
                              sal_uInt32 nFragmentDelimiter,
                              UniString * pSynPath)
{
    DBG_ASSERT(pBegin && pSynPath,
               "INetURLObject::parsePath(): Null output param");

    sal_Char cEscapePrefix = getEscapePrefix();

    sal_Unicode const * pPos = *pBegin;
    UniString aTheSynPath;

    switch (m_eScheme)
    {
        case INET_PROT_NOT_VALID:
            return false;

        case INET_PROT_FTP:
        case INET_PROT_IMAP:
            if (pPos < pEnd && *pPos != '/')
            {
                setInvalid();
                return false;
            }
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_HTTP_PATH, cEscapePrefix, eCharset, true);
            }
            if (aTheSynPath.Len() == 0)
                aTheSynPath = '/';
            break;

        case INET_PROT_HTTP:
        case INET_PROT_VND_SUN_STAR_WEBDAV:
        case INET_PROT_HTTPS:
            if (pPos < pEnd && *pPos != '/')
            {
                setInvalid();
                return false;
            }
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_HTTP_PATH, cEscapePrefix, eCharset, true);
            }
            if (aTheSynPath.Len() == 0)
                aTheSynPath = '/';
            break;

        case INET_PROT_FILE:
        {
            if (bSkippedInitialSlash)
                aTheSynPath = '/';
            else if (pPos < pEnd
                     && *pPos != nSegmentDelimiter
                     && *pPos != nAltSegmentDelimiter)
            {
                setInvalid();
                return false;
            }
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                if (eEscapeType == ESCAPE_NO)
                    if (nUTF32 == nSegmentDelimiter
                        || nUTF32 == nAltSegmentDelimiter)
                    {
                        aTheSynPath += '/';
                        continue;
                    }
                    else if (nUTF32 == '|'
                             && (pPos == pEnd
                                 || *pPos == nFragmentDelimiter
                                 || *pPos == nSegmentDelimiter
                                 || *pPos == nAltSegmentDelimiter)
                             && aTheSynPath.Len() == 2
                             && INetMIME::isAlpha(aTheSynPath.GetChar(1)))
                    {
                        // A first segment of <ALPHA "|"> is translated to
                        // <ALPHA ":">:
                        aTheSynPath += ':';
                        continue;
                    }
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_PCHAR, cEscapePrefix, eCharset, true);
            }
            if (aTheSynPath.Len() == 0)
                aTheSynPath = '/';
            break;
        }

        case INET_PROT_MAILTO:
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_MAILTO, cEscapePrefix, eCharset, true);
            }
            break;

        case INET_PROT_NEWS:
        case INET_PROT_POP3:
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_MESSAGE_ID_PATH, cEscapePrefix, eCharset,
                           true);
            }
            break;

        case INET_PROT_PRIV_SOFFICE:
        case INET_PROT_SLOT:
        case INET_PROT_MACRO:
        case INET_PROT_COMPONENT:
        case INET_PROT_LDAP:
            while (pPos < pEnd && *pPos != nQueryDelimiter
                   && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_PATH_BEFORE_QUERY, cEscapePrefix, eCharset,
                           true);
            }
            break;

        case INET_PROT_JAVASCRIPT:
        case INET_PROT_DATA:
        case INET_PROT_CID:
        case INET_PROT_UNO:
        case INET_PROT_DB:
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_URIC, cEscapePrefix, eCharset, true);
            }
            break;

        case INET_PROT_OUT:
            if (pEnd - pPos < 2 || *pPos++ != '/' || *pPos++ != '~')
            {
                setInvalid();
                return false;
            }
            aTheSynPath.AssignAscii(RTL_CONSTASCII_STRINGPARAM("/~"));
            while (pPos < pEnd && *pPos != nFragmentDelimiter)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_URIC, cEscapePrefix, eCharset, true);
            }
            break;

        case INET_PROT_VIM:
        {
            if (m_aUser.isEmpty())
            {
                setInvalid();
                return false;
            }
            sal_Unicode const * pPathEnd = pPos;
            while (pPathEnd < pEnd && *pPathEnd != nFragmentDelimiter)
                ++pPathEnd;
            aTheSynPath = '/';
            if (pPos == pPathEnd)
                break;
            else if (*pPos++ != '/')
            {
                setInvalid();
                return false;
            }
            if (pPos == pPathEnd)
                break;
            while (pPos < pPathEnd && *pPos != '/')
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pPathEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath,
                           eEscapeType == ESCAPE_NO ?
                               INetMIME::toLowerCase(nUTF32) : nUTF32,
                           eEscapeType, bOctets, PART_VIM, cEscapePrefix,
                           eCharset, false);
            }
            bool bInbox;
            if (aTheSynPath.EqualsAscii("/inbox"))
                bInbox = true;
            else if (aTheSynPath.EqualsAscii("/newsgroups"))
                bInbox = false;
            else
            {
                setInvalid();
                return false;
            }
            aTheSynPath += '/';
            if (pPos == pPathEnd)
                break;
            else if (*pPos++ != '/')
            {
                setInvalid();
                return false;
            }
            if (!bInbox)
            {
                bool bEmpty = true;
                while (pPos < pPathEnd && *pPos != '/')
                {
                    EscapeType eEscapeType;
                    sal_uInt32 nUTF32 = getUTF32(pPos, pPathEnd, bOctets,
                                                 cEscapePrefix, eMechanism,
                                                 eCharset, eEscapeType);
                    appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                               PART_VIM, cEscapePrefix, eCharset, false);
                    bEmpty = false;
                }
                if (bEmpty)
                {
                    setInvalid();
                    return false;
                }
                aTheSynPath += '/';
                if (pPos == pPathEnd)
                    break;
                else if (*pPos++ != '/')
                {
                    setInvalid();
                    return false;
                }
            }
            bool bEmpty = true;
            while (pPos < pPathEnd && *pPos != ':')
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(pPos, pPathEnd, bOctets,
                                             cEscapePrefix, eMechanism,
                                             eCharset, eEscapeType);
                appendUCS4(aTheSynPath, nUTF32, eEscapeType, bOctets,
                           PART_VIM, cEscapePrefix, eCharset, false);
                bEmpty = false;
            }
            if (bEmpty)
            {
                setInvalid();
                return false;
            }
            if (pPos == pPathEnd)
                break;
            else if (*pPos++ != ':')
            {
                setInvalid();
                return false;
            }
            aTheSynPath += ':';
            for (int i = 0; i < 3; ++i)
            {
                if (i != 0)
                {
                    if (pPos == pPathEnd || *pPos++ != '.')
                    {
                        setInvalid();
                        return false;
                    }
                    aTheSynPath += '.';
                }
                bool bEmpty = true;
                while (pPos < pPathEnd && *pPos != '.')
                {
                    EscapeType eEscapeType;
                    sal_uInt32 nUTF32 = getUTF32(pPos, pPathEnd, bOctets,
                                                 cEscapePrefix, eMechanism,
                                                 eCharset, eEscapeType);
                    if (!INetMIME::isDigit(nUTF32))
                    {
                        setInvalid();
                        return false;
                    }
                    aTheSynPath += sal_Unicode(nUTF32);
                    bEmpty = false;
                }
                if (bEmpty)
                {
                    setInvalid();
                    return false;
                }
            }
            if (pPos != pPathEnd)
            {
                setInvalid();
                return false;
            }
            break;
        }
    }

    *pBegin = pPos;
    *pSynPath = aTheSynPath;
    return true;
}

//============================================================================
bool INetURLObject::setPath(UniString const & rThePath, bool bOctets,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    UniString aSynPath;
    sal_Unicode const * p = rThePath.GetBuffer();
    if (!parsePath(&p, p + rThePath.Len(), bOctets, eMechanism, eCharset,
                   false, '/', 0x80000000, 0x80000000, 0x80000000, &aSynPath))
        return false;
    sal_Int32 nDelta = m_aPath.set(m_aAbsURIRef, aSynPath);
    m_aQuery += nDelta;
    m_aFragment += nDelta;
    return true;
}

//============================================================================
bool INetURLObject::appendSegment(UniString const & rTheSegment, bool bOctets,
                                  EncodeMechanism eMechanism,
                                  rtl_TextEncoding eCharset)
{
    return insertName(rTheSegment, bOctets, false, LAST_SEGMENT, true,
                      eMechanism, eCharset);
}

//============================================================================
INetURLObject::SubString INetURLObject::getSegment(sal_Int32 nIndex,
                                                   bool bIgnoreFinalSlash)
    const
{
    DBG_ASSERT(nIndex >= 0 || nIndex == LAST_SEGMENT,
               "INetURLObject::getSegment(): Bad index");

    if (!getSchemeInfo().m_bHierarchical)
        return SubString();

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathBegin == pPathEnd || *pPathBegin != '/')
        return SubString();

    sal_Unicode const * pSegBegin;
    sal_Unicode const * pSegEnd;
    if (nIndex == LAST_SEGMENT)
    {
        pSegEnd = pPathEnd;
        if (bIgnoreFinalSlash && pSegEnd[-1] == '/'
            && pSegEnd != pPathBegin + 1)
            --pSegEnd;
        pSegBegin = pSegEnd - 1;
        while (*pSegBegin != '/')
            --pSegBegin;
    }
    else
    {
        pSegBegin = pPathBegin;
        pSegEnd = pPathEnd;
        if (bIgnoreFinalSlash && pSegEnd[-1] == '/')
            --pSegEnd;
        while (nIndex-- > 0)
            do
            {
                ++pSegBegin;
                if (pSegBegin >= pSegEnd)
                    return SubString();
            }
            while (*pSegBegin != '/');
        pSegEnd = pSegBegin + 1;
        while (pSegEnd != pPathEnd && *pSegEnd != '/')
            ++pSegEnd;
    }

    return SubString(pSegBegin - m_aAbsURIRef.GetBuffer(),
                     pSegEnd - pSegBegin);
}

//============================================================================
bool INetURLObject::insertName(UniString const & rTheName, bool bOctets,
                               bool bAppendFinalSlash, sal_Int32 nIndex,
                               bool bIgnoreFinalSlash,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset)
{
    DBG_ASSERT(nIndex >= 0 || nIndex == LAST_SEGMENT,
               "INetURLObject::insertName(): Bad index");

    if (!getSchemeInfo().m_bHierarchical)
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathBegin == pPathEnd || *pPathBegin != '/')
        return false;

    sal_Unicode const * pPrefixEnd;
    sal_Unicode const * pSuffixBegin;
    bool bPrefixSlash = true;
    if (nIndex == LAST_SEGMENT)
    {
        pPrefixEnd = pPathEnd;
        if (bIgnoreFinalSlash && pPrefixEnd[-1] == '/')
            bPrefixSlash = false;
        pSuffixBegin = bAppendFinalSlash ? 0 : pPathEnd;
    }
    else
    {
        pPrefixEnd = pPathBegin;
        sal_Unicode const * pEnd = pPathEnd;
        if (bIgnoreFinalSlash && pEnd[-1] == '/')
            --pEnd;
        while (nIndex-- > 0)
            for (;;)
            {
                ++pPrefixEnd;
                if (pPrefixEnd >= pEnd)
                    if (nIndex == 0)
                    {
                        pSuffixBegin = bAppendFinalSlash ? 0 : pPathEnd;
                        break;
                    }
                    else
                        return false;
                if (*pPrefixEnd == '/')
                {
                    pSuffixBegin = pPrefixEnd;
                    break;
                }
            }
    }

    UniString aNewPath(pPathBegin, pPrefixEnd - pPathBegin);
    if (bPrefixSlash)
        aNewPath += '/';
    aNewPath += encodeText(rTheName, bOctets, PART_PCHAR, getEscapePrefix(),
                           eMechanism, eCharset, true);
    if (pSuffixBegin)
        aNewPath.Append(pSuffixBegin, pPathEnd - pSuffixBegin);
    else
        aNewPath += '/';

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
bool INetURLObject::clearQuery()
{
    if (HasError())
        return false;
    if (m_aQuery.isPresent())
    {
        m_aAbsURIRef.Erase(m_aQuery.getBegin() - 1, m_aQuery.getLength() + 1);
        m_aFragment += m_aQuery.clear() - 1;
    }
    return false;
}

//============================================================================
bool INetURLObject::setQuery(UniString const & rTheQuery, bool bOctets,
                             EncodeMechanism eMechanism,
                             rtl_TextEncoding eCharset)
{
    if (!getSchemeInfo().m_bQuery)
        return false;
    UniString aNewQuery(encodeText(rTheQuery, bOctets, PART_URIC,
                                   getEscapePrefix(), eMechanism, eCharset,
                                   true));
    sal_Int32 nDelta;
    if (m_aQuery.isPresent())
        nDelta = m_aQuery.set(m_aAbsURIRef, aNewQuery);
    else
    {
        m_aAbsURIRef.Insert('?', m_aPath.getEnd());
        nDelta = m_aQuery.set(m_aAbsURIRef, aNewQuery, m_aPath.getEnd() + 1)
                     + 1;
    }
    m_aFragment += nDelta;
    return true;
}

//============================================================================
bool INetURLObject::clearFragment()
{
    if (HasError())
        return false;
    if (m_aFragment.isPresent())
    {
        m_aAbsURIRef.Erase(m_aFragment.getBegin() - 1);
        m_aFragment.clear();
    }
    return true;
}

//============================================================================
bool INetURLObject::setFragment(UniString const & rTheFragment, bool bOctets,
                                EncodeMechanism eMechanism,
                                rtl_TextEncoding eCharset)
{
    if (HasError())
        return false;
    UniString aNewFragment(encodeText(rTheFragment, bOctets, PART_URIC,
                                      getEscapePrefix(), eMechanism,
                                      eCharset, true));
    if (m_aFragment.isPresent())
        m_aFragment.set(m_aAbsURIRef, aNewFragment);
    else
    {
        m_aAbsURIRef.Append('#');
        m_aFragment.set(m_aAbsURIRef, aNewFragment, m_aAbsURIRef.Len());
    }
    return true;
}

//============================================================================
INetURLObject::FTPType INetURLObject::getFTPType() const
{
    if (m_eScheme == INET_PROT_FTP
        && m_aPath.getLength() >= RTL_CONSTASCII_LENGTH(";type=") + 1
        && m_aAbsURIRef.
               EqualsIgnoreCaseAscii(";type=",
                                     m_aPath.getEnd()
                                         - (RTL_CONSTASCII_LENGTH(";type=")
                                                + 1),
                                     RTL_CONSTASCII_LENGTH(";type=")))
        switch (m_aAbsURIRef.GetChar(m_aPath.getEnd()))
        {
            case 'A':
            case 'a':
                return FTP_TYPE_A;

            case 'D':
            case 'd':
                return FTP_TYPE_D;

            case 'I':
            case 'i':
                return FTP_TYPE_I;
        }
    return FTP_TYPE_NONE;
}

//============================================================================
sal_uInt32 INetURLObject::getIMAPUID() const
{
    if (m_eScheme == INET_PROT_IMAP
        && m_aPath.getLength() >= RTL_CONSTASCII_LENGTH("/;uid=") + 1)
    {
        sal_Unicode const * pBegin = m_aAbsURIRef.GetBuffer()
                                         + m_aPath.getBegin()
                                         + RTL_CONSTASCII_LENGTH("/;uid=");
        sal_Unicode const * pEnd = pBegin + m_aPath.getLength();
        sal_Unicode const * p = pEnd;
        while (p > pBegin && INetMIME::isDigit(p[-1]))
            --p;
        if (p < pEnd && *--p != '0'
            && m_aAbsURIRef.
                   EqualsIgnoreCaseAscii("/;uid=",
                                         p - RTL_CONSTASCII_LENGTH("/;uid=")
                                             - m_aAbsURIRef.GetBuffer(),
                                         RTL_CONSTASCII_LENGTH("/;uid=")))
        {
            sal_uInt32 nUID;
            if (INetMIME::scanUnsigned(p, pEnd, false, nUID))
                return nUID;
        }
    }
    return 0;
}

//============================================================================
// static
UniString INetURLObject::encodeText(sal_Unicode const * pBegin,
                                    sal_Unicode const * pEnd, bool bOctets,
                                    Part ePart, sal_Char cEscapePrefix,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset,
                                    bool bKeepVisibleEscapes)
{
    UniString aResult;
    while (pBegin < pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, bOctets, cEscapePrefix,
                                     eMechanism, eCharset, eEscapeType);
        appendUCS4(aResult, nUTF32, eEscapeType, bOctets, ePart,
                   cEscapePrefix, eCharset, bKeepVisibleEscapes);
    }
    return aResult;
}

//============================================================================
// static
UniString INetURLObject::decode(sal_Unicode const * pBegin,
                                sal_Unicode const * pEnd,
                                sal_Char cEscapePrefix,
                                DecodeMechanism eMechanism,
                                rtl_TextEncoding eCharset)
{
    switch (eMechanism)
    {
        case NO_DECODE:
            return UniString(pBegin, pEnd - pBegin);

        case DECODE_TO_IURI:
            eCharset = RTL_TEXTENCODING_UTF8;
            break;
    }
    UniString aResult;
    while (pBegin < pEnd)
    {
        EscapeType eEscapeType;
        sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, false, cEscapePrefix,
                                     WAS_ENCODED, eCharset, eEscapeType);
        switch (eEscapeType)
        {
            case ESCAPE_NO:
                aResult += sal_Unicode(nUTF32);
                break;

            case ESCAPE_OCTET:
                appendEscape(aResult, cEscapePrefix, nUTF32);
                break;

            case ESCAPE_UTF32:
                if (INetMIME::isUSASCII(nUTF32)
                    && (eMechanism == DECODE_TO_IURI
                        || eMechanism == DECODE_UNAMBIGUOUS
                           && mustEncode(nUTF32, PART_UNAMBIGUOUS)))
                    appendEscape(aResult, cEscapePrefix, nUTF32);
                else
                    aResult += sal_Unicode(nUTF32);
                break;
        }
    }
    return aResult;
}

//============================================================================
UniString INetURLObject::GetURLNoPass(DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset) const
{
    INetURLObject aTemp(*this);
    aTemp.clearPassword();
    return aTemp.GetMainURL(eMechanism, eCharset);
}

//============================================================================
UniString INetURLObject::GetURLNoMark(DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset) const
{
    INetURLObject aTemp(*this);
    aTemp.clearFragment();
    return aTemp.GetMainURL(eMechanism, eCharset);
}

//============================================================================
bool INetURLObject::operator ==(INetURLObject const & rObject) const
{
    if (m_eScheme != rObject.m_eScheme)
        return false;
    if (m_eScheme == INET_PROT_NOT_VALID)
        return (m_aAbsURIRef == rObject.m_aAbsURIRef) != false;
    if (GetUser(NO_DECODE) != rObject.GetUser(NO_DECODE)
        || GetPass(NO_DECODE) != rObject.GetPass(NO_DECODE)
        || !GetHost(NO_DECODE).EqualsIgnoreCaseAscii(rObject.
                                                         GetHost(NO_DECODE))
        || GetPort() != rObject.GetPort()
        || HasParam() != rObject.HasParam()
        || GetParam(NO_DECODE) != rObject.GetParam(NO_DECODE)
        || GetMsgId(NO_DECODE) != rObject.GetMsgId(NO_DECODE))
        return false;
    UniString aPath1(GetURLPath(NO_DECODE));
    UniString aPath2(rObject.GetURLPath(NO_DECODE));
    if (m_eScheme == INET_PROT_FILE)
    {
        // If the URL paths of two file URLs only differ in that one has a
        // final '/' and the other has not, take the two paths as equivalent
        // (this could be usefull for other schemes, too):
        xub_StrLen nLength = aPath1.Len();
        switch (sal_Int32(nLength) - sal_Int32(aPath2.Len()))
        {
            case -1:
                if (aPath2.GetChar(nLength) != '/')
                    return false;
                break;

            case 0:
                break;

            case 1:
                if (aPath1.GetChar(--nLength) != '/')
                    return false;
                break;

            default:
                return false;
        }
        return aPath1.CompareTo(aPath2, nLength) == COMPARE_EQUAL;
    }
    else
        return (aPath1 == aPath2) != false;
}

//============================================================================
bool INetURLObject::operator <(INetURLObject const & rObject) const
{
    switch (GetScheme(m_eScheme).CompareTo(GetScheme(rObject.m_eScheme)))
    {
        case COMPARE_LESS:
            return true;

        case COMPARE_GREATER:
            return false;
    }
    sal_uInt32 nPort1 = GetPort();
    sal_uInt32 nPort2 = rObject.GetPort();
    if (nPort1 < nPort2)
        return true;
    else if (nPort1 > nPort2)
        return false;
    switch (GetUser(NO_DECODE).CompareTo(rObject.GetUser(NO_DECODE)))
    {
        case COMPARE_LESS:
            return true;

        case COMPARE_GREATER:
            return false;
    }
    switch (GetPass(NO_DECODE).CompareTo(rObject.GetPass(NO_DECODE)))
    {
        case COMPARE_LESS:
            return true;

        case COMPARE_GREATER:
            return false;
    }
    switch (GetHost(NO_DECODE).CompareTo(rObject.GetHost(NO_DECODE)))
    {
        case COMPARE_LESS:
            return true;

        case COMPARE_GREATER:
            return false;
    }
    UniString aPath1(GetURLPath(NO_DECODE));
    UniString aPath2(rObject.GetURLPath(NO_DECODE));
    switch (aPath1.CompareTo(aPath2))
    {
        case COMPARE_LESS:
            return true;

        case COMPARE_GREATER:
            return false;
    }
    switch (GetParam(NO_DECODE).CompareTo(rObject.GetParam(NO_DECODE)))
    {
        case COMPARE_LESS:
            return true;

        case COMPARE_GREATER:
            return false;
    }
    return GetMsgId(NO_DECODE).CompareTo(rObject.GetMsgId(NO_DECODE))
               == COMPARE_LESS;
}

//============================================================================
bool INetURLObject::ConcatData(INetProtocol eTheScheme,
                               UniString const & rTheUser,
                               UniString const & rThePassword,
                               UniString const & rTheHost,
                               sal_uInt32 nThePort,
                               UniString const & rThePath,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset)
{
    setInvalid();
    m_eScheme = eTheScheme;
    if (HasError())
        return false;
    m_aAbsURIRef.AssignAscii(getSchemeInfo().m_pScheme);
    m_aAbsURIRef += ':';
    if (getSchemeInfo().m_bAuthority)
    {
        m_aAbsURIRef.AppendAscii(RTL_CONSTASCII_STRINGPARAM("//"));
        bool bUserInfo = false;
        if (getSchemeInfo().m_bUser)
        {
            if (m_eScheme == INET_PROT_IMAP && rTheUser.Len() == 0)
            {
                setInvalid();
                return false;
            }
            if (rTheUser.Len() != 0)
            {
                m_aUser.set(m_aAbsURIRef,
                            encodeText(rTheUser, false,
                                       m_eScheme == INET_PROT_IMAP ?
                                           PART_IMAP_ACHAR :
                                       m_eScheme == INET_PROT_VIM ?
                                           PART_VIM :
                                           PART_USER_PASSWORD,
                                       getEscapePrefix(), eMechanism,
                                       eCharset, false),
                            m_aAbsURIRef.Len());
                bUserInfo = true;
            }
        }
        else if (rTheUser.Len() != 0)
        {
            setInvalid();
            return false;
        }
        if (rThePassword.Len() != 0)
            if (getSchemeInfo().m_bPassword)
            {
                m_aAbsURIRef += ':';
                m_aAuth.set(m_aAbsURIRef,
                            encodeText(rThePassword, false,
                                       m_eScheme == INET_PROT_VIM ?
                                           PART_VIM : PART_USER_PASSWORD,
                                       getEscapePrefix(), eMechanism,
                                       eCharset, false),
                            m_aAbsURIRef.Len());
                bUserInfo = true;
            }
            else
            {
                setInvalid();
                return false;
            }
        if (bUserInfo && getSchemeInfo().m_bHost)
            m_aAbsURIRef += '@';
        if (getSchemeInfo().m_bHost)
        {
            UniString aSynHost(rTheHost);
            switch (m_eScheme)
            {
                case INET_PROT_FILE:
                    if (aSynHost.EqualsIgnoreCaseAscii("localhost"))
                        aSynHost.Erase();
                    break;

                case INET_PROT_LDAP:
                    if (aSynHost.Len() == 0 && nThePort != 0)
                    {
                        setInvalid();
                        return false;
                    }
                    break;

                default:
                    if (aSynHost.Len() == 0)
                    {
                        setInvalid();
                        return false;
                    }
                    break;
            }
            if (aSynHost.Len() != 0)
            {
                sal_Unicode const * p = aSynHost.GetBuffer();
                sal_Unicode const * pEnd = p + aSynHost.Len();
                if (!parseHost(p, pEnd, false, eMechanism, eCharset, aSynHost)
                    || p != pEnd)
                {
                    setInvalid();
                    return false;
                }
            }
            m_aHost.set(m_aAbsURIRef, aSynHost, m_aAbsURIRef.Len());
            if (nThePort != 0)
                if (getSchemeInfo().m_bPort)
                {
                    m_aAbsURIRef += ':';
                    m_aPort.set(m_aAbsURIRef,
                                UniString::CreateFromInt64(nThePort),
                                m_aAbsURIRef.Len());
                }
                else
                {
                    setInvalid();
                    return false;
                }
        }
        else if (rTheHost.Len() != 0 || nThePort != 0)
        {
            setInvalid();
            return false;
        }
    }
    UniString aSynPath;
    if (getSchemeInfo().m_bHierarchical
        && rThePath.Len() == 0 || rThePath.GetChar(0) != '/')
        aSynPath = '/';
    aSynPath += rThePath;
    m_aPath.set(m_aAbsURIRef,
                encodeText(aSynPath, false,
                           m_eScheme == INET_PROT_FILE ?
                               PART_PATH_SEGMENTS_EXTRA :
                           m_eScheme == INET_PROT_NEWS
                           || m_eScheme == INET_PROT_POP3 ?
                               PART_MESSAGE_ID_PATH :
                               PART_HTTP_PATH,
                           getEscapePrefix(), eMechanism, eCharset, true),
                m_aAbsURIRef.Len());
    return true;
}

//============================================================================
UniString INetURLObject::getExternalURL(DecodeMechanism eMechanism,
                                        rtl_TextEncoding eCharset) const
{
    UniString aTheExtURIRef;
    translateToExternal(m_aAbsURIRef, aTheExtURIRef, NOT_CANONIC, eMechanism,
                        eCharset);
    return aTheExtURIRef;
}

//============================================================================
// static
UniString INetURLObject::GetScheme(INetProtocol eTheScheme)
{
    return UniString::CreateFromAscii(getSchemeInfo(eTheScheme).m_pPrefix);
}

//============================================================================
// static
INetProtocol INetURLObject::CompareProtocolScheme(UniString const &
                                                      rTheAbsURIRef)
{
    sal_Unicode const * p = rTheAbsURIRef.GetBuffer();
    PrefixInfo const * pPrefix = getPrefix(p, p + rTheAbsURIRef.Len());
    return pPrefix ? pPrefix->m_eScheme : INET_PROT_NOT_VALID;
}

//============================================================================
bool INetURLObject::hasPassword() const
{
    return m_aAuth.isPresent() && getSchemeInfo().m_bPassword;
}

//============================================================================
void INetURLObject::makeAuthCanonic()
{
    if (m_eScheme == INET_PROT_IMAP && m_aAuth.getLength() == 1
        && m_aAbsURIRef.GetChar(m_aAuth.getBegin()) == '*')
    {
        m_aAbsURIRef.Erase(m_aAuth.getBegin()
                               - RTL_CONSTASCII_LENGTH(";AUTH="),
                           RTL_CONSTASCII_LENGTH(";AUTH=*"));
        sal_Int32 nDelta = m_aAuth.clear() - RTL_CONSTASCII_LENGTH(";AUTH=");
        m_aPath += nDelta;
        m_aQuery += nDelta;
        m_aFragment += nDelta;
    }
}

//============================================================================
UniString INetURLObject::GetHostPort(DecodeMechanism eMechanism,
                                     rtl_TextEncoding eCharset)
{
    UniString aHostPort(decode(m_aHost, getEscapePrefix(), eMechanism,
                               eCharset));
    if (m_aPort.isPresent())
    {
        aHostPort += ':';
        aHostPort += decode(m_aPort, getEscapePrefix(), eMechanism, eCharset);
    }
    return aHostPort;
}

//============================================================================
sal_uInt32 INetURLObject::GetPort() const
{
    if (m_aPort.isPresent())
    {
        sal_Unicode const * p = m_aAbsURIRef.GetBuffer() + m_aPort.getBegin();
        sal_Unicode const * pEnd = p + m_aPort.getLength();
        sal_uInt32 nThePort;
        if (INetMIME::scanUnsigned(p, pEnd, true, nThePort) && p == pEnd)
            return nThePort;
    }
    return 0;
}

//============================================================================
bool INetURLObject::SetPort(sal_uInt32 nThePort)
{
    if (getSchemeInfo().m_bPort && m_aHost.isPresent())
    {
        UniString aNewPort(UniString::CreateFromInt64(nThePort));
        sal_Int32 nDelta;
        if (m_aPort.isPresent())
            nDelta = m_aPort.set(m_aAbsURIRef, aNewPort);
        else
        {
            m_aAbsURIRef.Insert(':', m_aHost.getEnd());
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

//============================================================================
void INetURLObject::makePortCanonic()
{
    if (m_aPort.isPresent())
    {
        sal_Unicode const * p = m_aAbsURIRef.GetBuffer() + m_aPort.getBegin();
        sal_Unicode const * pEnd = p + m_aPort.getLength();
        sal_uInt32 nThePort;
        if (INetMIME::scanUnsigned(p, pEnd, true, nThePort) && p == pEnd)
        {
            sal_Int32 nDelta;
            if (nThePort != 0 && nThePort == getSchemeInfo().m_nDefaultPort)
            {
                m_aAbsURIRef.Erase(m_aPort.getBegin() - 1,
                                   m_aPort.getLength() + 1);
                nDelta = m_aPort.clear() - 1;
            }
            else
                nDelta = m_aPort.set(m_aAbsURIRef,
                                     UniString::CreateFromInt64(nThePort));
            m_aPath += nDelta;
            m_aQuery += nDelta;
            m_aFragment += nDelta;
        }
    }
}

//============================================================================
// static
bool INetURLObject::parseHost(sal_Unicode const *& rBegin,
                              sal_Unicode const * pEnd, bool bOctets,
                              EncodeMechanism eMechanism,
                              rtl_TextEncoding eCharset, UniString & rCanonic)
{
    enum State { STATE_INITIAL, STATE_LABEL, STATE_LABEL_HYPHEN,
                 STATE_LABEL_DOT, STATE_TOPLABEL, STATE_TOPLABEL_HYPHEN,
                 STATE_TOPLABEL_DOT, STATE_IP4, STATE_IP4_DOT, STATE_IP6,
                 STATE_IP6_COLON, STATE_IP6_2COLON, STATE_IP6_3COLON,
                 STATE_IP6_HEXSEQ1, STATE_IP6_HEXSEQ1_COLON,
                 STATE_IP6_HEXSEQ1_MAYBE_IP4, STATE_IP6_HEXSEQ2,
                 STATE_IP6_HEXSEQ2_COLON, STATE_IP6_HEXSEQ2_MAYBE_IP4,
                 STATE_IP6_IP4, STATE_IP6_IP4_DOT, STATE_IP6_DONE };
    UniString aTheCanonic;
    sal_uInt32 nNumber;
    int nDigits;
    int nOctets;
    State eState = STATE_INITIAL;
    sal_Unicode const * p = rBegin;
    for (; p != pEnd; ++p)
        switch (eState)
        {
            case STATE_INITIAL:
                if (*p == '[')
                {
                    aTheCanonic = '[';
                    eState = STATE_IP6;
                }
                else if (INetMIME::isAlpha(*p))
                    eState = STATE_TOPLABEL;
                else if (INetMIME::isDigit(*p))
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
                else if (!INetMIME::isAlphanumeric(*p))
                    goto done;
                break;

            case STATE_LABEL_HYPHEN:
                if (INetMIME::isAlphanumeric(*p))
                    eState = STATE_LABEL;
                else if (*p != '-')
                    goto done;
                break;

            case STATE_LABEL_DOT:
                if (INetMIME::isAlpha(*p))
                    eState = STATE_TOPLABEL;
                else if (INetMIME::isDigit(*p))
                    eState = STATE_LABEL;
                else
                    goto done;
                break;

            case STATE_TOPLABEL:
                if (*p == '.')
                    eState = STATE_TOPLABEL_DOT;
                else if (*p == '-')
                    eState = STATE_TOPLABEL_HYPHEN;
                else if (!INetMIME::isAlphanumeric(*p))
                    goto done;
                break;

            case STATE_TOPLABEL_HYPHEN:
                if (INetMIME::isAlphanumeric(*p))
                    eState = STATE_TOPLABEL;
                else if (*p != '-')
                    goto done;
                break;

            case STATE_TOPLABEL_DOT:
                if (INetMIME::isAlpha(*p))
                    eState = STATE_TOPLABEL;
                else if (INetMIME::isDigit(*p))
                    eState = STATE_LABEL;
                else
                    goto done;
                break;

            case STATE_IP4:
                if (*p == '.')
                    if (nOctets < 4)
                    {
                        aTheCanonic += UniString::CreateFromInt32(nNumber);
                        aTheCanonic += '.';
                        ++nOctets;
                        eState = STATE_IP4_DOT;
                    }
                    else
                        eState = STATE_LABEL_DOT;
                else if (*p == '-')
                    eState = STATE_LABEL_HYPHEN;
                else if (INetMIME::isAlpha(*p))
                    eState = STATE_LABEL;
                else if (INetMIME::isDigit(*p))
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
                if (INetMIME::isAlpha(*p))
                    eState = STATE_TOPLABEL;
                else if (INetMIME::isDigit(*p))
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
                else if (INetMIME::isHexDigit(*p))
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
                    aTheCanonic.AppendAscii(RTL_CONSTASCII_STRINGPARAM("::"));
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
                    aTheCanonic += ':';
                    eState = STATE_IP6_3COLON;
                }
                else if (INetMIME::isHexDigit(*p))
                {
                    nNumber = INetMIME::getHexWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ2;
                }
                else
                    goto done;
                break;

            case STATE_IP6_3COLON:
                if (INetMIME::isDigit(*p))
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
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    aTheCanonic += ':';
                    eState = STATE_IP6_HEXSEQ1_COLON;
                }
                else if (INetMIME::isHexDigit(*p) && nDigits < 4)
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
                    aTheCanonic.AppendAscii(RTL_CONSTASCII_STRINGPARAM("::"));
                    eState = STATE_IP6_2COLON;
                }
                else if (INetMIME::isDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ1_MAYBE_IP4;
                }
                else if (INetMIME::isHexDigit(*p))
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
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    aTheCanonic += ':';
                    eState = STATE_IP6_HEXSEQ1_COLON;
                }
                else if (*p == '.')
                {
                    nNumber = 100 * (nNumber >> 8) + 10 * (nNumber >> 4 & 15)
                                  + (nNumber & 15);
                    aTheCanonic += UniString::CreateFromInt32(nNumber);
                    aTheCanonic += '.';
                    nOctets = 2;
                    eState = STATE_IP6_IP4_DOT;
                }
                else if (INetMIME::isDigit(*p) && nDigits < 3)
                {
                    nNumber = 16 * nNumber + INetMIME::getWeight(*p);
                    ++nDigits;
                }
                else if (INetMIME::isHexDigit(*p) && nDigits < 4)
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
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    aTheCanonic += ':';
                    eState = STATE_IP6_HEXSEQ2_COLON;
                }
                else if (INetMIME::isHexDigit(*p) && nDigits < 4)
                {
                    nNumber = 16 * nNumber + INetMIME::getHexWeight(*p);
                    ++nDigits;
                }
                else
                    goto done;
                break;

            case STATE_IP6_HEXSEQ2_COLON:
                if (INetMIME::isDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_HEXSEQ2_MAYBE_IP4;
                }
                else if (INetMIME::isHexDigit(*p))
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
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    eState = STATE_IP6_DONE;
                }
                else if (*p == ':')
                {
                    aTheCanonic += UniString::CreateFromInt32(nNumber, 16);
                    aTheCanonic += ':';
                    eState = STATE_IP6_HEXSEQ2_COLON;
                }
                else if (*p == '.')
                {
                    nNumber = 100 * (nNumber >> 8) + 10 * (nNumber >> 4 & 15)
                                  + (nNumber & 15);
                    aTheCanonic += UniString::CreateFromInt32(nNumber);
                    aTheCanonic += '.';
                    nOctets = 2;
                    eState = STATE_IP6_IP4_DOT;
                }
                else if (INetMIME::isDigit(*p) && nDigits < 3)
                {
                    nNumber = 16 * nNumber + INetMIME::getWeight(*p);
                    ++nDigits;
                }
                else if (INetMIME::isHexDigit(*p) && nDigits < 4)
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
                        aTheCanonic += UniString::CreateFromInt32(nNumber);
                        eState = STATE_IP6_DONE;
                    }
                    else
                        goto done;
                else if (*p == '.')
                    if (nOctets < 4)
                    {
                        aTheCanonic += UniString::CreateFromInt32(nNumber);
                        aTheCanonic += '.';
                        ++nOctets;
                        eState = STATE_IP6_IP4_DOT;
                    }
                    else
                        goto done;
                else if (INetMIME::isDigit(*p) && nDigits < 3)
                {
                    nNumber = 10 * nNumber + INetMIME::getWeight(*p);
                    ++nDigits;
                }
                else
                    goto done;
                break;

            case STATE_IP6_IP4_DOT:
                if (INetMIME::isDigit(*p))
                {
                    nNumber = INetMIME::getWeight(*p);
                    nDigits = 1;
                    eState = STATE_IP6_IP4;
                }
                else
                    goto done;
                break;
        }
 done:
    switch (eState)
    {
        case STATE_LABEL:
        case STATE_TOPLABEL:
        case STATE_TOPLABEL_DOT:
            aTheCanonic.Assign(rBegin, p - rBegin);
            rBegin = p;
            rCanonic = aTheCanonic;
            return true;

        case STATE_IP4:
            if (nOctets == 4)
            {
                aTheCanonic += UniString::CreateFromInt32(nNumber);
                rBegin = p;
                rCanonic = aTheCanonic;
                return true;
            }
            break;

        case STATE_IP6_DONE:
            aTheCanonic += ']';
            rBegin = p;
            rCanonic = aTheCanonic;
            return true;
    }
    return false;
}

//============================================================================
sal_Int32 INetURLObject::getSegmentCount(bool bIgnoreFinalSlash) const
{
    if (!getSchemeInfo().m_bHierarchical)
        return 0;

    sal_Unicode const * p = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pEnd = p + m_aPath.getLength();
    if (p == pEnd || *p != '/')
        return 0;

    if (bIgnoreFinalSlash && pEnd[-1] == '/')
        --pEnd;

    sal_Int32 n = 0;
    while (p != pEnd)
        if (*p++ == '/')
            ++n;
    return n;
}

//============================================================================
bool INetURLObject::removeSegment(sal_Int32 nIndex, bool bIgnoreFinalSlash)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    UniString aNewPath(m_aAbsURIRef, m_aPath.getBegin(),
                       aSegment.getBegin() - m_aPath.getBegin());
    if (bIgnoreFinalSlash && aSegment.getEnd() == m_aPath.getEnd())
        aNewPath += '/';
    else
        aNewPath.Append(m_aAbsURIRef.GetBuffer() + aSegment.getEnd(),
                        m_aPath.getEnd() - aSegment.getEnd());
    if (aNewPath.Len() == 0)
        aNewPath = '/';

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
UniString INetURLObject::getName(sal_Int32 nIndex, bool bIgnoreFinalSlash,
                                 DecodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset) const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return UniString();

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * p = pSegBegin;
    while (p != pSegEnd && *p != ';')
        ++p;

    return decode(pSegBegin, p, getEscapePrefix(), eMechanism, eCharset);
}

//============================================================================
bool INetURLObject::setName(UniString const & rTheName, sal_Int32 nIndex,
                            bool bIgnoreFinalSlash,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * p = pSegBegin;
    while (p != pSegEnd && *p != ';')
        ++p;

    UniString aNewPath(pPathBegin, pSegBegin - pPathBegin);
    aNewPath += encodeText(rTheName, false, PART_PCHAR, getEscapePrefix(),
                           eMechanism, eCharset, true);
    aNewPath.Append(p, pPathEnd - p);

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
bool INetURLObject::hasExtension(sal_Int32 nIndex, bool bIgnoreFinalSlash)
    const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    for (sal_Unicode const * p = pSegBegin; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            return true;
    return false;
}

//============================================================================
UniString INetURLObject::getBase(sal_Int32 nIndex, bool bIgnoreFinalSlash,
                                 DecodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset) const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return UniString();

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * pExtension = 0;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        pExtension = p;

    return decode(pSegBegin, pExtension, getEscapePrefix(), eMechanism,
                  eCharset);
}

//============================================================================
bool INetURLObject::setBase(UniString const & rTheBase, sal_Int32 nIndex,
                            bool bIgnoreFinalSlash,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * pExtension = 0;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        pExtension = p;

    UniString aNewPath(pPathBegin, pSegBegin - pPathBegin);
    aNewPath += encodeText(rTheBase, false, PART_PCHAR, getEscapePrefix(),
                           eMechanism, eCharset, true);
    aNewPath.Append(pExtension, pPathEnd - pExtension);

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
UniString INetURLObject::getExtension(sal_Int32 nIndex,
                                      bool bIgnoreFinalSlash,
                                      DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset) const
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return UniString();

    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * pExtension = 0;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;

    if (!pExtension)
        return UniString();

    return decode(pExtension + 1, p, getEscapePrefix(), eMechanism, eCharset);
}

//============================================================================
bool INetURLObject::setExtension(UniString const & rTheExtension,
                                 sal_Int32 nIndex, bool bIgnoreFinalSlash,
                                 EncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * pExtension = 0;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        pExtension = p;

    UniString aNewPath(pPathBegin, pExtension - pPathBegin);
    aNewPath += '.';
    aNewPath += encodeText(rTheExtension, false, PART_PCHAR,
                           getEscapePrefix(), eMechanism, eCharset, true);
    aNewPath.Append(p, pPathEnd - p);

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
bool INetURLObject::removeExtension(sal_Int32 nIndex, bool bIgnoreFinalSlash)
{
    SubString aSegment(getSegment(nIndex, bIgnoreFinalSlash));
    if (!aSegment.isPresent())
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    sal_Unicode const * pSegBegin
        = m_aAbsURIRef.GetBuffer() + aSegment.getBegin();
    sal_Unicode const * pSegEnd = pSegBegin + aSegment.getLength();

    ++pSegBegin;
    sal_Unicode const * pExtension = 0;
    sal_Unicode const * p = pSegBegin;
    for (; p != pSegEnd && *p != ';'; ++p)
        if (*p == '.' && p != pSegBegin)
            pExtension = p;
    if (!pExtension)
        return true;

    UniString aNewPath(pPathBegin, pExtension - pPathBegin);
    aNewPath.Append(p, pPathEnd - p);

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
bool INetURLObject::hasFinalSlash() const
{
    if (!getSchemeInfo().m_bHierarchical)
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathBegin == pPathEnd || *pPathBegin != '/')
        return false;

    return pPathEnd[-1] == '/';
}

//============================================================================
bool INetURLObject::setFinalSlash()
{
    if (!getSchemeInfo().m_bHierarchical)
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathBegin == pPathEnd || *pPathBegin != '/')
        return false;

    if (pPathEnd[-1] == '/')
        return true;

    UniString aNewPath(pPathBegin, pPathEnd - pPathBegin);
    aNewPath += '/';

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
bool INetURLObject::removeFinalSlash()
{
    if (!getSchemeInfo().m_bHierarchical)
        return false;

    sal_Unicode const * pPathBegin
        = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pPathEnd = pPathBegin + m_aPath.getLength();
    if (pPathBegin == pPathEnd || *pPathBegin != '/')
        return false;

    if (pPathEnd[-1] != '/')
        return true;

    --pPathEnd;
    if (pPathEnd == pPathBegin)
        return false;
    UniString aNewPath(pPathBegin, pPathEnd - pPathBegin);

    return setPath(aNewPath, false, NOT_CANONIC, RTL_TEXTENCODING_UTF8);
}

//============================================================================
// static
UniString INetURLObject::createFragment(UniString const & rText)
{
    UniString aFragment(rText);
    for (xub_StrLen i = 0; i < aFragment.Len();)
    {
        sal_Unicode c = aFragment.GetChar(i);
        if (mustEncode(aFragment.GetChar(i), PART_CREATEFRAGMENT))
            aFragment.Erase(i, 1);
        else
            ++i;
    }
    return aFragment;
}

//============================================================================
bool INetURLObject::setFSysPath(UniString const & rFSysPath, FSysStyle eStyle)
{
    sal_Unicode const * pFSysBegin = rFSysPath.GetBuffer();
    sal_Unicode const * pFSysEnd = pFSysBegin + rFSysPath.Len();

    switch ((eStyle & FSYS_VOS ? 1 : 0)
                + (eStyle & FSYS_UNX ? 1 : 0)
                + (eStyle & FSYS_DOS ? 1 : 0)
                + (eStyle & FSYS_MAC ? 1 : 0))
    {
        case 0:
            return false;

        case 1:
            break;

        default:
            if (eStyle & FSYS_VOS
                && pFSysEnd - pFSysBegin >= 2
                && pFSysBegin[0] == '/'
                && pFSysBegin[1] == '/')
            {
                if (pFSysEnd - pFSysBegin >= 3
                    && pFSysBegin[2] == '.'
                    && (pFSysEnd - pFSysBegin == 3 || pFSysBegin[3] == '/'))
                {
                    eStyle = FSYS_VOS; // Production T1
                    break;
                }

                sal_Unicode const * p = pFSysBegin + 2;
                UniString aHost;
                if (parseHost(p, pFSysEnd, false, ENCODE_ALL,
                              RTL_TEXTENCODING_UTF8, aHost)
                    && (p == pFSysEnd || *p == '/'))
                {
                    eStyle = FSYS_VOS; // Production T2
                    break;
                }
            }

            if (eStyle & FSYS_DOS
                && pFSysEnd - pFSysBegin >= 2
                && pFSysBegin[0] == '\\'
                && pFSysBegin[1] == '\\')
            {
                sal_Unicode const * p = pFSysBegin + 2;
                UniString aHost;
                if (parseHost(p, pFSysEnd, false, ENCODE_ALL,
                              RTL_TEXTENCODING_UTF8, aHost)
                    && (p == pFSysEnd || *p == '\\'))
                {
                    eStyle = FSYS_DOS; // Production T3
                    break;
                }
            }

            if (eStyle & FSYS_DOS
                && pFSysEnd - pFSysBegin >= 3
                && INetMIME::isAlpha(pFSysBegin[0])
                && pFSysBegin[1] == ':'
                && (pFSysBegin[2] == '/' || pFSysBegin[2] == '\\'))
            {
                eStyle = FSYS_DOS; // Productions T4, T5
                break;
            }

            if (!(eStyle & (FSYS_UNX | FSYS_DOS | FSYS_MAC)))
                return false;

            eStyle = guessFSysStyleByCounting(pFSysBegin, pFSysEnd, eStyle);
                // Production T6
            break;
    }

    UniString aSynAbsURIRef(RTL_CONSTASCII_USTRINGPARAM("file://"));
    switch (eStyle)
    {
        case FSYS_VOS:
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
                        appendEscape(aSynAbsURIRef, '%', *p);
                        break;

                    default:
                        aSynAbsURIRef += *p;
                        break;
                }
            break;
        }

        case FSYS_UNX:
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
                        appendEscape(aSynAbsURIRef, '%', *p);
                        break;

                    default:
                        aSynAbsURIRef += *p;
                        break;
                }
            break;
        }

        case FSYS_DOS:
        {
            sal_uInt32 nAltDelimiter = 0x80000000;
            sal_Unicode const * p = pFSysBegin;
            if (pFSysEnd - p >= 3 && p[0] == '\\' && p[1] == '\\')
                p += 2;
            else
            {
                aSynAbsURIRef += '/';
                if (pFSysEnd - p >= 3 && INetMIME::isAlpha(p[0])
                    && p[1] == ':' && (p[2] == '\\' || p[2] == '/'))
                    nAltDelimiter = '/';
            }
            for (; p != pFSysEnd; ++p)
                if (*p == '\\' || *p == nAltDelimiter)
                    aSynAbsURIRef += '/';
                else
                    switch (*p)
                    {
                        case '/':
                        case '#':
                        case '%':
                            appendEscape(aSynAbsURIRef, '%', *p);
                            break;

                        default:
                            aSynAbsURIRef += *p;
                            break;
                    }
            break;
        }

        case FSYS_MAC:
            aSynAbsURIRef += '/';
            {for (sal_Unicode const * p = pFSysBegin; p != pFSysEnd; ++p)
                switch (*p)
                {
                    case ':':
                        aSynAbsURIRef += '/';
                        break;

                    case '/':
                    case '|':
                    case '#':
                    case '%':
                        appendEscape(aSynAbsURIRef, '%', *p);
                        break;

                    default:
                        aSynAbsURIRef += *p;
                        break;
                }
            }
            break;
    }

    INetURLObject aTemp(aSynAbsURIRef, WAS_ENCODED, RTL_TEXTENCODING_UTF8);
    if (aTemp.HasError())
        return false;

    *this = aTemp;
    return true;
}

//============================================================================
UniString INetURLObject::getFSysPath(FSysStyle eStyle,
                                     sal_Unicode * pDelimiter) const
{
    if (m_eScheme != INET_PROT_FILE)
        return UniString();

    if ((eStyle & FSYS_VOS ? 1 : 0)
                + (eStyle & FSYS_UNX ? 1 : 0)
                + (eStyle & FSYS_DOS ? 1 : 0)
                + (eStyle & FSYS_MAC ? 1 : 0)
            > 1)
    {
        sal_Unicode const * p = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
        eStyle = eStyle & FSYS_VOS
                 && m_aHost.isPresent()
                 && m_aHost.getLength() > 0 ?
                     FSYS_VOS :
                 eStyle & FSYS_DOS
                 && (!m_aHost.isPresent() || m_aHost.getLength() == 0)
                 && m_aPath.getLength() >= 3
                 && p[0] == '/'
                 && INetMIME::isAlpha(p[1])
                 && p[2] == ':'
                 && (m_aPath.getLength() == 3 || p[3] == '/') ?
                     FSYS_DOS :
                 eStyle & FSYS_UNX
                 && (!m_aHost.isPresent() || m_aHost.getLength() == 0) ?
                     FSYS_UNX :
                     FSysStyle(0);
    }

    switch (eStyle)
    {
        case FSYS_VOS:
        {
            if (pDelimiter)
                *pDelimiter = '/';

            UniString aSynFSysPath(RTL_CONSTASCII_USTRINGPARAM("//"));
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
                aSynFSysPath += decode(m_aHost, '%', DECODE_WITH_CHARSET,
                                       RTL_TEXTENCODING_UTF8);
            else
                aSynFSysPath += '.';
            aSynFSysPath += decode(m_aPath, '%', DECODE_WITH_CHARSET,
                                   RTL_TEXTENCODING_UTF8);
            return aSynFSysPath;
        }

        case FSYS_UNX:
        {
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
                return UniString();

            if (pDelimiter)
                *pDelimiter = '/';

            return decode(m_aPath, '%', DECODE_WITH_CHARSET,
                          RTL_TEXTENCODING_UTF8);
        }

        case FSYS_DOS:
        {
            if (pDelimiter)
                *pDelimiter = '\\';

            UniString aSynFSysPath;
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
            {
                aSynFSysPath.AssignAscii(RTL_CONSTASCII_STRINGPARAM("\\\\"));
                aSynFSysPath += decode(m_aHost, '%', DECODE_WITH_CHARSET,
                                       RTL_TEXTENCODING_UTF8);
                aSynFSysPath += '\\';
            }
            sal_Unicode const * p
                = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
            sal_Unicode const * pEnd = p + m_aPath.getLength();
            DBG_ASSERT(p < pEnd && *p == '/',
                       "INetURLObject::getFSysPath(): Bad path");
            ++p;
            while (p < pEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(p, pEnd, false, '%', WAS_ENCODED,
                                             RTL_TEXTENCODING_UTF8,
                                             eEscapeType);
                if (eEscapeType == ESCAPE_NO && nUTF32 == '/')
                    aSynFSysPath += '\\';
                else
                    appendUTF32(aSynFSysPath, nUTF32);
            }
            return aSynFSysPath;
        }

        case FSYS_MAC:
        {
            if (m_aHost.isPresent() && m_aHost.getLength() > 0)
                return UniString();

            if (pDelimiter)
                *pDelimiter = ':';

            UniString aSynFSysPath;
            sal_Unicode const * p
                = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
            sal_Unicode const * pEnd = p + m_aPath.getLength();
            DBG_ASSERT(p < pEnd && *p == '/',
                       "INetURLObject::getFSysPath(): Bad path");
            ++p;
            while (p < pEnd)
            {
                EscapeType eEscapeType;
                sal_uInt32 nUTF32 = getUTF32(p, pEnd, false, '%', WAS_ENCODED,
                                             RTL_TEXTENCODING_UTF8,
                                             eEscapeType);
                if (eEscapeType == ESCAPE_NO && nUTF32 == '/')
                    aSynFSysPath += ':';
                else
                    appendUTF32(aSynFSysPath, nUTF32);
            }
            return aSynFSysPath;
        }
    }

    return UniString();
}

//============================================================================
bool INetURLObject::HasMsgId() const
{
    if (m_eScheme != INET_PROT_NEWS && m_eScheme != INET_PROT_POP3)
        return false;
    sal_Unicode const * p = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pEnd = p + m_aPath.getLength();
    for (; p < pEnd; ++p)
        if (*p == '<')
            return true;
    return false;
}

//============================================================================
UniString INetURLObject::GetMsgId(DecodeMechanism eMechanism,
                                  rtl_TextEncoding eCharset) const
{
    if (m_eScheme != INET_PROT_NEWS && m_eScheme != INET_PROT_POP3)
        return UniString();
    sal_Unicode const * p = m_aAbsURIRef.GetBuffer() + m_aPath.getBegin();
    sal_Unicode const * pEnd = p + m_aPath.getLength();
    for (; p < pEnd; ++p)
        if (*p == '<')
            return decode(p, pEnd, getEscapePrefix(), eMechanism, eCharset);
    return UniString();
}

//============================================================================
// static
void INetURLObject::appendUCS4Escape(UniString & rTheText,
                                     sal_Char cEscapePrefix, sal_uInt32 nUCS4)
{
    DBG_ASSERT(nUCS4 < 0x80000000,
               "INetURLObject::appendUCS4Escape(): Bad char");
    if (nUCS4 < 0x80)
        appendEscape(rTheText, cEscapePrefix, nUCS4);
    else if (nUCS4 < 0x800)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 | 0xC0);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else if (nUCS4 < 0x10000)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 | 0xE0);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else if (nUCS4 < 0x200000)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 18 | 0xF0);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else if (nUCS4 < 0x4000000)
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 24 | 0xF8);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 18 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
    else
    {
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 30 | 0xFC);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 24 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 18 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 12 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 >> 6 & 0x3F | 0x80);
        appendEscape(rTheText, cEscapePrefix, nUCS4 & 0x3F | 0x80);
    }
}

//============================================================================
// static
void INetURLObject::appendUCS4(UniString & rTheText, sal_uInt32 nUCS4,
                               EscapeType eEscapeType, bool bOctets,
                               Part ePart, sal_Char cEscapePrefix,
                               rtl_TextEncoding eCharset,
                               bool bKeepVisibleEscapes)
{
    bool bEscape;
    rtl_TextEncoding eTargetCharset;
    switch (eEscapeType)
    {
        case ESCAPE_NO:
            if (mustEncode(nUCS4, ePart))
            {
                bEscape = true;
                eTargetCharset = bOctets ? RTL_TEXTENCODING_ISO_8859_1 :
                                           RTL_TEXTENCODING_UTF8;
            }
            else
                bEscape = false;
            break;

        case ESCAPE_OCTET:
            bEscape = true;
            eTargetCharset = RTL_TEXTENCODING_ISO_8859_1;
            break;

        case ESCAPE_UTF32:
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
    }
    if (bEscape)
        switch (eTargetCharset)
        {
            default:
                DBG_ERROR("INetURLObject::appendUCS4(): Unsupported charset");
            case RTL_TEXTENCODING_ASCII_US:
            case RTL_TEXTENCODING_ISO_8859_1:
                appendEscape(rTheText, cEscapePrefix, nUCS4);
                break;

            case RTL_TEXTENCODING_UTF8:
                appendUCS4Escape(rTheText, cEscapePrefix, nUCS4);
                break;
        }
    else
        rTheText += sal_Unicode(nUCS4);
}

//============================================================================
// static
sal_uInt32 INetURLObject::getUTF32(sal_Unicode const *& rBegin,
                                   sal_Unicode const * pEnd, bool bOctets,
                                   sal_Char cEscapePrefix,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset,
                                   EscapeType & rEscapeType)
{
    DBG_ASSERT(rBegin < pEnd, "INetURLObject::getUTF32(): Bad sequence");
    sal_uInt32 nUTF32 = bOctets ? *rBegin++ :
                                  INetMIME::getUTF32Character(rBegin, pEnd);
    switch (eMechanism)
    {
        case ENCODE_ALL:
            rEscapeType = ESCAPE_NO;
            break;

        case WAS_ENCODED:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == cEscapePrefix && rBegin + 1 < pEnd
                && (nWeight1 = INetMIME::getHexWeight(rBegin[0])) >= 0
                && (nWeight2 = INetMIME::getHexWeight(rBegin[1])) >= 0)
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                switch (eCharset)
                {
                    default:
                        DBG_ERROR(
                            "INetURLObject::getUTF32(): Unsupported charset");
                    case RTL_TEXTENCODING_ASCII_US:
                        rEscapeType = INetMIME::isUSASCII(nUTF32) ?
                                          ESCAPE_UTF32 : ESCAPE_OCTET;
                        break;

                    case RTL_TEXTENCODING_ISO_8859_1:
                        rEscapeType = ESCAPE_UTF32;
                        break;

                    case RTL_TEXTENCODING_UTF8:
                        if (INetMIME::isUSASCII(nUTF32))
                            rEscapeType = ESCAPE_UTF32;
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
                                        || p[0] != cEscapePrefix
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
                                if (bUTF8 && nEncoded >= nMin
                                    && !INetMIME::isHighSurrogate(nEncoded)
                                    && !INetMIME::isLowSurrogate(nEncoded)
                                    && nEncoded <= 0x10FFFF)
                                {
                                    rBegin = p;
                                    nUTF32 = nEncoded;
                                    rEscapeType = ESCAPE_UTF32;
                                    break;
                                }
                            }
                            rEscapeType = ESCAPE_OCTET;
                        }
                        break;
                }
            }
            else
                rEscapeType = ESCAPE_NO;
            break;
        }

        case NOT_CANONIC:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == cEscapePrefix && rBegin + 1 < pEnd
                && ((nWeight1 = INetMIME::getHexWeight(rBegin[0])) >= 0)
                && ((nWeight2 = INetMIME::getHexWeight(rBegin[1])) >= 0))
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                rEscapeType = ESCAPE_OCTET;
            }
            else
                rEscapeType = ESCAPE_NO;
            break;
        }
    }
    return nUTF32;
}

//============================================================================
// static
sal_uInt32 INetURLObject::scanDomain(sal_Unicode const *& rBegin,
                                     sal_Unicode const * pEnd,
                                     bool bEager)
{
    enum State { STATE_DOT, STATE_LABEL, STATE_HYPHEN };
    State eState = STATE_DOT;
    xub_StrLen nLabels = 0;
    sal_Unicode const * pLastAlphanumeric = 0;
    for (sal_Unicode const * p = rBegin;; ++p)
        switch (eState)
        {
            case STATE_DOT:
                if (p != pEnd && INetMIME::isAlphanumeric(*p))
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
                    if (INetMIME::isAlphanumeric(*p))
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
                rBegin = p;
                return nLabels;

            case STATE_HYPHEN:
                if (p != pEnd)
                    if (INetMIME::isAlphanumeric(*p))
                    {
                        eState = STATE_LABEL;
                        break;
                    }
                    else if (*p == '-')
                        break;
                if (bEager)
                    return 0;
                rBegin = pLastAlphanumeric;
                return nLabels;
        }
}

//============================================================================
// static
bool INetURLObject::scanIPv6reference(sal_Unicode const *& rBegin,
                                      sal_Unicode const * pEnd,
                                      bool bEager)
{
    return false; //@@@
}

//============================================================================
// static
UniString INetURLObject::RelToAbs(ByteString const & rTheRelURIRef,
                                  bool bIgnoreFragment,
                                  EncodeMechanism eEncodeMechanism,
                                  DecodeMechanism eDecodeMechanism,
                                  rtl_TextEncoding eCharset,
                                  FSysStyle eStyle)
{
    // Backwards compatibility:
    if (rTheRelURIRef.Len() == 0 || rTheRelURIRef.GetChar(0) == '#')
        return extend(rTheRelURIRef);

    INetURLObject aTheAbsURIRef;
    bool bWasAbsolute;
    m_aBaseURIRef.convertRelToAbs(extend(rTheRelURIRef), true, aTheAbsURIRef,
                                  bWasAbsolute, eEncodeMechanism, eCharset,
                                  bIgnoreFragment, false, false, eStyle);
    return aTheAbsURIRef.GetMainURL(eDecodeMechanism, eCharset);
}

//============================================================================
// static
UniString INetURLObject::RelToAbs(UniString const & rTheRelURIRef,
                                  bool bIgnoreFragment,
                                  EncodeMechanism eEncodeMechanism,
                                  DecodeMechanism eDecodeMechanism,
                                  rtl_TextEncoding eCharset,
                                  FSysStyle eStyle)
{
    // Backwards compatibility:
    if (rTheRelURIRef.Len() == 0 || rTheRelURIRef.GetChar(0) == '#')
        return rTheRelURIRef;

    INetURLObject aTheAbsURIRef;
    bool bWasAbsolute;
    return m_aBaseURIRef.convertRelToAbs(rTheRelURIRef, false, aTheAbsURIRef,
                                         bWasAbsolute, eEncodeMechanism,
                                         eCharset, bIgnoreFragment, false,
                                         false, eStyle)
           || eEncodeMechanism != WAS_ENCODED
           || eDecodeMechanism != DECODE_TO_IURI
           || eCharset != RTL_TEXTENCODING_UTF8 ?
               aTheAbsURIRef.GetMainURL(eDecodeMechanism, eCharset) :
               rTheRelURIRef;
}

//============================================================================
// static
UniString INetURLObject::AbsToRel(ByteString const & rTheAbsURIRef,
                                  EncodeMechanism eEncodeMechanism,
                                  DecodeMechanism eDecodeMechanism,
                                  rtl_TextEncoding eCharset,
                                  FSysStyle eStyle)
{
    UniString aTheRelURIRef;
    m_aBaseURIRef.convertAbsToRel(extend(rTheAbsURIRef), true, aTheRelURIRef,
                                  eEncodeMechanism, eDecodeMechanism,
                                  eCharset, eStyle);
    return aTheRelURIRef;
}

//============================================================================
// static
UniString INetURLObject::AbsToRel(UniString const & rTheAbsURIRef,
                                  EncodeMechanism eEncodeMechanism,
                                  DecodeMechanism eDecodeMechanism,
                                  rtl_TextEncoding eCharset,
                                  FSysStyle eStyle)
{
    UniString aTheRelURIRef;
    m_aBaseURIRef.convertAbsToRel(rTheAbsURIRef, false, aTheRelURIRef,
                                  eEncodeMechanism, eDecodeMechanism,
                                  eCharset, eStyle);
    return aTheRelURIRef;
}

//============================================================================
// static
bool INetURLObject::SetBaseURL(ByteString const & rTheBaseURIRef,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset)
{
    return m_aBaseURIRef.SetURL(rTheBaseURIRef, eMechanism, eCharset);
}

//============================================================================
// static
bool INetURLObject::SetBaseURL(UniString const & rTheBaseURIRef,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset)
{
    return m_aBaseURIRef.SetURL(rTheBaseURIRef, eMechanism, eCharset);
}

//============================================================================
// static
UniString INetURLObject::GetBaseURL(DecodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset)
{
    return m_aBaseURIRef.GetMainURL(eMechanism, eCharset);
}

//============================================================================
UniString INetURLObject::GetPartBeforeLastName(DecodeMechanism eMechanism,
                                               rtl_TextEncoding eCharset)
    const
{
    if (!getSchemeInfo().m_bHierarchical)
        return UniString();
    INetURLObject aTemp(*this);
    aTemp.clearFragment();
    aTemp.clearQuery();
    aTemp.removeSegment(LAST_SEGMENT, false);
    aTemp.setFinalSlash();
    return aTemp.GetMainURL(eMechanism, eCharset);
}

//============================================================================
UniString INetURLObject::GetLastName(DecodeMechanism eMechanism,
                                     rtl_TextEncoding eCharset) const
{
    return getName(LAST_SEGMENT, true, eMechanism, eCharset);
}

//============================================================================
UniString INetURLObject::GetFileExtension(DecodeMechanism eMechanism,
                                          rtl_TextEncoding eCharset) const
{
    return getExtension(LAST_SEGMENT, false, eMechanism, eCharset);
}

//============================================================================
bool INetURLObject::CutLastName()
{
    INetURLObject aTemp(*this);
    aTemp.clearFragment();
    aTemp.clearQuery();
    if (!aTemp.removeSegment(LAST_SEGMENT, false))
        return false;
    *this = aTemp;
    return true;
}

//============================================================================
UniString INetURLObject::PathToFileName() const
{
    if (m_eScheme != INET_PROT_FILE)
        return UniString();
    rtl::OUString aNormalizedPath;
    if (osl::FileBase::getNormalizedPathFromFileURL(
                decode(m_aAbsURIRef.GetBuffer(),
                       m_aAbsURIRef.GetBuffer() + m_aPath.getEnd(),
                       getEscapePrefix(), NO_DECODE, RTL_TEXTENCODING_UTF8),
                aNormalizedPath)
            != osl::FileBase::E_None)
        return UniString();
    rtl::OUString aSystemPath;
    if (osl::FileBase::getSystemPathFromNormalizedPath(aNormalizedPath,
                                                       aSystemPath)
            != osl::FileBase::E_None)
        return UniString();
    return aSystemPath;
}

//============================================================================
UniString INetURLObject::GetFull() const
{
    INetURLObject aTemp(*this);
    aTemp.removeFinalSlash();
    return aTemp.PathToFileName();
}

//============================================================================
UniString INetURLObject::GetPath() const
{
    INetURLObject aTemp(*this);
    aTemp.removeSegment(LAST_SEGMENT, true);
    aTemp.removeFinalSlash();
    return aTemp.PathToFileName();
}

//============================================================================
void INetURLObject::SetBase(UniString const & rTheBase)
{
    setBase(rTheBase, LAST_SEGMENT, true, ENCODE_ALL);
}

//============================================================================
UniString INetURLObject::GetBase() const
{
    return getBase(LAST_SEGMENT, true, DECODE_WITH_CHARSET);
}

//============================================================================
void INetURLObject::SetName(UniString const & rTheName,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset)
{
    INetURLObject aTemp(*this);
    if (aTemp.removeSegment(LAST_SEGMENT, true)
        && aTemp.insertName(rTheName, false, LAST_SEGMENT, true, eMechanism,
                            eCharset))
        *this = aTemp;
}

//============================================================================
UniString INetURLObject::CutName(DecodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
{
    UniString aTheName(getName(LAST_SEGMENT, true, eMechanism, eCharset));
    return removeSegment(LAST_SEGMENT, true) ? aTheName : UniString();
}

//============================================================================
void INetURLObject::SetExtension(UniString const & rTheExtension,
                                 EncodeMechanism eMechanism,
                                 rtl_TextEncoding eCharset)
{
    setExtension(rTheExtension, LAST_SEGMENT, false, eMechanism, eCharset);
}

//============================================================================
UniString INetURLObject::CutExtension(DecodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset)
{
    UniString aTheExtension(getExtension(LAST_SEGMENT, false, eMechanism,
                                         eCharset));
    return removeExtension(LAST_SEGMENT, false) ? aTheExtension : UniString();
}

//============================================================================
bool INetURLObject::IsCaseSensitive() const
{
    return true;
}
