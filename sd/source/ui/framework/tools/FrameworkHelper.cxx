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

#include <osl/time.h>

#include "framework/FrameworkHelper.hxx"

#include "framework/ConfigurationController.hxx"
#include "framework/ResourceId.hxx"
#include "framework/ViewShellWrapper.hxx"
#include "ViewShellBase.hxx"
#include "FrameView.hxx"
#include "DrawViewShell.hxx"
#include "ViewShellHint.hxx"
#include "DrawController.hxx"
#include "app.hrc"
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <cppuhelper/compbase1.hxx>
#include <svl/lstner.hxx>

#include <comphelper/stl_types.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include "MutexOwner.hxx"
#include "vcl/svapp.hxx"
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>
#include <tools/diagnose_ex.h>
#include <boost/unordered_map.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;


namespace {


//----- CallbackCaller --------------------------------------------------------

typedef ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::drawing::framework::XConfigurationChangeListener
    > CallbackCallerInterfaceBase;

/** A CallbackCaller registers as listener at an XConfigurationController
    object and waits for the notification of one type of event.  When that
    event is received, or when the CallbackCaller detects at its
    construction that the event will not be sent in the near future, the
    actual callback object is called and the CallbackCaller destroys itself.
*/
class CallbackCaller
    : public ::sd::MutexOwner,
      public CallbackCallerInterfaceBase
{
public:
    /** Create a new CallbackCaller object.  This object controls its own
        lifetime by acquiring a reference to itself in the constructor.
        When it detects that the event will not be notified in the near
        future (because the queue of pending configuration change operations
        is empty and therefore no event will be sent int the near future, it
        does not acquires a reference and thus initiates its destruction in
        the constructor.)
        @param rBase
            This ViewShellBase object is used to determine the
            XConfigurationController at which to register.
        @param rsEventType
            The event type which the callback is waiting for.
        @param pCallback
            The callback object which is to be notified.  The caller will
            typically release his reference to the caller so that when the
            CallbackCaller dies (after having called the callback) the
            callback is destroyed.
    */
    CallbackCaller (
        ::sd::ViewShellBase& rBase,
        const OUString& rsEventType,
        const ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter& rFilter,
        const ::sd::framework::FrameworkHelper::Callback& rCallback);
    virtual ~CallbackCaller (void);

    virtual void SAL_CALL disposing (void);
    // XEventListener
    virtual void SAL_CALL disposing (const lang::EventObject& rEvent)
        throw (RuntimeException);
    // XConfigurationChangeListener
    virtual void SAL_CALL notifyConfigurationChange (const ConfigurationChangeEvent& rEvent)
        throw (RuntimeException);

private:
    OUString msEventType;
    Reference<XConfigurationController> mxConfigurationController;
    ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter maFilter;
    ::sd::framework::FrameworkHelper::Callback maCallback;
};




//----- LifetimeController ----------------------------------------------------

typedef ::cppu::WeakComponentImplHelper1 <
    ::com::sun::star::lang::XEventListener
    > LifetimeControllerInterfaceBase;

/** This class helps controlling the lifetime of the
    FrameworkHelper. Register at a ViewShellBase object and an XController
    object and call Dispose() at the associated FrameworkHelper object when
    one of them and Release() when both of them are destroyed.
*/
class LifetimeController
    : public ::sd::MutexOwner,
      public LifetimeControllerInterfaceBase,
      public SfxListener
{
public:
    explicit LifetimeController (::sd::ViewShellBase& rBase);
    virtual ~LifetimeController (void);

    virtual void SAL_CALL disposing (void);

    /** XEventListener.  This method is called when the frame::XController
        is being destroyed.
    */
    virtual void SAL_CALL disposing (const lang::EventObject& rEvent)
        throw (RuntimeException);

    /** This method is called when the ViewShellBase is being destroyed.
    */
    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint);

private:
    ::sd::ViewShellBase& mrBase;
    bool mbListeningToViewShellBase;
    bool mbListeningToController;

    /** When one or both of the mbListeningToViewShellBase and
        mbListeningToController members were modified then call this method
        to either dispose or release the associated FrameworkHelper.
    */
    void Update (void);
};



} // end of anonymous namespace

