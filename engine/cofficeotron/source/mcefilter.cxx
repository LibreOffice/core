/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2011 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mcefilter.hxx"

#include <cctype>
#include <vector>

namespace
{
const char* const NS_MCE = "http://schemas.openxmlformats.org/markup-compatibility/2006";
const char* const NS_XML = "http://www.w3.org/XML/1998/namespace";
const char* const TAG_XML = "xml";
const char* const TAG_ALTERNATE_CONTENT = "AlternateContent";
const char* const TAG_CHOICE = "Choice";
const char* const TAG_FALLBACK = "Fallback";

// One entry of an mc:ProcessContent list, with the prefix resolved to a
// namespace URI (the empty string when the name had no prefix or the
// prefix was not bound). A localName of "*" matches any local name.
struct ProcessContentTarget
{
    std::string nsHref;
    std::string localName;
};

// State recorded for each open element while walking the tree.
struct McScope
{
    std::string nsHref;
    std::string localName;
    std::vector<std::string> ignorablePrefixes;
    std::vector<ProcessContentTarget> processContentTargets;
    bool ignoreContent = false;
    bool processContent = false;
    bool isChoice = false;
};

std::vector<std::string> splitOnSpace(const std::string& value)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (start < value.size())
    {
        size_t space = value.find(' ', start);
        if (space == std::string::npos)
        {
            parts.push_back(value.substr(start));
            break;
        }
        parts.push_back(value.substr(start, space - start));
        start = space + 1;
    }
    return parts;
}

std::vector<std::string> currentIgnorables(const std::vector<McScope>& stack)
{
    std::vector<std::string> ignorables;
    for (const McScope& scope : stack)
    {
        for (const std::string& prefix : scope.ignorablePrefixes)
        {
            bool present = false;
            for (const std::string& existing : ignorables)
                if (existing == prefix)
                    present = true;
            if (!present)
                ignorables.push_back(prefix);
        }
    }
    return ignorables;
}

bool isIgnorable(const std::string& qualified, const std::vector<std::string>& ignorables,
                 bool vmlStream)
{
    if (vmlStream && qualified == TAG_XML)
        return true;

    for (const std::string& prefix : ignorables)
    {
        if (qualified.size() > prefix.size() + 1 && qualified.compare(0, prefix.size(), prefix) == 0
            && qualified[prefix.size()] == ':')
        {
            return true;
        }
    }
    return false;
}

bool isProcessContent(const std::vector<McScope>& stack, const std::string& nsHref,
                      const std::string& localName)
{
    for (const McScope& scope : stack)
    {
        for (const ProcessContentTarget& target : scope.processContentTargets)
        {
            if (target.nsHref == nsHref
                && (target.localName == localName || target.localName == "*"))
            {
                return true;
            }
        }
    }
    return false;
}

// Walks the open scopes from the outside in: entering an ignorable
// element turns suppression on, a matching ProcessContent declaration
// turns it back off for that element's content, and the content of
// mc:Choice is always suppressed (this validator understands no
// extension namespace, so it always takes the Fallback branch).
bool isInIgnoredContent(const std::vector<McScope>& stack)
{
    bool ignored = false;
    for (const McScope& scope : stack)
    {
        if (scope.ignoreContent)
            ignored = true;
        if (ignored && scope.processContent)
            ignored = false;
        if (scope.isChoice)
            ignored = true;
    }
    return ignored;
}

// Checks the structural constraints on elements in the MCE namespace.
// Returns true when the element is in that namespace.
bool checkMCEElement(const XmlNode* element, const std::vector<McScope>& stack, ErrorCapper& errors)
{
    bool mceElement = element->nsHref == NS_MCE;
    if (!mceElement)
        return false;

    const std::string& localName = element->localName;
    bool validName = localName == TAG_ALTERNATE_CONTENT || localName == TAG_CHOICE
                     || localName == TAG_FALLBACK;
    if (!validName)
        errors.error(element->line, 0, "Invalid MCE element: " + localName);

    std::string mcePrefix = prefixForNamespace(element, NS_MCE);

    if (localName != TAG_ALTERNATE_CONTENT)
    {
        if (stack.empty())
        {
            errors.error(element->line, 0, "Invalid root element: " + mcePrefix + ":" + localName);
        }
        else
        {
            const McScope& parent = stack.back();
            if (parent.nsHref != NS_MCE || parent.localName != TAG_ALTERNATE_CONTENT)
            {
                errors.error(element->line, 0,
                             "Parent of " + mcePrefix + ":" + localName + " element should be "
                                 + mcePrefix + ":" + TAG_ALTERNATE_CONTENT);
            }
        }
    }

    bool isChoice = localName == TAG_CHOICE;
    std::string requiresValue;
    bool haveRequires = false;

    for (const XmlAttribute& attribute : element->attributes)
    {
        if (attribute.nsHref.empty() && !(isChoice && attribute.localName == "Requires"))
        {
            std::string message;
            if (isChoice)
                message = "Invalid attribute " + attribute.localName + " with no namespace in "
                          + mcePrefix + ":Choice element";
            else
                message = mcePrefix + ":" + localName
                          + " element shouldn't have any attribute with no namespace";
            errors.error(element->line, 0, message);
        }
        else if (attribute.nsHref == NS_XML
                 && (attribute.localName == "lang" || attribute.localName == "space"))
        {
            errors.error(element->line, 0,
                         "xml:" + attribute.localName + " is not allowed in " + mcePrefix + ":"
                             + localName + " element");
        }

        if (isChoice)
        {
            if (attribute.nsHref.empty() && attribute.localName == "Requires")
            {
                haveRequires = true;
                requiresValue = attribute.value;
            }
            else if (attribute.nsHref == NS_MCE && attribute.localName == "Requires")
            {
                errors.error(element->line, 0,
                             mcePrefix
                                 + ":Choice element shouldn't have a Requires element with the "
                                 + mcePrefix + " prefix");
            }
        }
    }

    if (isChoice && (!haveRequires || requiresValue.empty()))
    {
        errors.error(element->line, 0,
                     mcePrefix + ":Choice element needs a Requires attribute with no namespace");
    }

    return mceElement;
}

