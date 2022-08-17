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
#include <comphelper/diagnose.hxx>
#include <tools/degree.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>

#include <vcl/font/Feature.hxx>
#include <vcl/metric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

namespace
{
OUString GetOctetTextEncodingName(sal_uInt16 encoding)
{
    switch (encoding)
    {
        case RTL_TEXTENCODING_APPLE_ARABIC:
            return "Arabic (Apple Macintosh)";

        case RTL_TEXTENCODING_IBM_864:
            return "Arabic (DOS/OS2-864)";

        case RTL_TEXTENCODING_ISO_8859_6:
            return "Arabic (ISO-8859-6)";

        case RTL_TEXTENCODING_MS_1256:
            return "Arabic (Windows-1256)";

        case RTL_TEXTENCODING_IBM_775:
            return "Baltic (DOS/OS2-775)";

        case RTL_TEXTENCODING_ISO_8859_4:
            return "Baltic (ISO-8859-4)";

        case RTL_TEXTENCODING_MS_1257:
            return "Baltic (Windows-1257)";

        case RTL_TEXTENCODING_APPLE_CENTEURO:
            return "Central European (Apple Macintosh)";

        case RTL_TEXTENCODING_APPLE_CROATIAN:
            return "Central European (Apple Macintosh/Croatian)";

        case RTL_TEXTENCODING_APPLE_ROMANIAN:
            return "Central European (Apple Macintosh/Romanian)";

        case RTL_TEXTENCODING_IBM_852:
            return "Central European (DOS/OS2-852)";

        case RTL_TEXTENCODING_ISO_8859_2:
            return "Central European (ISO-8859-2)";

        case RTL_TEXTENCODING_ISO_8859_10:
            return "Central European (ISO-8859-10)";

        case RTL_TEXTENCODING_ISO_8859_13:
            return "Central European (ISO-8859-13)";

        case RTL_TEXTENCODING_MS_1250:
            return "Central European (Windows-1250/WinLatin 2)";

        case RTL_TEXTENCODING_APPLE_CHINSIMP:
            return "Chinese Simplified (Apple Macintosh)";

        case RTL_TEXTENCODING_EUC_CN:
            return "Chinese Simplified (EUC-CN)";

        case RTL_TEXTENCODING_GB_2312:
            return "Chinese Simplified (GB-2312)";

        case RTL_TEXTENCODING_GBK:
            return "Chinese Simplified (GBK/GB-2312-80)";

        case RTL_TEXTENCODING_ISO_2022_CN:
            return "Chinese Simplified (ISO-2022-CN)";

        case RTL_TEXTENCODING_MS_936:
            return "Chinese Simplified (Windows-936)";

        case RTL_TEXTENCODING_GB_18030:
            return "Chinese Simplified (GB-18030)";

        case RTL_TEXTENCODING_APPLE_CHINTRAD:
            return "Chinese Traditional (Apple Macintosh)";

        case RTL_TEXTENCODING_BIG5:
            return "Chinese Traditional (BIG5)";

        case RTL_TEXTENCODING_EUC_TW:
            return "Chinese Traditional (EUC-TW)";

        case RTL_TEXTENCODING_GBT_12345:
            return "Chinese Traditional (GBT-12345)";

        case RTL_TEXTENCODING_MS_950:
            return "Chinese Traditional (Windows-950)";

        case RTL_TEXTENCODING_BIG5_HKSCS:
            return "Chinese Traditional (BIG5-HKSCS)";

        case RTL_TEXTENCODING_APPLE_CYRILLIC:
            return "Cyrillic (Apple Macintosh)";

        case RTL_TEXTENCODING_APPLE_UKRAINIAN:
            return "Cyrillic (Apple Macintosh/Ukrainian)";

        case RTL_TEXTENCODING_IBM_855:
            return "Cyrillic (DOS/OS2-855)";

        case RTL_TEXTENCODING_IBM_866:
            return "Cyrillic (DOS/OS2-866/Russian)";

        case RTL_TEXTENCODING_ISO_8859_5:
            return "Cyrillic (ISO-8859-5)";

        case RTL_TEXTENCODING_KOI8_R:
            return "Cyrillic (KOI8-R)";

        case RTL_TEXTENCODING_KOI8_U:
            return "Cyrillic (KOI8-U)";

        case RTL_TEXTENCODING_MS_1251:
            return "Cyrillic (Windows-1251)";

        case RTL_TEXTENCODING_APPLE_GREEK:
            return "Greek (Apple Macintosh)";

        case RTL_TEXTENCODING_IBM_737:
            return "Greek (DOS/OS2-737)";

        case RTL_TEXTENCODING_IBM_869:
            return "Greek (DOS/OS2-869/Modern)";

        case RTL_TEXTENCODING_ISO_8859_7:
            return "Greek (ISO-8859-7)";

        case RTL_TEXTENCODING_MS_1253:
            return "Greek (Windows-1253)";

        case RTL_TEXTENCODING_APPLE_HEBREW:
            return "Hebrew (Apple Macintosh)";

        case RTL_TEXTENCODING_IBM_862:
            return "Hebrew (DOS/OS2-862)";

        case RTL_TEXTENCODING_ISO_8859_8:
            return "Hebrew (ISO-8859-8)";

        case RTL_TEXTENCODING_MS_1255:
            return "Hebrew (Windows-1255)";

        case RTL_TEXTENCODING_APPLE_KOREAN:
            return "Korean (Apple Macintosh)";

        case RTL_TEXTENCODING_EUC_KR:
            return "Korean (EUC-KR)";

        case RTL_TEXTENCODING_ISO_2022_KR:
            return "Korean (ISO-2022-KR)";

        case RTL_TEXTENCODING_MS_949:
            return "Korean (Windows-Wansung-949)";

        case RTL_TEXTENCODING_MS_1361:
            return "Korean (Windows-Johab-1361)";

        case RTL_TEXTENCODING_ISO_8859_3:
            return "Latin 3 (ISO-8859-3)";

        case RTL_TEXTENCODING_ISCII_DEVANAGARI:
            return "Indian (ISCII Devanagari)";

        case RTL_TEXTENCODING_APPLE_JAPANESE:
            return "Japanese (Apple Macintosh)";

        case RTL_TEXTENCODING_EUC_JP:
            return "Japanese (EUC-JP)";

        case RTL_TEXTENCODING_ISO_2022_JP:
            return "Japanese (ISO-2022-JP)";

        case RTL_TEXTENCODING_SHIFT_JIS:
            return "Japanese (Shift-JIS)";

        case RTL_TEXTENCODING_MS_932:
            return "Japanese (Windows-932)";

        case RTL_TEXTENCODING_SYMBOL:
            return "Symbol";

        case RTL_TEXTENCODING_APPLE_THAI:
            return "Thai (Apple Macintosh)";

        case RTL_TEXTENCODING_MS_874:
            return "Thai (Dos/Windows-874)";

        case RTL_TEXTENCODING_TIS_620:
            return "Thai (TIS 620)";

        case RTL_TEXTENCODING_APPLE_TURKISH:
            return "Turkish (Apple Macintosh)";

        case RTL_TEXTENCODING_IBM_857:
            return "Turkish (DOS/OS2-857)";

        case RTL_TEXTENCODING_ISO_8859_9:
            return "Turkish (ISO-8859-9)";

        case RTL_TEXTENCODING_MS_1254:
            return "Turkish (Windows-1254)";

        case RTL_TEXTENCODING_UTF7:
            return "Unicode (UTF-7)";

        case RTL_TEXTENCODING_UTF8:
            return "Unicode (UTF-8)";

        case RTL_TEXTENCODING_JAVA_UTF8:
            return "Unicode (Java's modified UTF-8)";

        case RTL_TEXTENCODING_MS_1258:
            return "Vietnamese (Windows-1258)";

        case RTL_TEXTENCODING_APPLE_ROMAN:
            return "Western (Apple Macintosh)";

        case RTL_TEXTENCODING_APPLE_ICELAND:
            return "Western (Apple Macintosh/Icelandic)";

        case RTL_TEXTENCODING_ASCII_US:
            return "Western (ASCII/US)";

        case RTL_TEXTENCODING_IBM_437:
            return "Western (DOS/OS2-437/US)";

        case RTL_TEXTENCODING_IBM_850:
            return "Western (DOS/OS2-850/International)";

        case RTL_TEXTENCODING_IBM_860:
            return "Western (DOS/OS2-860/Portuguese)";

        case RTL_TEXTENCODING_IBM_861:
            return "Western (DOS/OS2-861/Icelandic)";

        case RTL_TEXTENCODING_IBM_863:
            return "Western (DOS/OS2-863/Canadian-French)";

        case RTL_TEXTENCODING_IBM_865:
            return "Western (DOS/OS2-865/Nordic)";

        case RTL_TEXTENCODING_ISO_8859_1:
            return "Western (ISO-8859-1)";

        case RTL_TEXTENCODING_ISO_8859_14:
            return "Western (ISO-8859-14)";

        case RTL_TEXTENCODING_ISO_8859_15:
            return "Western (ISO-8859-15/EURO)";

        case RTL_TEXTENCODING_MS_1252:
            return "Western (Window-1252/WinLatin 1)";

        case RTL_TEXTENCODING_UCS4:
            return "UCS4";

        case RTL_TEXTENCODING_UCS2:
            return "UCS2 (aka Unicode)";

        default:
        {
            OUString sUnknown = "Unknown (0x" + OUString::number(encoding, 16) + ")";
            return sUnknown;
        }
    }
}

class ListFontsWin : public WorkWindow
{
public:
    explicit ListFontsWin()
        : WorkWindow(nullptr, WB_HIDE)
    {
    }
};

class ListFonts : public Application
{
public:
    virtual int Main() override;

private:
    static void showHelp()
    {
        std::cerr << "Usage: listfonts --help | FILE | -v FILE\n";
        std::cerr << "Lists the current fonts installed on the system.\n";
        std::cerr << "To show the font features of each font, use -v before FILE.\n";
        std::cerr << "If outputting to stdout, use -- for FILE.\n";
        std::exit(0);
    }

