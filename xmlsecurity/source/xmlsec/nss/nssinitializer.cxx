/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


/*
 * and turn off the additional virtual methods which are part of some interfaces when compiled
 * with debug
 */
#ifdef DEBUG
#undef DEBUG
#endif


#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>

#include <officecfg/Office/Common.hxx>

#include <sal/types.h>
#include <rtl/instance.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>

#include "seinitializer_nssimpl.hxx"
#include "../diagnose.hxx"

#include "securityenvironment_nssimpl.hxx"
#include "digestcontext.hxx"
#include "ciphercontext.hxx"

#include <boost/scoped_array.hpp>

#include <nspr.h>
#include <cert.h>
#include <nss.h>
#include <pk11pub.h>
#include <secmod.h>
#include <nssckbi.h>

namespace cssu = css::uno;
namespace cssl = css::lang;

using namespace xmlsecurity;
using namespace com::sun::star;

#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.NSSInitializer_NssImpl"

#define ROOT_CERTS "Root Certs for OpenOffice.org"

extern "C" void nsscrypto_finalize();


namespace
{

bool nsscrypto_initialize( const css::uno::Reference< css::uno::XComponentContext > &rxContext, bool & out_nss_init );

struct InitNSSInitialize
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    InitNSSInitialize( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
    : m_xContext( rxContext )
    {
    }

    bool * operator()()
        {
            static bool bInitialized = false;
            bool bNSSInit = false;
            bInitialized = nsscrypto_initialize( m_xContext, bNSSInit );
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

OString getMozillaCurrentProfile( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    // first, try to get the profile from "MOZILLA_CERTIFICATE_FOLDER"
    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    if (pEnv)
    {
        SAL_INFO(
            "xmlsecurity.xmlsec",
            "Using Mozilla profile from MOZILLA_CERTIFICATE_FOLDER=" << pEnv);
        return OString(pEnv);
    }

    // second, try to get saved user-preference
    try
    {
        OUString sUserSetCertPath =
            officecfg::Office::Common::Security::Scripting::CertDir::get().get_value_or(OUString());

        if (!sUserSetCertPath.isEmpty())
        {
            SAL_INFO(
                "xmlsecurity.xmlsec",
                "Using Mozilla profile from /org.openoffice.Office.Common/"
                    "Security/Scripting/CertDir: " << sUserSetCertPath);
            return OUStringToOString(sUserSetCertPath, osl_getThreadTextEncoding());
        }
    }
    catch (const uno::Exception &e)
    {
        SAL_WARN(
            "xmlsecurity.xmlsec",
            "getMozillaCurrentProfile: caught exception " << e.Message);
    }

    // third, dig around to see if there's one available
    mozilla::MozillaProductType productTypes[3] = {
        mozilla::MozillaProductType_Thunderbird,
        mozilla::MozillaProductType_Firefox,
        mozilla::MozillaProductType_Mozilla };
    int nProduct = SAL_N_ELEMENTS(productTypes);

    uno::Reference<uno::XInterface> xInstance = rxContext->getServiceManager()->createInstanceWithContext("com.sun.star.mozilla.MozillaBootstrap", rxContext);
    OSL_ENSURE( xInstance.is(), "failed to create instance" );

    uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap
        =  uno::Reference<mozilla::XMozillaBootstrap>(xInstance,uno::UNO_QUERY);
    OSL_ENSURE( xMozillaBootstrap.is(), "failed to create instance" );

    if (xMozillaBootstrap.is())
    {
        for (int i=0; i<nProduct; ++i)
        {
            OUString profile = xMozillaBootstrap->getDefaultProfile(productTypes[i]);

            if (!profile.isEmpty())
            {
                OUString sProfilePath = xMozillaBootstrap->getProfilePath( productTypes[i], profile );
                SAL_INFO(
                    "xmlsecurity.xmlsec",
                    "Using Mozilla profile " << sProfilePath);
                return OUStringToOString(sProfilePath, osl_getThreadTextEncoding());
            }
        }
    }

    SAL_INFO("xmlsecurity.xmlsec", "No Mozilla profile found");
    return OString();
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
//When using SYSTEM_NSS then the libnss3.so lib is typically found in /usr/lib.
//This folder may, however, NOT contain the roots certificate module. That is,
//just providing the library name in SECMOD_LoadUserModule or
//SECMOD_AddNewModule will FAIL to load the mozilla unless the LD_LIBRARY_PATH
//contains an FF or TB installation.
//ATTENTION: DO NOT call this function directly instead use initNSS
//return true - whole initialization was successful
//param out_nss_init = true: at least the NSS initialization (NSS_InitReadWrite
//was successful and therefor NSS_Shutdown should be called when terminating.
bool nsscrypto_initialize( const css::uno::Reference< css::uno::XComponentContext > &rxContext, bool & out_nss_init )
{
    bool return_value = true;

    // this method must be called only once, no need for additional lock
    OString sCertDir;

#ifdef XMLSEC_CRYPTO_NSS
    sCertDir = getMozillaCurrentProfile(rxContext);
#else
    (void) rxContext;
#endif
    xmlsec_trace( "Using profile: %s", sCertDir.getStr() );

    PR_Init( PR_USER_THREAD, PR_PRIORITY_NORMAL, 1 ) ;

    bool bSuccess = true;
    // there might be no profile
    if ( !sCertDir.isEmpty() )
    {
        if( NSS_InitReadWrite( sCertDir.getStr() ) != SECSuccess )
        {
            xmlsec_trace("Initializing NSS with profile failed.");
            int errlen = PR_GetErrorTextLength();
            if(errlen > 0)
            {
                boost::scoped_array<char> const error(new char[errlen + 1]);
                PR_GetErrorText(error.get());
                xmlsec_trace("%s", error.get());
            }
            bSuccess = false;
        }
    }

    if( sCertDir.isEmpty() || !bSuccess )
    {
        xmlsec_trace("Initializing NSS without profile.");
        if ( NSS_NoDB_Init(NULL) != SECSuccess )
        {
            xmlsec_trace("Initializing NSS without profile failed.");
            int errlen = PR_GetErrorTextLength();
            if(errlen > 0)
            {
                boost::scoped_array<char> const error(new char[errlen + 1]);
                PR_GetErrorText(error.get());
                xmlsec_trace("%s", error.get());
            }
            return false ;
        }
    }
    out_nss_init = true;

#ifdef XMLSEC_CRYPTO_NSS
#if defined SYSTEM_NSS
    if (!SECMOD_HasRootCerts())
#endif
    {
        deleteRootsModule();

#if defined SYSTEM_NSS
        OUString rootModule("libnssckbi" SAL_DLLEXTENSION);
#else
        OUString rootModule(RTL_CONSTASCII_USTRINGPARAM("${LO_LIB_DIR}/libnssckbi" SAL_DLLEXTENSION));
#endif
        ::rtl::Bootstrap::expandMacros(rootModule);

        OUString rootModulePath;
        if (::osl::File::E_None == ::osl::File::getSystemPathFromFileURL(rootModule, rootModulePath))
        {
            OString ospath = OUStringToOString(rootModulePath, osl_getThreadTextEncoding());
            OStringBuffer pkcs11moduleSpec;
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
                              "\"" ROOT_CERTS "\" contained in \n%s", ospath.getStr());
                else
                {
                    xmlsec_trace("FAILED to load the new root certificate module "
                              "\"" ROOT_CERTS "\" contained in \n%s", ospath.getStr());
                    return_value = false;
                }
            }
            else
            {
                xmlsec_trace("FAILED to add new root certifice module: "
                          "\"" ROOT_CERTS "\" contained in \n%s", ospath.getStr());
                return_value = false;

            }
        }
        else
        {
            xmlsec_trace("Adding new root certificate module failed.");
            return_value = false;
        }
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
            xmlsec_trace("Unloaded module \"" ROOT_CERTS "\".");
        }
        else
        {
            xmlsec_trace("Failed unloading module \"" ROOT_CERTS "\".");
        }
        SECMOD_DestroyModule(RootsModule);
    }
    else
    {
        xmlsec_trace("Unloading module \"" ROOT_CERTS
                  "\" failed because it was not found.");
    }
    PK11_LogoutAll();
    NSS_Shutdown();
}
} // namespace

