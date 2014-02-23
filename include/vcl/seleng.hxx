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

class Window;
class CommandEvent;

// Timerticks
#define SELENG_DRAGDROP_TIMEOUT     400
#define SELENG_AUTOREPEAT_INTERVAL  50
#define SELENG_AUTOREPEAT_INTERVAL_MIN 25
#define SELENG_AUTOREPEAT_INTERVAL_MAX 300

enum SelectionMode { NO_SELECTION, SINGLE_SELECTION, RANGE_SELECTION, MULTIPLE_SELECTION };

// ---------------
// - FunctionSet -
// ---------------

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

// -------------------
// - SelectionEngine -
// -------------------

#define SELENG_DRG_ENAB     0x0001
#define SELENG_IN_SEL       0x0002
#define SELENG_IN_ADD       0x0004
#define SELENG_ADD_ALW      0x0008
#define SELENG_HAS_ANCH     0x0020
#define SELENG_CMDEVT       0x0040
#define SELENG_WAIT_UPEVT   0x0080
#define SELENG_EXPANDONMOVE 0x0100

class VCL_DLLPUBLIC SelectionEngine
{
private:
    FunctionSet*        pFunctionSet;
    Window*             pWin;
    Rectangle           aArea;
    Timer               aWTimer; // generate fake mouse moves
    MouseEvent          aLastMove;
    SelectionMode       eSelMode;
    sal_uLong               nUpdateInterval;
    sal_uInt16              nLockedMods;
    sal_uInt16              nFlags;
    DECL_DLLPRIVATE_LINK( ImpWatchDog, void* );

    inline bool         ShouldDeselect( bool bModifierKey1 ) const;
                                // determines to deselect or not when Ctrl-key is pressed on CursorPosChanging
public:

                        SelectionEngine( Window* pWindow,
                                         FunctionSet* pFunctions = NULL,
                                         sal_uLong nAutoRepeatInterval = SELENG_AUTOREPEAT_INTERVAL );
                        ~SelectionEngine();

    // true: Event was processed by Selection Engine
    bool                SelMouseButtonDown( const MouseEvent& rMEvt );
    bool                SelMouseButtonUp( const MouseEvent& rMEvt );
    bool                SelMouseMove( const MouseEvent& rMEvt );

    // Keyboard
    void                CursorPosChanging( bool bShift, bool bMod1 );

    // is needed to generate a Move event via a Timer
    // when the mouse is outside the area
    void                SetVisibleArea( const Rectangle rNewArea )
                            { aArea = rNewArea; }
    const Rectangle&    GetVisibleArea() const { return aArea; }

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

    void                SetWindow( Window*);
    Window*             GetWindow() const { return pWin; }

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
                                nFlags |= SELENG_EXPANDONMOVE;
                            else
                                nFlags &= ~SELENG_EXPANDONMOVE;
                        }
};

inline bool SelectionEngine::IsAddMode()  const
{
    if ( nFlags & (SELENG_IN_ADD | SELENG_ADD_ALW) )
        return true;
    else
        return false;
}

inline void SelectionEngine::SetAddMode( bool bNewMode )
{
    if ( bNewMode )
        nFlags |= SELENG_IN_ADD;
    else
        nFlags &= (~SELENG_IN_ADD);
}

inline void SelectionEngine::EnableDrag( bool bOn )
{
    if ( bOn )
        nFlags |= SELENG_DRG_ENAB;
    else
        nFlags &= (~SELENG_DRG_ENAB);
}

inline void SelectionEngine::AddAlways( bool bOn )
{
    if( bOn )
        nFlags |= SELENG_ADD_ALW;
    else
        nFlags &= (~SELENG_ADD_ALW);
}

inline bool SelectionEngine::IsAlwaysAdding() const
{
    if ( nFlags & SELENG_ADD_ALW )
        return true;
    else
        return false;
}

inline bool SelectionEngine::IsInSelection() const
{
    if ( nFlags & SELENG_IN_SEL )
        return true;
    else
        return false;
}

inline bool SelectionEngine::HasAnchor() const
{
    if ( nFlags & SELENG_HAS_ANCH )
        return true;
    else
        return false;
}

inline void SelectionEngine::SetAnchor( bool bAnchor )
{
    if ( bAnchor )
        nFlags |= SELENG_HAS_ANCH;
    else
        nFlags &= (~SELENG_HAS_ANCH);
}

#endif // INCLUDED_VCL_SELENG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
