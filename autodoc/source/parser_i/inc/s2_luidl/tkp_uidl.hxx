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

#pragma once
#if 1

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkp2.hxx>
    // COMPONENTS
    // PARAMETRS

class TkpDocuContext;


namespace csi
{
namespace uidl
{



class Token_Receiver;
class Context_UidlCode;


/** This is a TokenParser which is able to parse tokens from
    C++ source code.
*/
class TokenParser_Uidl : public TokenParse2
{
  public:
    // LIFECYCLE
                        TokenParser_Uidl(
                            Token_Receiver &    o_rUidlReceiver,
                            DYN TkpDocuContext &
                                                let_drDocuContext );
    virtual             ~TokenParser_Uidl();

    // OPERATIONS
  private:
    virtual ::TkpContext &
                        CurrentContext();

    virtual void        SetStartContext();
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext );
    // DATA
    Dyn<Context_UidlCode>
                        pBaseContext;
    ::TkpContext *      pCurContext;
};


}   // namespace uidl
}   // namespace csi

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
