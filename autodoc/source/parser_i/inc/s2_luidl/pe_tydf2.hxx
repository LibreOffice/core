/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_tydf2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:18:00 $
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

#ifndef LUIDL_PE_TYDF2_HXX
#define LUIDL_PE_TYDF2_HXX



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

class PE_Type;


class PE_Typedef : public UnoIDL_PE,
                   public ParseEnvState
{
  public:
                        PE_Typedef();
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_Typedef();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none = 0,
        expect_description,
        expect_name,
        got_name,
        e_STATES_MAX
    };
    enum E_TokenType    /// @ATTENTION  Do not change existing values (except of tt_MAX) !!! Else array-indices will break.
    {
        tt_any = 0,
        tt_identifier,
        tt_punctuation,
        tt_MAX
    };
    typedef void (PE_Typedef::*F_TOK)(const char *);


    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    void                On_expect_description_Any(const char * i_sText);
    void                On_expect_name_Identifier(const char * i_sText);
    void                On_got_name_Punctuation(const char * i_sText);
    void                On_Default(const char * );

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;
    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nType;
    String              sName;
};



}   // namespace uidl
}   // namespace csi



#endif

