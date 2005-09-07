/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tk_punct.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:06:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


