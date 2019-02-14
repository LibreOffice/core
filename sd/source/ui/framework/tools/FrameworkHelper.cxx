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

#include <framework/FrameworkHelper.hxx>

#include <framework/ConfigurationController.hxx>
#include <framework/ResourceId.hxx>
#include <framework/ViewShellWrapper.hxx>
#include <ViewShellBase.hxx>
#include <FrameView.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellHint.hxx>
#include <DrawController.hxx>
#include <app.hrc>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <cppuhelper/compbase.hxx>
#include <svl/lstner.hxx>

#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <MutexOwner.hxx>
#include <vcl/svapp.hxx>
#include <osl/doublecheckedlocking.h>
#include <osl/getglobalmutex.hxx>
#include <tools/diagnose_ex.h>
#include <memory>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace {

//----- CallbackCaller --------------------------------------------------------

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XConfigurationChangeListener
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

    virtual void SAL_CALL disposing() override;
    // XEventListener
    virtual void SAL_CALL disposing (const lang::EventObject& rEvent) override;
    // XConfigurationChangeListener
    virtual void SAL_CALL notifyConfigurationChange (const ConfigurationChangeEvent& rEvent) override;

private:
    OUString const msEventType;
    Reference<XConfigurationController> mxConfigurationController;
    ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter const maFilter;
    ::sd::framework::FrameworkHelper::Callback const maCallback;
};

//----- LifetimeController ----------------------------------------------------

typedef ::cppu::WeakComponentImplHelper <
    css::lang::XEventListener
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
    virtual ~LifetimeController() override;

    virtual void SAL_CALL disposing() override;

    /** XEventListener.  This method is called when the frame::XController
        is being destroyed.
    */
    virtual void SAL_CALL disposing (const lang::EventObject& rEvent) override;

    /** This method is called when the ViewShellBase is being destroyed.
    */
    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;

private:
    ::sd::ViewShellBase& mrBase;
    bool mbListeningToViewShellBase;
    bool mbListeningToController;

    /** When one or both of the mbListeningToViewShellBase and
        mbListeningToController members were modified then call this method
        to either dispose or release the associated FrameworkHelper.
    */
    void Update();
};

} // end of anonymous namespace

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
        explicit FrameworkHelperResourceIdFilter (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);
        bool operator() (const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        { return mxResourceId.is() && rEvent.ResourceId.is()
                && mxResourceId->compareTo(rEvent.ResourceId) == 0; }
    private:
        css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;
    };

} // end of anonymous namespace

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
    ::std::shared_ptr< ViewShell > lcl_getViewShell( const Reference< XResource >& i_rViewShellWrapper )
    {
        ::std::shared_ptr< ViewShell > pViewShell;
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
            DBG_UNHANDLED_EXCEPTION("sd");
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
            DBG_UNHANDLED_EXCEPTION("sd");
        }
        return nullptr;
    }
}

//----- FrameworkHelper::ViewURLMap -------------------------------------------

/** The ViewURLMap is used to translate between the view URLs used by the
    drawing framework and the enums defined in the ViewShell class.
*/
class FrameworkHelper::ViewURLMap
    : public std::unordered_map<
          OUString,
          ViewShell::ShellType>
{
public:
    ViewURLMap() {}
};

//----- Framework::DiposeListener ---------------------------------------------

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::lang::XEventListener
        > FrameworkHelperDisposeListenerInterfaceBase;
}

