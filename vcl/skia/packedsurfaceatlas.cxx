/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <skia/packedsurfaceatlas.hxx>

#include <memory>
#include <assert.h>

#include <sal/config.h>
#include <sal/log.hxx>

#include <skia/utils.hxx>

namespace
{
struct Node
{
    tools::Rectangle const mRectangle;
    std::unique_ptr<Node> mLeftNode;
    std::unique_ptr<Node> mRightNode;
    bool mOccupied;

    explicit Node(int nWidth, int nHeight);
    explicit Node(tools::Rectangle const& aRectangle);

    bool isLeaf() const;
    Node* insert(int nWidth, int nHeight, int nPadding);
};
}

Node::Node(int nWidth, int nHeight)
    : mRectangle(tools::Rectangle(Point(), Size(nWidth, nHeight)))
    , mLeftNode()
    , mRightNode()
    , mOccupied(false)
{
}

Node::Node(tools::Rectangle const& aRectangle)
    : mRectangle(aRectangle)
    , mLeftNode()
    , mRightNode()
    , mOccupied(false)
{
}

bool Node::isLeaf() const { return mLeftNode == nullptr && mRightNode == nullptr; }

Node* Node::insert(int nWidth, int nHeight, int nPadding)
{
    if (!isLeaf())
    {
        Node* pNewNode = mLeftNode->insert(nWidth, nHeight, nPadding);

        if (pNewNode != nullptr)
            return pNewNode;

        return mRightNode->insert(nWidth, nHeight, nPadding);
    }
    else
    {
        if (mOccupied)
        {
            return nullptr;
        }

        if (nWidth > mRectangle.GetWidth() || nHeight > mRectangle.GetHeight())
        { // does not fit
            return nullptr;
        }

        if (nWidth == mRectangle.GetWidth() && nHeight == mRectangle.GetHeight())
        { // perfect fit
            mOccupied = true;
            return this;
        }

        int dw = mRectangle.GetWidth() - nWidth;
        int dh = mRectangle.GetHeight() - nHeight;

        tools::Rectangle aLeftRect;
        tools::Rectangle aRightRect;
        if (dw > dh)
        {
            aLeftRect = tools::Rectangle(Point(mRectangle.Left(), mRectangle.Top()),
                                         Size(nWidth, mRectangle.GetHeight()));
            aRightRect = tools::Rectangle(
                Point(nPadding + mRectangle.Left() + nWidth, mRectangle.Top()),
                Size(mRectangle.GetWidth() - nWidth - nPadding, mRectangle.GetHeight()));
        }
        else
        {
            aLeftRect = tools::Rectangle(Point(mRectangle.Left(), mRectangle.Top()),
                                         Size(mRectangle.GetWidth(), nHeight));
            aRightRect = tools::Rectangle(
                Point(mRectangle.Left(), nPadding + mRectangle.Top() + nHeight),
                Size(mRectangle.GetWidth(), mRectangle.GetHeight() - nHeight - nPadding));
        }

        mLeftNode.reset(new Node(aLeftRect));
        mRightNode.reset(new Node(aRightRect));

        return mLeftNode->insert(nWidth, nHeight, nPadding);
    }
}

struct SkiaPackedSurfaceAtlasManager::PackedSurface
{
    sk_sp<SkSurface> mpSurface;
    std::unique_ptr<Node> mpRootNode;

    PackedSurface(int nWidth, int nHeight)
        : mpSurface(SkiaHelper::createSkSurface(nWidth, nHeight))
        , mpRootNode(new Node(nWidth, nHeight))
    {
        assert(mpSurface);
    }
};

SkiaPackedSurfaceAtlasManager::SkiaPackedSurfaceAtlasManager(int nSurfaceWidth, int nSurfaceHeight)
    : mnSurfaceWidth(nSurfaceWidth)
    , mnSurfaceHeight(nSurfaceHeight)
{
}

SkiaPackedSurfaceAtlasManager::~SkiaPackedSurfaceAtlasManager() {}

void SkiaPackedSurfaceAtlasManager::CreateNewSurface()
{
    std::unique_ptr<PackedSurface> pPackedSurface(
        new PackedSurface(mnSurfaceWidth, mnSurfaceHeight));
    maPackedSurfaces.push_back(std::move(pPackedSurface));
    SAL_INFO("vcl.skia.trace",
             "SkiaPackedSurfaceAtlas adding surface, count: " << maPackedSurfaces.size());
}

SkiaPackedSurface SkiaPackedSurfaceAtlasManager::Reserve(int nWidth, int nHeight)
{
    for (std::unique_ptr<PackedSurface>& pPackedSurface : maPackedSurfaces)
    {
        Node* pNode = pPackedSurface->mpRootNode->insert(nWidth, nHeight, 1);
        if (pNode != nullptr)
            return SkiaPackedSurface(pPackedSurface->mpSurface, pNode->mRectangle);
    }
    CreateNewSurface();
    std::unique_ptr<PackedSurface>& pPackedSurface = maPackedSurfaces.back();
    Node* pNode = pPackedSurface->mpRootNode->insert(nWidth, nHeight, 1);
    if (pNode != nullptr)
        return SkiaPackedSurface(pPackedSurface->mpSurface, pNode->mRectangle);
    return SkiaPackedSurface();
}

std::vector<sk_sp<SkSurface>>
SkiaPackedSurfaceAtlasManager::ReduceSurfaceNumber(int nMaxNumberOfSurfaces)
{
    std::vector<sk_sp<SkSurface>> aSurfaces;
    while (int(maPackedSurfaces.size()) > nMaxNumberOfSurfaces)
    {
        // Remove oldest created surface
        aSurfaces.push_back(maPackedSurfaces[0]->mpSurface);
        maPackedSurfaces.erase(maPackedSurfaces.begin());
        SAL_INFO("vcl.skia.trace",
                 "PackedSurfaceAtlas removing surface, count: " << maPackedSurfaces.size());
    }
    return aSurfaces;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
