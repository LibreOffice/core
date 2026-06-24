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
 * Configuration file manipulation utility.
 * Functions: Password generation, config setting/getting, XML editing
 */

#include <config.h>

#include <common/Anonymizer.hpp>
#include <common/ConfigUtil.hpp>
#include <common/Crypto.hpp>
#include <common/NumUtil.hpp>
#include <common/Util.hpp>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <Poco/Crypto/RSAKey.h>
#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/Util/Application.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/XMLConfiguration.h>

#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <termios.h>
#include <unistd.h>

using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::XMLConfiguration;

#define MIN_PWD_SALT_LENGTH 20
#define MIN_PWD_ITERATIONS 1000
#define MIN_PWD_HASH_LENGTH 20

bool EnableExperimental = false;

class CoolConfig final: public XMLConfiguration
{
public:
    CoolConfig() = default;
};

struct AdminConfig
{
private:
    unsigned _pwdSaltLength = 128;
    unsigned _pwdIterations = 10000;
    unsigned _pwdHashLength = 128;
    std::string _adminUser;
    std::string _adminPwd;

public:

    void setPwdSaltLength(unsigned pwdSaltLength) { _pwdSaltLength = pwdSaltLength; }
    unsigned getPwdSaltLength() const { return _pwdSaltLength; }
    void setPwdIterations(unsigned pwdIterations) { _pwdIterations = pwdIterations; }
    unsigned getPwdIterations() const { return _pwdIterations; }
    void setPwdHashLength(unsigned pwdHashLength) { _pwdHashLength = pwdHashLength; }
    unsigned getPwdHashLength() const { return _pwdHashLength; }
    const std::string& getAdminUser() const { return _adminUser; }
    void setAdminUser(const std::string& user) { _adminUser = user; }
    const std::string& getAdminPwd() const { return _adminPwd; }
    void setAdminPwd(const std::string& pwd) { _adminPwd = pwd; }
};

// Config tool to change coolwsd configuration (coolwsd.xml)
class Config: public Application
{
    // Display help information on the console
    void displayHelp();

    CoolConfig _coolConfig;

    AdminConfig _adminConfig;

public:
    static std::string ConfigFile;
    static std::string OldConfigFile;
    static std::string SupportKeyString;
    static bool SupportKeyStringProvided;
    static std::uint64_t AnonymizationSalt;
    static bool AnonymizationSaltProvided;
    static bool Write;

protected:
    void defineOptions(OptionSet&) override;
    void handleOption(const std::string&, const std::string&) override;
    int main(const std::vector<std::string>&) override;
};

std::string Config::ConfigFile =
#if ENABLE_DEBUG
    DEBUG_ABSSRCDIR
#else
    COOLWSD_CONFIGDIR
#endif
    "/coolwsd.xml";

std::string Config::OldConfigFile = "/etc/loolwsd/loolwsd.xml";
bool Config::Write = false;

std::string Config::SupportKeyString;
bool Config::SupportKeyStringProvided = false;
std::uint64_t Config::AnonymizationSalt = 0;
bool Config::AnonymizationSaltProvided = false;

int MigrateConfig(const std::string&, const std::string&, bool);

void Config::displayHelp()
{
    HelpFormatter helpFormatter(options());
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("COMMAND [OPTIONS]");
    helpFormatter.setHeader("coolconfig - Configuration tool for Collabora Online.\n"
                            "\n"
                            "Some options make sense only with a specific command.\n\n"
                            "Options:");

    helpFormatter.format(std::cout);

    // Command list
    std::cout << std::endl
              << "Commands: " << std::endl
              << "    migrateconfig [--old-config-file=<path>] [--config-file=<path>] [--write]" << std::endl
              << "        The migrateconfig command migrates config file of Collabora Online 6.4 or older to the new format." << std::endl
              << "    anonymize [string-1]...[string-n]" << std::endl
              << "    set-admin-password" << std::endl;
    if constexpr (ConfigUtil::isSupportKeyEnabled())
    {
        std::cout << "    set-support-key" << std::endl;
    }
    std::cout << "    set <key> <value>" << std::endl
              << "    generate-proof-key" << std::endl
              << "    update-system-template" << std::endl << std::endl;
}

