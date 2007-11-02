/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: a_docuaccess.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:23:12 $
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
