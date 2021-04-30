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

#include "cmdlineargs.hxx"
#include <osl/thread.hxx>
#include <tools/stream.hxx>
#include <rtl/ustring.hxx>
#include <rtl/process.h>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <unotools/bootstrap.hxx>

#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::uri;
using namespace com::sun::star::uno;

namespace desktop
{

namespace {

OUString translateExternalUris(OUString const & input) {
    OUString t(
        css::uri::ExternalUriReferenceTranslator::create(
            comphelper::getProcessComponentContext())->
        translateToInternal(input));
    return t.isEmpty() ? input : t;
}

std::vector< OUString > translateExternalUris(
    std::vector< OUString > const & input)
{
    std::vector< OUString > t;
    t.reserve(input.size());
    for (auto const& elem : input)
    {
        t.push_back(translateExternalUris(elem));
    }
    return t;
}

class ExtCommandLineSupplier: public CommandLineArgs::Supplier {
public:
    explicit ExtCommandLineSupplier():
        m_count(
            comphelper::LibreOfficeKit::isActive()
            ? 0 : rtl_getAppCommandArgCount()),
        m_index(0)
    {
        OUString url;
        if (utl::Bootstrap::getProcessWorkingDir(url)) {
            m_cwdUrl = url;
        }
    }

    virtual std::optional< OUString > getCwdUrl() override { return m_cwdUrl; }

