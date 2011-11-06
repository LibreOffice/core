/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
