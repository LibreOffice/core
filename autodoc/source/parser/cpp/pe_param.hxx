/*************************************************************************
 *
 *  $RCSfile: pe_param.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:31 $
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


#ifndef ADC_CPP_PE_PARAM_HXX
#define ADC_CPP_PE_PARAM_HXX


// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS
#include <ary/cpp/c_vfflag.hxx>


namespace cpp {

class PE_Type;
class PE_Variable;


class PE_Parameter : public Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expectName,
        afterName,
        finished,
        size_of_states
    };
    typedef ary::cpp::S_Parameter   S_ParamInfo;

                        PE_Parameter(
                            Cpp_PE *            i_pParent );
                        ~PE_Parameter();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

    ary::Tid            Result_FrontType() const;
    const S_ParamInfo &
                        Result_ParamInfo() const;

  private:
    typedef SubPe< PE_Parameter, PE_Type >          SP_Type;
    typedef SubPeUse< PE_Parameter, PE_Type >       SPU_Type;
    typedef SubPe< PE_Parameter, PE_Variable >      SP_Variable;
    typedef SubPeUse< PE_Parameter, PE_Variable >   SPU_Variable;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError( const char *);

    void                SpInit_Type();       // Type and Ignore.
    void                SpInit_Variable();
    void                SpReturn_Type();
    void                SpReturn_Variable();

    void                On_start_Type(const char * i_sText);
    void                On_start_Bracket_Right(const char * i_sText);
    void                On_start_Ellipse(const char * i_sText);

    void                On_expectName_Identifier(const char * i_sText);
    void                On_expectName_ArrayBracket_Left(const char * i_sText);
    void                On_expectName_Bracket_Right(const char * i_sText);
    void                On_expectName_Comma(const char * i_sText);
    void                On_expectName_Assign(const char * i_sText);

    void                On_afterName_ArrayBracket_Left(const char * i_sText);
    void                On_afterName_Bracket_Right(const char * i_sText);
    void                On_afterName_Comma(const char * i_sText);
    void                On_afterName_Assign(const char * i_sText);

    void                On_finished_Bracket_Right(const char * i_sText);
    void                On_finished_Comma(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Parameter> >
                        pStati;

    Dyn<SP_Type>        pSpType;
    Dyn<SPU_Type>       pSpuType;
    Dyn<SP_Variable>    pSpVariable;
    Dyn<SPU_Variable>   pSpuVariable;

    S_ParamInfo         aResultParamInfo;
};



// IMPLEMENTATION

inline ary::Tid
PE_Parameter::Result_FrontType() const
    { return aResultParamInfo.nType; }
inline const PE_Parameter::S_ParamInfo &
PE_Parameter::Result_ParamInfo() const
    { return aResultParamInfo; }



}   // namespace cpp




#endif



