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


#ifndef _SC_ACCESSIBLEPREVIEWTABLE_HXX
#define _SC_ACCESSIBLEPREVIEWTABLE_HXX

#include "AccessibleContextBase.hxx"
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <cppuhelper/implbase1.hxx>

class ScPreviewShell;
class ScPreviewTableInfo;

typedef cppu::ImplHelper1< ::com::sun::star::accessibility::XAccessibleTable>
                    ScAccessiblePreviewTableImpl;

class ScAccessiblePreviewTable :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewTableImpl
{
public:
    ScAccessiblePreviewTable( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewTable();

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

    //=====  XAccessibleTable  ================================================

    virtual sal_Int32 SAL_CALL getAccessibleRowCount() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount() throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                            getAccessibleRowHeaders() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleTable > SAL_CALL
                            getAccessibleColumnHeaders() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows()
                                throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns()
                                throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleCaption() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
                            getAccessibleSummary() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
                                throw (::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);

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

    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);

    //=====  internal  ========================================================
    void SetCurrentIndexInParent(sal_Int32 nNew) { mnIndex = nNew; }

protected:
    virtual OUString SAL_CALL createAccessibleDescription(void) throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL createAccessibleName(void) throw (::com::sun::star::uno::RuntimeException);

    virtual Rectangle GetBoundingBoxOnScreen(void) const throw(::com::sun::star::uno::RuntimeException);
    virtual Rectangle GetBoundingBox(void) const throw (::com::sun::star::uno::RuntimeException);

private:
    ScPreviewShell*     mpViewShell;
    sal_Int32           mnIndex;
    mutable ScPreviewTableInfo* mpTableInfo;

    sal_Bool IsDefunc(
        const com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    void    FillTableInfo() const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
