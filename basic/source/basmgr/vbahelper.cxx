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
#include "precompiled_basic.hxx"

#include "basic/vbahelper.hxx"
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

namespace basic {
namespace vba {

using namespace ::com::sun::star;

// ============================================================================

namespace {

/** Creates the global module manager needed to identify the type of documents.
 */
uno::Reference< frame::XModuleManager > lclCreateModuleManager()
{
    uno::Reference< frame::XModuleManager > xModuleManager;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );
        xModuleManager.set( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ModuleManager" ) ) ), uno::UNO_QUERY );
    }
    catch( uno::Exception& )
    {
    }
    OSL_ENSURE( xModuleManager.is(), "::basic::vba::lclCreateModuleManager - cannot create module manager" );
    return xModuleManager;
}

// ----------------------------------------------------------------------------

/** Returns the document service name of the specified document.
 */
::rtl::OUString lclIdentifyDocument( const uno::Reference< frame::XModuleManager >& rxModuleManager, const uno::Reference< frame::XModel >& rxModel )
{
    ::rtl::OUString aServiceName;
    if( rxModuleManager.is() )
    {
        try
        {
            aServiceName = rxModuleManager->identify( rxModel );
        }
        catch( uno::Exception& )
        {
        }
        OSL_ENSURE( aServiceName.getLength() > 0, "::basic::vba::lclIdentifyDocument - cannot identify document" );
    }
    return aServiceName;
}

// ----------------------------------------------------------------------------

/** Returns an enumeration of all open documents.
 */
uno::Reference< container::XEnumeration > lclCreateDocumentEnumeration()
{
    uno::Reference< container::XEnumeration > xEnumeration;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );
        uno::Reference< frame::XDesktop > xDesktop( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), uno::UNO_QUERY_THROW );
        uno::Reference< container::XEnumerationAccess > xComponentsEA( xDesktop->getComponents(), uno::UNO_SET_THROW );
        xEnumeration = xComponentsEA->createEnumeration();

    }
    catch( uno::Exception& )
    {
    }
    OSL_ENSURE( xEnumeration.is(), "::basic::vba::lclCreateDocumentEnumeration - cannot create enumeration of all documents" );
    return xEnumeration;
}

// ----------------------------------------------------------------------------

/** Locks or unlocks the controllers of the specified document model.
 */
void lclLockControllers( const uno::Reference< frame::XModel >& rxModel, sal_Bool bLockControllers )
{
    if( rxModel.is() ) try
    {
        if( bLockControllers )
            rxModel->lockControllers();
        else
            rxModel->unlockControllers();
    }
    catch( uno::Exception& )
    {
    }
}

// ----------------------------------------------------------------------------

/** Enables or disables the container windows of all controllers of the
    specified document model.
 */
void lclEnableContainerWindows( const uno::Reference< frame::XModel >& rxModel, sal_Bool bEnableWindows )
{
    try
    {
        uno::Reference< frame::XModel2 > xModel2( rxModel, uno::UNO_QUERY_THROW );
        uno::Reference< container::XEnumeration > xControllersEnum( xModel2->getControllers(), uno::UNO_SET_THROW );
        // iterate over all controllers
        while( xControllersEnum->hasMoreElements() )
        {
            try
            {
                uno::Reference< frame::XController > xController( xControllersEnum->nextElement(), uno::UNO_QUERY_THROW );
                uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW );
                uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow(), uno::UNO_SET_THROW );
                xWindow->setEnable( bEnableWindows );
            }
            catch( uno::Exception& )
            {
            }
        }
    }
    catch( uno::Exception& )
    {
    }
}

// ----------------------------------------------------------------------------

typedef void (*ModifyDocumentFunc)( const uno::Reference< frame::XModel >&, sal_Bool );

/** Implementation iterating over all documents that have the same type as the
    specified model, and calling the passed functor.
 */
void lclIterateDocuments( ModifyDocumentFunc pModifyDocumentFunc, const uno::Reference< frame::XModel >& rxModel, sal_Bool bModificator )
{
    uno::Reference< frame::XModuleManager > xModuleManager = lclCreateModuleManager();
    uno::Reference< container::XEnumeration > xDocumentsEnum = lclCreateDocumentEnumeration();
    ::rtl::OUString aIdentifier = lclIdentifyDocument( xModuleManager, rxModel );
    if( xModuleManager.is() && xDocumentsEnum.is() && (aIdentifier.getLength() > 0) )
    {
        // iterate over all open documents
        while( xDocumentsEnum->hasMoreElements() )
        {
            try
            {
                uno::Reference< frame::XModel > xCurrModel( xDocumentsEnum->nextElement(), uno::UNO_QUERY_THROW );
                ::rtl::OUString aCurrIdentifier = lclIdentifyDocument( xModuleManager, xCurrModel );
                if( aCurrIdentifier == aIdentifier )
                    pModifyDocumentFunc( xCurrModel, bModificator );
            }
            catch( uno::Exception& )
            {
            }
        }
    }
    else
    {
        // no module manager, no documents enumeration, no identifier -> at least process the passed document
        pModifyDocumentFunc( rxModel, bModificator );
    }
}

} // namespace

// ============================================================================

void lockControllersOfAllDocuments( const uno::Reference< frame::XModel >& rxModel, sal_Bool bLockControllers )
{
    lclIterateDocuments( &lclLockControllers, rxModel, bLockControllers );
}

// ============================================================================

void enableContainerWindowsOfAllDocuments( const uno::Reference< frame::XModel >& rxModel, sal_Bool bEnableWindows )
{
    lclIterateDocuments( &lclEnableContainerWindows, rxModel, bEnableWindows );
}

// ============================================================================

} // namespace vba
} // namespace basic
