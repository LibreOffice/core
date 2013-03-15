/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "pppoptimizerdialog.hxx"
#include "optimizerdialog.hxx"

#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <rtl/strbuf.hxx>
#include <rtl/ref.hxx>

#define SERVICE_NAME "com.sun.star.ui.dialogs.PresentationMinimizerDialog"
#define IMPLEMENTATION_NAME "com.sun.star.comp.ui.dialogs.PresentationMinimizerDialog"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

using ::rtl::OUString;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::awt::XWindowPeer;

// ----------------------
// - PPPOptimizerDialog -
// ----------------------

PPPOptimizerDialog::PPPOptimizerDialog(
    const Reference< XComponentContext > &rxContext )
    : m_xContext( rxContext )
    , mbInitialized( false )
{
    OSL_TRACE("PPPOptimizerDialog::PPPOptimizerDialog");
}

// -----------------------------------------------------------------------------

PPPOptimizerDialog::~PPPOptimizerDialog()
{
    OSL_TRACE("PPPOptimizerDialog::~PPPOptimizerDialog");
}

// -----------------------------------------------------------------------------
// XInitialization
// -----------------------------------------------------------------------------

void SAL_CALL PPPOptimizerDialog::initialize( const Sequence< Any >& aArguments )
    throw ( Exception, RuntimeException )
{
    OSL_TRACE("PPPOptimizerDialog::initialize");
    osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( mbInitialized )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "PPPOptimizerDialog has already been initialized!") ),
                    Reference< XInterface >() );
    aGuard.clear();

    Reference< XFrame > xFrame;
    Reference< XController > xController;
    Reference< XModel > xModel;
    Reference< XWindow > xWindow;

    const Any *pAny = aArguments.getConstArray();
    const Any *pEnd = pAny + aArguments.getLength();
    for ( ; pAny != pEnd && !xFrame.is() && !xWindow.is(); pAny++ )
    {
        if ( ( *pAny >>= xFrame ) && xFrame.is() )
        {
            xWindow = xFrame->getContainerWindow();
        }
        else if ( ( *pAny >>= xController ) && xController.is() )
        {
            xFrame = xController->getFrame();
            if ( xFrame.is() )
                xWindow = xFrame->getContainerWindow();
        }
        else if ( ( *pAny >>= xModel ) && xModel.is() )
        {
            xController = xModel->getCurrentController();
            if ( xController.is() )
            {
                xFrame = xController->getFrame();
                if ( xFrame.is() )
                    xWindow = xFrame->getContainerWindow();
            }
        }
        else
            *pAny >>= xWindow;
    }

    if ( !xFrame.is() )
        throw IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                "PPPOptimizerDialog must be initialized with an "
                "XFrame, XController or XModel!") ),
                    Reference< XInterface >(), 0 );

    aGuard.reset();
    mxFrame = xFrame;
    mxParentWindow.set( xWindow, UNO_QUERY );
    mbInitialized = true;
    aGuard.clear();
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


void SAL_CALL PPPOptimizerDialog::setTitle( const ::rtl::OUString& aTitle )
throw (::com::sun::star::uno::RuntimeException)
{
    osl::MutexGuard aGuard( m_aMutex );
    msTitle = aTitle;
}

::sal_Int16 SAL_CALL PPPOptimizerDialog::execute(  )
throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE("PPPOptimizerDialog::execute");
    sal_Int16 aRet = ::com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    bool bInit( mbInitialized );
    Reference< XFrame > xFrame( mxFrame );
    Reference< XWindowPeer > xParent( mxParentWindow );
    aGuard.clear();

    if ( !bInit || !xFrame.is() || !xParent.is() )
        throw RuntimeException();
    try
    {
        OptimizerDialog *pDialog(
            new OptimizerDialog( m_xContext, xFrame, xParent ) );
        pDialog->setTitle( msTitle );
        aRet = pDialog->execute();
        delete pDialog;
    }
    catch( ... )
    {
    }

    return aRet;
}

// -----------------------------------------------------------------------------

OUString PPPOptimizerDialog_getImplementationName()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
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
