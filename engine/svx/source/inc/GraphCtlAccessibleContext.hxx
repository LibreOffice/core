/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <comphelper/OAccessible.hxx>
#include <cppuhelper/basemutex.hxx>
#include <svl/lstner.hxx>

#include <map>

#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/AccessibleShape.hxx>

class GraphCtrl;
class SdrObject;
class SdrModel;
class SdrPage;
class SdrView;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

class SvxGraphCtrlAccessibleContext final
    : public cppu::ImplInheritanceHelper<comphelper::OAccessible,
                                         css::accessibility::XAccessibleSelection,
                                         css::lang::XServiceInfo, css::lang::XServiceName>,
      public SfxListener,
      public ::accessibility::IAccessibleViewForwarder
{
public:
    friend class GraphCtrl;

    // internal
    SvxGraphCtrlAccessibleContext(GraphCtrl& rRepresentation);

    void Notify( SfxBroadcaster& aBC, const SfxHint& aHint ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > getAccessibleAtPoint( const css::awt::Point& rPoint ) override;
    virtual void grabFocus() override;

    virtual sal_Int32 getForeground() override;

    virtual sal_Int32 getBackground() override;

    // XAccessibleContext
    virtual sal_Int64 getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible> getAccessibleChild (sal_Int64 nIndex) override;
    virtual css::uno::Reference< css::accessibility::XAccessible> getAccessibleParent() override;
    virtual sal_Int16 getAccessibleRole() override;
    virtual OUString getAccessibleDescription() override;
    virtual OUString getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> getAccessibleRelationSet() override;
    virtual sal_Int64 getAccessibleStateSet() override;
    virtual css::lang::Locale getLocale() override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService (const OUString& sServiceName) override;
    virtual cpo::uno::Sequence< OUString> getSupportedServiceNames() override;

    // XServiceName
    virtual OUString getServiceName() override;

    // XAccessibleSelection
    virtual void selectAccessibleChild( sal_Int64 nChildIndex ) override;
    virtual bool isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
    virtual void clearAccessibleSelection() override;
    virtual void selectAllAccessibleChildren() override;
    virtual sal_Int64 getSelectedAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
    virtual void deselectAccessibleChild( sal_Int64 nSelectedChildIndex ) override;

    // IAccessibleViewforwarder
    virtual tools::Rectangle GetVisibleArea() const override;
    virtual Point LogicToPixel (const Point& rPoint) const override;
    virtual Size LogicToPixel (const Size& rSize) const override;

    /** This method is used by the graph control to tell the
        accessibility object about a new model and view.
    */
    void setModelAndView (SdrModel* pModel, SdrView* pView);

protected:
    virtual css::awt::Rectangle implGetBounds() override;

private:
    /// @throws css::lang::IndexOutOfBoundsException
    void checkChildIndexOnSelection(sal_Int64 nIndexOfChild );

    virtual void disposing() final override;

    /// @throws cpo::uno::RuntimeException
    /// @throws css::lang::IndexOutOfBoundsException
    SdrObject* getSdrObject( sal_Int64 nIndex );

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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
