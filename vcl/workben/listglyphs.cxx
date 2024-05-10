/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/textenc.h>
#include <sal/main.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/degree.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/font/Feature.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>

#include <font/LogicalFontInstance.hxx>
#include <font/PhysicalFontFace.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

namespace
{
class ListGlyphsWin : public WorkWindow
{
public:
    explicit ListGlyphsWin()
        : WorkWindow(nullptr, WB_HIDE)
    {
    }
};

class ListGlyphs : public Application
{
public:
    virtual int Main() override;

private:
    static void showHelp()
    {
        std::cerr << "Usage: listglyphs --help | <fontname> FILE\n";
        std::cerr << "Lists the current glyphs in a font installed on the system.\n";
        std::cerr << "If outputting to stdout, use -- for FILE.\n";
        std::exit(0);
    }

    void Init() override;
    void DeInit() override;

    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager;
    bool mbStdOut = false;
    OUString maFilename;
    OUString maFontname;
};

int ListGlyphs::Main()
{
    try
    {
        VclPtrInstance<ListGlyphsWin> pWin;
        OutputDevice* pOutDev = pWin->GetOutDev();

        std::streambuf* coutbuf = nullptr;
        std::fstream out;

        if (!mbStdOut)
        {
            std::u16string_view filenamev = maFilename;
            std::string filename(filenamev.begin(), filenamev.end());

            out.open(filename, std::ios::out | std::ios::trunc);

            coutbuf = std::cout.rdbuf();
            std::cout.rdbuf(out.rdbuf());
        }

        bool bFontExists = false;

        for (sal_Int32 nFont = 0; nFont < pOutDev->GetFontFaceCollectionCount(); nFont++)
        {
            FontMetric aSystemFont = pOutDev->GetFontMetricFromCollection(nFont);
            if (aSystemFont.GetFamilyName() == maFontname)
            {
                bFontExists = true;
                break;
            }
        }

        if (!bFontExists)
        {
            std::cerr << maFontname << " does not exist\n";
            std::exit(1);
        }

        pOutDev->SetFont(vcl::Font(maFontname, Size(0, 11)));

        LogicalFontInstance const* pFontInstance = pOutDev->GetFontInstance();
        vcl::font::PhysicalFontFace const* pFontFace = pFontInstance->GetFontFace();
        FontCharMapRef pCharMap = pFontFace->GetFontCharMap();

        sal_UCS4 nLastChar = pCharMap->GetLastChar();
        for (sal_UCS4 nChar = pCharMap->GetFirstChar(); nChar < nLastChar;
             nChar = pCharMap->GetNextChar(nChar))
        {
            auto nGlyphIndex = pFontInstance->GetGlyphIndex(nChar);
            basegfx::B2DRectangle aGlyphBounds;
            pFontInstance->GetGlyphBoundRect(nGlyphIndex, aGlyphBounds, false);
            std::cout << "Codepoint: " << pFontFace->GetGlyphName(nGlyphIndex)
                      << "; glyph bounds: " << aGlyphBounds << "\n";
        }

        std::cout << std::flush;

        if (!mbStdOut)
        {
            std::cout.rdbuf(coutbuf);
            out.close();
        }

        std::exit(0);
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("vcl.app", "Fatal");
        return 1;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.app", "Fatal: " << e.what());
        return 1;
    }
    return 0;
}

void ListGlyphs::Init()
{
    const sal_uInt16 nCmdParams = GetCommandLineParamCount();
    OUString aArg;

    if (nCmdParams == 0)
    {
        showHelp();
        std::exit(1);
    }
    else
    {
        aArg = GetCommandLineParam(0);

        if (aArg == "--help" || aArg == "-h")
        {
            showHelp();
            std::exit(0);
        }

        if (nCmdParams == 2)
        {
            maFontname = GetCommandLineParam(0);

            aArg = GetCommandLineParam(1);

            if (aArg == "--")
                mbStdOut = true;

            maFilename = aArg;
        }
        else
        {
            std::cerr << "invalid arguments\n";
            showHelp();
            std::exit(1);
        }
    }

    if (!mbStdOut)
    {
        osl::File aFile(maFilename);

        if (!aFile.open(osl_File_OpenFlag_Create))
            throw css::uno::RuntimeException("Can not create file: " + aArg);

        aFile.close();
    }

    auto xContext = cppu::defaultBootstrap_InitialComponentContext();
    xServiceManager.set(xContext->getServiceManager(), css::uno::UNO_QUERY);

    if (!xServiceManager.is())
        Application::Abort(u"Bootstrap failure - no service manager"_ustr);

    comphelper::setProcessServiceFactory(xServiceManager);

    LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());
}

void ListGlyphs::DeInit()
{
    auto xContext = css::uno::Reference<css::lang::XComponent>(
        comphelper::getProcessComponentContext(), css::uno::UNO_QUERY_THROW);
    xContext->dispose();
    ::comphelper::setProcessServiceFactory(nullptr);
}
}

SAL_IMPLEMENT_MAIN()
{
    ListGlyphs aApp;
    InitVCL();
    int ret = aApp.Main();
    DeInitVCL();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