void Config::defineOptions(OptionSet& optionSet)
{
    Application::defineOptions(optionSet);

    optionSet.addOption(Option("help", "h", "Show this usage information.")
                        .required(false)
                        .repeatable(false));
    optionSet.addOption(Option("config-file", "", "Specify configuration file path manually.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));
    optionSet.addOption(Option("old-config-file", "", "Specify configuration file path to migrate manually.")
                        .required(false)
                        .repeatable(false)
                        .argument("path"));

    optionSet.addOption(Option("pwd-salt-length", "", "Length of the salt to use to hash password [set-admin-password].")
                        .required(false)
                        .repeatable(false)
                        .argument("number"));
    optionSet.addOption(Option("pwd-iterations", "", "Number of iterations to do in PKDBF2 password hashing [set-admin-password].")
                        .required(false)
                        .repeatable(false)
                        .argument("number"));
    optionSet.addOption(Option("pwd-hash-length", "", "Length of password hash to generate [set-admin-password].")
                        .required(false)
                        .repeatable(false)
                        .argument("number"));
    optionSet.addOption(Option("user", "", "Admin user name [set-admin-password].")
                        .required(false)
                        .repeatable(false)
                        .argument("name"));
    optionSet.addOption(Option("password", "", "Admin user password [set-admin-password].")
                        .required(false)
                        .repeatable(false)
                        .argument("password"));

    if constexpr (ConfigUtil::isSupportKeyEnabled())
    {
        optionSet.addOption(Option("support-key", "", "Specify the support key [set-support-key].")
                            .required(false)
                            .repeatable(false)
                            .argument("key"));
    }

    optionSet.addOption(Option("anonymization-salt", "", "Anonymize strings with the given 64-bit salt instead of the one in the config file.")
                        .required(false)
                        .repeatable(false)
                        .argument("salt"));

    optionSet.addOption(Option("write", "", "Write migrated configuration.")
                        .required(false)
                        .repeatable(false));
}

void Config::handleOption(const std::string& optionName, const std::string& optionValue)
{
    Application::handleOption(optionName, optionValue);
    if (optionName == "help")
    {
        displayHelp();
        Util::forcedExit(EX_OK);
    }
    else if (optionName == "config-file")
    {
        ConfigFile = optionValue;
    }
    else if (optionName == "old-config-file")
    {
        OldConfigFile = optionValue;
    }
    else if (optionName == "pwd-salt-length")
    {
        unsigned len = NumUtil::stoi(optionValue);
        if (len < MIN_PWD_SALT_LENGTH)
        {
            len = MIN_PWD_SALT_LENGTH;
            std::cout << "Password salt length adjusted to minimum " << len << std::endl;
        }
        _adminConfig.setPwdSaltLength(len);
    }
    else if (optionName == "pwd-iterations")
    {
        unsigned len = NumUtil::stoi(optionValue);
        if (len < MIN_PWD_ITERATIONS)
        {
            len = MIN_PWD_ITERATIONS;
            std::cout << "Password iteration adjusted to minimum " << len << std::endl;
        }
        _adminConfig.setPwdIterations(len);
    }
    else if (optionName == "pwd-hash-length")
    {
        unsigned len = NumUtil::stoi(optionValue);
        if (len < MIN_PWD_HASH_LENGTH)
        {
            len = MIN_PWD_HASH_LENGTH;
            std::cout << "Password hash length adjusted to minimum " << len << std::endl;
        }
        _adminConfig.setPwdHashLength(len);
    }
    else if (optionName == "user")
    {
        _adminConfig.setAdminUser(optionValue);
    }
    else if (optionName == "password")
    {
        _adminConfig.setAdminPwd(optionValue);
    }
    else if (optionName == "support-key")
    {
        SupportKeyString = optionValue;
        SupportKeyStringProvided = true;
    }
    else if (optionName == "anonymization-salt")
    {
        AnonymizationSalt = std::stoull(optionValue);
        AnonymizationSaltProvided = true;
        std::cout << "Anonymization Salt: [" << AnonymizationSalt << "]." << std::endl;
    }
    else if (optionName == "write")
    {
        Write = true;
    }
}

// coverity[root_function] : don't warn about uncaught exceptions
int Config::main(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        std::cerr << "Nothing to do." << std::endl;
        displayHelp();
        return EX_NOINPUT;
    }

    int retval = EX_OK;
    bool changed = false;
    _coolConfig.load(ConfigFile);

    if (args[0] == "set-admin-password")
    {
        std::vector<unsigned char> pwdhash(_adminConfig.getPwdHashLength());
        std::vector<unsigned char> salt(_adminConfig.getPwdSaltLength());
        RAND_bytes(salt.data(), _adminConfig.getPwdSaltLength());
        std::stringstream stream;

        // Ask for admin username
        if (_adminConfig.getAdminUser().empty()) {
            std::string adminUser;
            std::cout << "Enter admin username [admin]: ";
            std::getline(std::cin, adminUser);
            if (adminUser.empty())
            {
                adminUser = "admin";
            }
            _adminConfig.setAdminUser(adminUser);
        }

        // Ask for user password
        if (_adminConfig.getAdminPwd().empty()) {
            termios oldTermios;
            tcgetattr(STDIN_FILENO, &oldTermios);
            termios newTermios = oldTermios;
            // Disable user input mirroring on console for password input
            newTermios.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &newTermios);
            std::string adminPwd;
            std::cout << "Enter admin password: ";
            std::getline(std::cin, adminPwd);
            std::string reAdminPwd;
            std::cout << std::endl << "Confirm admin password: ";
            std::getline(std::cin, reAdminPwd);
            std::cout << std::endl;
            // Set the termios to old state
            tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
            if (adminPwd != reAdminPwd)
            {
                std::cout << "Password mismatch." << std::endl;
                return EX_DATAERR;
            }
            _adminConfig.setAdminPwd(adminPwd);
        }

        // Do the magic !
        PKCS5_PBKDF2_HMAC(_adminConfig.getAdminPwd().c_str(), -1,
                          salt.data(), _adminConfig.getPwdSaltLength(),
                          _adminConfig.getPwdIterations(),
                          EVP_sha512(),
                          _adminConfig.getPwdHashLength(), pwdhash.data());

        // Make salt randomness readable
        for (unsigned j = 0; j < _adminConfig.getPwdSaltLength(); ++j)
            stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(salt[j]);
        const std::string saltHash = stream.str();

        // Clear our used hex stream to make space for password hash
        stream.str("");
        stream.clear();

        // Make the hashed password readable
        for (unsigned j = 0; j < _adminConfig.getPwdHashLength(); ++j)
            stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pwdhash[j]);
        const std::string passwordHash = stream.str();

        std::stringstream pwdConfigValue("pbkdf2.sha512.", std::ios_base::in | std::ios_base::out | std::ios_base::ate);
        pwdConfigValue << std::to_string(_adminConfig.getPwdIterations()) << '.';
        pwdConfigValue << saltHash << '.' << passwordHash;
        _coolConfig.setString("admin_console.username", _adminConfig.getAdminUser());
        _coolConfig.setString("admin_console.secure_password[@desc]",
                              "Salt and password hash combination generated using PBKDF2 with SHA512 digest.");
        _coolConfig.setString("admin_console.secure_password", pwdConfigValue.str());

        changed = true;
    }
    else if (ConfigUtil::isSupportKeyEnabled() && args[0] == "set-support-key")
    {
        std::string supportKeyString;
        if (SupportKeyStringProvided)
            supportKeyString = SupportKeyString;
        else
        {
            std::cout << "Enter support key: ";
            std::getline(std::cin, supportKeyString);
        }

        if (!supportKeyString.empty())
        {
            SupportKey key(supportKeyString);
            if (!key.verify())
                std::cerr << "Invalid key\n";
            else {
                int validDays =  key.validDaysRemaining();
                if (validDays <= 0)
                    std::cerr << "Valid but expired key\n";
                else
                {
                    std::cerr << "Valid for " << validDays << " days - setting to config\n";
                    _coolConfig.setString("support_key", supportKeyString);
                    changed = true;
                }
            }
        }
        else
        {
            std::cerr << "Removing empty support key\n";
            _coolConfig.remove("support_key");
            changed = true;
        }
    }
    else if (args[0] == "set")
    {
        if (args.size() == 3)
        {
            // args[1] = key
            // args[2] = value
            if (_coolConfig.has(args[1]))
            {
                const std::string val = _coolConfig.getString(args[1]);
                std::cout << "Previous value found in config file: \""  << val << '"' << std::endl;
                std::cout << "Changing value to: \"" << args[2] << '"' << std::endl;
            }
            else
            {
                std::cout << "No property, \"" << args[1] << "\"," << " found in config file." << std::endl;
                std::cout << "Adding it as new with value: \"" << args[2] << '"' << std::endl;
            }
            _coolConfig.setString(args[1], args[2]);
            changed = true;
        }
        else
            std::cerr << "set expects a key and value as arguments" << std::endl
                      << "Eg: " << std::endl
                      << "    set logging.level trace" << std::endl;

    }
    else if (args[0] == "update-system-template")
    {
        const char command[] = "coolwsd-systemplate-setup /opt/cool/systemplate " LO_PATH " >/dev/null 2>&1";
        std::cout << "Running the following command:" << std::endl
                  << command << std::endl;

        retval = system(command);
        if (retval != 0)
            std::cerr << "Error when executing command." << std::endl;
    }
    else if (args[0] == "anonymize")
    {
        if (!AnonymizationSaltProvided)
        {
            const std::string val = _coolConfig.getString("logging.anonymize.anonymization_salt");
            AnonymizationSalt = std::stoull(val);
            std::cout << "Anonymization Salt: [" << AnonymizationSalt << "]." << std::endl;
        }

        Anonymizer::initialize(true, AnonymizationSalt);

        for (std::size_t i = 1; i < args.size(); ++i)
        {
            std::cout << '[' << args[i] << "]: " << Anonymizer::anonymizeUrl(args[i]) << std::endl;
        }
    }
    else if (args[0] == "migrateconfig")
    {
        if (!Write)
            std::cout << "The migrateconfig command migrates config file of Collabora Online 6.4 or older to the new format." << std::endl;
        std::cout << "Migrating old configuration from " << OldConfigFile << " to " << ConfigFile << "." << std::endl;
        if (!Write)
            std::cout << "This is a dry run, no changes are written to file." << std::endl;
        std::cout << std::endl;
        const std::string OldConfigMigrated = OldConfigFile + ".migrated";
        Poco::File AlreadyMigrated(OldConfigMigrated);
        if (AlreadyMigrated.exists())
        {
            std::cout << "Migration already performed, file " + OldConfigMigrated + " exists. Aborting." << std::endl;
        }
        else
        {
            const int Result = MigrateConfig(OldConfigFile, ConfigFile, Write);
            if (Result == 0)
            {
                std::cout << "Successful migration." << std::endl;
                if (Write)
                {
                    Poco::File ConfigToRename(OldConfigFile);
                    ConfigToRename.renameTo(OldConfigMigrated);
                }
            }
            else
                std::cout << "Migration of old configuration failed." << std::endl;
        }
    }
    else if (args[0] == "generate-proof-key")
    {
        std::string proofKeyPath =
#if ENABLE_DEBUG
            DEBUG_ABSSRCDIR
#else
            COOLWSD_CONFIGDIR
#endif
            "/proof_key";

#if !ENABLE_DEBUG
        struct passwd* pwd;
        pwd = getpwnam(COOL_USER_ID);
        if (pwd == NULL)
        {
            std::cerr << "User '" COOL_USER_ID
                         "' does not exist. Please reinstall coolwsd package, or in case of manual "
                         "installation from source, create the '" COOL_USER_ID "' user manually."
                      << std::endl;
            return EX_NOUSER;
        }
#endif

        Poco::File proofKeyFile(proofKeyPath);
        if (!proofKeyFile.exists())
        {
            Poco::Crypto::RSAKey proofKey =
                Poco::Crypto::RSAKey(Poco::Crypto::RSAKey::KeyLength::KL_4096,
                                     Poco::Crypto::RSAKey::Exponent::EXP_LARGE);
            proofKey.save(proofKeyPath + ".pub", proofKeyPath, "" /*no password*/);
#if !ENABLE_DEBUG
            if (chmod(proofKeyPath.c_str(), S_IRUSR | S_IWUSR) != 0)
                std::cerr << "Changing mode of " + proofKeyPath + " failed: " << strerror(errno) << std::endl;
            if (chown(proofKeyPath.c_str(), pwd->pw_uid, -1) != 0)
                std::cerr << "Changing owner of " + proofKeyPath + " failed: " << strerror(errno) << std::endl;
#endif
        }
        else
        {
            std::cerr << proofKeyPath << " exists already. New proof key was not generated."
                      << std::endl;
        }
    }
    else
    {
        std::cerr << "No such command, \"" << args[0]  << '"' << std::endl;
        displayHelp();
    }

    if (changed)
    {
        std::cout << "Saving configuration to : " << ConfigFile << " ..." << std::endl;
        _coolConfig.save(ConfigFile);
        std::cout << "Saved" << std::endl;
    }

    return retval;
}

// coverity[root_function] : don't warn about uncaught exceptions
POCO_APP_MAIN(Config);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
