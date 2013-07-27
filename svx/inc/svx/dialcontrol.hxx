/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SVX_DIALCONTROL_HXX
#define SVX_DIALCONTROL_HXX

#include <memory>
#include <vcl/ctrl.hxx>
#include <sfx2/itemconnect.hxx>
#include "svx/svxdllapi.h"

#include <boost/scoped_ptr.hpp>

class NumericField;

namespace svx {

// ============================================================================

class DialControlBmp : public VirtualDevice
{
public:
    explicit            DialControlBmp( Window& rParent );

    void                InitBitmap( const Size& rSize, const Font& rFont );
    void                CopyBackground( const DialControlBmp& rSrc );
    void                DrawBackground( const Size& rSize, bool bEnabled );
    virtual void        DrawBackground();
    virtual void        DrawElements( const String& rText, sal_Int32 nAngle );

protected:
    Rectangle           maRect;
    bool                mbEnabled;

private:
    const Color&        GetBackgroundColor() const;
    const Color&        GetTextColor() const;
    const Color&        GetScaleLineColor() const;
    const Color&        GetButtonLineColor() const;
    const Color&        GetButtonFillColor( bool bMain ) const;

    void                Init( const Size& rSize );

    Window&             mrParent;
    long                mnCenterX;
    long                mnCenterY;
};

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

protected:
    struct DialControl_Impl
    {
        ::boost::scoped_ptr<DialControlBmp>      mpBmpEnabled;
        ::boost::scoped_ptr<DialControlBmp>      mpBmpDisabled;
        ::boost::scoped_ptr<DialControlBmp>      mpBmpBuffered;
        Link                maModifyHdl;
        NumericField*       mpLinkField;
        Size                maWinSize;
        Font                maWinFont;
        sal_Int32           mnAngle;
        sal_Int32           mnOldAngle;
        long                mnCenterX;
        long                mnCenterY;
        bool                mbNoRot;

        explicit            DialControl_Impl( Window& rParent );
        void                Init( const Size& rWinSize, const Font& rWinFont );
    };
    std::auto_ptr< DialControl_Impl > mpImpl;

    virtual void        HandleMouseEvent( const Point& rPos, bool bInitial );
    virtual void        HandleEscapeEvent();

    void                SetRotation( sal_Int32 nAngle, bool bBroadcast );

    void                Init( const Size& rWinSize, const Font& rWinFont );
    void                Init( const Size& rWinSize );

private:
    void                InvalidateControl();

    void                ImplSetFieldLink( const Link& rLink );


    DECL_LINK( LinkedFieldModifyHdl, NumericField* );
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

