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

#ifndef LUIDL_PE_EXCP_HXX
#define LUIDL_PE_EXCP_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
#include <s2_luidl/semnode.hxx>
#include <ary/qualiname.hxx>
    // PARAMETERS



namespace csi
{
namespace uidl
{


class PE_StructElement;
class PE_Type;


class PE_Exception : public UnoIDL_PE
{
  public:
                        PE_Exception();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Exception();
    virtual void        ProcessToken(
                            const Token &       i_rToken );

  private:
    struct S_Work
    {
                            S_Work();

        void                InitData();
        void                Prepare_PE_QualifiedName();
        void                Prepare_PE_Element();
        void                Data_Set_Name(
                                const char *        i_sName );
        // DATA
        String              sData_Name;
        bool                bIsPreDeclaration;
        ary::idl::Ce_id     nCurStruct;

        Dyn<PE_StructElement>
                            pPE_Element;
        ary::idl::Ce_id     nCurParsed_ElementRef;
        Dyn<PE_Type>        pPE_Type;
        ary::idl::Type_id   nCurParsed_Base;
    };

    struct S_Stati;
    class PE_StructState;
    friend struct S_Stati;
    friend class PE_StructState;


    class PE_StructState : public ParseEnvState
    {
      public:

      protected:
                            PE_StructState(
                                PE_Exception &          i_rStruct )
                                                    :   rStruct(i_rStruct) {}
        void                MoveState(
                                ParseEnvState &     i_rState ) const;
        void                SetResult(
                                E_TokenDone         i_eDone,
                                E_EnvStackAction    i_eWhat2DoWithEnvStack,
                                UnoIDL_PE *         i_pParseEnv2Push = 0 ) const
                                                    { rStruct.SetResult(i_eDone, i_eWhat2DoWithEnvStack, i_pParseEnv2Push); }

        S_Stati &           Stati() const           { return *rStruct.pStati; }
        S_Work &            Work() const            { return rStruct.aWork; }
        PE_Exception &      PE() const              { return rStruct; }

      private:
        virtual UnoIDL_PE & MyPE();
        // DATA
        PE_Exception &          rStruct;
    };

    class State_None : public PE_StructState
    {
        public:
                            State_None(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
    };
    class State_WaitForName : public PE_StructState
    {   // -> Name
      public:
                            State_WaitForName(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Identifier(
                                const TokIdentifier &
                                                    i_rToken );
    };
    class State_GotName : public PE_StructState
    {   // -> : { ;
      public:
                            State_GotName(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };
    class State_WaitForBase : public PE_StructState
    {   // -> Base
      public:
                            State_WaitForBase(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        On_SubPE_Left();
    };
    class State_GotBase : public PE_StructState
    {   // -> {
      public:
                            State_GotBase(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };
    class State_WaitForElement : public PE_StructState
    {   // -> Typ }
      public:
                            State_WaitForElement(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Identifier(
                                const TokIdentifier &
                                                i_rToken );
        virtual void        Process_NameSeparator();
        virtual void        Process_BuiltInType(
                                const TokBuiltInType &
                                                i_rToken );
        virtual void        Process_TypeModifier(
                                const TokTypeModifier &
                                                    i_rToken );
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
//      virtual void        On_SubPE_Left();
    };
    class State_WaitForFinish : public PE_StructState
    { // -> ;
      public:
                            State_WaitForFinish(
                                PE_Exception &          i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };

    struct S_Stati
    {
                            S_Stati(
                                PE_Exception &          io_rStruct );
        void                SetState(
                                ParseEnvState &     i_rNextState )
                                                    { pCurStatus = &i_rNextState; }

        State_None          aNone;
        State_WaitForName   aWaitForName;
        State_GotName       aGotName;
        State_WaitForBase   aWaitForBase;
        State_GotBase       aGotBase;
        State_WaitForElement
                            aWaitForElement;
        State_WaitForFinish aWaitForFinish;

        ParseEnvState *     pCurStatus;
    };

    virtual void        InitData();
    virtual void        TransferData();
    virtual void        ReceiveData();

    public:

    void        store_Exception();

    private:

    S_Stati &           Stati()                 { return *pStati; }
    S_Work &            Work()                  { return aWork; }

    // DATA
    S_Work              aWork;
    Dyn<S_Stati>        pStati;
};


inline void
PE_Exception::PE_StructState::MoveState(
                                ParseEnvState &     i_rState ) const
                                                    { rStruct.Stati().SetState(i_rState); }

}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
