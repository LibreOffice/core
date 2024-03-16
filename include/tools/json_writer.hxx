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

#include <string_view>

/** Simple JSON encoder designed specifically for LibreOfficeKit purposes.
 *
 * (1) Minimal allocations/re-allocations/copying
 * (2) Small/simple JSON documents
 * (3) ascii property names
 */
namespace tools
{
class TOOLS_DLLPUBLIC JsonWriter
{
    // Auto-closes the node.
    template <char closing> struct ScopedJsonWriterNode
    {
        JsonWriter& mrWriter;
        ~ScopedJsonWriterNode() { mrWriter.endNode(closing); }
    };

    rtl_String* mpBuffer;
    char* mPos;
    int mSpaceAllocated;
    int mStartNodeCount;
    bool mbFirstFieldInNode;
    bool mbClosed; // cannot add to it anymore

public:
    JsonWriter();
    ~JsonWriter();

    [[nodiscard]] ScopedJsonWriterNode<'}'> startNode(std::string_view nodeName);
    [[nodiscard]] ScopedJsonWriterNode<']'> startArray(std::string_view nodeName);
    [[nodiscard]] ScopedJsonWriterNode<']'> startAnonArray();
    [[nodiscard]] ScopedJsonWriterNode<'}'> startStruct();

    void put(std::u16string_view pPropName, std::u16string_view rPropValue);

    void put(std::string_view pPropName, const OUString& rPropValue);
    // Assumes utf-8 property value encoding
    void put(std::string_view pPropName, std::string_view rPropValue);
    void put(std::string_view pPropName, const char* pPropVal)
    {
        put(pPropName, std::string_view(pPropVal));
    }
    template <size_t N> void put(std::string_view pPropName, const char (&pPropVal)[N])
    {
        put(pPropName, std::string_view(pPropVal, N));
    }

    template <typename N, std::enable_if_t<std::is_arithmetic_v<N>, int> = 0>
    void put(std::string_view pPropName, N n)
    {
        putLiteral(pPropName, OString::number(n));
    }
    void put(std::string_view pPropName, bool);

    void putSimpleValue(std::u16string_view rPropValue);

    /// This assumes that this data belongs at this point in the stream, and is valid, and properly encoded
    void putRaw(std::string_view);

    /** Closes the tags, and returns data.
     * After this no more document modifications may be written. */
    OString finishAndGetAsOString();

private:
    void endNode(char closing);
    void addCommaBeforeField();
    void writeEscapedOUString(std::u16string_view rPropVal);
    void closeDocument();
    void ensureSpace(int noMoreBytesRequired);
    void ensureSpaceAndWriteNameColon(std::string_view name, int valSize);
    void putLiteral(std::string_view propName, std::string_view propValue);

    // overflow validation in debug mode
    static constexpr unsigned char JSON_WRITER_DEBUG_MARKER = 0xde;

    inline void addValidationMark()
    {
#ifndef NDEBUG
        *(mpBuffer->buffer + mSpaceAllocated - 1) = JSON_WRITER_DEBUG_MARKER;
#endif
    }

    inline void validate()
    {
#ifndef NDEBUG
        unsigned char c = *(mpBuffer->buffer + mSpaceAllocated - 1);
        assert(c == JSON_WRITER_DEBUG_MARKER);
#endif
    }
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
