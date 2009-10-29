/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seinitializer_nssimpl.cxx,v $
 * $Revision: 1.22 $
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
#include "precompiled_xmlsecurity.hxx"

/*
 * Turn off DEBUG Assertions
 */
#ifdef _DEBUG
    #define _DEBUG_WAS_DEFINED _DEBUG
    #undef _DEBUG
#else
    #undef _DEBUG_WAS_DEFINED
#endif

/*
 * and turn off the additional virtual methods which are part of some interfaces when compiled
 * with debug
 */
#ifdef DEBUG
    #define DEBUG_WAS_DEFINED DEBUG
    #undef DEBUG
#else
    #undef DEBUG_WAS_DEFINED
#endif


#include <sal/types.h>
#include "rtl/instance.hxx"
#include "rtl/bootstrap.hxx"
#include "rtl/string.hxx"
#include "rtl/strbuf.hxx"
#include "osl/file.hxx"
#include "osl/thread.h"
#include <tools/debug.hxx>
#include <rtl/logfile.hxx>

#include "seinitializer_nssimpl.hxx"

#include "securityenvironment_nssimpl.hxx"
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>

#include "nspr.h"
#include "cert.h"
#include "nss.h"
#include "secmod.h"
#include "nssckbi.h"


namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;
namespace cssxc = com::sun::star::xml::crypto;

using namespace com::sun::star;
using ::rtl::OUString;
using ::rtl::OString;

#define SERVICE_NAME "com.sun.star.xml.crypto.SEInitializer"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.SEInitializer_NssImpl"
#define SECURITY_ENVIRONMENT "com.sun.star.xml.crypto.SecurityEnvironment"
#define SECURITY_CONTEXT "com.sun.star.xml.crypto.XMLSecurityContext"


#define ROOT_CERTS "Root Certs for OpenOffice.org"


extern "C" void nsscrypto_finalize();


