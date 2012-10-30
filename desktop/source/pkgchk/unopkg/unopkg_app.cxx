/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "desktopdllapi.h"
#include "dp_misc.h"
#include "unopkg_main.h"
#include "unopkg_shared.h"
#include "dp_identifier.hxx"
#include "tools/extendapplicationenvironment.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/conditn.hxx"
#include "osl/file.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/anytostring.hxx"
#include "comphelper/sequence.hxx"
#include "com/sun/star/deployment/ExtensionManager.hpp"

#include "com/sun/star/deployment/ui/PackageManagerDialog.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"
#include "boost/scoped_array.hpp"
#include "com/sun/star/ui/dialogs/XDialogClosedListener.hpp"
#include "com/sun/star/bridge/BridgeFactory.hpp"
#include <stdio.h>
#include <vector>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::unopkg;
using ::rtl::OUString;
namespace css = ::com::sun::star;
namespace {

struct ExtensionName
{
    OUString m_str;
    ExtensionName( OUString const & str ) : m_str( str ) {}
    bool operator () ( Reference<deployment::XPackage> const & e ) const
    {
        if (m_str.equals(dp_misc::getIdentifier(e))
             ||  m_str.equals(e->getName()))
            return true;
        return false;
    }
};

//------------------------------------------------------------------------------
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
"http://wiki.services.openoffice.org/wiki/Documentation/DevGuide/Extensions/Extensions\n\n";

//------------------------------------------------------------------------------
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

    { 0, 0, '\0', false }
};

class DialogClosedListenerImpl :
    public ::cppu::WeakImplHelper1< ui::dialogs::XDialogClosedListener >
{
    osl::Condition & m_rDialogClosedCondition;

public:
    DialogClosedListenerImpl( osl::Condition & rDialogClosedCondition )
        : m_rDialogClosedCondition( rDialogClosedCondition ) {}

    // XEventListener (base of XDialogClosedListener)
    virtual void SAL_CALL disposing( lang::EventObject const & Source )
        throw (RuntimeException);

    // XDialogClosedListener
    virtual void SAL_CALL dialogClosed(
        ui::dialogs::DialogClosedEvent const & aEvent )
        throw (RuntimeException);
};

// XEventListener (base of XDialogClosedListener)
void DialogClosedListenerImpl::disposing( lang::EventObject const & )
    throw (RuntimeException)
{
    // nothing to do
}

// XDialogClosedListener
void DialogClosedListenerImpl::dialogClosed(
    ui::dialogs::DialogClosedEvent const & )
    throw (RuntimeException)
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

