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



#ifndef ARY_CPP_C_TYPE_HXX
#define ARY_CPP_C_TYPE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_traits.hxx>


namespace ary
{
namespace cpp
{
    class Gate;
}
}





namespace ary
{
namespace cpp
{



/** Base of all C++ types
*/
class Type : public ::ary::Entity
{
  public:
    typedef Type_Traits       traits_t;

    virtual             ~Type() {}

    Type_id             TypeId() const          { return Type_id(Id()); }
    Ce_id               RelatedCe() const;
    bool                IsConst() const;
    void                Get_Text(
                            StreamStr &         o_rOut,
                            const Gate &        i_rGate ) const;

    /** It is guaranteed, that the output is correct, also,
        if all three output-streams are the same instance.
    */
    void                Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &        i_rGate ) const;

  private:
    virtual Rid         inq_RelatedCe() const;  // Defaulted to 0. Implemented in c_builtintype.cxx.
    virtual bool        inq_IsConst() const = 0;
    virtual void        inq_Get_Text(
                            StreamStr &         o_rPreName,
                            StreamStr &         o_rName,
                            StreamStr &         o_rPostName,
                            const Gate &        i_rGate ) const = 0;
};




// IMPLEMENTATION
inline Ce_id
Type::RelatedCe() const
{
    return Ce_id(inq_RelatedCe());
}

inline bool
Type::IsConst() const
{
    return inq_IsConst();
}

inline void
Type::Get_Text( StreamStr &         o_rOut,
                const Gate &        i_rGate ) const
{
    inq_Get_Text( o_rOut, o_rOut, o_rOut, i_rGate );
}

inline void
Type::Get_Text( StreamStr &         o_rPreName,
                StreamStr &         o_rName,
                StreamStr &         o_rPostName,
                const Gate &        i_rGate ) const
{
    inq_Get_Text( o_rPreName,o_rName,o_rPostName, i_rGate );
}





}   // namespace  cpp
}   // namespace  ary
#endif