class FrameworkHelper::DisposeListener
    : public ::sd::MutexOwner,
      public FrameworkHelperDisposeListenerInterfaceBase
{
public:
    explicit DisposeListener (const ::std::shared_ptr<FrameworkHelper>& rpHelper);

    virtual void SAL_CALL disposing() override;

    virtual void SAL_CALL disposing (const lang::EventObject& rEventObject) override;

private:
    ::std::shared_ptr<FrameworkHelper> mpHelper;
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

std::unique_ptr<FrameworkHelper::ViewURLMap> FrameworkHelper::mpViewURLMap(new ViewURLMap());

FrameworkHelper::InstanceMap FrameworkHelper::maInstanceMap;

::std::shared_ptr<FrameworkHelper> FrameworkHelper::Instance (ViewShellBase& rBase)
{

    ::std::shared_ptr<FrameworkHelper> pHelper;

    InstanceMap::const_iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper == maInstanceMap.end())
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (iHelper == maInstanceMap.end())
        {
            pHelper = ::std::shared_ptr<FrameworkHelper>(
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

void FrameworkHelper::Initialize()
{
    mxDisposeListener = new DisposeListener(shared_from_this());
}

FrameworkHelper::~FrameworkHelper()
{
}

void FrameworkHelper::Dispose()
{
    if (mxDisposeListener.is())
        mxDisposeListener->dispose();
    mxConfigurationController = nullptr;
}

bool FrameworkHelper::IsValid()
{
    return mxConfigurationController.is();
}

::std::shared_ptr<ViewShell> FrameworkHelper::GetViewShell (const OUString& rsPaneURL)
{
    if ( !mxConfigurationController.is() )
        return ::std::shared_ptr<ViewShell>();

    Reference<XResourceId> xPaneId( CreateResourceId( rsPaneURL ) );
    return lcl_getViewShell( lcl_getFirstViewInPane( mxConfigurationController, xPaneId ) );
}

::std::shared_ptr<ViewShell> FrameworkHelper::GetViewShell (const Reference<XView>& rxView)
{
    return lcl_getViewShell( rxView.get() );
}

Reference<XView> FrameworkHelper::GetView (const Reference<XResourceId>& rxPaneOrViewId)
{
    Reference<XView> xView;

    if ( ! rxPaneOrViewId.is() || ! mxConfigurationController.is())
        return nullptr;

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
        xViewId = nullptr;
    }
    catch (RuntimeException&)
    {
        xViewId = nullptr;
    }

    return xViewId;
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

namespace
{

void updateEditMode(const Reference<XView> &xView, const EditMode eEMode, bool updateFrameView)
{
    // Ensure we have the expected edit mode
    // The check is only for DrawViewShell as OutlineViewShell
    // and SlideSorterViewShell have no master mode
    const ::std::shared_ptr<ViewShell> pCenterViewShell (FrameworkHelper::GetViewShell(xView));
    DrawViewShell* pDrawViewShell
        = dynamic_cast<DrawViewShell*>(pCenterViewShell.get());
    if (pDrawViewShell != nullptr)
    {
        pCenterViewShell->Broadcast (
            ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_START));

        pDrawViewShell->ChangeEditMode(eEMode, pDrawViewShell->IsLayerModeActive());
        if (updateFrameView)
            pDrawViewShell->WriteFrameViewData();

        pCenterViewShell->Broadcast (
            ViewShellHint(ViewShellHint::HINT_CHANGE_EDIT_MODE_END));
    }
}

void asyncUpdateEditMode(FrameworkHelper* const pHelper, const EditMode eEMode)
{
    Reference<XResourceId> xPaneId (
        FrameworkHelper::CreateResourceId(framework::FrameworkHelper::msCenterPaneURL));
    Reference<XView> xView (pHelper->GetView(xPaneId));
    updateEditMode(xView, eEMode, true);
}

}

void FrameworkHelper::HandleModeChangeSlot (
    sal_uLong nSlotId,
    SfxRequest const & rRequest)
{
    if ( ! mxConfigurationController.is())
        return;

    // Parameters are allowed for NotesMasterPage and SlideMasterPage
    // for these command, transfor xxxxMasterPage with param = false
    // to ActivatexxxxxMode
    if (nSlotId == SID_NOTES_MASTER_MODE || nSlotId == SID_SLIDE_MASTER_MODE)
    {
        const SfxItemSet* pRequestArguments = rRequest.GetArgs();
        if (pRequestArguments)
        {
            const SfxBoolItem* pIsActive = rRequest.GetArg<SfxBoolItem>(static_cast<sal_uInt16>(nSlotId));
            if (!pIsActive->GetValue ())
            {
                if (nSlotId == SID_NOTES_MASTER_MODE)
                    nSlotId = SID_NOTES_MODE;
                else
                    nSlotId = SID_NORMAL_MULTI_PANE_GUI;
            }
        }
    }

    try
    {
        if ( ! mxConfigurationController.is())
            throw RuntimeException();

        Reference<XResourceId> xPaneId (
            CreateResourceId(framework::FrameworkHelper::msCenterPaneURL));
        Reference<XView> xView (GetView(xPaneId));

        // Compute requested view
        OUString sRequestedView;
        switch (nSlotId)
        {
            // draw
            case SID_DRAWINGMODE:
            // impress
            case SID_NORMAL_MULTI_PANE_GUI:
            case SID_SLIDE_MASTER_MODE:
                sRequestedView = FrameworkHelper::msImpressViewURL;
                break;

            case SID_NOTES_MODE:
            case SID_NOTES_MASTER_MODE:
                sRequestedView = FrameworkHelper::msNotesViewURL;
            break;

            case SID_HANDOUT_MASTER_MODE:
                sRequestedView = FrameworkHelper::msHandoutViewURL;
                break;

            case SID_SLIDE_SORTER_MULTI_PANE_GUI:
            case SID_SLIDE_SORTER_MODE:
                sRequestedView = FrameworkHelper::msSlideSorterURL;
                break;

            case SID_OUTLINE_MODE:
                sRequestedView = FrameworkHelper::msOutlineViewURL;
                break;
        }

        // Compute requested mode
        EditMode eEMode = EditMode::Page;
        if (nSlotId == SID_SLIDE_MASTER_MODE
            || nSlotId == SID_NOTES_MASTER_MODE
            || nSlotId == SID_HANDOUT_MASTER_MODE)
            eEMode = EditMode::MasterPage;
        // Ensure we have the expected view shell
        if (!(xView.is() && xView->getResourceId()->getResourceURL() == sRequestedView))

        {
            const auto xId = CreateResourceId(sRequestedView, msCenterPaneURL);
            mxConfigurationController->requestResourceActivation(
                xId,
                ResourceActivationMode_REPLACE);
            RunOnResourceActivation(xId, std::bind(&asyncUpdateEditMode, this, eEMode));
        }
        else
        {
            updateEditMode(xView, eEMode, false);
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
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
    explicit FlagUpdater (bool& rFlag) : mrFlag(rFlag) {}
    void operator() (bool) const {mrFlag = true;}
private:
    bool& mrFlag;
};

void FrameworkHelper::RequestSynchronousUpdate()
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

void FrameworkHelper::WaitForUpdate() const
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
        mxConfigurationController = nullptr;
}

void FrameworkHelper::UpdateConfiguration()
{
    if (!mxConfigurationController.is())
        return;

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
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

OUString FrameworkHelper::ResourceIdToString (const Reference<XResourceId>& rxResourceId)
{
    OUStringBuffer sString;
    if (rxResourceId.is())
    {
        sString.append(rxResourceId->getResourceURL());
        if (rxResourceId->hasAnchor())
        {
            Sequence<OUString> aAnchorURLs (rxResourceId->getAnchorURLs());
            for (sal_Int32 nIndex=0; nIndex < aAnchorURLs.getLength(); ++nIndex)
            {
                sString.append(" | ");
                sString.append(aAnchorURLs[nIndex]);
            }
        }
    }
    return sString.makeStringAndClear();
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

//----- FrameworkHelper::DisposeListener --------------------------------------

FrameworkHelper::DisposeListener::DisposeListener (
    const ::std::shared_ptr<FrameworkHelper>& rpHelper)
    : FrameworkHelperDisposeListenerInterfaceBase(maMutex),
      mpHelper(rpHelper)
{
    Reference<XComponent> xComponent (mpHelper->mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(this);
}

void SAL_CALL FrameworkHelper::DisposeListener::disposing()
{
    Reference<XComponent> xComponent (mpHelper->mxConfigurationController, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(this);

    mpHelper.reset();
}

void SAL_CALL FrameworkHelper::DisposeListener::disposing (const lang::EventObject& rEventObject)
{
    if (mpHelper != nullptr)
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
                mxConfigurationController = nullptr;
                maCallback(false);
            }
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

void CallbackCaller::disposing()
{
    try
    {
        if (mxConfigurationController.is())
        {
            Reference<XConfigurationController> xCC (mxConfigurationController);
            mxConfigurationController = nullptr;
            xCC->removeConfigurationChangeListener(this);
        }
    }
    catch (RuntimeException&)
    {
        DBG_UNHANDLED_EXCEPTION("sd");
    }
}

void SAL_CALL CallbackCaller::disposing (const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxConfigurationController)
    {
        mxConfigurationController = nullptr;
        maCallback(false);
    }
}

void SAL_CALL CallbackCaller::notifyConfigurationChange (
    const ConfigurationChangeEvent& rEvent)
{
    if (!(rEvent.Type == msEventType && maFilter(rEvent)))
        return;

    maCallback(true);
    if (mxConfigurationController.is())
    {
        // Reset the reference to the configuration controller so that
        // dispose() will not try to remove the listener a second time.
        Reference<XConfigurationController> xCC (mxConfigurationController);
        mxConfigurationController = nullptr;

        // Removing this object from the controller may very likely lead
        // to its destruction, so no calls after that.
        xCC->removeConfigurationChangeListener(this);
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

LifetimeController::~LifetimeController()
{
    OSL_ASSERT(!mbListeningToController && !mbListeningToViewShellBase);
}

void LifetimeController::disposing()
{
}

void SAL_CALL LifetimeController::disposing (const lang::EventObject&)
{
    mbListeningToController = false;
    Update();
}

void LifetimeController::Notify (SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        mbListeningToViewShellBase = false;
        Update();
        release();
    }
}

void LifetimeController::Update()
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
