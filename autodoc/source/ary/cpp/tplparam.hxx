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



#ifndef ARY_CPP_TPLPARAM_HXX
#define ARY_CPP_TPLPARAM_HXX

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>




namespace ary
{
namespace cpp
{
    class UsedType;
    class Gate;

namespace ut
{


class TemplateParameter
{
  public:
    virtual             ~TemplateParameter() {}

    virtual intt        Compare(
                            const TemplateParameter &
                                                i_rOther ) const = 0;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const = 0;
};


class TplParameter_Type : public TemplateParameter
{
  public:
                        TplParameter_Type(
                            Tid                 i_nType );
                        ~TplParameter_Type();

    virtual intt        Compare(
                            const TemplateParameter &
                                                i_rOther ) const;
    virtual void        Get_Text(
                            StreamStr &         o_rOut,
                            const ary::cpp::Gate &
                                                i_rGate ) const;
  private:
    Tid                 nType;
};

}   // namespace ut
}   // namespace cpp
}   // namespace ary
#endif
