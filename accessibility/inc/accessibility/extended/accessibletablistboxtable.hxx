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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETABLISTBOXTABLE_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETABLISTBOXTABLE_HXX

#include "AccessibleBrowseBoxTable.hxx"
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SvHeaderTabListBox;



namespace accessibility {

typedef ::cppu::ImplHelper< css::accessibility::XAccessibleSelection >
            AccessibleTabListBoxTableImplHelper;

class AccessibleTabListBoxTable : public AccessibleBrowseBoxTable, public AccessibleTabListBoxTableImplHelper
{
private:
    VclPtr<SvHeaderTabListBox>     m_pTabListBox;

    css::uno::Reference< css::accessibility::XAccessible >  m_xCurChild;

    void                    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    DECL_LINK_TYPED(        WindowEventListener, VclWindowEvent&, void );

    // helpers ----------------------------------------------------------------

    /** Throws an exception, if nIndex is not a valid child index. */
    void ensureValidIndex( sal_Int32 _nIndex ) const;

    /** Returns true, if the specified row is selected. */
    bool implIsRowSelected( sal_Int32 _nRow ) const;
    /** Selects the specified row. */
    void implSelectRow( sal_Int32 _nRow, bool _bSelect );

    /** Returns the count of rows in the table. */
    sal_Int32 implGetRowCount() const override;
    /** Returns the total column count in the table. */
    sal_Int32 implGetColumnCount() const override;
    /** Returns the count of selected rows in the table. */
    sal_Int32 implGetSelRowCount() const;
    /** Returns the total cell count in the table (including header). */
    inline sal_Int32 implGetCellCount() const { return implGetRowCount() * implGetColumnCount(); }

    /** Returns the row index from cell index. */
    inline sal_Int32 implGetRow( sal_Int32 _nIndex ) const { return _nIndex / implGetColumnCount(); }
    /** Returns the absolute row index of the nSelRow-th selected row. */
    sal_Int32 implGetSelRow( sal_Int32 _nSelRow ) const;

public:
    /** ctor()
        @param rxParent  XAccessible interface of the parent object.
        @param rBox  The HeaderTabListBox control. */
    AccessibleTabListBoxTable(
        const css::uno::Reference< css::accessibility::XAccessible >& rxParent,
        SvHeaderTabListBox& rBox );

protected:
    /** dtor() */
    virtual ~AccessibleTabListBoxTable();

public:
    // XInterface
    DECLARE_XINTERFACE( )

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection
    void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL clearAccessibleSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL selectAllAccessibleChildren(  ) throw (css::uno::RuntimeException, std::exception) override;
    sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
};



} // namespace accessibility



#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETABLISTBOXTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
