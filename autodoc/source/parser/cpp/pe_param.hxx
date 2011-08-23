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


#ifndef ADC_CPP_PE_PARAM_HXX
#define ADC_CPP_PE_PARAM_HXX

// BASE CLASSES
#include "cpp_pe.hxx"
// USED SERVICES
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_vfflag.hxx>




namespace cpp
{
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
    typedef ary::cpp::S_Parameter       S_ParamInfo;

    explicit			PE_Parameter(
                            Cpp_PE *			i_pParent );
                        ~PE_Parameter();

    virtual void		Call_Handler(
                            const cpp::Token &	i_rTok );

    ary::cpp::Type_id   Result_FrontType() const;
    const S_ParamInfo & Result_ParamInfo() const;

  private:
    typedef SubPe< PE_Parameter, PE_Type >		    SP_Type;
    typedef SubPeUse< PE_Parameter, PE_Type >		SPU_Type;
    typedef SubPe< PE_Parameter, PE_Variable >	    SP_Variable;
    typedef SubPeUse< PE_Parameter, PE_Variable >	SPU_Variable;

    void				Setup_StatusFunctions();
    virtual void		InitData();
    virtual void		TransferData();
    void  				Hdl_SyntaxError( const char *);

    void				SpInit_Type();       // Type and Ignore.
    void				SpInit_Variable();
    void				SpReturn_Type();
    void				SpReturn_Variable();

    void				On_start_Type(const char * i_sText);
    void				On_start_Bracket_Right(const char * i_sText);
    void                On_start_Ellipse(const char * i_sText);

    void				On_expectName_Identifier(const char * i_sText);
    void				On_expectName_ArrayBracket_Left(const char * i_sText);
    void				On_expectName_Bracket_Right(const char * i_sText);
    void				On_expectName_Comma(const char * i_sText);

    void				On_afterName_ArrayBracket_Left(const char * i_sText);
    void				On_afterName_Bracket_Right(const char * i_sText);
    void				On_afterName_Comma(const char * i_sText);
    void				On_afterName_Assign(const char * i_sText);

    void				On_finished_Bracket_Right(const char * i_sText);
    void				On_finished_Comma(const char * i_sText);

    // DATA
    Dyn< PeStatusArray<PE_Parameter> >
                        pStati;

    Dyn<SP_Type>	    pSpType;
    Dyn<SPU_Type>	 	pSpuType;
    Dyn<SP_Variable>	pSpVariable;
    Dyn<SPU_Variable>	pSpuVariable;

    S_ParamInfo         aResultParamInfo;
};




// IMPLEMENTATION
inline ary::cpp::Type_id
PE_Parameter::Result_FrontType() const
{
    return aResultParamInfo.nType;
}

inline const PE_Parameter::S_ParamInfo &
PE_Parameter::Result_ParamInfo() const
{
    return aResultParamInfo;
}




}   // namespace cpp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
