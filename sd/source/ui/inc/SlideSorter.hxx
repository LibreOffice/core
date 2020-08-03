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

#pragma once

#include <cppuhelper/weakref.hxx>
#include <vcl/scrbar.hxx>
#include <sddllapi.h>
#include <memory>

namespace vcl { class Window; }
namespace com::sun::star::frame { class XController; }
namespace rtl { template <class reference_type> class Reference; }

namespace sd {
class ViewShell;
class ViewShellBase;
class Window;
class FuPoor;
}

namespace sd::slidesorter::model { class SlideSorterModel; }

namespace sd::slidesorter::view {
    class SlideSorterView;
    class Theme;
}

namespace sd::slidesorter::controller {
    class SlideSorterController;
    class SlotManager;
    class Properties;
}

namespace sd::slidesorter {

/** Show previews for all the slides in a document and allow the user to
    insert or delete slides and modify the order of the slides.

    This class is a facade for the model, view, and controller classes.
    It is a hub that allows access to the various parts of a slide sorter.

    Note that this class is not in its final state.
*/
class SlideSorter final
{
    friend class controller::SlotManager;
public:
    ~SlideSorter();

    /// Forbid copy construction and copy assignment
    SlideSorter(const SlideSorter&) = delete;
    SlideSorter& operator=(const SlideSorter&) = delete;

    /** Return whether the called SlideSorter object is valid and calling
        its Get(Model,View,Controller) methods is safe.  When <FALSE/> is
        called then no other methods should be called.
        Calling this method should be necessary only during startup and
        shutdown (when that can be detected).
    */
    bool IsValid() const { return mbIsValid;}

    /** Create a new slide sorter that is strongly coupled to the given view
        shell.  Use this function for a slide sorter in the left pane.
        @param rViewShell
            Typically a SlideSorterViewShell object.
        @param rpContentWindow
            Typically the content window of the ViewShell.
        @param rpHorizontalScrollBar
            Typically the horizontal scroll bar of the ViewShell.
        @param rpVerticalScrollBar
            Typically the vertical scroll bar of the ViewShell.
        @param rpScrollBarBox
            The little square enclosed by the two scroll bars.  Typically
            the one from the ViewShell.
    */
    static std::shared_ptr<SlideSorter> CreateSlideSorter (
        ViewShell& rViewShell,
        sd::Window* pContentWindow,
        ScrollBar* pHorizontalScrollBar,
        ScrollBar* pVerticalScrollBar,
        ScrollBarBox* pScrollBarBox);

    /** Create a new slide sorter that is loosely coupled to the given view
        shell.  The view shell may even be missing.
        @param rBase
            ViewShellBase object of the enclosing application.
        @param pViewShell
            Supply when at hand.
        @param rParentWindow
            The parent window of the internally created content window and
            scroll bars.
    */
    static std::shared_ptr<SlideSorter> CreateSlideSorter (
        ViewShellBase& rBase,
        vcl::Window& rParentWindow);

    /** Return the control of the vertical scroll bar.
    */
    const VclPtr<ScrollBar>& GetVerticalScrollBar() const { return mpVerticalScrollBar;}

    /** Return the control of the horizontal scroll bar.
    */
    const VclPtr<ScrollBar>& GetHorizontalScrollBar() const { return mpHorizontalScrollBar;}

    /** Return the scroll bar filler that paints the little square that is
        enclosed by the two scroll bars.
    */
    const VclPtr<ScrollBarBox>& GetScrollBarFiller (void) const { return mpScrollBarBox;}

    /** Return the content window.  This is a sibling and is geometrically
        enclosed by the scroll bars.
    */
    const VclPtr<sd::Window>& GetContentWindow() const { return mpContentWindow;}

    model::SlideSorterModel& GetModel() const;

    view::SlideSorterView& GetView() const;

    // Exported for unit test
    SD_DLLPUBLIC controller::SlideSorterController& GetController() const;

    /** Return the view shell that was given at construction.
        @return
            May be empty.
    */
    ViewShell* GetViewShell() const { return mpViewShell;}

    /** Return the XController object of the main view.
    */
    css::uno::Reference<css::frame::XController>
        GetXController() const;

    /** Return the ViewShellBase object.
        @return
            May be empty.
    */
    ViewShellBase* GetViewShellBase() const { return mpViewShellBase;}

    void Paint (const ::tools::Rectangle& rRepaintArea);

    /** Place and size the controls and windows.  You may want to call this
        method when something has changed that for instance affects the
        visibility state of the scroll bars.
    */
    void ArrangeGUIElements (
        const Point& rOffset,
        const Size& rSize);

    void RelocateToWindow (vcl::Window* pWindow);

    /** Set the current function at the view shell or, when it is not
        present, set it at the content window.  This method supports the use
        of functions even when there is no SlideSorterViewShell.
    */
    void SetCurrentFunction (const rtl::Reference<FuPoor>& rpFunction);

    /** Return a collection of properties that are used throughout the slide
        sorter.
    */
    std::shared_ptr<controller::Properties> const & GetProperties() const;

    /** Return the active theme which gives access to colors and fonts.
    */
    std::shared_ptr<view::Theme> const & GetTheme() const;

private:
    /** This virtual method makes it possible to create a specialization of
        the slide sorter view shell that works with its own implementation
        of model, view, and controller.  The default implementation simply
        calls the CreateModel(), CreateView(), and CreateController()
        methods in this order.
    */
    void CreateModelViewController();

    /** Create the model for the view shell.  When called from the default
        implementation of CreateModelViewController() then neither view nor
        controller do exist.  Test their pointers when in doubt.
    */
    model::SlideSorterModel* CreateModel();

    bool mbIsValid;

    std::unique_ptr<controller::SlideSorterController> mpSlideSorterController;
    std::unique_ptr<model::SlideSorterModel> mpSlideSorterModel;
    std::unique_ptr<view::SlideSorterView> mpSlideSorterView;
    css::uno::WeakReference<css::frame::XController> mxControllerWeak;
    ViewShell* mpViewShell;
    ViewShellBase* mpViewShellBase;
    VclPtr<sd::Window> mpContentWindow;
    VclPtr<ScrollBar> mpHorizontalScrollBar;
    VclPtr<ScrollBar> mpVerticalScrollBar;
    VclPtr<ScrollBarBox> mpScrollBarBox;

    /** Some slide sorter wide properties that are used in different
        classes.
    */
    std::shared_ptr<controller::Properties> mpProperties;
    std::shared_ptr<view::Theme> mpTheme;

    SlideSorter (
        ViewShell& rViewShell,
        sd::Window* pContentWindow,
        ScrollBar* pHorizontalScrollBar,
        ScrollBar* pVerticalScrollBar,
        ScrollBarBox* pScrollBarBox);
    SlideSorter (
        ViewShellBase& rBase,
        vcl::Window& rParentWindow);

    void Init();
    /** Create the controls for the slide sorter.  This are the tab bar
       for switching the edit mode, the scroll bar, and the actual
       slide sorter view window.
       This method is usually called exactly one time from the
       constructor.
    */
    void SetupControls();

    /** This method is usually called exactly one time from the
        constructor.
    */
    void SetupListeners();

    /** Release the listeners that have been installed in SetupListeners().
    */
    void ReleaseListeners();
};

} // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
