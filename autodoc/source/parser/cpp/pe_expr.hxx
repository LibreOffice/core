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


#ifndef ADC_CPP_PE_EXPR_HXX
#define ADC_CPP_PE_EXPR_HXX



// USED SERVICES
    // BASE CLASSES
#include "cpp_pe.hxx"
    // COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
    // PARAMETERS


namespace cpp {


class PE_Expression : public Cpp_PE
{
  public:
    enum E_State
    {
        std,
        size_of_states
    };
                        PE_Expression(
                            Cpp_PE *			i_pParent );
                        ~PE_Expression();

    const char *        Result_Text() const;

    virtual void		Call_Handler(
                            const cpp::Token &	i_rTok );

  private:
    void				Setup_StatusFunctions();
    virtual void		InitData();
    virtual void		TransferData();
    void  				On_std_Default( const char *);

    void                On_std_SwBracket_Left( const char *);
    void                On_std_SwBracket_Right( const char *);
    void                On_std_ArrayBracket_Left( const char *);
    void                On_std_ArrayBracket_Right( const char *);
    void                On_std_Bracket_Left( const char *);
    void                On_std_Bracket_Right( const char *);
    void                On_std_Semicolon( const char *);
    void                On_std_Comma( const char *);

    // DATA
    Dyn< PeStatusArray<PE_Expression> >
                        pStati;

    csv::StreamStr      aResult_Text;

    intt                nBracketCounter;
};



// IMPLEMENTATION

inline const char *
PE_Expression::Result_Text() const
{
    return aResult_Text.c_str();
}


}   // namespace cpp




#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
