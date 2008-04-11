/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: event.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SV_EVENT_HXX
#define _SV_EVENT_HXX

#include <vcl/sv.h>
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

// ------------
// - KeyEvent -
// ------------
class VCL_DLLPUBLIC KeyEvent
{
private:
    KeyCode         maKeyCode;
    USHORT          mnRepeat;
    xub_Unicode     mnCharCode;

public:
                    KeyEvent();
                    KeyEvent( xub_Unicode nChar, const KeyCode& rKeyCode,
                              USHORT nRepeat = 0 );

                    /** inits this vcl KeyEvent with all settings from the given awt event **/
                    KeyEvent( const ::com::sun::star::awt::KeyEvent& rEvent );

    /** fills out the given awt KeyEvent with all settings from this vcl event **/
    void InitKeyEvent( ::com::sun::star::awt::KeyEvent& rEvent ) const;

    xub_Unicode     GetCharCode() const     { return mnCharCode; }
    const KeyCode&  GetKeyCode() const      { return maKeyCode;  }
    USHORT          GetRepeat() const       { return mnRepeat;   }

    KeyEvent        LogicalTextDirectionality (TextDirectionality eMode) const;
                    KeyEvent (const KeyEvent& rKeyEvent);

};

inline KeyEvent::KeyEvent()
{
    mnCharCode  = 0;
    mnRepeat    = 0;
}

inline KeyEvent::KeyEvent( xub_Unicode nChar, const KeyCode& rKeyCode,
                           USHORT nRepeat ) :
            maKeyCode( rKeyCode )

{
    mnCharCode  = nChar;
    mnRepeat    = nRepeat;
}

// --------------------
// - MouseEvent-Types -
// --------------------

// Maus-Move-Modi
#define MOUSE_SIMPLEMOVE        ((USHORT)0x0001)
#define MOUSE_DRAGMOVE          ((USHORT)0x0002)
#define MOUSE_DRAGCOPY          ((USHORT)0x0004)
#define MOUSE_ENTERWINDOW       ((USHORT)0x0010)
#define MOUSE_LEAVEWINDOW       ((USHORT)0x0020)
#define MOUSE_SYNTHETIC         ((USHORT)0x0040)
#define MOUSE_MODIFIERCHANGED   ((USHORT)0x0080)

// Maus-Button-Down/Up-Modi
#define MOUSE_SIMPLECLICK       ((USHORT)0x0001)
#define MOUSE_SELECT            ((USHORT)0x0002)
#define MOUSE_MULTISELECT       ((USHORT)0x0004)
#define MOUSE_RANGESELECT       ((USHORT)0x0008)

// Maus-Buttons
#define MOUSE_LEFT              ((USHORT)0x0001)
#define MOUSE_MIDDLE            ((USHORT)0x0002)
#define MOUSE_RIGHT             ((USHORT)0x0004)

// --------------
// - MouseEvent -
// --------------

class VCL_DLLPUBLIC MouseEvent
{
private:
    Point           maPos;
    USHORT          mnMode;
    USHORT          mnClicks;
    USHORT          mnCode;

public:
                    MouseEvent();
                    MouseEvent( const Point& rPos, USHORT nClicks = 1,
                                USHORT nMode = 0, USHORT nButtons = 0,
                                USHORT nModifier = 0 );

    const Point&    GetPosPixel() const     { return maPos; }
    USHORT          GetMode() const         { return mnMode; }
                    /** inits this vcl KeyEvent with all settings from the given awt event **/
                    MouseEvent( const ::com::sun::star::awt::MouseEvent& rEvent );

    /** fills out the given awt KeyEvent with all settings from this vcl event **/
    void InitMouseEvent( ::com::sun::star::awt::MouseEvent& rEvent ) const;

    USHORT          GetClicks() const       { return mnClicks; }

