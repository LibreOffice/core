/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
        void            BeginSource();
        intt            CurPos() const;

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
