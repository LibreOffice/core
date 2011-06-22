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
#include "i2s_calculator.hxx"

// NOT FULLY DEFINED SERVICES
#include <algorithm>
#include <string.h>
#include <cosv/file.hxx>
#include <ary/qualiname.hxx>
#include <ary/idl/i_enum.hxx>
#include <ary/idl/i_exception.hxx>
#include <ary/idl/i_function.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_interface.hxx>
#include <ary/idl/ik_interface.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_property.hxx>
#include <ary/idl/i_service.hxx>
#include <ary/idl/i_singleton.hxx>
#include <ary/idl/i_siservice.hxx>
#include <ary/idl/i_sisingleton.hxx>
#include <ary/idl/i_struct.hxx>
#include <ary/idl/i_structelem.hxx>
#include <ary/idl/i_typedef.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include <ary/namesort.hxx>
#include <nametreenode.hxx>
#include "i_nnfinder.hxx"
#include "ia_ce.hxx"
#include "ia_type.hxx"
#include "is_ce.hxx"
#include "is_type.hxx"
#include "it_ce.hxx"
#include "it_explicit.hxx"
#include "it_sequence.hxx"
#include "it_xnameroom.hxx"


namespace ary
{
namespace idl
{

template <class DEST>
DEST *
SecondariesCalculator::SearchCe4Type(Type_id i_type)
{
    Ce_id
        ce = lhf_Search_CeFromTypeId(i_type);
    if (ce.IsValid())
        return ary_cast<DEST>(& my_CeStorage()[ce]);
    return 0;
}


typedef stg::const_iterator<CodeEntity> stg_citerator;
typedef stg::iterator<CodeEntity>       stg_iterator;

typedef stg::filter_iterator<CodeEntity,Interface>
                                        interface_iterator;

typedef stg::filter_iterator<Type,ExplicitType>
                                        explicittype_iterator;

typedef ary::stg::const_filter_iterator<CodeEntity,Typedef>
                                        typedef_citerator;


inline Service *
SecondariesCalculator::lhf_SearchService( Type_id i_nType )
{
    return SearchCe4Type<Service>(i_nType);
}

inline Interface *
SecondariesCalculator::lhf_SearchInterface( Type_id i_nType )
{
    return SearchCe4Type<Interface>(i_nType);
}

inline Struct *
SecondariesCalculator::lhf_SearchStruct( Type_id i_nType )
{
    return SearchCe4Type<Struct>(i_nType);
}

inline Exception *
SecondariesCalculator::lhf_SearchException( Type_id i_nType )
{
    return SearchCe4Type<Exception>(i_nType);
}

inline const Ce_Storage &
SecondariesCalculator::my_CeStorage() const
{
    csv_assert(pCes != 0);
    return pCes->Storage();
}

inline const Type_Storage &
SecondariesCalculator::my_TypeStorage() const
{
    csv_assert(pTypes != 0);
    return pTypes->Storage();
}

inline Ce_Storage &
SecondariesCalculator::my_CeStorage()
{
    csv_assert(pCes != 0);
    return pCes->Storage();
}

inline Type_Storage &
SecondariesCalculator::my_TypeStorage()
{
    csv_assert(pTypes != 0);
    return pTypes->Storage();
}

inline void
SecondariesCalculator::insert_into2sList( CodeEntity &        o_out,
                                          int                 i_listIndex,
                                          Ce_id               i_nCe )
    { o_out.Secondaries().Access_List(i_listIndex).push_back(i_nCe); }


SecondariesCalculator::SecondariesCalculator( CeAdmin &      i_ces,
                                              TypeAdmin &    i_types )
    :   pCes(&i_ces),
        pTypes(&i_types)
{
}

SecondariesCalculator::~SecondariesCalculator()
{
}


void
SecondariesCalculator::CheckAllInterfaceBases()
{
    Module &
        rGlobalNamespace = pCes->GlobalNamespace();
    QualifiedName
        aXInterface("::com::sun::star::uno::XInterface","::");

    const Type &
        rType = pTypes->CheckIn_Type(   aXInterface,
                                        0,
                                        rGlobalNamespace.CeId(),
                                        0 );
    Type_id
        nTypeXInterface = rType.TypeId();
    const ExplicitType &
        rExplType = ary_cast<ExplicitType>(rType);
    Ce_id
        nCeXInterface = lhf_Search_CeForType(rExplType);

    interface_iterator itEnd( my_CeStorage().End() );
    for ( interface_iterator it( my_CeStorage().BeginUnreserved() );
          it != itEnd;
          ++it )
    {
        if (NOT it.IsValid())
            continue;

        Interface &
            rInterface = *it;
        if (     NOT rInterface.HasBase()               // According to UNO IDL syntax, an interface without base has com::sun::star::uno::XInterface as base.
             AND rInterface.CeId() != nCeXInterface )   // XInterface must not be base of itself.
        {
            rInterface.Add_Base(nTypeXInterface, 0);
        }
    }   // end for
}

void
SecondariesCalculator::Connect_Types2Ces()
{
    explicittype_iterator itEnd( my_TypeStorage().End() );
    for ( explicittype_iterator it( my_TypeStorage().BeginUnreserved() );
          it != itEnd;
          ++it )
    {
        if (NOT it.IsValid())
            continue;

        ExplicitType &
            rType = ary_cast<ExplicitType>(*it);
        Ce_id
            nRelatedCe = lhf_Search_CeForType(rType);
        if (nRelatedCe.IsValid())
        {
            Ce_Type *
                pNew = new Ce_Type(nRelatedCe, rType.TemplateParameters());
            my_TypeStorage().Replace_Entity( rType.TypeId(),
                                             *pNew );
        }
    }   // end for
}

void
SecondariesCalculator::Gather_CrossReferences()
{
    gather_Synonyms();

    for ( stg_iterator it = my_CeStorage().Begin();
          it != my_CeStorage().End();
          ++it )
    {
        (*it).Accept( static_cast< SPInst_asHost& >(*this) );

    }   // end for

    sort_All2s();
}

void
SecondariesCalculator::Make_Links2DeveloperManual(
                                const String &      i_devman_reffilepath )
{
    csv::File
        aFile(i_devman_reffilepath, csv::CFM_READ);
    csv::OpenCloseGuard
        aFileOpener(aFile);
    if (aFileOpener)
    {
        Read_Links2DevManual(aFile);
    }
}

namespace
{

enum E_LinkMode
{
    link2descr,
    link2ref
};

struct OrderCeIdsByName
{
                        OrderCeIdsByName(
                            const Ce_Storage &      i_storage )
                                :   rStorage(i_storage),
                                    aNameComparison() {}
    bool                operator()(
                            Ce_id                   i_ce1,
                            Ce_id                   i_ce2 ) const
    {
        return aNameComparison( rStorage[i_ce1].LocalName(),
                                rStorage[i_ce2].LocalName() );
    }

