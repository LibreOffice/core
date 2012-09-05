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


#include <basic/vbahelper.hxx>

#include <map>
#include <vector>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <rtl/instance.hxx>

namespace basic {
namespace vba {

using namespace ::com::sun::star;

// ============================================================================

namespace {

/** Create an instance of a module manager.
 */
uno::Reference< frame::XModuleManager2 > lclCreateModuleManager()
{
    uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext(), uno::UNO_QUERY_THROW );
    return frame::ModuleManager::create(xContext);
}

// ----------------------------------------------------------------------------

/** Implementation of an enumeration of all open documents of the same type.
 */
class DocumentsEnumeration : public ::cppu::WeakImplHelper1< container::XEnumeration >
{
public:
    DocumentsEnumeration( const uno::Reference< frame::XModel >& rxModel );
    virtual sal_Bool SAL_CALL hasMoreElements() throw (uno::RuntimeException);
    virtual uno::Any SAL_CALL nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException);
private:
    typedef ::std::vector< uno::Reference< frame::XModel > > ModelVector;
    ModelVector maModels;
    ModelVector::iterator maModelIt;
};

DocumentsEnumeration::DocumentsEnumeration( const uno::Reference< frame::XModel >& rxModel )
{
    try
    {
        uno::Reference< frame::XModuleManager2 > xModuleManager( lclCreateModuleManager() );
        ::rtl::OUString aIdentifier = xModuleManager->identify( rxModel );
        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        uno::Reference< frame::XDesktop > xDesktop( xFactory->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.Desktop" ) ) ), uno::UNO_QUERY_THROW );
        uno::Reference< container::XEnumerationAccess > xComponentsEA( xDesktop->getComponents(), uno::UNO_SET_THROW );
        uno::Reference< container::XEnumeration > xEnumeration( xComponentsEA->createEnumeration(), uno::UNO_SET_THROW );
        while( xEnumeration->hasMoreElements() )
        {
            uno::Reference< frame::XModel > xCurrModel( xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
            if( xModuleManager->identify( xCurrModel ) == aIdentifier )
                maModels.push_back( xCurrModel );
        }
    }
    catch(const uno::Exception& )
    {
    }
    maModelIt = maModels.begin();
}

sal_Bool SAL_CALL DocumentsEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    return maModelIt != maModels.end();
}

uno::Any SAL_CALL DocumentsEnumeration::nextElement() throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( maModelIt == maModels.end() )
        throw container::NoSuchElementException();
    return uno::Any( *maModelIt++ );
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
    catch(const uno::Exception& )
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
            catch(const uno::Exception& )
            {
            }
        }
    }
    catch(const uno::Exception& )
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
    uno::Reference< container::XEnumeration > xDocumentsEnum( new DocumentsEnumeration( rxModel ) );
    // iterate over all open documents
    while( xDocumentsEnum->hasMoreElements() ) try
    {
        uno::Reference< frame::XModel > xCurrModel( xDocumentsEnum->nextElement(), uno::UNO_QUERY_THROW );
        pModifyDocumentFunc( xCurrModel, bModificator );
    }
    catch(const uno::Exception& )
    {
    }
}

// ----------------------------------------------------------------------------

struct CurrDirPool
{
    ::osl::Mutex maMutex;
    ::std::map< ::rtl::OUString, ::rtl::OUString > maCurrDirs;
};

struct StaticCurrDirPool : public ::rtl::Static< CurrDirPool, StaticCurrDirPool > {};

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

void registerCurrentDirectory( const uno::Reference< frame::XModel >& rxModel, const ::rtl::OUString& rPath )
{
    if( !rPath.isEmpty() )
    {
        CurrDirPool& rPool = StaticCurrDirPool::get();
        ::osl::MutexGuard aGuard( rPool.maMutex );
        try
        {
            uno::Reference< frame::XModuleManager2 > xModuleManager( lclCreateModuleManager() );
            ::rtl::OUString aIdentifier = xModuleManager->identify( rxModel );
            if( !aIdentifier.isEmpty() )
                rPool.maCurrDirs[ aIdentifier ] = rPath;
        }
        catch(const uno::Exception& )
        {
        }
    }
}

// ============================================================================

} // namespace vba
} // namespace basic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
