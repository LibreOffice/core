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

#include "xmlutil.hxx"

#include <cstring>

#include <expat.h>

namespace
{

// Expat is set up with '\n' as the namespace separator and triplet
// names, so a resolved name arrives as "uri\nlocal\nprefix", with the
// parts that do not apply left out.
void splitTriplet(const char* name, std::string& nsHref, std::string& localName,
                  std::string& prefix)
{
    const char* firstSep = std::strchr(name, '\n');
    if (!firstSep)
    {
        nsHref.clear();
        localName = name;
        prefix.clear();
        return;
    }
    nsHref.assign(name, firstSep - name);
    const char* secondSep = std::strchr(firstSep + 1, '\n');
    if (!secondSep)
    {
        localName = firstSep + 1;
        prefix.clear();
        return;
    }
    localName.assign(firstSep + 1, secondSep - (firstSep + 1));
    prefix = secondSep + 1;
}

struct ParseContext
{
    XML_Parser parser = nullptr;
    XmlNode* current = nullptr;
    std::vector<XmlNamespaceDecl> pendingNsDecls;
};

void XMLCALL onStartNamespaceDecl(void* userData, const XML_Char* prefix, const XML_Char* uri)
{
    auto* context = static_cast<ParseContext*>(userData);
    // The xml prefix is implicitly declared; carrying an explicit
    // declaration around would only clutter serialized output.
    if (prefix && std::strcmp(prefix, "xml") == 0)
        return;
    XmlNamespaceDecl decl;
    decl.prefix = prefix ? prefix : "";
    decl.href = uri ? uri : "";
    context->pendingNsDecls.push_back(std::move(decl));
}

void XMLCALL onStartElement(void* userData, const XML_Char* name, const XML_Char** attrs)
{
    auto* context = static_cast<ParseContext*>(userData);

    auto element = std::make_unique<XmlNode>();
    element->type = XmlNode::Type::Element;
    splitTriplet(name, element->nsHref, element->localName, element->prefix);
    element->line = static_cast<int>(XML_GetCurrentLineNumber(context->parser));
    element->nsDecls = std::move(context->pendingNsDecls);
    context->pendingNsDecls.clear();

    for (int i = 0; attrs[i]; i += 2)
    {
        XmlAttribute attribute;
        splitTriplet(attrs[i], attribute.nsHref, attribute.localName, attribute.prefix);
        attribute.value = attrs[i + 1];
        element->attributes.push_back(std::move(attribute));
    }

    element->parent = context->current;
    XmlNode* raw = element.get();
    context->current->children.push_back(std::move(element));
    context->current = raw;
}

void XMLCALL onEndElement(void* userData, const XML_Char*)
{
    auto* context = static_cast<ParseContext*>(userData);
    context->current = context->current->parent;
}

void XMLCALL onCharacterData(void* userData, const XML_Char* data, int length)
{
    auto* context = static_cast<ParseContext*>(userData);
    if (context->current->type == XmlNode::Type::Document)
        return; // whitespace between top-level constructs

    if (!context->current->children.empty() && context->current->children.back()->isText())
    {
        context->current->children.back()->text.append(data, length);
        return;
    }

    auto textNode = std::make_unique<XmlNode>();
    textNode->type = XmlNode::Type::Text;
    textNode->text.assign(data, length);
    textNode->parent = context->current;
    context->current->children.push_back(std::move(textNode));
}

void escapeInto(const std::string& value, bool inAttribute, std::string& out)
{
    for (char character : value)
    {
        switch (character)
        {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                if (inAttribute)
                {
                    out += "&quot;";
                    break;
                }
                [[fallthrough]];
            default:
                out += character;
        }
    }
}

void serializeInto(const XmlNode* node, std::string& out)
{
    if (node->type == XmlNode::Type::Document)
    {
        for (const auto& child : node->children)
            serializeInto(child.get(), out);
        return;
    }

    if (node->isText())
    {
        escapeInto(node->text, false, out);
        return;
    }

    out += "<" + qualifiedName(node);
    for (const XmlNamespaceDecl& decl : node->nsDecls)
    {
        out += decl.prefix.empty() ? " xmlns" : " xmlns:" + decl.prefix;
        out += "=\"";
        escapeInto(decl.href, true, out);
        out += "\"";
    }
    for (const XmlAttribute& attribute : node->attributes)
    {
        out += " " + attribute.qualifiedName() + "=\"";
        escapeInto(attribute.value, true, out);
        out += "\"";
    }

    if (node->children.empty())
    {
        out += "/>";
        return;
    }

    out += ">";
    for (const auto& child : node->children)
        serializeInto(child.get(), out);
    out += "</" + qualifiedName(node) + ">";
}

} // namespace

