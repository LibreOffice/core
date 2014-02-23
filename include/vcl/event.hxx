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
#include <vcl/cmdevt.hxx>

class AllSettings;
class OutputDevice;
class Window;
struct IDataObject;

namespace com { namespace sun { namespace star { namespace awt {
    struct KeyEvent;
    struct MouseEvent;
} } } }

enum TextDirectionality {
    TextDirectionality_LeftToRight_TopToBottom,
    TextDirectionality_RightToLeft_TopToBottom,
    TextDirectionality_TopToBottom_RightToLeft
};


// - KeyEvent -

class VCL_DLLPUBLIC KeyEvent
{
private:
    KeyCode         maKeyCode;
    sal_uInt16          mnRepeat;
    sal_Unicode     mnCharCode;

public:
                    KeyEvent();
                    KeyEvent( sal_Unicode nChar, const KeyCode& rKeyCode,
                              sal_uInt16 nRepeat = 0 );

    sal_Unicode     GetCharCode() const     { return mnCharCode; }
    const KeyCode&  GetKeyCode() const      { return maKeyCode;  }
    sal_uInt16          GetRepeat() const       { return mnRepeat;   }

    KeyEvent        LogicalTextDirectionality (TextDirectionality eMode) const;
                    KeyEvent (const KeyEvent& rKeyEvent);

};

inline KeyEvent::KeyEvent()
{
    mnCharCode  = 0;
    mnRepeat    = 0;
}

inline KeyEvent::KeyEvent( sal_Unicode nChar, const KeyCode& rKeyCode,
                           sal_uInt16 nRepeat ) :
            maKeyCode( rKeyCode )

{
    mnCharCode  = nChar;
    mnRepeat    = nRepeat;
}


// - MouseEvent-Types -


// Maus-Move-Modi
#define MOUSE_SIMPLEMOVE        ((sal_uInt16)0x0001)
#define MOUSE_DRAGMOVE          ((sal_uInt16)0x0002)
#define MOUSE_DRAGCOPY          ((sal_uInt16)0x0004)
#define MOUSE_ENTERWINDOW       ((sal_uInt16)0x0010)
#define MOUSE_LEAVEWINDOW       ((sal_uInt16)0x0020)
#define MOUSE_SYNTHETIC         ((sal_uInt16)0x0040)
#define MOUSE_MODIFIERCHANGED   ((sal_uInt16)0x0080)

// Maus-Button-Down/Up-Modi
#define MOUSE_SIMPLECLICK       ((sal_uInt16)0x0001)
#define MOUSE_SELECT            ((sal_uInt16)0x0002)
#define MOUSE_MULTISELECT       ((sal_uInt16)0x0004)
#define MOUSE_RANGESELECT       ((sal_uInt16)0x0008)

// Maus-Buttons
#define MOUSE_LEFT              ((sal_uInt16)0x0001)
#define MOUSE_MIDDLE            ((sal_uInt16)0x0002)
#define MOUSE_RIGHT             ((sal_uInt16)0x0004)


// - MouseEvent -


class VCL_DLLPUBLIC MouseEvent
{
private:
    Point           maPos;
    sal_uInt16          mnMode;
    sal_uInt16          mnClicks;
    sal_uInt16          mnCode;

public:
    explicit        MouseEvent();
    explicit        MouseEvent( const Point& rPos, sal_uInt16 nClicks = 1,
                                sal_uInt16 nMode = 0, sal_uInt16 nButtons = 0,
                                sal_uInt16 nModifier = 0 );

    const Point&    GetPosPixel() const     { return maPos; }
    sal_uInt16          GetMode() const         { return mnMode; }
                    /** inits this vcl KeyEvent with all settings from the given awt event **/
                    MouseEvent( const ::com::sun::star::awt::MouseEvent& rEvent );

    sal_uInt16          GetClicks() const       { return mnClicks; }

    bool            IsEnterWindow() const
                        { return ((mnMode & MOUSE_ENTERWINDOW) != 0); }
    bool            IsLeaveWindow() const
                        { return ((mnMode & MOUSE_LEAVEWINDOW) != 0); }
    bool            IsSynthetic() const
                        { return ((mnMode & MOUSE_SYNTHETIC) != 0); }
    bool            IsModifierChanged() const
                        { return ((mnMode & MOUSE_MODIFIERCHANGED) != 0); }

    sal_uInt16          GetButtons() const
                        { return (mnCode & (MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)); }
    bool            IsLeft() const
                        { return ((mnCode & MOUSE_LEFT) != 0); }
    bool            IsMiddle() const
                        { return ((mnCode & MOUSE_MIDDLE) != 0); }
    bool            IsRight() const
                        { return ((mnCode & MOUSE_RIGHT) != 0); }

    sal_uInt16          GetModifier() const
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
    mnMode      = 0;
    mnClicks    = 0;
    mnCode      = 0;
}

inline MouseEvent::MouseEvent( const Point& rPos, sal_uInt16 nClicks,
                               sal_uInt16 nMode,
                               sal_uInt16 nButtons, sal_uInt16 nModifier ) :
            maPos( rPos )
{
    mnClicks    = nClicks;
    mnMode      = nMode;
    mnCode      = nButtons | nModifier;
}

