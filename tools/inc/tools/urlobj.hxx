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
#ifndef _URLOBJ_HXX
#define _URLOBJ_HXX

#include "tools/toolsdllapi.h"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/string.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/textenc.h"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace util {
    class XStringWidth;
} } } }

//============================================================================
// Special tokens:
#define INET_PATH_TOKEN '/'
#define INET_MARK_TOKEN '#'
#define INET_HEX_ESCAPE '%'

//============================================================================
// Common URL prefixes for various schemes:
#define INET_FTP_SCHEME "ftp://"
#define INET_HTTP_SCHEME "http://"
#define INET_HTTPS_SCHEME "https://"
#define INET_FILE_SCHEME "file://"
#define INET_MAILTO_SCHEME "mailto:"
#define INET_NEWS_SCHEME "news:"
#define INET_HID_SCHEME "hid:"

#define URL_PREFIX_PRIV_SOFFICE "private:"
enum
{
    URL_PREFIX_PRIV_SOFFICE_LEN
        = RTL_CONSTASCII_LENGTH(URL_PREFIX_PRIV_SOFFICE)
};

#define URL_PREFIX_PRIV_OBSOLETE URL_PREFIX_PRIV_SOFFICE
enum
{
    URL_PREFIX_PRIV_OBSOLETE_LEN
        = RTL_CONSTASCII_LENGTH(URL_PREFIX_PRIV_OBSOLETE)
};

#define URL_PREFIX_PRIV_EXTERN "staroffice:"
enum
{
    URL_PREFIX_PRIV_EXTERN_LEN = RTL_CONSTASCII_LENGTH(URL_PREFIX_PRIV_EXTERN)
};

//============================================================================
// Schemes:
enum INetProtocol
{
    INET_PROT_NOT_VALID = 0,
    INET_PROT_FTP = 1,
    INET_PROT_HTTP = 2,
    INET_PROT_FILE = 3,
    INET_PROT_MAILTO = 4,
    INET_PROT_VND_SUN_STAR_WEBDAV = 5,
    INET_PROT_NEWS = 6,
    INET_PROT_PRIV_SOFFICE = 7,
    INET_PROT_PRIVATE = INET_PROT_PRIV_SOFFICE, // obsolete
    INET_PROT_VND_SUN_STAR_HELP = 8,
    INET_PROT_HTTPS = 9,
    INET_PROT_SLOT = 10,
    INET_PROT_MACRO = 11,
    INET_PROT_JAVASCRIPT = 12,
    INET_PROT_IMAP = 13,
    INET_PROT_POP3 = 14,
    INET_PROT_DATA = 15,
    INET_PROT_CID = 16,
    INET_PROT_OUT = 17,
    INET_PROT_VND_SUN_STAR_HIER = 18,
    INET_PROT_VIM = 19,
    INET_PROT_UNO = 20,
    INET_PROT_COMPONENT = 21,
    INET_PROT_VND_SUN_STAR_PKG = 22,
    INET_PROT_LDAP = 23,
    INET_PROT_DB = 24,
    INET_PROT_VND_SUN_STAR_CMD = 25,
    INET_PROT_VND_SUN_STAR_ODMA = 26,
    INET_PROT_TELNET = 27,
    INET_PROT_VND_SUN_STAR_EXPAND = 28,
    INET_PROT_VND_SUN_STAR_TDOC = 29,
    INET_PROT_GENERIC = 30,
    INET_PROT_SMB = 31,
    INET_PROT_HID = 32,
    INET_PROT_SFTP = 33,
    INET_PROT_CMIS_ATOM = 34,
    INET_PROT_END = 35
};

//============================================================================
class TOOLS_DLLPUBLIC INetURLObject
{
public:
    //========================================================================
    // Get- and Set-Methods:

    /** The way input strings that represent (parts of) URIs are interpreted
        in set-methods.

        @descr  Most set-methods accept either a rtl::OString or a rtl::OUString
        as input.  Using a rtl::OString, octets in the range 0x80--0xFF are
        replaced by single escape sequences.  Using a rtl::OUString , UTF-32
        characters in the range 0x80--0x10FFFF are replaced by sequences of
        escape sequences, representing the UTF-8 coded characters.

        @descr  Along with an EncodeMechanism parameter, the set-methods all
        take an rtl_TextEncoding parameter, which is ignored unless the
        EncodeMechanism is WAS_ENCODED.
     */
    enum EncodeMechanism
    {
        /** All escape sequences that are already present are ignored, and are
            interpreted as literal sequences of three characters.
         */
        ENCODE_ALL,

        /** Sequences of escape sequences, that represent characters from the
            specified character set and that can be converted to UTF-32
            characters, are first decoded.  If they have to be encoded, they
            are converted to UTF-8 characters and are than translated into
            (sequences of) escape sequences.  Other escape sequences are
            copied verbatim (but using upper case hex digits).
         */
        WAS_ENCODED,

        /** All escape sequences that are already present are copied verbatim
            (but using upper case hex digits).
         */
        NOT_CANONIC
    };

    /** The way strings that represent (parts of) URIs are returned from get-
        methods.

        @descr  Along with a DecodeMechanism parameter, the get-methods all
        take an rtl_TextEncoding parameter, which is ignored unless the
        DecodeMechanism is DECODE_WITH_CHARSET or DECODE_UNAMBIGUOUS.
     */
    enum DecodeMechanism
    {
        /** The (part of the) URI is returned unchanged.  Since URIs are
            written using a subset of US-ASCII, the returned string is
            guaranteed to contain only US-ASCII characters.
         */
        NO_DECODE,

        /** All sequences of escape sequences that represent UTF-8 coded
            UTF-32 characters with a numerical value greater than 0x7F, are
            replaced by the respective UTF-16 characters.  All other escape
            sequences are not decoded.
         */
        DECODE_TO_IURI,

        /** All (sequences of) escape sequences that represent characters from
            the specified character set, and that can be converted to UTF-32,
            are replaced by the respective UTF-16 characters.  All other
            escape sequences are not decoded.
         */
        DECODE_WITH_CHARSET,

        /** All (sequences of) escape sequences that represent characters from
            the specified character set, that can be converted to UTF-32, and
            that (in the case of ASCII characters) can safely be decoded
            without altering the meaning of the (part of the) URI, are
            replaced by the respective UTF-16 characters.  All other escape
            sequences are not decoded.
         */
        DECODE_UNAMBIGUOUS
    };

    //========================================================================
    // General Structure:

    inline INetURLObject():
        m_eScheme(INET_PROT_NOT_VALID), m_eSmartScheme(INET_PROT_HTTP) {}

    inline bool HasError() const { return m_eScheme == INET_PROT_NOT_VALID; }

