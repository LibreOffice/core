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

#ifndef ADC_CPP_PE_TYDEF_HXX
#define ADC_CPP_PE_TYDEF_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>


namespace cpp
{
    class PE_Type;




class PE_Typedef : public cpp::Cpp_PE
{
  public:
    enum E_State
    {
        start,
        expectName,
        afterName,
        size_of_states
    };
                        PE_Typedef(
                            Cpp_PE *		i_pParent );
                        ~PE_Typedef();

    virtual void		Call_Handler(
                            const cpp::Token &	i_rTok );
  private:
    typedef SubPe< PE_Typedef, PE_Type >		SP_Type;
    typedef SubPeUse< PE_Typedef, PE_Type> 	    SPU_Type;

    void				Setup_StatusFunctions();
    virtual void		InitData();
    virtual void		TransferData();
    void  				Hdl_SyntaxError( const char *);

    void				SpReturn_Type();

    void				On_start_typedef( const char * );
    void				On_expectName_Identifier( const char * );
    void				On_afterName_Semicolon( const char * );

    // DATA
    Dyn< PeStatusArray<PE_Typedef> >
                        pStati;
    Dyn<SP_Type>		pSpType;
    Dyn<SPU_Type>		pSpuType;

    String              sName;
    ary::cpp::Type_id	nType;
};




}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
