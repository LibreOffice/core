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

#ifndef INCLUDED_VCL_EVENT_HXX
#define INCLUDED_VCL_EVENT_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <vcl/keycod.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/settings.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/outdev.hxx>
#include <vcl/window.hxx>

class AllSettings;
struct IDataObject;

namespace com { namespace sun { namespace star { namespace awt {
    struct KeyEvent;
    struct MouseEvent;
} } } }

enum class TextDirectionality {
    LeftToRight_TopToBottom,
    RightToLeft_TopToBottom,
    TopToBottom_RightToLeft,
    BottomToTop_LeftToRight
};

class VCL_DLLPUBLIC KeyEvent
{
private:
    vcl::KeyCode    maKeyCode;
    sal_uInt16      mnRepeat;
    sal_Unicode     mnCharCode;

public:
                    KeyEvent();
                    KeyEvent( sal_Unicode nChar, const vcl::KeyCode& rKeyCode,
                              sal_uInt16 nRepeat = 0 );

    sal_Unicode         GetCharCode() const     { return mnCharCode; }
    const vcl::KeyCode& GetKeyCode() const      { return maKeyCode;  }
    sal_uInt16          GetRepeat() const       { return mnRepeat;   }

    KeyEvent        LogicalTextDirectionality (TextDirectionality eMode) const;
};

inline KeyEvent::KeyEvent()
{
    mnCharCode  = 0;
    mnRepeat    = 0;
}

inline KeyEvent::KeyEvent( sal_Unicode nChar, const vcl::KeyCode& rKeyCode,
                           sal_uInt16 nRepeat ) :
            maKeyCode( rKeyCode )

{
    mnCharCode  = nChar;
    mnRepeat    = nRepeat;
}


enum class MouseEventModifiers
{
   NONE              = 0,
   // mouse move modifiers
   SIMPLEMOVE        = 0x0001,
   DRAGMOVE          = 0x0002,
   DRAGCOPY          = 0x0004,
   ENTERWINDOW       = 0x0010,
   LEAVEWINDOW       = 0x0020,
   SYNTHETIC         = 0x0040,
   MODIFIERCHANGED   = 0x0080,
   // mouse up/down-button modifiers
   SIMPLECLICK       = 0x0100,
   SELECT            = 0x0200,
   MULTISELECT       = 0x0400,
   RANGESELECT       = 0x0800
};
namespace o3tl
{
    template<> struct typed_flags<MouseEventModifiers> : is_typed_flags<MouseEventModifiers, 0xff7> {};
}

// Mouse buttons
#define MOUSE_LEFT              (sal_uInt16(0x0001))
#define MOUSE_MIDDLE            (sal_uInt16(0x0002))
#define MOUSE_RIGHT             (sal_uInt16(0x0004))


class VCL_DLLPUBLIC MouseEvent
{
private:
    Point               maPos;
    MouseEventModifiers mnMode;
    sal_uInt16          mnClicks;
    sal_uInt16          mnCode;

public:
    explicit        MouseEvent();
    explicit        MouseEvent( const Point& rPos, sal_uInt16 nClicks = 1,
                                MouseEventModifiers nMode = MouseEventModifiers::NONE, sal_uInt16 nButtons = 0,
                                sal_uInt16 nModifier = 0 );

    const Point&    GetPosPixel() const     { return maPos; }
    MouseEventModifiers GetMode() const         { return mnMode; }

    sal_uInt16      GetClicks() const       { return mnClicks; }

    bool            IsEnterWindow() const
                        { return bool(mnMode & MouseEventModifiers::ENTERWINDOW); }
    bool            IsLeaveWindow() const
                        { return bool(mnMode & MouseEventModifiers::LEAVEWINDOW); }
    bool            IsSynthetic() const
                        { return bool(mnMode & MouseEventModifiers::SYNTHETIC); }
    bool            IsModifierChanged() const
                        { return bool(mnMode & MouseEventModifiers::MODIFIERCHANGED); }

    sal_uInt16      GetButtons() const
                        { return (mnCode & (MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)); }
    bool            IsLeft() const
                        { return ((mnCode & MOUSE_LEFT) != 0); }
    bool            IsMiddle() const
                        { return ((mnCode & MOUSE_MIDDLE) != 0); }
    bool            IsRight() const
                        { return ((mnCode & MOUSE_RIGHT) != 0); }

