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

#ifndef ARY_CPP_CP_CE_HXX
#define ARY_CPP_CP_CE_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
    class QualifiedName;

namespace cpp
{
    class Class;
    class CodeEntity;
    class Enum;
    class EnumValue;
    class Function;
    class InputContext;
    class Namespace;
    class OperationSignature;
    class Typedef;
    class Variable;

    struct FunctionFlags;
    struct S_Parameter;
    struct VariableFlags;
}
}





namespace ary
{
namespace cpp
{



/** Acess to all declared C++ code entites (types, variables, operations)
    in the repository.
*/
class CePilot
{
  public:
    // LIFECYCLE
    virtual             ~CePilot() {}

    // OPERATIONS
    virtual Namespace &
                        CheckIn_Namespace(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName ) = 0;
    virtual Class &     Store_Class(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            E_ClassKey          i_classKey ) = 0;
    virtual Enum &      Store_Enum(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName ) = 0;
    virtual Typedef &   Store_Typedef(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_referredType ) = 0;

    /// @return 0, if the function is duplicate.
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
                            const std::vector<Tid> &
                                                i_exceptions ) = 0;
    virtual Variable &  Store_Variable(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            Type_id             i_type,
                            VariableFlags       i_flags,
                            const String  &     i_arraySize,
                            const String  &     i_initValue ) = 0;
    virtual EnumValue & Store_EnumValue(
                            const InputContext &
                                                i_context,
                            const String  &     i_localName,
                            const String  &     i_initValue ) = 0;
    // INQUIRY
    virtual const Namespace &
                        GlobalNamespace() const = 0;
    virtual const CodeEntity &
                        Find_Ce(
                            Ce_id               i_id ) const = 0;
    virtual const CodeEntity *
                        Search_Ce(
                            Ce_id               i_id ) const = 0;

    /// It's assumed that i_rSearchedName is an absolute name.
    virtual const CodeEntity *
                        Search_CeAbsolute(
                            const CodeEntity &  i_curScope,
                            const QualifiedName &
                                                i_absoluteName ) const = 0;
    virtual const CodeEntity *
                        Search_CeLocal(
                            const String  &     i_relativeName,
                            bool                i_isFunction,
                            const Namespace &   i_curNamespace,
                            const Class *       i_curClass ) const = 0;
    virtual void        Get_QualifiedName(
                            StreamStr &         o_result,
                            const String  &     i_localName,
                            Ce_id               i_owner,
                            const char *        i_delimiter = "::" ) const = 0;
    virtual void        Get_SignatureText(
                            StreamStr &         o_rOut,
                            const OperationSignature &
                                                i_signature,
                            const StringVector *
                                                i_sParameterNames = 0 ) const = 0;
    virtual CesResultList
                        Search_TypeName(
                            const String  &     i_sName ) const = 0;
    // ACCESS
       virtual Namespace & GlobalNamespace() = 0;
};




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
