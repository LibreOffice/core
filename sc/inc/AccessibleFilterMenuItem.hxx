/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessibleDataPilotControl.hxx,v $
 * $Revision: 1.6 $
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
#include "cppuhelper/implbase2.hxx"

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>

class ScMenuFloatingWindow;

typedef ::cppu::ImplHelper2<
    ::com::sun::star::accessibility::XAccessibleAction,
    ::com::sun::star::accessibility::XAccessibleStateSet > ScAccessibleFilterMenuItem_BASE;

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

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
            throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleStateSet

    virtual ::sal_Bool SAL_CALL isEmpty() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL contains(sal_Int16 nState) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL containsAll(const ::com::sun::star::uno::Sequence< ::sal_Int16 >& aStateSet) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getStates() throw (::com::sun::star::uno::RuntimeException);

    // XAccessibleAction

    virtual ::sal_Int32 SAL_CALL getAccessibleActionCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL doAccessibleAction( ::sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleActionDescription( ::sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( ::sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // Non-UNO Methods

    bool isSelected() const;

    void setEnabled(bool bEnabled);

private:
    ScMenuFloatingWindow* mpWindow;
    ::rtl::OUString maName;
    size_t mnMenuPos;
    bool mbSelected:1;
    bool mbEnabled:1;
};

#endif
