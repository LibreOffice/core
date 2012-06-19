/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ADC_TKPCONT2_HXX
#define ADC_TKPCONT2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
class CharacterSource;
class TkpNullContext;
class TkpNullContex2;

/** @task
    Specifies a context within which tokens are interpreted in a special
    way. For example in parsing C++ there could be a context for code,
    one for comments and a third one for preprocessor statements, because
    each of these would give the same token different meanings.
**/
class TkpContext
{
  public:
    // LIFECYCLE
    virtual                 ~TkpContext() {}

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
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText ) = 0;
    /** Has to pass the parsed token to a known receiver.
        @return true, if a token was passed.
                false, if no token was parsed complete by this context.
    */
    virtual bool        PassNewToken() = 0;
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
    virtual void        PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            UINT16              i_nTokenId,
                            CharacterSource &   io_rText ) = 0;
};

class TkpNullContex2 : public TkpContext
{
  public:
                        ~TkpNullContex2();

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
};

class TkpDocuContext : public TkpContext
{
  public:
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken ) = 0;
    virtual void        SetMode_IsMultiLine(
                            bool                i_bTrue ) = 0;
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
