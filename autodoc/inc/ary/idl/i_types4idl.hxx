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

#ifndef ARY_IDL_I_TYPES4IDL_HXX
#define ARY_IDL_I_TYPES4IDL_HXX

// USED SERVICES
#include <ary/types.hxx>




namespace ary
{
namespace idl
{
    class CodeEntity;
    class Type;


typedef TypedId<CodeEntity>         Ce_id;
typedef TypedId<Type>               Type_id;


/** This is used when an ->ary::idl::ExplicitType
    represents a templated struct and is sorted into the
    dictionary of an ->ary::idl::XNameRoom.
    Then local type name and template type id are concatenated
    to one string with this char as delimiter.
*/
const char C_cTemplateDelimiter = '<';

typedef std::vector<Ce_id>          Ce_idList;


enum E_ParameterDirection
{
    param_in,
    param_out,
    param_inout
};


enum E_SightLevel
{
    sl_Module,          // not file bound entities, like modules
    sl_File,            // entities on top level within one file, like interface or enum
    sl_Member           // member entities, like enumvalue or function
};

inline Ce_id
Ce_id_Null()
{
    return Ce_id(0);
}

inline void
NullPush_IdList(Ce_idList * o_pList)
{
    if (o_pList)
        o_pList->push_back( Ce_id_Null() );
}

inline void
NullPush_IdList_2(Ce_idList * o_pList)
{
    if (o_pList)
    {
        o_pList->push_back( Ce_id_Null() );
        o_pList->push_back( Ce_id_Null() );
    }
}

namespace alphabetical_index
{
    enum E_Letter
    {
        a = int('a'),
        b,
        c,
        d,
        e,
        f,
        g,
        h,
        i,
        j,
        k,
        l,
        m,
        n,
        o,
        p,
        q,
        r,
        s,
        t,
        u,
        v,
        w,
        x,
        y,
        z,
        non_alpha = int('_'),
        MAX
    };
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
