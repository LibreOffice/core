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



#ifndef ADC_CPP_SDOCDIST_HXX
#define ADC_CPP_SDOCDIST_HXX



// USED SERVICES
    // BASE CLASSES
#include "cxt2ary.hxx"
#include <ary/info/docstore.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

using ary::Documentation;

/** Implementation struct for cpp::ContextForAry.
*/

struct ContextForAry::S_DocuDistributor : public ary::info::DocuStore
{
  public:
                        S_DocuDistributor()     :   pCurRe(0) {}
                        ~S_DocuDistributor()    {}

    void                Reset()                 { pCurRe = 0; pLastStoredDocu = 0; }

    void                SetCurrentlyStoredRe(
                            ary::cpp::CppEntity &
                                                io_rRe );
    void                Event_LineBreak();

  private:
    // Interface ary::info::DocuStore
    virtual void        do_Store2CurFile(
                            DYN ary::doc::Node& let_drDocu );
    virtual void        do_Store2CurNamespace(
                            DYN ary::doc::Node& let_drDocu );

    virtual void        do_Store2ConnectedDeclaration(
                            DYN ary::doc::Node& let_drDocu );

    virtual void        do_Store2Glossary(
                            DYN ary::doc::Node& let_drDocu,
                            const String  &     i_sExplainedTerm );
    virtual void        do_Store2GlobalTexts(
                            DYN ary::doc::Node& let_drDocu,
                            ary::info::GlobalTextId
                                                i_nId );
    // DATA
    ary::cpp::CppEntity *
                        pCurRe;
    Dyn<ary::doc::Node> pLastStoredDocu;
};


// IMPLEMENTATION

/*  The implementation is in header, though not all inline, because this file
    is included in cxt2ary.cxx only!
*/


void
ContextForAry::
S_DocuDistributor::SetCurrentlyStoredRe( ary::cpp::CppEntity & io_rRe )
{
    pCurRe = &io_rRe;
    if ( pLastStoredDocu )
        pCurRe->Set_Docu( *pLastStoredDocu.Release() );
}

inline void
ContextForAry::
S_DocuDistributor::Event_LineBreak()
{
    pCurRe = 0;
}

void
ContextForAry::
S_DocuDistributor::do_Store2CurFile( DYN ary::doc::Node & let_drDocu )
{
    // KORR_FUTURE
    delete &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2CurNamespace( DYN ary::doc::Node & let_drDocu )
{
    // KORR_FUTURE
    delete &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2ConnectedDeclaration( DYN ary::doc::Node & let_drDocu )
{
    if ( pCurRe != 0 )
        pCurRe->Set_Docu(let_drDocu);
    else
        pLastStoredDocu = &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2Glossary( DYN ary::doc::Node & let_drDocu,
                                      const String &           // i_sExplainedTerm
                                      )
{
    // KORR_FUTURE
    delete &let_drDocu;
}

void
ContextForAry::
S_DocuDistributor::do_Store2GlobalTexts( DYN ary::doc::Node &       let_drDocu,
                                         ary::info::GlobalTextId    // i_nId
                                       )
{
    // KORR_FUTURE
    delete &let_drDocu;
}




}   // namespace cpp
#endif
