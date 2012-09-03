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

#ifndef TOOLKIT_TABPAGE_MODEL_HXX
#define TOOLKIT_TABPAGE_MODEL_HXX

#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "toolkit/helper/servicenames.hxx"
#include "toolkit/helper/macros.hxx"
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <list>
#include <cppuhelper/implbase2.hxx>

class UnoControlTabPageModel :  public ControlModelContainerBase
{
protected:
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();
public:
    UnoControlTabPageModel( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > const & i_factory);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    // XInitialization
    virtual void SAL_CALL initialize (const com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rArguments)
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageModel, ControlModelContainerBase, szServiceName_UnoControlTabPageModel )

};

//  ----------------------------------------------------
//  class UnoTabPageControl
//  ----------------------------------------------------
typedef ::cppu::AggImplInheritanceHelper2   <   ControlContainerBase
                                            ,   ::com::sun::star::awt::tab::XTabPage
                                            ,   ::com::sun::star::awt::XWindowListener
                                            >   UnoControlTabPage_Base;
class UnoControlTabPage : public UnoControlTabPage_Base
{
private:
    bool            m_bWindowListener;
public:

    UnoControlTabPage( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~UnoControlTabPage();
    ::rtl::OUString             GetComponentServiceName();

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

     // ::com::sun::star::awt::XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoControlTabPage, szServiceName_UnoControlTabPage)
};

#endif // TOOLKIT_TABPAGE_MODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
