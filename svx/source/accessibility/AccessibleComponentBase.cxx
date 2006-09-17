/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleComponentBase.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 03:59:15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"


#include "AccessibleComponentBase.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLE_ROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_SELECTION_HPP_
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessibleComponentBase::AccessibleComponentBase (void)
{
}




AccessibleComponentBase::~AccessibleComponentBase (void)
{
}




//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL AccessibleComponentBase::containsPoint (
        const ::com::sun::star::awt::Point& aPoint)
        throw (::com::sun::star::uno::RuntimeException)
{
    awt::Size aSize (getSize());
    return (aPoint.X >= 0)
        && (aPoint.X < aSize.Width)
        && (aPoint.Y >= 0)
        && (aPoint.Y < aSize.Height);
}




uno::Reference<XAccessible > SAL_CALL
    AccessibleComponentBase::getAccessibleAtPoint (
        const awt::Point& /*aPoint*/)
    throw (uno::RuntimeException)
{
    return uno::Reference<XAccessible>();
}




awt::Rectangle SAL_CALL AccessibleComponentBase::getBounds (void)
    throw (uno::RuntimeException)
{
    return awt::Rectangle();
}




awt::Point SAL_CALL AccessibleComponentBase::getLocation (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Point (aBBox.X, aBBox.Y);
}




awt::Point SAL_CALL AccessibleComponentBase::getLocationOnScreen (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return awt::Point();
}




::com::sun::star::awt::Size SAL_CALL AccessibleComponentBase::getSize (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Size (aBBox.Width, aBBox.Height);
}




void SAL_CALL AccessibleComponentBase::addFocusListener (
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& /*xListener*/)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::removeFocusListener (const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& /*xListener*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::grabFocus (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference<XAccessibleContext> xContext (this, uno::UNO_QUERY);
    uno::Reference<XAccessibleSelection> xSelection (
        xContext->getAccessibleParent(), uno::UNO_QUERY);
    if (xSelection.is())
    {
        // Do a single selection on this object.
        xSelection->clearAccessibleSelection();
        xSelection->selectAccessibleChild (xContext->getAccessibleIndexInParent());
    }
}




sal_Int32 SAL_CALL AccessibleComponentBase::getForeground (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return Color(COL_BLACK).GetColor();
}




sal_Int32 SAL_CALL AccessibleComponentBase::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return Color(COL_WHITE).GetColor();
}




//=====  XAccessibleExtendedComponent  ========================================

::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL
        AccessibleComponentBase::getFont (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference<awt::XFont>();
}




::rtl::OUString SAL_CALL AccessibleComponentBase::getTitledBorderText (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("");
}




::rtl::OUString SAL_CALL AccessibleComponentBase::getToolTipText (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("");
}




//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleComponentBase::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
    uno::Sequence<uno::Type> aTypeList (2);
    // ...and add the additional type for the component.
    const uno::Type aComponentType =
         ::getCppuType((const uno::Reference<XAccessibleComponent>*)0);
    const uno::Type aExtendedComponentType =
        ::getCppuType((const uno::Reference<XAccessibleExtendedComponent>*)0);
    aTypeList[0] = aComponentType;
    aTypeList[1] = aExtendedComponentType;

    return aTypeList;
}


} // end of namespace accessibility
