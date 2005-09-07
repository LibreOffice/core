/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_nnfinder.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:43:57 $
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
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <store/st_access.hxx>
#include <store/st_root.hxx>
#include <store/st_unit.hxx>
#include "is_ce.hxx"

namespace ary
{
namespace idl
{


/**
*/
class Find_ModuleNode
{
  public:
    typedef Ce_id                           id_type;
    typedef Module                          node_type;
    typedef StringVector::const_iterator    name_iterator;

    // LIFECYCLE
                        Find_ModuleNode(
                            const Ce_Storage &  i_rStorage,
                            StringVector::const_iterator
                                                it_begin,
                            StringVector::const_iterator
                                                it_end,
                            const String &      i_sName )
                            :   rStorage(i_rStorage),
                                itBegin(it_begin),
                                itEnd(it_end),
                                sName2Search(i_sName) { if (itBegin != itEnd ? (*itBegin).empty() : false) ++itBegin; }
    // OPERATIONS
    const node_type *   operator()(
                            id_type             i_nId ) const
                            { return store::search( rStorage, i_nId, T2T<Module>()); }

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




/** Implementation of a node in a namespace-tree.
*/
class Types_forSetCe_Id
{
  public:
    typedef Ce_id                           element_type;
    typedef Ce_Storage                      find_type;

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
            return rFinder[i_r1].Entity().LocalName()
                   < rFinder[i_r2].Entity().LocalName();
        }

      private:
        const find_type &     rFinder;

    };
};


}   // namespace idl
}   // namespace ary


#endif
