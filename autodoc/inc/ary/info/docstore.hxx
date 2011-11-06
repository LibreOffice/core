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



#ifndef ARY_INFO_DOCSTORE_HXX
#define ARY_INFO_DOCSTORE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/info/inftypes.hxx>

namespace ary
{
namespace doc
{
    class Node;
}



namespace info
{

class DocuStore
{
  public:
    virtual             ~DocuStore() {}

    void                Store2CurFile(
                            DYN doc::Node     & let_drDocu );
    void                Store2CurNamespace(
                            DYN doc::Node     & let_drDocu );

    void                Store2ConnectedDeclaration(
                            DYN doc::Node     & let_drDocu );

    void                Store2Glossary(
                            DYN doc::Node     & let_drDocu,
                            const String  &     i_sExplainedTerm );
    void                Store2GlobalTexts(
                            DYN doc::Node     & let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId );
  private:
    virtual void        do_Store2CurFile(
                            DYN doc::Node     & let_drDocu ) = 0;
    virtual void        do_Store2CurNamespace(
                            DYN doc::Node     & let_drDocu ) = 0;

    virtual void        do_Store2ConnectedDeclaration(
                            DYN doc::Node     & let_drDocu ) = 0;

    virtual void        do_Store2Glossary(
                            DYN doc::Node     & let_drDocu,
                            const String  &     i_sExplainedTerm ) = 0;
    virtual void        do_Store2GlobalTexts(
                            DYN doc::Node     & let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId ) = 0;
};




// IMPLEMENTATION
inline void
DocuStore::Store2CurFile( DYN doc::Node     & let_drDocu )
    { do_Store2CurFile(let_drDocu);  }
inline void
DocuStore::Store2CurNamespace( DYN doc::Node     & let_drDocu )
    { do_Store2CurNamespace(let_drDocu);  }
inline void
DocuStore::Store2ConnectedDeclaration( DYN doc::Node     & let_drDocu )
    { do_Store2ConnectedDeclaration(let_drDocu);  }
inline void
DocuStore::Store2Glossary( DYN doc::Node     &  let_drDocu,
                           const String  &      i_sExplainedTerm )
    { do_Store2Glossary(let_drDocu, i_sExplainedTerm);  }
inline void
DocuStore::Store2GlobalTexts( DYN doc::Node     &       let_drDocu,
                              ary::info::GlobalTextId   i_nId )
    { do_Store2GlobalTexts(let_drDocu, i_nId);  }




}   // namespace info
}   // namespace ary
#endif
