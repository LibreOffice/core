/*************************************************************************
 *
 *  $RCSfile: pe_file2.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:07:43 $
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
                            TokenDistributor &  i_rTokenAdmin );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::n22::Repository &  io_rRepository,
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

    E_State             eState;
    uintt               nBracketCount_inDefMode;
};


}   // namespace uidl
}   // namespace csi

#endif

