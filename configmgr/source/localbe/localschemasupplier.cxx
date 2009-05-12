/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localschemasupplier.cxx,v $
 * $Revision: 1.7 $
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

#include "localschemasupplier.hxx"
#include "localfilehelper.hxx"
#include "oslstream.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_
#include "serviceinfohelper.hxx"
#include "bootstrap.hxx"
#include "filehelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#include <osl/file.hxx>
#include <osl/process.h>
#include <memory>

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalSchemaSupplier::LocalSchemaSupplier(
        const uno::Reference<uno::XComponentContext>& xContext)
        : cppu::WeakComponentImplHelper3<backend::XVersionedSchemaSupplier, lang::XInitialization, lang::XServiceInfo>(mMutex), mFactory(xContext->getServiceManager(),uno::UNO_QUERY) {
}
//------------------------------------------------------------------------------

LocalSchemaSupplier::~LocalSchemaSupplier(void) {}
//------------------------------------------------------------------------------
static const rtl::OUString kSchemaDataUrl(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"SchemaDataUrl")) ;

static const rtl::OUString kSchemaVersion(
        RTL_CONSTASCII_USTRINGPARAM(CONTEXT_ITEM_PREFIX_"SchemaVersion")) ;

void SAL_CALL LocalSchemaSupplier::initialize(
        const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           css::configuration::InvalidBootstrapFileException,
           backend::CannotConnectException,
           backend::BackendSetupException)
{
    if (aParameters.getLength() == 0) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to LocalSchemaSupplier")),
                *this, 0) ;
    }
    uno::Reference<uno::XComponentContext> context ;

    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i) {
        if (aParameters [i] >>= context) { break ; }
    }

    // Setting: schema version
    // TODO: Add support for repository-specific versions
    uno::Any const aSchemaVersionSetting = context->getValueByName(kSchemaVersion);
    aSchemaVersionSetting >>= mSchemaVersion;

    // Setting: schema diretory(ies)
    uno::Any const aSchemaDataSetting = context->getValueByName(kSchemaDataUrl);
    uno::Sequence< rtl::OUString > aSchemas;
    rtl::OUString schemas;

    if (aSchemaDataSetting >>= schemas)
    {
        fillFromBlankSeparated(schemas, aSchemas) ;
    }
    else
    {
        aSchemaDataSetting >>=  aSchemas;
    }
    //validate SchemaDataUrls
    mSchemaDataUrls.realloc(aSchemas.getLength());

    sal_Int32 nSchemaLocations =0;
    sal_Int32 nExistingSchemaLocations = 0;
    for (sal_Int32 j = 0; j < aSchemas.getLength(); ++j)
    {
        bool bOptional = checkOptionalArg(aSchemas[j]);

        if(!bOptional)
            validateFileURL(aSchemas[j],*this);
        else if (!isValidFileURL(aSchemas[j]))
            continue;

        OSL_ASSERT(isValidFileURL(aSchemas[j]));

        //NormalizeURL
        implEnsureAbsoluteURL(aSchemas[j]);
        if(!normalizeURL(aSchemas[j],*this,bOptional))
            continue;

        //now we have a correct file URL, which we will use
        mSchemaDataUrls[nSchemaLocations++]= aSchemas[j];
        if (!bOptional)
            checkFileExists(aSchemas[j],*this);

        else if(!FileHelper::fileExists(aSchemas[j]))
            continue; // skip the directory check

        checkIfDirectory(aSchemas[j],*this);
        ++nExistingSchemaLocations;
    }
    if (0 == nExistingSchemaLocations)
    {
        rtl::OUString sMsg = rtl::OUString::createFromAscii("LocalBackend: No schema directories found");
        throw backend::BackendSetupException(sMsg,*this, uno::Any()) ;
    }
    mSchemaDataUrls.realloc(nSchemaLocations);
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL
    LocalSchemaSupplier::getSchemaVersion(const rtl::OUString& /*aComponent*/)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    // TODO: Add support for repository-specific versions
    return mSchemaVersion;
}
//------------------------------------------------------------------------------
static const rtl::OUString kSchemaSuffix(RTL_CONSTASCII_USTRINGPARAM(".xcs")) ;
static const rtl::OUString kXMLSchemaParser(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.xml.SchemaParser")) ;

uno::Reference<backend::XSchema> SAL_CALL
    LocalSchemaSupplier::getComponentSchema(const rtl::OUString& aComponent)
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    rtl::OUString subPath = componentToPath(aComponent) ;

    osl::File * schemaFile = NULL;
    rtl::OUString errorMessage;
    bool bInsufficientAccess = false;
    for (sal_Int32 ix = 0; ix < mSchemaDataUrls.getLength(); ++ix)
    {
        rtl::OUStringBuffer schemaUrl(mSchemaDataUrls[ix]) ;

        schemaUrl.append(subPath).append(kSchemaSuffix) ;

        rtl::OUString const aFileUrl = schemaUrl.makeStringAndClear();

        std::auto_ptr<osl::File> checkFile( new osl::File(aFileUrl) );
        osl::File::RC rc = checkFile->open(OpenFlag_Read) ;

        if (rc == osl::File::E_None)
        {
            schemaFile = checkFile.release();
            break;
        }
        else if (rc != osl::File::E_NOENT)
        {
            if (rc == osl::File::E_ACCES)
                bInsufficientAccess =true;

            // accumulate error messages
            rtl::OUStringBuffer sMsg(errorMessage);
            if (errorMessage.getLength())
                sMsg.appendAscii("LocalFile SchemaSupplier - Error accessing schema: ");

            sMsg.appendAscii("\n- Cannot open input file \"");
            sMsg.append(aFileUrl);
            sMsg.appendAscii("\" : ");
            sMsg.append(FileHelper::createOSLErrorString(rc));

            errorMessage = sMsg.makeStringAndClear();
        }
    }

    if (NULL == schemaFile)
    {
        if (errorMessage.getLength() != 0)
        {
            // a real error occured
            io::IOException ioe(errorMessage,*this);

            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("LocalFileLayer - Cannot readData: ").append(errorMessage);

            if (bInsufficientAccess)
                throw backend::InsufficientAccessRightsException(sMsg.makeStringAndClear(),*this,uno::makeAny(ioe));
            else
                throw backend::BackendAccessException(sMsg.makeStringAndClear(),*this,uno::makeAny(ioe));
        }
        // simply not found
        return NULL;
    }

    uno::Sequence<uno::Any> arguments(1) ;
    uno::Reference<io::XInputStream> stream( new OSLInputStreamWrapper(schemaFile, true) );

    arguments [0] <<= stream ;
    uno::Reference<backend::XSchema> schema(
            mFactory->createInstanceWithArguments(kXMLSchemaParser, arguments),
            uno::UNO_QUERY) ;

    if (!schema.is())
    {
        throw uno::RuntimeException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot instantiate Schema Parser for ")) + aComponent,
                *this) ;
    }
    return schema ;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

static const sal_Char * const kImplementation =
                "com.sun.star.comp.configuration.backend.LocalSchemaSupplier" ;
static const sal_Char * const kSchemaService =
                "com.sun.star.configuration.backend.SchemaSupplier" ;
static const sal_Char * const kLocalService =
                "com.sun.star.configuration.backend.LocalSchemaSupplier" ;

static sal_Char const * kServiceNames [] = {kLocalService, 0, kSchemaService, 0 } ;
static const ServiceImplementationInfo kServiceInfo = { kImplementation, kServiceNames,kServiceNames+2 } ;

const ServiceRegistrationInfo *getLocalSchemaSupplierServiceInfo()
{ return getRegistrationInfo(&kServiceInfo) ; }

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalSchemaSupplier(const uno::Reference< uno::XComponentContext >& xContext) {
    return *new LocalSchemaSupplier(xContext) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalSchemaSupplier::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalSchemaSupplier::supportsService(
                                        const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(&kServiceInfo).supportsService(aServiceName);
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL LocalSchemaSupplier::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(&kServiceInfo).getSupportedServiceNames() ;
}

// ---------------------------------------------------------------------------------------

} } // configmgr.localbe
