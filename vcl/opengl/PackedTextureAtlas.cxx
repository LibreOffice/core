/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include "opengl/framebuffer.hxx"
#include "opengl/texture.hxx"

#include "opengl/PackedTextureAtlas.hxx"

struct Node
{
    Rectangle mRectangle;
    std::unique_ptr<Node> mLeftNode;
    std::unique_ptr<Node> mRightNode;
    bool mOccupied;

    explicit Node(Rectangle& rRectangle);

    bool isLeaf();
    Node* insert(int nWidth, int nHeight, int nPadding);
};

Node::Node(Rectangle& aRectangle)
    : mRectangle(aRectangle)
    , mLeftNode()
    , mRightNode()
    , mOccupied(false)
{}

bool Node::isLeaf()
{
    return mLeftNode.get()  == nullptr &&
           mRightNode.get() == nullptr;
}

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
        {   // does not fit
            return nullptr;
        }

        if (nWidth == mRectangle.GetWidth() && nHeight == mRectangle.GetHeight())
        {   // perfect fit
            mOccupied = true;
            return this;
        }

        int dw = mRectangle.GetWidth() - nWidth;
        int dh = mRectangle.GetHeight() - nHeight;

        Rectangle aLeftRect;
        Rectangle aRightRect;
        if (dw > dh)
        {
            aLeftRect = Rectangle(Point(mRectangle.Left(), mRectangle.Top()),
                                  Size(nWidth, mRectangle.GetHeight()));
            aRightRect = Rectangle(Point(nPadding + mRectangle.Left() + nWidth, mRectangle.Top()),
                                   Size(mRectangle.GetWidth() - nWidth - nPadding, mRectangle.GetHeight()));
        }
        else
        {
            aLeftRect = Rectangle(Point(mRectangle.Left(), mRectangle.Top()),
                                  Size(mRectangle.GetWidth(), nHeight));
            aRightRect = Rectangle(Point(mRectangle.Left(), nPadding + mRectangle.Top() + nHeight),
                                   Size(mRectangle.GetWidth(), mRectangle.GetHeight() - nHeight - nPadding));
        }

        mLeftNode.reset(new Node(aLeftRect));
        mRightNode.reset(new Node(aRightRect));

        return mLeftNode->insert(nWidth, nHeight, nPadding);
    }
}

struct PackedTexture
{
    std::unique_ptr<Node> mpRootNode;
    ImplOpenGLTexture* mpTexture;
};

PackedTextureAtlasManager::PackedTextureAtlasManager(int nTextureWidth, int nTextureHeight)
    : mnTextureWidth(nTextureWidth)
    , mnTextureHeight(nTextureHeight)
{
}

PackedTextureAtlasManager::~PackedTextureAtlasManager()
{
    for (std::unique_ptr<PackedTexture>& pPackedTexture : maPackedTextures)
    {
        // Free texture early in VCL shutdown while we have a context.
        pPackedTexture->mpTexture->Dispose();
        pPackedTexture->mpTexture->DecreaseRefCount(0);
    }
}

void PackedTextureAtlasManager::CreateNewTexture()
{
    std::unique_ptr<PackedTexture> pPackedTexture(new PackedTexture);
    pPackedTexture->mpTexture = new ImplOpenGLTexture(mnTextureWidth, mnTextureHeight, true);
    Rectangle aInitialRect(Point(0, 0), Size(mnTextureWidth, mnTextureHeight));
    pPackedTexture->mpRootNode.reset(new Node(aInitialRect));
    maPackedTextures.push_back(std::move(pPackedTexture));
}

OpenGLTexture PackedTextureAtlasManager::Reserve(int nWidth, int nHeight)
{
    for (std::unique_ptr<PackedTexture>& pPackedTexture : maPackedTextures)
    {
        Node* pNode = pPackedTexture->mpRootNode->insert(nWidth, nHeight, 1);
        if (pNode != nullptr)
        {
            return OpenGLTexture(pPackedTexture->mpTexture, pNode->mRectangle, -1);
        }
    }
    CreateNewTexture();
    std::unique_ptr<PackedTexture>& pPackedTexture = maPackedTextures.back();
    Node* pNode = pPackedTexture->mpRootNode->insert(nWidth, nHeight, 1);
    if (pNode != nullptr)
    {
        return OpenGLTexture(pPackedTexture->mpTexture, pNode->mRectangle, -1);
    }
    return OpenGLTexture();
}

OpenGLTexture PackedTextureAtlasManager::InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8* pData)
{
    OpenGLTexture aTexture = Reserve(nWidth, nHeight);
    if (aTexture && pData == nullptr)
        return aTexture;

    aTexture.CopyData(nWidth, nHeight, nFormat, nType, pData);

    return aTexture;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
