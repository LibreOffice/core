/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Unit test for the landlock lock-down of kit which runs without a chroot jail
 * and without mount namespaces, relying on landlock alone.
 */

#include <config.h>

#include <test/UnitWSDClient.hpp>
#include <Unit.hpp>
#include <common/FileUtil.hpp>
#include <common/Landlock.hpp>
#include <common/Log.hpp>
#include <helpers.hpp>

#include <Poco/File.h>
#include <Poco/Util/LayeredConfiguration.h>

#include <dirent.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
/// A sibling jail directory the wsd side plants next to the real jails before loading the
/// document. The kit must not be able to read it.
constexpr const char* DecoyJailName = "unit-landlock-decoy";
constexpr const char* DecoySecretName = "secret.txt";

/// The path with any trailing slashes and the last component removed.
std::string parentOf(std::string path)
{
    while (!path.empty() && path.back() == '/')
        path.pop_back();
    const std::size_t n = path.find_last_of('/');
    return n == std::string::npos ? std::string() : path.substr(0, n);
}
}

class UnitLandlock : public UnitWSDClient
{
    STATE_ENUM(Phase, Load, WaitResult) _phase;

    /// The configured jails directory. The per-process jails parent is created beneath it.
    std::string _childRootBase;

public:
    UnitLandlock()
        : UnitWSDClient("UnitLandlock")
        , _phase(Phase::Load)
    {
        setHasKitHooks();
        // Kit spawning without capabilities can be slow, so allow 4x the default.
        setTimeout(std::chrono::minutes(2));
    }

    void configure(Poco::Util::LayeredConfiguration& config) override
    {
        UnitWSD::configure(config);

        // The chroot jail and the mount namespaces both need capabilities, so
        // this leaves landlock as the only lock-down of the kit processes.
        config.setBool("security.capabilities", false);

        _childRootBase = config.getString("child_root_path", std::string());
    }

    void endTest(const std::string& reason) override
    {
        if (!_childRootBase.empty())
            FileUtil::removeFile(_childRootBase + '/' + DecoyJailName, /*recursive=*/true);
        UnitWSDClient::endTest(reason);
    }

    void invokeWSDTest() override
    {
        switch (_phase)
        {
            case Phase::Load:
            {
                TRANSITION_STATE(_phase, Phase::WaitResult);

                if (!Landlock::isSupported())
                {
                    passTest("Landlock is not available on this kernel, nothing to verify");
                    return;
                }

                if (_childRootBase.empty())
                {
                    failTest("The child_root_path setting is empty");
                    return;
                }

                // Plant a decoy sibling jail with a secret for the kit to probe.
                const std::string decoyDir = _childRootBase + '/' + DecoyJailName;
                Poco::File(decoyDir).createDirectories();
                std::ofstream secret(decoyDir + '/' + DecoySecretName);
                secret << "the kit must not be able to read this" << std::endl;
                secret.close();
                TST_LOG("Created decoy jail: " << decoyDir);

                connectAndLoadLocalDocument("empty.odt");
                break;
            }
            case Phase::WaitResult:
            {
                // wait for the kit-side verdict, delivered as a unitresult: message
                break;
            }
        }
    }
};

// Inside the kit process
class UnitKitLandlock : public UnitKit
{
    bool _checked;

    /// Appends a failure to the report when path can be opened as a directory, or when opening
    /// it fails with the wrong errno.
    static void checkDirDenied(std::ostringstream& failures, const std::string& path)
    {
        DIR* dir = opendir(path.c_str());
        if (dir)
        {
            closedir(dir);
            failures << "directory [" << path << "] can be listed; ";
        }
        else if (errno != EACCES)
            failures << "listing [" << path << "] failed with errno " << errno
                     << " instead of EACCES; ";
    }

