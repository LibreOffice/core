/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
            ::com::sun::star::accessibility::XAccessible>& rxParent, ScMenuFloatingWindow* pWin, const ::rtl::OUString& rName, size_t nMenuPos);

    virtual ~ScAccessibleFilterMenuItem();

    // XAccessibleContext

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
            throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleAction

    virtual ::sal_Int32 SAL_CALL getAccessibleActionCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL doAccessibleAction(sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription(sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL
        getAccessibleActionKeyBinding(sal_Int32 nIndex)
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    // XInterface

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // Non-UNO Methods

    void setEnabled(bool bEnabled);

protected:

    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (::com::sun::star::uno::RuntimeException);

    virtual Rectangle GetBoundingBox() const
        throw (::com::sun::star::uno::RuntimeException);

private:
    bool isSelected() const;
    bool isFocused() const;
    void updateStateSet();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

    ScMenuFloatingWindow* mpWindow;
    ::rtl::OUString maName;
    size_t mnMenuPos;
    bool mbEnabled;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
