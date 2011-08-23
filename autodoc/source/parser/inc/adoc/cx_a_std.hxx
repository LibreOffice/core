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

#ifndef ADC_ADOC_CX_A_STD_HXX
#define ADC_ADOC_CX_A_STD_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS

class TextToken;


namespace adoc {

class Cx_LineStart;
class Cx_CheckStar;
class Cx_AtTagCompletion;


/**
@descr
*/
class Context_AdocStd : public autodoc::TkpDocuContext,
                        private StateMachineContext
{
  public:
    // 	LIFECYCLE
                        Context_AdocStd();
    virtual void	  	SetParentContext(
                            TkpContext &		io_rParentContext,
                            const char *		i_sMultiLineEndToken );
                        ~Context_AdocStd();

    //	OPERATIONS
    virtual void	  	AssignDealer(
                            TokenDealer &		o_rDealer );

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    virtual bool		PassNewToken();
    virtual void	   	SetMode_IsMultiLine(
                            bool				i_bTrue );
    //	INQUIRY
    virtual TkpContext &
                        FollowUpContext();
  private:
    //	SERVICE FUNCTIONS
    virtual	void		PerformStatusFunction(
                            uintt				i_nStatusSignal,
                            F_CRTOK		        i_fTokenCreateFunction,
                            CharacterSource &	io_rText );

    void				SetupStateMachine();

    //	DATA
    StateMachine		aStateMachine;
    TokenDealer *		pDealer;

        // Contexts
    TkpContext *    	pParentContext;
    TkpContext *		pFollowUpContext;
    Dyn<Cx_LineStart>	pCx_LineStart;
    Dyn<Cx_CheckStar>	pCx_CheckStar;
    Dyn<Cx_AtTagCompletion>
                        pCx_AtTagCompletion;

        // Temporary data, used during ReadCharChain()
    Dyn<TextToken>		pNewToken;
    bool                bIsMultiline;
};


}   // namespace adoc


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
