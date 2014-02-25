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


#ifndef _SC_ACCESSIBLEDOCUMENTPAGEPREVIEW_HXX
#define _SC_ACCESSIBLEDOCUMENTPAGEPREVIEW_HXX

#include "AccessibleDocumentBase.hxx"

class ScPreviewShell;
class ScNotesChildren;
class ScShapeChildren;
class ScAccessiblePreviewTable;
class ScAccessiblePageHeader;

class ScAccessibleDocumentPagePreview
    :   public ScAccessibleDocumentBase
{
public:
    //=====  internal  ========================================================
    ScAccessibleDocumentPagePreview(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell );
protected:
    virtual ~ScAccessibleDocumentPagePreview(void);

    using ScAccessibleDocumentBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;

     virtual void SAL_CALL disposing();

   ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint(
        const ::com::sun::star::awt::Point& rPoint )
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL grabFocus(  )
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    ///=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /// Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
               ::com::sun::star::lang::IndexOutOfBoundsException,
               std::exception);

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual OUString SAL_CALL getAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);
    ///=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /** Returns a list of all supported services.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    ///=====  internal  ========================================================

protected:
    /// Return this object's description.
    virtual OUString SAL_CALL
        createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current name.
    virtual OUString SAL_CALL
        createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

public: // needed in ScShapeChildren
    /// Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen(void) const
        throw (::com::sun::star::uno::RuntimeException, std::exception);

protected:
    /// Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox(void) const
        throw (::com::sun::star::uno::RuntimeException, std::exception);

private:
    ScPreviewShell* mpViewShell;
    ScNotesChildren* mpNotesChildren;
    ScShapeChildren* mpShapeChildren;
    ScAccessiblePreviewTable* mpTable;
    ScAccessiblePageHeader* mpHeader;
    ScAccessiblePageHeader* mpFooter;

    bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    ScNotesChildren* GetNotesChildren();
    ScShapeChildren* GetShapeChildren();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
