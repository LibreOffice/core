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

#include "AccessibleFocusManager.hxx"
#include "AccessibleNotes.hxx"
#include "PresenterAccessibility.hxx"
#include "PresenterTextView.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"

#include <strings.hrc>
#include <sdresid.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <framework/AbstractPane.hxx>
#include <framework/AbstractView.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext::presenter {

namespace {

//===== AccessiblePreview =====================================================

class AccessiblePreview
{
public:
    static rtl::Reference<AccessibleObject> Create(
        const Reference<awt::XWindow>& rxContentWindow,
        const Reference<awt::XWindow>& rxBorderWindow)
    {
        const OUString sName = SdResId(STR_A11Y_PRESENTER_PREVIEW);
        rtl::Reference<AccessibleObject> pObject (
            new AccessibleObject(
                AccessibleRole::LABEL,
                sName));
        pObject->LateInitialization();
        pObject->UpdateStateSet();
        pObject->SetWindow(rxContentWindow, rxBorderWindow);

        return pObject;
    }
};

}

//===== PresenterAccessible ===================================================

PresenterAccessible::PresenterAccessible(
    const rtl::Reference<PresenterController>& xPresenterController,
    const rtl::Reference<sd::framework::AbstractPane>& rxMainPane)
    : ImplInheritanceHelper(AccessibleRole::PANEL, SdResId(STR_A11Y_PRESENTER_CONSOLE))
    , mpPresenterController(xPresenterController)
    , mxMainPane(rxMainPane)
{
    assert(mxMainPane.is());
    mxMainWindow = mxMainPane->getWindow();
    if (VclPtr<vcl::Window> pMainPaneWin = VCLUnoHelper::GetWindow(mxMainWindow))
    {
        pMainPaneWin->SetAccessible(this);
        mxAccessibleParent = pMainPaneWin->GetAccessibleParent();
    }
}

PresenterAccessible::~PresenterAccessible()
{
}

PresenterPaneContainer::SharedPaneDescriptor PresenterAccessible::GetPreviewPane() const
{
    PresenterPaneContainer::SharedPaneDescriptor pPreviewPane;

    if ( ! mpPresenterController.is())
        return pPreviewPane;

    rtl::Reference<PresenterPaneContainer> pContainer (mpPresenterController->GetPaneContainer());
    if ( ! pContainer.is())
        return pPreviewPane;

    pPreviewPane = pContainer->FindPaneURL(PresenterPaneFactory::msCurrentSlidePreviewPaneURL);
    rtl::Reference<sd::framework::AbstractPane> xPreviewPane;
    if (pPreviewPane)
        xPreviewPane = pPreviewPane->mxPane.get();
    if ( ! xPreviewPane.is())
    {
        pPreviewPane = pContainer->FindPaneURL(PresenterPaneFactory::msSlideSorterPaneURL);
    }
    return pPreviewPane;
}

void PresenterAccessible::UpdateAccessibilityHierarchy()
{
    if ( ! mpPresenterController.is())
        return;

    rtl::Reference<PresenterPaneContainer> pPaneContainer (
        mpPresenterController->GetPaneContainer());
    if ( ! pPaneContainer.is())
        return;

    // Get the preview pane (standard or notes view) or the slide overview
    // pane.
    PresenterPaneContainer::SharedPaneDescriptor pPreviewPane(GetPreviewPane());

    // Get the notes pane.
    PresenterPaneContainer::SharedPaneDescriptor pNotesPane(
        pPaneContainer->FindPaneURL(PresenterPaneFactory::msNotesPaneURL));

    // Get the notes view.
    rtl::Reference<sd::framework::AbstractView> xNotesView;
    if (pNotesPane)
        xNotesView = pNotesPane->mxView;
    rtl::Reference<PresenterNotesView> pNotesView (
        dynamic_cast<PresenterNotesView*>(xNotesView.get()));

    UpdateAccessibilityHierarchy(
        pPreviewPane ? pPreviewPane->mxContentWindow : Reference<awt::XWindow>(),
        pPreviewPane ? pPreviewPane->mxBorderWindow : Reference<awt::XWindow>(),
        pPreviewPane ? pPreviewPane->msAccessibleName : OUString(),
        pNotesPane ? pNotesPane->mxContentWindow : Reference<awt::XWindow>(),
        pNotesPane ? pNotesPane->mxBorderWindow : Reference<awt::XWindow>(),
        pNotesView.is()
            ? pNotesView->GetTextView()
            : std::shared_ptr<PresenterTextView>());
}

