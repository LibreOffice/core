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

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

#define SERVICE_NAME "com.sun.star.comp.SunPresentationMinimizer"
#include <rtl/ustrbuf.hxx>

// ----------------------
// - PPPOptimizerDialog -
// ----------------------

PPPOptimizerDialog::PPPOptimizerDialog( const Reference< XComponentContext > &rxMSF ) :
    mxMSF( rxMSF ),
    mpOptimizerDialog( NULL )
{
}

// -----------------------------------------------------------------------------

PPPOptimizerDialog::~PPPOptimizerDialog()
{
}

// -----------------------------------------------------------------------------
// XInitialization
// -----------------------------------------------------------------------------

void SAL_CALL PPPOptimizerDialog::initialize( const Sequence< Any >& aArguments )
    throw ( Exception, RuntimeException )
{
    if( aArguments.getLength() != 1 )
        throw IllegalArgumentException();

    aArguments[ 0 ] >>= mxFrame;
    if ( mxFrame.is() )
        mxController = mxFrame->getController();
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString SAL_CALL PPPOptimizerDialog::getImplementationName()
    throw (RuntimeException)
{
    return PPPOptimizerDialog_getImplementationName();
}

sal_Bool SAL_CALL PPPOptimizerDialog::supportsService( const OUString& ServiceName )
    throw ( RuntimeException )
{
    return ServiceName == SERVICE_NAME;
}

Sequence< OUString > SAL_CALL PPPOptimizerDialog::getSupportedServiceNames()
    throw (RuntimeException)
{
    return PPPOptimizerDialog_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------
// XDispatchProvider
// -----------------------------------------------------------------------------

Reference< com::sun::star::frame::XDispatch > SAL_CALL PPPOptimizerDialog::queryDispatch(
    const URL& aURL, const ::rtl::OUString& /* aTargetFrameName */, sal_Int32 /* nSearchFlags */ ) throw( RuntimeException )
{
    Reference < XDispatch > xRet;
    if ( aURL.Protocol.compareToAscii( "vnd.com.sun.star.comp.SunPresentationMinimizer:" ) == 0 )
        xRet = this;

    return xRet;
}

//------------------------------------------------------------------------------

Sequence< Reference< com::sun::star::frame::XDispatch > > SAL_CALL PPPOptimizerDialog::queryDispatches(
    const Sequence< com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw( RuntimeException )
{
    Sequence< Reference< com::sun::star::frame::XDispatch> > aReturn( aDescripts.getLength() );
    Reference< com::sun::star::frame::XDispatch>* pReturn = aReturn.getArray();
    const com::sun::star::frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_Int16 i = 0; i < aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}

// -----------------------------------------------------------------------------
// XDispatch
// -----------------------------------------------------------------------------

void SAL_CALL PPPOptimizerDialog::dispatch( const URL& rURL,
                                            const Sequence< PropertyValue >& rArguments )
    throw( RuntimeException )
{

    if ( mxController.is() && ( rURL.Protocol.compareToAscii( "vnd.com.sun.star.comp.SunPresentationMinimizer:" ) == 0 ) )
    {
        if ( rURL.Path.compareToAscii( "execute" ) == 0 )
        {
            try
            {
                sal_Int64 nFileSizeSource = 0;
                sal_Int64 nFileSizeDest = 0;
                mpOptimizerDialog = new OptimizerDialog( mxMSF, mxFrame, this );
                mpOptimizerDialog->execute();

                const Any* pVal( mpOptimizerDialog->maStats.GetStatusValue( TK_FileSizeSource ) );
                if ( pVal )
                    *pVal >>= nFileSizeSource;
                pVal = mpOptimizerDialog->maStats.GetStatusValue( TK_FileSizeDestination );
                if ( pVal )
                    *pVal >>= nFileSizeDest;

                if ( nFileSizeSource && nFileSizeDest )
                {
                    rtl::OUStringBuffer sBuf( rtl::OUString("Your Presentation has been minimized from:") );
                    sBuf.append( rtl::OUString::valueOf( nFileSizeSource >> 10 ) );
                    sBuf.append( rtl::OUString("KB to ") );
                    sBuf.append( rtl::OUString::valueOf( nFileSizeDest >> 10 ) );
                    sBuf.append( rtl::OUString("KB.") );
                    OUString sResult( sBuf.makeStringAndClear() );
//                  mpOptimizerDialog->showMessageBox( sResult, sResult, sal_False );
                }
                delete mpOptimizerDialog, mpOptimizerDialog = NULL;
            }
            catch( ... )
            {

            }
        }
        else if ( rURL.Path.compareToAscii( "statusupdate" ) == 0 )
        {
            if ( mpOptimizerDialog )
                mpOptimizerDialog->UpdateStatus( rArguments );
        }
    }
}

//===============================================
void SAL_CALL PPPOptimizerDialog::addStatusListener( const Reference< XStatusListener >&, const URL& )
    throw( RuntimeException )
{
    // TODO
    // OSL_FAIL( "PPPOptimizerDialog::addStatusListener()\nNot implemented yet!" );
}

//===============================================
void SAL_CALL PPPOptimizerDialog::removeStatusListener( const Reference< XStatusListener >&, const URL& )
    throw( RuntimeException )
{
    // TODO
    // OSL_FAIL( "PPPOptimizerDialog::removeStatusListener()\nNot implemented yet!" );
}

// -----------------------------------------------------------------------------

OUString PPPOptimizerDialog_getImplementationName()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.SunPresentationMinimizerImp" ) );
}

Sequence< OUString > PPPOptimizerDialog_getSupportedServiceNames()
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME  );
    return aRet;
}

Reference< XInterface > PPPOptimizerDialog_createInstance( const Reference< XComponentContext > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new PPPOptimizerDialog( rSMgr );
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
