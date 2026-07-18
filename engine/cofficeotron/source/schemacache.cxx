/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2010-2011 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "schemacache.hxx"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

extern "C" {
#include <rn.h>
#include <rnl.h>
#include <rnv.h>
#include <rnx.h>
#include <drv.h>
#include <dxl.h>
#include <dsl.h>
#include <er.h>
}

namespace
{

// RNV reports everything through the er_printf hooks; the messages are
// collected here and attached to whatever operation is running.
std::string* messageCollector = nullptr;

int collectPrintf(char* format, ...)
{
    if (!messageCollector)
        return 0;
    va_list ap;
    va_start(ap, format);
    char buffer[1024];
    int n = vsnprintf(buffer, sizeof(buffer), format, ap);
    va_end(ap);
    if (n > 0)
        messageCollector->append(buffer);
    return n;
}

int collectVprintf(char* format, va_list ap)
{
    if (!messageCollector)
        return 0;
    char buffer[1024];
    int n = vsnprintf(buffer, sizeof(buffer), format, ap);
    if (n > 0)
        messageCollector->append(buffer);
    return n;
}

// One line per message: internal newlines separate the message from the
// lists of expected names, which read better joined up.
std::string collapse(const std::string& text)
{
    std::string out;
    for (char character : text)
    {
        if (character == '\n' || character == '\t')
        {
            if (!out.empty() && out.back() != ' ')
                out += ' ';
        }
        else
        {
            out += character;
        }
    }
    while (!out.empty() && out.back() == ' ')
        out.pop_back();
    return out;
}

// The walk that is currently validating, for the error handler.
struct ValidationWalk
{
    ErrorCapper* capper = nullptr;
    int line = 0;
    int current = 0;
    int previous = 0;
    bool mixed = false;
    int skipDepth = 0;
    std::string pendingText;
};

ValidationWalk* activeWalk = nullptr;

void rnvErrorHandler(int erno, va_list ap)
{
    std::string message;
    std::string* outer = messageCollector;
    messageCollector = &message;
    rnv_default_verror_handler(erno, ap);
    messageCollector = outer;

    if (activeWalk && activeWalk->capper)
        activeWalk->capper->error(activeWalk->line, 0, collapse(message));
}

std::string* schemaLoadErrors = nullptr;

void rnlErrorHandler(int erno, va_list ap)
{
    std::string message;
    std::string* outer = messageCollector;
    messageCollector = &message;
    rnl_default_verror_handler(erno, ap);
    messageCollector = outer;

    if (schemaLoadErrors)
    {
        if (!schemaLoadErrors->empty())
            *schemaLoadErrors += "; ";
        *schemaLoadErrors += collapse(message);
    }
}

// RNV takes element and attribute names in the form the expat bindings
// produce: the namespace URI and the local name joined by a colon, or
// the bare local name outside any namespace.
std::string rnvName(const std::string& nsHref, const std::string& localName)
{
    return nsHref.empty() ? localName : nsHref + ":" + localName;
}

void flushText(ValidationWalk& walk)
{
    rnv_text(&walk.current, &walk.previous, walk.pendingText.data(),
             static_cast<int>(walk.pendingText.size()), walk.mixed ? 1 : 0);
    walk.pendingText.clear();
}

void walkElement(ValidationWalk& walk, const XmlNode* element)
{
    std::string name = rnvName(element->nsHref, element->localName);

    std::vector<std::string> attributeStorage;
    for (const XmlAttribute& attribute : element->attributes)
    {
        attributeStorage.push_back(rnvName(attribute.nsHref, attribute.localName));
        attributeStorage.push_back(attribute.value);
    }
    std::vector<char*> attributes;
    for (std::string& item : attributeStorage)
        attributes.push_back(item.data());
    attributes.push_back(nullptr);

    if (walk.current != rn_notAllowed)
    {
        walk.line = element->line;
        walk.mixed = true;
        flushText(walk);
        rnv_start_tag(&walk.current, &walk.previous, name.data(), attributes.data());
        walk.mixed = false;
    }
    else
    {
        ++walk.skipDepth;
    }

    for (const auto& child : element->children)
    {
        if (child->isElement())
            walkElement(walk, child.get());
        else if (child->isText() && walk.current != rn_notAllowed)
            walk.pendingText += child->text;
    }

    if (walk.current != rn_notAllowed)
    {
        flushText(walk);
        rnv_end_tag(&walk.current, &walk.previous, name.data());
        walk.mixed = true;
    }
    else
    {
        // Inside an invalid element nothing more is checked; the state
        // from before the element resumes when it ends.
        if (walk.skipDepth == 0)
            walk.current = walk.previous;
        else
            --walk.skipDepth;
    }
}

bool isDirectory(const std::string& path)
{
    struct stat status;
    return ::stat(path.c_str(), &status) == 0 && S_ISDIR(status.st_mode);
}

} // namespace

