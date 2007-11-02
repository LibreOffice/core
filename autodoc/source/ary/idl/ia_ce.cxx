/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ia_ce.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:49:24 $
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
#include "ia_ce.hxx"


// NOT FULLY DEFINED SERVICES
#include <algorithm>
#include <cosv/tpl/tpltools.hxx>
#include <ary/idl/i_attribute.hxx>
#include <ary/idl/i_constant.hxx>
#include <ary/idl/i_constgroup.hxx>
#include <ary/idl/i_enum.hxx>
#include <ary/idl/i_enumvalue.hxx>
#include <ary/idl/i_exception.hxx>
#include <ary/idl/i_function.hxx>
#include <ary/idl/i_interface.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_namelookup.hxx>
#include <ary/idl/i_property.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/i_singleton.hxx>
#include <ary/idl/i_siservice.hxx>
#include <ary/idl/i_sisingleton.hxx>
#include <ary/idl/i_struct.hxx>
#include <ary/idl/i_structelem.hxx>
#include <ary/idl/i_traits.hxx>
#include <ary/idl/i_typedef.hxx>
#include <idsort.hxx>
#include "ia_type.hxx"
#include "is_ce.hxx"
#include "it_tplparam.hxx"




namespace ary
{
namespace idl
{

inline Module &
CeAdmin::lhf_Access_Module( Ce_id i_nId )
    { return ary_cast<Module>(Storage()[i_nId]); }

inline void
CeAdmin::lhf_Put2Storage_and_AssignId( CodeEntity & pass_io_rCe )
    { // This also assigns an ID to pass_io_rCe:
      Storage().Store_Entity(pass_io_rCe);
      my_NameDictionary().Add_Name( pass_io_rCe.LocalName(),
                                    pass_io_rCe.CeId(),
                                    pass_io_rCe.AryClass(),
                                    pass_io_rCe.Owner() );
    }

inline void
CeAdmin::lhf_Store_NewEntity( DYN CodeEntity &  pass_io_rCe,
                                   Module &          i_rOwner )
{
    lhf_Put2Storage_and_AssignId(pass_io_rCe);
    i_rOwner.Add_Name(pass_io_rCe.LocalName(), pass_io_rCe.CeId());
}

inline void
CeAdmin::lhf_Store_NewEntity( DYN CodeEntity &  pass_io_rCe,
                                   Ce_id             i_nOwnerModule )
{
    lhf_Store_NewEntity(pass_io_rCe, lhf_Access_Module(i_nOwnerModule));
}



CeAdmin::CeAdmin( NameLookup &        io_rNameDictionary,
                  TypeAdmin &         io_rTypePilot )
    :   pStorage(new Ce_Storage),
        pGlobalNamespace(0),
        pNameDictionary(&io_rNameDictionary),
        pTypePilot(&io_rTypePilot)
{
    Storage().Set_Reserved(
                    predefined::ce_GlobalNamespace,
                    *new Module );
    pGlobalNamespace = &lhf_Access_Module(Ce_id(predefined::ce_GlobalNamespace));
}

CeAdmin::~CeAdmin()
{
}



Module &
CeAdmin::CheckIn_Module( Ce_id               i_nParentId,
                            const String &      i_sName )
{
    Module & rOwner  = lhf_Access_Module(i_nParentId);
    Ce_id    nId     = rOwner.Search_Name(i_sName);
    if (nId.IsValid())
    {
        return lhf_Access_Module(nId);
    }

    Module & ret = *new Module( i_sName,
                                rOwner );
    lhf_Store_NewEntity(ret, rOwner);
    return ret;
}

Service &
CeAdmin::Store_Service( Ce_id           i_nOwner,
                        const String &  i_sName )
{
    Service & ret = *new Service( i_sName,
                                  i_nOwner );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

SglIfcService &
CeAdmin::Store_SglIfcService( Ce_id               i_nOwner,
                                   const String &      i_sName,
                                   Type_id             i_nBaseInterface )
{
    SglIfcService &
        ret = *new SglIfcService( i_sName,
                                  i_nOwner,
                                  i_nBaseInterface );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

Interface &
CeAdmin::Store_Interface( Ce_id               i_nOwner,
                                  const String &      i_sName,
                                  Type_id             i_nBase )
{
    Interface & ret = *new Interface( i_sName,
                                      i_nOwner );
    lhf_Store_NewEntity(ret, i_nOwner);
    if (i_nBase.IsValid())
        ret.Add_Base(i_nBase, 0);
    return ret;
}

Struct &
CeAdmin::Store_Struct( Ce_id               i_nOwner,
                               const String &      i_sName,
                               Type_id             i_nBase,
                               const String &      i_sTemplateParam )
{
    if (NOT i_sTemplateParam.empty())
    {
        return lhf_Store_TplStruct( i_nOwner,
                                    i_sName,
                                    i_nBase,
                                    i_sTemplateParam );
    }

    Struct & ret = *new Struct( i_sName,
                                i_nOwner,
                                i_nBase,
                                String::Null_(),
                                Type_id::Null_() );
    lhf_Store_NewEntity(ret, i_nOwner);

    return ret;
}

Exception &
CeAdmin::Store_Exception( Ce_id               i_nOwner,
                                  const String &      i_sName,
                                  Type_id             i_nBase )
{
    Exception & ret = *new Exception( i_sName,
                                      i_nOwner,
                                      i_nBase );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

Enum &
CeAdmin::Store_Enum( Ce_id               i_nOwner,
                             const String &      i_sName )
{
    Enum & ret = *new Enum( i_sName,
                            i_nOwner );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

Typedef &
CeAdmin::Store_Typedef( Ce_id               i_nOwner,
                                const String &      i_sName,
                                Type_id             i_nDefiningType )
{
    Typedef & ret = *new Typedef( i_sName,
                                  i_nOwner,
                                  i_nDefiningType );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}


ConstantsGroup &
CeAdmin::Store_ConstantsGroup( Ce_id               i_nOwner,
                                       const String &      i_sName )
{
    ConstantsGroup & ret = *new ConstantsGroup( i_sName,
                                                i_nOwner );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

Singleton &
CeAdmin::Store_Singleton( Ce_id               i_nOwner,
                                  const String &      i_sName )
{
    Singleton & ret = *new Singleton( i_sName,
                                      i_nOwner );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

SglIfcSingleton &
CeAdmin::Store_SglIfcSingleton( Ce_id               i_nOwner,
                                     const String &      i_sName,
                                     Type_id             i_nBaseInterface )
{
    SglIfcSingleton &
        ret = *new SglIfcSingleton( i_sName,
                                    i_nOwner,
                                    i_nBaseInterface );
    lhf_Store_NewEntity(ret, i_nOwner);
    return ret;
}

Constant &
CeAdmin::Store_Constant( Ce_id               i_nOwner,
                                 const String &      i_sName,
                                 Type_id             i_nType,
                                 const String &      i_sValue )
{
    ConstantsGroup &
        rOwner  = ary_cast<ConstantsGroup>(Storage()[i_nOwner]);
    Constant & ret = *new Constant( i_sName,
                                    i_nOwner,
                                    rOwner.NameRoom(),
                                    i_nType,
                                    i_sValue );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Constant(ret.CeId());
    return ret;
}

Property &
CeAdmin::Store_Property( Ce_id                  i_nOwner,
                                 const String &         i_sName,
                                 Type_id                i_nType,
                                 Property::Stereotypes  i_stereotypes )
{
    Service &
        rOwner = ary_cast<Service>(Storage()[i_nOwner]);
    Property & ret = *new Property( i_sName,
                                    i_nOwner,
                                    rOwner.NameRoom(),
                                    i_nType,
                                    i_stereotypes );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Property(ret.CeId());
    return ret;
}

Function &
CeAdmin::Store_Function( Ce_id               i_nOwner,
                                 const String &      i_sName,
                                 Type_id             i_nReturnType,
                                 bool                i_bOneWay )
{
    Interface &
        rOwner = ary_cast<Interface>(Storage()[i_nOwner]);
    Function & ret = *new Function( i_sName,
                                    i_nOwner,
                                    rOwner.NameRoom(),
                                    i_nReturnType,
                                      i_bOneWay);
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Function(ret.CeId());
    return ret;
}

Function &
CeAdmin::Store_ServiceConstructor( Ce_id               i_nOwner,
                                        const String &      i_sName )
{
    SglIfcService &
                rOwner = ary_cast<SglIfcService>(Storage()[i_nOwner]);
    Function &  ret = *new Function( i_sName,
                                     i_nOwner,
                                     rOwner.NameRoom() );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Constructor(ret.CeId());
    return ret;
}

StructElement &
CeAdmin::Store_StructMember( Ce_id               i_nOwner,
                                     const String &      i_sName,
                                     Type_id             i_nType )
{
    Struct &
        rOwner = ary_cast<Struct>(Storage()[i_nOwner]);
    StructElement & ret = *new StructElement( i_sName,
                                              i_nOwner,
                                              rOwner.NameRoom(),
                                              i_nType );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Member(ret.CeId());
    return ret;
}

StructElement &
CeAdmin::Store_ExceptionMember( Ce_id               i_nOwner,
                                        const String &      i_sName,
                                        Type_id             i_nType )
{
    Exception &
        rOwner = ary_cast<Exception>(Storage()[i_nOwner]);
    StructElement & ret = *new StructElement( i_sName,
                                              i_nOwner,
                                              rOwner.NameRoom(),
                                              i_nType );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Member(ret.CeId());
    return ret;
}

EnumValue &
CeAdmin::Store_EnumValue( Ce_id               i_nOwner,
                                  const String &      i_sName,
                                  const String &      i_sValue )
{
    Enum &
        rOwner = ary_cast<Enum>(Storage()[i_nOwner]);
    EnumValue & ret = *new EnumValue( i_sName,
                                      i_nOwner,
                                      rOwner.NameRoom(),
                                      i_sValue );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Value(ret.CeId());
    return ret;
}

Attribute &
CeAdmin::Store_Attribute( Ce_id               i_nOwner,
                                  const String &      i_sName,
                                  Type_id             i_nType,
                                  bool                i_bReadOnly,
                                  bool                i_bBound )
{
    Interface &
        rOwner = ary_cast<Interface>(Storage()[i_nOwner]);

    Attribute & ret = *new Attribute ( i_sName,
                                       i_nOwner,
                                       rOwner.NameRoom(),
                                       i_nType,
                                       i_bReadOnly,
                                       i_bBound );
    lhf_Put2Storage_and_AssignId(ret);
    rOwner.Add_Attribute(ret.CeId());
    return ret;
}

const Module &
CeAdmin::GlobalNamespace() const
{
    csv_assert(pGlobalNamespace);
    return *pGlobalNamespace;
}

const CodeEntity &
CeAdmin::Find_Ce( Ce_id i_nId ) const
{
    return Storage()[i_nId];

}

const Module &
CeAdmin::Find_Module( Ce_id i_nId ) const
{
    return ary_cast<Module>(Storage()[i_nId]);
}

const Module *
CeAdmin::Search_Module( Ce_id i_nId ) const
{
    if (NOT i_nId.IsValid())
        return 0;
    return ary_cast<Module>( & Storage()[i_nId] );
}

const Function &
CeAdmin::Find_Function( Ce_id i_nId ) const
{
    return ary_cast<Function>(Storage()[i_nId]);
}

const Property &
CeAdmin::Find_Property( Ce_id i_nId ) const
{
    return ary_cast<Property>(Storage()[i_nId]);
}

const EnumValue &
CeAdmin::Find_EnumValue( Ce_id i_nId ) const
{
    return ary_cast<EnumValue>(Storage()[i_nId]);
}

const Constant &
CeAdmin::Find_Constant( Ce_id i_nId ) const
{
    return ary_cast<Constant>(Storage()[i_nId]);
}

const StructElement &
CeAdmin::Find_StructElement( Ce_id i_nId ) const
{
    return ary_cast<StructElement>(Storage()[i_nId]);
}

void
CeAdmin::Get_Text( StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const CodeEntity &  i_ce ) const
{
    const CodeEntity * pCe = &i_ce;
    csv::erase_container(o_module);
    o_ce.clear();
    o_member.clear();

    switch ( pCe->SightLevel() )
    {
        // Here are intentionally no breaks!
        case sl_Member:
                        if ( is_type<Function>(*pCe) )
                            o_member = StreamLock(200)()
                                            << pCe->LocalName()
                                            << "()"
                                            << c_str;
                        else
                            o_member = pCe->LocalName();
                        pCe = & Storage()[pCe->Owner()];
        case sl_File:
                        o_ce = pCe->LocalName();
                        pCe = & Storage()[pCe->NameRoom()];
        case sl_Module:
                        get_ModuleText(o_module,*pCe);
                        break;
        default:
                        csv_assert(false);
    }   // end switch
}

const NameLookup &
CeAdmin::NameDictionary() const
{
    return *pNameDictionary;
}


void
CeAdmin::Get_AlphabeticalIndex( std::vector<Ce_id> &            o_rResult,
                                alphabetical_index::E_Letter    i_cLetter ) const
{
    const int C_nLowerUpperDiff = 'a'-'A';

    // Establishing filter:
    UINT8 filter[256];

    UINT8 nLetter = static_cast<UINT8>(i_cLetter);
    memset(filter, 0, 256);
    filter[nLetter] = 1;
    if ( i_cLetter != alphabetical_index::non_alpha )
        filter[nLetter - C_nLowerUpperDiff] = 1;

    // Gather entities which start with i_cLetter:
    o_rResult.reserve(1000);
    idl::Ce_Storage::c_iter
        itEnd = Storage().End();
    for ( idl::Ce_Storage::c_iter it = Storage().BeginUnreserved();
          it != itEnd;
          ++it )
    {
        if ( filter[ static_cast<UINT8>(*(*it).LocalName().c_str()) ] == 1 )
            o_rResult.push_back( (*it).CeId() );
    }

    std::sort( o_rResult.begin(),
               o_rResult.end(),
               IdSorter<Ce_Compare>() );
}


Module &
CeAdmin::GlobalNamespace()
{
    csv_assert(pGlobalNamespace);
    return *pGlobalNamespace;
}

CodeEntity &
CeAdmin::Find_Ce( Ce_id i_nId )
{
    return Storage()[i_nId];
}

void
CeAdmin::get_ModuleText( StringVector &      o_module,
                              const CodeEntity &  i_ce ) const
{
    if (i_ce.NameRoom().IsValid())
    {
        const CodeEntity &
            rParent = Storage()[i_ce.NameRoom()];
        get_ModuleText(o_module, rParent);
        o_module.push_back(i_ce.LocalName());
    }
}

Struct &
CeAdmin::lhf_Store_TplStruct( Ce_id               i_nOwner,
                                   const String &      i_sName,
                                   Type_id             i_nBase,
                                   const String &      i_sTemplateParam )
{
    csv_assert(NOT i_sTemplateParam.empty());

    TemplateParamType &
        rTpt = pTypePilot->Store_TemplateParamType(i_sTemplateParam);

    Struct & ret = *new Struct( i_sName,
                                i_nOwner,
                                i_nBase,
                                i_sTemplateParam,
                                rTpt.TypeId() );
    lhf_Store_NewEntity(ret, i_nOwner);
    rTpt.Set_StructId(ret.CeId());

    return ret;
}


}   // namespace idl
}   // namespace ary
