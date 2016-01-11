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
/**
  * This file is largely copied from the ICU project,
  * under folder source/extra/scrptrun/scrptrun.cpp
  */
#include "unicode/utypes.h"
#include "unicode/uscript.h"

#include "scrptrun.h"
#include <algorithm>

namespace {

struct PairIndices
{
    int8_t ma00[0xff];
    int8_t ma20[0x7f];
    int8_t ma30[0x7f];

    PairIndices()
    {
        std::fill_n(ma00, 0xff, -1);
        std::fill_n(ma20, 0x7f, -1);
        std::fill_n(ma30, 0x7f, -1);

        // characters in the range 0x0000 - 0x007e (inclusive)
        // ascii paired punctuation
        ma00[0x28] =  0;
        ma00[0x29] =  1;
        ma00[0x3c] =  2;
        ma00[0x3e] =  3;
        ma00[0x5b] =  4;
        ma00[0x5d] =  5;
        ma00[0x7b] =  6;
        ma00[0x7d] =  7;
        // guillemets
        ma00[0xab] =  8;
        ma00[0xbb] =  9;

        // characters in the range 0x2000 - 0x207e (inclusive)
        // general punctuation
        ma20[0x18] = 10;
        ma20[0x19] = 11;
        ma20[0x1c] = 12;
        ma20[0x1d] = 13;
        ma20[0x39] = 14;
        ma20[0x3a] = 15;

        // characters in the range 0x3000 - 0x307e (inclusive)
        // chinese paired punctuation
        ma30[0x08] = 16;
        ma30[0x09] = 17;
        ma30[0x0a] = 18;
        ma30[0x0b] = 19;
        ma30[0x0c] = 20;
        ma30[0x0d] = 21;
        ma30[0x0e] = 22;
        ma30[0x0f] = 23;
        ma30[0x10] = 24;
        ma30[0x11] = 25;
        ma30[0x14] = 26;
        ma30[0x15] = 27;
        ma30[0x16] = 28;
        ma30[0x17] = 29;
        ma30[0x18] = 30;
        ma30[0x19] = 31;
        ma30[0x1a] = 32;
        ma30[0x1b] = 33;
    }

    inline int32_t getPairIndex(UChar32 ch) const
    {
        if (ch < 0xff)
            return ma00[ch];
        if (ch >= 0x2000 && ch < 0x207f)
            return ma20[ch - 0x2000];
        if (ch >= 0x3000 && ch < 0x307f)
            return ma30[ch - 0x3000];
        return -1;
    }

};

}

static const PairIndices gPairIndices;


namespace vcl {

const char ScriptRun::fgClassID=0;

static inline UBool sameScript(int32_t scriptOne, int32_t scriptTwo)
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
        int32_t pairIndex = gPairIndices.getPairIndex(ch);

        // Paired character handling:

        // if it's an open character, push it onto the stack.
        // if it's a close character, find the matching open on the
        // stack, and use that script code. Any non-matching open
        // characters above it on the stack will be poped.
        if (pairIndex >= 0) {
            if ((pairIndex & 1) == 0) {
                ++parenSP;
                int32_t nVecSize = parenStack.size();
                if (parenSP == nVecSize)
                    parenStack.resize(nVecSize + 128);
                parenStack[parenSP].pairIndex = pairIndex;
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

}
