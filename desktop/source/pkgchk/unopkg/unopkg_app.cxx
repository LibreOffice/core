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


#include <desktop/dllapi.h>
#include "dp_misc.h"
#include "unopkg_main.h"
#include "unopkg_shared.h"
#include "dp_identifier.hxx"
#include <tools/extendapplicationenvironment.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/conditn.hxx>
#include <osl/file.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/anytostring.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/deployment/DeploymentException.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>

#include <com/sun/star/deployment/ui/PackageManagerDialog.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XDialogClosedListener.hpp>
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <stdio.h>
#if defined(UNX)
  #include <unistd.h>
#endif
#include <vector>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::unopkg;
namespace {

struct ExtensionName
{
    OUString m_str;
    explicit ExtensionName( OUString const & str ) : m_str( str ) {}
    bool operator () ( Reference<deployment::XPackage> const & e ) const
    {
        if (m_str.equals(dp_misc::getIdentifier(e))
             ||  m_str.equals(e->getName()))
            return true;
        return false;
    }
};


const char s_usingText [] =
"\n"
"using: " APP_NAME " add <options> extension-path...\n"
"       " APP_NAME " validate <options> extension-identifier...\n"
"       " APP_NAME " remove <options> extension-identifier...\n"
"       " APP_NAME " list <options> extension-identifier...\n"
"       " APP_NAME " reinstall <options>\n"
"       " APP_NAME " gui\n"
"       " APP_NAME " -V\n"
"       " APP_NAME " -h\n"
"\n"
"sub-commands:\n"
" add                     add extension\n"
" validate                checks the prerequisites of an installed extension and"
"                         registers it if possible\n"
" remove                  remove extensions by identifier\n"
" reinstall               expert feature: reinstall all deployed extensions\n"
" list                    list information about deployed extensions\n"
" gui                     raise Extension Manager Graphical User Interface (GUI)\n"
"\n"
"options:\n"
" -h, --help              this help\n"
" -V, --version           version information\n"
" -v, --verbose           verbose output to stdout\n"
" -f, --force             force overwriting existing extensions\n"
" -s, --suppress-license  prevents showing the license\n"
" --log-file <file>       custom log file; default: <cache-dir>/log.txt\n"
" --shared                expert feature: operate on shared installation\n"
"                                         deployment context;\n"
"                                         run only when no concurrent Office\n"
"                                         process(es) are running!\n"
" --bundled               expert feature: operate on bundled extensions. Only\n"
"                                         works with list, validate, reinstall;\n"
" --deployment-context    expert feature: explicit deployment context\n"
"     <context>\n"
"\n"
"To learn more about the Extension Manager and extensions, see:\n"
"http://wiki.openoffice.org/wiki/Documentation/DevGuide/Extensions/Extensions\n\n";


const OptionInfo s_option_infos [] = {
    { RTL_CONSTASCII_STRINGPARAM("help"), 'h', false },
    { RTL_CONSTASCII_STRINGPARAM("version"), 'V', false },
    { RTL_CONSTASCII_STRINGPARAM("verbose"), 'v', false },
    { RTL_CONSTASCII_STRINGPARAM("force"), 'f', false },
    { RTL_CONSTASCII_STRINGPARAM("log-file"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("shared"), '\0', false },
    { RTL_CONSTASCII_STRINGPARAM("deployment-context"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("bundled"), '\0', false},
    { RTL_CONSTASCII_STRINGPARAM("suppress-license"), 's', false},

    { nullptr, 0, '\0', false }
};

class DialogClosedListenerImpl :
    public ::cppu::WeakImplHelper< ui::dialogs::XDialogClosedListener >
{
    osl::Condition & m_rDialogClosedCondition;

public:
    explicit DialogClosedListenerImpl( osl::Condition & rDialogClosedCondition )
        : m_rDialogClosedCondition( rDialogClosedCondition ) {}

    // XEventListener (base of XDialogClosedListener)
    virtual void SAL_CALL disposing( lang::EventObject const & Source ) override;

    // XDialogClosedListener
    virtual void SAL_CALL dialogClosed(
        ui::dialogs::DialogClosedEvent const & aEvent ) override;
};

// XEventListener (base of XDialogClosedListener)
void DialogClosedListenerImpl::disposing( lang::EventObject const & )
{
    // nothing to do
}

// XDialogClosedListener
void DialogClosedListenerImpl::dialogClosed(
    ui::dialogs::DialogClosedEvent const & )
{
    m_rDialogClosedCondition.set();
}

// If a package had been installed with a pre OOo 2.2, it could not normally be
// found via its identifier; similarly (and for ease of use), a package
// installed with OOo 2.2 or later could not normally be found via its file
// name.
Reference<deployment::XPackage> findPackage(
    OUString const & repository,
    Reference<deployment::XExtensionManager> const & manager,
    Reference<ucb::XCommandEnvironment > const & environment,
    OUString const & idOrFileName )
{
    Sequence< Reference<deployment::XPackage> > ps(
        manager->getDeployedExtensions(repository,
            Reference<task::XAbortChannel>(), environment ) );
    for ( sal_Int32 i = 0; i < ps.getLength(); ++i )
        if ( dp_misc::getIdentifier( ps[i] ) == idOrFileName )
            return ps[i];
    for ( sal_Int32 i = 0; i < ps.getLength(); ++i )
        if ( ps[i]->getName() == idOrFileName )
            return ps[i];
    return Reference<deployment::XPackage>();
}

} // anon namespace

extern "C" int unopkg_main()
{
    tools::extendApplicationEnvironment();
    bool bNoOtherErrorMsg = false;
    OUString subCommand;
    bool option_shared = false;
    bool option_force = false;
    bool option_verbose = false;
    bool option_bundled = false;
    bool option_suppressLicense = false;
    bool option_help = false;
    bool subcmd_gui = false;
    OUString logFile;
    OUString repository;
    OUString cmdArg;
    ::std::vector<OUString> cmdPackages;

    OptionInfo const * info_shared = getOptionInfo(
        s_option_infos, "shared" );
    OptionInfo const * info_force = getOptionInfo(
        s_option_infos, "force" );
    OptionInfo const * info_verbose = getOptionInfo(
        s_option_infos, "verbose" );
    OptionInfo const * info_log = getOptionInfo(
        s_option_infos, "log-file" );
    OptionInfo const * info_context = getOptionInfo(
        s_option_infos, "deployment-context" );
    OptionInfo const * info_help = getOptionInfo(
        s_option_infos, "help" );
    OptionInfo const * info_version = getOptionInfo(
        s_option_infos, "version" );
    OptionInfo const * info_bundled = getOptionInfo(
        s_option_infos, "bundled" );
    OptionInfo const * info_suppressLicense = getOptionInfo(
        s_option_infos, "suppress-license" );


    Reference<XComponentContext> xComponentContext;
    Reference<XComponentContext> xLocalComponentContext;

    try {
        sal_uInt32 nPos = 0;
        sal_uInt32 nCount = osl_getCommandArgCount();
        if (nCount == 0 || isOption( info_help, &nPos ))
        {
            dp_misc::writeConsole(s_usingText);
            return 0;
        }
        else if (isOption( info_version, &nPos )) {
            dp_misc::writeConsole("\n" APP_NAME " Version 3.3\n");
            return 0;
        }
        //consume all bootstrap variables which may occur before the subcommannd
        while(isBootstrapVariable(&nPos))
            ;

        if(nPos >= nCount)
            return 0;
        //get the sub command
        osl_getCommandArg( nPos, &subCommand.pData );
        ++nPos;
        subCommand = subCommand.trim();
        bool subcmd_add = subCommand == "add";
        subcmd_gui = subCommand == "gui";

        // sun-command options and packages:
        while (nPos < nCount)
        {
            if (readArgument( &cmdArg, info_log, &nPos )) {
                logFile = makeAbsoluteFileUrl(
                    cmdArg.trim(), getProcessWorkingDir() );
            }
            else if (!readOption( &option_verbose, info_verbose, &nPos ) &&
                     !readOption( &option_shared, info_shared, &nPos ) &&
                     !readOption( &option_force, info_force, &nPos ) &&
                     !readOption( &option_bundled, info_bundled, &nPos ) &&
                     !readOption( &option_suppressLicense, info_suppressLicense, &nPos ) &&
                     !readOption( &option_help, info_help, &nPos ) &&
                     !readArgument( &repository, info_context, &nPos ) &&
                     !isBootstrapVariable(&nPos))
            {
                osl_getCommandArg( nPos, &cmdArg.pData );
                ++nPos;
                cmdArg = cmdArg.trim();
                if (!cmdArg.isEmpty())
                {
                    if (cmdArg[ 0 ] == '-')
                    {
                        // is option:
                        dp_misc::writeConsoleError(
                                 "\nERROR: unexpected option " +
                                 cmdArg +
                                 "!\n       Use " APP_NAME " " +
                                 toString(info_help) +
                                 " to print all options.\n");
                        return 1;
                    }
                    else
                    {
                        // is package:
                        cmdPackages.push_back(
                            subcmd_add || subcmd_gui
                            ? makeAbsoluteFileUrl(
                                cmdArg, getProcessWorkingDir() )
                            : cmdArg );
                    }
                }
            }
        }

        if (repository.isEmpty())
        {
            if (option_shared)
                repository = "shared";
            else if (option_bundled)
                repository = "bundled";
            else
                repository = "user";
        }
        else
        {
            if ( repository == "shared" ) {
                option_shared = true;
            }
            else if (option_shared) {
                dp_misc::writeConsoleError(
                    "WARNING: explicit context given!  Ignoring option " +
                    toString( info_shared ) + "!\n" );
            }
        }
#if defined(UNX)
        if ( geteuid() == 0 )
        {
            if ( !(option_shared || option_bundled || option_help) )
            {
                dp_misc::writeConsoleError(
                    "ERROR: cannot run "  APP_NAME  " as root without " +
                    toString( info_shared ) + " or " + toString( info_bundled )
                    + " option.\n");
                return 1;
            }

        }
#endif

        if (subCommand == "reinstall")
        {
            //We must prevent that services and types are loaded by UNO,
            //otherwise we cannot delete the registry data folder.
            OUString extensionUnorc;
            if (repository == "user")
                extensionUnorc = "$UNO_USER_PACKAGES_CACHE/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc";
            else if (repository == "shared")
                extensionUnorc = "$SHARED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc";
            else if (repository == "bundled")
                extensionUnorc = "$BUNDLED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc";
            else
                OSL_ASSERT(false);

            ::rtl::Bootstrap::expandMacros(extensionUnorc);
            oslFileError e = osl_removeFile(extensionUnorc.pData);
            if (e != osl_File_E_None && e != osl_File_E_NOENT)
                throw Exception("Could not delete " + extensionUnorc, nullptr);
        }

        xComponentContext = getUNO(
            option_verbose, option_shared, subcmd_gui, xLocalComponentContext );

        Reference<deployment::XExtensionManager> xExtensionManager(
            deployment::ExtensionManager::get( xComponentContext ) );

        Reference< css::ucb::XCommandEnvironment > xCmdEnv(
            createCmdEnv( xComponentContext, logFile,
                          option_force, option_verbose, option_suppressLicense) );

        //synchronize bundled/shared extensions
        //Do not synchronize when command is "reinstall". This could add types and services to UNO and
        //prevent the deletion of the registry data folder
        //synching is done in XExtensionManager.reinstall
        if (!subcmd_gui && ! (subCommand == "reinstall")
            && ! dp_misc::office_is_running())
            dp_misc::syncRepositories(false, xCmdEnv);

        if ( subcmd_add || subCommand == "remove" )
        {
            for (OUString & cmdPackage : cmdPackages)
            {
                if (subcmd_add)
                {
                    beans::NamedValue nvSuppress(
                        "SUPPRESS_LICENSE", option_suppressLicense ?
                        makeAny(OUString("1")):makeAny(OUString("0")));
                        xExtensionManager->addExtension(
                            cmdPackage, Sequence<beans::NamedValue>(&nvSuppress, 1),
                            repository, Reference<task::XAbortChannel>(), xCmdEnv);
                }
                else
                {
                    try
                    {
                        xExtensionManager->removeExtension(
                            cmdPackage, cmdPackage, repository,
                            Reference<task::XAbortChannel>(), xCmdEnv );
                    }
                    catch (const lang::IllegalArgumentException &)
                    {
                        Reference<deployment::XPackage> p(
                             findPackage(repository,
                                xExtensionManager, xCmdEnv, cmdPackage ) );
                        if ( !p.is())
                            throw;
                        else if (p.is())
                            xExtensionManager->removeExtension(
                                ::dp_misc::getIdentifier(p), p->getName(),
                                repository,
                                Reference<task::XAbortChannel>(), xCmdEnv );
                    }
                }
            }
        }
        else if ( subCommand == "reinstall" )
        {
            xExtensionManager->reinstallDeployedExtensions(
                false, repository, Reference<task::XAbortChannel>(), xCmdEnv);
        }
        else if ( subCommand == "list" )
        {
            ::std::vector<Reference<deployment::XPackage> > vecExtUnaccepted;
            ::comphelper::sequenceToContainer(vecExtUnaccepted,
                    xExtensionManager->getExtensionsWithUnacceptedLicenses(
                        repository, xCmdEnv));

            //This vector tells what XPackage  in allExtensions has an
            //unaccepted license.
            std::vector<bool> vecUnaccepted;
            std::vector<Reference<deployment::XPackage> > allExtensions;
            if (cmdPackages.empty())
            {
                Sequence< Reference<deployment::XPackage> >
                    packages = xExtensionManager->getDeployedExtensions(
                        repository, Reference<task::XAbortChannel>(), xCmdEnv );

                ::std::vector<Reference<deployment::XPackage> > vec_packages;
                ::comphelper::sequenceToContainer(vec_packages, packages);

                //First copy the extensions with the unaccepted license
                //to vector allExtensions.
                allExtensions.resize(vecExtUnaccepted.size() + vec_packages.size());

                ::std::vector<Reference<deployment::XPackage> >::iterator i_all_ext =
                      ::std::copy(vecExtUnaccepted.begin(), vecExtUnaccepted.end(),
                                  allExtensions.begin());
                //Now copy those we got from getDeployedExtensions
                ::std::copy(vec_packages.begin(), vec_packages.end(), i_all_ext);

                //Now prepare the vector which tells what extension has an
                //unaccepted license
                vecUnaccepted.resize(vecExtUnaccepted.size() + vec_packages.size());
                ::std::fill_n(vecUnaccepted.begin(), vecExtUnaccepted.size(), true);
                ::std::fill_n(vecUnaccepted.begin() + vecExtUnaccepted.size(),
                      vec_packages.size(), false);

                dp_misc::writeConsole(
                    "All deployed " + repository + " extensions:\n\n");
            }
            else
            {
                //The user provided the names (ids or file names) of the extensions
                //which shall be listed
                for (OUString & cmdPackage : cmdPackages)
                {
                    Reference<deployment::XPackage> extension;
                    try
                    {
                        extension = xExtensionManager->getDeployedExtension(
                            repository, cmdPackage, cmdPackage, xCmdEnv );
                    }
                    catch (const lang::IllegalArgumentException &)
                    {
                        extension = findPackage(repository,
                            xExtensionManager, xCmdEnv, cmdPackage );
                    }

                    //Now look if the requested extension has an unaccepted license
                    bool bUnacceptedLic = false;
                    if (!extension.is())
                    {
                        ::std::vector<Reference<deployment::XPackage> >::const_iterator
                            i = ::std::find_if(
                                vecExtUnaccepted.begin(),
                                vecExtUnaccepted.end(), ExtensionName(cmdPackage));
                        if (i != vecExtUnaccepted.end())
                        {
                            extension = *i;
                            bUnacceptedLic = true;
                        }
                    }

                    if (extension.is())
                    {
                        allExtensions.push_back(extension);
                        vecUnaccepted.push_back(bUnacceptedLic);
                    }

                    else
                        throw lang::IllegalArgumentException(
                            "There is no such extension deployed: " +
                            cmdPackage,nullptr,-1);
                }

            }

            printf_packages(allExtensions, vecUnaccepted, xCmdEnv );
        }
        else if ( subCommand == "validate" )
        {
            ::std::vector<Reference<deployment::XPackage> > vecExtUnaccepted;
            ::comphelper::sequenceToContainer(
                vecExtUnaccepted, xExtensionManager->getExtensionsWithUnacceptedLicenses(
                    repository, xCmdEnv));

            for (OUString & cmdPackage : cmdPackages)
            {
                Reference<deployment::XPackage> extension;
                try
                {
                    extension = xExtensionManager->getDeployedExtension(
                        repository, cmdPackage, cmdPackage, xCmdEnv );
                }
                catch (const lang::IllegalArgumentException &)
                {
                    extension = findPackage(
                        repository, xExtensionManager, xCmdEnv, cmdPackage );
                }

                if (!extension.is())
                {
                    ::std::vector<Reference<deployment::XPackage> >::const_iterator
                        i = ::std::find_if(
                            vecExtUnaccepted.begin(),
                            vecExtUnaccepted.end(), ExtensionName(cmdPackage));
                    if (i != vecExtUnaccepted.end())
                    {
                        extension = *i;
                    }
                }

                if (extension.is())
                    xExtensionManager->checkPrerequisitesAndEnable(
                        extension, Reference<task::XAbortChannel>(), xCmdEnv);
            }
        }
        else if ( subCommand == "gui" )
        {
            Reference<ui::dialogs::XAsynchronousExecutableDialog> xDialog(
                deployment::ui::PackageManagerDialog::createAndInstall(
                    xComponentContext,
                    cmdPackages.size() > 0 ? cmdPackages[0] : OUString() ));

            osl::Condition dialogEnded;
            dialogEnded.reset();

            Reference< ui::dialogs::XDialogClosedListener > xListener(
                new DialogClosedListenerImpl( dialogEnded ) );

            xDialog->startExecuteModal(xListener);
            dialogEnded.wait();
            return 0;
        }
        else
        {
            dp_misc::writeConsoleError(
                "\nERROR: unknown sub-command " +
                subCommand + "!\n       Use " APP_NAME " " +
                toString(info_help) + " to print all options.\n");
            return 1;
        }

        if (option_verbose)
            dp_misc::writeConsole("\n" APP_NAME " done.\n");
        //Force to release all bridges which connect us to the child processes
        dp_misc::disposeBridges(xLocalComponentContext);
        return 0;
    }
    catch (const ucb::CommandFailedException &e)
    {
        dp_misc::writeConsoleError(e.Message + "\n");
        bNoOtherErrorMsg = true;
    }
    catch (const ucb::CommandAbortedException &)
    {
        dp_misc::writeConsoleError("\n" APP_NAME " aborted!\n");
    }
    catch (const deployment::DeploymentException & exc)
    {
        OUString cause;
        if (option_verbose)
        {
            cause = ::comphelper::anyToString(exc.Cause);
        }
        else
        {
            css::uno::Exception e;
            if (exc.Cause >>= e)
                cause = e.Message;
        }

        dp_misc::writeConsoleError("\nERROR: " + exc.Message + "\n");
        if (!cause.isEmpty())
            dp_misc::writeConsoleError("       Cause: " + cause + "\n");
    }
    catch (const LockFileException & e)
    {
        if (!subcmd_gui)
            dp_misc::writeConsoleError(e.Message + "\n");
        bNoOtherErrorMsg = true;
    }
    catch (const css::uno::Exception & e ) {
        Any exc( ::cppu::getCaughtException() );

        dp_misc::writeConsoleError("\nERROR: " +
            OUString(option_verbose ? e.Message + "\nException details: \n" +
            ::comphelper::anyToString(exc) : e.Message) + "\n");
    }
    if (!bNoOtherErrorMsg)
        dp_misc::writeConsoleError("\n" APP_NAME " failed.\n");
    dp_misc::disposeBridges(xLocalComponentContext);
    return 1;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