namespace sd { namespace framework {

// Pane URLS.

const OUString FrameworkHelper::msPaneURLPrefix("private:resource/pane/");
const OUString FrameworkHelper::msCenterPaneURL( msPaneURLPrefix + "CenterPane");
const OUString FrameworkHelper::msFullScreenPaneURL( msPaneURLPrefix + "FullScreenPane");
const OUString FrameworkHelper::msLeftImpressPaneURL( msPaneURLPrefix + "LeftImpressPane");
const OUString FrameworkHelper::msLeftDrawPaneURL( msPaneURLPrefix + "LeftDrawPane");
const OUString FrameworkHelper::msSidebarPaneURL( msPaneURLPrefix + "SidebarPane");


// View URLs.

const OUString FrameworkHelper::msViewURLPrefix("private:resource/view/");
const OUString FrameworkHelper::msImpressViewURL( msViewURLPrefix + "ImpressView");
const OUString FrameworkHelper::msDrawViewURL( msViewURLPrefix + "GraphicView");
const OUString FrameworkHelper::msOutlineViewURL( msViewURLPrefix + "OutlineView");
const OUString FrameworkHelper::msNotesViewURL( msViewURLPrefix + "NotesView");
const OUString FrameworkHelper::msHandoutViewURL( msViewURLPrefix + "HandoutView");
const OUString FrameworkHelper::msSlideSorterURL( msViewURLPrefix + "SlideSorter");
const OUString FrameworkHelper::msPresentationViewURL( msViewURLPrefix + "PresentationView");
const OUString FrameworkHelper::msSidebarViewURL( msViewURLPrefix + "SidebarView");


// Tool bar URLs.

const OUString FrameworkHelper::msToolBarURLPrefix("private:resource/toolbar/");
const OUString FrameworkHelper::msViewTabBarURL( msToolBarURLPrefix + "ViewTabBar");


// Task panel URLs.
const OUString FrameworkHelper::msTaskPanelURLPrefix( "private:resource/toolpanel/" );
const OUString FrameworkHelper::msAllMasterPagesTaskPanelURL( msTaskPanelURLPrefix + "AllMasterPages" );
const OUString FrameworkHelper::msRecentMasterPagesTaskPanelURL( msTaskPanelURLPrefix + "RecentMasterPages" );
const OUString FrameworkHelper::msUsedMasterPagesTaskPanelURL( msTaskPanelURLPrefix + "UsedMasterPages" );
const OUString FrameworkHelper::msLayoutTaskPanelURL( msTaskPanelURLPrefix + "Layouts" );
const OUString FrameworkHelper::msTableDesignPanelURL( msTaskPanelURLPrefix + "TableDesign" );
const OUString FrameworkHelper::msCustomAnimationTaskPanelURL( msTaskPanelURLPrefix + "CustomAnimations" );
const OUString FrameworkHelper::msSlideTransitionTaskPanelURL( msTaskPanelURLPrefix + "SlideTransitions" );


// Event URLs.
const OUString FrameworkHelper::msResourceActivationRequestEvent( "ResourceActivationRequested" );
const OUString FrameworkHelper::msResourceDeactivationRequestEvent( "ResourceDeactivationRequest" );
const OUString FrameworkHelper::msResourceActivationEvent( "ResourceActivation" );
const OUString FrameworkHelper::msResourceDeactivationEvent( "ResourceDeactivation" );
const OUString FrameworkHelper::msResourceDeactivationEndEvent( "ResourceDeactivationEnd" );
const OUString FrameworkHelper::msConfigurationUpdateStartEvent( "ConfigurationUpdateStart" );
const OUString FrameworkHelper::msConfigurationUpdateEndEvent( "ConfigurationUpdateEnd" );


// Service names of controllers.
const OUString FrameworkHelper::msModuleControllerService("com.sun.star.drawing.framework.ModuleController");
const OUString FrameworkHelper::msConfigurationControllerService("com.sun.star.drawing.framework.ConfigurationController");

//----- helper ----------------------------------------------------------------
namespace
{
    static ::boost::shared_ptr< ViewShell > lcl_getViewShell( const Reference< XResource >& i_rViewShellWrapper )
    {
        ::boost::shared_ptr< ViewShell > pViewShell;
        if ( !i_rViewShellWrapper.is() )
            return pViewShell;

        try
        {
            Reference<lang::XUnoTunnel> xViewTunnel( i_rViewShellWrapper, UNO_QUERY_THROW );
            pViewShell = reinterpret_cast< ViewShellWrapper* >(
                xViewTunnel->getSomething( ViewShellWrapper::getUnoTunnelId() ) )->GetViewShell();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return pViewShell;
    }
    Reference< XResource > lcl_getFirstViewInPane( const Reference< XConfigurationController >& i_rConfigController,
        const Reference< XResourceId >& i_rPaneId )
    {
        try
        {
            Reference< XConfiguration > xConfiguration( i_rConfigController->getRequestedConfiguration(), UNO_SET_THROW );
            Sequence< Reference< XResourceId > > aViewIds( xConfiguration->getResources(
                i_rPaneId, FrameworkHelper::msViewURLPrefix, AnchorBindingMode_DIRECT ) );
            if ( aViewIds.getLength() > 0 )
                return i_rConfigController->getResource( aViewIds[0] );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return NULL;
    }
}


//----- FrameworkHelper::ViewURLMap -------------------------------------------

/** The ViewURLMap is used to translate between the view URLs used by the
    drawing framework and the enums defined in the ViewShell class.
*/
class FrameworkHelper::ViewURLMap
    : public ::boost::unordered_map<
          OUString,
          ViewShell::ShellType,
          OUStringHash,
          ::comphelper::UStringEqual>
{
public:
    ViewURLMap (void) {}
};




//----- Framework::DiposeListener ---------------------------------------------

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        ::com::sun::star::lang::XEventListener
        > FrameworkHelperDisposeListenerInterfaceBase;
}

class FrameworkHelper::DisposeListener
    : public ::sd::MutexOwner,
      public FrameworkHelperDisposeListenerInterfaceBase
{
public:
    DisposeListener (const ::boost::shared_ptr<FrameworkHelper>& rpHelper);
    ~DisposeListener (void);

    virtual void SAL_CALL disposing (void);

    virtual void SAL_CALL disposing (const lang::EventObject& rEventObject)
        throw(RuntimeException);

private:
    ::boost::shared_ptr<FrameworkHelper> mpHelper;
};




//----- FrameworkHelper::Deleter ----------------------------------------------

class FrameworkHelper::Deleter
{
public:
    void operator()(FrameworkHelper* pObject)
    {
        delete pObject;
    }
};




//----- FrameworkHelper -------------------------------------------------------

::boost::scoped_ptr<FrameworkHelper::ViewURLMap> FrameworkHelper::mpViewURLMap(new ViewURLMap());


FrameworkHelper::InstanceMap FrameworkHelper::maInstanceMap;



::boost::shared_ptr<FrameworkHelper> FrameworkHelper::Instance (
    const Reference<frame::XController>& rxController)
{
    // Tunnel through the controller to obtain a ViewShellBase.
    Reference<lang::XUnoTunnel> xTunnel (rxController, UNO_QUERY);
    if (xTunnel.is())
    {
        ::sd::DrawController* pController = reinterpret_cast<sd::DrawController*>(
            xTunnel->getSomething(sd::DrawController::getUnoTunnelId()));
        if (pController != NULL)
        {
            ViewShellBase* pBase = pController->GetViewShellBase();
            if (pBase != NULL)
                return Instance(*pBase);
        }
    }

    return ::boost::shared_ptr<FrameworkHelper>();
}




::boost::shared_ptr<FrameworkHelper> FrameworkHelper::Instance (ViewShellBase& rBase)
{

    ::boost::shared_ptr<FrameworkHelper> pHelper;

    InstanceMap::const_iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper == maInstanceMap.end())
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (iHelper == maInstanceMap.end())
        {
            pHelper = ::boost::shared_ptr<FrameworkHelper>(
                new FrameworkHelper(rBase),
                FrameworkHelper::Deleter());
            pHelper->Initialize();
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            maInstanceMap[&rBase] = pHelper;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
        pHelper = iHelper->second;
    }