extern "C" DESKTOP_DLLPUBLIC int unopkg_main()
{
    tools::extendApplicationEnvironment();
    bool bNoOtherErrorMsg = false;
    OUString subCommand;
    bool option_shared = false;
    bool option_force = false;
    bool option_verbose = false;
    bool option_bundled = false;
    bool option_suppressLicense = false;
    bool subcmd_add = false;
    bool subcmd_gui = false;
    OUString logFile;
    OUString repository;
    OUString cmdArg;
    ::std::vector<OUString> cmdPackages;

    OptionInfo const * info_shared = getOptionInfo(
        s_option_infos, OUSTR("shared") );
    OptionInfo const * info_force = getOptionInfo(
        s_option_infos, OUSTR("force") );
    OptionInfo const * info_verbose = getOptionInfo(
        s_option_infos, OUSTR("verbose") );
    OptionInfo const * info_log = getOptionInfo(
        s_option_infos, OUSTR("log-file") );
    OptionInfo const * info_context = getOptionInfo(
        s_option_infos, OUSTR("deployment-context") );
    OptionInfo const * info_help = getOptionInfo(
        s_option_infos, OUSTR("help") );
    OptionInfo const * info_version = getOptionInfo(
        s_option_infos, OUSTR("version") );
    OptionInfo const * info_bundled = getOptionInfo(
        s_option_infos, OUSTR("bundled") );
    OptionInfo const * info_suppressLicense = getOptionInfo(
        s_option_infos, OUSTR("suppress-license") );


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
        subcmd_add = subCommand == "add";
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
                                 OUSTR("\nERROR: unexpected option ") +
                                 cmdArg +
                                 OUSTR("!\n") +
                                 OUSTR("       Use " APP_NAME " ") +
                                 toString(info_help) +
                                 OUSTR(" to print all options.\n"));
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
                repository = OUSTR("shared");
            else if (option_bundled)
                repository = OUSTR("bundled");
            else
                repository = OUSTR("user");
        }
        else
        {
            if ( repository == "shared" ) {
                option_shared = true;
            }
            else if (option_shared) {
                dp_misc::writeConsoleError(
                    OUSTR("WARNING: explicit context given!  ") +
                    OUSTR("Ignoring option ") +
                    toString( info_shared ) +
                    OUSTR("!\n") );
            }
        }

        if (subCommand.equals(OUSTR("reinstall")))
        {
            //We must prevent that services and types are loaded by UNO,
            //otherwise we cannot delete the registry data folder.
            OUString extensionUnorc;
            if (repository.equals(OUSTR("user")))
                extensionUnorc = OUSTR("$UNO_USER_PACKAGES_CACHE/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc");
            else if (repository.equals(OUSTR("shared")))
                extensionUnorc = OUSTR("$SHARED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc");
            else if (repository.equals(OUSTR("bundled")))
                extensionUnorc = OUSTR("$BUNDLED_EXTENSIONS_USER/registry/com.sun.star.comp.deployment.component.PackageRegistryBackend/unorc");
            else
                OSL_ASSERT(0);

            ::rtl::Bootstrap::expandMacros(extensionUnorc);
            oslFileError e = osl_removeFile(extensionUnorc.pData);
            if (e != osl_File_E_None && e != osl_File_E_NOENT)
                throw Exception(OUSTR("Could not delete ") + extensionUnorc, 0);
        }

        xComponentContext = getUNO(
            option_verbose, option_shared, subcmd_gui, xLocalComponentContext );

        Reference<deployment::XExtensionManager> xExtensionManager(
            deployment::ExtensionManager::get( xComponentContext ) );

        Reference< ::com::sun::star::ucb::XCommandEnvironment > xCmdEnv(
            createCmdEnv( xComponentContext, logFile,
                          option_force, option_verbose, option_suppressLicense) );

        //synchronize bundled/shared extensions
        //Do not synchronize when command is "reinstall". This could add types and services to UNO and
        //prevent the deletion of the registry data folder
        //synching is done in XExtensionManager.reinstall
        if (!subcmd_gui && ! subCommand.equals(OUSTR("reinstall"))
            && ! dp_misc::office_is_running())
            dp_misc::syncRepositories(false, xCmdEnv);

        if ( subcmd_add || subCommand == "remove" )
        {
            for ( ::std::size_t pos = 0; pos < cmdPackages.size(); ++pos )
            {
                OUString const & cmdPackage = cmdPackages[ pos ];
                if (subcmd_add)
                {
                    beans::NamedValue nvSuppress(
                        OUSTR("SUPPRESS_LICENSE"), option_suppressLicense ?
                        makeAny(OUSTR("1")):makeAny(OUSTR("0")));
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
        else if (subCommand.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM("reinstall") ))
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
                    OUSTR("All deployed ") + repository + OUSTR(" extensions:\n\n"));
            }
            else
            {
                //The user provided the names (ids or file names) of the extensions
                //which shall be listed
                for ( ::std::size_t pos = 0; pos < cmdPackages.size(); ++pos )
                {
                    Reference<deployment::XPackage> extension;
                    try
                    {
                        extension = xExtensionManager->getDeployedExtension(
                            repository, cmdPackages[ pos ], cmdPackages[ pos ], xCmdEnv );
                    }
                    catch (const lang::IllegalArgumentException &)
                    {
                        extension = findPackage(repository,
                            xExtensionManager, xCmdEnv, cmdPackages[ pos ] );
                    }

                    //Now look if the requested extension has an unaccepted license
                    bool bUnacceptedLic = false;
                    if (!extension.is())
                    {
                        ::std::vector<Reference<deployment::XPackage> >::const_iterator
                            i = ::std::find_if(
                                vecExtUnaccepted.begin(),
                                vecExtUnaccepted.end(), ExtensionName(cmdPackages[pos]));
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
                            OUSTR("There is no such extension deployed: ") +
                            cmdPackages[pos],0,-1);
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

            for ( ::std::size_t pos = 0; pos < cmdPackages.size(); ++pos )
            {
                Reference<deployment::XPackage> extension;
                try
                {
                    extension = xExtensionManager->getDeployedExtension(
                        repository, cmdPackages[ pos ], cmdPackages[ pos ], xCmdEnv );
                }
                catch (const lang::IllegalArgumentException &)
                {
                    extension = findPackage(
                        repository, xExtensionManager, xCmdEnv, cmdPackages[ pos ] );
                }

                if (!extension.is())
                {
                    ::std::vector<Reference<deployment::XPackage> >::const_iterator
                        i = ::std::find_if(
                            vecExtUnaccepted.begin(),
                            vecExtUnaccepted.end(), ExtensionName(cmdPackages[pos]));
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
                OUSTR("\nERROR: unknown sub-command ") +
                subCommand +
                OUSTR("!\n") +
                OUSTR("       Use " APP_NAME " ") +
                toString(info_help) +
                OUSTR(" to print all options.\n"));
            return 1;
        }

        if (option_verbose)
            dp_misc::writeConsole(OUSTR("\n" APP_NAME " done.\n"));
        //Force to release all bridges which connect us to the child processes
        dp_misc::disposeBridges(xLocalComponentContext);
        return 0;
    }
    catch (const ucb::CommandFailedException &e)
    {
        dp_misc::writeConsoleError(e.Message + OUSTR("\n"));
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

        dp_misc::writeConsoleError(
            OUSTR("\nERROR: ") + exc.Message + OUSTR("\n"));
        if (!cause.isEmpty())
            dp_misc::writeConsoleError(
                OUSTR("       Cause: ") + cause + OUSTR("\n"));
    }
    catch (const LockFileException & e)
    {
        if (!subcmd_gui)
            dp_misc::writeConsoleError(e.Message);
        bNoOtherErrorMsg = true;
    }
    catch (const ::com::sun::star::uno::Exception & e ) {
        Any exc( ::cppu::getCaughtException() );

        dp_misc::writeConsoleError(
            OUSTR("\nERROR: ") +
            OUString(option_verbose  ? e.Message + OUSTR("\nException details: \n") +
            ::comphelper::anyToString(exc) : e.Message) +
            OUSTR("\n"));
    }
    if (!bNoOtherErrorMsg)
        dp_misc::writeConsoleError("\n" APP_NAME " failed.\n");
    dp_misc::disposeBridges(xLocalComponentContext);
    return 1;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
