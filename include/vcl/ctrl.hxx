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

#ifndef INCLUDED_VCL_CTRL_HXX
#define INCLUDED_VCL_CTRL_HXX

#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <memory>

// forward
namespace vcl { struct ImplControlData; }
class StyleSettings;


class VCL_DLLPUBLIC Control : public vcl::Window
{
protected:
    std::unique_ptr<vcl::ImplControlData> mpControlData;

private:
    bool                    mbHasControlFocus;
    bool                    mbShowAccelerator;
    Link<Control&,void>     maGetFocusHdl;
    Link<Control&,void>     maLoseFocusHdl;

    SAL_DLLPRIVATE void     ImplInitControlData();

                            Control (const Control &) = delete;
                            Control & operator= (const Control &) = delete;

protected:
                    Control( WindowType nType );
    virtual void    FillLayoutData() const;

    // helper method for composite controls
    void            AppendLayoutData( const Control& rSubControl ) const;

    /// creates the mpData->mpLayoutData structure
    void            CreateLayoutData() const;
    /// determines whether we currently have layout data
    bool            HasLayoutData() const;

    /** this calls both our event listeners, and a specified handler

        If the Control instance is destroyed during any of those calls, the
        method properly handles this (in particular, it doesn't crash :)

        @param nEvent
            the event to notify to our event listeners
        @param callHandler
            the lambda function that calls the handler
        @return
            if the Control instance has been destroyed in any of the call
    */
    bool        ImplCallEventListenersAndHandler(
                    VclEventId nEvent, std::function<void()> const & callHandler
                );

    void        CallEventListeners( VclEventId nEvent, void* pData = nullptr );

    /** draws the given text onto the given device

        If no reference device is set, the draw request will simply be forwarded to OutputDevice::DrawText. Otherwise,
        the text will be rendered according to the metrics at the reference device.

        return will contain the result of a GetTextRect call (either directly
        at the target device, or taking the reference device into account) when
        returning.
    */
    tools::Rectangle DrawControlText( OutputDevice& _rTargetDevice, const tools::Rectangle& _rRect,
                               const OUString& _rStr, DrawTextFlags _nStyle,
                               MetricVector* _pVector, OUString* _pDisplayText,
                               const Size* i_pDeviceSize = nullptr ) const;

    tools::Rectangle GetControlTextRect( OutputDevice& _rTargetDevice, const tools::Rectangle & rRect,
                                  const OUString& _rStr, DrawTextFlags _nStyle,
                                  Size* o_pDeviceSize = nullptr ) const;

    virtual const vcl::Font&
                GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

    void ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

public:
    SAL_DLLPRIVATE void ImplClearLayoutData() const;
    /** draws a frame around the give rectangle, onto the given device

        only to be used from within the Window::Draw method of your sub class.

        The frame is always drawn with a single line (without 3D effects). In addition, any mono
        color set at the control's settings is respected. Yet more additionally, if we're living
        in a themed desktop, this theming is ignored.

        Note that this makes sense, since the *only known* clients of Window::Draw
        are form controls, when printed or print-previewed. For form controls embedded in office documents,
        you don't want to have the theme look.

        @param pDev
            the device to draw onto
        @param rRect
            the rect for drawing the frame. Upon returning from the call, the rect will be inflated
            by the space occupied by the drawn pixels.
    */
    SAL_DLLPRIVATE void ImplDrawFrame( OutputDevice* pDev, tools::Rectangle& rRect );

public:
    explicit        Control( vcl::Window* pParent, WinBits nWinStyle = 0 );
    virtual         ~Control() override;
    virtual void    dispose() override;

    virtual void    EnableRTL ( bool bEnable = true ) override;

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    Resize() override;

    // invalidates layout data
    virtual void    SetText( const OUString& rStr ) override;
    // gets the displayed text
    virtual OUString GetDisplayText() const override;
    // returns the bounding box for the character at index nIndex (in control coordinates)
    tools::Rectangle GetCharacterBounds( long nIndex ) const;
    // returns the character index for corresponding to rPoint (in control coordinates)
    // -1 is returned if no character is at that point
    long GetIndexForPoint( const Point& rPoint ) const;
    // returns the interval [start,end] of line nLine
    // returns [-1,-1] for an invalid line
    Pair GetLineStartEnd( long nLine ) const;
    /** ToRelativeLineIndex changes a layout data index to a count relative to its line.

    This is equivalent to getting the line start/end pairs with
    GetLineStartEnd() until the index lies within [start,end] of a line

    @param nIndex
    the absolute index inside the display text to be changed to a relative index

    @returns
    the relative index inside the displayed line or -1 if the absolute index does
    not match any line
    */
    long ToRelativeLineIndex( long nIndex ) const;

    void            SetGetFocusHdl( const Link<Control&,void>& rLink ) { maGetFocusHdl = rLink; }
    void            SetLoseFocusHdl( const Link<Control&,void>& rLink ) { maLoseFocusHdl = rLink; }

    /** determines whether the control currently has the focus
    */
    bool            HasControlFocus() const { return mbHasControlFocus; }

    void            SetLayoutDataParent( const Control* pParent ) const;

    virtual Size    GetOptimalSize() const override;

    /** sets a reference device used for rendering control text
        @see DrawControlText
    */
    void            SetReferenceDevice( OutputDevice* _referenceDevice );
    OutputDevice*   GetReferenceDevice() const;

    vcl::Font       GetUnzoomedControlPointFont() const;
    void            SetShowAccelerator (bool val);
};

#endif // INCLUDED_VCL_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