    return pHelper;
}




void FrameworkHelper::DisposeInstance (ViewShellBase& rBase)
{
    InstanceMap::iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper != maInstanceMap.end())
    {
        iHelper->second->Dispose();
    }
}




void FrameworkHelper::ReleaseInstance (ViewShellBase& rBase)
{
    InstanceMap::iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper != maInstanceMap.end())
        maInstanceMap.erase(iHelper);
}




FrameworkHelper::FrameworkHelper (ViewShellBase& rBase)
    : mrBase(rBase),
      mxConfigurationController(),
      mxDisposeListener()

{
    Reference<XControllerManager> xControllerManager (rBase.GetController(), UNO_QUERY);
    if (xControllerManager.is())
    {
        mxConfigurationController = xControllerManager->getConfigurationController();
    }

    new LifetimeController(mrBase);
}




void FrameworkHelper::Initialize (void)
{
    mxDisposeListener = new DisposeListener(shared_from_this());
}




FrameworkHelper::~FrameworkHelper (void)
{
}




void FrameworkHelper::Dispose (void)
{
    if (mxDisposeListener.is())
        mxDisposeListener->dispose();
    mxConfigurationController = NULL;
}




bool FrameworkHelper::IsValid (void)
{
    return mxConfigurationController.is();
}




::boost::shared_ptr<ViewShell> FrameworkHelper::GetViewShell (const OUString& rsPaneURL)
{
    if ( !mxConfigurationController.is() )
        return ::boost::shared_ptr<ViewShell>();

    Reference<XResourceId> xPaneId( CreateResourceId( rsPaneURL ) );
    return lcl_getViewShell( lcl_getFirstViewInPane( mxConfigurationController, xPaneId ) );
}




