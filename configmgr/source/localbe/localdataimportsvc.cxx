/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localdataimportsvc.cxx,v $
 * $Revision: 1.9 $
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

#include "localdataimportsvc.hxx"
#include "localsinglebackend.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/configuration/backend/XLayerImporter.hpp>

// -----------------------------------------------------------------------------
#define OUSTRING( constascii ) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(constascii))
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace localbe
    {
// -----------------------------------------------------------------------------
        namespace backend = ::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

sal_Char const * const aLocalDataImportServices[] =
{
    "com.sun.star.configuration.backend.LocalDataImporter",
    0,
    "com.sun.star.configuration.backend.DataImporter",
    0
};
const ServiceImplementationInfo aLocalDataImportSI =
{
    "com.sun.star.comp.configuration.backend.LocalDataImporter",
    aLocalDataImportServices,
    aLocalDataImportServices + 3
};
// -----------------------------------------------------------------------------

const ServiceRegistrationInfo* getLocalDataImportServiceInfo()
{ return getRegistrationInfo(& aLocalDataImportSI); }
// -----------------------------------------------------------------------------

inline
ServiceInfoHelper LocalDataImportService::getServiceInfo()
{
    return & aLocalDataImportSI;
}
// -----------------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL instantiateLocalDataImporter
( uno::Reference< uno::XComponentContext > const& xContext )
{
    return * new LocalDataImportService( xContext );
}
// -----------------------------------------------------------------------------

LocalDataImportService::LocalDataImportService(uno::Reference< uno::XComponentContext > const & _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
{
    if (!m_xServiceFactory.is())
    {
        rtl::OUString sMessage = OUSTRING("Configuration Importer: Context has no service manager (or interface is missing)");
        throw lang::NullPointerException(sMessage,NULL);
    }
}
// -----------------------------------------------------------------------------

LocalDataImportService::~LocalDataImportService()
{}
// -----------------------------------------------------------------------------

namespace
{
// -----------------------------------------------------------------------------
    struct JobDesc
    {
        explicit JobDesc(task::XJob * pJob, const uno::Sequence< beans::NamedValue >& aArguments);

        rtl::OUString aLayerDataUrl;
        rtl::OUString aImporterService;

        rtl::OUString aComponent;
        rtl::OUString aEntity;

        uno::Reference< backend::XLayer > xLayerFilter;

        sal_Bool overwrite;
        sal_Bool truncate;

        sal_Bool use_component;
        sal_Bool use_entity;
        sal_Bool use_overwrite;
        sal_Bool use_truncate;
    };
    // -----------------------------------------------------------------------------

    JobDesc::JobDesc(task::XJob * pJob, const uno::Sequence< beans::NamedValue >& aArguments)
    : aLayerDataUrl()
    , aImporterService()
    , aComponent()
    , aEntity()
    , xLayerFilter()
    , overwrite(true)
    , truncate(false)
    , use_component(false)
    , use_entity(false)
    , use_overwrite(false)
    , use_truncate(false)
    {
        sal_Int16 const nCount = static_cast<sal_Int16>(aArguments.getLength());

        if (sal_Int32(nCount) != aArguments.getLength())
        {
            rtl::OUString sMessage = OUSTRING("Too many arguments for LocalDataImporter Job");
            throw lang::IllegalArgumentException(sMessage,pJob,0);
        }

        for (sal_Int16 i=0; i < nCount; ++i)
        {
            sal_Bool bKnown = false;
            sal_Bool bGood  = false;

            if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("LayerDataUrl")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= aLayerDataUrl);
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ImporterService")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= aImporterService);
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Component")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= aComponent);
                use_component = bGood && (aComponent.getLength() != 0);
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Entity")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= aEntity);
                use_entity = bGood && (aEntity.getLength() != 0);
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("LayerFilter")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= xLayerFilter);
                if (xLayerFilter.is() && !uno::Reference<lang::XInitialization>::query(xLayerFilter).is())
                    bGood = false;
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("OverwriteExisting")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= overwrite);
                use_overwrite = bGood;
            }
            else if (aArguments[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("TruncateExisting")))
            {
                bKnown = true;
                bGood  = (aArguments[i].Value >>= truncate);
                use_truncate = bGood;
            }

            if (!bGood)
            {
                rtl::OUStringBuffer sMsg;
                sMsg.appendAscii("LocalDataImportService - Illegal argument: ");
                if (bKnown)
                    sMsg.appendAscii("Wrong value type for argument '");
                else
                    sMsg.appendAscii("Unknown argument '");

                sMsg.append(aArguments[i].Name).appendAscii("'.");

                throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),pJob,i+1);
            }
        }
        if (aLayerDataUrl.getLength() == 0)
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("LocalDataImportService - Missing argument: ");
            sMsg.appendAscii("No data URL available");
            throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),pJob,0);
        }
        if (aImporterService.getLength() == 0)
        {
            if ( (use_truncate && truncate) || (use_overwrite && !overwrite) )
                aImporterService = OUSTRING("com.sun.star.configuration.backend.CopyImporter");
            else
                aImporterService = OUSTRING("com.sun.star.configuration.backend.MergeImporter");
        }
    }
    // -----------------------------------------------------------------------------

    static
    inline
    uno::Type getOverwriteFailedExceptionType()
    {
        lang::IllegalAccessException const * const selected = 0;
        return ::getCppuType(selected);
    }
