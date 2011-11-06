/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    virtual Lid         inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

    // Local Types
    typedef StringVector                ParameterInfoList;
    typedef std::vector<Type_id>        ExceptionTypeList;

    // DATA
    CeEssentials        aEssentials;
    List_TplParam       aTemplateParameterTypes;
    OperationSignature  aSignature;
    Type_id             nReturnType;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
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
