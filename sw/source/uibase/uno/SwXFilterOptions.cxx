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
#include <vcl/svapp.hxx>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotxdoc.hxx>

#include <swabstdlg.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::lang;

constexpr OUString FILTER_OPTIONS_NAME = u"FilterOptions"_ustr;

SwXFilterOptions::SwXFilterOptions()
{
}

SwXFilterOptions::~SwXFilterOptions()
{
}

uno::Sequence< beans::PropertyValue > SwXFilterOptions::getPropertyValues()
{
    return comphelper::InitPropertySequence({
            { FILTER_OPTIONS_NAME, uno::Any(m_sFilterOptions) }
        });
}

void   SwXFilterOptions::setPropertyValues( const uno::Sequence<beans::PropertyValue >& aProps )
{
    for (const beans::PropertyValue& rProp : aProps)
    {
        OUString aPropName = rProp.Name;

        if ( aPropName == FILTER_OPTIONS_NAME )
            rProp.Value >>= m_sFilterOptions;
        else if ( aPropName == "InputStream" )
            rProp.Value >>= m_xInputStream;
    }
}

void   SwXFilterOptions::setTitle( const OUString& /*rTitle*/ )
{
}

sal_Int16 SwXFilterOptions::execute()
{
    sal_Int16 nRet = ui::dialogs::ExecutableDialogResults::CANCEL;

    std::unique_ptr<SvStream> pInStream;
    if ( m_xInputStream.is() )
        pInStream = utl::UcbStreamHelper::CreateStream( m_xInputStream );

    SwDocShell* pDocShell = nullptr;
    if (auto pXDoc = comphelper::getFromUnoTunnel<SwXTextDocument>(m_xModel); pXDoc)
        pDocShell = pXDoc->GetDocShell();

    if(pDocShell)
    {
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSwAsciiFilterDlg> pAsciiDlg(pFact->CreateSwAsciiFilterDlg(Application::GetFrameWeld(m_xDialogParent), *pDocShell,
            pInStream.get()));
        if(RET_OK == pAsciiDlg->Execute())
        {
            SwAsciiOptions aOptions;
            pAsciiDlg->FillOptions( aOptions );
            aOptions.WriteUserData(m_sFilterOptions);
            nRet = ui::dialogs::ExecutableDialogResults::OK;
        }
    }

    return nRet;
}

void   SwXFilterOptions::setTargetDocument( const uno::Reference< XComponent >& xDoc )
{
    m_xModel = xDoc;
}

void   SwXFilterOptions::setSourceDocument( const uno::Reference<XComponent >& xDoc )
{
    m_xModel = xDoc;
}

void SAL_CALL SwXFilterOptions::initialize(const uno::Sequence<uno::Any>& rArguments)
{
    ::comphelper::NamedValueCollection aProperties(rArguments);
    if (aProperties.has(u"ParentWindow"_ustr))
        aProperties.get(u"ParentWindow"_ustr) >>= m_xDialogParent;
}

OUString SwXFilterOptions::getImplementationName()
{
    return u"com.sun.star.comp.Writer.FilterOptionsDialog"_ustr;
}

sal_Bool SwXFilterOptions::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXFilterOptions::getSupportedServiceNames()
{
    return { u"com.sun.star.ui.dialogs.FilterOptionsDialog"_ustr };
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