XmlNode* XmlNode::rootElement() const
{
    for (const auto& child : children)
        if (child->isElement())
            return child.get();
    return nullptr;
}

std::unique_ptr<XmlNode> XmlNode::removeChild(size_t index)
{
    std::unique_ptr<XmlNode> removed = std::move(children[index]);
    children.erase(children.begin() + index);
    removed->parent = nullptr;
    return removed;
}

void XmlNode::unwrap()
{
    XmlNode* container = parent;
    size_t position = 0;
    while (position < container->children.size() && container->children[position].get() != this)
        ++position;

    std::vector<std::unique_ptr<XmlNode>> moved;
    moved.swap(children);
    for (auto& child : moved)
        child->parent = container;

    container->children.insert(container->children.begin() + position + 1,
                               std::make_move_iterator(moved.begin()),
                               std::make_move_iterator(moved.end()));
    container->children.erase(container->children.begin() + position);
}

XmlDocPtr parseXmlMemory(const std::string& bytes, const std::string& documentName,
                         std::string& errorMessage)
{
    XML_Parser parser = XML_ParserCreateNS(nullptr, '\n');
    if (!parser)
    {
        errorMessage = "cannot create XML parser";
        return nullptr;
    }
    XML_SetReturnNSTriplet(parser, 1);

    auto document = std::make_unique<XmlNode>();
    document->type = XmlNode::Type::Document;

    ParseContext context;
    context.parser = parser;
    context.current = document.get();

    XML_SetUserData(parser, &context);
    XML_SetElementHandler(parser, onStartElement, onEndElement);
    XML_SetCharacterDataHandler(parser, onCharacterData);
    XML_SetNamespaceDeclHandler(parser, onStartNamespaceDecl, nullptr);

    XML_Status status
        = XML_Parse(parser, bytes.data(), static_cast<int>(bytes.size()), 1 /* final */);
    if (status != XML_STATUS_OK)
    {
        errorMessage = XML_ErrorString(XML_GetErrorCode(parser));
        errorMessage += " (" + documentName + " line "
                        + std::to_string(XML_GetCurrentLineNumber(parser)) + ")";
        XML_ParserFree(parser);
        return nullptr;
    }

    XML_ParserFree(parser);
    return document;
}

std::string serializeXml(const XmlNode* node)
{
    std::string out;
    serializeInto(node, out);
    return out;
}

std::string qualifiedName(const XmlNode* node)
{
    return node->prefix.empty() ? node->localName : node->prefix + ":" + node->localName;
}

std::string namespaceOf(const XmlNode* node) { return node->nsHref; }

std::string localNameOf(const XmlNode* node) { return node->localName; }

bool getNsAttribute(const XmlNode* element, const char* nsHref, const char* localName,
                    std::string& value)
{
    for (const XmlAttribute& attribute : element->attributes)
    {
        if (attribute.localName == localName
            && ((nsHref == nullptr && attribute.nsHref.empty()) || (nsHref && attribute.nsHref == nsHref)))
        {
            value = attribute.value;
            return true;
        }
    }
    return false;
}

std::string directTextContent(const XmlNode* element)
{
    std::string text;
    for (const auto& child : element->children)
        if (child->isText())
            text += child->text;
    return text;
}

std::string resolvePrefix(const XmlNode* element, const std::string& prefix)
{
    for (const XmlNode* scope = element; scope && scope->isElement(); scope = scope->parent)
        for (const XmlNamespaceDecl& decl : scope->nsDecls)
            if (decl.prefix == prefix)
                return decl.href;
    return std::string();
}

std::string prefixForNamespace(const XmlNode* element, const std::string& nsHref)
{
    for (const XmlNode* scope = element; scope && scope->isElement(); scope = scope->parent)
        for (const XmlNamespaceDecl& decl : scope->nsDecls)
            if (decl.href == nsHref && !decl.prefix.empty())
                return decl.prefix;
    return std::string();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
