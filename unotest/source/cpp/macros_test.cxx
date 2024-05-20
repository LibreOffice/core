/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/macros_test.hxx>

#include <vector>

#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>

#include <basic/basrdll.hxx>
#include <cppunit/TestAssert.h>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <unotest/directories.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <tools/datetime.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/scheduler.hxx>

using namespace css;

namespace unotest
{
MacrosTest::MacrosTest()
    : mpDll(std::make_unique<BasicDLL>())
{
}

MacrosTest::~MacrosTest() = default;

uno::Reference<css::lang::XComponent>
MacrosTest::loadFromDesktop(const OUString& rURL, const OUString& rDocService,
                            const uno::Sequence<beans::PropertyValue>& rExtraArgs)
{
    CPPUNIT_ASSERT_MESSAGE("no desktop", mxDesktop.is());
    std::vector<beans::PropertyValue> args;
    beans::PropertyValue aMacroValue;
    aMacroValue.Name = "MacroExecutionMode";
    aMacroValue.Handle = -1;
    aMacroValue.Value <<= document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;
    aMacroValue.State = beans::PropertyState_DIRECT_VALUE;
    args.push_back(aMacroValue);

    if (!rDocService.isEmpty())
    {
        beans::PropertyValue aValue;
        aValue.Name = "DocumentService";
        aValue.Handle = -1;
        aValue.Value <<= rDocService;
        aValue.State = beans::PropertyState_DIRECT_VALUE;
        args.push_back(aValue);
    }

    args.insert(args.end(), rExtraArgs.begin(), rExtraArgs.end());

    uno::Reference<lang::XComponent> xComponent = mxDesktop->loadComponentFromURL(
        rURL, u"_default"_ustr, 0, comphelper::containerToSequence(args));
    OUString sMessage = "loading failed: " + rURL;
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(sMessage, RTL_TEXTENCODING_UTF8).getStr(),
                           xComponent.is());
    return xComponent;
}

css::uno::Any
MacrosTest::dispatchCommand(const uno::Reference<lang::XComponent>& xComponent,
                            const OUString& rCommand,
                            const uno::Sequence<beans::PropertyValue>& rPropertyValues)
{
    uno::Reference<frame::XController> xController
        = uno::Reference<frame::XModel>(xComponent, uno::UNO_QUERY_THROW)->getCurrentController();
    CPPUNIT_ASSERT(xController.is());
    uno::Reference<frame::XDispatchProvider> xFrame(xController->getFrame(), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xFrame.is());

    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
    uno::Reference<frame::XDispatchHelper> xDispatchHelper(frame::DispatchHelper::create(xContext));
    CPPUNIT_ASSERT(xDispatchHelper.is());

    auto ret = xDispatchHelper->executeDispatch(xFrame, rCommand, OUString(), 0, rPropertyValues);
    Scheduler::ProcessEventsToIdle();

    return ret;
}

std::unique_ptr<SvStream> MacrosTest::parseExportStream(const OUString& url,
                                                        const OUString& rStreamName)
{
    uno::Reference<uno::XComponentContext> xComponentContext
        = comphelper::getProcessComponentContext();
    uno::Reference<packages::zip::XZipFileAccess2> const xZipNames(
        packages::zip::ZipFileAccess::createWithURL(xComponentContext, url));
    uno::Reference<io::XInputStream> const xInputStream(xZipNames->getByName(rStreamName),
                                                        uno::UNO_QUERY);
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    return pStream;
}

void MacrosTest::setUpX509(const test::Directories& rDirectories, const OUString& rTestName)
{
    static bool isDone{ false };
    if (isDone) // must only be done once on MacOSX - see below!
    {
        return;
    }
    isDone = true;

    OUString aSourceDir = rDirectories.getURLFromSrc(u"/test/signing-keys/");
    OUString aTargetDir
        = rDirectories.getURLFromWorkdir(Concat2View("CppunitTest/" + rTestName + ".test.user"));

    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);

#ifdef _WIN32
    // CryptoAPI test certificates
    osl::File::copy(aSourceDir + "test.p7b", aTargetDir + "/test.p7b");
    OUString caVar("LIBO_TEST_CRYPTOAPI_PKCS7");
    osl_setEnvironment(caVar.pData, aTargetPath.pData);
#else
    // Set up NSS database in workdir/CppunitTest/
    // WARNING: on MacOSX, this *must only be done once* - once NSS has opened
    // the files, SQLite will *stop using them* if they are overwritten or renamed!
    osl::File::copy(aSourceDir + "cert9.db", aTargetDir + "/cert9.db");
    osl::File::copy(aSourceDir + "key4.db", aTargetDir + "/key4.db");
    osl::File::copy(aSourceDir + "pkcs11.txt", aTargetDir + "/pkcs11.txt");

    OUString mozCertVar(u"MOZILLA_CERTIFICATE_FOLDER"_ustr);
    // explicit prefix with "sql:" needed for CentOS7 system NSS 3.67
    osl_setEnvironment(mozCertVar.pData, OUString("sql:" + aTargetPath).pData);
