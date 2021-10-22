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
#include <algorithm>
#include <cstring>
#include <rtl/math.hxx>

namespace tools
{
/** These buffers are short-lived, so rather waste some space and avoid the cost of
 * repeated calls into the allocator */
constexpr int DEFAULT_BUFFER_SIZE = 2048;

JsonWriter::JsonWriter()
    : mpBuffer(static_cast<char*>(malloc(DEFAULT_BUFFER_SIZE)))
    , mPos(mpBuffer)
    , mSpaceAllocated(DEFAULT_BUFFER_SIZE)
    , mStartNodeCount(0)
    , mbFirstFieldInNode(true)
{
    *mPos = '{';
    ++mPos;
    *mPos = ' ';
    ++mPos;
}

JsonWriter::~JsonWriter()
{
    assert(!mpBuffer && "forgot to extract data?");
    free(mpBuffer);
}

ScopedJsonWriterNode JsonWriter::startNode(const char* pNodeName)
{
    int len = strlen(pNodeName);
    ensureSpace(len + 6);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pNodeName, len);
    mPos += len;
    memcpy(mPos, "\": { ", 5);
    mPos += 5;
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
}

ScopedJsonWriterArray JsonWriter::startArray(const char* pNodeName)
{
    int len = strlen(pNodeName);
    ensureSpace(len + 6);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pNodeName, len);
    mPos += len;
    memcpy(mPos, "\": [ ", 5);
    mPos += 5;
    mStartNodeCount++;
    mbFirstFieldInNode = true;
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
}

ScopedJsonWriterStruct JsonWriter::startStruct()
{
    ensureSpace(6);

    addCommaBeforeField();

    *mPos = '{';
    ++mPos;
    *mPos = ' ';
    ++mPos;
    mStartNodeCount++;
    mbFirstFieldInNode = true;
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
}

void JsonWriter::put(const char* pPropName, const OUString& rPropVal)
{
    int nPropNameLength = strlen(pPropName);
    // But values can be any UTF-8,
    // if the string only contains of 0x2028, it will be expanded 6 times (see writeEscapedSequence)
    int nWorstCasePropValLength = rPropVal.getLength() * 6;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": \"", 4);
    mPos += 4;

    writeEscapedOUString(rPropVal);

    *mPos = '"';
    ++mPos;
}

void JsonWriter::put(const char* pPropName, std::string_view rPropVal)
{
    // we assume property names are ascii
    int nPropNameLength = strlen(pPropName);
    // escaping can double the length
    int nWorstCasePropValLength = rPropVal.size() * 2;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": \"", 4);
    mPos += 4;

    // copy and perform escaping
    for (size_t i = 0; i < rPropVal.size(); ++i)
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
}

void JsonWriter::put(const char* pPropName, sal_Int64 nPropVal)
{
    int nPropNameLength = strlen(pPropName);
    int nWorstCasePropValLength = 32;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": ", 3);
    mPos += 3;

    mPos += sprintf(mPos, "%" SAL_PRIdINT64, nPropVal);
}

void JsonWriter::put(const char* pPropName, double fPropVal)
{
    OString sPropVal = rtl::math::doubleToString(fPropVal, rtl_math_StringFormat_F, 12, '.');
    int nPropNameLength = strlen(pPropName);
    ensureSpace(nPropNameLength + sPropVal.getLength() + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": ", 3);
    mPos += 3;

    memcpy(mPos, sPropVal.getStr(), sPropVal.getLength());
    mPos += sPropVal.getLength();
}

void JsonWriter::put(const char* pPropName, bool nPropVal)
{
    int nPropNameLength = strlen(pPropName);
    ensureSpace(nPropNameLength + 5 + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": ", 3);
    mPos += 3;

    const char* pVal;
    if (nPropVal)
        pVal = "true";
    else
        pVal = "false";
    memcpy(mPos, pVal, strlen(pVal));
    mPos += strlen(pVal);
}

void JsonWriter::putSimpleValue(const OUString& rPropVal)
{
    int nWorstCasePropValLength = rPropVal.getLength() * 3;
    ensureSpace(nWorstCasePropValLength + 4);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;

    writeEscapedOUString(rPropVal);

    *mPos = '"';
    ++mPos;
}

void JsonWriter::putRaw(std::string_view rRawBuf)
{
    ensureSpace(rRawBuf.size() + 2);

    addCommaBeforeField();

    memcpy(mPos, rRawBuf.data(), rRawBuf.size());
    mPos += rRawBuf.size();
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
    assert(mpBuffer && "already extracted data");
    int currentUsed = mPos - mpBuffer;
    if (currentUsed + noMoreBytesRequired >= mSpaceAllocated)
    {
        int newSize = (currentUsed + noMoreBytesRequired) * 2;
        mpBuffer = static_cast<char*>(realloc(mpBuffer, newSize));
        mPos = mpBuffer + currentUsed;
        mSpaceAllocated = newSize;
    }
}

/** Hands ownership of the underlying storage buffer to the caller,
  * after this no more document modifications may be written. */
std::pair<char*, int> JsonWriter::extractDataImpl()
{
    assert(mStartNodeCount == 0 && "did not close all nodes");
    assert(mpBuffer && "data already extracted");
    ensureSpace(2);
    // add closing brace
    *mPos = '}';
    ++mPos;
    // null-terminate
    *mPos = 0;
    const int sz = mPos - mpBuffer;
    mPos = nullptr;
    return { std::exchange(mpBuffer, nullptr), sz };
}

OString JsonWriter::extractAsOString()
{
    auto[pChar, sz] = extractDataImpl();
    OString ret(pChar, sz);
    free(pChar);
    return ret;
}

std::string JsonWriter::extractAsStdString()
{
    auto[pChar, sz] = extractDataImpl();
    std::string ret(pChar, sz);
    free(pChar);
    return ret;
}

bool JsonWriter::isDataEquals(const std::string& s) const
{
    return s.length() == static_cast<size_t>(mPos - mpBuffer)
           && memcmp(s.data(), mpBuffer, s.length()) == 0;
}

} // namespace tools
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
