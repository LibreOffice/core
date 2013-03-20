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

#ifndef _ACCHYPERLINK_HXX
#define _ACCHYPERLINK_HXX

#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase1.hxx>
#include <fmtinfmt.hxx>

class SwAccessibleParagraph;
class SwTxtAttr;

class SwAccessibleHyperlink :
        public ::cppu::WeakImplHelper1<
        ::com::sun::star::accessibility::XAccessibleHyperlink >
{
    friend class SwAccessibleParagraph;
    friend class SwAccessibleHyperTextData;
    sal_uInt16 nHintPos;
    ::rtl::Reference< SwAccessibleParagraph > xPara;
    sal_Int32 nStartIdx;
    sal_Int32 nEndIdx;

    SwAccessibleHyperlink( sal_uInt16 nHintPos,
                           SwAccessibleParagraph *p,
                              sal_Int32 nStt, sal_Int32 nEnd    );

    const SwTxtAttr *GetTxtAttr() const;
    void Invalidate();

public:
    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleActionDescription(
                sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL
               getAccessibleActionKeyBinding( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);

    // XAccessibleHyperlink
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionAnchor(
                sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleActionObject(
            sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getStartIndex()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getEndIndex()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isValid(  )
        throw (::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
