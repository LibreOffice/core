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

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/mozilla/XMozillaBootstrap.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <officecfg/Office/Common.hxx>
#include <sal/types.h>
#include <rtl/bootstrap.hxx>
#include <rtl/string.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/singletonref.hxx>
#include <comphelper/sequence.hxx>

#include <nss/nssinitializer.hxx>

#include "digestcontext.hxx"
#include "ciphercontext.hxx"
#include <memory>
#include <utility>
#include <vector>

#include <nss.h>
#include <pk11pub.h>
#include <secmod.h>
#include <prerror.h>
#include <prinit.h>

namespace cssu = css::uno;
namespace cssl = css::lang;

using namespace com::sun::star;

#define ROOT_CERTS "Root Certs for OpenOffice.org"

extern "C" {

static void nsscrypto_finalize();

}

namespace
{

class InitNSSPrivate
{
private:
    std::optional<utl::TempFileNamed> m_oTempFileDatabaseDirectory;

public:
    OUString getTempDatabasePath()
    {
        if (!m_oTempFileDatabaseDirectory)
        {
            m_oTempFileDatabaseDirectory.emplace(nullptr, true);
            m_oTempFileDatabaseDirectory->EnableKillingFile();
        }
        return m_oTempFileDatabaseDirectory->GetFileName();
    }

    void reset()
    {
        if (m_oTempFileDatabaseDirectory)
        {
            m_oTempFileDatabaseDirectory.reset();
        }
    }
};

comphelper::SingletonRef<InitNSSPrivate>* getInitNSSPrivate()
{
    static comphelper::SingletonRef<InitNSSPrivate> aInitNSSPrivate;
    return &aInitNSSPrivate;
}

bool nsscrypto_initialize( const css::uno::Reference< css::uno::XComponentContext > &rxContext, bool & out_nss_init );

#ifdef XMLSEC_CRYPTO_NSS

void deleteRootsModule()
{
    SECMODModule *RootsModule = nullptr;
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
                    SAL_INFO("xmlsecurity.xmlsec", "The root certificates module \"" << module->commonName << "\" is already loaded: " << module->dllName);

                    RootsModule = SECMOD_ReferenceModule(module);
                    break;
                }
            }
        }
        list = list->next;
    }
    SECMOD_ReleaseReadLock(lock);

    if (!RootsModule)
        return;

    PRInt32 modType;
    if (SECSuccess == SECMOD_DeleteModule(RootsModule->commonName, &modType))
    {
        SAL_INFO("xmlsecurity.xmlsec", "Deleted module \"" << RootsModule->commonName << "\".");
    }
    else
    {
        SAL_INFO("xmlsecurity.xmlsec", "Failed to delete \"" << RootsModule->commonName << "\": " << RootsModule->dllName);
    }
    SECMOD_DestroyModule(RootsModule);
    RootsModule = nullptr;
}

#endif

bool lcl_pathExists(const OUString& sPath)
{
    if (sPath.isEmpty())
        return false;

    ::osl::DirectoryItem aPathItem;
    OUString sURL;
    osl::FileBase::getFileURLFromSystemPath(sPath, sURL);
    if (::osl::FileBase::E_None == ::osl::DirectoryItem::get(sURL, aPathItem))
    {
        ::osl::FileStatus aStatus = osl_FileStatus_Mask_Validate;
        if (::osl::FileBase::E_None == aPathItem.getFileStatus(aStatus))
            return true;
    }

    return false;
}

} // namespace

