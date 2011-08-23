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

#ifndef ARY_CPP_C_FUNCT_HXX
#define ARY_CPP_C_FUNCT_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_slntry.hxx>
#include <ary/cpp/c_vfflag.hxx>
#include <ary/cpp/c_osigna.hxx>




namespace ary
{
namespace cpp
{



/** A C++ function declaration.
*/
class Function : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 1004 };

                        Function(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            loc::Le_id          i_nFile,
                            Type_id             i_nReturnType,
                            const std::vector<S_Parameter> &
                                                i_parameters,
                            E_ConVol            i_conVol,
                            E_Virtuality        i_eVirtuality,
                            FunctionFlags       i_aFlags,
                            bool                i_bThrowExists,
                            const std::vector<Type_id> &
                                                i_rExceptions );
                        ~Function();


    // OPERATIONS
    void                Add_TemplateParameterType(
                            const String  &     i_sLocalName,
                            Type_id             i_nIdAsType );

    // INQUIRY
    const OperationSignature &
                        Signature() const;
    Type_id             ReturnType() const;
    E_Protection        Protection() const      { return eProtection; }
    E_Virtuality        Virtuality() const      { return eVirtuality; }
    const FunctionFlags &
                        Flags() const           { return aFlags; }
    const StringVector &
                        ParamInfos() const      { return aParameterInfos; }
    const std::vector<Type_id> *
                        Exceptions() const      { return pExceptions.Ptr(); }

    const List_TplParam &
                        TemplateParameters() const
                                                { return aTemplateParameterTypes; }
    bool                IsIdentical(
                            const Function &    i_f ) const;

  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::cpp::CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid			inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

    // Local Types
    typedef StringVector                ParameterInfoList;
    typedef std::vector<Type_id>        ExceptionTypeList;

    // DATA
    CeEssentials		aEssentials;
    List_TplParam       aTemplateParameterTypes;
    OperationSignature  aSignature;
    Type_id             nReturnType;
    E_Protection		eProtection;
    E_Virtuality		eVirtuality;
    FunctionFlags       aFlags;
    ParameterInfoList   aParameterInfos;
    Dyn<ExceptionTypeList>
                        pExceptions;            //  if (NOT pExceptions) there is no throw,
                                                //    else, there is one, but the list still may be empty.
};




// IMPLEMENTATION
inline const OperationSignature &
Function::Signature() const
    { return aSignature; }
inline Type_id
Function::ReturnType() const
    { return nReturnType; }




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
