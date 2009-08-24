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

#ifndef INCLUDED_CONFIGMGR_XMLREADER_HXX
#define INCLUDED_CONFIGMGR_XMLREADER_HXX

#include "sal/config.h"

#include <stack>
#include <vector>

#include "boost/noncopyable.hpp"
#include "osl/file.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "span.hxx"

namespace configmgr {

class XmlReader: private boost::noncopyable {
public:
    explicit XmlReader(rtl::OUString const & fileUrl);

    ~XmlReader();

    enum Namespace {
        NAMESPACE_NONE, NAMESPACE_XML, NAMESPACE_OOR, NAMESPACE_XS,
        NAMESPACE_XSI, NAMESPACE_OTHER };

    enum Result { RESULT_BEGIN, RESULT_END, RESULT_DONE };

    // Result: _BEGIN, _END, _DONE
    Result nextTag(
        Namespace * ns, Span * localName,
        void (* textHandler)(void * userData, Span const & text, bool terminal),
        void * userData);

    bool nextAttribute(Namespace * ns, Span * localName);

    Span getAttributeValue(bool fullyNormalize);

    Namespace getNamespace(Span const & prefix) const;

    rtl::OUString getUrl() const;

private:
    inline char read() { return pos_ == end_ ? '\0' : *pos_++; }

    inline char peek() { return pos_ == end_ ? '\0' : *pos_; }

    void padAppend(char const * begin, sal_Int32 length, bool terminal);

    void skipSpace();

    bool skipProcessingInstruction();

    bool scanName(char const ** nameColon);

    Namespace scanNamespaceIri(char const * begin, char const * end);

    char const * handleReference(
        char const * begin, char const * position, char const * end);

    void handleAttributeValue(
        char const * begin, char const * end, bool fullyNormalize);

    Result handleStartTag(Namespace * ns, Span * localName);

    Result handleEndTag();

    void handleElementEnd();

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
        STATE_START, STATE_EMPTY_ELEMENT_TAG, STATE_CONTENT, STATE_DONE };

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
    rtl::OStringBuffer padBuffer_;
    Span pad_;
};

}

#endif