  private:
    const Ce_Storage &  rStorage;
    LesserName          aNameComparison;
};


}



void
SecondariesCalculator::do_Process( const Service & i_rData )
{
    const Service &
        rService = ary_cast<Service>(i_rData);

    // Interfaces:
    assignImplementation_toAServicesInterfaces( rService.CeId(),
                                                rService.CeId(),
                                                interface_2s_ExportingServices );
    // Services and their interfaces:
    recursive_AssignIncludingService(rService.CeId(), rService);
}

void
SecondariesCalculator::do_Process( const Interface & i_rData )
{
    assign_AsDerivedInterface( ary_cast<Interface>(i_rData) );
}

void
SecondariesCalculator::do_Process( const Struct &  i_rData )
{
    assign_AsDerivedStruct( ary_cast<Struct>(i_rData) );
}

void
SecondariesCalculator::do_Process( const Exception &  i_rData )
{
    assign_AsDerivedException( ary_cast<Exception>(i_rData) );
}

void
SecondariesCalculator::do_Process( const Typedef &  )
{
    // KORR_FUTURE
    // Find out what was meant here ???

//    const Typedef &
//        rTypedef = ary_cast<Typedef>(i_rData);
}

void
SecondariesCalculator::do_Process( const Singleton &  i_rData )
{
    const Singleton &
        rSingleton = ary_cast<Singleton>(i_rData);

    Service *
        pServ = lhf_SearchService(rSingleton.AssociatedService());
    if (pServ != 0)
    {
        insert_into2sUnique( *pServ,
                             service_2s_InstantiatingSingletons,
                             rSingleton.CeId() );
    }

    // Interfaces:
    assignImplementation_toAServicesInterfaces( rSingleton.CeId(),
                                                lhf_Search_CeFromTypeId(rSingleton.AssociatedService()),
                                                interface_2s_ExportingSingletons );
}

void
SecondariesCalculator::do_Process( const SglIfcService &  i_rData )
{
    const SglIfcService &
        rSglIfcService = ary_cast<SglIfcService>(i_rData);

    assignImplementation_toAServicesInterfaces( rSglIfcService.CeId(),
                                                rSglIfcService.CeId(),
                                                interface_2s_ExportingServices );
}

void
SecondariesCalculator::do_Process( const SglIfcSingleton &  i_rData )
{
    const SglIfcSingleton &
        rSglIfcSingleton = ary_cast<SglIfcSingleton>(i_rData);

    Type_id nBase = rSglIfcSingleton.BaseInterface();
    recursive_AssignImplementation_toExportedInterface( rSglIfcSingleton.CeId(),
                                                        nBase,
                                                        interface_2s_ExportingSingletons );
}

void
SecondariesCalculator::do_Process( const Function & i_rData )
{
    const Function &
        rFunction = ary_cast<Function>(i_rData);

    recursive_AssignFunction_toCeAsReturn(rFunction.CeId(), rFunction.ReturnType());

    for ( Function::ParamList::const_iterator itp = rFunction.Parameters().begin();
          itp != rFunction.Parameters().end();
          ++itp )
    {
        recursive_AssignFunction_toCeAsParameter(rFunction.CeId(), (*itp).Type());
    }   // end for (itp)

    for ( Function::ExceptionList::const_iterator itx = rFunction.Exceptions().begin();
          itx != rFunction.Exceptions().end();
          ++itx )
    {
        Exception *
            pX = lhf_SearchException(*itx);
        if (pX != 0)
        {
            insert_into2sUnique(*pX, exception_2s_RaisingFunctions, rFunction.CeId());
        }
    }   // end for (itx)
}

void
SecondariesCalculator::do_Process( const StructElement & i_rData )
{
    const StructElement &
        rStructElement = ary_cast<StructElement>(i_rData);

    recursive_AssignStructElement_toCeAsDataType(rStructElement.CeId(), rStructElement.Type());
}

void
SecondariesCalculator::do_Process( const Property & i_rData )
{
    const Property &
        rProperty = ary_cast<Property>(i_rData);

    recursive_AssignStructElement_toCeAsDataType(rProperty.CeId(), rProperty.Type());
}

Ce_id
SecondariesCalculator::lhf_Search_CeForType( const ExplicitType & i_rType ) const
{
    const ExplicitNameRoom &
        rExplicitNameRoom = ary_cast<ExplicitNameRoom>(
                                my_TypeStorage()[i_rType.NameRoom()] );
    Find_ModuleNode
        rNodeFinder( my_CeStorage(),
                     rExplicitNameRoom.NameChain_Begin(),
                     rExplicitNameRoom.NameChain_End(),
                     i_rType.Name() );

    if ( rExplicitNameRoom.IsAbsolute() )
    {
        const Module &
            rGlobalNamespace = ary_cast<Module>(
                                my_CeStorage()[predefined::ce_GlobalNamespace]);
        return Search_SubTree(  rGlobalNamespace,
                                rNodeFinder );
    }
    else
    {
        const Module &
            rStartModule = ary_cast<Module>(
                                my_CeStorage()[i_rType.ModuleOfOccurrence()]);
        Ce_id ret = Search_SubTree_UpTillRoot( rStartModule,
                                               rNodeFinder );
        return ret;
    }   // endif (rExplicitNameRoom.IsAbsolute()) else
}

Ce_id
SecondariesCalculator::lhf_Search_CeFromTypeId( Type_id i_nType ) const
{
    if (NOT i_nType.IsValid())
        return Ce_id(0);
    const Ce_Type *
        pType = ary_cast<Ce_Type>( & my_TypeStorage()[i_nType] );
    return pType != 0
                ?   pType->RelatedCe()
                :   Ce_id_Null();
}

void
SecondariesCalculator::assign_CurLink( char *              i_text,
                                       const String &      i_link,
                                       const String &      i_linkUI,
                                       bool                i_isDescr,
                                       int                 i_lineCount )
{
    csv_assert(i_text != 0);

    const ary::idl::Module *
        pModule = & ary_cast<Module>(
                        my_CeStorage()[predefined::ce_GlobalNamespace]);

    char * pPastNext = 0;
    char * pNext = i_text;
    for ( ;
          (pPastNext = strstr(pNext,".")) != 0;
          pNext = pPastNext + 1 )
    {
        String sNext(pNext, pPastNext-pNext);
        Ce_id nModule = pModule->Search_Name(sNext);
        if (nModule.IsValid())
        {
            pModule = ary_cast<Module>( & my_CeStorage()[nModule] );
        }
        else
        {
            pModule = 0;
        }

        if (pModule == 0)
        {
            Cerr() << "Warning: Invalid line nr. "
                   << i_lineCount
                   << " in DevelopersGuide reference file:\n"
                   << reinterpret_cast< const char* >(i_text)
                   << "\n"
                   << Endl();
            return;
        }
    }   // end for

    pPastNext = strchr(pNext,':');
    bool bMember = pPastNext != 0;
    String sCe( pNext, (bMember ? csv::str::size(pPastNext-pNext) : csv::str::maxsize) );

//  KORR_FUTURE
//  String sMember(bMember ? pPastNext+1, "");

    Ce_id nCe = pModule->Search_Name(sCe);
    if (NOT nCe.IsValid())
    {
        Cerr() << "Warning: Invalid line nr. "
               << i_lineCount
               << " in DevelopersGuide reference file:\n"
               << reinterpret_cast< const char* >(i_text)
               << "\n"
               << Endl();
        return;
    }

    CodeEntity &
        rCe = my_CeStorage()[nCe];
    if (NOT bMember)
    {
        if (i_isDescr)
            rCe.Secondaries().Add_Link2DescriptionInManual(i_link, i_linkUI);
        else
            rCe.Secondaries().Add_Link2RefInManual(i_link, i_linkUI);
        return;
    }
    else
    {
    // KORR_FUTURE
    //   Provisorial just doing nothing (or may be
    //   adding a link at main Ces lists).
//    if (i_isDescr)
//        rCe.Secondaries().Add_Link2DescriptionInManual(i_link);
//    else
//        rCe.Secondaries().Add_Link2RefInManual(i_link);
    }
}

void
SecondariesCalculator::gather_Synonyms()
{
    const Ce_Storage &
        cstrg = my_CeStorage();
    typedef_citerator itEnd(cstrg.End());
    for ( typedef_citerator it(cstrg.Begin());
          it != itEnd;
          ++it )
    {
        if (NOT it.IsValid())
            continue;

        const Typedef &
            rTypedef = *it;
        recursive_AssignAsSynonym(rTypedef.CeId(), rTypedef);
    }   // end for (itTd)
}

void
SecondariesCalculator::recursive_AssignAsSynonym( Ce_id               i_synonymousTypedefsId,
                                                  const Typedef &     i_TypedefToCheck )
{
    Ce_id
        nCe = lhf_Search_CeFromTypeId(i_TypedefToCheck.DefiningType());
    if (NOT nCe.IsValid())
        return;
    CodeEntity &
        rCe = my_CeStorage()[nCe];

    switch (rCe.AryClass())  // KORR_FUTURE: make this faster, remove switch.
    {
        case Interface::class_id:
                    insert_into2sList( rCe,
                                       interface_2s_SynonymTypedefs,
                                       i_synonymousTypedefsId );
                    break;
        case Struct::class_id:
                    insert_into2sList( rCe,
                                       struct_2s_SynonymTypedefs,
                                       i_synonymousTypedefsId );
                    break;
        case Enum::class_id:
                    insert_into2sList( rCe,
                                       enum_2s_SynonymTypedefs,
                                       i_synonymousTypedefsId );
                    break;
        case Typedef::class_id:
                    insert_into2sList( rCe,
                                       typedef_2s_SynonymTypedefs,
                                       i_synonymousTypedefsId );
                    recursive_AssignAsSynonym( i_synonymousTypedefsId,
                                               static_cast< Typedef& >(rCe) );
                    break;
                // default: do nothing.
    }
}

void
SecondariesCalculator::recursive_AssignIncludingService( Ce_id               i_includingServicesId,
                                                         const Service &     i_ServiceToCheckItsIncludes )
{
    Dyn_StdConstIterator<CommentedRelation>
        pIncludedServices;
    i_ServiceToCheckItsIncludes.Get_IncludedServices(pIncludedServices);

    for ( StdConstIterator<CommentedRelation> &
                itServ = *pIncludedServices;
          itServ;
          ++itServ )
    {
        Service *
            pServ = lhf_SearchService((*itServ).Type());
        if (pServ != 0)
        {
            insert_into2sUnique( *pServ,
                                 service_2s_IncludingServices,
                                 i_includingServicesId
                               );
            recursive_AssignIncludingService(i_includingServicesId, *pServ);

        }   // end if

        assignImplementation_toAServicesInterfaces( i_includingServicesId,
                                                    lhf_Search_CeFromTypeId( (*itServ).Type() ),
                                                    interface_2s_ExportingServices );
    }   // end for
}

void
SecondariesCalculator::assign_AsDerivedInterface( const Interface & i_rDerived )
{
    ary::Dyn_StdConstIterator<ary::idl::CommentedRelation>
        pHelp;
    ary::idl::ifc_interface::attr::Get_Bases(pHelp, i_rDerived);

    for ( ary::StdConstIterator<ary::idl::CommentedRelation> & it = *pHelp;
          it.operator bool();
          ++it )
    {
        Interface *
            pIfc = lhf_SearchInterface( (*it).Type() );
        if (pIfc == 0)
            continue;

        insert_into2sList( *pIfc,
                           interface_2s_Derivations,
                           i_rDerived.CeId() );
    }   // end for
}

void
SecondariesCalculator::assign_AsDerivedStruct( const Struct &   i_rDerived )
{
    Type_id
        nBase = i_rDerived.Base();
    if (nBase.IsValid())
    {
        Struct *
            pParent = lhf_SearchStruct(nBase);
        if (pParent != 0)
        {
            insert_into2sList( *pParent,
                               struct_2s_Derivations,
                               i_rDerived.CeId() );
        }
    }
}

void
SecondariesCalculator::assign_AsDerivedException( const Exception &   i_rDerived )
{
    Type_id
        nBase = i_rDerived.Base();
    if (nBase.IsValid())
    {
        Exception *
            pParent = lhf_SearchException(nBase);
        if (pParent != 0)
        {
            insert_into2sList( *pParent,
                               exception_2s_Derivations,
                               i_rDerived.CeId() );
        }   // end if
    }   // end if
}

void
SecondariesCalculator::assignImplementation_toAServicesInterfaces(
                                                    Ce_id               i_nImpl,
                                                    Ce_id               i_nService,
                                                    E_2s_of_Interface   i_eList )
{
    if (NOT i_nService.IsValid())
        return;
    Service *
        pService = ary_cast<Service>( & my_CeStorage()[i_nService] );
    SglIfcService *
        pSglIfcService = ary_cast<SglIfcService>( & my_CeStorage()[i_nService] );

    if (pService != 0)
    {
        Dyn_StdConstIterator<CommentedRelation>
            pSupportedInterfaces;
        pService->Get_SupportedInterfaces(pSupportedInterfaces);

        for ( StdConstIterator<CommentedRelation> &
                    itInfc = *pSupportedInterfaces;
              itInfc.operator bool();
              ++itInfc )
        {
            recursive_AssignImplementation_toExportedInterface( i_nImpl,
                                                                (*itInfc).Type(),
                                                                i_eList );
        }   // end for
    }
    else if (pSglIfcService != 0)
    {
        Type_id nBase = pSglIfcService->BaseInterface();
        recursive_AssignImplementation_toExportedInterface( i_nImpl,
                                                            nBase,
                                                            i_eList );
    }   // end if
}

void
SecondariesCalculator::recursive_AssignImplementation_toExportedInterface(
                                                    Ce_id               i_nService,
                                                    Type_id             i_nExportedInterface,
                                                    E_2s_of_Interface   i_eList )
{
    Interface *
        pIfc = lhf_SearchInterface(i_nExportedInterface);
    if (pIfc == 0)
        return;

    insert_into2sUnique( *pIfc,
                         i_eList,
                         i_nService );
    Dyn_StdConstIterator<CommentedRelation>
        pBases;
    ary::idl::ifc_interface::attr::Get_Bases(pBases, *pIfc);
    for ( StdConstIterator<CommentedRelation> & it = *pBases;
          it.operator bool();
          ++it )
    {
        recursive_AssignImplementation_toExportedInterface(i_nService, (*it).Type(), i_eList);
    }
}

void
SecondariesCalculator::recursive_AssignFunction_toCeAsReturn( Ce_id         i_nFunction,
                                                              Type_id       i_nReturnType )
{
    Ce_id
        nCe = lhf_Search_CeFromTypeId(i_nReturnType);
    if (NOT nCe.IsValid())
        return;

    CodeEntity &
        rCe = my_CeStorage()[nCe];
    switch (rCe.AryClass())  // KORR_FUTURE: make this faster, remove switch.
    {
        case Interface::class_id:
                    insert_into2sList( rCe,
                                       interface_2s_AsReturns,
                                       i_nFunction );
                    break;
        case Struct::class_id:
                    insert_into2sList( rCe,
                                       struct_2s_AsReturns,
                                       i_nFunction );
                    break;
        case Enum::class_id:
                    insert_into2sList( rCe,
                                       enum_2s_AsReturns,
                                       i_nFunction );
                    break;
        case Typedef::class_id:
                    insert_into2sList( rCe,
                                       typedef_2s_AsReturns,
                                       i_nFunction );
                    recursive_AssignFunction_toCeAsReturn( i_nFunction,
                                                           static_cast< Typedef& >(rCe).DefiningType() );
                    break;
        // default: do nothing.
    }
}

void
SecondariesCalculator::recursive_AssignFunction_toCeAsParameter( Ce_id      i_nFunction,
                                                                 Type_id    i_nParameterType )
{
    Ce_id
        nCe = lhf_Search_CeFromTypeId(i_nParameterType);
    if (NOT nCe.IsValid())
        return;

    CodeEntity &
        rCe = my_CeStorage()[nCe];
    switch (rCe.AryClass())  // KORR_FUTURE: make this faster, remove switch.
    {
        case Interface::class_id:
                    insert_into2sList( rCe,
                                       interface_2s_AsParameters,
                                       i_nFunction );
                    break;
        case Struct::class_id:
                    insert_into2sList( rCe,
                                       struct_2s_AsParameters,
                                       i_nFunction );
                    break;
        case Enum::class_id:
                    insert_into2sList( rCe,
                                       enum_2s_AsParameters,
                                       i_nFunction );
                    break;
        case Typedef::class_id:
                    insert_into2sList( rCe,
                                       typedef_2s_AsParameters,
                                       i_nFunction );
                    recursive_AssignFunction_toCeAsParameter( i_nFunction,
                                                              static_cast< Typedef& >(rCe).DefiningType() );
                    break;
        // default: do nothing.
    }
}

void
SecondariesCalculator::recursive_AssignStructElement_toCeAsDataType( Ce_id   i_nDataElement,
                                                                     Type_id i_nDataType )
{
    Ce_id
        nCe = lhf_Search_CeFromTypeId(i_nDataType);
    if (NOT nCe.IsValid())
        return;

    CodeEntity &
        rCe = my_CeStorage()[nCe];
    switch (rCe.AryClass())  // KORR_FUTURE: make this faster, remove switch.
    {
        case Interface::class_id:
                    insert_into2sList( rCe,
                                       interface_2s_AsDataTypes,
                                       i_nDataElement );
                    break;
        case Struct::class_id:
                    insert_into2sList( rCe,
                                       struct_2s_AsDataTypes,
                                       i_nDataElement );
                    break;
        case Enum::class_id:
                    insert_into2sList( rCe,
                                       enum_2s_AsDataTypes,
                                       i_nDataElement );
                    break;
        case Typedef::class_id:
                    insert_into2sList( rCe,
                                       typedef_2s_AsDataTypes,
                                       i_nDataElement );
                    recursive_AssignFunction_toCeAsParameter( i_nDataElement,
                                                              static_cast< Typedef& >(rCe).DefiningType() );
                    break;
        // default: do nothing.
    }   // end switch
}

void
SecondariesCalculator::insert_into2sUnique( CodeEntity &        o_out,
                                            int                 i_listIndex,
                                            Ce_id               i_nCe )
{
    std::vector<Ce_id> &
        rOut = o_out.Secondaries().Access_List(i_listIndex);
    if (std::find(rOut.begin(),rOut.end(),i_nCe) != rOut.end())
        return;
    rOut.push_back(i_nCe);
}

void
SecondariesCalculator::sort_All2s()
{
    OrderCeIdsByName
        aIdOrdering(my_CeStorage());

    for ( stg_iterator it = my_CeStorage().Begin();
          it != my_CeStorage().End();
          ++it )
    {
        Ce_2s &
            r2s = (*it).Secondaries();
        int iCount = r2s.CountXrefLists();
        for (int i = 0; i < iCount; ++i)
        {
            std::sort( r2s.Access_List(i).begin(),
                       r2s.Access_List(i).end(),
                       aIdOrdering );
        }   // end for (i)
    }   // end for (it)
}

void
SecondariesCalculator::Read_Links2DevManual( csv::bstream & i_file )
{
    StreamLock  aLine(300);
    StreamStr & rLine = aLine();


    String      sCurLink;
    String      sCurLinkUI;
    E_LinkMode  eCurMode = link2ref;

    int lineCount = 0;
    const char * sLink = "LINK:";
    const char * sDescr = "DESCR:";
    const char * sTopic = "TOPIC:";
    const char * sRef = "REF:";
    const UINT8  cMaxASCIINumWhiteSpace = 32;

    while (NOT i_file.eod())
    {
        ++lineCount;

        rLine.reset();
        rLine.operator_read_line(i_file);

        if ( *rLine.c_str() >= 'a' )
        {
            assign_CurLink(rLine.begin(), sCurLink, sCurLinkUI, eCurMode == link2descr, lineCount);
        }
        else if ( strncmp(rLine.c_str(), sLink, strlen(sLink)) == 0 )
        {
            sCurLink = rLine.c_str()+5;
            sCurLinkUI.clear();
        }
        else if ( strncmp(rLine.c_str(), sDescr, strlen(sDescr)) == 0 )
        {
            sCurLinkUI = rLine.c_str()+6;
        }
        else if ( strncmp(rLine.c_str(), sTopic, strlen(sTopic)) == 0 )
        {
            eCurMode = link2descr;
        }
        else if ( strncmp(rLine.c_str(), sRef, strlen(sRef)) == 0 )
        {
            eCurMode = link2ref;
        }
        else if (static_cast<UINT8>(*rLine.c_str()) > cMaxASCIINumWhiteSpace)
        {
            assign_CurLink(rLine.begin(), sCurLink, sCurLinkUI, eCurMode == link2descr, lineCount);
        }
     // else
        //  Ignore empty line.

    }   // end while
}



}   // namespace idl
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
