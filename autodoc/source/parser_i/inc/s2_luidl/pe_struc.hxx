/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef LUIDL_PE_STRUC_HXX
#define LUIDL_PE_STRUC_HXX



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
namespace prl
{
    class TNamespace;
}
}



namespace csi
{
namespace uidl
{


class Struct;
class StructElement;
class PE_StructElement;
class PE_Type;


class PE_Struct : public UnoIDL_PE
{
  public:
                        PE_Struct();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Struct();
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
        void                Data_Set_TemplateParam(
                                const char *        i_sTemplateParam );

        String              sData_Name;
        String              sData_TemplateParam;
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
                                PE_Struct &         i_rStruct )
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
        PE_Struct &         PE() const              { return rStruct; }

      private:
        virtual UnoIDL_PE & MyPE();
        // DATA
        PE_Struct &         rStruct;
    };

    class State_None : public PE_StructState
    {
        public:
                            State_None(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
    };
    class State_WaitForName : public PE_StructState
    {   // -> Name
      public:
                            State_WaitForName(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Identifier(
                                const TokIdentifier &
                                                    i_rToken );
    };
    class State_GotName : public PE_StructState
    {   // -> : { ; <
      public:
                            State_GotName(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };
    class State_WaitForTemplateParam : public PE_StructState
    {   // -> Template parameter identifier
      public:
                            State_WaitForTemplateParam(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Identifier(
                                const TokIdentifier &
                                                    i_rToken );
    };
    class State_WaitForTemplateEnd : public PE_StructState
    {   // -> >
      public:
                            State_WaitForTemplateEnd(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };
    class State_WaitForBase : public PE_StructState
    {   // -> Base
      public:
                            State_WaitForBase(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        On_SubPE_Left();
    };
    class State_GotBase : public PE_StructState
    {   // -> {
      public:
                            State_GotBase(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };
    class State_WaitForElement : public PE_StructState
    {   // -> Typ }
      public:
                            State_WaitForElement(
                                PE_Struct &         i_rStruct )
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
    };
    class State_WaitForFinish : public PE_StructState
    { // -> ;
      public:
                            State_WaitForFinish(
                                PE_Struct &         i_rStruct )
                                                    :   PE_StructState(i_rStruct) {}
        virtual void        Process_Punctuation(
                                const TokPunctuation &
                                                    i_rToken );
    };

    struct S_Stati
    {
                            S_Stati(
                                PE_Struct &         io_rStruct );
        void                SetState(
                                ParseEnvState &     i_rNextState )
                                                    { pCurStatus = &i_rNextState; }

        State_None          aNone;
        State_WaitForName   aWaitForName;
        State_GotName       aGotName;
        State_WaitForTemplateParam
                            aWaitForTemplateParam;
        State_WaitForTemplateEnd
                            aWaitForTemplateEnd;
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

    void        store_Struct();

    private:

    S_Stati &           Stati()                 { return *pStati; }
    S_Work &            Work()                  { return aWork; }

    // DATA
    S_Work              aWork;
    Dyn<S_Stati>        pStati;
};


inline void
PE_Struct::PE_StructState::MoveState(
                                ParseEnvState &     i_rState ) const
                                                    { rStruct.Stati().SetState(i_rState); }

}   // namespace uidl
}   // namespace csi


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
