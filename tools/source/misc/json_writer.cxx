/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/json_writer.hxx>
#include <stdio.h>
#include <cstring>
#include <rtl/math.hxx>

namespace tools
{
/** These buffers are short-lived, so rather waste some space and avoid the cost of
 * repeated calls into the allocator */
constexpr int DEFAULT_BUFFER_SIZE = 2048;

JsonWriter::JsonWriter()
    : mpBuffer(rtl_string_alloc(DEFAULT_BUFFER_SIZE))
    , mPos(mpBuffer->buffer)
    , mSpaceAllocated(DEFAULT_BUFFER_SIZE)
    , mStartNodeCount(0)
    , mbFirstFieldInNode(true)
    , mbClosed(false)
{
    *mPos = '{';
    ++mPos;
    *mPos = ' ';
    ++mPos;

    addValidationMark();
}

JsonWriter::~JsonWriter()
{
    assert(mbClosed && "forgot to extract data?");
    rtl_string_release(mpBuffer);
}

ScopedJsonWriterNode JsonWriter::startNode(std::string_view pNodeName)
{
    putLiteral(pNodeName, "{ ");

    mStartNodeCount++;
    mbFirstFieldInNode = true;

    return ScopedJsonWriterNode(*this);
}

void JsonWriter::endNode()
{
    assert(mStartNodeCount && "mismatched StartNode/EndNode somewhere");
    --mStartNodeCount;
    ensureSpace(1);
    *mPos = '}';
    ++mPos;
    mbFirstFieldInNode = false;

    validate();
}

ScopedJsonWriterArray JsonWriter::startArray(std::string_view pNodeName)
{
    putLiteral(pNodeName, "[ ");

    mStartNodeCount++;
    mbFirstFieldInNode = true;

    return ScopedJsonWriterArray(*this);
}

ScopedJsonWriterArray JsonWriter::startAnonArray()
{
    startAnonBlock('[');
    return ScopedJsonWriterArray(*this);
}

void JsonWriter::endArray()
{
    assert(mStartNodeCount && "mismatched StartNode/EndNode somewhere");
    --mStartNodeCount;
    ensureSpace(1);
    *mPos = ']';
    ++mPos;
    mbFirstFieldInNode = false;

    validate();
}

void JsonWriter::startAnonBlock(const char cType)
{
    ensureSpace(6);

    addCommaBeforeField();

    *mPos = cType;
    ++mPos;
    *mPos = ' ';
    ++mPos;
    mStartNodeCount++;
    mbFirstFieldInNode = true;

    validate();
}

ScopedJsonWriterStruct JsonWriter::startStruct()
{
    startAnonBlock('{');
    return ScopedJsonWriterStruct(*this);
}

void JsonWriter::endStruct()
{
    assert(mStartNodeCount && "mismatched StartNode/EndNode somewhere");
    --mStartNodeCount;
    ensureSpace(1);
    *mPos = '}';
    ++mPos;
    mbFirstFieldInNode = false;

    validate();
}

static char getEscapementChar(char ch)
{
    switch (ch)
    {
        case '\b':
            return 'b';
        case '\t':
            return 't';
        case '\n':
            return 'n';
        case '\f':
            return 'f';
        case '\r':
            return 'r';
        default:
            return ch;
    }
}

static bool writeEscapedSequence(sal_uInt32 ch, char*& pos)
{
    // control characters
    if (ch <= 0x1f)
    {
        int written = snprintf(pos, 7, "\\u%.4x", static_cast<unsigned int>(ch));
        if (written > 0)
            pos += written;
        return true;
    }

    switch (ch)
    {
        case '"':
        case '/':
        case '\\':
            *pos++ = '\\';
            *pos++ = getEscapementChar(ch);
            return true;
        // Special processing of U+2028 and U+2029, which are valid JSON, but invalid JavaScript
        // Write them in escaped '\u2028' or '\u2029' form
        case 0x2028:
        case 0x2029:
            *pos++ = '\\';
            *pos++ = 'u';
            *pos++ = '2';
            *pos++ = '0';
            *pos++ = '2';
            *pos++ = ch == 0x2028 ? '8' : '9';
            return true;
        default:
            return false;
    }
}

void JsonWriter::writeEscapedOUString(const OUString& rPropVal)
{
    *mPos = '"';
    ++mPos;

    // Convert from UTF-16 to UTF-8 and perform escaping
    sal_Int32 i = 0;
    while (i < rPropVal.getLength())
    {
        sal_uInt32 ch = rPropVal.iterateCodePoints(&i);
        if (writeEscapedSequence(ch, mPos))
            continue;
        if (ch <= 0x7F)
        {
            *mPos = static_cast<char>(ch);
            ++mPos;
        }
        else if (ch <= 0x7FF)
        {
            *mPos = 0xC0 | (ch >> 6); /* 110xxxxx */
            ++mPos;
            *mPos = 0x80 | (ch & 0x3F); /* 10xxxxxx */
            ++mPos;
        }
        else if (ch <= 0xFFFF)
        {
            *mPos = 0xE0 | (ch >> 12); /* 1110xxxx */
            ++mPos;
            *mPos = 0x80 | ((ch >> 6) & 0x3F); /* 10xxxxxx */
            ++mPos;
            *mPos = 0x80 | (ch & 0x3F); /* 10xxxxxx */
            ++mPos;
        }
        else
        {
            *mPos = 0xF0 | (ch >> 18); /* 11110xxx */
            ++mPos;
            *mPos = 0x80 | ((ch >> 12) & 0x3F); /* 10xxxxxx */
            ++mPos;
            *mPos = 0x80 | ((ch >> 6) & 0x3F); /* 10xxxxxx */
            ++mPos;
            *mPos = 0x80 | (ch & 0x3F); /* 10xxxxxx */
            ++mPos;
        }
    }

    *mPos = '"';
    ++mPos;

    validate();
}

void JsonWriter::put(std::u16string_view pPropName, const OUString& rPropVal)
{
    auto nPropNameLength = pPropName.length();
    // But values can be any UTF-8,
    // if the string only contains of 0x2028, it will be expanded 6 times (see writeEscapedSequence)
    auto nWorstCasePropValLength = rPropVal.getLength() * 6;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    writeEscapedOUString(OUString(pPropName));

    memcpy(mPos, ": ", 2);
    mPos += 2;

    writeEscapedOUString(rPropVal);

    validate();
}

void JsonWriter::put(std::string_view pPropName, const OUString& rPropVal)
{
    // Values can be any UTF-8,
    // if the string only contains of 0x2028, it will be expanded 6 times (see writeEscapedSequence)
    auto nWorstCasePropValLength = rPropVal.getLength() * 6 + 2;
    ensureSpaceAndWriteNameColon(pPropName, nWorstCasePropValLength);

    writeEscapedOUString(rPropVal);
}

void JsonWriter::put(std::string_view pPropName, std::string_view rPropVal)
{
    // escaping can double the length, plus quotes
    auto nWorstCasePropValLength = rPropVal.size() * 2 + 2;
    ensureSpaceAndWriteNameColon(pPropName, nWorstCasePropValLength);

    *mPos = '"';
    ++mPos;

    // copy and perform escaping
    bool bReachedEnd = false;
    for (size_t i = 0; i < rPropVal.size() && !bReachedEnd; ++i)
    {
        char ch = rPropVal[i];
        switch (ch)
        {
            case '\b':
            case '\t':
            case '\n':
            case '\f':
            case '\r':
            case '"':
            case '/':
            case '\\':
                writeEscapedSequence(ch, mPos);
                break;
            case 0:
                bReachedEnd = true;
                break;
            case '\xE2': // Special processing of U+2028 and U+2029
                if (i + 2 < rPropVal.size() && rPropVal[i + 1] == '\x80'
                    && (rPropVal[i + 2] == '\xA8' || rPropVal[i + 2] == '\xA9'))
                {
                    writeEscapedSequence(rPropVal[i + 2] == '\xA8' ? 0x2028 : 0x2029, mPos);
                    i += 2;
                    break;
                }
                [[fallthrough]];
            default:
                *mPos = ch;
                ++mPos;
                break;
        }
    }

    *mPos = '"';
    ++mPos;

    validate();
}

void JsonWriter::put(std::string_view pPropName, bool nPropVal)
{
    putLiteral(pPropName, nPropVal ? std::string_view("true") : std::string_view("false"));
}

void JsonWriter::putSimpleValue(const OUString& rPropVal)
{
    auto nWorstCasePropValLength = rPropVal.getLength() * 6;
    ensureSpace(nWorstCasePropValLength + 4);

    addCommaBeforeField();

    writeEscapedOUString(rPropVal);
}

void JsonWriter::putRaw(std::string_view rRawBuf)
{
    ensureSpace(rRawBuf.size() + 2);

    addCommaBeforeField();

    memcpy(mPos, rRawBuf.data(), rRawBuf.size());
    mPos += rRawBuf.size();

    validate();
}

void JsonWriter::addCommaBeforeField()
{
    if (mbFirstFieldInNode)
        mbFirstFieldInNode = false;
    else
    {
        *mPos = ',';
        ++mPos;
        *mPos = ' ';
        ++mPos;
    }
}

void JsonWriter::ensureSpace(int noMoreBytesRequired)
{
    assert(!mbClosed && "already extracted data");
    int currentUsed = mPos - mpBuffer->buffer;
    if (currentUsed + noMoreBytesRequired >= mSpaceAllocated)
    {
        auto newSize = (currentUsed + noMoreBytesRequired) * 2;
        rtl_String* pNewBuffer = rtl_string_alloc(newSize);
        memcpy(pNewBuffer->buffer, mpBuffer->buffer, currentUsed);
        rtl_string_release(mpBuffer);
        mpBuffer = pNewBuffer;
        mPos = mpBuffer->buffer + currentUsed;
        mSpaceAllocated = newSize;

        addValidationMark();
    }
}

void JsonWriter::ensureSpaceAndWriteNameColon(std::string_view name, int valSize)
{
    // we assume property names are ascii
    ensureSpace(name.size() + valSize + 6);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, name.data(), name.size());
    mPos += name.size();
    memcpy(mPos, "\": ", 3);
    mPos += 3;
}

void JsonWriter::putLiteral(std::string_view propName, std::string_view propValue)
{
    ensureSpaceAndWriteNameColon(propName, propValue.size());
    memcpy(mPos, propValue.data(), propValue.size());
    mPos += propValue.size();

    validate();
}

OString JsonWriter::finishAndGetAsOString()
{
    assert(mStartNodeCount == 0 && "did not close all nodes");
    assert(!mbClosed && "data already extracted");
    ensureSpace(2);
    // add closing brace
    *mPos = '}';
    ++mPos;
    // null-terminate
    *mPos = 0;
    mbClosed = true;

    mpBuffer->length = mPos - mpBuffer->buffer;
    return mpBuffer;
}

bool JsonWriter::isDataEquals(std::string_view s) const
{
    return std::string_view(mpBuffer->buffer, static_cast<size_t>(mPos - mpBuffer->buffer)) == s;
}

} // namespace tools
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
