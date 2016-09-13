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
public:
    OUString msID;
    NodeType meType;
    std::vector<std::unique_ptr<Node>> mChildren;
    std::unique_ptr<NodePrivate> mPrivate;

    Node(OUString sID, NodeType eType)
        : msID(sID)
        , meType(eType)
    {}

    virtual ~Node()
    {}
};

class VCL_DLLPUBLIC RootNode : public Node
{
public:
    RootNode(OUString sID)
        : Node(sID, NodeType::ROOT)
    {}
};

}} // end vcl::sg namespace

#endif // INCLUDED_VCL_INC_SCENEGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
