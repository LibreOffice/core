/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef OFFICEOTRON_XMLUTIL_HXX
#define OFFICEOTRON_XMLUTIL_HXX

#include <memory>
#include <string>
#include <vector>

// A small document tree parsed with expat. Element and text nodes carry
// everything the validator needs: namespace-resolved names, attributes
// in document order, the namespace declarations written on the element,
// and the line number the element started on.

struct XmlAttribute
{
    std::string nsHref;
    std::string prefix;
    std::string localName;
    std::string value;

    // The attribute name as written in the document.
    std::string qualifiedName() const
    {
        return prefix.empty() ? localName : prefix + ":" + localName;
    }
};

struct XmlNamespaceDecl
{
    std::string prefix; // empty for the default namespace
    std::string href;
};

class XmlNode
{
public:
    enum class Type
    {
        Document,
        Element,
        Text
    };

    Type type = Type::Element;

    // Element fields.
    std::string nsHref;
    std::string prefix;
    std::string localName;
    std::vector<XmlAttribute> attributes;
    std::vector<XmlNamespaceDecl> nsDecls;
    int line = 0;

    // Text content, for text nodes.
    std::string text;

    XmlNode* parent = nullptr;
    std::vector<std::unique_ptr<XmlNode>> children;

    bool isElement() const { return type == Type::Element; }
    bool isText() const { return type == Type::Text; }

    // The first element child of a document is its root.
    XmlNode* rootElement() const;

    // Removes the child at the given index and returns it.
    std::unique_ptr<XmlNode> removeChild(size_t index);

    // Replaces this element in its parent by its own children, in place.
    void unwrap();
};

using XmlDocPtr = std::unique_ptr<XmlNode>;

// Parses a byte buffer as namespace-aware XML. External entities are
// not resolved. On failure returns null and stores the parser message
// in errorMessage.
XmlDocPtr parseXmlMemory(const std::string& bytes, const std::string& documentName,
                         std::string& errorMessage);

// Serializes a document or element without an XML declaration.
std::string serializeXml(const XmlNode* node);

// The element name as written in the document: "prefix:local" when the
// element's namespace has a prefix, plain "local" otherwise.
std::string qualifiedName(const XmlNode* node);

// The namespace URI of an element, or the empty string when it has none.
std::string namespaceOf(const XmlNode* node);

std::string localNameOf(const XmlNode* node);

// Returns true and stores the value when the element carries the
// attribute localName in namespace nsHref (null nsHref matches an
// attribute with no namespace).
bool getNsAttribute(const XmlNode* element, const char* nsHref, const char* localName,
                    std::string& value);

// The concatenated direct text children of an element.
std::string directTextContent(const XmlNode* element);

// Resolves a namespace prefix against the declarations in scope at the
// given element. Returns the empty string for an unbound prefix.
std::string resolvePrefix(const XmlNode* element, const std::string& prefix);

// Finds a prefix bound to the given namespace URI in scope at the
// element, or the empty string.
std::string prefixForNamespace(const XmlNode* element, const std::string& nsHref);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
