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



#ifndef ARY_CPP_C_CPPENTITY_HXX
#define ARY_CPP_C_CPPENTITY_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // OTHER
#include <ary/doc/d_docu.hxx>



namespace ary
{
namespace cpp
{


/** A C++ code entity as parsed by Autodoc.
*/
class CppEntity : public Entity
{
  public:
    // LIFECYCLE
    virtual             ~CppEntity() {}

    // OPERATIONS

    // INQUIRY
    const ary::doc::Documentation &
                        Docu() const;
    // ACCESS
    void                Set_Docu(
                            DYN ary::doc::Node &
                                                pass_docudata );
  private:
    // DATA
    ary::doc::Documentation
                        aDocu;
};




// IMPLEMENTATION
inline const doc::Documentation &
CppEntity::Docu() const
{
    return aDocu;
}

inline void
CppEntity::Set_Docu(ary::doc::Node & pass_docudata)
{
    aDocu.Set_Data(pass_docudata);
}




}   // namespace cpp
}   // namespace ary
#endif
