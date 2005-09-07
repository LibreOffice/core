/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sorted_idset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:01:24 $
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

#ifndef ARY_SORTED_IDSET_HXX
#define ARY_SORTED_IDSET_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <set>
    // PARAMETERS
#include "csi_impl.hxx"


template <class XY> class SortedIdSet;

class Interface_2s
{
  public:
    /// Checks for double occurences
    void                Add_ExportingService(
                            Ce_id               i_nId );
    void                Get_ExportingServices(
                            Dyn_StdConstIterator<Ce_id> &
                                                o_rResult ) const;
  private:
    Dyn<SortedIdSet>    pExportingServices;
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