::boost::shared_ptr<ViewShell> FrameworkHelper::GetViewShell (const Reference<XView>& rxView)
{
    return lcl_getViewShell( rxView.get() );
}




Reference<XView> FrameworkHelper::GetView (const Reference<XResourceId>& rxPaneOrViewId)
{
    Reference<XView> xView;

    if ( ! rxPaneOrViewId.is() || ! mxConfigurationController.is())
        return NULL;

    try
    {
        if (rxPaneOrViewId->getResourceURL().match(msViewURLPrefix))
        {
            xView.set( mxConfigurationController->getResource( rxPaneOrViewId ), UNO_QUERY );
        }
        else
        {
            xView.set( lcl_getFirstViewInPane( mxConfigurationController, rxPaneOrViewId ), UNO_QUERY );
        }
    }
    catch (lang::DisposedException&)
    {
        Dispose();
    }
    catch (RuntimeException&)
    {
    }

    return xView;
}




Reference<awt::XWindow> FrameworkHelper::GetPaneWindow (const Reference<XResourceId>& rxPaneId)
{
    Reference<awt::XWindow> xWindow;

    if (rxPaneId.is() && mxConfigurationController.is())
    {
        try
        {
            if (rxPaneId->getResourceURL().match(msPaneURLPrefix))
            {
                Reference<XPane> xPane (mxConfigurationController->getResource(rxPaneId), UNO_QUERY);
                if (xPane.is())
                    xWindow = xPane->getWindow();
            }
        }
        catch (lang::DisposedException&)
        {
            Dispose();
        }
        catch (RuntimeException&)
        {
        }
    }

    return xWindow;
}




Reference<XResource> FrameworkHelper::GetResource (const Reference<XResourceId>& rxResourceId)
{
    Reference<XResource> xResource;

    if (rxResourceId.is() && mxConfigurationController.is())
    {
        try
        {
            return mxConfigurationController->getResource(rxResourceId);
        }
        catch (lang::DisposedException&)
        {
            Dispose();
        }
        catch (RuntimeException&)
        {
        }
    }

    return NULL;
}




Reference<XResourceId> FrameworkHelper::RequestView (
    const OUString& rsResourceURL,
    const OUString& rsAnchorURL)
{
    Reference<XResourceId> xViewId;

    try
    {
        if (mxConfigurationController.is())
        {
            mxConfigurationController->requestResourceActivation(
                CreateResourceId(rsAnchorURL),
                ResourceActivationMode_ADD);
            xViewId = CreateResourceId(rsResourceURL, rsAnchorURL);
            mxConfigurationController->requestResourceActivation(
                xViewId,
                ResourceActivationMode_REPLACE);
        }
    }
    catch (lang::DisposedException&)
    {
        Dispose();
        xViewId = NULL;
    }
    catch (RuntimeException&)
    {
        xViewId = NULL;
    }

    return xViewId;
}




