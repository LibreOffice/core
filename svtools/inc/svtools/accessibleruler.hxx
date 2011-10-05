/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2010.
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVTRULERACCESSIBLE_HXX
#define _SVTRULERACCESSIBLE_HXX

#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/weak.hxx>

namespace com { namespace sun { namespace star { namespace awt {
       struct Point;
       struct Rectangle;
       struct Size;
       class XFocusListener;
} } } };

class Rectangle;
class Ruler;
class Window;

namespace css = com::sun::star;

typedef ::cppu::WeakAggComponentImplHelper5<
    ::css::accessibility::XAccessible,
    ::css ::accessibility::XAccessibleComponent,
    ::css::accessibility::XAccessibleContext,
    ::css::accessibility::XAccessibleEventBroadcaster,
    ::css::lang::XServiceInfo > SvtRulerAccessible_Base;

class SvtRulerAccessible : public ::comphelper::OBaseMutex, public SvtRulerAccessible_Base
{
public:
    //=====  internal  ========================================================
    SvtRulerAccessible(
        const ::css::uno::Reference< ::css::accessibility::XAccessible>& rxParent, Ruler& rRepresentation, const ::rtl::OUString& rName );
protected:
    virtual ~SvtRulerAccessible();
public:
    //=====  XAccessible  =====================================================

    virtual ::css::uno::Reference< ::css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( void ) throw( ::css::uno::RuntimeException );

    //=====  XAccessibleComponent  ============================================

    virtual sal_Bool SAL_CALL containsPoint( const ::css::awt::Point& rPoint ) throw( ::css::uno::RuntimeException );

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const ::css::awt::Point& rPoint ) throw( ::css::uno::RuntimeException );

    virtual ::css::awt::Rectangle SAL_CALL getBounds() throw( ::css::uno::RuntimeException );

    virtual ::css::awt::Point SAL_CALL getLocation() throw( ::css::uno::RuntimeException );

    virtual ::css::awt::Point SAL_CALL getLocationOnScreen() throw( ::css::uno::RuntimeException );

    virtual ::css::awt::Size SAL_CALL getSize() throw( ::css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isShowing() throw( ::css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isVisible() throw( ::css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isFocusTraversable() throw( ::css::uno::RuntimeException );

    virtual void SAL_CALL addFocusListener(
        const ::css::uno::Reference< ::css::awt::XFocusListener >& xListener )
        throw( ::css::uno::RuntimeException );

    virtual void SAL_CALL removeFocusListener(
        const ::css::uno::Reference< ::css::awt::XFocusListener >& xListener )
        throw( ::css::uno::RuntimeException );

    virtual void SAL_CALL  grabFocus() throw( ::css::uno::RuntimeException );

    virtual ::css::uno::Any SAL_CALL getAccessibleKeyBinding() throw( ::css::uno::RuntimeException );
    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::css::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAccessibleChildCount( void ) throw( ::css::uno::RuntimeException );

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible> SAL_CALL getAccessibleChild( sal_Int32 nIndex )
        throw( ::css::uno::RuntimeException, ::css::lang::IndexOutOfBoundsException );

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible> SAL_CALL getAccessibleParent( void ) throw( ::css::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent( void ) throw( ::css::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL getAccessibleRole( void ) throw( ::css::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getAccessibleDescription( void ) throw (::css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getAccessibleName( void ) throw (::css::uno::RuntimeException);

    virtual ::css::uno::Reference< ::css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet( void )
        throw( ::css::uno::RuntimeException );

    virtual ::css::uno::Reference< ::css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet( void )
        throw( ::css::uno::RuntimeException );

    virtual ::css::lang::Locale SAL_CALL getLocale( void )
        throw(  ::css::uno::RuntimeException,
                ::css::accessibility::IllegalAccessibleComponentStateException );
    //=====  XAccessibleEventBroadcaster  =====================================
    virtual void SAL_CALL addEventListener( const ::css::uno::Reference< ::css::accessibility::XAccessibleEventListener >& xListener )
        throw( css::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener( const ::css::uno::Reference< ::css::accessibility::XAccessibleEventListener >& xListener )
        throw( ::css::uno::RuntimeException );

    // Needed, because some compilers would complain about virtual functions above would hide
    // cppu::WeakAggComponentImplHelperBase::add/removeEventListener(const Reference<lang::XEventListener>&)...
    virtual void SAL_CALL addEventListener( const ::css::uno::Reference< ::css::lang::XEventListener >& xListener )
        throw( ::css::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener( const ::css::uno::Reference< ::css::lang::XEventListener >& xListener )
        throw( ::css::uno::RuntimeException );

    //=====  XServiceInfo  ====================================================

    virtual ::rtl::OUString SAL_CALL getImplementationName( void ) throw( ::css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& sServiceName ) throw( ::css::uno::RuntimeException );

    virtual ::css::uno::Sequence< ::rtl::OUString> SAL_CALL
    getSupportedServiceNames( void ) throw( ::css::uno::RuntimeException );

    //=====  XTypeProvider  ===================================================

    virtual ::css::uno::Sequence<sal_Int8> SAL_CALL
    getImplementationId( void ) throw( ::css::uno::RuntimeException );

public:

    /// Sets the name
    void setName( const ::rtl::OUString& rName );

    /// Sets the description
    void setDescription( const ::rtl::OUString& rDescr );
private:
    static ::css::uno::Sequence< sal_Int8 > getUniqueId( void );

protected:
    /// @Return the object's current bounding box relative to the desktop.
    virtual Rectangle GetBoundingBoxOnScreen( void ) throw( ::css::uno::RuntimeException );

    /// @Return the object's current bounding box relative to the parent object.
    virtual Rectangle GetBoundingBox( void ) throw( ::css::uno::RuntimeException );


    virtual void SAL_CALL disposing();

    /// @returns true if it's disposed or in disposing
    inline bool IsAlive( void ) const;

    /// @returns true if it's not disposed and no in disposing
    inline bool IsNotAlive( void ) const;

    /// throws the exception DisposedException if it's not alive
    void ThrowExceptionIfNotAlive( void ) throw( ::css::lang::DisposedException );

private:
    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    ::rtl::OUString msDescription;

    /** Name of this object.
     */
    ::rtl::OUString msName;

    /// Reference to the parent object.
    ::css::uno::Reference< ::css::accessibility::XAccessible > mxParent;

    /// pointer to internal representation
    Ruler* mpRepr;

    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 mnClientId;

};

inline bool SvtRulerAccessible::IsAlive( void ) const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose;
}

inline bool SvtRulerAccessible::IsNotAlive( void ) const
{
    return rBHelper.bDisposed || rBHelper.bInDispose;
}

#endif

