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
#include "PresenterController.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <algorithm>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext::presenter {

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
            rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
        virtual bool IsEnabled() const override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class GotoNextSlideCommand : public Command
    {
    public:
        explicit GotoNextSlideCommand (
            rtl::Reference<PresenterController> xPresenterController);
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
            rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
        virtual bool IsEnabled() const override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SwitchMonitorCommand : public Command
    {
    public:
        explicit SwitchMonitorCommand (
            rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class PauseResumeCommand : public Command
    {
    public:
        explicit PauseResumeCommand(rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    /// This command restarts the presentation timer.
    class RestartTimerCommand : public Command
    {
    public:
        explicit RestartTimerCommand(rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetNotesViewCommand : public Command
    {
    public:
        SetNotesViewCommand (
            const bool bOn,
            rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        bool mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetSlideSorterCommand : public Command
    {
    public:
        SetSlideSorterCommand (
            const bool bOn,
            rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        bool mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetHelpViewCommand : public Command
    {
    public:
        SetHelpViewCommand (
            const bool bOn,
            rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
        virtual Any GetState() const override;
    private:
        bool mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class NotesFontSizeCommand : public Command
    {
    public:
        NotesFontSizeCommand(
            rtl::Reference<PresenterController> xPresenterController,
            const sal_Int32 nSizeChange);
        virtual void Execute() override;
        virtual Any GetState() const override;
    protected:
        ::rtl::Reference<PresenterNotesView> GetNotesView() const;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
        const sal_Int32 mnSizeChange;
    };

    class ExitPresenterCommand : public Command
    {
    public:
        explicit ExitPresenterCommand(rtl::Reference<PresenterController> xPresenterController);
        virtual void Execute() override;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
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
    /** Create a new Dispatch object.  When the given command name
        (rsURLPath) is not known then an empty reference is returned.
    */
    static Reference<frame::XDispatch> Create (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    void SAL_CALL disposing() override;
    static Command* CreateCommand (
        std::u16string_view rsURLPath,
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
    return u"org.libreoffice.comp.PresenterScreenProtocolHandler"_ustr;
}

sal_Bool PresenterProtocolHandler::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString>
PresenterProtocolHandler::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ProtocolHandler"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sd_PresenterProtocolHandler_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PresenterProtocolHandler());
}

//----- XDispatchProvider -----------------------------------------------------

Reference<frame::XDispatch> SAL_CALL PresenterProtocolHandler::queryDispatch (
    const css::util::URL& rURL,
    const OUString&,
    sal_Int32)
{
    ThrowIfDisposed();

    Reference<frame::XDispatch> xDispatch;

    // tdf#154546 skip dispatch when presenter controller is not set
    // mpPresenterController is sometimes unset and this will cause a
    // crash when pressing the presenter console's Exchange button.
    if (rURL.Protocol == "vnd.org.libreoffice.presenterscreen:" && mpPresenterController.is())
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
            u"PresenterProtocolHandler object has already been disposed"_ustr,
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
        return pDispatch;
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
      mbIsListeningToWindowManager(false)
{
    if (mpCommand != nullptr)
    {
        mpPresenterController->GetWindowManager()->AddLayoutListener(this);
        mbIsListeningToWindowManager = true;
    }
}

Command* PresenterProtocolHandler::Dispatch::CreateCommand (
    std::u16string_view rsURLPath,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    if (rsURLPath.size() <= 5)
        return nullptr;

    if (rsURLPath == u"CloseNotes")
        return new SetNotesViewCommand(false, rpPresenterController);
    if (rsURLPath == u"CloseSlideSorter")
        return new SetSlideSorterCommand(false, rpPresenterController);
    if (rsURLPath == u"CloseHelp")
        return new SetHelpViewCommand(false, rpPresenterController);
    if (rsURLPath == u"GrowNotesFont")
        return new NotesFontSizeCommand(rpPresenterController, +1);
    if (rsURLPath == u"NextEffect")
        return new GotoNextEffectCommand(rpPresenterController);
    if (rsURLPath == u"NextSlide")
        return new GotoNextSlideCommand(rpPresenterController);
    if (rsURLPath == u"PrevSlide")
        return new GotoPreviousSlideCommand(rpPresenterController);
    if (rsURLPath == u"SwitchMonitor")
        return new SwitchMonitorCommand(rpPresenterController);
    if (rsURLPath == u"PauseResumeTimer")
        return new PauseResumeCommand(rpPresenterController);
    if (rsURLPath == u"RestartTimer")
        return new RestartTimerCommand(rpPresenterController);
    if (rsURLPath == u"ShowNotes")
        return new SetNotesViewCommand(true, rpPresenterController);
    if (rsURLPath == u"ShowSlideSorter")
        return new SetSlideSorterCommand(true, rpPresenterController);
    if (rsURLPath == u"ShowHelp")
        return new SetHelpViewCommand(true, rpPresenterController);
    if (rsURLPath == u"ShrinkNotesFont")
        return new NotesFontSizeCommand(rpPresenterController, -1);
    if (rsURLPath == u"ExitPresenter")
        return new ExitPresenterCommand(rpPresenterController);

    return nullptr;
}

PresenterProtocolHandler::Dispatch::~Dispatch()
{
}

void PresenterProtocolHandler::Dispatch::disposing()
{
    if (mbIsListeningToWindowManager)
    {
        if (mpPresenterController)
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
            u"PresenterProtocolHandler::Dispatch object has already been disposed"_ustr,
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
    rtl::Reference<PresenterController> xPresenterController)
    : mpPresenterController(std::move(xPresenterController))
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
    rtl::Reference<PresenterController> xPresenterController)
    : mpPresenterController(std::move(xPresenterController))
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
    rtl::Reference<PresenterController> xPresenterController)
    : mpPresenterController(std::move(xPresenterController))
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
    rtl::Reference<PresenterController> xPresenterController)
    : mpPresenterController(std::move(xPresenterController))
{
}

void SwitchMonitorCommand::Execute()
{
    mpPresenterController->SwitchMonitors();
}

//===== PauseResumeCommand ==============================================

PauseResumeCommand::PauseResumeCommand (rtl::Reference<PresenterController> xPresenterController)
: mpPresenterController(std::move(xPresenterController))
{
}

void PauseResumeCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    IPresentationTime* pPresentationTime = mpPresenterController->GetPresentationTime();
    if (!pPresentationTime)
        return;

    if(pPresentationTime->isPaused())
    {
        pPresentationTime->setPauseStatus(false);
        pWindowManager->SetPauseState(false);
    }
    else
    {
        pPresentationTime->setPauseStatus(true);
        pWindowManager->SetPauseState(true);
    }
}

Any PauseResumeCommand::GetState() const
{
    if ( ! mpPresenterController.is())
        return Any(false);

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return Any(false);

    if (IPresentationTime* pPresentationTime = mpPresenterController->GetPresentationTime())
    {
        return Any(pPresentationTime->isPaused());
    }
    else
        return Any(false);
}

RestartTimerCommand::RestartTimerCommand (rtl::Reference<PresenterController> xPresenterController)
: mpPresenterController(std::move(xPresenterController))
{
}

void RestartTimerCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    if (IPresentationTime* pPresentationTime = mpPresenterController->GetPresentationTime())
    {
        //Resets the pause status and restarts the timer
        pPresentationTime->setPauseStatus(false);
        pWindowManager->SetPauseState(false);
        pPresentationTime->restart();
    }
}

//===== SetNotesViewCommand ===================================================

SetNotesViewCommand::SetNotesViewCommand (
    const bool bOn,
    rtl::Reference<PresenterController> xPresenterController)
    : mbOn(bOn),
      mpPresenterController(std::move(xPresenterController))
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
    rtl::Reference<PresenterController> xPresenterController)
    : mbOn(bOn),
      mpPresenterController(std::move(xPresenterController))
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
    rtl::Reference<PresenterController> xPresenterController)
    : mbOn(bOn),
      mpPresenterController(std::move(xPresenterController))
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
    rtl::Reference<PresenterController> xPresenterController,
    const sal_Int32 nSizeChange)
    : mpPresenterController(std::move(xPresenterController)),
      mnSizeChange(nSizeChange)
{
}

::rtl::Reference<PresenterNotesView> NotesFontSizeCommand::GetNotesView() const
{
    if (!mpPresenterController)
        return nullptr;

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPresenterController->GetPaneContainer()->FindViewURL(
            PresenterViewFactory::msNotesViewURL));
    if (!pDescriptor)
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

//===== ExitPresenterCommand ==================================================

ExitPresenterCommand::ExitPresenterCommand (rtl::Reference<PresenterController> xPresenterController)
: mpPresenterController(std::move(xPresenterController))
{
}

void ExitPresenterCommand::Execute()
{
    if ( ! mpPresenterController.is())
        return;

    mpPresenterController->ExitPresenter();
}

} // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
