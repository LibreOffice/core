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
        make
        cp workdir/LinkTarget/Executable/fftester instdir/program
        LD_LIBRARY_PATH=`pwd`/instdir/program instdir/program/fftester <foo> png
    or on macOS
        make
        cp workdir/LinkTarget/Executable/fftester instdir/LibreOfficeDev.app/Contents/MacOS/
        DYLD_LIBRARY_PATH=`pwd`/instdir/LibreOfficeDev.app/Contents/Frameworks instdir/LibreOfficeDev.app/Contents/MacOS/fftester <foo> png
  */

#include <sal/config.h>

#include <string_view>

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/configuration.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/event.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/filter/SvmReader.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/wmf.hxx>
#include <vcl/wrkwin.hxx>
#include <fltcall.hxx>
#include <filter/TiffReader.hxx>
#include <filter/TgaReader.hxx>
#include <filter/PictReader.hxx>
#include <filter/MetReader.hxx>
#include <filter/RasReader.hxx>
#include <filter/PcxReader.hxx>
#include <filter/EpsReader.hxx>
#include <filter/PsdReader.hxx>
#include <filter/PcdReader.hxx>
#include <filter/PbmReader.hxx>
#include <filter/DxfReader.hxx>
#include <filter/WebpReader.hxx>
#include <filter/XpmReader.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/stream.hxx>
#include <vcl/gdimtf.hxx>
#include <fontsubset.hxx>

#include "../source/filter/igif/gifread.hxx"
#include "../source/filter/ixbm/xbmread.hxx"
#include "../source/filter/jpeg/jpeg.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

typedef bool (*FFilterCall)(SvStream &rStream);

#ifndef DISABLE_DYNLOADING
namespace {

FFilterCall load(std::u16string_view library, char const * function) {
    OUString path = OUString::Concat("$LO_LIB_DIR/") + library;
    rtl::Bootstrap::expandMacros(path); //TODO: check for failure
    osl::Module aLibrary(path, SAL_LOADMODULE_LAZY);
    auto const fn = reinterpret_cast<FFilterCall>(aLibrary.getFunctionSymbol(function));
    aLibrary.release();
    return fn;
}

}
#endif

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
        setenv("SC_MAX_MATRIX_ELEMENTS", "60000000", 1);
        setenv("SC_NO_THREADED_CALCULATION", "1", 1);
        setenv("SAL_DISABLE_PRINTERLIST", "1", 1);
        setenv("SAL_DISABLE_DEFAULTPRINTER", "1", 1);
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
        comphelper::EnableFuzzing();

        // initialise unconfigured UCB:
        css::uno::Reference<css::ucb::XUniversalContentBroker> xUcb(comphelper::getProcessServiceFactory()->
            createInstance("com.sun.star.ucb.UniversalContentBroker"), css::uno::UNO_QUERY_THROW);
        css::uno::Sequence<css::uno::Any> aArgs{ css::uno::Any(OUString("NoConfig")) };
        css::uno::Reference<css::ucb::XContentProvider> xFileProvider(comphelper::getProcessServiceFactory()->
            createInstanceWithArguments("com.sun.star.ucb.FileContentProvider", aArgs), css::uno::UNO_QUERY_THROW);
        xUcb->registerContentProvider(xFileProvider, "file", true);

        Application::EnableHeadlessMode(false);
        InitVCL();

        if (strcmp(argv[2], "wmf") == 0 || strcmp(argv[2], "emf") == 0)
        {
            GDIMetaFile aGDIMetaFile;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ReadWindowMetafile(aFileStream, aGDIMetaFile));
        }
        else if (strcmp(argv[2], "jpg") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportJPEG(aFileStream, aGraphic, GraphicFilterImportFlags::NONE, nullptr));
        }
        else if (strcmp(argv[2], "gif") == 0)
        {
            SvFileStream aFileStream(out, StreamMode::READ);
            Graphic aGraphic;
            ret = static_cast<int>(ImportGIF(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "xbm") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportXBM(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "xpm") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportXPM(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "png") == 0)
        {
            SvFileStream aFileStream(out, StreamMode::READ);
            vcl::PngImageReader aReader(aFileStream);
            ret = static_cast<int>(!aReader.read().IsEmpty());
        }
        else if (strcmp(argv[2], "bmp") == 0)
        {
            Bitmap aTarget;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ReadDIB(aTarget, aFileStream, true));
        }
        else if (strcmp(argv[2], "pcd") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportPcdGraphic(aFileStream, aGraphic, nullptr));
        }
        else if (strcmp(argv[2], "dxf") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportDxfGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "met") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportMetGraphic(aFileStream, aGraphic));
        }
        else if ((strcmp(argv[2], "pbm") == 0) || strcmp(argv[2], "ppm") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportPbmGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "psd") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportPsdGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "eps") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportEpsGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "pct") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportPictGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "pcx") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportPcxGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "ras") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportRasGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "tga") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportTgaGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "tif") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportTiffGraphicImport(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "webp") == 0)
        {
            Graphic aGraphic;
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>(ImportWebpGraphic(aFileStream, aGraphic));
        }
        else if (strcmp(argv[2], "sft") == 0)
        {
            SvFileStream aFileStream(out, StreamMode::READ);
            std::vector<sal_uInt8> aData(aFileStream.remainingSize());
            aFileStream.ReadBytes(aData.data(), aData.size());
            ret = TestFontSubset(aData.data(), aData.size());
        }
