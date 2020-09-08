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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_FRAMEWORKHELPER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_FRAMEWORKHELPER_HXX

#include <ViewShell.hxx>

#include <tools/SdGlobalResourceContainer.hxx>

#include <functional>
#include <map>
#include <memory>

namespace com::sun::star::drawing::framework { class XConfigurationController; }
namespace com::sun::star::drawing::framework { class XResourceId; }
namespace com::sun::star::drawing::framework { class XView; }
namespace com::sun::star::drawing::framework { struct ConfigurationChangeEvent; }

namespace sd {
class ViewShellBase;
}


namespace sd::framework {

/** The FrameworkHelper is a convenience class that simplifies the
    access to the drawing framework.
    It has three main tasks:
    1. Provide frequently used strings of resource URLs and event names.
    2. Provide shortcuts for accessing the sd framework.
    3. Ease the migration to the drawing framework.

    Note that a FrameworkHelper disposes itself when one of the resource
    controllers called by it throws a DisposedException.
*/
class FrameworkHelper
    : public std::enable_shared_from_this<FrameworkHelper>,
      public SdGlobalResource
{
public:
    // URLs of frequently used panes.
    static const OUStringLiteral msPaneURLPrefix;
    static const OUString msCenterPaneURL;
    static const OUString msFullScreenPaneURL;
    static const OUString msLeftImpressPaneURL;
    static const OUString msLeftDrawPaneURL;

    // URLs of frequently used views.
    static const OUStringLiteral msViewURLPrefix;
    static const OUString msImpressViewURL;
    static const OUString msDrawViewURL;
    static const OUString msOutlineViewURL;
    static const OUString msNotesViewURL;
    static const OUString msHandoutViewURL;
    static const OUString msSlideSorterURL;
    static const OUString msPresentationViewURL;
    static const OUString msSidebarViewURL;

    // URLs of frequently used tool bars.
    static const OUStringLiteral msToolBarURLPrefix;
    static const OUString msViewTabBarURL;

    // URLs of task panels.
    static const OUStringLiteral msTaskPanelURLPrefix;

    // Names of frequently used events.
    static const OUStringLiteral msResourceActivationRequestEvent;
    static const OUStringLiteral msResourceDeactivationRequestEvent;
    static const OUStringLiteral msResourceActivationEvent;
    static const OUStringLiteral msResourceDeactivationEvent;
    static const OUStringLiteral msResourceDeactivationEndEvent;
    static const OUStringLiteral msConfigurationUpdateStartEvent;
    static const OUStringLiteral msConfigurationUpdateEndEvent;

    /** Return the FrameworkHelper object that is associated with the given
        ViewShellBase.  If such an object does not yet exist, a new one is
        created.
    */
    static ::std::shared_ptr<FrameworkHelper> Instance (ViewShellBase& rBase);

    /** Mark the FrameworkHelper object for the given ViewShellBase as
        disposed.  A following ReleaseInstance() call will destroy the
        FrameworkHelper object.

        Do not call this method.  It is an internally used method that can
        not be made private.
    */
    static void DisposeInstance (const ViewShellBase& rBase);

    /** Destroy the FrameworkHelper object for the given ViewShellBase.

        Do not call this method.  It is an internally used method that can
        not be made private.
    */
    static void ReleaseInstance (const ViewShellBase& rBase);

    /** Return an identifier for the given view URL.  This identifier can be
        used in a switch statement.  See GetViewURL() for a mapping in the
        opposite direction.
    */
    static ViewShell::ShellType GetViewId (const OUString& rsViewURL);

    /** Return a view URL for the given identifier.  See GetViewId() for a
        mapping in the opposite direction.
    */
    static OUString GetViewURL (ViewShell::ShellType eType);

    /** Return a ViewShell pointer for the given XView reference.  This
        assumes that the given reference is implemented by the
        ViewShellWrapper class that supports the XTunnel interface.
        @return
            When the ViewShell pointer can not be inferred from the given
            reference then an empty pointer is returned.
    */
    static ::std::shared_ptr<ViewShell> GetViewShell (
        const css::uno::Reference<css::drawing::framework::XView>& rxView);

    typedef ::std::function<bool (const css::drawing::framework::ConfigurationChangeEvent&)>
        ConfigurationChangeEventFilter;
    typedef ::std::function<void (bool bEventSeen)> Callback;
    typedef ::std::function<
        void (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>&)
        > ResourceFunctor;

    /** Test whether the called FrameworkHelper object is valid.
        @return
            When the object has already been disposed then <FALSE/> is returned.
    */
    bool IsValid() const;

    /** Return a pointer to the view shell that is displayed in the
        specified pane.  See GetView() for a variant that returns a
        reference to XView instead of a ViewShell pointer.
        @return
            An empty pointer is returned when for example the specified pane
            does not exist or is not visible or does not show a view or one
            of the involved objects does not support XUnoTunnel (where
            necessary).
    */
    ::std::shared_ptr<ViewShell> GetViewShell (const OUString& rsPaneURL);

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
    css::uno::Reference<css::drawing::framework::XView> GetView (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneOrViewId);

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
        const OUString& rsResourceURL,
        const OUString& rsAnchorURL);

