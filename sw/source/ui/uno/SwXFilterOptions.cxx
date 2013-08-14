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

#include <SwXFilterOptions.hxx>
#include <shellio.hxx>
#include <swdll.hxx>
#include <unoprnms.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <unotools/ucbstreamhelper.hxx>
#include <unotxdoc.hxx>

#include "swabstdlg.hxx"
#include "dialog.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;

#define SWFILTEROPTIONSOBJ_SERVICE      "com.sun.star.ui.dialogs.FilterOptionsDialog"
#define SWFILTEROPTIONSOBJ_IMPLNAME     "com.sun.star.comp.Writer.FilterOptionsDialog"
#define FILTER_OPTIONS_NAME             "FilterOptions"

SwXFilterOptions::SwXFilterOptions() :
    bExport( sal_False )
{
}

SwXFilterOptions::~SwXFilterOptions()
{
}

OUString  SwXFilterOptions::getImplementationName_Static()
{
    return OUString(SWFILTEROPTIONSOBJ_IMPLNAME);
}

uno::Sequence< OUString> SwXFilterOptions::getSupportedServiceNames_Static()
{
    OUString sService(SWFILTEROPTIONSOBJ_SERVICE);
    return uno::Sequence< OUString> (&sService, 1);
}

uno::Sequence< beans::PropertyValue > SwXFilterOptions::getPropertyValues() throw (uno::RuntimeException)
{
    uno::Sequence<beans::PropertyValue> aRet(1);
    beans::PropertyValue* pArray = aRet.getArray();

    pArray[0].Name = OUString( FILTER_OPTIONS_NAME );
    pArray[0].Value <<= sFilterOptions;

    return aRet;
}

void   SwXFilterOptions::setPropertyValues( const uno::Sequence<beans::PropertyValue >& aProps )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
       IllegalArgumentException, WrappedTargetException, uno::RuntimeException)
{
    const beans::PropertyValue* pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        OUString aPropName = rProp.Name;

        if ( aPropName.equalsAscii( SW_PROP_NAME_STR(UNO_NAME_FILTER_NAME) ) )
            rProp.Value >>= sFilterName;
        else if ( aPropName == OUString(FILTER_OPTIONS_NAME) )
            rProp.Value >>= sFilterOptions;
        else if ( aPropName == "InputStream" )
            rProp.Value >>= xInputStream;
    }
}

void   SwXFilterOptions::setTitle( const OUString& /*rTitle*/ )
    throw (uno::RuntimeException)
{
}

sal_Int16 SwXFilterOptions::execute() throw (uno::RuntimeException)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    SvStream* pInStream = NULL;
    if ( xInputStream.is() )
        pInStream = utl::UcbStreamHelper::CreateStream( xInputStream );

    uno::Reference< XUnoTunnel > xTunnel(xModel, uno::UNO_QUERY);
    SwDocShell* pDocShell = 0;
    if(xTunnel.is())
    {
        SwXTextDocument* pXDoc = reinterpret_cast< SwXTextDocument * >(
                sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SwXTextDocument::getUnoTunnelId())));
        pDocShell = pXDoc ? pXDoc->GetDocShell() : 0;
    }
    if(pDocShell)
    {

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        AbstractSwAsciiFilterDlg* pAsciiDlg = pFact->CreateSwAsciiFilterDlg(NULL, *pDocShell,
            pInStream);
        OSL_ENSURE(pAsciiDlg, "Dialogdiet fail!");
        if(RET_OK == pAsciiDlg->Execute())
        {
            SwAsciiOptions aOptions;
            pAsciiDlg->FillOptions( aOptions );
            aOptions.WriteUserData(sFilterOptions);
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
        delete pAsciiDlg;
    }

    delete pInStream;

    return nRet;
}

void   SwXFilterOptions::setTargetDocument( const uno::Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, uno::RuntimeException)
{
    bExport = sal_False;
    xModel = xDoc;
}

void   SwXFilterOptions::setSourceDocument( const uno::Reference<XComponent >& xDoc )
        throw (IllegalArgumentException,uno::RuntimeException)
{
    bExport = sal_True;
    xModel = xDoc;
}

OUString SwXFilterOptions::getImplementationName() throw(uno::RuntimeException)
{
    return OUString(SWFILTEROPTIONSOBJ_IMPLNAME);
}

sal_Bool SwXFilterOptions::supportsService( const OUString& rServiceName )
    throw(uno::RuntimeException)
{
    return rServiceName == OUString(SWFILTEROPTIONSOBJ_SERVICE);
}

uno::Sequence< OUString > SwXFilterOptions::getSupportedServiceNames()
                throw(uno::RuntimeException)
{
    return SwXFilterOptions::getSupportedServiceNames_Static();
}

uno::Reference<uno::XInterface> SAL_CALL SwXFilterOptions_createInstance(
                        const uno::Reference<lang::XMultiServiceFactory>& )
{
    SolarMutexGuard aGuard;
    SwGlobals::ensure();
    return (::cppu::OWeakObject*) new SwXFilterOptions;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
