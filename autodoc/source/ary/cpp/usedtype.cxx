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
#include <ary/cpp/usedtype.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/symtreenode.hxx>
#include <ary/cpp/c_ce.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_slntry.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_traits.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <ary/cpp/cp_type.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <ary/getncast.hxx>
#include "tplparam.hxx"



namespace
{

using namespace ::ary::cpp;
typedef std::vector< ary::cpp::E_ConVol >	PtrLevelVector;


inline bool
result2bool( intt i_nResult )
    { return i_nResult < 0; }


intt 				compare_PtrLevelVector(
                        const PtrLevelVector &
                                            i_r1,
                        const PtrLevelVector &
                                            i_r2 );
inline intt
compare_ConVol(	E_ConVol i_e1,
                E_ConVol i_e2 )
    { return intt(i_e1) - intt(i_e2); }

inline intt
compare_bool(	bool i_b1,
                bool i_b2 )
    { return i_b1 == i_b2
                    ?	0
                    :	i_b1
                            ?	-1
                            :	+1; }
inline intt
compare_Specialisation(	E_TypeSpecialisation i_e1,
                        E_TypeSpecialisation i_e2 )
    { return intt(i_e1) - intt(i_e2); }

inline bool
is_const( E_ConVol i_eCV )
    { return ( intt(i_eCV) & intt(CONVOL_const) ) != 0; }

inline bool
is_volatile( E_ConVol i_eCV )
    { return ( intt(i_eCV) & intt(CONVOL_volatile) ) != 0; }


intt
compare_PtrLevelVector( const PtrLevelVector & i_r1,
                        const PtrLevelVector & i_r2 )
{
    intt nResult = i_r1.size() - i_r2.size();
    if ( nResult != 0 )
        return nResult;

    PtrLevelVector::const_iterator it1 = i_r1.begin();
    PtrLevelVector::const_iterator it1End = i_r1.end();
    PtrLevelVector::const_iterator it2 = i_r2.begin();

    for ( ; it1 != it1End; ++it1, ++it2 )
    {
        nResult = compare_ConVol(*it1, *it2);
        if ( nResult != 0 )
            return nResult;
    }

    return 0;
}


}	// anonymous namespace




