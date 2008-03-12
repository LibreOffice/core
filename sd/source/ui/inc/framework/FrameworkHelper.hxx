/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FrameworkHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:45:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_FRAMEWORK_FRAMEWORK_HELPER_HXX
#define SD_FRAMEWORK_FRAMEWORK_HELPER_HXX

#include "ViewShell.hxx"

#include "tools/SdGlobalResourceContainer.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONFIGURATIONCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XVIEWCONTROLLER_HPP_
#include <com/sun/star/drawing/framework/XViewController.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XVIEW_HPP_
#include <com/sun/star/drawing/framework/XView.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

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

    /** Return the FrameworkHelper object that is associated with the given
        ViewShellBase.  If such an object does not yet exist, a new one is
        created.
    */
    static ::boost::shared_ptr<FrameworkHelper> Instance (ViewShellBase& rBase);

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
        const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XView>& rxView);

    ~FrameworkHelper (void);

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
        @return
            An empty reference is returned when for example the specified pane
            does not exist or is not visible or does not show a view or one
            of the involved objects does not support XTunnel (where
            necessary).
    */
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XView>
        GetView (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::framework::XResourceId>& rxResourceId);

    /** Request the specified view to be displayed in the specified pane.
        When the pane is not visible its creation is also requested.  The
        update that creates the actual view object is done asynchronously.
        @param rsResourceURL
            The resource URL of the view to show.
        @param rsAnchorURL
            The URL of the pane in which to show the view.
    */
    void RequestView (
        const ::rtl::OUString& rsResourceURL,
        const ::rtl::OUString& rsAnchorURL);

    /** Request the activation of the specified task panel in the standard
        task pane.
    */
    void RequestTaskPanel (
        const ::rtl::OUString& rsTaskPanelURL);

    /** Process a slot call that requests a view shell change.
    */
    void HandleModeChangeSlot (
        ULONG nSlotId,
        SfxRequest& rRequest);

    class Callback { public: virtual void operator() (bool bEventSeen) = 0; };
    /** Run the given callback when the specified event is notified by the
        ConfigurationManager.  When there are no pending requests and
        therefore no events would be notified (in the foreseeable future)
        then the callback is called immediately.
        The callback is called with a flag that tells the callback whether
        the event it waits for has been sent.
    */
    template<class C>
    void RunOnConfigurationEvent(
        const ::rtl::OUString& rsEventType,
        const C& rCallback);

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

    typedef ::boost::function<
        void(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::drawing::framework::XResourceId>&)
        > ResourceFunctor;

    /** Return a string representation of the given XResourceId object.
    */
    static ::rtl::OUString ResourceIdToString (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::framework::XResourceId>& rxResourceId);

    /** Create a new XResourceId object for the given resource URL.
    */
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL);

    /** Create a new XResourceId object for the given resource URL and a
        single anchor URL.
    */
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL,
                const ::rtl::OUString& rsAnchorURL);

    /** Create a new XResourceId object for the given resource URL and the
        two given anchor URLs.
    */
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL,
                const ::rtl::OUString& rsFirstAnchorURL,
                const ::rtl::OUString& rsSecondAnchorURL);

    /** Create a new XResourceId object for the given resource URL.
    */
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XResourceId>
            CreateResourceId (
                const ::rtl::OUString& rsResourceURL,
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::drawing::framework::XResourceId>& rxAnchor);

    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        GetConfigurationController (void) const;

private:
    typedef ::std::map<ViewShellBase*,::boost::shared_ptr<FrameworkHelper> > InstanceMap;
    /** The instance map holds (at least) one FrameworkHelper instance for
        every ViewShellBase object.
    */
    static InstanceMap maInstanceMap;
    class ViewURLMap;
    static ::boost::scoped_ptr<ViewURLMap> mpViewURLMap;

    ViewShellBase& mrBase;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XViewController>
        mxViewController;

    class DisposeListener;
    friend class DisposeListener;
    ::com::sun::star::uno::Reference<com::sun::star::lang::XComponent>
        mxDisposeListener;

    FrameworkHelper (ViewShellBase& rBase);
    FrameworkHelper (const FrameworkHelper& rHelper); // Not implemented.
    FrameworkHelper& operator= (const FrameworkHelper& rHelper); // Not implemented.

    void Initialize (void);

    void Dispose (void);

    /** Run the given callback when an event of the specified type is
        received from the ConfigurationController or when the
        ConfigurationController has no pending change requests.
    */
    void RunOnEvent(
        const ::rtl::OUString& rsEventType,
        ::std::auto_ptr<Callback> pCallback) const;

    /** This disposing method is forwarded from the inner DisposeListener class.
    */
    void disposing (const ::com::sun::star::lang::EventObject& rEventObject);
};



/** This functor wraps a slot call.  It is used to call a slot after an
    asynchronous configuration update, eg after switching views.
*/
class DispatchCaller
{
    SfxDispatcher& mrDispatcher;
    USHORT mnSId;
public:
    /** Create a new DispatchCaller object that, when executed, will call
        the given slot at the given dispatcher.
    */
    DispatchCaller(SfxDispatcher& rDispatcher, USHORT nSId);
    void operator() (bool bEventSeen);
};


} } // end of namespace sd::framework



namespace {

template<class T>
    class CallbackAdapter : public sd::framework::FrameworkHelper::Callback
    {
    public:
        CallbackAdapter<T> (const CallbackAdapter<T>& rCA) : mT(rCA.mT) {}
        CallbackAdapter<T> (const T& t) : mT(t) {}
        virtual ~CallbackAdapter<T> (void) {}
        virtual void operator() (bool bEventSeen) { mT(bEventSeen); }
    private:
        T mT;
    };



} // end of anonymous namespace



namespace sd { namespace framework {

template<class C>
    void FrameworkHelper::RunOnConfigurationEvent(
        const ::rtl::OUString& rsEventType,
        const C& rCallback)
{
    RunOnEvent(rsEventType, ::std::auto_ptr<Callback>(new CallbackAdapter<C>(rCallback)));
}

} } // end of namespace sd::framework


#endif
