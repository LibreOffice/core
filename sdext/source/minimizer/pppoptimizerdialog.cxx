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


#include "pppoptimizerdialog.hxx"
#include "optimizerdialog.hxx"
#include <sal/log.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

PPPOptimizerDialog::PPPOptimizerDialog( const Reference< XComponentContext > &xContext ) :
    mxContext( xContext ),
    mpOptimizerDialog( nullptr )
{
}

PPPOptimizerDialog::~PPPOptimizerDialog()
{
}

void SAL_CALL PPPOptimizerDialog::initialize( const Sequence< Any >& aArguments )
{
    if( aArguments.getLength() != 1 )
        throw IllegalArgumentException();

    aArguments[ 0 ] >>= mxFrame;
    if ( mxFrame.is() )
        mxController = mxFrame->getController();
}

OUString SAL_CALL PPPOptimizerDialog::getImplementationName()
{
    return u"com.sun.star.comp.PresentationMinimizerImp"_ustr;
}

sal_Bool SAL_CALL PPPOptimizerDialog::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL PPPOptimizerDialog::getSupportedServiceNames()
{
    return { u"com.sun.star.comp.PresentationMinimizer"_ustr };
}

Reference< css::frame::XDispatch > SAL_CALL PPPOptimizerDialog::queryDispatch(
    const URL& aURL, const OUString& /* aTargetFrameName */, sal_Int32 /* nSearchFlags */ )
{
    Reference < XDispatch > xRet;
    if ( aURL.Protocol.equalsIgnoreAsciiCase( "vnd.com.sun.star.comp.PresentationMinimizer:" ) )
        xRet = this;

    return xRet;
}

Sequence< Reference< css::frame::XDispatch > > SAL_CALL PPPOptimizerDialog::queryDispatches(
    const Sequence< css::frame::DispatchDescriptor >& aDescripts )
{
    Sequence< Reference< css::frame::XDispatch> > aReturn( aDescripts.getLength() );
    std::transform(aDescripts.begin(), aDescripts.end(), aReturn.getArray(),
        [this](const css::frame::DispatchDescriptor& rDescr) -> Reference<css::frame::XDispatch> {
            return queryDispatch(rDescr.FeatureURL, rDescr.FrameName, rDescr.SearchFlags); });
    return aReturn;
}

void SAL_CALL PPPOptimizerDialog::dispatch( const URL& rURL,
                                            const Sequence< PropertyValue >& rArguments )
{

    if ( !(mxController.is() && rURL.Protocol.equalsIgnoreAsciiCase( "vnd.com.sun.star.comp.PresentationMinimizer:" )) )
        return;

    if ( rURL.Path == "execute" )
    {
        try
        {
            sal_Int64 nFileSizeSource = 0;
            sal_Int64 nFileSizeDest = 0;
            mpOptimizerDialog = new OptimizerDialog( mxContext, mxFrame, this );
            mpOptimizerDialog->execute();

            const Any* pVal( mpOptimizerDialog->maStats.GetStatusValue( TK_FileSizeSource ) );
            if ( pVal )
                *pVal >>= nFileSizeSource;
            pVal = mpOptimizerDialog->maStats.GetStatusValue( TK_FileSizeDestination );
            if ( pVal )
                *pVal >>= nFileSizeDest;

            if ( nFileSizeSource && nFileSizeDest )
            {
                OUString sResult = "Your Presentation has been minimized from:" +
                    OUString::number( nFileSizeSource >> 10 ) +
                    "KB to " +
                    OUString::number( nFileSizeDest >> 10 ) +
                    "KB.";
                SAL_INFO("sdext.minimizer", sResult );
            }
        }
        catch( ... )
        {
        }
        delete mpOptimizerDialog;
        mpOptimizerDialog = nullptr;
    }
    else if ( rURL.Path == "statusupdate" )
    {
        if ( mpOptimizerDialog )
            mpOptimizerDialog->UpdateStatus( rArguments );
    }
}

void SAL_CALL PPPOptimizerDialog::addStatusListener( const Reference< XStatusListener >&, const URL& )
{
    // TODO
    // OSL_FAIL( "PPPOptimizerDialog::addStatusListener()\nNot implemented yet!" );
}

void SAL_CALL PPPOptimizerDialog::removeStatusListener( const Reference< XStatusListener >&, const URL& )
{
    // TODO
    // OSL_FAIL( "PPPOptimizerDialog::removeStatusListener()\nNot implemented yet!" );
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sdext_PPPOptimizerDialog_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new PPPOptimizerDialog(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