#ifndef DISABLE_DYNLOADING
        else if ((strcmp(argv[2], "doc") == 0) || (strcmp(argv[2], "ww8") == 0))
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libmswordlo.so", "TestImportWW8");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "ww6") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libmswordlo.so", "TestImportWW6");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "ww2") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libmswordlo.so", "TestImportWW2");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "rtf") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libmswordlo.so", "TestImportRTF");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "html") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libswlo.so", "TestImportHTML");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "fodt") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libswlo.so", "TestImportFODT");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "fodt2pdf") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libswlo.so", "TestPDFExportFODT");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "fods2xls") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsclo.so", "TestFODSExportXLS");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "docx") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libswlo.so", "TestImportDOCX");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "fods") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsclo.so", "TestImportFODS");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "xlsx") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsclo.so", "TestImportXLSX");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "fodp") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsdlo.so", "TestImportFODP");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "pptx") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsdlo.so", "TestImportPPTX");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "xls") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libscfiltlo.so", "TestImportXLS");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "wks") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libscfiltlo.so", "TestImportWKS");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "hwp") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libhwplo.so", "TestImportHWP");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "602") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libt602filterlo.so", "TestImport602");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "lwp") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"liblwpftlo.so", "TestImportLWP");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "ppt") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsdlo.so", "TestImportPPT");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "cgm") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsdlo.so", "TestImportCGM");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "qpw") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libscfiltlo.so", "TestImportQPW");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "dbf") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsclo.so", "TestImportDBF");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "dif") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libscfiltlo.so", "TestImportDIF");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "sc-rtf") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libscfiltlo.so", "TestImportCalcRTF");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "slk") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsclo.so", "TestImportSLK");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "ole") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsotlo.so", "TestImportOLE2");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "mml") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsmlo.so", "TestImportMML");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "mtp") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsmlo.so", "TestImportMathType");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "svm") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libvcllo.so", "TestImportSVM");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "zip") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libpackage2.so", "TestImportZip");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
        else if (strcmp(argv[2], "svg") == 0)
        {
            static FFilterCall pfnImport(nullptr);
            if (!pfnImport)
            {
                pfnImport = load(u"libsvgiolo.so", "TestImportSVG");
            }
            SvFileStream aFileStream(out, StreamMode::READ);
            ret = static_cast<int>((*pfnImport)(aFileStream));
        }
#endif
    }
    catch (...)
    {
        abort();
    }

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
