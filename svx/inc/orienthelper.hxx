/*************************************************************************
 *
 *  $RCSfile: orienthelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:26:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVX_ORIENTHELPER_HXX
#define SVX_ORIENTHELPER_HXX

#include <memory>

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif

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
class OrientationHelper : public Window
{
public:
    explicit            OrientationHelper(
                            Window* pParent,
                            DialControl& rCtrlDial,
                            CheckBox& rCbStacked );

    /** @param rNfRotation  A numeric field that will be connected to the DialControl. */
    explicit            OrientationHelper(
                            Window* pParent,
                            DialControl& rCtrlDial,
                            NumericField& rNfRotation,
                            CheckBox& rCbStacked );

    virtual             ~OrientationHelper();

    /** Handles Enable/Show events for all registered windows. */
    virtual void        StateChanged( StateChangedType nStateChange );

    /** Registers the passed window to be enabled/disabled on call of Enable().
        @param eDisableIfStacked
        STATE_CHECK:    Window always disabled, if stacked text is turned on.
        STATE_NOCHECK:  Window always disabled, if stacked text is turned off.
        STATE_DONTKNOW: Window will be enabled/disabled independent from stacked text. */
    void                AddDependentWindow( Window& rWindow, TriState eDisableIfStacked = STATE_DONTKNOW );

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
class OrientStackedWrapper : public sfx::SingleControlWrapper< OrientationHelper, bool >
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