class VCL_DLLPUBLIC ZoomEvent
{
private:
    Point           maCenter;
    float           mfScale;

public:
    ZoomEvent() :
        mfScale( 1 )
    {
    }

    ZoomEvent( const Point& rCenter,
               float fScale ) :
        maCenter( rCenter ),
        mfScale( fScale )
    {
    }

    const Point& GetCenter() const
    {
        return maCenter;
    }

    float GetScale() const
    {
        return mfScale;
    }
};

class VCL_DLLPUBLIC ScrollEvent
{
private:
    int mnXOffset;
    int mnYOffset;

public:
    ScrollEvent() :
        mnXOffset( 0 ),
        mnYOffset( 0 )
    {
    }

    ScrollEvent( int xOffset, int yOffset ) :
        mnXOffset( xOffset ),
        mnYOffset( yOffset )
    {
    }

    int GetXOffset() const
    {
        return mnXOffset;
    }

    int GetYOffset() const
    {
        return mnYOffset;
    }
};


// - HelpEvent -


#define HELPMODE_CONTEXT        ((sal_uInt16)0x0001)
#define HELPMODE_EXTENDED       ((sal_uInt16)0x0002)
#define HELPMODE_BALLOON        ((sal_uInt16)0x0004)
#define HELPMODE_QUICK          ((sal_uInt16)0x0008)

class VCL_DLLPUBLIC HelpEvent
{
private:
    Point           maPos;
    sal_uInt16          mnMode;
    bool            mbKeyboardActivated;

public:
    explicit        HelpEvent();
    explicit        HelpEvent( sal_uInt16 nHelpMode );
    explicit        HelpEvent( const Point& rMousePos, sal_uInt16 nHelpMode );

    const Point&    GetMousePosPixel() const;
    sal_uInt16          GetMode() const { return mnMode; }
    bool            KeyboardActivated() const { return mbKeyboardActivated; }
    void            SetKeyboardActivated( bool bKeyboard ) { mbKeyboardActivated = bKeyboard; }
};

inline HelpEvent::HelpEvent()
{
    mnMode  = HELPMODE_CONTEXT;
    mbKeyboardActivated = true;
}

inline HelpEvent::HelpEvent( const Point& rMousePos, sal_uInt16 nHelpMode ) :
            maPos( rMousePos )
{
    mnMode  = nHelpMode;
    mbKeyboardActivated = false;
}

inline HelpEvent::HelpEvent( sal_uInt16 nHelpMode )
{
    mnMode  = nHelpMode;
    mbKeyboardActivated = true;
}


// - UserDrawEvent -


class VCL_DLLPUBLIC UserDrawEvent
{
private:
    OutputDevice*       mpOutDev;
    Rectangle           maOutRect;
    sal_uInt16              mnItemId;
    sal_uInt16              mnStyle;

public:
                        UserDrawEvent();
                        UserDrawEvent( OutputDevice* pOut,
                                       const Rectangle& rOutRect,
                                       sal_uInt16 nId, sal_uInt16 nStyle = 0 );

    OutputDevice*       GetDevice() const { return mpOutDev; }
    const Rectangle&    GetRect() const { return maOutRect; }
    sal_uInt16              GetItemId() const { return mnItemId; }
    sal_uInt16              GetStyle() const { return mnStyle; }
};

inline UserDrawEvent::UserDrawEvent()
{
    mpOutDev    = NULL;
    mnItemId    = 0;
    mnStyle     = 0;
}

inline UserDrawEvent::UserDrawEvent( OutputDevice* pOut,
                                     const Rectangle& rOutRect,
                                     sal_uInt16 nId, sal_uInt16 nStyle ) :
            maOutRect( rOutRect )
{
    mpOutDev    = pOut;
    mnItemId    = nId;
    mnStyle     = nStyle;
}


// - Tracking-Types -


#define ENDTRACK_CANCEL         ((sal_uInt16)0x0001)
#define ENDTRACK_KEY            ((sal_uInt16)0x0002)
#define ENDTRACK_FOCUS          ((sal_uInt16)0x0004)
#define ENDTRACK_END            ((sal_uInt16)0x1000)
#define ENDTRACK_DONTCALLHDL    ((sal_uInt16)0x8000)

#define TRACKING_REPEAT         ((sal_uInt16)0x0100)


// - TrackingEvent -


class VCL_DLLPUBLIC TrackingEvent
{
private:
    MouseEvent          maMEvt;
    sal_uInt16              mnFlags;

public:
    explicit            TrackingEvent();
    explicit            TrackingEvent( const MouseEvent&,
                                       sal_uInt16 nTrackFlags = 0 );

    const MouseEvent&   GetMouseEvent() const { return maMEvt; }

    bool                IsTrackingRepeat() const
                            { return ((mnFlags & TRACKING_REPEAT) != 0); }

