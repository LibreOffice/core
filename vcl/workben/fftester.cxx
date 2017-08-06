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

 /* e.g.
        export CC=afl-clang-fast
        export CXX=afl-clang-fast++
        make
        cp workdir/LinkTarget/Executable/fftester instdir/program
        LD_LIBRARY_PATH=`pwd`/instdir/program AFL_PERSISTENT=1 afl-fuzz -t 50 -i ~/fuzz/in.png -o ~/fuzz/out.png -d -T png -m 50000000 instdir/program/fftester @@ png

        On slower file formats like .doc you can probably drop the -t and rely on the
        estimations, on faster file formats ironically not specifying a timeout will
        result in a hilarious dramatic falloff in performance from thousands per second
        to teens per second as tiny variations from the initial calculated
        timeout will trigger a shutdown of the fftester and a restart and the
        startup time is woeful (hence the AFL_PERSISTENT mode in the first place)
  */

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/configmgr.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/event.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/pngread.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmf.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/fltcall.hxx>
#include <osl/file.hxx>
#include <unistd.h>
#include <signal.h>

#include <../source/filter/igif/gifread.hxx>
#include <../source/filter/ixbm/xbmread.hxx>
#include <../source/filter/ixpm/xpmread.hxx>
#include <../source/filter/jpeg/jpeg.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

#ifndef DISABLE_DYNLOADING
extern "C" { static void SAL_CALL thisModule() {} }
#endif

typedef bool (*WFilterCall)(const OUString &rUrl);
typedef bool (*FFilterCall)(SvStream &rStream);

/* This constant specifies the number of inputs to process before restarting.
 * This is optional, but helps limit the impact of memory leaks and similar
 * hiccups. */

#define PERSIST_MAX 1000
static unsigned int persist_cnt;

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int ret = -1;
    try
    {
        if (argc < 3)
        {
            fprintf(stderr, "Usage: fftester <filename> <wmf|jpg>\n");
            return -1;
        }

        setenv("SAL_USE_VCLPLUGIN", "svp", 1);
        setenv("JPEGMEM", "768M", 1);
        setenv("SAL_WMF_COMPLEXCLIP_VIA_REGION", "1", 1);
        setenv("SAL_DISABLE_PRINTERLIST", "1", 1);
        setenv("SAL_NO_FONT_LOOKUP", "1", 1);

        OUString in(argv[1], strlen(argv[1]), RTL_TEXTENCODING_UTF8);
        OUString out;
        osl::File::getFileURLFromSystemPath(in, out);

        tools::extendApplicationEnvironment();

        Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );
        if( !xServiceManager.is() )
            Application::Abort( "Failed to bootstrap" );
        comphelper::setProcessServiceFactory( xServiceManager );
        utl::ConfigManager::EnableAvoidConfig();
        InitVCL();

try_again:

        {
            if (strcmp(argv[2], "wmf") == 0 || strcmp(argv[2], "emf") == 0)
            {
                GDIMetaFile aGDIMetaFile;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) ReadWindowMetafile(aFileStream, aGDIMetaFile);
            }
            else if (strcmp(argv[2], "jpg") == 0)
            {
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) ImportJPEG(aFileStream, aGraphic, GraphicFilterImportFlags::NONE, nullptr);
            }
            else if (strcmp(argv[2], "gif") == 0)
            {
                SvFileStream aFileStream(out, StreamMode::READ);
                Graphic aGraphic;
                ret = (int) ImportGIF(aFileStream, aGraphic);
            }
            else if (strcmp(argv[2], "xbm") == 0)
            {
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) ImportXBM(aFileStream, aGraphic);
            }
            else if (strcmp(argv[2], "xpm") == 0)
            {
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) ImportXPM(aFileStream, aGraphic);
            }
            else if (strcmp(argv[2], "png") == 0)
            {
                SvFileStream aFileStream(out, StreamMode::READ);
                vcl::PNGReader aReader(aFileStream);
            }
            else if (strcmp(argv[2], "bmp") == 0)
            {
                Bitmap aTarget;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) ReadDIB(aTarget, aFileStream, true);
            }
            else if (strcmp(argv[2], "svm") == 0)
            {
                GDIMetaFile aGDIMetaFile;
                SvFileStream aFileStream(out, StreamMode::READ);
                ReadGDIMetaFile(aFileStream, aGDIMetaFile);
            }
#ifndef DISABLE_DYNLOADING
            else if (strcmp(argv[2], "pcd") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("icdGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "dxf") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("idxGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "met") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("imeGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if ((strcmp(argv[2], "pbm") == 0) || strcmp(argv[2], "ppm") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("ipbGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "psd") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("ipdGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "eps") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("ipsGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "pct") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("iptGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "pcx") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("ipxGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "ras") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("iraGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "tga") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("itgGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if (strcmp(argv[2], "tif") == 0)
            {
                static PFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libgielo.so");
                    pfnImport = reinterpret_cast<PFilterCall>(
                        aLibrary.getFunctionSymbol("itiGraphicImport"));
                    aLibrary.release();
                }
                Graphic aGraphic;
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream, aGraphic, nullptr);
            }
            else if ((strcmp(argv[2], "doc") == 0) || (strcmp(argv[2], "ww8") == 0))
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libmswordlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportWW8"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "ww6") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libmswordlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportWW6"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "ww2") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libmswordlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportWW2"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "rtf") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libmswordlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportRTF"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "xls") == 0)
            {
                static WFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libscfiltlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<WFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportXLS"));
                    aLibrary.release();
                }
                ret = (int) (*pfnImport)(out);
            }
            else if (strcmp(argv[2], "hwp") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libhwplo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportHWP"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "602") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libt602filterlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImport602"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "lwp") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "liblwpftlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportLWP"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "ppt") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libsdfiltlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportPPT"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "cgm") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libsdlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportCGM"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "qpw") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libscfiltlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportQPW"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "sc-rtf") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libscfiltlo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportCalcRTF"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }
            else if (strcmp(argv[2], "slk") == 0)
            {
                static FFilterCall pfnImport(nullptr);
                if (!pfnImport)
                {
                    osl::Module aLibrary;
                    aLibrary.loadRelative(&thisModule, "libsclo.so", SAL_LOADMODULE_LAZY);
                    pfnImport = reinterpret_cast<FFilterCall>(
                        aLibrary.getFunctionSymbol("TestImportSLK"));
                    aLibrary.release();
                }
                SvFileStream aFileStream(out, StreamMode::READ);
                ret = (int) (*pfnImport)(aFileStream);
            }

#endif
        }

        /* To signal successful completion of a run, we need to deliver
           SIGSTOP to our own process, then loop to the very beginning
           once we're resumed by the supervisor process. We do this only
           if AFL_PERSISTENT is set to retain normal behavior when the
           program is executed directly; and take note of PERSIST_MAX. */
        if (getenv("AFL_PERSISTENT") && persist_cnt++ < PERSIST_MAX)
        {
            raise(SIGSTOP);
            goto try_again;
        }

        /* If AFL_PERSISTENT not set or PERSIST_MAX exceeded, exit normally. */
    }
    catch (...)
    {
        abort();
    }

    _exit(ret);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
