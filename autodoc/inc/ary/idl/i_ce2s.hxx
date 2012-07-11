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

#ifndef ARY_IDL_I_CE2S_HXX
#define ARY_IDL_I_CE2S_HXX

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
    explicit            Ce_2s() {}
    virtual             ~Ce_2s();

    static DYN Ce_2s *  Create_();
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
