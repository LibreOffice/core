/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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



/**	@descr

    dpSource:

    1||||||||||||||||||||||a||||||||||||b|||c||||||||||||||||||||...


    1 := first character of Sourcecode.
    a := nLastTokenStart, there starts the last cut token.
    b := nLastCut, there is a '\0'-char which marks the end of
         the last cut token. The original character at b is stored
         in cCharAtLastCut and will replace the '\0'-char, when the
         next token is cut.
    c := The current cursor position.


    @needs 	cosv.lib

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
        void			LoadText(
                            csv::bstream &		io_rSource);

        void            InsertTextAtCurPos(
                            const char *        i_sText2Insert );

        ///	@return CurChar() after moving forward one char.
            char            MoveOn();
        /** @return
            The token which starts at the char which was CurChar(), when
            CutToken() was called the last time - or at the beginning of the text.
            The token ends by the CurChar() being replaced by a '\0'.

            Value is valid until the next call of CutToken() or ~CharacterSource().
        **/
        const char *	CutToken();

        // INQUIRY
        char            CurChar() const;
        /// @return The result of the last CutToken(). Or NULL, if there was none yet.
        const char *	CurToken() const;

    // INQUIRY
        /// @return true, if
        bool			IsFinished() const;

    private:
        struct S_SourceState
        {
            DYN char *		dpSource;
            intt			nSourceSize;

            intt			nCurPos;
            intt			nLastCut;
            intt			nLastTokenStart;
            char 			cCharAtLastCut;

                            S_SourceState(
                                DYN char *		dpSource,
                                intt			nSourceSize,
                                intt			nCurPos,
                                intt			nLastCut,
                                intt			nLastTokenStart,
                                char 			cCharAtLastCut );
        };

        void            BeginSource();
        intt			CurPos() const;
        char            MoveOn_OverStack();

        // DATA
        std::stack< S_SourceState >
                        aSourcesStack;

        DYN char *		dpSource;
        intt			nSourceSize;

        intt			nCurPos;
        intt			nLastCut;
        intt			nLastTokenStart;
        char 			cCharAtLastCut;
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
