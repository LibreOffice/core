/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pe_vari.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:59:32 $
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

#ifndef ADC_CPP_PE_VARI_HXX
#define ADC_CPP_PE_VARI_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_vfflag.hxx>
    // PARAMETERS


namespace cpp {

class PE_Expression;


class PE_Variable : public Cpp_PE
{
  public:
    enum E_State
    {
        afterName,                  //
        afterSize,                  // after ]
        expectFinish,               // after InitExpression
        size_of_states
    };
                        PE_Variable(
                            Cpp_PE *            i_pParent );
                        ~PE_Variable();

    /** @return
        Bit 0x0001 != 0, if there is a size and
        bit 0x0002 != 0, if there is an initialisation.
    */
    UINT16              Result_Pattern() const;
    const String  &     Result_SizeExpression() const;
    const String  &     Result_InitExpression() const;

    virtual void        Call_Handler(
                            const cpp::Token &  i_rTok );

  private:
    typedef SubPe< PE_Variable, PE_Expression >         SP_Expression;
    typedef SubPeUse< PE_Variable, PE_Expression>       SPU_Expression;

    void                Setup_StatusFunctions();
    virtual void        InitData();
    virtual void        TransferData();
    void                Hdl_SyntaxError(const char *);

    void                SpReturn_ArraySizeExpression();
    void                SpReturn_InitExpression();

    void                On_afterName_ArrayBracket_Left(const char * i_sText);
    void                On_afterName_Semicolon(const char * i_sText);
    void                On_afterName_Comma(const char * i_sText);
    void                On_afterName_Assign(const char * i_sText);

    void                On_afterSize_ArrayBracket_Right(const char * i_sText);

    void                On_expectFinish_Semicolon(const char * i_sText);
    void                On_expectFinish_Comma(const char * i_sText);
    void                On_expectFinish_Bracket_Right(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Variable> >
                        pStati;

    Dyn<SP_Expression>  pSpExpression;
    Dyn<SPU_Expression> pSpuArraySizeExpression;
    Dyn<SPU_Expression> pSpuInitExpression;

    String              sResultSizeExpression;
    String              sResultInitExpression;
};



// IMPLEMENTATION


inline UINT16
PE_Variable::Result_Pattern() const
    { return ( sResultSizeExpression.length() > 0 ? 1 : 0 )
             + ( sResultInitExpression.length() > 0 ? 2 : 0 ); }
inline const String  &
PE_Variable::Result_SizeExpression() const
    { return sResultSizeExpression; }
inline const String  &
PE_Variable::Result_InitExpression() const
    { return sResultInitExpression; }


}   // namespace cpp


#endif

