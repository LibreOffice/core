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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_ELEMENTSELECTOR_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_ELEMENTSELECTOR_HXX

#include "ObjectHierarchy.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svtools/toolboxcontroller.hxx>

#include <vcl/lstbox.hxx>
#include <cppuhelper/weakref.hxx>

namespace chart
{

struct ListBoxEntryData
{
    OUString UIName;
    ObjectHierarchy::tOID OID;
    sal_Int32 nHierarchyDepth;

    ListBoxEntryData() : nHierarchyDepth(0)
    {
    }
};

class SelectorListBox : public ListBox
{
    public:
        SelectorListBox( vcl::Window* pParent, WinBits nStyle );

        virtual void Select() SAL_OVERRIDE;
        virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;

        void ReleaseFocus_Impl();

        void SetChartController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xChartController );
        void UpdateChartElementsListAndSelection();

    private:
        ::com::sun::star::uno::WeakReference<
            ::com::sun::star::frame::XController >   m_xChartController;

        ::std::vector< ListBoxEntryData > m_aEntries;

        bool m_bReleaseFocus;
};

typedef ::cppu::ImplHelper1 < ::com::sun::star::lang::XServiceInfo> ElementSelectorToolbarController_BASE;

class ElementSelectorToolbarController : public ::svt::ToolboxController
                                                , ElementSelectorToolbarController_BASE
{
public:
    explicit ElementSelectorToolbarController( ::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~ElementSelectorToolbarController();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XInterface
     virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
     virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;
     virtual void SAL_CALL release() throw () SAL_OVERRIDE;

     // XInitialization
     virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
     // XStatusListener
     virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
     // XToolbarController
     virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    //no default constructor
    ElementSelectorToolbarController(){}

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>        m_xCC;
    VclPtr< SelectorListBox > m_apSelectorListBox;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
