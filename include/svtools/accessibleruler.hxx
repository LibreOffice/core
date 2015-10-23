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
            css::accessibility::XAccessible,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleEventBroadcaster,
            css::lang::XServiceInfo >
            SvtRulerAccessible_Base;

class SvtRulerAccessible : public ::comphelper::OBaseMutex, public SvtRulerAccessible_Base
{
public:
    //=====  internal  ========================================================
    SvtRulerAccessible(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent, Ruler& rRepresentation, const ::rtl::OUString& rName );
protected:
    virtual ~SvtRulerAccessible();
public:

    bool SAL_CALL
        isVisible() throw( css::uno::RuntimeException );

    //=====  XAccessible  =====================================================

    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw( css::uno::RuntimeException, std::exception ) override;

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL
        containsPoint( const css::awt::Point& rPoint ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint( const css::awt::Point& rPoint ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Rectangle SAL_CALL
        getBounds() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL
        getLocation() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Point SAL_CALL
        getLocationOnScreen() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::awt::Size SAL_CALL
        getSize() throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        grabFocus() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL
        getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild( sal_Int32 nIndex )
            throw( css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleParent() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int16 SAL_CALL
        getAccessibleRole() throw( css::uno::RuntimeException, std::exception ) override;

    virtual ::rtl::OUString SAL_CALL
        getAccessibleDescription() throw (css::uno::RuntimeException, std::exception) override;

    virtual ::rtl::OUString SAL_CALL
        getAccessibleName() throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet() throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::lang::Locale SAL_CALL
        getLocale()
            throw(  css::uno::RuntimeException,
                    css::accessibility::IllegalAccessibleComponentStateException, std::exception ) override;
    //=====  XAccessibleEventBroadcaster  =====================================

    virtual void SAL_CALL
        addAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener )throw( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL
        removeAccessibleEventListener( const css::uno::Reference< css::accessibility::XAccessibleEventListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;

    //=====  XServiceInfo  ====================================================

    virtual ::rtl::OUString SAL_CALL
        getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
        supportsService( const ::rtl::OUString& sServiceName ) throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    //=====  XTypeProvider  ===================================================

    virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId() throw( css::uno::RuntimeException, std::exception ) override;

protected:

    /// @Return the object's current bounding box relative to the desktop.
    Rectangle GetBoundingBoxOnScreen() throw( css::uno::RuntimeException );

    /// @Return the object's current bounding box relative to the parent object.
    Rectangle GetBoundingBox() throw( css::uno::RuntimeException );


    virtual void SAL_CALL disposing() override;

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive() const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive() const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive() throw( css::lang::DisposedException );

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    ::rtl::OUString                     msDescription;

    /** Name of this object.
    */
    ::rtl::OUString                     msName;

    /// Reference to the parent object.
    css::uno::Reference< css::accessibility::XAccessible >
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
