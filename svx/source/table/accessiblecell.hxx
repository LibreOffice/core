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

typedef ::cppu::ImplInheritanceHelper< AccessibleContextBase, ::com::sun::star::accessibility::XAccessibleExtendedComponent > AccessibleCellBase;

class AccessibleCell : boost::noncopyable, public AccessibleCellBase, public AccessibleComponentBase, public IAccessibleViewForwarderListener
{
public:
    AccessibleCell( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent, const sdr::table::CellRef& rCell, sal_Int32 nIndex, const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleCell();

    void Init();

    bool operator== (const AccessibleCell& rAccessibleCell);

    virtual bool SetState (sal_Int16 aState) override;
    virtual bool ResetState (sal_Int16 aState) override;

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw () override;
    virtual void SAL_CALL release(  ) throw () override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL getAccessibleChild(sal_Int32 nIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    sdr::table::CellRef getCellRef() { return mxCell;}
    void UpdateChildren();
    static OUString getCellName( sal_Int32 nCol, sal_Int32 nRow );

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(const ::com::sun::star::awt::Point& aPoint) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addFocusListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener) throw (::com::sun::star::uno::RuntimeException) override;
    virtual void SAL_CALL removeFocusListener (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) override;
    virtual void SAL_CALL grabFocus() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getForeground() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleExtendedComponent
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitledBorderText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTipText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& rxListener) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

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
