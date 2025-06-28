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
#include <framework/ConfigurationChangeEvent.hxx>
#include <ResourceId.hxx>
#include <framework/ViewShellWrapper.hxx>
#include <ViewShellBase.hxx>
#include <DrawViewShell.hxx>
#include <ViewShellHint.hxx>
#include <DrawController.hxx>
#include <app.hrc>
#include <com/sun/star/frame/XController.hpp>
#include <comphelper/servicehelper.hxx>
#include <comphelper/compbase.hxx>
#include <svl/lstner.hxx>
#include <rtl/ustrbuf.hxx>

#include <sfx2/request.hxx>

#include <utility>
#include <vcl/svapp.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <memory>
#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace {

//----- CallbackCaller --------------------------------------------------------

/** A CallbackCaller registers as listener at the ConfigurationController
    object and waits for the notification of one type of event.  When that
    event is received, or when the CallbackCaller detects at its
    construction that the event will not be sent in the near future, the
    actual callback object is called and the CallbackCaller destroys itself.
*/
class CallbackCaller : public sd::framework::ConfigurationChangeListener
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
            ConfigurationController at which to register.
        @param rsEventType
            The event type which the callback is waiting for.
        @param pCallback
            The callback object which is to be notified.  The caller will
            typically release his reference to the caller so that when the
            CallbackCaller dies (after having called the callback) the
            callback is destroyed.
    */
    CallbackCaller (
        const ::sd::ViewShellBase& rBase,
        sd::framework::ConfigurationChangeEventType sEventType,
        ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter aFilter,
        ::sd::framework::FrameworkHelper::Callback aCallback);

    virtual void disposing(std::unique_lock<std::mutex>&) override;
    // XEventListener
    virtual void SAL_CALL disposing (const lang::EventObject& rEvent) override;
    // ConfigurationChangeListener
    virtual void notifyConfigurationChange (const sd::framework::ConfigurationChangeEvent& rEvent) override;

private:
    sd::framework::ConfigurationChangeEventType mnEventType;
    rtl::Reference<::sd::framework::ConfigurationController> mxConfigurationController;
    ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter maFilter;
    ::sd::framework::FrameworkHelper::Callback maCallback;
};

//----- LifetimeController ----------------------------------------------------

typedef comphelper::WeakComponentImplHelper <
    css::lang::XEventListener
    > LifetimeControllerInterfaceBase;

