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

#ifndef INCLUDED_XMLREADER_XMLREADER_HXX
#define INCLUDED_XMLREADER_XMLREADER_HXX

#include <sal/config.h>

#include <stack>
#include <vector>

#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <osl/file.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <xmlreader/detail/xmlreaderdllapi.hxx>
#include <xmlreader/pad.hxx>
#include <xmlreader/span.hxx>

namespace xmlreader {

class OOO_DLLPUBLIC_XMLREADER XmlReader {
public:
    explicit XmlReader(OUString const & fileUrl);

    ~XmlReader();

    enum { NAMESPACE_NONE = -2, NAMESPACE_UNKNOWN = -1, NAMESPACE_XML = 0 };

    enum class Text { NONE, Raw, Normalized };

    enum class Result { Begin, End, Text, Done };

    int registerNamespaceIri(Span const & iri);

    // RESULT_BEGIN: data = localName, ns = ns
    // RESULT_END: data, ns unused
    // RESULT_TEXT: data = text, ns unused
    Result nextItem(Text reportText, Span * data, int * nsId);

    bool nextAttribute(int * nsId, Span * localName);

    // the span returned by getAttributeValue is only valid until the next call
    // to nextItem or getAttributeValue
    Span getAttributeValue(bool fullyNormalize);

    int getNamespaceId(Span const & prefix) const;

    const OUString& getUrl() const { return fileUrl_;}

private:
    XmlReader(const XmlReader&) = delete;
    XmlReader& operator=(const XmlReader&) = delete;

    typedef std::vector< Span > NamespaceIris;

    // If NamespaceData (and similarly ElementData and AttributeData) is made
    // SAL_DLLPRIVATE, at least gcc 4.2.3 erroneously warns about
    // "'xmlreader::XmlReader' declared with greater visibility than the type of
    // its field 'xmlreader::XmlReader::namespaces_'" (and similarly for
    // elements_ and attributes_):

    struct NamespaceData {
        Span const prefix;
        int const nsId;

        NamespaceData():
            nsId(-1) {}

        NamespaceData(Span const & thePrefix, int theNsId):
            prefix(thePrefix), nsId(theNsId) {}
    };

    typedef std::vector< NamespaceData > NamespaceList;

    struct ElementData {
        Span const name;
        NamespaceList::size_type const inheritedNamespaces;
        int const defaultNamespaceId;

        ElementData(
            Span const & theName,
            NamespaceList::size_type theInheritedNamespaces,
            int theDefaultNamespaceId):
            name(theName), inheritedNamespaces(theInheritedNamespaces),
            defaultNamespaceId(theDefaultNamespaceId)
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

    enum class State { Content, StartTag, EndTag, EmptyElementTag, Done };

    SAL_DLLPRIVATE char read() { return pos_ == end_ ? '\0' : *pos_++; }

    SAL_DLLPRIVATE char peek() const { return pos_ == end_ ? '\0' : *pos_; }

    SAL_DLLPRIVATE void normalizeLineEnds(Span const & text);

    SAL_DLLPRIVATE void skipSpace();

    SAL_DLLPRIVATE bool skipComment();

    SAL_DLLPRIVATE void skipProcessingInstruction();

    SAL_DLLPRIVATE void skipDocumentTypeDeclaration();

    SAL_DLLPRIVATE Span scanCdataSection();

    SAL_DLLPRIVATE bool scanName(char const ** nameColon);

    SAL_DLLPRIVATE int scanNamespaceIri(
        char const * begin, char const * end);

    SAL_DLLPRIVATE char const * handleReference(
        char const * position, char const * end);

    SAL_DLLPRIVATE Span handleAttributeValue(
        char const * begin, char const * end, bool fullyNormalize);

    SAL_DLLPRIVATE Result handleStartTag(int * nsId, Span * localName);

    SAL_DLLPRIVATE Result handleEndTag();

    SAL_DLLPRIVATE void handleElementEnd();

    SAL_DLLPRIVATE Result handleSkippedText(Span * data, int * nsId);

    SAL_DLLPRIVATE Result handleRawText(Span * text);

    SAL_DLLPRIVATE Result handleNormalizedText(Span * text);

    SAL_DLLPRIVATE static int toNamespaceId(NamespaceIris::size_type pos);

    OUString const fileUrl_;
    oslFileHandle fileHandle_;
    sal_uInt64 fileSize_;
    void * fileAddress_;
    NamespaceIris namespaceIris_;
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
