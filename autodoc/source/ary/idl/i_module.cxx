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

#include <precomp.h>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ik_module.hxx>

// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/i_interface.hxx>
#include <ary/idl/i_struct.hxx>
#include <ary/idl/i_exception.hxx>
#include <ary/idl/i_enum.hxx>
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/i_constgroup.hxx>
#include <ary/idl/i_singleton.hxx>
#include <ary/idl/i_siservice.hxx>
#include <ary/idl/i_sisingleton.hxx>
#include <ary/idl/ip_ce.hxx>
#include <nametreenode.hxx>


namespace ary
{
namespace idl
{

Module::Module()
    :   pImpl( new NameTreeNode<Ce_id> )
{
}

Module::Module( const String &      i_sName,
                const Module &      i_rParent )
    :   pImpl( new NameTreeNode<Ce_id>( i_sName,
                                        *i_rParent.pImpl,
                                        i_rParent.CeId() ) )
{
}

Module::~Module()
{
}

void
Module::Add_Name( const String &    i_sName,
                  Ce_id             i_nCodeEntity )
{
    pImpl->Add_Name(i_sName, i_nCodeEntity);
}

Ce_id
Module::Search_Name( const String & i_sName ) const
{
    return pImpl->Search_Name(i_sName);
}

void
Module::Get_Names( Dyn_StdConstIterator<Ce_id> & o_rResult ) const
{
    pImpl->Get_Names( o_rResult );
}

void
Module::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Module::get_AryClass() const
{
    return class_id;
}

const String &
Module::inq_LocalName() const
{
    return pImpl->Name();
}

Ce_id
Module::inq_NameRoom() const
{
    return pImpl->Parent();
}

Ce_id
Module::inq_Owner() const
{
    return pImpl->Parent();
}

E_SightLevel
Module::inq_SightLevel() const
{
    return sl_Module;
}


namespace ifc_module
{

inline const Module &
module_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Module::class_id );
    return static_cast< const Module& >(i_ce);
}

typedef NameTreeNode<Ce_id>::Map_LocalNames NameMap;

void
attr::Get_AllChildrenSeparated( std::vector< const CodeEntity* > & o_nestedModules,
                                std::vector< const CodeEntity* > & o_services,
                                std::vector< const CodeEntity* > & o_interfaces,
                                std::vector< const CodeEntity* > & o_structs,
                                std::vector< const CodeEntity* > & o_exceptions,
                                std::vector< const CodeEntity* > & o_enums,
                                std::vector< const CodeEntity* > & o_typedefs,
                                std::vector< const CodeEntity* > & o_constantGroups,
                                std::vector< const CodeEntity* > & o_singletons,
                                const CePilot &                    i_pilot,
                                const CodeEntity &                 i_ce )
{
    const CodeEntity *
        pCe = 0;
    NameMap::const_iterator
        itEnd = module_cast(i_ce).pImpl->LocalNames().end();
    for ( NameMap::const_iterator
            it = module_cast(i_ce).pImpl->LocalNames().begin();
          it != itEnd;
          ++it )
    {
        pCe = &i_pilot.Find_Ce( (*it).second );
        switch (pCe->AryClass())
        {
            case Module::class_id:
                        o_nestedModules.push_back(pCe);
                        break;
            case SglIfcService::class_id:
            case Service::class_id:
                        o_services.push_back(pCe);
                        break;
            case Interface::class_id:
                        o_interfaces.push_back(pCe);
                        break;
            case Struct::class_id:
                        o_structs.push_back(pCe);
                        break;
            case Exception::class_id:
                        o_exceptions.push_back(pCe);
                        break;
            case Enum::class_id:
                        o_enums.push_back(pCe);
                        break;
            case Typedef::class_id:
                        o_typedefs.push_back(pCe);
                        break;
            case ConstantsGroup::class_id:
                        o_constantGroups.push_back(pCe);
                        break;
            case SglIfcSingleton::class_id:
            case Singleton::class_id:
                        o_singletons.push_back(pCe);
                        break;
        }
    }   // end for
}


} // namespace ifc_module



}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
