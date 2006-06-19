/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localmultistratum.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:25:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "localmultistratum.hxx"

#ifndef CONFIGMGR_LOCALBE_LOCALFILEHELPER_HXX_
#include "localfilehelper.hxx"
#endif
#ifndef _CONFIGMGR_FILEHELPER_HXX_
#include "filehelper.hxx"
#endif

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif // CONFIGMGR_API_FACTORY_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif // CONFIGMGR_SERVICEINFOHELPER_HXX_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

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
: MultiStratumImplBase(xContext)
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
    typedef uno::Sequence< rtl::OUString > ResultType;

    rtl::OUString const aLayerUrl   = impl_getLayerDataDirectory(getBaseUrl());
    rtl::OUString const aComponentUrl = aLayerUrl + componentToPath(aComponent);

    using namespace osl;
    const sal_uInt32 k_STATUS_FIELDS =  FileStatusMask_Type | FileStatusMask_FileName;
    Directory aComponentDirectory(aComponentUrl);
    DirectoryItem aItem;
    std::vector< rtl::OUString > aResult;

    Directory::RC errcode = aComponentDirectory.open();
    switch (errcode)
    {
    case Directory::E_NOENT:
        return ResultType();

    case Directory::E_None:
        while (Directory::E_None == (errcode=aComponentDirectory.getNextItem(aItem)))
        {
            FileStatus aItemDescriptor( k_STATUS_FIELDS );
            errcode = aItem.getFileStatus(aItemDescriptor);

            if ( errcode != DirectoryItem::E_None )
            {
                OSL_ASSERT(errcode != Directory::E_NOENT); // unexpected failure for getFileStatus for existing file
                if (errcode == Directory::E_NOENT) continue;

                OSL_TRACE("Reading Component Directory - Error (%u) getting status of directory item.\n", unsigned(errcode));
                break;
            }

            OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_Type), "Could not get type of directory item");
            if (aItemDescriptor.getFileType() != FileStatus::Regular)
                continue;

            OSL_ENSURE( aItemDescriptor.isValid(FileStatusMask_FileName), "Could not get Name of component found");
            OUString const aFileName = aItemDescriptor.getFileName();
            if (!aFileName.endsWithIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(kLocalDataSuffix)))
                continue;

            aResult.push_back( makeLayerId(aComponent,aFileName) );
        }
        OSL_ASSERT(errcode != Directory::E_None); // Loop postcond

        // joint error handling with open failure
        if (errcode != Directory::E_NOENT) // normal loop termination
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

        return ResultType(&aResult.front(), static_cast<sal_Int32>(aResult.size()));
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
    aSubLayerUrl = OUString();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static const sal_Char * const kMultiStratumImplementation =
                "com.sun.star.comp.configuration.backend.LocalMultiStratum" ;
static const sal_Char * const kBackendService =
                "com.sun.star.configuration.backend.MultiStratum" ;
static const sal_Char * const kLocalService =
                "com.sun.star.configuration.backend.LocalMultiStratum" ;

static AsciiServiceName kServiceNames [] = { kLocalService, 0, kBackendService, 0 } ;
static const ServiceImplementationInfo kMultiStratumServiceInfo   = { kMultiStratumImplementation  , kServiceNames, kServiceNames + 2 } ;

const ServiceRegistrationInfo *getLocalMultiStratumServiceInfo()
{ return getRegistrationInfo(&kMultiStratumServiceInfo) ; }

uno::Reference<uno::XInterface> SAL_CALL
instantiateLocalMultiStratum(const CreationContext& xContext) {
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
