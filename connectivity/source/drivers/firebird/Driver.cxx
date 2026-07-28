/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "Connection.hxx"
#include "Driver.hxx"
#include "SubComponent.hxx"

#include <connectivity/dbexception.hxx>
#include <strings.hrc>
#include <resource/sharedresources.hxx>

#include <config_firebird.h>

// firebird's own header has functions with parameters it does not use
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#endif
#include <firebird/Interface.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#if defined __GNUC__
#pragma GCC diagnostic pop
#endif

#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uuid.h>
#include <sal/log.hxx>

#include <string_view>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;

using namespace ::osl;

using namespace connectivity::firebird;

// Static const variables
namespace {
constexpr OUString our_sFirebirdTmpVar = u"FIREBIRD_TMP"_ustr;
constexpr OUString our_sFirebirdLockVar = u"FIREBIRD_LOCK"_ustr;
constexpr OUString our_sFirebirdMsgVar = u"FIREBIRD_MSG"_ustr;
constexpr OUString our_sFirebirdRootVar = u"FIREBIRD"_ustr;
#ifdef MACOSX
constexpr OUString our_sFirebirdLibVar = u"LIBREOFFICE_FIREBIRD_LIB"_ustr;
#endif

// The name of the file in the firebird directory that maps a name to the absolute path of a
// database. firebird looks a name up here before it treats it as a path, and re-reads the file
// whenever it has changed.
constexpr OUString our_sDatabasesConfName = u"databases.conf"_ustr;

// Put rContent in the file at rURL, discarding whatever was there before.
bool writeFile(const OUString& rURL, std::string_view aContent)
{
    ::osl::File aFile(rURL);
    if (aFile.open(osl_File_OpenFlag_Create | osl_File_OpenFlag_Write) != ::osl::FileBase::E_None)
    {
        if (aFile.open(osl_File_OpenFlag_Write) != ::osl::FileBase::E_None)
            return false;
        if (aFile.setSize(0) != ::osl::FileBase::E_None)
        {
            aFile.close();
            return false;
        }
    }

    bool bWholeContentWritten = true;
    if (!aContent.empty())
    {
        sal_uInt64 nWritten = 0;
        bWholeContentWritten
            = aFile.write(aContent.data(), aContent.size(), nWritten) == ::osl::FileBase::E_None
              && nWritten == aContent.size();
    }
    aFile.close();
    return bWholeContentWritten;
}

// firebird takes its root directory from the FIREBIRD variable on its first use, and reads the
// firebird.conf that confines it to that directory only from there. Ask firebird which directory
// it settled on and treat anything but our own as unconfined.
bool isFirebirdRootedAt(const OUString& rDataDirPath)
{
    Firebird::IMaster* pMaster = Firebird::fb_get_master_interface();
    if (!pMaster)
        return false;

    Firebird::IConfigManager* pConfigManager = pMaster->getConfigManager();
    if (!pConfigManager)
        return false;

    const char* pRootDirectory = pConfigManager->getRootDirectory();
    if (!pRootDirectory)
        return false;

    OUString sRootPath = OStringToOUString(pRootDirectory, osl_getThreadTextEncoding());
    if (sRootPath.endsWith("/") || sRootPath.endsWith("\\"))
        sRootPath = sRootPath.copy(0, sRootPath.getLength() - 1);

#if defined(_WIN32)
    return sRootPath.equalsIgnoreAsciiCase(rDataDirPath);
#else
    return sRootPath == rDataDirPath;
#endif
}
};