namespace ary
{
namespace cpp
{

typedef symtree::Node<CeNode_Traits>    CeNode;
typedef ut::NameChain::const_iterator   nc_iter;

Ce_id               CheckForRelatedCe_inNode(
                        const CeNode &      i_node,
                        const StringVector& i_qualification,
                        const String &      i_name );


UsedType::UsedType(Ce_id i_scope )
    :   aPath(),
        aPtrLevels(),
        eConVol_Type(CONVOL_none),
        bIsReference(false),
        bIsAbsolute(false),
        bRefers2BuiltInType(false),
        eTypeSpecialisation(TYSP_none),
        nRelatedCe(0),
        nScope(i_scope)
{
}

UsedType::~UsedType()
{
}


bool
UsedType::operator<( const UsedType & i_rType ) const
{
    intt nResult = compare_bool( bIsAbsolute, i_rType.bIsAbsolute );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = static_cast<intt>(nScope.Value())
                -
              static_cast<intt>(i_rType.nScope.Value());
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = aPath.Compare( i_rType.aPath );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_ConVol( eConVol_Type, i_rType.eConVol_Type );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_PtrLevelVector( aPtrLevels, i_rType.aPtrLevels );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_bool( bIsReference, i_rType.bIsReference );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_Specialisation( eTypeSpecialisation, i_rType.eTypeSpecialisation );
    if ( nResult != 0 )
        return result2bool(nResult);

    return false;
}

void
UsedType::Set_Absolute()
{
    bIsAbsolute = true;
}

void
UsedType::Add_NameSegment( const char * i_sSeg )
{
    aPath.Add_Segment(i_sSeg);
}

ut::List_TplParameter &
UsedType::Enter_Template()
{
    return aPath.Templatize_LastSegment();
}

void
UsedType::Set_Unsigned()
{
    eTypeSpecialisation = TYSP_unsigned;
}

void
UsedType::Set_Signed()
{
    eTypeSpecialisation = TYSP_signed;
}

void
UsedType::Set_BuiltIn( const char * i_sType )
{
    aPath.Add_Segment(i_sType);
    bRefers2BuiltInType = true;
}

void
UsedType::Set_Const()
{
    if (PtrLevel() == 0)
        eConVol_Type = E_ConVol(eConVol_Type | CONVOL_const);
    else
        aPtrLevels.back() = E_ConVol(aPtrLevels.back() | CONVOL_const);
}

void
UsedType::Set_Volatile()
{
    if (PtrLevel() == 0)
        eConVol_Type = E_ConVol(eConVol_Type | CONVOL_volatile);
    else
        aPtrLevels.back() = E_ConVol(aPtrLevels.back() | CONVOL_volatile);
}

void
UsedType::Add_PtrLevel()
{
    aPtrLevels.push_back(CONVOL_none);
}

void
UsedType::Set_Reference()
{
    bIsReference = true;
}

inline bool
IsInternal(const ary::cpp::CodeEntity & i_ce)
{
    const ary::doc::OldCppDocu *
        docu = dynamic_cast< const ary::doc::OldCppDocu* >(i_ce.Docu().Data());
    if (docu != 0)
        return docu->IsInternal();
    return false;
}


void
UsedType::Connect2Ce( const CePilot & i_ces)
{
    StringVector
        qualification;
    String
        name;
    Get_NameParts(qualification, name);

    for ( const CeNode * scope_node = CeNode_Traits::NodeOf_(
                                            i_ces.Find_Ce(nScope));
          scope_node != 0;
          scope_node = scope_node->Parent() )
    {
        nRelatedCe = CheckForRelatedCe_inNode(*scope_node, qualification, name);
        if ( nRelatedCe.IsValid() )
        {
            if (  IsInternal(i_ces.Find_Ce(nRelatedCe)) )
                nRelatedCe = Ce_id(0);
            return;
        }
    }   // end for
}

void
UsedType::Connect2CeOnlyKnownViaBaseClass(const Gate & i_gate)
{
    csv_assert(nScope.IsValid());
    CesResultList
        instances = i_gate.Ces().Search_TypeName( LocalName() );

    // If there are no matches, or only one match that was already
    //   accepted, all work is done.
    if (     (nRelatedCe.IsValid() AND instances.size() == 1)
         OR  instances.size() == 0 )
        return;

    StringVector
        qualification;
    String
        name;
    Get_NameParts(qualification, name);

    const CodeEntity &
        scopece = i_gate.Ces().Find_Ce(nScope);

    // Else search for declaration in own class and then in base classes.
    //   These would be of higher priority than those in parent namespaces.
    Ce_id
        foundce = RecursiveSearchCe_InBaseClassesOf(
                                    scopece, qualification, name, i_gate);
    if (foundce.IsValid())
        nRelatedCe = foundce;

    if ( nRelatedCe.IsValid() AND IsInternal(i_gate.Ces().Find_Ce(nRelatedCe)) )
    {
        nRelatedCe = Ce_id(0);
    }
}

bool
UsedType::IsBuiltInType() const
{
    return bRefers2BuiltInType
           AND aPtrLevels.size() == 0
           AND NOT bIsReference
           AND eConVol_Type == ary::cpp::CONVOL_none;
}

const String  &
UsedType::LocalName() const
{
    return aPath.LastSegment();
}

E_TypeSpecialisation
UsedType::TypeSpecialisation() const
{
    return eTypeSpecialisation;
}

void
UsedType::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ary::ClassId
UsedType::get_AryClass() const
{
    return class_id;
}

Rid
UsedType::inq_RelatedCe() const
{
     return nRelatedCe.Value();
}

bool
UsedType::inq_IsConst() const
{
    if ( is_const(eConVol_Type) )
        return true;
    for ( PtrLevelVector::const_iterator it = aPtrLevels.begin();
          it != aPtrLevels.end();
          ++it )
    {
        if ( is_const(*it) )
            return true;
    }

    return false;
}

void
UsedType::inq_Get_Text( StreamStr &         o_rPreName,
                        StreamStr &         o_rName,
                        StreamStr &         o_rPostName,
                        const Gate &        i_rGate ) const
{
    if ( is_const(eConVol_Type) )
        o_rPreName << "const ";
    if ( is_volatile(eConVol_Type) )
        o_rPreName << "volatile ";
    if ( bIsAbsolute )
        o_rPreName << "::";

    aPath.Get_Text( o_rPreName, o_rName, o_rPostName, i_rGate );

    for ( PtrLevelVector::const_iterator it = aPtrLevels.begin();
          it != aPtrLevels.end();
          ++it )
    {
        o_rPostName << " *";
        if ( is_const(*it) )
            o_rPostName << " const";
        if ( is_volatile(*it) )
            o_rPostName << " volatile";
    }
    if ( bIsReference )
        o_rPostName << " &";
}

Ce_id
UsedType::RecursiveSearchCe_InBaseClassesOf( const CodeEntity &   i_mayBeClass,
                                             const StringVector & i_myQualification,
                                             const String &       i_myName,
                                             const Gate &         i_gate  ) const
{
    // Find in this class?
    const CeNode *
        basenode = CeNode_Traits::NodeOf_(i_mayBeClass);
    if (basenode == 0)
        return Ce_id(0);
    Ce_id
        found = CheckForRelatedCe_inNode(*basenode, i_myQualification, i_myName);
    if (found.IsValid())
        return found;


    const Class *
        cl = ary_cast<Class>(&i_mayBeClass);
    if (cl == 0)
        return Ce_id(0);

    for ( List_Bases::const_iterator it = cl->BaseClasses().begin();
          it != cl->BaseClasses().end();
          ++it )
    {
        csv_assert((*it).nId.IsValid());
        Ce_id
            base = i_gate.Types().Find_Type((*it).nId).RelatedCe();
        while (base.IsValid() AND is_type<Typedef>(i_gate.Ces().Find_Ce(base)) )
        {
            base = i_gate.Types().Find_Type(
                            ary_cast<Typedef>(i_gate.Ces().Find_Ce(base))
                            .DescribingType() )
                            .RelatedCe();
        }

        if (base.IsValid())
        {
            const CodeEntity &
                basece = i_gate.Ces().Find_Ce(base);
            found = RecursiveSearchCe_InBaseClassesOf(
                        basece, i_myQualification, i_myName, i_gate);
            if (found.IsValid())
                return found;
        }
    }   // end for

    return Ce_id(0);
}


void
UsedType::Get_NameParts(    StringVector &      o_qualification,
                            String &            o_name )
{
    nc_iter nit     = aPath.begin();
    nc_iter nit_end = aPath.end();
    csv_assert(nit != nit_end); // Each UsedType has to have a local name.

    --nit_end;
    o_name = (*nit_end).Name();
    for ( ;
          nit != nit_end;
          ++nit )
    {
        o_qualification.push_back( (*nit).Name() );
    }
}

Ce_id
CheckForRelatedCe_inNode( const CeNode &        i_node,
                          const StringVector &  i_qualification,
                          const String &        i_name )
{
    if (i_qualification.size() > 0)
    {
        Ce_id
            ret(0);
        i_node.SearchBelow( ret,
                            i_qualification.begin(),
                            i_qualification.end(),
                            i_name );
        return ret;
    }
    else
    {
        return i_node.Search(i_name);
    }
}


namespace ut
{

List_TplParameter::List_TplParameter()
  :   aTplParameters()
{
}

List_TplParameter::~List_TplParameter()
{
    csv::erase_container_of_heap_ptrs(aTplParameters);
}

void
List_TplParameter::AddParam_Type( Type_id i_nType )
{
    aTplParameters.push_back( new TplParameter_Type(i_nType) );
}

void
List_TplParameter::Get_Text( StreamStr &                    o_rOut,
                             const ary::cpp::Gate &  i_rGate ) const
{
    Vector_TplArgument::const_iterator it    = aTplParameters.begin();
    Vector_TplArgument::const_iterator itEnd = aTplParameters.end();

    if ( it == itEnd )
    {
        o_rOut << "<>";
        return;
    }

    o_rOut << "< ";

    (*it)->Get_Text( o_rOut, i_rGate );

    for ( ++it; it != itEnd; ++it )
    {
        o_rOut << ", ";
        (*it)->Get_Text( o_rOut, i_rGate );
    }

    o_rOut << " >";
}

intt
List_TplParameter::Compare(	const List_TplParameter & i_rOther ) const
{
    intt nResult = intt(aTplParameters.size()) - intt(i_rOther.aTplParameters.size());

    if (nResult != 0)
        return nResult;

    Vector_TplArgument::const_iterator it1 		= aTplParameters.begin();
    Vector_TplArgument::const_iterator it1End 	= aTplParameters.end();
    Vector_TplArgument::const_iterator it2      = i_rOther.aTplParameters.begin();

    for ( ; it1 != it1End; ++it1, ++it2 )
    {
        nResult = (*it1)->Compare( *(*it2) );
        if (nResult != 0)
            return nResult;
    }

    return 0;
}


}   // namespace ut
}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