ONSSInitializer::ONSSInitializer(
    const css::uno::Reference< css::uno::XComponentContext > &rxContext)
    :m_xContext( rxContext )
{
}

ONSSInitializer::~ONSSInitializer()
{
}

bool ONSSInitializer::initNSS( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    return *rtl_Instance< bool, InitNSSInitialize, ::osl::MutexGuard, GetNSSInitStaticMutex >
                ::create( InitNSSInitialize( rxContext ), GetNSSInitStaticMutex() );
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
        throw css::lang::IllegalArgumentException("Unexpected digest requested.", css::uno::Reference< css::uno::XInterface >(), 1 );

    if ( aParams.getLength() )
        throw css::lang::IllegalArgumentException("Unexpected arguments provided for digest creation.", css::uno::Reference< css::uno::XInterface >(), 2 );

    css::uno::Reference< css::xml::crypto::XDigestContext > xResult;
    if( initNSS( m_xContext ) )
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
            throw css::lang::IllegalArgumentException("Unexpected key length.", css::uno::Reference< css::uno::XInterface >(), 2 );

        if ( aParams.getLength() )
            throw css::lang::IllegalArgumentException("Unexpected arguments provided for cipher creation.", css::uno::Reference< css::uno::XInterface >(), 5 );
    }
    else
        throw css::lang::IllegalArgumentException("Unexpected cipher requested.", css::uno::Reference< css::uno::XInterface >(), 1 );

    css::uno::Reference< css::xml::crypto::XCipherContext > xResult;
    if( initNSS( m_xContext ) )
    {
        if ( aInitializationVector.getLength() != PK11_GetIVLength( nNSSCipherID ) )
            throw css::lang::IllegalArgumentException("Unexpected length of initialization vector.", css::uno::Reference< css::uno::XInterface >(), 3 );

        xResult = OCipherContext::Create( nNSSCipherID, aKey, aInitializationVector, bEncryption, bW3CPadding );
    }

    return xResult;
}

