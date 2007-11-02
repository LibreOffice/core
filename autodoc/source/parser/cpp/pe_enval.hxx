/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_enval.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:54:39 $
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

#ifndef ADC_CPP_PE_ENVAL_HXX
#define ADC_CPP_PE_ENVAL_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp {

class PE_Expression;


class PE_EnumValue : public Cpp_PE
{
  public:
    enum E_State
    {
        start,                      // before name
        afterName,
        expectFinish,               // after init-expression
        size_of_states
    };
                        PE_EnumValue(
                            Cpp_PE *            i_pParent );
                        ~PE_EnumValue();

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    typedef SubPe< PE_EnumValue, PE_Expression >        SP_Expression;
    typedef SubPeUse< PE_EnumValue, PE_Expression>      SPU_Expression;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError(const char *);

    void                SpReturn_InitExpression();

    void                On_start_Identifier(const char * );

    void                On_afterName_SwBracket_Right(const char * );
    void                On_afterName_Comma(const char * );
    void                On_afterName_Assign(const char * );

    void                On_expectFinish_SwBracket_Right(const char * );
    void                On_expectFinish_Comma(const char * );

    // DATA
    Dyn< PeStatusArray<PE_EnumValue> >
                        pStati;
    Dyn<SP_Expression>  pSpExpression;
    Dyn<SPU_Expression> pSpuInitExpression;

    String              sName;
    String              sInitExpression;
};




}   // namespace cpp
#endif

