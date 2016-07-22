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

#ifndef INCLUDED_SD_SOURCE_UI_INC_VIEWSHELLBASE_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VIEWSHELLBASE_HXX

#include <com/sun/star/frame/XFrame.hpp>

#include "ViewShell.hxx"

#include "glob.hxx"
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfac.hxx>
#include <memory>

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
    SFX_DECL_INTERFACE(SD_IF_SDVIEWSHELLBASE)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    /** This constructor is used by the view factory of the SFX macros.
        Note that LateInit() has to be called after the constructor
        terminates and before doing anything else.
    */
    ViewShellBase (
        SfxViewFrame *pFrame,
        SfxViewShell* pOldShell);

    virtual ~ViewShellBase();

    /** This method is part of the object construction.  It HAS to be called
        after the constructor has created a new object.
    */
    void LateInit (const OUString& rsDefaultView);

    std::shared_ptr<ViewShellManager> GetViewShellManager() const;

    /** Return the main view shell stacked on the called ViewShellBase
        object.  This is usually the view shell displayed in the center
        pane.
    */
    std::shared_ptr<ViewShell> GetMainViewShell() const;

    /** When given a view frame this static method returns the
        corresponding sd::ViewShellBase object.
        @return
            When the SfxViewShell of the given frame is not a
            ViewShellBase object then NULL is returned.
    */
    static ViewShellBase* GetViewShellBase (SfxViewFrame* pFrame);

    DrawDocShell* GetDocShell() const { return mpDocShell;}
    SdDrawDocument* GetDocument() const { return mpDocument;}

    /** Callback function for general slot calls.  At the moment these are
        slots for switching the pane docking windows on and off.
    */
    virtual void Execute (SfxRequest& rRequest);

    /** Callback function for retrieving item values related to certain
        slots.  This is the companion of Execute() and handles the slots
        concerned with showing the pane docking windows.
    */
    void GetState (SfxItemSet& rSet);

    /* override these from SfxViewShell */
    virtual OUString GetSelectionText(bool = false) override;
    virtual bool HasSelection(bool = true ) const override;

    SvBorder GetBorder (bool bOuterResize);
    virtual void InnerResizePixel (const Point& rOrigin, const Size& rSize) override;
    virtual void OuterResizePixel (const Point& rOrigin, const Size& rSize) override;

    /** This call is forwarded to the main sub-shell.
    */
    virtual ErrCode DoVerb (long nVerb) override;

    /** Return a new renderer that can be used for example for printing the
        document.
    */
    virtual css::uno::Reference<css::view::XRenderable> GetRenderable() override;

    /// Forwarded to the print manager.
    virtual SfxPrinter* GetPrinter (bool bCreate = false) override;

    /// Forwarded to the print manager.
    virtual sal_uInt16 SetPrinter (
        SfxPrinter* pNewPrinter,
        SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL) override;

    /// Forward methods to main sub shell.
    virtual void WriteUserDataSequence (
        css::uno::Sequence< css::beans::PropertyValue >&) override;

    /** Pass the given properties to the main view shell.  After that we
        ensure that the right view shell type is displayed in the center
        pane.
    */
    virtual void ReadUserDataSequence (
        const css::uno::Sequence< css::beans::PropertyValue >&) override;

    virtual void UIActivating( SfxInPlaceClient* ) override;
    virtual void UIDeactivated( SfxInPlaceClient* ) override;
    virtual void Activate (bool IsMDIActivate) override;
    virtual void Deactivate (bool IsMDIActivate) override;
    virtual void SetZoomFactor (
        const Fraction &rZoomX,
        const Fraction &rZoomY) override;
    virtual bool PrepareClose (bool bUI = true) override;
    virtual void WriteUserData (OUString&, bool bBrowse = false) override;
    virtual void ReadUserData (const OUString&, bool bBrowse = false) override;
    virtual SdrView* GetDrawView() const override;

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
    void Rearrange();

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

    /** Return an event multiplexer.  It is a single class that forwards
        events from various sources.  This method must not be called before
        LateInit() has terminated.
    */
    std::shared_ptr<tools::EventMultiplexer> GetEventMultiplexer();

    /** returns the complete area of the current view relative to the frame
        window
    */
    const Rectangle& getClientRectangle() const;

    std::shared_ptr<ToolBarManager> GetToolBarManager() const;
    std::shared_ptr<FormShellManager> GetFormShellManager() const;

    DrawController& GetDrawController() const;

    void SetViewTabBar (const ::rtl::Reference<ViewTabBar>& rViewTabBar);

    /** Return the window that is used by the main view shell to display its
        view and other UI elements, like scroll bars and rulers.  Ownership
        of that window remains with the called ViewShellBase object.
    */
    vcl::Window* GetViewWindow();

    /** returns the ui descriptive name for the given uno slot. The result is taken from the configuration
        and not cached, so do not use it excessive (f.e. in status updates) */
    OUString RetrieveLabelFromCommand( const OUString& aCmdURL ) const;
    /// See SfxViewShell::getPart().
    int getPart() const override;

protected:

    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    virtual void InitializeFramework();

private:
    class Implementation;
    std::unique_ptr<Implementation> mpImpl;
    DrawDocShell* mpDocShell;
    SdDrawDocument* mpDocument;

    /** Determine from the properties of the document shell the initial type
        of the view shell in the center pane.  We use this method to avoid
        starting with the wrong type.  When ReadUserDataSequence() is called
        we check that the right type is active and change again if that is
        not the case because something went wrong.
    */
    OUString GetInitialViewShellType();
};

OUString ImplRetrieveLabelFromCommand( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& aCmdURL );


} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
