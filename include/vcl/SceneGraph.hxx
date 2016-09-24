/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_VCL_INC_SCENEGRAPH_HXX
#define INCLUDED_VCL_INC_SCENEGRAPH_HXX

#include <rtl/ustring.hxx>
#include <vector>
#include <memory>

namespace vcl
{
namespace sg
{

enum class NodeType
{
    ROOT,
    CLIPPING,
    TRANSFORM,
    GEOMETRY,
};

class VCL_DLLPUBLIC NodePrivate
{
public:
    NodePrivate() = default;

    virtual ~NodePrivate()
    {}
};

class VCL_DLLPUBLIC Node
{
    OUString msName;

public:
    NodeType meType;
    std::vector<std::shared_ptr<Node>> mChildren;
    std::unique_ptr<NodePrivate> mPrivate;

    Node(NodeType eType)
        : meType(eType)
    {}

    virtual ~Node()
    {}

    OUString& getName()
    {
        return msName;
    }

    void setName(const OUString& sName)
    {
        msName = sName;
    }

    std::shared_ptr<Node> findByName(OUString sName)
    {
        for (auto& pChild : mChildren)
        {
            if (pChild->getName() == sName)
                return pChild;

            std::shared_ptr<Node> aFoundNode = pChild->findByName(sName);
            if (aFoundNode)
                return aFoundNode;
        }
        return std::shared_ptr<Node>();
    }
};

}} // end vcl::sg namespace

#endif // INCLUDED_VCL_INC_SCENEGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
