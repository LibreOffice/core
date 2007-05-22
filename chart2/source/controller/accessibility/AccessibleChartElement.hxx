/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleChartElement.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:14:45 $
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
#ifndef _CHART2_ACCESSIBLECHARTELEMENT_HXX_
#define _CHART2_ACCESSIBLECHARTELEMENT_HXX_

#include "AccessibleBase.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTBROADCASTER_HPP_
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEXTENDEDCOMPONENT_HPP_
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>

class SfxItemSet;
class SdrObject;

namespace chart
{

/** Base class for all Chart Accessibility objects except the root node (see AccessibleChartView)

    This class contains a reference to the ChartModel, thus, components can easily access all core functionality.

    Usage Instructions:

    <ul>
     <li>define the getAccessibleName() method of XAccessibleContext</li>
     <li>set the ChartModel using SetChartModel() for the first node before
         creating any children</li>
     <li>overload UpdateChildren()</li>
    </ul>
 */

namespace impl
{
typedef ::cppu::ImplInheritanceHelper1<
        AccessibleBase,
        ::com::sun::star::accessibility::XAccessibleExtendedComponent
        > AccessibleChartElement_Base;
}

class AccessibleChartElement :
    public impl::AccessibleChartElement_Base
{
public:
    AccessibleChartElement( const AccessibleElementInfo & rAccInfo,
                            bool bMayHaveChildren,
                            bool bAlwaysTransparent = false );
    virtual ~AccessibleChartElement();

    // ________ AccessibleBase ________
    virtual bool ImplUpdateChildren();
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >
        ImplGetAccessibleChildById( sal_Int32 i ) const
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 ImplGetAccessibleChildCount() const
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleContext ________
    virtual ::rtl::OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleExtendedComponent ________
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitledBorderText()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTipText()
        throw (::com::sun::star::uno::RuntimeException);

    // the following interface is implemented in AccessibleBase, however it is
    // also a (non-virtual) base class of XAccessibleExtendedComponent Thus
    // these methods have to be overloaded and forward to AccessibleBase

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground() throw (::com::sun::star::uno::RuntimeException);

    // ________ XServiceInfo ________
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

private:
    bool                m_bHasText;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleContext >
                        m_xTextHelper;

    void InitTextEdit();
};

}  // namespace chart

#endif
