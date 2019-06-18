/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>
#include <sal/config.h>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <opengl/framebuffer.hxx>
#include <opengl/texture.hxx>

#include <opengl/PackedTextureAtlas.hxx>

struct Node
{
    tools::Rectangle const mRectangle;
    std::unique_ptr<Node> mLeftNode;
    std::unique_ptr<Node> mRightNode;
    bool mOccupied;

    explicit Node(int nWidth, int nHeight);
    explicit Node(tools::Rectangle const & aRectangle);

    bool isLeaf() const;
    Node* insert(int nWidth, int nHeight, int nPadding);
};

Node::Node(int nWidth, int nHeight)
    : mRectangle(tools::Rectangle(Point(), Size(nWidth, nHeight)))
    , mLeftNode()
    , mRightNode()
    , mOccupied(false)
{}

Node::Node(tools::Rectangle const & aRectangle)
    : mRectangle(aRectangle)
    , mLeftNode()
    , mRightNode()
    , mOccupied(false)
{}

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

        tools::Rectangle aLeftRect;
        tools::Rectangle aRightRect;
        if (dw > dh)
        {
            aLeftRect = tools::Rectangle(Point(mRectangle.Left(), mRectangle.Top()),
                                  Size(nWidth, mRectangle.GetHeight()));
            aRightRect = tools::Rectangle(Point(nPadding + mRectangle.Left() + nWidth, mRectangle.Top()),
                                   Size(mRectangle.GetWidth() - nWidth - nPadding, mRectangle.GetHeight()));
        }
        else
        {
            aLeftRect = tools::Rectangle(Point(mRectangle.Left(), mRectangle.Top()),
                                  Size(mRectangle.GetWidth(), nHeight));
            aRightRect = tools::Rectangle(Point(mRectangle.Left(), nPadding + mRectangle.Top() + nHeight),
                                   Size(mRectangle.GetWidth(), mRectangle.GetHeight() - nHeight - nPadding));
        }

        mLeftNode.reset(new Node(aLeftRect));
        mRightNode.reset(new Node(aRightRect));

        return mLeftNode->insert(nWidth, nHeight, nPadding);
    }
}

struct PackedTexture
{
    std::shared_ptr<ImplOpenGLTexture> mpTexture;
    std::unique_ptr<Node> mpRootNode;

    PackedTexture(int nWidth, int nHeight)
        : mpTexture(new ImplOpenGLTexture(nWidth, nHeight, true))
        , mpRootNode(new Node(nWidth, nHeight))
    {}
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
        pPackedTexture->mpTexture.reset();
    }
}

void PackedTextureAtlasManager::CreateNewTexture()
{
    std::unique_ptr<PackedTexture> pPackedTexture(new PackedTexture(mnTextureWidth, mnTextureHeight));
    GLuint nTextureID = pPackedTexture->mpTexture->mnTexture;
    maPackedTextures.push_back(std::move(pPackedTexture));
    VCL_GL_INFO("PackedTextureAtlas::CreateNewTexture adding texture: " << nTextureID <<
                " atlases: " << maPackedTextures.size());
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

OpenGLTexture PackedTextureAtlasManager::InsertBuffer(int nWidth, int nHeight, int nFormat, int nType, sal_uInt8 const * pData)
{
    OpenGLTexture aTexture = Reserve(nWidth, nHeight);
    if (aTexture && pData == nullptr)
        return aTexture;

    aTexture.CopyData(nWidth, nHeight, nFormat, nType, pData);

    return aTexture;
}

std::vector<GLuint> PackedTextureAtlasManager::ReduceTextureNumber(int nMaxNumberOfTextures)
{
    std::vector<GLuint> aTextureIDs;
    while (int(maPackedTextures.size()) > nMaxNumberOfTextures)
    {
        // Remove oldest created texture
        GLuint nTextureID = maPackedTextures[0]->mpTexture->mnTexture;
        aTextureIDs.push_back(nTextureID);
        maPackedTextures.erase(maPackedTextures.begin());
        VCL_GL_INFO("PackedTextureAtlas::ReduceTextureNumber removing texture: " << nTextureID <<
                    " atlases: " << maPackedTextures.size());
    }
    return aTextureIDs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
