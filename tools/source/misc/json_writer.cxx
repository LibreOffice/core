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
#include <rtl/strbuf.hxx>
#include <rtl/math.hxx>

namespace tools
{
/** These buffers are short-lived, so rather waste some space and avoid the cost of
 * repeated calls into the allocator */
constexpr int DEFAULT_BUFFER_SIZE = 2048;

JsonWriter::JsonWriter()
    : mSpaceAllocated(DEFAULT_BUFFER_SIZE)
    , mpBuffer(static_cast<char*>(malloc(mSpaceAllocated)))
    , mStartNodeCount(0)
    , mPos(mpBuffer)
    , mbFirstFieldInNode(true)
{
    *mPos = '{';
    ++mPos;
    *mPos = ' ';
    ++mPos;

    addValidationMark();
}

JsonWriter::~JsonWriter()
{
    assert(!mpBuffer && "forgot to extract data?");
    free(mpBuffer);
}

ScopedJsonWriterNode JsonWriter::startNode(const char* pNodeName)
{
    auto len = strlen(pNodeName);
    ensureSpace(len + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pNodeName, len);
    mPos += len;
    memcpy(mPos, "\": { ", 5);
    mPos += 5;
    mStartNodeCount++;
    mbFirstFieldInNode = true;

    validate();

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

ScopedJsonWriterArray JsonWriter::startArray(const char* pNodeName)
{
    auto len = strlen(pNodeName);
    ensureSpace(len + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pNodeName, len);
    mPos += len;
    memcpy(mPos, "\": [ ", 5);
    mPos += 5;
    mStartNodeCount++;
    mbFirstFieldInNode = true;

    validate();

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

    validate();

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

void JsonWriter::writeEscapedOUString(const OUString& rPropVal)
{
    // Convert from UTF-16 to UTF-8 and perform escaping
    sal_Int32 i = 0;
    while (i < rPropVal.getLength())
    {
        sal_uInt32 ch = rPropVal.iterateCodePoints(&i);
        if (ch == '\\')
        {
            *mPos = static_cast<char>(ch);
            ++mPos;
            *mPos = static_cast<char>(ch);
            ++mPos;
        }
        else if (ch == '"')
        {
            *mPos = '\\';
            ++mPos;
            *mPos = static_cast<char>(ch);
            ++mPos;
        }
        else if (ch == '\n')
        {
            *mPos = '\\';
            ++mPos;
            *mPos = 'n';
            ++mPos;
        }
        else if (ch == '\r')
        {
            *mPos = '\\';
            ++mPos;
            *mPos = 'r';
            ++mPos;
        }
        else if (ch == '\f')
        {
            *mPos = '\\';
            ++mPos;
            *mPos = 'f';
            ++mPos;
        }
        else if (ch <= 0x7F)
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

    validate();
}

void JsonWriter::put(const char* pPropName, const OUString& rPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    // But values can be any UTF-8,
    // see rtl_ImplGetFastUTF8ByteLen in sal/rtl/string.cxx for why a factor 3
    // is the worst case
    auto nWorstCasePropValLength = rPropVal.getLength() * 3;
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

    validate();
}

void JsonWriter::put(const char* pPropName, const OString& rPropVal)
{
    // we assume property names are ascii
    auto nPropNameLength = strlen(pPropName);
    // escaping can double the length
    auto nWorstCasePropValLength = rPropVal.getLength() * 2;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": \"", 4);
    mPos += 4;

    // copy and perform escaping
    for (int i = 0; i < rPropVal.getLength(); ++i)
    {
        char ch = rPropVal[i];
        if (ch == '\\')
        {
            *mPos = ch;
            ++mPos;
            *mPos = ch;
            ++mPos;
        }
        else if (ch == '"')
        {
            *mPos = '\\';
            ++mPos;
            *mPos = ch;
            ++mPos;
        }
        else
        {
            *mPos = ch;
            ++mPos;
        }
    }

    *mPos = '"';
    ++mPos;

    validate();
}

void JsonWriter::put(const char* pPropName, const char* pPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    auto nPropValLength = strlen(pPropVal);
    auto nWorstCasePropValLength = nPropValLength * 2;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": \"", 4);
    mPos += 4;

    // copy and perform escaping
    for (;;)
    {
        char ch = *pPropVal;
        if (!ch)
            break;
        ++pPropVal;
        if (ch == '\\')
        {
            *mPos = ch;
            ++mPos;
            *mPos = ch;
            ++mPos;
        }
        else if (ch == '"')
        {
            *mPos = '\\';
            ++mPos;
            *mPos = ch;
            ++mPos;
        }
        else
        {
            *mPos = ch;
            ++mPos;
        }
    }

    *mPos = '"';
    ++mPos;

    validate();
}

void JsonWriter::put(const char* pPropName, sal_Int64 nPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = 32;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 8);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    memcpy(mPos, "\": ", 3);
    mPos += 3;

    mPos += sprintf(mPos, "%" SAL_PRIdINT64, nPropVal);

    validate();
}

void JsonWriter::put(const char* pPropName, double fPropVal)
{
    OString sPropVal = rtl::math::doubleToString(fPropVal, rtl_math_StringFormat_F, 12, '.');
    auto nPropNameLength = strlen(pPropName);
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

    validate();
}

void JsonWriter::put(const char* pPropName, bool nPropVal)
{
    auto nPropNameLength = strlen(pPropName);
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

    validate();
}

void JsonWriter::putSimpleValue(const OUString& rPropVal)
{
    auto nWorstCasePropValLength = rPropVal.getLength() * 3;
    ensureSpace(nWorstCasePropValLength + 4);

    addCommaBeforeField();

    *mPos = '"';
    ++mPos;

    writeEscapedOUString(rPropVal);

    *mPos = '"';
    ++mPos;

    validate();
}

void JsonWriter::putRaw(const rtl::OStringBuffer& rRawBuf)
{
    ensureSpace(rRawBuf.getLength() + 2);

    addCommaBeforeField();

    memcpy(mPos, rRawBuf.getStr(), rRawBuf.getLength());
    mPos += rRawBuf.getLength();

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

void JsonWriter::reallocBuffer(int noMoreBytesRequired)
{
    int currentUsed = mPos - mpBuffer;
    auto newSize = std::max<int>(mSpaceAllocated * 2, (currentUsed + noMoreBytesRequired) * 2);
    char* pNew = static_cast<char*>(malloc(newSize));
    memcpy(pNew, mpBuffer, currentUsed);
    free(mpBuffer);
    mpBuffer = pNew;
    mPos = mpBuffer + currentUsed;
    mSpaceAllocated = newSize;

    addValidationMark();
}

/** Hands ownership of the underlying storage buffer to the caller,
  * after this no more document modifications may be written. */
char* JsonWriter::extractData()
{
    assert(mStartNodeCount == 0 && "did not close all nodes");
    assert(mpBuffer && "data already extracted");
    ensureSpace(2);
    // add closing brace
    *mPos = '}';
    ++mPos;
    // null-terminate
    *mPos = 0;
    mPos = nullptr;
    char* pRet = nullptr;
    std::swap(pRet, mpBuffer);
    return pRet;
}

OString JsonWriter::extractAsOString()
{
    char* pChar = extractData();
    OString ret(pChar);
    free(pChar);
    return ret;
}

std::string JsonWriter::extractAsStdString()
{
    char* pChar = extractData();
    std::string ret(pChar);
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
