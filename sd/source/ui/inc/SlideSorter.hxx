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

#ifndef SD_SLIDESORTER_SLIDE_SORTER_HXX
#define SD_SLIDESORTER_SLIDE_SORTER_HXX

#include "fupoor.hxx"
#include "Window.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/weakref.hxx>
#include <sfx2/viewfrm.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/current_function.hpp>


class ScrollBar;
class ScrollBarBox;
class Window;


namespace sd {
class ViewShell;
class ViewShellBase;
class Window;
}

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
class Theme;
} } }

namespace sd { namespace slidesorter { namespace controller {
class Listener;
class SlideSorterController;
class SlotManager;
class Properties;
} } }


typedef ::boost::shared_ptr<sd::Window> SharedSdWindow;


namespace sd { namespace slidesorter {

/** Show previews for all the slides in a document and allow the user to
    insert or delete slides and modify the order of the slides.

    This class is a facade for the model, view, and controller classes.
    It is a hub that allows access to the various parts of a slide sorter.

    Note that this class is not in its final state.
*/
class SlideSorter
    : private ::boost::noncopyable
{
    friend class controller::SlotManager;
public:
    virtual ~SlideSorter (void);

    /** Return whether the called SlideSorter object is valid and calling
        its Get(Model,View,Controller) methods is safe.  When <FALSE/> is
        called then no other methods should be called.
        Calling this method should be necessary only during startup and
        shutdown (when that can be detected).
    */
    bool IsValid (void) const;

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
    static ::boost::shared_ptr<SlideSorter> CreateSlideSorter (
        ViewShell& rViewShell,
        const ::boost::shared_ptr<sd::Window>& rpContentWindow,
        const ::boost::shared_ptr<ScrollBar>& rpHorizontalScrollBar,
        const ::boost::shared_ptr<ScrollBar>& rpVerticalScrollBar,
        const ::boost::shared_ptr<ScrollBarBox>& rpScrollBarBox);

    /** Create a new slide sorter that is losely coupled to the given view
        shell.  The view shell may even be missing.
        @param rBase
            ViewShellBase object of the enclosing application.
        @param pViewShell
            Supply when at hand.
        @param rParentWindow
            The parent window of the internally created content window and
            scroll bars.
    */
    static ::boost::shared_ptr<SlideSorter> CreateSlideSorter (
        ViewShellBase& rBase,
        ViewShell* pViewShell,
        ::Window& rParentWindow);

    /** Return the control of the vertical scroll bar.
    */
    ::boost::shared_ptr<ScrollBar> GetVerticalScrollBar (void) const;

    /** Return the control of the horizontal scroll bar.
    */
    ::boost::shared_ptr<ScrollBar> GetHorizontalScrollBar (void) const;

    /** Return the scroll bar filler that paints the little square that is
        enclosed by the two scroll bars.
    */
    ::boost::shared_ptr<ScrollBarBox> GetScrollBarFiller (void) const;

    /** Return the content window.  This is a sibling and is geometrically
        enclosed by the scroll bars.
    */
    SharedSdWindow GetContentWindow (void) const;

    model::SlideSorterModel& GetModel (void) const;

    view::SlideSorterView& GetView (void) const;

    controller::SlideSorterController& GetController (void) const;

    /** Return the view shell that was given at construction.
        @return
            May be empty.
    */
    ViewShell* GetViewShell (void) const;

    /** Return the XController object of the main view.
    */
    ::com::sun::star::uno::Reference<com::sun::star::frame::XController>
        GetXController (void) const;

    /** Return the ViewShellBase object.
        @return
            May be empty.
    */
    ViewShellBase* GetViewShellBase (void) const;

    void Paint (const Rectangle& rRepaintArea);

    /** Place and size the controls and windows.  You may want to call this
        method when something has changed that for instance affects the
        visibility state of the scroll bars.
    */
    void ArrangeGUIElements (
        const Point& rOffset,
        const Size& rSize);
    SvBorder GetBorder (void);

    bool RelocateToWindow (::Window* pWindow);

    /** Set the current function at the view shell or, when it is not
        present, set it at the content window.  This method supports the use
        of functions even when there is no SlideSorterViewShell.
    */
    void SetCurrentFunction (const FunctionReference& rpFunction);

    /** Return a collection of properties that are used througout the slide
        sorter.
    */
    ::boost::shared_ptr<controller::Properties> GetProperties (void) const;

    /** Return the active theme wich gives access to colors and fonts.
    */
    ::boost::shared_ptr<view::Theme> GetTheme (void) const;

protected:
    /** This virtual method makes it possible to create a specialization of
        the slide sorter view shell that works with its own implementation
        of model, view, and controller.  The default implementation simply
        calls the CreateModel(), CreateView(), and CreateController()
        methods in this order.
    */
    virtual void CreateModelViewController (void);

    /** Create the model for the view shell.  When called from the default
        implementation of CreateModelViewController() then neither view nor
        controller do exist.  Test their pointers when in doubt.
    */
    virtual model::SlideSorterModel* CreateModel (void);

    /** Create the view for the view shell.  When called from the default
        implementation of CreateModelViewController() then the model but not
        the controller does exist.  Test their pointers when in doubt.
    */
    virtual view::SlideSorterView* CreateView (void);

    /** Create the controller for the view shell.  When called from the default
        implementation of CreateModelViewController() then both the view and
        the controller do exist.  Test their pointers when in doubt.
    */
    virtual controller::SlideSorterController* CreateController (void);

private:
    bool mbIsValid;

    ::boost::scoped_ptr<controller::SlideSorterController> mpSlideSorterController;
    ::boost::scoped_ptr<model::SlideSorterModel> mpSlideSorterModel;
    ::boost::scoped_ptr<view::SlideSorterView> mpSlideSorterView;
    ::com::sun::star::uno::WeakReference<com::sun::star::frame::XController> mxControllerWeak;
    ViewShell* mpViewShell;
    ViewShellBase* mpViewShellBase;
    SharedSdWindow mpContentWindow;
    bool mbOwnesContentWindow;
    ::boost::shared_ptr<ScrollBar> mpHorizontalScrollBar;
    ::boost::shared_ptr<ScrollBar> mpVerticalScrollBar;
    ::boost::shared_ptr<ScrollBarBox> mpScrollBarBox;

    /** Set this flag to <TRUE/> to force a layout before the next paint.
    */
    bool mbLayoutPending;

    /** Some slide sorter wide properties that are used in different
        classes.
    */
    ::boost::shared_ptr<controller::Properties> mpProperties;
    ::boost::shared_ptr<view::Theme> mpTheme;

    SlideSorter (
        ViewShell& rViewShell,
        const ::boost::shared_ptr<sd::Window>& rpContentWindow,
        const ::boost::shared_ptr<ScrollBar>& rpHorizontalScrollBar,
        const ::boost::shared_ptr<ScrollBar>& rpVerticalScrollBar,
        const ::boost::shared_ptr<ScrollBarBox>& rpScrollBarBox);
    SlideSorter (
        ViewShellBase& rBase,
        ViewShell* pViewShell,
        ::Window& rParentWindow);

    void Init (void);
    /** Create the controls for the slide sorter.  This are the tab bar
       for switching the edit mode, the scroll bar, and the actual
       slide sorter view window.
       This method is usually called exactly one time from the
       constructor.
    */
    void SetupControls (::Window* pParentWindow);

    /** This method is usually called exactly one time from the
        constructor.
    */
    void SetupListeners (void);

    /** Release the listeners that have been installed in SetupListeners().
    */
    void ReleaseListeners (void);
};

} } // end of namespace ::sd::slidesorter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
