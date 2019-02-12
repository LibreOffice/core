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

#include <memory>
#include "PresenterProtocolHandler.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterController.hxx"
#include "PresenterHelper.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/drawing/SlideSorter.hpp>
#include <com/sun/star/drawing/framework/Configuration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

namespace {
    class Command
    {
    public:
        virtual ~Command() {}
        virtual void Execute() = 0;
        virtual bool IsEnabled() const { return true; }
        virtual Any GetState() const { return Any(false); }
    };

    class GotoPreviousSlideCommand : public Command
    {
    public:
        explicit GotoPreviousSlideCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
        virtual bool IsEnabled() const override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class GotoNextSlideCommand : public Command
    {
    public:
        explicit GotoNextSlideCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
        // The next slide command is always enabled, even when the current slide
        // is the last slide:  from the last slide it goes to the pause slide,
        // and from there it ends the slide show.
        virtual bool IsEnabled() const override { return true; }
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class GotoNextEffectCommand : public Command
    {
    public:
        explicit GotoNextEffectCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
        virtual bool IsEnabled() const override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SwitchMonitorCommand : public Command
    {
    public:
        explicit SwitchMonitorCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    /// This command restarts the presentation timer.
    class RestartTimerCommand : public Command
    {
    public:
        explicit RestartTimerCommand(const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetNotesViewCommand : public Command
    {
    public:
        SetNotesViewCommand (
            const bool bOn,
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        bool const mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetSlideSorterCommand : public Command
    {
    public:
        SetSlideSorterCommand (
            const bool bOn,
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        bool const mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetHelpViewCommand : public Command
    {
    public:
        SetHelpViewCommand (
            const bool bOn,
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        bool const mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class NotesFontSizeCommand : public Command
    {
    public:
        NotesFontSizeCommand(
            const rtl::Reference<PresenterController>& rpPresenterController,
            const sal_Int32 nSizeChange);
        virtual void Execute() override;
        virtual Any GetState() const override;
    protected:
        ::rtl::Reference<PresenterNotesView> GetNotesView() const;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
        const sal_Int32 mnSizeChange;
    };

} // end of anonymous namespace

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::frame::XDispatch,
        css::document::XEventListener
        > PresenterDispatchInterfaceBase;
}

class PresenterProtocolHandler::Dispatch
    : protected ::cppu::BaseMutex,
      public PresenterDispatchInterfaceBase
{
public:
    typedef void (PresenterProtocolHandler::Dispatch::* Action)();

    /** Create a new Dispatch object.  When the given command name
        (rsURLPath) is not known then an empty reference is returned.
    */
    static Reference<frame::XDispatch> Create (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    void SAL_CALL disposing() override;
    static Command* CreateCommand (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    // XDispatch
    virtual void SAL_CALL dispatch(
        const css::util::URL& aURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments) override;

    virtual void SAL_CALL addStatusListener(
        const css::uno::Reference<css::frame::XStatusListener>& rxListener,
        const css::util::URL& rURL) override;

    virtual void SAL_CALL removeStatusListener (
        const css::uno::Reference<css::frame::XStatusListener>& rxListener,
        const css::util::URL& rURL) override;

    // document::XEventListener

    virtual void SAL_CALL notifyEvent (const css::document::EventObject& rEvent) override;

    // lang::XEventListener

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

private:
    OUString msURLPath;
    std::unique_ptr<Command> mpCommand;
    ::rtl::Reference<PresenterController> mpPresenterController;
    typedef ::std::vector<Reference<frame::XStatusListener> > StatusListenerContainer;
    StatusListenerContainer maStatusListenerContainer;
    bool mbIsListeningToWindowManager;

    Dispatch (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~Dispatch() override;
};

//----- Service ---------------------------------------------------------------

OUString PresenterProtocolHandler::getImplementationName_static()
{
    return OUString("org.libreoffice.comp.PresenterScreenProtocolHandler");
}

Sequence<OUString> PresenterProtocolHandler::getSupportedServiceNames_static()
{
    static const OUString sServiceName("com.sun.star.frame.ProtocolHandler");
    return Sequence<OUString>(&sServiceName, 1);
}

Reference<XInterface> PresenterProtocolHandler::Create (
    SAL_UNUSED_PARAMETER const Reference<uno::XComponentContext>&)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterProtocolHandler));
}

//===== PresenterProtocolHandler =========================================================

PresenterProtocolHandler::PresenterProtocolHandler ()
    : PresenterProtocolHandlerInterfaceBase(m_aMutex)
{
}

PresenterProtocolHandler::~PresenterProtocolHandler()
{
}

void SAL_CALL PresenterProtocolHandler::disposing()
{
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterProtocolHandler::initialize (const Sequence<Any>& aArguments)
{
    ThrowIfDisposed();
    if (aArguments.getLength() <= 0)
        return;

    try
    {
        Reference<frame::XFrame> xFrame;
        if (aArguments[0] >>= xFrame)
        {
            mpPresenterController = PresenterController::Instance(xFrame);
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}

OUString PresenterProtocolHandler::getImplementationName()
{
    return getImplementationName_static();
}

sal_Bool PresenterProtocolHandler::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
PresenterProtocolHandler::getSupportedServiceNames()
{
    return getSupportedServiceNames_static();
}

//----- XDispatchProvider -----------------------------------------------------

Reference<frame::XDispatch> SAL_CALL PresenterProtocolHandler::queryDispatch (
    const css::util::URL& rURL,
    const OUString&,
    sal_Int32)
{
    ThrowIfDisposed();

    Reference<frame::XDispatch> xDispatch;

    if (rURL.Protocol == "vnd.org.libreoffice.presenterscreen:")
    {
        xDispatch.set(Dispatch::Create(rURL.Path, mpPresenterController));
    }

    return xDispatch;
}

Sequence<Reference<frame::XDispatch> > SAL_CALL PresenterProtocolHandler::queryDispatches(
    const Sequence<frame::DispatchDescriptor>&)
{
    ThrowIfDisposed();
    return Sequence<Reference<frame::XDispatch> >();
}


void PresenterProtocolHandler::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterProtocolHandler object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

//===== PresenterProtocolHandler::Dispatch ====================================

Reference<frame::XDispatch> PresenterProtocolHandler::Dispatch::Create (
    const OUString& rsURLPath,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    ::rtl::Reference<Dispatch> pDispatch (new Dispatch (rsURLPath, rpPresenterController));
    if (pDispatch->mpCommand != nullptr)
        return Reference<frame::XDispatch>(pDispatch.get());
    else
        return nullptr;
}

PresenterProtocolHandler::Dispatch::Dispatch (
    const OUString& rsURLPath,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterDispatchInterfaceBase(m_aMutex),
      msURLPath(rsURLPath),
      mpCommand(CreateCommand(rsURLPath, rpPresenterController)),
      mpPresenterController(rpPresenterController),
      maStatusListenerContainer(),
      mbIsListeningToWindowManager(false)
{
    if (mpCommand != nullptr)
    {
        mpPresenterController->GetWindowManager()->AddLayoutListener(this);
        mbIsListeningToWindowManager = true;
    }
}

Command* PresenterProtocolHandler::Dispatch::CreateCommand (
    const OUString& rsURLPath,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    if (rsURLPath.getLength() <= 5)
        return nullptr;

    if (rsURLPath == "CloseNotes")
        return new SetNotesViewCommand(false, rpPresenterController);
    if (rsURLPath == "CloseSlideSorter")
        return new SetSlideSorterCommand(false, rpPresenterController);
    if (rsURLPath == "CloseHelp")
        return new SetHelpViewCommand(false, rpPresenterController);
    if (rsURLPath == "GrowNotesFont")
        return new NotesFontSizeCommand(rpPresenterController, +1);
    if (rsURLPath == "NextEffect")
        return new GotoNextEffectCommand(rpPresenterController);
    if (rsURLPath == "NextSlide")
        return new GotoNextSlideCommand(rpPresenterController);
    if (rsURLPath == "PrevSlide")
        return new GotoPreviousSlideCommand(rpPresenterController);
    if (rsURLPath == "SwitchMonitor")
        return new SwitchMonitorCommand(rpPresenterController);
    if (rsURLPath == "RestartTimer")
        return new RestartTimerCommand(rpPresenterController);
    if (rsURLPath == "ShowNotes")
        return new SetNotesViewCommand(true, rpPresenterController);
    if (rsURLPath == "ShowSlideSorter")
        return new SetSlideSorterCommand(true, rpPresenterController);
    if (rsURLPath == "ShowHelp")
        return new SetHelpViewCommand(true, rpPresenterController);
    if (rsURLPath == "ShrinkNotesFont")
        return new NotesFontSizeCommand(rpPresenterController, -1);

    return nullptr;
}

PresenterProtocolHandler::Dispatch::~Dispatch()
{
}

void PresenterProtocolHandler::Dispatch::disposing()
{
    if (mbIsListeningToWindowManager)
    {
        if (mpPresenterController.get() != nullptr)
            mpPresenterController->GetWindowManager()->RemoveLayoutListener(this);
        mbIsListeningToWindowManager = false;
    }

    msURLPath.clear();
    mpCommand.reset();
}

//----- XDispatch -------------------------------------------------------------

void SAL_CALL PresenterProtocolHandler::Dispatch::dispatch(
    const css::util::URL& rURL,
    const css::uno::Sequence<css::beans::PropertyValue>& /*rArguments*/)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterProtocolHandler::Dispatch object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }

    if (rURL.Protocol != "vnd.org.libreoffice.presenterscreen:"
        || rURL.Path != msURLPath)
    {
        // We can not throw an IllegalArgumentException
        throw RuntimeException();
    }

    if (mpCommand != nullptr)
        mpCommand->Execute();
}

void SAL_CALL PresenterProtocolHandler::Dispatch::addStatusListener(
    const css::uno::Reference<css::frame::XStatusListener>& rxListener,
    const css::util::URL& rURL)
{
    if (rURL.Path != msURLPath)
        throw RuntimeException();

    maStatusListenerContainer.push_back(rxListener);

    frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL = rURL;
    aEvent.IsEnabled = mpCommand->IsEnabled();
    aEvent.Requery = false;
    aEvent.State = mpCommand->GetState();
    rxListener->statusChanged(aEvent);
}

void SAL_CALL PresenterProtocolHandler::Dispatch::removeStatusListener (
    const css::uno::Reference<css::frame::XStatusListener>& rxListener,
    const css::util::URL& rURL)
{
    if (rURL.Path != msURLPath)
        throw RuntimeException();

    StatusListenerContainer::iterator iListener (
        ::std::find(
            maStatusListenerContainer.begin(),
            maStatusListenerContainer.end(),
            rxListener));
    if (iListener != maStatusListenerContainer.end())
        maStatusListenerContainer.erase(iListener);
}

//----- document::XEventListener ----------------------------------------------

void SAL_CALL PresenterProtocolHandler::Dispatch::notifyEvent (
    const css::document::EventObject&)
{
    mpCommand->GetState();
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterProtocolHandler::Dispatch::disposing (const css::lang::EventObject&)
{
    mbIsListeningToWindowManager = false;
}

//===== GotoPreviousSlideCommand ==============================================

GotoPreviousSlideCommand::GotoPreviousSlideCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}

void GotoPreviousSlideCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return;

    mpPresenterController->GetSlideShowController()->gotoPreviousSlide();
}

bool GotoPreviousSlideCommand::IsEnabled() const
{
    if ( ! mpPresenterController.is())
        return false;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return false;

    return mpPresenterController->GetSlideShowController()->getCurrentSlideIndex()>0;
}

//===== GotoNextEffect ========================================================

GotoNextEffectCommand::GotoNextEffectCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}

void GotoNextEffectCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return;

    mpPresenterController->GetSlideShowController()->gotoNextEffect();
}

bool GotoNextEffectCommand::IsEnabled() const
{
    if ( ! mpPresenterController.is())
        return false;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return false;

    return ( mpPresenterController->GetSlideShowController()->getNextSlideIndex() < mpPresenterController->GetSlideShowController()->getSlideCount() );

}

//===== GotoNextSlide =========================================================

GotoNextSlideCommand::GotoNextSlideCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}

void GotoNextSlideCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return;

    mpPresenterController->GetSlideShowController()->gotoNextSlide();
}

//===== SwitchMonitorCommand ==============================================

SwitchMonitorCommand::SwitchMonitorCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}

void SwitchMonitorCommand::Execute()
{
    mpPresenterController->SwitchMonitors();
}

RestartTimerCommand::RestartTimerCommand (const rtl::Reference<PresenterController>& rpPresenterController)
: mpPresenterController(rpPresenterController)
{
}

void RestartTimerCommand::Execute()
{
    if (IPresentationTime* pPresentationTime = mpPresenterController->GetPresentationTime())
        pPresentationTime->restart();
}

//===== SetNotesViewCommand ===================================================

SetNotesViewCommand::SetNotesViewCommand (
    const bool bOn,
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mbOn(bOn),
      mpPresenterController(rpPresenterController)
{
}

void SetNotesViewCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    if (mbOn)
        pWindowManager->SetViewMode(PresenterWindowManager::VM_Notes);
    else
        pWindowManager->SetViewMode(PresenterWindowManager::VM_Standard);
}

Any SetNotesViewCommand::GetState() const
{
    if ( ! mpPresenterController.is())
        return Any(false);

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return Any(false);

    return Any(pWindowManager->GetViewMode() == PresenterWindowManager::VM_Notes);
}

//===== SetSlideSorterCommand =================================================

SetSlideSorterCommand::SetSlideSorterCommand (
    const bool bOn,
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mbOn(bOn),
      mpPresenterController(rpPresenterController)
{
}

void SetSlideSorterCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    pWindowManager->SetSlideSorterState(mbOn);
}

Any SetSlideSorterCommand::GetState() const
{
    if ( ! mpPresenterController.is())
        return Any(false);

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return Any(false);

    return Any(pWindowManager->GetViewMode()==PresenterWindowManager::VM_SlideOverview);
}

//===== SetHelpViewCommand ===================================================

SetHelpViewCommand::SetHelpViewCommand (
    const bool bOn,
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mbOn(bOn),
      mpPresenterController(rpPresenterController)
{
}

void SetHelpViewCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    pWindowManager->SetHelpViewState(mbOn);
}

Any SetHelpViewCommand::GetState() const
{
    if ( ! mpPresenterController.is())
        return Any(false);

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return Any(false);

    return Any(pWindowManager->GetViewMode()==PresenterWindowManager::VM_Help);
}

//===== NotesFontSizeCommand ==================================================

NotesFontSizeCommand::NotesFontSizeCommand(
    const rtl::Reference<PresenterController>& rpPresenterController,
    const sal_Int32 nSizeChange)
    : mpPresenterController(rpPresenterController),
      mnSizeChange(nSizeChange)
{
}

::rtl::Reference<PresenterNotesView> NotesFontSizeCommand::GetNotesView() const
{
    if (mpPresenterController.get() == nullptr)
        return nullptr;

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPresenterController->GetPaneContainer()->FindViewURL(
            PresenterViewFactory::msNotesViewURL));
    if (pDescriptor.get() == nullptr)
        return nullptr;

    return dynamic_cast<PresenterNotesView*>(pDescriptor->mxView.get());
}

void NotesFontSizeCommand::Execute()
{
    ::rtl::Reference<PresenterNotesView> pView (GetNotesView());
    if (pView.is())
        pView->ChangeFontSize(mnSizeChange);
}

Any NotesFontSizeCommand::GetState() const
{
    return Any();
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
