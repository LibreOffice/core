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

#ifndef ARY_IDL_IA_CE_HXX
#define ARY_IDL_IA_CE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ip_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

class Ce_Storage;
class TypeAdmin;


/** @resp
    Implements ::ary::idl::CePilot. Provides the access logic for all
    IDL code entities.

    @collab Ce_Storage
    @collab TypeAdmin

    @see CodeEntity
*/
class CeAdmin : public CePilot
{
  public:
    // LIFECYCLE
                        CeAdmin(
                            NameLookup &        io_rNameDictionary,
                            TypeAdmin &         io_rTypePilot );
    virtual             ~CeAdmin();

    // OPERATIONS

    // INQUIRY
    const Ce_Storage &  Storage() const;

    // ACCESS
    Ce_Storage &        Storage();

    // INHERITED
        // Interface ::ary::idl::CePilot:
    virtual Module &    CheckIn_Module(
                            Ce_id               i_nParentId,
                            const String &      i_sName );
    virtual Service &   Store_Service(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual SglIfcService &
                        Store_SglIfcService(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBaseInterface );
    virtual Interface & Store_Interface(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase );
    virtual Struct &    Store_Struct(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase,
                            const String &      i_sTemplateParam );
    virtual Exception & Store_Exception(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase );
    virtual Enum &      Store_Enum(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual Typedef &   Store_Typedef(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nDefiningType );
    virtual ConstantsGroup &
                        Store_ConstantsGroup(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual Singleton & Store_Singleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual SglIfcSingleton &
                        Store_SglIfcSingleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBaseInterface );

    virtual Constant &  Store_Constant(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            const String &      i_sValue );
    virtual Property &  Store_Property(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            Property::Stereotypes
                                                i_stereotypes );
    virtual Function &  Store_Function(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nReturnType,
                            bool                i_bOneWay );
    virtual Function &  Store_ServiceConstructor(
                            Ce_id               i_nOwner,
                            const String &      i_sName );
    virtual StructElement &
                        Store_StructMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType );
    virtual StructElement &
                        Store_ExceptionMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType );
    virtual EnumValue & Store_EnumValue(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            const String &      i_sValue );
    virtual Attribute & Store_Attribute(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            bool                i_bReadOnly,
                            bool                i_bBound );

    virtual const Module &
                        GlobalNamespace() const;
    virtual const CodeEntity &
                        Find_Ce(
                            Ce_id               i_nId ) const;
    virtual const Module &
                        Find_Module(
                            Ce_id               i_nId ) const;
    virtual const Module *
                        Search_Module(
                            Ce_id               i_nId ) const;
    virtual const Function &
                        Find_Function(
                            Ce_id               i_nId ) const;
    virtual const Property &
                        Find_Property(
                            Ce_id               i_nId ) const;
    virtual const EnumValue &
                        Find_EnumValue(
                            Ce_id               i_nId ) const;
    virtual const Constant &
                        Find_Constant(
                            Ce_id               i_nId ) const;
    virtual const StructElement &
                        Find_StructElement(
                            Ce_id               i_nId ) const;
    virtual void        Get_Text(
                            StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const CodeEntity &  i_ce ) const;
    virtual const NameLookup &
                        NameDictionary() const;
    virtual void        Get_AlphabeticalIndex(
                            std::vector<Ce_id> &
                                                o_rResult,
                            alphabetical_index::E_Letter
                                                i_cLetter) const;
    // ACCESS
    virtual Module &    GlobalNamespace();
    virtual CodeEntity &
                        Find_Ce(
                            Ce_id               i_nId );
  private:
    // Locals
    Module &            lhf_Access_Module(
                            Ce_id               i_nId );
    void                lhf_Put2Storage_and_AssignId(
                            CodeEntity &        pass_io_rCe );
    void                lhf_Store_NewEntity(
                            DYN CodeEntity &    pass_io_rCe,
                            Module &            i_rOwner );
    void                lhf_Store_NewEntity(
                            DYN CodeEntity &    pass_io_rCe,
                            Ce_id               i_nOwnerModule );
    void                get_ModuleText(
                            StringVector &      o_module,
                            const CodeEntity &  i_ce ) const;
    Struct &            lhf_Store_TplStruct(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase,
                            const String &      i_sTemplateParam );

    const Ce_Storage &  my_Storage() const;
    Ce_Storage &        my_Storage();
    NameLookup &        my_NameDictionary()     { return *pNameDictionary; }

    // DATA
    Dyn<Ce_Storage>     pStorage;           /// @inv pStorage != 0;
    Module *            pGlobalNamespace;
    NameLookup *        pNameDictionary;
    TypeAdmin *         pTypePilot;
};


// IMPLEMENTATION
inline const Ce_Storage &
CeAdmin::Storage() const
{
    return *pStorage;
}

inline Ce_Storage &
CeAdmin::Storage()
{
    return *pStorage;
}



}   // namespace idl
}   // namespace ary
#endif