    sal_uInt16      GetModifier() const
                        { return (mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)); }
    bool            IsShift() const
                        { return ((mnCode & KEY_SHIFT) != 0); }
    bool            IsMod1() const
                        { return ((mnCode & KEY_MOD1) != 0); }
    bool            IsMod2() const
                        { return ((mnCode & KEY_MOD2) != 0); }
    bool            IsMod3() const
                        { return ((mnCode & KEY_MOD3) != 0); }
};

inline MouseEvent::MouseEvent()
{
    mnMode      = MouseEventModifiers::NONE;
    mnClicks    = 0;
    mnCode      = 0;
}

inline MouseEvent::MouseEvent( const Point& rPos, sal_uInt16 nClicks,
                               MouseEventModifiers nMode,
                               sal_uInt16 nButtons, sal_uInt16 nModifier ) :
            maPos( rPos )
{
    mnClicks    = nClicks;
    mnMode      = nMode;
    mnCode      = nButtons | nModifier;
}

enum class HelpEventMode
{
    NONE           = 0x0000,
    CONTEXT        = 0x0001,
    EXTENDED       = 0x0002,
    BALLOON        = 0x0004,
    QUICK          = 0x0008
};
namespace o3tl
{
    template<> struct typed_flags<HelpEventMode> : is_typed_flags<HelpEventMode, 0x0f> {};
}

class VCL_DLLPUBLIC HelpEvent
{
private:
    Point           maPos;
    HelpEventMode   mnMode;
    bool            mbKeyboardActivated;

public:
    explicit        HelpEvent( const Point& rMousePos, HelpEventMode nHelpMode );

    const Point&    GetMousePosPixel() const { return maPos; }
    HelpEventMode   GetMode() const { return mnMode; }
    bool            KeyboardActivated() const { return mbKeyboardActivated; }
    void            SetKeyboardActivated( bool bKeyboard ) { mbKeyboardActivated = bKeyboard; }
};

inline HelpEvent::HelpEvent( const Point& rMousePos, HelpEventMode nHelpMode ) :
            maPos( rMousePos )
{
    mnMode  = nHelpMode;
    mbKeyboardActivated = false;
}

/// Event to pass information for UserDraw() handling eg. in comboboxes.
class VCL_DLLPUBLIC UserDrawEvent
{
private:
    /// Window that owns the user draw.
    VclPtr<vcl::Window> mpWindow;

    /// RenderContext to which we should draw - can be a VirtualDevice or anything.
    VclPtr<vcl::RenderContext> mpRenderContext;

    tools::Rectangle           maOutRect;
    sal_uInt16          mnItemId;
    sal_uInt16          mnStyle;

public:
    UserDrawEvent(vcl::Window* pWindow, vcl::RenderContext* pRenderContext,
            const tools::Rectangle& rOutRect, sal_uInt16 nId, sal_uInt16 nStyle = 0);

    vcl::Window*        GetWindow() const { return mpWindow; }
    vcl::RenderContext* GetRenderContext() const { return mpRenderContext; }
    const tools::Rectangle&    GetRect() const { return maOutRect; }
    sal_uInt16          GetItemId() const { return mnItemId; }
    sal_uInt16          GetStyle() const { return mnStyle; }
};

inline UserDrawEvent::UserDrawEvent(vcl::Window* pWindow, vcl::RenderContext* pRenderContext,
        const tools::Rectangle& rOutRect, sal_uInt16 nId, sal_uInt16 nStyle)
    : mpWindow(pWindow)
    , mpRenderContext(pRenderContext)
    , maOutRect( rOutRect )
    , mnItemId(nId)
    , mnStyle(nStyle)
{
}


class VCL_DLLPUBLIC TrackingEvent
{
private:
    MouseEvent          maMEvt;
    TrackingEventFlags  mnFlags;

public:
    explicit            TrackingEvent( const MouseEvent&,
                                       TrackingEventFlags nTrackFlags = TrackingEventFlags::NONE );

