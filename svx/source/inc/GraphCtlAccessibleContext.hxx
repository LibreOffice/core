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
                ::com::sun::star::accessibility::XAccessible,
                ::com::sun::star::accessibility::XAccessibleComponent,
                ::com::sun::star::accessibility::XAccessibleContext,
                ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
                ::com::sun::star::accessibility::XAccessibleSelection,
                ::com::sun::star::lang::XServiceInfo,
                ::com::sun::star::lang::XServiceName >
                SvxGraphCtrlAccessibleContext_Base;

class SvxGraphCtrlAccessibleContext:
    comphelper::OBaseMutex, public SvxGraphCtrlAccessibleContext_Base,
    SfxListener, accessibility::IAccessibleViewForwarder
{
public:
    friend class GraphCtrl;

    // internal
    SvxGraphCtrlAccessibleContext(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent,
        GraphCtrl&              rRepresentation,
        const OUString*  pName = NULL,
        const OUString*  pDescription = NULL );

    void Notify( SfxBroadcaster& aBC, const SfxHint& aHint ) SAL_OVERRIDE;

protected:
    virtual ~SvxGraphCtrlAccessibleContext();
public:
    // XAccessible
    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleComponent
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& rPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL grabFocus() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getBackground()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL getAccessibleChild (sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL getAccessibleParent() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL getAccessibleRole() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleDescription() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL getAccessibleRelationSet() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception) SAL_OVERRIDE;
//  virtual void SAL_CALL addPropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) {}
//  virtual void SAL_CALL removePropertyChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException) {}

    // XAccessibleEventBroadcaster
    virtual void SAL_CALL addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener>& xListener) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeAccessibleEventListener(  const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener>& xListener) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService (const OUString& sServiceName) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceName
    virtual OUString SAL_CALL getServiceName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL clearAccessibleSelection() throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL selectAllAccessibleChildren() throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // IAccessibleViewforwarder
    virtual Rectangle GetVisibleArea() const SAL_OVERRIDE;
    virtual Point LogicToPixel (const Point& rPoint) const SAL_OVERRIDE;
    virtual Size LogicToPixel (const Size& rSize) const SAL_OVERRIDE;

protected:
    void checkChildIndexOnSelection( long nIndexOfChild ) throw (::com::sun::star::lang::IndexOutOfBoundsException );

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
     Rectangle GetBoundingBoxOnScreen() throw (::com::sun::star::uno::RuntimeException);

    /// Return the object's current bounding box relative to the parent object.
    Rectangle GetBoundingBox() throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

private:
    SdrObject* getSdrObject( sal_Int32 nIndex )
        throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException );

    void CommitChange (sal_Int16 aEventId, const ::com::sun::star::uno::Any& rNewValue, const ::com::sun::star::uno::Any& rOldValue);
    void FireEvent (const ::com::sun::star::accessibility::AccessibleEventObject& aEvent);

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessible( const SdrObject* pObj );

    accessibility::AccessibleShapeTreeInfo maTreeInfo;

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference<
         ::com::sun::star::accessibility::XAccessible> mxParent;

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
