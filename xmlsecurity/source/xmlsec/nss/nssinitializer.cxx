/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>

#include <sal/types.h>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <tools/debug.hxx>
#include <rtl/logfile.hxx>

#include "seinitializer_nssimpl.hxx"
#include "../diagnose.hxx"

#include "securityenvironment_nssimpl.hxx"
#include "digestcontext.hxx"
#include "ciphercontext.hxx"

#include <nspr.h>
#include <cert.h>
#include <nss.h>
#include <pk11pub.h>
#include <secmod.h>
#include <nssckbi.h>


namespace css = ::com::sun::star;
namespace cssu = css::uno;
namespace cssl = css::lang;
namespace cssxc = css::xml::crypto;

using namespace xmlsecurity;
using namespace com::sun::star;
using ::rtl::OUString;
using ::rtl::OString;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.NSSInitializer_NssImpl"

#define ROOT_CERTS "Root Certs for OpenOffice.org"

extern "C" void nsscrypto_finalize();


namespace
{

bool nsscrypto_initialize( const css::uno::Reference< css::lang::XMultiServiceFactory > &xMSF, bool & out_nss_init );

struct InitNSSInitialize
{
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;

    InitNSSInitialize( const css::uno::Reference< css::lang::XMultiServiceFactory > &xMSF )
    : mxMSF( xMSF )
    {
    }

    bool * operator()()
        {
            static bool bInitialized = false;
            bool bNSSInit = false;
            bInitialized = nsscrypto_initialize( mxMSF, bNSSInit );
            if (bNSSInit)
                atexit(nsscrypto_finalize );
             return & bInitialized;
        }
};

struct GetNSSInitStaticMutex
{
    ::osl::Mutex* operator()()
    {
        static ::osl::Mutex aNSSInitMutex;
        return &aNSSInitMutex;
    }
};

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
                    xmlsec_trace("The root certifificates module \"%s"
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
            xmlsec_trace("Deleted module \"%s\".", RootsModule->commonName);
        }
        else
        {
            xmlsec_trace("Failed to delete \"%s\" : \n%s",
                      RootsModule->commonName, RootsModule->dllName);
        }
        SECMOD_DestroyModule(RootsModule);
        RootsModule = 0;
    }
}

::rtl::OString getMozillaCurrentProfile( const css::uno::Reference< css::lang::XMultiServiceFactory > &rxMSF )
{
    ::rtl::OString sResult;
    // first, try to get the profile from "MOZILLA_CERTIFICATE_FOLDER"
    char* pEnv = getenv( "MOZILLA_CERTIFICATE_FOLDER" );
    if ( pEnv )
    {
        sResult = ::rtl::OString( pEnv );
        RTL_LOGFILE_PRODUCT_TRACE1( "XMLSEC: Using env MOZILLA_CERTIFICATE_FOLDER: %s", sResult.getStr() );
    }
    else
    {
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

                if (profile != NULL && profile.getLength()>0)
                {
                    ::rtl::OUString sProfilePath = xMozillaBootstrap->getProfilePath( productTypes[i], profile );
                    sResult = ::rtl::OUStringToOString( sProfilePath, osl_getThreadTextEncoding() );
                    RTL_LOGFILE_PRODUCT_TRACE1( "XMLSEC: Using Mozilla Profile: %s", sResult.getStr() );
                }
            }
        }

        RTL_LOGFILE_PRODUCT_TRACE( "XMLSEC: No Mozilla Profile found!" );
    }

    return sResult;
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
bool nsscrypto_initialize( const css::uno::Reference< css::lang::XMultiServiceFactory > &xMSF, bool & out_nss_init )
{
    bool return_value = true;

    // this method must be called only once, no need for additional lock
    rtl::OString sCertDir;
    if ( xMSF.is() )
        sCertDir = getMozillaCurrentProfile( xMSF );

    xmlsec_trace( "Using profile: %s", sCertDir.getStr() );

    PR_Init( PR_USER_THREAD, PR_PRIORITY_NORMAL, 1 ) ;

    // there might be no profile
    if ( sCertDir.getLength() > 0 )
    {
        if( NSS_InitReadWrite( sCertDir.getStr() ) != SECSuccess )
        {
            xmlsec_trace("Initializing NSS with profile failed.");
            char * error = NULL;

            PR_GetErrorText(error);
            if (error)
                xmlsec_trace("%s",error);
            return false ;
        }
    }
    else
    {
        xmlsec_trace("Initializing NSS without profile.");
        if ( NSS_NoDB_Init(NULL) != SECSuccess )
        {
            xmlsec_trace("Initializing NSS without profile failed.");
            char * error = NULL;
            PR_GetErrorText(error);
            if (error)
                xmlsec_trace("%s",error);
            return false ;
        }
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
                    xmlsec_trace("Added new root certificate module "
                              "\""ROOT_CERTS"\" contained in \n%s", ospath.getStr());
                else
                {
                    xmlsec_trace("FAILED to load the new root certificate module "
                              "\""ROOT_CERTS"\" contained in \n%s", ospath.getStr());
                    return_value = false;
                }
            }
            else
            {
                xmlsec_trace("FAILED to add new root certifice module: "
                          "\""ROOT_CERTS"\" contained in \n%s", ospath.getStr());
                return_value = false;

            }
        }
        else
        {
            xmlsec_trace("Adding new root certificate module failed.");
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
            xmlsec_trace("Unloaded module \""ROOT_CERTS"\".");
        }
        else
        {
            xmlsec_trace("Failed unloadeding module \""ROOT_CERTS"\".");
        }
        SECMOD_DestroyModule(RootsModule);
    }
    else
    {
        xmlsec_trace("Unloading module \""ROOT_CERTS
                  "\" failed because it was not found.");
    }
    PK11_LogoutAll();
    NSS_Shutdown();
}
} // namespace

