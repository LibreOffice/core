/*************************************************************************
 *
 *  $RCSfile: dialcontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:23:46 $
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

#ifndef SVX_DIALCONTROL_HXX
#define SVX_DIALCONTROL_HXX

#include <memory>

#ifndef _SV_CTRL_HXX
#include <vcl/ctrl.hxx>
#endif
#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif

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
class DialControl : public Control
{
public:
    explicit            DialControl( Window* pParent, const Size& rSize, const Font& rFont, WinBits nWinStyle = 0 );
    explicit            DialControl( Window* pParent, const Size& rSize, WinBits nWinStyle = 0 );
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
    /** Returns the linked numeric edit field, or 0. */
    NumericField*       GetLinkedField() const;

    /** The passed handler is called whenever the totation value changes. */
    void                SetModifyHdl( const Link& rLink );
    /** Returns the current modify handler. */
    const Link&         GetModifyHdl() const;

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
class DialControlWrapper : public sfx::SingleControlWrapper< DialControl, sal_Int32 >
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

