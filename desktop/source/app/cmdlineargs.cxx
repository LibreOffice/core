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

#include <config_features.h>

#if HAVE_FEATURE_MACOSX_SANDBOX
#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>
#endif

#include <cmdlineargs.hxx>
#include <vcl/svapp.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustring.hxx>
#include "rtl/process.h"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include "tools/getprocessworkingdir.hxx"

#include <svl/documentlockfile.hxx>

#include <cstdio>

using namespace com::sun::star::lang;
using namespace com::sun::star::uri;
using namespace com::sun::star::uno;


namespace desktop
{

namespace {

OUString translateExternalUris(OUString const & input) {
    OUString t(
        com::sun::star::uri::ExternalUriReferenceTranslator::create(
            comphelper::getProcessComponentContext())->
        translateToInternal(input));
    return t.isEmpty() ? input : t;
}

std::vector< OUString > translateExternalUris(
    std::vector< OUString > const & input)
{
    std::vector< OUString > t;
    for (std::vector< OUString >::const_iterator i(input.begin());
         i != input.end(); ++i)
    {
        t.push_back(translateExternalUris(*i));
    }
    return t;
}

class ExtCommandLineSupplier: public CommandLineArgs::Supplier {
public:
    explicit ExtCommandLineSupplier():
        m_count(rtl_getAppCommandArgCount()),
        m_index(0)
    {
        OUString url;
        if (tools::getProcessWorkingDir(url)) {
            m_cwdUrl.reset(url);
        }
    }

    virtual ~ExtCommandLineSupplier() {}

    virtual boost::optional< OUString > getCwdUrl() { return m_cwdUrl; }

