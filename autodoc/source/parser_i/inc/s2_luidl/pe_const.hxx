/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_const.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:14:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef LUIDL_PE_CONST_HXX
#define LUIDL_PE_CONST_HXX


// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS


namespace udm {
class Agent_Struct;
}   // namespace udm


namespace csi
{
namespace uidl
{

class ConstantsGroup;

class PE_Type;
class PE_Value;

class PE_Constant : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
                        PE_Constant();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &
                                                io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Constant();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );

  private:
    enum E_State
    {
        e_none,
        expect_name,
        expect_curl_bracket_open,
        expect_const,
        expect_value,
        expect_finish,
        e_STATES_MAX
    };
    enum E_TokenType
    {
        tt_stereotype,
        tt_identifier,
        tt_punctuation,
        tt_MAX
    };
    typedef void (PE_Constant::*F_TOK)(const char *);


    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    void                On_expect_name_Identifier(const char * i_sText);
    void                On_expect_curl_bracket_open_Punctuation(const char * i_sText);
    void                On_expect_const_Stereotype(const char * i_sText);
    void                On_expect_const_Punctuation(const char * i_sText);
    void                On_expect_value_Identifier(const char * i_sText);
    void                On_expect_finish_Punctuation(const char * i_sText);
    void                On_Default(const char * );

    void                EmptySingleConstData();
    void                CreateSingleConstant();

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

  // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;

    String              sData_Name;
    ary::idl::Ce_id     nDataId;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nType;

    Dyn<PE_Value>       pPE_Value;
    String              sName;
    String              sAssignment;
};



}   // namespace uidl
}   // namespace csi


#endif