void initValidation()
{
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;

    er_printf = collectPrintf;
    er_vprintf = collectVprintf;

    rnl_init();
    rnl_verror_handler = rnlErrorHandler;
    rnv_init();
    rnv_verror_handler = rnvErrorHandler;
    rnx_init();

    // The datatype libraries for W3C XML Schema datatypes.
    drv_add_dtl(const_cast<char *>(DXL_URL), &dxl_equal, &dxl_allows);
    drv_add_dtl(const_cast<char *>(DSL_URL), &dsl_equal, &dsl_allows);
}

SchemaCache::SchemaCache(const std::string& schemaDirectory)
    : schemaDir(schemaDirectory)
{
}

int SchemaCache::getSchema(const std::string& relativePath, std::string& error)
{
    auto cached = startByPath.find(relativePath);
    if (cached != startByPath.end())
    {
        if (!cached->second)
            error = failureByPath[relativePath];
        return cached->second;
    }

    std::string fullPath = schemaDir + "/" + relativePath;

    std::string loadErrors;
    schemaLoadErrors = &loadErrors;
    int start = rnl_fn(fullPath.data());
    schemaLoadErrors = nullptr;

    startByPath[relativePath] = start;
    if (!start)
    {
        error = "cannot compile schema " + fullPath
                + (loadErrors.empty() ? "" : ": " + loadErrors);
        failureByPath[relativePath] = error;
    }
    return start;
}

bool validateElement(const XmlNode* element, int startPattern, ErrorCapper& capper)
{
    ValidationWalk walk;
    walk.capper = &capper;
    walk.current = startPattern;
    walk.previous = startPattern;
    walk.line = element->line;

    ValidationWalk* outer = activeWalk;
    activeWalk = &walk;
    int errorsBefore = capper.getInstanceErrCount();
    walkElement(walk, element);
    activeWalk = outer;

    return capper.getInstanceErrCount() == errorsBefore;
}

std::string locateSchemaDirectory(const std::string& override_)
{
    if (!override_.empty())
        return override_;

    const char* fromEnvironment = std::getenv("COFFICEOTRON_SCHEMA_DIR");
    if (fromEnvironment && *fromEnvironment)
        return fromEnvironment;

    // Resolve locations relative to the executable so the tool works no
    // matter where it is invoked from.
    char linkTarget[4096];
    ssize_t length = ::readlink("/proc/self/exe", linkTarget, sizeof(linkTarget) - 1);
    if (length > 0)
    {
        linkTarget[length] = '\0';
        std::string executablePath(linkTarget);
        size_t slash = executablePath.rfind('/');
        if (slash != std::string::npos)
        {
            std::string executableDir = executablePath.substr(0, slash);
            for (const char* suffix : { "/etc/schema", "/../etc/schema",
                                        "/../share/cofficeotron/schema" })
            {
                std::string candidate = executableDir + suffix;
                if (isDirectory(candidate))
                    return candidate;
            }
        }
    }

    if (isDirectory("etc/schema"))
        return "etc/schema";

    return std::string();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
