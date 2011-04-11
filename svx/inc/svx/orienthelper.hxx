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

#ifndef SVX_ORIENTHELPER_HXX
#define SVX_ORIENTHELPER_HXX

#include <memory>
#include <vcl/window.hxx>
#include <sfx2/itemconnect.hxx>
#include "svx/svxdllapi.h"

class NumericField;
class CheckBox;

namespace svx {

class DialControl;

// ============================================================================

struct OrientationHelper_Impl;

/** A helper class that manages a DialControl and a "Stacked text" check box.

    This helper remembers a DialControl for entering a rotation angle, and a
    check box for stacked text, that enables/disables other controls dependent
    on its state.

    It is possible to register more controls that have to be enabled/disabled
    together with this helper object (optionally dependent on the stacked text
    check box), using the function AddDependentWindow(). All registered windows
    are handled on a call of Enable(), or Show(), or on changing the state of
    the "Stacked text" check box.

    Note: The member function SetStackedState() should be used instead of
    direct modifications of the "Stacked text" check box. Otherwise the update
    mechanism of registered controls will not work.
 */
class SVX_DLLPUBLIC OrientationHelper
{
public:
    explicit            OrientationHelper(
                            DialControl& rCtrlDial,
                            CheckBox& rCbStacked );

    /** @param rNfRotation  A numeric field that will be connected to the DialControl. */
    explicit            OrientationHelper(
                            DialControl& rCtrlDial,
                            NumericField& rNfRotation,
                            CheckBox& rCbStacked );

    virtual             ~OrientationHelper();

    /** Registers the passed window to be enabled/disabled on call of Enable().
        @param eDisableIfStacked
        STATE_CHECK:    Window always disabled, if stacked text is turned on.
        STATE_NOCHECK:  Window always disabled, if stacked text is turned off.
        STATE_DONTKNOW: Window will be enabled/disabled independent from stacked text. */
    void                AddDependentWindow( Window& rWindow, TriState eDisableIfStacked = STATE_DONTKNOW );

    /** Enables or disables the dial control and all dependent windows. */
    void                Enable( bool bEnable = true );
    /** Disables the dial control and all dependent windows. */
    inline void         Disable() { Enable( false ); }

    /** Shows or hides the dial control and all dependent windows. */
    void                Show( bool bShow = true );
    /** Hides the dial control and all dependent windows. */
    inline void         Hide() { Show( false ); }

    /** Sets the "stacked" check box to the passed state and updates dependent controls. */
    void                SetStackedState( TriState eState );
    /** Returns the state of the "stacked" check box. */
    TriState            GetStackedState() const;

    /** Enables/disables the "don't know" state of the "Stacked text" check box. */
    void                EnableStackedTriState( bool bEnable = true );

private:
    std::auto_ptr< OrientationHelper_Impl > mpImpl;
};

// ============================================================================

/** Wrapper for usage of the stacked attribute of an OrientationHelper in item connections. */
class SVX_DLLPUBLIC OrientStackedWrapper : public sfx::SingleControlWrapper< OrientationHelper, bool >
{
public:
    explicit            OrientStackedWrapper( OrientationHelper& rOrientHlp );

    virtual bool        IsControlDontKnow() const;
    virtual void        SetControlDontKnow( bool bSet );

    virtual bool        GetControlValue() const;
    virtual void        SetControlValue( bool bValue );
};

// ----------------------------------------------------------------------------

/** An item<->control connection for the stacked attribute of an OrientationHelper. */
typedef sfx::ItemControlConnection< sfx::BoolItemWrapper, OrientStackedWrapper > OrientStackedConnection;

// ============================================================================

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
