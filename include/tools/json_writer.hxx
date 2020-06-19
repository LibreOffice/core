/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <tools/toolsdllapi.h>
#include <rtl/ustring.hxx>
#include <algorithm>

/** Simple JSON encoder designed specifically for LibreOfficeKit purposes.
 *
 * (1) Minimal allocations/re-allocations/copying
 * (2) Small/simple JSON documents
 * (3) ascii property names
 */
namespace tools
{
class ScopedJsonWriterNode;

class TOOLS_DLLPUBLIC JsonWriter
{
    friend class ScopedJsonWriterNode;

    int mSpaceAllocated;
    char* mpBuffer;
    int mStartNodeCount;
    char* mPos;
    bool mbFirstFieldInNode;

public:
    JsonWriter();
    ~JsonWriter();

    [[nodiscard]] ScopedJsonWriterNode startNode(const char*);

    void put(const char* pPropName, const OUString& rPropValue);
    void put(const char* pPropName, const OString& rPropValue);
    void put(const char* pPropName, const char* pPropVal);
    void put(const char*, int);

    /** Hands ownership of the underlying storage buffer to the caller,
     * after this no more document modifications may be written. */
    char* extractData();

private:
    void endNode();
    void addCommaBeforeField();

    inline void ensureSpace(int noMoreBytesRequired)
    {
        int currentUsed = mPos - mpBuffer;
        if (currentUsed + noMoreBytesRequired >= mSpaceAllocated)
        {
            auto newSize = std::max(mSpaceAllocated * 2, (currentUsed + noMoreBytesRequired) * 2);
            char* pNew = static_cast<char*>(malloc(newSize));
            memcpy(pNew, mpBuffer, currentUsed);
            free(mpBuffer);
            mpBuffer = pNew;
            mPos = mpBuffer;
        }
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
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
