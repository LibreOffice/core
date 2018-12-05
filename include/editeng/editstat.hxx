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

#ifndef INCLUDED_EDITENG_EDITSTAT_HXX
#define INCLUDED_EDITENG_EDITSTAT_HXX

#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <i18nlangtag/lang.h>
#include <o3tl/typed_flags_set.hxx>

enum class EEControlBits
{
    NONE               = 0x00000000,
    USECHARATTRIBS     = 0x00000001,  // Use of hard character attributes
    DOIDLEFORMAT       = 0x00000008,  // Formatting idle
    PASTESPECIAL       = 0x00000010,  // Allow PasteSpecial
    AUTOINDENTING      = 0x00000020,  // Automatic indenting
    UNDOATTRIBS        = 0x00000040,  // Undo for Attributes....
    ONECHARPERLINE     = 0x00000080,  // One character per line
    NOCOLORS           = 0x00000100,  // Engine: No Color
    OUTLINER           = 0x00000200,  // Special treatment Outliner/Outline mode
    OUTLINER2          = 0x00000400,  // Special treatment Outliner/Page
    ALLOWBIGOBJS       = 0x00000800,  // Portion info in text object
    ONLINESPELLING     = 0x00001000,  // During the edit Spelling
    STRETCHING         = 0x00002000,  // Stretch mode
    MARKNONURLFIELDS   = 0x00004000,  // Mark fields other than URL with color
    MARKURLFIELDS      = 0x00008000,  // Mark URL fields with color
    MARKFIELDS         = (MARKNONURLFIELDS | MARKURLFIELDS),
    RTFSTYLESHEETS     = 0x00020000,  // Use Stylesheets when imported
    AUTOCORRECT        = 0x00080000,  // AutoCorrect
    AUTOCOMPLETE       = 0x00100000,  // AutoComplete
    AUTOPAGESIZEX      = 0x00200000,  // Adjust paper width to Text
    AUTOPAGESIZEY      = 0x00400000,  // Adjust paper height to Text
    AUTOPAGESIZE       = (AUTOPAGESIZEX | AUTOPAGESIZEY),
    FORMAT100          = 0x01000000,  // Always format to 100%
    ULSPACESUMMATION   = 0x02000000,  // MS Compat: sum SA and SB, not maximum value
};
namespace o3tl
{
    template<> struct typed_flags<EEControlBits> : is_typed_flags<EEControlBits, 0x037efff9> {};
}

enum class EVControlBits
{
    AUTOSCROLL         = 0x0001,  // Auto scrolling horizontally
    ENABLEPASTE        = 0x0004,  // Enable Paste
    SINGLELINEPASTE    = 0x0008,  // View: Paste in input line ...
    OVERWRITE          = 0x0010,  // Overwrite mode
    INVONEMORE         = 0x0020,  // Invalidate one pixel more
    AUTOSIZEX          = 0x0040,  // Automatically adapt to text width
    AUTOSIZEY          = 0x0080,  // Automatically adapt to Text width
    AUTOSIZE           = (AUTOSIZEX|AUTOSIZEY)
};
namespace o3tl
{
    template<> struct typed_flags<EVControlBits> : is_typed_flags<EVControlBits, 0xfd> {};
}

enum class EditStatusFlags
{
    NONE                = 0x0000,
    HSCROLL             = 0x0001,
    VSCROLL             = 0x0002,
    CURSOROUT           = 0x0004,
    TEXTWIDTHCHANGED    = 0x0010,
    TextHeightChanged   = 0x0020,
    WRONGWORDCHANGED    = 0x0040
};
namespace o3tl
{
    template<> struct typed_flags<EditStatusFlags> : is_typed_flags<EditStatusFlags, 0x77> {};
}

inline void SetFlags( EVControlBits& rBits, EVControlBits nMask, bool bOn )
{
    if ( bOn )
        rBits |= nMask;
    else
        rBits &= ~nMask;
}

class EditStatus
{
protected:
    EditStatusFlags nStatusBits;
    EEControlBits   nControlBits;

public:
            EditStatus()                { nStatusBits = EditStatusFlags::NONE; nControlBits = EEControlBits::NONE; }

    void    Clear()                     { nStatusBits = EditStatusFlags::NONE; }

    EditStatusFlags  GetStatusWord() const       { return nStatusBits; }
    EditStatusFlags& GetStatusWord()             { return nStatusBits; }

    EEControlBits& GetControlWord()            { return nControlBits; }
};

enum class SpellCallbackCommand
{
    IGNOREWORD          = 0x0001,
    STARTSPELLDLG       = 0x0002,
    ADDTODICTIONARY     = 0x0003,
    WORDLANGUAGE        = 0x0004,
    PARALANGUAGE        = 0x0005,
    AUTOCORRECT_OPTIONS = 0x0006,
};

struct SpellCallbackInfo
{
    SpellCallbackCommand nCommand;
    OUString        aWord;

    SpellCallbackInfo( SpellCallbackCommand nCMD, const OUString& rWord )
        : aWord( rWord )
    {
        nCommand = nCMD;
    }

    SpellCallbackInfo( SpellCallbackCommand nCMD )
    {
        nCommand = nCMD;
    }
};

#endif // INCLUDED_EDITENG_EDITSTAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
