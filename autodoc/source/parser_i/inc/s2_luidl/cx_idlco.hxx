/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_idlco.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:58:48 $
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

#ifndef LUIDL_CX_IDLCO_HXX
#define LUIDL_CX_IDLCO_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcont2.hxx>
    // COMPONENTS
#include <tokens/tkpstam2.hxx>
    // PARAMETERS


namespace csi
{
namespace uidl
{

class Token_Receiver;
class Token;

/**
*/
class Context_UidlCode : public TkpContext,
                         private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_UidlCode(
                            Token_Receiver &    o_rReceiver,
                            DYN TkpDocuContext &
                                                let_drContext_Docu  );
                        ~Context_UidlCode();
    //  OPERATORS

    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();

    //  INQUIRY
    virtual TkpContext &
                        FollowUpContext();
  private:
    //  SERVICE FUNCTIONS
    void                PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            UINT16              i_nTokenId,
                            CharacterSource &   io_rText );
    void                SetupStateMachine();

    //  DATA
    StateMachin2        aStateMachine;
    Token_Receiver *    pReceiver;

        // Contexts
    Dyn<TkpDocuContext> pDocuContext;

    Dyn<TkpContext>    dpContext_MLComment;
    Dyn<TkpContext>    dpContext_SLComment;
    Dyn<TkpContext>    dpContext_Preprocessor;
    Dyn<TkpContext>    dpContext_Assignment;

        // Temporary data, used during ReadCharChain()
    Dyn<Token>          pNewToken;
    ::TkpContext *      pFollowUpContext;
};


}   // namespace uidl
}   // namespace csi

#endif
