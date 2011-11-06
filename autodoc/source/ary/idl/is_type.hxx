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



#ifndef ARY_IDL_IS_TYPE_HXX
#define ARY_IDL_IS_TYPE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <ary/idl/i_type.hxx>




namespace ary
{
namespace idl
{


/** The data base for all ->ary::idl::CodeEntity objects.
*/
class Type_Storage : public ::ary::stg::Storage< ::ary::idl::Type >
{
  public:
                        Type_Storage();
                        ~Type_Storage();


    void                Add_Sequence(
                            Type_id             i_nRelatedType,
                            Type_id             i_nSequence );

    Type_id             Search_SequenceOf(
                            Type_id             i_nRelatedType );

    static Type_Storage &
                        Instance_();
  private:
    /**  value_type.first   := id of the base type
         value_type.second  := id of the sequence<base type>
    */
    typedef std::map<Type_id,Type_id>           Map_Sequences;

    // DATA
    Map_Sequences       aSequenceIndex;

    static Type_Storage *
                        pInstance_;
};



namespace predefined
{

enum E_Type
{
    type_Root_ofXNameRooms = 1,
    type_GlobalXNameRoom,
    type_any,
    type_boolean,
    type_byte,
    type_char,
    type_double,
    type_float,
    type_hyper,
    type_long,
    type_short,
    type_string,
    type_type,
    type_void,
    type_u_hyper,
    type_u_long,
    type_u_short,
    type_ellipse,    // ...
    type_MAX
};

}   // namespace predefined




// IMPLEMENTATION
inline Type_Storage &
Type_Storage::Instance_()
{
    csv_assert(pInstance_ != 0);
    return *pInstance_;
}




}   // namespace idl
}   // namespace ary
#endif