    void runChecks()
    {
        std::ostringstream failures;

        // a locked-down kit runs with the no-new-privs flag set
        if (prctl(PR_GET_NO_NEW_PRIVS, 0, 0, 0, 0) != 1)
            failures << "the process can still acquire new privileges; ";

        // The jail tmp directory is <base>/<per-process dir>/<jailId>/tmp, so the jails
        // parent is two levels up and the configured jails directory is three levels up.
        const std::string tmpDir = FileUtil::getSysTempDirectoryPath();
        const std::string jailDir = parentOf(tmpDir);
        const std::string childRoot = parentOf(jailDir);
        const std::string childRootBase = parentOf(childRoot);
        TST_LOG("Checking lock-down with jail [" << jailDir << "] under [" << childRoot << ']');

        if (childRoot.empty() || jailDir.empty() || childRootBase.empty())
            failures << "cannot derive the jails parent from tmp dir [" << tmpDir << "]; ";

        // control: the jail's own tmp directory stays read-write
        const std::string probePath = tmpDir + "/unit-landlock-probe.txt";
        std::ofstream probe(probePath);
        probe << "probe" << std::endl;
        probe.close();
        if (!FileUtil::Stat(probePath).exists())
            failures << "cannot write inside the own jail tmp directory; ";

        // control: shortening a file in the own jail tmp directory is possible
        if (truncate(probePath.c_str(), 0) != 0)
            failures << "cannot truncate a file in the own jail tmp directory; ";
        if (FileUtil::Stat(probePath).size() != 0)
            failures << "truncating a file in the own jail tmp directory left it unchanged; ";

        // control: moving a file from one directory to another
        if (Landlock::allowsCrossDirectoryRename())
        {
            const std::string subDir = tmpDir + "/unit-landlock-subdir";
            Poco::File(subDir).createDirectories();
            const std::string movedPath = subDir + "/unit-landlock-moved.txt";
            if (rename(probePath.c_str(), movedPath.c_str()) != 0)
                failures << "cannot move a file into a directory inside the own jail; ";
            else if (rename(movedPath.c_str(), probePath.c_str()) != 0)
                failures << "cannot move a file back out of a directory inside the own jail; ";
            FileUtil::removeFile(subDir, /*recursive=*/true);
        }

        // a file cannot be moved out of the jail
        if (rename(probePath.c_str(), (childRoot + "/unit-landlock-escaped.txt").c_str()) == 0)
            failures << "a file can be moved out of the jail; ";

        FileUtil::removeFile(probePath);

        // the jails parent directory can neither be listed nor traversed into siblings
        checkDirDenied(failures, childRoot);
        checkDirDenied(failures, childRootBase + '/' + DecoyJailName);

        // the secret of another jail stays unreadable
        const std::string secretPath = childRootBase + '/' + DecoyJailName + '/' + DecoySecretName;
        const int secretFd = open(secretPath.c_str(), O_RDONLY | O_CLOEXEC);
        if (secretFd >= 0)
        {
            close(secretFd);
            failures << "secret [" << secretPath << "] of another jail can be read; ";
        }
        else if (errno != EACCES)
            failures << "reading [" << secretPath << "] failed with errno " << errno
                     << " instead of EACCES; ";

        // the secret of another jail cannot be destroyed
        if (Landlock::restrictsTruncate())
        {
            if (truncate(secretPath.c_str(), 0) == 0)
                failures << "secret [" << secretPath << "] of another jail can be truncated; ";
            else if (errno != EACCES)
                failures << "truncating [" << secretPath << "] failed with errno " << errno
                         << " instead of EACCES; ";
        }

        // no reason to list, only access to specific files allowed
        checkDirDenied(failures, "/etc");

        // control: one of the few files that are still allowed
        const int passwdFd = open("/etc/passwd", O_RDONLY | O_CLOEXEC);
        if (passwdFd < 0)
            failures << "cannot read /etc/passwd; ";
        else
            close(passwdFd);

        // nothing can be created in the jails parent directory
        if (mkdir((childRoot + "/unit-landlock-escape").c_str(), S_IRWXU) == 0)
            failures << "a directory can be created in the jails parent; ";
        const int escapeFd =
            open((childRoot + "/unit-landlock-escape.txt").c_str(),
                 O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, S_IRUSR | S_IWUSR);
        if (escapeFd >= 0)
        {
            close(escapeFd);
            failures << "a file can be created in the jails parent; ";
        }

        const std::string report = failures.str();
        if (report.empty())
            passTest("Kit is locked down: " + jailDir);
        else
            failTest("Kit is not locked down: " + report);
    }

public:
    UnitKitLandlock()
        : UnitKit("UnitKitLandlock")
        , _checked(false)
    {
        // The checks only run once the first document session exists, so give
        // the client side time to connect. 4x the default.
        setTimeout(std::chrono::minutes(2));
    }

    void initialize() override
    {
        // Empty, so the socket poll thread is not started, which is unused by this test.
    }

    void postKitSessionCreated(Session* /*session*/) override
    {
        // The session exists, so the jail setup and the landlock lock-down are long done.
        if (_checked)
            return;
        _checked = true;

        if (!Landlock::isSupported())
        {
            passTest("Landlock is not available on this kernel, nothing to verify");
            return;
        }

        runChecks();
    }
};

UnitBase* unit_create_wsd(void) { return new UnitLandlock(); }

UnitBase* unit_create_kit(void) { return new UnitKitLandlock(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
