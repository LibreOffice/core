/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_VIEW_HXX
#define SD_ACCESSIBILITY_ACCESSIBLE_OUTLINE_VIEW_HXX

#include "AccessibleDocumentViewBase.hxx"
#include <svx/AccessibleTextHelper.hxx>

namespace sd {
class OutlineViewShell;
class Window;
}

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

    virtual ~AccessibleOutlineView  (void);

    /** Complete the initialization begun in the constructor.
    */
    virtual void Init (void);


    //=====  IAccessibleViewForwarderListener  ================================

    virtual void ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleEventBroadcaster  ========================================

    virtual void SAL_CALL
        addEventListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeEventListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);

    using cppu::WeakComponentImplHelperBase::addEventListener;
    using cppu::WeakComponentImplHelperBase::removeEventListener;

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual ::rtl::OUString SAL_CALL
        getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  lang::XEventListener  ============================================

    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XPropertyChangeListener  =========================================

    virtual void SAL_CALL
        propertyChange (const ::com::sun::star::beans::PropertyChangeEvent& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


protected:

    // overridden, as we hold the listeners ourselves
    virtual void FireEvent (const ::com::sun::star::accessibility::AccessibleEventObject& aEvent);

    // overridden to detect focus changes
    virtual void Activated (void);

    // overridden to detect focus changes
    virtual void Deactivated (void);

    // declared, but not defined
    AccessibleOutlineView( const AccessibleOutlineView& );
    AccessibleOutlineView& operator= ( const AccessibleOutlineView& );

    // This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing (void);

    /// Create an accessible name that contains the current view mode.
    virtual ::rtl::OUString
        CreateAccessibleName ()
        throw (::com::sun::star::uno::RuntimeException);

    /// Create an accessible description that contains the current
    /// view mode.
    virtual ::rtl::OUString
        CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException);

private:

    /// Invalidate text helper, updates visible children
    void UpdateChildren();

    AccessibleTextHelper maTextHelper;

};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