/** This class helps controlling the lifetime of the
    FrameworkHelper. Register at a ViewShellBase object and an XController
    object and call Dispose() at the associated FrameworkHelper object when
    one of them and Release() when both of them are destroyed.
*/
class LifetimeController
    : public LifetimeControllerInterfaceBase,
      public SfxListener
{
public:
    explicit LifetimeController (::sd::ViewShellBase& rBase);
    virtual ~LifetimeController() override;

    /** XEventListener.  This method is called when the frame::XController
        is being destroyed.
    */
    using WeakComponentImplHelperBase::disposing;
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

namespace sd::framework {

namespace {

    class FrameworkHelperAllPassFilter
    {
    public:
        bool operator() (const sd::framework::ConfigurationChangeEvent&) { return true; }
    };

    class FrameworkHelperResourceIdFilter
    {
    public:
        explicit FrameworkHelperResourceIdFilter (
            const rtl::Reference<sd::framework::ResourceId>& rxResourceId);
        bool operator() (const sd::framework::ConfigurationChangeEvent& rEvent)
        { return mxResourceId.is() && rEvent.ResourceId.is()
                && mxResourceId->compareTo(rEvent.ResourceId) == 0; }
    private:
        rtl::Reference<sd::framework::ResourceId> mxResourceId;
    };

} // end of anonymous namespace

// Pane URLS.

const OUString FrameworkHelper::msCenterPaneURL( msPaneURLPrefix + "CenterPane");
const OUString FrameworkHelper::msFullScreenPaneURL( msPaneURLPrefix + "FullScreenPane");
const OUString FrameworkHelper::msLeftImpressPaneURL( msPaneURLPrefix + "LeftImpressPane");
const OUString FrameworkHelper::msBottomImpressPaneURL( msPaneURLPrefix + "BottomImpressPane");
const OUString FrameworkHelper::msLeftDrawPaneURL( msPaneURLPrefix + "LeftDrawPane");

// View URLs.

const OUString FrameworkHelper::msImpressViewURL( msViewURLPrefix + "ImpressView");
const OUString FrameworkHelper::msDrawViewURL( msViewURLPrefix + "GraphicView");
const OUString FrameworkHelper::msOutlineViewURL( msViewURLPrefix + "OutlineView");
const OUString FrameworkHelper::msNotesViewURL( msViewURLPrefix + "NotesView");
const OUString FrameworkHelper::msHandoutViewURL( msViewURLPrefix + "HandoutView");
const OUString FrameworkHelper::msSlideSorterURL( msViewURLPrefix + "SlideSorter");
const OUString FrameworkHelper::msPresentationViewURL( msViewURLPrefix + "PresentationView");
const OUString FrameworkHelper::msSidebarViewURL( msViewURLPrefix + "SidebarView");
const OUString FrameworkHelper::msNotesPanelViewURL( msViewURLPrefix + "NotesPanelView");

// Tool bar URLs.

const OUString FrameworkHelper::msViewTabBarURL( msToolBarURLPrefix + "ViewTabBar");

//----- helper ----------------------------------------------------------------
namespace
{
    ::std::shared_ptr< ViewShell > lcl_getViewShell( const rtl::Reference< AbstractResource >& i_rViewShellWrapper )
    {
        ::std::shared_ptr< ViewShell > pViewShell;
        try
        {
            if (auto pWrapper = dynamic_cast<ViewShellWrapper*>(i_rViewShellWrapper.get()))
                pViewShell = pWrapper->GetViewShell();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("sd");
        }
        return pViewShell;
    }
    rtl::Reference< AbstractResource > lcl_getFirstViewInPane( const rtl::Reference< ConfigurationController >& i_rConfigController,
        const rtl::Reference< ResourceId >& i_rPaneId )
    {
        try
        {
            rtl::Reference< sd::framework::Configuration > xConfiguration( i_rConfigController->getRequestedConfiguration() );
            std::vector< rtl::Reference< ResourceId > > aViewIds( xConfiguration->getResources(
                i_rPaneId, FrameworkHelper::msViewURLPrefix, AnchorBindingMode_DIRECT ) );
            if ( !aViewIds.empty() )
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

//----- Framework::DisposeListener ---------------------------------------------

namespace {
    typedef comphelper::WeakComponentImplHelper <
        css::lang::XEventListener
        > FrameworkHelperDisposeListenerInterfaceBase;
}

class FrameworkHelper::DisposeListener
    : public FrameworkHelperDisposeListenerInterfaceBase
{
public:
    explicit DisposeListener (::std::shared_ptr<FrameworkHelper> pHelper);

    virtual void disposing(std::unique_lock<std::mutex>&) override;

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

FrameworkHelper::ViewURLMap FrameworkHelper::maViewURLMap;

FrameworkHelper::InstanceMap FrameworkHelper::maInstanceMap;
std::mutex FrameworkHelper::maInstanceMapMutex;

::std::shared_ptr<FrameworkHelper> FrameworkHelper::Instance (ViewShellBase& rBase)
{
    std::unique_lock aGuard(maInstanceMapMutex);

    InstanceMap::const_iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper != maInstanceMap.end())
        return iHelper->second;

    ::std::shared_ptr<FrameworkHelper> pHelper(
        new FrameworkHelper(rBase),
        FrameworkHelper::Deleter());
    pHelper->Initialize();
    maInstanceMap[&rBase] = pHelper;

    return pHelper;
}

void FrameworkHelper::DisposeInstance (const ViewShellBase& rBase)
{
    InstanceMap::iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper != maInstanceMap.end())
    {
        iHelper->second->Dispose();
    }
}

void FrameworkHelper::ReleaseInstance (const ViewShellBase& rBase)
{
    InstanceMap::iterator iHelper (maInstanceMap.find(&rBase));
    if (iHelper != maInstanceMap.end())
        maInstanceMap.erase(iHelper);
}

FrameworkHelper::FrameworkHelper (ViewShellBase& rBase)
    : mrBase(rBase)
{
    DrawController* pDrawController = rBase.GetDrawController();
    if (pDrawController)
    {
        mxConfigurationController = pDrawController->getConfigurationController();
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

bool FrameworkHelper::IsValid() const
{
    return mxConfigurationController.is();
}

::std::shared_ptr<ViewShell> FrameworkHelper::GetViewShell (const OUString& rsPaneURL)
{
    if ( !mxConfigurationController.is() )
        return ::std::shared_ptr<ViewShell>();

    rtl::Reference<ResourceId> xPaneId( CreateResourceId( rsPaneURL ) );
    return lcl_getViewShell( lcl_getFirstViewInPane( mxConfigurationController, xPaneId ) );
}

::std::shared_ptr<ViewShell> FrameworkHelper::GetViewShell (const rtl::Reference<AbstractView>& rxView)
{
    return lcl_getViewShell( rxView );
}

rtl::Reference<AbstractView> FrameworkHelper::GetView (const rtl::Reference<ResourceId>& rxPaneOrViewId)
{
    rtl::Reference<AbstractView> xView;

    if ( ! rxPaneOrViewId.is() || ! mxConfigurationController.is())
        return nullptr;

    try
    {
        if (rxPaneOrViewId->getResourceURL().match(msViewURLPrefix))
        {
            xView = dynamic_cast<AbstractView*>(mxConfigurationController->getResource( rxPaneOrViewId ).get());
        }
        else
        {
            xView = dynamic_cast<AbstractView*>(lcl_getFirstViewInPane( mxConfigurationController, rxPaneOrViewId ).get());
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

rtl::Reference<ResourceId> FrameworkHelper::RequestView (
    const OUString& rsResourceURL,
    const OUString& rsAnchorURL)
{
    rtl::Reference<ResourceId> xViewId;

    try
    {
        if (mxConfigurationController.is())
        {
            mxConfigurationController->requestResourceActivation(
                CreateResourceId(rsAnchorURL),
                ResourceActivationMode::ADD);
            xViewId = CreateResourceId(rsResourceURL, rsAnchorURL);
            mxConfigurationController->requestResourceActivation(
                xViewId,
                ResourceActivationMode::REPLACE);
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
    if (maViewURLMap.empty())
    {
        maViewURLMap[msImpressViewURL] = ViewShell::ST_IMPRESS;
        maViewURLMap[msDrawViewURL] = ViewShell::ST_DRAW;
        maViewURLMap[msOutlineViewURL] = ViewShell::ST_OUTLINE;
        maViewURLMap[msNotesViewURL] = ViewShell::ST_NOTES;
        maViewURLMap[msHandoutViewURL] = ViewShell::ST_HANDOUT;
        maViewURLMap[msSlideSorterURL] = ViewShell::ST_SLIDE_SORTER;
        maViewURLMap[msPresentationViewURL] = ViewShell::ST_PRESENTATION;
        maViewURLMap[msSidebarViewURL] = ViewShell::ST_SIDEBAR;
        maViewURLMap[msNotesPanelViewURL] = ViewShell::ST_NOTESPANEL;
    }
    ViewURLMap::const_iterator iView (maViewURLMap.find(rsViewURL));
    if (iView != maViewURLMap.end())
        return iView->second;
    else
        return ViewShell::ST_NONE;
}

const OUString & FrameworkHelper::GetViewURL (ViewShell::ShellType eType)
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
        case ViewShell::ST_NOTESPANEL: return msNotesPanelViewURL;
        default:
            return EMPTY_OUSTRING;
    }
}

namespace
{

void updateEditMode(const rtl::Reference<AbstractView> &xView, const EditMode eEMode, bool updateFrameView)
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
    rtl::Reference<ResourceId> xPaneId (
        FrameworkHelper::CreateResourceId(framework::FrameworkHelper::msCenterPaneURL));
    rtl::Reference<AbstractView> xView (pHelper->GetView(xPaneId));
    updateEditMode(xView, eEMode, true);
}

}

void FrameworkHelper::HandleModeChangeSlot (
    sal_uInt16 nSlotId,
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
            const SfxBoolItem* pIsActive = rRequest.GetArg<SfxBoolItem>(nSlotId);
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

        rtl::Reference<ResourceId> xPaneId (
            CreateResourceId(framework::FrameworkHelper::msCenterPaneURL));
        rtl::Reference<AbstractView> xView (GetView(xPaneId));

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
                ResourceActivationMode::REPLACE);
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
    ConfigurationChangeEventType rsEventType,
    const Callback& rCallback)
{
    RunOnEvent(
        rsEventType,
        FrameworkHelperAllPassFilter(),
        rCallback);
}

void FrameworkHelper::RunOnResourceActivation(
    const rtl::Reference<sd::framework::ResourceId>& rxResourceId,
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
            ConfigurationChangeEventType::ResourceActivation,
            FrameworkHelperResourceIdFilter(rxResourceId),
            rCallback);
    }
}

namespace {

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

}

void FrameworkHelper::RequestSynchronousUpdate()
{
    if (mxConfigurationController)
        mxConfigurationController->RequestSynchronousUpdate();
}

void FrameworkHelper::WaitForEvent (ConfigurationChangeEventType rsEventType) const
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
    WaitForEvent(ConfigurationChangeEventType::ConfigurationUpdateEnd);
}

void FrameworkHelper::RunOnEvent(
    ConfigurationChangeEventType rsEventType,
    const ConfigurationChangeEventFilter& rFilter,
    const Callback& rCallback) const
{
    new CallbackCaller(mrBase,rsEventType,rFilter,rCallback);
}

void FrameworkHelper::disposing (const lang::EventObject& rEventObject)
{
    if (rEventObject.Source == cppu::getXWeak(mxConfigurationController.get()))
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

OUString FrameworkHelper::ResourceIdToString (const rtl::Reference<ResourceId>& rxResourceId)
{
    OUStringBuffer sString;
    if (rxResourceId.is())
    {
        sString.append(rxResourceId->getResourceURL());
        if (rxResourceId->hasAnchor())
        {
            std::vector<OUString> aAnchorURLs (rxResourceId->getAnchorURLs());
            for (const auto& rAnchorURL : aAnchorURLs)
            {
                sString.append(" | " + rAnchorURL);
            }
        }
    }
    return sString.makeStringAndClear();
}

rtl::Reference<ResourceId> FrameworkHelper::CreateResourceId (const OUString& rsResourceURL)
{
    return new ::sd::framework::ResourceId(rsResourceURL);
}

rtl::Reference<ResourceId> FrameworkHelper::CreateResourceId (
    const OUString& rsResourceURL,
    const OUString& rsAnchorURL)
{
    return new ::sd::framework::ResourceId(rsResourceURL, rsAnchorURL);
}

rtl::Reference<ResourceId> FrameworkHelper::CreateResourceId (
    const OUString& rsResourceURL,
    const rtl::Reference<ResourceId>& rxAnchorId)
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
    ::std::shared_ptr<FrameworkHelper> pHelper)
    : mpHelper(std::move(pHelper))
{
    if (mpHelper->mxConfigurationController.is())
        mpHelper->mxConfigurationController->addEventListener(this);
}

void FrameworkHelper::DisposeListener::disposing(std::unique_lock<std::mutex>&)
{
    if (mpHelper->mxConfigurationController.is())
        mpHelper->mxConfigurationController->removeEventListener(this);

    mpHelper.reset();
}

void SAL_CALL FrameworkHelper::DisposeListener::disposing (const lang::EventObject& rEventObject)
{
    if (mpHelper != nullptr)
        mpHelper->disposing(rEventObject);
}

//===== FrameworkHelperResourceIdFilter =======================================

FrameworkHelperResourceIdFilter::FrameworkHelperResourceIdFilter (
    const rtl::Reference<ResourceId>& rxResourceId)
    : mxResourceId(rxResourceId)
{
}

} // end of namespace sd::framework

namespace {

//===== CallbackCaller ========================================================

CallbackCaller::CallbackCaller (
    const ::sd::ViewShellBase& rBase,
    sd::framework::ConfigurationChangeEventType rsEventType,
    ::sd::framework::FrameworkHelper::ConfigurationChangeEventFilter aFilter,
    ::sd::framework::FrameworkHelper::Callback aCallback)
    : mnEventType(rsEventType),
      maFilter(std::move(aFilter)),
      maCallback(std::move(aCallback))
{
    try
    {
        sd::DrawController* pDrawController = rBase.GetDrawController();
        if (!pDrawController)
            return;
        mxConfigurationController = pDrawController->getConfigurationController();
        if (mxConfigurationController.is())
        {
            if (mxConfigurationController->hasPendingRequests())
                mxConfigurationController->addConfigurationChangeListener(this,mnEventType,Any());
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

void CallbackCaller::disposing(std::unique_lock<std::mutex>&)
{
    try
    {
        if (mxConfigurationController.is())
        {
            rtl::Reference<sd::framework::ConfigurationController> xCC (mxConfigurationController);
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
    if (rEvent.Source == cppu::getXWeak(mxConfigurationController.get()))
    {
        mxConfigurationController = nullptr;
        maCallback(false);
    }
}

void CallbackCaller::notifyConfigurationChange (
    const sd::framework::ConfigurationChangeEvent& rEvent)
{
    if (!(rEvent.Type == mnEventType && maFilter(rEvent)))
        return;

    maCallback(true);
    if (mxConfigurationController.is())
    {
        // Reset the reference to the configuration controller so that
        // dispose() will not try to remove the listener a second time.
        rtl::Reference<sd::framework::ConfigurationController> xCC (mxConfigurationController);
        mxConfigurationController = nullptr;

        // Removing this object from the controller may very likely lead
        // to its destruction, so no calls after that.
        xCC->removeConfigurationChangeListener(this);
    }
}

//----- LifetimeController -------------------------------------------------

LifetimeController::LifetimeController (::sd::ViewShellBase& rBase)
    : mrBase(rBase),
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

    Reference<XComponent> xComponent = rBase.GetController();
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
