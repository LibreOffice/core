/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_nnfinder.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:45:59 $
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

#ifndef ARY_IDL_NNFINDER_HXX
#define ARY_IDL_NNFINDER_HXX

// USED SERVICES
#include "is_ce.hxx"




namespace ary
{
namespace idl
{


/** Gives context info for tree search functions.

    @collab ->ary::Search_SubTree<>()
    @collab ->ary::Search_SubTree_UpTillRoot<>()
*/
class Find_ModuleNode
{
  public:
    typedef Ce_id                           id_type;
    typedef StringVector::const_iterator    name_iterator;

    // LIFECYCLE
                        Find_ModuleNode(
                            const Ce_Storage &  i_rStorage,
                            name_iterator       it_begin,
                            name_iterator       it_end,
                            const String &      i_sName )
                            :   rStorage(i_rStorage),
                                itBegin(it_begin),
                                itEnd(it_end),
                                sName2Search(i_sName) { if (itBegin != itEnd ? (*itBegin).empty() : false) ++itBegin; }
    // OPERATIONS
    const Module *      operator()(
                            id_type             i_id ) const
                            { return i_id.IsValid()
                                        ?   & ary_cast<Module>(rStorage[i_id])
                                        :   0; }

    name_iterator       Begin() const           { return itBegin; }
    name_iterator       End() const             { return itEnd; }
    const String &      Name2Search() const     { return sName2Search; }

  private:
    // DATA
    const Ce_Storage &  rStorage;
    name_iterator       itBegin;
    name_iterator       itEnd;
    String              sName2Search;
};




class Types_forSetCe_Id
{
  public:
    typedef Ce_id                           element_type;
    typedef Ce_Storage                      find_type;

    //  KORR_FUTURE: Check, if this sorting is right or the ary standard
    //  sorting should be used.
    struct sort_type
    {
                        sort_type(
                            const find_type &   i_rFinder )
                                                : rFinder(i_rFinder) {}
        bool            operator()(
                            const element_type   &
                                                i_r1,
                            const element_type   &
                                                i_r2 ) const
        {
            return rFinder[i_r1].LocalName()
                   < rFinder[i_r2].LocalName();
        }

      private:
        const find_type &     rFinder;

    };
};


}   // namespace idl
}   // namespace ary
#endif
