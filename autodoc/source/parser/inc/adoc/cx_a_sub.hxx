/*************************************************************************
 *
 *  $RCSfile: cx_a_sub.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

