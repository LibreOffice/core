/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_file2.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:15:45 $
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

#ifndef LUIDL_PE_FILE2_HXX
#define LUIDL_PE_FILE2_HXX



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
class Module;
}   // namespace idl
}   // namespace ary


namespace csi
{
namespace uidl
{

class TokenDistributor;
class PE_Service;
class PE_Singleton;
class PE_Interface;
class PE_Struct;
class PE_Exception;
class PE_Constant;
class PE_Enum;
class PE_Typedef;


class PE_File : public UnoIDL_PE,
                public ParseEnvState
{
  public:
                        PE_File(
                            TokenDistributor &  i_rTokenAdmin,
                            const ParserInfo &  i_parseInfo );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_File();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_MetaType(
                            const TokMetaType & i_rToken );
    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none,
        e_std,
        wait_for_module,
        wait_for_module_bracket,
        wait_for_module_semicolon,
        in_sub_pe,
        on_default
    };

    virtual void        InitData();
    virtual void        TransferData();
    virtual void        ReceiveData();
    virtual UnoIDL_PE & MyPE();
    virtual const ary::idl::Module &
                        CurNamespace() const;
    virtual const ParserInfo &
                        ParseInfo() const;
    // DATA
    TokenDistributor *  pTokenAdmin;
    Dyn<PE_Service>     pPE_Service;
    Dyn<PE_Singleton>   pPE_Singleton;
    Dyn<PE_Interface>   pPE_Interface;
    Dyn<PE_Struct>      pPE_Struct;
    Dyn<PE_Exception>   pPE_Exception;
    Dyn<PE_Constant>    pPE_Constant;
    Dyn<PE_Enum>        pPE_Enum;
    Dyn<PE_Typedef>     pPE_Typedef;

    const ary::idl::Module *
                        pCurNamespace;
    const ParserInfo *  pParseInfo;

    E_State             eState;
    uintt               nBracketCount_inDefMode;
};


}   // namespace uidl
}   // namespace csi

#endif

