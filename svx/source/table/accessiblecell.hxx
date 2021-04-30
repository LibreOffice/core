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

#include <editeng/AccessibleContextBase.hxx>
#include <editeng/AccessibleComponentBase.hxx>
#include <svx/IAccessibleViewForwarderListener.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>
#include <AccessibleTableShape.hxx>

#include <cppuhelper/implbase.hxx>

#include <celltypes.hxx>


namespace accessibility
{

class AccessibleShapeTreeInfo;

typedef ::cppu::ImplInheritanceHelper< AccessibleContextBase, css::accessibility::XAccessibleExtendedComponent > AccessibleCellBase;

class AccessibleCell : public AccessibleCellBase
                     , public AccessibleComponentBase
                     , public IAccessibleViewForwarderListener
{
public:
    AccessibleCell( const css::uno::Reference< css::accessibility::XAccessible>& rxParent, const sdr::table::CellRef& rCell, sal_Int32 nIndex, const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleCell() override;
    AccessibleCell(const AccessibleCell&) = delete;
    AccessibleCell& operator=(const AccessibleCell&) = delete;

    void Init();

    virtual bool SetState (sal_Int16 aState) override;
    virtual bool ResetState (sal_Int16 aState) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) noexcept override;
    virtual void SAL_CALL release(  ) noexcept override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleChild(sal_Int32 nIndex) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet() override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    const sdr::table::CellRef& getCellRef() const { return mxCell;}
    void UpdateChildren();
    static OUString getCellName( sal_Int32 nCol, sal_Int32 nRow );

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& aPoint) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(const css::awt::Point& aPoint) override;
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::awt::Point SAL_CALL getLocation() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL grabFocus() override;
    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont() override;
    virtual OUString SAL_CALL getTitledBorderText() override;
    virtual OUString SAL_CALL getToolTipText() override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& rxListener) override;
    virtual void SAL_CALL removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& rxListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

    // IAccessibleViewForwarderListener
    virtual void ViewForwarderChanged() override;

    // Misc

    /** set the index _nIndex at the accessible cell param  _nIndex The new index in parent.
    */
    void setIndexInParent(sal_Int32 _nIndex) { mnIndexInParent = _nIndex; }

    //Get the parent table
    AccessibleTableShape* GetParentTable() { return pAccTable; }
protected:
    /// Bundle of information passed to all shapes in a document tree.
    AccessibleShapeTreeInfo maShapeTreeInfo;

    /// the index in parent.
    sal_Int32 mnIndexInParent;

    /// The accessible text engine.  May be NULL if it can not be created.
    std::unique_ptr<AccessibleTextHelper> mpText;

    sdr::table::CellRef mxCell;

    /// This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing() override;

    AccessibleTableShape *pAccTable;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
