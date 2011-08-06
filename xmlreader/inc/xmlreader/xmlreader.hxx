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

#ifndef INCLUDED_XMLREADER_XMLREADER_HXX
#define INCLUDED_XMLREADER_XMLREADER_HXX

#include "sal/config.h"

#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/container/NoSuchElementException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "osl/file.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "xmlreader/detail/xmlreaderdllapi.hxx"
#include "xmlreader/pad.hxx"
#include "xmlreader/span.hxx"

namespace xmlreader {

class OOO_DLLPUBLIC_XMLREADER XmlReader: private boost::noncopyable {
public:
    explicit XmlReader(rtl::OUString const & fileUrl)
        SAL_THROW((
            com::sun::star::container::NoSuchElementException,
            com::sun::star::uno::RuntimeException));

    ~XmlReader();

    enum { NAMESPACE_NONE = -2, NAMESPACE_UNKNOWN = -1, NAMESPACE_XML = 0 };

    enum Text { TEXT_NONE, TEXT_RAW, TEXT_NORMALIZED };

    enum Result { RESULT_BEGIN, RESULT_END, RESULT_TEXT, RESULT_DONE };

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

    rtl::OUString getUrl() const;

private:
    typedef std::vector< Span > NamespaceIris;

    // If NamespaceData (and similarly ElementData and AttributeData) is made
    // SAL_DLLPRIVATE, at least gcc 4.2.3 erroneously warns about
    // "'xmlreader::XmlReader' declared with greater visibility than the type of
    // its field 'xmlreader::XmlReader::namespaces_'" (and similarly for
    // elements_ and attributes_):

    struct NamespaceData {
        Span prefix;
        int nsId;

        NamespaceData() {}

        NamespaceData(Span const & thePrefix, int theNsId):
            prefix(thePrefix), nsId(theNsId) {}
    };

    typedef std::vector< NamespaceData > NamespaceList;

    struct ElementData {
        Span name;
        NamespaceList::size_type inheritedNamespaces;
        int defaultNamespaceId;

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

    enum State {
        STATE_CONTENT, STATE_START_TAG, STATE_END_TAG, STATE_EMPTY_ELEMENT_TAG,
        STATE_DONE };

    SAL_DLLPRIVATE inline char read() { return pos_ == end_ ? '\0' : *pos_++; }

    SAL_DLLPRIVATE inline char peek() { return pos_ == end_ ? '\0' : *pos_; }

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

    SAL_DLLPRIVATE int toNamespaceId(NamespaceIris::size_type pos);

    rtl::OUString fileUrl_;
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