    virtual bool next(OUString * argument) override {
        OSL_ASSERT(argument != nullptr);
        if (m_index < m_count) {
            rtl_getAppCommandArg(m_index++, &argument->pData);
            return true;
        } else {
            return false;
        }
    }

private:
    std::optional< OUString > m_cwdUrl;
    sal_uInt32 m_count;
    sal_uInt32 m_index;
};

enum class CommandLineEvent {
    Open, Print, View, Start, PrintTo,
    ForceOpen, ForceNew, Conversion, BatchPrint
};

// Office URI Schemes: see https://msdn.microsoft.com/en-us/library/dn906146
// This functions checks if the arg is an Office URI.
// If applicable, it updates arg to inner URI.
// If no event argument is explicitly set in command line,
// then it returns updated command line event,
// according to Office URI command.
CommandLineEvent CheckOfficeURI(/* in,out */ OUString& arg, CommandLineEvent curEvt)
{
    // 1. Strip the scheme name
    OUString rest1;
    bool isOfficeURI = ( arg.startsWithIgnoreAsciiCase("vnd.libreoffice.command:", &rest1) // Proposed extended schema
                      || arg.startsWithIgnoreAsciiCase("ms-word:",                 &rest1)
                      || arg.startsWithIgnoreAsciiCase("ms-powerpoint:",           &rest1)
                      || arg.startsWithIgnoreAsciiCase("ms-excel:",                &rest1)
                      || arg.startsWithIgnoreAsciiCase("ms-visio:",                &rest1)
                      || arg.startsWithIgnoreAsciiCase("ms-access:",               &rest1));
    if (!isOfficeURI)
        return curEvt;

    OUString rest2;
    tools::Long nURIlen = -1;

    // URL might be encoded
    OUString decoded_rest = rest1.replaceAll("%7C", "|").replaceAll("%7c", "|");

    // 2. Discriminate by command name (incl. 1st command argument descriptor)
    //    Extract URI: everything up to possible next argument
    if (decoded_rest.startsWith("ofv|u|", &rest2))
    {
        // Open for view - override only in default mode
        if (curEvt == CommandLineEvent::Open)
            curEvt = CommandLineEvent::View;
        nURIlen = rest2.indexOf("|");
    }
    else if (decoded_rest.startsWith("ofe|u|", &rest2))
    {
        // Open for editing - override only in default mode
        if (curEvt == CommandLineEvent::Open)
            curEvt = CommandLineEvent::ForceOpen;
        nURIlen = rest2.indexOf("|");
    }
    else if (decoded_rest.startsWith("nft|u|", &rest2))
    {
        // New from template - override only in default mode
        if (curEvt == CommandLineEvent::Open)
            curEvt = CommandLineEvent::ForceNew;
        nURIlen = rest2.indexOf("|");
        // TODO: process optional second argument (default save-to location)
        // For now, we just ignore it
    }
    else
    {
        // Abbreviated scheme: <scheme-name>:URI
        // "ofv|u|" implied
        // override only in default mode
        if (curEvt == CommandLineEvent::Open)
            curEvt = CommandLineEvent::View;
        rest2 = rest1;
    }
    if (nURIlen < 0)
        nURIlen = rest2.getLength();
    arg = rest2.copy(0, nURIlen);
    return curEvt;
}

// Skip single newline (be it *NIX LF, MacOS CR, of Win CRLF)
// Changes the offset, and returns true if moved
bool SkipNewline(const char* & pStr)
{
    if ((*pStr != '\r') && (*pStr != '\n'))
        return false;
    if (*pStr == '\r')
        ++pStr;
    if (*pStr == '\n')
        ++pStr;
    return true;
}

// Web query: http://support.microsoft.com/kb/157482
CommandLineEvent CheckWebQuery(/* in,out */ OUString& arg, CommandLineEvent curEvt)
{
    // Only handle files with extension .iqy
    if (!arg.endsWithIgnoreAsciiCase(".iqy"))
        return curEvt;

    static osl::Mutex aMutex;
    osl::MutexGuard aGuard(aMutex);

    try
    {
        OUString sFileURL;
        // Cannot use translateExternalUris yet, because process service factory is not yet available
        if (osl::FileBase::getFileURLFromSystemPath(arg, sFileURL) != osl::FileBase::RC::E_None)
            return curEvt;
        SvFileStream stream(sFileURL, StreamMode::READ);

        const sal_Int32 nBufLen = 32000;
        char sBuffer[nBufLen];
        size_t nRead = stream.ReadBytes(sBuffer, nBufLen);
        if (nRead < 8) // WEB\n1\n...
            return curEvt;

        const char* pPos = sBuffer;
        if (strncmp(pPos, "WEB", 3) != 0)
            return curEvt;
        pPos += 3;
        if (!SkipNewline(pPos))
            return curEvt;
        if (*pPos != '1')
            return curEvt;
        ++pPos;
        if (!SkipNewline(pPos))
            return curEvt;

        OStringBuffer aResult(static_cast<unsigned int>(nRead));
        do
        {
            const char* pPos1 = pPos;
            const char* pEnd = sBuffer + nRead;
            while ((pPos1 < pEnd) && (*pPos1 != '\r') && (*pPos1 != '\n'))
                ++pPos1;
            aResult.append(pPos, pPos1 - pPos);
            if (pPos1 < pEnd) // newline
                break;
            pPos = sBuffer;
        } while ((nRead = stream.ReadBytes(sBuffer, nBufLen)) > 0);

        stream.Close();

        arg = OStringToOUString(aResult.makeStringAndClear(), osl_getThreadTextEncoding());
        return CommandLineEvent::ForceNew;
    }
    catch (...)
    {
        SAL_WARN("desktop.app", "An error processing Web Query file: " << arg);
    }

    return curEvt;
}

} // namespace

CommandLineArgs::Supplier::Exception::Exception() {}

CommandLineArgs::Supplier::Exception::Exception(Exception const &) {}

CommandLineArgs::Supplier::Exception::~Exception() {}

CommandLineArgs::Supplier::Exception &
CommandLineArgs::Supplier::Exception::operator =(Exception const &)
{ return *this; }

CommandLineArgs::Supplier::~Supplier() {}

// initialize class with command line parameters from process environment
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

void CommandLineArgs::ParseCommandLine_Impl( Supplier& supplier )
{
#ifdef ENABLE_WASM_STRIP
    // use hard-coded init-params for wasm '-nolockcheck -norestore -nologo -writer'
    // no restore tries
    m_norestore = true;
    // no logo needed
    m_nologo = true;
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
    // no lock-checks needed
    m_nolockcheck = true;
#endif
    // start with writer only
    m_writer = true;
    m_bDocumentArgs = true;

    m_bEmpty = false;

    // return to avoid #elif
    return;
#endif

    m_cwdUrl = supplier.getCwdUrl();
    CommandLineEvent eCurrentEvent = CommandLineEvent::Open;

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
            OUString oDeprecatedArg;
            if (!aArg.startsWith("--", &oArg) && aArg.startsWith("-", &oArg)
                && aArg.getLength() > 2) // -h, -?, -n, -o, -p are still valid
            {
                oDeprecatedArg = aArg; // save here, since aArg can change later
            }

            OUString rest;
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
                setHeadless();
            }
            else if ( oArg == "eventtesting" )
            {
                m_eventtesting = true;
            }
            else if ( oArg == "safe-mode" )
            {
                m_safemode = true;
            }
            else if ( oArg == "cat" )
            {
                m_textcat = true;
                m_conversionparams = "txt:Text";
                eCurrentEvent = CommandLineEvent::Conversion;
                setHeadless();
            }
            else if ( oArg == "script-cat" )
            {
                m_scriptcat = true;
                eCurrentEvent = CommandLineEvent::Conversion;
                setHeadless();
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
                // Do nothing, accept only for backward compatibility
            }
            else if ( oArg == "nologo" )
            {
                m_nologo = true;
            }
#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
            else if ( oArg == "nolockcheck" )
            {
                m_nolockcheck = true;
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
            else if ( oArg == "protector" )
            {
                // Not relevant for us here, but can be used in unit tests.
                // Usually unit tests would not end up here, but e.g. the
                // LOK Tiled Rendering tests end up running a full soffice
                // process, and we can't bail on the use of --protector.

                // We specifically need to consume the following 2 arguments
                // for --protector
                if ((!supplier.next(&aArg) || !supplier.next(&aArg)) && m_unknown.isEmpty())
                    m_unknown = "--protector must be followed by two arguments";
            }
            else if ( oArg == "version" )
            {
                m_version = true;
            }
            else if ( oArg.startsWith("splash-pipe=") )
            {
                m_splashpipe = true;
            }
#ifdef MACOSX
            /* #i84053# ignore -psn on Mac
               Platform dependent #ifdef here is ugly, however this is currently
               the only platform dependent parameter. Should more appear
               we should find a better solution
            */
            else if ( aArg.startsWith("-psn") )
            {
                oDeprecatedArg.clear();
            }
#endif
#if HAVE_FEATURE_MACOSX_SANDBOX
            else if ( oArg == "nstemporarydirectory" )
            {
                printf("%s\n", [NSTemporaryDirectory() UTF8String]);
                exit(0);
            }
#endif
#ifdef _WIN32
            /* fdo#57203 ignore -Embedding on Windows
               when LibreOffice is launched by COM+
            */
            else if ( oArg == "Embedding" )
            {
                oDeprecatedArg.clear();
            }
#endif
            else if ( oArg.startsWith("infilter=", &rest))
            {
                m_infilter.push_back(rest);
            }
            else if ( oArg.startsWith("accept=", &rest))
            {
                m_accept.push_back(rest);
            }
            else if ( oArg.startsWith("unaccept=", &rest))
            {
                m_unaccept.push_back(rest);
            }
            else if ( oArg.startsWith("language=", &rest))
            {
                m_language = rest;
            }
            else if ( oArg.startsWith("pidfile=", &rest))
            {
                m_pidfile = rest;
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
            else if ( aArg == "-n" )
            {
                // force new documents based on the following documents
                eCurrentEvent = CommandLineEvent::ForceNew;
            }
            else if ( aArg == "-o" )
            {
                // force open documents regardless if they are templates or not
                eCurrentEvent = CommandLineEvent::ForceOpen;
            }
            else if ( oArg == "pt" )
            {
                // Print to special printer
                eCurrentEvent = CommandLineEvent::PrintTo;
                // first argument after "-pt" must be the printer name
                if (supplier.next(&aArg))
                    m_printername = aArg;
                else if (m_unknown.isEmpty())
                    m_unknown = "--pt must be followed by printername";
            }
            else if ( aArg == "-p" )
            {
                // Print to default printer
                eCurrentEvent = CommandLineEvent::Print;
            }
            else if ( oArg == "view")
            {
                // open in viewmode
                eCurrentEvent = CommandLineEvent::View;
            }
            else if ( oArg == "show" )
            {
                // open in viewmode
                eCurrentEvent = CommandLineEvent::Start;
            }
            else if ( oArg == "display" )
            {
                // The command line argument following --display should
                // always be treated as the argument of --display.
                // --display and its argument are handled "out of line"
                // in Unix-only desktop/unx/source/splashx.c and vcl/unx/*,
                // and just ignored here
                (void)supplier.next(&aArg);
            }
            else if ( oArg == "convert-to" )
            {
                eCurrentEvent = CommandLineEvent::Conversion;
                // first argument must be the params
                if (supplier.next(&aArg))
                {
                    m_conversionparams = aArg;
                    // It doesn't make sense to use convert-to without headless.
                    setHeadless();
                }
                else if (m_unknown.isEmpty())
                    m_unknown = "--convert-to must be followed by output_file_extension[:output_filter_name]";
            }
            else if ( oArg == "print-to-file" )
            {
                eCurrentEvent = CommandLineEvent::BatchPrint;
            }
            else if ( oArg == "printer-name" )
            {
                if (eCurrentEvent == CommandLineEvent::BatchPrint)
                {
                    // first argument is the printer name
                    if (supplier.next(&aArg))
                        m_printername = aArg;
                    else if (m_unknown.isEmpty())
                        m_unknown = "--printer-name must be followed by printername";
                }
                else if (m_unknown.isEmpty())
                {
                    m_unknown = "--printer-name must directly follow --print-to-file";
                }
            }
            else if ( oArg == "outdir" )
            {
                if (eCurrentEvent == CommandLineEvent::Conversion ||
                    eCurrentEvent == CommandLineEvent::BatchPrint)
                {
                    if (supplier.next(&aArg))
                        m_conversionout = aArg;
                    else if (m_unknown.isEmpty())
                        m_unknown = "--outdir must be followed by output directory path";
                }
                else if (m_unknown.isEmpty())
                {
                    m_unknown = "--outdir must directly follow either output filter specification of --convert-to, or --print-to-file or its printer specification";
                }
            }
            else if ( eCurrentEvent == CommandLineEvent::Conversion
                      && oArg == "convert-images-to" )
            {
                if (supplier.next(&aArg))
                    m_convertimages = aArg;
                else if (m_unknown.isEmpty())
                    m_unknown = "--convert-images-to must be followed by an image type";
            }
            else if ( aArg.startsWith("-") )
            {
                // because it's impossible to filter these options that
                // are handled in the soffice shell script with the
                // primitive tools that /bin/sh offers, ignore them here
                if (
#if defined UNX
                    oArg != "record" &&
                    oArg != "backtrace" &&
                    oArg != "strace" &&
                    oArg != "valgrind" &&
                    // for X Session Management, handled in
                    // vcl/unx/generic/app/sm.cxx:
                    oArg != "session=" &&
#endif
                    //ignore additional legacy options that don't do anything anymore
                    oArg != "nocrashreport" &&
                    m_unknown.isEmpty())
                {
                    m_unknown = aArg;
                }
                oDeprecatedArg.clear();
            }
            else
            {
                // handle this argument as a filename

                // First check if this is an Office URI
                // This will possibly adjust event for this argument
                // and put real URI to aArg
                CommandLineEvent eThisEvent = CheckOfficeURI(aArg, eCurrentEvent);

                // Now check if this is a Web Query file
                eThisEvent = CheckWebQuery(aArg, eThisEvent);

                switch (eThisEvent)
                {
                case CommandLineEvent::Open:
                    m_openlist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::View:
                    m_viewlist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::Start:
                    m_startlist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::Print:
                    m_printlist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::PrintTo:
                    m_printtolist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::ForceNew:
                    m_forcenewlist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::ForceOpen:
                    m_forceopenlist.push_back(aArg);
                    m_bDocumentArgs = true;
                    break;
                case CommandLineEvent::Conversion:
                case CommandLineEvent::BatchPrint:
                    m_conversionlist.push_back(aArg);
                    break;
                }
            }

            if (!oDeprecatedArg.isEmpty())
            {
                OString sArg(OUStringToOString(oDeprecatedArg, osl_getThreadTextEncoding()));
                fprintf(stderr, "Warning: %s is deprecated.  Use -%s instead.\n", sArg.getStr(), sArg.getStr());
            }
        }
    }
}

void CommandLineArgs::InitParamValues()
{
    m_minimized = false;
    m_norestore = false;
#if HAVE_FEATURE_UI
    m_invisible = false;
    m_headless = false;
#else
    m_invisible = true;
    m_headless = true;
#endif
    m_eventtesting = false;
    m_quickstart = false;
    m_noquickstart = false;
    m_terminateafterinit = false;
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
    m_textcat = false;
    m_scriptcat = false;
    m_safemode = false;
}

bool CommandLineArgs::HasModuleParam() const
{
    return m_writer || m_calc || m_draw || m_impress || m_global || m_math
        || m_web || m_base;
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

std::vector< OUString > CommandLineArgs::GetConversionList() const
{
    return translateExternalUris(m_conversionlist);
}

OUString CommandLineArgs::GetConversionOut() const
{
    return translateExternalUris(m_conversionout);
}

} // namespace desktop

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
