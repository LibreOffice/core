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

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <svl/languageoptions.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include "slidelayoutcontroller.hxx"
#include "ViewShellBase.hxx"
#include <SlideSorterViewShell.hxx>
#include <DrawController.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <TransitionPreset.hxx>
#include <sfx2/viewfrm.hxx>
#include <SlideTransitionPane.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;

namespace sd
{

class SlideTransitionController : public svt::ToolboxController, public lang::XServiceInfo
{
    Timer m_aLateInitTimer;
    VclPtr<VclVBox> m_pContainer;
    VclPtr<SlideTransitionPane> m_pPane;

public:
    SlideTransitionController(const uno::Reference<uno::XComponentContext>& rContext);
    ~SlideTransitionController();
    virtual void dispose() throw (css::uno::RuntimeException, std::exception) override;
    virtual uno::Any SAL_CALL queryInterface(const uno::Type& aType) throw (uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    virtual void initialize(const uno::Sequence<uno::Any>& aArguments) throw (uno::Exception, uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& rEvent) throw (uno::RuntimeException, std::exception) override;
    virtual uno::Reference<awt::XWindow> createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getImplementationName() throw(RuntimeException, std::exception) override;
    virtual Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception) override;

    DECL_LINK_TYPED(LateInitCallback, Timer *, void);
};

IMPL_LINK_NOARG_TYPED(SlideTransitionController, LateInitCallback, Timer *, void)
{
    ViewShellBase* pBase = nullptr;
    css::uno::Reference<css::drawing::XDrawView> xView;
    css::uno::Reference<css::frame::XModel> xModel;

    if(SfxViewFrame::Current())
        pBase = ViewShellBase::GetViewShellBase(SfxViewFrame::Current());

    if(pBase && pBase->GetDocShell())
    {
        SdDrawDocument* pDoc = pBase->GetDocShell()->GetDoc();
        if(pDoc)
            xModel.set(pDoc->getUnoModel(), uno::UNO_QUERY);
        if(xModel.is())
            xView.set(xModel->getCurrentController(), uno::UNO_QUERY);
        if(xView.is())
            m_pPane = VclPtr<SlideTransitionPane>::Create(m_pContainer, *pBase, pDoc, m_xFrame, false);
    }
}

SlideTransitionController::SlideTransitionController(const uno::Reference<uno::XComponentContext>& rContext)
    : svt::ToolboxController(rContext, uno::Reference<frame::XFrame>(), OUString(".uno:SlideTransitionController"))
{
}

SlideTransitionController::~SlideTransitionController()
{
}

void SlideTransitionController::dispose() throw (css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarMutexGuard;

    m_aLateInitTimer.Stop();
    m_pPane.disposeAndClear();
    m_pContainer.disposeAndClear();
    svt::ToolboxController::dispose();
}

uno::Any SAL_CALL SlideTransitionController::queryInterface(const uno::Type& aType) throw (uno::RuntimeException, std::exception)
{
    uno::Any a(ToolboxController::queryInterface(aType));
    if (a.hasValue())
        return a;

    return ::cppu::queryInterface(aType, static_cast<lang::XServiceInfo*>(this));
}

void SAL_CALL SlideTransitionController::acquire() throw ()
{
    ToolboxController::acquire();
}

void SAL_CALL SlideTransitionController::release() throw ()
{
    ToolboxController::release();
}

void SlideTransitionController::initialize(const uno::Sequence<uno::Any>& aArguments) throw (uno::Exception, uno::RuntimeException, std::exception)
{
    svt::ToolboxController::initialize(aArguments);
}

void SAL_CALL SlideTransitionController::statusChanged(const frame::FeatureStateEvent& /*rEvent*/) throw (uno::RuntimeException, std::exception)
{
}

uno::Reference<awt::XWindow> SlideTransitionController::createItemWindow(const uno::Reference<awt::XWindow>& rParent) throw (uno::RuntimeException, std::exception)
{
    vcl::Window* pParent = VCLUnoHelper::GetWindow(rParent);
    ToolBox* pToolbar = dynamic_cast<ToolBox*>(pParent);
    if (pToolbar)
    {
        m_pContainer = VclPtr<VclVBox>::Create(pToolbar);
        m_pContainer->Show();
    }

    m_aLateInitTimer.SetTimeout(200);
    m_aLateInitTimer.SetTimeoutHdl(LINK(this, SlideTransitionController, LateInitCallback));
    m_aLateInitTimer.Start();

    return uno::Reference<awt::XWindow>(VCLUnoHelper::GetInterface(m_pContainer));
}

OUString SAL_CALL SlideTransitionController::getImplementationName() throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.sd.SlideTransitionController");
}

Sequence<OUString> SAL_CALL SlideTransitionController::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    return Sequence<OUString> { "com.sun.star.frame.ToolbarController" };
}

sal_Bool SAL_CALL SlideTransitionController::supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_sd_SlideTransitionController_get_implementation(css::uno::XComponentContext* context,
                                                              css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new sd::SlideTransitionController(context));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