    BOOL            IsEnterWindow() const
                        { return ((mnMode & MOUSE_ENTERWINDOW) != 0); }
    BOOL            IsLeaveWindow() const
                        { return ((mnMode & MOUSE_LEAVEWINDOW) != 0); }
    BOOL            IsSynthetic() const
                        { return ((mnMode & MOUSE_SYNTHETIC) != 0); }
    BOOL            IsModifierChanged() const
                        { return ((mnMode & MOUSE_MODIFIERCHANGED) != 0); }

    USHORT          GetButtons() const
                        { return (mnCode & (MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)); }
    BOOL            IsLeft() const
                        { return ((mnCode & MOUSE_LEFT) != 0); }
    BOOL            IsMiddle() const
                        { return ((mnCode & MOUSE_MIDDLE) != 0); }
    BOOL            IsRight() const
                        { return ((mnCode & MOUSE_RIGHT) != 0); }

    USHORT          GetModifier() const
                        { return (mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)); }
    BOOL            IsShift() const
                        { return ((mnCode & KEY_SHIFT) != 0); }
    BOOL            IsMod1() const
                        { return ((mnCode & KEY_MOD1) != 0); }
    BOOL            IsMod2() const
                        { return ((mnCode & KEY_MOD2) != 0); }
};

inline MouseEvent::MouseEvent()
{
    mnMode      = 0;
    mnClicks    = 0;
    mnCode      = 0;
}

inline MouseEvent::MouseEvent( const Point& rPos, USHORT nClicks,
                               USHORT nMode,
                               USHORT nButtons, USHORT nModifier ) :
            maPos( rPos )
{
    mnClicks    = nClicks;
    mnMode      = nMode;
    mnCode      = nButtons | nModifier;
}

// -------------
// - HelpEvent -
// -------------

#define HELPMODE_CONTEXT        ((USHORT)0x0001)
#define HELPMODE_EXTENDED       ((USHORT)0x0002)
#define HELPMODE_BALLOON        ((USHORT)0x0004)
#define HELPMODE_QUICK          ((USHORT)0x0008)

class VCL_DLLPUBLIC HelpEvent
{
private:
    Point           maPos;
    USHORT          mnMode;
    BOOL            mbKeyboardActivated;

public:
                    HelpEvent();
                    HelpEvent( USHORT nHelpMode );
                    HelpEvent( const Point& rMousePos, USHORT nHelpMode );

    const Point&    GetMousePosPixel() const;
    USHORT          GetMode() const { return mnMode; }
    BOOL            KeyboardActivated() const { return mbKeyboardActivated; }
    void            SetKeyboardActivated( BOOL bKeyboard ) { mbKeyboardActivated = bKeyboard; }
};

inline HelpEvent::HelpEvent()
{
    mnMode  = HELPMODE_CONTEXT;
    mbKeyboardActivated = TRUE;
}

inline HelpEvent::HelpEvent( const Point& rMousePos, USHORT nHelpMode ) :
            maPos( rMousePos )
{
    mnMode  = nHelpMode;
    mbKeyboardActivated = FALSE;
}

inline HelpEvent::HelpEvent( USHORT nHelpMode )
{
    mnMode  = nHelpMode;
    mbKeyboardActivated = TRUE;
}

// -----------------
// - UserDrawEvent -
// -----------------

class VCL_DLLPUBLIC UserDrawEvent
{
private:
    OutputDevice*       mpOutDev;
    Rectangle           maOutRect;
    USHORT              mnItemId;
    USHORT              mnStyle;

public:
                        UserDrawEvent();
                        UserDrawEvent( OutputDevice* pOut,
                                       const Rectangle& rOutRect,
                                       USHORT nId, USHORT nStyle = 0 );

    OutputDevice*       GetDevice() const { return mpOutDev; }
    const Rectangle&    GetRect() const { return maOutRect; }
    USHORT              GetItemId() const { return mnItemId; }
    USHORT              GetStyle() const { return mnStyle; }
};

inline UserDrawEvent::UserDrawEvent()
{
    mpOutDev    = NULL;
    mnItemId    = 0;
    mnStyle     = 0;
}