const OUString & ONSSInitializer::getMozillaCurrentProfile(const css::uno::Reference< css::uno::XComponentContext > &rxContext, bool bSetActive)
{
    if (m_bIsNSSinitialized)
         return m_sNSSPath;
    if (bSetActive)
        m_bIsNSSinitialized = true;

    // first, try to get the profile from "MOZILLA_CERTIFICATE_FOLDER"
    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    if (pEnv)
    {
        SAL_INFO(
            "xmlsecurity.xmlsec",
            "Using Mozilla profile from MOZILLA_CERTIFICATE_FOLDER=" << pEnv);
        m_sNSSPath = OStringToOUString(pEnv, osl_getThreadTextEncoding());
    }

    // second, try to get saved user-preference
    if (m_sNSSPath.isEmpty())
    {
        try
        {
            OUString sUserSetCertPath =
                officecfg::Office::Common::Security::Scripting::CertDir::get().value_or(OUString());

            if (lcl_pathExists(sUserSetCertPath))
            {
                SAL_INFO(
                    "xmlsecurity.xmlsec",
                    "Using Mozilla profile from /org.openoffice.Office.Common/"
                        "Security/Scripting/CertDir: " << sUserSetCertPath);
                m_sNSSPath = sUserSetCertPath;
            }
        }
        catch (const uno::Exception &)
        {
            TOOLS_WARN_EXCEPTION("xmlsecurity.xmlsec", "getMozillaCurrentProfile:");
        }
    }

    // third, dig around to see if there's one default available
    mozilla::MozillaProductType productTypes[3] = {
        mozilla::MozillaProductType_Thunderbird,
        mozilla::MozillaProductType_Firefox,
        mozilla::MozillaProductType_Mozilla };

    uno::Reference<uno::XInterface> xInstance = rxContext->getServiceManager()->createInstanceWithContext("com.sun.star.mozilla.MozillaBootstrap", rxContext);
    OSL_ENSURE( xInstance.is(), "failed to create instance" );

    uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap(xInstance,uno::UNO_QUERY);
    OSL_ENSURE( xMozillaBootstrap.is(), "failed to create instance" );

    if (xMozillaBootstrap.is())
    {
        for (auto const productTypeIter : productTypes)
        {
            OUString profile = xMozillaBootstrap->getDefaultProfile(productTypeIter);

            if (!profile.isEmpty())
            {
                OUString sProfilePath = xMozillaBootstrap->getProfilePath(productTypeIter, profile);
                if (m_sNSSPath.isEmpty())
                {
                    SAL_INFO("xmlsecurity.xmlsec", "Using Mozilla profile " << sProfilePath);
                    m_sNSSPath = sProfilePath;
                }
                break;
            }
        }
    }

    SAL_INFO_IF(m_sNSSPath.isEmpty(), "xmlsecurity.xmlsec", "No Mozilla profile found");
    return m_sNSSPath;
}

css::uno::Sequence<css::xml::crypto::NSSProfile> SAL_CALL ONSSInitializer::getNSSProfiles()
{
    ONSSInitializer::getMozillaCurrentProfile(m_xContext);

    std::vector<xml::crypto::NSSProfile> aProfileList;
    aProfileList.reserve(10);

    mozilla::MozillaProductType productTypes[3] = {
        mozilla::MozillaProductType_Thunderbird,
        mozilla::MozillaProductType_Firefox,
        mozilla::MozillaProductType_Mozilla };

    uno::Reference<uno::XInterface> xInstance = m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.mozilla.MozillaBootstrap", m_xContext);
    OSL_ENSURE(xInstance.is(), "failed to create instance" );

    uno::Reference<mozilla::XMozillaBootstrap> xMozillaBootstrap(xInstance,uno::UNO_QUERY);

    if (xMozillaBootstrap.is())
    {
        for (auto const productTypeIter : productTypes)
        {
            uno::Sequence<OUString> aProductProfileList;
            xMozillaBootstrap->getProfileList(productTypeIter, aProductProfileList);
            for (const auto& sProfile : aProductProfileList)
                aProfileList.push_back({sProfile, xMozillaBootstrap->getProfilePath(productTypeIter, sProfile), productTypeIter});
        }
    }

    OUString sUserSelect;
    try
    {
        sUserSelect = officecfg::Office::Common::Security::Scripting::CertDir::get().value_or(OUString());;
        if (!lcl_pathExists(sUserSelect))
            sUserSelect = OUString();
    }
    catch (const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION("xmlsecurity.xmlsec", "getMozillaCurrentProfile:");
    }
    aProfileList.push_back({"MANUAL", sUserSelect, mozilla::MozillaProductType_Default});

    const char* pEnv = getenv("MOZILLA_CERTIFICATE_FOLDER");
    aProfileList.push_back({"MOZILLA_CERTIFICATE_FOLDER",
                            pEnv ? OStringToOUString(pEnv, osl_getThreadTextEncoding()) : OUString(),
                            mozilla::MozillaProductType_Default});

    return comphelper::containerToSequence(aProfileList);
}

