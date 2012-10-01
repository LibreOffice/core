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

#ifndef SDEXT_PRESENTER_PRESENTER_ACCESSIBILITY_HXX
#define SDEXT_PRESENTER_PRESENTER_ACCESSIBILITY_HXX

#include "PresenterPaneContainer.hxx"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/WindowEvent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XPane2.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>
#include <rtl/ref.hxx>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;
namespace cssa = ::com::sun::star::accessibility;

namespace sdext { namespace presenter {

class PresenterController;
class PresenterTextView;

namespace {
    typedef ::cppu::WeakComponentImplHelper3 <
        css::accessibility::XAccessible,
        css::lang::XInitialization,
        css::awt::XFocusListener
    > PresenterAccessibleInterfaceBase;
}

class PresenterAccessible
    : public ::cppu::BaseMutex,
      public PresenterAccessibleInterfaceBase
{
public:
    PresenterAccessible (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController,
        const css::uno::Reference<css::drawing::framework::XPane>& rxMainPane);
    virtual ~PresenterAccessible (void);

    void SetAccessibleParent (const cssu::Reference<cssa::XAccessible>& rxAccessibleParent);

    void UpdateAccessibilityHierarchy (void);

    void NotifyCurrentSlideChange (
        const sal_Int32 nCurrentSlideIndex,
        const sal_Int32 nSlideCount);

    virtual void SAL_CALL disposing (void);

    //----- XAccessible -------------------------------------------------------

    virtual cssu::Reference<cssa::XAccessibleContext> SAL_CALL
        getAccessibleContext (void)
        throw (cssu::RuntimeException);

    //----- XFocusListener ----------------------------------------------------

    virtual void SAL_CALL focusGained (const css::awt::FocusEvent& rEvent)
        throw (cssu::RuntimeException);

    virtual void SAL_CALL focusLost (const css::awt::FocusEvent& rEvent)
        throw (cssu::RuntimeException);

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (cssu::RuntimeException);

    //----- XInitialization ---------------------------------------------------

    virtual void SAL_CALL initialize (const cssu::Sequence<cssu::Any>& rArguments)
        throw (cssu::RuntimeException);

    class AccessibleObject;
    class AccessibleParagraph;

private:
    const css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainPaneId;
    css::uno::Reference<css::drawing::framework::XPane2> mxMainPane;
    css::uno::Reference<css::awt::XWindow> mxMainWindow;
    css::uno::Reference<css::awt::XWindow> mxPreviewContentWindow;
    css::uno::Reference<css::awt::XWindow> mxPreviewBorderWindow;
    css::uno::Reference<css::awt::XWindow> mxNotesContentWindow;
    css::uno::Reference<css::awt::XWindow> mxNotesBorderWindow;
    ::rtl::Reference<AccessibleObject> mpAccessibleConsole;
    ::rtl::Reference<AccessibleObject> mpAccessiblePreview;
    ::rtl::Reference<AccessibleObject> mpAccessibleNotes;
    css::uno::Reference<css::accessibility::XAccessible> mxAccessibleParent;

    void UpdateAccessibilityHierarchy (
        const css::uno::Reference<css::awt::XWindow>& rxPreviewContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxPreviewBorderWindow,
        const ::rtl::OUString& rsTitle,
        const css::uno::Reference<css::awt::XWindow>& rxNotesContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxNotesBorderWindow,
        const ::boost::shared_ptr<PresenterTextView>& rpNotesTextView);
    PresenterPaneContainer::SharedPaneDescriptor GetPreviewPane (void) const;
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
