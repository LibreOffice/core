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

#ifndef INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEOUTLINEVIEW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_ACCESSIBLEOUTLINEVIEW_HXX

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
class AccessibleOutlineView
    : public AccessibleDocumentViewBase
{
public:
    AccessibleOutlineView (
        ::sd::Window* pSdWindow,
        ::sd::OutlineViewShell* pViewShell,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XController>& rxController,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleOutlineView();

    /** Complete the initialization begun in the constructor.
    */
    virtual void Init() override;

    //=====  IAccessibleViewForwarderListener  ================================

    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    //=====  XAccessibleEventBroadcaster  ========================================

    virtual void SAL_CALL
        addAccessibleEventListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeAccessibleEventListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XPropertyChangeListener  =========================================

    virtual void SAL_CALL
        propertyChange (const ::com::sun::star::beans::PropertyChangeEvent& rEventObject)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:

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
        CreateAccessibleName ()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Create an accessible description that contains the current
    /// view mode.
    virtual OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:

    /// Invalidate text helper, updates visible children
    void UpdateChildren();

    AccessibleTextHelper maTextHelper;

};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
