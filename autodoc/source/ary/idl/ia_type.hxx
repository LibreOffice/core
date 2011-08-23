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

#ifndef ARY_IDL_IA_TYPE_HXX
#define ARY_IDL_IA_TYPE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ip_type.hxx>
    // COMPONENTS
    // PARAMETERS
#include "is_type.hxx"


namespace ary
{
namespace idl
{


class Type_Storage;
class CeAdmin;
class TemplateParamType;


class TypeAdmin : public TypePilot
{
  public:
    // LIFECYCLE
                        TypeAdmin();

    void                Assign_CePilot(
                            CeAdmin &           io_rCes );
    virtual             ~TypeAdmin();

    // OPERATIONS
    TemplateParamType & Store_TemplateParamType(
                            String              i_sName );
    // INQUIRY
    const Type_Storage &
                        Storage() const;
    // ACCESS
    Type_Storage &      Storage();

    // INHERITED
        // Interface TypePilot:
    virtual const Type &
                        CheckIn_Type(
                            QualifiedName &     i_rFullName,
                            uintt               i_nSequenceCount,
                            Ce_id               i_nModuleOfOccurrence,
                            const std::vector<Type_id> *
                                                i_templateParameters );
    virtual const Type &
                        Find_Type(
                            Type_id             i_nType ) const;
    virtual String      Search_LocalNameOf(
                            Type_id             i_nType ) const;
    virtual Ce_id       Search_CeRelatedTo(
                            Type_id             i_nType ) const;
    virtual const ExplicitNameRoom &
                        Find_XNameRoom(
                            Type_id             i_nType ) const;
    virtual bool        IsBuiltInOrRelated(
                            const Type &        i_rType ) const;
  private:
    // Locals
    CeAdmin &           my_Ces() const;

    void                lhf_Put2Storage_and_AssignId(
                            DYN Type &          pass_io_rType );

    ExplicitNameRoom &  lhf_CheckIn_XNameRoom(
                            const QualifiedName &
                                                i_rName,
                            Ce_id               i_nModuleOfOccurrence );
    Type_id             lhf_CheckIn_TypeName(
                            const String &      i_sLocalName,
                            ExplicitNameRoom &  io_rExplicitNameRoom,
                            Ce_id               i_nModuleOfOccurrence,
                            const std::vector<Type_id> *
                                                i_templateParameters );
    Type_id             lhf_CheckIn_Sequence(
                            Type_id             i_nType );
    void                lhf_CheckIn_BuiltInType(
                            const char *        i_sName,
                            Rid                 i_nId );
    const ExplicitNameRoom &
                        find_ExplicitNameRoom(
                            Type_id             i_nType ) const;
    ExplicitNameRoom &  find_ExplicitNameRoom(
                            Type_id             i_nType );
    ExplicitNameRoom &  lhf_Get_NameRoomRoot_forModuleofOccurrence(
                            Ce_id               i_nModuleOfOccurrence );

    /// @return Type_id::Null_(), if not found.
    Type_id             lhf_findBuiltInType(
                            const String &      i_sName );
    /// @precond nGlobalNamespace must be valid.
    void                lhf_Setup_BuildInTypes();

    // DATA
    Type_Storage *      pStorage;           /// @inv pStorage != 0
    CeAdmin *           pCes;               /// @inv pCes != 0

    // Data for saving time:
    Type_id             nXNameRoom_Root;    /** @descr  This is different from nXNameRoom_Global, because
                                                the root of explicit name rooms in code without leading "::" is unknown.
                                            */
    Type_id             nXNameRoom_Global;

    // HACK, because this needs to be saved somehow and is not in storage:
    std::map<Ce_id, Type_id>
                        aMap_ModuleOfOccurrence2NameRoomRoot;
};





// IMPLEMENTATION
inline const Type_Storage &
TypeAdmin::Storage() const
{
    return *pStorage;
}

inline Type_Storage &
TypeAdmin::Storage()
{
    return *pStorage;
}

inline void
TypeAdmin::Assign_CePilot( CeAdmin & io_rCes )
{
    pCes = &io_rCes;
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
