/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmdevt.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _VCL_CMDEVT_HXX
#define _VCL_CMDEVT_HXX

#include <tools/gen.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/keycod.hxx>
#include <vcl/font.hxx>

// ---------------------------
// - CommandExtTextInputData -
// ---------------------------

#define EXTTEXTINPUT_ATTR_GRAYWAVELINE          ((USHORT)0x0100)
#define EXTTEXTINPUT_ATTR_UNDERLINE             ((USHORT)0x0200)
#define EXTTEXTINPUT_ATTR_BOLDUNDERLINE         ((USHORT)0x0400)
#define EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE       ((USHORT)0x0800)
#define EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE      ((USHORT)0x1000)
#define EXTTEXTINPUT_ATTR_HIGHLIGHT             ((USHORT)0x2000)
#define EXTTEXTINPUT_ATTR_REDTEXT               ((USHORT)0x4000)
#define EXTTEXTINPUT_ATTR_HALFTONETEXT          ((USHORT)0x8000)

#define EXTTEXTINPUT_CURSOR_INVISIBLE           ((USHORT)0x0001)
#define EXTTEXTINPUT_CURSOR_OVERWRITE           ((USHORT)0x0002)

class VCL_DLLPUBLIC CommandExtTextInputData
{
private:
    XubString           maText;
    USHORT*             mpTextAttr;
    xub_StrLen          mnCursorPos;
    xub_StrLen          mnDeltaStart;
    xub_StrLen          mnOldTextLen;
    USHORT              mnCursorFlags;
    BOOL                mbOnlyCursor;

public:
                        CommandExtTextInputData();
                        CommandExtTextInputData( const XubString& rText,
                                                 const USHORT* pTextAttr,
                                                 xub_StrLen nCursorPos,
                                                 USHORT nCursorFlags,
                                                 xub_StrLen nDeltaStart,
                                                 xub_StrLen nOldTextLen,
                                                 BOOL bOnlyCursor );
                        CommandExtTextInputData( const CommandExtTextInputData& rData );
                        ~CommandExtTextInputData();

    const XubString&    GetText() const { return maText; }
    const USHORT*       GetTextAttr() const { return mpTextAttr; }
    USHORT              GetCharTextAttr( USHORT nIndex ) const;
    xub_StrLen          GetCursorPos() const { return mnCursorPos; }
    BOOL                IsCursorVisible() const { return (mnCursorFlags & EXTTEXTINPUT_CURSOR_INVISIBLE) == 0; }
    BOOL                IsCursorOverwrite() const { return (mnCursorFlags & EXTTEXTINPUT_CURSOR_OVERWRITE) != 0; }
    USHORT              GetCursorFlags() const { return mnCursorFlags; }
    xub_StrLen          GetDeltaStart() const { return mnDeltaStart; }
    xub_StrLen          GetOldTextLen() const { return mnOldTextLen; }
    BOOL                IsOnlyCursorChanged() const { return mbOnlyCursor; }
};

inline USHORT CommandExtTextInputData::GetCharTextAttr( USHORT nIndex ) const
{
    if ( mpTextAttr && (nIndex < maText.Len()) )
        return mpTextAttr[nIndex];
    else
        return 0;
}

// ---------------------------
// - CommandInputContextData -
// ---------------------------

class VCL_DLLPUBLIC CommandInputContextData
{
private:
    LanguageType    meLanguage;

public:
                    CommandInputContextData();
                    CommandInputContextData( LanguageType eLang );

    LanguageType    GetLanguage() const { return meLanguage; }
};

inline CommandInputContextData::CommandInputContextData()
{
    meLanguage = LANGUAGE_DONTKNOW;
}

inline CommandInputContextData::CommandInputContextData( LanguageType eLang )
{
    meLanguage = eLang;
}

// --------------------
// - CommandWheelData -
// --------------------

#define COMMAND_WHEEL_SCROLL            ((USHORT)0x0001)
#define COMMAND_WHEEL_ZOOM              ((USHORT)0x0002)
#define COMMAND_WHEEL_DATAZOOM          ((USHORT)0x0004)

#define COMMAND_WHEEL_PAGESCROLL        ((ULONG)0xFFFFFFFF)

class VCL_DLLPUBLIC CommandWheelData
{
private:
    long            mnDelta;
    long            mnNotchDelta;
    ULONG           mnLines;
    USHORT          mnMode;
    USHORT          mnCode;
    BOOL            mbHorz;

public:
                    CommandWheelData();
                    CommandWheelData( long nWheelDelta, long nWheelNotchDelta,
                                      ULONG nScrollLines,
                                      USHORT nWheelMode, USHORT nKeyModifier,
                                      BOOL mbHorz = FALSE );

    long            GetDelta() const { return mnDelta; }
    long            GetNotchDelta() const { return mnNotchDelta; }
    ULONG           GetScrollLines() const { return mnLines; }
    BOOL            IsHorz() const { return mbHorz; }

