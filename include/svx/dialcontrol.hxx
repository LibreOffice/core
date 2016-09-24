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

#ifndef INCLUDED_SVX_DIALCONTROL_HXX
#define INCLUDED_SVX_DIALCONTROL_HXX

#include <memory>
#include <vcl/ctrl.hxx>
#include <sfx2/itemconnect.hxx>
#include <svx/svxdllapi.h>

class NumericField;

namespace svx {


class SAL_WARN_UNUSED DialControlBmp : public VirtualDevice
{
public:
    explicit            DialControlBmp( vcl::Window& rParent );

    void                InitBitmap(const vcl::Font& rFont);
    void                SetSize(const Size& rSize);
    void                CopyBackground( const DialControlBmp& rSrc );
    void                DrawBackground( const Size& rSize, bool bEnabled );
    void                DrawBackground();
    void                DrawElements( const OUString& rText, sal_Int32 nAngle );

protected:
    Rectangle           maRect;
    bool                mbEnabled;

private:
    const Color&        GetBackgroundColor() const;
    const Color&        GetTextColor() const;
    const Color&        GetScaleLineColor() const;
    const Color&        GetButtonLineColor() const;
    const Color&        GetButtonFillColor( bool bMain ) const;

    void                Init();

    vcl::Window&        mrParent;
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
class SAL_WARN_UNUSED SVX_DLLPUBLIC DialControl : public Control
{
public:
    explicit            DialControl( vcl::Window* pParent, WinBits nBits );

    virtual void        Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override;

    virtual void        StateChanged( StateChangedType nStateChange ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;
    virtual void        LoseFocus() override;

    virtual Size        GetOptimalSize() const override;
    virtual void        Resize() override;

    /** Returns true, if the control is not in "don't care" state. */
    bool                HasRotation() const;
    /** Sets the control to "don't care" state. */
    void                SetNoRotation();

    /** Returns the current rotation angle in 1/100 degrees. */
    sal_Int32           GetRotation() const;
    /** Sets the rotation to the passed value (in 1/100 degrees). */
    void                SetRotation( sal_Int32 nAngle );

    /** Links the passed numeric edit field to the control (bi-directional).
     *  nDecimalPlaces:
     *     field value is usign given decimal places
     *     default is 0 which means field values are in degrees,
     *     2 means 100th of degree
     */
    void                SetLinkedField( NumericField* pField, sal_Int32 nDecimalPlaces = 0);

    /** The passed handler is called whenever the rotation value changes. */
    void                SetModifyHdl( const Link<DialControl*,void>& rLink );

    /** Save value for later comparison */
    void                SaveValue();

    /** Compare value with the saved value */
    bool                IsValueModified();

protected:
    struct DialControl_Impl
    {
        ScopedVclPtr<DialControlBmp> mxBmpEnabled;
        ScopedVclPtr<DialControlBmp> mxBmpDisabled;
        ScopedVclPtr<DialControlBmp> mxBmpBuffered;
        Link<DialControl*,void>      maModifyHdl;
        VclPtr<NumericField>         mpLinkField;
        sal_Int32           mnLinkedFieldValueMultiplyer;
        Size                maWinSize;
        vcl::Font           maWinFont;
        sal_Int32           mnAngle;
        sal_Int32           mnInitialAngle;
        sal_Int32           mnOldAngle;
        long                mnCenterX;
        long                mnCenterY;
        bool                mbNoRot;

        explicit            DialControl_Impl( vcl::Window& rParent );
        void                Init( const Size& rWinSize, const vcl::Font& rWinFont );
        void                SetSize( const Size& rWinSize );
    };
    std::unique_ptr< DialControl_Impl > mpImpl;

    virtual void        HandleMouseEvent( const Point& rPos, bool bInitial );
    void                HandleEscapeEvent();

    void                SetRotation( sal_Int32 nAngle, bool bBroadcast );

    void                Init( const Size& rWinSize, const vcl::Font& rWinFont );
    void                Init( const Size& rWinSize );

private:
    void                InvalidateControl();

    DECL_LINK_TYPED( LinkedFieldModifyHdl, Edit&, void );
    DECL_LINK_TYPED( LinkedFieldFocusHdl, Control&, void );
    void LinkedFieldModifyHdl();
    DECL_LINK_TYPED(SpinFieldHdl, SpinField&, void);
};

/** Wrapper for usage of a DialControl in item connections. */
class SAL_WARN_UNUSED SVX_DLLPUBLIC DialControlWrapper : public sfx::SingleControlWrapper< DialControl, sal_Int32 >
{
public:
    explicit            DialControlWrapper( DialControl& rDial );

    virtual bool        IsControlDontKnow() const override;
    virtual void        SetControlDontKnow( bool bSet ) override;

    virtual sal_Int32   GetControlValue() const override;
    virtual void        SetControlValue( sal_Int32 nValue ) override;
};

/** An item<->control connection for a DialControl. */
typedef sfx::ItemControlConnection< sfx::Int32ItemWrapper, DialControlWrapper > DialControlConnection;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
