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
#ifndef INCLUDED_SVTOOLS_ACCESSIBLERULER_HXX
#define INCLUDED_SVTOOLS_ACCESSIBLERULER_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <cppuhelper/implbase5.hxx>
#include <comphelper/servicehelper.hxx>
#include <vcl/vclptr.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } }

class Rectangle;
class Ruler;
namespace vcl { class Window; }


typedef ::cppu::WeakAggComponentImplHelper5<
            ::com::sun::star::accessibility::XAccessible,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::lang::XServiceInfo >
            SvtRulerAccessible_Base;

class SvtRulerAccessible : public ::comphelper::OBaseMutex, public SvtRulerAccessible_Base
{
public:
    //=====  internal  ========================================================
    SvtRulerAccessible(
        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent, Ruler& rRepresentation, const ::rtl::OUString& rName );
protected:
    virtual ~SvtRulerAccessible();
public:

    bool SAL_CALL
        isVisible() throw( ::com::sun::star::uno::RuntimeException );

    //=====  XAccessible  =====================================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL
        containsPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::awt::Rectangle SAL_CALL
        getBounds() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::awt::Point SAL_CALL
        getLocation() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::awt::Point SAL_CALL
        getLocationOnScreen() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::awt::Size SAL_CALL
        getSize() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        grabFocus() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int32 nIndex )
            throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::rtl::OUString SAL_CALL
        getAccessibleName() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
            throw(  ::com::sun::star::uno::RuntimeException,
                    ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception ) override;
    //=====  XAccessibleEventBroadcaster  =====================================

    virtual void SAL_CALL
        addAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener )throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleEventListener >& xListener ) throw( com::sun::star::uno::RuntimeException, std::exception ) override;

    //=====  XServiceInfo  ====================================================

    virtual ::rtl::OUString SAL_CALL
        getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

protected:

    /// @Return the object's current bounding box relative to the desktop.
    Rectangle GetBoundingBoxOnScreen() throw( ::com::sun::star::uno::RuntimeException );

    /// @Return the object's current bounding box relative to the parent object.
    Rectangle GetBoundingBox() throw( ::com::sun::star::uno::RuntimeException );


    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive() const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive() throw( ::com::sun::star::lang::DisposedException );

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    ::rtl::OUString                     msDescription;

    /** Name of this object.
    */
    ::rtl::OUString                     msName;

    /// Reference to the parent object.
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        mxParent;

    /// pointer to internal representation
    VclPtr<Ruler>                       mpRepr;

        /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;


};

inline bool SvtRulerAccessible::IsAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvtRulerAccessible::IsNotAlive() const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
