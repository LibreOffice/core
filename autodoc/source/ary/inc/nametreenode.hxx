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

#ifndef ARY_NAMETREENODE_HXX
#define ARY_NAMETREENODE_HXX
//  KORR_DEPRECATED_3.0
//      Replace by ::ary::symtree::Node.

// USED SERVICES
#include <cosv/tpl/tpltools.hxx>
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
    LocalNames().insert( typename Map_LocalNames::value_type(i_sName, i_nId) );
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
    return csv::value_from_map(LocalNames(),i_sName, ITEM_ID(0));
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
    const ary::idl::Module *
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
    for ( const ary::idl::Module * start = &i_rStart;
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
