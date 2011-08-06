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

#ifndef SD_FRAMEWORK_FRAMEWORK_HELPER_HXX
#define SD_FRAMEWORK_FRAMEWORK_HELPER_HXX

#include "ViewShell.hxx"

#include "tools/SdGlobalResourceContainer.hxx"

#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/lang/XEventListener.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <map>

namespace sd {
class ViewShell;
class ViewShellBase;
}

namespace rtl {
class OUString;
}

namespace css = ::com::sun::star;

namespace sd { namespace framework {

/** The FrameworkHelper is a convenience class that simplifies the
    access to the drawing framework.
    It has three main tasks:
    1. Provide frequently used strings of resource URLs and event names.
    2. Provide shortcuts for accessing the sd framework.
    3. Easy the migration to the drawing framwork.

    Note that a FrameworkHelper disposes itself when one of the resource
    controllers called by it throw a DisposedException.
*/
class FrameworkHelper
    : public ::boost::enable_shared_from_this<FrameworkHelper>,
      public SdGlobalResource
{
public:
    // URLs of frequently used panes.
    static const ::rtl::OUString msPaneURLPrefix;
    static const ::rtl::OUString msCenterPaneURL;
    static const ::rtl::OUString msFullScreenPaneURL;
    static const ::rtl::OUString msLeftImpressPaneURL;
    static const ::rtl::OUString msLeftDrawPaneURL;
    static const ::rtl::OUString msRightPaneURL;

    // URLs of frequently used views.
    static const ::rtl::OUString msViewURLPrefix;
    static const ::rtl::OUString msImpressViewURL;
    static const ::rtl::OUString msDrawViewURL;
    static const ::rtl::OUString msOutlineViewURL;
    static const ::rtl::OUString msNotesViewURL;
    static const ::rtl::OUString msHandoutViewURL;
    static const ::rtl::OUString msSlideSorterURL;
    static const ::rtl::OUString msPresentationViewURL;
    static const ::rtl::OUString msTaskPaneURL;

    // URLs of frequently used tool bars.
    static const ::rtl::OUString msToolBarURLPrefix;
    static const ::rtl::OUString msViewTabBarURL;

    // URLs of task panels.
    static const ::rtl::OUString msTaskPanelURLPrefix;
    static const ::rtl::OUString msMasterPagesTaskPanelURL;
    static const ::rtl::OUString msLayoutTaskPanelURL;
    static const ::rtl::OUString msTableDesignPanelURL;
    static const ::rtl::OUString msCustomAnimationTaskPanelURL;
    static const ::rtl::OUString msSlideTransitionTaskPanelURL;

    // Names of frequently used events.
    static const ::rtl::OUString msResourceActivationRequestEvent;
    static const ::rtl::OUString msResourceDeactivationRequestEvent;
    static const ::rtl::OUString msResourceActivationEvent;
    static const ::rtl::OUString msResourceDeactivationEvent;
    static const ::rtl::OUString msConfigurationUpdateStartEvent;
    static const ::rtl::OUString msConfigurationUpdateEndEvent;

    // Service names of the common controllers.
    static const ::rtl::OUString msModuleControllerService;
    static const ::rtl::OUString msConfigurationControllerService;

    /** Return the FrameworkHelper object that is associated with the given
        ViewShellBase.  If such an object does not yet exist, a new one is
        created.
    */
    static ::boost::shared_ptr<FrameworkHelper> Instance (ViewShellBase& rBase);

    static ::boost::shared_ptr<FrameworkHelper> Instance (
        const css::uno::Reference<css::frame::XController>& rxController);

    /** Mark the FrameworkHelper object for the given ViewShellBase as
        disposed.  A following ReleaseInstance() call will destroy the
        FrameworkHelper object.

        Do not call this method.  It is an internally used method that can
        not be made private.
    */
    static void DisposeInstance (ViewShellBase& rBase);

    /** Destroy the FrameworkHelper object for the given ViewShellBase.

        Do not call this method.  It is an internally used method that can
        not be made private.
    */
    static void ReleaseInstance (ViewShellBase& rBase);

    /** Return an identifier for the given view URL.  This identifier can be
        used in a switch statement.  See GetViewURL() for a mapping in the
        opposite direction.
    */
    static ViewShell::ShellType GetViewId (const rtl::OUString& rsViewURL);

    /** Return a view URL for the given identifier.  See GetViewId() for a
        mapping in the opposite direction.
    */
    static ::rtl::OUString GetViewURL (ViewShell::ShellType eType);

    /** Return a ViewShell pointer for the given XView reference.  This
        assumes that the given reference is implemented by the
        ViewShellWrapper class that supports the XTunnel interface.
        @return
            When the ViewShell pointer can not be inferred from the given
            reference then an empty pointer is returned.
    */
    static ::boost::shared_ptr<ViewShell> GetViewShell (
        const css::uno::Reference<css::drawing::framework::XView>& rxView);

    ~FrameworkHelper (void);

    typedef ::boost::function<bool(const css::drawing::framework::ConfigurationChangeEvent&)>
        ConfigurationChangeEventFilter;
    typedef ::boost::function<void(bool bEventSeen)> Callback;
    typedef ::boost::function<
        void(
            const css::uno::Reference<
                css::drawing::framework::XResourceId>&)
        > ResourceFunctor;

    /** Test whether the called FrameworkHelper object is valid.
        @return
            When the object has already been disposed then <FALSE/> is returned.
    */
    bool IsValid (void);

    /** Return a pointer to the view shell that is displayed in the
        specified pane.  See GetView() for a variant that returns a
        reference to XView instead of a ViewShell pointer.
        @return
            An empty pointer is returned when for example the specified pane
            does not exist or is not visible or does not show a view or one
            of the involved objects does not support XUnoTunnel (where
            necessary).
    */
    ::boost::shared_ptr<ViewShell> GetViewShell (const ::rtl::OUString& rsPaneURL);

    /** Return a reference to the view that is displayed in the specified
        pane.  See GetViewShell () for a variant that returns a ViewShell
        pointer instead of a reference to XView.
        @param rxPaneOrViewId
            When this ResourceId specifies a view then that view is
            returned.  When it belongs to a pane then one view in that pane
            is returned.
        @return
            An empty reference is returned when for example the specified pane
            does not exist or is not visible or does not show a view or one
            of the involved objects does not support XTunnel (where
            necessary).
    */
    css::uno::Reference<css::drawing::framework::XView>
        GetView (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxPaneOrViewId);

    /** Request the specified view to be displayed in the specified pane.
        When the pane is not visible its creation is also requested.  The
        update that creates the actual view object is done asynchronously.
        @param rsResourceURL
            The resource URL of the view to show.
        @param rsAnchorURL
            The URL of the pane in which to show the view.
        @return
            The resource id of the requested view is returned.  With that
            the caller can, for example, call RunOnResourceActivation() to
            do some initialization after the requested view becomes active.
    */
    css::uno::Reference<css::drawing::framework::XResourceId> RequestView (
        const ::rtl::OUString& rsResourceURL,
        const ::rtl::OUString& rsAnchorURL);

    /** Request the activation of the specified task panel in the standard
        task pane.
        @param rsTaskPanelURL
            The panel that is to be activated.
        @param bEnsureTaskPaneIsVisible
            When this is <TRUE/> then the task pane is activated when not
            yet active.
            When this flag is <FALSE/> then the requested panel
            is activated only when the task pane is already active.  When it
            is not active then this call is silently ignored.
    */
    void RequestTaskPanel (
        const ::rtl::OUString& rsTaskPanelURL,
        const bool bEnsureTaskPaneIsVisible = true);

    /** Process a slot call that requests a view shell change.
    */
    void HandleModeChangeSlot (
        sal_uLong nSlotId,
        SfxRequest& rRequest);

    /** Run the given callback when the specified event is notified by the
        ConfigurationManager.  When there are no pending requests and
        therefore no events would be notified (in the foreseeable future)
        then the callback is called immediately.
        The callback is called with a flag that tells the callback whether
        the event it waits for has been sent.
    */
    void RunOnConfigurationEvent(
        const ::rtl::OUString& rsEventType,
        const Callback& rCallback);

    /** Run the given callback when the specified resource has been
        activated.  When the resource is active already when this method is
        called then rCallback is called before this method returns.
        @param rxResourceId
            Wait for the activation of this resource before calling
            rCallback.
        @param rCallback
            The callback to be called when the resource is activated.

    */
    void RunOnResourceActivation(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
        const Callback& rCallback);

    /** Normally the requested changes of the configuration are executed
        asynchronously.  However, there is at least one situation (searching
        with the Outliner) where the surrounding code does not cope with
        this.  So, instead of calling Reschedule until the global event loop
        executes the configuration update, this method does (almost) the
        same without the reschedules.

        Do not use this method until there is absolutely no other way.
    */
    void RequestSynchronousUpdate (void);

    /** Block until the specified event is notified by the configuration
        controller.  When the configuration controller is not processing any
        requests the method returns immediately.
    */
    void WaitForEvent (const ::rtl::OUString& rsEventName) const;

    /** This is a short cut for WaitForEvent(msConfigurationUpdateEndEvent).
        Call this method to execute the pending requests.
    */
    void WaitForUpdate (void) const;

    /** Explicit request for an update of the current configuration.  Call
        this method when one of the resources managed by the sd framework
        has been activated or deactivated from the outside, i.e. not by the
        framework itself.  An example for this is a click on the closer
        button of one of the side panes.
    */
    void UpdateConfiguration (void);

    /** Return a string representation of the given XResourceId object.
    */
    static ::rtl::OUString ResourceIdToString (
        const css::uno::Reference<
            css::drawing::framework::XResourceId>& rxResourceId);

    /** Create a new XResourceId object for the given resource URL.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL);

    /** Create a new XResourceId object for the given resource URL and a
        single anchor URL.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL,
                const ::rtl::OUString& rsAnchorURL);

    /** Create a new XResourceId object for the given resource URL and the
        two given anchor URLs.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL,
                const ::rtl::OUString& rsFirstAnchorURL,
                const ::rtl::OUString& rsSecondAnchorURL);

    /** Create a new XResourceId object for the given resource URL.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL,
                const css::uno::Reference<
                    css::drawing::framework::XResourceId>& rxAnchor);

    css::uno::Reference<css::drawing::framework::XConfigurationController>
        GetConfigurationController (void) const;


private:
    typedef ::std::map<
        ViewShellBase*,
        ::boost::shared_ptr<FrameworkHelper> > InstanceMap;
    /** The instance map holds (at least) one FrameworkHelper instance for
        every ViewShellBase object.
    */
    static InstanceMap maInstanceMap;
    class ViewURLMap;
    static ::boost::scoped_ptr<ViewURLMap> mpViewURLMap;

    ViewShellBase& mrBase;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;

    class DisposeListener;
    friend class DisposeListener;
    css::uno::Reference<css::lang::XComponent>
        mxDisposeListener;

    FrameworkHelper (ViewShellBase& rBase);
    FrameworkHelper (const FrameworkHelper& rHelper); // Not implemented.
    FrameworkHelper& operator= (const FrameworkHelper& rHelper); // Not implemented.

    void Initialize (void);

    void Dispose (void);

    /** Run the given callback when an event of the specified type is
        received from the ConfigurationController or when the
        ConfigurationController has no pending change requests.
        @param rsEventType
            Run rCallback only on this event.
        @param rFilter
            This filter has to return <TRUE/> in order for rCallback to be
            called.
        @param rCallback
            The callback functor to be called.
    */
    void RunOnEvent(
        const ::rtl::OUString& rsEventType,
        const ConfigurationChangeEventFilter& rFilter,
        const Callback& rCallback) const;

    /** This disposing method is forwarded from the inner DisposeListener class.
    */
    void disposing (const css::lang::EventObject& rEventObject);
};


} } // end of namespace sd::framework



namespace sd { namespace framework {

namespace {

    class FrameworkHelperAllPassFilter
    {
    public:
        bool operator() (const css::drawing::framework::ConfigurationChangeEvent&) { return true; }
    };


    class FrameworkHelperResourceIdFilter
    {
    public:
        FrameworkHelperResourceIdFilter (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);
        bool operator() (const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        { return mxResourceId.is() && rEvent.ResourceId.is()
                && mxResourceId->compareTo(rEvent.ResourceId) == 0; }
    private:
        css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;
    };

} // end of anonymous namespace




} } // end of namespace sd::framework


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
