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
#ifndef _CHART_ELEMENTSELECTOR_HXX
#define _CHART_ELEMENTSELECTOR_HXX

#include "ServiceMacros.hxx"
#include "ObjectHierarchy.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svtools/toolboxcontroller.hxx>

#include <vcl/lstbox.hxx>
#include <cppuhelper/weakref.hxx>

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

struct ListBoxEntryData
{
    rtl::OUString UIName;
    ObjectHierarchy::tOID OID;
    sal_Int32 nHierarchyDepth;

    ListBoxEntryData() : nHierarchyDepth(0)
    {
    }
};

class SelectorListBox : public ListBox
{
    public:
        SelectorListBox( Window* pParent, WinBits nStyle );
        virtual ~SelectorListBox();

        virtual void Select();
        virtual long Notify( NotifyEvent& rNEvt );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

        void ReleaseFocus_Impl();

        void SetChartController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xChartController );
        void UpdateChartElementsListAndSelection();

    private:
        ::com::sun::star::uno::WeakReference<
            ::com::sun::star::frame::XController >   m_xChartController;

        ::std::vector< ListBoxEntryData > m_aEntries;

        bool m_bReleaseFocus;
};

// ------------------------------------------------------------------
// ------------------------------------------------------------------

typedef ::cppu::ImplHelper1 < ::com::sun::star::lang::XServiceInfo> ElementSelectorToolbarController_BASE;

class ElementSelectorToolbarController : public ::svt::ToolboxController
                                                , ElementSelectorToolbarController_BASE
{
public:
    ElementSelectorToolbarController( ::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~ElementSelectorToolbarController();

    // XServiceInfo
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ElementSelectorToolbarController)

    // XInterface
     virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
     virtual void SAL_CALL acquire() throw ();
     virtual void SAL_CALL release() throw ();

     // XInitialization
     virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
     // XStatusListener
     virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
     // XToolbarController
     virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);

private:
    //no default constructor
    ElementSelectorToolbarController(){}

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>        m_xCC;
    ::std::auto_ptr< SelectorListBox > m_apSelectorListBox;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
