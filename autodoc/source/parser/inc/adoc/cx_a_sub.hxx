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

#ifndef ADC_ADOC_CX_A_SUB_HXX
#define ADC_ADOC_CX_A_SUB_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS
#include <tokens/token.hxx>


namespace adoc {




class Cx_LineStart : public TkpContext
{
  public:
                        Cx_LineStart(
                            TkpContext &		i_rFollowUpContext );

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    virtual bool		PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    void				SetCurToken(
                            TextToken::F_CRTOK	i_fTokenCreateFunction )
                                                { fCur_TokenCreateFunction = i_fTokenCreateFunction; }
    void                AssignDealer(
                            TokenDealer &	    o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *		pDealer;
    TkpContext *		pFollowUpContext;

    Dyn<TextToken>		pNewToken;

    TextToken::F_CRTOK	fCur_TokenCreateFunction;
};


/**
@descr
*/

class Cx_CheckStar : public TkpContext
{
  public:
    // LIFECYCLE
                        Cx_CheckStar(
                            TkpContext &		i_rFollowUpContext );
    void				Set_End_FollowUpContext(
                            TkpContext &		i_rEnd_FollowUpContext )
                                                { pEnd_FollowUpContext = &i_rEnd_FollowUpContext; }

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    virtual bool		PassNewToken();

    void				SetCanBeEnd(
                            bool 				i_bCanBeEnd  )
                                                { bCanBeEnd = i_bCanBeEnd; }
    virtual TkpContext &
                        FollowUpContext();
    void                AssignDealer(
                            TokenDealer &	    o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *		pDealer;
    TkpContext *		pFollowUpContext;
    TkpContext *		pEnd_FollowUpContext;

    Dyn<TextToken>		pNewToken;

    bool				bCanBeEnd;
    bool				bEndTokenFound;
};


class Cx_AtTagCompletion : public TkpContext
{
  public:
                        Cx_AtTagCompletion(
                            TkpContext &		i_rFollowUpContext );

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    virtual bool		PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    void				SetCurToken(
                            TextToken::F_CRTOK	i_fTokenCreateFunction )
                                                { fCur_TokenCreateFunction = i_fTokenCreateFunction; }
    void                AssignDealer(
                            TokenDealer &	    o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *		pDealer;
    TkpContext *		pFollowUpContext;

    Dyn<TextToken>		pNewToken;

    TextToken::F_CRTOK	fCur_TokenCreateFunction;
};


}   // namespace adoc


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
