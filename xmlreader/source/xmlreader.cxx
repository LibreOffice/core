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

#include "sal/config.h"

#include <cassert>
#include <climits>
#include <cstddef>

#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/file.h"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "xmlreader/pad.hxx"
#include "xmlreader/span.hxx"
#include "xmlreader/xmlreader.hxx"

namespace xmlreader {

namespace {

namespace css = com::sun::star;

bool isSpace(char c) {
    switch (c) {
    case '\x09':
    case '\x0A':
    case '\x0D':
    case ' ':
        return true;
    default:
        return false;
    }
}

}

XmlReader::XmlReader(rtl::OUString const & fileUrl)
    SAL_THROW((
        css::container::NoSuchElementException, css::uno::RuntimeException)):
    fileUrl_(fileUrl)
{
    oslFileError e = osl_openFile(
        fileUrl_.pData, &fileHandle_, osl_File_OpenFlag_Read);
    switch (e)
    {
    case osl_File_E_None:
        break;
    case osl_File_E_NOENT:
        throw css::container::NoSuchElementException(
            fileUrl_, css::uno::Reference< css::uno::XInterface >());
    default:
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot open ")) +
             fileUrl_ + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(": ")) +
             rtl::OUString::valueOf(static_cast< sal_Int32 >(e))),
            css::uno::Reference< css::uno::XInterface >());
    }
    e = osl_getFileSize(fileHandle_, &fileSize_);
    if (e == osl_File_E_None) {
        e = osl_mapFile(
            fileHandle_, &fileAddress_, fileSize_, 0,
            osl_File_MapFlag_WillNeed);
    }
    if (e != osl_File_E_None) {
        oslFileError e2 = osl_closeFile(fileHandle_);
        if (e2 != osl_File_E_None) {
            SAL_WARN(
                "xmlreader",
                "osl_closeFile of \"" << fileUrl_ << "\" failed with " << +e2);
        }
        throw css::uno::RuntimeException(
            ("cannot mmap " + fileUrl_ + " (" +
             rtl::OUString::valueOf(static_cast< sal_Int32 >(e)) + ")"),
            css::uno::Reference< css::uno::XInterface >());
    }
    init();
    pos_ = static_cast< char * >(fileAddress_);
    end_ = pos_ + fileSize_;
}

XmlReader::XmlReader(const char * str, sal_uInt64 len)
    SAL_THROW(())
    : fileHandle_(NULL)
    , fileSize_(len)
    , fileAddress_(NULL)
{
    init();
    pos_ = str;
    end_ = pos_ + fileSize_;
}

void XmlReader::init()
{
    namespaceIris_.push_back(
        Span(
            RTL_CONSTASCII_STRINGPARAM(
                "http://www.w3.org/XML/1998/namespace")));
    namespaces_.push_back(
        NamespaceData(Span(RTL_CONSTASCII_STRINGPARAM("xml")), NAMESPACE_XML));
    state_ = STATE_CONTENT;
    firstAttribute_ = true;
}

XmlReader::~XmlReader() {
    if (!fileHandle_)
        return;

    oslFileError e = osl_unmapMappedFile(fileHandle_, fileAddress_, fileSize_);
    if (e != osl_File_E_None) {
        SAL_WARN(
            "xmlreader",
            "osl_unmapMappedFile of \"" << fileUrl_ << "\" failed with " << +e);
    }
    e = osl_closeFile(fileHandle_);
    if (e != osl_File_E_None) {
        SAL_WARN(
            "xmlreader",
            "osl_closeFile of \"" << fileUrl_ << "\" failed with " << +e);
    }
}

int XmlReader::registerNamespaceIri(Span const & iri) {
    int id = toNamespaceId(namespaceIris_.size());
    namespaceIris_.push_back(iri);
    if (iri.equals(
            Span(
                RTL_CONSTASCII_STRINGPARAM(
                    "http://www.w3.org/2001/XMLSchema-instance"))))
    {
        // Old user layer .xcu files used the xsi namespace prefix without
        // declaring a corresponding namespace binding, see issue 77174; reading
        // those files during migration would fail without this hack that can be
        // removed once migration is no longer relevant (see
        // configmgr::Components::parseModificationLayer):
        namespaces_.push_back(
            NamespaceData(Span(RTL_CONSTASCII_STRINGPARAM("xsi")), id));
    }
    return id;
}

