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

#ifndef ADC_TKPCONT2_HXX
#define ADC_TKPCONT2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
class CharacterSource;
class TkpNullContext;
class TkpNullContex2;

/**	@task
    Specifies a context within which tokens are interpreted in a special
    way. For example in parsing C++ there could be a context for code,
    one for comments and a third one for preprocessor statements, because
    each of these would give the same token different meanings.
**/
class TkpContext
{
  public:
    // LIFECYCLE
    virtual					~TkpContext() {}

    // OPERATIONS
    /** @descr
        The functions starts to parse with the CurToken() of io_rText.
        It leaves io_rText at the first char of the following Token or
        the following Context.

        This function returns, when a context has parsed some characterss
        and completed a token OR left the context.
        If the token is to be ignored, PassNewToken() returns false
        and cuts the token from io_rText.
        If the token is to be parsed further in a different context,
        PassNewToken() returns false, but the token is
        NOT cut from io_rText.

        If the function has found a valid and complete token, PassNewToken()
        passes the parsed token to the internally known receiver and
        returns true. The token is cut from io_rText.
    **/
    virtual void		ReadCharChain(
                            CharacterSource &	io_rText ) = 0;
    /** Has to pass the parsed token to a known receiver.
        @return true, if a token was passed.
                false, if no token was parsed complete by this context.
    */
    virtual bool		PassNewToken() = 0;
    virtual TkpContext &
                        FollowUpContext() = 0;

    static TkpNullContext &
                        Null_();
};

TkpNullContex2 &    TkpContext_Null2_();

class StateMachineContext
{
  public:
    virtual ~StateMachineContext() {}

    /// Is used by StmBoundsStatu2 only.
    virtual	void		PerformStatusFunction(
                            uintt				i_nStatusSignal,
                            UINT16				i_nTokenId,
                            CharacterSource &	io_rText ) = 0;
};

class TkpNullContex2 : public TkpContext
{
  public:
                        ~TkpNullContex2();

    virtual void		ReadCharChain(
                            CharacterSource &	io_rText );
    virtual bool		PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
};

class TkpDocuContext : public TkpContext
{
  public:
    virtual void	  	SetParentContext(
                            TkpContext &		io_rParentContext,
                            const char *		i_sMultiLineEndToken ) = 0;
    virtual void	   	SetMode_IsMultiLine(
                            bool				i_bTrue ) = 0;
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
