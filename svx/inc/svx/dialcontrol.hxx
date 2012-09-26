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

#ifndef SVX_DIALCONTROL_HXX
#define SVX_DIALCONTROL_HXX

#include <memory>
#include <vcl/ctrl.hxx>
#include <sfx2/itemconnect.hxx>
#include "svx/svxdllapi.h"

class NumericField;

namespace svx {

// ============================================================================

struct DialControl_Impl;

/** This control allows to input a rotation angle, visualized by a dial.

    Usage: A single click sets a rotation angle rounded to steps of 15 degrees.
    Dragging with the left mouse button sets an exact rotation angle. Pressing
    the ESCAPE key during mouse drag cancels the operation and restores the old
    state of the control.

    It is possible to link a numeric field to this control using the function
    SetLinkedField(). The DialControl will take full control of this numeric
    field:
    -   Sets the rotation angle to the numeric field in mouse operations.
    -   Shows the value entered/modified in the numeric field.
    -   Enables/disables/shows/hides the field according to own state changes.
 */
class SVX_DLLPUBLIC DialControl : public Control
{
public:
    explicit            DialControl( Window* pParent, const ResId& rResId );
    virtual             ~DialControl();

    virtual void        Paint( const Rectangle& rRect );

    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        LoseFocus();

    /** Returns true, if the control is not in "don't care" state. */
    bool                HasRotation() const;
    /** Sets the control to "don't care" state. */
    void                SetNoRotation();

    /** Returns the current rotation angle in 1/100 degrees. */
    sal_Int32           GetRotation() const;
    /** Sets the rotation to the passed value (in 1/100 degrees). */
    void                SetRotation( sal_Int32 nAngle );

    /** Links the passed numeric edit field to the control (bi-directional). */
    void                SetLinkedField( NumericField* pField );

    /** Save value for later comparison */
    void                SaveValue();

    /** Compare value with the saved value */
    bool                IsValueModified();

private:
    void                Init( const Size& rWinSize, const Font& rWinFont );
    void                Init( const Size& rWinSize );
    void                InvalidateControl();

    void                ImplSetRotation( sal_Int32 nAngle, bool bBroadcast );
    void                ImplSetFieldLink( const Link& rLink );

    void                HandleMouseEvent( const Point& rPos, bool bInitial );
    void                HandleEscapeEvent();

    DECL_LINK( LinkedFieldModifyHdl, NumericField* );

    std::auto_ptr< DialControl_Impl > mpImpl;
};

// ============================================================================

/** Wrapper for usage of a DialControl in item connections. */
class SVX_DLLPUBLIC DialControlWrapper : public sfx::SingleControlWrapper< DialControl, sal_Int32 >
{
public:
    explicit            DialControlWrapper( DialControl& rDial );

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual sal_Int32   GetControlValue() const;
    virtual void        SetControlValue( sal_Int32 nValue );
};

// ----------------------------------------------------------------------------

/** An item<->control connection for a DialControl. */
typedef sfx::ItemControlConnection< sfx::Int32ItemWrapper, DialControlWrapper > DialControlConnection;

// ============================================================================

} // namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