    USHORT          GetMode() const { return mnMode; }

    USHORT          GetModifier() const
                        { return (mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)); }
    BOOL            IsShift() const
                        { return ((mnCode & KEY_SHIFT) != 0); }
    BOOL            IsMod1() const
                        { return ((mnCode & KEY_MOD1) != 0); }
    BOOL            IsMod2() const
                        { return ((mnCode & KEY_MOD2) != 0); }
    BOOL            IsMod3() const
                        { return ((mnCode & KEY_MOD3) != 0); }
};

inline CommandWheelData::CommandWheelData()
{
    mnDelta         = 0;
    mnNotchDelta    = 0;
    mnLines         = 0;
    mnMode          = 0;
    mnCode          = 0;
    mbHorz          = FALSE;
}

inline CommandWheelData::CommandWheelData( long nWheelDelta, long nWheelNotchDelta,
                                           ULONG nScrollLines,
                                           USHORT nWheelMode, USHORT nKeyModifier,
                                           BOOL bHorz )
{
    mnDelta         = nWheelDelta;
    mnNotchDelta    = nWheelNotchDelta;
    mnLines         = nScrollLines;
    mnMode          = nWheelMode;
    mnCode          = nKeyModifier;
    mbHorz          = bHorz;
}

// ---------------------
// - CommandScrollData -
// ---------------------

class VCL_DLLPUBLIC CommandScrollData
{
private:
    long            mnDeltaX;
    long            mnDeltaY;

public:
                    CommandScrollData();
                    CommandScrollData( long nDeltaX, long nDeltaY );

    long            GetDeltaX() const { return mnDeltaX; }
    long            GetDeltaY() const { return mnDeltaY; }
};

inline CommandScrollData::CommandScrollData()
{
    mnDeltaX    = 0;
    mnDeltaY    = 0;
}

inline CommandScrollData::CommandScrollData( long nDeltaX, long nDeltaY )
{
    mnDeltaX    = nDeltaX;
    mnDeltaY    = nDeltaY;
}

// --------------------
// - CommandVoiceData -
// --------------------

enum DictationCommandType
{
    VOICECOMMANDTYPE_DICTATION,
    VOICECOMMANDTYPE_CONTROL
};

class VCL_DLLPUBLIC CommandVoiceData
{
private:
    XubString               maText;
    USHORT                  mnCommand;
    DictationCommandType    meType;

public:
                            CommandVoiceData();
                            CommandVoiceData( const XubString& rStr,
                                              DictationCommandType eType,
                                              USHORT nCommand );

    const XubString&        GetText() const { return maText; }
    USHORT                  GetCommand() const { return mnCommand; }
    DictationCommandType    GetType() const { return meType; }
};

inline CommandVoiceData::CommandVoiceData()
{
    meType = VOICECOMMANDTYPE_DICTATION;
    mnCommand = 0;
}

inline CommandVoiceData::CommandVoiceData( const XubString& rStr,
                                           DictationCommandType eType,
                                           USHORT nCommand ) :
    maText( rStr ),
    mnCommand( nCommand ),
    meType( eType )
{
}

// ---------------------
// - CommandModKeyData -
// ---------------------

class VCL_DLLPUBLIC CommandModKeyData
{
private:
    USHORT          mnCode;

public:
                    CommandModKeyData();
                    CommandModKeyData( USHORT nCode );

    BOOL            IsShift()   const { return (mnCode & MODKEY_SHIFT) ? TRUE : FALSE; }
    BOOL            IsMod1()    const { return (mnCode & MODKEY_MOD1) ? TRUE : FALSE; }
    BOOL            IsMod2()    const { return (mnCode & MODKEY_MOD2) ? TRUE : FALSE; }

    BOOL            IsLeftShift() const { return (mnCode & MODKEY_LSHIFT) ? TRUE : FALSE; }
    BOOL            IsLeftMod1()  const { return (mnCode & MODKEY_LMOD1) ? TRUE : FALSE; }
    BOOL            IsLeftMod2()  const { return (mnCode & MODKEY_LMOD2) ? TRUE : FALSE; }

    BOOL            IsRightShift() const { return (mnCode & MODKEY_RSHIFT) ? TRUE : FALSE; }
    BOOL            IsRightMod1()  const { return (mnCode & MODKEY_RMOD1) ? TRUE : FALSE; }
    BOOL            IsRightMod2()  const { return (mnCode & MODKEY_RMOD2) ? TRUE : FALSE; }
};

inline CommandModKeyData::CommandModKeyData()
{
    mnCode = 0L;
}

inline CommandModKeyData::CommandModKeyData( USHORT nCode )
{
    mnCode = nCode;
}

// --------------------
// - CommanDialogData -
// --------------------

