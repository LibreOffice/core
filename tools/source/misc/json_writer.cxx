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

namespace tools
{
/** These buffers are short-lived, so rather waste some space and avoid the cost of
 * repeated calls into the allocator */
constexpr int DEFAULT_BUFFER_SIZE = 2048;

JsonWriter::JsonWriter()
    : mSpaceAllocated(DEFAULT_BUFFER_SIZE)
    , maBuffer(new char[mSpaceAllocated])
    , mStartNodeCount(0)
    , mPos(maBuffer.get())
{
    *mPos = '{';
    ++mPos;
    *mPos = ' ';
    ++mPos;
}

JsonWriter::~JsonWriter() { assert(!maBuffer && "forgot to extract data?"); }

ScopedJsonWriterNode JsonWriter::startNode(const char* pNodeName)
{
    auto len = strlen(pNodeName);
    ensureSpace(len + 4);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pNodeName, len);
    mPos += len;
    strncpy(mPos, "\": { ", 5);
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
}

void JsonWriter::put(const char* pPropName, const OUString& rPropVal)
{
    addCommaBeforeField();

    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = rPropVal.getLength() * 2;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    strncpy(mPos, "\": \"", 4);
    mPos += 4;

    // Convert from UTF-16 to UTF-8 and perform escaping
    for (int i = 0; i < rPropVal.getLength(); ++i)
    {
        sal_Unicode ch = rPropVal[i];
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
        else
        {
            *mPos = 0xE0 | (ch >> 12); /* 1110xxxx */
            ++mPos;
            *mPos = 0x80 | ((ch >> 6) & 0x3F); /* 10xxxxxx */
            ++mPos;
            *mPos = 0x80 | (ch & 0x3F); /* 10xxxxxx */
            ++mPos;
        }
    }

    *mPos = '"';
    ++mPos;
}

void JsonWriter::put(const char* pPropName, const OString& rPropVal)
{
    addCommaBeforeField();

    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = rPropVal.getLength();
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    strncpy(mPos, "\": \"", 4);
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
}

void JsonWriter::put(const char* pPropName, const char* pPropVal)
{
    addCommaBeforeField();

    auto nPropNameLength = strlen(pPropName);
    auto nPropValLength = strlen(pPropVal);
    auto nWorstCasePropValLength = nPropValLength * 2;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    strncpy(mPos, "\": \"", 4);
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
}

void JsonWriter::put(const char* pPropName, int nPropVal)
{
    addCommaBeforeField();

    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = 32;
    ensureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    mPos += nPropNameLength;
    strncpy(mPos, "\": ", 3);
    mPos += 3;

    mPos += sprintf(mPos, "%d", nPropVal);
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

/** Hands ownership of the the underlying storage buffer to the caller,
  * after this no more document modifications may be written. */
char* JsonWriter::extractData()
{
    assert(mStartNodeCount == 0 && "did not close all nodes");
    assert(maBuffer && "data already extracted");
    // add closing brace
    *mPos = '}';
    ++mPos;
    // null-terminate
    *mPos = 0;
    mPos = nullptr;
    return maBuffer.release();
}

} // namespace tools
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
