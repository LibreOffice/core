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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPREVIEWTABLE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPREVIEWTABLE_HXX

#include "AccessibleContextBase.hxx"
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <cppuhelper/implbase.hxx>

class ScPreviewShell;
class ScPreviewTableInfo;

typedef cppu::ImplHelper < css::accessibility::XAccessibleTable >
                    ScAccessiblePreviewTableImpl;

class ScAccessiblePreviewTable :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewTableImpl
{
public:
    ScAccessiblePreviewTable( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewTable() override;

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::disposing;
     virtual void SAL_CALL disposing() override;

    //=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    ///=====  XInterface  =====================================================

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & rType )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;

    virtual void SAL_CALL release() throw () override;

    //=====  XAccessibleTable  ================================================

    virtual sal_Int32 SAL_CALL getAccessibleRowCount()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnCount()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual OUString SAL_CALL getAccessibleRowDescription( sal_Int32 nRow )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual OUString SAL_CALL getAccessibleColumnDescription( sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                            getAccessibleRowHeaders() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleTable > SAL_CALL
                            getAccessibleColumnHeaders() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleRows()
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int32 > SAL_CALL getSelectedAccessibleColumns()
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleRowSelected( sal_Int32 nRow )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleColumnSelected( sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleCaption() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleSummary() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleRow( sal_Int32 nChildIndex )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleColumn( sal_Int32 nChildIndex )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;

    //=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const css::awt::Point& aPoint )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   grabFocus() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (css::lang::IndexOutOfBoundsException,
                                       css::uno::RuntimeException,
                                       std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    //=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

protected:
    virtual OUString SAL_CALL createAccessibleDescription() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL createAccessibleName()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;

    virtual Rectangle GetBoundingBoxOnScreen() const throw(css::uno::RuntimeException, std::exception) override;
    virtual Rectangle GetBoundingBox() const throw (css::uno::RuntimeException, std::exception) override;

private:
    ScPreviewShell*     mpViewShell;
    sal_Int32           mnIndex;
    mutable ScPreviewTableInfo* mpTableInfo;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void    FillTableInfo() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
