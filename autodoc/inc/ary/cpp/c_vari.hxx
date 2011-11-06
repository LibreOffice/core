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



#ifndef ARY_CPP_C_VARI_HXX
#define ARY_CPP_C_VARI_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_vfflag.hxx>



namespace ary
{
namespace cpp
{


/** A C++ variable or constant declaration.
*/
class Variable : public CodeEntity
{
  public:
    // LIFECYCLE
    enum E_ClassId { class_id = 1005 };

                        Variable(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            loc::Le_id          i_nFile,
                            Type_id             i_nType,
                            VariableFlags       i_aFlags,
                            const String  &     i_sArraySize,
                            const String  &     i_sInitValue );
                        ~Variable();


    // INQUIRY
    Type_id             Type() const;
    const String  &     ArraySize() const;
    const String  &     Initialisation() const;
    E_Protection        Protection() const      { return eProtection; }

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

    // DATA
    CeEssentials        aEssentials;
    Type_id             nType;
    E_Protection        eProtection;
    VariableFlags       aFlags;
    String              sArraySize;
    String              sInitialisation;
};



// IMPLEMENTATION
inline Type_id
Variable::Type() const
    { return nType; }
inline const String  &
Variable::ArraySize() const
    { return sArraySize; }
inline const String  &
Variable::Initialisation() const
    { return sInitialisation; }



}   // namespace cpp
}   // namespace ary
#endif