    inline rtl::OUString GetMainURL(DecodeMechanism eMechanism,
                                rtl_TextEncoding eCharset
                                    = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aAbsURIRef, getEscapePrefix(), eMechanism, eCharset); }

    rtl::OUString GetURLNoPass(DecodeMechanism eMechanism = DECODE_TO_IURI,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    rtl::OUString GetURLNoMark(DecodeMechanism eMechanism = DECODE_TO_IURI,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    rtl::OUString
    getAbbreviated(com::sun::star::uno::Reference<
                           com::sun::star::util::XStringWidth > const &
                       rStringWidth,
                   sal_Int32 nWidth,
                   DecodeMechanism eMechanism = DECODE_TO_IURI,
                   rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    bool operator ==(INetURLObject const & rObject) const;

    inline bool operator !=(INetURLObject const & rObject) const
    { return !(*this == rObject); }

    bool operator <(INetURLObject const & rObject) const;

    inline bool operator >(INetURLObject const & rObject) const
    { return rObject < *this; }

    //========================================================================
    // Strict Parsing:

    inline INetURLObject(const rtl::OString& rTheAbsURIRef,
                         EncodeMechanism eMechanism = WAS_ENCODED,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline INetURLObject(rtl::OUString const & rTheAbsURIRef,
                         EncodeMechanism eMechanism = WAS_ENCODED,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetURL(const rtl::OString& rTheAbsURIRef,
                       EncodeMechanism eMechanism = WAS_ENCODED,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetURL(rtl::OUString const & rTheAbsURIRef,
                       EncodeMechanism eMechanism = WAS_ENCODED,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    bool ConcatData(INetProtocol eTheScheme, rtl::OUString const & rTheUser,
                    rtl::OUString const & rThePassword,
                    rtl::OUString const & rTheHost, sal_uInt32 nThePort,
                    rtl::OUString const & rThePath,
                    EncodeMechanism eMechanism = WAS_ENCODED,
                    rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    //========================================================================
    // Smart Parsing:

    /** The supported notations for file system paths.
     */
    enum FSysStyle
    {
        /** VOS notation (e.g., "//server/dir/file").
         */
        FSYS_VOS = 0x1,

        /** Unix notation (e.g., "/dir/file").
         */
        FSYS_UNX = 0x2,

        /** DOS notation (e.g., "a:\dir\file" and "\\server\dir\file").
         */
        FSYS_DOS = 0x4,

        /** Mac notation (e.g., "dir:file").
         */
        FSYS_MAC = 0x8,

        /** Detect the used notation.

            @descr  For the following descriptions, please note that
            whereas FSYS_DEFAULT includes all style bits, combinations of only
            a few style bits are also possible, and are also described.

            @descr  When used to translate a file system path to a file URL,
            the subset of the following productions for which the appropriate
            style bit is set are checked in order (using the conventions of
            RFC 2234, RFC 2396, and RFC 2732; UCS4 stands for any UCS4
            character):

             Production T1 (VOS local; FSYS_VOS only):
                "//." ["/" *UCS4]
              becomes
                "file:///" *UCS4

             Production T2 (VOS host; FSYS_VOS only):
                "//" [host] ["/" *UCS4]
              becomes
                "file://" host "/" *UCS4

             Production T3 (UNC; FSYS_DOS only):
                "\\" [host] ["\" *UCS4]
              becomes
                "file://" host "/" *UCS4
              replacing "\" by "/" within <*UCS4>

             Production T4 (Unix-like DOS; FSYS_DOS only):
                ALPHA ":" ["/" *UCS4]
              becomes
                "file:///" ALPHA ":/" *UCS4
              replacing "\" by "/" within <*UCS4>

             Production T5 (DOS; FSYS_DOS only):
                ALPHA ":" ["\" *UCS4]
              becomes
                "file:///" ALPHA ":/" *UCS4
              replacing "\" by "/" within <*UCS4>

             Production T6 (any):
                *UCS4
              becomes
                "file:///" *UCS4
              replacing the delimiter by "/" within <*UCS4>.  The delimiter is
              that character from the set { "/", "\", ":" } which appears most
              often in <*UCS4> (if FSYS_UNX is not among the style bits, "/"
              is removed from the set; if FSYS_DOS is not among the style
              bits, "\" is removed from the set; if FSYS_MAC is not among the
              style bits, ":" is removed from the set).  If two or more
              characters appear the same number of times, the character
              mentioned first in that set is chosen.  If the first character
              of <*UCS4> is the delimiter, that character is not copied.

            @descr  When used to translate a file URL to a file system path,
            the following productions are checked in order (using the
            conventions of RFC 2234, RFC 2396, and RFC 2732):

             Production F1 (VOS; FSYS_VOS):
                "file://" host "/" fpath ["#" fragment]
              becomes
                "//" host "/" fpath

             Production F2 (DOS; FSYS_DOS):
                "file:///" ALPHA ":" ["/" fpath] ["#" fragment]
              becomes
                ALPHA ":" ["\" fpath]
              replacing "/" by "\" in <fpath>

             Production F3 (Unix; FSYS_UNX):
                "file:///" fpath ["#" fragment]
              becomes
                "/" fpath
         */
        FSYS_DETECT = FSYS_VOS | FSYS_UNX | FSYS_DOS
    };

    inline INetURLObject(rtl::OUString const & rTheAbsURIRef,
                         INetProtocol eTheSmartScheme,
                         EncodeMechanism eMechanism = WAS_ENCODED,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                         FSysStyle eStyle = FSYS_DETECT);

    inline void SetSmartProtocol(INetProtocol eTheSmartScheme)
    { m_eSmartScheme = eTheSmartScheme; }

    inline bool
    SetSmartURL(const rtl::OString& rTheAbsURIRef,
                EncodeMechanism eMechanism = WAS_ENCODED,
                rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                FSysStyle eStyle = FSYS_DETECT);

    inline bool
    SetSmartURL(rtl::OUString const & rTheAbsURIRef,
                EncodeMechanism eMechanism = WAS_ENCODED,
                rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                FSysStyle eStyle = FSYS_DETECT);

    inline INetURLObject
    smartRel2Abs(const rtl::OString& rTheRelURIRef,
                 bool & rWasAbsolute,
                 bool bIgnoreFragment = false,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                 bool bRelativeNonURIs = false,
                 FSysStyle eStyle = FSYS_DETECT) const;

    inline INetURLObject
    smartRel2Abs(rtl::OUString const & rTheRelURIRef,
                 bool & rWasAbsolute,
                 bool bIgnoreFragment = false,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                 bool bRelativeNonURIs = false,
                 FSysStyle eStyle = FSYS_DETECT) const;

    //========================================================================
    // Relative URLs:

    inline bool
    GetNewAbsURL(const rtl::OString& rTheRelURIRef,
                 INetURLObject * pTheAbsURIRef,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                 FSysStyle eStyle = FSYS_DETECT, bool bIgnoreFragment = false)
        const;

    inline bool
    GetNewAbsURL(rtl::OUString const & rTheRelURIRef,
                 INetURLObject * pTheAbsURIRef,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                 FSysStyle eStyle = FSYS_DETECT, bool bIgnoreFragment = false)
        const;

    /** @descr  If rTheRelURIRef cannot be converted to an absolute URL
        (because of syntactic reasons), either rTheRelURIRef or an empty
        string is returned:  If all of the parameters eEncodeMechanism,
        eDecodeMechanism and eCharset have their respective default values,
        then rTheRelURIRef is returned unmodified; otherwise, an empty string
        is returned.
     */
    static rtl::OUString
    GetAbsURL(rtl::OUString const & rTheBaseURIRef,
              rtl::OUString const & rTheRelURIRef,
              bool bIgnoreFragment = false,
              EncodeMechanism eEncodeMechanism = WAS_ENCODED,
              DecodeMechanism eDecodeMechanism = DECODE_TO_IURI,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
              FSysStyle eStyle = FSYS_DETECT);

    static inline rtl::OUString
    GetRelURL(const rtl::OString& rTheBaseURIRef,
              const rtl::OString& rTheAbsURIRef,
              EncodeMechanism eEncodeMechanism = WAS_ENCODED,
              DecodeMechanism eDecodeMechanism = DECODE_TO_IURI,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
              FSysStyle eStyle = FSYS_DETECT);

    static inline rtl::OUString
    GetRelURL(rtl::OUString const & rTheBaseURIRef,
              rtl::OUString const & rTheAbsURIRef,
              EncodeMechanism eEncodeMechanism = WAS_ENCODED,
              DecodeMechanism eDecodeMechanism = DECODE_TO_IURI,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
              FSysStyle eStyle = FSYS_DETECT);

    //========================================================================
    // External URLs:

    rtl::OUString getExternalURL(DecodeMechanism eMechanism = DECODE_TO_IURI,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const;

    static inline bool translateToExternal(const rtl::OString& rTheIntURIRef,
                                           rtl::OUString & rTheExtURIRef,
                                           DecodeMechanism eDecodeMechanism
                                               = DECODE_TO_IURI,
                                           rtl_TextEncoding eCharset
                                               = RTL_TEXTENCODING_UTF8);

    static inline bool translateToExternal(rtl::OUString const & rTheIntURIRef,
                                           rtl::OUString & rTheExtURIRef,
                                           DecodeMechanism eDecodeMechanism
                                               = DECODE_TO_IURI,
                                           rtl_TextEncoding eCharset
                                               = RTL_TEXTENCODING_UTF8);

    static inline bool translateToInternal(const rtl::OString& rTheExtURIRef,
                                           rtl::OUString & rTheIntURIRef,
                                           DecodeMechanism eDecodeMechanism
                                               = DECODE_TO_IURI,
                                           rtl_TextEncoding eCharset
                                               = RTL_TEXTENCODING_UTF8);

    static inline bool translateToInternal(rtl::OUString const & rTheExtURIRef,
                                           rtl::OUString & rTheIntURIRef,
                                           DecodeMechanism eDecodeMechanism
                                               = DECODE_TO_IURI,
                                           rtl_TextEncoding eCharset
                                               = RTL_TEXTENCODING_UTF8);

    //========================================================================
    // Scheme:

    struct SchemeInfo;

    inline INetProtocol GetProtocol() const { return m_eScheme; }

    /** Return the URL 'prefix' for a given scheme.

        @param eTheScheme  One of the supported URL schemes.

        @return  The 'prefix' of URLs of the given scheme.
     */
    static rtl::OUString GetScheme(INetProtocol eTheScheme);

    static inline INetProtocol CompareProtocolScheme(const rtl::OString&
                                                         rTheAbsURIRef)
    { return CompareProtocolScheme(extend(rTheAbsURIRef)); }

    static INetProtocol CompareProtocolScheme(rtl::OUString const &
                                                  rTheAbsURIRef);

    //========================================================================
    // User Info:

    inline bool HasUserData() const { return m_aUser.isPresent(); }

    inline bool IsEmptyUser() const
    { return m_aUser.isPresent() && m_aUser.isEmpty(); }

    inline rtl::OUString GetUser(DecodeMechanism eMechanism = DECODE_TO_IURI,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aUser, getEscapePrefix(), eMechanism, eCharset); }

    inline rtl::OUString GetPass(DecodeMechanism eMechanism = DECODE_TO_IURI,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aAuth, getEscapePrefix(), eMechanism, eCharset); }

    inline bool SetUser(const rtl::OString& rTheUser,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setUser(extend(rTheUser), true, eMechanism, eCharset); }

    inline bool SetUser(rtl::OUString const & rTheUser,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setUser(rTheUser, false, eMechanism, eCharset); }

    inline bool SetPass(const rtl::OString& rThePassword,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetPass(rtl::OUString const & rThePassword,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetUserAndPass(const rtl::OString& rTheUser,
                               const rtl::OString& rThePassword,
                               EncodeMechanism eMechanism = WAS_ENCODED,
                               rtl_TextEncoding eCharset
                                   = RTL_TEXTENCODING_UTF8);

    inline bool SetUserAndPass(rtl::OUString const & rTheUser,
                               rtl::OUString const & rThePassword,
                               EncodeMechanism eMechanism = WAS_ENCODED,
                               rtl_TextEncoding eCharset
                                   = RTL_TEXTENCODING_UTF8);

    //========================================================================
    // Host and Port:

    inline bool HasPort() const { return m_aPort.isPresent(); }

    inline rtl::OUString GetHost(DecodeMechanism eMechanism = DECODE_TO_IURI,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aHost, getEscapePrefix(), eMechanism, eCharset); }

    rtl::OUString GetHostPort(DecodeMechanism eMechanism = DECODE_TO_IURI,
                          rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    sal_uInt32 GetPort() const;

    inline bool SetHost(const rtl::OString& rTheHost,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setHost(extend(rTheHost), true, eMechanism, eCharset); }

    inline bool SetHost(rtl::OUString const & rTheHost,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setHost(rTheHost, false, eMechanism, eCharset); }

    bool SetPort(sal_uInt32 nThePort);

    //========================================================================
    // Path:

    inline bool HasURLPath() const { return !m_aPath.isEmpty(); }

    inline rtl::OUString GetURLPath(DecodeMechanism eMechanism = DECODE_TO_IURI,
                                rtl_TextEncoding eCharset
                                    = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aPath, getEscapePrefix(), eMechanism, eCharset); }

    inline bool SetURLPath(const rtl::OString& rThePath,
                           EncodeMechanism eMechanism = WAS_ENCODED,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setPath(extend(rThePath), true, eMechanism, eCharset); }

    inline bool SetURLPath(rtl::OUString const & rThePath,
                           EncodeMechanism eMechanism = WAS_ENCODED,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setPath(rThePath, false, eMechanism, eCharset); }

    //========================================================================
    // Hierarchical Path:

    /** A constant to address the last segment in various methods dealing with
        hierarchical paths.

        @descr  It is often more efficient to address the last segment using
        this constant, than to determine its ordinal value using
        getSegmentCount().
     */
    enum { LAST_SEGMENT = -1 };

    /** The number of segments in the hierarchical path.

        @descr  Using RFC 2396 and RFC 2234, a hierarchical path is of the
        form

          hierarchical-path = 1*("/" segment)

          segment = name *(";" param)

          name = [base ["." extension]]

          base = 1*pchar

          extension = *<any pchar except ".">

          param = *pchar

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @return  The number of segments in the hierarchical path.  If the path
        is not hierarchical, 0 is returned.
     */
    sal_Int32 getSegmentCount(bool bIgnoreFinalSlash = true) const;

    /** Remove a segment from the hierarchical path.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @return  True if the segment has successfully been removed (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool removeSegment(sal_Int32 nIndex = LAST_SEGMENT,
                       bool bIgnoreFinalSlash = true);

    /** Insert a new segment into the hierarchical path.

        @param rTheName  The name part of the new segment.  The new segment
        will contain no parameters.

        @param bAppendFinalSlash  If the new segment is appended at the end of
        the hierarchical path, this parameter specifies whether to add a final
        slash after it or not.

        @param nIndex  The non-negative index of the segment before which
        to insert the new segment.  LAST_SEGMENT or an nIndex that equals
        getSegmentCount() inserts the new segment at the end of the
        hierarchical path.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the segment has successfully been inserted (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified place to insert the new segment does not exist, false is
        returned.  If false is returned, the object is not modified.
     */
    inline bool insertName(rtl::OUString const & rTheName,
                           bool bAppendFinalSlash = false,
                           sal_Int32 nIndex = LAST_SEGMENT,
                           bool bIgnoreFinalSlash = true,
                           EncodeMechanism eMechanism = WAS_ENCODED,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    /** Get the name of a segment of the hierarchical path.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  The name part of the specified segment.  If the path is not
        hierarchical, or the specified segment does not exits, an empty string
        is returned.
     */
    rtl::OUString getName(sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true,
                      DecodeMechanism eMechanism = DECODE_TO_IURI,
                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Set the name of a segment (preserving any parameters and any query or
        fragment part).

        @param rTheName  The new name.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the name has successfully been modified (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool setName(rtl::OUString const & rTheName,
                 sal_Int32 nIndex = LAST_SEGMENT,
                 bool bIgnoreFinalSlash = true,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    /** Get the base of the name of a segment.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  The base part of the specified segment.  If the path is
        not hierarchical, or the specified segment does not exits, an empty
        string is returned.
     */
    rtl::OUString getBase(sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true,
                      DecodeMechanism eMechanism = DECODE_TO_IURI,
                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Set the base of the name of a segment (preserving the extension).

        @param rTheBase  The new base.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the base has successfully been modified (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool setBase(rtl::OUString const & rTheBase,
                 sal_Int32 nIndex = LAST_SEGMENT,
                 bool bIgnoreFinalSlash = true,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    /** Determine whether the name of a segment has an extension.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @return  True if the name of the specified segment has an extension.
        If the path is not hierarchical, or the specified segment does not
        exist, false is returned.
     */
    bool hasExtension(sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true) const;

    /** Get the extension of the name of a segment.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  The extension part of the specified segment.  If the path is
        not hierarchical, or the specified segment does not exits, an empty
        string is returned.
     */
    rtl::OUString getExtension(sal_Int32 nIndex = LAST_SEGMENT,
                           bool bIgnoreFinalSlash = true,
                           DecodeMechanism eMechanism = DECODE_TO_IURI,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Set the extension of the name of a segment (replacing an already
        existing extension).

        @param rTheExtension  The new extension.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the extension has successfully been modified (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool setExtension(rtl::OUString const & rTheExtension,
                      sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true,
                      EncodeMechanism eMechanism = WAS_ENCODED,
                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    /** Remove the extension of the name of a segment.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @return  True if the extension has successfully been removed (and the
        resulting URI is still valid), or if the name did not have an
        extension.  If the path is not hierarchical, or the specified segment
        does not exist, false is returned.  If false is returned, the object
        is not modified.
     */
    bool removeExtension(sal_Int32 nIndex = LAST_SEGMENT,
                         bool bIgnoreFinalSlash = true);

    /** Determine whether the hierarchical path ends in a final slash.

        @return  True if the hierarchical path ends in a final slash.  If the
        path is not hierarchical, false is returned.
     */
    bool hasFinalSlash() const;

    /** Make the hierarchical path end in a final slash (if it does not
        already do so).

        @return  True if a final slash has successfully been appended (and the
        resulting URI is still valid), or if the hierarchical path already
        ended in a final slash.  If the path is not hierarchical, false is
        returned.  If false is returned, the object is not modified.
     */
    bool setFinalSlash();

    /** Remove a final slash from the hierarchical path.

        @return  True if a final slash has successfully been removed (and the
        resulting URI is still valid), or if the hierarchical path already did
        not end in a final slash.  If the path is not hierarchical, false is
        returned.  If false is returned, the object is not modified.
     */
    bool removeFinalSlash();

    //========================================================================
    // Query:

    inline bool HasParam() const { return m_aQuery.isPresent(); }

    inline rtl::OUString GetParam(DecodeMechanism eMechanism = DECODE_TO_IURI,
                              rtl_TextEncoding eCharset
                                  = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aQuery, getEscapePrefix(), eMechanism, eCharset); }

    inline bool SetParam(const rtl::OString& rTheQuery,
                         EncodeMechanism eMechanism = WAS_ENCODED,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetParam(rtl::OUString const & rTheQuery,
                         EncodeMechanism eMechanism = WAS_ENCODED,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    //========================================================================
    // Fragment:

    inline bool HasMark() const { return m_aFragment.isPresent(); }

    inline rtl::OUString GetMark(DecodeMechanism eMechanism = DECODE_TO_IURI,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aFragment, getEscapePrefix(), eMechanism, eCharset); }

    inline bool SetMark(const rtl::OString& rTheFragment,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetMark(rtl::OUString const & rTheFragment,
                        EncodeMechanism eMechanism = WAS_ENCODED,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    //========================================================================
    // File URLs:

    /** Create an INetURLObject from a file system path.

        @param rFSysPath  A file system path.  An URL is not allowed here!

        @param eStyle  The notation of rFSysPath.
     */
    inline INetURLObject(rtl::OUString const & rFSysPath, FSysStyle eStyle);

    /** Set this INetURLObject to a file URL constructed from a file system
        path.

        @param rFSysPath  A file system path.  An URL is not allowed here!

        @param eStyle  The notation of rFSysPath.

        @return  True if this INetURLObject has successfully been changed.  If
        false is returned, this INetURLObject has not been modified.
     */
    bool setFSysPath(rtl::OUString const & rFSysPath, FSysStyle eStyle);

    /** Return the file system path represented by a file URL (ignoring any
        fragment part).

        @param eStyle  The notation of the returned file system path.

        @param pDelimiter  Upon successful return, this parameter can return
        the character that is the 'main' delimiter within the returned file
        system path (e.g., "/" for Unix, "\" for DOS, ":" for Mac).  This is
        especially useful for routines that later try to shorten the returned
        file system path at a 'good' position, e.g. to fit it into some
        limited display space.

        @return  The file system path represented by this file URL.  If this
        file URL does not represent a file system path according to the
        specified notation, or if this is not a file URL at all, an empty
        string is returned.
     */
    rtl::OUString getFSysPath(FSysStyle eStyle, sal_Unicode * pDelimiter = 0)
        const;

    //========================================================================
    // POP3 and URLs:

    rtl::OUString GetMsgId(DecodeMechanism eMechanism = DECODE_TO_IURI,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    //========================================================================
    // Coding:

    enum Part
    {
        PART_OBSOLETE_NORMAL = 0x001, // Obsolete, do not use!
        PART_OBSOLETE_FILE = 0x002, // Obsolete, do not use!
        PART_OBSOLETE_PARAM = 0x004, // Obsolete, do not use!
        PART_USER_PASSWORD = 0x008,
        PART_IMAP_ACHAR = 0x010,
        PART_VIM = 0x020,
        PART_HOST_EXTRA = 0x040,
        PART_FPATH = 0x080,
        PART_AUTHORITY = 0x100,
        PART_PATH_SEGMENTS_EXTRA = 0x200,
        PART_REL_SEGMENT_EXTRA = 0x400,
        PART_URIC = 0x800,
        PART_HTTP_PATH = 0x1000,
        PART_FILE_SEGMENT_EXTRA = 0x2000, // Obsolete, do not use!
        PART_MESSAGE_ID = 0x4000,
        PART_MESSAGE_ID_PATH = 0x8000,
        PART_MAILTO = 0x10000,
        PART_PATH_BEFORE_QUERY = 0x20000,
        PART_PCHAR = 0x40000,
        PART_FRAGMENT = 0x80000, // Obsolete, do not use!
        PART_VISIBLE = 0x100000,
        PART_VISIBLE_NONSPECIAL = 0x200000,
        PART_CREATEFRAGMENT = 0x400000,
        PART_UNO_PARAM_VALUE = 0x800000,
        PART_UNAMBIGUOUS = 0x1000000,
        PART_URIC_NO_SLASH = 0x2000000,
        PART_HTTP_QUERY = 0x4000000, //TODO! unused?
        PART_NEWS_ARTICLE_LOCALPART = 0x8000000,
        max_part = 0x80000000
            // Do not use!  Only there to allow compatible changes in the
            // future.
    };

    enum EscapeType
    {
        ESCAPE_NO,
        ESCAPE_OCTET,
        ESCAPE_UTF32
    };

    /** Encode some text as part of a URI.

        @param rText  Some text (for its interpretation, see the general
        discussion for set-methods).

        @param ePart  The part says which characters are 'forbidden' and must
        be encoded (replaced by escape sequences).  Characters outside the US-
        ASCII range are always 'forbidden.'

        @param cEscapePrefix  The first character in an escape sequence
        (normally '%').

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  The encoded representation of the text ('forbidden'
        characters replaced by escape sequences).
     */
    static inline rtl::OUString encode(const rtl::OString& rText, Part ePart,
                                   sal_Char cEscapePrefix,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    /** Encode some text as part of a URI.

        @param rText  Some text (for its interpretation, see the general
        discussion for set-methods).

        @param ePart  The part says which characters are 'forbidden' and must
        be encoded (replaced by escape sequences).  Characters outside the US-
        ASCII range are always 'forbidden.'

        @param cEscapePrefix  The first character in an escape sequence
        (normally '%').

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  The text, encoded according to the given mechanism and
        charset ('forbidden' characters replaced by escape sequences).
     */
    static inline rtl::OUString encode(rtl::OUString const & rText, Part ePart,
                                   sal_Char cEscapePrefix,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    /** Decode some text.

        @param rText  Some (encoded) text.

        @param cEscapePrefix  The first character in an escape sequence
        (normally '%').

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  The text, decoded according to the given mechanism and
        charset (escape sequences replaced by 'raw' characters).
     */
    static inline rtl::OUString decode(rtl::OUString const & rText,
                                   sal_Char cEscapePrefix,
                                   DecodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    static inline rtl::OUString decode(rtl::OUStringBuffer const & rText,
                                   sal_Char cEscapePrefix,
                                   DecodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    static void appendUCS4Escape(rtl::OUStringBuffer & rTheText,
                                 sal_Char cEscapePrefix,
                                 sal_uInt32 nUCS4);

    static void appendUCS4(rtl::OUStringBuffer & rTheText, sal_uInt32 nUCS4,
                           EscapeType eEscapeType, bool bOctets, Part ePart,
                           sal_Char cEscapePrefix, rtl_TextEncoding eCharset,
                           bool bKeepVisibleEscapes);

    static sal_uInt32 getUTF32(sal_Unicode const *& rBegin,
                               sal_Unicode const * pEnd, bool bOctets,
                               sal_Char cEscapePrefix,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset,
                               EscapeType & rEscapeType);

    //========================================================================
    // Specialized helpers:

    static sal_uInt32 scanDomain(sal_Unicode const *& rBegin,
                                 sal_Unicode const * pEnd,
                                 bool bEager = true);

    //========================================================================
    // OBSOLETE Hierarchical Path:

    rtl::OUString GetPartBeforeLastName(DecodeMechanism eMechanism
                                        = DECODE_TO_IURI,
                                    rtl_TextEncoding eCharset
                                        = RTL_TEXTENCODING_UTF8) const;

    /** Get the last segment in the path.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  For a hierarchical URL, the last segment (everything after
        the last unencoded '/').  Not that this last segment may be empty.  If
        the URL is not hierarchical, an empty string is returned.
     */
    rtl::OUString GetLastName(DecodeMechanism eMechanism = DECODE_TO_IURI,
                          rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Get the 'extension' of the last segment in the path.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  For a hierarchical URL, everything after the first unencoded
        '.' in the last segment of the path.  Note that this 'extension' may
        be empty.  If the URL is not hierarchical, or if the last segment does
        not contain an unencoded '.', an empty string is returned.
     */
    rtl::OUString GetFileExtension(DecodeMechanism eMechanism = DECODE_TO_IURI,
                               rtl_TextEncoding eCharset
                                   = RTL_TEXTENCODING_UTF8) const;

    inline bool Append(const rtl::OString& rTheSegment,
                       EncodeMechanism eMechanism = WAS_ENCODED,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return appendSegment(extend(rTheSegment), true, eMechanism, eCharset); }

    inline bool Append(rtl::OUString const & rTheSegment,
                       EncodeMechanism eMechanism = WAS_ENCODED,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return appendSegment(rTheSegment, false, eMechanism, eCharset); }

    bool CutLastName();

    //========================================================================
    // OBSOLETE File URLs:

    rtl::OUString PathToFileName() const;

    rtl::OUString GetFull() const;

    rtl::OUString GetPath() const;

    void SetBase(rtl::OUString const & rTheBase);

    rtl::OUString GetBase() const;

    void SetName(rtl::OUString const & rTheName,
                 EncodeMechanism eMechanism = WAS_ENCODED,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline rtl::OUString GetName(DecodeMechanism eMechanism = DECODE_TO_IURI,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return GetLastName(eMechanism, eCharset); }

    void SetExtension(rtl::OUString const & rTheExtension,
                      EncodeMechanism eMechanism = WAS_ENCODED,
                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline rtl::OUString GetExtension(
                                  DecodeMechanism eMechanism = DECODE_TO_IURI,
                                  rtl_TextEncoding eCharset
                                      = RTL_TEXTENCODING_UTF8) const
    { return GetFileExtension(eMechanism, eCharset); }

    rtl::OUString CutExtension(DecodeMechanism eMechanism = DECODE_TO_IURI,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    bool IsCaseSensitive() const;

    //========================================================================

private:
    // General Structure:

    class SubString
    {
        sal_Int32 m_nBegin;
        sal_Int32 m_nLength;

    public:
        explicit inline SubString(sal_Int32 nTheBegin = -1,
                                  sal_Int32 nTheLength = 0):
            m_nBegin(nTheBegin), m_nLength(nTheLength) {}

        inline bool isPresent() const { return m_nBegin != -1; }

        inline bool isEmpty() const { return m_nLength == 0; }

        inline sal_Int32 getBegin() const { return m_nBegin; }

        inline sal_Int32 getLength() const { return m_nLength; }

        inline sal_Int32 getEnd() const { return m_nBegin + m_nLength; }

        inline sal_Int32 clear();

        inline sal_Int32 set(rtl::OUStringBuffer & rString,
                             rtl::OUString const & rSubString,
                             sal_Int32 nTheBegin);

        inline sal_Int32 set(rtl::OUString & rString,
                             rtl::OUString const & rSubString);

        inline sal_Int32 set(rtl::OUStringBuffer & rString,
                             rtl::OUString const & rSubString);

        inline void operator +=(sal_Int32 nDelta);

        int compare(SubString const & rOther,
            rtl::OUStringBuffer const & rThisString,
            rtl::OUStringBuffer const & rOtherString) const;
    };

    rtl::OUStringBuffer m_aAbsURIRef;
    SubString m_aScheme;
    SubString m_aUser;
    SubString m_aAuth;
    SubString m_aHost;
    SubString m_aPort;
    SubString m_aPath;
    SubString m_aQuery;
    SubString m_aFragment;
    INetProtocol m_eScheme;
    INetProtocol m_eSmartScheme;

    TOOLS_DLLPRIVATE void setInvalid();

    bool setAbsURIRef(
        rtl::OUString const & rTheAbsURIRef, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset, bool bSmart,
        FSysStyle eStyle);

    // Relative URLs:

    bool convertRelToAbs(
        rtl::OUString const & rTheRelURIRef, bool bOctets,
        INetURLObject & rTheAbsURIRef, bool & rWasAbsolute,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
        bool bIgnoreFragment, bool bSmart, bool bRelativeNonURIs,
        FSysStyle eStyle) const;

    bool convertAbsToRel(
        rtl::OUString const & rTheAbsURIRef, bool bOctets,
        rtl::OUString & rTheRelURIRef, EncodeMechanism eEncodeMechanism,
        DecodeMechanism eDecodeMechanism, rtl_TextEncoding eCharset,
        FSysStyle eStyle) const;

    // External URLs:

    static bool convertIntToExt(
        rtl::OUString const & rTheIntURIRef, bool bOctets,
        rtl::OUString & rTheExtURIRef, DecodeMechanism eDecodeMechanism,
        rtl_TextEncoding eCharset);

    static bool convertExtToInt(
        rtl::OUString const & rTheExtURIRef, bool bOctets,
        rtl::OUString & rTheIntURIRef, DecodeMechanism eDecodeMechanism,
        rtl_TextEncoding eCharset);

    // Scheme:

    struct PrefixInfo;

    TOOLS_DLLPRIVATE static inline SchemeInfo const & getSchemeInfo(
        INetProtocol eTheScheme);

    TOOLS_DLLPRIVATE inline SchemeInfo const & getSchemeInfo() const;

    TOOLS_DLLPRIVATE static PrefixInfo const * getPrefix(
        sal_Unicode const *& rBegin, sal_Unicode const * pEnd);

    // Authority:

    TOOLS_DLLPRIVATE sal_Int32 getAuthorityBegin() const;

    TOOLS_DLLPRIVATE SubString getAuthority() const;

    // User Info:

    bool setUser(
        rtl::OUString const & rTheUser, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    bool clearPassword();

    bool setPassword(
        rtl::OUString const & rThePassword, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // Host and Port:

    TOOLS_DLLPRIVATE static bool parseHost(
        sal_Unicode const *& rBegin, sal_Unicode const * pEnd,
        rtl::OUString & rCanonic);

    TOOLS_DLLPRIVATE static bool parseHostOrNetBiosName(
        sal_Unicode const * pBegin, sal_Unicode const * pEnd, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
        bool bNetBiosName, rtl::OUStringBuffer* pCanonic);

    bool setHost(
        rtl::OUString const & rTheHost, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // Path:

    TOOLS_DLLPRIVATE static bool parsePath(
        INetProtocol eScheme, sal_Unicode const ** pBegin,
        sal_Unicode const * pEnd, bool bOctets, EncodeMechanism eMechanism,
        rtl_TextEncoding eCharset, bool bSkippedInitialSlash,
        sal_uInt32 nSegmentDelimiter, sal_uInt32 nAltSegmentDelimiter,
        sal_uInt32 nQueryDelimiter, sal_uInt32 nFragmentDelimiter,
        rtl::OUStringBuffer &rSynPath);

    bool setPath(
        rtl::OUString const & rThePath, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // Hierarchical Path:

    TOOLS_DLLPRIVATE bool checkHierarchical() const;

    bool appendSegment(
        rtl::OUString const & rTheSegment, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    TOOLS_DLLPRIVATE SubString getSegment(
        sal_Int32 nIndex, bool bIgnoreFinalSlash) const;

    bool insertName(
        rtl::OUString const & rTheName, bool bOctets, bool bAppendFinalSlash,
        sal_Int32 nIndex, bool bIgnoreFinalSlash, EncodeMechanism eMechanism,
        rtl_TextEncoding eCharset);

    // Query:

    bool clearQuery();

    bool setQuery(
        rtl::OUString const & rTheQuery, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // Fragment:

    bool clearFragment();

    bool setFragment(
        rtl::OUString const & rTheMark, bool bOctets,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // FILE URLs:

    TOOLS_DLLPRIVATE bool hasDosVolume(FSysStyle eStyle) const;

    // Coding:

    static inline rtl::OUString extend(const rtl::OString& rOctets)
    {
        return rtl::OStringToOUString(rOctets, RTL_TEXTENCODING_ISO_8859_1);
    }

    static inline sal_Char getEscapePrefix(INetProtocol eTheScheme)
    { return eTheScheme == INET_PROT_VIM ? '=' : '%'; }

    inline sal_Char getEscapePrefix() const
    { return getEscapePrefix(m_eScheme); }

    TOOLS_DLLPRIVATE static inline void appendEscape(
        rtl::OUStringBuffer & rTheText, sal_Char cEscapePrefix,
        sal_uInt32 nOctet);

    static rtl::OUString encodeText(
        sal_Unicode const * pBegin, sal_Unicode const * pEnd, bool bOctets,
        Part ePart, sal_Char cEscapePrefix, EncodeMechanism eMechanism,
        rtl_TextEncoding eCharset, bool bKeepVisibleEscapes);

    static inline rtl::OUString encodeText(
        rtl::OUString const & rTheText, bool bOctets, Part ePart,
        sal_Char cEscapePrefix, EncodeMechanism eMechanism,
        rtl_TextEncoding eCharset, bool bKeepVisibleEscapes);

    static rtl::OUString decode(
        sal_Unicode const * pBegin, sal_Unicode const * pEnd,
        sal_Char cEscapePrefix, DecodeMechanism, rtl_TextEncoding eCharset);

    inline rtl::OUString decode(
        SubString const & rSubString, sal_Char cEscapePrefix,
        DecodeMechanism eMechanism, rtl_TextEncoding eCharset) const;

    // Specialized helpers:

    TOOLS_DLLPRIVATE static bool scanIPv6reference(
        sal_Unicode const *& rBegin, sal_Unicode const * pEnd);

private:
    void changeScheme(INetProtocol eTargetScheme);
};

// static
inline rtl::OUString INetURLObject::encodeText(rtl::OUString const & rTheText,
                                           bool bOctets, Part ePart,
                                           sal_Char cEscapePrefix,
                                           EncodeMechanism eMechanism,
                                           rtl_TextEncoding eCharset,
                                           bool bKeepVisibleEscapes)
{
    return encodeText(rTheText.getStr(),
                      rTheText.getStr() + rTheText.getLength(), bOctets, ePart,
                      cEscapePrefix, eMechanism, eCharset,
                      bKeepVisibleEscapes);
}

inline rtl::OUString INetURLObject::decode(SubString const & rSubString,
                                       sal_Char cEscapePrefix,
                                       DecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset) const
{
    return rSubString.isPresent() ?
               decode(m_aAbsURIRef.getStr() + rSubString.getBegin(),
                      m_aAbsURIRef.getStr() + rSubString.getEnd(),
                      cEscapePrefix, eMechanism, eCharset) :
               rtl::OUString();
}

inline INetURLObject::INetURLObject(const rtl::OString& rTheAbsURIRef,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset):
    m_eScheme(INET_PROT_NOT_VALID), m_eSmartScheme(INET_PROT_HTTP)
{
    setAbsURIRef(extend(rTheAbsURIRef), true, eMechanism, eCharset, false,
                 FSysStyle(0));
}

inline INetURLObject::INetURLObject(rtl::OUString const & rTheAbsURIRef,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset):
    m_eScheme(INET_PROT_NOT_VALID), m_eSmartScheme(INET_PROT_HTTP)
{
    setAbsURIRef(rTheAbsURIRef, false, eMechanism, eCharset, false,
                 FSysStyle(0));
}

inline bool INetURLObject::SetURL(const rtl::OString& rTheAbsURIRef,
                                  EncodeMechanism eMechanism,
                                  rtl_TextEncoding eCharset)
{
    return setAbsURIRef(extend(rTheAbsURIRef), true, eMechanism, eCharset,
                        false, FSysStyle(0));
}

inline bool INetURLObject::SetURL(rtl::OUString const & rTheAbsURIRef,
                                  EncodeMechanism eMechanism,
                                  rtl_TextEncoding eCharset)
{
    return setAbsURIRef(rTheAbsURIRef, false, eMechanism, eCharset, false,
                        FSysStyle(0));
}

inline INetURLObject::INetURLObject(rtl::OUString const & rTheAbsURIRef,
                                    INetProtocol eTheSmartScheme,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset,
                                    FSysStyle eStyle):
    m_eScheme(INET_PROT_NOT_VALID), m_eSmartScheme(eTheSmartScheme)
{
    setAbsURIRef(rTheAbsURIRef, false, eMechanism, eCharset, true, eStyle);
}

inline bool INetURLObject::SetSmartURL(const rtl::OString& rTheAbsURIRef,
                                       EncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset,
                                       FSysStyle eStyle)
{
    return setAbsURIRef(extend(rTheAbsURIRef), true, eMechanism, eCharset,
                        true, eStyle);
}

inline bool INetURLObject::SetSmartURL(rtl::OUString const & rTheAbsURIRef,
                                       EncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset,
                                       FSysStyle eStyle)
{
    return setAbsURIRef(rTheAbsURIRef, false, eMechanism, eCharset, true,
                        eStyle);
}

inline INetURLObject
INetURLObject::smartRel2Abs(const rtl::OString& rTheRelURIRef,
                            bool & rWasAbsolute,
                            bool bIgnoreFragment,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset,
                            bool bRelativeNonURIs,
                            FSysStyle eStyle) const
{
    INetURLObject aTheAbsURIRef;
    convertRelToAbs(extend(rTheRelURIRef), true, aTheAbsURIRef, rWasAbsolute,
                    eMechanism, eCharset, bIgnoreFragment, true,
                    bRelativeNonURIs, eStyle);
    return aTheAbsURIRef;
}

inline INetURLObject
INetURLObject::smartRel2Abs(rtl::OUString const & rTheRelURIRef,
                            bool & rWasAbsolute,
                            bool bIgnoreFragment,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset,
                            bool bRelativeNonURIs,
                            FSysStyle eStyle) const
{
    INetURLObject aTheAbsURIRef;
    convertRelToAbs(rTheRelURIRef, false, aTheAbsURIRef, rWasAbsolute,
                    eMechanism, eCharset, bIgnoreFragment, true,
                    bRelativeNonURIs, eStyle);
    return aTheAbsURIRef;
}

inline bool INetURLObject::GetNewAbsURL(const rtl::OString& rTheRelURIRef,
                                        INetURLObject * pTheAbsURIRef,
                                        EncodeMechanism eMechanism,
                                        rtl_TextEncoding eCharset,
                                        FSysStyle eStyle, bool bIgnoreFragment)
    const
{
    INetURLObject aTheAbsURIRef;
    bool bWasAbsolute;
    if (!convertRelToAbs(extend(rTheRelURIRef), true, aTheAbsURIRef,
                         bWasAbsolute, eMechanism, eCharset, bIgnoreFragment,
                         false, false, eStyle))
        return false;
    if (pTheAbsURIRef)
        *pTheAbsURIRef = aTheAbsURIRef;
    return true;
}

inline bool INetURLObject::GetNewAbsURL(rtl::OUString const & rTheRelURIRef,
                                        INetURLObject * pTheAbsURIRef,
                                        EncodeMechanism eMechanism,
                                        rtl_TextEncoding eCharset,
                                        FSysStyle eStyle, bool bIgnoreFragment)
    const
{
    INetURLObject aTheAbsURIRef;
    bool bWasAbsolute;
    if (!convertRelToAbs(rTheRelURIRef, false, aTheAbsURIRef, bWasAbsolute,
                         eMechanism, eCharset, bIgnoreFragment, false, false,
                         eStyle))
        return false;
    if (pTheAbsURIRef)
        *pTheAbsURIRef = aTheAbsURIRef;
    return true;
}

// static
inline rtl::OUString INetURLObject::GetRelURL(const rtl::OString& rTheBaseURIRef,
                                          const rtl::OString& rTheAbsURIRef,
                                          EncodeMechanism eEncodeMechanism,
                                          DecodeMechanism eDecodeMechanism,
                                          rtl_TextEncoding eCharset,
                                          FSysStyle eStyle)
{
    rtl::OUString aTheRelURIRef;
    INetURLObject(rTheBaseURIRef, eEncodeMechanism, eCharset).
        convertAbsToRel(extend(rTheAbsURIRef), true, aTheRelURIRef,
                        eEncodeMechanism, eDecodeMechanism, eCharset, eStyle);
    return aTheRelURIRef;
}

// static
inline rtl::OUString INetURLObject::GetRelURL(rtl::OUString const & rTheBaseURIRef,
                                          rtl::OUString const & rTheAbsURIRef,
                                          EncodeMechanism eEncodeMechanism,
                                          DecodeMechanism eDecodeMechanism,
                                          rtl_TextEncoding eCharset,
                                          FSysStyle eStyle)
{
    rtl::OUString aTheRelURIRef;
    INetURLObject(rTheBaseURIRef, eEncodeMechanism, eCharset).
        convertAbsToRel(rTheAbsURIRef, false, aTheRelURIRef, eEncodeMechanism,
                        eDecodeMechanism, eCharset, eStyle);
    return aTheRelURIRef;
}

// static
inline bool INetURLObject::translateToExternal(const rtl::OString& rTheIntURIRef,
                                               rtl::OUString & rTheExtURIRef,
                                               DecodeMechanism
                                                   eDecodeMechanism,
                                               rtl_TextEncoding eCharset)
{
    rtl::OUString aTheExtURIRef;
    bool bRet = convertIntToExt(extend(rTheIntURIRef), true, aTheExtURIRef,
                                eDecodeMechanism, eCharset);
    rTheExtURIRef = aTheExtURIRef;
    return bRet;
}

// static
inline bool INetURLObject::translateToExternal(rtl::OUString const &
                                                   rTheIntURIRef,
                                               rtl::OUString & rTheExtURIRef,
                                               DecodeMechanism
                                                   eDecodeMechanism,
                                               rtl_TextEncoding eCharset)
{
    return convertIntToExt(rTheIntURIRef, false, rTheExtURIRef,
                           eDecodeMechanism, eCharset);
}

// static
inline bool INetURLObject::translateToInternal(const rtl::OString&
                                                   rTheExtURIRef,
                                               rtl::OUString & rTheIntURIRef,
                                               DecodeMechanism
                                                   eDecodeMechanism,
                                               rtl_TextEncoding eCharset)
{
    rtl::OUString aTheIntURIRef;
    bool bRet = convertExtToInt(extend(rTheExtURIRef), true, aTheIntURIRef,
                           eDecodeMechanism, eCharset);
    rTheIntURIRef = aTheIntURIRef;
    return bRet;
}

// static
inline bool INetURLObject::translateToInternal(rtl::OUString const &
                                                   rTheExtURIRef,
                                               rtl::OUString & rTheIntURIRef,
                                               DecodeMechanism
                                                   eDecodeMechanism,
                                               rtl_TextEncoding eCharset)
{
    return convertExtToInt(rTheExtURIRef, false, rTheIntURIRef,
                           eDecodeMechanism, eCharset);
}

inline bool INetURLObject::SetPass(const rtl::OString& rThePassword,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset)
{
    return rThePassword.isEmpty() ?
               clearPassword() :
               setPassword(extend(rThePassword), true, eMechanism, eCharset);
}

inline bool INetURLObject::SetPass(rtl::OUString const & rThePassword,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset)
{
    return rThePassword.isEmpty() ?
               clearPassword() :
               setPassword(rThePassword, false, eMechanism, eCharset);
}

inline bool INetURLObject::SetUserAndPass(const rtl::OString& rTheUser,
                                          const rtl::OString& rThePassword,
                                          EncodeMechanism eMechanism,
                                          rtl_TextEncoding eCharset)
{
    return setUser(extend(rTheUser), true, eMechanism, eCharset)
           && (rThePassword.isEmpty() ?
                   clearPassword() :
                   setPassword(extend(rThePassword), true, eMechanism,
                               eCharset));
}

inline bool INetURLObject::SetUserAndPass(rtl::OUString const & rTheUser,
                                          rtl::OUString const & rThePassword,
                                          EncodeMechanism eMechanism,
                                          rtl_TextEncoding eCharset)
{
    return setUser(rTheUser, false, eMechanism, eCharset)
           && (rThePassword.isEmpty() ?
                   clearPassword() :
                   setPassword(rThePassword, false, eMechanism, eCharset));
}

inline bool INetURLObject::insertName(rtl::OUString const & rTheName,
                                      bool bAppendFinalSlash,
                                      sal_Int32 nIndex,
                                      bool bIgnoreFinalSlash,
                                      EncodeMechanism eMechanism,
                                      rtl_TextEncoding eCharset)
{
    return insertName(rTheName, false, bAppendFinalSlash, nIndex,
                      bIgnoreFinalSlash, eMechanism, eCharset);
}

inline bool INetURLObject::SetParam(const rtl::OString& rTheQuery,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset)
{
    return rTheQuery.isEmpty() ?
               clearQuery() :
               setQuery(extend(rTheQuery), true, eMechanism, eCharset);
}

inline bool INetURLObject::SetParam(rtl::OUString const & rTheQuery,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset)
{
    return rTheQuery.isEmpty() ?
               clearQuery() :
               setQuery(rTheQuery, false, eMechanism, eCharset);
}

inline bool INetURLObject::SetMark(const rtl::OString& rTheFragment,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset)
{
    return rTheFragment.isEmpty() ?
               clearFragment() :
               setFragment(extend(rTheFragment), true, eMechanism, eCharset);
}

inline bool INetURLObject::SetMark(rtl::OUString const & rTheFragment,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset)
{
    return rTheFragment.isEmpty() ?
               clearFragment() :
               setFragment(rTheFragment, false, eMechanism, eCharset);
}

inline INetURLObject::INetURLObject(rtl::OUString const & rFSysPath,
                                    FSysStyle eStyle):
    m_eScheme(INET_PROT_NOT_VALID), m_eSmartScheme(INET_PROT_HTTP)
{
    setFSysPath(rFSysPath, eStyle);
}

// static
inline rtl::OUString INetURLObject::encode(const rtl::OString& rText, Part ePart,
                                       sal_Char cEscapePrefix,
                                       EncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return encodeText(extend(rText), true, ePart, cEscapePrefix, eMechanism,
                      eCharset, false);
}

// static
inline rtl::OUString INetURLObject::encode(rtl::OUString const & rText, Part ePart,
                                       sal_Char cEscapePrefix,
                                       EncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return encodeText(rText, false, ePart, cEscapePrefix, eMechanism,
                      eCharset, false);
}

// static
inline rtl::OUString INetURLObject::decode(rtl::OUString const & rText,
                                       sal_Char cEscapePrefix,
                                       DecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return decode(rText.getStr(), rText.getStr() + rText.getLength(),
                  cEscapePrefix, eMechanism, eCharset);
}

inline rtl::OUString INetURLObject::decode(rtl::OUStringBuffer const & rText,
                                       sal_Char cEscapePrefix,
                                       DecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return decode(rText.getStr(), rText.getStr() + rText.getLength(),
                  cEscapePrefix, eMechanism, eCharset);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
