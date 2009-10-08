/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localmultistratum.cxx,v $
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

#include "localmultistratum.hxx"
#include "localfilehelper.hxx"
#include "filehelper.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_
#include "serviceinfohelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#include <osl/file.hxx>

namespace configmgr { namespace localbe {

//==============================================================================

static inline
rtl::OUString const & impl_getLayerDataDirectory(rtl::OUString const & aLayerBaseUrl)
{ return aLayerBaseUrl; }
//------------------------------------------------------------------------------
static //inline
rtl::OUString makeLayerId(rtl::OUString const & aComponent,rtl::OUString const & aParticleFile)
{
    OSL_ASSERT(aParticleFile.endsWithIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(kLocalDataSuffix)));
    const sal_Int32 kExtLength = RTL_CONSTASCII_LENGTH(kLocalDataSuffix);
    rtl::OUString const aParticleName = aParticleFile.copy(0,aParticleFile.getLength() - kExtLength);

    rtl::OUStringBuffer aLayerId(aComponent);
    aLayerId.append(k_cLayerIdSeparator);
    aLayerId.append(aParticleName);

    return aLayerId.makeStringAndClear();
}

LocalMultiStratum::LocalMultiStratum(const uno::Reference<uno::XComponentContext>& xContext)
: cppu::ImplInheritanceHelper1< LocalStratumBase, backend::XMultiLayerStratum >(xContext)
{
}
//------------------------------------------------------------------------------

LocalMultiStratum::~LocalMultiStratum() {}

//------------------------------------------------------------------------------
uno::Sequence< rtl::OUString > SAL_CALL
    LocalMultiStratum::listLayerIds( const rtl::OUString& aComponent,
                                     const rtl::OUString& /*aEntity*/ )
        throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    rtl::OUString const aLayerUrl   = impl_getLayerDataDirectory(getBaseUrl());
    rtl::OUString const aComponentUrl = aLayerUrl + componentToPath(aComponent);

    const sal_uInt32 k_STATUS_FIELDS =  FileStatusMask_Type | FileStatusMask_FileName;
    osl::Directory aComponentDirectory(aComponentUrl);
    osl::DirectoryItem aItem;
    std::vector< rtl::OUString > aResult;

    osl::Directory::RC errcode = aComponentDirectory.open();
    switch (errcode)
    {
    case osl::Directory::E_NOENT:
        return uno::Sequence< rtl::OUString >();

    case osl::Directory::E_None:
        while (osl::Directory::E_None == (errcode=aComponentDirectory.getNextItem(aItem)))
        {
            osl::FileStatus aItemDescriptor( k_STATUS_FIELDS );
            errcode = aItem.getFileStatus(aItemDescriptor);

            if ( errcode != osl::DirectoryItem::E_None )
            {
                OSL_ASSERT(errcode != osl::Directory::E_NOENT); // unexpected failure for getFileStatus for existing file
                if (errcode == osl::Directory::E_NOENT) continue;

                OSL_TRACE("Reading Component Directory - Error (%u) getting status of directory item.\n", unsigned(errcode));
                break;
            }

            OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_Type), "Could not get type of directory item");
            if (aItemDescriptor.getFileType() != osl::FileStatus::Regular)
                continue;

            OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileName), "Could not get name of component found");
            rtl::OUString const aFileName = aItemDescriptor.getFileName();
            if (!aFileName.endsWithIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(kLocalDataSuffix)))
                continue;

            aResult.push_back( makeLayerId(aComponent,aFileName) );
        }
        OSL_ASSERT(errcode != osl::Directory::E_None); // Loop postcond

        // joint error handling with open failure
        if (errcode != osl::Directory::E_NOENT) // normal loop termination
        {
    default: // if open() truly failed we also go here
            rtl::OUStringBuffer errbuf;
            errbuf.appendAscii("LocalMultiStratum::listLayerIds: ");
            errbuf.appendAscii("Error scanning directory ").append(aComponentUrl)
                  .appendAscii(" for particle files. ");
            errbuf.appendAscii("Error: ").append(FileHelper::createOSLErrorString(errcode));
            rtl::OUString const errmsg = errbuf.makeStringAndClear();
            throw backend::BackendAccessException(errmsg,*this,uno::Any());
        }

        return aResult.empty()
            ? uno::Sequence< rtl::OUString >()
            : uno::Sequence< rtl::OUString >(
                &aResult.front(), static_cast<sal_Int32>(aResult.size()));
    }
}

