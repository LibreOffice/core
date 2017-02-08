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

#ifndef INCLUDED_VCL_COMMANDEVENT_HXX
#define INCLUDED_VCL_COMMANDEVENT_HXX

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/keycod.hxx>
#include <vcl/font.hxx>
#include <o3tl/typed_flags_set.hxx>

class CommandExtTextInputData;
class CommandWheelData;
class CommandScrollData;
class CommandModKeyData;
class CommandDialogData;
class CommandMediaData;
class CommandSelectionChangeData;
class CommandSwipeData;
class CommandLongPressData;
enum class CommandEventId;

enum class ExtTextInputAttr {
    NONE                  = 0x0000,
    GrayWaveline          = 0x0100,
    Underline             = 0x0200,
    BoldUnderline         = 0x0400,
    DottedUnderline       = 0x0800,
    DashDotUnderline      = 0x1000,
    Highlight             = 0x2000,
    RedText               = 0x4000,
    HalfToneText          = 0x8000
};
namespace o3tl
{
    template<> struct typed_flags<ExtTextInputAttr> : is_typed_flags<ExtTextInputAttr, 0xff00> {};
}

#define EXTTEXTINPUT_CURSOR_INVISIBLE           ((sal_uInt16)0x0001)
#define EXTTEXTINPUT_CURSOR_OVERWRITE           ((sal_uInt16)0x0002)


class VCL_DLLPUBLIC CommandEvent
{
private:
    Point                               maPos;
    void*                               mpData;
    CommandEventId                      mnCommand;
    bool                                mbMouseEvent;

public:
                                        CommandEvent();
                                        CommandEvent( const Point& rMousePos, CommandEventId nCmd,
                                                      bool bMEvt = false, const void* pCmdData = nullptr );

    CommandEventId                      GetCommand() const { return mnCommand; }
    const Point&                        GetMousePosPixel() const { return maPos; }
    bool                                IsMouseEvent() const { return mbMouseEvent; }
    void*                               GetEventData() const { return mpData; }

    const CommandExtTextInputData*      GetExtTextInputData() const;
    const CommandWheelData*             GetWheelData() const;
    const CommandScrollData*            GetAutoScrollData() const;
    const CommandModKeyData*            GetModKeyData() const;
    const CommandDialogData*            GetDialogData() const;
          CommandMediaData*             GetMediaData() const;
    const CommandSelectionChangeData*   GetSelectionChangeData() const;
    const CommandSwipeData*             GetSwipeData() const;
    const CommandLongPressData*         GetLongPressData() const;
};

class VCL_DLLPUBLIC CommandExtTextInputData
{
private:
    OUString            maText;
    std::unique_ptr<ExtTextInputAttr[]> mpTextAttr;
    sal_Int32           mnCursorPos;
    sal_uInt16          mnCursorFlags;
    bool                mbOnlyCursor;

public:
                        CommandExtTextInputData( const OUString& rText,
                                                 const ExtTextInputAttr* pTextAttr,
                                                 sal_Int32 nCursorPos,
                                                 sal_uInt16 nCursorFlags,
                                                 bool bOnlyCursor );
                        CommandExtTextInputData( const CommandExtTextInputData& rData );
                        ~CommandExtTextInputData();

    const OUString&     GetText() const { return maText; }
    const ExtTextInputAttr* GetTextAttr() const { return mpTextAttr.get(); }

    sal_Int32           GetCursorPos() const { return mnCursorPos; }
    bool                IsCursorVisible() const { return (mnCursorFlags & EXTTEXTINPUT_CURSOR_INVISIBLE) == 0; }
    bool                IsCursorOverwrite() const { return (mnCursorFlags & EXTTEXTINPUT_CURSOR_OVERWRITE) != 0; }
    bool                IsOnlyCursorChanged() const { return mbOnlyCursor; }
};

class VCL_DLLPUBLIC CommandInputContextData
{
private:
    LanguageType    meLanguage;

public:
                    CommandInputContextData( LanguageType eLang );
};

enum class CommandWheelMode
{
    NONE              = 0,
    SCROLL            = 1,
    ZOOM              = 2,
    ZOOM_SCALE        = 3,
    DATAZOOM          = 4
};

// Magic value used in mnLines field in CommandWheelData
#define COMMAND_WHEEL_PAGESCROLL        ((sal_uLong)0xFFFFFFFF)

class VCL_DLLPUBLIC CommandWheelData
{
private:
    long              mnDelta;
    long              mnNotchDelta;
    sal_uLong         mnLines;
    CommandWheelMode  mnWheelMode;
    sal_uInt16        mnCode;
    bool              mbHorz;
    bool              mbDeltaIsPixel;

public:
                    CommandWheelData();
                    CommandWheelData( long nWheelDelta, long nWheelNotchDelta,
                                      sal_uLong nScrollLines,
                                      CommandWheelMode nWheelMode, sal_uInt16 nKeyModifier,
                                      bool bHorz, bool bDeltaIsPixel = false );

    long            GetDelta() const { return mnDelta; }
    long            GetNotchDelta() const { return mnNotchDelta; }
    sal_uLong       GetScrollLines() const { return mnLines; }
    bool            IsHorz() const { return mbHorz; }
    bool            IsDeltaPixel() const { return mbDeltaIsPixel; }

    CommandWheelMode GetMode() const { return mnWheelMode; }

