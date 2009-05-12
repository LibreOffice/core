/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: a_docuaccess.cxx,v $
 * $Revision: 1.3 $
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

#include <precomp.h>
#include <ary/adoc/a_docuaccess.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/d_boolean.hxx>
#include <ary/doc/d_docu.hxx>
#include <ary/doc/d_namedlist.hxx>
#include <ary/doc/d_parametrized.hxx>
#include <ary/doc/d_struct.hxx>
#include <ary/doc/d_unit.hxx>




namespace ary
{
namespace adoc
{

using namespace ::ary::doc;
typedef ::ary::doc::nodetype::id nt;



inline ary::doc::Struct &
DocuAccess::Data()
{
    csv_assert(pContainer != 0);
    if (pData != 0)
        return *pData;
    return Get_Data();
}





DocuAccess::DocuAccess( ary::doc::Documentation * io_docu )
    :   pContainer(io_docu)
{
}


ary::doc::Documentation *
DocuAccess::Assign_Container( ary::doc::Documentation * io_docu )
{
    ary::doc::Documentation *
        ret = Release_Container();
    pContainer = io_docu;
    return ret;
}

ary::doc::Documentation *
DocuAccess::Release_Container()
{
    // KORR_FUTURE
    // Is this enough?
    return pContainer;
}

void
DocuAccess::Set_Deprecated()
{
    Data().Add_Node(* new Boolean( nt(ti_deprecated) ));
}

void
DocuAccess::Set_Internal()
{
    Data().Add_Node(* new Boolean( nt(ti_internal) ));
}

void
DocuAccess::Set_Published()
{
    Data().Add_Node(* new Boolean( nt(ti_published) ));
}

ary::doc::HyperText &
DocuAccess::Add_Description()
{
    Unit &
        nu = * new Unit( nt(ti_descr) );
    Data().Add_Node(nu);
    return nu.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_StdTag( E_TagId i_tag )
{
    Unit &
        nu = * new Unit( nt(i_tag) );
    Data().Add_Node(nu);
    return nu.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_base( const String & i_baseTypeName )
{
    Unit &
        nu = * new Unit( nt(ti_base) );
    Get_List(ti_base).Add_Node(     i_baseTypeName,
                                    nu,
                                    NamedList::hdn_put_together );
    return nu.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_collab( const String & i_partner )
{
    Parametrized<String> &
        np = * new Parametrized<String>( nt(ti_collab), i_partner );
    Data().Add_Node(np);
    return np.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_key( const String & i_keyWord )
{
    Parametrized<String> &
        np = * new Parametrized<String>( nt(ti_key), i_keyWord );
    Data().Add_Node(np);
    return np.Doc();
}

typedef std::pair<String, String>   ParamNameAndRange;

ary::doc::HyperText &
DocuAccess::Add_param( const String &      i_parameterName,
                       const String &      i_validRange )
{
    NamedList &
        params = Get_List(ti_param);

    Parametrized<ParamNameAndRange> &
        np = * new Parametrized<ParamNameAndRange>(
                            nt(ti_param),
                            ParamNameAndRange(i_parameterName,i_validRange) );
    params.Add_Node(  i_parameterName,
                      np,
                      NamedList::hdn_put_together );
    return np.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_see( const String & i_referencedEntity )
{
    Parametrized<String> &
        np = * new Parametrized<String>( nt(ti_see), i_referencedEntity );
    Data().Add_Node(np);
    return np.Doc();
}

typedef std::pair<String, String>   SinceInfo;

ary::doc::HyperText &
DocuAccess::Add_since( const String &      i_versionId,
                       const String &      i_fullVersionText )
{
    Parametrized<SinceInfo> &
        np = * new Parametrized<SinceInfo>(
                            nt(ti_since),
                            SinceInfo(i_versionId, i_fullVersionText) );
    Data().Add_Node(np);
    return np.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_throws( const String & i_exceptionName )
{
    Parametrized<String> &
        np = * new Parametrized<String>(
                            nt(ti_throws),
                            i_exceptionName );
    Data().Add_Node(np);
    return np.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_tpl( const String & i_templateParameter )
{
    Unit &
        nu = * new Unit( nt(ti_tpl) );
    Get_List(ti_tpl).Add_Node(      i_templateParameter,
                                    nu,
                                    NamedList::hdn_put_together );
    return nu.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_ExtraTag( const String & i_tagName )
{
    Unit &
        nu = * new Unit( nt(ti_extra) );
    Get_List(ti_extra).Add_Node(    i_tagName,
                                    nu,
                                    NamedList::hdn_normal );
    return nu.Doc();
}

ary::doc::HyperText &
DocuAccess::Add_TagWithMissingParameter(E_TagId i_tag)
{
    // KORR

    Parametrized<bool> &
        np = * new Parametrized<bool>(
                            nt(i_tag),
                            false );
    Data().Add_Node(np);
    return np.Doc();
}

const int C_MainStructNodeId = 1;

ary::doc::Struct &
DocuAccess::Get_Data()
{
    csv_assert(pContainer != 0);
    if (pContainer->Data() == 0)
    {
        pContainer->Set_Data(* new Struct(C_MainStructNodeId));
    }

    // KORR - was, wenn keine struct da ist?
    pData = dynamic_cast< Struct* >(pContainer->Data());
    return *pData;
}

ary::doc::NamedList &
DocuAccess::Get_List(E_TagId i_tag)
{
    Node *
        ret = Data().Slot( nt(i_tag) );
    if (ret == 0)
        ret = & Data().Add_Node(* new NamedList( nt(i_tag) ));
    csv_assert( dynamic_cast< NamedList* >(ret) != 0 );
    return static_cast< NamedList& >(*ret);
}




}   // Namespace adoc
}   // Namespace ary
