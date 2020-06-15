/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/json.hxx>
#include <stdio.h>

namespace tools
{
/** These buffers are short-lived, so rather waste some space and avoid the cost of
 * repeated calls into the allocator */
constexpr int DEFAULT_BUFFER_SIZE = 2048;

Json::Json()
    : mSpaceAllocated(DEFAULT_BUFFER_SIZE)
    , maBuffer(new char[mSpaceAllocated])
    , mStartNodeCount(0)
    , mPos(maBuffer.get())
{
}

Json::~Json() = default;

void Json::StartNode(const char* pNodeName)
{
    auto len = strlen(pNodeName);
    EnsureSpace(len + 4);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pNodeName, len);
    mPos += len;
    *mPos = '"';
    ++mPos;
    *mPos = ':';
    ++mPos;
    *mPos = '{';
    ++mPos;
}

void Json::EndNode()
{
    assert(mStartNodeCount && "mismatched StartNode/EndNode somewhere");
    --mStartNodeCount;
    EnsureSpace(1);
    *mPos = '}';
    ++mPos;
}

void Json::put(const char* pPropName, const OUString& rPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = rPropVal.getLength() * 2;
    EnsureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    *mPos = '"';
    ++mPos;
    *mPos = ':';
    ++mPos;
    *mPos = ' ';
    ++mPos;
    *mPos = '"';
    ++mPos;

    // Convert from UTF-16 to UTF-8 and perform escaping
    for (int i = 0; i < rPropVal.getLength(); ++i)
    {
        sal_Unicode ch = rPropVal[i];
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
        else if (ch <= 0x7F)
        {
            *mPos = ch;
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
    *mPos = ',';
    ++mPos;
}

void Json::put(const char* pPropName, const OString& rPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = rPropVal.getLength();
    EnsureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    *mPos = '"';
    ++mPos;
    *mPos = ':';
    ++mPos;
    *mPos = ' ';
    ++mPos;
    *mPos = '"';
    ++mPos;

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
    *mPos = ',';
    ++mPos;
}

void Json::put(const char* pPropName, const char* pPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    auto nPropValLength = strlen(pPropVal);
    auto nWorstCasePropValLength = nPropValLength * 2;
    EnsureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    *mPos = '"';
    ++mPos;
    *mPos = ':';
    ++mPos;
    *mPos = ' ';
    ++mPos;
    *mPos = '"';
    ++mPos;

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
    *mPos = ',';
    ++mPos;
}

void Json::put(const char* pPropName, int nPropVal)
{
    auto nPropNameLength = strlen(pPropName);
    auto nWorstCasePropValLength = 32;
    EnsureSpace(nPropNameLength + nWorstCasePropValLength + 6);
    *mPos = '"';
    ++mPos;
    memcpy(mPos, pPropName, nPropNameLength);
    *mPos = '"';
    ++mPos;
    *mPos = ':';
    ++mPos;
    *mPos = ' ';
    ++mPos;
    *mPos = '"';
    ++mPos;

    mPos += sprintf(mPos, "%d", nPropVal);

    *mPos = '"';
    ++mPos;
    *mPos = ',';
    ++mPos;
}

/** Hands ownership of the the underlying storage buffer to the caller,
  * after this no more document modifications may be written. */
char* Json::ExtractData()
{
    assert(maBuffer);
    assert(mStartNodeCount && "mismatched StartNode/EndNode somewhere");
    *mPos = 0;
    ++mPos;
    mPos = nullptr;
    auto ret = maBuffer.release();
    return ret;
}

void Json::EnsureSpace(int noMoreBytesRequired)
{
    int currentUsed = mPos - maBuffer.get();
    if (currentUsed + noMoreBytesRequired < mSpaceAllocated)
        return;
    auto newSize = std::max(mSpaceAllocated * 2, (currentUsed + noMoreBytesRequired) * 2);
    auto pNew = new char[newSize];
    memcpy(pNew, maBuffer.get(), currentUsed);
    maBuffer.reset(pNew);
    mPos = maBuffer.get();
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
