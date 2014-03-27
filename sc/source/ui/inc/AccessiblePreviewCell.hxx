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


#ifndef _SC_ACCESSIBLEPREVIEWCELL_HXX
#define _SC_ACCESSIBLEPREVIEWCELL_HXX

#include "AccessibleCellBase.hxx"

class ScPreviewShell;

namespace accessibility
{
    class AccessibleTextHelper;
}

class ScAccessiblePreviewCell : public ScAccessibleCellBase
{
public:
    //=====  internal  ========================================================
    ScAccessiblePreviewCell(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell, /* const */ ScAddress& rCellAddress, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewCell();

    using ScAccessibleCellBase::IsDefunc;

public:
    using ScAccessibleCellBase::disposing;
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   grabFocus() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //=====  XAccessibleContext  ==============================================

    // overloaded to calculate this on demand
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    virtual Rectangle GetBoundingBoxOnScreen() const throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual Rectangle GetBoundingBox() const throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    ScPreviewShell* mpViewShell;

    accessibility::AccessibleTextHelper* mpTextHelper;

    bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
    virtual bool IsEditable(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates) SAL_OVERRIDE;
    bool IsOpaque(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