namespace
{

bool nsscrypto_initialize( const char * sProfile, bool & out_nss_init);

struct InitNSSInitialize
{
    //path to the database folder
    const OString m_sProfile;
    InitNSSInitialize(const OString & sProfile): m_sProfile(sProfile) {};
    bool * operator()()
        {
            static bool bInitialized = false;
            bool bNSSInit = false;
            bInitialized = nsscrypto_initialize(m_sProfile.getStr(), bNSSInit);
            if (bNSSInit)
                atexit(nsscrypto_finalize );
             return & bInitialized;

        }
};

bool * initNSS(const OString & sProfile)
{
    return rtl_Instance< bool, InitNSSInitialize,
        ::osl::MutexGuard, ::osl::GetGlobalMutex >::create(
            InitNSSInitialize(sProfile), ::osl::GetGlobalMutex());
}

void deleteRootsModule()
{
    SECMODModule *RootsModule = 0;
    SECMODModuleList *list = SECMOD_GetDefaultModuleList();
    SECMODListLock *lock = SECMOD_GetDefaultModuleListLock();
    SECMOD_GetReadLock(lock);

    while (!RootsModule && list)
    {
        SECMODModule *module = list->module;

        for (int i=0; i < module->slotCount; i++)
        {
            PK11SlotInfo *slot = module->slots[i];
            if (PK11_IsPresent(slot))
            {
                if (PK11_HasRootCerts(slot))
                {
                    OSL_TRACE("[xmlsecurity] The root certifificates module \"%s"
                              "\" is already loaded: \n%s",
                              module->commonName,  module->dllName);

                    RootsModule = SECMOD_ReferenceModule(module);
                    break;
                }
            }
        }
        list = list->next;
    }
    SECMOD_ReleaseReadLock(lock);

    if (RootsModule)
    {
        PRInt32 modType;
        if (SECSuccess == SECMOD_DeleteModule(RootsModule->commonName, &modType))
        {
            OSL_TRACE("[xmlsecurity] Deleted module \"%s\".", RootsModule->commonName);
        }
        else
        {
            OSL_TRACE("[xmlsecurity] Failed to delete \"%s\" : \n%s",
                      RootsModule->commonName, RootsModule->dllName);
        }
        SECMOD_DestroyModule(RootsModule);
        RootsModule = 0;
    }
}

//Older versions of Firefox (FF), for example FF2, and Thunderbird (TB) 2 write
//the roots certificate module (libnssckbi.so), which they use, into the
//profile. This module will then already be loaded during NSS_Init (and the
//other init functions). This fails in two cases. First, FF3 was used to create
//the profile, or possibly used that profile before, and second the profile was
//used on a different platform.
//
//Then one needs to add the roots module oneself. This should be done with
//SECMOD_LoadUserModule rather then SECMOD_AddNewModule. The latter would write
//the location of the roots module to the profile, which makes FF2 and TB2 use
//it instead of there own module.
//
//When using SYSTEM_MOZILLA then the libnss3.so lib is typically found in
///usr/lib. This folder may, however, NOT contain the roots certificate
//module. That is, just providing the library name in SECMOD_LoadUserModule or
//SECMOD_AddNewModule will FAIL to load the mozilla unless the LD_LIBRARY_PATH
//contains an FF or TB installation.
//ATTENTION: DO NOT call this function directly instead use initNSS
//return true - whole initialization was successful
//param out_nss_init = true: at least the NSS initialization (NSS_InitReadWrite
//was successful and therefor NSS_Shutdown should be called when terminating.
bool nsscrypto_initialize( const char* token, bool & out_nss_init )
{
    bool return_value = true;

    OSL_TRACE("[xmlsecurity] Using profile: %s", token);

    PR_Init( PR_USER_THREAD, PR_PRIORITY_NORMAL, 1 ) ;

    if( NSS_InitReadWrite( token ) != SECSuccess )
    {
        char * error = NULL;

        PR_GetErrorText(error);
        if (error)
            printf("%s",error);
        return false ;
    }
    out_nss_init = true;

#if defined SYSTEM_MOZILLA
    if (!SECMOD_HasRootCerts())
    {
#endif
        deleteRootsModule();

#if defined SYSTEM_MOZILLA
        OUString rootModule(RTL_CONSTASCII_USTRINGPARAM("libnssckbi"SAL_DLLEXTENSION));
#else
        OUString rootModule(RTL_CONSTASCII_USTRINGPARAM("${OOO_BASE_DIR}/program/libnssckbi"SAL_DLLEXTENSION));
#endif
        ::rtl::Bootstrap::expandMacros(rootModule);

        OUString rootModulePath;
        if (::osl::File::E_None == ::osl::File::getSystemPathFromFileURL(rootModule, rootModulePath))
        {
            ::rtl::OString ospath = ::rtl::OUStringToOString(rootModulePath, osl_getThreadTextEncoding());
            ::rtl::OStringBuffer pkcs11moduleSpec;
            pkcs11moduleSpec.append("name=\"");
            pkcs11moduleSpec.append(ROOT_CERTS);
            pkcs11moduleSpec.append("\" library=\"");
            pkcs11moduleSpec.append(ospath.getStr());
            pkcs11moduleSpec.append("\"");

            SECMODModule * RootsModule =
                SECMOD_LoadUserModule(
                    const_cast<char*>(pkcs11moduleSpec.makeStringAndClear().getStr()),
                    0, // no parent
                    PR_FALSE); // do not recurse

            if (RootsModule)
            {

                bool found = RootsModule->loaded;

                SECMOD_DestroyModule(RootsModule);
                RootsModule = 0;
                if (found)
                    OSL_TRACE("[xmlsecurity] Added new root certificate module "
                              "\""ROOT_CERTS"\" contained in \n%s", ospath.getStr());
                else
                {
                    OSL_TRACE("[xmlsecurity] FAILED to load the new root certificate module "
                              "\""ROOT_CERTS"\" contained in \n%s", ospath.getStr());
                    return_value = false;
                }
            }
            else
            {
                OSL_TRACE("[xmlsecurity] FAILED to add new root certifice module: "
                          "\""ROOT_CERTS"\" contained in \n%s", ospath.getStr());
                return_value = false;

            }
        }
        else
        {
            OSL_TRACE("[xmlsecurity] Adding new root certificate module failed.");
            return_value = false;
        }
#if SYSTEM_MOZILLA
    }
#endif

    return return_value;
}


// must be extern "C" because we pass the function pointer to atexit
extern "C" void nsscrypto_finalize()
{
    SECMODModule *RootsModule = SECMOD_FindModule(ROOT_CERTS);

    if (RootsModule)
    {

        if (SECSuccess == SECMOD_UnloadUserModule(RootsModule))
        {
            OSL_TRACE("[xmlsecurity] Unloaded module \""ROOT_CERTS"\".");
        }
        else
        {
            OSL_TRACE("[xmlsecurity] Failed unloadeding module \""ROOT_CERTS"\".");
        }
        SECMOD_DestroyModule(RootsModule);
    }
    else
    {
        OSL_TRACE("[xmlsecurity] Unloading module \""ROOT_CERTS
                  "\" failed because it was not found.");
    }
    PK11_LogoutAll();
    NSS_Shutdown();
}


bool getMozillaCurrentProfile(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &rxMSF,
    rtl::OUString& profilePath)
{
    /*
     * first, try to get the profile from "MOZILLA_CERTIFICATE_FOLDER"
     */
        char * env = getenv("MOZILLA_CERTIFICATE_FOLDER");
        if (env)
        {
            profilePath = rtl::OUString::createFromAscii( env );
            RTL_LOGFILE_PRODUCT_TRACE1( "XMLSEC: Using env MOZILLA_CERTIFICATE_FOLDER: %s", rtl::OUStringToOString( profilePath, RTL_TEXTENCODING_ASCII_US ).getStr() );
            return true;
        }
        else
        {
            RTL_LOGFILE_TRACE( "getMozillaCurrentProfile: Using MozillaBootstrap..." );
            mozilla::MozillaProductType productTypes[4] = {
            mozilla::MozillaProductType_Thunderbird,
            mozilla::MozillaProductType_Mozilla,
            mozilla::MozillaProductType_Firefox,
            mozilla::MozillaProductType_Default };
        int nProduct = 4;

        uno::Reference<uno::XInterface> xInstance = rxMSF->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.mozilla.MozillaBootstrap")) );
        OSL_ENSURE( xInstance.is(), "failed to create instance" );

        uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap
            =  uno::Reference<mozilla::XMozillaBootstrap>(xInstance,uno::UNO_QUERY);
        OSL_ENSURE( xMozillaBootstrap.is(), "failed to create instance" );

        if (xMozillaBootstrap.is())
        {
            for (int i=0; i<nProduct; i++)
            {
                ::rtl::OUString profile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);

                RTL_LOGFILE_TRACE2( "getMozillaCurrentProfile: getDefaultProfile [%i] returns %s", i, rtl::OUStringToOString( profile, RTL_TEXTENCODING_ASCII_US ).getStr() );

                if (profile != NULL && profile.getLength()>0)
                {
                    profilePath = xMozillaBootstrap->getProfilePath(productTypes[i],profile);
                    RTL_LOGFILE_PRODUCT_TRACE1( "XMLSEC: Using Mozilla Profile: %s", rtl::OUStringToOString( profilePath, RTL_TEXTENCODING_ASCII_US ).getStr() );
                    return true;
                }
            }
        }

        RTL_LOGFILE_PRODUCT_TRACE( "XMLSEC: No Mozilla Profile found!" );
        return false;
    }
}

} // namespace