ONSSInitializer::ONSSInitializer(
    const css::uno::Reference< css::lang::XMultiServiceFactory > &rxMSF)
    :mxMSF( rxMSF )
{
}

ONSSInitializer::~ONSSInitializer()
{
}

bool ONSSInitializer::initNSS( const css::uno::Reference< css::lang::XMultiServiceFactory > &xMSF )
{
    return *rtl_Instance< bool, InitNSSInitialize, ::osl::MutexGuard, GetNSSInitStaticMutex >
                ::create( InitNSSInitialize( xMSF ), GetNSSInitStaticMutex() );
}

css::uno::Reference< css::xml::crypto::XDigestContext > SAL_CALL ONSSInitializer::getDigestContext( ::sal_Int32 nDigestID, const css::uno::Sequence< css::beans::NamedValue >& aParams )
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    SECOidTag nNSSDigestID = SEC_OID_UNKNOWN;
    sal_Int32 nDigestLength = 0;
    bool b1KData = false;
    if ( nDigestID == css::xml::crypto::DigestID::SHA256
      || nDigestID == css::xml::crypto::DigestID::SHA256_1K )
    {
        nNSSDigestID = SEC_OID_SHA256;
        nDigestLength = 32;
        b1KData = ( nDigestID == css::xml::crypto::DigestID::SHA256_1K );
    }
    else if ( nDigestID == css::xml::crypto::DigestID::SHA1
           || nDigestID == css::xml::crypto::DigestID::SHA1_1K )
    {
        nNSSDigestID = SEC_OID_SHA1;
        nDigestLength = 20;
        b1KData = ( nDigestID == css::xml::crypto::DigestID::SHA1_1K );
    }
    else
        throw css::lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected digest requested." ) ), css::uno::Reference< css::uno::XInterface >(), 1 );

    if ( aParams.getLength() )
        throw css::lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected arguments provided for digest creation." ) ), css::uno::Reference< css::uno::XInterface >(), 2 );

    css::uno::Reference< css::xml::crypto::XDigestContext > xResult;
    if( initNSS( mxMSF ) )
    {
        PK11Context* pContext = PK11_CreateDigestContext( nNSSDigestID );
        if ( pContext && PK11_DigestBegin( pContext ) == SECSuccess )
            xResult = new ODigestContext( pContext, nDigestLength, b1KData );
    }

    return xResult;
}

css::uno::Reference< css::xml::crypto::XCipherContext > SAL_CALL ONSSInitializer::getCipherContext( ::sal_Int32 nCipherID, const css::uno::Sequence< ::sal_Int8 >& aKey, const css::uno::Sequence< ::sal_Int8 >& aInitializationVector, ::sal_Bool bEncryption, const css::uno::Sequence< css::beans::NamedValue >& aParams )
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    CK_MECHANISM_TYPE nNSSCipherID = 0;
    bool bW3CPadding = false;
    if ( nCipherID == css::xml::crypto::CipherID::AES_CBC_W3C_PADDING )
    {
        nNSSCipherID = CKM_AES_CBC;
        bW3CPadding = true;

        if ( aKey.getLength() != 16 && aKey.getLength() != 24 && aKey.getLength() != 32 )
            throw css::lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected key length." ) ), css::uno::Reference< css::uno::XInterface >(), 2 );

        if ( aParams.getLength() )
            throw css::lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected arguments provided for cipher creation." ) ), css::uno::Reference< css::uno::XInterface >(), 5 );
    }
    else
        throw css::lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected cipher requested." ) ), css::uno::Reference< css::uno::XInterface >(), 1 );

    css::uno::Reference< css::xml::crypto::XCipherContext > xResult;
    if( initNSS( mxMSF ) )
    {
        if ( aInitializationVector.getLength() != PK11_GetIVLength( nNSSCipherID ) )
            throw css::lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Unexpected length of initialization vector." ) ), css::uno::Reference< css::uno::XInterface >(), 3 );

        xResult = OCipherContext::Create( nNSSCipherID, aKey, aInitializationVector, bEncryption, bW3CPadding );
    }

    return xResult;
}

rtl::OUString ONSSInitializer_getImplementationName ()
    throw (cssu::RuntimeException)
{

    return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL ONSSInitializer_supportsService( const rtl::OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( NSS_SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL ONSSInitializer_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < rtl::OUString > aRet(1);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( NSS_SERVICE_NAME ) );
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL ONSSInitializer_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new ONSSInitializer( rSMgr );
}

/* XServiceInfo */
rtl::OUString SAL_CALL ONSSInitializer::getImplementationName()
    throw (cssu::RuntimeException)
{
    return ONSSInitializer_getImplementationName();
}
sal_Bool SAL_CALL ONSSInitializer::supportsService( const rtl::OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return ONSSInitializer_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL ONSSInitializer::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return ONSSInitializer_getSupportedServiceNames();
}

