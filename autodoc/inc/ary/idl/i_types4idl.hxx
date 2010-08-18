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

#ifndef ARY_IDL_I_TYPES4IDL_HXX
#define ARY_IDL_I_TYPES4IDL_HXX

// USED SERVICES
#include <ary/types.hxx>




namespace ary
{
namespace idl
{
    class Module;
    class CodeEntity;
    class Type;
    class Gate;
    class CePilot;
    class TypePilot;


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
