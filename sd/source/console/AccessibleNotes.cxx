/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <strings.hrc>
#include <sdresid.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>


namespace sdext::presenter {

AccessibleNotes::AccessibleNotes(const OUString& rsName)
    : AccessibleObject(AccessibleRole::PANEL, rsName)
{
}

rtl::Reference<AccessibleObject> AccessibleNotes::Create(
    const Reference<awt::XWindow>& rxContentWindow,
    const Reference<awt::XWindow>& rxBorderWindow,
    const std::shared_ptr<PresenterTextView>& rpTextView)
{
    const OUString sName = SdResId(STR_A11Y_PRESENTER_NOTES);
    rtl::Reference<AccessibleNotes> pObject (
        new AccessibleNotes(sName));
    pObject->LateInitialization();
    pObject->SetTextView(rpTextView);
    pObject->UpdateStateSet();
    pObject->SetWindow(rxContentWindow, rxBorderWindow);

    return pObject;
}

void AccessibleNotes::SetTextView (
    const std::shared_ptr<PresenterTextView>& rpTextView)
{
    ::std::vector<rtl::Reference<AccessibleObject> > aChildren;

    // Release any listeners to the current text view.
    if (mpTextView)
    {
        mpTextView->GetCaret()->SetCaretMotionBroadcaster(
            ::std::function<void (sal_Int32,sal_Int32,sal_Int32,sal_Int32)>());
        mpTextView->SetTextChangeBroadcaster(
            ::std::function<void ()>());
    }

    mpTextView = rpTextView;

    if (!mpTextView)
        return;

    // Create a new set of children, one for each paragraph.
    const sal_Int32 nParagraphCount (mpTextView->GetParagraphCount());
    for (sal_Int32 nIndex=0; nIndex<nParagraphCount; ++nIndex)
    {
        rtl::Reference<AccessibleParagraph> pParagraph (
            new AccessibleParagraph(
                rpTextView->GetParagraph(nIndex),
                nIndex));
        pParagraph->LateInitialization();
        pParagraph->SetWindow(mxContentWindow, mxBorderWindow);
        pParagraph->SetAccessibleParent(this);
        aChildren.emplace_back(pParagraph.get());
    }
    maChildren.swap(aChildren);
    FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());

    // Dispose the old children. (This will remove them from the focus
    // manager).
    for (const auto& rxChild : aChildren)
    {
        if (rxChild.is())
            rxChild->dispose();
    }

    // This class acts as a controller of who broadcasts caret motion
    // events and handles text changes.  Register the corresponding
    // listeners here.
    mpTextView->GetCaret()->SetCaretMotionBroadcaster(
        [this](sal_Int32 a, sal_Int32 b, sal_Int32 c, sal_Int32 d)
            { return this->NotifyCaretChange(a, b, c, d); });
    mpTextView->SetTextChangeBroadcaster(
        [this]() { return SetTextView(mpTextView); });
}

void AccessibleNotes::SetWindow (
    const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
    const css::uno::Reference<css::awt::XWindow>& rxBorderWindow)
{
    AccessibleObject::SetWindow(rxContentWindow, rxBorderWindow);

    // Set the windows at the children as well, so that every paragraph can
    // setup its geometry.
    for (auto& rxChild : maChildren)
    {
        rxChild->SetWindow(rxContentWindow, rxBorderWindow);
    }
}

void AccessibleNotes::NotifyCaretChange (
    const sal_Int32 nOldParagraphIndex,
    const sal_Int32 nOldCharacterIndex,
    const sal_Int32 nNewParagraphIndex,
    const sal_Int32 nNewCharacterIndex)
{
    AccessibleFocusManager::Instance()->FocusObject(
        nNewParagraphIndex >= 0
            ? maChildren[nNewParagraphIndex]
            : this);

    if (nOldParagraphIndex != nNewParagraphIndex)
    {
        // Moved caret from one paragraph to another (or showed or
        // hid the caret).  Move focus from one accessible
        // paragraph to another.
        if (nOldParagraphIndex >= 0)
        {
            maChildren[nOldParagraphIndex]->FireAccessibleEvent(
                AccessibleEventId::CARET_CHANGED,
                Any(nOldCharacterIndex),
                Any(sal_Int32(-1)));
        }
        if (nNewParagraphIndex >= 0)
        {
            maChildren[nNewParagraphIndex]->FireAccessibleEvent(
                AccessibleEventId::CARET_CHANGED,
                Any(sal_Int32(-1)),
                Any(nNewCharacterIndex));
        }
    }
    else if (nNewParagraphIndex >= 0)
    {
        // Caret moved inside one paragraph.
        maChildren[nNewParagraphIndex]->FireAccessibleEvent(
            AccessibleEventId::CARET_CHANGED,
            Any(nOldCharacterIndex),
            Any(nNewCharacterIndex));
    }
}

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
