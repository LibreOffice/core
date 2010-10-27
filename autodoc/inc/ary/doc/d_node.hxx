/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ARY_DOC_D_NODE_HXX
#define ARY_DOC_D_NODE_HXX

// BASE CLASSES
#include <cosv/tpl/processor.hxx>
// USED SERVICES
#include <cosv/tpl/vvector.hxx>
#include <ary/doc/d_types4doc.hxx>




namespace ary
{
namespace doc
{


/** The abstract base class for any type of documentation content.

    A ->Documentation has as content a hierarchy of Nodes, each can be a
    different kind of content, like descriptions of single items or structs
    or lists of Nodes.
*/
class Node : public csv::ConstProcessorClient
{
  public:
    // LIFECYCLE
    virtual             ~Node();

    // OPERATIONS
    void                Add_toChain(
                            DYN Node &          pass_nextNode );
    // INQUIRY
    nodetype::id        Type() const;
    const Node *        Next() const;
    bool                IsSingle() const;
    uintt               ListSize() const;

  protected:
    explicit            Node(
                            nodetype::id        i_type);
  private:
    // Forbid copies:
    Node(const Node&);
    Node & operator=(const Node&);

    // DATA
    nodetype::id        nType;
    Dyn<Node>           pNext;      /// Next ->Node in same list.
};

typedef csv::VirtualVector<Node>    NodeList;




// IMPLEMENTATION
inline nodetype::id
Node::Type() const
{
    return nType;
}

inline const Node *
Node::Next() const
{
    return pNext.Ptr();
}

inline bool
Node::IsSingle() const
{
    return pNext.operator bool();
}




}   // namespace doc
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
