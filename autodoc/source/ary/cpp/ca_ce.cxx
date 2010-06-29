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
#include "ca_ce.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/qualiname.hxx>
#include <ary/cpp/inpcontx.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_type.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/cp_type.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/getncast.hxx>






namespace
{

String              Get_NewAnonymousNamespaceName();
String              Get_NewAnonymousName(
                        char                i_start );


}   // anonymous namespace




namespace ary
{
namespace cpp
{


// KORR_FUTURE
// What about namespace visibility ?
// Perhaps handle all/some visibility transfer only after parse is complete.
void
transfer_visibility( const Class *  i_owner,
                     CodeEntity &   o_child )
{
    if ( i_owner != 0 ? NOT i_owner->IsVisible() : false )
        o_child.Set_InVisible();
}

inline const TypePilot &
CeAdmin::Types() const
{
    csv_assert(pTypes != 0);
    return *pTypes;
}






CeAdmin::CeAdmin(RepositoryPartition & io_myReposyPartition)
    :   aStorage(),
        pTypes(0),
        pCppRepositoryPartition(&io_myReposyPartition)
{
}

void
CeAdmin::Set_Related(const TypePilot & i_types)
{
    pTypes = &i_types;
}

CeAdmin::~CeAdmin()
{
}

Namespace &
CeAdmin::CheckIn_Namespace( const InputContext & i_context,
                            const String  &      i_localName )
{
    const String
        local_name = NOT i_localName.empty()
                                ?   i_localName
                                :   Get_NewAnonymousNamespaceName();
    Namespace &
        rParent = i_context.CurNamespace();
    Namespace *
        ret = rParent.Search_LocalNamespace(local_name);
    if ( ret == 0 )
    {
        ret = &Create_Namespace(rParent, local_name);
    }
    return *ret;
}

Class &
CeAdmin::Store_Class( const InputContext & i_context,
                      const String  &      i_localName,
                      E_ClassKey           i_eClassKey )
{
    const String
        local_name = i_localName.empty()
                        ?   Get_NewAnonymousName( i_eClassKey == CK_class
                                                    ?   'c'
                                                    :   i_eClassKey == CK_struct
                                                            ?   's'
                                                            :   'u' )
                        :   i_localName;

    Class &
        ret = * new Class( local_name,
                           i_context.CurOwner().CeId(),
                           i_context.CurProtection(),
                           i_context.CurFile().LeId(),
                           i_eClassKey );
    aStorage.Store_Type(ret);
    i_context.CurOwner().Add_Class(local_name, ret.CeId());
    transfer_visibility(i_context.CurClass(), ret);

    return ret;
}

Enum &
CeAdmin::Store_Enum( const InputContext & i_context,
                     const String  &      i_localName )
{
    const String
        local_name = i_localName.empty()
                        ?   Get_NewAnonymousName('e')
                        :   i_localName;
    Enum &
        ret  = * new Enum( local_name,
                           i_context.CurOwner().CeId(),
                           i_context.CurProtection(),
                           i_context.CurFile().LeId() );
    aStorage.Store_Type(ret);
    i_context.CurOwner().Add_Enum(local_name, ret.CeId());
    transfer_visibility(i_context.CurClass(), ret);

    return ret;
}

Typedef &
CeAdmin::Store_Typedef( const InputContext& i_context,
                        const String  &     i_localName,
                        Type_id             i_referredType )
{
    Typedef &
        ret  = * new Typedef( i_localName,
                              i_context.CurOwner().CeId(),
                              i_context.CurProtection(),
                              i_context.CurFile().LeId(),
                              i_referredType );
    aStorage.Store_Type(ret);
    i_context.CurOwner().Add_Typedef(i_localName, ret.CeId());
    transfer_visibility(i_context.CurClass(), ret);

    return ret;
}

Function *
CeAdmin::Store_Operation(   const InputContext &                i_context,
                            const String  &                     i_localName,
                            Type_id                             i_returnType,
                            const std::vector<S_Parameter> &    i_parameters,
                            E_Virtuality                        i_virtuality,
                            E_ConVol                            i_conVol,
                            FunctionFlags                       i_flags,
                            bool                                i_throwExists,
                            const std::vector<Type_id> &        i_exceptions )
{
    Function &
        ret     = * new Function(   i_localName,
                                    i_context.CurOwner().CeId(),
                                    i_context.CurProtection(),
                                    i_context.CurFile().LeId(),
                                    i_returnType,
                                    i_parameters,
                                    i_conVol,
                                    i_virtuality,
                                    i_flags,
                                    i_throwExists,
                                    i_exceptions );

    // Check for double declaration:
    Ce_id
        nAlreadyExistingFunction(0);
    switch ( lhf_CheckAndHandle_DuplicateOperation(
                                        nAlreadyExistingFunction,
                                        i_context,
                                        ret) )
    {
        case df_discard_new:
                delete &ret;
                return 0;
        case df_replace:
                csv_assert(nAlreadyExistingFunction.IsValid());
                aStorage.Replace_Entity(
                                nAlreadyExistingFunction,
                                ret );
                break;
        case df_no:
                aStorage.Store_Operation(ret);   // Now it has a valid id.
                i_context.CurOwner().Add_Operation( i_localName, ret.CeId(), i_flags.IsStaticMember() );
                break;
        default:
                csv_assert(false);
    }

    transfer_visibility(i_context.CurClass(), ret);
    if ( i_context.CurProtection() != PROTECT_global )
    {
        Class *
            pClass = i_context.CurClass();
        if ( pClass != 0  AND i_virtuality != VIRTUAL_none)
        {
            pClass->UpdateVirtuality(i_virtuality);
        }
    }

    return &ret;
}

Variable &
CeAdmin::Store_Variable( const InputContext& i_context,
                         const String  &     i_localName,
                         Type_id             i_type,
                         VariableFlags       i_flags,
                         const String  &     i_arraySize,
                         const String  &     i_initValue )
{
    Variable &
        ret = * new Variable( i_localName,
                              i_context.CurOwner().CeId(),
                              i_context.CurProtection(),
                              i_context.CurFile().LeId(),
                              i_type,
                              i_flags,
                              i_arraySize,
                              i_initValue );

    bool
        is_const = Types().Find_Type(i_type).IsConst();
    aStorage.Store_Datum(ret);
    i_context.CurOwner().Add_Variable(
                                i_localName,
                                ret.CeId(),
                                is_const,
                                i_flags.IsStaticMember() );
    transfer_visibility(i_context.CurClass(), ret);

    return ret;
}

EnumValue &
CeAdmin::Store_EnumValue( const InputContext & i_context,
                          const String  &      i_localName,
                          const String  &      i_initValue )
{
    Enum *
        parent  = i_context.CurEnum();
    csv_assert( parent != 0 );

    EnumValue &
        ret = * new EnumValue( i_localName,
                               parent->CeId(),
                               i_initValue );
    aStorage.Store_Datum(ret);
    parent->Add_Value(ret.CeId());

    // KORR also for current enum:
    transfer_visibility(i_context.CurClass(), ret);

    return ret;
}

const Namespace &
CeAdmin::GlobalNamespace() const
{
    return ary_cast<Namespace>( aStorage[predefined::ce_GlobalNamespace] );
}

const CodeEntity &
CeAdmin::Find_Ce(Ce_id i_id) const
{
    return aStorage[i_id];
}

const CodeEntity *
CeAdmin::Search_Ce(Ce_id i_id) const
{
    return aStorage.Exists(i_id)
                ?   & aStorage[i_id]
                :   (const CodeEntity*)(0);
}

const CodeEntity *
CeAdmin::Search_CeAbsolute( const CodeEntity &      i_curScope,
                            const QualifiedName &   i_rSearchedName ) const
{
    const symtree::Node<CeNode_Traits> *
        cur_node = CeNode_Traits::NodeOf_(i_curScope);
    csv_assert(cur_node != 0);

    Ce_id
        ret(0);
    cur_node->SearchUp( ret,
                        i_rSearchedName.first_namespace(),
                        i_rSearchedName.end_namespace(),
                        i_rSearchedName.LocalName()  );
    return Search_Ce(ret);
}

const CodeEntity *
CeAdmin::Search_CeLocal( const String  &     i_localName,
                         bool                i_bIsFunction,
                         const Namespace &   i_rCurNamespace,
                         const Class *       i_pCurClass ) const
{
    // KORR_FUTURE
    // See if this is correct.

    Ce_id
        ret(0);

    if ( NOT i_bIsFunction )
    {
        CesResultList
            type_instances = aStorage.TypeIndex().SearchAll(i_localName);
        CesResultList
            data_instances = aStorage.DataIndex().SearchAll(i_localName);
        Ce_id
            ret1 = Search_MatchingInstance(
                        type_instances,
                        (i_pCurClass
                                ?   i_pCurClass->CeId()
                                :   i_rCurNamespace.CeId())
                                     );
        Ce_id
            ret2 = Search_MatchingInstance(
                        data_instances,
                        (i_pCurClass
                                ?   i_pCurClass->CeId()
                                :   i_rCurNamespace.CeId())
                                     );
        if (NOT ret2.IsValid())
            ret = ret1;
        else if (NOT ret1.IsValid())
            ret = ret2;
    }
    else
    {
        CesResultList
            function_instances = aStorage.OperationIndex().SearchAll(i_localName);
        if ( function_instances.size() == 1 )
            ret = *function_instances.begin();
        else
        {
            ret = Search_MatchingInstance(
                        function_instances,
                        (i_pCurClass
                                ?   i_pCurClass->CeId()
                                :   i_rCurNamespace.CeId())
                                     );
        }
    }

    if ( ret.IsValid() )
        return & Find_Ce(ret);

    return 0;
}

void
CeAdmin::Get_QualifiedName( StreamStr &         o_rOut,
                            const String  &     i_localName,
                            Ce_id               i_nOwner,
                            const char *        i_sDelimiter ) const
{
    if ( i_localName.empty() OR NOT i_nOwner.IsValid() )
        return;

    const CodeEntity *
        pOwner = & Find_Ce( i_nOwner );
    if ( is_type<Enum>(*pOwner) )
        pOwner = &Find_Ce( Ce_id(pOwner->Owner()) );

    Get_QualifiedName( o_rOut,
                       pOwner->LocalName(),
                       Ce_id(pOwner->Owner()),
                       i_sDelimiter );
    o_rOut
        << i_sDelimiter
        << i_localName;
}

void
CeAdmin::Get_SignatureText( StreamStr &                 o_rOut,
                            const OperationSignature &  i_signature,
                            const StringVector *        i_sParameterNames ) const
{
    OperationSignature::ParameterTypeList::const_iterator
            it = i_signature.Parameters().begin();
    OperationSignature::ParameterTypeList::const_iterator
            it_end = i_signature.Parameters().end();

    const StringVector aDummy;
    StringVector::const_iterator
            itName = i_sParameterNames != 0
                            ?   i_sParameterNames->begin()
                            :   aDummy.begin();
    StringVector::const_iterator
            itName_end = i_sParameterNames != 0
                            ?   i_sParameterNames->end()
                            :   aDummy.end();

    bool
        bEmpty = (it == it_end);
    if (NOT bEmpty)
    {
        o_rOut << "( ";
        Types().Get_TypeText(o_rOut, *it);
        if (itName != itName_end)
            o_rOut << " " << (*itName);

        for ( ++it; it != it_end; ++it )
        {
            o_rOut << ", ";
            Types().Get_TypeText(o_rOut, *it);
            if (itName != itName_end)
            {
                ++itName;
                if (itName != itName_end)
                   o_rOut << " " << (*itName);
            }
        }
        o_rOut << " )";
    }
    else
    {
        o_rOut << "( )";
    }

    if ( intt(i_signature.ConVol()) & intt(ary::cpp::CONVOL_const) )
        o_rOut << " const";
    if ( intt(i_signature.ConVol()) & intt(ary::cpp::CONVOL_volatile) )
        o_rOut << " volatile";
}

CesResultList
CeAdmin::Search_TypeName(const String & i_sName) const
{
    return aStorage.TypeIndex().SearchAll(i_sName);
}

Namespace &
CeAdmin::GlobalNamespace()
{
    return ary_cast<Namespace>( aStorage[predefined::ce_GlobalNamespace] );
}

CeAdmin::E_DuplicateFunction
CeAdmin::lhf_CheckAndHandle_DuplicateOperation(
                                Ce_id &                 o_existentFunction,
                                const InputContext &    i_context,
                                const Function &        i_newFunction )
{
    if (i_context.CurProtection() != PROTECT_global)
    {
        // Assume, there will be no duplicates within the same class.

        // KORR_FUTURE
        // Assumption may be wrong in case of #defines providing different
        // versions for different compilers.
        return df_no;
    }

    std::vector<Ce_id>
        aOperationsWithSameName;
    i_context.CurNamespace().Search_LocalOperations(
                                        aOperationsWithSameName,
                                        i_newFunction.LocalName() );

    for ( std::vector<Ce_id>::const_iterator
             it = aOperationsWithSameName.begin();
          it != aOperationsWithSameName.end();
          ++it )
    {
        const Function &
            rFunction = ary_cast<Function>(aStorage[*it]);
        if (     rFunction.LocalName() == i_newFunction.LocalName()
             AND rFunction.Signature() == i_newFunction.Signature() )
        {
            if (NOT rFunction.IsIdentical(i_newFunction))
            {
                // KORR_FUTURE Make this more detailed.
                Cerr() << "Non identical function with same signature "
                       << "found: "
                       <<  i_context.CurNamespace().LocalName()
                       << "::"
                       << i_newFunction.LocalName()
                       << "(..)"
                       << Endl();
            }
            o_existentFunction = rFunction.CeId();
            if (rFunction.Docu().Data() == 0)
                return df_replace;
            else
                return df_discard_new;
        }
    }   // end for

    return df_no;
}

Namespace &
CeAdmin::Create_Namespace( Namespace &      o_parent,
                           const String  &  i_localName )
{
    DYN Namespace &
        ret = *new Namespace(i_localName, o_parent);
    aStorage.Store_Entity(ret);
    o_parent.Add_LocalNamespace(ret);
    return ret;
}

Ce_id
CeAdmin::Search_MatchingInstance( CesResultList         i_list,
                                  Ce_id                 i_owner ) const
{
    // KORR
    // Multiple results?

    for ( CesList::const_iterator it = i_list.begin();
          it != i_list.end();
          ++it )
    {
        const CodeEntity &
            ce = aStorage[*it];
        if ( ce.Owner() == i_owner)
        {
            return *it;
        }
    }
    return Ce_id(0);
}



}   // namespace cpp
}   // namespace ary



namespace
{

uintt G_nLastFreeAnonymousNamespaceNr = 0;
uintt G_nLastFreeAnonymousEntityNr = 0;

String
Get_NewAnonymousNamespaceName()
{
    StreamLock
        sl(100);
    return String( sl()
                    << "namespace_anonymous_"
                    << ++G_nLastFreeAnonymousNamespaceNr
                    << csv::c_str );

}

String
Get_NewAnonymousName(char i_cStart)
{
    StreamLock
        sl(100);
    return String( sl()
                    << i_cStart
                    << "_Anonymous__"
                    << ++G_nLastFreeAnonymousEntityNr
                    << c_str );
}



}   // namespace anonymous