#define SHOWDIALOG_ID_PREFERENCES       1
#define SHOWDIALOG_ID_ABOUT             2

class VCL_DLLPUBLIC CommandDialogData
{
    int             m_nDialogId;
    public:
    CommandDialogData( int nDialogId = SHOWDIALOG_ID_PREFERENCES )
    : m_nDialogId( nDialogId )
    {}

    int GetDialogId() const { return m_nDialogId; }
};

// ----------------
// - CommandEvent -
// ----------------

#define COMMAND_CONTEXTMENU             ((USHORT)1)
#define COMMAND_STARTDRAG               ((USHORT)2)
#define COMMAND_WHEEL                   ((USHORT)3)
#define COMMAND_STARTAUTOSCROLL         ((USHORT)4)
#define COMMAND_AUTOSCROLL              ((USHORT)5)
#define COMMAND_VOICE                   ((USHORT)6)
#define COMMAND_STARTEXTTEXTINPUT       ((USHORT)7)
#define COMMAND_EXTTEXTINPUT            ((USHORT)8)
#define COMMAND_ENDEXTTEXTINPUT         ((USHORT)9)
#define COMMAND_INPUTCONTEXTCHANGE      ((USHORT)10)
#define COMMAND_CURSORPOS               ((USHORT)11)
#define COMMAND_PASTESELECTION          ((USHORT)12)
#define COMMAND_MODKEYCHANGE            ((USHORT)13)
#define COMMAND_HANGUL_HANJA_CONVERSION ((USHORT)14)
#define COMMAND_INPUTLANGUAGECHANGE     ((USHORT)15)
#define COMMAND_SHOWDIALOG              ((USHORT)16)

#define COMMAND_USER                    ((USHORT)4096)

class VCL_DLLPUBLIC CommandEvent
{
private:
    Point                               maPos;
    void*                               mpData;
    USHORT                              mnCommand;
    BOOL                                mbMouseEvent;

public:
                                        CommandEvent();
                                        CommandEvent( const Point& rMousePos, USHORT nCmd,
                                                      BOOL bMEvt = FALSE, const void* pCmdData = NULL );

    USHORT                              GetCommand() const { return mnCommand; }
    const Point&                        GetMousePosPixel() const { return maPos; }
    BOOL                                IsMouseEvent() const { return mbMouseEvent; }
    void*                               GetData() const { return mpData; }

    const CommandExtTextInputData*      GetExtTextInputData() const;
    const CommandInputContextData*      GetInputContextChangeData() const;
    const CommandWheelData*             GetWheelData() const;
    const CommandScrollData*            GetAutoScrollData() const;
    const CommandVoiceData*             GetVoiceData() const;
    const CommandModKeyData*            GetModKeyData() const;
    const CommandDialogData*            GetDialogData() const;
};

inline CommandEvent::CommandEvent()
{
    mpData          = NULL;
    mnCommand       = 0;
    mbMouseEvent    = FALSE;
}

inline CommandEvent::CommandEvent( const Point& rMousePos,
                                   USHORT nCmd, BOOL bMEvt, const void* pCmdData ) :
            maPos( rMousePos )
{
    mpData          = (void*)pCmdData;
    mnCommand       = nCmd;
    mbMouseEvent    = bMEvt;
}

inline const CommandExtTextInputData* CommandEvent::GetExtTextInputData() const
{
    if ( mnCommand == COMMAND_EXTTEXTINPUT )
        return (const CommandExtTextInputData*)mpData;
    else
        return NULL;
}

inline const CommandInputContextData* CommandEvent::GetInputContextChangeData() const
{
    if ( mnCommand == COMMAND_INPUTCONTEXTCHANGE )
        return (const CommandInputContextData*)mpData;
    else
        return NULL;
}

inline const CommandWheelData* CommandEvent::GetWheelData() const
{
    if ( mnCommand == COMMAND_WHEEL )
        return (const CommandWheelData*)mpData;
    else
        return NULL;
}

inline const CommandScrollData* CommandEvent::GetAutoScrollData() const
{
    if ( mnCommand == COMMAND_AUTOSCROLL )
        return (const CommandScrollData*)mpData;
    else
        return NULL;
}

inline const CommandVoiceData* CommandEvent::GetVoiceData() const
{
    if ( mnCommand == COMMAND_VOICE )
        return (const CommandVoiceData*)mpData;
    else
        return NULL;
}

inline const CommandModKeyData* CommandEvent::GetModKeyData() const
{
    if( mnCommand == COMMAND_MODKEYCHANGE )
        return (const CommandModKeyData*)mpData;
    else
        return NULL;
}

inline const CommandDialogData* CommandEvent::GetDialogData() const
{
    if( mnCommand == COMMAND_SHOWDIALOG )
        return (const CommandDialogData*)mpData;
    else
        return NULL;
}

#endif // _VCL_CMDEVT_HXX
