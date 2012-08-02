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

#ifndef SD_SLIDESORTER_VIEW_BUTTON_BAR_HXX
#define SD_SLIDESORTER_VIEW_BUTTON_BAR_HXX

#include "model/SlsSharedPageDescriptor.hxx"
#include <tools/gen.hxx>
#include <rtl/ustring.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <boost/scoped_ptr.hpp>

namespace sd { namespace slidesorter {
class SlideSorter;
} }


namespace sd { namespace slidesorter { namespace view {

class Theme;

class Button;
typedef ::boost::shared_ptr<Button> SharedButton;

/** This is a container of buttons and a coordinating controller.
    The last means that it receives mouse events and forwards them to
    the right button.
*/
class ButtonBar
{
public:
    ButtonBar (SlideSorter& rSlideSorter);
    ~ButtonBar (void);

    void ProcessButtonDownEvent (
        const model::SharedPageDescriptor& rpDescriptor,
        const Point aMouseModelLocation);
    void ProcessButtonUpEvent (
        const model::SharedPageDescriptor& rpDescriptor,
        const Point aMouseModelLocation);
    void ProcessMouseMotionEvent (
        const model::SharedPageDescriptor& rpDescriptor,
        const Point aMouseModelLocation,
        const bool bIsMouseButtonDown);

    void ResetPage (void);

    bool IsMouseOverBar (void) const;

    /** Paint the specified page object.  When this is not the same as the
        one under the mouse (mpDescriptor) then the buttons are all
        painted in their normal state.
    */
    void Paint (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpPageDescriptor);

    bool IsMouseOverButton (void) const;

    /** Return the help text for the button under the mouse.
        @return
            When the mouse is not over a button then an empty string
            is returned.
    */
    ::rtl::OUString GetButtonHelpText (void) const;

    /** Request the button bar to be shown.
        @param bAnimate
            This flag controls whether to just show the button bar (<FALSE/>)
            or to fade it in smoothly (<TRUE/>.)
    */
    void RequestFadeIn (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bAnimate);

    /** Request the button bar to be hidden.
        @param bAnimate
            This flag controls whether to just hide the button bar (<FALSE/>)
            or to fade it out smoothly (<TRUE/>.)
    */
    void RequestFadeOut (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bAnimate);

    /** Return whether the button bar is visible for the givn descriptor (or
        being faded in.)
    */
    bool IsVisible (const model::SharedPageDescriptor& rpDescriptor);

    void HandleDataChangeEvent (void);

    class BackgroundTheme;

    /** While at least one Lock object exists the button bar will not be
        displayed.  Used, e.g. during a mouse multiselection to avoid
        confusing and unhelpfull visual signals.
    */
    class Lock
    {
    public:
        Lock (SlideSorter& rSlideSorter);
        ~Lock (void);
    private:
        ButtonBar& mrButtonBar;
    };

private:
    SlideSorter& mrSlideSorter;
    Size maPageObjectSize;
    Rectangle maButtonBoundingBox;
    Point maBackgroundLocation;
    model::SharedPageDescriptor mpDescriptor;
    bool mbIsExcluded;
    boost::shared_ptr<Button> mpButtonUnderMouse;
    // The button on which the mouse button was pressed.
    boost::shared_ptr<Button> mpDownButton;
    ::std::vector<SharedButton> maRegularButtons;
    ::std::vector<SharedButton> maExcludedButtons;
    BitmapEx maNormalBackground;
    bool mbIsMouseOverBar;
    ::boost::scoped_ptr<BackgroundTheme> mpBackgroundTheme;
    int mnLockCount;

    /** Remember the specified page.  If it differs from mpDescriptor then
        the buttons are placed anew.
        @return
            The returned flag indicates whether the mpDescriptor member
            is set to a new value.
    */
    bool SetPage (const model::SharedPageDescriptor& rpDescriptor);
    SharedButton GetButtonAt (const Point aModelLocation);
    bool SetButtonUnderMouse (const SharedButton& rButton = SharedButton());
    void PaintButtonBackground (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpPageDescriptor,
        const Point aOffset);
    void LayoutButtons (const Size aPageModelSize);
    bool LayoutButtons (void);
    BitmapEx CreateBackground (
        const OutputDevice& rTemplateDevice,
        const bool bIsButtonDown) const;
    bool IsMouseOverBar (const Point aModelLocation) const;
    void StartFadeAnimation (
        const model::SharedPageDescriptor& rpDescriptor,
        const double nTargetAlpha,
        const bool bFadeIn);

    void AcquireLock (void);
    void ReleaseLock (void);
};




class Button
{
public:
    Button (
        SlideSorter& rSlideSorter,
        const BitmapEx& rLargeIcon,
        const BitmapEx& rLargeHoverIcon,
        const BitmapEx& rMediumIcon,
        const BitmapEx& rMediumHoverIcon,
        const BitmapEx& rSmallIcon,
        const BitmapEx& rSmallHoverIcon,
        const ::rtl::OUString& rsHelpText);
    virtual ~Button (void);

    enum State { State_Normal, State_Hover, State_Down };
    enum IconSize { IconSize_Large, IconSize_Medium, IconSize_Small };

    /** Set a new state.
        @return
            When the new state is different from the old state
            then <TRUE/> is returned.
    */
    bool SetState (const State eState);

    virtual void Place (const Rectangle aButtonBarBox);
    virtual void Paint (
        OutputDevice& rDevice,
        const Point aOffset,
        const double nAlpha,
        const ::boost::shared_ptr<Theme>& rpTheme) const;
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor) = 0;

    /** Return the bounding box of the layouted button.
    */
    Rectangle GetBoundingBox (void) const;
    /** Return the minimum size required to completely paint the
        button.
    */
    virtual Size GetSize (void) const;
    virtual Size GetSize (const IconSize eIconSize) const;
    ::rtl::OUString GetHelpText (void) const;
    void SetActiveState (const bool bIsActive);
    bool IsActive (void) const;
    void SetIconSize (const IconSize eIconSize);
    /** By default a button is always enabled.  Override to change this.
    */
    virtual bool IsEnabled (void) const;

protected:
    SlideSorter& mrSlideSorter;
    State meState;
    Rectangle maBoundingBox;
    const ::rtl::OUString msHelpText;
    // Buttons that lie (partly) outside the button bar are deactivated.
    bool mbIsActive;
    IconSize meIconSize;

    const BitmapEx maLargeIcon;
    const BitmapEx maLargeHoverIcon;
    const BitmapEx maMediumIcon;
    const BitmapEx maMediumHoverIcon;
    const BitmapEx maSmallIcon;
    const BitmapEx maSmallHoverIcon;
};



class UnhideButton : public Button
{
public:
    UnhideButton (SlideSorter& rSlideSorter);

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


class StartShowButton : public Button
{
public:
    StartShowButton (SlideSorter& rSlideSorter);
    virtual bool IsEnabled (void) const;

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


class HideButton : public Button
{
public:
    HideButton (SlideSorter& rSlideSorter);

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


class DuplicateButton : public Button
{
public:
    DuplicateButton (SlideSorter& rSlideSorter);
    virtual bool IsEnabled (void) const;

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
