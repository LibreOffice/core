/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ia_type.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:49:55 $
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
#include "ia_type.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/qualiname.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_type.hxx>
#include <ary/idl/ip_ce.hxx>
#include "ia_ce.hxx"
#include "is_type.hxx"
#include "it_builtin.hxx"
#include "it_ce.hxx"
#include "it_explicit.hxx"
#include "it_sequence.hxx"
#include "it_tplparam.hxx"
#include "it_xnameroom.hxx"



namespace ary
{
namespace idl
{


inline CeAdmin &
TypeAdmin::my_Ces() const
    { return *pCes; }

inline void
TypeAdmin::lhf_Put2Storage_and_AssignId( DYN Type & pass_io_rType )
    { // This also assigns an ID to pass_io_rType:
      Storage().Store_Entity(pass_io_rType); }

inline Type_id
TypeAdmin::lhf_findBuiltInType( const String & i_sName )
    { return ary_cast<ExplicitNameRoom>(Storage()[nXNameRoom_Root])
                        .Search_Name(i_sName); }

inline const ExplicitNameRoom &
TypeAdmin::find_ExplicitNameRoom( Type_id i_nType ) const
{
    return ary_cast<ExplicitNameRoom>(Storage()[i_nType]);
}

inline ExplicitNameRoom &
TypeAdmin::find_ExplicitNameRoom( Type_id i_nType )
{
    return ary_cast<ExplicitNameRoom>(Storage()[i_nType]);
}

ExplicitNameRoom &
TypeAdmin::lhf_CheckIn_XNameRoom( const QualifiedName & i_rName,
                                       Ce_id                 i_nModuleOfOccurrence )
{
    Type_id nRoot = i_rName.IsAbsolute()
                        ?   Type_id( predefined::type_GlobalXNameRoom )
                        :   lhf_Get_NameRoomRoot_forModuleofOccurrence( i_nModuleOfOccurrence ).TypeId();

    if ( i_rName.NamespaceDepth() == 0 )
        return find_ExplicitNameRoom(nRoot);

    QualifiedName::namespace_iterator it = i_rName.first_namespace();
    ExplicitNameRoom *
        ret = & find_ExplicitNameRoom(nRoot);
    for ( ; it != i_rName.end_namespace(); ++it )
    {
        Type_id
            found = ret->Search_Name(*it);
        if (found.IsValid())
        {
            ret = & find_ExplicitNameRoom(found);
        }
        else
        {
            ExplicitNameRoom &
                rNew = *new ExplicitNameRoom(*it, *ret);
            lhf_Put2Storage_and_AssignId(rNew);
            ret->Add_Name( rNew.Name(), rNew.TypeId() );
            ret = &rNew;
        }

    }   // end for
    return *ret;
}

Type_id
TypeAdmin::lhf_CheckIn_TypeName( const String &        i_sLocalName,
                                      ExplicitNameRoom &    io_rXNameRoom,
                                      Ce_id                 i_nModuleOfOccurrence,
                                      Type_id               i_nTemplateType )
{
    String sSearchLocalName( i_sLocalName );
    if ( i_nTemplateType.IsValid() )
    {
        sSearchLocalName =
            StreamLock(200)()
                << i_sLocalName
                << C_cTemplateDelimiter
                << i_nTemplateType.Value()
                << c_str;
    }

    Type_id
        ret = io_rXNameRoom.Search_Name(sSearchLocalName);
    if (NOT ret.IsValid())
    {
        DYN Type &
            rNewType = *new ExplicitType(  i_sLocalName,
                                           io_rXNameRoom.TypeId(),
                                           i_nModuleOfOccurrence,
                                           i_nTemplateType );
        lhf_Put2Storage_and_AssignId(rNewType);
        ret = rNewType.TypeId();
        io_rXNameRoom.Add_Name( sSearchLocalName, ret );
    }
    return ret;
}

Type_id
TypeAdmin::lhf_CheckIn_Sequence(Type_id i_nType)
{
    Type_id
        ret = Storage().Search_SequenceOf(i_nType);

    if (NOT ret.IsValid())
    {
        DYN Type &
            rNewSeq = *new Sequence(i_nType);
        lhf_Put2Storage_and_AssignId(rNewSeq);
        ret = rNewSeq.Id();
        Storage().Add_Sequence(i_nType, ret);
    }
    return ret;
}

void
TypeAdmin::lhf_CheckIn_BuiltInType( const char *       i_sName,
                                    Rid                i_nId )
{
    DYN BuiltInType &
        rNewType = *new BuiltInType(i_sName);
    Storage().Set_Reserved(i_nId, rNewType);

    // Put them into both roots, to catch the syntactically correct
    //   (though unlikely) ::Any, ::long etc.
    Type_id
        nId(i_nId);
    find_ExplicitNameRoom(nXNameRoom_Root).Add_Name(i_sName, nId);
    find_ExplicitNameRoom(nXNameRoom_Global).Add_Name(i_sName, nId);
}

ExplicitNameRoom &
TypeAdmin::lhf_Get_NameRoomRoot_forModuleofOccurrence( Ce_id i_nModuleOfOccurrence )
{
    const Type_id *
        pFound = csv::find_in_map( aMap_ModuleOfOccurrence2NameRoomRoot,
                                   i_nModuleOfOccurrence );
    if (pFound != 0)
        return find_ExplicitNameRoom(*pFound);

    ExplicitNameRoom &
        ret = *new ExplicitNameRoom;
    lhf_Put2Storage_and_AssignId(ret);
    aMap_ModuleOfOccurrence2NameRoomRoot.insert(std::pair< const Ce_id, Type_id>(i_nModuleOfOccurrence,ret.TypeId()));
    return ret;
}

TypeAdmin::TypeAdmin()
    :   pStorage(new Type_Storage),
        pCes(0),    // Needs to be set directly after creation.
        nXNameRoom_Root( static_cast<ary::Rid>(predefined::type_Root_ofXNameRooms) ),
        nXNameRoom_Global( static_cast<ary::Rid>(predefined::type_GlobalXNameRoom) ),
        aMap_ModuleOfOccurrence2NameRoomRoot()
{
    DYN ExplicitNameRoom &
        drRoot = *new ExplicitNameRoom;
    Storage().Set_Reserved( nXNameRoom_Root.Value(), drRoot );

    DYN ExplicitNameRoom &
        drGlobal = *new ExplicitNameRoom(String::Null_(), drRoot);
    Storage().Set_Reserved( nXNameRoom_Global.Value(), drGlobal );
    drRoot.Add_Name( drGlobal.Name(), nXNameRoom_Global );

    lhf_Setup_BuildInTypes();
}

TypeAdmin::~TypeAdmin()
{
}

void
TypeAdmin::lhf_Setup_BuildInTypes()
{
    lhf_CheckIn_BuiltInType("any", predefined::type_any);
    lhf_CheckIn_BuiltInType("boolean", predefined::type_boolean);
    lhf_CheckIn_BuiltInType("byte", predefined::type_byte);
    lhf_CheckIn_BuiltInType("char", predefined::type_char);
    lhf_CheckIn_BuiltInType("double", predefined::type_double);
    lhf_CheckIn_BuiltInType("float", predefined::type_float);
    lhf_CheckIn_BuiltInType("hyper", predefined::type_hyper);
    lhf_CheckIn_BuiltInType("long", predefined::type_long);
    lhf_CheckIn_BuiltInType("short", predefined::type_short);
    lhf_CheckIn_BuiltInType("string", predefined::type_string);
    lhf_CheckIn_BuiltInType("type", predefined::type_type);
    lhf_CheckIn_BuiltInType("void", predefined::type_void);
    lhf_CheckIn_BuiltInType("unsigned hyper", predefined::type_u_hyper);
    lhf_CheckIn_BuiltInType("unsigned long", predefined::type_u_long);
    lhf_CheckIn_BuiltInType("unsigned short", predefined::type_u_short);
}

const Type &
TypeAdmin::CheckIn_Type( QualifiedName &     i_rFullName,
                                 uintt               i_nSequenceCount,
                                 Ce_id               i_nModuleOfOccurrence,
                                 Type_id             i_nTemplateType )
{
    // Look in built-in types:
    Type_id
        nType = lhf_findBuiltInType(i_rFullName.LocalName());
    if (NOT nType.IsValid())
    {   // No built-in type:
        ExplicitNameRoom &
            rNameRoom = lhf_CheckIn_XNameRoom(i_rFullName,i_nModuleOfOccurrence);
        nType = lhf_CheckIn_TypeName( i_rFullName.LocalName(),
                                      rNameRoom,
                                      i_nModuleOfOccurrence,
                                      i_nTemplateType );
    }   // endif

    for ( uintt s = 0; s < i_nSequenceCount; ++s )
    {
        nType = lhf_CheckIn_Sequence(nType);
    }

    return Storage()[nType];
}

TemplateParamType &
TypeAdmin::Store_TemplateParamType( String i_sName )
{
    DYN TemplateParamType &
        ret = *new TemplateParamType( i_sName );
    lhf_Put2Storage_and_AssignId(ret);
    return ret;
}

const Type &
TypeAdmin::Find_Type( Type_id i_nType ) const
{
    return Storage()[i_nType];
}

String
TypeAdmin::Search_LocalNameOf( Type_id i_nType ) const
{
    const Type *
        pType = Storage().Exists(i_nType)
                    ? 0
                    : & Storage()[i_nType];
    if (pType != 0)
    {
        switch (pType->AryClass())
        {
            case Ce_Type::class_id:
            case ExplicitType::class_id:
            case BuiltInType::class_id:
                    return static_cast< const Named_Type& >(*pType).Name();
        }
    }
    return String::Null_();
}

Ce_id
TypeAdmin::Search_CeRelatedTo( Type_id i_nType ) const
{
    const Ce_Type *
        ret = ary_cast<Ce_Type>( & Storage()[i_nType] );
    return ret != 0
            ?   ret->RelatedCe()
            :   Ce_id_Null();
}

const ExplicitNameRoom &
TypeAdmin::Find_XNameRoom( Type_id i_nType ) const
{
    return find_ExplicitNameRoom(i_nType);
}

bool
TypeAdmin::IsBuiltInOrRelated( const Type & i_rType ) const
{
    if ( is_type<BuiltInType>(i_rType) )
        return true;
    else
    {
        const Type *
            pType = &i_rType;
        while (is_type<Sequence>(*pType))
        {
            Type_id
                nt = ary_cast<Sequence>(pType)->RelatedType();
            if (NOT nt.IsValid())
                return false;
            pType = & Storage()[nt];
        }
        return is_type<BuiltInType>(*pType);
    }
}



}   // namespace idl
}   // namespace ary
