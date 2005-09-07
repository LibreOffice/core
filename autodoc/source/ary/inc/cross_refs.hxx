/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cross_refs.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:57:57 $
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

#ifndef ARY_CROSS_REFS_HXX
#define ARY_CROSS_REFS_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include "sorted_idset.hxx"


template <class VALUE_LIST, class TYPES>
class CrossReferences
{
  public:
    typedef TYPES::element_type         element;

    /// Checks for double occurences
    void                Add(
                            VALUE_LIST::index_type
                                                i_nPosition
                            const element &     i_rElem );
    void                Get_List(
                            Dyn_StdConstIterator<element> &
                                                o_rResult ) const;
  private:
    SortedIdSet<TYPES>  aData[VALUE_LIST::max];
};



namespace ary
{

template <class TYPES>
class SortedIdSet
{
  public:
    typedef typename TYPES::element_type   element;
    typedef typename TYPES::sort_type      sorter;
    typedef typename TYPES::find_type      finder;

                        SortedIdSet(
                            const finder &      i_rFinder )
                                                : aSorter(i_rFinder),
                                                  aData(aSorter) {}
                        ~SortedIdSet()          {}

    void                Get_Begin(
                            Dyn_StdConstIterator<element> &
                                                o_rResult )
                                                { o_rResult = new SCI_Set<FINDER>(aData); }
    void                Add(
                            const element &     i_rElement )
                                                { aData.insert(i_rElement); }

  private:
    typedef std::set<element, sorter>       Set;

    // DATA
    sorter              aSorter;
    Set                 aData;
};


}   // namespace ary



#endif

