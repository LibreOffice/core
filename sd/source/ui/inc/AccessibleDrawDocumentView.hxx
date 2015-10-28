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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEDRAWDOCUMENTVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEDRAWDOCUMENTVIEW_HXX

#include "AccessibleDocumentViewBase.hxx"

#include <com/sun/star/accessibility/XAccessibleGroupPosition.hpp>

namespace accessibility {

/** This class makes draw documents in the general view modes
    accessible.  It passes all shapes on the current draw page to a
    children manager and additionally creates a new shape that
    represents the actual draw page.

    Please see the documentation of the base class for further
    explanations of the individual methods.
*/
class AccessibleDrawDocumentView :
    public AccessibleDocumentViewBase
    ,public css::accessibility::XAccessibleGroupPosition
{
public:
    //=====  internal  ========================================================

    AccessibleDrawDocumentView (::sd::Window* pSdWindow,
        ::sd::ViewShell* pViewShell,
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleDrawDocumentView();

    /** Complete the initialization begun in the constructor.
    */
    virtual void Init() override;

    //=====  IAccessibleViewForwarderListener  ================================

    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (css::uno::RuntimeException,
            css::lang::IndexOutOfBoundsException, std::exception) override;

    virtual OUString SAL_CALL
        getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException, std::exception) override;

    //=====  XPropertyChangeListener  =========================================

    virtual void SAL_CALL
        propertyChange (const css::beans::PropertyChangeEvent& rEventObject)
        throw (css::uno::RuntimeException, std::exception) override;
    //=====  XInterface  ======================================================

    virtual css::uno::Any SAL_CALL
        queryInterface (const css::uno::Type & rType)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        acquire()
        throw () override;

    virtual void SAL_CALL
        release()
        throw () override;

    //=====  XAccessibleGroupPosition  =========================================
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL
        getGroupPosition( const css::uno::Any& rAny )
        throw (css::uno::RuntimeException,
               css::lang::IndexOutOfBoundsException, std::exception) override;
    virtual OUString SAL_CALL getObjectLink( const css::uno::Any& accoject )
        throw (css::uno::RuntimeException,
               std::exception) override;

protected:

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL
        getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual bool
        implIsSelected( sal_Int32 nAccessibleChildIndex )
        throw (css::uno::RuntimeException) override;

    /** Select or deselect the specified child or all children if the given
        index has the special value ACCESSIBLE_SELECTION_CHILD_ALL.
        Selecting or deselecting a child sets or resets the
        <const>SELECTED</const> state and selects or deselects the UNO shape
        being made accessible by the child.
        @param nAccessibleChildIndex
            Index of the child to select or deselect.  If the parameter has
            the value ACCESSIBLE_SELECTION_CHILD_ALL then all children are
            selected or deslected.
        @param bSelect
            Indicates whether to select or deselect the specified child
            reps. children.
    */
    virtual void
        implSelect( sal_Int32 nAccessibleChildIndex, bool bSelect )
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException) override;
private:
    ::sd::ViewShell* mpSdViewSh;

protected:
    /** This object manages the shapes of the represented draw page.  It is
        responsible to determine the visible shapes and create on demand the
        accessible objects representing them.
    */
    ChildrenManager* mpChildrenManager;

    // This method is called from the component helper base class while
    // disposing.
    virtual void SAL_CALL disposing() override;

    /** Create a shape the represents the page as seen on the screen.
    */
    rtl::Reference<AccessiblePageShape> CreateDrawPageShape();

    /// Create an accessible name that contains the current view mode.
    virtual OUString
        CreateAccessibleName ()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Create an accessible description that contains the current
        view mode.
    */
    virtual OUString
        CreateAccessibleDescription ()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Make sure that the currently focused shape sends a FOCUSED state
        change event indicating that it has (regained) the focus.
    */
    virtual void Activated() override;

    /** Make sure that the currently focused shape sends a FOCUSED state
        change event indicating that it has lost the focus.
    */
    virtual void Deactivated() override;

    virtual void impl_dispose() override;

    //=====  XAccessibleGetAccFromXShape  ============================================
    css::uno::Sequence< css::uno::Any >
        SAL_CALL getAccFlowTo(const css::uno::Any& rAny, sal_Int32 nType)
        throw ( css::uno::RuntimeException, std::exception ) override;
    css::uno::Reference< css::accessibility::XAccessible >
        GetSelAccContextInTable();

private:
    void UpdateAccessibleName();
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
