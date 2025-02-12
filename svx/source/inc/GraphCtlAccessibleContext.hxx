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


#ifndef INCLUDED_SVX_SOURCE_INC_GRAPHCTLACCESSIBLECONTEXT_HXX
#define INCLUDED_SVX_SOURCE_INC_GRAPHCTLACCESSIBLECONTEXT_HXX

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <cppuhelper/basemutex.hxx>
#include <svl/lstner.hxx>

#include <map>

#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/AccessibleShape.hxx>

namespace com::sun::star::awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
}
namespace tools { class Rectangle; }
class GraphCtrl;
class SdrObject;
class SdrModel;
class SdrPage;
class SdrView;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef ::cppu::WeakComponentImplHelper<
                css::accessibility::XAccessible,
                css::accessibility::XAccessibleComponent,
                css::accessibility::XAccessibleContext,
                css::accessibility::XAccessibleEventBroadcaster,
                css::accessibility::XAccessibleSelection,
                css::lang::XServiceInfo,
                css::lang::XServiceName >
                SvxGraphCtrlAccessibleContext_Base;

class SvxGraphCtrlAccessibleContext final :
    private cppu::BaseMutex, public SvxGraphCtrlAccessibleContext_Base,
    public SfxListener, public ::accessibility::IAccessibleViewForwarder
{
public:
    friend class GraphCtrl;

    // internal
    SvxGraphCtrlAccessibleContext(GraphCtrl& rRepresentation);

    void Notify( SfxBroadcaster& aBC, const SfxHint& aHint ) override;

    // XAccessible
    /// Return the XAccessibleContext.
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& rPoint ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint ) override;
    virtual css::awt::Rectangle SAL_CALL getBounds() override;
    virtual css::awt::Point SAL_CALL getLocation() override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;
    virtual css::awt::Size SAL_CALL getSize() override;
    virtual void SAL_CALL grabFocus() override;

    virtual sal_Int32 SAL_CALL getForeground() override;

    virtual sal_Int32 SAL_CALL getBackground() override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleChild (sal_Int64 nIndex) override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleParent() override;
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL getAccessibleRelationSet() override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;
    virtual css::lang::Locale SAL_CALL getLocale() override;

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener>& xListener) override;
    virtual void SAL_CALL removeAccessibleEventListener(  const css::uno::Reference< css::accessibility::XAccessibleEventListener>& xListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override;

    // XServiceName
    virtual OUString SAL_CALL getServiceName() override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int64 nChildIndex ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection() override;
    virtual void SAL_CALL selectAllAccessibleChildren() override;
    virtual sal_Int64 SAL_CALL getSelectedAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    // IAccessibleViewforwarder
    virtual tools::Rectangle GetVisibleArea() const override;
    virtual Point LogicToPixel (const Point& rPoint) const override;
    virtual Size LogicToPixel (const Size& rSize) const override;

    /** This method is used by the graph control to tell the
        accessibility object about a new model and view.
    */
    void setModelAndView (SdrModel* pModel, SdrView* pView);

private:
    virtual ~SvxGraphCtrlAccessibleContext() override;
    /// @throws css::lang::IndexOutOfBoundsException
    void checkChildIndexOnSelection(sal_Int64 nIndexOfChild );

    virtual void SAL_CALL disposing() final override;

    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IndexOutOfBoundsException
    SdrObject* getSdrObject( sal_Int64 nIndex );

    void CommitChange (sal_Int16 aEventId, const css::uno::Any& rNewValue, const css::uno::Any& rOldValue);

    css::uno::Reference< css::accessibility::XAccessible > getAccessible( const SdrObject* pObj );

    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    OUString msDescription;

    /** Name of this object.
    */
    OUString msName;

    /// map of accessible shapes
    typedef ::std::map< const SdrObject*, rtl::Reference<::accessibility::AccessibleShape> > ShapesMapType;
    ShapesMapType mxShapes;

    GraphCtrl*  mpControl;

    SdrPage* mpPage;
    SdrView* mpView;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    bool mbDisposed;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
