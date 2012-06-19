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
