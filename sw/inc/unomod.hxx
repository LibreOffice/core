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
#ifndef _UNOMOD_HXX
#define _UNOMOD_HXX

#include <com/sun/star/text/XModule.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/view/XPrintSettingsSupplier.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <cppuhelper/implbase4.hxx> // helper for implementations
#include <comphelper/ChainablePropertySet.hxx>
#include <comphelper/SettingsHelper.hxx>
#include <usrpref.hxx>

class SwView;
class SwViewOption;
class SwPrintData;
class SwDoc;

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL SwXModule_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );

class SwXModule : public cppu::WeakImplHelper4
<
    ::com::sun::star::text::XModule,
    ::com::sun::star::view::XViewSettingsSupplier,
    ::com::sun::star::view::XPrintSettingsSupplier,
    ::com::sun::star::lang::XServiceInfo
>
{

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *     pxViewSettings;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *     pxPrintSettings;

protected:
    virtual ~SwXModule();
public:
    SwXModule();


    //XViewSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getViewSettings(void)
        throw( ::com::sun::star::uno::RuntimeException );

    //XPrintSettings
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  SAL_CALL getPrintSettings(void)
        throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw( ::com::sun::star::uno::RuntimeException );
};

enum SwXPrintSettingsType
{
    PRINT_SETTINGS_MODULE,
    PRINT_SETTINGS_WEB,
    PRINT_SETTINGS_DOCUMENT
};

class SwXPrintSettings : public comphelper::ChainableHelperNoState
{
    friend class SwXDocumentSettings;
protected:
    SwXPrintSettingsType meType;
    SwPrintData * mpPrtOpt;
    SwDoc *mpDoc;

    virtual void _preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual void _preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual ~SwXPrintSettings()
        throw();
public:
    SwXPrintSettings( SwXPrintSettingsType eType, SwDoc * pDoc = NULL );


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw( ::com::sun::star::uno::RuntimeException );
};

class SwXViewSettings : public comphelper::ChainableHelperNoState
{

    friend class SwXDocumentSettings;
protected:
    SwView*                     pView;
    SwViewOption*       mpViewOption;
    const SwViewOption*         mpConstViewOption;
    sal_Bool                    bObjectValid:1, bWeb:1, mbApplyZoom;

    sal_Int32   eHRulerUnit;
    sal_Bool    mbApplyHRulerMetric;
    sal_Int32   eVRulerUnit;
    sal_Bool    mbApplyVRulerMetric;

    virtual void _preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual void _preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual ~SwXViewSettings()
        throw();
public:
    SwXViewSettings(sal_Bool bWeb, SwView*  pView);


    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    sal_Bool    IsValid() const {return bObjectValid;}
    void    Invalidate() {bObjectValid = sal_False;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
