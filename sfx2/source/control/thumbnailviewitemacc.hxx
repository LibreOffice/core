/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>

class ThumbnailViewItem;
class ThumbnailViewItemAcc
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible>
{
private:
    ThumbnailViewItem*                                                  mpThumbnailViewItem;

public:
    ThumbnailViewItemAcc(ThumbnailViewItem* pThumbnailViewItem);
    virtual ~ThumbnailViewItemAcc() override;

    void ThumbnailViewItemDestroyed();

public:

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds() override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
