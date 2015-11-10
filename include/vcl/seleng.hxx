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

#ifndef INCLUDED_VCL_SELENG_HXX
#define INCLUDED_VCL_SELENG_HXX

#include <vcl/dllapi.h>
#include <vcl/timer.hxx>
#include <vcl/event.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace vcl { class Window; }
class CommandEvent;

// Timerticks
#define SELENG_DRAGDROP_TIMEOUT     400
#define SELENG_AUTOREPEAT_INTERVAL  50
#define SELENG_AUTOREPEAT_INTERVAL_MIN 25
#define SELENG_AUTOREPEAT_INTERVAL_MAX 300

enum SelectionMode { NO_SELECTION, SINGLE_SELECTION, RANGE_SELECTION, MULTIPLE_SELECTION };


// - FunctionSet -


class VCL_DLLPUBLIC FunctionSet
{
public:
    virtual ~FunctionSet() = 0;

    virtual void    BeginDrag() = 0;

    virtual void    CreateAnchor() = 0;  // Anker-Pos := Cursor-Pos
    virtual void    DestroyAnchor() = 0;

    // move cursor, at the same time match cursor position to the selection
    // starting at anchor. true == Ok
    virtual bool    SetCursorAtPoint( const Point& rPointPixel,
                                          bool bDontSelectAtCursor = false ) = 0;

    virtual bool    IsSelectionAtPoint( const Point& rPointPixel ) = 0;
    virtual void        DeselectAtPoint( const Point& rPointPixel ) = 0;
    // delete anchor & deselect all
    virtual void        DeselectAll() = 0;
};


// - SelectionEngine -

enum class SelectionEngineFlags {
    DRG_ENAB     = 0x0001,
    IN_SEL       = 0x0002,
    IN_ADD       = 0x0004,
    ADD_ALW      = 0x0008,
    HAS_ANCH     = 0x0020,
    CMDEVT       = 0x0040,
    WAIT_UPEVT   = 0x0080,
    EXPANDONMOVE = 0x0100,
};
namespace o3tl
{
    template<> struct typed_flags<SelectionEngineFlags> : is_typed_flags<SelectionEngineFlags, 0x01ff> {};
}

class VCL_DLLPUBLIC SelectionEngine
{
private:
    FunctionSet*        pFunctionSet;
    VclPtr<vcl::Window> pWin;
    Rectangle           aArea;
    Timer               aWTimer; // generate fake mouse moves
    MouseEvent          aLastMove;
    SelectionMode       eSelMode;
    sal_uLong               nUpdateInterval;
    sal_uInt16              nLockedMods;
    SelectionEngineFlags    nFlags;
    DECL_DLLPRIVATE_LINK_TYPED( ImpWatchDog, Timer*, void );

    inline bool         ShouldDeselect( bool bModifierKey1 ) const;
                                // determines to deselect or not when Ctrl-key is pressed on CursorPosChanging
public:

                        SelectionEngine( vcl::Window* pWindow,
                                         FunctionSet* pFunctions = nullptr,
                                         sal_uLong nAutoRepeatInterval = SELENG_AUTOREPEAT_INTERVAL );
                        ~SelectionEngine();

    // true: Event was processed by Selection Engine
    bool                SelMouseButtonDown( const MouseEvent& rMEvt );
    bool                SelMouseButtonUp( const MouseEvent& rMEvt );
    bool                SelMouseMove( const MouseEvent& rMEvt );
    //SelMouseButtonDown captures mouse events, SelMouseButtonUp
    //releases the capture. If you need to release the mouse
    //capture after SelMouseButtonDown but before
    //SelMouseButtonUp, e.g. to allow events to go to a
    //context menu via "Command" which is delivered after
    //mouse down but before mouse up, then use this
    void                ReleaseMouse();

    // Keyboard
    void                CursorPosChanging( bool bShift, bool bMod1 );

    // is needed to generate a Move event via a Timer
    // when the mouse is outside the area
    void                SetVisibleArea( const Rectangle& rNewArea )
                            { aArea = rNewArea; }

    void                SetAddMode( bool);
    bool                IsAddMode() const;

    void                AddAlways( bool bOn );
    bool                IsAlwaysAdding() const;

    void                EnableDrag( bool bOn );

    void                SetSelectionMode( SelectionMode eMode );
    SelectionMode       GetSelectionMode() const { return eSelMode; }

    void                SetFunctionSet( FunctionSet* pFuncs )
                            { pFunctionSet = pFuncs; }
    const FunctionSet*  GetFunctionSet() const { return pFunctionSet; }

    const Point&        GetMousePosPixel() const
                            { return aLastMove.GetPosPixel(); }
    const MouseEvent&   GetMouseEvent() const { return aLastMove; }

    void                SetWindow( vcl::Window*);
    vcl::Window*             GetWindow() const { return pWin; }

    void                LockModifiers( sal_uInt16 nModifiers )
                            { nLockedMods = nModifiers; }
    sal_uInt16              GetLockedModifiers() const { return nLockedMods; }

    bool                IsInSelection() const;
    void                Reset();

    void                Command( const CommandEvent& rCEvt );

    bool                HasAnchor() const;
    void                SetAnchor( bool bAnchor );

    void                SetUpdateInterval( sal_uLong nInterval );

    // wird im Ctor eingeschaltet
    void                ExpandSelectionOnMouseMove( bool bExpand = true )
                        {
                            if( bExpand )
                                nFlags |= SelectionEngineFlags::EXPANDONMOVE;
                            else
                                nFlags &= ~SelectionEngineFlags::EXPANDONMOVE;
                        }
};

inline bool SelectionEngine::IsAddMode()  const
{
    if ( nFlags & (SelectionEngineFlags::IN_ADD | SelectionEngineFlags::ADD_ALW) )
        return true;
    else
        return false;
}

inline void SelectionEngine::SetAddMode( bool bNewMode )
{
    if ( bNewMode )
        nFlags |= SelectionEngineFlags::IN_ADD;
    else
        nFlags &= (~SelectionEngineFlags::IN_ADD);
}

inline void SelectionEngine::EnableDrag( bool bOn )
{
    if ( bOn )
        nFlags |= SelectionEngineFlags::DRG_ENAB;
    else
        nFlags &= (~SelectionEngineFlags::DRG_ENAB);
}

inline void SelectionEngine::AddAlways( bool bOn )
{
    if( bOn )
        nFlags |= SelectionEngineFlags::ADD_ALW;
    else
        nFlags &= (~SelectionEngineFlags::ADD_ALW);
}

inline bool SelectionEngine::IsAlwaysAdding() const
{
    if ( nFlags & SelectionEngineFlags::ADD_ALW )
        return true;
    else
        return false;
}

inline bool SelectionEngine::IsInSelection() const
{
    if ( nFlags & SelectionEngineFlags::IN_SEL )
        return true;
    else
        return false;
}

inline bool SelectionEngine::HasAnchor() const
{
    if ( nFlags & SelectionEngineFlags::HAS_ANCH )
        return true;
    else
        return false;
}

inline void SelectionEngine::SetAnchor( bool bAnchor )
{
    if ( bAnchor )
        nFlags |= SelectionEngineFlags::HAS_ANCH;
    else
        nFlags &= (~SelectionEngineFlags::HAS_ANCH);
}

#endif // INCLUDED_VCL_SELENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
