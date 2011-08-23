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

#ifndef ADC_CPP_CX_C_PP_HXX
#define ADC_CPP_CX_C_PP_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace cpp
{

class Context_Preprocessor : public Cx_Base
{
  public:
                        Context_Preprocessor(
                            TkpContext &		i_rFollowUpContext );
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  private:
    // Locals
    void                ReadDefault(
                            CharacterSource &	io_rText );
    void                ReadDefine(
                            CharacterSource &	io_rText );

    // DATA
    TkpContext *   	    pContext_Parent;
    Dyn<Cx_Base>    	pContext_PP_MacroParams;
    Dyn<Cx_Base>       	pContext_PP_Definition;
};

class Context_PP_MacroParams : public Cx_Base
{
  public:
                        Context_PP_MacroParams(
                            Cx_Base &           i_rFollowUpContext );

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
  private:
    // DATA
    Cx_Base *   	    pContext_PP_Definition;
};

class Context_PP_Definition : public Cx_Base
{
  public:
                        Context_PP_Definition(
                            TkpContext &	    i_rFollowUpContext );

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );

  private:
    // DATA
    TkpContext *       	pContext_Parent;
};


}   // namespace cpp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
