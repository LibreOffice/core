/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include <cstddef>

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "span.hxx"
#include "xmlreader.hxx"

namespace configmgr {

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

XmlReader::XmlReader(rtl::OUString const & fileUrl):
    fileUrl_(fileUrl)
{
    osl::File f(fileUrl_);
    osl::FileBase::RC e = f.open(OpenFlag_Read);
    if (e != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot open ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    sal_uInt64 size;
    e = f.getSize(size);
    if (e != osl::FileBase::E_None) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot get size of ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    fileData_.reset(new char[size]);
    sal_uInt64 read;
    e = f.read(fileData_.get(), size, read);
        //TODO: use private mmap (modified by write to *end_ below) instead
    if (e != osl::FileBase::E_None || read != size) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("cannot read ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    namespaces_.push_back(
        NamespaceData(Span(RTL_CONSTASCII_STRINGPARAM("xml")), NAMESPACE_XML));
    pos_ = fileData_.get();
    end_ = pos_ + size;
    while (end_ != pos_ && isSpace(end_[-1])) {
        --end_;
    }
    if (end_ == pos_ || *--end_ != '>') {
        throw css::uno::RuntimeException(
            (rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("not well-formed XML ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    *const_cast< char * >(end_) = '\0';
    state_ = STATE_START;
}

XmlReader::~XmlReader() {}

XmlReader::Result XmlReader::nextTag(
    Namespace * ns, Span * localName,
    void (* textHandler)(void * userData, Span const & text, bool terminal),
    void * userData)
{
    for (;;) {
        switch (state_) {
        case STATE_START:
            switch (*pos_++) {
            case '\x09':
            case '\x0A':
            case '\x0D':
            case ' ':
                break;
            case '<':
                switch (*pos_) {
                case '!':
                    if(true)abort();*(char*)0=0;throw 0;//TODO
                case '?':
                    ++pos_;
                    if (!skipProcessingInstruction()) {
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("bad '<?' in ")) +
                             fileUrl_),
                            css::uno::Reference< css::uno::XInterface >());
                    }
                    break;
                default:
                    return handleStartTag(ns, localName);
                }
                break;
            default:
                throw css::uno::RuntimeException(
                    (rtl::OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "bad character at start of ")) +
                     fileUrl_),
                    css::uno::Reference< css::uno::XInterface >());
            }
            break;
        case STATE_EMPTY_ELEMENT_TAG:
            handleElementEnd();
            return RESULT_END;
        case STATE_CONTENT:
            {
                padBuffer_.setLength(0);
                char const * begin = pos_;
                for (;;) {
                    switch (*pos_) {
                    case '\0': // i.e., '>'
                        throw css::uno::RuntimeException(
                            (rtl::OUString(
                                RTL_CONSTASCII_USTRINGPARAM("bad '>' in ")) +
                             fileUrl_),
                            css::uno::Reference< css::uno::XInterface >());
                    case '&':
                        pos_ = handleReference(begin, pos_, end_);
                        begin = pos_;
                        break;
                    case '<':
                        {
                            sal_Int32 len = pos_ - begin;
                            ++pos_;
                            bool endTag = false;
                            if (*pos_ == '/') {
                                ++pos_;
                                endTag = true;
                            }
                            if (textHandler != 0) {
                                padAppend(begin, len, true);
                                (*textHandler)(userData, pad_, endTag);
                            }
                            return endTag
                                ? handleEndTag()
                                : handleStartTag(ns, localName);
                        }
                    default:
                        ++pos_;
                        break;
                    }
                }
            }
        case STATE_DONE:
            return RESULT_DONE;
        }
    }
}

bool XmlReader::nextAttribute(Namespace * ns, Span * localName) {
    OSL_ASSERT(ns != 0 && localName != 0);
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
        *ns = NAMESPACE_NONE;
        *localName = Span(
            currentAttribute_->nameBegin,
            currentAttribute_->nameEnd - currentAttribute_->nameBegin);
    } else {
        *ns = getNamespace(
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
    handleAttributeValue(
        currentAttribute_->valueBegin, currentAttribute_->valueEnd,
        fullyNormalize);
    return pad_;
}

XmlReader::Namespace XmlReader::getNamespace(Span const & prefix) const {
    for (NamespaceList::const_reverse_iterator i(namespaces_.rbegin());
         i != namespaces_.rend(); ++i)
    {
        if (prefix.equals(i->prefix)) {
            return i->ns;
        }
    }
    return NAMESPACE_OTHER;
}

rtl::OUString XmlReader::getUrl() const {
    return fileUrl_;
}

void XmlReader::padAppend(char const * begin, sal_Int32 length, bool terminal) {
    if (terminal && padBuffer_.getLength() == 0) {
        pad_ = Span(begin, length);
    } else {
        padBuffer_.append(begin, length);
        if (terminal) {
            pad_ = Span(padBuffer_.getStr(), padBuffer_.getLength());
        }
    }
}

void XmlReader::skipSpace() {
    while (isSpace(*pos_)) {
        ++pos_;
    }
}

bool XmlReader::skipProcessingInstruction() {
    sal_Int32 i = rtl_str_indexOfStr(pos_, "?>");
    if (i < 0) {
        return false;
    }
    pos_ += i + RTL_CONSTASCII_LENGTH("?>");
    return true;
}

bool XmlReader::scanName(char const ** nameColon) {
    OSL_ASSERT(nameColon != 0 && *nameColon == 0);
    for (char const * begin = pos_;; ++pos_) {
        switch (*pos_) {
        case '\0': // i.e., '>'
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

XmlReader::Namespace XmlReader::scanNamespaceIri(
    char const * begin, char const * end)
{
    OSL_ASSERT(begin != 0 && begin <= end);
    handleAttributeValue(begin, end, false);
    struct Iri {
        char const * begin;
        sal_Int32 length;
        XmlReader::Namespace ns;
    };
    static Iri const iris[] = {
        { RTL_CONSTASCII_STRINGPARAM("http://openoffice.org/2001/registry"),
          XmlReader::NAMESPACE_OOR },
        { RTL_CONSTASCII_STRINGPARAM("http://www.w3.org/2001/XMLSchema"),
          XmlReader::NAMESPACE_XS },
        { RTL_CONSTASCII_STRINGPARAM(
                "http://www.w3.org/2001/XMLSchema-instance"),
          XmlReader::NAMESPACE_XSI } };
    for (std::size_t i = 0; i < sizeof iris / sizeof iris[0]; ++i) {
        if (rtl_str_shortenedCompare_WithLength(
                pad_.begin, pad_.length, iris[i].begin, iris[i].length,
                iris[i].length) ==
            0)
        {
            return iris[i].ns;
        }
    }
    return XmlReader::NAMESPACE_OTHER;
}

char const * XmlReader::handleReference(
    char const * begin, char const * position, char const * end)
{
    OSL_ASSERT(
        begin != 0 && begin <= position && *position == '&' &&
        position < end);
    padAppend(begin, position - begin, false);
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
        OSL_ASSERT(val >= 0 && val <= 0x10FFFF);
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
        padAppend(buf, len, position == end);
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
                padAppend(refs[i].outBegin, refs[i].outLength, position == end);
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

void XmlReader::handleAttributeValue(
    char const * begin, char const * end, bool /*TODO fullyNormalize*/)
{
    padBuffer_.setLength(0);
    char const * p = begin;
    while (p != end) {
        switch (*p) {
        case '\x09':
        case '\x0A':
        case '\x0D':
            padAppend(begin, p - begin, false);
            begin = ++p;
            padAppend(RTL_CONSTASCII_STRINGPARAM(" "), p == end);
            break;
        case '&':
            p = handleReference(begin, p, end);
            begin = p;
            break;
        default:
            ++p;
            break;
        }
    }
    padAppend(begin, p - begin, true);
}

XmlReader::Result XmlReader::handleStartTag(Namespace * ns, Span * localName) {
    OSL_ASSERT(ns != 0 && localName);
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
    Namespace defaultNs = NAMESPACE_NONE;
    attributes_.clear();
    for (;;) {
        skipSpace();
        if (*pos_ == '/' || *pos_ == '>') {
            break;
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
        if (*pos_++ != '=') {
            throw css::uno::RuntimeException(
                (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("missing '=' in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        skipSpace();
        char del = *pos_++;
        if (del != '\'' && del != '"') {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("bad attribute value in ")) +
                 fileUrl_),
                css::uno::Reference< css::uno::XInterface >());
        }
        char const * valueBegin = pos_;
        sal_Int32 i = rtl_str_indexOfChar(pos_, del);
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
            defaultNs = scanNamespaceIri(valueBegin, valueEnd);
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
        defaultNs = elements_.top().defaultNamespace;
    }
    firstAttribute_ = true;
    if (*pos_ == '/') {
        state_ = STATE_EMPTY_ELEMENT_TAG;
        ++pos_;
    } else {
        state_ = STATE_CONTENT;
    }
    if (*pos_ != '>' && (pos_ != end_ || state_ != STATE_EMPTY_ELEMENT_TAG)) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("missing '>' in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    ++pos_;
    elements_.push(
        ElementData(
            Span(nameBegin, nameEnd - nameBegin), inheritedNamespaces,
            defaultNs));
    if (nameColon == 0) {
        *ns = defaultNs;
        *localName = Span(nameBegin, nameEnd - nameBegin);
    } else {
        *ns = getNamespace(Span(nameBegin, nameColon - nameBegin));
        *localName = Span(nameColon + 1, nameEnd - (nameColon + 1));
    }
    return RESULT_BEGIN;
}

XmlReader::Result XmlReader::handleEndTag() {
    OSL_ASSERT(!elements_.empty());
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
    if (*pos_ != '>' && (pos_ != end_ || state_ != STATE_DONE)) {
        throw css::uno::RuntimeException(
            (rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("missing '>' in ")) +
             fileUrl_),
            css::uno::Reference< css::uno::XInterface >());
    }
    ++pos_;
    return RESULT_END;
}

void XmlReader::handleElementEnd() {
    OSL_ASSERT(!elements_.empty());
    namespaces_.resize(elements_.top().inheritedNamespaces);
    elements_.pop();
    state_ = elements_.empty() ? STATE_DONE : STATE_CONTENT;
}

}