    const MouseEvent&   GetMouseEvent() const { return maMEvt; }

    bool                IsTrackingRepeat() const
                            { return bool(mnFlags & TrackingEventFlags::Repeat); }
    bool                IsTrackingEnded() const
                            { return bool(mnFlags & TrackingEventFlags::End); }
    bool                IsTrackingCanceled() const
                            { return bool(mnFlags & TrackingEventFlags::Cancel); }
};

inline TrackingEvent::TrackingEvent( const MouseEvent& rMEvt,
                                     TrackingEventFlags nTrackFlags ) :
            maMEvt( rMEvt )
{
    mnFlags = nTrackFlags;
}


enum class MouseNotifyEvent
{
    NONE             = 0,
    MOUSEBUTTONDOWN  = 1,
    MOUSEBUTTONUP    = 2,
    MOUSEMOVE        = 3,
    KEYINPUT         = 4,
    KEYUP            = 5,
    GETFOCUS         = 6,
    LOSEFOCUS        = 7,
    COMMAND          = 8,
    DESTROY          = 9,
    INPUTENABLE      = 10,
    INPUTDISABLE     = 11,
    EXECUTEDIALOG    = 100,
    ENDEXECUTEDIALOG = 101
};

class VCL_DLLPUBLIC NotifyEvent
{
private:
    VclPtr<vcl::Window>     mpWindow;
    void*                   mpData;
    MouseNotifyEvent        mnEventType;

public:
                            NotifyEvent( MouseNotifyEvent nEventType,
                                         vcl::Window* pWindow,
                                         const void* pEvent = nullptr );

    MouseNotifyEvent        GetType() const { return mnEventType; }
    vcl::Window*            GetWindow() const { return mpWindow; }
    void*                   GetData() const { return mpData; }
    const KeyEvent*         GetKeyEvent() const;
    const MouseEvent*       GetMouseEvent() const;
    const CommandEvent*     GetCommandEvent() const;
};

inline const KeyEvent* NotifyEvent::GetKeyEvent() const
{
    if ( (mnEventType == MouseNotifyEvent::KEYINPUT) || (mnEventType == MouseNotifyEvent::KEYUP) )
        return static_cast<const KeyEvent*>(mpData);
    else
        return nullptr;
}

inline const MouseEvent* NotifyEvent::GetMouseEvent() const
{
    if ( (mnEventType >= MouseNotifyEvent::MOUSEBUTTONDOWN) && (mnEventType <= MouseNotifyEvent::MOUSEMOVE) )
        return static_cast<const MouseEvent*>(mpData);
    else
        return nullptr;
}

inline const CommandEvent* NotifyEvent::GetCommandEvent() const
{
    if ( mnEventType == MouseNotifyEvent::COMMAND )
        return static_cast<const CommandEvent*>(mpData);
    else
        return nullptr;
}


enum class DataChangedEventType {
    NONE               = 0,
    SETTINGS           = 1,
    DISPLAY            = 2,
    FONTS              = 4,
    PRINTER            = 5,
    FONTSUBSTITUTION   = 6,
    USER               = 10000
};

class VCL_DLLPUBLIC DataChangedEvent
{
private:
    void*                   mpData;
    AllSettingsFlags        mnFlags;
    DataChangedEventType    mnType;

public:
    explicit                DataChangedEvent( DataChangedEventType nType,
                                              const void* pData = nullptr,
                                              AllSettingsFlags nFlags = AllSettingsFlags::NONE );

    DataChangedEventType    GetType() const { return mnType; }
    AllSettingsFlags        GetFlags() const { return mnFlags; }

    const AllSettings*      GetOldSettings() const;
};

inline DataChangedEvent::DataChangedEvent( DataChangedEventType nType,
                                           const void* pData,
                                           AllSettingsFlags nChangeFlags )
{
    mpData  = const_cast<void*>(pData);
    mnFlags = nChangeFlags;
    mnType  = nType;
}

inline const AllSettings* DataChangedEvent::GetOldSettings() const
{
    if ( mnType == DataChangedEventType::SETTINGS )
        return static_cast<const AllSettings*>(mpData);
    else
        return nullptr;
}

#endif // INCLUDED_VCL_EVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
