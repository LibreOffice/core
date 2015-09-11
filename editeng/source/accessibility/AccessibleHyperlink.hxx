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

#ifndef INCLUDED_EDITENG_SOURCE_ACCESSIBILITY_ACCESSIBLEHYPERLINK_HXX
#define INCLUDED_EDITENG_SOURCE_ACCESSIBILITY_ACCESSIBLEHYPERLINK_HXX

#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessibleHyperlink.hpp>

class SvxFieldItem;
class SvxAccessibleTextAdapter;

namespace accessibility
{

    class AccessibleHyperlink : public ::cppu::WeakImplHelper< css::accessibility::XAccessibleHyperlink >
    {
    private:

        SvxAccessibleTextAdapter& rTA;
        SvxFieldItem* pFld;
        sal_Int32 nPara;  // EE values
        sal_uInt16 nRealIdx;  // EE values
        sal_Int32 nStartIdx, nEndIdx;   // translated values
        OUString aDescription;

    public:
        AccessibleHyperlink( SvxAccessibleTextAdapter& r, SvxFieldItem* p, sal_Int32 nP, sal_uInt16 nR, sal_Int32 nStt, sal_Int32 nEnd, const OUString& rD );
        virtual ~AccessibleHyperlink();

        // XAccessibleAction
        virtual sal_Int32 SAL_CALL getAccessibleActionCount() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL doAccessibleAction( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getAccessibleActionDescription( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XAccessibleHyperlink
        virtual css::uno::Any SAL_CALL getAccessibleActionAnchor( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Any SAL_CALL getAccessibleActionObject( sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getStartIndex() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getEndIndex() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isValid() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
