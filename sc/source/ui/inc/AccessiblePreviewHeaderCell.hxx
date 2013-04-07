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


#ifndef _SC_ACCESSIBLEPREVIEWHEADERCELL_HXX
#define _SC_ACCESSIBLEPREVIEWHEADERCELL_HXX

#include "AccessibleContextBase.hxx"
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <tools/gen.hxx>
#include "global.hxx"
#include "address.hxx"
#include <cppuhelper/implbase1.hxx>

class ScPreviewShell;
class ScPreviewTableInfo;
namespace accessibility {
    class AccessibleTextHelper;
}

typedef cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleValue>
                    ScAccessiblePreviewHeaderCellImpl;

class ScAccessiblePreviewHeaderCell :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewHeaderCellImpl
{
public:
    ScAccessiblePreviewHeaderCell( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellPos, sal_Bool bIsColHdr, sal_Bool bIsRowHdr,
                            sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewHeaderCell();

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;
     virtual void SAL_CALL disposing();

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  XInterface  =====================================================

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ();

    virtual void SAL_CALL release() throw ();

    //=====  XAccessibleValue  ================================================

    virtual ::com::sun::star::uno::Any SAL_CALL getCurrentValue() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL setCurrentValue( const ::com::sun::star::uno::Any& aNumber )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMaximumValue() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getMinimumValue() throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   grabFocus() throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException);

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

    ///=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
    virtual OUString SAL_CALL createAccessibleDescription(void) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL createAccessibleName(void) throw (::com::sun::star::uno::RuntimeException);

    virtual Rectangle GetBoundingBoxOnScreen(void) const throw(::com::sun::star::uno::RuntimeException);
    virtual Rectangle GetBoundingBox(void) const throw (::com::sun::star::uno::RuntimeException);

private:
    ScPreviewShell*     mpViewShell;
    accessibility::AccessibleTextHelper* mpTextHelper;
    sal_Int32           mnIndex;
    ScAddress           maCellPos;
    sal_Bool            mbColumnHeader;
    sal_Bool            mbRowHeader;
    mutable ScPreviewTableInfo* mpTableInfo;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();
    void    FillTableInfo() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
