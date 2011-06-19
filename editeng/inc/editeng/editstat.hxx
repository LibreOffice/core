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

#ifndef _EDITSTAT_HXX
#define _EDITSTAT_HXX

#include <tools/string.hxx>
#include <i18npool/lang.h>

#define EE_CNTRL_USECHARATTRIBS     0x00000001  // Use of hard character attributes
#define EE_CNTRL_USEPARAATTRIBS     0x00000002  // Using paragraph attributes.
#define EE_CNTRL_CRSRLEFTPARA       0x00000004  // Cursor is moved to another paragraph
#define EE_CNTRL_DOIDLEFORMAT       0x00000008  // Formatting idle
#define EE_CNTRL_PASTESPECIAL       0x00000010  // Allow PasteSpecial
#define EE_CNTRL_AUTOINDENTING      0x00000020  // Automatic indenting
#define EE_CNTRL_UNDOATTRIBS        0x00000040  // Undo for Attributes....
#define EE_CNTRL_ONECHARPERLINE     0x00000080  // One character per line
#define EE_CNTRL_NOCOLORS           0x00000100  // Engine: No Color
#define EE_CNTRL_OUTLINER           0x00000200  // Special treatment Outliner/Outline mode
#define EE_CNTRL_OUTLINER2          0x00000400  // Special treatment Outliner/Page
#define EE_CNTRL_ALLOWBIGOBJS       0x00000800  // Portion info in text object
#define EE_CNTRL_ONLINESPELLING     0x00001000  // During the edit Spelling
#define EE_CNTRL_STRETCHING         0x00002000  // Stretch mode
#define EE_CNTRL_MARKFIELDS         0x00004000  // Mark Fields with color
#define EE_CNTRL_URLSFXEXECUTE      0x00008000  // !!!OLD!!!: SFX-URL-Execute.
#define EE_CNTRL_RESTOREFONT        0x00010000  // Restore Font in OutDev
#define EE_CNTRL_RTFSTYLESHEETS     0x00020000  // Use Stylesheets when imported
//#define EE_CNTRL_NOREDLINES            0x00040000  // No RedLines when OnlineSpellError     /* removed #i91949 */
#define EE_CNTRL_AUTOCORRECT        0x00080000  // AutoCorrect
#define EE_CNTRL_AUTOCOMPLETE       0x00100000  // AutoComplete
#define EE_CNTRL_AUTOPAGESIZEX      0x00200000  // Adjust paper width to Text
#define EE_CNTRL_AUTOPAGESIZEY      0x00400000  // Adjust paper height to Text
#define EE_CNTRL_AUTOPAGESIZE       (EE_CNTRL_AUTOPAGESIZEX|EE_CNTRL_AUTOPAGESIZEY)
#define EE_CNTRL_TABINDENTING       0x00800000  // Indent with tab
#define EE_CNTRL_FORMAT100          0x01000000  // Always format to 100%
#define EE_CNTRL_ULSPACESUMMATION   0x02000000  // MS Compat: sum SA and SB, not maximum value
#define EE_CNTRL_ULSPACEFIRSTPARA   0x04000000  // MS Compat: evaluate also at the first paragraph

#define EV_CNTRL_AUTOSCROLL         0x00000001  // Auto scrolling horizontally
#define EV_CNTRL_BIGSCROLL          0x00000002  // Scroll further to the cursor
#define EV_CNTRL_ENABLEPASTE        0x00000004  // Enable Paste
#define EV_CNTRL_SINGLELINEPASTE    0x00000008  // View: Paste in input line ...
#define EV_CNTRL_OVERWRITE          0x00000010  // Overwrite mode
#define EV_CNTRL_INVONEMORE         0x00000020  // Invalidate one pixel more
#define EV_CNTRL_AUTOSIZEX          0x00000040  // Automatically adapt to text width
#define EV_CNTRL_AUTOSIZEY          0x00000080  // Automatically adapt to Text width
#define EV_CNTRL_AUTOSIZE           (EV_CNTRL_AUTOSIZEX|EV_CNTRL_AUTOSIZEY)

#define EE_STAT_HSCROLL             0x00000001
#define EE_STAT_VSCROLL             0x00000002
#define EE_STAT_CURSOROUT           0x00000004
#define EE_STAT_CRSRMOVEFAIL        0x00000008
#define EE_STAT_CRSRLEFTPARA        0x00000010
#define EE_STAT_TEXTWIDTHCHANGED    0x00000020
#define EE_STAT_TEXTHEIGHTCHANGED   0x00000040
#define EE_STAT_WRONGWORDCHANGED    0x00000080
// #define EE_STAT_MODIFIED         0x00000100

// Only for Update:
#define EE_CNTRL_AUTOCENTER     EE_CNTRL_AUTOPAGESIZE
#define EE_STAT_GROWY           EE_STAT_TEXTHEIGHTCHANGED

/*
    EE_STAT_CRSRLEFTPARA at the time cursor movement and the enter.
*/

inline void SetFlags( sal_uLong& rBits, const sal_uInt32 nMask, bool bOn )
{
    if ( bOn )
        rBits |= nMask;
    else
        rBits &= ~nMask;
}

class EditStatus
{
protected:
    sal_uLong   nStatusBits;
    sal_uLong   nControlBits;
    sal_uInt16  nPrevPara;                  // for EE_STAT_CRSRLEFTPARA

public:
            EditStatus()                { nStatusBits = 0; nControlBits = 0; nPrevPara = 0xFFFF; }

    void    Clear()                     { nStatusBits = 0; }
    void    SetControlBits( sal_uLong nMask, bool bOn )
                { SetFlags( nControlBits, nMask, bOn ); }

    sal_uLong   GetStatusWord() const       { return nStatusBits; }
    sal_uLong&  GetStatusWord()             { return nStatusBits; }

    sal_uLong   GetControlWord() const      { return nControlBits; }
    sal_uLong&  GetControlWord()            { return nControlBits; }

    sal_uInt16  GetPrevParagraph() const    { return nPrevPara; }
    sal_uInt16& GetPrevParagraph()          { return nPrevPara; }
};

#define SPELLCMD_IGNOREWORD         0x0001
#define SPELLCMD_STARTSPELLDLG      0x0002
#define SPELLCMD_ADDTODICTIONARY    0x0003
#define SPELLCMD_WORDLANGUAGE       0x0004
#define SPELLCMD_PARALANGUAGE       0x0005

struct SpellCallbackInfo
{
    sal_uInt16          nCommand;
    String          aWord;
    LanguageType    eLanguage;

    SpellCallbackInfo( sal_uInt16 nCMD, const String& rWord )
    : aWord( rWord )
    {
        nCommand = nCMD;
        eLanguage = LANGUAGE_DONTKNOW;
    }

    SpellCallbackInfo( sal_uInt16 nCMD, LanguageType eLang )
    {
        nCommand = nCMD;
        eLanguage = eLang;
    }
};

#endif // _EDITSTAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
