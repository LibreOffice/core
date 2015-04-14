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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_SHAPETOOLBARCONTROLLER_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_SHAPETOOLBARCONTROLLER_HXX

#include <sal/config.h>

#include <map>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XSubToolbarController.hpp>

#include <cppuhelper/implbase2.hxx>
#include <rtl/ref.hxx>
#include <svtools/toolboxcontroller.hxx>

class SfxToolBoxControl;

namespace chart
{

typedef ::cppu::ImplHelper2 < ::com::sun::star::lang::XServiceInfo,
                              ::com::sun::star::frame::XSubToolbarController> ShapeToolbarController_Base;

typedef rtl::Reference<SfxToolBoxControl> TToolbarHelper;

class ShapeToolbarController : public ::svt::ToolboxController
                              ,public ShapeToolbarController_Base
{
    typedef std::map<OUString, sal_Bool> TCommandState;
    TCommandState   m_aStates;
    TToolbarHelper  m_pToolbarController;
    sal_uInt16      m_nToolBoxId;
    sal_uInt16      m_nSlotId;
    ShapeToolbarController( const ShapeToolbarController& ) SAL_DELETED_FUNCTION;
    void operator =( const ShapeToolbarController& ) SAL_DELETED_FUNCTION;

public:
    ShapeToolbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~ShapeToolbarController();

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;
    virtual void SAL_CALL release() throw () SAL_OVERRIDE;

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // needed by registration
    static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::lang::XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::frame::XStatusListener
    virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::frame::XToolbarController
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createPopupWindow() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL execute( sal_Int16 KeyModifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::frame::XSubToolbarController
    virtual sal_Bool SAL_CALL opensSubToolbar() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getSubToolbarName() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL functionSelected( const OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL updateImage() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

} //  namespace chart

#endif // INCLUDED_CHART2_SOURCE_CONTROLLER_MAIN_SHAPETOOLBARCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