void transformElement(XmlNode* element, std::vector<McScope>& stack, bool vmlStream,
                      ErrorCapper& errors)
{
    std::string qualified = qualifiedName(element);

    bool mceElement = checkMCEElement(element, stack, errors);

    // The element-name check fires a second time for the end tag, so a
    // misnamed MCE element is reported twice.
    if (mceElement && element->localName != TAG_ALTERNATE_CONTENT
        && element->localName != TAG_CHOICE && element->localName != TAG_FALLBACK)
    {
        errors.error(element->line, 0, "Invalid MCE element: " + element->localName);
    }

    McScope scope;
    scope.nsHref = element->nsHref;
    scope.localName = element->localName;

    std::string attributeValue;
    if (getNsAttribute(element, NS_MCE, "Ignorable", attributeValue))
        scope.ignorablePrefixes = splitOnSpace(MCENormalizeWhitespaces(attributeValue));

    if (getNsAttribute(element, NS_MCE, "ProcessContent", attributeValue))
    {
        for (const std::string& item : splitOnSpace(MCENormalizeWhitespaces(attributeValue)))
        {
            size_t colon = item.find(':');
            if (colon != std::string::npos && item.find(':', colon + 1) != std::string::npos)
                continue; // more than two segments: not a QName

            ProcessContentTarget target;
            if (colon == std::string::npos)
            {
                target.localName = item;
            }
            else
            {
                target.localName = item.substr(colon + 1);
                target.nsHref = resolvePrefix(element, item.substr(0, colon));
            }
            scope.processContentTargets.push_back(target);
        }
    }

    std::vector<std::string> ignorables = currentIgnorables(stack);

    bool ignoreFirstVmlElement = false;
    if (vmlStream && stack.empty())
    {
        if (qualified != TAG_XML)
        {
            errors.error(element->line, 0, "VML streams need to start with <xml>: 8.1 (Part 4)");
        }
        else
        {
            ignoreFirstVmlElement = true;
        }
    }

    bool ignoreThis = ignoreFirstVmlElement || isIgnorable(qualified, ignorables, vmlStream);
    scope.ignoreContent = ignoreThis;
    scope.processContent = isProcessContent(stack, element->nsHref, element->localName);
    scope.isChoice = mceElement && element->localName == TAG_CHOICE;

    bool emit = !ignoreThis && !isInIgnoredContent(stack) && !mceElement;

    if (emit)
    {
        std::vector<XmlAttribute> kept;
        for (const XmlAttribute& attribute : element->attributes)
        {
            bool remove = isIgnorable(attribute.qualifiedName(), ignorables, vmlStream);
            if (!remove && attribute.nsHref == NS_MCE)
                remove = true;
            if (!remove)
                kept.push_back(attribute);
        }
        element->attributes.swap(kept);
    }

    stack.push_back(scope);

    for (size_t index = 0; index < element->children.size();)
    {
        XmlNode* child = element->children[index].get();
        size_t sizeBefore = element->children.size();
        if (child->isElement())
        {
            transformElement(child, stack, vmlStream, errors);
            // An unwrapped child was replaced by its own, already
            // transformed, content; continue after it.
            index += 1 + (element->children.size() - sizeBefore);
        }
        else if (isInIgnoredContent(stack))
        {
            element->removeChild(index);
        }
        else
        {
            ++index;
        }
    }

    stack.pop_back();

    if (!emit)
    {
        // The tag goes away but the surviving content stays in place.
        element->unwrap();
    }
}

} // namespace

std::string MCENormalizeWhitespaces(const std::string& value)
{
    std::string normalized;
    bool pendingSpace = false;
    for (char character : value)
    {
        if (std::isspace(static_cast<unsigned char>(character)))
        {
            pendingSpace = !normalized.empty();
        }
        else
        {
            if (pendingSpace)
                normalized += ' ';
            pendingSpace = false;
            normalized += character;
        }
    }
    return normalized;
}

void MCEFilter(XmlNode* document, bool vmlStream, ErrorCapper& errors)
{
    XmlNode* root = document->rootElement();
    if (!root)
        return;

    std::vector<McScope> stack;
    transformElement(root, stack, vmlStream, errors);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
