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



#ifndef ARY_IDL_IK_CE_HXX
#define ARY_IDL_IK_CE_HXX
//  KORR_DEPRECATED_3.0



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>
#include <ary/stdconstiter.hxx>

namespace ary
{
namespace info
{
     class Text;
}
namespace idl
{


namespace ifc_ce
{


typedef ::ary::Dyn_StdConstIterator<Ce_id>      Dyn_CeIterator;
typedef ::ary::Dyn_StdConstIterator<Type_id>    Dyn_TypeIterator;
typedef ::ary::info::Text                       DocText;



struct attr
{
    static Ce_id        CeId(
                            const CodeEntity &  i_ce );
    static const String &
                        LocalName(
                            const CodeEntity &  i_ce );
    static Ce_id        NameRoom(
                            const CodeEntity &  i_ce );
    static Rid          Owner(
                            const CodeEntity &  i_ce );
    static E_SightLevel SightLevel(
                            const CodeEntity &  i_ce );
    static bool         Search_Member(
                            const CodeEntity &  ,
                            const String &      )
                            { return true; }    // KORR_FUTURE
};

struct xref
{
};

struct doc
{
    static const DocText &
                        ShortInfo(          /// @return a short description of the CodeEntity
                            const CodeEntity &  i_ce );

    static const DocText &
                        TagAuthor(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagExample(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagDescr(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagGuarantees(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagKey(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagMissing(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagSee(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagShort(
                            const CodeEntity &  i_ce );
    static const DocText &
                        TagVersion(
                            const CodeEntity &  i_ce );

    void                Get_UnkownTags(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );

    bool                IsDeprecated(
                            const CodeEntity &  i_ce );
    bool                IsIncomplete(
                            const CodeEntity &  i_ce );
    bool                IsInternal(
                            const CodeEntity &  i_ce );
    bool                IsNodoc(
                            const CodeEntity &  i_ce );
    bool                IsOptional(
                            const CodeEntity &  i_ce );
    bool                IsSuspicious(
                            const CodeEntity &  i_ce );

};


}   // namespace ifc_ce


}   // namspace idl
}   // namspace ary

#endif


