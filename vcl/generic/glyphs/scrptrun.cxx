/*
 *******************************************************************************
 *
 *   Copyright (c) 1995-2013 International Business Machines Corporation and others
 *
 *   All rights reserved.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy of
 *   this software and associated documentation files (the "Software"), to deal in
 *   the Software without restriction, including without limitation the rights to
 *   use, copy, modify, merge, publish, distribute, and/or sell copies of the
 *   Software, and to permit persons to whom the Software is furnished to do so,
 *   provided that the above copyright notice(s) and this permission notice appear
 *   in all copies of the Software and that both the above copyright notice(s) and
 *   this permission notice appear in supporting documentation.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN
 *   NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE
 *   LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY
 *   DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 *   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *   Except as contained in this notice, the name of a copyright holder shall not be
 *   used in advertising or otherwise to promote the sale, use or other dealings in
 *   this Software without prior written authorization of the copyright holder.
 *
 *******************************************************************************
 *   file name:  scrptrun.cpp
 *
 *   created on: 10/17/2001
 *   created by: Eric R. Mader
 */

#include "unicode/utypes.h"
#include "unicode/uscript.h"

#include "scrptrun.h"

const char ScriptRun::fgClassID=0;

// WARNING: In the unlikely event that this array needs to be changed,
// the following arrays and getPairIndex() must also be changed
const UChar32 ScriptRun::pairedChars[] = {
    0x0028, 0x0029, // ascii paired punctuation
    0x003c, 0x003e,
    0x005b, 0x005d,
    0x007b, 0x007d,
    0x00ab, 0x00bb, // guillemets
    0x2018, 0x2019, // general punctuation
    0x201c, 0x201d,
    0x2039, 0x203a,
    0x3008, 0x3009, // chinese paired punctuation
    0x300a, 0x300b,
    0x300c, 0x300d,
    0x300e, 0x300f,
    0x3010, 0x3011,
    0x3014, 0x3015,
    0x3016, 0x3017,
    0x3018, 0x3019,
    0x301a, 0x301b
};

// The return values for getPairIndex() from 0x28 to 0xbb
const int8_t ScriptRun::pairedCharsIndex0x0028[] = {
     0,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1,  2, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1,  4, -1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1,  6, -1,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1,  9
};

// The return values for getPairIndex() from 0x2018 to 0x203a
const int8_t ScriptRun::pairedCharsIndex0x2018[] = {
    10, 11, -1, -1, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 14, 15
};

// The return values for getPairIndex() from 0x3008 to 0x301b
const int8_t ScriptRun::pairedCharsIndex0x3008[] = {
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, 26, 27, 28, 29,
    30, 31, 32, 33
};

// If ch is present in pairedChars, return its index, else -1
inline int32_t ScriptRun::getPairIndex(UChar32 ch)
{
    if (ch <= 0xbb) {
        if (ch >= 0x28)
            return pairedCharsIndex0x0028[ch - 0x28];
        return -1;
    }

    if (ch <= 0x203a) {
        if (ch >= 0x2018)
            return pairedCharsIndex0x2018[ch - 0x2018];
        return -1;
    }

    if (ch >= 0x3008 && ch <= 0x301b) {
        return pairedCharsIndex0x3008[ch - 0x3008];
    }

    return -1;
}

UBool ScriptRun::sameScript(int32_t scriptOne, int32_t scriptTwo)
{
    return scriptOne <= USCRIPT_INHERITED || scriptTwo <= USCRIPT_INHERITED || scriptOne == scriptTwo;
}

UBool ScriptRun::next()
{
    int32_t startSP  = parenSP;  // used to find the first new open character
    UErrorCode error = U_ZERO_ERROR;

    // if we've fallen off the end of the text, we're done
    if (scriptEnd >= charLimit) {
        return false;
    }

    scriptCode = USCRIPT_COMMON;

    for (scriptStart = scriptEnd; scriptEnd < charLimit; scriptEnd += 1) {
        UChar   high = charArray[scriptEnd];
        UChar32 ch   = high;

        // if the character is a high surrogate and it's not the last one
        // in the text, see if it's followed by a low surrogate
        if (high >= 0xD800 && high <= 0xDBFF && scriptEnd < charLimit - 1)
        {
            UChar low = charArray[scriptEnd + 1];

            // if it is followed by a low surrogate,
            // consume it and form the full character
            if (low >= 0xDC00 && low <= 0xDFFF) {
                ch = (high - 0xD800) * 0x0400 + low - 0xDC00 + 0x10000;
                scriptEnd += 1;
            }
        }

        UScriptCode sc = uscript_getScript(ch, &error);
        int32_t pairIndex = getPairIndex(ch);

        // Paired character handling:

        // if it's an open character, push it onto the stack.
        // if it's a close character, find the matching open on the
        // stack, and use that script code. Any non-matching open
        // characters above it on the stack will be poped.
        if (pairIndex >= 0) {
            if ((pairIndex & 1) == 0) {
                parenStack[++parenSP].pairIndex = pairIndex;
                parenStack[parenSP].scriptCode  = scriptCode;
            } else if (parenSP >= 0) {
                int32_t pi = pairIndex & ~1;

                while (parenSP >= 0 && parenStack[parenSP].pairIndex != pi) {
                    parenSP -= 1;
                }

                if (parenSP < startSP) {
                    startSP = parenSP;
                }

                if (parenSP >= 0) {
                    sc = parenStack[parenSP].scriptCode;
                }
            }
        }

        if (sameScript(scriptCode, sc)) {
            if (scriptCode <= USCRIPT_INHERITED && sc > USCRIPT_INHERITED) {
                scriptCode = sc;

                // now that we have a final script code, fix any open
                // characters we pushed before we knew the script code.
                while (startSP < parenSP) {
                    parenStack[++startSP].scriptCode = scriptCode;
                }
            }

            // if this character is a close paired character,
            // pop it from the stack
            if (pairIndex >= 0 && (pairIndex & 1) != 0 && parenSP >= 0) {
                parenSP -= 1;
                /* decrement startSP only if it is >= 0,
                   decrementing it unnecessarily will lead to memory corruption
                   while processing the above while block.
                   e.g. startSP = -4 , parenSP = -1
                */
                if (startSP >= 0) {
                    startSP -= 1;
                }
            }
        } else {
            // if the run broke on a surrogate pair,
            // end it before the high surrogate
            if (ch >= 0x10000) {
                scriptEnd -= 1;
            }

            break;
        }
    }

    return true;
}

