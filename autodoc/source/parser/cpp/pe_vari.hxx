/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
        expectFinish,		        // after InitExpression
        size_of_states
    };
                        PE_Variable(
                            Cpp_PE *			i_pParent );
                        ~PE_Variable();

    /** @return
        Bit 0x0001 != 0, if there is a size and
        bit 0x0002 != 0, if there is an initialisation.
    */
    UINT16              Result_Pattern() const;
    const String  &     Result_SizeExpression() const;
    const String  &	    Result_InitExpression() const;

    virtual void		Call_Handler(
                            const cpp::Token &	i_rTok );

  private:
    typedef SubPe< PE_Variable, PE_Expression >		 	SP_Expression;
    typedef SubPeUse< PE_Variable, PE_Expression> 		SPU_Expression;

    void				Setup_StatusFunctions();
    virtual void		InitData();
    virtual void		TransferData();
    void				Hdl_SyntaxError(const char *);

    void                SpReturn_ArraySizeExpression();
    void                SpReturn_InitExpression();

    void				On_afterName_ArrayBracket_Left(const char * i_sText);
    void				On_afterName_Semicolon(const char * i_sText);
    void				On_afterName_Comma(const char * i_sText);
    void				On_afterName_Assign(const char * i_sText);

    void				On_afterSize_ArrayBracket_Right(const char * i_sText);

    void				On_expectFinish_Semicolon(const char * i_sText);
    void				On_expectFinish_Comma(const char * i_sText);
    void				On_expectFinish_Bracket_Right(const char * i_sText);

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
