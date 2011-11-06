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



#ifndef ARY_CPP_C_SLNTRY_HXX
#define ARY_CPP_C_SLNTRY_HXX
//  KORR_DEPRECATED_3.0

// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/namesort.hxx>


namespace ary
{
namespace cpp
{
    class Namespace;



typedef Namespace * NamespacePtr;
struct Less_NamespacePtr
{
    bool                operator()(             /// @return true if (i_p1->Name() < i_p2->Name()) .
                            const NamespacePtr& i_p1,
                            const NamespacePtr& i_p2 );
};




struct S_Classes_Base
{
    Type_id             nId;
    E_Protection        eProtection;
    E_Virtuality        eVirtuality;
    String              sComment;

                        S_Classes_Base()
                            :   nId(0),
                                eProtection(PROTECT_global),
                                eVirtuality(VIRTUAL_none)
                                // sComment
                                { }
};

struct S_TplParam
{
    String              sName;
    Type_id             nId;

                        S_TplParam(
                            String              i_sName,
                            Type_id             i_nId )
                            :   sName(i_sName), nId(i_nId) {}
    const String  &     Name() const            { return sName; }
};


struct S_LocalCe
{
    String              sLocalName;
    Ce_id               nId;

                        S_LocalCe()             : nId(0) {}
                        S_LocalCe(
                            const String  &     i_sLocalName,
                            Cid                 i_nId )
                                                : sLocalName(i_sLocalName), nId(i_nId) {}
    bool                operator<(
                            const S_LocalCe &   i_rCe ) const
                                                { return LesserName()(sLocalName,i_rCe.sLocalName); }
};

typedef std::vector< S_LocalCe >        List_LocalCe;


typedef std::map<String, NamespacePtr>          Map_NamespacePtr;
typedef std::vector< S_Classes_Base >           List_Bases;
typedef std::vector< S_TplParam >               List_TplParam;



}   // namespace cpp
}   // namespace ary
#endif
