/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unopkg_app.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 18:15:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "../unopkg_shared.h"
#include "sal/main.h"
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
#include "boost/scoped_array.hpp"
#include "com/sun/star/ui/dialogs/XDialogClosedListener.hpp"

#include <stdio.h>
#include <vector>

#define APP_NAME "unopkg"


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::unopkg;
using ::rtl::OUString;

namespace {

//------------------------------------------------------------------------------
const char s_usingText [] =
"\n"
"using: " APP_NAME " add <options> extension-path...\n"
"       " APP_NAME " remove <options> extension-name...\n"
"       " APP_NAME " list <options> extension-name...\n"
"       " APP_NAME " reinstall <options>\n"
"       " APP_NAME " gui\n"
"       " APP_NAME " -V\n"
"       " APP_NAME " -h\n"
"\n"
"sub-commands:\n"
" add                     add extension\n"
" remove                  remove extensions by name\n"
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

} // anon namespace

//##############################################################################
SAL_IMPLEMENT_MAIN()
{
    (void) argv;
    (void) argc;
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
            oslProcessError rc = osl_getCommandArg( 0, &subCommand.pData );
            if (rc == osl_Process_E_None )
                OSL_ASSERT(0);
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
                     !readArgument( &deploymentContext, info_context, &nPos ))
            {
                oslProcessError rc = osl_getCommandArg( nPos, &cmdArg.pData );
                if (rc == osl_Process_E_None )
                    OSL_ASSERT(0);
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

        Reference<XComponentContext> xComponentContext(
            getUNO( disposeGuard, option_verbose, subcmd_gui ) );

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
                    xPackageManager->removePackage(
                        cmdPackage,
                        Reference<task::XAbortChannel>(), xCmdEnv );
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
                    packages[ pos ] = xPackageManager->getDeployedPackage(
                        cmdPackages[ pos ], xCmdEnv );
            }
            printf_packages( packages, xCmdEnv );
        }
        else if (subCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("gui") ))
        {

            ::std::size_t cUrls = cmdPackages.size();
            ::boost::scoped_array<OUString> sarUrls(new OUString[cUrls]);
            OUString * arString = sarUrls.get();

            for ( ::std::size_t pos = 0; pos < cUrls; ++pos )
            {
                arString[pos] = cmdPackages[pos];
            }
            Reference<ui::dialogs::XAsynchronousExecutableDialog> xDialog(
                deployment::ui::PackageManagerDialog::createAndInstall(
                    xComponentContext, Sequence<OUString>(arString, cUrls), deploymentContext) );

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
    catch (::com::sun::star::uno::Exception &) {
        Any exc( ::cppu::getCaughtException() );
        fprintf( stderr, "\nERROR: %s\n", ::rtl::OUStringToOString(
                     ::comphelper::anyToString(exc), textenc ).getStr() );
    }
    fprintf( stderr, "\n%s failed.\n", APP_NAME );
    return 1;
}

