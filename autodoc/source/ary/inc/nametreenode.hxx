/*************************************************************************
 *
 *  $RCSfile: nametreenode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:20:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ARY_NAMETREENODE_HXX
#define ARY_NAMETREENODE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/template/tpltools.hxx>
#include <sci_impl.hxx>
// HACK because of SunPro 5.2 compiler bug with templates:
#include <ary/idl/i_module.hxx>



namespace ary
{



/** Implementation of a node in a namespace-tree.
*/
template<class ITEM_ID>
class NameTreeNode
{
  public:
    typedef NameTreeNode                    self;
    typedef ITEM_ID                         item_id;
    typedef StringVector::const_iterator    name_iterator;
    typedef std::map<String, item_id>       Map_LocalNames;

    // LIFECYCLE
                        NameTreeNode();
                        NameTreeNode(
                            const String &      i_sName,
                            const self &        i_rParent,
                            ITEM_ID             i_nParentId );
    virtual             ~NameTreeNode();

    // OPERATIONS
    void                Add_Name(
                            const String &      i_sName,
                            item_id             i_nId );
    // INQUIRY
    const String &      Name() const            { return Depth() > 0 ? aCompleteNameChain.back() : String::Null_(); }
    item_id             Parent() const          { return nParent; }
    intt                Depth() const           { return aCompleteNameChain.size(); }

    bool                IsEquivalent(
                            const NameTreeNode &
                                                i_rNode ) const;
    name_iterator       NameChain_Begin() const { return aCompleteNameChain.begin(); }
    name_iterator       NameChain_End() const   { return aCompleteNameChain.end(); }

    item_id             Search_Name(
                            const String &      i_sName ) const;
    void                Get_Names(
                            Dyn_StdConstIterator<ITEM_ID> &
                                                o_rResult ) const;
    const Map_LocalNames &
                        LocalNames() const      { return aLocalNames; }
  private:
    // Locals
    Map_LocalNames &    LocalNames()            { return aLocalNames; }

    // DATA
    Map_LocalNames      aLocalNames;
    StringVector        aCompleteNameChain;
    item_id             nParent;
};


#if 0
//  Prototypes have to be left out, because of a VC++ 6 compiler bug
//     ( compiler reports ambiguous overload ).

template <class FIND_NODE>
typename FIND_NODE::id_type
Search_SubTree( const typename FIND_NODE::node_type &   i_rStart,
                const FIND_NODE &                       i_rNodeFinder );

template <class  FIND_NODE>
typename FIND_NODE::id_type
Search_SubTree_UpTillRoot( const typename FIND_NODE::node_type &    i_rStart,
                           const FIND_NODE &                        i_rNodeFinder );
#endif // 0



// IMPLEMENTATION

template<class ITEM_ID>
NameTreeNode<ITEM_ID>::NameTreeNode()
    :   aLocalNames(),
        aCompleteNameChain(),
        nParent(0)
{
}

template<class ITEM_ID>
NameTreeNode<ITEM_ID>::NameTreeNode( const String &      i_sName,
                                     const self &        i_rParent,
                                     ITEM_ID             i_nParentId )
    :   aLocalNames(),
        aCompleteNameChain(),
        nParent(i_nParentId)
{
    aCompleteNameChain.reserve(i_rParent.Depth()+1);
    for ( name_iterator it = i_rParent.NameChain_Begin();
          it != i_rParent.NameChain_End();
          ++it )
    {
        aCompleteNameChain.push_back(*it);
    }
    aCompleteNameChain.push_back(i_sName);
}

template<class ITEM_ID>
NameTreeNode<ITEM_ID>::~NameTreeNode()
{
}


template<class ITEM_ID>
inline void
NameTreeNode<ITEM_ID>::Add_Name( const String &      i_sName,
                                 item_id             i_nId )
{
    LocalNames().insert( Map_LocalNames::value_type(i_sName, i_nId) );
}


template<class ITEM_ID>
inline bool
NameTreeNode<ITEM_ID>::IsEquivalent( const NameTreeNode & i_rNode ) const
{
    return aCompleteNameChain == i_rNode.aCompleteNameChain;
}

template<class ITEM_ID>
inline ITEM_ID
NameTreeNode<ITEM_ID>::Search_Name( const String & i_sName ) const
{
    return csv::value_from_map(LocalNames(),i_sName);
}

template<class ITEM_ID>
inline void
NameTreeNode<ITEM_ID>::Get_Names( Dyn_StdConstIterator<ITEM_ID> & o_rResult ) const
{
    o_rResult = new SCI_DataInMap<String,item_id>(LocalNames());
}


// HACK because of SunPro 5.2 compiler bug with templates:
//   ary::idl::Module has to be "FIND_NODE::node_type"
//   must be solved later somehow.
template <class FIND_NODE>
typename FIND_NODE::id_type
Search_SubTree( const ary::idl::Module &    i_rStart,
                const FIND_NODE &           i_rNodeFinder )
{
    const typename FIND_NODE::node_type *
        ret = &i_rStart;

    for ( StringVector::const_iterator  it = i_rNodeFinder.Begin();
          it != i_rNodeFinder.End() AND ret != 0;
          ++it )
    {
        ret = i_rNodeFinder(ret->Search_Name(*it));
    }

    typename FIND_NODE::id_type nret(0);
    return ret != 0
            ?   ret->Search_Name(i_rNodeFinder.Name2Search())
            :   nret;
}

template <class  FIND_NODE>
typename FIND_NODE::id_type
Search_SubTree_UpTillRoot( const ary::idl::Module &    i_rStart,
                           const FIND_NODE &           i_rNodeFinder )
{
    typename FIND_NODE::id_type
        ret(0);
    for ( const typename FIND_NODE::node_type * start = &i_rStart;
          start != 0 AND NOT ret.IsValid();
          start = i_rNodeFinder(start->Owner()) )
    {
        ret = Search_SubTree( *start,
                              i_rNodeFinder );
    }
    return ret;
}
// END Hack for SunPro 5.2 compiler bug.


}   // namespace ary


#endif
