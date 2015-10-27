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

#ifndef INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPREVIEWCELL_HXX
#define INCLUDED_SC_SOURCE_UI_INC_ACCESSIBLEPREVIEWCELL_HXX

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
        const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
        ScPreviewShell* pViewShell, /* const */ ScAddress& rCellAddress, sal_Int32 nIndex );

protected:
    virtual ~ScAccessiblePreviewCell();

    using ScAccessibleCellBase::IsDefunc;

public:
    using ScAccessibleCellBase::disposing;
    virtual void SAL_CALL disposing() override;

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    //=====  XAccessibleComponent  ============================================

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleAtPoint( const css::awt::Point& aPoint )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   grabFocus() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    // override to calculate this on demand
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
                            getAccessibleChild( sal_Int32 i )
                                throw (css::lang::IndexOutOfBoundsException,
                                    css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
                            getAccessibleStateSet() throw (css::uno::RuntimeException, std::exception) override;

    //=====  XServiceInfo  ====================================================

    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    ///=====  XTypeProvider  ===================================================

    /** Returns a implementation id.
    */
    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    virtual Rectangle GetBoundingBoxOnScreen() const throw(css::uno::RuntimeException, std::exception) override;
    virtual Rectangle GetBoundingBox() const throw (css::uno::RuntimeException, std::exception) override;

private:
    ScPreviewShell* mpViewShell;

    accessibility::AccessibleTextHelper* mpTextHelper;

    bool IsDefunc(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);
    virtual bool IsEditable(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates) override;
    bool IsOpaque(
        const css::uno::Reference<css::accessibility::XAccessibleStateSet>& rxParentStates);

    void CreateTextHelper();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
