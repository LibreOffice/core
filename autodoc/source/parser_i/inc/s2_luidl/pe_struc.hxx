/*************************************************************************
 *
 *  $RCSfile: pe_struc.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <csi/prl/quname2.hxx>
    // PARAMETERS
#include <csi/prl/tsk_type.hxx>



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
        void                Data_Add_CurParsed_ElementRef();

        Struct *            pData;
        bool                bIsPreDeclaration;
        udm::IRef< Struct > pCurStruct;

        Dyn<PE_StructElement>
                            pPE_Element;
        udm::IRef< StructElement >
                            aCurParsed_ElementRef;
        Dyn<PE_Type>        pPE_Type;
        csi::prl::RefType   aCurParsed_Base;
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
    {   // -> : { ;
      public:
                            State_GotName(
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
        virtual void        On_SubPE_Left();
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

    friend class PE_StructState;

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