XmlReader::Result XmlReader::nextItem(Text reportText, Span * data, int * nsId)
{
    switch (state_) {
    case STATE_CONTENT:
        switch (reportText) {
        case TEXT_NONE:
            return handleSkippedText(data, nsId);
        case TEXT_RAW:
            return handleRawText(data);
        case TEXT_NORMALIZED:
            return handleNormalizedText(data);
        }
    case STATE_START_TAG:
        return handleStartTag(nsId, data);
    case STATE_END_TAG:
        return handleEndTag();
    case STATE_EMPTY_ELEMENT_TAG:
        handleElementEnd();
        return RESULT_END;
    default: // STATE_DONE
        return RESULT_DONE;
    }
}

bool XmlReader::nextAttribute(int * nsId, Span * localName) {
    assert(nsId != 0 && localName != 0);
    if (firstAttribute_) {
        currentAttribute_ = attributes_.begin();
        firstAttribute_ = false;
    } else {
        ++currentAttribute_;
    }
    if (currentAttribute_ == attributes_.end()) {
        return false;
    }
    if (currentAttribute_->nameColon == 0) {
        *nsId = NAMESPACE_NONE;
        *localName = Span(
            currentAttribute_->nameBegin,
            currentAttribute_->nameEnd - currentAttribute_->nameBegin);
    } else {
        *nsId = getNamespaceId(
            Span(
                currentAttribute_->nameBegin,
                currentAttribute_->nameColon - currentAttribute_->nameBegin));
        *localName = Span(
            currentAttribute_->nameColon + 1,
            currentAttribute_->nameEnd - (currentAttribute_->nameColon + 1));
    }
    return true;
}

Span XmlReader::getAttributeValue(bool fullyNormalize) {
    return handleAttributeValue(
        currentAttribute_->valueBegin, currentAttribute_->valueEnd,
        fullyNormalize);
}

int XmlReader::getNamespaceId(Span const & prefix) const {
    for (NamespaceList::const_reverse_iterator i(namespaces_.rbegin());
         i != namespaces_.rend(); ++i)
    {
        if (prefix.equals(i->prefix)) {
            return i->nsId;
        }
    }
    return NAMESPACE_UNKNOWN;
}

rtl::OUString XmlReader::getUrl() const {
    return fileUrl_;
}

void XmlReader::normalizeLineEnds(Span const & text) {
    char const * p = text.begin;
    sal_Int32 n = text.length;
    for (;;) {
        sal_Int32 i = rtl_str_indexOfChar_WithLength(p, n, '\x0D');
        if (i < 0) {
            break;
        }
        pad_.add(p, i);
        p += i + 1;
        n -= i + 1;
        if (n == 0 || *p != '\x0A') {
            pad_.add(RTL_CONSTASCII_STRINGPARAM("\x0A"));
        }
    }
    pad_.add(p, n);
}

void XmlReader::skipSpace() {
    while (isSpace(peek())) {
        ++pos_;
    }
}

