/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_reposypart.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-05 12:01:03 $
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
#include "c_reposypart.hxx"


// NOT FULLY DECLARED SERVICES
#include <ary/getncast.hxx>
#include <ary/namesort.hxx>
#include <ary/cpp/c_builtintype.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_cppentity.hxx>
#include <ary/cpp/c_define.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_macro.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_type.hxx>
#include <ary/cpp/usedtype.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/loc/locp_le.hxx>
#include <ary/getncast.hxx>
#include <loc_internalgate.hxx>
#include <nametree.hxx>
#include <reposy.hxx>
#include "ca_ce.hxx"
#include "ca_def.hxx"
#include "ca_type.hxx"
#include "cs_ce.hxx"
#include "cs_def.hxx"
#include "cs_type.hxx"



namespace
{

using ::ary::GlobalId;
using ::ary::Rid;
using namespace ::ary::cpp;


inline bool
IsDefine( const GlobalId & i_id )
{
    return i_id.Class() == Define::class_id
           OR
           i_id.Class() == Macro::class_id;
}


/// Find Ces
class TypeConnector
{
  public:
                        TypeConnector(
                            Gate &              i_gate )
                            :   pGate(&i_gate) {}
                        ~TypeConnector() {}

    void                operator()(
                            Type &              io_rType ) const;
  private:
    // DATA
    Gate *              pGate;
};

/// Find Ces only known from base class name scope.
class TypeConnector2ndTry
{
  public:
                        TypeConnector2ndTry(
                            Gate &              i_gate )
                            :   pGate(&i_gate) {}
                        ~TypeConnector2ndTry() {}

    void                operator()(
                            Type &              io_rType ) const;
  private:
    // DATA
    Gate *              pGate;
};

/// Reconnect (in both directions) base-derived relations of classes.
class HierarchyLinker
{
  public:
                        HierarchyLinker(
                            Gate &              i_gate )
                            :   pGate(&i_gate) {}

                        ~HierarchyLinker() {}

    void                operator()(
                            Class &             io_rCe ) const;
  private:
    // DATA
    Gate *              pGate;
};



/// Helper functor for ->RepositoryPartition::Get_AlphabeticalList().
template <class TRAITS>
struct MakeGlobalId
{
    GlobalId            operator()(
                            typename TRAITS::id_type
                                                i_id ) const
                        {
                            return GlobalId( TRAITS::EntityOf_(i_id).AryClass(),
                                             i_id.Value() );
                        }
};




/** Compare two {->GlobalId}s.


    @todo   Move this up to the definition of GlobalId<>.
*/
struct LesserGlobal
{
                        LesserGlobal(
                            const Ce_Storage &  i_ces,
                            const Def_Storage & i_des )
                            :   rCes(i_ces), rDes(i_des) {}

    bool                operator()(
                            GlobalId            i_1,
                            GlobalId            i_2 ) const;

  private:
    const String &      NameOf(
                            GlobalId            i_id ) const;
    // DATA
    const Ce_Storage &  rCes;
    const Def_Storage & rDes;
    ::ary::LesserName   aLess;
};


bool
LesserGlobal::operator()(   GlobalId            i_1,
                            GlobalId            i_2 ) const
                        {
                            String  s1 = NameOf(i_1);
                            String  s2 = NameOf(i_2);

                            if (s1 != s2)
                                return aLess(s1, s2);

                            if ( IsDefine(i_1) != IsDefine(i_2) )
                            {
                                return NOT IsDefine(i_2);
                            }

                            return Ce_GlobalCompare::Lesser_(
                                        rCes[i_1.Id()],
                                        rCes[i_2.Id()] );
                        }


}   // namespace anonymous