    void Init() override;
    void DeInit() override;

    css::uno::Reference<css::lang::XMultiServiceFactory> xServiceManager;
    bool mbStdOut = false;
    bool mbShowFeatures = false;
    OUString maFilename;
};

int ListFonts::Main()
{
    try
    {
        VclPtrInstance<ListFontsWin> pWin;
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

        for (int i = 0; i < pOutDev->GetFontFaceCollectionCount(); i++)
        {
            // note: to get the correct font metrics, you actually have to get the font metric from the
            // system, and *then* you must set it as the current font of OutputDevice... then you need
            // to get the font metric (which corrects a variety of things like the orientation, line
            // height, slant, etc. - including converting from logical coords to device coords)

            FontMetric aSystemFont = pOutDev->GetFontMetricFromCollection(i);
            pOutDev->SetFont(aSystemFont);

            FontMetric aFont = pOutDev->GetFontMetric();

            std::cout << aFont.GetFamilyName() << "\n\tFamily type: " << aFont.GetFamilyType()
                      << "\n\tStyle name: " << aFont.GetStyleName()
                      << "\n\tWeight: " << aFont.GetWeight() << "\n\tItalic: " << aFont.GetItalic()
                      << "\n\tPitch: " << aFont.GetPitch()
                      << "\n\tWidth type: " << aFont.GetWidthType()
                      << "\n\tAlignment: " << aFont.GetAlignment()
                      << "\n\tCharset: " << GetOctetTextEncodingName(aFont.GetCharSet())
                      << "\n\tSymbol font? " << (aFont.IsSymbolFont() ? "yes" : "no")
                      << "\n\tAscent: " << aFont.GetAscent()
                      << "\n\tDescent: " << aFont.GetDescent()
                      << "\n\tInternal leading: " << aFont.GetInternalLeading()
                      << "\n\tExternal leading: " << aFont.GetExternalLeading()
                      << "\n\tLine height: " << aFont.GetLineHeight()
                      << "\n\tSlant: " << aFont.GetSlant()
                      << "\n\tBullet offset: " << aFont.GetBulletOffset()
                      << "\n\tFullstop centered? " << (aFont.IsFullstopCentered() ? "yes" : "no")
                      << "\n\tOrientation: " << toDegrees(aFont.GetOrientation())
                      << " degrees\n\tQuality: " << aFont.GetQuality() << "\n";

            if (mbShowFeatures)
            {
                std::vector<vcl::font::Feature> features;
                pOutDev->GetFontFeatures(features);

                for (auto const& feature : features)
                {
                    std::ios init(nullptr);
                    init.copyfmt(std::cout);

                    std::cout << "\t"
                              << (feature.m_eType == vcl::font::FeatureType::OpenType ? "OpenType"
                                                                                      : "Graphite")
                              << " Feature:\n\t\tId = { feature code = "
                              << vcl::font::featureCodeAsString(feature.m_aID.m_aFeatureCode)
                              << ", script code = "
                              << vcl::font::featureCodeAsString(feature.m_aID.m_aScriptCode)
                              << ", language code = "
                              << vcl::font::featureCodeAsString(feature.m_aID.m_aLanguageCode)
                              << " }\n";

                    std::cout << "\t\tDescription: " << feature.m_aDefinition.getDescription()
                              << "\n";
                    std::cout << "\t\tType: "
                              << (feature.m_aDefinition.getType()
                                          == vcl::font::FeatureParameterType::BOOL
                                      ? "BOOL"
                                      : "ENUM")
                              << "\n";

                    std::cout.copyfmt(init);

                    if (feature.m_aDefinition.getType() == vcl::font::FeatureParameterType::ENUM)
                    {
                        for (auto const& param : feature.m_aDefinition.getEnumParameters())
                        {
                            std::cout << "\t\t\t" << param.getDescription() << ": "
                                      << param.getCode() << "\n";
                        }
                    }

                    std::cout << "\t\tDefault: 0x" << std::hex << feature.m_aDefinition.getDefault()
                              << "\n";

                    std::cout.copyfmt(init);
                }
            }
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
}

void ListFonts::Init()
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
        else if (nCmdParams == 2 && (aArg == "--verbose" || aArg == "-v"))
        {
            aArg = GetCommandLineParam(1);
            mbShowFeatures = true;

            if (aArg == "--")
                mbStdOut = true;
        }
        else if (nCmdParams == 1)
        {
            if (aArg == "--")
                mbStdOut = true;
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
        maFilename = aArg;

        osl::File aFile(maFilename);

        if (!aFile.open(osl_File_OpenFlag_Create))
            throw css::uno::RuntimeException("Can not create file: " + aArg);

        aFile.close();
    }

    auto xContext = cppu::defaultBootstrap_InitialComponentContext();
    xServiceManager.set(xContext->getServiceManager(), css::uno::UNO_QUERY);

    if (!xServiceManager.is())
        Application::Abort("Bootstrap failure - no service manager");

    comphelper::setProcessServiceFactory(xServiceManager);

    LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());
}

void ListFonts::DeInit()
{
    auto xContext = css::uno::Reference<css::lang::XComponent>(
        comphelper::getProcessComponentContext(), css::uno::UNO_QUERY_THROW);
    xContext->dispose();
    ::comphelper::setProcessServiceFactory(nullptr);
}

SAL_IMPLEMENT_MAIN()
{
    ListFonts aApp;
    InitVCL();
    int ret = aApp.Main();
    DeInitVCL();

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
