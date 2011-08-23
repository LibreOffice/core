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

#ifndef ADC_LUIDL_CX_SUB_HXX
#define ADC_LUIDL_CX_SUB_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcont2.hxx>
    // COMPONENTS
    // PARAMETERS

#include "uidl_tok.hxx"

namespace csi
{
namespace uidl
{

class Token_Receiver;
class Token;


class Cx_Base : public ::TkpContext
{
  public:
    virtual bool		PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
  protected:
    // LIFECYCLE
                        Cx_Base(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	rReceiver(o_rReceiver),
                                                    pFollowUpContext(&i_rFollowUpContext),
                                                    pNewToken()
                                                    {}
  protected:
    void				SetToken(
                            DYN Token *			let_dpToken )
                                                { pNewToken = let_dpToken; }
    Token_Receiver &	Receiver()              { return rReceiver; }

  private:
    // DATA
    Token_Receiver &	rReceiver;
    TkpContext *		pFollowUpContext;
    Dyn<Token>			pNewToken;
};



/**
@descr
*/

class Context_MLComment : public Cx_Base
{
  public:
                        Context_MLComment(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};

class Context_SLComment : public Cx_Base
{
  public:
                        Context_SLComment(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};

class Context_Praeprocessor : public Cx_Base
{
  public:
                        Context_Praeprocessor(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};

class Context_Assignment : public Cx_Base
{
  public:
                        Context_Assignment(
                            Token_Receiver &	o_rReceiver,
                            TkpContext &		i_rFollowUpContext )
                                                : 	Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
};


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
