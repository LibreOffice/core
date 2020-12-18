/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/BitmapColor.hxx>

class BitmapReadAccess;

struct OctreeNode
{
    OctreeNode()
        : nCount(0)
        , nRed(0)
        , nGreen(0)
        , nBlue(0)
        , pNext(nullptr)
        , nPalIndex(0)
        , bLeaf(false)
    {
    }

    sal_uLong nCount;
    sal_uLong nRed;
    sal_uLong nGreen;
    sal_uLong nBlue;
    std::unique_ptr<OctreeNode> pChild[8];
    OctreeNode* pNext;
    sal_uInt16 nPalIndex;
    bool bLeaf;
};

class VCL_PLUGIN_PUBLIC Octree
{
private:
    void CreatePalette(OctreeNode* pNode);
    void GetPalIndex(const OctreeNode* pNode);

    SAL_DLLPRIVATE void add(std::unique_ptr<OctreeNode>& rpNode);
    SAL_DLLPRIVATE void reduce();

    BitmapPalette maPalette;
    sal_uLong mnLeafCount;
    sal_uLong mnLevel;
    std::unique_ptr<OctreeNode> pTree;
    std::vector<OctreeNode*> mpReduce;
    BitmapColor const* mpColor;
    sal_uInt16 mnPalIndex;

public:
    Octree(const BitmapReadAccess& rReadAcc, sal_uLong nColors);
    ~Octree();

    const BitmapPalette& GetPalette();
    sal_uInt16 GetBestPaletteIndex(const BitmapColor& rColor);
};

class InverseColorMap
{
private:
    std::vector<sal_uInt8> mpBuffer;
    std::vector<sal_uInt8> mpMap;

    void ImplCreateBuffers();

public:
    explicit InverseColorMap(const BitmapPalette& rPal);
    ~InverseColorMap();

    sal_uInt16 GetBestPaletteIndex(const BitmapColor& rColor);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
