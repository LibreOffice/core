/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cx_a_sub.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:36:12 $
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

#ifndef ADC_ADOC_CX_A_SUB_HXX
#define ADC_ADOC_CX_A_SUB_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS
#include <tokens/token.hxx>


namespace adoc {




class Cx_LineStart : public TkpContext
{
  public:
                        Cx_LineStart(
                            TkpContext &        i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    void                SetCurToken(
                            TextToken::F_CRTOK  i_fTokenCreateFunction )
                                                { fCur_TokenCreateFunction = i_fTokenCreateFunction; }
    void                AssignDealer(
                            TokenDealer &       o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *       pDealer;
    TkpContext *        pFollowUpContext;

    Dyn<TextToken>      pNewToken;

    TextToken::F_CRTOK  fCur_TokenCreateFunction;
};


/**
@descr
*/

class Cx_CheckStar : public TkpContext
{
  public:
    // LIFECYCLE
                        Cx_CheckStar(
                            TkpContext &        i_rFollowUpContext );
    void                Set_End_FollowUpContext(
                            TkpContext &        i_rEnd_FollowUpContext )
                                                { pEnd_FollowUpContext = &i_rEnd_FollowUpContext; }

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();

    void                SetCanBeEnd(
                            bool                i_bCanBeEnd  )
                                                { bCanBeEnd = i_bCanBeEnd; }
    virtual TkpContext &
                        FollowUpContext();
    void                AssignDealer(
                            TokenDealer &       o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *       pDealer;
    TkpContext *        pFollowUpContext;
    TkpContext *        pEnd_FollowUpContext;

    Dyn<TextToken>      pNewToken;

    bool                bCanBeEnd;
    bool                bEndTokenFound;
};


class Cx_AtTagCompletion : public TkpContext
{
  public:
                        Cx_AtTagCompletion(
                            TkpContext &        i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    void                SetCurToken(
                            TextToken::F_CRTOK  i_fTokenCreateFunction )
                                                { fCur_TokenCreateFunction = i_fTokenCreateFunction; }
    void                AssignDealer(
                            TokenDealer &       o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *       pDealer;
    TkpContext *        pFollowUpContext;

    Dyn<TextToken>      pNewToken;

    TextToken::F_CRTOK  fCur_TokenCreateFunction;
};


}   // namespace adoc


#endif

