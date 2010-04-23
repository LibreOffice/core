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

    /** In order to place and size the buttons the ButtonBar has to
        know how large the page objects are.
    */
    void SetPageObjectSize (const Size aPageObjectSize);

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

    /** Set the page object for which to control the buttons.  The set
        of displayed buttons depends on whether this page is excluded
        from the slide show or not.
        @param rsPageHelpText
            When the mouse is moved over a button then a help text for
            that button is displayed.  When it is moved off the button
            area then the given page help text is displayed instead.
    */
    /*
    void SetPage (
        const model::SharedPageDescriptor& rpDescriptor,
        const Point aMouseModelLocation,
        const bool bIsMouseButtonDown,
        const ::rtl::OUString& rsPageHelpText);
    */
    void ResetPage (void);

    /** Return the number of buttons that are to be displayed in page
        objects which the mouse hovers over.
        @param bIsExcluded
            When this flag is <TRUE/> then return the number of
            buttons that is to be displayed for pages that are
            excluded from the slide show.
    */
    sal_Int32 GetButtonCount (const bool bIsExcluded) const;

    /** Return the specified button.
        @param nIndex
            Valid values lie in the range [0,GetButtonCount()).
        @param bIsExcluded
            When this flag is <TRUE/> then return a button that is to
            be displayed for pages that are excluded from the slide
            show.
        @return
            Returns an empty pointer when the given index is not valid.
    */
    ::boost::shared_ptr<Button> GetButton (
        const bool bIsExcluded,
        const sal_Int32 nIndex) const;

    bool IsMouseOverBar (void) const;

    /** Paint the specified page object.  When this is not the same as the
        one under the mouse (mpDescriptor) then the buttons are all
        painted in their normal state.
    */
    void Paint (
        OutputDevice& rDevice,
        const model::SharedPageDescriptor& rpPageDescriptor);

    bool IsMouseOverButton (void) const;

    void RequestLayout (void);

    /** Return the help text for the button under the mouse.
        @return
            When the mouse is not over a button then an empty string
            is returned.
    */
    ::rtl::OUString GetButtonHelpText (void) const;

private:
    SlideSorter& mrSlideSorter;
    Size maPageObjectSize;
    Rectangle maBoundingBox;
    model::SharedPageDescriptor mpDescriptor;
    bool mbIsExcluded;
    boost::shared_ptr<Button> mpButtonUnderMouse;
    // The button on which the mouse button was pressed.
    boost::shared_ptr<Button> mpDownButton;
    ::std::vector<SharedButton> maRegularButtons;
    ::std::vector<SharedButton> maExcludedButtons;
    Bitmap maNormalBackground;
    Bitmap maButtonDownBackground;
    bool mbIsMouseOverBar;

    /** In contrast to the other SetPage variant, this method only
        sets mpDescriptor.  If the given descriptor differs from
        mpDescriptor then the buttons are placed anew.
        @return
            The returned flag indicates wether the mpDescriptor member
            is set to a new value.
    */
    bool SetPage (const model::SharedPageDescriptor& rpDescriptor);
    SharedButton GetButtonAt (const Point aModelLocation);
    bool SetButtonUnderMouse (const SharedButton& rButton = SharedButton());
    void PaintButtonBackground (
        OutputDevice& rDevice,
        const Point aOffset);
    void LayoutButtons (const Size aPageModelSize);
    bool LayoutButtons (
        const Size aPageObjectSize,
        const bool bIsSmall);
    Bitmap CreateBackground (
        const OutputDevice& rTemplateDevice,
        const Size aSize,
        const bool bIsButtonDown) const;
    bool IsMouseOverBar (const Point aModelLocation) const;
};




class Button
{
public:
    Button (
        SlideSorter& rSlideSorter,
        const ::rtl::OUString& rsHelpText);
    virtual ~Button (void);

    enum State { ST_Normal, ST_Hover, ST_Down };
    /** Set a new state.
        @return
            When the new state is different from the old state
            then <TRUE/> is returned.
    */
    bool SetState (const State eState);
    State GetState (void) const;

    virtual void Place (const Rectangle aButtonBarBox, const sal_Int32 nIndex) = 0;
    virtual void Paint (
        OutputDevice& rDevice,
        const Point aOffset,
        const double nAlpha,
        const ::boost::shared_ptr<Theme>& rpTheme) const = 0;
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor) = 0;

    /** Return the bounding box of the layouted button.
    */
    Rectangle GetBoundingBox (void) const;
    /** Return the minimum size required to completely paint the
        button.
    */
    virtual Size GetSize (void) const = 0;
    ::rtl::OUString GetHelpText (void) const;
    bool IsDown (void) const;
    void SetActiveState (const bool bIsActive);
    bool IsActive (void) const;
    void SetIsSmall (const bool bIsSmall);

protected:
    SlideSorter& mrSlideSorter;
    State meState;
    Rectangle maBoundingBox;
    const ::rtl::OUString msHelpText;
    // Buttons that lie (partly) outside the button bar are deactivated.
    bool mbIsActive;
    bool mbIsSmall;
};



class TextButton : public Button
{
public:
    TextButton (
        SlideSorter& rSlideSorter,
        const ::rtl::OUString& rsText,
        const ::rtl::OUString& rsHelpText);

    virtual void Place (const Rectangle aButtonBarBox, const sal_Int32 nIndex);
    virtual void Paint (
        OutputDevice& rDevice,
        const Point aOffset,
        const double nAlpha,
        const ::boost::shared_ptr<Theme>& rpTheme) const;
    virtual Size GetSize (void) const;

private:
    const ::rtl::OUString msText;
};



class ImageButton : public Button
{
public:
    ImageButton (
        SlideSorter& rSlideSorter,
        const BitmapEx& rRegularIcon,
        const BitmapEx& rHoverIcon,
        const BitmapEx& rDownIcon,
        const BitmapEx& rSmallRegularIcon,
        const BitmapEx& rSmallHoverIcon,
        const BitmapEx& rSmallDownIcon,
        const ::rtl::OUString& rsHelpText);

    virtual void Place (const Rectangle aButtonBarBox, const sal_Int32 nIndex);
    virtual void Paint (
        OutputDevice& rDevice,
        const Point aOffset,
        const double nAlpha,
        const ::boost::shared_ptr<Theme>& rpTheme) const;
    virtual Size GetSize (void) const;

private:
    const BitmapEx maNormalIcon;
    const BitmapEx maHoverIcon;
    const BitmapEx maDownIcon;
    const BitmapEx maSmallIcon;
    const BitmapEx maSmallHoverIcon;
    const BitmapEx maSmallDownIcon;
};


class UnhideButton : public TextButton
{
public:
    UnhideButton (SlideSorter& rSlideSorter);

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


class StartShowButton : public ImageButton
{
public:
    StartShowButton (SlideSorter& rSlideSorter);

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


class HideButton : public ImageButton
{
public:
    HideButton (SlideSorter& rSlideSorter);

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


class DuplicateButton : public ImageButton
{
public:
    DuplicateButton (SlideSorter& rSlideSorter);

protected:
    virtual void ProcessClick (const model::SharedPageDescriptor& rpDescriptor);
};


} } } // end of namespace ::sd::slidesorter::view

#endif