namespace ary
{
namespace cpp
{

DYN InternalGate &
InternalGate::Create_Partition_(RepositoryCenter & i_center)
{
    return *new RepositoryPartition(i_center);
}


RepositoryPartition::RepositoryPartition(RepositoryCenter & i_center)
    :   pRepositoryCenter(&i_center),
        pCes(0),
        pTypes(0),
        pDefs(0),
        pLocations(& loc::InternalGate::Create_Locations_())
{
    pCes        = new CeAdmin(*this);
    pTypes      = new TypeAdmin(*this);
    pDefs       = new DefAdmin(*this);
    pCes->Set_Related(*pTypes);
}

RepositoryPartition::~RepositoryPartition()
{
}

void
RepositoryPartition::Calculate_AllSecondaryInformation()
//                                        const ::autodoc::Options & )
{
    // KORR_FUTURE
    //  Forward the options from here.

    Connect_AllTypes_2_TheirRelated_CodeEntites();
}

const String &
RepositoryPartition::RepositoryTitle() const
{
    return static_cast< ary::Repository* >(pRepositoryCenter)->Title();
}

const CodeEntity *
RepositoryPartition::Search_RelatedCe(Type_id i_type) const
{
    if (NOT i_type.IsValid())
        return 0;

    Ce_id
        ce_id = pTypes->Find_Type(i_type).RelatedCe();
    return ce_id.IsValid()
                ?   & pCes->Find_Ce(ce_id)
                :   (CodeEntity*)(0);
}

const ::ary::cpp::CppEntity *
RepositoryPartition::Search_Entity(GlobalId i_id) const
{
    if (i_id.Id() == 0)
        return 0;

    if ( NOT IsDefine(i_id) )
    {
        // Shall make sure this is a C++ CodeEntity:
        csv_assert( i_id.Class() >= Namespace::class_id
                    AND
                    i_id.Class() < BuiltInType::class_id
                    && "Unexpected entity type in cpp::RepositoryPartition"
                       "::Search_Entity()." );
        return & Ces().Find_Ce( Ce_id(i_id.Id()) );
    }
    else
    {
        return & Defs().Find_Def( De_id(i_id.Id()) );
    }
}


const CePilot &
RepositoryPartition::Ces() const
{
    csv_assert(pCes != 0);
    return *pCes;
}

const DefPilot &
RepositoryPartition::Defs() const
{
    csv_assert(pDefs != 0);
    return *pDefs;
}

const TypePilot &
RepositoryPartition::Types() const
{
    csv_assert(pTypes != 0);
    return *pTypes;
}

const loc::LocationPilot &
RepositoryPartition::Locations() const
{
    csv_assert(pLocations != 0);
    return *pLocations;
}

CePilot &
RepositoryPartition::Ces()
{
    csv_assert(pCes != 0);
    return *pCes;
}

DefPilot &
RepositoryPartition::Defs()
{
    csv_assert(pDefs != 0);
    return *pDefs;
}

TypePilot &
RepositoryPartition::Types()
{
    csv_assert(pTypes != 0);
    return *pTypes;
}

loc::LocationPilot &
RepositoryPartition::Locations()
{
    csv_assert(pLocations != 0);
    return *pLocations;
}


void
RepositoryPartition::Connect_AllTypes_2_TheirRelated_CodeEntites()
{
    TypeConnector
        aConnector(*this);
    std::for_each( pTypes->Storage().BeginUnreserved(),
                   pTypes->Storage().End(),
                   aConnector );

    typedef ::ary::stg::filter_iterator<CodeEntity,Class>
        filter_class_iter;

    HierarchyLinker
        aHierarchyLinker(*this);
    filter_class_iter itEnd( pCes->Storage().End() );
    for ( filter_class_iter it( pCes->Storage().BeginUnreserved() );
          it != itEnd AND it.IsValid();
          ++it )
    {
        if (is_type<Class>(*it))
            aHierarchyLinker(ary_cast<Class>(*it));
    }

    TypeConnector2ndTry
        aConnector2ndTry(*this);
    std::for_each( pTypes->Storage().BeginUnreserved(),
                   pTypes->Storage().End(),
                   aConnector2ndTry );
}

template <class COMPARE>
void                    Add2Result(
                            List_GlobalIds &    o_result,
                            const SortedIds<COMPARE> &
                                                i_data,
                            const char *        i_begin,
                            const char *        i_end );
template <class COMPARE>
void
Add2Result( List_GlobalIds &            o_result,
            const SortedIds<COMPARE> &  i_data,
            const char *                i_begin,
            const char *                i_end )
{
    const size_t
        previous_size   = o_result.size();
    typename std::vector<typename COMPARE::id_type>::const_iterator
        it_beg          = i_data.LowerBound(i_begin);
    typename std::vector<typename COMPARE::id_type>::const_iterator
        it_end          = i_data.LowerBound(i_end);
    size_t
        count_added = static_cast<size_t>( std::distance(it_beg,it_end) );
    o_result.insert(    o_result.end(),
                        count_added,
                        GlobalId() );
    List_GlobalIds::iterator
        it_out          = o_result.begin() + previous_size;
    std::transform(     it_beg, it_end,
                        it_out,
                        MakeGlobalId<COMPARE>() );
}


uintt
RepositoryPartition::Get_AlphabeticalList( List_GlobalIds &    o_result,
                                           const char *        i_begin,
                                           const char *        i_end ) const
{
    size_t
        ret = o_result.size();

    const Ce_Storage &
        ce_storage = pCes->Storage();
    const Def_Storage &
        def_storage = pDefs->Storage();

    Add2Result( o_result,
                ce_storage.TypeIndex(),
                i_begin, i_end );
    Add2Result( o_result,
                ce_storage.OperationIndex(),
                i_begin, i_end );
    Add2Result( o_result,
                ce_storage.DataIndex(),
                i_begin, i_end );
    Add2Result( o_result,
                def_storage.DefineIndex(),
                i_begin, i_end );
    Add2Result( o_result,
                def_storage.MacroIndex(),
                i_begin, i_end );

    LesserGlobal
        aLess(ce_storage, def_storage);

    std::sort(o_result.begin(), o_result.end(), aLess);

    return o_result.size() - ret;
}




}   // namespace cpp
}   // namespace ary





