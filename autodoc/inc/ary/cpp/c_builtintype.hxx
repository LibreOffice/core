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



#ifndef ARY_CPP_C_BUILTINTYPE_HXX
#define ARY_CPP_C_BUILTINTYPE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_type.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace cpp
{


/** Represents types like void, int, double.
*/
class BuiltInType : public Type
{
  public:
    enum E_ClassId { class_id = 1200 };

                        BuiltInType(
                            const String  &     i_sName,
                            E_TypeSpecialisation
                                                i_Specialisation );

    String              SpecializedName() const;

    static String       SpecializedName_(
                            const char *        i_sName,
                            E_TypeSpecialisation
                                                i_eTypeSpecialisation );
  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface Object:
    virtual ClassId     get_AryClass() const;

    // Interface Type:
    virtual bool        inq_IsConst() const;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &        i_rGate ) const;
    // DATA
    String              sName;
    E_TypeSpecialisation
                        eSpecialisation;
};



inline String
BuiltInType::SpecializedName() const
{
    return SpecializedName_(sName, eSpecialisation);
}




}   // namespace cpp
}   // namespace ary
#endif
