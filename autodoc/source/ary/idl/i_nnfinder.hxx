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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
