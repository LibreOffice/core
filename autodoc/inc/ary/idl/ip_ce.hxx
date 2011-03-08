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

#ifndef ARY_IDL_IP_CE_HXX
#define ARY_IDL_IP_CE_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>
#include <ary/idl/i_property.hxx>




namespace ary
{
namespace idl
{
    class Module;

    class ConstantsGroup;
    class Enum;
    class Exception;
    class Interface;
    class Service;
    class SglIfcService;
    class Singleton;
    class SglIfcSingleton;
    class Struct;
    class Typedef;

    class Attribute;
    class Constant;
    class EnumValue;
    class Function;
    class Property;
    class StructElement;
    class Variable;

    class NameLookup;


/** Provides the access logic for all code entities.
*/
class CePilot
{
  public:
    // LIFECYCLE
    virtual             ~CePilot() {}

    // OPERATIONS
    virtual Module &    CheckIn_Module(
                            Ce_id               i_nParentId,
                            const String &      i_sName ) = 0;
    virtual Service &   Store_Service(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual SglIfcService &
                        Store_SglIfcService(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBaseInterface ) = 0;
    virtual Interface & Store_Interface(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase ) = 0;
    virtual Struct &    Store_Struct(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase,
                            const String &      i_sTemplateParam = String::Null_() ) = 0;
    virtual Exception & Store_Exception(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBase ) = 0;
    virtual Enum &      Store_Enum(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual Typedef &   Store_Typedef(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nDefiningType ) = 0;
    virtual ConstantsGroup &
                        Store_ConstantsGroup(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual Singleton & Store_Singleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual SglIfcSingleton &
                        Store_SglIfcSingleton(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nBaseInterface ) = 0;

    virtual Constant &  Store_Constant(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            const String &      i_sValue ) = 0;
    virtual Property &  Store_Property(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            Property::Stereotypes
                                                i_stereotypes ) = 0;
    virtual Function &  Store_Function(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nReturnType,
                            bool                i_bOneWay ) = 0;
    virtual Function &  Store_ServiceConstructor(
                            Ce_id               i_nOwner,
                            const String &      i_sName ) = 0;
    virtual StructElement &
                        Store_StructMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType ) = 0;
    virtual StructElement &
                        Store_ExceptionMember(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType ) = 0;
    virtual EnumValue & Store_EnumValue(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            const String &      i_sValue ) = 0;
    virtual Attribute & Store_Attribute(
                            Ce_id               i_nOwner,
                            const String &      i_sName,
                            Type_id             i_nType,
                            bool                i_bReadOnly,
                            bool                i_bBound ) = 0;
    // INQUIRY
    virtual const Module &
                        GlobalNamespace() const = 0;
    virtual const CodeEntity &
                        Find_Ce(
                            Ce_id               i_nId ) const = 0;

    virtual const Module &
                        Find_Module(
                            Ce_id               i_nId ) const = 0;
    virtual const Module *
                        Search_Module(
                            Ce_id               i_nId ) const = 0;
    virtual const Function &
                        Find_Function(
                            Ce_id               i_nId ) const = 0;
    virtual const Property &
                        Find_Property(
                            Ce_id               i_nId ) const = 0;
    virtual const EnumValue &
                        Find_EnumValue(
                            Ce_id               i_nId ) const = 0;
    virtual const Constant &
                        Find_Constant(
                            Ce_id               i_nId ) const = 0;
    virtual const StructElement &
                        Find_StructElement(
                            Ce_id               i_nId ) const = 0;
    virtual void        Get_Text(
                            StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const CodeEntity &  i_ce ) const = 0;
    virtual const NameLookup &
                        NameDictionary() const = 0;
    virtual void        Get_AlphabeticalIndex(
                            std::vector<Ce_id> &
                                                o_rResult,
                            alphabetical_index::E_Letter
                                                i_cLetter) const = 0;
    // ACCESS
    virtual Module &    GlobalNamespace() = 0;
    virtual CodeEntity &
                        Find_Ce(
                            Ce_id               i_nId ) = 0;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
