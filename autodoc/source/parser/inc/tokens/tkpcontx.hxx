/*************************************************************************
 *
 *  $RCSfile: tkpcontx.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:33 $
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

#ifndef ADC_TKPCONTX_HXX
#define ADC_TKPCONTX_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <tokens/token.hxx>
class CharacterSource;
class TkpNullContext;

/** @task
    Specifies a context within which tokens are interpreted in a special
    way. For example in parsing C++ there could be a context for code,
    one for comments and a third one for preprocessor statements, because
    each of these would give the same token different meanings.

    The three functions
            ReadCharChain()
            PassNewToken()
            FollowUpContext()
    have to be called in this sequence.

**/
class TkpContext
{
  public:
    // LIFECYCLE
    virtual                 ~TkpContext() {}

    // OPERATIONS
    /** @descr
        The functions starts to parse with the CurChar() of io_rText.
        It leaves io_rText.CurChar() at the first char of the following Token or
        the following Context.

        This function returns, when a context has parsed some characterss
        and completed a token OR left the context.
        If the token is to be ignored, it is cut from io_rText.

        If the token is to be parsed further in a different context,
        it is NOT cut from io_rText.

        After this function PassNewToken() has to be called.

        If the function has found a valid and complete token, PassNewToken()
        passes the parsed token to the internally known receiver and
        returns true. The token is cut from io_rText.
    **/
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText ) = 0;
    /** Has to pass the parsed token to a known receiver.
        If the token is to be parsed further in a different context,
        PassNewToken() returns false, but the token is NOT cut from io_rText.

        @return true, if a token was passed.
                false, if the token was not parsed completely by this context
                       or if the token is to be ignored.
    */
    virtual bool        PassNewToken() = 0;
    virtual TkpContext &
                        FollowUpContext() = 0;

    static TkpNullContext &
                        Null_();
};

class StateMachineContext
{
  public:
    typedef TextToken::F_CRTOK F_CRTOK;

    /// Is used by StmBoundsStatus only.
    virtual void        PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            F_CRTOK             i_fTokenCreateFunction,
                            CharacterSource &   io_rText ) = 0;
};

class TkpNullContext : public TkpContext
{
  public:
                        ~TkpNullContext();

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
};

namespace autodoc
{

class TkpDocuContext : public TkpContext
{
  public:
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken ) = 0;
    virtual void        AssignDealer(
                            TokenDealer &       o_rDealer ) = 0;
    virtual void        SetMode_IsMultiLine(
                            bool                i_bTrue ) = 0;
};

} // namespace autodoc

#endif


