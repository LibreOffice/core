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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPREVIEWHEADERCELL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPREVIEWHEADERCELL_HXX

#include "AccessibleContextBase.hxx"
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <tools/gen.hxx>
#include "global.hxx"
#include "address.hxx"
#include <cppuhelper/implbase.hxx>

class ScPreviewShell;
class ScPreviewTableInfo;
namespace accessibility {
    class AccessibleTextHelper;
}

typedef cppu::ImplHelper < css::accessibility::XAccessibleValue >
                    ScAccessiblePreviewHeaderCellImpl;

class ScAccessiblePreviewHeaderCell :
        public ScAccessibleContextBase,
        public ScAccessiblePreviewHeaderCellImpl
{
public:
    ScAccessiblePreviewHeaderCell( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell,
                            const ScAddress& rCellPos, bool bIsColHdr, bool bIsRowHdr,
                            sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewHeaderCell() override;

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

    //=====  XAccessibleValue  ================================================

    virtual css::uno::Any SAL_CALL getCurrentValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setCurrentValue( const css::uno::Any& aNumber )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getMaximumValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getMinimumValue() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const css::awt::Point& aPoint )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   grabFocus() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (css::lang::IndexOutOfBoundsException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
        getTypes()
        throw (css::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    virtual OUString SAL_CALL createAccessibleDescription() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL createAccessibleName() throw (css::uno::RuntimeException, std::exception) override;

    virtual Rectangle GetBoundingBoxOnScreen() const throw(css::uno::RuntimeException, std::exception) override;
    virtual Rectangle GetBoundingBox() const throw (css::uno::RuntimeException, std::exception) override;

private:
    ScPreviewShell*     mpViewShell;
    accessibility::AccessibleTextHelper* mpTextHelper;
    sal_Int32           mnIndex;
    ScAddress           maCellPos;
    bool                mbColumnHeader;
    bool                mbRowHeader;
    mutable ScPreviewTableInfo* mpTableInfo;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();
    void    FillTableInfo() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