bool XmlReader::skipComment() {
    if (rtl_str_shortenedCompare_WithLength(
            pos_, end_ - pos_, RTL_CONSTASCII_STRINGPARAM("--"),
            RTL_CONSTASCII_LENGTH("--")) !=
        0)
    {
        return false;
    }
    pos_ += RTL_CONSTASCII_LENGTH("--");
    sal_Int32 i = rtl_str_indexOfStr_WithLength(
        pos_, end_ - pos_, RTL_CONSTASCII_STRINGPARAM("--"));
    if (i < 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "premature end (within comment) of ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    pos_ += i + RTL_CONSTASCII_LENGTH("--");
    if (read() != '>') {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "illegal \"--\" within comment in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    return true;
}

void XmlReader::skipProcessingInstruction() {
    sal_Int32 i = rtl_str_indexOfStr_WithLength(
        pos_, end_ - pos_, RTL_CONSTASCII_STRINGPARAM("?>"));
    if (i < 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad '<?' in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    pos_ += i + RTL_CONSTASCII_LENGTH("?>");
}

void XmlReader::skipDocumentTypeDeclaration() {
    // Neither is it checked that the doctypedecl is at the correct position in
    // the document, nor that it is well-formed:
    for (;;) {
        char c = read();
        switch (c) {
        case '\0': // i.e., EOF
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "premature end (within DTD) of ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        case '"':
        case '\'':
            {
                sal_Int32 i = rtl_str_indexOfChar_WithLength(
                    pos_, end_ - pos_, c);
                if (i < 0) {
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "premature end (within DTD) of ")) +
                         fileUrl_),
                        css::uno::Reference< css::uno::XInterface >());
                }
                pos_ += i + 1;
            }
            break;
        case '>':
            return;
        case '[':
            for (;;) {
                c = read();
                switch (c) {
                case '\0': // i.e., EOF
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "premature end (within DTD) of ")) +
                         fileUrl_),
                        css::uno::Reference< css::uno::XInterface >());
                case '"':
                case '\'':
                    {
                        sal_Int32 i = rtl_str_indexOfChar_WithLength(
                            pos_, end_ - pos_, c);
                        if (i < 0) {
                            throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "premature end (within DTD) of ")) +
                             fileUrl_),
                            css::uno::Reference< css::uno::XInterface >());
                        }
                        pos_ += i + 1;
                    }
                    break;
                case '<':
                    switch (read()) {
                    case '\0': // i.e., EOF
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "premature end (within DTD) of ")) +
                             fileUrl_),
                            css::uno::Reference< css::uno::XInterface >());
                    case '!':
                        skipComment();
                        break;
                    case '?':
                        skipProcessingInstruction();
                        break;
                    default:
                        break;
                    }
                    break;
                case ']':
                    skipSpace();
                    if (read() != '>') {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "missing \">\" of DTD in ")) +
                             fileUrl_),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    return;
                default:
                    break;
                }
            }
        default:
            break;
        }
    }
}

Span XmlReader::scanCdataSection() {
    if (rtl_str_shortenedCompare_WithLength(
            pos_, end_ - pos_, RTL_CONSTASCII_STRINGPARAM("[CDATA["),
            RTL_CONSTASCII_LENGTH("[CDATA[")) !=
        0)
    {
        return Span();
    }
    pos_ += RTL_CONSTASCII_LENGTH("[CDATA[");
    char const * begin = pos_;
    sal_Int32 i = rtl_str_indexOfStr_WithLength(
        pos_, end_ - pos_, RTL_CONSTASCII_STRINGPARAM("]]>"));
    if (i < 0) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "premature end (within CDATA section) of ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    pos_ += i + RTL_CONSTASCII_LENGTH("]]>");
    return Span(begin, i);
}

bool XmlReader::scanName(char const ** nameColon) {
    assert(nameColon != 0 && *nameColon == 0);
    for (char const * begin = pos_;; ++pos_) {
        switch (peek()) {
        case '\0': // i.e., EOF
        case '\x09':
        case '\x0A':
        case '\x0D':
        case ' ':
        case '/':
        case '=':
        case '>':
            return pos_ != begin;
        case ':':
            *nameColon = pos_;
            break;
        default:
            break;
        }
    }
}

int XmlReader::scanNamespaceIri(char const * begin, char const * end) {
    assert(begin != 0 && begin <= end);
    Span iri(handleAttributeValue(begin, end, false));
    for (NamespaceIris::size_type i = 0; i < namespaceIris_.size(); ++i) {
        if (namespaceIris_[i].equals(iri)) {
            return toNamespaceId(i);
        }
    }
    return XmlReader::NAMESPACE_UNKNOWN;
}

