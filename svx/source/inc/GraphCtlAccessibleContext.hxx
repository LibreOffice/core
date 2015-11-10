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

#include <cppuhelper/compbase7.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <osl/mutex.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <svl/lstner.hxx>

#include <set>
#include <map>

#include <comphelper/servicehelper.hxx>
#include <svx/rectenum.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/AccessibleShape.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } }

class Rectangle;
class GraphCtrl;
class SdrObject;
class SdrModel;
class SdrPage;
class SdrView;

/** @descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef ::cppu::WeakAggComponentImplHelper7<
                css::accessibility::XAccessible,
                css::accessibility::XAccessibleComponent,
                css::accessibility::XAccessibleContext,
                css::accessibility::XAccessibleEventBroadcaster,
                css::accessibility::XAccessibleSelection,
                css::lang::XServiceInfo,
                css::lang::XServiceName >
                SvxGraphCtrlAccessibleContext_Base;

class SvxGraphCtrlAccessibleContext:
    comphelper::OBaseMutex, public SvxGraphCtrlAccessibleContext_Base,
    SfxListener, accessibility::IAccessibleViewForwarder
{
public:
    friend class GraphCtrl;

    // internal
    SvxGraphCtrlAccessibleContext(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent,
        GraphCtrl&              rRepresentation,
        const OUString*  pName = nullptr,
        const OUString*  pDescription = nullptr );

    void Notify( SfxBroadcaster& aBC, const SfxHint& aHint ) override;

protected:
    virtual ~SvxGraphCtrlAccessibleContext();
public:
    // XAccessible
    /// Return the XAccessibleContext.
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& rPoint ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint )    throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Rectangle SAL_CALL getBounds() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocation() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::awt::Size SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL grabFocus() throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground()
        throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleChild (sal_Int32 nIndex) throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleParent() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleDescription() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getAccessibleName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet> SAL_CALL getAccessibleRelationSet() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLocale() throw (css::uno::RuntimeException, css::accessibility::IllegalAccessibleComponentStateException, std::exception) override;
//  virtual void SAL_CALL addPropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::uno::RuntimeException) {}
//  virtual void SAL_CALL removePropertyChangeListener( const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::uno::RuntimeException) {}

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener>& xListener) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeAccessibleEventListener(  const css::uno::Reference< css::accessibility::XAccessibleEventListener>& xListener) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() throw (css::uno::RuntimeException, std::exception) override;

    // XServiceName
    virtual OUString SAL_CALL getServiceName() throw (css::uno::RuntimeException, std::exception) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL clearAccessibleSelection() throw (css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL selectAllAccessibleChildren() throw (css::uno::RuntimeException, std::exception ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount() throw (css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception ) override;

    // IAccessibleViewforwarder
    virtual Rectangle GetVisibleArea() const override;
    virtual Point LogicToPixel (const Point& rPoint) const override;
    virtual Size LogicToPixel (const Size& rSize) const override;

protected:
    void checkChildIndexOnSelection( long nIndexOfChild ) throw (css::lang::IndexOutOfBoundsException );

public:

    /** This method is used by the graph control to tell the
        accessibility object about a new model and view.
    */
    void setModelAndView (SdrModel* pModel, SdrView* pView);

protected:

    /** Return the object's current bounding box relative to the desktop,
        i.e in absolute pixel coordinates.
        @return
            The returned rectangle is a bounding box of the object given in
            absolute screen coordinates.
        @raise DisposedException
            When the object is already disposed then a
            <type>DisposedException</type> is thrown.
    */
     Rectangle GetBoundingBoxOnScreen() throw (css::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    Rectangle GetBoundingBox() throw (css::uno::RuntimeException);

    virtual void SAL_CALL disposing() override;

private:
    SdrObject* getSdrObject( sal_Int32 nIndex )
        throw( css::uno::RuntimeException, css::lang::IndexOutOfBoundsException );

    void CommitChange (sal_Int16 aEventId, const css::uno::Any& rNewValue, const css::uno::Any& rOldValue);
    void FireEvent (const css::accessibility::AccessibleEventObject& aEvent);

    css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessible( const SdrObject* pObj );

    accessibility::AccessibleShapeTreeInfo maTreeInfo;

    /// Reference to the parent object.
    css::uno::Reference<css::accessibility::XAccessible> mxParent;

    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    OUString msDescription;

    /** Name of this object.
    */
    OUString msName;

    /// map of accessible shapes
    struct SdrObjectCompareLess
    {
        bool operator()(const SdrObject* p1, const SdrObject* p2) const
        {
            return p1 < p2;
        }
    };
    typedef ::std::map< const SdrObject*, rtl::Reference<accessibility::AccessibleShape>, SdrObjectCompareLess > ShapesMapType;
    ShapesMapType mxShapes;

    VclPtr<GraphCtrl>  mpControl;

    SdrModel* mpModel;
    SdrPage* mpPage;
    SdrView* mpView;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

    bool mbDisposed;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
