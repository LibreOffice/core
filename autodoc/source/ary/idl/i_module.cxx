/*************************************************************************
 *
 *  $RCSfile: i_module.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:15:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ik_module.hxx>

// NOT FULLY DECLARED SERVICES
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
#include <ary/idl/ihost_ce.hxx>
#include <ary/idl/ip_ce.hxx>
#include <nametreenode.hxx>
#include "i_strconst.hxx"


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


//void
//Module::Get_FullName( StringVector &      o_rText,
//                      Ce_idList *         o_pRelatedCes,
//                      const Gate &        i_rGate ) const
//{
//    if (pImpl->Depth() < 1)
//        return;
//
//    if (o_pRelatedCes == 0)
//    {
//        for ( StringVector::const_iterator it = pImpl->NameChain_Begin();
//              it != pImpl->NameChain_End();
//              ++it )
//        {
//            o_rText.push_back(strconst::NamespaceSeparator());
//            o_rText.push_back(*it);
//        }
//    }
//    else
//    {
//        if (pImpl->Depth() > 1)
//        {
//            i_rGate.Ces().Find_Module(pImpl->Parent())
//                .Get_FullName(  o_rText,
//                                o_pRelatedCes,
//                                i_rGate );
//        }
//
//        o_rText.push_back(strconst::NamespaceSeparator());
//        o_rText.push_back(pImpl->Name());
//        o_pRelatedCes->push_back(CeId());
//    }
//}

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

intt
Module::Depth() const
{
    return pImpl->Depth();
}

void
Module::do_Visit_CeHost( CeHost & o_rHost ) const
{
    Dyn_StdConstIterator<Ce_id>
        pLocalNames;
    pImpl->Get_Names(pLocalNames);
    o_rHost.Do_Module(*this);
}

RCid
Module::inq_ClassId() const
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
    csv_assert( i_ce.ClassId() == Module::class_id );
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
        switch (pCe->ClassId())
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
