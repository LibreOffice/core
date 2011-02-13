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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

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
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( SERVICE_NAME ) );
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
                    rtl::OUStringBuffer sBuf( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Your Presentation has been minimized from:")) );
                    sBuf.append( rtl::OUString::valueOf( nFileSizeSource >> 10 ) );
                    sBuf.append( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KB to ")) );
                    sBuf.append( rtl::OUString::valueOf( nFileSizeDest >> 10 ) );
                    sBuf.append( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KB.")) );
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
    // OSL_ENSURE( sal_False, "PPPOptimizerDialog::addStatusListener()\nNot implemented yet!" );
}

//===============================================
void SAL_CALL PPPOptimizerDialog::removeStatusListener( const Reference< XStatusListener >&, const URL& )
    throw( RuntimeException )
{
    // TODO
    // OSL_ENSURE( sal_False, "PPPOptimizerDialog::removeStatusListener()\nNot implemented yet!" );
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
    pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
    return aRet;
}

Reference< XInterface > PPPOptimizerDialog_createInstance( const Reference< XComponentContext > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new PPPOptimizerDialog( rSMgr );
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
