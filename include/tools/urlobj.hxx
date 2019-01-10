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
#ifndef INCLUDED_TOOLS_URLOBJ_HXX
#define INCLUDED_TOOLS_URLOBJ_HXX

#include <tools/toolsdllapi.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/textenc.h>
#include <sal/types.h>
#include <o3tl/typed_flags_set.hxx>

#include <memory>
#include <string_view>

class SvMemoryStream;

namespace com { namespace sun { namespace star { namespace util {
    class XStringWidth;
} } } }

namespace com { namespace sun { namespace star { namespace uno { template <typename > class Reference; } } } }

// Common URL prefixes for various schemes:
#define INET_FTP_SCHEME "ftp://"
#define INET_HTTP_SCHEME "http://"
#define INET_HTTPS_SCHEME "https://"
#define INET_FILE_SCHEME "file://"
#define INET_MAILTO_SCHEME "mailto:"
#define INET_HID_SCHEME "hid:"

#define URL_PREFIX_PRIV_SOFFICE "private:"

// Schemes:
enum class INetProtocol
{
    NotValid,
    Ftp,
    Http,
    File,
    Mailto,
    VndSunStarWebdav,
    PrivSoffice,
    VndSunStarHelp,
    Https,
    Slot,
    Macro,
    Javascript,
    Data,
    Cid,
    VndSunStarHier,
    Uno,
    Component,
    VndSunStarPkg,
    Ldap,
    Db,
    VndSunStarCmd,
    Telnet,
    VndSunStarExpand,
    VndSunStarTdoc,
    Generic,
    Smb,
    Hid,
    Sftp,
    Cmis,
    LAST = Cmis
};

/** The supported notations for file system paths.
 */
enum class FSysStyle
{
    /** VOS notation (e.g., "//server/dir/file").
     */
    Vos = 0x1,

    /** Unix notation (e.g., "/dir/file").
     */
    Unix = 0x2,

    /** DOS notation (e.g., "a:\dir\file" and "\\server\dir\file").
     */
    Dos = 0x4,