    /** Process a slot call that requests a view shell change.
    */
    void HandleModeChangeSlot (
        sal_uLong nSlotId,
        SfxRequest const & rRequest);

    /** Run the given callback when the specified event is notified by the
        ConfigurationManager.  When there are no pending requests and
        therefore no events would be notified (in the foreseeable future)
        then the callback is called immediately.
        The callback is called with a flag that tells the callback whether
        the event it waits for has been sent.
    */
    void RunOnConfigurationEvent(
        const OUString& rsEventType,
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
    void RequestSynchronousUpdate();

    /** Block until the specified event is notified by the configuration
        controller.  When the configuration controller is not processing any
        requests the method returns immediately.
    */
    void WaitForEvent (const OUString& rsEventName) const;

    /** This is a short cut for WaitForEvent(msConfigurationUpdateEndEvent).
        Call this method to execute the pending requests.
    */
    void WaitForUpdate() const;

    /** Explicit request for an update of the current configuration.  Call
        this method when one of the resources managed by the sd framework
        has been activated or deactivated from the outside, i.e. not by the
        framework itself.  An example for this is a click on the closer
        button of one of the side panes.
    */
    void UpdateConfiguration();

    /** Return a string representation of the given XResourceId object.
    */
    static OUString ResourceIdToString (
        const css::uno::Reference<
            css::drawing::framework::XResourceId>& rxResourceId);

    /** Create a new XResourceId object for the given resource URL.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const OUString& rsResourceURL);

    /** Create a new XResourceId object for the given resource URL and a
        single anchor URL.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const OUString& rsResourceURL,
                const OUString& rsAnchorURL);

    /** Create a new XResourceId object for the given resource URL.
    */
    static css::uno::Reference<
        css::drawing::framework::XResourceId>
            CreateResourceId (
                const OUString& rsResourceURL,
                const css::uno::Reference<
                    css::drawing::framework::XResourceId>& rxAnchor);

    const css::uno::Reference<css::drawing::framework::XConfigurationController>&
        GetConfigurationController() const { return mxConfigurationController;}

private:
    typedef ::std::map<
        const ViewShellBase*,
        ::std::shared_ptr<FrameworkHelper> > InstanceMap;
    /** The instance map holds (at least) one FrameworkHelper instance for
        every ViewShellBase object.
    */
    static InstanceMap maInstanceMap;
    class ViewURLMap;
    static std::unique_ptr<ViewURLMap> mpViewURLMap;

    ViewShellBase& mrBase;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;

    class DisposeListener;
    friend class DisposeListener;
    css::uno::Reference<css::lang::XComponent>
        mxDisposeListener;

    FrameworkHelper (ViewShellBase& rBase);
    FrameworkHelper (const FrameworkHelper& rHelper) = delete;
    virtual ~FrameworkHelper() override;
    class Deleter; friend class Deleter;
    FrameworkHelper& operator= (const FrameworkHelper& rHelper) = delete;

    void Initialize();

    void Dispose();

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
        const OUString& rsEventType,
        const ConfigurationChangeEventFilter& rFilter,
        const Callback& rCallback) const;

    /** This disposing method is forwarded from the inner DisposeListener class.
    */
    void disposing (const css::lang::EventObject& rEventObject);
};

} // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