    virtual bool next(OUString * argument) {
        OSL_ASSERT(argument != NULL);
        if (m_index < m_count) {
            rtl_getAppCommandArg(m_index++, &argument->pData);
            return true;
        } else {
            return false;
        }
    }

private:
    boost::optional< OUString > m_cwdUrl;
    sal_uInt32 m_count;
    sal_uInt32 m_index;
};

}

CommandLineArgs::Supplier::Exception::Exception() {}

CommandLineArgs::Supplier::Exception::Exception(Exception const &) {}

CommandLineArgs::Supplier::Exception::~Exception() {}

CommandLineArgs::Supplier::Exception &
CommandLineArgs::Supplier::Exception::operator =(Exception const &)
{ return *this; }

CommandLineArgs::Supplier::~Supplier() {}

// intialize class with command line parameters from process environment
CommandLineArgs::CommandLineArgs()
{
    InitParamValues();
    ExtCommandLineSupplier s;
    ParseCommandLine_Impl( s );
}

CommandLineArgs::CommandLineArgs( Supplier& supplier )
{
    InitParamValues();
    ParseCommandLine_Impl( supplier );
}

// ----------------------------------------------------------------------------

void CommandLineArgs::ParseCommandLine_Impl( Supplier& supplier )
{
    m_cwdUrl = supplier.getCwdUrl();

    // parse command line arguments
    bool bOpenEvent(true);
    bool bPrintEvent(false);
    bool bViewEvent(false);
    bool bStartEvent(false);
    bool bPrintToEvent(false);
    bool bPrinterName(false);
    bool bForceOpenEvent(false);
    bool bForceNewEvent(false);
    bool bDisplaySpec(false);
    bool bOpenDoc(false);
    bool bConversionEvent(false);
    bool bConversionParamsEvent(false);
    bool bBatchPrintEvent(false);
    bool bBatchPrinterNameEvent(false);
    bool bConversionOutEvent(false);

    for (;;)
    {
        OUString aArg;
        if ( !supplier.next( &aArg ) )
        {
            break;
        }

        if ( !aArg.isEmpty() )
        {
            m_bEmpty = false;
            OUString oArg;
            if ( !InterpretCommandLineParameter( aArg, oArg ))
            {
                if ( aArg.toChar() == '-' )
                {
                    // handle this argument as an option
                    if ( aArg.equalsIgnoreAsciiCase("-n"))
                    {
                        // force new documents based on the following documents
                        bForceNewEvent  = true;
                        bOpenEvent      = false;
                        bForceOpenEvent = false;
                        bPrintToEvent   = false;
                        bPrintEvent     = false;
                        bViewEvent      = false;
                        bStartEvent     = false;
                        bDisplaySpec    = false;
                    }
                    else if ( aArg.equalsIgnoreAsciiCase("-o"))
                    {
                        // force open documents regardless if they are templates or not
                        bForceOpenEvent = true;
                        bOpenEvent      = false;
                        bForceNewEvent  = false;
                        bPrintToEvent   = false;
                        bPrintEvent     = false;
                        bViewEvent      = false;
                        bStartEvent     = false;
                        bDisplaySpec    = false;
                    }
                    else if ( aArg.equalsIgnoreAsciiCase("-pt"))
                    {
                        // Print to special printer
                        bPrintToEvent   = true;
                        bPrinterName    = true;
                        bPrintEvent     = false;
                        bOpenEvent      = false;
                        bForceNewEvent  = false;
                        bViewEvent      = false;
                        bStartEvent     = false;
                        bDisplaySpec    = false;
                        bForceOpenEvent = false;
                   }
                   else if ( aArg.equalsIgnoreAsciiCase("-p"))
                   {
                        // Print to default printer
                        bPrintEvent     = true;
                        bPrintToEvent   = false;
                        bOpenEvent      = false;
                        bForceNewEvent  = false;
                        bForceOpenEvent = false;
                        bViewEvent      = false;
                        bStartEvent     = false;
                        bDisplaySpec    = false;
                   }
                   else if ( oArg.equalsIgnoreAsciiCase("view"))
                   {
                        // open in viewmode
                        bOpenEvent      = false;
                        bPrintEvent     = false;
                        bPrintToEvent   = false;
                        bForceNewEvent  = false;
                        bForceOpenEvent = false;
                        bViewEvent      = true;
                        bStartEvent     = false;
                        bDisplaySpec    = false;
                   }
                   else if ( oArg.equalsIgnoreAsciiCase("show"))
                   {
                        // open in viewmode
                        bOpenEvent      = false;
                        bViewEvent      = false;
                        bStartEvent     = true;
                        bPrintEvent     = false;
                        bPrintToEvent   = false;
                        bForceNewEvent  = false;
                        bForceOpenEvent = false;
                        bDisplaySpec    = false;
                    }
                    else if ( oArg.equalsIgnoreAsciiCase("display"))
                    {
                        // set display
                        bOpenEvent      = false;
                        bPrintEvent     = false;
                        bForceOpenEvent = false;
                        bPrintToEvent   = false;
                        bForceNewEvent  = false;
                        bViewEvent      = false;
                        bStartEvent     = false;
                        bDisplaySpec    = true;
                    }
                    else if ( oArg.equalsIgnoreAsciiCase("language"))
                    {
                        bOpenEvent      = false;
                        bPrintEvent     = false;
                        bForceOpenEvent = false;
                        bPrintToEvent   = false;
                        bForceNewEvent  = false;
                        bViewEvent      = false;
                        bStartEvent     = false;
                        bDisplaySpec    = false;
                    }
                    else if ( oArg.equalsIgnoreAsciiCase("convert-to"))
                    {
                        bOpenEvent = false;
                        bConversionEvent = true;
                        bConversionParamsEvent = true;
                    }
                    else if ( oArg.equalsIgnoreAsciiCase("print-to-file"))
                    {
                        bOpenEvent = false;
                        bBatchPrintEvent = true;
                    }
                    else if ( oArg.equalsIgnoreAsciiCase("printer-name") &&
                              bBatchPrintEvent )
                    {
                        bBatchPrinterNameEvent = true;
                    }
                    else if ( oArg.equalsIgnoreAsciiCase("outdir") &&
                              (bConversionEvent || bBatchPrintEvent) )
                    {
                        bConversionOutEvent = true;
                    }
                    else
                    // because it's impossible to filter these options that
                    // are handled in the soffice shell script with the
                    // primitive tools that /bin/sh offers, ignore them here
                    if (
#if defined UNX
                        !oArg.equalsIgnoreAsciiCase("backtrace") &&
                        !oArg.equalsIgnoreAsciiCase("strace") &&
                        !oArg.equalsIgnoreAsciiCase("valgrind") &&
                    // for X Session Management, handled in
                    // vcl/unx/generic/app/sm.cxx:
                        !oArg.match("session=") &&
#endif
                    //ignore additional legacy options that don't do anything anymore
                        !oArg.equalsIgnoreAsciiCase("nocrashreport") &&
                        m_unknown.isEmpty())
                    {
                        m_unknown = aArg;
                    }
                }
                else
                {
                    if ( bPrinterName && bPrintToEvent )
                    {
                        // first argument after "-pt" this must be the printer name
                        m_printername = aArg;
                        bPrinterName = false;
                    }
                    else if ( bConversionParamsEvent && bConversionEvent )
                    {
                        // first argument must be the params
                        m_conversionparams = aArg;
                        bConversionParamsEvent = false;
                    }
                    else if ( bBatchPrinterNameEvent && bBatchPrintEvent )
                    {
                        // first argument is the printer name
                        m_printername = aArg;
                        bBatchPrinterNameEvent = false;
                    }
                    else if ( (bConversionEvent || bBatchPrintEvent) && bConversionOutEvent )
                    {
                        m_conversionout = aArg;
                        bConversionOutEvent = false;
                    }
                    else
                    {
                        // handle this argument as a filename
                        if ( bOpenEvent )
                        {
                            m_openlist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bViewEvent )
                        {
                            m_viewlist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bStartEvent )
                        {
                            m_startlist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bPrintEvent )
                        {
                            m_printlist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bPrintToEvent )
                        {
                            m_printtolist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bForceNewEvent )
                        {
                            m_forcenewlist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bForceOpenEvent )
                        {
                            m_forceopenlist.push_back(aArg);
                            bOpenDoc = true;
                        }
                        else if ( bDisplaySpec )
                        {
                            bDisplaySpec = false; // only one display, not a lsit
                            bOpenEvent = true;    // set back to standard
                        }
                        else if ( bConversionEvent || bBatchPrintEvent )
                            m_conversionlist.push_back(aArg);
                    }
                }
            }
        }
    }

    if ( bOpenDoc )
        m_bDocumentArgs = true;
}

bool CommandLineArgs::InterpretCommandLineParameter( const OUString& aArg, OUString& oArg )
{
    bool bDeprecated = false;
    if (aArg.matchIgnoreAsciiCase("--"))
    {
        oArg = OUString(aArg.getStr()+2, aArg.getLength()-2);
    }
    else if (aArg.toChar() == '-')
    {
        if ( aArg.getLength() > 2 ) // -h, -o, -n, -? are still valid
            bDeprecated = true;
        oArg = OUString(aArg.getStr()+1, aArg.getLength()-1);
    }
    else
    {
        return false;
    }

    if ( oArg == "minimized" )
    {
        m_minimized = true;
    }
    else if ( oArg == "invisible" )
    {
        m_invisible = true;
    }
    else if ( oArg == "norestore" )
    {
        m_norestore = true;
    }
    else if ( oArg == "nodefault" )
    {
        m_nodefault = true;
    }
    else if ( oArg == "headless" )
    {
        // Headless means also invisibile, so set this parameter to true!
        m_headless = true;
        m_invisible = true;
    }
    else if ( oArg == "quickstart" )
    {
#if defined(ENABLE_QUICKSTART_APPLET)
        m_quickstart = true;
#endif
        m_noquickstart = false;
    }
    else if ( oArg == "quickstart=no" )
    {
        m_noquickstart = true;
        m_quickstart = false;
    }
    else if ( oArg == "terminate_after_init" )
    {
        m_terminateafterinit = true;
    }
    else if ( oArg == "nofirststartwizard" )
    {
        m_nofirststartwizard = true;
    }
    else if ( oArg == "nologo" )
    {
        m_nologo = true;
    }
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    else if ( oArg == "nolockcheck" )
    {
        m_nolockcheck = true;
        // Workaround for automated testing
        ::svt::DocumentLockFile::AllowInteraction( false );
    }
#endif
    else if ( oArg == "help" || aArg == "-h" || aArg == "-?" )
    {
        m_help = true;
    }
    else if ( oArg == "helpwriter" )
    {
        m_helpwriter = true;
    }
    else if ( oArg == "helpcalc" )
    {
        m_helpcalc = true;
    }
    else if ( oArg == "helpdraw" )
    {
        m_helpdraw = true;
    }
    else if ( oArg == "helpimpress" )
    {
        m_helpimpress = true;
    }
    else if ( oArg == "helpbase" )
    {
        m_helpbase = true;
    }
    else if ( oArg == "helpbasic" )
    {
        m_helpbasic = true;
    }
    else if ( oArg == "helpmath" )
    {
        m_helpmath = true;
    }
    else if ( oArg == "version" )
    {
        m_version = true;
    }
    else if ( oArg.matchIgnoreAsciiCase("splash-pipe=") )
    {
        m_splashpipe = true;
    }
#ifdef MACOSX
    /* #i84053# ignore -psn on Mac
       Platform dependent #ifdef here is ugly, however this is currently
       the only platform dependent parameter. Should more appear
       we should find a better solution
    */
    else if ( aArg.match("-psn") )
    {
        return true;
    }
#endif
#if HAVE_FEATURE_MACOSX_SANDBOX
    else if ( oArg == "nstemporarydirectory" )
    {
        printf("%s\n", [NSTemporaryDirectory() UTF8String]);
        exit(0);
    }
#endif
#ifdef WIN32
    /* fdo#57203 ignore -Embedding on Windows
       when LibreOffice is launched by COM+
     */
    else if ( oArg == "Embedding" )
    {
        return true;
    }
#endif
    else if ( oArg.matchIgnoreAsciiCase("infilter="))
    {
        m_infilter.push_back(oArg.copy(RTL_CONSTASCII_LENGTH("infilter=")));
    }
    else if ( oArg.matchIgnoreAsciiCase("accept="))
    {
        m_accept.push_back(oArg.copy(RTL_CONSTASCII_LENGTH("accept=")));
    }
    else if ( oArg.matchIgnoreAsciiCase("unaccept="))
    {
        m_unaccept.push_back(oArg.copy(RTL_CONSTASCII_LENGTH("unaccept=")));
    }
    else if ( oArg.matchIgnoreAsciiCase("language="))
    {
        m_language = oArg.copy(RTL_CONSTASCII_LENGTH("language="));
    }
    else if ( oArg.matchIgnoreAsciiCase("pidfile="))
    {
        m_pidfile = oArg.copy(RTL_CONSTASCII_LENGTH("pidfile="));
    }
    else if ( oArg == "writer" )
    {
        m_writer = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "calc" )
    {
        m_calc = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "draw" )
    {
        m_draw = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "impress" )
    {
        m_impress = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "base" )
    {
        m_base = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "global" )
    {
        m_global = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "math" )
    {
        m_math = true;
        m_bDocumentArgs = true;
    }
    else if ( oArg == "web" )
    {
        m_web = true;
        m_bDocumentArgs = true;
    }
    else
        return false;

    if (bDeprecated)
    {
        OString sArg(OUStringToOString(aArg, osl_getThreadTextEncoding()));
        fprintf(stderr, "Warning: %s is deprecated.  Use -%s instead.\n", sArg.getStr(), sArg.getStr());
    }
    return true;
}

void CommandLineArgs::InitParamValues()
{
    m_minimized = false;
    m_norestore = false;
#ifdef LIBO_HEADLESS
    m_invisible = true;
    m_headless = true;
#else
    m_invisible = false;
    m_headless = false;
#endif
    m_quickstart = false;
    m_noquickstart = false;
    m_terminateafterinit = false;
    m_nofirststartwizard = false;
    m_nologo = false;
    m_nolockcheck = false;
    m_nodefault = false;
    m_help = false;
    m_writer = false;
    m_calc = false;
    m_draw = false;
    m_impress = false;
    m_global = false;
    m_math = false;
    m_web = false;
    m_base = false;
    m_helpwriter = false;
    m_helpcalc = false;
    m_helpdraw = false;
    m_helpbasic = false;
    m_helpmath = false;
    m_helpimpress = false;
    m_helpbase = false;
    m_version = false;
    m_splashpipe = false;
    m_bEmpty = true;
    m_bDocumentArgs  = false;
}

bool CommandLineArgs::IsMinimized() const
{
    return m_minimized;
}

bool CommandLineArgs::IsInvisible() const
{
    return m_invisible;
}

bool CommandLineArgs::IsNoRestore() const
{
    return m_norestore;
}

bool CommandLineArgs::IsNoDefault() const
{
    return m_nodefault;
}

bool CommandLineArgs::IsHeadless() const
{
    return m_headless;
}

bool CommandLineArgs::IsQuickstart() const
{
    return m_quickstart;
}

bool CommandLineArgs::IsNoQuickstart() const
{
    return m_noquickstart;
}

bool CommandLineArgs::IsTerminateAfterInit() const
{
    return m_terminateafterinit;
}

bool CommandLineArgs::IsNoLogo() const
{
    return m_nologo;
}

bool CommandLineArgs::IsNoLockcheck() const
{
    return m_nolockcheck;
}

bool CommandLineArgs::IsHelp() const
{
    return m_help;
}
bool CommandLineArgs::IsHelpWriter() const
{
    return m_helpwriter;
}

bool CommandLineArgs::IsHelpCalc() const
{
    return m_helpcalc;
}

bool CommandLineArgs::IsHelpDraw() const
{
    return m_helpdraw;
}

bool CommandLineArgs::IsHelpImpress() const
{
    return m_helpimpress;
}

bool CommandLineArgs::IsHelpBase() const
{
    return m_helpbase;
}
bool CommandLineArgs::IsHelpMath() const
{
    return m_helpmath;
}
bool CommandLineArgs::IsHelpBasic() const
{
    return m_helpbasic;
}

bool CommandLineArgs::IsWriter() const
{
    return m_writer;
}

bool CommandLineArgs::IsCalc() const
{
    return m_calc;
}

bool CommandLineArgs::IsDraw() const
{
    return m_draw;
}

bool CommandLineArgs::IsImpress() const
{
    return m_impress;
}

bool CommandLineArgs::IsBase() const
{
    return m_base;
}

bool CommandLineArgs::IsGlobal() const
{
    return m_global;
}

bool CommandLineArgs::IsMath() const
{
    return m_math;
}

bool CommandLineArgs::IsWeb() const
{
    return m_web;
}

bool CommandLineArgs::IsVersion() const
{
    return m_version;
}

OUString CommandLineArgs::GetUnknown() const
{
    return m_unknown;
}

bool CommandLineArgs::HasModuleParam() const
{
    return m_writer || m_calc || m_draw || m_impress || m_global || m_math
        || m_web || m_base;
}

bool CommandLineArgs::HasSplashPipe() const
{
    return m_splashpipe;
}

std::vector< OUString > const & CommandLineArgs::GetAccept() const
{
    return m_accept;
}

std::vector< OUString > const & CommandLineArgs::GetUnaccept() const
{
    return m_unaccept;
}

std::vector< OUString > CommandLineArgs::GetOpenList() const
{
    return translateExternalUris(m_openlist);
}

std::vector< OUString > CommandLineArgs::GetViewList() const
{
    return translateExternalUris(m_viewlist);
}

std::vector< OUString > CommandLineArgs::GetStartList() const
{
    return translateExternalUris(m_startlist);
}

std::vector< OUString > CommandLineArgs::GetForceOpenList() const
{
    return translateExternalUris(m_forceopenlist);
}

std::vector< OUString > CommandLineArgs::GetForceNewList() const
{
    return translateExternalUris(m_forcenewlist);
}

std::vector< OUString > CommandLineArgs::GetPrintList() const
{
    return translateExternalUris(m_printlist);
}

std::vector< OUString > CommandLineArgs::GetPrintToList() const
{
    return translateExternalUris(m_printtolist);
}

OUString CommandLineArgs::GetPrinterName() const
{
    return m_printername;
}

OUString CommandLineArgs::GetLanguage() const
{
    return m_language;
}

std::vector< OUString > const & CommandLineArgs::GetInFilter() const
{
    return m_infilter;
}

std::vector< OUString > CommandLineArgs::GetConversionList() const
{
    return translateExternalUris(m_conversionlist);
}

OUString CommandLineArgs::GetConversionParams() const
{
    return m_conversionparams;
}
OUString CommandLineArgs::GetConversionOut() const
{
    return translateExternalUris(m_conversionout);
}

bool CommandLineArgs::IsEmpty() const
{
    return m_bEmpty;
}

bool CommandLineArgs::WantsToLoadDocument() const
{
    return m_bDocumentArgs;
}

OUString CommandLineArgs::GetPidfileName() const
{
    return m_pidfile;
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
