/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleChartView.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:51:50 $
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
#ifndef _CHART2_ACCESSIBLE_CHART_VIEW_HXX
#define _CHART2_ACCESSIBLE_CHART_VIEW_HXX

#include "AccessibleBase.hxx"
#include "MutexContainer.hxx"
#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
// header for class WeakReference
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#include <boost/shared_ptr.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

class ExplicitValueProvider;
class ObjectHierarchy;

namespace impl
{
typedef ::cppu::ImplInheritanceHelper2<
        ::chart::AccessibleBase,
        ::com::sun::star::lang::XInitialization,
        ::com::sun::star::view::XSelectionChangeListener >
    AccessibleChartView_Base;
}

class AccessibleChartView :
        public impl::AccessibleChartView_Base
{
public:
    AccessibleChartView(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~AccessibleChartView();

    // ____ lang::XServiceInfo ____
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER( AccessibleChartView )

protected:
    // ____ WeakComponentHelper (called from XComponent::dispose()) ____
    virtual void SAL_CALL disposing();

    // ____ lang::XInitialization ____
    // 0: view::XSelectionSupplier offers notifications for selection changes and access to the selection itself
    // 1: frame::XModel representing the chart model - offers access to object data
    // 2: lang::XInterface representing the normal chart view - offers access to some extra object data
    // 3: accessibility::XAccessible representing the parent accessible
    // 4: awt::XWindow representing the view's window (is a vcl Window)
    // all arguments are only valid until next initialization - don't keep them longer
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw (::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);

    // ____ view::XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // ________ XEventListener ________
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleContext ________
    virtual ::rtl::OUString SAL_CALL getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleComponent ________
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw (::com::sun::star::uno::RuntimeException);

    // ________ AccessibleChartElement ________
    virtual ::com::sun::star::awt::Point   GetUpperLeftOnScreen() const;

private: // methods
    /** @return the result that m_xWindow->getPosSize() _should_ return.  It
                returns (0,0) as upper left corner.  When calling
                getAccessibleParent, you get the parent's parent, which contains
                a decoration.  Thus you have an offset of (currently) (2,2)
                which isn't taken into account.
     */
    virtual ::com::sun::star::awt::Rectangle GetWindowPosSize() const;

    ExplicitValueProvider* getExplicitValueProvider();

private: // members
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext>    m_xContext;
    ::com::sun::star::uno::WeakReference<
                       ::com::sun::star::view::XSelectionSupplier > m_xSelectionSupplier;
    ::com::sun::star::uno::WeakReference<
                       ::com::sun::star::frame::XModel >            m_xChartModel;
    ::com::sun::star::uno::WeakReference<
                       ::com::sun::star::uno::XInterface >          m_xChartView;
    ::com::sun::star::uno::WeakReference<
                        ::com::sun::star::awt::XWindow >            m_xWindow;
    ::com::sun::star::uno::WeakReference<
                       ::com::sun::star::accessibility::XAccessible > m_xParent;

    ::boost::shared_ptr< ObjectHierarchy >                          m_spObjectHierarchy;
    AccessibleUniqueId                                              m_aCurrentSelectionCID;

    //no default constructor
    AccessibleChartView();
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
