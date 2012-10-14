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

#ifndef SD_VIEW_SHELL_BASE_HXX
#define SD_VIEW_SHELL_BASE_HXX

#include <com/sun/star/frame/XFrame.hpp>

#include "ViewShell.hxx"

#include "glob.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfac.hxx>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <set>

class SdDrawDocument;
class SfxRequest;

namespace sd { namespace tools {
class EventMultiplexer;
} }

namespace sd {

class DrawController;
class DrawDocShell;
class FormShellManager;
class ToolBarManager;
class ViewShell;
class ViewShellManager;
class ViewTabBar;

/** SfxViewShell descendant that the stacked Draw/Impress shells are
    based on.

    <p>The "base" part of the name does not mean that this is a base
    class of some class hierarchy.  It rather is the base of the
    stacked shells.</p>

    <p>This class starts as a new and relatively small class.  Over
    time as much code as possible should be moved from the stacked
    shells to this class.</p>
*/
class ViewShellBase
    : public SfxViewShell
{
public:
    TYPEINFO();
    SFX_DECL_VIEWFACTORY(ViewShellBase);
    SFX_DECL_INTERFACE(SD_IF_SDVIEWSHELLBASE)

    /** This constructor is used by the view factory of the SFX macros.
        Note that LateInit() has to be called after the constructor
        terminates and before doing anything else.
    */
    ViewShellBase (
        SfxViewFrame *pFrame,
        SfxViewShell* pOldShell);

    virtual ~ViewShellBase (void);

    /** This method is part of the object construction.  It HAS to be called
        after the constructor has created a new object.
    */
    virtual void LateInit (const ::rtl::OUString& rsDefaultView);

    ::boost::shared_ptr<ViewShellManager> GetViewShellManager (void) const;

    /** Return the main view shell stacked on the called ViewShellBase
        object.  This is usually the view shell displayed in the center
        pane.
    */
    ::boost::shared_ptr<ViewShell> GetMainViewShell (void) const;

    /** When given a view frame this static method returns the
        corresponding sd::ViewShellBase object.
        @return
            When the SfxViewShell of the given frame is not a
            ViewShellBase object then NULL is returned.
    */
    static ViewShellBase* GetViewShellBase (SfxViewFrame* pFrame);

    DrawDocShell* GetDocShell (void) const;
    SdDrawDocument* GetDocument (void) const;

    /** Callback function for general slot calls.  At the moment these are
        slots for switching the pane docking windows on and off.
    */
    virtual void Execute (SfxRequest& rRequest);

    /** Callback function for retrieving item values related to certain
        slots.  This is the companion of Execute() and handles the slots
        concerned with showing the pane docking windows.
    */
    virtual void GetState (SfxItemSet& rSet);

    /* override these from SfxViewShell */
    virtual String GetSelectionText(sal_Bool);
    virtual sal_Bool HasSelection(sal_Bool) const;

    SvBorder GetBorder (bool bOuterResize);
    virtual void InnerResizePixel (const Point& rOrigin, const Size& rSize);
    virtual void OuterResizePixel (const Point& rOrigin, const Size& rSize);

    /** This call is forwarded to the main sub-shell.
    */
    virtual ErrCode DoVerb (long nVerb);

    /** Return a new renderer that can be used for example for printing the
        document.
    */
    virtual com::sun::star::uno::Reference<com::sun::star::view::XRenderable> GetRenderable (void);

    /// Forwarded to the print manager.
    virtual SfxPrinter* GetPrinter (sal_Bool bCreate = sal_False);

    /// Forwarded to the print manager.
    virtual sal_uInt16 SetPrinter (
        SfxPrinter* pNewPrinter,
        sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsApi=false);

    /// Forwarded to the print manager.
    virtual SfxTabPage* CreatePrintOptionsPage (
        ::Window *pParent,
        const SfxItemSet &rOptions);

    /// Forward methods to main sub shell.
    virtual void WriteUserDataSequence (
        ::com::sun::star::uno::Sequence <
        ::com::sun::star::beans::PropertyValue >&,
        sal_Bool bBrowse = sal_False);

    /** Pass the given properties to the main view shell.  After that we
        ensure that the right view shell type is displayed in the center
        pane.
    */
    virtual void ReadUserDataSequence (
        const ::com::sun::star::uno::Sequence <
        ::com::sun::star::beans::PropertyValue >&,
        sal_Bool bBrowse = sal_False);

    virtual void UIActivating( SfxInPlaceClient* );
    virtual void UIDeactivated( SfxInPlaceClient* );
    virtual void Activate (sal_Bool IsMDIActivate);
    virtual void Deactivate (sal_Bool IsMDIActivate);
    virtual void SetZoomFactor (
        const Fraction &rZoomX,
        const Fraction &rZoomY);
    virtual sal_uInt16 PrepareClose (sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False);
    virtual void WriteUserData (String&, sal_Bool bBrowse = sal_False);
    virtual void ReadUserData (const String&, sal_Bool bBrowse = sal_False);
    virtual SdrView* GetDrawView (void) const;
    virtual void AdjustPosSizePixel (const Point &rOfs, const Size &rSize);

    /** When <TRUE/> is given, then the mouse shape is set to hour glass (or
        whatever the busy shape looks like on the system.)
    */
    void SetBusyState (bool bBusy);

    /** Call this method when the controls of this view shell or the
        embedded sub shell need to be rearranged.  This is necessary
        e.g. when the border has been modified (UpdateBorder() calls this
        method).

        This method is like ResizePixel() with no arguments.
    */
    void Rearrange (void);

    /** Update the border that is set with SfxViewShell::SetBorderPixel().
        This is done by adding the border used by the ViewShellBase itself
        with the border used by the main view shell.

        @param bForce   if true the borders are also updated if old border
                        and new border are same.
    */
    void UpdateBorder ( bool bForce = false );

    /** With this method the UI controls can be turned on or off.  It is
        used by the FuSlideShow to hide the UI controls while showing a
        non-full-screen or in-window presentation in the center pane.
    */
    void ShowUIControls (bool bVisible);

    /** this method starts the presentation by
        executing the slot SID_PRESENTATION asynchronous */
    void StartPresentation();

    /** Return an event multiplexer.  It is a single class that forwards
        events from various sources.  This method must not be called before
        LateInit() has terminated.
    */
    ::boost::shared_ptr<tools::EventMultiplexer> GetEventMultiplexer (void);

    /** returns the complete area of the current view relative to the frame
        window
    */
    const Rectangle& getClientRectangle() const;

    ::boost::shared_ptr<ToolBarManager> GetToolBarManager (void) const;
    ::boost::shared_ptr<FormShellManager> GetFormShellManager (void) const;

    DrawController& GetDrawController (void) const;

    void SetViewTabBar (const ::rtl::Reference<ViewTabBar>& rViewTabBar);

    /** Return the window that is used by the main view shell to display its
        view and other UI elements, like scroll bars and rulers.  Ownership
        of that window remains with the called ViewShellBase object.
    */
    ::Window* GetViewWindow (void);

    /** returns the ui descriptive name for the given uno slot. The result is taken from the configuration
        and not cached, so do not use it excessive (f.e. in status updates) */
    ::rtl::OUString RetrieveLabelFromCommand( const ::rtl::OUString& aCmdURL ) const;

protected:
    osl::Mutex maMutex;

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    virtual void InitializeFramework (void);

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImpl;
    DrawDocShell* mpDocShell;
    SdDrawDocument* mpDocument;

    /** Determine from the properties of the document shell the initial type
        of the view shell in the center pane.  We use this method to avoid
        starting with the wrong type.  When ReadUserDataSequence() is called
        we check that the right type is active and change again if that is
        not the case because something went wrong.
    */
    ::rtl::OUString GetInitialViewShellType (void);
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
