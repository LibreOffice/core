/*************************************************************************
 *
 *  $RCSfile: pe_iface.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:45:05 $
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

#ifndef ADC_UIDL_PE_IFACE_HXX
#define ADC_UIDL_PE_IFACE_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace idl
{
    class Interface;
}
}

namespace csi
{
namespace uidl
{



class PE_Function;
class PE_Attribute;
class PE_Type;

class PE_Interface : public UnoIDL_PE,
                     public ParseEnvState
{
  public:
                        PE_Interface();
    virtual             ~PE_Interface();

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::n22::Repository &  io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_NameSeparator();
    virtual void        Process_BuiltInType(
                            const TokBuiltInType &
                                                i_rToken );
    virtual void        Process_TypeModifier(
                            const TokTypeModifier &
                                                i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State        /// @ATTENTION  Do not change existing values (except of e_STATES_MAX) !!! Else array-indices will break.
    {
        e_none = 0,
        need_uik,
        uik,
        need_ident,
        ident,
        need_interface,
        need_name,
        wait_for_base,
        in_base,            // in header, after ":"
        need_curlbr_open,
        e_std,
        in_function,
        in_attribute,
        need_finish,
        in_base_interface,  // in body, after "interface"
        e_STATES_MAX
    };
    enum E_TokenType    /// @ATTENTION  Do not change existing values (except of tt_MAX) !!! Else array-indices will break.
    {
        tt_metatype = 0,
        tt_identifier = 1,
        tt_punctuation = 2,
        tt_startoftype = 3,
        tt_stereotype = 4,
        tt_MAX
    };
    typedef void (PE_Interface::*F_TOK)(const char *);


    void                On_need_uik_MetaType(const char * i_sText);
    void                On_uik_Identifier(const char * i_sText);
    void                On_uik_Punctuation(const char * i_sText);
    void                On_need_ident_MetaType(const char * i_sText);
    void                On_ident_Identifier(const char * i_sText);
    void                On_ident_Punctuation(const char * i_sText);
    void                On_need_interface_MetaType(const char * i_sText);
    void                On_need_name_Identifer(const char * i_sText);
    void                On_wait_for_base_Punctuation(const char * i_sText);
    void                On_need_curlbr_open_Punctuation(const char * i_sText);
    void                On_std_Metatype(const char * i_sText);
    void                On_std_Punctuation(const char * i_sText);
    void                On_std_Stereotype(const char * i_sText);
    void                On_std_GotoFunction(const char * i_sText);
    void                On_std_GotoAttribute(const char * i_sText);
    void                On_std_GotoBaseInterface(const char * i_sText);
    void                On_need_finish_Punctuation(const char * i_sText);
    void                On_Default(const char * i_sText);

    void                CallHandler(
                            const char *        i_sTokenText,
                            E_TokenType         i_eTokenType );

    virtual void        InitData();
    virtual void        TransferData();
    virtual void        ReceiveData();
    virtual UnoIDL_PE & MyPE();

    void                store_Interface();

    // DATA
    static F_TOK        aDispatcher[e_STATES_MAX][tt_MAX];

    E_State             eState;
    String              sData_Name;
    bool                bIsPreDeclaration;
    ary::idl::Interface *
                        pCurInterface;
    ary::idl::Ce_id     nCurInterface;

    Dyn<PE_Function>    pPE_Function;
    Dyn<PE_Attribute>   pPE_Attribute;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nCurParsed_Base;
    bool                bOptional;
};



// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi


#endif

