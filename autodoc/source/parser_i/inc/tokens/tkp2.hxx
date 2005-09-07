/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkp2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:09:14 $
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

#ifndef ADC_TKP2_HXX
#define ADC_TKP2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
class CharacterSource;
class TkpContext;
    // PARAMETRS



/** This is the interface for parser classes, which get a sequence of Token s from
    a text.

    Start() starts to parse the text from the given i_rSource.
    GetNextToken() returns a Token on the heap as long as there are
    still characters in the text left. The last time GetNextToken()
    returns NULL.

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/
class TokenParse2
{
  public:
    // LIFECYCLE
                        TokenParse2();
    virtual             ~TokenParse2() {}

    // OPERATIONS
    virtual void        Start(
                            CharacterSource &
                                            i_rSource );

    /** @short  Gets the next identifiable token out of the
        source code.
        @return true, if there was passed a valid token.
                false, if the parsed stream is finished or
                       an error occured.
    */
    bool                GetNextToken();

  private:
    virtual void        SetStartContext() = 0;
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext ) = 0;
    virtual TkpContext &
                        CurrentContext() = 0;
    // DATA
    CharacterSource *   pChars;
};


#endif


