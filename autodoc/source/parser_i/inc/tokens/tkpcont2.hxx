/*************************************************************************
 *
 *  $RCSfile: tkpcont2.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:36 $
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

#ifndef ADC_TKPCONT2_HXX
#define ADC_TKPCONT2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
class CharacterSource;
class TkpNullContext;
class TkpNullContex2;

/** @task
    Specifies a context within which tokens are interpreted in a special
    way. For example in parsing C++ there could be a context for code,
    one for comments and a third one for preprocessor statements, because
    each of these would give the same token different meanings.
**/
class TkpContext
{
  public:
    // LIFECYCLE
    virtual                 ~TkpContext() {}

    // OPERATIONS
    /** @descr
        The functions starts to parse with the CurToken() of io_rText.
        It leaves io_rText at the first char of the following Token or
        the following Context.

        This function returns, when a context has parsed some characterss
        and completed a token OR left the context.
        If the token is to be ignored, PassNewToken() returns false
        and cuts the token from io_rText.
        If the token is to be parsed further in a different context,
        PassNewToken() returns false, but the token is
        NOT cut from io_rText.

        If the function has found a valid and complete token, PassNewToken()
        passes the parsed token to the internally known receiver and
        returns true. The token is cut from io_rText.
    **/
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText ) = 0;
    /** Has to pass the parsed token to a known receiver.
        @return true, if a token was passed.
                false, if no token was parsed complete by this context.
    */
    virtual bool        PassNewToken() = 0;
    virtual TkpContext &
                        FollowUpContext() = 0;

    static TkpNullContext &
                        Null_();
};

TkpNullContex2 &    TkpContext_Null2_();

class StateMachineContext
{
  public:
    /// Is used by StmBoundsStatu2 only.
    virtual void        PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            UINT16              i_nTokenId,
                            CharacterSource &   io_rText ) = 0;
};

class TkpNullContex2 : public TkpContext
{
  public:
                        ~TkpNullContex2();

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();
};

class TkpDocuContext : public TkpContext
{
  public:
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken ) = 0;
    virtual void        SetMode_IsMultiLine(
                            bool                i_bTrue ) = 0;
};



#endif