FirebirdDriver::FirebirdDriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext)
    : ODriver_BASE(m_aMutex)
    , m_aContext(_rxContext)
    , m_firebirdTMPDirectory(nullptr, true)
    , m_firebirdLockDirectory(nullptr, true)
    , m_firebirdDataDirectory(nullptr, true)
    , m_bConfined(false)
{
    // ::utl::TempFile uses a unique temporary directory (subdirectory of
    // /tmp or other user specific tmp directory) per instance in which
    // we can create directories for firebird at will.
    m_firebirdTMPDirectory.EnableKillingFile(true);
    m_firebirdLockDirectory.EnableKillingFile(true);
    m_firebirdDataDirectory.EnableKillingFile(true);

    // Overrides firebird's default of /tmp or c:\temp
    osl_setEnvironment(our_sFirebirdTmpVar.pData, m_firebirdTMPDirectory.GetFileName().pData);

    // Overrides firebird's default of /tmp/firebird or c:\temp\firebird
    osl_setEnvironment(our_sFirebirdLockVar.pData, m_firebirdLockDirectory.GetFileName().pData);

    // Keep the files firebird opens or creates together in one directory.
    // Embedded databases are extracted here, and DatabaseAccess = Restrict
    // keeps any associated files firebird makes for it under the same
    // directory. firebird reads this firebird.conf from the directory the
    // FIREBIRD variable names, once, on its first use, so it has to be set
    // before the first connection.
    OUString sDataDirPath;
    ::osl::FileBase::getSystemPathFromFileURL(m_firebirdDataDirectory.GetURL(), sDataDirPath);

#if defined(_WIN32)
    // firebird upper-cases the database path before the exact-match
    // DatabaseAccess check, so match that here.
    sDataDirPath = sDataDirPath.toAsciiUpperCase();
#endif

    // ExternalFileAccess is already None by default. Saying so here keeps a database firebird
    // opens from reading databases.conf back out of the directory it shares with it.
    OString sConf = "DatabaseAccess = Restrict "
        + OUStringToOString(sDataDirPath, RTL_TEXTENCODING_UTF8)
        + "\nExternalFileAccess = None\n";

    OUString sConfURL = m_firebirdDataDirectory.GetURL() + "/firebird.conf";
    if (writeFile(sConfURL, std::string_view(sConf))
        // An empty databases.conf, so that firebird has the file in hand from its first use and
        // notices the names added to it later.
        && writeFile(m_firebirdDataDirectory.GetURL() + "/" + our_sDatabasesConfName,
                     std::string_view()))
    {
        osl_setEnvironment(our_sFirebirdRootVar.pData, sDataDirPath.pData);
        m_bConfined = isFirebirdRootedAt(sDataDirPath);
    }

    SAL_WARN_IF(!m_bConfined, "connectivity.firebird",
                "firebird is not confined to " << sDataDirPath << ", connections will be refused");

#ifndef SYSTEM_FIREBIRD
    // Overrides firebird's hardcoded default of /usr/local/firebird on *nix,
    // however on Windows it seems to use the current directory as a default.
    OUString sMsgURL(u"$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/firebird"_ustr);
    ::rtl::Bootstrap::expandMacros(sMsgURL);
    OUString sMsgPath;
    ::osl::FileBase::getSystemPathFromFileURL(sMsgURL, sMsgPath);
    osl_setEnvironment(our_sFirebirdMsgVar.pData, sMsgPath.pData);
#ifdef MACOSX
    // Set an env. variable to specify library location
    // for dlopen used in fbclient.
    OUString sLibURL("$LO_LIB_DIR");
    ::rtl::Bootstrap::expandMacros(sLibURL);
    OUString sLibPath;
    ::osl::FileBase::getSystemPathFromFileURL(sLibURL, sLibPath);
    osl_setEnvironment(our_sFirebirdLibVar.pData, sLibPath.pData);
#endif /*MACOSX*/
#endif /*!SYSTEM_FIREBIRD*/
}

FirebirdDriver::~FirebirdDriver() = default;

OUString FirebirdDriver::addExternalDatabaseName(const OUString& rDatabasePath)
{
    MutexGuard aGuard(m_aMutex);

    sal_uInt8 aUuid[16];
    rtl_createUuid(aUuid, nullptr, false);

    OUStringBuffer aName(u"database");
    for (const sal_uInt8 nByte : aUuid)
    {
        aName.append(OUString::number(nByte >> 4, 16) + OUString::number(nByte & 0x0F, 16));
    }
    const OUString sName = aName.makeStringAndClear();

    m_aExternalDatabaseNames.emplace(sName, rDatabasePath);
    writeExternalDatabaseNames();

    return sName;
}

void FirebirdDriver::removeExternalDatabaseName(const OUString& rName)
{
    MutexGuard aGuard(m_aMutex);

    if (m_aExternalDatabaseNames.erase(rName) != 0)
        writeExternalDatabaseNames();
}

void FirebirdDriver::writeExternalDatabaseNames()
{
    MutexGuard aGuard(m_aMutex);

    OStringBuffer aContent;
    for (const auto& [rName, rPath] : m_aExternalDatabaseNames)
    {
        aContent.append(OUStringToOString(rName, RTL_TEXTENCODING_UTF8) + " = "
                        + OUStringToOString(rPath, RTL_TEXTENCODING_UTF8) + "\n");
    }
    const OString sContent = aContent.makeStringAndClear();

    const OUString sURL = m_firebirdDataDirectory.GetURL() + "/" + our_sDatabasesConfName;
    SAL_WARN_IF(!writeFile(sURL, std::string_view(sContent)), "connectivity.firebird",
                "could not write " << sURL);
}

