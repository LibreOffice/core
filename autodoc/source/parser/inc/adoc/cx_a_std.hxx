/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_a_std.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:35:56 $
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

#ifndef ADC_ADOC_CX_A_STD_HXX
#define ADC_ADOC_CX_A_STD_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS

class TextToken;


namespace adoc {

class Cx_LineStart;
class Cx_CheckStar;
class Cx_AtTagCompletion;


/**
@descr
*/
class Context_AdocStd : public autodoc::TkpDocuContext,
                        private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_AdocStd();
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken );
                        ~Context_AdocStd();

    //  OPERATIONS
    virtual void        AssignDealer(
                            TokenDealer &       o_rDealer );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual void        SetMode_IsMultiLine(
                            bool                i_bTrue );
    //  INQUIRY
    virtual TkpContext &
                        FollowUpContext();
  private:
    //  SERVICE FUNCTIONS
    virtual void        PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            F_CRTOK             i_fTokenCreateFunction,
                            CharacterSource &   io_rText );

    void                SetupStateMachine();

    //  DATA
    StateMachine        aStateMachine;
    TokenDealer *       pDealer;

        // Contexts
    TkpContext *        pParentContext;
    TkpContext *        pFollowUpContext;
    Dyn<Cx_LineStart>   pCx_LineStart;
    Dyn<Cx_CheckStar>   pCx_CheckStar;
    Dyn<Cx_AtTagCompletion>
                        pCx_AtTagCompletion;

        // Temporary data, used during ReadCharChain()
    Dyn<TextToken>      pNewToken;
    bool                bIsMultiline;
};


}   // namespace adoc


#endif

