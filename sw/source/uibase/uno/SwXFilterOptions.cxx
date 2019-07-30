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
#include <vcl/svapp.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotxdoc.hxx>

#include <swabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;

#define FILTER_OPTIONS_NAME             "FilterOptions"

SwXFilterOptions::SwXFilterOptions()
{
}

SwXFilterOptions::~SwXFilterOptions()
{
}

uno::Sequence< beans::PropertyValue > SwXFilterOptions::getPropertyValues()
{
    return comphelper::InitPropertySequence({
            { FILTER_OPTIONS_NAME, uno::Any(sFilterOptions) }
        });
}

void   SwXFilterOptions::setPropertyValues( const uno::Sequence<beans::PropertyValue >& aProps )
{
    for (const beans::PropertyValue& rProp : aProps)
    {
        OUString aPropName = rProp.Name;

        if ( aPropName == FILTER_OPTIONS_NAME )
            rProp.Value >>= sFilterOptions;
        else if ( aPropName == "InputStream" )
            rProp.Value >>= xInputStream;
    }
}

void   SwXFilterOptions::setTitle( const OUString& /*rTitle*/ )
{
}

sal_Int16 SwXFilterOptions::execute()
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    std::unique_ptr<SvStream> pInStream;
    if ( xInputStream.is() )
        pInStream = utl::UcbStreamHelper::CreateStream( xInputStream );

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
        ScopedVclPtr<AbstractSwAsciiFilterDlg> pAsciiDlg(pFact->CreateSwAsciiFilterDlg(Application::GetFrameWeld(xDialogParent), *pDocShell,
            pInStream.get()));
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
{
    xModel = xDoc;
}

void   SwXFilterOptions::setSourceDocument( const uno::Reference<XComponent >& xDoc )
{
    xModel = xDoc;
}

void SAL_CALL SwXFilterOptions::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    ::comphelper::NamedValueCollection aProperties(rArguments);
    if (aProperties.has("ParentWindow"))
        aProperties.get("ParentWindow") >>= xDialogParent;
}

OUString SwXFilterOptions::getImplementationName()
{
    return "com.sun.star.comp.Writer.FilterOptionsDialog";
}

sal_Bool SwXFilterOptions::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXFilterOptions::getSupportedServiceNames()
{
    OUString sService("com.sun.star.ui.dialogs.FilterOptionsDialog");
    return uno::Sequence< OUString> (&sService, 1);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_Writer_FilterOptionsDialog_get_implementation(css::uno::XComponentContext*,
                                css::uno::Sequence<css::uno::Any> const &)
{
    SolarMutexGuard aGuard;

    //the module may not be loaded
    SwGlobals::ensure();
    return cppu::acquire(new SwXFilterOptions());

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
