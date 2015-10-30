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

#ifndef INCLUDED_SVX_SOURCE_TABLE_ACCESSIBLECELL_HXX
#define INCLUDED_SVX_SOURCE_TABLE_ACCESSIBLECELL_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/drawing/XShape.hpp>

#include <rtl/ref.hxx>

#include <editeng/AccessibleContextBase.hxx>
#include <editeng/AccessibleComponentBase.hxx>
#include <svx/IAccessibleViewForwarderListener.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>
#include <AccessibleTableShape.hxx>

#include <cppuhelper/implbase.hxx>

#include "cell.hxx"

#include <boost/noncopyable.hpp>


namespace accessibility
{

class AccessibleShapeTreeInfo;

typedef ::cppu::ImplInheritanceHelper< AccessibleContextBase, css::accessibility::XAccessibleExtendedComponent > AccessibleCellBase;

class AccessibleCell : boost::noncopyable, public AccessibleCellBase, public AccessibleComponentBase, public IAccessibleViewForwarderListener
{
public:
    AccessibleCell( const css::uno::Reference< css::accessibility::XAccessible>& rxParent, const sdr::table::CellRef& rCell, sal_Int32 nIndex, const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleCell();

    void Init();

    bool operator== (const AccessibleCell& rAccessibleCell);

    virtual bool SetState (sal_Int16 aState) override;
    virtual bool ResetState (sal_Int16 aState) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw () override;
    virtual void SAL_CALL release(  ) throw () override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleChild(sal_Int32 nIndex) throw(css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName() throw (css::uno::RuntimeException, std::exception) override;
    sdr::table::CellRef getCellRef() { return mxCell;}
    void UpdateChildren();
    static OUString getCellName( sal_Int32 nCol, sal_Int32 nRow );

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(const css::awt::Point& aPoint) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addFocusListener ( const css::uno::Reference< css::awt::XFocusListener >& xListener) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL removeFocusListener (const css::uno::Reference< css::awt::XFocusListener >& xListener ) throw (css::uno::RuntimeException) override;
    virtual void SAL_CALL grabFocus() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground() throw(css::uno::RuntimeException, std::exception) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& rxListener) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& rxListener) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // IAccessibleViewForwarderListener
    virtual void ViewForwarderChanged (ChangeType aChangeType, const IAccessibleViewForwarder* pViewForwarder) override;

    // Misc

    /** set the index _nIndex at the accessible cell param  _nIndex The new index in parent.
    */
    inline void setIndexInParent(sal_Int32 _nIndex) { mnIndexInParent = _nIndex; }

    //Get the parent table
    AccessibleTableShape* GetParentTable() { return pAccTable; }
protected:
    /// Bundle of information passed to all shapes in a document tree.
    AccessibleShapeTreeInfo maShapeTreeInfo;

    /// the index in parent.
    sal_Int32 mnIndexInParent;

    /// The accessible text engine.  May be NULL if it can not be created.
    AccessibleTextHelper* mpText;

    sdr::table::CellRef mxCell;

    /// This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing() override;

    AccessibleTableShape *pAccTable;

private:
    AccessibleCell(const AccessibleCell&) = delete;
    AccessibleCell& operator=(const AccessibleCell&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