Reference<XResourceId> FrameworkHelper::RequestSidebarPanel (
    const OUString& rsTaskPanelURL,
    const bool bEnsureTaskPaneIsVisible)
{
    try
    {
        if (mxConfigurationController.is())
        {
            // Check the existence of the task pane.
            if ( ! bEnsureTaskPaneIsVisible)
            {
                Reference<XConfiguration> xConfiguration (
                    mxConfigurationController->getCurrentConfiguration());
                if (xConfiguration.is())
                    if ( ! xConfiguration->hasResource(
                            CreateResourceId(msSidebarViewURL, msSidebarPaneURL)))
                    {
                        // Task pane is not active.  Do not force it.
                        return NULL;
                    }
            }

            // Create the resource id from URLs for the sidebar pane
            // and view and the requested panel.
            mxConfigurationController->requestResourceActivation(
                CreateResourceId(msSidebarPaneURL),
                ResourceActivationMode_ADD);
            mxConfigurationController->requestResourceActivation(
                CreateResourceId(msSidebarViewURL, msSidebarPaneURL),
                ResourceActivationMode_REPLACE);
            Reference<XResourceId> xPanelId (CreateResourceId(rsTaskPanelURL, msSidebarViewURL, msSidebarPaneURL));
            mxConfigurationController->requestResourceActivation(
                xPanelId,
                ResourceActivationMode_REPLACE);

            return xPanelId;
        }
    }
    catch (lang::DisposedException&)
    {
        Dispose();
    }
    catch (RuntimeException&)
    {}

    return NULL;
}

ViewShell::ShellType FrameworkHelper::GetViewId (const OUString& rsViewURL)
{
    if (mpViewURLMap->empty())
    {
        (*mpViewURLMap)[msImpressViewURL] = ViewShell::ST_IMPRESS;
        (*mpViewURLMap)[msDrawViewURL] = ViewShell::ST_DRAW;
        (*mpViewURLMap)[msOutlineViewURL] = ViewShell::ST_OUTLINE;
        (*mpViewURLMap)[msNotesViewURL] = ViewShell::ST_NOTES;
        (*mpViewURLMap)[msHandoutViewURL] = ViewShell::ST_HANDOUT;
        (*mpViewURLMap)[msSlideSorterURL] = ViewShell::ST_SLIDE_SORTER;
        (*mpViewURLMap)[msPresentationViewURL] = ViewShell::ST_PRESENTATION;
        (*mpViewURLMap)[msSidebarViewURL] = ViewShell::ST_SIDEBAR;
    }
    ViewURLMap::const_iterator iView (mpViewURLMap->find(rsViewURL));
    if (iView != mpViewURLMap->end())
        return iView->second;
    else
        return ViewShell::ST_NONE;
}




OUString FrameworkHelper::GetViewURL (ViewShell::ShellType eType)
{
    switch (eType)
    {
        case ViewShell::ST_IMPRESS : return msImpressViewURL;
        case ViewShell::ST_DRAW : return msDrawViewURL;
        case ViewShell::ST_OUTLINE : return msOutlineViewURL;
        case ViewShell::ST_NOTES : return msNotesViewURL;
        case ViewShell::ST_HANDOUT : return msHandoutViewURL;
        case ViewShell::ST_SLIDE_SORTER : return msSlideSorterURL;
        case ViewShell::ST_PRESENTATION : return msPresentationViewURL;
        case ViewShell::ST_SIDEBAR : return msSidebarViewURL;
        default:
            return OUString();
    }
}




