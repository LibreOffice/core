/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef ARY_IDL_I_CE2S_HXX
#define ARY_IDL_I_CE2S_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>


namespace ary
{

namespace idl
{



/** Abstract base for all secondary productions of code entities
*/
class Ce_2s
{
  public:
    // LIFECYCLE
    virtual             ~Ce_2s();

    static DYN Ce_2s *  Create_(
                            ClassId             i_nCeClass );
    // OPERATIONS
    void                Add_Link2DescriptionInManual(
                            const String &      i_link,
                            const String &      i_linkUI )
                                                { aDescriptionsInManual.push_back(i_link); aDescriptionsInManual.push_back(i_linkUI); }
    void                Add_Link2RefInManual(
                            const String &      i_link,
                            const String &      i_linkUI )
                                                { aRefsInManual.push_back(i_link); aRefsInManual.push_back(i_linkUI); }
    std::vector<Ce_id> &
                        Access_List(
                            int                 i_indexOfList );
    // INQUIRY
    const StringVector &
                        Links2DescriptionInManual() const
                                                { return aDescriptionsInManual; }
    const StringVector &
                        Links2RefsInManual() const
                                                { return aRefsInManual; }
    int                 CountXrefLists() const  { return aXrefLists.size(); }
    const std::vector<Ce_id> &
                        List(
                            int                 i_indexOfList ) const;
  private:
    typedef DYN std::vector<Ce_id> * ListPtr;

    // DATA
    StringVector        aDescriptionsInManual;
    StringVector        aRefsInManual;
    std::vector<ListPtr>
                        aXrefLists;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
