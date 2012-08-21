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

#ifndef _SV_CTRL_HXX
#define _SV_CTRL_HXX

#include <tools/link.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <vcl/salnativewidgets.hxx>

// forward
namespace vcl { struct ImplControlData; struct ControlLayoutData; }

// -----------
// - Control -
// -----------

class VCL_DLLPUBLIC Control : public Window
{
protected:
    ::vcl::ImplControlData* mpControlData;

private:
    bool                    mbHasControlFocus;
    Link                    maGetFocusHdl;
    Link                    maLoseFocusHdl;

    SAL_DLLPRIVATE void     ImplInitControlData();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE          Control (const Control &);
    SAL_DLLPRIVATE          Control & operator= (const Control &);

protected:
                    Control( WindowType nType );
    virtual void    FillLayoutData() const;

    // helper method for composite controls
    void            AppendLayoutData( const Control& rSubControl ) const;

    /// creates the mpData->mpLayoutData structure
    void            CreateLayoutData() const;
    /// determines whether we currently have layout data
    bool            HasLayoutData() const;
    /// returns the current layout data
    ::vcl::ControlLayoutData*
                    GetLayoutData() const;

    /** this calls both our event listeners, and a specified handler

        If the Control instance is destroyed during any of those calls, the
        method properly handles this (in particular, it doesn't crash :)

        @param nEvent
            the event to notify to our event listeners
        @param rHandler
            the handler to call
        @param pCaller
            the parameter to pass to the handler call
        @return
            if the Control instance has been destroyed in any of the call
    */
    sal_Bool        ImplCallEventListenersAndHandler(
                    sal_uLong nEvent, const Link& rHandler, void* pCaller
                );

    /** draws the given text onto the given device

        If no reference device is set, the draw request will simply be forwarded to OutputDevice::DrawText. Otherwise,
        the text will be rendered according to the metrics at the reference device.

        Note that the given rectangle might be modified, it will contain the result of a GetTextRect call (either
        directly at the target device, or taking the reference device into account) when returning.
    */
    void        DrawControlText( OutputDevice& _rTargetDevice, Rectangle& _io_rRect,
                                 const XubString& _rStr, sal_uInt16 _nStyle,
                                 MetricVector* _pVector, String* _pDisplayText ) const;

    virtual const Font&
                GetCanonicalFont( const StyleSettings& _rStyle ) const;
    virtual const Color&
                GetCanonicalTextColor( const StyleSettings& _rStyle ) const;

    void ImplInitSettings( const sal_Bool _bFont, const sal_Bool _bForeground );

public:
    SAL_DLLPRIVATE void ImplClearLayoutData() const;
    /** draws a frame around the give rectangle, onto the given device

        only to be used from within the <member>Window::Draw</member> method of your sub class.

        The frame is always drawn with a single line (without 3D effects). In addition, any mono
        color set at the control's settings is respected. Yet more additionally, if we're living
        in a themed desktop, this theming is ignored.

        Note that this makes sense, since the *only known* clients of <member>Window::Draw</member>
        are form controls, when printed or print-previewed. For form controls embedded in office documents,
        you don't want to have the theme look.

        @param pDev
            the device to draw onto
        @param rRect
            the rect for drawing the frame. Upon returning from the call, the rect will be inflated
            by the space occupied by the drawn pixels.
    */
    SAL_DLLPRIVATE void ImplDrawFrame( OutputDevice* pDev, Rectangle& rRect );

public:
                    Control( Window* pParent, WinBits nWinStyle = 0 );
                    Control( Window* pParent, const ResId& rResId );
                    ~Control();

    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    Resize();
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    // invalidates layout data
    virtual void    SetText( const String& rStr );
    // gets the displayed text
    virtual String GetDisplayText() const;
    // returns the bounding box for the character at index nIndex (in control coordinates)
    Rectangle GetCharacterBounds( long nIndex ) const;
    // returns the character index for corresponding to rPoint (in control coordinates)
    // -1 is returned if no character is at that point
    long GetIndexForPoint( const Point& rPoint ) const;
    // returns the interval [start,end] of line nLine
    // returns [-1,-1] for an invalid line
    Pair GetLineStartEnd( long nLine ) const;
    /** ToRelativeLineIndex changes a layout data index to a count relative to its line.

    <p>This is equivalent to getting the line start/end pairs with
    <member>GetLineStartEnd</member> until the index lies within [start,end] of a line
    </p>

    @param nIndex
    the absolute index inside the display text to be changed to a relative index

    @returns
    the relative index inside the displayed line or -1 if the absolute index does
    not match any line
    */
    long ToRelativeLineIndex( long nIndex ) const;

    void            SetGetFocusHdl( const Link& rLink ) { maGetFocusHdl = rLink; }
    const Link&     GetGetFocusHdl() const   { return maGetFocusHdl; }
    void            SetLoseFocusHdl( const Link& rLink ) { maLoseFocusHdl = rLink; }
    const Link&     GetLoseFocusHdl() const { return maLoseFocusHdl; }

    /** determines whether the control currently has the focus
    */
    bool            HasControlFocus() const { return mbHasControlFocus; }

    void            SetLayoutDataParent( const Control* pParent ) const;

    virtual Size    GetOptimalSize(WindowSizeType eType) const;

    /** sets a reference device used for rendering control text
        @seealso DrawControlText
    */
    void            SetReferenceDevice( OutputDevice* _referenceDevice );
    OutputDevice*   GetReferenceDevice() const;

    Font            GetUnzoomedControlPointFont() const
    {
        Font aFont( GetCanonicalFont( GetSettings().GetStyleSettings() ) );
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        return aFont;
    }
};

#endif  // _SV_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
