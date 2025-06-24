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

#pragma once

#include "AccessibleObject.hxx"
#include "AccessibleParagraph.hxx"
#include "PresenterPaneContainer.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <framework/AbstractPane.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <memory>


namespace sdext::presenter {

class PresenterController;
class PresenterTextView;

class PresenterAccessible
    : public cppu::ImplInheritanceHelper<AccessibleObject, css::awt::XFocusListener>
{
    PresenterAccessible(const rtl::Reference<PresenterController>& xPresenterController,
                        const rtl::Reference<sd::framework::AbstractPane>& rxMainPane);

public:
    static rtl::Reference<PresenterAccessible>
    Create(const rtl::Reference<PresenterController>& xPresenterController,
           const rtl::Reference<sd::framework::AbstractPane>& rxMainPane);

    virtual ~PresenterAccessible() override;

    void UpdateAccessibilityHierarchy();

    void NotifyCurrentSlideChange ();

    virtual void SAL_CALL disposing() override;

    //----- XFocusListener ----------------------------------------------------

    virtual void SAL_CALL focusGained (const css::awt::FocusEvent& rEvent) override;

    virtual void SAL_CALL focusLost (const css::awt::FocusEvent& rEvent) override;

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

private:
    ::rtl::Reference<PresenterController> mpPresenterController;
    rtl::Reference<sd::framework::AbstractPane> mxMainPane;
    css::uno::Reference<css::awt::XWindow> mxMainWindow;
    css::uno::Reference<css::awt::XWindow> mxPreviewContentWindow;
    css::uno::Reference<css::awt::XWindow> mxPreviewBorderWindow;
    css::uno::Reference<css::awt::XWindow> mxNotesContentWindow;
    css::uno::Reference<css::awt::XWindow> mxNotesBorderWindow;
    ::rtl::Reference<AccessibleObject> mpAccessiblePreview;
    ::rtl::Reference<AccessibleObject> mpAccessibleNotes;
    css::uno::Reference<css::accessibility::XAccessible> mxAccessibleParent;

    void UpdateAccessibilityHierarchy (
        const css::uno::Reference<css::awt::XWindow>& rxPreviewContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxPreviewBorderWindow,
        const OUString& rsTitle,
        const css::uno::Reference<css::awt::XWindow>& rxNotesContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxNotesBorderWindow,
        const std::shared_ptr<PresenterTextView>& rpNotesTextView);
    PresenterPaneContainer::SharedPaneDescriptor GetPreviewPane() const;
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