void FrameworkHelper::HandleModeChangeSlot (
    sal_uLong nSlotId,
    SfxRequest& rRequest)
{
    sal_Bool bIsActive = sal_True;

    if ( ! mxConfigurationController.is())
        return;

    switch (nSlotId)
    {
        case SID_DRAWINGMODE:
        case SID_NOTESMODE:
        case SID_HANDOUTMODE:
        case SID_DIAMODE:
        case SID_OUTLINEMODE:
        {
            const SfxItemSet* pRequestArguments = rRequest.GetArgs();
            if (pRequestArguments)
            {
                SFX_REQUEST_ARG (rRequest,
                    pIsActive,
                    SfxBoolItem,
                    (sal_uInt16)nSlotId,
                    sal_False);
                bIsActive = pIsActive->GetValue ();
            }
        }
        break;
    }

    try
    {
        if ( ! mxConfigurationController.is())
            throw RuntimeException();


        Reference<XResourceId> xPaneId (
            CreateResourceId(framework::FrameworkHelper::msCenterPaneURL));
        Reference<XView> xView (GetView(xPaneId));
        ::boost::shared_ptr<ViewShell> pCenterViewShell (GetViewShell(xView));

        OUString sRequestedView;
        if (bIsActive)
        {
            switch (nSlotId)
            {
                case SID_NORMAL_MULTI_PANE_GUI:
                case SID_DRAWINGMODE:
                    sRequestedView = FrameworkHelper::msImpressViewURL;
                    break;

                case SID_NOTESMODE:
                    sRequestedView = FrameworkHelper::msNotesViewURL;
                break;

                case SID_HANDOUTMODE:
                    sRequestedView = FrameworkHelper::msHandoutViewURL;
                    break;

                case SID_SLIDE_SORTER_MULTI_PANE_GUI:
                case SID_DIAMODE:
                    sRequestedView = FrameworkHelper::msSlideSorterURL;
                    break;

                case SID_OUTLINEMODE:
                    sRequestedView = FrameworkHelper::msOutlineViewURL;
                    break;
            }
        }

        if (xView.is()
            && xView->getResourceId()->getResourceURL().equals(sRequestedView))
        {
            // We do not have to switch the view shell but maybe the edit mode
            // has changed.
            DrawViewShell* pDrawViewShell
                = dynamic_cast<DrawViewShell*>(pCenterViewShell.get());
            if (pDrawViewShell != NULL)
            {
                pCenterViewShell->Broadcast (
                    ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

                pDrawViewShell->ChangeEditMode (
                    EM_PAGE, pDrawViewShell->IsLayerModeActive());

                pCenterViewShell->Broadcast (
                    ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_END));
            }
        }
        else
        {
            mxConfigurationController->requestResourceActivation(
                CreateResourceId(sRequestedView, msCenterPaneURL),
                ResourceActivationMode_REPLACE);
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




void FrameworkHelper::RunOnConfigurationEvent(
    const OUString& rsEventType,
    const Callback& rCallback)
{
    RunOnEvent(
        rsEventType,
        FrameworkHelperAllPassFilter(),
        rCallback);
}




void FrameworkHelper::RunOnResourceActivation(
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId,
    const Callback& rCallback)
{
    if (mxConfigurationController.is()
        && mxConfigurationController->getResource(rxResourceId).is())
    {
        rCallback(false);
    }
    else
    {
        RunOnEvent(
            msResourceActivationEvent,
            FrameworkHelperResourceIdFilter(rxResourceId),
            rCallback);
    }
}

/** A callback that sets a flag to a specified value when the callback is
    called.
*/
class FlagUpdater
{
public:
    FlagUpdater (bool& rFlag) : mrFlag(rFlag) {}
    void operator() (bool) const {mrFlag = true;}
private:
    bool& mrFlag;
};




void FrameworkHelper::RequestSynchronousUpdate (void)
{
    rtl::Reference<ConfigurationController> pCC (
        dynamic_cast<ConfigurationController*>(mxConfigurationController.get()));
    if (pCC.is())
        pCC->RequestSynchronousUpdate();
}




void FrameworkHelper::WaitForEvent (const OUString& rsEventType) const
{
    bool bConfigurationUpdateSeen (false);

    RunOnEvent(
        rsEventType,
        FrameworkHelperAllPassFilter(),
        FlagUpdater(bConfigurationUpdateSeen));

    sal_uInt32 nStartTime = osl_getGlobalTimer();
    while ( ! bConfigurationUpdateSeen)
    {
        Application::Reschedule();

        if( (osl_getGlobalTimer() - nStartTime) > 60000  )
        {
            OSL_FAIL("FrameworkHelper::WaitForEvent(), no event for a minute? giving up!");
            break;
        }
    }
}




void FrameworkHelper::WaitForUpdate (void) const
{
    WaitForEvent(msConfigurationUpdateEndEvent);
}




void FrameworkHelper::RunOnEvent(
    const OUString& rsEventType,
    const ConfigurationChangeEventFilter& rFilter,
    const Callback& rCallback) const
{
    new CallbackCaller(mrBase,rsEventType,rFilter,rCallback);
}




void FrameworkHelper::disposing (const lang::EventObject& rEventObject)
{
    if (rEventObject.Source == mxConfigurationController)
        mxConfigurationController = NULL;
}




void FrameworkHelper::UpdateConfiguration (void)
{
    if (mxConfigurationController.is())
    {
        try
        {
            if (mxConfigurationController.is())
                mxConfigurationController->update();
        }
        catch (lang::DisposedException&)
        {
            Dispose();
        }
        catch (RuntimeException&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}




OUString FrameworkHelper::ResourceIdToString (const Reference<XResourceId>& rxResourceId)
{
    OUString sString;
    if (rxResourceId.is())
    {
        sString += rxResourceId->getResourceURL();
        if (rxResourceId->hasAnchor())
        {
            Sequence<OUString> aAnchorURLs (rxResourceId->getAnchorURLs());
            for (sal_Int32 nIndex=0; nIndex < aAnchorURLs.getLength(); ++nIndex)
            {
                sString += " | ";
                sString += aAnchorURLs[nIndex];
            }
        }
    }
    return sString;
}




Reference<XResourceId> FrameworkHelper::CreateResourceId (const OUString& rsResourceURL)
{
    return new ::sd::framework::ResourceId(rsResourceURL);
}




Reference<XResourceId> FrameworkHelper::CreateResourceId (
    const OUString& rsResourceURL,
    const OUString& rsAnchorURL)
{
    return new ::sd::framework::ResourceId(rsResourceURL, rsAnchorURL);
}




Reference<XResourceId> FrameworkHelper::CreateResourceId (
    const OUString& rsResourceURL,
    const OUString& rsFirstAnchorURL,
    const OUString& rsSecondAnchorURL)
{
    ::std::vector<OUString> aAnchorURLs (2);
    aAnchorURLs[0] = rsFirstAnchorURL;
    aAnchorURLs[1] = rsSecondAnchorURL;
    return new ::sd::framework::ResourceId(rsResourceURL, aAnchorURLs);
}




Reference<XResourceId> FrameworkHelper::CreateResourceId (
    const OUString& rsResourceURL,
    const Reference<XResourceId>& rxAnchorId)
{
    if (rxAnchorId.is())
        return new ::sd::framework::ResourceId(
            rsResourceURL,
            rxAnchorId->getResourceURL(),
            rxAnchorId->getAnchorURLs());
    else
        return new ::sd::framework::ResourceId(rsResourceURL);
}




Reference<XConfigurationController> FrameworkHelper::GetConfigurationController (void) const
{
    return mxConfigurationController;
}




//----- FrameworkHelper::DisposeListener --------------------------------------

FrameworkHelper::DisposeListener::DisposeListener (
    const ::boost::shared_ptr<FrameworkHelper>& rpHelper)
    : FrameworkHelperDisposeListenerInterfaceBase(maMutex),
      mpHelper(rpHelper)
{
    Reference<XComponent> xComponent (mpHelper->mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(this);
}




FrameworkHelper::DisposeListener::~DisposeListener (void)
{
}




void SAL_CALL FrameworkHelper::DisposeListener::disposing (void)
{
    Reference<XComponent> xComponent (mpHelper->mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(this);

    mpHelper.reset();
}




void SAL_CALL FrameworkHelper::DisposeListener::disposing (const lang::EventObject& rEventObject)
    throw(RuntimeException)
{
    if (mpHelper.get() != NULL)
        mpHelper->disposing(rEventObject);
}




//===== FrameworkHelperResourceIdFilter =======================================

FrameworkHelperResourceIdFilter::FrameworkHelperResourceIdFilter (
    const Reference<XResourceId>& rxResourceId)
    : mxResourceId(rxResourceId)
{
}


} } // end of namespace sd::framework

namespace {

//===== CallbackCaller ========================================================

CallbackCaller::CallbackCaller (
    ::sd::ViewShellBase& rBase,
    const OUString& rsEventType,
    const ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter& rFilter,
    const ::sd::framework::FrameworkHelper::Callback& rCallback)
    : CallbackCallerInterfaceBase(MutexOwner::maMutex),
      msEventType(rsEventType),
      mxConfigurationController(),
      maFilter(rFilter),
      maCallback(rCallback)
{
    try
    {
        Reference<XControllerManager> xControllerManager (rBase.GetController(), UNO_QUERY_THROW);
        mxConfigurationController = xControllerManager->getConfigurationController();
        if (mxConfigurationController.is())
        {
            if (mxConfigurationController->hasPendingRequests())
                mxConfigurationController->addConfigurationChangeListener(this,msEventType,Any());
            else
            {
                // There are no requests waiting to be processed.  Therefore
                // no event, especially not the one we are waiting for, will
                // be sent in the near future and the callback would never be
                // called.
                // Call the callback now and tell him that the event it is
                // waiting for was not sent.
                mxConfigurationController = NULL;
                maCallback(false);
            }
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




CallbackCaller::~CallbackCaller (void)
{
}




void CallbackCaller::disposing (void)
{
    try
    {
        if (mxConfigurationController.is())
        {
            Reference<XConfigurationController> xCC (mxConfigurationController);
            mxConfigurationController = NULL;
            xCC->removeConfigurationChangeListener(this);
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}




void SAL_CALL CallbackCaller::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxConfigurationController)
    {
        mxConfigurationController = NULL;
        maCallback(false);
    }
}




void SAL_CALL CallbackCaller::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Type.equals(msEventType) && maFilter(rEvent))
    {
        maCallback(true);
        if (mxConfigurationController.is())
        {
            // Reset the reference to the configuration controller so that
            // dispose() will not try to remove the listener a second time.
            Reference<XConfigurationController> xCC (mxConfigurationController);
            mxConfigurationController = NULL;

            // Removing this object from the controller may very likely lead
            // to its destruction, so no calls after that.
            xCC->removeConfigurationChangeListener(this);
        }
    }
}




//----- LifetimeController -------------------------------------------------

LifetimeController::LifetimeController (::sd::ViewShellBase& rBase)
    : LifetimeControllerInterfaceBase(maMutex),
      mrBase(rBase),
      mbListeningToViewShellBase(false),
      mbListeningToController(false)
{
    // Register as listener at the ViewShellBase.  Because that is not done
    // via a reference we have to increase the reference count manually.
    // This is necessary even though listening to the XController did
    // increase the reference count because the controller may release its
    // reference to us before the ViewShellBase is destroyed.
    StartListening(mrBase);
    acquire();
    mbListeningToViewShellBase = true;

    Reference<XComponent> xComponent (rBase.GetController(), UNO_QUERY);
    if (xComponent.is())
    {
        xComponent->addEventListener(this);
        mbListeningToController = true;
    }
}




LifetimeController::~LifetimeController (void)
{
    OSL_ASSERT(!mbListeningToController && !mbListeningToViewShellBase);
}




void LifetimeController::disposing (void)
{
}




void SAL_CALL LifetimeController::disposing (const lang::EventObject& rEvent)
    throw(RuntimeException)
{
    (void)rEvent;
    mbListeningToController = false;
    Update();
}




void LifetimeController::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    (void)rBroadcaster;
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint != NULL && pSimpleHint->GetId() == SFX_HINT_DYING)
    {
        mbListeningToViewShellBase = false;
        Update();
        release();
    }
}




void LifetimeController::Update (void)
{
    if (mbListeningToViewShellBase && mbListeningToController)
    {
        // Both the controller and the ViewShellBase are alive.  Keep
        // waiting for their destruction.
    }
    else if (mbListeningToViewShellBase)
    {
        // The controller has been destroyed but the ViewShellBase is still
        // alive.  Dispose the associated FrameworkHelper but keep it around
        // so that no new instance is created for the dying framework.
        ::sd::framework::FrameworkHelper::DisposeInstance(mrBase);
    }
    else
    {
        // Both the controller and the ViewShellBase have been destroyed.
        // Remove the FrameworkHelper so that the next call its Instance()
        // method can create a new instance.
        ::sd::framework::FrameworkHelper::ReleaseInstance(mrBase);
    }
}



} // end of anonymous namespace.

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