SEInitializer_NssImpl::SEInitializer_NssImpl(
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > &rxMSF)
    :mxMSF( rxMSF )
{
}

SEInitializer_NssImpl::~SEInitializer_NssImpl()
{
}

/* XSEInitializer */
cssu::Reference< cssxc::XXMLSecurityContext > SAL_CALL
    SEInitializer_NssImpl::createSecurityContext(
    const rtl::OUString& sCertDB )
    throw (cssu::RuntimeException)
{
    CERTCertDBHandle    *pCertHandle = NULL ;

    rtl::OString sCertDir;
    if( sCertDB.getLength() )
    {
        sCertDir = rtl::OString(sCertDB, sCertDB.getLength(), RTL_TEXTENCODING_ASCII_US);
    }
    else
    {
        static rtl::OString* pDefaultCertDir = NULL;
        if ( !pDefaultCertDir )
        {
            pDefaultCertDir = new rtl::OString;
            rtl::OUString ouCertDir;



            if ( getMozillaCurrentProfile(mxMSF, ouCertDir) )
                *pDefaultCertDir = rtl::OString(ouCertDir, ouCertDir.getLength(), RTL_TEXTENCODING_ASCII_US);
        }
        sCertDir = *pDefaultCertDir;

    }

    if( !sCertDir.getLength() )
    {
        RTL_LOGFILE_TRACE( "XMLSEC: Error - No certificate directory!" );
        // return NULL;
    }


    /* Initialize NSPR and NSS */
    /* Replaced with new methods by AF. ----
    //PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1 ) ;
    PR_Init( PR_USER_THREAD, PR_PRIORITY_NORMAL, 1 ) ;

    if (NSS_Init(sCertDir.getStr()) != SECSuccess )
    {
        PK11_LogoutAll();
        return NULL;
    }
    ----*/
    if( ! *initNSS( sCertDir.getStr() ) )
    {
        RTL_LOGFILE_TRACE( "XMLSEC: Error - nsscrypto_initialize() failed." );
        if ( NSS_NoDB_Init(NULL) != SECSuccess )
        {
            RTL_LOGFILE_TRACE( "XMLSEC: NSS_NoDB_Init also failed, NSS Security not available!" );
            return NULL;
        }
        else
        {
            RTL_LOGFILE_TRACE( "XMLSEC: NSS_NoDB_Init works, enough for verifying signatures..." );
        }
    }

    pCertHandle = CERT_GetDefaultCertDB() ;

    try
    {
        /* Build XML Security Context */
        const rtl::OUString sSecyrutyContext ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_CONTEXT ) );
        cssu::Reference< cssxc::XXMLSecurityContext > xSecCtx( mxMSF->createInstance ( sSecyrutyContext ), cssu::UNO_QUERY );
        if( !xSecCtx.is() )
            return NULL;

        const rtl::OUString sSecyrutyEnvironment ( RTL_CONSTASCII_USTRINGPARAM( SECURITY_ENVIRONMENT ) );
        cssu::Reference< cssxc::XSecurityEnvironment > xSecEnv( mxMSF->createInstance ( sSecyrutyEnvironment ), cssu::UNO_QUERY );
        cssu::Reference< cssl::XUnoTunnel > xEnvTunnel( xSecEnv , cssu::UNO_QUERY ) ;
        if( !xEnvTunnel.is() )
            return NULL;
        SecurityEnvironment_NssImpl* pSecEnv = reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xEnvTunnel->getSomething(SecurityEnvironment_NssImpl::getUnoTunnelId() ))) ;
        pSecEnv->setCertDb(pCertHandle);

        sal_Int32 n = xSecCtx->addSecurityEnvironment(xSecEnv);
        //originally the SecurityEnvironment with the internal slot was set as default
        xSecCtx->setDefaultSecurityEnvironmentIndex( n );
        return xSecCtx;
    }
    catch( cssu::Exception& )
    {
        //PK11_LogoutAll();
        //NSS_Shutdown();
        return NULL;
    }
}

void SAL_CALL SEInitializer_NssImpl::freeSecurityContext( const cssu::Reference< cssxc::XXMLSecurityContext >& )
    throw (cssu::RuntimeException)
{
    /*
     * because the security context will free all its content when it
     * is destructed, so here no free process for the security context
     * is needed.
     */
    //PK11_LogoutAll();
    //NSS_Shutdown();
}

rtl::OUString SEInitializer_NssImpl_getImplementationName ()
    throw (cssu::RuntimeException)
{

    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL SEInitializer_NssImpl_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
    return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL SEInitializer_NssImpl_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new SEInitializer_NssImpl(rSMgr);
}

/* XServiceInfo */
rtl::OUString SAL_CALL SEInitializer_NssImpl::getImplementationName(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_getImplementationName();
}
sal_Bool SAL_CALL SEInitializer_NssImpl::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL SEInitializer_NssImpl::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return SEInitializer_NssImpl_getSupportedServiceNames();
}