    /** Detect the used notation.

        @descr  For the following descriptions, please note that
        whereas FSYS_DEFAULT includes all style bits, combinations of only
        a few style bits are also possible, and are also described.

        @descr  When used to translate a file system path to a file URL,
        the subset of the following productions for which the appropriate
        style bit is set are checked in order (using the conventions of
        RFC 2234, RFC 2396, and RFC 2732; UCS4 stands for any UCS4
        character):

         Production T1 (VOS local; FSysStyle::Vos only):
            "//." ["/" *UCS4]
          becomes
            "file:///" *UCS4

         Production T2 (VOS host; FSysStyle::Vos only):
            "//" [host] ["/" *UCS4]
          becomes
            "file://" host "/" *UCS4

         Production T3 (UNC; FSysStyle::Dos only):
            "\\" [host] ["\" *UCS4]
          becomes
            "file://" host "/" *UCS4
          replacing "\" by "/" within <*UCS4>

         Production T4 (Unix-like DOS; FSysStyle::Dos only):
            ALPHA ":" ["/" *UCS4]
          becomes
            "file:///" ALPHA ":/" *UCS4
          replacing "\" by "/" within <*UCS4>

         Production T5 (DOS; FSysStyle::Dos only):
            ALPHA ":" ["\" *UCS4]
          becomes
            "file:///" ALPHA ":/" *UCS4
          replacing "\" by "/" within <*UCS4>

         Production T6 (any):
            *UCS4
          becomes
            "file:///" *UCS4
          replacing the delimiter by "/" within <*UCS4>.  The delimiter is
          that character from the set { "/", "\" } which appears most
          often in <*UCS4> (if FSysStyle::Unix is not among the style bits, "/"
          is removed from the set; if FSysStyle::Dos is not among the style
          bits, "\" is removed from the set).  If two or more
          characters appear the same number of times, the character
          mentioned first in that set is chosen.  If the first character
          of <*UCS4> is the delimiter, that character is not copied.

        @descr  When used to translate a file URL to a file system path,
        the following productions are checked in order (using the
        conventions of RFC 2234, RFC 2396, and RFC 2732):

         Production F1 (VOS; FSysStyle::Vos):
            "file://" host "/" fpath ["#" fragment]
          becomes
            "//" host "/" fpath

         Production F2 (DOS; FSysStyle::Dos):
            "file:///" ALPHA ":" ["/" fpath] ["#" fragment]
          becomes
            ALPHA ":" ["\" fpath]
          replacing "/" by "\" in <fpath>

         Production F3 (Unix; FSysStyle::Unix):
            "file:///" fpath ["#" fragment]
          becomes
            "/" fpath
     */
    Detect = Vos | Unix | Dos
};
namespace o3tl {
    template<> struct typed_flags<FSysStyle> : is_typed_flags<FSysStyle, 0x07> {};
}

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC INetURLObject
{
public:
    // Get- and Set-Methods:

    /** The way input strings that represent (parts of) URIs are interpreted
        in set-methods.

        @descr  UTF-32 characters in the range 0x80--0x10FFFF are replaced by
        sequences of escape sequences, representing the UTF-8 coded characters.

        @descr  Along with an EncodeMechanism parameter, the set-methods all
        take an rtl_TextEncoding parameter, which is ignored unless the
        EncodeMechanism is EncodeMechanism::WasEncoded.
     */
    enum class EncodeMechanism
    {
        /** All escape sequences that are already present are ignored, and are
            interpreted as literal sequences of three characters.
         */
        All,

        /** Sequences of escape sequences, that represent characters from the
            specified character set and that can be converted to UTF-32
            characters, are first decoded.  If they have to be encoded, they
            are converted to UTF-8 characters and are than translated into
            (sequences of) escape sequences. Other escape sequences are
            copied verbatim (but using upper case hex digits).
         */
        WasEncoded,

        /** All escape sequences that are already present are copied verbatim
            (but using upper case hex digits).
         */
        NotCanonical
    };

    /** The way strings that represent (parts of) URIs are returned from get-
        methods.

        @descr  Along with a DecodeMechanism parameter, the get-methods all
        take an rtl_TextEncoding parameter, which is ignored unless the
        DecodeMechanism is DecodeMechanism::WithCharset or DecodeMechanism::Unambiguous.
     */
    enum class DecodeMechanism
    {
        /** The (part of the) URI is returned unchanged.  Since URIs are
            written using a subset of US-ASCII, the returned string is
            guaranteed to contain only US-ASCII characters.
         */
        NONE,

        /** All sequences of escape sequences that represent UTF-8 coded
            UTF-32 characters with a numerical value greater than 0x7F, are
            replaced by the respective UTF-16 characters.  All other escape
            sequences are not decoded.
         */
        ToIUri,

        /** All (sequences of) escape sequences that represent characters from
            the specified character set, and that can be converted to UTF-32,
            are replaced by the respective UTF-16 characters.  All other
            escape sequences are not decoded.
         */
        WithCharset,

        /** All (sequences of) escape sequences that represent characters from
            the specified character set, that can be converted to UTF-32, and
            that (in the case of ASCII characters) can safely be decoded
            without altering the meaning of the (part of the) URI, are
            replaced by the respective UTF-16 characters.  All other escape
            sequences are not decoded.
         */
        Unambiguous
    };

    // General Structure:

    INetURLObject():
        m_eScheme(INetProtocol::NotValid), m_eSmartScheme(INetProtocol::Http) {}

    bool HasError() const { return m_eScheme == INetProtocol::NotValid; }

    OUString GetMainURL(DecodeMechanism eMechanism,
                                rtl_TextEncoding eCharset
                                    = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aAbsURIRef, eMechanism, eCharset); }

    OUString GetURLNoPass(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    OUString GetURLNoMark(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    OUString
    getAbbreviated(css::uno::Reference< css::util::XStringWidth > const & rStringWidth,
                   sal_Int32 nWidth,
                   DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                   rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    bool operator ==(INetURLObject const & rObject) const;

    bool operator !=(INetURLObject const & rObject) const
    { return !(*this == rObject); }

    // Strict Parsing:

    inline explicit INetURLObject(
        OUString const & rTheAbsURIRef,
        EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    inline bool SetURL(OUString const & rTheAbsURIRef,
                       EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    bool ConcatData(INetProtocol eTheScheme, OUString const & rTheUser,
                    OUString const & rThePassword,
                    OUString const & rTheHost, sal_uInt32 nThePort,
                    OUString const & rThePath);

    // Smart Parsing:

    inline INetURLObject(OUString const & rTheAbsURIRef,
                         INetProtocol eTheSmartScheme,
                         EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                         FSysStyle eStyle = FSysStyle::Detect);

    void SetSmartProtocol(INetProtocol eTheSmartScheme)
    { m_eSmartScheme = eTheSmartScheme; }

    inline bool
    SetSmartURL(OUString const & rTheAbsURIRef,
                EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                FSysStyle eStyle = FSysStyle::Detect);

    inline INetURLObject
    smartRel2Abs(OUString const & rTheRelURIRef,
                 bool & rWasAbsolute,
                 bool bIgnoreFragment = false,
                 EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
                 bool bRelativeNonURIs = false,
                 FSysStyle eStyle = FSysStyle::Detect) const;

    // Relative URLs:

    inline bool
    GetNewAbsURL(OUString const & rTheRelURIRef,
                 INetURLObject * pTheAbsURIRef)
        const;

    /** @descr  If rTheRelURIRef cannot be converted to an absolute URL
        (because of syntactic reasons), either rTheRelURIRef or an empty
        string is returned:  If all of the parameters eEncodeMechanism,
        eDecodeMechanism and eCharset have their respective default values,
        then rTheRelURIRef is returned unmodified; otherwise, an empty string
        is returned.
     */
    static OUString
    GetAbsURL(OUString const & rTheBaseURIRef,
              OUString const & rTheRelURIRef,
              EncodeMechanism eEncodeMechanism = EncodeMechanism::WasEncoded,
              DecodeMechanism eDecodeMechanism = DecodeMechanism::ToIUri,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    static inline OUString
    GetRelURL(OUString const & rTheBaseURIRef,
              OUString const & rTheAbsURIRef,
              EncodeMechanism eEncodeMechanism = EncodeMechanism::WasEncoded,
              DecodeMechanism eDecodeMechanism = DecodeMechanism::ToIUri,
              rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8,
              FSysStyle eStyle = FSysStyle::Detect);

    // External URLs:

    OUString getExternalURL() const;

    static inline bool translateToExternal(OUString const & rTheIntURIRef,
                                           OUString & rTheExtURIRef,
                                           DecodeMechanism eDecodeMechanism
                                               = DecodeMechanism::ToIUri,
                                           rtl_TextEncoding eCharset
                                               = RTL_TEXTENCODING_UTF8);

    static inline bool translateToInternal(OUString const & rTheExtURIRef,
                                           OUString & rTheIntURIRef,
                                           DecodeMechanism eDecodeMechanism
                                               = DecodeMechanism::ToIUri,
                                           rtl_TextEncoding eCharset
                                               = RTL_TEXTENCODING_UTF8);

    // Scheme:

    struct SchemeInfo;

    INetProtocol GetProtocol() const { return m_eScheme; }

    bool isSchemeEqualTo(INetProtocol scheme) const { return scheme == m_eScheme; }

    bool isSchemeEqualTo(std::u16string_view scheme) const;

    /** Check if the scheme is one of the WebDAV scheme
     *  we know about.
     *
     *  @return true is one other scheme either public scheme or private scheme.
     */
    bool isAnyKnownWebDAVScheme() const;

    /** Return the URL 'prefix' for a given scheme.

        @param eTheScheme  One of the supported URL schemes.

        @return  The 'prefix' of URLs of the given scheme.
     */
    static OUString GetScheme(INetProtocol eTheScheme);

    /** Return the a human-readable name for a given scheme.

        @param eTheScheme  One of the supported URL schemes.

        @return  The protocol name of URLs of the given scheme.
     */
    static OUString GetSchemeName(INetProtocol eTheScheme);

    static INetProtocol CompareProtocolScheme(OUString const &
                                                  rTheAbsURIRef);

    // User Info:

    bool HasUserData() const { return m_aUser.isPresent(); }

    OUString GetUser(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aUser, eMechanism, eCharset); }

    OUString GetPass(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aAuth, eMechanism, eCharset); }

    bool SetUser(OUString const & rTheUser)
    { return setUser(rTheUser, RTL_TEXTENCODING_UTF8); }

    inline bool SetPass(OUString const & rThePassword);

    inline bool SetUserAndPass(OUString const & rTheUser,
                               OUString const & rThePassword);

    // Host and Port:

    bool HasPort() const { return m_aPort.isPresent(); }

    OUString GetHost(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aHost, eMechanism, eCharset); }

    OUString GetHostPort(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                          rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8) const;

    sal_uInt32 GetPort() const;

    bool SetHost(OUString const & rTheHost)
    { return setHost(rTheHost, RTL_TEXTENCODING_UTF8); }

    bool SetPort(sal_uInt32 nThePort);

    // Path:

    bool HasURLPath() const { return !m_aPath.isEmpty(); }

    OUString GetURLPath(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                                rtl_TextEncoding eCharset
                                    = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aPath, eMechanism, eCharset); }

    bool SetURLPath(OUString const & rThePath,
                           EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
    { return setPath(rThePath, eMechanism, eCharset); }

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
        A final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param rTheName  The name part of the new segment.  The new segment
        will contain no parameters.

        @param bAppendFinalSlash  If the new segment is appended at the end of
        the hierarchical path, this parameter specifies whether to add a final
        slash after it or not.

        @param nIndex  The non-negative index of the segment before which
        to insert the new segment.  LAST_SEGMENT or an nIndex that equals
        getSegmentCount() inserts the new segment at the end of the
        hierarchical path.

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the segment has successfully been inserted (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified place to insert the new segment does not exist, false is
        returned.  If false is returned, the object is not modified.
     */
    bool insertName(OUString const & rTheName,
                           bool bAppendFinalSlash = false,
                           sal_Int32 nIndex = LAST_SEGMENT,
                           EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
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
    OUString getName(sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true,
                      DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Set the name of a segment (preserving any parameters and any query or
        fragment part).

        @param rTheName  The new name.

        @return  True if the name has successfully been modified (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool setName(OUString const & rTheName);

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
    OUString getBase(sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true,
                      DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                      rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Set the base of the name of a segment (preserving the extension).
        A final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param rTheBase  The new base.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the base has successfully been modified (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool setBase(OUString const & rTheBase,
                 sal_Int32 nIndex = LAST_SEGMENT,
                 EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    /** Determine whether the name of the last segment has an extension.

        @return  True if the name of the specified segment has an extension.
        If the path is not hierarchical, or the specified segment does not
        exist, false is returned.
     */
    bool hasExtension() const;

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
    OUString getExtension(sal_Int32 nIndex = LAST_SEGMENT,
                           bool bIgnoreFinalSlash = true,
                           DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                           rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Set the extension of the name of a segment (replacing an already
        existing extension).

        @param rTheExtension  The new extension.

        @param nIndex  The non-negative index of the segment, or LAST_SEGMENT
        if addressing the last segment.

        @param bIgnoreFinalSlash  If true, a final slash at the end of the
        hierarchical path does not denote an empty segment, but is ignored.

        @param eCharset  See the general discussion for set-methods.

        @return  True if the extension has successfully been modified (and the
        resulting URI is still valid).  If the path is not hierarchical, or
        the specified segment does not exist, false is returned.  If false is
        returned, the object is not modified.
     */
    bool setExtension(OUString const & rTheExtension,
                      sal_Int32 nIndex = LAST_SEGMENT,
                      bool bIgnoreFinalSlash = true,
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

    // Query:

    bool HasParam() const { return m_aQuery.isPresent(); }

    OUString GetParam(rtl_TextEncoding eCharset
                                  = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aQuery, DecodeMechanism::NONE, eCharset); }

    inline bool SetParam(OUString const & rTheQuery,
                         EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                         rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    // Fragment:

    bool HasMark() const { return m_aFragment.isPresent(); }

    OUString GetMark(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return decode(m_aFragment, eMechanism, eCharset); }

    inline bool SetMark(OUString const & rTheFragment,
                        EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                        rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    // File URLs:

    /** Create an INetURLObject from a file system path.

        @param rFSysPath  A file system path.  An URL is not allowed here!

        @param eStyle  The notation of rFSysPath.
     */
    inline INetURLObject(OUString const & rFSysPath, FSysStyle eStyle);

    /** Set this INetURLObject to a file URL constructed from a file system
        path.

        @param rFSysPath  A file system path.  An URL is not allowed here!

        @param eStyle  The notation of rFSysPath.

        @return  True if this INetURLObject has successfully been changed.  If
        false is returned, this INetURLObject has not been modified.
     */
    bool setFSysPath(OUString const & rFSysPath, FSysStyle eStyle);

    /** Return the file system path represented by a file URL (ignoring any
        fragment part).

        @param eStyle  The notation of the returned file system path.

        @param pDelimiter  Upon successful return, this parameter can return
        the character that is the 'main' delimiter within the returned file
        system path (e.g., "/" for Unix, "\" for DOS).  This is
        especially useful for routines that later try to shorten the returned
        file system path at a 'good' position, e.g. to fit it into some
        limited display space.

        @return  The file system path represented by this file URL.  If this
        file URL does not represent a file system path according to the
        specified notation, or if this is not a file URL at all, an empty
        string is returned.
     */
    OUString getFSysPath(FSysStyle eStyle, sal_Unicode * pDelimiter = nullptr)
        const;

    // Data URLs:
    std::unique_ptr<SvMemoryStream> getData() const;

    // Coding:

    enum Part
    {
        PART_USER_PASSWORD          = 0x00001,
        PART_FPATH                  = 0x00008,
        PART_AUTHORITY              = 0x00010,
        PART_REL_SEGMENT_EXTRA      = 0x00020,
        PART_URIC                   = 0x00040,
        PART_HTTP_PATH              = 0x00080,
        PART_MESSAGE_ID_PATH        = 0x00100,
        PART_MAILTO                 = 0x00200,
        PART_PATH_BEFORE_QUERY      = 0x00400,
        PART_PCHAR                  = 0x00800,
        PART_VISIBLE                = 0x01000,
        PART_VISIBLE_NONSPECIAL     = 0x02000,
        PART_UNO_PARAM_VALUE        = 0x04000,
        PART_UNAMBIGUOUS            = 0x08000,
        PART_URIC_NO_SLASH          = 0x10000,
        PART_HTTP_QUERY             = 0x20000, //TODO! unused?
    };

    enum class EscapeType
    {
        NONE,
        Octet,
        Utf32
    };

    /** Encode some text as part of a URI.

        @param rText  Some text (for its interpretation, see the general
        discussion for set-methods).

        @param ePart  The part says which characters are 'forbidden' and must
        be encoded (replaced by escape sequences).  Characters outside the US-
        ASCII range are always 'forbidden.'

        @param eMechanism  See the general discussion for set-methods.

        @param eCharset  See the general discussion for set-methods.

        @return  The text, encoded according to the given mechanism and
        charset ('forbidden' characters replaced by escape sequences).
     */
    static inline OUString encode(OUString const & rText, Part ePart,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    /** Decode some text.

        @param rText  Some (encoded) text.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  The text, decoded according to the given mechanism and
        charset (escape sequences replaced by 'raw' characters).
     */
    static inline OUString decode(OUString const & rText,
                                   DecodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    static inline OUString decode(OUStringBuffer const & rText,
                                   DecodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset
                                       = RTL_TEXTENCODING_UTF8);

    static void appendUCS4Escape(OUStringBuffer & rTheText, sal_uInt32 nUCS4);

    static void appendUCS4(OUStringBuffer & rTheText, sal_uInt32 nUCS4,
                           EscapeType eEscapeType, Part ePart,
                           rtl_TextEncoding eCharset, bool bKeepVisibleEscapes);

    static sal_uInt32 getUTF32(sal_Unicode const *& rBegin,
                               sal_Unicode const * pEnd,
                               EncodeMechanism eMechanism,
                               rtl_TextEncoding eCharset,
                               EscapeType & rEscapeType);

    // Specialized helpers:

    static sal_uInt32 scanDomain(sal_Unicode const *& rBegin,
                                 sal_Unicode const * pEnd,
                                 bool bEager = true);

    // OBSOLETE Hierarchical Path:

    OUString GetPartBeforeLastName() const;

    /** Get the last segment in the path.

        @param eMechanism  See the general discussion for get-methods.

        @param eCharset  See the general discussion for get-methods.

        @return  For a hierarchical URL, the last segment (everything after
        the last unencoded '/').  Note that this last segment may be empty.  If
        the URL is not hierarchical, an empty string is returned.
     */
    OUString GetLastName(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                          rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8)
        const;

    /** Get the 'extension' of the last segment in the path.

        @return  For a hierarchical URL, everything after the first unencoded
        '.' in the last segment of the path.  Note that this 'extension' may
        be empty.  If the URL is not hierarchical, or if the last segment does
        not contain an unencoded '.', an empty string is returned.
     */
    OUString GetFileExtension() const;

    bool Append(OUString const & rTheSegment,
                       EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                       rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    void CutLastName();

    // OBSOLETE File URLs:

    OUString PathToFileName() const;

    OUString GetFull() const;

    OUString GetPath() const;

    void SetBase(OUString const & rTheBase);

    OUString GetBase() const;

    void SetName(OUString const & rTheName,
                 EncodeMechanism eMechanism = EncodeMechanism::WasEncoded,
                 rtl_TextEncoding eCharset = RTL_TEXTENCODING_UTF8);

    OUString GetName(DecodeMechanism eMechanism = DecodeMechanism::ToIUri,
                             rtl_TextEncoding eCharset
                                 = RTL_TEXTENCODING_UTF8) const
    { return GetLastName(eMechanism, eCharset); }

    void SetExtension(OUString const & rTheExtension);

    OUString GetExtension() const
    { return GetFileExtension(); }

    OUString CutExtension();

    static bool IsCaseSensitive() { return true; }

    void changeScheme(INetProtocol eTargetScheme);

private:
    // General Structure:

    class SubString
    {
        sal_Int32 m_nBegin;
        sal_Int32 m_nLength;

    public:
        explicit SubString(sal_Int32 nTheBegin = -1,
                                  sal_Int32 nTheLength = 0):
            m_nBegin(nTheBegin), m_nLength(nTheLength) {}

        bool isPresent() const { return m_nBegin != -1; }

        bool isEmpty() const { return m_nLength == 0; }

        sal_Int32 getBegin() const { return m_nBegin; }

        sal_Int32 getLength() const { return m_nLength; }

        sal_Int32 getEnd() const { return m_nBegin + m_nLength; }

        inline sal_Int32 clear();

        inline sal_Int32 set(OUStringBuffer & rString,
                             OUString const & rSubString,
                             sal_Int32 nTheBegin);

        inline sal_Int32 set(OUString & rString,
                             OUString const & rSubString);

        inline sal_Int32 set(OUStringBuffer & rString,
                             OUString const & rSubString);

        inline void operator +=(sal_Int32 nDelta);

        int compare(SubString const & rOther,
            OUStringBuffer const & rThisString,
            OUStringBuffer const & rOtherString) const;
    };

    OUStringBuffer m_aAbsURIRef;
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
        OUString const & rTheAbsURIRef,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset, bool bSmart,
        FSysStyle eStyle);

    // Relative URLs:

    bool convertRelToAbs(
        OUString const & rTheRelURIRef,
        INetURLObject & rTheAbsURIRef, bool & rWasAbsolute,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
        bool bIgnoreFragment, bool bSmart, bool bRelativeNonURIs,
        FSysStyle eStyle) const;

    bool convertAbsToRel(
        OUString const & rTheAbsURIRef,
        OUString & rTheRelURIRef, EncodeMechanism eEncodeMechanism,
        DecodeMechanism eDecodeMechanism, rtl_TextEncoding eCharset,
        FSysStyle eStyle) const;

    // External URLs:

    static bool convertIntToExt(
        OUString const & rTheIntURIRef,
        OUString & rTheExtURIRef, DecodeMechanism eDecodeMechanism,
        rtl_TextEncoding eCharset);

    static bool convertExtToInt(
        OUString const & rTheExtURIRef,
        OUString & rTheIntURIRef, DecodeMechanism eDecodeMechanism,
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
        OUString const & rTheUser,
        rtl_TextEncoding eCharset);

    bool clearPassword();

    bool setPassword(
        OUString const & rThePassword,
        rtl_TextEncoding eCharset);

    // Host and Port:

    TOOLS_DLLPRIVATE static bool parseHost(
        sal_Unicode const *& rBegin, sal_Unicode const * pEnd,
        OUString & rCanonic);

    TOOLS_DLLPRIVATE static bool parseHostOrNetBiosName(
        sal_Unicode const * pBegin, sal_Unicode const * pEnd,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
        bool bNetBiosName, OUStringBuffer* pCanonic);

    bool setHost(
        OUString const & rTheHost,
        rtl_TextEncoding eCharset);

    // Path:

    TOOLS_DLLPRIVATE static bool parsePath(
        INetProtocol eScheme, sal_Unicode const ** pBegin,
        sal_Unicode const * pEnd, EncodeMechanism eMechanism,
        rtl_TextEncoding eCharset, bool bSkippedInitialSlash,
        sal_uInt32 nSegmentDelimiter, sal_uInt32 nAltSegmentDelimiter,
        sal_uInt32 nQueryDelimiter, sal_uInt32 nFragmentDelimiter,
        OUStringBuffer &rSynPath);

    bool setPath(
        OUString const & rThePath,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // Hierarchical Path:

    TOOLS_DLLPRIVATE bool checkHierarchical() const;

    TOOLS_DLLPRIVATE SubString getSegment(
        sal_Int32 nIndex, bool bIgnoreFinalSlash) const;

    // Query:

    void clearQuery();

    bool setQuery(
        OUString const & rTheQuery,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // Fragment:

    bool clearFragment();

    bool setFragment(
        OUString const & rTheMark,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset);

    // FILE URLs:

    TOOLS_DLLPRIVATE bool hasDosVolume(FSysStyle eStyle) const;

    // Coding:

    TOOLS_DLLPRIVATE static inline void appendEscape(
        OUStringBuffer & rTheText, sal_uInt32 nOctet);

    static OUString encodeText(
        sal_Unicode const * pBegin, sal_Unicode const * pEnd,
        Part ePart, EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
        bool bKeepVisibleEscapes);

    static inline OUString encodeText(
        OUString const & rTheText, Part ePart,
        EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
        bool bKeepVisibleEscapes);

    static OUString decode(
        sal_Unicode const * pBegin, sal_Unicode const * pEnd,
        DecodeMechanism, rtl_TextEncoding eCharset);

    inline OUString decode(
        SubString const & rSubString,
        DecodeMechanism eMechanism, rtl_TextEncoding eCharset) const;

    // Specialized helpers:

    TOOLS_DLLPRIVATE static bool scanIPv6reference(
        sal_Unicode const *& rBegin, sal_Unicode const * pEnd);
};

// static
inline OUString INetURLObject::encodeText(OUString const & rTheText,
                                           Part ePart,
                                           EncodeMechanism eMechanism,
                                           rtl_TextEncoding eCharset,
                                           bool bKeepVisibleEscapes)
{
    return encodeText(rTheText.getStr(),
                      rTheText.getStr() + rTheText.getLength(), ePart,
                      eMechanism, eCharset, bKeepVisibleEscapes);
}

inline OUString INetURLObject::decode(SubString const & rSubString,
                                       DecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset) const
{
    return rSubString.isPresent() ?
               decode(m_aAbsURIRef.getStr() + rSubString.getBegin(),
                      m_aAbsURIRef.getStr() + rSubString.getEnd(),
                      eMechanism, eCharset) :
               OUString();
}

inline INetURLObject::INetURLObject(OUString const & rTheAbsURIRef,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset):
    m_eScheme(INetProtocol::NotValid), m_eSmartScheme(INetProtocol::Http)
{
    setAbsURIRef(rTheAbsURIRef, eMechanism, eCharset, false,
                 FSysStyle(0));
}

inline bool INetURLObject::SetURL(OUString const & rTheAbsURIRef,
                                  EncodeMechanism eMechanism,
                                  rtl_TextEncoding eCharset)
{
    return setAbsURIRef(rTheAbsURIRef, eMechanism, eCharset, false,
                        FSysStyle(0));
}

inline INetURLObject::INetURLObject(OUString const & rTheAbsURIRef,
                                    INetProtocol eTheSmartScheme,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset,
                                    FSysStyle eStyle):
    m_eScheme(INetProtocol::NotValid), m_eSmartScheme(eTheSmartScheme)
{
    setAbsURIRef(rTheAbsURIRef, eMechanism, eCharset, true, eStyle);
}

inline bool INetURLObject::SetSmartURL(OUString const & rTheAbsURIRef,
                                       EncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset,
                                       FSysStyle eStyle)
{
    return setAbsURIRef(rTheAbsURIRef, eMechanism, eCharset, true,
                        eStyle);
}

inline INetURLObject
INetURLObject::smartRel2Abs(OUString const & rTheRelURIRef,
                            bool & rWasAbsolute,
                            bool bIgnoreFragment,
                            EncodeMechanism eMechanism,
                            rtl_TextEncoding eCharset,
                            bool bRelativeNonURIs,
                            FSysStyle eStyle) const
{
    INetURLObject aTheAbsURIRef;
    convertRelToAbs(rTheRelURIRef, aTheAbsURIRef, rWasAbsolute,
                    eMechanism, eCharset, bIgnoreFragment, true,
                    bRelativeNonURIs, eStyle);
    return aTheAbsURIRef;
}

inline bool INetURLObject::GetNewAbsURL(OUString const & rTheRelURIRef,
                                        INetURLObject * pTheAbsURIRef)
    const
{
    INetURLObject aTheAbsURIRef;
    bool bWasAbsolute;
    if (!convertRelToAbs(rTheRelURIRef, aTheAbsURIRef, bWasAbsolute,
                         EncodeMechanism::WasEncoded, RTL_TEXTENCODING_UTF8, false/*bIgnoreFragment*/, false, false,
                         FSysStyle::Detect))
        return false;
    if (pTheAbsURIRef)
        *pTheAbsURIRef = aTheAbsURIRef;
    return true;
}

// static
inline OUString INetURLObject::GetRelURL(OUString const & rTheBaseURIRef,
                                          OUString const & rTheAbsURIRef,
                                          EncodeMechanism eEncodeMechanism,
                                          DecodeMechanism eDecodeMechanism,
                                          rtl_TextEncoding eCharset,
                                          FSysStyle eStyle)
{
    OUString aTheRelURIRef;
    INetURLObject(rTheBaseURIRef, eEncodeMechanism, eCharset).
        convertAbsToRel(rTheAbsURIRef, aTheRelURIRef, eEncodeMechanism,
                        eDecodeMechanism, eCharset, eStyle);
    return aTheRelURIRef;
}

// static
inline bool INetURLObject::translateToExternal(OUString const &
                                                   rTheIntURIRef,
                                               OUString & rTheExtURIRef,
                                               DecodeMechanism
                                                   eDecodeMechanism,
                                               rtl_TextEncoding eCharset)
{
    return convertIntToExt(rTheIntURIRef, rTheExtURIRef,
                           eDecodeMechanism, eCharset);
}

// static
inline bool INetURLObject::translateToInternal(OUString const &
                                                   rTheExtURIRef,
                                               OUString & rTheIntURIRef,
                                               DecodeMechanism
                                                   eDecodeMechanism,
                                               rtl_TextEncoding eCharset)
{
    return convertExtToInt(rTheExtURIRef, rTheIntURIRef,
                           eDecodeMechanism, eCharset);
}

inline bool INetURLObject::SetPass(OUString const & rThePassword)
{
    return rThePassword.isEmpty() ?
               clearPassword() :
               setPassword(rThePassword, RTL_TEXTENCODING_UTF8);
}

inline bool INetURLObject::SetUserAndPass(OUString const & rTheUser,
                                          OUString const & rThePassword)
{
    return setUser(rTheUser, RTL_TEXTENCODING_UTF8)
           && (rThePassword.isEmpty() ?
                   clearPassword() :
                   setPassword(rThePassword, RTL_TEXTENCODING_UTF8));
}

inline bool INetURLObject::SetParam(OUString const & rTheQuery,
                                    EncodeMechanism eMechanism,
                                    rtl_TextEncoding eCharset)
{
    if (rTheQuery.isEmpty())
    {
        clearQuery();
        return false;
    }
    return setQuery(rTheQuery, eMechanism, eCharset);
}

inline bool INetURLObject::SetMark(OUString const & rTheFragment,
                                   EncodeMechanism eMechanism,
                                   rtl_TextEncoding eCharset)
{
    return rTheFragment.isEmpty() ?
               clearFragment() :
               setFragment(rTheFragment, eMechanism, eCharset);
}

inline INetURLObject::INetURLObject(OUString const & rFSysPath,
                                    FSysStyle eStyle):
    m_eScheme(INetProtocol::NotValid), m_eSmartScheme(INetProtocol::Http)
{
    setFSysPath(rFSysPath, eStyle);
}

// static
inline OUString INetURLObject::encode(OUString const & rText, Part ePart,
                                       EncodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return encodeText(rText, ePart, eMechanism, eCharset, false);
}

// static
inline OUString INetURLObject::decode(OUString const & rText,
                                       DecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return decode(rText.getStr(), rText.getStr() + rText.getLength(),
                  eMechanism, eCharset);
}

inline OUString INetURLObject::decode(OUStringBuffer const & rText,
                                       DecodeMechanism eMechanism,
                                       rtl_TextEncoding eCharset)
{
    return decode(rText.getStr(), rText.getStr() + rText.getLength(),
                  eMechanism, eCharset);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
