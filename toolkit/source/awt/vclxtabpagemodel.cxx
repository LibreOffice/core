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

#include <toolkit/awt/vclxtabpagemodel.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <cppuhelper/typeprovider.hxx>
//  ----------------------------------------------------
//  class VCLXDialog
//  ----------------------------------------------------

VCLXTabPageModel::VCLXTabPageModel()
{
}

VCLXTabPageModel::~VCLXTabPageModel()
{
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s", __FUNCTION__);
#endif
}

void SAL_CALL VCLXTabPageModel::draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    Window* pWindow = GetWindow();

    if ( pWindow )
    {
        OutputDevice* pDev = VCLUnoHelper::GetOutputDevice( getGraphics() );
        if ( !pDev )
            pDev = pWindow->GetParent();

        Size aSize = pDev->PixelToLogic( pWindow->GetSizePixel() );
        Point aPos = pDev->PixelToLogic( Point( nX, nY ) );

        pWindow->Draw( pDev, aPos, aSize, WINDOW_DRAW_NOCONTROLS );
    }
}

::com::sun::star::awt::DeviceInfo VCLXTabPageModel::getInfo() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo;// = VCLXDevice::getInfo();
    return aInfo;
}


void SAL_CALL VCLXTabPageModel::setProperty(
    const ::rtl::OUString& /*PropertyName*/,
    const ::com::sun::star::uno::Any& /*Value*/ )
throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );

    /*TabPage* pTabPage = (TabPage*)GetWindow();
    if ( pTabPage )
    {
        VCLXWindow::setProperty( PropertyName, Value );
    }*/
}
//XTabPageModel
::sal_Int16 SAL_CALL VCLXTabPageModel::getTabPageID() throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}
::sal_Bool SAL_CALL VCLXTabPageModel::getEnabled() throw (::com::sun::star::uno::RuntimeException)
{
    return false;
}
void SAL_CALL VCLXTabPageModel::setEnabled( ::sal_Bool _enabled ) throw (::com::sun::star::uno::RuntimeException)
{
    //TabControl* pTabControl = (TabControl*)GetWindow();
    //if ( pTabControl )
    //  pTabControl->EnablePage(0, true);
}
::rtl::OUString SAL_CALL VCLXTabPageModel::getTitle() throw (::com::sun::star::uno::RuntimeException)
{
    //TabControl* pTabControl = (TabControl*)GetWindow();
    //if ( pTabControl )
    //  return pTabControl->GetPageText(0);
    //else
        return ::rtl::OUString();
}
void SAL_CALL VCLXTabPageModel::setTitle( const ::rtl::OUString& _title ) throw (::com::sun::star::uno::RuntimeException)
{
    //TabControl* pTabControl = (TabControl*)GetWindow();
    //if ( pTabControl )
    //  pTabControl->SetPageText(0, _title);

}
::rtl::OUString SAL_CALL VCLXTabPageModel::getImageURL() throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString();
}
void SAL_CALL VCLXTabPageModel::setImageURL( const ::rtl::OUString& /*_imageurl*/ ) throw (::com::sun::star::uno::RuntimeException)
{
    //m_sImageURL = _imageurl;
}
::rtl::OUString SAL_CALL VCLXTabPageModel::getToolTip() throw (::com::sun::star::uno::RuntimeException)
{
    //return m_sTooltip;
    return ::rtl::OUString();
}
void SAL_CALL VCLXTabPageModel::setToolTip( const ::rtl::OUString& _tooltip ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)_tooltip;
}
::cppu::IPropertyArrayHelper& VCLXTabPageModel::getInfoHelper()
{
     static UnoPropertyArrayHelper* pHelper = NULL;
     if ( !pHelper )
     {
        com::sun::star::uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
         pHelper = new UnoPropertyArrayHelper( aIDs );
    }
     return *pHelper;
}
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > VCLXTabPageModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
