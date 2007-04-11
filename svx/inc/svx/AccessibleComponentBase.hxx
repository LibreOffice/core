/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleComponentBase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:33:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_EXTENDED_COMPONENT_BASE_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_EXTENDED_COMPONENT_BASE_HXX


#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEXTENDEDCOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#endif
#include <com/sun/star/awt/XFocusListener.hpp>

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

namespace accessibility {

/** @descr
        This base class provides (will provice) a base implementation of the
        <type>XAccessibleComponent</type> and the
        <type>XAccessibleExtendedComponent</type> for all shapes.  For
        more detailed documentation about the methods refer to the interface
        descriptions of <type>XAccessible</type>,
        <type>XAccessibleContext</type>, <type>XInterface</type>,
        <type>XServiceInfo</type>, and <type>XTypeProvider</type>.
*/
class SVX_DLLPUBLIC AccessibleComponentBase
    :   public ::com::sun::star::accessibility::XAccessibleExtendedComponent
{
public:
    //=====  internal  ========================================================
    AccessibleComponentBase (void);
    virtual ~AccessibleComponentBase (void);


    //=====  XAccessibleComponent  ================================================

    /** The default implementation uses the result of
        <member>getBounds</member> to determine whether the given point lies
        inside this object.
    */
    virtual sal_Bool SAL_CALL containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty reference.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleAtPoint (
            const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty rectangle.
    */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the location.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation returns an empty position, i.e. the
    * result of the default constructor of <type>com::sun::star::awt::Point</type>.
    */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation uses the result of
        <member>getBounds</member> to determine the size.
    */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation ignores this call.
    */
    virtual void SAL_CALL addFocusListener (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation ignores this call.
    */
    virtual void SAL_CALL removeFocusListener (const ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    /** The default implementation does nothing.
    */
    virtual void SAL_CALL grabFocus (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns black as the default foreground color.
    */
    virtual sal_Int32 SAL_CALL getForeground (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns white as the default background color.
    */
    virtual sal_Int32 SAL_CALL getBackground (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XAccessibleExtendedComponent  ====================================
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL
        getFont (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText (void)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XTypeProvider  ===================================================

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes (void)
        throw (::com::sun::star::uno::RuntimeException);

};

} // end of namespace accessibility

#endif
