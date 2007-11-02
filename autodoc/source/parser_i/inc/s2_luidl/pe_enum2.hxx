/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_enum2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:14:44 $
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

#ifndef ADC_UIDL_PE_ENUM2_HXX
#define ADC_UIDL_PE_ENUM2_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS



namespace csi
{
namespace uidl
{

// class Enum;

class PE_Value;

class PE_Enum : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
                        PE_Enum();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Enum();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );

  private:
    enum E_State
    {
        e_none,
        expect_name,
        expect_curl_bracket_open,
        expect_value,
        expect_finish,
        e_STATES_MAX
    };
    enum E_TokenType
    {
        tt_identifier,
        tt_punctuation,
        tt_MAX
    };
    typedef void (PE_Enum::*F_TOK)(const char *);


    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    void                On_expect_name_Identifier(const char * i_sText);
    void                On_expect_curl_bracket_open_Punctuation(const char * i_sText);
    void                On_expect_value_Punctuation(const char * i_sText);
    void                On_expect_value_Identifier(const char * i_sText);
    void                On_expect_finish_Punctuation(const char * i_sText);
    void                On_Default(const char * );

    void                EmptySingleValueData();
    void                CreateSingleValue();

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

  // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;

    String              sData_Name;
    ary::idl::Ce_id     nDataId;

    Dyn<PE_Value>       pPE_Value;
    String              sName;
    String              sAssignment;
};



}   // namespace uidl
}   // namespace csi


#endif