// -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
// XJob

uno::Any SAL_CALL
    LocalDataImportService::execute( const uno::Sequence< beans::NamedValue >& Arguments )
        throw (lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    JobDesc const aJob(this,Arguments);

    uno::Reference< lang::XMultiServiceFactory > aFactory = this->getServiceFactory();

    uno::Reference< backend::XLayer > xLayer = aJob.use_component ?
        LocalSingleBackend::createSimpleLayer(aFactory,aJob.aLayerDataUrl, aJob.aComponent ):
        LocalSingleBackend::createSimpleLayer(aFactory,aJob.aLayerDataUrl) ;

    if (!xLayer.is())
    {
        rtl::OUString sMessage = OUSTRING("LocalDataImportService - Cannot create layer to import from");
        throw lang::NullPointerException(sMessage,*this);
    }

    uno::Reference< lang::XInitialization > xFilterInit(aJob.xLayerFilter,uno::UNO_QUERY);
    if (xFilterInit.is())
    {
        beans::NamedValue argvalue(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Source") ),
            uno::makeAny( xLayer) );

        uno::Sequence< uno::Any > args(1);
        args[0] <<= argvalue;

        xFilterInit->initialize(args);

        xLayer = aJob.xLayerFilter;
    }

    uno::Reference< backend::XLayerImporter > xImporter;

    int nArgCount = 0;
    if (aJob.use_overwrite) ++nArgCount;
    if (aJob.use_truncate)  ++nArgCount;

    if (nArgCount)
    {
        uno::Sequence< uno::Any > aArgs(nArgCount);

        int n = 0;
        if (aJob.use_overwrite)
            aArgs[n++] <<= beans::NamedValue(OUSTRING("Overwrite"), uno::makeAny(aJob.overwrite));

        if (aJob.use_truncate)
            aArgs[n++] <<= beans::NamedValue(OUSTRING("Truncate"),  uno::makeAny(aJob.truncate));

        OSL_ASSERT(n == nArgCount);

        xImporter.set( aFactory->createInstanceWithArguments(aJob.aImporterService,aArgs), uno::UNO_QUERY);
    }
    else
        xImporter.set( aFactory->createInstance(aJob.aImporterService), uno::UNO_QUERY);

    if (!xImporter.is())
    {
        rtl::OUString sMessage = OUSTRING("LocalDataImportService - Cannot create importer service: ") + aJob.aImporterService;
        throw lang::NullPointerException(sMessage,*this);
    }

    try
    {
        if (aJob.use_entity)
            xImporter->importLayerForEntity(xLayer,aJob.aEntity);

        else
            xImporter->importLayer(xLayer);
    }
    catch (lang::WrappedTargetException & e)
    {
        if (aJob.overwrite || !e.TargetException.isExtractableTo(getOverwriteFailedExceptionType())) throw;
        return e.TargetException;
    }
    catch (lang::WrappedTargetRuntimeException & e)
    {
        if (aJob.overwrite || !e.TargetException.isExtractableTo(getOverwriteFailedExceptionType())) throw;
        return e.TargetException;
    }

    return uno::Any();
}
// -----------------------------------------------------------------------------

// XServiceInfo

rtl::OUString SAL_CALL
    LocalDataImportService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------


sal_Bool SAL_CALL
    LocalDataImportService::supportsService( const rtl::OUString& ServiceName )
        throw (uno::RuntimeException)
{
    return getServiceInfo().supportsService( ServiceName );
}
// -----------------------------------------------------------------------------


uno::Sequence< ::rtl::OUString > SAL_CALL
    LocalDataImportService::getSupportedServiceNames(  )
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

