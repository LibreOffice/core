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



#ifndef ARY_CPP_CS_TYPE_HXX
#define ARY_CPP_CS_TYPE_HXX



// USED SERVICES
    // BASE CLASSES
#include <store/s_storage.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_type.hxx>
#include <ary/cpp/c_traits.hxx>
#include <ary/cpp/usedtype.hxx>
#include <sortedids.hxx>



namespace ary
{
namespace cpp
{



/** The data base for all ->ary::cpp::Type objects.
*/
class Type_Storage : public ::ary::stg::Storage<Type>
{
  public:
    typedef SortedIds<UsedType_Compare>     UT_Index;

                        Type_Storage();
    virtual             ~Type_Storage();

    const UT_Index &    UsedTypeIndex() const;

    UT_Index &          UsedTypeIndex();
    Type_id             Search_BuiltInType(
                            const String &      i_specializedName ) const;

    static Type_Storage &
                        Instance_()             { csv_assert(pInstance_ != 0);
                                                  return *pInstance_; }
  private:
    // Locals
    void                Setup_BuiltInTypes();
    void                Set_BuiltInType(
                            Rid                 i_nId,
                            const char *        i_sName,
                            ary::cpp::E_TypeSpecialisation
                                                i_eSpecialisation = TYSP_none );
    // DATA
    UT_Index            aUsedTypes;
    std::map<String,Type_id>
                        aBuiltInTypes;


    static Type_Storage *
                        pInstance_;
};




namespace predefined
{

enum E_Type
{
    // 0 is always unused with repository storages.
    t_void = 1,
    t_bool,
    t_char,
    t_signed_char,
    t_unsigned_char,
    t_short,
    t_unsigned_short,
    t_int,
    t_unsigned_int,
    t_long,
    t_unsigned_long,
    t_float,
    t_double,
    t_size_t,
    t_wchar_t,
    t_ptrdiff_t,
    t_ellipse,
    t_MAX
};

}   // namespace predefined



// IMPLEMENTATION
inline const Type_Storage::UT_Index &
Type_Storage::UsedTypeIndex() const
{
    return aUsedTypes;
}

inline Type_Storage::UT_Index &
Type_Storage::UsedTypeIndex()
{
    return aUsedTypes;
}


}   // namespace cpp
}   // namespace ary
#endif
