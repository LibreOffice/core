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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _SWXFILTEROPTIONS_HXX
#include <SwXFilterOptions.hxx>
#endif

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef SW_SWDLL_HXX
#include <swdll.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTXDOC_HXX //autogen
#include <unotxdoc.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;

#define SWFILTEROPTIONSOBJ_SERVICE      RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.dialogs.FilterOptionsDialog")
#define SWFILTEROPTIONSOBJ_IMPLNAME     RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.FilterOptionsDialog")
#define FILTER_OPTIONS_NAME             RTL_CONSTASCII_USTRINGPARAM("FilterOptions")
/*-- 2002/06/21 11:01:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFilterOptions::SwXFilterOptions() :
    bExport( sal_False )
{
}
/*-- 2002/06/21 11:01:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFilterOptions::~SwXFilterOptions()
{
}
/*-- 2002/06/21 11:01:24---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString  SwXFilterOptions::getImplementationName_Static()
{
    return ::rtl::OUString(SWFILTEROPTIONSOBJ_IMPLNAME);
}
/*-- 2002/06/21 11:01:24---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< ::rtl::OUString> SwXFilterOptions::getSupportedServiceNames_Static()
{
    ::rtl::OUString sService(SWFILTEROPTIONSOBJ_SERVICE);
    return uno::Sequence< ::rtl::OUString> (&sService, 1);
}
/*-- 2002/06/21 11:01:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyValue > SwXFilterOptions::getPropertyValues() throw (uno::RuntimeException)
{
    uno::Sequence<beans::PropertyValue> aRet(1);
    beans::PropertyValue* pArray = aRet.getArray();

    pArray[0].Name = ::rtl::OUString( FILTER_OPTIONS_NAME );
    pArray[0].Value <<= sFilterOptions;

    return aRet;
}
/*-- 2002/06/21 11:01:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void   SwXFilterOptions::setPropertyValues( const uno::Sequence<beans::PropertyValue >& aProps )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
       IllegalArgumentException, WrappedTargetException, uno::RuntimeException)
{
    const beans::PropertyValue* pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        ::rtl::OUString aPropName = rProp.Name;

        if ( aPropName.equalsAscii( SW_PROP_NAME_STR(UNO_NAME_FILTER_NAME) ) )
            rProp.Value >>= sFilterName;
        else if ( aPropName == ::rtl::OUString(FILTER_OPTIONS_NAME) )
            rProp.Value >>= sFilterOptions;
        else if ( aPropName.equalsAscii( "InputStream" ) )
            rProp.Value >>= xInputStream;
    }
}
/*-- 2002/06/21 11:01:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void   SwXFilterOptions::setTitle( const ::rtl::OUString& aTitle )
    throw (uno::RuntimeException)
{
}
/*-- 2002/06/21 11:01:25---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 SwXFilterOptions::execute() throw (uno::RuntimeException)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    SvStream* pInStream = NULL;
    if ( xInputStream.is() )
        pInStream = ::utl::UcbStreamHelper::CreateStream( xInputStream );

    uno::Reference< XUnoTunnel > xTunnel(xModel, uno::UNO_QUERY);
    SwDocShell* pDocShell = 0;
    if(xTunnel.is())
    {
        SwXTextDocument* pXDoc = (SwXTextDocument*)xTunnel->getSomething(SwXTextDocument::getUnoTunnelId());
        pDocShell = pXDoc ? pXDoc->GetDocShell() : 0;
    }
    if(pDocShell)
    {
    }
    
    if( pInStream )
        delete pInStream;

    return nRet;
}
/*-- 2002/06/21 11:01:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void   SwXFilterOptions::setTargetDocument( const uno::Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, uno::RuntimeException)
{
    bExport = sal_False;
    xModel = xDoc;
}
/*-- 2002/06/21 11:01:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void   SwXFilterOptions::setSourceDocument( const uno::Reference<XComponent >& xDoc )
        throw (IllegalArgumentException,uno::RuntimeException)
{
    bExport = sal_True;
    xModel = xDoc;
}
/*-- 2002/06/21 11:01:26---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString SwXFilterOptions::getImplementationName() throw(uno::RuntimeException)
{
    return ::rtl::OUString(SWFILTEROPTIONSOBJ_IMPLNAME);
}
/*-- 2002/06/21 11:01:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXFilterOptions::supportsService( const ::rtl::OUString& rServiceName )
    throw(uno::RuntimeException)
{
    return rServiceName == ::rtl::OUString(SWFILTEROPTIONSOBJ_SERVICE);
}
/*-- 2002/06/21 11:01:28---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< ::rtl::OUString > SwXFilterOptions::getSupportedServiceNames()
                throw(uno::RuntimeException)
{
    return SwXFilterOptions::getSupportedServiceNames_Static();
}


uno::Reference<uno::XInterface> SAL_CALL SwXFilterOptions_createInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    SW_DLL()->Load();       // load module

    return (::cppu::OWeakObject*) new SwXFilterOptions;
}

}
