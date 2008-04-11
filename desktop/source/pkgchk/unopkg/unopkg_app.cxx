/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unopkg_app.cxx,v $
 * $Revision: 1.13 $
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
#include "precompiled_desktop.hxx"

#include "unopkg_main.h"
#include "unopkg_shared.h"
#include "dp_identifier.hxx"
#include "sal/main.h"
#include "tools/extendapplicationenvironment.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/conditn.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "comphelper/anytostring.hxx"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include "com/sun/star/deployment/ui/PackageManagerDialog.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "boost/scoped_array.hpp"
#include "com/sun/star/ui/dialogs/XDialogClosedListener.hpp"
#include "com/sun/star/bridge/XBridgeFactory.hpp"
#include <stdio.h>
#include <vector>

#define APP_NAME "unopkg"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::unopkg;
using ::rtl::OUString;
namespace css = ::com::sun::star;
namespace {

//------------------------------------------------------------------------------
const char s_usingText [] =
"\n"
"using: " APP_NAME " add <options> extension-path...\n"
"       " APP_NAME " remove <options> extension-identifier...\n"
"       " APP_NAME " list <options> extension-identifier...\n"
"       " APP_NAME " reinstall <options>\n"
"       " APP_NAME " gui\n"
"       " APP_NAME " -V\n"
"       " APP_NAME " -h\n"
"\n"
"sub-commands:\n"
" add                     add extension\n"
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
" --log-file <file>       custom log file; default: <cache-dir>/log.txt\n"
" --shared                expert feature: operate on shared installation\n"
"                                         deployment context;\n"
"                                         run only when no concurrent Office\n"
"                                         process(es) are running!\n"
" --deployment-context    expert feature: explicit deployment context\n"
"     <context>\n"
"\n"
"For details concerning deployment and this tool, please read the developer's\n"
"guide: http://api.openoffice.org/DevelopersGuide/" APP_NAME ".html\n";

//------------------------------------------------------------------------------
const OptionInfo s_option_infos [] = {
    { RTL_CONSTASCII_STRINGPARAM("help"), 'h', false },
    { RTL_CONSTASCII_STRINGPARAM("version"), 'V', false },
    { RTL_CONSTASCII_STRINGPARAM("verbose"), 'v', false },
    { RTL_CONSTASCII_STRINGPARAM("force"), 'f', false },
    { RTL_CONSTASCII_STRINGPARAM("log-file"), '\0', true },
    { RTL_CONSTASCII_STRINGPARAM("shared"), '\0', false },
    { RTL_CONSTASCII_STRINGPARAM("deployment-context"), '\0', true },
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
    Reference<deployment::XPackageManager> const & manager,
    Reference<ucb::XCommandEnvironment > const & environment,
    OUString const & idOrFileName )
{
    Sequence< Reference<deployment::XPackage> > ps(
        manager->getDeployedPackages(
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


//workaround for some reason the bridge threads which communicate with the uno.exe
//process are not releases on time
void disposeBridges(Reference<css::uno::XComponentContext> ctx)
{
    if (!ctx.is())
        return;

    Reference<css::bridge::XBridgeFactory> bridgeFac(
        ctx->getServiceManager()->createInstanceWithContext(
            OUSTR("com.sun.star.bridge.BridgeFactory"), ctx),
        UNO_QUERY);

    if (bridgeFac.is())
    {
        const Sequence< Reference<css::bridge::XBridge> >seqBridges = bridgeFac->getExistingBridges();
        for (sal_Int32 i = 0; i < seqBridges.getLength(); i++)
        {
            Reference<css::lang::XComponent> comp(seqBridges[i], UNO_QUERY);
            if (comp.is())
            {
                try {
                    comp->dispose();
                }
                catch (css::lang::DisposedException& )
                {
                }
            }
        }
    }
}

//##############################################################################
extern "C" int unopkg_main()
{
    tools::extendApplicationEnvironment();

    DisposeGuard disposeGuard;
    rtl_TextEncoding textenc = osl_getThreadTextEncoding();

    OUString subCommand;
    bool option_shared = false;
    bool option_force = false;
    bool option_verbose = false;
    bool subcmd_add = false;
    bool subcmd_gui = false;
    OUString logFile;
    OUString deploymentContext;
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

    Reference<XComponentContext> xComponentContext;
    Reference<XComponentContext> xLocalComponentContext;

    try {
        sal_uInt32 nPos = 0;
        sal_uInt32 nCount = osl_getCommandArgCount();
        if (nCount == 0 || isOption( info_help, &nPos )) {
            printf( "%s\n", s_usingText );
            return 0;
        }
        else if (isOption( info_version, &nPos )) {
            printf( "\n%s Version 1.0\n", APP_NAME );
            return 0;
        }
        else {
            osl_getCommandArg( 0, &subCommand.pData );
            ++nPos;
            subCommand = subCommand.trim();
            subcmd_add = subCommand.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("add") );
            subcmd_gui = subCommand.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("gui") );
        }

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
                     !readArgument( &deploymentContext, info_context, &nPos ) &&
                     !isBootstrapVariable(&nPos))
            {
                osl_getCommandArg( nPos, &cmdArg.pData );
                ++nPos;
                cmdArg = cmdArg.trim();
                if (cmdArg.getLength() > 0)
                {
                    if (cmdArg[ 0 ] == '-')
                    {
                        // is option:
                        fprintf( stderr,
                                 "\nERROR: unexpected option %s!\n"
                                 "       Use " APP_NAME
                                 " %s to print all options.\n",
                                 ::rtl::OUStringToOString(
                                     cmdArg, textenc ).getStr(),
                                 toString( info_help ).getStr() );
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

        xComponentContext = getUNO(
            disposeGuard, option_verbose, option_shared, subcmd_gui,
            xLocalComponentContext );

        if (deploymentContext.getLength() == 0) {
            deploymentContext = option_shared ? OUSTR("shared") : OUSTR("user");
        }
        else
        {
            if (deploymentContext.equalsAsciiL(
                    RTL_CONSTASCII_STRINGPARAM("shared") )) {
                option_shared = true;
            }
            else if (option_shared) {
                fprintf( stderr,
                         "WARNING: explicit context given!  "
                         "Ignoring option %s!\n",
                         toString( info_shared ).getStr() );
            }
        }

        Reference<deployment::XPackageManagerFactory> xPackageManagerFactory(
            deployment::thePackageManagerFactory::get( xComponentContext ) );
        Reference<deployment::XPackageManager> xPackageManager(
            xPackageManagerFactory->getPackageManager( deploymentContext ) );

        Reference< ::com::sun::star::ucb::XCommandEnvironment > xCmdEnv(
            createCmdEnv( xComponentContext, logFile,
                          option_force, option_verbose) );

        if (subcmd_add ||
            subCommand.equalsAsciiL(
                RTL_CONSTASCII_STRINGPARAM("remove") ))
        {
            for ( ::std::size_t pos = 0; pos < cmdPackages.size(); ++pos )
            {
                OUString const & cmdPackage = cmdPackages[ pos ];
                if (subcmd_add)
                {
                    Reference<deployment::XPackage> xPackage(
                        xPackageManager->addPackage(
                            cmdPackage, OUString() /* to be detected */,
                            Reference<task::XAbortChannel>(), xCmdEnv ) );
                    OSL_ASSERT( xPackage.is() );
                }
                else
                {
                    try
                    {
                        xPackageManager->removePackage(
                            cmdPackage, cmdPackage,
                            Reference<task::XAbortChannel>(), xCmdEnv );
                    }
                    catch (lang::IllegalArgumentException &)
                    {
                        Reference<deployment::XPackage> p(
                            findPackage(
                                xPackageManager, xCmdEnv, cmdPackage ) );
                        if ( !p.is() )
                            throw;
                        xPackageManager->removePackage(
                            ::dp_misc::getIdentifier(p), p->getName(),
                            Reference<task::XAbortChannel>(), xCmdEnv );
                    }
                }
            }
        }
        else if (subCommand.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM("reinstall") ))
        {
            xPackageManager->reinstallDeployedPackages(
                Reference<task::XAbortChannel>(), xCmdEnv );
        }
        else if (subCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("list") ))
        {
            Sequence< Reference<deployment::XPackage> > packages;
            if (cmdPackages.empty())
            {
                packages = xPackageManager->getDeployedPackages(
                    Reference<task::XAbortChannel>(), xCmdEnv );
                printf( "all deployed %s packages:\n",
                        ::rtl::OUStringToOString(
                            deploymentContext, textenc ).getStr() );
            }
            else
            {
                packages.realloc( cmdPackages.size() );
                for ( ::std::size_t pos = 0; pos < cmdPackages.size(); ++pos )
                    try
                    {
                        packages[ pos ] = xPackageManager->getDeployedPackage(
                            cmdPackages[ pos ], cmdPackages[ pos ], xCmdEnv );
                    }
                    catch (lang::IllegalArgumentException &)
                    {
                        packages[ pos ] = findPackage(
                            xPackageManager, xCmdEnv, cmdPackages[ pos ] );
                        if ( !packages[ pos ].is() )
                            throw;
                    }
            }
            printf_packages( packages, xCmdEnv );
        }
        else if (subCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("gui") ))
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
        }
        else
        {
            fprintf( stderr,
                     "\nERROR: unknown sub-command %s!\n"
                     "       Use " APP_NAME " %s to print all options.\n",
                     ::rtl::OUStringToOString( subCommand, textenc ).getStr(),
                     toString( info_help ).getStr() );
            return 1;
        }

        if (option_verbose)
            printf( "\n%s done.\n", APP_NAME );
        //Force to release all bridges which connect us to the child processes
        disposeBridges(xLocalComponentContext);
        return 0;
    }
    catch (ucb::CommandFailedException &) {
        // already handled by cmdenv
    }
    catch (ucb::CommandAbortedException &) {
        fprintf( stderr, "\n%s aborted!\n", APP_NAME );
    }
    catch (deployment::DeploymentException & exc) {
        fprintf( stderr,
                 "\nERROR: %s\n"
                 "       Cause: %s\n",
                 ::rtl::OUStringToOString(
                     exc.Message, textenc ).getStr(),
                 ::rtl::OUStringToOString(
                     option_verbose
                     ? ::comphelper::anyToString(exc.Cause)
                     : reinterpret_cast<
                     ::com::sun::star::uno::Exception const *>(
                         exc.Cause.getValue())->Message, textenc).getStr() );
    }
    catch (::com::sun::star::uno::Exception & e ) {
        Any exc( ::cppu::getCaughtException() );

        fprintf( stderr, "\nERROR: %s\n", ::rtl::OUStringToOString(
            option_verbose  ? e.Message + OUSTR("\nException details: \n") +
            ::comphelper::anyToString(exc) : e.Message, textenc).getStr() );
    }
    fprintf( stderr, "\n%s failed.\n", APP_NAME );
    disposeBridges(xLocalComponentContext);
    return 1;
}


