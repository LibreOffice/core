/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapuserprofilebe.cxx,v $
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
#include "precompiled_extensions.hxx"

#include "ldapuserprofilebe.hxx"
#include "ldapuserprofilelayer.hxx"
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <osl/process.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.h>

#ifndef INCLUDED_RTL_INSTANCE_HXX_
#include <rtl/instance.hxx>
#endif
#include <com/sun/star/beans/NamedValue.hpp>
#include <osl/security.hxx>

//==============================================================================
namespace extensions { namespace config { namespace ldap {


static void checkIOError(
    osl::File::RC aErrorCode,
    const rtl::OUString& aFileUrl)
{
    switch (aErrorCode)
    {
        case osl::File::E_None: // got it
        {

        }
        break;
        default:
        {
            rtl::OUStringBuffer sMsg;
            sMsg.appendAscii("LdapUserProfileBe: Cannot Read Meta-Configuration file:");
            sMsg.append(aFileUrl);
            throw backend::BackendSetupException(sMsg.makeStringAndClear(),
                NULL, uno::Any());
        }
    }
}

//------------------------------------------------------------------------------
#if 0
static rtl::OUString getCurrentModuleDirectory() // URL excluding terminating slash
{
    rtl::OUString aFileURL;
    if ( !osl::Module::getUrlFromAddress((void*)&getCurrentModuleDirectory,aFileURL) )
    {
        OSL_TRACE(false, "Cannot locate current module - using executable instead");

        OSL_VERIFY(osl_Process_E_None == osl_getExecutableFile(&aFileURL.pData));
    }

    OSL_ENSURE(0 < aFileURL.lastIndexOf('/'), "Cannot find directory for module URL");

    return aFileURL.copy(0, aFileURL.lastIndexOf('/') );
}
#endif
//------------------------------------------------------------------------------
LdapUserProfileBe::LdapUserProfileBe( const uno::Reference<uno::XComponentContext>& xContext)
// throw (backend::BackendAccessException, backend::BackendSetupException, RuntimeException)
: LdapProfileMutexHolder(),
  BackendBase(mMutex),
  mFactory(xContext->getServiceManager(),uno::UNO_QUERY_THROW),
  mContext(xContext),
  mLdapSource( new LdapUserProfileSource ),
  mLoggedOnUser(),
  mUserDN()
{
    LdapDefinition aDefinition;

    // This whole rigmarole is to prevent an infinite recursion where reading
    // the configuration for the backend would create another instance of the
    // backend, which would try and read the configuration which would...
    {
        osl::Mutex & aInitMutex = rtl::Static< osl::Mutex, LdapUserProfileBe >::get();
        osl::MutexGuard aInitGuard(aInitMutex);

        static bool bReentrantCall; // = false
        OSL_ENSURE(!bReentrantCall, "configuration: Ldap Backend constructor called reentrantly - probably a registration error.");

        if (!bReentrantCall)
        try
        {
            bReentrantCall = true ;
            if (! readLdapConfiguration(aDefinition) )
            {
                throw backend::BackendSetupException(
                    rtl::OUString::createFromAscii("LdapUserProfileBe- LDAP not configured"),
                    NULL, uno::Any());
            }

            bReentrantCall = false ;
        }
        catch (uno::Exception&)
        {
            bReentrantCall = false;
            throw;
        }
    }

    try
    {
        mLdapSource->mConnection.connectSimple(aDefinition);
        //Set the UserDN
        mUserDN = mLdapSource->mConnection.findUserDn(
            rtl::OUStringToOString(mLoggedOnUser, RTL_TEXTENCODING_ASCII_US));
    }
    catch (lang::IllegalArgumentException& exception)
    {
        throw backend::BackendSetupException(exception.Message, NULL,
                                             uno::makeAny(exception)) ;
    }
    catch (ldap::LdapConnectionException& exception)
    {
        throw backend::CannotConnectException(exception.Message, NULL,
                                             uno::makeAny(exception)) ;
    }
    catch(ldap::LdapGenericException& exception)
    {
        mapGenericException(exception) ;
    }

    initializeMappingTable(
        rtl::OStringToOUString(aDefinition.mMapping,
        RTL_TEXTENCODING_ASCII_US));

    OSL_POSTCOND(mLdapSource->mConnection.isConnected(),"Erroneously constructed a LdapUserProfileBackend without a LDAP connection");
}
//------------------------------------------------------------------------------
LdapUserProfileBe::~LdapUserProfileBe()
{
}
//------------------------------------------------------------------------------

void LdapUserProfileBe::initializeMappingTable(const rtl::OUString& aFileMapName)
{
    rtl::OUString aMappingFileUrl = getMappingFileUrl(aFileMapName );

    osl::File aFile (aMappingFileUrl);
    checkIOError( aFile.open(OpenFlag_Read),  aMappingFileUrl);

    sal_uInt64 nFileLength = 0;
    checkIOError( aFile.getSize(nFileLength), aMappingFileUrl);

    sal_uInt32 nDataLength = sal_uInt32(nFileLength);
    if (nDataLength != nFileLength)
    {
        throw backend::BackendSetupException(rtl::OUString::createFromAscii
            ("LdapUserProfileBe - can not read entire Mapping File: too big"),
                NULL, uno::Any());
    }

    struct RawBuffer
    {
        RawBuffer(sal_Size size) : data(rtl_allocateMemory(size)) {}
        ~RawBuffer() { rtl_freeMemory(data); }

        void * data;
    };
    RawBuffer buffer( nDataLength );

    sal_uInt64 nRead = 0;
    osl::File::RC result = aFile.read (static_cast<sal_uInt8*>(buffer.data), nDataLength, nRead);
    if (result != osl::File::E_None)
    {
        checkIOError( result, aMappingFileUrl );
        OSL_ASSERT(!"unreached");
    }

    if (nRead != nDataLength)
    {
        throw backend::BackendSetupException(rtl::OUString::createFromAscii
            ("LdapUserProfileBe - can not read entire Mapping File"),
                NULL, uno::Any());
    }

    rtl::OString aStrBuffer ( static_cast<char*>(buffer.data), sal_uInt32(nDataLength) );
    mLdapSource->mProfileMap.source(aStrBuffer);

}
//------------------------------------------------------------------------------

static const rtl::OUString kMappingFileSuffix(RTL_CONSTASCII_USTRINGPARAM(
    "-attr.map"));

static const rtl::OUString kMappingUrl(
    RTL_CONSTASCII_USTRINGPARAM("/modules/com.sun.star.configuration/bootstrap/LdapMappingUrl"));


static const sal_Unicode kPathSeparator = '/' ;
static const rtl::OUString  kBootstrapContextSingletonName(
    RTL_CONSTASCII_USTRINGPARAM(
    "/singletons/com.sun.star.configuration.bootstrap.theBootstrapContext"));

rtl::OUString LdapUserProfileBe::getMappingFileUrl(const rtl::OUString& aFileMapName) const
{
    uno::Any aContext = mContext->getValueByName(kBootstrapContextSingletonName);
    uno::Reference<uno::XComponentContext> aBootStrapContext;

    rtl::OUString aFileUrl;
    if (aContext >>= aBootStrapContext)
        aBootStrapContext->getValueByName(kMappingUrl)  >>= aFileUrl;

    if (aFileUrl.getLength() == 0 )
    {
#if 0
        static const rtl::OUString kMappingDirectory(RTL_CONSTASCII_USTRINGPARAM( "/share/registry/ldap"));

        rtl::OUString aModuleUrl = getCurrentModuleDirectory();
        sal_Int32 nIndex = aModuleUrl.lastIndexOf('/');
        if (nIndex == 0)
        {
            throw backend::BackendSetupException(rtl::OUString::createFromAscii
                ("LdapUserProfileBe - can not access Mapping File"),
                    NULL, uno::Any());
        }
        rtl::OUString aMappingFileUrl = aModuleUrl.copy(0, nIndex);
        aMappingFileUrl += kMappingDirectory;
        aFileUrl =  aMappingFileUrl;
#else
        throw backend::BackendSetupException(rtl::OUString::createFromAscii
            ("LdapUserProfileBe - can not locate Mapping File"),
                NULL, uno::Any());
#endif
    }

    rtl::OUStringBuffer sFileBuffer(aFileUrl);
    sFileBuffer.append(kPathSeparator);
    sFileBuffer.append (aFileMapName);
    sFileBuffer.append(kMappingFileSuffix);
    return sFileBuffer.makeStringAndClear();
}
//------------------------------------------------------------------------------
void LdapUserProfileBe::mapGenericException(ldap::LdapGenericException& aException)
    throw (backend::InsufficientAccessRightsException,
           backend::ConnectionLostException,
           backend::BackendAccessException)
{
    switch (aException.ErrorCode)
    {
        case LDAP_INSUFFICIENT_ACCESS:
            throw backend::InsufficientAccessRightsException(aException.Message,
                                    NULL, uno::makeAny(aException)) ;

        case LDAP_SERVER_DOWN:
        case LDAP_CONNECT_ERROR:
            throw backend::ConnectionLostException(aException.Message, NULL,
                                                   uno::makeAny(aException)) ;
        default:
            throw backend::BackendAccessException(aException.Message, NULL,
                                                  uno::makeAny(aException)) ;
    }
}
//------------------------------------------------------------------------------

bool LdapUserProfileBe::readLdapConfiguration(LdapDefinition& aDefinition)
{
    const rtl::OUString kConfigurationProviderService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationProvider")) ;
    const rtl::OUString kReadOnlyViewService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")) ;
    const rtl::OUString kComponent( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.LDAP/UserDirectory"));
    const rtl::OUString kServerDefiniton(RTL_CONSTASCII_USTRINGPARAM ("ServerDefinition"));
    const rtl::OUString kServer(RTL_CONSTASCII_USTRINGPARAM ("Server"));
    const rtl::OUString kPort(RTL_CONSTASCII_USTRINGPARAM("Port"));
    const rtl::OUString kBaseDN(RTL_CONSTASCII_USTRINGPARAM("BaseDN"));
    const rtl::OUString kUser(RTL_CONSTASCII_USTRINGPARAM("SearchUser"));
    const rtl::OUString kPassword(RTL_CONSTASCII_USTRINGPARAM("SearchPassword"));
    const rtl::OUString kUserObjectClass(RTL_CONSTASCII_USTRINGPARAM("UserObjectClass"));
    const rtl::OUString kUserUniqueAttr(RTL_CONSTASCII_USTRINGPARAM("UserUniqueAttribute"));
    const rtl::OUString kMapping(RTL_CONSTASCII_USTRINGPARAM("Mapping"));
    const rtl::OString kDefaultMappingFile("oo-ldap");

    uno::Reference< XInterface > xIface;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xCfgProvider(
                                                        mFactory->createInstance(kConfigurationProviderService),
                                                        uno::UNO_QUERY);
        OSL_ENSURE(xCfgProvider.is(),"LdapUserProfileBe: could not create the configuration provider");
        if (!xCfgProvider.is())
            return false;

        css::beans::NamedValue aPath(rtl::OUString::createFromAscii("nodepath"), uno::makeAny(kComponent) );

        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<=  aPath;

        xIface = xCfgProvider->createInstanceWithArguments(kReadOnlyViewService, aArgs);

        uno::Reference<container::XNameAccess > xAccess(xIface, uno::UNO_QUERY_THROW);
        xAccess->getByName(kServerDefiniton) >>= xIface;

        uno::Reference<container::XNameAccess > xChildAccess(xIface, uno::UNO_QUERY_THROW);

        if (!getLdapStringParam(xChildAccess, kServer, aDefinition.mServer))
            return false;
        if (!getLdapStringParam(xChildAccess, kBaseDN, aDefinition.mBaseDN))
            return false;

        aDefinition.mPort=0;
        xChildAccess->getByName(kPort) >>= aDefinition.mPort ;
        if (aDefinition.mPort == 0)
            return false;

        if (!getLdapStringParam(xAccess, kUserObjectClass, aDefinition.mUserObjectClass))
            return false;
        if (!getLdapStringParam(xAccess, kUserUniqueAttr, aDefinition.mUserUniqueAttr))
            return false;

        getLdapStringParam(xAccess, kUser, aDefinition.mAnonUser);
        getLdapStringParam(xAccess, kPassword, aDefinition.mAnonCredentials);

        if (!getLdapStringParam(xAccess, kMapping, aDefinition.mMapping))
            aDefinition.mMapping =  kDefaultMappingFile;
    }
    catch (uno::Exception & e)
    {
        OSL_TRACE("LdapUserProfileBackend: access to configuration data failed: %s",
                rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        return false;
    }

    osl::Security aSecurityContext;
    if (!aSecurityContext.getUserName(mLoggedOnUser))
        OSL_TRACE("LdapUserProfileBackend - could not get Logged on user from system");

    sal_Int32 nIndex = mLoggedOnUser.indexOf('/');
    if (nIndex > 0)
        mLoggedOnUser = mLoggedOnUser.copy(nIndex+1);

    //Remember to remove
    OSL_TRACE("Logged on user is %s", rtl::OUStringToOString(mLoggedOnUser,RTL_TEXTENCODING_ASCII_US).getStr());

    return true;
}

//------------------------------------------------------------------------------
bool LdapUserProfileBe::getLdapStringParam(
    uno::Reference<container::XNameAccess>& xAccess,
    const rtl::OUString& aLdapSetting,
    rtl::OString& aServerParameter)
{
    rtl::OUString sParam;
    xAccess->getByName(aLdapSetting) >>= sParam;
    aServerParameter = rtl::OUStringToOString(sParam, RTL_TEXTENCODING_ASCII_US);

    return aServerParameter.getLength() != 0;
}
//------------------------------------------------------------------------------
uno::Reference<backend::XLayer> SAL_CALL LdapUserProfileBe::getLayer(
        const rtl::OUString& /*aComponent*/, const rtl::OUString& /*aTimestamp*/)
    throw (backend::BackendAccessException, lang::IllegalArgumentException,uno::RuntimeException)
{
    OSL_PRECOND(mLdapSource->mConnection.isConnected(), "LdapUserProfileBackend invoked without a connection");
    if (!mLdapSource->mConnection.isConnected())
        return NULL;

    const rtl::OString kModificationAttribute = "modifyTimeStamp";

    rtl::OString aTimeStamp = mLdapSource->mConnection.
        getSingleAttribute( mUserDN, kModificationAttribute);

    return new LdapUserProfileLayer(
        mFactory, mLoggedOnUser, mLdapSource,
        rtl::OStringToOUString(aTimeStamp, RTL_TEXTENCODING_ASCII_US));
}

//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer> SAL_CALL
LdapUserProfileBe::getUpdatableLayer(const rtl::OUString& /*aComponent*/)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException,uno::RuntimeException)
{
   throw lang::NoSupportException(
        rtl::OUString::createFromAscii(
        "LdapUserProfileBe: No Update Operation allowed, Read Only access"),
        *this) ;
}
//------------------------------------------------------------------------------
rtl::OUString SAL_CALL LdapUserProfileBe::getLdapUserProfileBeName(void) {
    return rtl::OUString::createFromAscii("com.sun.star.comp.configuration.backend.LdapUserProfileBe") ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL LdapUserProfileBe::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getLdapUserProfileBeName() ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL LdapUserProfileBe::getLdapUserProfileBeServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServices(2) ;
    aServices[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.LdapUserProfileBe")) ;
    aServices[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.PlatformBackend")) ;

    return aServices ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL LdapUserProfileBe::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getLdapUserProfileBeServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;
    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL LdapUserProfileBe::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getLdapUserProfileBeServiceNames() ;
}
// ---------------------------------------------------------------------------------------
}}}
// ---------------------------------------------------------------------------------------