namespace
{


void
TypeConnector::operator()( Type & io_rType ) const
{
    csv_assert(pGate != 0);
    UsedType *
        pt = ::ary::ary_cast<UsedType>(&io_rType);
    if (pt != 0)
        pt->Connect2Ce(pGate->Ces());
}

void
TypeConnector2ndTry::operator()( Type & io_rType ) const
{
    csv_assert(pGate != 0);
    UsedType *
        pt = ::ary::ary_cast<UsedType>(&io_rType);
    if (pt != 0)
        pt->Connect2CeOnlyKnownViaBaseClass(*pGate);
}

void
HierarchyLinker::operator()( Class & io_rCe ) const
{
    csv_assert( ::ary::is_type<Class>(io_rCe) );
    Class &
        rClass = io_rCe;

    for ( List_Bases::const_iterator it = rClass.BaseClasses().begin();
          it != rClass.BaseClasses().end();
          ++it )
    {
        const CodeEntity *
            pCe = 0;
        Type_id
            nTid = (*it).nId;
        for ( pCe = pGate->Search_RelatedCe(nTid);
              ary::ary_cast<Typedef>(pCe) != 0;
              pCe = pGate->Search_RelatedCe(nTid) )
        {
            nTid = static_cast< const Typedef* >(pCe)->DescribingType();
        }
        const Class *
            pClass = ary::ary_cast<Class>(pCe);
        if (pClass == 0)
            return;
        // KORR_FUTURE:  we need a non const Find_Class()
        const_cast< Class* >(pClass)->Add_KnownDerivative( io_rCe.CeId() );
    }
}

const String &
LesserGlobal::NameOf(GlobalId  i_id) const
{
    if ( NOT IsDefine(i_id) )
    {
        return rCes[i_id.Id()].LocalName();
    }
    else
    {
        return rDes[i_id.Id()].LocalName();
    }
}



}   // namespace anonymous
