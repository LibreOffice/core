/*************************************************************************
 *
 *  $RCSfile: pe_func2.hxx,v $
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

#ifndef ADC_UIDL_PE_FUNC2_HXX
#define ADC_UIDL_PE_FUNC2_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
#include <csi/l_uidl/param.hxx>
    // PARAMETERS
#include <csi/prl/tsk_type.hxx>
#include <ary_i/uidl/gate.hxx>


namespace csi
{
namespace uidl
{

class Interface;

class PE_Type;
class PE_Variable;

class PE_Function : public UnoIDL_PE,
                    public ParseEnvState
{
  public:
    typedef ary::uidl::Gate::RInterface     RInterface;
    typedef ary::uidl::Gate::RFunction      RFunction;

                        PE_Function(
                            RFunction &         o_rResult,
                            const RInterface &  i_rCurInterface );

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
    virtual             ~PE_Function();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Stereotype(
                            const TokStereotype &
                                                i_rToken );
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );
    virtual void        Process_ParameterHandling(
                            const TokParameterHandling &
                                                i_rToken );
    virtual void        Process_Raises();
    virtual void        Process_Default();

  private:
    enum E_State
    {
        e_none,
        e_start,
        in_return_type,
        expect_name,
        expect_params_list,
        expect_parameter,
        expect_parameter_variable,
        in_parameter_variable,
        expect_parameter_separator,
        params_finished,
        expect_exceptions_list,
        expect_exception,
        in_exception,
        expect_exception_separator,
        exceptions_finished
    };

    void                GoIntoReturnType();
    void                GoIntoParameterVariable();
    void                GoIntoException();
    void                OnDefault();

    virtual void        InitData();
    virtual void        ReceiveData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    E_State             eState;
    Function *          pData;
    RFunction *         pResult;
    const RInterface *  pCurInterface;

    Dyn<PE_Type>        pPE_Type;
    csi::prl::RefType   aCurParsedType;     // ReturnType or Exception

    udmstri             sName;

    Dyn<PE_Variable>    pPE_Variable;
    E_ParameterDirection
                        eCurParsedParam_Direction;
    csi::prl::RefType   aCurParsedParam_Type;
    udmstri             sCurParsedParam_Name;
};



}   // namespace uidl
}   // namespace csi


#endif

