/*************************************************************************
 *
 *  $RCSfile: tkpchars.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-05-14 09:02:18 $
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

#ifndef ADC_TKPCHARS_HXX
#define ADC_TKPCHARS_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETRS
#include <adc_cl.hxx>
#include <stack>



/** @descr

    dpSource:

    1||||||||||||||||||||||a||||||||||||b|||c||||||||||||||||||||...


    1 := first character of Sourcecode.
    a := nLastTokenStart, there starts the last cut token.
    b := nLastCut, there is a '\0'-char which marks the end of
         the last cut token. The original character at b is stored
         in cCharAtLastCut and will replace the '\0'-char, when the
         next token is cut.
    c := The current cursor position.


    @needs  cosv.lib

    @use    This class can be used by any parser to get the chars of a
            text one by one and separate them to tokens.
**/

class CharacterSource
{
    public:
        // LIFECYCLE
                        CharacterSource();
                        ~CharacterSource();

        // OPERATIONS
        /** Loads the complete contents of in_rSource into the classes private memory.
            If in_rSource is a file, it has to be open of course.
            After loading the text, the CurChar() is set on the begin of the text.
        **/
        void            LoadText(
                            csv::bstream &      io_rSource);
        /** Loads the complete contents of in_rSource into the classes private memory.
            If in_rSource is a file, it has to be open of course.
            After loading the text, the CurChar() is set on the begin of the text.
        **/
        void            LoadText(
                            const char *        i_sSourceText );

        void            InsertTextAtCurPos(
                            const char *        i_sText2Insert );

        /// @return CurChar() after moving forward one char.
            char            MoveOn();
        /** @return
            The token which starts at the char which was CurChar(), when
            CutToken() was called the last time - or at the beginning of the text.
            The token ends by the CurChar() being replaced by a '\0'.

            Value is valid until the next call of CutToken() or ~CharacterSource().
        **/
        const char *    CutToken();

        // INQUIRY
        char            CurChar() const;
        /// @return The result of the last CutToken(). Or NULL, if there was none yet.
        const char *    CurToken() const;

    // INQUIRY
        /// @return true, if
        bool            IsFinished() const;

    private:
        struct S_SourceState
        {
            DYN char *      dpSource;
            intt            nSourceSize;

            intt            nCurPos;
            intt            nLastCut;
            intt            nLastTokenStart;
            char            cCharAtLastCut;

                            S_SourceState(
                                DYN char *      dpSource,
                                intt            nSourceSize,
                                intt            nCurPos,
                                intt            nLastCut,
                                intt            nLastTokenStart,
                                char            cCharAtLastCut );
        };

        void            BeginSource();
        intt            CurPos() const;
        char            MoveOn_OverStack();

        // DATA
        std::stack< S_SourceState >
                        aSourcesStack;

        DYN char *      dpSource;
        intt            nSourceSize;

        intt            nCurPos;
        intt            nLastCut;
        intt            nLastTokenStart;
        char            cCharAtLastCut;
};


inline char
CharacterSource::MoveOn()
    {
if (DEBUG_ShowText())
{
        Cerr() << char(dpSource[nCurPos+1]) << Flush();
}
        if ( nCurPos < nSourceSize-1 )
            return dpSource[++nCurPos];
        else if ( aSourcesStack.size() > 0 )
            return MoveOn_OverStack();
        else
            return dpSource[nCurPos = nSourceSize];
    }
inline char
CharacterSource::CurChar() const
    { return nCurPos != nLastCut ? dpSource[nCurPos] : cCharAtLastCut; }
inline const char *
CharacterSource::CurToken() const
    { return &dpSource[nLastTokenStart]; }
inline bool
CharacterSource::IsFinished() const
    { return nCurPos >= nSourceSize; }
inline intt
CharacterSource::CurPos() const
    { return nCurPos; }




#endif


