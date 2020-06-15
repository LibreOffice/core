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
#include <memory>

namespace tools
{
/** Simple JSON encoder designed specifically for LibreOfficeKit purposes.
 *
 * (1) Minimal allocations/re-allocations/copying
 * (2) Small/simple JSON documents
 * (3) ascii property names
 */
class TOOLS_DLLPUBLIC Json
{
    int mSpaceAllocated;
    std::unique_ptr<char[]> maBuffer;
    int mStartNodeCount;
    char* mPos;

public:
    Json();
    ~Json();
    void StartNode(const char*);
    void EndNode();
    void put(const char* pPropName, const OUString& rPropValue);
    void put(const char* pPropName, const OString& rPropValue);
    void put(const char* pPropName, const char* pPropVal);
    void put(const char*, int);

    /** Hands ownership of the the underlying storage buffer to the caller,
     * after this no more document modifications may be written. */
    char* ExtractData();

private:
    void EnsureSpace(int noMoreBytesRequired);
};
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
