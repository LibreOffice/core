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

#include "AccessibleDocumentViewBase.hxx"
#include <svx/AccessibleTextHelper.hxx>

namespace sd { class OutlineViewShell; }

namespace accessibility {

/** This class makes the Impress outline view accessible.

    Please see the documentation of the base class for further
    explanations of the individual methods. This class is a mere
    wrapper around the AccessibleTextHelper class; as basically the
    Outline View is a big Outliner.
*/
class AccessibleOutlineView final
    : public AccessibleDocumentViewBase
{
public:
    AccessibleOutlineView (
        ::sd::Window* pSdWindow,
        ::sd::OutlineViewShell* pViewShell,
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleOutlineView() override;

    /** Complete the initialization begun in the constructor.
    */
    virtual void Init() override;

    //=====  IAccessibleViewForwarderListener  ================================

    virtual void ViewForwarderChanged() override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex) override;
    virtual OUString SAL_CALL
        getAccessibleName() override;
    //=====  XAccessibleEventBroadcaster  ========================================

    virtual void SAL_CALL
        addAccessibleEventListener (
            const css::uno::Reference<css::accessibility::XAccessibleEventListener >& xListener) override;

    virtual void SAL_CALL
        removeAccessibleEventListener (
            const css::uno::Reference<css::accessibility::XAccessibleEventListener >& xListener) override;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName() override;

    //=====  lang::XEventListener  ============================================

    using AccessibleDocumentViewBase::disposing;

    //=====  XPropertyChangeListener  =========================================

    virtual void SAL_CALL
        propertyChange (const css::beans::PropertyChangeEvent& rEventObject) override;

private:

    // overridden to detect focus changes
    virtual void Activated() override;

    // overridden to detect focus changes
    virtual void Deactivated() override;

    // declared, but not defined
    AccessibleOutlineView( const AccessibleOutlineView& );
    AccessibleOutlineView& operator= ( const AccessibleOutlineView& );

    // This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing() override;

    /// Create an accessible name that contains the current view mode.
    virtual OUString
        CreateAccessibleName () override;

    /// Invalidate text helper, updates visible children
    void UpdateChildren();

    AccessibleTextHelper maTextHelper;

};

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
