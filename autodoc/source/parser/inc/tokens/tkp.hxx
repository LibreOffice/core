/*************************************************************************
 *
 *  $RCSfile: tkp.hxx,v $
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

#ifndef ADC_TKP_HXX
#define ADC_TKP_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
class CharacterSource;
class TkpContext;
    // PARAMETRS



/** This is the interface for parser classes, which get a sequence of tokens from
    a text.

    Start() starts to parse the text from the given i_rSource.
    GetNextToken() returns a Token on the heap as long as there are
    still characters in the text left. This can be checked by
    HasMore().

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/
#if 0
/**
    Parsing can be interrupted for a different source by PushSource().
    The parsing before interruption is continued after PopSource().
*/
#endif // 0

class TokenParser
{
  public:
    // LIFECYCLE
                        TokenParser();
    virtual             ~TokenParser() {}

    // OPERATIONS
    /** Start parsing a character source. Any previously parsed sources
        are discarded.
    */
    virtual void        Start(
                            CharacterSource &
                                            i_rSource );

    /** @short  Gets the next identifiable token out of the
        source code.
    */
    void                GetNextToken();

    /// @return true, if there are more tokens to parse.
    bool                HasMore() const         { return bHasMore; }

  private:
    void                InitSource(
                            CharacterSource &
                                            i_rSource );

    virtual void        SetStartContext() = 0;
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext ) = 0;
    virtual TkpContext &
                        CurrentContext() = 0;
    // DATA
    CharacterSource *   pChars;
    bool                bHasMore;
};


#endif


