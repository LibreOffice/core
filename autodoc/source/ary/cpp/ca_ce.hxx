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

#ifndef ARY_CPP_CA_CE_HXX
#define ARY_CPP_CA_CE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cp_ce.hxx>
    // OTHER
#include "cs_ce.hxx"


namespace ary
{
namespace cpp
{
    class Ce_Storage;
    class RepositoryPartition;
}
}





namespace ary
{
namespace cpp
{



/** Administrates all C++ code entities (types, operations, variables).
*/
class CeAdmin : public CePilot
{
  public:
    // LIFECYCLE
                        CeAdmin(
                            RepositoryPartition &
                                                io_myReposyPartition );
    void                Set_Related(
                            const TypePilot &   i_types );
    virtual 		    ~CeAdmin();

    // INQUIRY
    const Ce_Storage &  Storage() const;

    // ACCESS
    Ce_Storage &        Storage();

    // INHERITED
    // Interface CePilot:
    virtual Namespace & CheckIn_Namespace(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName );
    virtual Class &     Store_Class(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            E_ClassKey          i_classKey );
    virtual Enum &      Store_Enum(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName );
    virtual Typedef &   Store_Typedef(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_referredType );
    virtual Function *  Store_Operation(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_returnType,
                            const std::vector<S_Parameter> &
                                                i_parameters,
                            E_Virtuality        i_virtuality,
                            E_ConVol            i_conVol,
                            FunctionFlags       i_flags,
                            bool                i_throwExists,
                            const std::vector<Type_id> &
                                                i_exceptions );
    virtual Variable &  Store_Variable(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_type,
                            VariableFlags       i_flags,
                            const String  &     i_arraySize,
                            const String  &     i_initValue );
    virtual EnumValue & Store_EnumValue(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            const String  &     i_initValue );
    virtual const Namespace &
                        GlobalNamespace() const;
    virtual const CodeEntity &
                        Find_Ce(
                            Ce_id				i_id ) const;
    virtual const CodeEntity *
                        Search_Ce(
                            Ce_id		        i_id ) const;
    virtual const CodeEntity *
                        Search_CeAbsolute(
                            const CodeEntity &  i_curScope,
                            const QualifiedName &
                                                i_absoluteName ) const;
    virtual const CodeEntity *
                        Search_CeLocal(
                            const String  &     i_relativeName,
                            bool                i_isFunction,
                            const Namespace &   i_curNamespace,
                            const Class *       i_curClass ) const;
    virtual void        Get_QualifiedName(
                            StreamStr &         o_result,
                            const String  &     i_localName,
                            Ce_id               i_owner,
                            const char *        i_delimiter = "::" ) const;
    virtual void        Get_SignatureText(
                            StreamStr &         o_rOut,
                            const OperationSignature &
                                                i_signature,
                            const StringVector *
                                                i_sParameterNames = 0 ) const;
    virtual CesResultList
                        Search_TypeName(
                            const String  &     i_sName ) const;
       virtual Namespace & GlobalNamespace();

  private:
    // Locals
    /// @return true, if function is duplicate.
    enum E_DuplicateFunction
    {
        df_no,
        df_replace,
        df_discard_new
    };

    /** @param o_existentFunction
            The id of the already existing function, else unset.
    */
    E_DuplicateFunction lhf_CheckAndHandle_DuplicateOperation(
                            Ce_id &             o_existentFunction,
                            const InputContext &
                                                i_context,
                            const Function &    i_newFunction );
    Namespace &         Create_Namespace(
                            Namespace &         o_parent,
                            const String  &     i_localName );
    Ce_id               Search_MatchingInstance(
                            CesResultList       i_list,
                            Ce_id               i_owner ) const;
    const TypePilot  &  Types() const;

    // DATA
    Ce_Storage          aStorage;
    const TypePilot *   pTypes;
    RepositoryPartition *
                        pCppRepositoryPartition;
};




// IMPLEMENTATION
inline const Ce_Storage &
CeAdmin::Storage() const
{
    return aStorage;
}

inline Ce_Storage &
CeAdmin::Storage()
{
    return aStorage;
}





}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