char const * XmlReader::handleReference(char const * position, char const * end)
{
    assert(position != 0 && *position == '&' && position < end);
    ++position;
    if (*position == '#') {
        ++position;
        sal_Int32 val = 0;
        char const * p;
        if (*position == 'x') {
            ++position;
            p = position;
            for (;; ++position) {
                char c = *position;
                if (c >= '0' && c <= '9') {
                    val = 16 * val + (c - '0');
                } else if (c >= 'A' && c <= 'F') {
                    val = 16 * val + (c - 'A') + 10;
                } else if (c >= 'a' && c <= 'f') {
                    val = 16 * val + (c - 'a') + 10;
                } else {
                    break;
                }
                if (val > 0x10FFFF) { // avoid overflow
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "'&#x...' too large in ")) +
                         fileUrl_),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
        } else {
            p = position;
            for (;; ++position) {
                char c = *position;
                if (c >= '0' && c <= '9') {
                    val = 10 * val + (c - '0');
                } else {
                    break;
                }
                if (val > 0x10FFFF) { // avoid overflow
                    throw css::uno::RuntimeException(
                        (rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "'&#...' too large in ")) +
                         fileUrl_),
                        css::uno::Reference< css::uno::XInterface >());
                }
            }
        }
        if (position == p || *position++ != ';') {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("'&#...' missing ';' in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        assert(val >= 0 && val <= 0x10FFFF);
        if ((val < 0x20 && val != 0x9 && val != 0xA && val != 0xD) ||
            (val >= 0xD800 && val <= 0xDFFF) || val == 0xFFFE || val == 0xFFFF)
        {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "character reference denoting invalid character in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        char buf[4];
        sal_Int32 len;
        if (val < 0x80) {
            buf[0] = static_cast< char >(val);
            len = 1;
        } else if (val < 0x800) {
            buf[0] = static_cast< char >((val >> 6) | 0xC0);
            buf[1] = static_cast< char >((val & 0x3F) | 0x80);
            len = 2;
        } else if (val < 0x10000) {
            buf[0] = static_cast< char >((val >> 12) | 0xE0);
            buf[1] = static_cast< char >(((val >> 6) & 0x3F) | 0x80);
            buf[2] = static_cast< char >((val & 0x3F) | 0x80);
            len = 3;
        } else {
            buf[0] = static_cast< char >((val >> 18) | 0xF0);
            buf[1] = static_cast< char >(((val >> 12) & 0x3F) | 0x80);
            buf[2] = static_cast< char >(((val >> 6) & 0x3F) | 0x80);
            buf[3] = static_cast< char >((val & 0x3F) | 0x80);
            len = 4;
        }
        pad_.addEphemeral(buf, len);
        return position;
    } else {
        struct EntityRef {
            char const * inBegin;
            sal_Int32 inLength;
            char const * outBegin;
            sal_Int32 outLength;
        };
        static EntityRef const refs[] = {
            { RTL_CONSTASCII_STRINGPARAM("amp;"),
              RTL_CONSTASCII_STRINGPARAM("&") },
            { RTL_CONSTASCII_STRINGPARAM("lt;"),
              RTL_CONSTASCII_STRINGPARAM("<") },
            { RTL_CONSTASCII_STRINGPARAM("gt;"),
              RTL_CONSTASCII_STRINGPARAM(">") },
            { RTL_CONSTASCII_STRINGPARAM("apos;"),
              RTL_CONSTASCII_STRINGPARAM("'") },
            { RTL_CONSTASCII_STRINGPARAM("quot;"),
              RTL_CONSTASCII_STRINGPARAM("\"") } };
        for (std::size_t i = 0; i < sizeof refs / sizeof refs[0]; ++i) {
            if (rtl_str_shortenedCompare_WithLength(
                    position, end - position, refs[i].inBegin, refs[i].inLength,
                    refs[i].inLength) ==
                0)
            {
                position += refs[i].inLength;
                pad_.add(refs[i].outBegin, refs[i].outLength);
                return position;
            }
        }
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("unknown entity reference in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
}

Span XmlReader::handleAttributeValue(
    char const * begin, char const * end, bool fullyNormalize)
{
    pad_.clear();
    if (fullyNormalize) {
        while (begin != end && isSpace(*begin)) {
            ++begin;
        }
        while (end != begin && isSpace(end[-1])) {
            --end;
        }
        char const * p = begin;
        enum Space { SPACE_NONE, SPACE_SPAN, SPACE_BREAK };
            // a single true space character can go into the current span,
            // everything else breaks the span
        Space space = SPACE_NONE;
        while (p != end) {
            switch (*p) {
            case '\x09':
            case '\x0A':
            case '\x0D':
                switch (space) {
                case SPACE_NONE:
                    pad_.add(begin, p - begin);
                    pad_.add(RTL_CONSTASCII_STRINGPARAM(" "));
                    space = SPACE_BREAK;
                    break;
                case SPACE_SPAN:
                    pad_.add(begin, p - begin);
                    space = SPACE_BREAK;
                    break;
                case SPACE_BREAK:
                    break;
                }
                begin = ++p;
                break;
            case ' ':
                switch (space) {
                case SPACE_NONE:
                    ++p;
                    space = SPACE_SPAN;
                    break;
                case SPACE_SPAN:
                    pad_.add(begin, p - begin);
                    begin = ++p;
                    space = SPACE_BREAK;
                    break;
                case SPACE_BREAK:
                    begin = ++p;
                    break;
                }
                break;
            case '&':
                pad_.add(begin, p - begin);
                p = handleReference(p, end);
                begin = p;
                space = SPACE_NONE;
                break;
            default:
                ++p;
                space = SPACE_NONE;
                break;
            }
        }
        pad_.add(begin, p - begin);
    } else {
        char const * p = begin;
        while (p != end) {
            switch (*p) {
            case '\x09':
            case '\x0A':
                pad_.add(begin, p - begin);
                begin = ++p;
                pad_.add(RTL_CONSTASCII_STRINGPARAM(" "));
                break;
            case '\x0D':
                pad_.add(begin, p - begin);
                ++p;
                if (peek() == '\x0A') {
                    ++p;
                }
                begin = p;
                pad_.add(RTL_CONSTASCII_STRINGPARAM(" "));
                break;
            case '&':
                pad_.add(begin, p - begin);
                p = handleReference(p, end);
                begin = p;
                break;
            default:
                ++p;
                break;
            }
        }
        pad_.add(begin, p - begin);
    }
    return pad_.get();
}

XmlReader::Result XmlReader::handleStartTag(int * nsId, Span * localName) {
    assert(nsId != 0 && localName);
    char const * nameBegin = pos_;
    char const * nameColon = 0;
    if (!scanName(&nameColon)) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("bad tag name in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    char const * nameEnd = pos_;
    NamespaceList::size_type inheritedNamespaces = namespaces_.size();
    bool hasDefaultNs = false;
    int defaultNsId = NAMESPACE_NONE;
    attributes_.clear();
    for (;;) {
        char const * p = pos_;
        skipSpace();
        if (peek() == '/' || peek() == '>') {
            break;
        }
        if (pos_ == p) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "missing whitespace before attribute in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        char const * attrNameBegin = pos_;
        char const * attrNameColon = 0;
        if (!scanName(&attrNameColon)) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad attribute name in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        char const * attrNameEnd = pos_;
        skipSpace();
        if (read() != '=') {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("missing '=' in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        skipSpace();
        char del = read();
        if (del != '\'' && del != '"') {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad attribute value in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        char const * valueBegin = pos_;
        sal_Int32 i = rtl_str_indexOfChar_WithLength(pos_, end_ - pos_, del);
        if (i < 0) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "unterminated attribute value in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        char const * valueEnd = pos_ + i;
        pos_ += i + 1;
        if (attrNameColon == 0 &&
            Span(attrNameBegin, attrNameEnd - attrNameBegin).equals(
                RTL_CONSTASCII_STRINGPARAM("xmlns")))
        {
            hasDefaultNs = true;
            defaultNsId = scanNamespaceIri(valueBegin, valueEnd);
        } else if (attrNameColon != 0 &&
                   Span(attrNameBegin, attrNameColon - attrNameBegin).equals(
                       RTL_CONSTASCII_STRINGPARAM("xmlns")))
        {
            namespaces_.push_back(
                NamespaceData(
                    Span(attrNameColon + 1, attrNameEnd - (attrNameColon + 1)),
                    scanNamespaceIri(valueBegin, valueEnd)));
        } else {
            attributes_.push_back(
                AttributeData(
                    attrNameBegin, attrNameEnd, attrNameColon, valueBegin,
                    valueEnd));
        }
    }
    if (!hasDefaultNs && !elements_.empty()) {
        defaultNsId = elements_.top().defaultNamespaceId;
    }
    firstAttribute_ = true;
    if (peek() == '/') {
        state_ = STATE_EMPTY_ELEMENT_TAG;
        ++pos_;
    } else {
        state_ = STATE_CONTENT;
    }
    if (peek() != '>') {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("missing '>' in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    ++pos_;
    elements_.push(
        ElementData(
            Span(nameBegin, nameEnd - nameBegin), inheritedNamespaces,
            defaultNsId));
    if (nameColon == 0) {
        *nsId = defaultNsId;
        *localName = Span(nameBegin, nameEnd - nameBegin);
    } else {
        *nsId = getNamespaceId(Span(nameBegin, nameColon - nameBegin));
        *localName = Span(nameColon + 1, nameEnd - (nameColon + 1));
    }
    return RESULT_BEGIN;
}

XmlReader::Result XmlReader::handleEndTag() {
    if (elements_.empty()) {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("spurious end tag in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    char const * nameBegin = pos_;
    char const * nameColon = 0;
    if (!scanName(&nameColon) ||
        !elements_.top().name.equals(nameBegin, pos_ - nameBegin))
    {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tag mismatch in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    handleElementEnd();
    skipSpace();
    if (peek() != '>') {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("missing '>' in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    ++pos_;
    return RESULT_END;
}

void XmlReader::handleElementEnd() {
    assert(!elements_.empty());
    namespaces_.resize(elements_.top().inheritedNamespaces);
    elements_.pop();
    state_ = elements_.empty() ? STATE_DONE : STATE_CONTENT;
}

XmlReader::Result XmlReader::handleSkippedText(Span * data, int * nsId) {
    for (;;) {
        sal_Int32 i = rtl_str_indexOfChar_WithLength(pos_, end_ - pos_, '<');
        if (i < 0) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("premature end of ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        pos_ += i + 1;
        switch (peek()) {
        case '!':
            ++pos_;
            if (!skipComment() && !scanCdataSection().is()) {
                skipDocumentTypeDeclaration();
            }
            break;
        case '/':
            ++pos_;
            return handleEndTag();
        case '?':
            ++pos_;
            skipProcessingInstruction();
            break;
        default:
            return handleStartTag(nsId, data);
        }
    }
}

XmlReader::Result XmlReader::handleRawText(Span * text) {
    pad_.clear();
    for (char const * begin = pos_;;) {
        switch (peek()) {
        case '\0': // i.e., EOF
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("premature end of ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        case '\x0D':
            pad_.add(begin, pos_ - begin);
            ++pos_;
            if (peek() != '\x0A') {
                pad_.add(RTL_CONSTASCII_STRINGPARAM("\x0A"));
            }
            begin = pos_;
            break;
        case '&':
            pad_.add(begin, pos_ - begin);
            pos_ = handleReference(pos_, end_);
            begin = pos_;
            break;
        case '<':
            pad_.add(begin, pos_ - begin);
            ++pos_;
            switch (peek()) {
            case '!':
                ++pos_;
                if (!skipComment()) {
                    Span cdata(scanCdataSection());
                    if (cdata.is()) {
                        normalizeLineEnds(cdata);
                    } else {
                        skipDocumentTypeDeclaration();
                    }
                }
                begin = pos_;
                break;
            case '/':
                *text = pad_.get();
                ++pos_;
                state_ = STATE_END_TAG;
                return RESULT_TEXT;
            case '?':
                ++pos_;
                skipProcessingInstruction();
                begin = pos_;
                break;
            default:
                *text = pad_.get();
                state_ = STATE_START_TAG;
                return RESULT_TEXT;
            }
            break;
        default:
            ++pos_;
            break;
        }
    }
}

XmlReader::Result XmlReader::handleNormalizedText(Span * text) {
    pad_.clear();
    char const * flowBegin = pos_;
    char const * flowEnd = pos_;
    enum Space { SPACE_START, SPACE_NONE, SPACE_SPAN, SPACE_BREAK };
        // a single true space character can go into the current flow,
        // everything else breaks the flow
    Space space = SPACE_START;
    for (;;) {
        switch (peek()) {
        case '\0': // i.e., EOF
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("premature end of ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        case '\x09':
        case '\x0A':
        case '\x0D':
            switch (space) {
            case SPACE_START:
            case SPACE_BREAK:
                break;
            case SPACE_NONE:
            case SPACE_SPAN:
                space = SPACE_BREAK;
                break;
            }
            ++pos_;
            break;
        case ' ':
            switch (space) {
            case SPACE_START:
            case SPACE_BREAK:
                break;
            case SPACE_NONE:
                space = SPACE_SPAN;
                break;
            case SPACE_SPAN:
                space = SPACE_BREAK;
                break;
            }
            ++pos_;
            break;
        case '&':
            switch (space) {
            case SPACE_START:
                break;
            case SPACE_NONE:
            case SPACE_SPAN:
                pad_.add(flowBegin, pos_ - flowBegin);
                break;
            case SPACE_BREAK:
                pad_.add(flowBegin, flowEnd - flowBegin);
                pad_.add(RTL_CONSTASCII_STRINGPARAM(" "));
                break;
            }
            pos_ = handleReference(pos_, end_);
            flowBegin = pos_;
            flowEnd = pos_;
            space = SPACE_NONE;
            break;
        case '<':
            ++pos_;
            switch (peek()) {
            case '!':
                ++pos_;
                if (skipComment()) {
                    space = SPACE_BREAK;
                } else {
                    Span cdata(scanCdataSection());
                    if (cdata.is()) {
                        // CDATA is not normalized (similar to character
                        // references; it keeps the code simple), but it might
                        // arguably be better to normalize it:
                        switch (space) {
                        case SPACE_START:
                            break;
                        case SPACE_NONE:
                        case SPACE_SPAN:
                            pad_.add(flowBegin, pos_ - flowBegin);
                            break;
                        case SPACE_BREAK:
                            pad_.add(flowBegin, flowEnd - flowBegin);
                            pad_.add(RTL_CONSTASCII_STRINGPARAM(" "));
                            break;
                        }
                        normalizeLineEnds(cdata);
                        flowBegin = pos_;
                        flowEnd = pos_;
                        space = SPACE_NONE;
                    } else {
                        skipDocumentTypeDeclaration();
                    }
                }
                break;
            case '/':
                ++pos_;
                pad_.add(flowBegin, flowEnd - flowBegin);
                *text = pad_.get();
                state_ = STATE_END_TAG;
                return RESULT_TEXT;
            case '?':
                ++pos_;
                skipProcessingInstruction();
                space = SPACE_BREAK;
                break;
            default:
                pad_.add(flowBegin, flowEnd - flowBegin);
                *text = pad_.get();
                state_ = STATE_START_TAG;
                return RESULT_TEXT;
            }
            break;
        default:
            switch (space) {
            case SPACE_START:
                flowBegin = pos_;
                break;
            case SPACE_NONE:
            case SPACE_SPAN:
                break;
            case SPACE_BREAK:
                pad_.add(flowBegin, flowEnd - flowBegin);
                pad_.add(RTL_CONSTASCII_STRINGPARAM(" "));
                flowBegin = pos_;
                break;
            }
            flowEnd = ++pos_;
            space = SPACE_NONE;
            break;
        }
    }
}

int XmlReader::toNamespaceId(NamespaceIris::size_type pos) {
    assert(pos <= INT_MAX);
    return static_cast< int >(pos);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
