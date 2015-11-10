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
#include <cppuhelper/supportsservice.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotxdoc.hxx>

#include "swabstdlg.hxx"
#include "dialog.hrc"
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;

#define FILTER_OPTIONS_NAME             "FilterOptions"

SwXFilterOptions::SwXFilterOptions() :
    bExport( false )
{
}

SwXFilterOptions::~SwXFilterOptions()
{
}

uno::Sequence< beans::PropertyValue > SwXFilterOptions::getPropertyValues() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence<beans::PropertyValue> aRet(1);
    beans::PropertyValue* pArray = aRet.getArray();

    pArray[0].Name = FILTER_OPTIONS_NAME;
    pArray[0].Value <<= sFilterOptions;

    return aRet;
}

void   SwXFilterOptions::setPropertyValues( const uno::Sequence<beans::PropertyValue >& aProps )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException,
       IllegalArgumentException, WrappedTargetException, uno::RuntimeException, std::exception)
{
    const beans::PropertyValue* pPropArray = aProps.getConstArray();
    long nPropCount = aProps.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        OUString aPropName = rProp.Name;

        if ( aPropName == UNO_NAME_FILTER_NAME )
            rProp.Value >>= sFilterName;
        else if ( aPropName == FILTER_OPTIONS_NAME )
            rProp.Value >>= sFilterOptions;
        else if ( aPropName == "InputStream" )
            rProp.Value >>= xInputStream;
    }
}

void   SwXFilterOptions::setTitle( const OUString& /*rTitle*/ )
    throw (uno::RuntimeException, std::exception)
{
}

sal_Int16 SwXFilterOptions::execute() throw (uno::RuntimeException, std::exception)
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    std::unique_ptr<SvStream> pInStream;
    if ( xInputStream.is() )
        pInStream.reset(utl::UcbStreamHelper::CreateStream( xInputStream ));

    uno::Reference< XUnoTunnel > xTunnel(xModel, uno::UNO_QUERY);
    SwDocShell* pDocShell = nullptr;
    if(xTunnel.is())
    {
        SwXTextDocument* pXDoc = reinterpret_cast< SwXTextDocument * >(
                sal::static_int_cast< sal_IntPtr >(xTunnel->getSomething(SwXTextDocument::getUnoTunnelId())));
        pDocShell = pXDoc ? pXDoc->GetDocShell() : nullptr;
    }
    if(pDocShell)
    {

        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

        std::unique_ptr<AbstractSwAsciiFilterDlg> pAsciiDlg(pFact->CreateSwAsciiFilterDlg(nullptr, *pDocShell,
            pInStream.get()));
        OSL_ENSURE(pAsciiDlg, "Dialog creation failed!");
        if(RET_OK == pAsciiDlg->Execute())
        {
            SwAsciiOptions aOptions;
            pAsciiDlg->FillOptions( aOptions );
            aOptions.WriteUserData(sFilterOptions);
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
    }

    return nRet;
}

void   SwXFilterOptions::setTargetDocument( const uno::Reference< XComponent >& xDoc )
    throw (IllegalArgumentException, uno::RuntimeException, std::exception)
{
    bExport = false;
    xModel = xDoc;
}

void   SwXFilterOptions::setSourceDocument( const uno::Reference<XComponent >& xDoc )
        throw (IllegalArgumentException,uno::RuntimeException, std::exception)
{
    bExport = true;
    xModel = xDoc;
}

OUString SwXFilterOptions::getImplementationName() throw(uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Writer.FilterOptionsDialog");
}

sal_Bool SwXFilterOptions::supportsService( const OUString& rServiceName )
    throw(uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXFilterOptions::getSupportedServiceNames()
                throw(uno::RuntimeException, std::exception)
{
    OUString sService("com.sun.star.ui.dialogs.FilterOptionsDialog");
    return uno::Sequence< OUString> (&sService, 1);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_Writer_FilterOptionsDialog_get_implementation(css::uno::XComponentContext*,
                                css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;

    //the module may not be loaded
    SwGlobals::ensure();
    return cppu::acquire(new SwXFilterOptions());

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
