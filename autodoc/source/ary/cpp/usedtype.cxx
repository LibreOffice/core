/*************************************************************************
 *
 *  $RCSfile: usedtype.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-15 18:44:44 $
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
#include <ary/cpp/usedtype.hxx>


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/ce.hxx>
#include <ary/cpp/c_etypes.hxx>
#include <ary/info/codeinfo.hxx>
#include <cpp/c_gate.hxx>
#include <instlist.hxx>
#include "namechai.hxx"
#include "tplparam.hxx"



namespace ary
{
namespace cpp
{

typedef std::vector< ary::cpp::E_ConVol >   PtrLevelVector;

struct UsedType::CheshireCat
{
    ut::NameChain       aPath;
    PtrLevelVector      aPtrLevels;
    ary::cpp::E_ConVol  eConVol_Type;
    bool                bIsReference;
    bool                bIsAbsolute;
    bool                bRefers2BuiltInType;
    E_TypeSpecialisation
                        eTypeSpecialisation;
    Tid                 nId;
    Rid                 nRelatedCe;

    // Operations
    uintt               PtrLevel() const        { return uintt(aPtrLevels.size()); }

                        CheshireCat();
};



UsedType::
CheshireCat::CheshireCat()
    :   // aPath,
        // aPtrLevels,
        eConVol_Type(CONVOL_none),
        bIsReference(false),
        bIsAbsolute(false),
        bRefers2BuiltInType(false),
        eTypeSpecialisation(TYSP_none),
        nId(0),
        nRelatedCe(0)
{
}

UsedType::UsedType()
    :   pi(new CheshireCat)
{
}

UsedType::~UsedType()
{
}

namespace
{

inline bool
result2bool( intt i_nResult )
    { return i_nResult < 0; }

intt                compare_PtrLevelVector(
                        const PtrLevelVector &
                                            i_r1,
                        const PtrLevelVector &
                                            i_r2 );
inline intt
compare_ConVol( E_ConVol i_e1,
                E_ConVol i_e2 )
    { return intt(i_e1) - intt(i_e2); }

inline intt
compare_bool(   bool i_b1,
                bool i_b2 )
    { return i_b1 == i_b2
                    ?   0
                    :   NOT i_b1
                            ?   -1
                            :   +1; }
inline intt
compare_Specialisation( E_TypeSpecialisation i_e1,
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

}   // anonymous namespace

bool
UsedType::operator<( const UsedType & i_rType ) const
{
    intt nResult = pi->aPath.Compare( i_rType.pi->aPath );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_ConVol( pi->eConVol_Type, i_rType.pi->eConVol_Type );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_PtrLevelVector( pi->aPtrLevels, i_rType.pi->aPtrLevels );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_bool( pi->bIsReference, i_rType.pi->bIsReference );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_bool( pi->bIsAbsolute, i_rType.pi->bIsAbsolute );
    if ( nResult != 0 )
        return result2bool(nResult);

    nResult = compare_Specialisation( pi->eTypeSpecialisation, i_rType.pi->eTypeSpecialisation );
    if ( nResult != 0 )
        return result2bool(nResult);

    return false;
}

void
UsedType::Set_Id( Tid i_nId )
{
    pi->nId  = i_nId;
}

void
UsedType::Set_Absolute()
{
    pi->bIsAbsolute = true;
}

void
UsedType::Add_NameSegment( const char * i_sSeg )
{
    pi->aPath.Add_Segment(i_sSeg);
}

ut::List_TplParameter &
UsedType::Enter_Template()
{
    return pi->aPath.Templatize_LastSegment();
}

void
UsedType::LeaveTemplate()
{
}

void
UsedType::Set_Unsigned()
{
    pi->eTypeSpecialisation = TYSP_unsigned;
}

void
UsedType::Set_Signed()
{
    pi->eTypeSpecialisation = TYSP_signed;
}

void
UsedType::Set_BuiltIn( const char * i_sType )
{
    pi->aPath.Add_Segment(i_sType);
    pi->bRefers2BuiltInType = true;
}

void
UsedType::Set_Const()
{
    if (pi->PtrLevel() == 0)
        pi->eConVol_Type = E_ConVol(pi->eConVol_Type | CONVOL_const);
    else
        pi->aPtrLevels.back() = E_ConVol(pi->aPtrLevels.back() | CONVOL_const);
}

void
UsedType::Set_Volatile()
{
    if (pi->PtrLevel() == 0)
        pi->eConVol_Type = E_ConVol(pi->eConVol_Type | CONVOL_volatile);
    else
        pi->aPtrLevels.back() = E_ConVol(pi->aPtrLevels.back() | CONVOL_volatile);
}

void
UsedType::Add_PtrLevel()
{
    pi->aPtrLevels.push_back(CONVOL_none);
}

void
UsedType::Set_Reference()
{
    pi->bIsReference = true;
}

Rid
UsedType::Connect2Ce( const Gate & i_rGate )
{
    Rid ret = 0;
    const InstanceList &
            rInstances = i_rGate.Search_TypeName( LocalName() );
    if ( rInstances.empty() )
        return 0;

    uintt nMatchCounter = 0;
    for ( InstanceList::const_iterator it = rInstances.begin();
          it != rInstances.end() AND nMatchCounter < 2;
          ++it )
    {
        if ( DoesMatch_Ce(*it, i_rGate) )
        {
            if ( NOT static_cast< const info::CodeInfo& >( i_rGate.Ref_Ce(*it).Info()).IsInternal() )
            {
                ret = *it;
                nMatchCounter++;
            }
        }
    }  // end for
    if ( nMatchCounter == 1 )
    {
         pi->nRelatedCe = ret;
        return ret;
    }
    if ( nMatchCounter > 1 )
    {
#if 0 // Only for debugging, yet.
         Cerr() << "Warning: Type "
             << LocalName()
             << " found more than one time."
             << Endl();
#endif // 0
         pi->nRelatedCe = ret;
        return ret;
    }

    return 0;
}

bool
UsedType::IsBuiltInType() const
{
    return pi->bRefers2BuiltInType
           AND pi->aPtrLevels.size() == 0
           AND NOT pi->bIsReference
           AND pi->eConVol_Type == ary::cpp::CONVOL_none;
}

const udmstri &
UsedType::LocalName() const
{
    return pi->aPath.LastSegment();
}

E_TypeSpecialisation
UsedType::TypeSpecialisation() const
{
    return pi->eTypeSpecialisation;
}

Rid
UsedType::inq_RelatedCe() const
{
     return pi->nRelatedCe;
}


Tid
UsedType::inq_Id_Type() const
{
    return pi->nId;
}

bool
UsedType::inq_IsConst() const
{
    if ( is_const(pi->eConVol_Type) )
        return true;
    for ( PtrLevelVector::const_iterator it = pi->aPtrLevels.begin();
          it != pi->aPtrLevels.end();
          ++it )
    {
        if ( is_const(*it) )
            return true;
    }

    return false;
}

void
UsedType::inq_Get_Text( StreamStr &          o_rPreName,
                        StreamStr &          o_rName,
                        StreamStr &          o_rPostName,
                        const DisplayGate &  i_rGate ) const
{
    if ( is_const(pi->eConVol_Type) )
        o_rPreName << "const ";
    if ( is_volatile(pi->eConVol_Type) )
        o_rPreName << "volatile ";
    if ( pi->bIsAbsolute )
        o_rPreName << "::";

    pi->aPath.Get_Text( o_rPreName, o_rName, o_rPostName, i_rGate );

    for ( PtrLevelVector::const_iterator it = pi->aPtrLevels.begin();
          it != pi->aPtrLevels.end();
          ++it )
    {
        o_rPostName << " *";
        if ( is_const(*it) )
            o_rPostName << " const";
        if ( is_volatile(*it) )
            o_rPostName << " volatile";
    }
    if ( pi->bIsReference )
        o_rPostName << " &";
}


namespace
{

class NameScope_const_iterator
{
  public:
                        NameScope_const_iterator(
                            Rid                 i_nId,
                            const Gate &        i_rGate );

                        operator bool() const   { return pCe != 0; }
    const udmstri &     operator*() const;

    void                go_up();

  private:
    const CodeEntity *  pCe;
    const Gate *        pGate;
};


NameScope_const_iterator::NameScope_const_iterator( Rid          i_nId,
                                                    const Gate & i_rGate )
    :   pCe(i_rGate.Find_Ce(i_nId)),
        pGate(&i_rGate)
{
}

const udmstri &
NameScope_const_iterator::operator*() const
{
     return pCe ? pCe->LocalName()
               : udmstri::Null_();
}

void
NameScope_const_iterator::go_up()
{
     if (pCe == 0)
        return;
    pCe = pGate->Find_Ce(pCe->Owner());

    if (pCe != 0 ? pCe->Owner() == 0 : false)
        pCe = 0;    // Global namespace provides no scope name.
}



}   // anonymous namespace



bool
UsedType::DoesMatch_Ce( Rid                 i_nId,
                        const Gate &        i_rGate ) const
{
    NameScope_const_iterator
            itScope( i_nId, i_rGate );
    ut::NameChain::const_iterator
            itPath = pi->aPath.end();
    if ( itPath == pi->aPath.begin() )
        return false;
    itPath--;
    if (itPath == pi->aPath.begin())
    {
        if ( NOT pi->bIsAbsolute )
            return true;
        itScope.go_up();
        return NOT itScope;
    }

    for ( itScope.go_up(), --itPath;
          itScope ;
          itScope.go_up(), --itPath )
    {
        if ( (*itPath).Name() != *itScope )
            return false;
        if ( itPath == pi->aPath.begin() )
        {
            if ( NOT pi->bIsAbsolute )
                return true;
            itScope.go_up();
            return NOT itScope;
        }
    }
    return false;
}





namespace ut
{

List_TplParameter::List_TplParameter()
//  :   aTplParameters
{
}

List_TplParameter::~List_TplParameter()
{
    csv::erase_container_of_heap_ptrs(aTplParameters);
}

void
List_TplParameter::AddParam_Type( Tid i_nType )
{
    aTplParameters.push_back( new TplParameter_Type(i_nType) );
}

void
List_TplParameter::AddParam_Constant( const udmstri & i_sConst )
{
    aTplParameters.push_back( new TplParameter_Const(i_sConst) );
}

List_TplParameter::const_iterator
List_TplParameter::Begin() const
{
    return aTplParameters.begin();
}

List_TplParameter::const_iterator
List_TplParameter::End() const
{
    return aTplParameters.end();
}

void
List_TplParameter::Get_Text( StreamStr &                    o_rOut,
                             const ary::cpp::DisplayGate &  i_rGate ) const
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
List_TplParameter::Compare( const List_TplParameter & i_rOther ) const
{
    intt nResult = intt(aTplParameters.size()) - intt(i_rOther.aTplParameters.size());

    if (nResult != 0)
        return nResult;

    Vector_TplArgument::const_iterator it1      = aTplParameters.begin();
    Vector_TplArgument::const_iterator it1End   = aTplParameters.end();
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

