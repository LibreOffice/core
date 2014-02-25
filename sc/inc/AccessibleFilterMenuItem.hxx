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

#ifndef SC_ACCESSIBLEFILTERMENUITEM_HXX
#define SC_ACCESSIBLEFILTERMENUITEM_HXX

#include "AccessibleContextBase.hxx"
#include "cppuhelper/implbase1.hxx"

#include <com/sun/star/accessibility/XAccessibleAction.hpp>

class ScMenuFloatingWindow;

typedef ::cppu::ImplHelper1<
    ::com::sun::star::accessibility::XAccessibleAction > ScAccessibleFilterMenuItem_BASE;

class ScAccessibleFilterMenuItem :
    public ScAccessibleContextBase,
    public ScAccessibleFilterMenuItem_BASE
{
public:
    explicit ScAccessibleFilterMenuItem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos);

    virtual ~ScAccessibleFilterMenuItem();

    /// XAccessibleContext

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
            throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /// XAccessibleAction

    virtual ::sal_Int32 SAL_CALL getAccessibleActionCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::sal_Bool SAL_CALL doAccessibleAction(sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);

    virtual OUString SAL_CALL getAccessibleActionDescription(sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL
        getAccessibleActionKeyBinding(sal_Int32 nIndex)
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception);

    /// XInterface

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
            throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    /// Non-UNO Methods

    void setEnabled(bool bEnabled);

protected:

    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    virtual Rectangle GetBoundingBox() const
        throw (::com::sun::star::uno::RuntimeException, std::exception);

private:
    bool isSelected() const;
    bool isFocused() const;
    void updateStateSet();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

    ScMenuFloatingWindow* mpWindow;
    OUString maName;
    size_t mnMenuPos;
    bool mbEnabled;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
