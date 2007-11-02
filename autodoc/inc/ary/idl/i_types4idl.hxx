/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_types4idl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:12:28 $
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