bool ONSSInitializer::m_bIsNSSinitialized = false;
OUString ONSSInitializer::m_sNSSPath;

OUString SAL_CALL ONSSInitializer::getNSSPath()
{
    ONSSInitializer::getMozillaCurrentProfile(m_xContext);
    return m_sNSSPath;
};

sal_Bool SAL_CALL ONSSInitializer::getIsNSSinitialized()
{
    return m_bIsNSSinitialized;
}

ONSSInitializer::ONSSInitializer(css::uno::Reference< css::uno::XComponentContext > xContext)
    : m_xContext(std::move(xContext))
{
}

ONSSInitializer::ONSSInitializer()
{
}

namespace
{

//Older versions of Firefox (FF), for example FF2, and Thunderbird (TB) 2 write
//the roots certificate module (libnssckbi.so), which they use, into the
//profile. This module will then already be loaded during NSS_Init (and the
//other init functions). This fails in two cases. First, FF3 was used to create
//the profile, or possibly used that profile before, and second the profile was
//used on a different platform.
//
//Then one needs to add the roots module oneself. This should be done with
//SECMOD_LoadUserModule rather than SECMOD_AddNewModule. The latter would write
//the location of the roots module to the profile, which makes FF2 and TB2 use
//it instead of their own module.
//
//When using SYSTEM_NSS then the libnss3.so lib is typically found in /usr/lib.
//This folder may, however, NOT contain the roots certificate module. That is,
//just providing the library name in SECMOD_LoadUserModule or
//SECMOD_AddNewModule will FAIL to load the mozilla unless the LD_LIBRARY_PATH
//contains an FF or TB installation.
//ATTENTION: DO NOT call this function directly instead use initNSS
//return true - whole initialization was successful
//param out_nss_init = true: at least the NSS initialization (NSS_InitReadWrite
//was successful and therefore NSS_Shutdown should be called when terminating.
bool nsscrypto_initialize(css::uno::Reference<css::uno::XComponentContext> const & rxContext, bool & out_nss_init)
{
    // this method must be called only once, no need for additional lock
    OString sCertDir;

#ifdef XMLSEC_CRYPTO_NSS
    sCertDir = OUStringToOString(ONSSInitializer::getMozillaCurrentProfile(rxContext, true), osl_getThreadTextEncoding());
#else
    (void) rxContext;
#endif
    SAL_INFO("xmlsecurity.xmlsec",  "Using profile: " << sCertDir );

    PR_Init( PR_USER_THREAD, PR_PRIORITY_NORMAL, 1 ) ;

    bool bSuccess = false;
    // there might be no profile
    if (!sCertDir.isEmpty())
    {
        if (sCertDir.indexOf(':') == -1) //might be env var with explicit prefix
        {
            OUString sCertDirURL;
            osl::FileBase::getFileURLFromSystemPath(
                OStringToOUString(sCertDir, osl_getThreadTextEncoding()),
                sCertDirURL);
            osl::DirectoryItem item;
            if (osl::FileBase::E_NOENT != osl::DirectoryItem::get(sCertDirURL + "/cert8.db", item) &&
                osl::FileBase::E_NOENT == osl::DirectoryItem::get(sCertDirURL + "/cert9.db", item))
            {
                SAL_INFO("xmlsecurity.xmlsec", "nsscrypto_initialize: trying to avoid profile migration");
                sCertDir = "dbm:" + sCertDir;
            }
        }
        if (NSS_InitReadWrite(sCertDir.getStr()) != SECSuccess)
        {
            SAL_INFO("xmlsecurity.xmlsec", "Initializing NSS with profile failed.");
            int errlen = PR_GetErrorTextLength();
            if (errlen > 0)
            {
                std::unique_ptr<char[]> const error(new char[errlen + 1]);
                PR_GetErrorText(error.get());
                SAL_INFO("xmlsecurity.xmlsec", error.get());
            }
        }
        else
        {
            bSuccess = true;
        }
    }

    if (!bSuccess) // Try to create a database in temp dir
    {
        SAL_INFO("xmlsecurity.xmlsec", "Initializing NSS with a temporary profile.");
        OUString rString = (*getInitNSSPrivate())->getTempDatabasePath();

        if (NSS_InitReadWrite(rString.toUtf8().getStr()) != SECSuccess)
        {
            SAL_INFO("xmlsecurity.xmlsec", "Initializing NSS with a temporary profile.");
            int errlen = PR_GetErrorTextLength();
            if(errlen > 0)
            {
                std::unique_ptr<char[]> const error(new char[errlen + 1]);
                PR_GetErrorText(error.get());
                SAL_INFO("xmlsecurity.xmlsec", error.get());
            }
            return false;
        }
    }

    // Initialize and set empty password if needed
    // note: it's possible that the first NSS_InitReadWrite() succeeds by
    // creating a new DB; in this case it may also be necessary to call
    // PK11_InitPin()
    PK11SlotInfo* pSlot = PK11_GetInternalKeySlot();
    if (pSlot)
    {
        if (PK11_NeedUserInit(pSlot))
            PK11_InitPin(pSlot, nullptr, nullptr);
        PK11_FreeSlot(pSlot);
    }

    out_nss_init = true;

#ifdef XMLSEC_CRYPTO_NSS
    bool return_value = true;

#if defined SYSTEM_NSS || defined IOS // The statically linked nss on iOS acts as a "system" nss in this regards
    if (!SECMOD_HasRootCerts())
#endif
    {
        deleteRootsModule();

#ifdef IOS // Use statically linked NSS
        OUString rootModulePath("NSSCKBI");

        if (true)
#else
#if defined SYSTEM_NSS || defined ANDROID
        OUString rootModule("libnssckbi" SAL_DLLEXTENSION);
#else
        OUString rootModule("${LO_LIB_DIR}/libnssckbi" SAL_DLLEXTENSION);
#endif
        ::rtl::Bootstrap::expandMacros(rootModule);

        OUString rootModulePath;
        if (::osl::File::E_None == ::osl::File::getSystemPathFromFileURL(rootModule, rootModulePath))
#endif
        {
            OString ospath = OUStringToOString(rootModulePath, osl_getThreadTextEncoding());
            OString aStr = "name=\"" ROOT_CERTS "\" library=\"" + ospath + "\"";

            SECMODModule * RootsModule =
                SECMOD_LoadUserModule(
                    const_cast<char*>(aStr.getStr()),
                    nullptr, // no parent
                    PR_FALSE); // do not recurse

            if (RootsModule)
            {

                bool found = RootsModule->loaded;

                SECMOD_DestroyModule(RootsModule);
                RootsModule = nullptr;
                if (found)
                    SAL_INFO("xmlsecurity.xmlsec", "Added new root certificate module " ROOT_CERTS " contained in " << ospath);
                else
                {
                    SAL_INFO("xmlsecurity.xmlsec", "FAILED to load the new root certificate module " ROOT_CERTS "contained in " << ospath);
                    return_value = false;
                }
            }
            else
            {
                SAL_INFO("xmlsecurity.xmlsec", "FAILED to add new root certificate module " ROOT_CERTS  " contained in " << ospath);
                return_value = false;

            }
        }
        else
        {
            SAL_INFO("xmlsecurity.xmlsec", "Adding new root certificate module failed.");
            return_value = false;
        }
    }

    return return_value;
#else
    return true;
#endif
}

} // namespace

