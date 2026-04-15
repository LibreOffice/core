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
 *   file name:  scrptrun.h
 *
 *   created on: 10/17/2001
 *   created by: Eric R. Mader
 */

#pragma once

#include <config_options.h>
#include <sal/config.h>

#include <vcl/dllapi.h>

#include <unicode/uobject.h>
#include <unicode/uscript.h>
#include <vector>

namespace vcl
{
struct ParenStackEntry
{
    int32_t pairIndex;
    UScriptCode scriptCode;
    ParenStackEntry()
        : pairIndex(0)
        , scriptCode(USCRIPT_INVALID_CODE)
    {
    }
};

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) ScriptRun final : public icu::UObject
{
public:
    ScriptRun(const UChar chars[], int32_t length);

    void reset();

    void reset(int32_t start, int32_t count);

    void reset(const UChar chars[], int32_t start, int32_t length);

    int32_t getScriptStart() const;

    int32_t getScriptEnd() const;

    UScriptCode getScriptCode() const;

    UBool next();

    /**
s     * ICU "poor man's RTTI", returns a UClassID for the actual class.
     *
     * @stable ICU 2.2
     */
    virtual UClassID getDynamicClassID() const override { return getStaticClassID(); }

    /**
     * ICU "poor man's RTTI", returns a UClassID for this class.
     *
     * @stable ICU 2.2
     */
    static UClassID getStaticClassID()
    {
        return static_cast<UClassID>(const_cast<char*>(&fgClassID));
    }

private:
    int32_t charStart;
    int32_t charLimit;
    const UChar* charArray;

    int32_t scriptStart;
    int32_t scriptEnd;
    UScriptCode scriptCode;

    std::vector<ParenStackEntry> parenStack;
    int32_t parenSP;

    /**
     * The address of this static class variable serves as this class's ID
     * for ICU "poor man's RTTI".
     */
    static const char fgClassID;
};

inline ScriptRun::ScriptRun(const UChar chars[], int32_t length)
{
    parenStack.reserve(128);
    reset(chars, 0, length);
}

inline int32_t ScriptRun::getScriptStart() const { return scriptStart; }

inline int32_t ScriptRun::getScriptEnd() const { return scriptEnd; }

inline UScriptCode ScriptRun::getScriptCode() const { return scriptCode; }

inline void ScriptRun::reset()
{
    scriptStart = charStart;
    scriptEnd = charStart;
    scriptCode = USCRIPT_INVALID_CODE;
    parenSP = -1;
    parenStack.clear();
}

inline void ScriptRun::reset(int32_t start, int32_t length)
{
    charStart = start;
    charLimit = start + length;

    reset();
}

inline void ScriptRun::reset(const UChar chars[], int32_t start, int32_t length)
{
    charArray = chars;

    reset(start, length);
}
}
