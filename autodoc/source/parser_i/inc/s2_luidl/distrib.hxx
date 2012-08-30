/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef LUIDL_DISTRIB_HXX
#define LUIDL_DISTRIB_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tokrecv.hxx>
#include <s2_dsapi/tokrecv.hxx>
#include <s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS


class ParserInfo;

namespace ary
{
    class Repository;

namespace doc
{
    class OldIdlDocu;
}   // namespace inf
}   // namespace ary)



namespace csi
{
namespace dsapi
{
    class Token_Receiver;
    class SapiDocu_PE;
}



namespace uidl
{


typedef std::vector< DYN Token * >  TokenQueue;
typedef TokenQueue::iterator        TokenIterator;

class TokenParser_Uidl;
class UnoIDL_PE;
class Token;


class TokenDistributor : private TokenProcessing_Types

{
  public:
                        TokenDistributor(
                            ary::Repository &   io_rRepository,
                            ParserInfo &            io_rParserInfo );
    void                SetTokenProvider(
                            TokenParser_Uidl &  io_rTokenSource );
    void                SetTopParseEnvironment(
                            UnoIDL_PE &         io_pTopParseEnvironment );
                        ~TokenDistributor();


    void                Reset()                 { aDocumentation.Reset(); }
    /** calls pTokenSource->GetNextToken() and checks the incoming tokens, until a
        usable token is found. This token will be forwarded to
        pTopParseEnv;
    */
    void                TradeToken();

    csi::uidl::Token_Receiver &
                        CodeTokens_Receiver();
    csi::dsapi::Token_Receiver &
                        DocuTokens_Receiver();

    /** Used from PE_File, if there is a docu to get without
        an environment to push (this is the case for modules).
    */
    DYN ary::doc::OldIdlDocu *
                        ReleaseLastParsedDocu()
                            { return aDocumentation.ReleaseLastParsedDocu(); }

    /** Used from PE_File, if the term "published" was parsed.
        The next opened parse environment will be set to be published
        (call ->UnoIDL_PE::SetPublished()).
    */
    void                Set_PublishedOn()
                            { aProcessingData.Set_PublishedOn(); }


  private:
    class Documentation;
    class ProcessingData;
    friend class ProcessingData;

    class  ProcessingData : public  csi::uidl::Token_Receiver,
                            private TokenProcessing_Types
    {
      public:
                            ProcessingData(
                                ary::Repository &   io_rRepository,
                                Documentation &         i_rDocuProcessor,
                                ParserInfo &            io_rParserInfo );
                            ~ProcessingData();
        void                SetTopParseEnvironment(
                                UnoIDL_PE &         io_pTopParseEnvironment );


        /** is called from pTokenSource before finishing a ::TokenParse2::GetNextToken()
            call and passes the just parsed token to this class.
        */
        virtual void        Receive(
                                DYN csi::uidl::Token &
                                                let_drToken );
        virtual void        Increment_CurLine();

        void                ProcessCurToken();

        UnoIDL_PE &         CurEnvironment() const;
        bool                NextTokenExists() const;
        void                Set_PublishedOn()
                                { bPublishedRecentlyOn = true; }

      private:
        typedef uintt   TokenQ_Position;
        typedef std::pair< UnoIDL_PE *, TokenQ_Position >   EnvironmentInfo;
        typedef std::vector< EnvironmentInfo >              EnvironmentStack;

        void                AcknowledgeResult();
        const csi::uidl::Token &
                            CurToken() const;
        UnoIDL_PE &         CurEnv() const;
        UnoIDL_PE &         PushEnv() const;
        uintt               CurTokenPosition() const;
        uintt               CurEnv_TriedTokenPosition() const;
        void                DecrementTryCount();

        EnvironmentStack    aEnvironments;
        TokenQueue          aTokenQueue;
        TokenIterator       itCurToken;
        TokenProcessing_Result
                            aCurResult;
        uintt               nTryCount;
        ary::Repository &
                            rRepository;
        ParserInfo &        rParserInfo;
        Documentation *     pDocuProcessor;
        bool                bPublishedRecentlyOn;
    };

    class Documentation : public csi::dsapi::Token_Receiver
    {
      public:
                            Documentation(
                                ParserInfo &    io_rParserInfo);
                            ~Documentation();

        void                Reset()             { bIsPassedFirstDocu = false; }

        virtual void        Receive(
                                DYN csi::dsapi::Token &
                                                let_drToken );
        virtual void        Increment_CurLine();
        DYN ary::doc::OldIdlDocu *
                            ReleaseLastParsedDocu()
                                                { return pMostRecentDocu.Release(); }
      private:
        Dyn<csi::dsapi::SapiDocu_PE>
                            pDocuParseEnv;
        ParserInfo &        rParserInfo;
        Dyn<ary::doc::OldIdlDocu>
                            pMostRecentDocu;
        bool                bIsPassedFirstDocu;
    };

    // DATA
    TokenParser_Uidl *  pTokenSource;
    Documentation       aDocumentation;
    ProcessingData      aProcessingData;
};



// IMPLEMENTATION

inline void
TokenDistributor::SetTokenProvider( TokenParser_Uidl &  io_rTokenSource )
    { pTokenSource = &io_rTokenSource; }

inline void
TokenDistributor::SetTopParseEnvironment( UnoIDL_PE & io_pTopParseEnvironment )
    { aProcessingData.SetTopParseEnvironment(io_pTopParseEnvironment); }

inline csi::uidl::Token_Receiver &
TokenDistributor::CodeTokens_Receiver()
    { return aProcessingData; }

inline csi::dsapi::Token_Receiver &
TokenDistributor::DocuTokens_Receiver()
    { return aDocumentation; }

inline const csi::uidl::Token &
TokenDistributor::ProcessingData::CurToken() const
{
    csv_assert( itCurToken != aTokenQueue.end() );
    csv_assert( *itCurToken != 0 );
    return *(*itCurToken);
}

inline UnoIDL_PE &
TokenDistributor::ProcessingData::CurEnv() const
{
    csv_assert( aEnvironments.size() > 0 );
    csv_assert( aEnvironments.back().first != 0 );
    return *aEnvironments.back().first;
}

inline UnoIDL_PE &
TokenDistributor::ProcessingData::PushEnv() const
{
    csv_assert( aCurResult.pEnv2Push != 0 );
    return *aCurResult.pEnv2Push;
}

inline uintt
TokenDistributor::ProcessingData::CurTokenPosition() const
{
    return itCurToken - aTokenQueue.begin();
}

inline uintt
TokenDistributor::ProcessingData::CurEnv_TriedTokenPosition() const
{
    csv_assert( aEnvironments.size() > 0 );
    return aEnvironments.back().second;
}


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
