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
        BracketOpen =  1, 			// (
        BracketClose = 2,           // )
        ArrayBracketOpen = 3,       // [
        ArrayBracketClose = 4,      // ]
        CurledBracketOpen = 5,      // {
        CurledBracketClose = 6,     // }
        Semicolon = 7, 	            // ;
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
                            EV_TokenId			i_eTag )
                                                :	eTag(i_eTag) {}
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
    EV_TokenId			Id() const				{ return eTag; }


  private:
    // DATA
    EV_TokenId   		eTag;
};

class Tok_EOL : public Token
{
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
};

class Tok_EOF : public Token
{
    // OPERATIONS
    virtual void		Trigger(
                            TokenInterpreter &	io_rInterpreter ) const;
    // INQUIRY
    virtual const char *
                        Text() const;
};


}   // namespace uidl
}   // namespace csi

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
