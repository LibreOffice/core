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

#ifndef ADC_UIDL_TK_PUNCT_HXX
#define ADC_UIDL_TK_PUNCT_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/uidl_tok.hxx>
    // COMPONENTS
#include <luxenum.hxx>
    // PARAMETERS


namespace csi
{
namespace uidl
{


class TokPunctuation : public Token
{
  public:
    // TYPES
    enum E_TokenId
    {
        e_none = 0,
        BracketOpen =  1,           // (
        BracketClose = 2,           // )
        ArrayBracketOpen = 3,       // [
        ArrayBracketClose = 4,      // ]
        CurledBracketOpen = 5,      // {
        CurledBracketClose = 6,     // }
        Semicolon = 7,              // ;
        Colon = 8,                  // :
        DoubleColon = 9,            // ::
        Comma = 10,                 // ,
        Minus = 11,                 // -
        Fullstop = 12,              // .
        Lesser = 13,                // <
        Greater = 14                // >
    };
    typedef lux::Enum<E_TokenId> EV_TokenId;


                        TokPunctuation(
                            EV_TokenId          i_eTag )
                                                :   eTag(i_eTag) {}
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
    EV_TokenId          Id() const              { return eTag; }


  private:
    // DATA
    EV_TokenId          eTag;
};

class Tok_EOL : public Token
{
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
};

class Tok_EOF : public Token
{
    // OPERATIONS
    virtual void        Trigger(
                            TokenInterpreter &  io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
};


}   // namespace uidl
}   // namespace csi

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
