/*************************************************************************
 *
 *  $RCSfile: ldapuserprofilebe.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:38:43 $
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
 *  WITHOUT LIMITATION, WARRANTIES THAT THE OOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILEBE_HXX_
#include "ldapuserprofilebe.hxx"
#endif
#ifndef EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
#include "ldapuserprofilelayer.hxx"
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

#ifndef _RTL_BYTESEQ_H_
#include <rtl/byteseq.h>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _OSL_SECURITY_HXX_
#include <osl/security.hxx>
#endif

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
//------------------------------------------------------------------------------
LdapUserProfileBe::LdapUserProfileBe(
        const uno::Reference<uno::XComponentContext>& xContext)
        throw (backend::BackendAccessException,
               backend::BackendSetupException)
        : BackendBase(mMutex),
          mFactory(xContext->getServiceManager(),uno::UNO_QUERY_THROW),
          mContext(xContext),
          mUserProfileMap(),
          mLdapConnection()
{
    static bool isInitializing =false;
    if (!isInitializing)
    {
        isInitializing=true;

        LdapDefinition aDefinition;
        if (isLdapConfigured(aDefinition))
        {

            try
            {
                connectToLdapServer(aDefinition);
                //Set the UserDN
                mUserDN = mLdapConnection.findUserDn(
                    rtl::OUStringToOString(mLoggedOnUser, RTL_TEXTENCODING_ASCII_US));
            }
            catch(ldap::LdapGenericException& exception)
            {
                mapGenericException(exception) ;
            }
            initializeMappingTable(
                rtl::OStringToOUString(aDefinition.mMapping,
                RTL_TEXTENCODING_ASCII_US));
        }
        else
        {
            throw backend::BackendSetupException(
                rtl::OUString::createFromAscii("LdapUserProfileBe- LDAP not configured"),
                NULL, uno::Any());
        }
    }
}
//------------------------------------------------------------------------------
LdapUserProfileBe::~LdapUserProfileBe(void) {
}
//------------------------------------------------------------------------------
void LdapUserProfileBe::connectToLdapServer(const LdapDefinition& aDefinition)
{
    mLdapConnection.connectSimple(aDefinition);

}
//------------------------------------------------------------------------------

void LdapUserProfileBe::initializeMappingTable(
    const rtl::OUString& aFileMapName)
{
    rtl::OUString aMappingFileUrl;
    getMappingFileUrl(aMappingFileUrl,aFileMapName );

    osl::File aFile (aMappingFileUrl);

    checkIOError( aFile.open (OpenFlag_Read),aMappingFileUrl);
    checkIOError( aFile.setPos (Pos_End, 0),aMappingFileUrl);

    sal_uInt64 nLength = 0;
    checkIOError( aFile.getPos (nLength),aMappingFileUrl);
    nLength = sal_uInt32(nLength);

    checkIOError( aFile.setPos (Pos_Absolut, 0), aMappingFileUrl);
    sal_uInt8 *pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory ( sal_uInt32(nLength)));
    sal_uInt64 nRead = 0;
    osl::File::RC result = aFile.read (pBuffer, nLength, nRead);
    if (result != osl::FileBase::E_None)
    {
        rtl_freeMemory (pBuffer);
        checkIOError( result, aMappingFileUrl );
    }
    if (nRead != nLength)
    {
        throw backend::BackendSetupException(rtl::OUString::createFromAscii
            ("LdapUserProfileBe - can not read entire Mapping File"),
                NULL, uno::Any());
    }

    rtl::OString aStrBuffer ( reinterpret_cast<char*>(pBuffer),sal_uInt32(nLength));
    mUserProfileMap.source(aStrBuffer);

}
//------------------------------------------------------------------------------

static const rtl::OUString kMappingFileSuffix(RTL_CONSTASCII_USTRINGPARAM(
    "-attr.map"));

static const rtl::OUString kMappingUrl(
    RTL_CONSTASCII_USTRINGPARAM("/modules/com.sun.star.configuration/bootstrap/LdapMappingUrl"));


static const rtl::OUString kMappingDirectory(RTL_CONSTASCII_USTRINGPARAM(
    "/share/registry/ldap"));
static const sal_Unicode kPathSeparator = '/' ;
static const rtl::OUString  kBootstrapContextSingletonName(
    RTL_CONSTASCII_USTRINGPARAM(
    "/singletons/com.sun.star.configuration.bootstrap.theBootstrapContext"));

void LdapUserProfileBe::getMappingFileUrl(
    rtl::OUString& aFileUrl, const rtl::OUString& aFileMapName)const
{
    uno::Any aContext = mContext->getValueByName(kBootstrapContextSingletonName);
    uno::Reference<uno::XComponentContext> aBootStrapContext;
    aContext >>= aBootStrapContext;
    aBootStrapContext->getValueByName(kMappingUrl)  >>= aFileUrl;

    if (aFileUrl.getLength() == 0 )
    {
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

    }

    rtl::OUStringBuffer sFileBuffer(aFileUrl);
    sFileBuffer.append(kPathSeparator);
    sFileBuffer.append (aFileMapName);
    sFileBuffer.append(kMappingFileSuffix);
    aFileUrl = sFileBuffer.makeStringAndClear();
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
static const rtl::OUString kConfigurationProviderService(
    RTL_CONSTASCII_USTRINGPARAM(
    "com.sun.star.configuration.ConfigurationProvider")) ;
static const rtl::OUString kReadOnlyViewService(
    RTL_CONSTASCII_USTRINGPARAM(
    "com.sun.star.configuration.ConfigurationAccess")) ;
static const rtl::OUString kComponent(
    RTL_CONSTASCII_USTRINGPARAM(
    "org.openoffice.LDAP/UserDirectory"));
static const rtl::OUString kServerDefiniton(RTL_CONSTASCII_USTRINGPARAM
    ("ServerDefinition"));
static const rtl::OUString kServer(RTL_CONSTASCII_USTRINGPARAM
    ("Server"));
static const rtl::OUString kPort(RTL_CONSTASCII_USTRINGPARAM(
    "Port"));
static const rtl::OUString kBaseDN(RTL_CONSTASCII_USTRINGPARAM(
    "BaseDN"));
static const rtl::OUString kUser(RTL_CONSTASCII_USTRINGPARAM(
    "SearchUser"));
static const rtl::OUString kPassword(RTL_CONSTASCII_USTRINGPARAM(
    "SearchPassword"));
static const rtl::OUString kUserObjectClass(RTL_CONSTASCII_USTRINGPARAM(
    "UserObjectClass"));
static const rtl::OUString kUserUniqueAttr(RTL_CONSTASCII_USTRINGPARAM(
    "UserUniqueAttribute"));
static const rtl::OUString kMapping(RTL_CONSTASCII_USTRINGPARAM(
    "Mapping"));
static const rtl::OString kDefaultMappingFile("oo-ldap");

bool LdapUserProfileBe::isLdapConfigured(LdapDefinition& aDefinition)
{

    uno::Reference< XInterface > xIface;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xCfgProvider
        ( mFactory->createInstance(kConfigurationProviderService), uno::UNO_QUERY);
        OSL_ASSERT(xCfgProvider.is());
        if (!xCfgProvider.is())
        {
            throw backend::BackendSetupException(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "LdapUserProfileBe: could not create the configuration provider")),
                            NULL, uno::Any()) ;
        }
        using namespace css::beans;

        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= kComponent;

        NamedValue aPath (rtl::OUString::createFromAscii("nodepath"), aArgs[0]);
        aArgs[0] <<=  aPath;

        xIface = xCfgProvider->createInstanceWithArguments
            (kReadOnlyViewService,aArgs);
    }
    catch(uno::Exception& e)
    {
        OSL_ENSURE(false,"LdapUserProfileBe - Could not create Configuration Provider");
        return false;
    }

    uno::Reference<container::XNameAccess > xAccess(xIface, uno::UNO_QUERY);

    xAccess->getByName(kServerDefiniton) >>= xIface;
    uno::Reference<container::XNameAccess > xChildAccess(xIface, uno::UNO_QUERY);

    if (!getLdapStringParam(xChildAccess, kServer, aDefinition.mServer))
        return false;
    if (!getLdapStringParam(xChildAccess, kBaseDN, aDefinition.mBaseDN))
        return false;
    aDefinition.mPort=0;
    xChildAccess->getByName(kPort) >>= aDefinition.mPort ;
    if (aDefinition.mPort == 0)
    {
        return false;
    }
    if (!getLdapStringParam(xAccess, kUserObjectClass, aDefinition.mUserObjectClass))
        return false;
    if (!getLdapStringParam(xAccess, kUserUniqueAttr, aDefinition.mUserUniqueAttr))
        return false;
    getLdapStringParam(xAccess, kUser, aDefinition.mAnonUser);
    getLdapStringParam(xAccess, kPassword, aDefinition.mAnonCredentials);

    if (!getLdapStringParam(xAccess, kMapping, aDefinition.mMapping))
    {
        aDefinition.mMapping =  kDefaultMappingFile;
    }
    osl::Security aSecurityContext;

    if (!aSecurityContext.getUserName(mLoggedOnUser))
    {
        OSL_TRACE("LdapUserProfileLayer::ReadData-could not get Logged on user from system");
    }
    sal_Int32 nIndex = mLoggedOnUser.indexOf('/');
    if (nIndex != 0)
    {
        mLoggedOnUser = mLoggedOnUser.copy(nIndex+1);
    }
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
    if (aServerParameter.getLength()==0)
    {
        return false;
    }
    return true;
}
//------------------------------------------------------------------------------
static const rtl::OString kModificationAttribute = "modifyTimeStamp";
uno::Reference<backend::XLayer> SAL_CALL LdapUserProfileBe::getLayer(
        const rtl::OUString& aComponent, const rtl::OUString& aTimestamp)
    throw (backend::BackendAccessException, lang::IllegalArgumentException)
{
    rtl::OString aTimeStamp = mLdapConnection.
        getSingleAttribute( mUserDN, kModificationAttribute);

    return new LdapUserProfileLayer(
        mFactory, mLoggedOnUser, mUserProfileMap, mLdapConnection,
        rtl::OStringToOUString(aTimeStamp, RTL_TEXTENCODING_ASCII_US));
}

//------------------------------------------------------------------------------
uno::Reference<backend::XUpdatableLayer> SAL_CALL
LdapUserProfileBe::getUpdatableLayer(const rtl::OUString& aComponent)
    throw (backend::BackendAccessException,lang::NoSupportException,
           lang::IllegalArgumentException)
{
   throw lang::NoSupportException(
        rtl::OUString::createFromAscii(
        "LdapUserProfileBe: No Update Operation allowed, Read Only access"),
        *this) ;

    return NULL;
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

sal_Bool SAL_CALL LdapUserProfileBe::supportsService(
                                        const rtl::OUString& aServiceName)
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


