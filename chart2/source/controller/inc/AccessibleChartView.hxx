/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
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
#ifndef _CHART2_ACCESSIBLE_CHART_VIEW_HXX
#define _CHART2_ACCESSIBLE_CHART_VIEW_HXX

#include "AccessibleBase.hxx"
#include "MutexContainer.hxx"
#include "ServiceMacros.hxx"
#include <cppuhelper/implbase2.hxx>
// header for class WeakReference
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <boost/shared_ptr.hpp>

namespace accessibility
{
class IAccessibleViewForwarder;
}

//.............................................................................
namespace chart
{
//.............................................................................

class ExplicitValueProvider;

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
            ::com::sun::star::uno::XComponentContext >& xContext, SdrView* pView );
    virtual ~AccessibleChartView();

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

protected:
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
    AccessibleUniqueId                                              m_aCurrentSelectionOID;
    SdrView*                                                        m_pSdrView;
    ::accessibility::IAccessibleViewForwarder*                      m_pViewForwarder;

    //no default constructor
    AccessibleChartView();
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