    bool                IsTrackingEnded() const
                            { return ((mnFlags & ENDTRACK_END) != 0); }
    bool                IsTrackingCanceled() const
                            { return ((mnFlags & ENDTRACK_CANCEL) != 0); }
    sal_uInt16              GetTrackingFlags() const { return mnFlags; }
};

inline TrackingEvent::TrackingEvent()
{
    mnFlags = 0;
}

inline TrackingEvent::TrackingEvent( const MouseEvent& rMEvt,
                                     sal_uInt16 nTrackFlags ) :
            maMEvt( rMEvt )
{
    mnFlags = nTrackFlags;
}


// - NotifyEvent -


#define EVENT_MOUSEBUTTONDOWN   1
#define EVENT_MOUSEBUTTONUP     2
#define EVENT_MOUSEMOVE         3
#define EVENT_KEYINPUT          4
#define EVENT_KEYUP             5
#define EVENT_GETFOCUS          6
#define EVENT_LOSEFOCUS         7
#define EVENT_COMMAND           8
#define EVENT_DESTROY           9
#define EVENT_INPUTENABLE       10
#define EVENT_INPUTDISABLE      11
#define EVENT_EXECUTEDIALOG     100
#define EVENT_ENDEXECUTEDIALOG  101

class VCL_DLLPUBLIC NotifyEvent
{
private:
    Window*                 mpWindow;
    void*                   mpData;
    sal_uInt16                  mnType;
    long                    mnRetValue;

public:
                            NotifyEvent();
                            NotifyEvent( sal_uInt16 nType,
                                         Window* pWindow,
                                         const void* pEvent = NULL,
                                         long nRet = 0 );

    sal_uInt16                  GetType() const { return mnType; }
    Window*                 GetWindow() const { return mpWindow; }
    void*                   GetData() const { return mpData; }

    void                    SetReturnValue( long nRet ) { mnRetValue = nRet; }
    long                    GetReturnValue() const { return mnRetValue; }

    const KeyEvent*         GetKeyEvent() const;
    const MouseEvent*       GetMouseEvent() const;
    const CommandEvent*     GetCommandEvent() const;
};

inline NotifyEvent::NotifyEvent()
{
    mpWindow    = NULL;
    mpData      = NULL;
    mnType      = 0;
    mnRetValue  = 0;
}

inline NotifyEvent::NotifyEvent( sal_uInt16 nType, Window* pWindow,
                                 const void* pEvent, long nRet )
{
    mpWindow    = pWindow;
    mpData      = (void*)pEvent;
    mnType      = nType;
    mnRetValue  = nRet;
}

inline const KeyEvent* NotifyEvent::GetKeyEvent() const
{
    if ( (mnType == EVENT_KEYINPUT) || (mnType == EVENT_KEYUP) )
        return (const KeyEvent*)mpData;
    else
        return NULL;
}

inline const MouseEvent* NotifyEvent::GetMouseEvent() const
{
    if ( (mnType >= EVENT_MOUSEBUTTONDOWN) && (mnType <= EVENT_MOUSEMOVE) )
        return (const MouseEvent*)mpData;
    else
        return NULL;
}

inline const CommandEvent* NotifyEvent::GetCommandEvent() const
{
    if ( mnType == EVENT_COMMAND )
        return (const CommandEvent*)mpData;
    else
        return NULL;
}


// - DataChangedEvent -


#define DATACHANGED_SETTINGS            ((sal_uInt16)1)
#define DATACHANGED_DISPLAY             ((sal_uInt16)2)
#define DATACHANGED_DATETIME            ((sal_uInt16)3)
#define DATACHANGED_FONTS               ((sal_uInt16)4)
#define DATACHANGED_PRINTER             ((sal_uInt16)5)
#define DATACHANGED_FONTSUBSTITUTION    ((sal_uInt16)6)
#define DATACHANGED_USER                ((sal_uInt16)10000)

class VCL_DLLPUBLIC DataChangedEvent
{
private:
    void*                   mpData;
    sal_uLong                   mnFlags;
    sal_uInt16                  mnType;

public:
    explicit                DataChangedEvent();
    explicit                DataChangedEvent( sal_uInt16 nType,
                                              const void* pData = NULL,
                                              sal_uLong nFlags = 0 );

    sal_uInt16                  GetType() const { return mnType; }
    void*                   GetData() const { return mpData; }
    sal_uLong                   GetFlags() const { return mnFlags; }

    const AllSettings*      GetOldSettings() const;
};

inline DataChangedEvent::DataChangedEvent()
{
    mpData  = NULL;
    mnFlags = 0;
    mnType  = 0;
}

inline DataChangedEvent::DataChangedEvent( sal_uInt16 nType,
                                           const void* pData,
                                           sal_uLong nChangeFlags )
{
    mpData  = (void*)pData;
    mnFlags = nChangeFlags;
    mnType  = nType;
}

inline const AllSettings* DataChangedEvent::GetOldSettings() const
{
    if ( mnType == DATACHANGED_SETTINGS )
        return (const AllSettings*)mpData;
    else
        return NULL;
}

#endif // INCLUDED_VCL_EVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