inline UserDrawEvent::UserDrawEvent( OutputDevice* pOut,
                                     const Rectangle& rOutRect,
                                     USHORT nId, USHORT nStyle ) :
            maOutRect( rOutRect )
{
    mpOutDev    = pOut;
    mnItemId    = nId;
    mnStyle     = nStyle;
}

// ------------------
// - Tracking-Types -
// ------------------

#define ENDTRACK_CANCEL         ((USHORT)0x0001)
#define ENDTRACK_KEY            ((USHORT)0x0002)
#define ENDTRACK_FOCUS          ((USHORT)0x0004)
#define ENDTRACK_END            ((USHORT)0x1000)
#define ENDTRACK_DONTCALLHDL    ((USHORT)0x8000)

#define TRACKING_REPEAT         ((USHORT)0x0100)

// -----------------
// - TrackingEvent -
// -----------------

class VCL_DLLPUBLIC TrackingEvent
{
private:
    MouseEvent          maMEvt;
    USHORT              mnFlags;

public:
                        TrackingEvent();
                        TrackingEvent( const MouseEvent& rMEvt,
                                       USHORT nTrackFlags = 0 );

    const MouseEvent&   GetMouseEvent() const { return maMEvt; }

    BOOL                IsTrackingRepeat() const
                            { return ((mnFlags & TRACKING_REPEAT) != 0); }

    BOOL                IsTrackingEnded() const
                            { return ((mnFlags & ENDTRACK_END) != 0); }
    BOOL                IsTrackingCanceled() const
                            { return ((mnFlags & ENDTRACK_CANCEL) != 0); }
    USHORT              GetTrackingFlags() const { return mnFlags; }
};

inline TrackingEvent::TrackingEvent()
{
    mnFlags = 0;
}

inline TrackingEvent::TrackingEvent( const MouseEvent& rMEvt,
                                     USHORT nTrackFlags ) :
            maMEvt( rMEvt )
{
    mnFlags = nTrackFlags;
}

// ---------------
// - NotifyEvent -
// ---------------

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
#define EVENT_USER              10000

class VCL_DLLPUBLIC NotifyEvent
{
private:
    Window*                 mpWindow;
    void*                   mpData;
    USHORT                  mnType;
    long                    mnRetValue;

public:
                            NotifyEvent();
                            NotifyEvent( USHORT nType,
                                         Window* pWindow,
                                         const void* pEvent = NULL,
                                         long nRet = 0 );

    USHORT                  GetType() const { return mnType; }
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

inline NotifyEvent::NotifyEvent( USHORT nType, Window* pWindow,
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

// --------------------
// - DataChangedEvent -
// --------------------

#define DATACHANGED_SETTINGS            ((USHORT)1)
#define DATACHANGED_DISPLAY             ((USHORT)2)
#define DATACHANGED_DATETIME            ((USHORT)3)
#define DATACHANGED_FONTS               ((USHORT)4)
#define DATACHANGED_PRINTER             ((USHORT)5)
#define DATACHANGED_FONTSUBSTITUTION    ((USHORT)6)
#define DATACHANGED_USER                ((USHORT)10000)

class VCL_DLLPUBLIC DataChangedEvent
{
private:
    void*                   mpData;
    ULONG                   mnFlags;
    USHORT                  mnType;

public:
                            DataChangedEvent();
                            DataChangedEvent( USHORT nType,
                                              const void* pData = NULL,
                                              ULONG nFlags = 0 );

    USHORT                  GetType() const { return mnType; }
    void*                   GetData() const { return mpData; }
    ULONG                   GetFlags() const { return mnFlags; }

    const AllSettings*      GetOldSettings() const;
};

inline DataChangedEvent::DataChangedEvent()
{
    mpData  = NULL;
    mnFlags = 0;
    mnType  = 0;
}

inline DataChangedEvent::DataChangedEvent( USHORT nType,
                                           const void* pData,
                                           ULONG nChangeFlags )
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

#endif // _SV_EVENT_HXX

