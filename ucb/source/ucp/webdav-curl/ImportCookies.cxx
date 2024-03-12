/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "CurlSession.hxx"
#include "CurlUri.hxx"

#include <comphelper/base64.hxx>
#include <comphelper/scopeguard.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <osl/file.hxx>
#include <sal/log.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#ifdef _WIN32
#include <comphelper/windowserrorstring.hxx>

#include <boost/property_tree/json_parser.hpp>

#include <sqlite3.h>
#include <pk11pub.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlobj.h>
#include <Knownfolders.h>
#include <dpapi.h>
#endif

using namespace ::com::sun::star;

namespace http_dav_ucp
{
#ifdef _WIN32
struct Value
{
    OString value;
    OString encryptedValue;
};

static int callback(void* pArg, int argc, char** argv, char** ppColNames)
{
    Value* const pValue(static_cast<Value*>(pArg));
    assert(argc == 3);
    assert(strcmp(ppColNames[0], "value") == 0);
    assert(strcmp(ppColNames[2], "encrypted_value") == 0);
    pValue->value = OString(argv[0]); // base64 has no embedded 0
    auto const len(OString(argv[1]).toInt32());
    assert(len >= 0);
    pValue->encryptedValue = OString(argv[2], len);
    return 0;
}
#endif

OString TryImportCookies(uno::Reference<uno::XComponentContext> const& xContext[[maybe_unused]],
                         OUString const& rHost[[maybe_unused]])
{
#ifdef _WIN32
    // Sharepoint authentication: try to find a cookie from Microsoft Edge
    PWSTR ladPath;
    if (S_OK != SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &ladPath))
    {
        SAL_INFO("ucb.ucp.webdav.curl", "ShGetKnownFolderPath failed");
        return OString();
    }
    OUString const localAppDirPath(o3tl::toU(ladPath));
    CoTaskMemFree(ladPath);
    OUString localAppDirUrl;
    ::osl::File::getFileURLFromSystemPath(localAppDirPath, localAppDirUrl);
    ::osl::DirectoryItem temp;
    OUString dbUrlU = localAppDirUrl + "/Microsoft/Edge/User Data/Default/Network/Cookies";
    if (::osl::DirectoryItem::get(dbUrlU, temp) != osl_File_E_None)
    {
        dbUrlU = localAppDirUrl + "/Microsoft/Edge/User Data/Default/Cookies";
        if (::osl::DirectoryItem::get(dbUrlU, temp) != osl_File_E_None)
        {
            SAL_INFO("ucb.ucp.webdav.curl", "no Cookies file");
            return OString();
        }
    }
    OString const dbUrl(::rtl::OUStringToOString(dbUrlU, RTL_TEXTENCODING_UTF8));
    sqlite3* db;
    int rc = sqlite3_open_v2(dbUrl.getStr(), &db, SQLITE_OPEN_READONLY | SQLITE_OPEN_URI, nullptr);
    if (rc != SQLITE_OK)
    {
        // apparently this may crash, and sqlite3_errstr() isn't exported?
        // SAL_INFO("ucb.ucp.webdav.curl", "sqlite3_open failed: " << sqlite3_errmsg(db));
        SAL_INFO("ucb.ucp.webdav.curl", "sqlite3_open failed: " << rc);
        sqlite3_close(db);
    }
    char* err(nullptr);
    Value value;
    OString const statement("SELECT value, LENGTH(encrypted_value), encrypted_value FROM cookies "
                            "WHERE name = \"FedAuth\" AND host_key = \""
                            + ::rtl::OUStringToOString(rHost, RTL_TEXTENCODING_ASCII_US) + "\";");
    rc = sqlite3_exec(db, statement.getStr(), callback, &value, &err);
    if (rc != SQLITE_OK)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "sqlite3_exec failed: " << err);
        sqlite3_free(err);
    }
    sqlite3_close(db);
    if (!value.value.isEmpty())
    {
        return value.value;
    }
    if (value.encryptedValue.getLength() < 3 + 12 + 16)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "encrypted_value too short: " << value.encryptedValue.getLength());
        return OString();
    }

    OString const iv(value.encryptedValue.copy(3, 12));
    OString const encryptedValue(
        value.encryptedValue.copy(3 + 12, value.encryptedValue.getLength() - 3 - 12 - 16));
    OString const tag(value.encryptedValue.copy(value.encryptedValue.getLength() - 16, 16));

    OUString const stateUrl = localAppDirUrl + "/Microsoft/Edge/User Data/Local State";
    OUString statePathU;
    ::osl::File::getSystemPathFromFileURL(stateUrl, statePathU);
    OString const statePath(::rtl::OUStringToOString(statePathU, RTL_TEXTENCODING_UTF8));
    ::std::string sEncryptedKey;
    try
    {
        ::boost::property_tree::ptree localState;
        ::boost::property_tree::read_json(::std::string(statePath.getStr()), localState);
        sEncryptedKey = localState.get<std::string>("os_crypt.encrypted_key");
    }
    catch (...)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "failed to parse Local State");
        return OString();
    }
    OUString const encodedEncryptedKey(sEncryptedKey.data(), sEncryptedKey.size(),
                                       RTL_TEXTENCODING_UTF8);
    uno::Sequence<sal_Int8> decodedEncryptedKey;
    ::comphelper::Base64::decode(decodedEncryptedKey, encodedEncryptedKey);
    if (decodedEncryptedKey.getLength() < 5)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "decoded key too short: " << decodedEncryptedKey.getLength());
        return OString();
    }
    DATA_BLOB protectedKey;
    protectedKey.cbData = decodedEncryptedKey.getLength() - 5;
    protectedKey.pbData
        = reinterpret_cast<BYTE*>(const_cast<sal_Int8*>(decodedEncryptedKey.getConstArray())) + 5;
    DATA_BLOB unprotectedKey;
    if (CryptUnprotectData(&protectedKey, nullptr, nullptr, nullptr, nullptr,
                           CRYPTPROTECT_UI_FORBIDDEN, &unprotectedKey)
        == FALSE)
    {
        SAL_INFO("ucb.ucp.webdav.curl", "CryptUnprotectData failed: " << WindowsErrorString(GetLastError()));
        assert(false);
        return OString();
    }
    ::comphelper::ScopeGuard g([&]() {
        SecureZeroMemory(unprotectedKey.pbData, unprotectedKey.cbData);
        LocalFree(unprotectedKey.pbData);
    });
    if (unprotectedKey.cbData < 16)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "CryptUnprotectData result too short: " << unprotectedKey.cbData);
        return OString();
    }

    // first, init NSS - but can't do AES GCM via API so do it directly
    uno::Reference<xml::crypto::XNSSInitializer> xNSS(
        xml::crypto::NSSInitializer::create(xContext));
    xNSS->getDigestContext(xml::crypto::DigestID::SHA256, {});
    SECItem keyItem = { siBuffer, reinterpret_cast<unsigned char*>(unprotectedKey.pbData),
                        sal::static_int_cast<unsigned>(unprotectedKey.cbData) };
    ::std::unique_ptr<PK11SlotInfo, deleter_from_fn<PK11SlotInfo, PK11_FreeSlot>> pSlot(
        PK11_GetBestSlot(CKM_AES_GCM, nullptr));
    if (!pSlot)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "PK11_GetBestSlot failed");
        return OString();
    }
    ::std::unique_ptr<PK11SymKey, deleter_from_fn<PK11SymKey, PK11_FreeSymKey>> pSymKey(
        PK11_ImportSymKey(pSlot.get(), CKM_AES_GCM, PK11_OriginDerive, CKA_DECRYPT, &keyItem,
                          nullptr));
    if (!pSymKey)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "PK11_ImportSymKey failed");
        return OString();
    }
    SECItem dummy = { siBuffer, nullptr, 0 };
    struct ContextDeleter
    {
        void operator()(PK11Context* p) const { PK11_DestroyContext(p, PR_TRUE); }
    };
    ::std::unique_ptr<PK11Context, ContextDeleter> pContext(PK11_CreateContextBySymKey(
        CKM_AES_GCM, CKA_NSS_MESSAGE | CKA_DECRYPT, pSymKey.get(), &dummy));
    if (!pContext)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "PK11_CreateContextBySymKey failed");
        return OString();
    }

    ::std::vector<unsigned char> unencryptedValue;
    unencryptedValue.resize(encryptedValue.getLength());
    int outLength(0);
    SECStatus rv = PK11_AEADOp(
        pContext.get(), CKG_NO_GENERATE, 0, // only used for encryption
        reinterpret_cast<unsigned char*>(const_cast<sal_Char*>(iv.getStr())), iv.getLength(),
        nullptr, 0, // "additional data" not used
        unencryptedValue.data(), &outLength, encryptedValue.getLength(),
        reinterpret_cast<unsigned char*>(const_cast<sal_Char*>(tag.getStr())), tag.getLength(),
        reinterpret_cast<const unsigned char*>(encryptedValue.getStr()),
        encryptedValue.getLength());
    if (rv != SECSuccess)
    {
        SAL_WARN("ucb.ucp.webdav.curl", "PK11_AEADOp failed: " << rv);
        return OString();
    }
    if (outLength != encryptedValue.getLength())
    {
        SAL_WARN("ucb.ucp.webdav.curl", "PK11_AEADOp unexpected output length");
        return OString();
    }

    return "FedAuth="
           + OString(reinterpret_cast<const char*>(unencryptedValue.data()),
                     unencryptedValue.size())
           + ";";

#else
    return OString();
#endif
}

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
