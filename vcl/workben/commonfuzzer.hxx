/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/configmgr.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <vcl/graph.hxx>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmf.hxx>
#include <unistd.h>
#include <stdlib.h>
#include "headless/svpgdi.hxx"
#include "unx/fontmanager.hxx"
#include "unx/glyphcache.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

namespace
{
    OUString getExecutableDir()
    {
        OUString uri;
        if (osl_getExecutableFile(&uri.pData) != osl_Process_E_None) {
            abort();
        }
        sal_Int32 lastDirSeparatorPos = uri.lastIndexOf('/');
        if (lastDirSeparatorPos >= 0) {
            uri = uri.copy(0, lastDirSeparatorPos + 1);
        }
        return uri;
    }

    OUString getExecutableName()
    {
        OUString uri;
        if (osl_getExecutableFile(&uri.pData) != osl_Process_E_None) {
            abort();
        }
        return uri.copy(uri.lastIndexOf('/') + 1);
    }

    void setFontConfigConf(const OUString &execdir)
    {
        osl::File aFontConfig("file:///tmp/wmffuzzerfonts.conf");
        if (aFontConfig.open(osl_File_OpenFlag_Create | osl_File_OpenFlag_Write) == osl::File::E_None)
        {
            OUString sExecDir;
            osl::FileBase::getSystemPathFromFileURL(execdir, sExecDir);

            OStringBuffer aBuffer("<?xml version=\"1.0\"?>\n<fontconfig><dir>");
            aBuffer.append(OUStringToOString(sExecDir, osl_getThreadTextEncoding()))
                   .append(OUStringToOString(getExecutableName(), osl_getThreadTextEncoding())).append(".fonts");
            aBuffer.append("</dir><cachedir>/tmp/cache/fontconfig</cachedir></fontconfig>");
            OString aConf = aBuffer.makeStringAndClear();
            sal_uInt64 aBytesWritten;
            aFontConfig.write(aConf.getStr(), aConf.getLength(), aBytesWritten);
            assert(aBytesWritten == aConf.getLength());
        }
        setenv("FONTCONFIG_FILE", "/tmp/wmffuzzerfonts.conf", 0);
    }
}

extern "C"
{
    __attribute__((weak)) void __lsan_disable();
    __attribute__((weak)) void __lsan_enable();
}

void CommonInitialize(int *argc, char ***argv)
{
    setenv("SAL_USE_VCLPLUGIN", "svp", 1);
    setenv("JPEGMEM", "768M", 1);
    setenv("SC_MAX_MATRIX_ELEMENTS", "60000000", 1);
    setenv("SC_NO_THREADED_CALCULATION", "1", 1);
    setenv("SAL_DISABLE_PRINTERLIST", "1", 1);
    setenv("SAL_DISABLE_DEFAULTPRINTER", "1", 1);
    setenv("SAL_NO_FONT_LOOKUP", "1", 1);

    //allow bubbling of max input len to fuzzer targets
    int nMaxLen = 0;
    for (int i = 0; i < *argc; ++i)
    {
        if (strncmp((*argv)[i], "-max_len=", 9) == 0)
            nMaxLen = atoi((*argv)[i] + 9);
    }
    setenv("FUZZ_MAX_INPUT_LEN", "1", nMaxLen);

    osl_setCommandArgs(*argc, *argv);

    OUString sExecDir = getExecutableDir();
    rtl::Bootstrap::set("BRAND_BASE_DIR", sExecDir);
    setFontConfigConf(sExecDir);

    tools::extendApplicationEnvironment();

    Reference< XComponentContext > xContext =
        defaultBootstrap_InitialComponentContext(sExecDir + getExecutableName() + ".unorc");
    Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );
    if( !xServiceManager.is() )
        Application::Abort( "Failed to bootstrap" );
    comphelper::setProcessServiceFactory( xServiceManager );
    utl::ConfigManager::EnableFuzzing();
    Application::EnableHeadlessMode(false);
    InitVCL();

    //we don't have a de-init, so inside this leak disabled region...
    //get the font info
    psp::PrintFontManager::get();
    //get the printer info
    Printer::GetPrinterQueues();

    //https://github.com/google/oss-fuzz/issues/1449
    //https://github.com/google/oss-fuzz/issues/5441
    //release the solarmutex so a fork can acquire it which should
    //allow these fuzzers to work without AFL_DRIVER_DONT_DEFER set
    //removing the confusion of #5441 and the need for AFL_DRIVER_DONT_DEFER
    //in .options files
    Application::ReleaseSolarMutex();
}

void TypicalFuzzerInitialize(int *argc, char ***argv)
{
    if (__lsan_disable)
        __lsan_disable();

    CommonInitialize(argc, argv);

    if (__lsan_enable)
        __lsan_enable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