void FirebirdDriver::disposing()
{
    MutexGuard aGuard(m_aMutex);

    for (auto const& elem : m_xConnections)
    {
        rtl::Reference< Connection > xComp(elem.get());
        if (xComp.is())
            xComp->dispose();
    }
    m_xConnections.clear();

    osl_clearEnvironment(our_sFirebirdTmpVar.pData);
    osl_clearEnvironment(our_sFirebirdLockVar.pData);
    osl_clearEnvironment(our_sFirebirdRootVar.pData);

#ifndef SYSTEM_FIREBIRD
    osl_clearEnvironment(our_sFirebirdMsgVar.pData);
#ifdef MACOSX
    osl_clearEnvironment(our_sFirebirdLibVar.pData);
#endif /*MACOSX*/
#endif /*!SYSTEM_FIREBIRD*/

    OSL_VERIFY(fb_shutdown(0, 1) == 0);

    ODriver_BASE::disposing();
}

OUString SAL_CALL FirebirdDriver::getImplementationName()
{
    return u"com.sun.star.comp.sdbc.firebird.Driver"_ustr;
}

sal_Bool SAL_CALL FirebirdDriver::supportsService(const OUString& _rServiceName)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL FirebirdDriver::getSupportedServiceNames()
{
    return { u"com.sun.star.sdbc.Driver"_ustr, u"com.sun.star.sdbcx.Driver"_ustr };
}

// ----  XDriver -------------------------------------------------------------
Reference< XConnection > SAL_CALL FirebirdDriver::connect(
    const OUString& url, const Sequence< PropertyValue >& info )
{
    SAL_INFO("connectivity.firebird", "connect(), URL: " << url );

    MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
       throw DisposedException();

    if ( ! acceptsURL(url) )
        return nullptr;

    // Without the firebird.conf that keeps firebird inside its own directory
    // we do not open any database.
    if (!m_bConfined)
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_COULD_NOT_LOAD_FILE).replaceFirst(
            "$filename$", u"firebird.conf");
        ::dbtools::throwGenericSQLException(sMessage, *this);
    }

    rtl::Reference<Connection> pCon = new Connection();
    pCon->construct(url, info, *this);

    m_xConnections.push_back(pCon);

    return pCon;
}

sal_Bool SAL_CALL FirebirdDriver::acceptsURL( const OUString& url )
{
    return (url == "sdbc:embedded:firebird" || url.startsWith("sdbc:firebird:"));
}

Sequence< DriverPropertyInfo > SAL_CALL FirebirdDriver::getPropertyInfo(
    const OUString& url, const Sequence< PropertyValue >& )
{
    if ( ! acceptsURL(url) )
    {
        ::connectivity::SharedResources aResources;
        const OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
        ::dbtools::throwGenericSQLException(sMessage ,*this);
    }

    return Sequence< DriverPropertyInfo >();
}

sal_Int32 SAL_CALL FirebirdDriver::getMajorVersion(  )
{
    // The major and minor version are sdbc driver specific. Must begin with 1.0
    // as per https://api.libreoffice.org/docs/common/ref/com/sun/star/sdbc/XDriver.html
    return 1;
}

sal_Int32 SAL_CALL FirebirdDriver::getMinorVersion(  )
{
    return 0;
}

//----- XDataDefinitionSupplier
uno::Reference< XTablesSupplier > SAL_CALL FirebirdDriver::getDataDefinitionByConnection(
                                    const uno::Reference< XConnection >& rConnection)
{
    if (Connection* pConnection = comphelper::getFromUnoTunnel<Connection>(rConnection))
        return pConnection->createCatalog();
    return {};
}

uno::Reference< XTablesSupplier > SAL_CALL FirebirdDriver::getDataDefinitionByURL(
                    const OUString& rURL,
                    const uno::Sequence< PropertyValue >& rInfo)
{
    uno::Reference< XConnection > xConnection = connect(rURL, rInfo);
    return getDataDefinitionByConnection(xConnection);
}

namespace connectivity::firebird
{
        void checkDisposed(bool _bThrow)
        {
            if (_bThrow)
                throw DisposedException();

        }

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
connectivity_FirebirdDriver_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    try {
        return cppu::acquire(new FirebirdDriver(context));
    } catch (...) {
        return nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