//------------------------------------------------------------------------------
rtl::OUString SAL_CALL
    LocalMultiStratum::getUpdateLayerId( const rtl::OUString& aComponent,
                                         const rtl::OUString& /*aEntity*/ )
        throw (backend::BackendAccessException, lang::NoSupportException,
                lang::IllegalArgumentException, uno::RuntimeException)
{
    failReadonly();
    return aComponent;
}

//------------------------------------------------------------------------------
uno::Reference< backend::XLayer > SAL_CALL
    LocalMultiStratum::getLayer( const rtl::OUString& aLayerId,
                                 const rtl::OUString& aTimestamp )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    return LocalStratumBase::getLayer(aLayerId,aTimestamp);
}
//------------------------------------------------------------------------------
uno::Sequence< uno::Reference< backend::XLayer > > SAL_CALL
    LocalMultiStratum::getLayers( const uno::Sequence< rtl::OUString >& aLayerIds,
                                  const rtl::OUString& aTimestamp )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    sal_Int32 const nLayers = aLayerIds.getLength();
    uno::Sequence< uno::Reference< backend::XLayer > > aResult(nLayers);
    for (sal_Int32 ix=0; ix<nLayers; ++ix)
    {
        aResult[ix] = LocalStratumBase::getLayer(aLayerIds[ix],aTimestamp);
    }
    return aResult;
}

//------------------------------------------------------------------------------
uno::Sequence< uno::Reference< backend::XLayer > > SAL_CALL
    LocalMultiStratum::getMultipleLayers( const uno::Sequence< rtl::OUString >& aLayerIds,
                                          const uno::Sequence< rtl::OUString >& aTimestamps )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{
    if (aLayerIds.getLength() != aTimestamps.getLength()) {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "LocalStratum::getMultipleLayers(): Timestamp count does not match layer count")),
                *this, 0) ;
    }
    sal_Int32 const nLayers = aLayerIds.getLength();
    uno::Sequence< uno::Reference< backend::XLayer > > aResult(nLayers);
    for (sal_Int32 ix=0; ix<nLayers; ++ix)
    {
        aResult[ix] = LocalStratumBase::getLayer(aLayerIds[ix],aTimestamps[ix]);
    }
    return aResult;
}

//------------------------------------------------------------------------------
uno::Reference< backend::XUpdatableLayer > SAL_CALL
    LocalMultiStratum::getUpdatableLayer( const rtl::OUString& /*aLayerId*/ )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                lang::NoSupportException, uno::RuntimeException)
{
    failReadonly();
    return 0;
}
//------------------------------------------------------------------------------

void LocalMultiStratum::getLayerDirectories(rtl::OUString& aLayerUrl,
                                             rtl::OUString& aSubLayerUrl) const
{
    aLayerUrl   = impl_getLayerDataDirectory(getBaseUrl());
    aSubLayerUrl = rtl::OUString();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static const sal_Char * const kMultiStratumImplementation =
                "com.sun.star.comp.configuration.backend.LocalMultiStratum" ;
static const sal_Char * const kBackendService =
                "com.sun.star.configuration.backend.MultiStratum" ;
static const sal_Char * const kLocalService =
                "com.sun.star.configuration.backend.LocalMultiStratum" ;

static sal_Char const * kServiceNames [] = { kLocalService, 0, kBackendService, 0 } ;
static const ServiceImplementationInfo kMultiStratumServiceInfo   = { kMultiStratumImplementation  , kServiceNames, kServiceNames + 2 } ;

const ServiceRegistrationInfo *getLocalMultiStratumServiceInfo()
{ return getRegistrationInfo(&kMultiStratumServiceInfo) ; }

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalMultiStratum(const uno::Reference< uno::XComponentContext >& xContext) {
    return *new LocalMultiStratum(xContext) ;
}

//------------------------------------------------------------------------------

const ServiceImplementationInfo * LocalMultiStratum::getServiceInfoData() const
{
    return &kMultiStratumServiceInfo;
}
//------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

} } // configmgr.localsinglestratum
