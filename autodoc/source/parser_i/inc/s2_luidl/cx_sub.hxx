/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_sub.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:59:04 $
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
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
  protected:
    // LIFECYCLE
                        Cx_Base(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   rReceiver(o_rReceiver),
                                                    pFollowUpContext(&i_rFollowUpContext),
                                                    pNewToken()
                                                    {}
  protected:
    void                SetToken(
                            DYN Token *         let_dpToken )
                                                { pNewToken = let_dpToken; }
    Token_Receiver &    Receiver()              { return rReceiver; }

  private:
    // DATA
    Token_Receiver &    rReceiver;
    TkpContext *        pFollowUpContext;
    Dyn<Token>          pNewToken;
};



/**
@descr
*/

class Context_MLComment : public Cx_Base
{
  public:
                        Context_MLComment(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};

class Context_SLComment : public Cx_Base
{
  public:
                        Context_SLComment(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};

class Context_Praeprocessor : public Cx_Base
{
  public:
                        Context_Praeprocessor(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};

class Context_Assignment : public Cx_Base
{
  public:
                        Context_Assignment(
                            Token_Receiver &    o_rReceiver,
                            TkpContext &        i_rFollowUpContext )
                                                :   Cx_Base(o_rReceiver, i_rFollowUpContext) {}
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
};


}   // namespace uidl
}   // namespace csi

#endif

