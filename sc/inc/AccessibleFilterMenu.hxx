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

#ifndef INCLUDED_SC_INC_ACCESSIBLEFILTERMENU_HXX
#define INCLUDED_SC_INC_ACCESSIBLEFILTERMENU_HXX

#include "AccessibleContextBase.hxx"
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/TextSegment.hpp>
#include <vcl/vclptr.hxx>

#include <vector>

namespace com { namespace sun { namespace star {
    namespace accessibility {
        struct AccessibleEventObject;
    }
}}}

class ScMenuFloatingWindow;

typedef ::cppu::ImplHelper1<
        ::com::sun::star::accessibility::XAccessibleSelection > ScAccessibleFilterMenu_BASE;

class ScAccessibleFilterMenu :
    public ScAccessibleContextBase,
    public ScAccessibleFilterMenu_BASE
{
public:
    explicit ScAccessibleFilterMenu(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
            ScMenuFloatingWindow* pWin, const OUString& rName, size_t nMenuPos);
    virtual ~ScAccessibleFilterMenu();

    virtual bool SAL_CALL isVisible()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// XAccessibleComponent

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
        SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// XAccessibleContext

    virtual OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild(sal_Int32 nIndex)
            throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) override;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// XAccessibleEventBroadcaster
    virtual void SAL_CALL
        addAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    ///  Remove an existing event listener.
    virtual void SAL_CALL
        removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

    /// XAccessibleSelection

    virtual void SAL_CALL selectAccessibleChild(sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isAccessibleChildSelected(sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL clearAccessibleSelection()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL selectAllAccessibleChildren()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::sal_Int32 SAL_CALL getSelectedAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getSelectedAccessibleChild(sal_Int32 nChildIndex)
            throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL deselectAccessibleChild(sal_Int32 nChildIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    /// XInterface

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
        ::com::sun::star::uno::Type const & rType )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    /// XTypeProvider

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// non-UNO methods

    void appendMenuItem(const OUString& rName, bool bEnabled, size_t nMenuPos);
    void setMenuPos(size_t nMenuPos);
    void setEnabled(bool bEnabled);

protected:

    sal_Int32 getMenuItemCount() const;

    virtual Rectangle GetBoundingBoxOnScreen() const
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual Rectangle GetBoundingBox() const
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    bool isSelected() const;
    bool isFocused() const;

    void updateStates();

private:
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > > maMenuItems;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > mxStateSet;

    size_t mnMenuPos;
    VclPtr<ScMenuFloatingWindow> mpWindow;

    bool mbEnabled:1;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
