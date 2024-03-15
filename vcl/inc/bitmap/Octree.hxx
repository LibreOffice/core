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

#ifndef INCLUDED_VCL_INC_OCTREE_HXX
#define INCLUDED_VCL_INC_OCTREE_HXX

#include <vcl/dllapi.h>
#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapPalette.hxx>
#include <tools/solar.h>

struct OctreeNode
{
    sal_uLong nCount = 0;
    sal_uLong nRed = 0;
    sal_uLong nGreen = 0;
    sal_uLong nBlue = 0;
    std::unique_ptr<OctreeNode> pChild[8];
    OctreeNode* pNext = nullptr;
    sal_uInt16 nPalIndex = 0;
    bool bLeaf = false;
};

class BitmapReadAccess;

class VCL_PLUGIN_PUBLIC Octree
{
private:
    SAL_DLLPRIVATE void CreatePalette(OctreeNode* pNode);
    SAL_DLLPRIVATE void GetPalIndex(const OctreeNode* pNode, BitmapColor const& color);

    SAL_DLLPRIVATE void add(std::unique_ptr<OctreeNode>& rpNode, BitmapColor const& color);
    SAL_DLLPRIVATE void reduce();

    BitmapPalette maPalette;
    sal_uLong mnLeafCount;
    sal_uLong mnLevel;
    std::unique_ptr<OctreeNode> pTree;
    std::vector<OctreeNode*> mpReduce;
    sal_uInt16 mnPalIndex;

public:
    Octree(const BitmapReadAccess& rReadAcc, sal_uLong nColors);
    ~Octree();

    const BitmapPalette& GetPalette();
    SAL_DLLPRIVATE sal_uInt16 GetBestPaletteIndex(const BitmapColor& rColor);
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

#endif // INCLUDED_VCL_INC_OCTREE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