// must be extern "C" because we pass the function pointer to atexit
extern "C" void nsscrypto_finalize()
{
    SECMODModule *RootsModule = SECMOD_FindModule(ROOT_CERTS);

    if (RootsModule)
    {

        if (SECSuccess == SECMOD_UnloadUserModule(RootsModule))
        {
            SAL_INFO("xmlsecurity.xmlsec", "Unloaded module \"" ROOT_CERTS "\".");
        }
        else
        {
            SAL_INFO("xmlsecurity.xmlsec", "Failed unloading module \"" ROOT_CERTS "\".");
        }
        SECMOD_DestroyModule(RootsModule);
    }
    else
    {
        SAL_INFO("xmlsecurity.xmlsec", "Unloading module \"" ROOT_CERTS "\" failed because it was not found.");
    }
    PK11_LogoutAll();
    (void)NSS_Shutdown();

    (*getInitNSSPrivate())->reset();
}


ONSSInitializer::~ONSSInitializer()
{
}

bool ONSSInitializer::initNSS( const css::uno::Reference< css::uno::XComponentContext > &rxContext )
{
    static bool gbInitialized = [&rxContext]()
        {
            bool bNSSInit = false;
            bool bInitialized = nsscrypto_initialize( rxContext, bNSSInit );
            if (bNSSInit)
                atexit(nsscrypto_finalize);
            return bInitialized;
        }();
    return gbInitialized;
}

