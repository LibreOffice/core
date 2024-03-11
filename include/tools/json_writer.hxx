/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>

#include <tools/toolsdllapi.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <string>
#include <string_view>
#include <utility>

/** Simple JSON encoder designed specifically for LibreOfficeKit purposes.
 *
 * (1) Minimal allocations/re-allocations/copying
 * (2) Small/simple JSON documents
 * (3) ascii property names
 */
namespace tools
{
class ScopedJsonWriterNode;
class ScopedJsonWriterArray;
class ScopedJsonWriterStruct;

class TOOLS_DLLPUBLIC JsonWriter
{
    friend class ScopedJsonWriterNode;
    friend class ScopedJsonWriterArray;
    friend class ScopedJsonWriterStruct;

    char* mpBuffer;
    char* mPos;
    int mSpaceAllocated;
    int mStartNodeCount;
    bool mbFirstFieldInNode;

public:
    JsonWriter();
    ~JsonWriter();

    [[nodiscard]] ScopedJsonWriterNode startNode(const char*);
    [[nodiscard]] ScopedJsonWriterArray startArray(const char*);
    [[nodiscard]] ScopedJsonWriterArray startAnonArray();
    [[nodiscard]] ScopedJsonWriterStruct startStruct();

    void put(const OUString& pPropName, const OUString& rPropValue);

    void put(const char* pPropName, const OUString& rPropValue);
    // Assumes utf-8 property value encoding
    void put(const char* pPropName, std::string_view rPropValue);
    void put(const char* pPropName, const char* pPropVal)
    {
        put(pPropName, std::string_view(pPropVal));
    }

    void put(const char* pPropName, sal_uInt16 nPropVal) { put(pPropName, sal_Int64(nPropVal)); }
    void put(const char* pPropName, sal_Int16 nPropVal) { put(pPropName, sal_Int64(nPropVal)); }
    void put(const char* pPropName, sal_Int32 nPropVal) { put(pPropName, sal_Int64(nPropVal)); }
    void put(const char* pPropName, sal_uInt32 nPropVal) { put(pPropName, sal_Int64(nPropVal)); }
    void put(const char* pPropName, sal_Int64);
    void put(const char* pPropName, bool);
    void put(const char* pPropName, double);

    void putSimpleValue(const OUString& rPropValue);

    /// This assumes that this data belongs at this point in the stream, and is valid, and properly encoded
    void putRaw(std::string_view);

    /** Hands ownership of the underlying storage buffer to the caller,
     * after this no more document modifications may be written. */
    char* extractData() { return extractDataImpl().first; }
    OString extractAsOString();
    std::string extractAsStdString();

    /** returns true if the current JSON data matches the string */
    bool isDataEquals(const std::string&) const;

private:
    void startAnonBlock(const char cType);
    void endNode();
    void endArray();
    void endStruct();
    void addCommaBeforeField();
    void writeEscapedOUString(const OUString& rPropVal);
    std::pair<char*, int> extractDataImpl();
    void ensureSpace(int noMoreBytesRequired);

    // overflow validation in debug mode
    static constexpr unsigned char JSON_WRITER_DEBUG_MARKER = 0xde;

    inline void addValidationMark()
    {
#ifndef NDEBUG
        *(mpBuffer + mSpaceAllocated - 1) = JSON_WRITER_DEBUG_MARKER;
#endif
    }

    inline void validate()
    {
#ifndef NDEBUG
        unsigned char c = *(mpBuffer + mSpaceAllocated - 1);
        assert(c == JSON_WRITER_DEBUG_MARKER);
#endif
    }
};

/**
 * Auto-closes the node.
 */
class ScopedJsonWriterNode
{
    friend class JsonWriter;

    JsonWriter& mrWriter;

    ScopedJsonWriterNode(JsonWriter& rWriter)
        : mrWriter(rWriter)
    {
    }

public:
    ~ScopedJsonWriterNode() { mrWriter.endNode(); }
};

/**
 * Auto-closes the node.
 */
class ScopedJsonWriterArray
{
    friend class JsonWriter;

    JsonWriter& mrWriter;

    ScopedJsonWriterArray(JsonWriter& rWriter)
        : mrWriter(rWriter)
    {
    }

public:
    ~ScopedJsonWriterArray() { mrWriter.endArray(); }
};

/**
 * Auto-closes the node.
 */
class ScopedJsonWriterStruct
{
    friend class JsonWriter;

    JsonWriter& mrWriter;

    ScopedJsonWriterStruct(JsonWriter& rWriter)
        : mrWriter(rWriter)
    {
    }

public:
    ~ScopedJsonWriterStruct() { mrWriter.endStruct(); }
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
