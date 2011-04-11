/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
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
*
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_SOURCE_XMLREADER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_XMLREADER_HXX

#include "sal/config.h"

#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/file.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "pad.hxx"
#include "span.hxx"

namespace configmgr {

class XmlReader: private boost::noncopyable {
public:
    explicit XmlReader(rtl::OUString const & fileUrl)
        SAL_THROW((
            com::sun::star::container::NoSuchElementException,
            com::sun::star::uno::RuntimeException));

    ~XmlReader();

    enum Namespace {
        NAMESPACE_NONE, NAMESPACE_XML, NAMESPACE_OOR, NAMESPACE_XS,
        NAMESPACE_XSI, NAMESPACE_OTHER };

    enum Text { TEXT_NONE, TEXT_RAW, TEXT_NORMALIZED };

    enum Result { RESULT_BEGIN, RESULT_END, RESULT_TEXT, RESULT_DONE };

    // RESULT_BEGIN: data = localName, ns = ns
    // RESULT_END: data, ns unused
    // RESULT_TEXT: data = text, ns unused
    Result nextItem(Text reportText, Span * data, Namespace * ns);

    bool nextAttribute(Namespace * ns, Span * localName);

    // the span returned by getAttributeValue is only valid until the next call
    // to nextItem or getAttributeValue
    Span getAttributeValue(bool fullyNormalize);

    Namespace getNamespace(Span const & prefix) const;

    rtl::OUString getUrl() const;

private:
    inline char read() { return pos_ == end_ ? '\0' : *pos_++; }
    inline char peek() { return pos_ == end_ ? '\0' : *pos_; }

    void normalizeLineEnds(Span const & text);

    void skipSpace();
    bool skipComment();
    void skipProcessingInstruction();
    void skipDocumentTypeDeclaration();

    Span scanCdataSection();

    bool scanName(char const ** nameColon);

    Namespace scanNamespaceIri(char const * begin, char const * end);

    char const * handleReference(char const * position, char const * end);

    Span handleAttributeValue(
        char const * begin, char const * end, bool fullyNormalize);

    Result handleStartTag(Namespace * ns, Span * localName);
    Result handleEndTag();

    void handleElementEnd();

    Result handleSkippedText(Span * data, Namespace * ns);
    Result handleRawText(Span * text);
    Result handleNormalizedText(Span * text);

    struct NamespaceData {
        Span prefix;
        Namespace ns;

        NamespaceData() {}

        NamespaceData(Span const & thePrefix, Namespace theNs):
            prefix(thePrefix), ns(theNs) {}
    };

    typedef std::vector< NamespaceData > NamespaceList;

    struct ElementData {
        Span name;
        NamespaceList::size_type inheritedNamespaces;
        Namespace defaultNamespace;

        ElementData(
            Span const & theName,
            NamespaceList::size_type theInheritedNamespaces,
            Namespace theDefaultNamespace):
            name(theName), inheritedNamespaces(theInheritedNamespaces),
            defaultNamespace(theDefaultNamespace)
        {}
    };

    typedef std::stack< ElementData > ElementStack;

    struct AttributeData {
        char const * nameBegin;
        char const * nameEnd;
        char const * nameColon;
        char const * valueBegin;
        char const * valueEnd;

        AttributeData(
            char const * theNameBegin, char const * theNameEnd,
            char const * theNameColon, char const * theValueBegin,
            char const * theValueEnd):
            nameBegin(theNameBegin), nameEnd(theNameEnd),
            nameColon(theNameColon), valueBegin(theValueBegin),
            valueEnd(theValueEnd)
        {}
    };

    typedef std::vector< AttributeData > Attributes;

    enum State {
        STATE_CONTENT, STATE_START_TAG, STATE_END_TAG, STATE_EMPTY_ELEMENT_TAG,
        STATE_DONE };

    rtl::OUString fileUrl_;
    oslFileHandle fileHandle_;
    sal_uInt64 fileSize_;
    void * fileAddress_;
    NamespaceList namespaces_;
    ElementStack elements_;
    char const * pos_;
    char const * end_;
    State state_;
    Attributes attributes_;
    Attributes::iterator currentAttribute_;
    bool firstAttribute_;
    Pad pad_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