css::uno::Reference< css::xml::crypto::XDigestContext > SAL_CALL ONSSInitializer::getDigestContext( ::sal_Int32 nDigestID, const css::uno::Sequence< css::beans::NamedValue >& aParams )
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
    else if ( nDigestID == css::xml::crypto::DigestID::SHA512
           || nDigestID == css::xml::crypto::DigestID::SHA512_1K )
    {
        nNSSDigestID = SEC_OID_SHA512;
        nDigestLength = 64;
        b1KData = ( nDigestID == css::xml::crypto::DigestID::SHA512_1K );
    }
    else
        throw css::lang::IllegalArgumentException("Unexpected digest requested.", css::uno::Reference< css::uno::XInterface >(), 1 );

    if ( aParams.hasElements() )
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

css::uno::Reference< css::xml::crypto::XCipherContext > SAL_CALL ONSSInitializer::getCipherContext( ::sal_Int32 nCipherID, const css::uno::Sequence< ::sal_Int8 >& aKey, const css::uno::Sequence< ::sal_Int8 >& aInitializationVector, sal_Bool bEncryption, const css::uno::Sequence< css::beans::NamedValue >& aParams )
{
    CK_MECHANISM_TYPE nNSSCipherID = 0;
    bool bW3CPadding = false;
    switch (nCipherID)
    {
        case css::xml::crypto::CipherID::AES_CBC_W3C_PADDING:
            nNSSCipherID = CKM_AES_CBC;
            bW3CPadding = true;
            break;
        case css::xml::crypto::CipherID::AES_GCM_W3C:
            nNSSCipherID = CKM_AES_GCM;
            break;
        default:
            throw css::lang::IllegalArgumentException("Unexpected cipher requested.", css::uno::Reference< css::uno::XInterface >(), 1);
    }

    if ( aKey.getLength() != 16 && aKey.getLength() != 24 && aKey.getLength() != 32 )
        throw css::lang::IllegalArgumentException("Unexpected key length.", css::uno::Reference< css::uno::XInterface >(), 2 );

    if ( aParams.hasElements() )
        throw css::lang::IllegalArgumentException("Unexpected arguments provided for cipher creation.", css::uno::Reference< css::uno::XInterface >(), 5 );

    css::uno::Reference< css::xml::crypto::XCipherContext > xResult;
    if( initNSS( m_xContext ) )
    {
        if ( aInitializationVector.getLength() != PK11_GetIVLength( nNSSCipherID ) )
            throw css::lang::IllegalArgumentException("Unexpected length of initialization vector.", css::uno::Reference< css::uno::XInterface >(), 3 );

        xResult = OCipherContext::Create( nNSSCipherID, aKey, aInitializationVector, bEncryption, bW3CPadding );
        assert(xResult.is());
    }

    return xResult;
}

/* XServiceInfo */
OUString SAL_CALL ONSSInitializer::getImplementationName()
{
    return "com.sun.star.xml.crypto.NSSInitializer";
}

sal_Bool SAL_CALL ONSSInitializer::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

cssu::Sequence< OUString > SAL_CALL ONSSInitializer::getSupportedServiceNames(  )
{
    return { NSS_SERVICE_NAME };
}

#ifndef XMLSEC_CRYPTO_NSS
extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_xml_crypto_NSSInitializer_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new ONSSInitializer(pCtx));
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