    sal_uInt16      GetModifier() const
                        { return (mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)); }
    bool            IsShift() const
                        { return ((mnCode & KEY_SHIFT) != 0); }
    bool            IsMod1() const
                        { return ((mnCode & KEY_MOD1) != 0); }
    bool            IsMod2() const
                        { return ((mnCode & KEY_MOD2) != 0); }
};

class VCL_DLLPUBLIC CommandScrollData
{
private:
    long            mnDeltaX;
    long            mnDeltaY;

public:
                    CommandScrollData( long nDeltaX, long nDeltaY );

    long            GetDeltaX() const { return mnDeltaX; }
    long            GetDeltaY() const { return mnDeltaY; }
};

class VCL_DLLPUBLIC CommandModKeyData
{
private:
    ModKeyFlags     mnCode;

public:
                    CommandModKeyData( ModKeyFlags nCode );

    bool            IsMod1()       const { return bool(mnCode & ModKeyFlags::Mod1Msk); }
    bool            IsMod2()       const { return bool(mnCode & ModKeyFlags::Mod2Msk); }
    bool            IsLeftShift()  const { return bool(mnCode & ModKeyFlags::LeftShift); }
    bool            IsRightShift() const { return bool(mnCode & ModKeyFlags::RightShift); }
};

enum class ShowDialogId
{
    Preferences       = 1,
    About             = 2,
};

class VCL_DLLPUBLIC CommandDialogData
{
    ShowDialogId   m_nDialogId;
public:
    CommandDialogData( ShowDialogId nDialogId )
    : m_nDialogId( nDialogId )
    {}

    ShowDialogId GetDialogId() const { return m_nDialogId; }
};

// Media Commands
enum class MediaCommand
{
    ChannelDown           = 1, // Decrement the channel value, for example, for a TV or radio tuner.
    ChannelUp             = 2, // Increment the channel value, for example, for a TV or radio tuner.
    NextTrack             = 3, // Go to next media track/slide.
    Pause                 = 4, // Pause. If already paused, take no further action. This is a direct PAUSE command that has no state.
    Play                  = 5, // Begin playing at the current position. If already paused, it will resume. This is a direct PLAY command that has no state.
    PlayPause             = 6, // Play or pause playback.
    PreviousTrack         = 7, // Go to previous media track/slide.
    Record                = 8, // Begin recording the current stream.
    Rewind                = 9,// Go backward in a stream at a higher rate of speed.
    Stop                  = 10,// Stop playback.
    MicOnOffToggle        = 11,// Toggle the microphone.
    MicrophoneVolumeDown  = 12,// Increase microphone volume.
    MicrophoneVolumeMute  = 13,// Mute the microphone.
    MicrophoneVolumeUp    = 14,// Decrease microphone volume.
    VolumeDown            = 15,// Lower the volume.
    VolumeMute            = 16,// Mute the volume.
    VolumeUp              = 17,// Raise the volume.
    Menu                  = 18,// Button Menu pressed.
    PlayHold              = 20,// Button Play (long) pressed.
    NextTrackHold         = 21,// Button Right holding pressed.
};

class VCL_DLLPUBLIC CommandMediaData
{
    MediaCommand m_nMediaId;
    bool m_bPassThroughToOS;
public:
    CommandMediaData(MediaCommand nMediaId)
        : m_nMediaId(nMediaId)
        , m_bPassThroughToOS(true)
    {
    }
    MediaCommand GetMediaId() const { return m_nMediaId; }
    void SetPassThroughToOS(bool bPassThroughToOS) { m_bPassThroughToOS = bPassThroughToOS; }
    bool GetPassThroughToOS() const { return m_bPassThroughToOS; }
};

class VCL_DLLPUBLIC CommandSelectionChangeData
{
private:
    sal_uLong          mnStart;
    sal_uLong          mnEnd;

public:
    CommandSelectionChangeData( sal_uLong nStart, sal_uLong nEnd );

    sal_uLong          GetStart() const { return mnStart; }
    sal_uLong          GetEnd() const { return mnEnd; }
};

class VCL_DLLPUBLIC CommandSwipeData
{
    double mnVelocityX;
public:
    CommandSwipeData()
        : mnVelocityX(0)
    {
    }
    CommandSwipeData(double nVelocityX)
        : mnVelocityX(nVelocityX)
    {
    }
    double getVelocityX() const { return mnVelocityX; }
};


class VCL_DLLPUBLIC CommandLongPressData
{
    double mnX;
    double mnY;
public:
    CommandLongPressData()
        : mnX(0)
        , mnY(0)
    {
    }
    CommandLongPressData(double nX, double nY)
        : mnX(nX)
        , mnY(nY)
    {
    }
    double getX() const { return mnX; }
    double getY() const { return mnY; }
};

enum class CommandEventId
{
    NONE                    = 0,
    ContextMenu             = 1,
    StartDrag               = 2,
    Wheel                   = 3,
    StartAutoScroll         = 4,
    AutoScroll              = 5,
    StartExtTextInput       = 7,
    ExtTextInput            = 8,
    EndExtTextInput         = 9,
    InputContextChange      = 10,
    CursorPos               = 11,
    PasteSelection          = 12,
    ModKeyChange            = 13,
    InputLanguageChange     = 15,
    ShowDialog              = 16,
    Media                   = 17,
    SelectionChange         = 18,
    PrepareReconversion     = 19,
    QueryCharPosition       = 20,
    Swipe                   = 21,
    LongPress               = 22,
};

#endif // INCLUDED_VCL_COMMANDEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
