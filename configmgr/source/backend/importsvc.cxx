/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: importsvc.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_configmgr.hxx"

#include "importsvc.hxx"
#include "importmergehandler.hxx"
#include "backendfactory.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include <com/sun/star/beans/NamedValue.hpp>
// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace backend
    {
// -----------------------------------------------------------------------------
        namespace uno   = ::com::sun::star::uno;
        namespace lang  = ::com::sun::star::lang;
        namespace beans = ::com::sun::star::beans;
        namespace backenduno = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

sal_Char const * const aMergeImporterServices[] =
{
    "com.sun.star.configuration.backend.MergeImporter",
    0,
    "com.sun.star.configuration.backend.Importer",
    0
};
const ServiceImplementationInfo aMergeImporterSI =
{
    "com.sun.star.comp.configuration.backend.MergeImporter",
    aMergeImporterServices, aMergeImporterServices + 2
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getMergeImportServiceInfo()
{ return getRegistrationInfo(& aMergeImporterSI); }
// -----------------------------------------------------------------------------

MergeImportService::MergeImportService(uno::Reference< uno::XComponentContext > const & _xContext)
: ImportService(_xContext, &aMergeImporterSI)
{
}
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateMergeImporter
( uno::Reference< uno::XComponentContext > const& xContext )
{
    return * new MergeImportService( xContext );
}
// -----------------------------------------------------------------------------

uno::Reference< backenduno::XLayerHandler > MergeImportService::createImportHandler(uno::Reference< backenduno::XBackend > const & xBackend, rtl::OUString const & aEntity)
{
    if (!xBackend.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import without a backend. No default backend could be created") );
        throw lang::NullPointerException(sMessage,*this);
    }

    uno::Reference< backenduno::XLayerHandler > aHandler( new ImportMergeHandler(xBackend, ImportMergeHandler::merge, aEntity, m_bSendNotification) );

    return aHandler;
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

sal_Char const * const aCopyImporterServices[] =
{
    "com.sun.star.configuration.backend.CopyImporter",
    0,
    "com.sun.star.configuration.backend.Importer",
    0
};
const ServiceImplementationInfo aCopyImporterSI =
{
    "com.sun.star.comp.configuration.backend.CopyImporter",
    aCopyImporterServices, aCopyImporterServices + 2
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getCopyImportServiceInfo()
{ return getRegistrationInfo(& aCopyImporterSI); }
// -----------------------------------------------------------------------------

CopyImportService::CopyImportService(uno::Reference< uno::XComponentContext > const & _xContext)
: ImportService(_xContext, &aCopyImporterSI)
{
}
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateCopyImporter
( uno::Reference< uno::XComponentContext > const& xContext )
{
    return * new CopyImportService( xContext );
}
// -----------------------------------------------------------------------------

uno::Reference< backenduno::XLayerHandler > CopyImportService::createImportHandler(uno::Reference< backenduno::XBackend > const & xBackend, rtl::OUString const & aEntity)
{
    if (!xBackend.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import without a backend. No default backend could be created") );
        throw lang::NullPointerException(sMessage,*this);
    }

    ImportMergeHandler::Mode aMode = m_bOverwrite ? ImportMergeHandler::copy : ImportMergeHandler::no_overwrite;
    uno::Reference< backenduno::XLayerHandler > aHandler( new ImportMergeHandler(xBackend,aMode,aEntity) );

    return aHandler;
}
// -----------------------------------------------------------------------------

sal_Bool CopyImportService::setImplementationProperty(rtl::OUString const & aName, uno::Any const & aValue)
{
    if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Overwrite")))
    {
        return (aValue >>= m_bOverwrite);
    }

    return ImportService::setImplementationProperty(aName,aValue);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

ImportService::ImportService(uno::Reference< uno::XComponentContext > const & _xContext, ServiceInfoHelper const & aSvcInfo )
: m_bSendNotification(false)
, m_aMutex()
, m_xContext(_xContext)
, m_xDestinationBackend()
, m_aServiceInfo(aSvcInfo)
{
    if (!m_xContext.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Configuration Importer: Unexpected NULL context"));
        throw lang::NullPointerException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

ImportService::~ImportService()
{}
// -----------------------------------------------------------------------------

uno::Reference< backenduno::XBackend > ImportService::createDefaultBackend() const
{
    return BackendFactory::instance( m_xContext ).getUnoBackend();
}
// -----------------------------------------------------------------------------

sal_Bool ImportService::setImplementationProperty(rtl::OUString const & aName, uno::Any const & aValue)
{
    if (aName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Notify")))
    {
        return (aValue >>= m_bSendNotification);
    }

    return false;
}
// -----------------------------------------------------------------------------

// XInitialize

void SAL_CALL
    ImportService::initialize( const uno::Sequence< uno::Any >& aArguments )
        throw (uno::Exception, uno::RuntimeException)
{
    sal_Int16 const nCount = static_cast<sal_Int16>(aArguments.getLength());

    if (sal_Int32(nCount) != aArguments.getLength())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Too many arguments to initialize a Configuration Importer"));
        throw lang::IllegalArgumentException(sMessage,*this,0);
    }

    for (sal_Int16 i = 0; i < nCount; ++i)
    {
        beans::NamedValue aExtraArg;
        if (aArguments[i] >>= aExtraArg)
        {
            OSL_VERIFY( setImplementationProperty(aExtraArg.Name, aExtraArg.Value) );

            continue;
        }

        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("Cannot use argument to initialize a Configuration Importer"
                                                        "- NamedValue expected"));
        throw lang::IllegalArgumentException(sMessage,*this,i+1);
    }
}

// -----------------------------------------------------------------------------

// XLayerImporter

uno::Reference< backenduno::XBackend > SAL_CALL
    ImportService::getTargetBackend(  )
        throw (uno::RuntimeException)
{
    uno::Reference< backenduno::XBackend > xRet;
    {
        osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xDestinationBackend;
    }

    if (!xRet.is())
    {
        xRet = createDefaultBackend();

        osl::MutexGuard aGuard(m_aMutex);
        if (!m_xDestinationBackend.is())
            m_xDestinationBackend = xRet;
    }
    return xRet;
}
// -----------------------------------------------------------------------------

void SAL_CALL
    ImportService::setTargetBackend( const uno::Reference< backenduno::XBackend >& aBackend )
        throw (lang::NullPointerException, uno::RuntimeException)
{
    if (!aBackend.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to set a NULL backend") );
        throw lang::NullPointerException(sMessage,*this);
    }

    osl::MutexGuard aGuard(m_aMutex);
    m_xDestinationBackend = aBackend;
}
// -----------------------------------------------------------------------------

void SAL_CALL
    ImportService::importLayer( const uno::Reference< backenduno::XLayer >& aLayer )
        throw ( backenduno::MalformedDataException,
                lang::WrappedTargetException, lang::IllegalArgumentException,
                lang::NullPointerException, uno::RuntimeException)
{
    if (!aLayer.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import a NULL layer") );
        throw lang::NullPointerException(sMessage,*this);
    }

    uno::Reference< backenduno::XLayerHandler > aInputHandler = createImportHandler( getTargetBackend() );
    aLayer->readData( aInputHandler );

}
// -----------------------------------------------------------------------------

void SAL_CALL
    ImportService::importLayerForEntity( const uno::Reference< backenduno::XLayer >& aLayer, const rtl::OUString& aEntity )
        throw ( backenduno::MalformedDataException,
                lang::WrappedTargetException, lang::IllegalArgumentException,
                lang::NullPointerException, uno::RuntimeException)
{
    if (!aLayer.is())
    {
        rtl::OUString sMessage( RTL_CONSTASCII_USTRINGPARAM("configmgr::backend::ImportService: Trying to import a NULL layer") );
        throw lang::NullPointerException(sMessage,*this);
    }

    uno::Reference< backenduno::XLayerHandler > aInputHandler = createImportHandler( getTargetBackend(), aEntity );
    aLayer->readData( aInputHandler );
}
// -----------------------------------------------------------------------------

// XServiceInfo

rtl::OUString SAL_CALL
    ImportService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------


sal_Bool SAL_CALL
    ImportService::supportsService( const rtl::OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------


uno::Sequence< ::rtl::OUString > SAL_CALL
    ImportService::getSupportedServiceNames(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getSupportedServiceNames( );
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
    } // namespace

// -----------------------------------------------------------------------------
} // namespace

