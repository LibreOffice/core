/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: out_node.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:04:03 $
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

Node::Node( const String &  i_name,
            Node &          i_parent )
    :   sName(i_name),
        pParent(&i_parent),
        aChildren(),
        nDepth(i_parent.Depth()+1),
          nNameRoomId(0)
{
}

Node::~Node()
{
    for ( List::iterator it = aChildren.begin();
          it != aChildren.end();
          ++it )
    {
        delete *it;
    }
}

Node &
Node::Provide_Child( const String & i_name )
{
    Node * ret = find_Child(i_name);
    if (ret != 0)
        return *ret;
    return add_Child(i_name);
}

void
Node::Get_Path( StreamStr &         o_result,
                intt                i_maxDepth ) const
{
    // Intentionally 'i_maxDepth != 0', so max_Depth == -1 sets no limit:
    if (i_maxDepth != 0)
    {
        if (pParent != 0)
            pParent->Get_Path(o_result, i_maxDepth-1);
        o_result << sName << '/';
    }
}

void
Node::Get_Chain( StringVector & o_result,
                 intt           i_maxDepth ) const
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

Node *
Node::find_Child( const String & i_name )
{
    // Trying to optimise search in a vector:

    uintt nSize = aChildren.size();
    if ( nSize > 0 AND nSize < 20 )
    {
        List::const_iterator it = aChildren.begin();
        List::const_iterator itMiddle = it + nSize/2;
        int comp = 0;

        if ( i_name < (*itMiddle)->Name() )
        {
            for ( ;
                  it != itMiddle
                        ? (comp = csv::compare(i_name, (*it)->Name())) <= 0
                        : false;
                  ++it )
            {
                if ( comp == 0 )
                    return *it;
            }   // end for
        }
        else
        {
            List::const_iterator itEnd = aChildren.end();
            for ( it = itMiddle;
                  it != itEnd
                        ? (comp = csv::compare(i_name, (*it)->Name())) <= 0
                        : false;
                  ++it )
            {
                if ( comp == 0 )
                    return *it;
            }   // end for
        }
    }
    else if (nSize > 0)
    {
        Node aSearch;
        aSearch.sName = i_name;

        List::const_iterator
            ret = std::lower_bound( aChildren.begin(),
                                    aChildren.end(),
                                    &aSearch,
                                    C_Less_NodePtr );
        if ( ret != aChildren.end() ? (*ret)->Name() == i_name : false )
            return *ret;
    }

    return 0;
}

Node &
Node::add_Child( const String & i_name )
{
    DYN Node * pNew = new Node(i_name,*this);
    aChildren.insert( std::upper_bound( aChildren.begin(),
                                        aChildren.end(),
                                        pNew,
                                        C_Less_NodePtr ),
                      pNew );
    return *pNew;
}

Node &
Node::provide_Child( StringVector::const_iterator i_next,
                       StringVector::const_iterator i_end )
{
    if (i_next == i_end)
        return *this;
    return Provide_Child(*i_next).provide_Child(i_next+1,i_end);
}




Node &
Node::Null_()
{
    return C_aNullNode;
}


}   // namespace output
