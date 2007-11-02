/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_node.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:59:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
