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

#ifndef INCLUDED_SVX_ORIENTHELPER_HXX
#define INCLUDED_SVX_ORIENTHELPER_HXX

#include <memory>
#include <vcl/window.hxx>
#include <sfx2/itemconnect.hxx>
#include <svx/svxdllapi.h>

class NumericField;
class CheckBox;

namespace svx {

class DialControl;



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
    /** @param rNfRotation  A numeric field that will be connected to the DialControl. */
    explicit            OrientationHelper(
                            DialControl& rCtrlDial,
                            NumericField& rNfRotation,
                            CheckBox& rCbStacked );

    virtual             ~OrientationHelper();

    /** Registers the passed window to be enabled/disabled on call of Enable().
        @param eDisableIfStacked
        TRISTATE_TRUE:    Window always disabled, if stacked text is turned on.
        TRISTATE_FALSE:  Window always disabled, if stacked text is turned off.
        TRISTATE_INDET: Window will be enabled/disabled independent from stacked text. */
    void                AddDependentWindow( vcl::Window& rWindow, TriState eDisableIfStacked = TRISTATE_INDET );

    /** Enables or disables the dial control and all dependent windows. */
    void                Enable( bool bEnable = true );

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
    std::unique_ptr< OrientationHelper_Impl > mpImpl;
};



/** Wrapper for usage of the stacked attribute of an OrientationHelper in item connections. */
class SVX_DLLPUBLIC OrientStackedWrapper : public sfx::SingleControlWrapper< OrientationHelper, bool >
{
public:
    explicit            OrientStackedWrapper( OrientationHelper& rOrientHlp );

    virtual bool        IsControlDontKnow() const override;
    virtual void        SetControlDontKnow( bool bSet ) override;

    virtual bool        GetControlValue() const override;
    virtual void        SetControlValue( bool bValue ) override;
};



/** An item<->control connection for the stacked attribute of an OrientationHelper. */
typedef sfx::ItemControlConnection< sfx::BoolItemWrapper, OrientStackedWrapper > OrientStackedConnection;



} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