#endif
}

void MacrosTest::setUpGpg(const test::Directories& rDirectories, const OUString& rTestName)
{
    OUString aSourceDir = rDirectories.getURLFromSrc(u"/test/signing-keys/");
    OUString aTargetDir
        = rDirectories.getURLFromWorkdir(Concat2View("CppunitTest/" + rTestName + ".test.user"));

    OUString aTargetPath;
    osl::FileBase::getSystemPathFromFileURL(aTargetDir, aTargetPath);

    // Make gpg use our own defined setup & keys
    osl::File::copy(aSourceDir + "pubring.gpg", aTargetDir + "/pubring.gpg");
    osl::File::copy(aSourceDir + "random_seed", aTargetDir + "/random_seed");
    osl::File::copy(aSourceDir + "secring.gpg", aTargetDir + "/secring.gpg");
    osl::File::copy(aSourceDir + "trustdb.gpg", aTargetDir + "/trustdb.gpg");

    OUString gpgHomeVar(u"GNUPGHOME"_ustr);
    osl_setEnvironment(gpgHomeVar.pData, aTargetPath.pData);

#if HAVE_GPGCONF_SOCKETDIR
    auto const ldPath = std::getenv("LIBO_LD_PATH");
    m_gpgconfCommandPrefix
        = ldPath == nullptr ? OString() : OString::Concat("LD_LIBRARY_PATH=") + ldPath + " ";
    OString path;
    bool ok = aTargetPath.convertToString(&path, osl_getThreadTextEncoding(),
                                          RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                                              | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
    // if conversion fails, at least provide a best-effort conversion in the message here, for
    // context
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString(aTargetPath, RTL_TEXTENCODING_UTF8).getStr(), ok);
    m_gpgconfCommandPrefix += "GNUPGHOME=" + path + " " GPGME_GPGCONF;
    // HAVE_GPGCONF_SOCKETDIR is only defined in configure.ac for Linux for now, so (a) std::system
    // behavior will conform to POSIX (and the relevant env var to set is named LD_LIBRARY_PATH), and
    // (b) gpgconf --create-socketdir should return zero:
    OString cmd = m_gpgconfCommandPrefix + " --create-socketdir";
    int res = std::system(cmd.getStr());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(cmd.getStr(), 0, res);
#else
    (void)this;
#endif
}

void MacrosTest::tearDownGpg()
{
#if HAVE_GPGCONF_SOCKETDIR
    // HAVE_GPGCONF_SOCKETDIR is only defined in configure.ac for Linux for now, so (a) std::system
    // behavior will conform to POSIX, and (b) gpgconf --remove-socketdir should return zero:
    OString cmd = m_gpgconfCommandPrefix + " --remove-socketdir";
    int res = std::system(cmd.getStr());
    CPPUNIT_ASSERT_EQUAL_MESSAGE(cmd.getStr(), 0, res);
#else
    (void)this;
#endif
}

namespace
{
struct Valid
{
    DateTime now;
    OUString subjectName;
    const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env;
    Valid(const css::uno::Sequence<css::beans::PropertyValue>& rFilterData,
          const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& rEnv)
        : now(DateTime::SYSTEM)
        , env(rEnv)
    {
        for (const auto& propVal : rFilterData)
        {
            if (propVal.Name == "SignCertificateSubjectName")
                propVal.Value >>= subjectName;
        }
    }
    bool operator()(const css::uno::Reference<css::security::XCertificate>& cert) const
    {
        if (!now.IsBetween(cert->getNotValidBefore(), cert->getNotValidAfter()))
            return false;
        if (!subjectName.isEmpty() && subjectName != cert->getSubjectName())
            return false;
        if (env->verifyCertificate(cert, {}) != css::security::CertificateValidity::VALID)
            return false;
        return true;
    }
};
}

bool MacrosTest::IsValid(const css::uno::Reference<css::security::XCertificate>& cert,
                         const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env)
{
    const Valid test({}, env);
    return test(cert);
}

css::uno::Reference<css::security::XCertificate> MacrosTest::GetValidCertificate(
    const css::uno::Sequence<css::uno::Reference<css::security::XCertificate>>& certs,
    const css::uno::Reference<css::xml::crypto::XSecurityEnvironment>& env,
    const css::uno::Sequence<css::beans::PropertyValue>& rFilterData)
{
    if (auto it = std::find_if(certs.begin(), certs.end(), Valid(rFilterData, env));
        it != certs.end())
        return *it;
    return {};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