OUString ONSSInitializer_getImplementationName ()
    throw (cssu::RuntimeException)
{

    return OUString ( IMPLEMENTATION_NAME );
}

sal_Bool SAL_CALL ONSSInitializer_supportsService( const OUString& ServiceName )
    throw (cssu::RuntimeException)
{
    return ServiceName == NSS_SERVICE_NAME;
}

cssu::Sequence< OUString > SAL_CALL ONSSInitializer_getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    cssu::Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString ( NSS_SERVICE_NAME );
    return aRet;
}

cssu::Reference< cssu::XInterface > SAL_CALL ONSSInitializer_createInstance( const cssu::Reference< cssl::XMultiServiceFactory > & rSMgr)
    throw( cssu::Exception )
{
    return (cppu::OWeakObject*) new ONSSInitializer( comphelper::getComponentContext(rSMgr) );
}

/* XServiceInfo */
OUString SAL_CALL ONSSInitializer::getImplementationName()
    throw (cssu::RuntimeException)
{
    return ONSSInitializer_getImplementationName();
}
sal_Bool SAL_CALL ONSSInitializer::supportsService( const OUString& rServiceName )
    throw (cssu::RuntimeException)
{
    return ONSSInitializer_supportsService( rServiceName );
}
cssu::Sequence< OUString > SAL_CALL ONSSInitializer::getSupportedServiceNames(  )
    throw (cssu::RuntimeException)
{
    return ONSSInitializer_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
