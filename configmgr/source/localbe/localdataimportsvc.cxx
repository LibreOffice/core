/*************************************************************************
 *
 *  $RCSfile: localdataimportsvc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "localdataimportsvc.hxx"

#ifndef CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
#include "localsinglebackend.hxx"
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETRUNTIMEEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#endif
#include <drafts/com/sun/star/configuration/backend/XImportLayer.hpp>

// -----------------------------------------------------------------------------
#define OUSTRING( constascii ) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(constascii))
// -----------------------------------------------------------------------------
namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace localbe
    {
// -----------------------------------------------------------------------------
        namespace backend = drafts::com::sun::star::configuration::backend;
// -----------------------------------------------------------------------------

AsciiServiceName const aLocalDataImportServices[] =
{
    "com.sun.star.configuration.backend.local.DataImporter",
    "com.sun.star.configuration.backend.LocalDataImporter",
    0,
    "com.sun.star.configuration.backend.DataImporter",
    0
};
const ServiceImplementationInfo aLocalDataImportSI =
{
    "com.sun.star.comp.configuration.backend.local.DataImporter",
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
( CreationContext const& xContext )
{
    return * new LocalDataImportService( xContext );
}
// -----------------------------------------------------------------------------

LocalDataImportService::LocalDataImportService(CreationArg _xContext)
: m_xServiceFactory(_xContext->getServiceManager(), uno::UNO_QUERY)
{
    if (!m_xServiceFactory.is())
    {
        OUString sMessage = OUSTRING("Configuration Importer: Context has no service manager (or interface is missing)");
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

        OUString aLayerDataUrl;
        OUString aImporterService;

        OUString aComponent;
        OUString aEntity;

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
            OUString sMessage = OUSTRING("Too many arguments for LocalDataImporter Job");
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
            if (use_truncate && truncate || use_overwrite && !overwrite)
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

    ServiceFactory aFactory = this->getServiceFactory();

    uno::Reference< backend::XLayer > xLayer = aJob.use_component ?
        LocalSingleBackend::createSimpleLayer(aFactory,aJob.aLayerDataUrl, aJob.aComponent ):
        LocalSingleBackend::createSimpleLayer(aFactory,aJob.aLayerDataUrl) ;

    if (!xLayer.is())
    {
        OUString sMessage = OUSTRING("LocalDataImportService - Cannot create layer to import from");
        throw lang::NullPointerException(sMessage,*this);
    }

    uno::Reference< backend::XImportLayer > xImporter;

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
        OUString sMessage = OUSTRING("LocalDataImportService - Cannot create importer service: ") + aJob.aImporterService;
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

OUString SAL_CALL
    LocalDataImportService::getImplementationName(  )
        throw (uno::RuntimeException)
{
    return getServiceInfo().getImplementationName( );
}
// -----------------------------------------------------------------------------


sal_Bool SAL_CALL
    LocalDataImportService::supportsService( const OUString& ServiceName )
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

