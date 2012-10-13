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

#ifndef _TOOLKIT_AWT_VCLXTABPAGEMODEL_HXX_
#define _TOOLKIT_AWT_VCLXTABPAGEMODEL_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include "toolkit/awt/vclxwindow.hxx"
#include <toolkit/controls/unocontrolmodel.hxx>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <cppuhelper/implbase1.hxx>
//  ----------------------------------------------------
typedef ::cppu::AggImplInheritanceHelper1 <    UnoControlModel,
                                            ::com::sun::star::awt::tab::XTabPageModel
                                             > VCLXTabPageModel_Base;
class VCLXTabPageModel : public VCLXTabPageModel_Base
{
public:
    VCLXTabPageModel();
    VCLXTabPageModel( const VCLXTabPageModel& rModel ) : VCLXTabPageModel_Base( rModel ) {;}
    ~VCLXTabPageModel();

    // ::com::sun::star::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDevice,
    ::com::sun::star::awt::DeviceInfo SAL_CALL getInfo() throw(::com::sun::star::uno::RuntimeException);

    UnoControlModel*    Clone() const { return new VCLXTabPageModel( *this ); }

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::tab::XTabPageModel
    virtual ::sal_Int16 SAL_CALL getTabPageID() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getEnabled() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEnabled( ::sal_Bool _enabled ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitle() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::rtl::OUString& _title ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImageURL() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setImageURL( const ::rtl::OUString& _imageurl ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getToolTip() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setToolTip( const ::rtl::OUString& _tooltip ) throw (::com::sun::star::uno::RuntimeException);
protected:
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
};
#endif // _TOOLKIT_AWT_VCLXTABPAGEMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
