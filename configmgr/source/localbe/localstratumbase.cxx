/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localstratumbase.cxx,v $
 * $Revision: 1.8 $
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

#include "localstratumbase.hxx"
#include "localfilehelper.hxx"
#include "localfilelayer.hxx"
#include "oslstream.hxx"
#include "serviceinfohelper.hxx"
#include "bootstrap.hxx"
#include "filehelper.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#include <osl/file.hxx>
#include <osl/process.h>
#include <memory>

namespace configmgr { namespace localbe {

//==============================================================================

//------------------------------------------------------------------------------

LocalStratumBase::LocalStratumBase(const uno::Reference<uno::XComponentContext>& xContext)
: cppu::WeakComponentImplHelper3<lang::XInitialization, backend::XBackendEntities, lang::XServiceInfo>(mMutex)
, mFactory(xContext->getServiceManager(),uno::UNO_QUERY)
{
}
//------------------------------------------------------------------------------

LocalStratumBase::~LocalStratumBase()
{}

//------------------------------------------------------------------------------
void SAL_CALL LocalStratumBase::initialize(const uno::Sequence<uno::Any>& aParameters)
    throw (uno::RuntimeException, uno::Exception,
           css::configuration::InvalidBootstrapFileException,
           backend::CannotConnectException,
           backend::BackendSetupException)
{



    if (aParameters.getLength() == 0)
    {
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "No parameters provided to local Stratum")),
                *this, 0) ;
    }


    for (sal_Int32 i = 0 ; i < aParameters.getLength() ; ++ i)
    {
        if (aParameters [i] >>= mStrataDataUrl )
        { break ; }

    }

    // get modified base for special layer implementations (e.g. resources)
    this->adjustBaseURL(mStrataDataUrl);

    validateFileURL(mStrataDataUrl, *this);
    implEnsureAbsoluteURL(mStrataDataUrl);
    normalizeURL(mStrataDataUrl,*this, true);

    if(FileHelper::fileExists(mStrataDataUrl))
    {
        checkIfDirectory(mStrataDataUrl, *this);
    }
}

//------------------------------------------------------------------------------

void LocalStratumBase::adjustBaseURL(rtl::OUString& )
{
}
//------------------------------------------------------------------------------

sal_Bool LocalStratumBase::isMoreRecent(const rtl::OUString& aFileUrl,
                                              const rtl::OUString& aTimestamp) {
    rtl::OUString layerUrl ;
    rtl::OUString subLayerUrl ;

    getLayerDirectories(layerUrl, subLayerUrl) ;

    return layerUrl.getLength() == 0 ||
            !BasicLocalFileLayer::getTimestamp(layerUrl + aFileUrl).equals( aTimestamp);
}
//------------------------------------------------------------------------------

uno::Reference<backend::XLayer> SAL_CALL
    LocalStratumBase::getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
        throw (backend::BackendAccessException, lang::IllegalArgumentException,
                uno::RuntimeException)
{

    if (aLayerId.getLength() == 0){
        throw lang::IllegalArgumentException(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "LocalStratum:getLayer - no LayerId specified")),
                *this, 0) ;
    }
    rtl::OUString const componentSubPath = layeridToPath(aLayerId) + getDataSuffix();
    if (!isMoreRecent(componentSubPath, aTimestamp)) { return NULL ; }

    return createReadonlyFileLayer(componentSubPath);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalStratumBase::getOwnerEntity()
    throw (uno::RuntimeException)
{
    return mStrataDataUrl ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalStratumBase::getAdminEntity()
    throw (uno::RuntimeException)
{
    return rtl::OUString();
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalStratumBase::supportsEntity( const rtl::OUString& aEntity )
    throw (backend::BackendAccessException, uno::RuntimeException)
{
    if(mStrataDataUrl.getLength() == 0)
    {
        return false;
    }
    if (aEntity.getLength() == 0)
    {
        return false;
    }
    return isEqualEntity(mStrataDataUrl,aEntity);
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalStratumBase::isEqualEntity(const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity)
    throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException)
{
    if (aEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 1);
    }
    if (aOtherEntity.getLength() == 0)
    {
        rtl::OUString const sMsg(RTL_CONSTASCII_USTRINGPARAM(
                "LocalSingleBackend - Invalid empty entity."));

        throw lang::IllegalArgumentException(sMsg, *this, 2);
    }
    rtl::OUString aNormalizedEntity(aEntity);
    normalizeURL(aNormalizedEntity,*this);

    rtl::OUString aNormalizedOther(aOtherEntity);
    normalizeURL(aNormalizedOther,*this);

    return aNormalizedEntity == aNormalizedOther;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uno::Reference<backend::XLayer>
    LocalStratumBase::createReadonlyFileLayer(const rtl::OUString& aSubpath)
         throw (lang::IllegalArgumentException)
{
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(layerPath, subLayerPath) ;
    return createReadonlyLocalFileLayer(mFactory, layerPath, aSubpath, subLayerPath) ;
}
//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer>
    LocalStratumBase::createUpdatableFileLayer(const rtl::OUString& aSubpath)
         throw (lang::IllegalArgumentException)
{
    rtl::OUString layerPath ;
    rtl::OUString subLayerPath ;

    getLayerDirectories(layerPath, subLayerPath) ;
    return createUpdatableLocalFileLayer(mFactory, layerPath, aSubpath, subLayerPath) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LocalStratumBase::getImplementationName()
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(getServiceInfoData()).getImplementationName() ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LocalStratumBase::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    return  ServiceInfoHelper(getServiceInfoData()).supportsService(aServiceName);
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LocalStratumBase::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return ServiceInfoHelper(getServiceInfoData()).getSupportedServiceNames() ;
}

// ---------------------------------------------------------------------------------------

void LocalStratumBase::failReadonly()
{
    rtl::OUStringBuffer aMessage;
    aMessage.appendAscii("Configurations - ")
            .appendAscii("Cannot get update access to layer: ")
            .appendAscii("Local file-based stratum at ")
            .append(this->getBaseUrl())
            .appendAscii(" is readonly.");
    throw lang::NoSupportException(aMessage.makeStringAndClear(),*this);
}
//------------------------------------------------------------------------------

} } // configmgr.localsinglestratum