void PresenterAccessible::UpdateAccessibilityHierarchy (
    const Reference<awt::XWindow>& rxPreviewContentWindow,
    const Reference<awt::XWindow>& rxPreviewBorderWindow,
    const OUString& rsTitle,
    const Reference<awt::XWindow>& rxNotesContentWindow,
    const Reference<awt::XWindow>& rxNotesBorderWindow,
    const std::shared_ptr<PresenterTextView>& rpNotesTextView)
{
    if (mxPreviewContentWindow != rxPreviewContentWindow)
    {
        if (mpAccessiblePreview.is())
        {
            RemoveChild(mpAccessiblePreview);
            mpAccessiblePreview->dispose();
            mpAccessiblePreview = nullptr;
        }

        mxPreviewContentWindow = rxPreviewContentWindow;
        mxPreviewBorderWindow = rxPreviewBorderWindow;

        if (mxPreviewContentWindow.is())
        {
            mpAccessiblePreview = AccessiblePreview::Create(
                mxPreviewContentWindow,
                mxPreviewBorderWindow);
            AddChild(mpAccessiblePreview);
            mpAccessiblePreview->SetAccessibleName(rsTitle);
            mpAccessiblePreview->SetAccessibleParent(this);
        }
    }

    if (mxNotesContentWindow == rxNotesContentWindow)
        return;

    if (mpAccessibleNotes.is())
    {
        RemoveChild(mpAccessibleNotes);
        mpAccessibleNotes->dispose();
        mpAccessibleNotes = nullptr;
    }

    mxNotesContentWindow = rxNotesContentWindow;
    mxNotesBorderWindow = rxNotesBorderWindow;

    if (mxNotesContentWindow.is())
    {
        mpAccessibleNotes = AccessibleNotes::Create(
            mxNotesContentWindow,
            mxNotesBorderWindow,
            rpNotesTextView);
        AddChild(mpAccessibleNotes);
    }
}

void PresenterAccessible::NotifyCurrentSlideChange ()
{
    if (mpAccessiblePreview.is())
    {
        PresenterPaneContainer::SharedPaneDescriptor pPreviewPane (GetPreviewPane());
        mpAccessiblePreview->SetAccessibleName(
            pPreviewPane ? pPreviewPane->msAccessibleName : OUString());
    }

    // Play some focus ping-pong to trigger AT tools.
    //AccessibleFocusManager::Instance()->FocusObject(this);
    AccessibleFocusManager::Instance()->FocusObject(mpAccessiblePreview);
}

void SAL_CALL PresenterAccessible::disposing()
{
    UpdateAccessibilityHierarchy(
        nullptr,
        nullptr,
        OUString(),
        nullptr,
        nullptr,
        std::shared_ptr<PresenterTextView>());

    if (mxMainWindow.is())
    {
        mxMainWindow->removeFocusListener(this);

        if (mxMainPane.is())
        {
            if (VclPtr<vcl::Window> pMainPaneWin = VCLUnoHelper::GetWindow(mxMainPane->getWindow()))
                pMainPaneWin->SetAccessible(nullptr);
        }
    }

    if (mpAccessiblePreview)
        mpAccessiblePreview->dispose();
    mpAccessiblePreview = nullptr;

    if (mpAccessibleNotes)
        mpAccessibleNotes->dispose();
    mpAccessibleNotes = nullptr;

    AccessibleObject::disposing();
}

rtl::Reference<PresenterAccessible>
PresenterAccessible::Create(const rtl::Reference<PresenterController>& xPresenterController,
                            const rtl::Reference<sd::framework::AbstractPane>& rxMainPane)
{
    rtl::Reference<PresenterAccessible> pPresenterAcc
        = new PresenterAccessible(xPresenterController, rxMainPane);
    pPresenterAcc->mxMainWindow->addFocusListener(pPresenterAcc);

    pPresenterAcc->LateInitialization();
    pPresenterAcc->UpdateStateSet();

    pPresenterAcc->SetWindow(pPresenterAcc->mxMainWindow, nullptr);
    pPresenterAcc->SetAccessibleParent(pPresenterAcc->mxAccessibleParent);
    pPresenterAcc->UpdateAccessibilityHierarchy();

    return pPresenterAcc;
}

//----- XFocusListener ----------------------------------------------------

void SAL_CALL PresenterAccessible::focusGained (const css::awt::FocusEvent&)
{
    SAL_INFO("sdext.presenter", __func__ << ": PresenterAccessible::focusGained at " << this
        << " and window " << mxMainWindow.get());
    AccessibleFocusManager::Instance()->FocusObject(this);
}

void SAL_CALL PresenterAccessible::focusLost (const css::awt::FocusEvent&)
{
    SAL_INFO("sdext.presenter", __func__ << ": PresenterAccessible::focusLost at " << this);
    AccessibleFocusManager::Instance()->FocusObject(nullptr);
}

//----- XEventListener ----------------------------------------------------

void SAL_CALL PresenterAccessible::disposing (const css::lang::EventObject& rEvent)
{
    if (rEvent.Source == mxMainWindow)
        mxMainWindow = nullptr;
}

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
