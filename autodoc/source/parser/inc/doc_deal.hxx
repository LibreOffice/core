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



#ifndef ADC_DOC_DEAL_HXX
#define ADC_DOC_DEAL_HXX

// BASE CLASSES
#include <tokens/tokproct.hxx>
// USED SERVICES
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace doc
{
    class OldCppDocu;
}
}




class DocuDealer
{
  public:
    // INQUIRY
    virtual             ~DocuDealer() {}

    // OPERATIONS
    /** @descr
        This distributes the let_drDocu to the matching ary::RepositoryEntity .

        If the docu is not inline, it will be saved and later given to the next
        ary::CodeEntity. Or it will be discarded, if there does not come a matching
        ary::CodeEntity .

        If the docu is inline after a function header or after an enum value
        or after a function parameter or after a base class, it will be stored
        together with the matching function, enum value, parameter or base class.

        If the documentation is @file or @project or @glos(sary) it will be
        stored at the matching ary::cpp::FileGroup, ary::cpp::ProjectGroup
        or ary::Glossary.
    */
    void                TakeDocu(
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo );
  private:
    virtual void        do_TakeDocu(
                            DYN ary::doc::OldCppDocu &
                                                let_drInfo ) = 0;
};




// IMPLEMENTATION
inline void
DocuDealer::TakeDocu( DYN ary::doc::OldCppDocu & let_drInfo )
    { do_TakeDocu(let_drInfo); }




#endif
