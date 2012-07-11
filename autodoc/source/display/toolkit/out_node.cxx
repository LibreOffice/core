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

#include <precomp.h>
#include <toolkit/out_node.hxx>


// NOT FULLY DEFINED SERVICES
#include <algorithm>


namespace output
{


namespace
{

struct Less_NodePtr
{
    bool                operator()(
                            Node *              p1,
                            Node *              p2 ) const
                        { return p1->Name() < p2->Name(); }
};

struct Less_NodePtr     C_Less_NodePtr;


Node  C_aNullNode(Node::null_object);


}   // namepace anonymous


//**********************        Node        ***************************//


Node::Node()
    :   sName(),
        pParent(0),
        aChildren(),
        nDepth(0),
        nNameRoomId(0)
{
}

Node::Node( E_NullObject )
    :   sName(),
        pParent(0),
        aChildren(),
         nDepth(-1),
        nNameRoomId(0)
{
}

Node::Node(
    const String &  i_name,
    Node &          i_parent
) :
    sName(i_name),
    pParent(&i_parent),
    aChildren(),
    nDepth(i_parent.Depth()+1),
    nNameRoomId(0)
{
}

Node::~Node()
{
    for ( NodeList::iterator it = aChildren.begin();
          it != aChildren.end();
          ++it )
    {
        delete *it;
    }
}

Node& Node::Provide_Child( const String & i_name )
{
    Node* ret = find_Child(i_name);
    if (ret != 0)
        return *ret;
    return add_Child(i_name);
}

void Node::Get_Path(
    StreamStr&  o_result,
    intt        i_maxDepth
) const
{
    // Intentionally 'i_maxDepth != 0', so max_Depth == -1 sets no limit:
    if (i_maxDepth != 0)
    {
        if (pParent != 0)
            pParent->Get_Path(o_result, i_maxDepth-1);
        o_result << sName << '/';
    }
}

void Node::Get_Chain(
    StringVector & o_result,
    intt           i_maxDepth
) const
{
    if (i_maxDepth != 0)
    {
        // This is called also for the toplevel Node,
        //   but there happens nothing:
        if (pParent != 0)
        {
            pParent->Get_Chain(o_result, i_maxDepth-1);
            o_result.push_back(sName);
        }
    }
}

Node* Node::find_Child( const String & i_name )
{
    Node aSearch;
    aSearch.sName = i_name;

    NodeList::const_iterator
        ret = std::lower_bound( aChildren.begin(),
                                aChildren.end(),
                                &aSearch,
                                C_Less_NodePtr );
    if ( ret != aChildren.end() ? (*ret)->Name() == i_name : false )
        return *ret;

    return 0;
}

Node& Node::add_Child( const String & i_name )
{
    DYN Node* pNew = new Node(i_name,*this);
    aChildren.insert( std::lower_bound( aChildren.begin(),
                                        aChildren.end(),
                                        pNew,
                                        C_Less_NodePtr ),
                      pNew );
    return *pNew;
}

Node& Node::provide_Child(
    StringVector::const_iterator i_next,
    StringVector::const_iterator i_end
)
{
    if (i_next == i_end)
        return *this;
    return Provide_Child(*i_next).provide_Child(i_next+1,i_end);
}




Node& Node::Null_()
{
    return C_aNullNode;
}


}   // namespace output

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
