/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterProtocolHandler.cxx,v $
 *
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterProtocolHandler.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterController.hxx"
#include "PresenterHelper.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/drawing/SlideSorter.hpp>
#include <com/sun/star/drawing/framework/Configuration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <cppuhelper/compbase2.hxx>
#include <boost/bind.hpp>
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace sdext { namespace presenter {

namespace {
    const static OUString gsProtocol (A2S("vnd.com.sun.star.comp.PresenterScreen:"));

    class Command
    {
    public:
        virtual void Execute (void) = 0;
        virtual bool IsEnabled (void) const = 0;
        virtual Any GetState (void) const = 0;
    };

    class GotoPreviousSlideCommand : public Command
    {
    public:
        GotoPreviousSlideCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual ~GotoPreviousSlideCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class GotoNextSlideCommand : public Command
    {
    public:
        GotoNextSlideCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual ~GotoNextSlideCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class GotoNextEffectCommand : public Command
    {
    public:
        GotoNextEffectCommand (
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual ~GotoNextEffectCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetNotesViewCommand : public Command
    {
    public:
        SetNotesViewCommand (
            const bool bOn,
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual ~SetNotesViewCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    private:
        bool mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
        bool IsActive (const ::rtl::Reference<PresenterWindowManager>& rpWindowManager) const;
    };

    class SetSlideSorterCommand : public Command
    {
    public:
        SetSlideSorterCommand (
            const bool bOn,
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual ~SetSlideSorterCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    private:
        bool mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class SetHelpViewCommand : public Command
    {
    public:
        SetHelpViewCommand (
            const bool bOn,
            const rtl::Reference<PresenterController>& rpPresenterController);
        virtual ~SetHelpViewCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    private:
        bool mbOn;
        rtl::Reference<PresenterController> mpPresenterController;
    };

    class NotesFontSizeCommand : public Command
    {
    public:
        NotesFontSizeCommand(
            const rtl::Reference<PresenterController>& rpPresenterController,
            const sal_Int32 nSizeChange);
        virtual ~NotesFontSizeCommand (void) {}
        virtual void Execute (void);
        virtual bool IsEnabled (void) const;
        virtual Any GetState (void) const;
    protected:
        ::rtl::Reference<PresenterNotesView> GetNotesView (void) const;
    private:
        rtl::Reference<PresenterController> mpPresenterController;
        const sal_Int32 mnSizeChange;
    };

} // end of anonymous namespace


namespace {
    typedef ::cppu::WeakComponentImplHelper2 <
        css::frame::XDispatch,
        css::document::XEventListener
        > PresenterDispatchInterfaceBase;
}

class PresenterProtocolHandler::Dispatch
    : protected ::cppu::BaseMutex,
      public PresenterDispatchInterfaceBase
{
public:
    typedef void (PresenterProtocolHandler::Dispatch::* Action)(void);

    /** Create a new Dispatch object.  When the given command name
        (rsURLPath) is not known then an empty reference is returned.
    */
    static Reference<frame::XDispatch> Create (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    void SAL_CALL disposing (void);
    static Command* CreateCommand (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);


    // XDispatch
    virtual void SAL_CALL dispatch(
        const css::util::URL& aURL,
        const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL addStatusListener(
        const css::uno::Reference<css::frame::XStatusListener>& rxListener,
        const css::util::URL& rURL)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL removeStatusListener (
        const css::uno::Reference<css::frame::XStatusListener>& rxListener,
        const css::util::URL& rURL)
        throw(css::uno::RuntimeException);


    // document::XEventListener

    virtual void SAL_CALL notifyEvent (const css::document::EventObject& rEvent)
        throw(css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw(css::uno::RuntimeException);

private:
    OUString msURLPath;
    ::boost::scoped_ptr<Command> mpCommand;
    ::rtl::Reference<PresenterController> mpPresenterController;
    typedef ::std::vector<Reference<frame::XStatusListener> > StatusListenerContainer;
    StatusListenerContainer maStatusListenerContainer;
    bool mbIsListeningToWindowManager;

    Dispatch (
        const OUString& rsURLPath,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~Dispatch (void);

    void ThrowIfDisposed (void) const throw (css::lang::DisposedException);
};




//----- Service ---------------------------------------------------------------

OUString PresenterProtocolHandler::getImplementationName_static (void)
{
    return A2S("vnd.sun.star.sdext.presenter.PresenterProtocolHandler");
}




Sequence<OUString> PresenterProtocolHandler::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(A2S("com.sun.star.frame.ProtocolHandler"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterProtocolHandler::Create (
    const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterProtocolHandler(rxContext)));
}




//===== PresenterProtocolHandler =========================================================


PresenterProtocolHandler::PresenterProtocolHandler (const Reference<XComponentContext>& rxContext)
    : PresenterProtocolHandlerInterfaceBase(m_aMutex)
{
    (void)rxContext;
}




PresenterProtocolHandler::~PresenterProtocolHandler (void)
{
}




void SAL_CALL PresenterProtocolHandler::disposing (void)
{
}




//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterProtocolHandler::initialize (const Sequence<Any>& aArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();
    if (aArguments.getLength() > 0)
    {
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
}




//----- XDispatchProvider -----------------------------------------------------

Reference<frame::XDispatch> SAL_CALL PresenterProtocolHandler::queryDispatch (
    const css::util::URL& rURL,
    const rtl::OUString& rsTargetFrameName,
    sal_Int32 nSearchFlags)
    throw(RuntimeException)
{
    (void)rsTargetFrameName;
    (void)nSearchFlags;
    ThrowIfDisposed();

    Reference<frame::XDispatch> xDispatch;

    if (rURL.Protocol == gsProtocol)
    {
        xDispatch.set(Dispatch::Create(rURL.Path, mpPresenterController));
    }

    return xDispatch;
}




Sequence<Reference<frame::XDispatch> > SAL_CALL PresenterProtocolHandler::queryDispatches(
    const Sequence<frame::DispatchDescriptor>& rDescriptors)
    throw(RuntimeException)
{
    (void)rDescriptors;
    ThrowIfDisposed();
    return Sequence<Reference<frame::XDispatch> >();
}




//-----------------------------------------------------------------------------

void PresenterProtocolHandler::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterProtocolHandler object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//===== PresenterProtocolHandler::Dispatch ====================================

Reference<frame::XDispatch> PresenterProtocolHandler::Dispatch::Create (
    const OUString& rsURLPath,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    ::rtl::Reference<Dispatch> pDispatch (new Dispatch (rsURLPath, rpPresenterController));
    if (pDispatch->mpCommand.get() != NULL)
        return Reference<frame::XDispatch>(pDispatch.get());
    else
        return NULL;
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
    if (mpCommand.get() != NULL)
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
        return NULL;
    switch (rsURLPath[0])
    {
        case  sal_Char('C') :
            switch (rsURLPath[5])
            {
                case sal_Char('N'):
                    if (rsURLPath == A2S("CloseNotes"))
                        return new SetNotesViewCommand(false, rpPresenterController);
                    break;
                case sal_Char('S'):
                    if (rsURLPath == A2S("CloseSlideSorter"))
                        return new SetSlideSorterCommand(false, rpPresenterController);
                    break;
                case sal_Char('H'):
                    if (rsURLPath == A2S("CloseHelp"))
                        return new SetHelpViewCommand(false, rpPresenterController);
                    break;
            }
            break;
        case  sal_Char('G') :
            if (rsURLPath == A2S("GrowNotesFont"))
                return new NotesFontSizeCommand(rpPresenterController, +1);
            break;

        case sal_Char('N') :
            switch (rsURLPath[4])
            {
                case sal_Char('E'):
                    if (rsURLPath == A2S("NextEffect"))
                        return new GotoNextEffectCommand(rpPresenterController);
                case sal_Char('S'):
                    if (rsURLPath == A2S("NextSlide"))
                        return new GotoNextSlideCommand(rpPresenterController);
                    break;
            }
            break;

        case sal_Char('P') :
            if (rsURLPath == A2S("PrevSlide"))
                return new GotoPreviousSlideCommand(rpPresenterController);
            break;

        case  sal_Char('S') :
            switch (rsURLPath[4])
            {
                case sal_Char('N'):
                    if (rsURLPath == A2S("ShowNotes"))
                        return new SetNotesViewCommand(true, rpPresenterController);
                    break;

                case sal_Char('S'):
                    if (rsURLPath == A2S("ShowSlideSorter"))
                        return new SetSlideSorterCommand(true, rpPresenterController);
                    break;

                case sal_Char('H'):
                    if (rsURLPath == A2S("ShowHelp"))
                        return new SetHelpViewCommand(true, rpPresenterController);
                    break;

                case sal_Char('n'):
                    if (rsURLPath == A2S("ShrinkNotesFont"))
                        return new NotesFontSizeCommand(rpPresenterController, -1);
                    break;
            }
            break;

        default:
            break;
    }

    return NULL;
}




PresenterProtocolHandler::Dispatch::~Dispatch (void)
{
}




void PresenterProtocolHandler::Dispatch::disposing (void)
{
    if (mbIsListeningToWindowManager)
    {
        if (mpPresenterController.get() != NULL)
            mpPresenterController->GetWindowManager()->RemoveLayoutListener(this);
        mbIsListeningToWindowManager = false;
    }

    msURLPath = OUString();
    mpCommand.reset();
}




//----- XDispatch -------------------------------------------------------------

void SAL_CALL PresenterProtocolHandler::Dispatch::dispatch(
    const css::util::URL& rURL,
    const css::uno::Sequence<css::beans::PropertyValue>& rArguments)
    throw(css::uno::RuntimeException)
{
    (void)rArguments;
    ThrowIfDisposed();

    if (rURL.Protocol == gsProtocol
        && rURL.Path == msURLPath)
    {
        if (mpCommand.get() != NULL)
            mpCommand->Execute();
    }
    else
    {
        // We can not throw an IllegalArgumentException
        throw RuntimeException();
    }
}




void SAL_CALL PresenterProtocolHandler::Dispatch::addStatusListener(
    const css::uno::Reference<css::frame::XStatusListener>& rxListener,
    const css::util::URL& rURL)
    throw(css::uno::RuntimeException)
{
    if (rURL.Path == msURLPath)
    {
        maStatusListenerContainer.push_back(rxListener);

        frame::FeatureStateEvent aEvent;
        aEvent.FeatureURL = rURL;
        aEvent.IsEnabled = mpCommand->IsEnabled();
        aEvent.Requery = sal_False;
        aEvent.State = mpCommand->GetState();
        rxListener->statusChanged(aEvent);
    }
    else
        throw RuntimeException();
}




void SAL_CALL PresenterProtocolHandler::Dispatch::removeStatusListener (
    const css::uno::Reference<css::frame::XStatusListener>& rxListener,
    const css::util::URL& rURL)
    throw(css::uno::RuntimeException)
{
    if (rURL.Path == msURLPath)
    {
        StatusListenerContainer::iterator iListener (
            ::std::find(
                maStatusListenerContainer.begin(),
                maStatusListenerContainer.end(),
                rxListener));
        if (iListener != maStatusListenerContainer.end())
            maStatusListenerContainer.erase(iListener);
    }
    else
        throw RuntimeException();
}




//-----------------------------------------------------------------------------

void PresenterProtocolHandler::Dispatch::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterProtocolHandler::Dispatch object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//----- document::XEventListener ----------------------------------------------

void SAL_CALL PresenterProtocolHandler::Dispatch::notifyEvent (
    const css::document::EventObject& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;

    mpCommand->GetState();
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterProtocolHandler::Dispatch::disposing (const css::lang::EventObject& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    mbIsListeningToWindowManager = false;
}





//===== GotoPreviousSlideCommand ==============================================

GotoPreviousSlideCommand::GotoPreviousSlideCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}



void GotoPreviousSlideCommand::Execute (void)
{
    if ( ! mpPresenterController.is())
        return;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return;

    mpPresenterController->GetSlideShowController()->gotoPreviousSlide();
}




bool GotoPreviousSlideCommand::IsEnabled (void) const
{
    if ( ! mpPresenterController.is())
        return false;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return false;

    return mpPresenterController->GetSlideShowController()->getCurrentSlideIndex()>0;
}




Any GotoPreviousSlideCommand::GetState (void) const
{
    return Any(sal_False);
}




//===== GotoNextEffect ========================================================

GotoNextEffectCommand::GotoNextEffectCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}



void GotoNextEffectCommand::Execute (void)
{
    if ( ! mpPresenterController.is())
        return;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return;

    mpPresenterController->GetSlideShowController()->gotoNextEffect();
}




bool GotoNextEffectCommand::IsEnabled (void) const
{
    // The next slide command is always enabled, even when the current slide
    // is the last slide:  from the last slide it goes to the pause slide,
    // and from there it ends the slide show.
    return true;
}




Any GotoNextEffectCommand::GetState (void) const
{
    return Any(sal_False);
}




//===== GotoNextSlide =========================================================

GotoNextSlideCommand::GotoNextSlideCommand (
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mpPresenterController(rpPresenterController)
{
}



void GotoNextSlideCommand::Execute (void)
{
    if ( ! mpPresenterController.is())
        return;

    if ( ! mpPresenterController->GetSlideShowController().is())
        return;

    mpPresenterController->GetSlideShowController()->gotoNextSlide();
}




bool GotoNextSlideCommand::IsEnabled (void) const
{
    // The next slide command is always enabled, even when the current slide
    // is the last slide:  from the last slide it goes to the pause slide,
    // and from there it ends the slide show.
    return true;
}




Any GotoNextSlideCommand::GetState (void) const
{
    return Any(sal_False);
}




//===== SetNotesViewCommand ===================================================

SetNotesViewCommand::SetNotesViewCommand (
    const bool bOn,
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mbOn(bOn),
      mpPresenterController(rpPresenterController)
{
}




void SetNotesViewCommand::Execute (void)
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




bool SetNotesViewCommand::IsEnabled (void) const
{
    return true;
}




Any SetNotesViewCommand::GetState (void) const
{
    if ( ! mpPresenterController.is())
        return Any(false);

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return Any(false);

    return Any(IsActive(pWindowManager));
}




bool SetNotesViewCommand::IsActive (
    const ::rtl::Reference<PresenterWindowManager>& rpWindowManager) const
{
    return rpWindowManager->GetViewMode() == PresenterWindowManager::VM_Notes;
}




//===== SetSlideSorterCommand =================================================

SetSlideSorterCommand::SetSlideSorterCommand (
    const bool bOn,
    const rtl::Reference<PresenterController>& rpPresenterController)
    : mbOn(bOn),
      mpPresenterController(rpPresenterController)
{
}




void SetSlideSorterCommand::Execute (void)
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    pWindowManager->SetSlideSorterState(mbOn);
}




bool SetSlideSorterCommand::IsEnabled (void) const
{
    return true;
}




Any SetSlideSorterCommand::GetState (void) const
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




void SetHelpViewCommand::Execute (void)
{
    if ( ! mpPresenterController.is())
        return;

    ::rtl::Reference<PresenterWindowManager> pWindowManager (
        mpPresenterController->GetWindowManager());
    if ( ! pWindowManager.is())
        return;

    pWindowManager->SetHelpViewState(mbOn);
}




bool SetHelpViewCommand::IsEnabled (void) const
{
    return true;
}




Any SetHelpViewCommand::GetState (void) const
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




::rtl::Reference<PresenterNotesView> NotesFontSizeCommand::GetNotesView (void) const
{
    if (mpPresenterController.get() == NULL)
        return NULL;

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPresenterController->GetPaneContainer()->FindViewURL(
            PresenterViewFactory::msNotesViewURL));
    if (pDescriptor.get() == NULL)
        return NULL;

    return dynamic_cast<PresenterNotesView*>(pDescriptor->mxView.get());
}




void NotesFontSizeCommand::Execute (void)
{
    ::rtl::Reference<PresenterNotesView> pView (GetNotesView());
    if (pView.is())
        pView->ChangeFontSize(mnSizeChange);
}




bool NotesFontSizeCommand::IsEnabled (void) const
{
    return true;
}




Any NotesFontSizeCommand::GetState (void) const
{
    return Any();
}


} } // end of namespace ::sdext::presenter
