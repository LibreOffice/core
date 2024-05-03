/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/lokhelper.hxx>
#include <sal/types.h>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <config_buildconfig.h>
#include <config_cairo_rgba.h>
#include <config_features.h>
#include <editeng/unolingu.hxx>

#include <stdio.h>

#ifdef IOS
#include <sys/mman.h>
#include <sys/stat.h>
#include <unicode/udata.h>
#include <unicode/ucnv.h>
#include <premac.h>
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#include <postmac.h>
#endif

#undef HAVE_MALLOC_TRIM

#ifdef UNX
#  include <fcntl.h>
#endif
#ifdef LINUX
#if defined __GLIBC__
#  include <malloc.h>
#  define HAVE_MALLOC_TRIM
#endif
#endif

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#endif

#ifdef EMSCRIPTEN
#include <osl/detail/emscripten-bootstrap.h>
#endif

#include <algorithm>
#include <memory>
#include <iostream>
#include <string_view>

#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sal/log.hxx>
#include <utility>
#include <vcl/errinf.hxx>
#include <vcl/lok.hxx>
#include <o3tl/any.hxx>
#include <o3tl/unit_conversion.hxx>
#include <o3tl/string_view.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <linguistic/misc.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/random.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/threadpool.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <com/sun/star/connection/XConnection.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/XTransferable2.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/uno/XNamingService.hpp>

#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XCertificateCreator.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <com/sun/star/linguistic2/DictionaryList.hpp>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/i18n/LocaleCalendar2.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>

#include <editeng/flstitem.hxx>
#ifdef IOS
#include <sfx2/app.hxx>
#endif
#include <sfx2/objsh.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/lokcomponenthelpers.hxx>
#include <sfx2/DocumentSigner.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <svl/numformat.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <svx/ucsubset.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/GestureEventPan.hxx>
#include <vcl/svapp.hxx>
#include <unotools/resmgr.hxx>
#include <tools/fract.hxx>
#include <tools/json_writer.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <vcl/fontcharmap.hxx>
#ifdef IOS
#include <vcl/sysdata.hxx>
#endif
#include <vcl/virdev.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/dialoghelper.hxx>
#ifdef _WIN32
#include <vcl/BitmapReadAccess.hxx>
#endif
#include <unicode/uchar.h>
#include <unotools/securityoptions.hxx>
#include <unotools/confignode.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/streamwrap.hxx>
#include <osl/module.hxx>
#include <comphelper/sequence.hxx>
#include <sfx2/sfxbasemodel.hxx>
#include <svl/undo.hxx>
#include <unotools/datetime.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/abstdlg.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/jsdialog/executor.hxx>

// Needed for getUndoManager()
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/document/XLinkTargetSupplier.hpp>
#include <editeng/sizeitem.hxx>
#include <svx/rulritem.hxx>
#include <svx/pageitem.hxx>

#include <app.hxx>

#include "../app/cmdlineargs.hxx"
// We also need to hackily be able to start the main libreoffice thread:
#include "../app/sofficemain.h"
#include "../app/officeipcthread.hxx"
#include <lib/init.hxx>

#include "lokinteractionhandler.hxx"
#include "lokclipboard.hxx"
#include <officecfg/Office/Common.hxx>
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <unotools/optionsdlg.hxx>
#include <svl/ctloptions.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/cmdoptions.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/searchopt.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/settings.hxx>

#include <officecfg/Setup.hxx>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <svtools/acceleratorexecute.hxx>

#include <tools/hostfilter.hxx>

using namespace css;
using namespace vcl;
using namespace desktop;
using namespace utl;
using namespace bridge;
using namespace uno;
using namespace lang;

#ifdef UNX

static int urandom = -1;

extern "C" {
    int SAL_JNI_EXPORT lok_open_urandom()
    {
        return dup(urandom);
    }
};

#endif


using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;

static LibLibreOffice_Impl *gImpl = nullptr;
static bool lok_preinit_2_called = false;
static std::weak_ptr< LibreOfficeKitClass > gOfficeClass;
static std::weak_ptr< LibreOfficeKitDocumentClass > gDocumentClass;

static void SetLastExceptionMsg(const OUString& s = OUString())
{
    SAL_WARN_IF(!s.isEmpty(), "lok", "lok exception '" + s + "'");
    if (gImpl)
        gImpl->maLastExceptionMsg = s;
}

namespace {

struct ExtensionMap
{
    std::string_view extn;
    OUString filterName;
};

class TraceEventDumper : public AutoTimer
{
    static const int dumpTimeoutMS = 5000;

public:
    TraceEventDumper() : AutoTimer( "Trace Event dumper" )
    {
        SetTimeout(dumpTimeoutMS);
        Start();
    }

    virtual void Invoke() override
    {
        flushRecordings();
    }

    static void flushRecordings()
    {
        const css::uno::Sequence<OUString> aEvents =
            comphelper::TraceEvent::getRecordingAndClear();
        OStringBuffer aOutput;
        for (const auto &s : aEvents)
        {
            aOutput.append(OUStringToOString(s, RTL_TEXTENCODING_UTF8)
                + "\n");
        }
        if (aOutput.getLength() > 0)
        {
            OString aChunk = aOutput.makeStringAndClear();
            if (gImpl && gImpl->mpCallback)
                gImpl->mpCallback(LOK_CALLBACK_PROFILE_FRAME, aChunk.getStr(), gImpl->mpCallbackData);
        }
    }
};

TraceEventDumper *traceEventDumper = nullptr;

constexpr ExtensionMap aWriterExtensionMap[] =
{
    { "doc",   u"MS Word 97"_ustr },
    { "docm",  u"MS Word 2007 XML VBA"_ustr },
    { "docx",  u"MS Word 2007 XML"_ustr },
    { "fodt",  u"OpenDocument Text Flat XML"_ustr },
    { "html",  u"HTML (StarWriter)"_ustr },
    { "odt",   u"writer8"_ustr },
    { "ott",   u"writer8_template"_ustr },
    { "pdf",   u"writer_pdf_Export"_ustr },
    { "epub",  u"EPUB"_ustr },
    { "rtf",   u"Rich Text Format"_ustr },
    { "txt",   u"Text"_ustr },
    { "xhtml", u"XHTML Writer File"_ustr },
    { "png",   u"writer_png_Export"_ustr },
    { "xml",   u"writer_indexing_export"_ustr },
};

constexpr ExtensionMap aCalcExtensionMap[] =
{
    { "csv",   u"Text - txt - csv (StarCalc)"_ustr },
    { "fods",  u"OpenDocument Spreadsheet Flat XML"_ustr },
    { "html",  u"HTML (StarCalc)"_ustr },
    { "ods",   u"calc8"_ustr },
    { "ots",   u"calc8_template"_ustr },
    { "pdf",   u"calc_pdf_Export"_ustr },
    { "xhtml", u"XHTML Calc File"_ustr },
    { "xls",   u"MS Excel 97"_ustr },
    { "xlsm",  u"Calc MS Excel 2007 VBA XML"_ustr },
    { "xlsx",  u"Calc MS Excel 2007 XML"_ustr },
    { "png",   u"calc_png_Export"_ustr },
};

constexpr ExtensionMap aImpressExtensionMap[] =
{
    { "fodp",  u"OpenDocument Presentation Flat XML"_ustr },
    { "html",  u"impress_html_Export"_ustr },
    { "odg",   u"impress8_draw"_ustr },
    { "odp",   u"impress8"_ustr },
    { "otp",   u"impress8_template"_ustr },
    { "pdf",   u"impress_pdf_Export"_ustr },
    { "potm",  u"Impress MS PowerPoint 2007 XML Template"_ustr },
    { "pot",   u"MS PowerPoint 97 Vorlage"_ustr },
    { "pptm",  u"Impress MS PowerPoint 2007 XML VBA"_ustr },
    { "pptx",  u"Impress MS PowerPoint 2007 XML"_ustr },
    { "pps",   u"MS PowerPoint 97 Autoplay"_ustr },
    { "ppt",   u"MS PowerPoint 97"_ustr },
    { "svg",   u"impress_svg_Export"_ustr },
    { "xhtml", u"XHTML Impress File"_ustr },
    { "png",   u"impress_png_Export"_ustr },
};

constexpr ExtensionMap aDrawExtensionMap[] =
{
    { "fodg",  u"draw_ODG_FlatXML"_ustr },
    { "html",  u"draw_html_Export"_ustr },
    { "odg",   u"draw8"_ustr },
    { "pdf",   u"draw_pdf_Export"_ustr },
    { "svg",   u"draw_svg_Export"_ustr },
    { "xhtml", u"XHTML Draw File"_ustr },
    { "png",   u"draw_png_Export"_ustr },
};

OUString getUString(const char* pString)
{
    if (pString == nullptr)
        return OUString();

    return OStringToOUString(pString, RTL_TEXTENCODING_UTF8);
}

// Tolerate embedded \0s etc.
char *convertOString(const OString &rStr)
{
    char* pMemory = static_cast<char*>(malloc(rStr.getLength() + 1));
    assert(pMemory); // don't tolerate failed allocations.
    memcpy(pMemory, rStr.getStr(), rStr.getLength() + 1);
    return pMemory;
}

char *convertOUString(std::u16string_view aStr)
{
    return convertOString(OUStringToOString(aStr, RTL_TEXTENCODING_UTF8));
}

/// Try to convert a relative URL to an absolute one, unless it already looks like a URL.
OUString getAbsoluteURL(const char* pURL)
{
    OUString aURL(getUString(pURL));
    if (aURL.isEmpty())
        return aURL;

    // convert relative paths to absolute ones
    OUString aWorkingDir;
    osl_getProcessWorkingDir(&aWorkingDir.pData);
    if (!aWorkingDir.endsWith("/"))
        aWorkingDir += "/";

    try
    {
        return rtl::Uri::convertRelToAbs(aWorkingDir, aURL);
    }
    catch (const rtl::MalformedUriException &)
    {
    }

    return OUString();
}

} // unnamed namespace

std::vector<beans::PropertyValue> desktop::jsonToPropertyValuesVector(const char* pJSON)
{
    std::vector<beans::PropertyValue> aArguments;
    if (pJSON && pJSON[0] != '\0')
    {
        aArguments = comphelper::JsonToPropertyValues(pJSON);
    }
    return aArguments;
}

static void extractLinks(const uno::Reference< container::XNameAccess >& xLinks, bool subcontent, tools::JsonWriter& aJson)
{
    for (const OUString& aLink : xLinks->getElementNames())
    {
        uno::Any aAny;

        try
        {
            aAny = xLinks->getByName( aLink );
        }
        catch(const uno::Exception&)
        {
            // if the name of the target was invalid (like empty headings)
            // no object can be provided
            continue;
        }

        uno::Reference< beans::XPropertySet > xTarget;
        if( aAny >>= xTarget )
        {
            try
            {
                // get name to display
                aAny = xTarget->getPropertyValue(u"LinkDisplayName"_ustr);
                OUString aStrDisplayname;
                aAny >>= aStrDisplayname;

                if (subcontent)
                {
                    aJson.put(aStrDisplayname, aLink);
                }
                else
                {
                    uno::Reference<lang::XServiceInfo> xSI(xTarget, uno::UNO_QUERY_THROW);
                    if (xSI->supportsService(u"com.sun.star.document.LinkTarget"_ustr))
                    {
                        aJson.put(aStrDisplayname, aLink);
                        continue;
                    }
                    else
                    {
                        auto aNode = aJson.startNode(
                            OUStringToOString(aStrDisplayname, RTL_TEXTENCODING_UTF8));

                        uno::Reference< document::XLinkTargetSupplier > xLTS( xTarget, uno::UNO_QUERY );
                        if( xLTS.is() )
                            extractLinks(xLTS->getLinks(), true, aJson);
                    }
                }
            }
            catch(...)
            {
                SAL_WARN("lok", "extractLinks: Exception");
            }
        }
    }
}

static void unoAnyToJson(tools::JsonWriter& rJson, std::string_view pNodeName, const uno::Any& anyItem)
{
    auto aNode = rJson.startNode(pNodeName);
    OUString aType = anyItem.getValueTypeName();
    rJson.put("type", aType);

    if (aType == "string")
        rJson.put("value", anyItem.get<OUString>());
    else if (aType == "unsigned long")
        rJson.put("value", OString::number(anyItem.get<sal_uInt32>()));
    else if (aType == "long")
        rJson.put("value", OString::number(anyItem.get<sal_Int32>()));
    else if (aType == "[]any")
    {
        uno::Sequence<uno::Any> aSeq;
        if (anyItem >>= aSeq)
        {
            auto valueNode = rJson.startNode("value");

            for (auto i = 0; i < aSeq.getLength(); ++i)
            {
                unoAnyToJson(rJson, OString::number(i), aSeq[i]);
            }
        }
    }
}

static int lcl_getViewId(std::string_view payload);

namespace desktop {

RectangleAndPart RectangleAndPart::Create(const OString& rPayload)
{
    RectangleAndPart aRet;
    if (rPayload.startsWith("EMPTY")) // payload starts with "EMPTY"
    {
        aRet.m_aRectangle = tools::Rectangle(0, 0, SfxLokHelper::MaxTwips, SfxLokHelper::MaxTwips);
        if (comphelper::LibreOfficeKit::isPartInInvalidation())
        {
            int nSeparatorPos = rPayload.indexOf(',', 6);
            bool bHasMode = nSeparatorPos > 0;
            if (bHasMode)
            {
                aRet.m_nPart = o3tl::toInt32(rPayload.subView(6, nSeparatorPos - 6));
                assert(rPayload.getLength() > nSeparatorPos);
                aRet.m_nMode = o3tl::toInt32(rPayload.subView(nSeparatorPos + 1));
            }
            else
            {
                aRet.m_nPart = o3tl::toInt32(rPayload.subView(6));
                aRet.m_nMode = 0;
            }
        }

        return aRet;
    }

    // Read '<left>, <top>, <width>, <height>[, <part>, <mode>]'. C++ streams are simpler but slower.
    const char* pos = rPayload.getStr();
    const char* end = rPayload.getStr() + rPayload.getLength();
    tools::Long nLeft = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    while (pos < end && *pos != ',')
        ++pos;
    if (pos < end)
        ++pos;
    assert(pos < end);
    tools::Long nTop = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    while (pos < end && *pos != ',')
        ++pos;
    if (pos < end)
        ++pos;
    assert(pos < end);
    tools::Long nWidth = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    while (pos < end && *pos != ',')
        ++pos;
    if (pos < end)
        ++pos;
    assert(pos < end);
    tools::Long nHeight = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    tools::Long nPart = INT_MIN;
    tools::Long nMode = 0;
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
    {
        while (pos < end && *pos != ',')
            ++pos;
        if (pos < end)
            ++pos;
        assert(pos < end);
        nPart = rtl_str_toInt64_WithLength(pos, 10, end - pos);

        while (pos < end && *pos != ',')
            ++pos;
        if (pos < end)
        {
            ++pos;
            assert(pos < end);
            nMode = rtl_str_toInt64_WithLength(pos, 10, end - pos);
        }
    }

    aRet.m_aRectangle = SanitizedRectangle(nLeft, nTop, nWidth, nHeight);
    aRet.m_nPart = nPart;
    aRet.m_nMode = nMode;
    return aRet;
}

tools::Rectangle RectangleAndPart::SanitizedRectangle(tools::Long nLeft, tools::Long nTop, tools::Long nWidth, tools::Long nHeight)
{
    if (nWidth <= 0 || nHeight <= 0)
        return tools::Rectangle();

    // The top-left corner starts at (0, 0).
    // Anything negative is invalid.
    if (nLeft < 0)
    {
        nWidth += nLeft;
        nLeft = 0;
    }

    if (nTop < 0)
    {
        nHeight += nTop;
        nTop = 0;
    }

    if (nWidth > 0 && nHeight > 0)
        return tools::Rectangle(nLeft, nTop, nLeft + nWidth, nTop + nHeight);
    // Else set empty rect.
    return tools::Rectangle();
}

tools::Rectangle RectangleAndPart::SanitizedRectangle(const tools::Rectangle& rect)
{
    return SanitizedRectangle(rect.Left(), rect.Top(), rect.getOpenWidth(), rect.getOpenHeight());
}

const OString& CallbackFlushHandler::CallbackData::getPayload() const
{
    if(PayloadString.isEmpty())
    {
        // Do to-string conversion on demand, as many calls will get dropped without
        // needing the string.
        if(PayloadObject.which() == 1)
            PayloadString = getRectangleAndPart().toString();
    }
    return PayloadString;
}

void CallbackFlushHandler::CallbackData::updateRectangleAndPart(const RectangleAndPart& rRectAndPart)
{
    PayloadObject = rRectAndPart;
    PayloadString.clear(); // will be set on demand if needed
}

const RectangleAndPart& CallbackFlushHandler::CallbackData::getRectangleAndPart() const
{
    // TODO: In case of unittests, they do not pass invalidations in binary but as text messages.
    // LO core should preferably always pass binary for performance.
    if(PayloadObject.which() != 1)
        PayloadObject = RectangleAndPart::Create(PayloadString);
    return boost::get<RectangleAndPart>(PayloadObject);
}

boost::property_tree::ptree& CallbackFlushHandler::CallbackData::setJson(const std::string& payload)
{
    boost::property_tree::ptree aTree;
    std::stringstream aStream(payload);
    boost::property_tree::read_json(aStream, aTree);

    // Let boost normalize the payload so it always matches the cache.
    setJson(aTree);

    // Return reference to the cached object.
    return boost::get<boost::property_tree::ptree>(PayloadObject);
}

void CallbackFlushHandler::CallbackData::setJson(const boost::property_tree::ptree& rTree)
{
    std::stringstream aJSONStream;
    constexpr bool bPretty = false; // Don't waste time and bloat logs.
    boost::property_tree::write_json(aJSONStream, rTree, bPretty);
    PayloadString = OString(o3tl::trim(aJSONStream.str()));

    PayloadObject = rTree;
}

const boost::property_tree::ptree& CallbackFlushHandler::CallbackData::getJson() const
{
    assert(PayloadObject.which() == 2);
    return boost::get<boost::property_tree::ptree>(PayloadObject);
}

int CallbackFlushHandler::CallbackData::getViewId() const
{
    if (isCached())
    {
        assert(PayloadObject.which() == 3);
        return boost::get<int>(PayloadObject);
    }
    return lcl_getViewId(getPayload());
}

bool CallbackFlushHandler::CallbackData::validate() const
{
    switch (PayloadObject.which())
    {
        // Not cached.
        case 0:
            return true;

        // RectangleAndPart.
        case 1:
            return getRectangleAndPart().toString().getStr() == getPayload();

        // Json.
        case 2:
        {
            std::stringstream aJSONStream;
            boost::property_tree::write_json(aJSONStream, getJson(), false);
            const std::string aExpected = boost::trim_copy(aJSONStream.str());
            return getPayload() == std::string_view(aExpected);
        }

        // View id.
        case 3:
            return getViewId() == lcl_getViewId( getPayload());

        default:
            assert(!"Unknown variant type; please add an entry to validate.");
    }

    return false;
}

} // namespace desktop

static bool lcl_isViewCallbackType(const int type)
{
    switch (type)
    {
        case LOK_CALLBACK_CELL_VIEW_CURSOR:
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
            return true;

        default:
            return false;
    }
}

static bool isUpdatedType(int type)
{
    switch (type)
    {
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
            return true;
        default:
            return false;
    }
}

static bool isUpdatedTypePerViewId(int type)
{
    switch (type)
    {
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            return true;
        default:
            return false;
    }
}

static int lcl_getViewId(std::string_view payload)
{
    // this is a cheap way how to get the viewId from a JSON message; proper
    // parsing is terribly expensive, and we just need the viewId here
    size_t viewIdPos = payload.find("viewId");
    if (viewIdPos == std::string::npos)
        return 0;

    size_t numberPos = payload.find(":", viewIdPos + 6);
    if (numberPos == std::string::npos)
        return 0;

    for (++numberPos; numberPos < payload.length(); ++numberPos)
    {
        if (payload[numberPos] == ',' || payload[numberPos] == '}' || (payload[numberPos] >= '0' && payload[numberPos] <= '9'))
            break;
    }

    if (numberPos < payload.length() && payload[numberPos] >= '0' && payload[numberPos] <= '9')
        return o3tl::toInt32(payload.substr(numberPos));

    return 0;
}

namespace {

std::string extractCertificate(const std::string & certificate)
{
    static constexpr std::string_view header("-----BEGIN CERTIFICATE-----");
    static constexpr std::string_view footer("-----END CERTIFICATE-----");

    std::string result;

    size_t pos1 = certificate.find(header);
    if (pos1 == std::string::npos)
        return result;

    size_t pos2 = certificate.find(footer, pos1 + 1);
    if (pos2 == std::string::npos)
        return result;

    pos1 = pos1 + header.length();
    pos2 = pos2 - pos1;

    return certificate.substr(pos1, pos2);
}

std::string extractPrivateKey(const std::string & privateKey)
{
    static constexpr std::string_view header("-----BEGIN PRIVATE KEY-----");
    static constexpr std::string_view footer("-----END PRIVATE KEY-----");

    std::string result;

    size_t pos1 = privateKey.find(header);
    if (pos1 == std::string::npos)
        return result;

    size_t pos2 = privateKey.find(footer, pos1 + 1);
    if (pos2 == std::string::npos)
        return result;

    pos1 = pos1 + header.length();
    pos2 = pos2 - pos1;

    return privateKey.substr(pos1, pos2);
}

OUString lcl_getCurrentDocumentMimeType(const LibLODocument_Impl* pDocument)
{
    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(pDocument->mxComponent.get());
    if (!pBaseModel)
        return u""_ustr;

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    if (!pObjectShell)
        return u""_ustr;

    SfxMedium* pMedium = pObjectShell->GetMedium();
    if (!pMedium)
        return u""_ustr;

    auto pFilter = pMedium->GetFilter();
    if (!pFilter)
        return u""_ustr;

    return pFilter->GetMimeType();
}

// Gets an undo manager to enter and exit undo context. Needed by ToggleOrientation
css::uno::Reference< css::document::XUndoManager > getUndoManager( const css::uno::Reference< css::frame::XFrame >& rxFrame )
{
    const css::uno::Reference< css::frame::XController >& xController = rxFrame->getController();
    if ( xController.is() )
    {
        const css::uno::Reference< css::frame::XModel >& xModel = xController->getModel();
        if ( xModel.is() )
        {
            const css::uno::Reference< css::document::XUndoManagerSupplier > xSuppUndo( xModel, css::uno::UNO_QUERY_THROW );
            return css::uno::Reference< css::document::XUndoManager >( xSuppUndo->getUndoManager(), css::uno::UNO_SET_THROW );
        }
    }

    return css::uno::Reference< css::document::XUndoManager > ();
}

// Adjusts page margins for Writer doc. Needed by ToggleOrientation
void ExecuteMarginLRChange(
    const tools::Long nPageLeftMargin,
    const tools::Long nPageRightMargin,
    SvxLongLRSpaceItem* pPageLRMarginItem)
{
    pPageLRMarginItem->SetLeft( nPageLeftMargin );
    pPageLRMarginItem->SetRight( nPageRightMargin );
    SfxViewShell* pViewSh = SfxViewShell::Current();
    if (!pViewSh)
        return;
    pViewSh->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_LRSPACE,
            SfxCallMode::RECORD, { pPageLRMarginItem });
}

// Adjusts page margins for Writer doc. Needed by ToggleOrientation
void ExecuteMarginULChange(
        const tools::Long nPageTopMargin,
        const tools::Long nPageBottomMargin,
        SvxLongULSpaceItem* pPageULMarginItem)
{
    pPageULMarginItem->SetUpper( nPageTopMargin );
    pPageULMarginItem->SetLower( nPageBottomMargin );
    SfxViewShell* pViewSh = SfxViewShell::Current();
    if (!pViewSh)
        return;
    pViewSh->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_ULSPACE,
            SfxCallMode::RECORD, { pPageULMarginItem });
}

// Main function which toggles page orientation of the Writer doc. Needed by ToggleOrientation
void ExecuteOrientationChange()
{
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    std::unique_ptr<SvxPageItem> pPageItem(new SvxPageItem(SID_ATTR_PAGE));

    // 1mm in twips rounded
    // This should be in sync with MINBODY in sw/source/uibase/sidebar/PageMarginControl.hxx
    constexpr tools::Long MINBODY = o3tl::toTwips(1, o3tl::Length::mm);

    css::uno::Reference< css::document::XUndoManager > mxUndoManager(
                getUndoManager( pViewFrm->GetFrame().GetFrameInterface() ) );

    if ( mxUndoManager.is() )
        mxUndoManager->enterUndoContext( u""_ustr );

    SfxPoolItemHolder aResult;
    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_SIZE, aResult);
    std::unique_ptr<SvxSizeItem> pPageSizeItem(static_cast<const SvxSizeItem*>(aResult.getItem())->Clone());

    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_LRSPACE, aResult);
    std::unique_ptr<SvxLongLRSpaceItem> pPageLRMarginItem(static_cast<const SvxLongLRSpaceItem*>(aResult.getItem())->Clone());

    pViewFrm->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_ULSPACE, aResult);
    std::unique_ptr<SvxLongULSpaceItem> pPageULMarginItem(static_cast<const SvxLongULSpaceItem*>(aResult.getItem())->Clone());

    {
        bool bIsLandscape = false;
        if ( pPageSizeItem->GetSize().Width() > pPageSizeItem->GetSize().Height())
            bIsLandscape = true;

        // toggle page orientation
        pPageItem->SetLandscape(!bIsLandscape);


        // swap the width and height of the page size
        const tools::Long nRotatedWidth = pPageSizeItem->GetSize().Height();
        const tools::Long nRotatedHeight = pPageSizeItem->GetSize().Width();
        pPageSizeItem->SetSize(Size(nRotatedWidth, nRotatedHeight));


        // apply changed attributes
        if (SfxViewShell* pCurrent = SfxViewShell::Current())
        {
            pCurrent->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE,
                SfxCallMode::RECORD, { pPageSizeItem.get(), pPageItem.get() });
        }
    }


    // check, if margin values still fit to the changed page size.
    // if not, adjust margin values
    {
        const tools::Long nML = pPageLRMarginItem->GetLeft();
        const tools::Long nMR = pPageLRMarginItem->GetRight();
        const tools::Long nTmpPW = nML + nMR + MINBODY;

        const tools::Long nPW  = pPageSizeItem->GetSize().Width();

        if ( nTmpPW > nPW )
        {
            if ( nML <= nMR )
            {
                ExecuteMarginLRChange( pPageLRMarginItem->GetLeft(), nMR - (nTmpPW - nPW ), pPageLRMarginItem.get() );
            }
            else
            {
                ExecuteMarginLRChange( nML - (nTmpPW - nPW ), pPageLRMarginItem->GetRight(), pPageLRMarginItem.get() );
            }
        }

        const tools::Long nMT = pPageULMarginItem->GetUpper();
        const tools::Long nMB = pPageULMarginItem->GetLower();
        const tools::Long nTmpPH = nMT + nMB + MINBODY;

        const tools::Long nPH  = pPageSizeItem->GetSize().Height();

        if ( nTmpPH > nPH )
        {
            if ( nMT <= nMB )
            {
                ExecuteMarginULChange( pPageULMarginItem->GetUpper(), nMB - ( nTmpPH - nPH ), pPageULMarginItem.get() );
            }
            else
            {
                ExecuteMarginULChange( nMT - ( nTmpPH - nPH ), pPageULMarginItem->GetLower(), pPageULMarginItem.get() );
            }
        }
    }

    if ( mxUndoManager.is() )
        mxUndoManager->leaveUndoContext();
}

void hideSidebar()
{
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell ? &pViewShell->GetViewFrame() : nullptr;
    if (pViewFrame)
        pViewFrame->SetChildWindow(SID_SIDEBAR, false , false );
    else
        SetLastExceptionMsg(u"No view shell or sidebar"_ustr);
}

}  // end anonymous namespace

// Could be anonymous in principle, but for the unit testing purposes, we
// declare it in init.hxx.
OUString desktop::extractParameter(OUString& rOptions, std::u16string_view rName)
{
    OUString aValue;

    OUString aNameEquals(OUString::Concat(rName) + "=");
    OUString aCommaNameEquals(OUString::Concat(",") + rName + "=");

    int nIndex = -1;
    if (rOptions.startsWith(aNameEquals))
    {
        size_t nLen = aNameEquals.getLength();
        int nComma = rOptions.indexOf(",", nLen);
        if (nComma >= 0)
        {
            aValue = rOptions.copy(nLen, nComma - nLen);
            rOptions = rOptions.copy(nComma + 1);
        }
        else
        {
            aValue = rOptions.copy(nLen);
            rOptions.clear();
        }
    }
    else if ((nIndex = rOptions.indexOf(aCommaNameEquals)) >= 0)
    {
        size_t nLen = aCommaNameEquals.getLength();
        int nComma = rOptions.indexOf(",", nIndex + nLen);
        if (nComma >= 0)
        {
            aValue = rOptions.copy(nIndex + nLen, nComma - nIndex - nLen);
            rOptions = OUString::Concat(rOptions.subView(0, nIndex)) + rOptions.subView(nComma);
        }
        else
        {
            aValue = rOptions.copy(nIndex + nLen);
            rOptions = rOptions.copy(0, nIndex);
        }
    }

    return aValue;
}

extern "C"
{

static void doc_destroy(LibreOfficeKitDocument* pThis);
static int doc_saveAs(LibreOfficeKitDocument* pThis, const char* pUrl, const char* pFormat, const char* pFilterOptions);
static int doc_getDocumentType(LibreOfficeKitDocument* pThis);
static int doc_getParts(LibreOfficeKitDocument* pThis);
static char* doc_getPartPageRectangles(LibreOfficeKitDocument* pThis);
static int doc_getPart(LibreOfficeKitDocument* pThis);
static void doc_setPart(LibreOfficeKitDocument* pThis, int nPart);
static void doc_selectPart(LibreOfficeKitDocument* pThis, int nPart, int nSelect);
static void doc_moveSelectedParts(LibreOfficeKitDocument* pThis, int nPosition, bool bDuplicate);
static char* doc_getPartName(LibreOfficeKitDocument* pThis, int nPart);
static void doc_setPartMode(LibreOfficeKitDocument* pThis, int nPartMode);
static int doc_getEditMode(LibreOfficeKitDocument* pThis);
static void doc_paintTile(LibreOfficeKitDocument* pThis,
                          unsigned char* pBuffer,
                          const int nCanvasWidth, const int nCanvasHeight,
                          const int nTilePosX, const int nTilePosY,
                          const int nTileWidth, const int nTileHeight);
static void doc_paintPartTile(LibreOfficeKitDocument* pThis,
                              unsigned char* pBuffer,
                              const int nPart,
                              const int nMode,
                              const int nCanvasWidth, const int nCanvasHeight,
                              const int nTilePosX, const int nTilePosY,
                              const int nTileWidth, const int nTileHeight);
static int doc_getTileMode(LibreOfficeKitDocument* pThis);
static void doc_getDocumentSize(LibreOfficeKitDocument* pThis,
                                long* pWidth,
                                long* pHeight);
static void doc_getDataArea(LibreOfficeKitDocument* pThis,
                            long nTab,
                            long* pCol,
                            long* pRow);
static void doc_initializeForRendering(LibreOfficeKitDocument* pThis,
                                       const char* pArguments);

static void doc_registerCallback(LibreOfficeKitDocument* pThis,
                                LibreOfficeKitCallback pCallback,
                                void* pData);
static void doc_postKeyEvent(LibreOfficeKitDocument* pThis,
                             int nType,
                             int nCharCode,
                             int nKeyCode);
static void doc_setBlockedCommandList(LibreOfficeKitDocument* pThis,
                                int nViewId,
                                const char* blockedCommandList);

static void doc_postWindowExtTextInputEvent(LibreOfficeKitDocument* pThis,
                                            unsigned nWindowId,
                                            int nType,
                                            const char* pText);
static void doc_removeTextContext(LibreOfficeKitDocument* pThis,
                                  unsigned nLOKWindowId,
                                  int nCharBefore,
                                  int nCharAfter);
static void doc_sendDialogEvent(LibreOfficeKitDocument* pThis,
                               unsigned long long int nLOKWindowId,
                               const char* pArguments);
static void doc_postWindowKeyEvent(LibreOfficeKitDocument* pThis,
                                   unsigned nLOKWindowId,
                                   int nType,
                                   int nCharCode,
                                   int nKeyCode);
static void doc_postMouseEvent (LibreOfficeKitDocument* pThis,
                                int nType,
                                int nX,
                                int nY,
                                int nCount,
                                int nButtons,
                                int nModifier);
static void doc_postWindowMouseEvent (LibreOfficeKitDocument* pThis,
                                      unsigned nLOKWindowId,
                                      int nType,
                                      int nX,
                                      int nY,
                                      int nCount,
                                      int nButtons,
                                      int nModifier);
static void doc_postWindowGestureEvent(LibreOfficeKitDocument* pThis,
                                      unsigned nLOKWindowId,
                                      const char* pType,
                                      int nX,
                                      int nY,
                                      int nOffset);
static void doc_postUnoCommand(LibreOfficeKitDocument* pThis,
                               const char* pCommand,
                               const char* pArguments,
                               bool bNotifyWhenFinished);
static void doc_setWindowTextSelection(LibreOfficeKitDocument* pThis,
                                       unsigned nLOKWindowId,
                                       bool swap,
                                       int nX,
                                       int nY);
static void doc_setTextSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static char* doc_getTextSelection(LibreOfficeKitDocument* pThis,
                                  const char* pMimeType,
                                  char** pUsedMimeType);
static int doc_getSelectionType(LibreOfficeKitDocument* pThis);
static int doc_getSelectionTypeAndText(LibreOfficeKitDocument* pThis,
                                       const char* pMimeType,
                                       char** pText,
                                       char** pUsedMimeType);
static int doc_getClipboard (LibreOfficeKitDocument* pThis,
                             const char **pMimeTypes,
                             size_t      *pOutCount,
                             char      ***pOutMimeTypes,
                             size_t     **pOutSizes,
                             char      ***pOutStreams);
static int doc_setClipboard (LibreOfficeKitDocument* pThis,
                             const size_t   nInCount,
                             const char   **pInMimeTypes,
                             const size_t  *pInSizes,
                             const char   **pInStreams);
static bool doc_paste(LibreOfficeKitDocument* pThis,
                      const char* pMimeType,
                      const char* pData,
                      size_t nSize);
static void doc_setGraphicSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static void doc_resetSelection (LibreOfficeKitDocument* pThis);
static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand);
static void doc_setClientZoom(LibreOfficeKitDocument* pThis,
                                    int nTilePixelWidth,
                                    int nTilePixelHeight,
                                    int nTileTwipWidth,
                                    int nTileTwipHeight);
static void doc_setClientVisibleArea(LibreOfficeKitDocument* pThis, int nX, int nY, int nWidth, int nHeight);
static void doc_setOutlineState(LibreOfficeKitDocument* pThis, bool bColumn, int nLevel, int nIndex, bool bHidden);
static int doc_createView(LibreOfficeKitDocument* pThis);
static int doc_createViewWithOptions(LibreOfficeKitDocument* pThis, const char* pOptions);
static void doc_destroyView(LibreOfficeKitDocument* pThis, int nId);
static void doc_setView(LibreOfficeKitDocument* pThis, int nId);
static int doc_getView(LibreOfficeKitDocument* pThis);
static int doc_getViewsCount(LibreOfficeKitDocument* pThis);
static bool doc_getViewIds(LibreOfficeKitDocument* pThis, int* pArray, size_t nSize);
static void doc_setViewLanguage(LibreOfficeKitDocument* pThis, int nId, const char* language);
static unsigned char* doc_renderFontOrientation(LibreOfficeKitDocument* pThis,
                          const char *pFontName,
                          const char *pChar,
                          int* pFontWidth,
                          int* pFontHeight,
                          int pOrientation);
static unsigned char* doc_renderFont(LibreOfficeKitDocument* pThis,
                          const char *pFontName,
                          const char *pChar,
                          int* pFontWidth,
                          int* pFontHeight);
static char* doc_getPartHash(LibreOfficeKitDocument* pThis, int nPart);

static void doc_paintWindow(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId, unsigned char* pBuffer,
                            const int nX, const int nY,
                            const int nWidth, const int nHeight);

static void doc_paintWindowDPI(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId, unsigned char* pBuffer,
                               const int nX, const int nY,
                               const int nWidth, const int nHeight,
                               const double fDPIScale);

static void doc_paintWindowForView(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId, unsigned char* pBuffer,
                                   const int nX, const int nY,
                                   const int nWidth, const int nHeight,
                                   const double fDPIScale, int viewId);

static void doc_postWindow(LibreOfficeKitDocument* pThis, unsigned
 nLOKWindowId, int nAction, const char* pData);

static char* doc_getPartInfo(LibreOfficeKitDocument* pThis, int nPart);

static bool doc_insertCertificate(LibreOfficeKitDocument* pThis,
                                  const unsigned char* pCertificateBinary,
                                  const int nCertificateBinarySize,
                                  const unsigned char* pPrivateKeyBinary,
                                  const int nPrivateKeyBinarySize);

static bool doc_addCertificate(LibreOfficeKitDocument* pThis,
                                 const unsigned char* pCertificateBinary,
                                 const int nCertificateBinarySize);

static int doc_getSignatureState(LibreOfficeKitDocument* pThis);

static size_t doc_renderShapeSelection(LibreOfficeKitDocument* pThis, char** pOutput);

static void doc_resizeWindow(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId,
                             const int nWidth, const int nHeight);

static void doc_completeFunction(LibreOfficeKitDocument* pThis, const char*);


static void doc_sendFormFieldEvent(LibreOfficeKitDocument* pThis,
                                   const char* pArguments);

static bool doc_renderSearchResult(LibreOfficeKitDocument* pThis,
                                 const char* pSearchResult, unsigned char** pBitmapBuffer,
                                 int* pWidth, int* pHeight, size_t* pByteSize);

static void doc_sendContentControlEvent(LibreOfficeKitDocument* pThis, const char* pArguments);

static void doc_setViewTimezone(LibreOfficeKitDocument* pThis, int nId, const char* timezone);

static void doc_setViewReadOnly(LibreOfficeKitDocument* pThis, int nId, const bool readonly);

static void doc_setAllowChangeComments(LibreOfficeKitDocument* pThis, int nId, const bool allow);

static void doc_setAccessibilityState(LibreOfficeKitDocument* pThis, int nId, bool bEnabled);

static char* doc_getA11yFocusedParagraph(LibreOfficeKitDocument* pThis);

static int doc_getA11yCaretPosition(LibreOfficeKitDocument* pThis);
} // extern "C"

namespace {
ITiledRenderable* getTiledRenderable(LibreOfficeKitDocument* pThis)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    return dynamic_cast<ITiledRenderable*>(pDocument->mxComponent.get());
}

#ifndef IOS

/*
 * Unfortunately clipboard creation using UNO is insanely baroque.
 * we also need to ensure that this works for the first view which
 * has no clear 'createView' called for it (unfortunately).
 */
rtl::Reference<LOKClipboard> forceSetClipboardForCurrentView(LibreOfficeKitDocument *pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    rtl::Reference<LOKClipboard> xClip(LOKClipboardFactory::getClipboardForCurView());

    SAL_INFO("lok", "Set to clipboard for view " << xClip.get());
    // FIXME: using a hammer here - should not be necessary if all tests used createView.
    pDoc->setClipboard(uno::Reference<datatransfer::clipboard::XClipboard>(xClip->getXI(), UNO_QUERY));

    return xClip;
}

#endif

const vcl::Font* FindFont(std::u16string_view rFontName)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if (!pDocSh)
        return nullptr;
    const SvxFontListItem* pFonts
        = static_cast<const SvxFontListItem*>(pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;
    if (pList && !rFontName.empty())
        if (sal_Handle hMetric = pList->GetFirstFontMetric(rFontName))
            return &FontList::GetFontMetric(hMetric);
    return nullptr;
}

vcl::Font FindFont_FallbackToDefault(std::u16string_view rFontName)
{
    if (auto pFound = FindFont(rFontName))
        return *pFound;

    return OutputDevice::GetDefaultFont(DefaultFontType::SANS_UNICODE, LANGUAGE_NONE,
                                        GetDefaultFontFlags::NONE);
}

int getDocumentType (LibreOfficeKitDocument* pThis)
{
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    try
    {
        uno::Reference<lang::XServiceInfo> xDocument(pDocument->mxComponent, uno::UNO_QUERY_THROW);

        if (xDocument->supportsService(u"com.sun.star.sheet.SpreadsheetDocument"_ustr))
        {
            return LOK_DOCTYPE_SPREADSHEET;
        }
        else if (xDocument->supportsService(u"com.sun.star.presentation.PresentationDocument"_ustr))
        {
            return LOK_DOCTYPE_PRESENTATION;
        }
        else if (xDocument->supportsService(u"com.sun.star.drawing.DrawingDocument"_ustr))
        {
            return LOK_DOCTYPE_DRAWING;
        }
        else if (xDocument->supportsService(u"com.sun.star.text.TextDocument"_ustr) || xDocument->supportsService(u"com.sun.star.text.WebDocument"_ustr))
        {
            return LOK_DOCTYPE_TEXT;
        }
        else
        {
            SetLastExceptionMsg(u"unknown document type"_ustr);
        }
    }
    catch (const uno::Exception& exception)
    {
        SetLastExceptionMsg("exception: " + exception.Message);
    }
    return LOK_DOCTYPE_OTHER;
}

} // anonymous namespace

LibLODocument_Impl::LibLODocument_Impl(uno::Reference <css::lang::XComponent> xComponent, int nDocumentId)
    : mxComponent(std::move(xComponent))
    , mnDocumentId(nDocumentId)
{
    assert(nDocumentId != -1 && "Cannot set mnDocumentId to -1");

    m_pDocumentClass = gDocumentClass.lock();
    if (!m_pDocumentClass)
    {
        m_pDocumentClass = std::make_shared<LibreOfficeKitDocumentClass>();

        m_pDocumentClass->nSize = sizeof(LibreOfficeKitDocumentClass);

        m_pDocumentClass->destroy = doc_destroy;
        m_pDocumentClass->saveAs = doc_saveAs;
        m_pDocumentClass->getDocumentType = doc_getDocumentType;
        m_pDocumentClass->getParts = doc_getParts;
        m_pDocumentClass->getPartPageRectangles = doc_getPartPageRectangles;
        m_pDocumentClass->getPart = doc_getPart;
        m_pDocumentClass->setPart = doc_setPart;
        m_pDocumentClass->selectPart = doc_selectPart;
        m_pDocumentClass->moveSelectedParts = doc_moveSelectedParts;
        m_pDocumentClass->getPartName = doc_getPartName;
        m_pDocumentClass->setPartMode = doc_setPartMode;
        m_pDocumentClass->getEditMode = doc_getEditMode;
        m_pDocumentClass->paintTile = doc_paintTile;
        m_pDocumentClass->paintPartTile = doc_paintPartTile;
        m_pDocumentClass->getTileMode = doc_getTileMode;
        m_pDocumentClass->getDocumentSize = doc_getDocumentSize;
        m_pDocumentClass->getDataArea = doc_getDataArea;
        m_pDocumentClass->initializeForRendering = doc_initializeForRendering;
        m_pDocumentClass->registerCallback = doc_registerCallback;
        m_pDocumentClass->postKeyEvent = doc_postKeyEvent;
        m_pDocumentClass->postWindowExtTextInputEvent = doc_postWindowExtTextInputEvent;
        m_pDocumentClass->removeTextContext = doc_removeTextContext;
        m_pDocumentClass->postWindowKeyEvent = doc_postWindowKeyEvent;
        m_pDocumentClass->postMouseEvent = doc_postMouseEvent;
        m_pDocumentClass->postWindowMouseEvent = doc_postWindowMouseEvent;
        m_pDocumentClass->sendDialogEvent = doc_sendDialogEvent;
        m_pDocumentClass->postUnoCommand = doc_postUnoCommand;
        m_pDocumentClass->setTextSelection = doc_setTextSelection;
        m_pDocumentClass->setWindowTextSelection = doc_setWindowTextSelection;
        m_pDocumentClass->getTextSelection = doc_getTextSelection;
        m_pDocumentClass->getSelectionType = doc_getSelectionType;
        m_pDocumentClass->getSelectionTypeAndText = doc_getSelectionTypeAndText;
        m_pDocumentClass->getClipboard = doc_getClipboard;
        m_pDocumentClass->setClipboard = doc_setClipboard;
        m_pDocumentClass->paste = doc_paste;
        m_pDocumentClass->setGraphicSelection = doc_setGraphicSelection;
        m_pDocumentClass->resetSelection = doc_resetSelection;
        m_pDocumentClass->getCommandValues = doc_getCommandValues;
        m_pDocumentClass->setClientZoom = doc_setClientZoom;
        m_pDocumentClass->setClientVisibleArea = doc_setClientVisibleArea;
        m_pDocumentClass->setOutlineState = doc_setOutlineState;

        m_pDocumentClass->createView = doc_createView;
        m_pDocumentClass->destroyView = doc_destroyView;
        m_pDocumentClass->setView = doc_setView;
        m_pDocumentClass->getView = doc_getView;
        m_pDocumentClass->getViewsCount = doc_getViewsCount;
        m_pDocumentClass->getViewIds = doc_getViewIds;

        m_pDocumentClass->renderFont = doc_renderFont;
        m_pDocumentClass->renderFontOrientation = doc_renderFontOrientation;
        m_pDocumentClass->getPartHash = doc_getPartHash;

        m_pDocumentClass->paintWindow = doc_paintWindow;
        m_pDocumentClass->paintWindowDPI = doc_paintWindowDPI;
        m_pDocumentClass->paintWindowForView = doc_paintWindowForView;
        m_pDocumentClass->postWindow = doc_postWindow;
        m_pDocumentClass->resizeWindow = doc_resizeWindow;

        m_pDocumentClass->setViewLanguage = doc_setViewLanguage;

        m_pDocumentClass->getPartInfo = doc_getPartInfo;

        m_pDocumentClass->insertCertificate = doc_insertCertificate;
        m_pDocumentClass->addCertificate = doc_addCertificate;
        m_pDocumentClass->getSignatureState = doc_getSignatureState;

        m_pDocumentClass->renderShapeSelection = doc_renderShapeSelection;
        m_pDocumentClass->postWindowGestureEvent = doc_postWindowGestureEvent;

        m_pDocumentClass->createViewWithOptions = doc_createViewWithOptions;
        m_pDocumentClass->completeFunction = doc_completeFunction;

        m_pDocumentClass->sendFormFieldEvent = doc_sendFormFieldEvent;
        m_pDocumentClass->renderSearchResult = doc_renderSearchResult;

        m_pDocumentClass->setBlockedCommandList = doc_setBlockedCommandList;

        m_pDocumentClass->sendContentControlEvent = doc_sendContentControlEvent;

        m_pDocumentClass->setViewTimezone = doc_setViewTimezone;

        m_pDocumentClass->setAccessibilityState = doc_setAccessibilityState;

        m_pDocumentClass->getA11yFocusedParagraph = doc_getA11yFocusedParagraph;
        m_pDocumentClass->getA11yCaretPosition = doc_getA11yCaretPosition;

        m_pDocumentClass->setViewReadOnly = doc_setViewReadOnly;

        m_pDocumentClass->setAllowChangeComments = doc_setAllowChangeComments;

        gDocumentClass = m_pDocumentClass;
    }
    pClass = m_pDocumentClass.get();

#ifndef IOS
    forceSetClipboardForCurrentView(this);
#endif
}

LibLODocument_Impl::~LibLODocument_Impl()
{
    try
    {
        mxComponent->dispose();
    }
    catch (const css::lang::DisposedException&)
    {
        TOOLS_WARN_EXCEPTION("lok", "failed to dispose document");
    }
}

static OUString getGenerator()
{
    OUString sGenerator(
        Translate::ExpandVariables(u"%PRODUCTNAME %PRODUCTVERSION%PRODUCTEXTENSION (%1)"_ustr));
    OUString os(u"$_OS"_ustr);
    ::rtl::Bootstrap::expandMacros(os);
    return sGenerator.replaceFirst("%1", os);
}

extern "C" {

// One of these is created per view to handle events cf. doc_registerCallback
CallbackFlushHandler::CallbackFlushHandler(LibreOfficeKitDocument* pDocument, LibreOfficeKitCallback pCallback, void* pData)
    : m_pDocument(pDocument),
      m_pCallback(pCallback),
      m_pFlushEvent(nullptr),
      m_pData(pData),
      m_nDisableCallbacks(0)
{
    // Add the states that are safe to skip duplicates on, even when
    // not consequent (i.e. do no emit them if unchanged from last).
    m_states.emplace(LOK_CALLBACK_TEXT_SELECTION, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_GRAPHIC_SELECTION, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_STATE_CHANGED, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_MOUSE_POINTER, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_CELL_CURSOR, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_CELL_FORMULA, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_CELL_ADDRESS, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_CURSOR_VISIBLE, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_SET_PART, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_TABLE_SELECTED, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_TAB_STOP_LIST, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_RULER_UPDATE, "NIL"_ostr);
    m_states.emplace(LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE, "NIL"_ostr);
}

void CallbackFlushHandler::stop()
{
    if (m_pFlushEvent)
    {
        Application::RemoveUserEvent(m_pFlushEvent);
        m_pFlushEvent = nullptr;
    }
}

CallbackFlushHandler::~CallbackFlushHandler()
{
    stop();
}

CallbackFlushHandler::queue_type2::iterator CallbackFlushHandler::toQueue2(CallbackFlushHandler::queue_type1::iterator pos)
{
    int delta = std::distance(m_queue1.begin(), pos);
    return m_queue2.begin() + delta;
}

CallbackFlushHandler::queue_type2::reverse_iterator CallbackFlushHandler::toQueue2(CallbackFlushHandler::queue_type1::reverse_iterator pos)
{
    int delta = std::distance(m_queue1.rbegin(), pos);
    return m_queue2.rbegin() + delta;
}

void CallbackFlushHandler::setUpdatedType( int nType, bool value )
{
    assert(isUpdatedType(nType));
    if( m_updatedTypes.size() <= o3tl::make_unsigned( nType ))
        m_updatedTypes.resize( nType + 1 ); // new are default-constructed, i.e. false
    m_updatedTypes[ nType ] = value;
    if(value)
        scheduleFlush();
}

void CallbackFlushHandler::resetUpdatedType( int nType )
{
    setUpdatedType( nType, false );
}

void CallbackFlushHandler::setUpdatedTypePerViewId( int nType, int nViewId, int nSourceViewId, bool value )
{
    assert(isUpdatedTypePerViewId(nType));
    std::vector<PerViewIdData>& types = m_updatedTypesPerViewId[ nViewId ];
    if( types.size() <= o3tl::make_unsigned( nType ))
        types.resize( nType + 1 ); // new are default-constructed, i.e. 'set' is false
    types[ nType ] = PerViewIdData{ value, nSourceViewId };
    if(value)
        scheduleFlush();
}

void CallbackFlushHandler::resetUpdatedTypePerViewId( int nType, int nViewId )
{
    assert(isUpdatedTypePerViewId(nType));
    bool allViewIds = false;
    // Handle specially messages that do not have viewId for backwards compatibility.
    if( nType == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR && !comphelper::LibreOfficeKit::isViewIdForVisCursorInvalidation())
        allViewIds = true;
    if( !allViewIds )
    {
        setUpdatedTypePerViewId( nType, nViewId, -1, false );
        return;
    }
    for( auto& it : m_updatedTypesPerViewId )
    {
        std::vector<PerViewIdData>& types = it.second;
        if( types.size() >= o3tl::make_unsigned( nType ))
            types[ nType ].set = false;
    }
}

void CallbackFlushHandler::libreOfficeKitViewCallback(int nType, const OString& pPayload)
{
    CallbackData callbackData(pPayload);
    queue(nType, callbackData);
}

void CallbackFlushHandler::libreOfficeKitViewCallbackWithViewId(int nType, const OString& pPayload, int nViewId)
{
    CallbackData callbackData(pPayload, nViewId);
    queue(nType, callbackData);
}

void CallbackFlushHandler::libreOfficeKitViewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart, int nMode)
{
    CallbackData callbackData(pRect, nPart, nMode);
    queue(LOK_CALLBACK_INVALIDATE_TILES, callbackData);
}

void CallbackFlushHandler::libreOfficeKitViewUpdatedCallback(int nType)
{
    assert(isUpdatedType( nType ));
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    SAL_INFO("lok", "Updated: [" << nType << "]");
    setUpdatedType(nType, true);
}

void CallbackFlushHandler::libreOfficeKitViewUpdatedCallbackPerViewId(int nType, int nViewId, int nSourceViewId)
{
    assert(isUpdatedTypePerViewId( nType ));
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    SAL_INFO("lok", "Updated: [" << nType << "]");
    setUpdatedTypePerViewId(nType, nViewId, nSourceViewId, true);
}

void CallbackFlushHandler::dumpState(rtl::OStringBuffer &rState)
{
    // NB. no locking
    rState.append("\nView:\t");
    rState.append(static_cast<sal_Int32>(m_viewId));
    rState.append("\n\tDisableCallbacks:\t");
    rState.append(static_cast<sal_Int32>(m_nDisableCallbacks));
    rState.append("\n\tStates:\n");
    for (const auto &i : m_states)
    {
        rState.append("\n\t\t");
        rState.append(static_cast<sal_Int32>(i.first));
        rState.append("\t");
        rState.append(i.second);
    }
}

void CallbackFlushHandler::libreOfficeKitViewAddPendingInvalidateTiles()
{
    // Invoke() will call flushPendingLOKInvalidateTiles(), so just make sure the timer is active.
    scheduleFlush();
}

void CallbackFlushHandler::queue(const int type, const OString& data)
{
    CallbackData callbackData(data);
    queue(type, callbackData);
}

void CallbackFlushHandler::queue(const int type, CallbackData& aCallbackData)
{
    comphelper::ProfileZone aZone("CallbackFlushHandler::queue");

    SAL_INFO("lok", "Queue: [" << type << "]: [" << aCallbackData.getPayload() << "] on " << m_queue1.size() << " entries.");

    bool bIsChartActive = false;
    bool bIsComment = false;
    if (type == LOK_CALLBACK_GRAPHIC_SELECTION)
    {
        LokChartHelper aChartHelper(SfxViewShell::Current());
        bIsChartActive = aChartHelper.GetWindow() != nullptr;
    }
    else if (type == LOK_CALLBACK_COMMENT)
    {
        bIsComment = true;
    }

    if (callbacksDisabled() && !bIsChartActive && !bIsComment)
    {
        // We drop notifications when this is set, except for important ones.
        // When we issue a complex command (such as .uno:InsertAnnotation)
        // there will be multiple notifications. On the first invalidation
        // we will start painting, but other events will get fired
        // while the complex command in question executes.
        // We don't want to suppress everything here on the wrong assumption
        // that no new events are fired during painting.
        if (type != LOK_CALLBACK_STATE_CHANGED &&
            type != LOK_CALLBACK_INVALIDATE_TILES &&
            type != LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR &&
            type != LOK_CALLBACK_CURSOR_VISIBLE &&
            type != LOK_CALLBACK_VIEW_CURSOR_VISIBLE &&
            type != LOK_CALLBACK_TEXT_SELECTION &&
            type != LOK_CALLBACK_TEXT_SELECTION_START &&
            type != LOK_CALLBACK_TEXT_SELECTION_END &&
            type != LOK_CALLBACK_MEDIA_SHAPE &&
            type != LOK_CALLBACK_REFERENCE_MARKS)
        {
            SAL_INFO("lok", "Skipping while painting [" << type << "]: [" << aCallbackData.getPayload() << "].");
            return;
        }

        // In Writer we drop all notifications during painting.
        if (doc_getDocumentType(m_pDocument) == LOK_DOCTYPE_TEXT)
            return;
    }

    // Suppress invalid payloads.
    if (type == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR &&
        aCallbackData.getPayload().indexOf(", 0, 0, ") != -1 &&
        aCallbackData.getPayload().indexOf("\"hyperlink\":\"\"") == -1 &&
        aCallbackData.getPayload().indexOf("\"hyperlink\": {}") == -1)
    {
        // The cursor position is often the relative coordinates of the widget
        // issuing it, instead of the absolute one that we expect.
        // This is temporary however, and, once the control is created and initialized
        // correctly, it eventually emits the correct absolute coordinates.
        SAL_INFO("lok", "Skipping invalid event [" << type << "]: [" << aCallbackData.getPayload() << "].");
        return;
    }

    std::unique_lock<std::recursive_mutex> lock(m_mutex);

    // Update types should be received via the updated callbacks for performance,
    // getting them as normal callbacks is technically not wrong, but probably should be avoided.
    // Reset the updated flag if we get a normal message.
    if(isUpdatedType(type))
    {
        SAL_INFO("lok", "Received event with updated type [" << type << "] as normal callback");
        resetUpdatedType(type);
    }
    if(isUpdatedTypePerViewId(type))
    {
        SAL_INFO("lok", "Received event with updated type [" << type << "] as normal callback");
        resetUpdatedTypePerViewId(type, aCallbackData.getViewId());
    }

    // drop duplicate callbacks for the listed types
    switch (type)
    {
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
        case LOK_CALLBACK_STATE_CHANGED:
        case LOK_CALLBACK_MOUSE_POINTER:
        case LOK_CALLBACK_CELL_CURSOR:
        case LOK_CALLBACK_CELL_VIEW_CURSOR:
        case LOK_CALLBACK_CELL_FORMULA:
        case LOK_CALLBACK_CELL_ADDRESS:
        case LOK_CALLBACK_CELL_SELECTION_AREA:
        case LOK_CALLBACK_CURSOR_VISIBLE:
        case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        case LOK_CALLBACK_SET_PART:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        case LOK_CALLBACK_INVALIDATE_HEADER:
        case LOK_CALLBACK_WINDOW:
        case LOK_CALLBACK_CALC_FUNCTION_LIST:
        case LOK_CALLBACK_INVALIDATE_SHEET_GEOMETRY:
        case LOK_CALLBACK_REFERENCE_MARKS:
        case LOK_CALLBACK_CELL_AUTO_FILL_AREA:
        case LOK_CALLBACK_A11Y_FOCUS_CHANGED:
        case LOK_CALLBACK_A11Y_CARET_CHANGED:
        case LOK_CALLBACK_A11Y_TEXT_SELECTION_CHANGED:
        case LOK_CALLBACK_A11Y_FOCUSED_CELL_CHANGED:
        case LOK_CALLBACK_COLOR_PALETTES:
        case LOK_CALLBACK_A11Y_EDITING_IN_SELECTION_STATE:
        case LOK_CALLBACK_A11Y_SELECTION_CHANGED:
        {
            const auto& pos = std::find(m_queue1.rbegin(), m_queue1.rend(), type);
            auto pos2 = toQueue2(pos);
            if (pos != m_queue1.rend() && pos2->getPayload() == aCallbackData.getPayload())
            {
                SAL_INFO("lok", "Skipping queue duplicate [" << type << + "]: [" << aCallbackData.getPayload() << "].");
                return;
            }
        }
        break;
    }

    if (type == LOK_CALLBACK_TEXT_SELECTION && aCallbackData.isEmpty())
    {
        const auto& posStart = std::find(m_queue1.rbegin(), m_queue1.rend(), LOK_CALLBACK_TEXT_SELECTION_START);
        auto posStart2 = toQueue2(posStart);
        if (posStart != m_queue1.rend())
            posStart2->clear();

        const auto& posEnd = std::find(m_queue1.rbegin(), m_queue1.rend(), LOK_CALLBACK_TEXT_SELECTION_END);
        auto posEnd2 = toQueue2(posEnd);
        if (posEnd != m_queue1.rend())
            posEnd2->clear();
    }

    // When payload is empty discards any previous state.
    if (aCallbackData.isEmpty())
    {
        switch (type)
        {
            case LOK_CALLBACK_TEXT_SELECTION_START:
            case LOK_CALLBACK_TEXT_SELECTION_END:
            case LOK_CALLBACK_TEXT_SELECTION:
            case LOK_CALLBACK_GRAPHIC_SELECTION:
            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            case LOK_CALLBACK_INVALIDATE_TILES:
            case LOK_CALLBACK_TOOLTIP:
                if (removeAll(type))
                    SAL_INFO("lok", "Removed dups of [" << type << "]: [" << aCallbackData.getPayload() << "].");
                break;
        }
    }
    else
    {
        switch (type)
        {
            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones.
            case LOK_CALLBACK_TEXT_SELECTION_START:
            case LOK_CALLBACK_TEXT_SELECTION_END:
            case LOK_CALLBACK_TEXT_SELECTION:
            case LOK_CALLBACK_MOUSE_POINTER:
            case LOK_CALLBACK_CELL_CURSOR:
            case LOK_CALLBACK_CELL_FORMULA:
            case LOK_CALLBACK_CELL_ADDRESS:
            case LOK_CALLBACK_CURSOR_VISIBLE:
            case LOK_CALLBACK_SET_PART:
            case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
            case LOK_CALLBACK_RULER_UPDATE:
            case LOK_CALLBACK_A11Y_FOCUS_CHANGED:
            case LOK_CALLBACK_A11Y_CARET_CHANGED:
            case LOK_CALLBACK_A11Y_TEXT_SELECTION_CHANGED:
            case LOK_CALLBACK_A11Y_FOCUSED_CELL_CHANGED:
            case LOK_CALLBACK_COLOR_PALETTES:
            case LOK_CALLBACK_TOOLTIP:
            case LOK_CALLBACK_SHAPE_INNER_TEXT:
            {
                if (removeAll(type))
                    SAL_INFO("lok", "Removed dups of [" << type << "]: [" << aCallbackData.getPayload() << "].");
            }
            break;

            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones,
            // but only if the view is the same.
            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
                // deleting the duplicate of visible cursor message can cause hyperlink popup not to show up on second/or more click on the same place.
                // If the hyperlink is not empty we can bypass that to show the popup
                if (aCallbackData.getPayload().indexOf("\"hyperlink\":\"\"") == -1
                    && aCallbackData.getPayload().indexOf("\"hyperlink\": {}") == -1)
                    break;
                [[fallthrough]];
            case LOK_CALLBACK_CELL_VIEW_CURSOR:
            case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
            case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
            case LOK_CALLBACK_CALC_FUNCTION_LIST:
            case LOK_CALLBACK_FORM_FIELD_BUTTON:
            {
                const int nViewId = aCallbackData.getViewId();
                removeAll(type, [nViewId] (const CallbackData& elemData) {
                        return (nViewId == elemData.getViewId());
                    }
                );
            }
            break;

            case LOK_CALLBACK_INVALIDATE_TILES:
                if (processInvalidateTilesEvent(type, aCallbackData))
                    return;
            break;

            // State changes with same name override previous ones with a different value.
            // Ex. ".uno:PageStatus=Slide 20 of 83" overwrites any previous PageStatus.
            case LOK_CALLBACK_STATE_CHANGED:
            {
                // Compare the state name=value and overwrite earlier entries with same name.
                const auto pos = aCallbackData.getPayload().indexOf('=');
                if (pos != -1)
                {
                    const std::string_view name = aCallbackData.getPayload().subView(0, pos + 1);
                    // This is needed because otherwise it creates some problems when
                    // a save occurs while a cell is still edited in Calc.
                    if (name != ".uno:ModifiedStatus=")
                    {
                        removeAll(type, [&name] (const CallbackData& elemData) {
                                return elemData.getPayload().startsWith(name);
                            }
                        );
                    }
                }
            }
            break;

            case LOK_CALLBACK_WINDOW:
                if (processWindowEvent(type, aCallbackData))
                    return;
            break;

            case LOK_CALLBACK_GRAPHIC_SELECTION:
            {
                // remove only selection ranges and 'EMPTY' messages
                // always send 'INPLACE' and 'INPLACE EXIT' messages
                removeAll(type, [] (const CallbackData& elemData)
                    { return (elemData.getPayload().indexOf("INPLACE") == -1); });
            }
            break;
        }
    }

    // Validate that the cached data and the payload string are identical.
    assert(aCallbackData.validate() && "Cached callback payload object and string mismatch!");
    m_queue1.emplace_back(type);
    m_queue2.emplace_back(aCallbackData);
    SAL_INFO("lok", "Queued #" << (m_queue1.size() - 1) <<
             " [" << type << "]: [" << aCallbackData.getPayload() << "] to have " << m_queue1.size() << " entries.");

#ifdef DBG_UTIL
    {
        // Dump the queue state and validate cached data.
        int i = 1;
        std::ostringstream oss;
        if (m_queue1.empty())
            oss << "Empty";
        else
            oss << m_queue1.size() << " items\n";
        auto it1 = m_queue1.begin();
        auto it2 = m_queue2.begin();
        for (; it1 != m_queue1.end(); ++it1, ++it2)
            oss << i++ << ": [" << *it1 << "] [" << it2->getPayload() << "].\n";
        SAL_INFO("lok", "Current Queue: " << oss.str());
        assert(
            std::all_of(
                m_queue2.begin(), m_queue2.end(),
                [](const CallbackData& c) { return c.validate(); }));
    }
#endif

    lock.unlock();
    scheduleFlush();
}

bool CallbackFlushHandler::processInvalidateTilesEvent(int type, CallbackData& aCallbackData)
{
    RectangleAndPart rcNew = aCallbackData.getRectangleAndPart();
    if (rcNew.isEmpty())
    {
        SAL_INFO("lok", "Skipping invalid event [" << type << "]: [" << aCallbackData.getPayload() << "].");
        return true;
    }

    // If we have to invalidate all tiles, we can skip any new tile invalidation.
    // Find the last INVALIDATE_TILES entry, if any to see if it's invalidate-all.
    const auto& pos
        = std::find(m_queue1.rbegin(), m_queue1.rend(), LOK_CALLBACK_INVALIDATE_TILES);
    if (pos != m_queue1.rend())
    {
        auto pos2 = toQueue2(pos);
        const RectangleAndPart& rcOld = pos2->getRectangleAndPart();
        if (rcOld.isInfinite() && (rcOld.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart) &&
            (rcOld.m_nMode == rcNew.m_nMode))
        {
            SAL_INFO("lok", "Skipping queue [" << type << "]: [" << aCallbackData.getPayload()
                                               << "] since all tiles need to be invalidated.");
            return true;
        }

        if ((rcOld.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart) && (rcOld.m_nMode == rcNew.m_nMode))
        {
            // If fully overlapping.
            if (rcOld.m_aRectangle.Contains(rcNew.m_aRectangle))
            {
                SAL_INFO("lok", "Skipping queue [" << type << "]: [" << aCallbackData.getPayload()
                                                   << "] since overlaps existing all-parts.");
                return true;
            }
        }
    }

    if (rcNew.isInfinite())
    {
        SAL_INFO("lok", "Have Empty [" << type << "]: [" << aCallbackData.getPayload()
                                       << "] so removing all with part " << rcNew.m_nPart << ".");
        removeAll(LOK_CALLBACK_INVALIDATE_TILES, [&rcNew](const CallbackData& elemData) {
            // Remove exiting if new is all-encompassing, or if of the same part.
            return ((rcNew.m_nPart == -1 || rcNew.m_nPart == elemData.getRectangleAndPart().m_nPart)
                && (rcNew.m_nMode == elemData.getRectangleAndPart().m_nMode));
        });
    }
    else
    {
        const auto rcOrig = rcNew;

        SAL_INFO("lok", "Have [" << type << "]: [" << aCallbackData.getPayload() << "] so merging overlapping.");
        removeAll(LOK_CALLBACK_INVALIDATE_TILES,[&rcNew](const CallbackData& elemData) {
            const RectangleAndPart& rcOld = elemData.getRectangleAndPart();
            if (rcNew.m_nPart != -1 && rcOld.m_nPart != -1 &&
                (rcOld.m_nPart != rcNew.m_nPart || rcOld.m_nMode != rcNew.m_nMode))
            {
                SAL_INFO("lok", "Nothing to merge between new: "
                                    << rcNew.toString() << ", and old: " << rcOld.toString());
                return false;
            }

            if (rcNew.m_nPart == -1)
            {
                // Don't merge unless fully overlapped.
                SAL_INFO("lok", "New " << rcNew.toString() << " has " << rcOld.toString()
                                       << "?");
                if (rcNew.m_aRectangle.Contains(rcOld.m_aRectangle) && rcOld.m_nMode == rcNew.m_nMode)
                {
                    SAL_INFO("lok", "New " << rcNew.toString() << " engulfs old "
                                           << rcOld.toString() << ".");
                    return true;
                }
            }
            else if (rcOld.m_nPart == -1)
            {
                // Don't merge unless fully overlapped.
                SAL_INFO("lok", "Old " << rcOld.toString() << " has " << rcNew.toString()
                                       << "?");
                if (rcOld.m_aRectangle.Contains(rcNew.m_aRectangle) && rcOld.m_nMode == rcNew.m_nMode)
                {
                    SAL_INFO("lok", "New " << rcNew.toString() << " engulfs old "
                                           << rcOld.toString() << ".");
                    return true;
                }
            }
            else
            {
                const tools::Rectangle rcOverlap
                    = rcNew.m_aRectangle.GetIntersection(rcOld.m_aRectangle);
                const bool bOverlap = !rcOverlap.IsEmpty() && rcOld.m_nMode == rcNew.m_nMode;
                SAL_INFO("lok", "Merging " << rcNew.toString() << " & " << rcOld.toString()
                                           << " => " << rcOverlap.toString()
                                           << " Overlap: " << bOverlap);
                if (bOverlap)
                {
                    rcNew.m_aRectangle.Union(rcOld.m_aRectangle);
                    SAL_INFO("lok", "Merged: " << rcNew.toString());
                    return true;
                }
            }

            // Keep others.
            return false;
        });

        if (rcNew.m_aRectangle != rcOrig.m_aRectangle)
        {
            SAL_INFO("lok", "Replacing: " << rcOrig.toString() << " by " << rcNew.toString());
            if (rcNew.m_aRectangle.GetWidth() < rcOrig.m_aRectangle.GetWidth()
                || rcNew.m_aRectangle.GetHeight() < rcOrig.m_aRectangle.GetHeight())
            {
                SAL_WARN("lok", "Error: merged rect smaller.");
            }
        }
    }

    aCallbackData.updateRectangleAndPart(rcNew);
    // Queue this one.
    return false;
}

bool CallbackFlushHandler::processWindowEvent(int type, CallbackData& aCallbackData)
{
    const OString& payload = aCallbackData.getPayload();

    boost::property_tree::ptree& aTree = aCallbackData.setJson(std::string(payload));
    const unsigned nLOKWindowId = aTree.get<unsigned>("id", 0);
    const std::string aAction = aTree.get<std::string>("action", "");
    if (aAction == "invalidate")
    {
        std::string aRectStr = aTree.get<std::string>("rectangle", "");
        // no 'rectangle' field => invalidate all of the window =>
        // remove all previous window part invalidations
        if (aRectStr.empty())
        {
            removeAll(LOK_CALLBACK_WINDOW,[&nLOKWindowId](const CallbackData& elemData) {
                const boost::property_tree::ptree& aOldTree = elemData.getJson();
                if (nLOKWindowId == aOldTree.get<unsigned>("id", 0)
                    && aOldTree.get<std::string>("action", "") == "invalidate")
                {
                    return true;
                }
                return false;
            });
        }
        else
        {
            // if we have to invalidate all of the window, ignore
            // any part invalidation message
            bool invAllExist = false;
            auto it1 = m_queue1.rbegin();
            auto it2 = m_queue2.rbegin();
            for (;it1 != m_queue1.rend(); ++it1, ++it2)
            {
                if (*it1 != LOK_CALLBACK_WINDOW)
                    continue;
                const boost::property_tree::ptree& aOldTree = it2->getJson();
                if (nLOKWindowId == aOldTree.get<unsigned>("id", 0)
                     && aOldTree.get<std::string>("action", "") == "invalidate"
                     && aOldTree.get<std::string>("rectangle", "").empty())
                {
                    invAllExist = true;
                    break;
                }
            }

            // we found a invalidate-all window callback
            if (invAllExist)
            {
                SAL_INFO("lok.dialog", "Skipping queue ["
                                           << type << "]: [" << payload
                                           << "] since whole window needs to be invalidated.");
                return true;
            }

            std::istringstream aRectStream(aRectStr);
            tools::Long nLeft, nTop, nWidth, nHeight;
            char nComma;
            aRectStream >> nLeft >> nComma >> nTop >> nComma >> nWidth >> nComma >> nHeight;
            tools::Rectangle aNewRect(nLeft, nTop, nLeft + nWidth, nTop + nHeight);
            bool currentIsRedundant = false;
            removeAll(LOK_CALLBACK_WINDOW, [&aNewRect, &nLOKWindowId,
                       &currentIsRedundant](const CallbackData& elemData) {
                const boost::property_tree::ptree& aOldTree = elemData.getJson();
                if (aOldTree.get<std::string>("action", "") == "invalidate")
                {
                    // Not possible that we encounter an empty rectangle here; we already handled this case above.
                    std::istringstream aOldRectStream(aOldTree.get<std::string>("rectangle", ""));
                    tools::Long nOldLeft, nOldTop, nOldWidth, nOldHeight;
                    char nOldComma;
                    aOldRectStream >> nOldLeft >> nOldComma >> nOldTop >> nOldComma >> nOldWidth
                        >> nOldComma >> nOldHeight;
                    const tools::Rectangle aOldRect = tools::Rectangle(
                        nOldLeft, nOldTop, nOldLeft + nOldWidth, nOldTop + nOldHeight);

                    if (nLOKWindowId == aOldTree.get<unsigned>("id", 0))
                    {
                        if (aNewRect == aOldRect)
                        {
                            SAL_INFO("lok.dialog", "Duplicate rect [" << aNewRect.toString()
                                                                      << "]. Skipping new.");
                            // We have a rectangle in the queue already that makes the current Callback useless.
                            currentIsRedundant = true;
                            return false;
                        }
                        // new one engulfs the old one?
                        else if (aNewRect.Contains(aOldRect))
                        {
                            SAL_INFO("lok.dialog",
                                     "New rect [" << aNewRect.toString() << "] engulfs old ["
                                                  << aOldRect.toString() << "]. Replacing old.");
                            return true;
                        }
                        // old one engulfs the new one?
                        else if (aOldRect.Contains(aNewRect))
                        {
                            SAL_INFO("lok.dialog",
                                     "Old rect [" << aOldRect.toString() << "] engulfs new ["
                                                  << aNewRect.toString() << "]. Skipping new.");
                            // We have a rectangle in the queue already that makes the current Callback useless.
                            currentIsRedundant = true;
                            return false;
                        }
                        else
                        {
                            // Overlapping rects.
                            const tools::Rectangle aPreMergeRect = aNewRect;
                            aNewRect.Union(aOldRect);
                            SAL_INFO("lok.dialog", "Merging rects ["
                                                       << aPreMergeRect.toString() << "] & ["
                                                       << aOldRect.toString() << "] = ["
                                                       << aNewRect.toString()
                                                       << "]. Replacing old.");
                            return true;
                        }
                    }
                }

                // keep rest
                return false;
            });

            // Do not enqueue if redundant.
            if (currentIsRedundant)
                return true;

            aTree.put("rectangle", aNewRect.toString().getStr());
            aCallbackData.setJson(aTree);
            assert(aCallbackData.validate() && "Validation after setJson failed!");
        }
    }
    else if (aAction == "created")
    {
        // Remove all previous actions on same dialog, if we are creating it anew.
        removeAll(LOK_CALLBACK_WINDOW,[&nLOKWindowId](const CallbackData& elemData) {
            const boost::property_tree::ptree& aOldTree = elemData.getJson();
            if (nLOKWindowId == aOldTree.get<unsigned>("id", 0))
                return true;
            return false;
        });

        VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
        if (!pWindow)
        {
            SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
            return false;
        }

#ifndef IOS
        auto xClip = forceSetClipboardForCurrentView(m_pDocument);

        uno::Reference<datatransfer::clipboard::XClipboard> xClipboard(xClip);
        pWindow->SetClipboard(xClipboard);
#endif
    }
    else if (aAction == "size_changed")
    {
        // A size change is practically re-creation of the window.
        // But at a minimum it's a full invalidation.
        removeAll(LOK_CALLBACK_WINDOW, [&nLOKWindowId](const CallbackData& elemData) {
            const boost::property_tree::ptree& aOldTree = elemData.getJson();
            if (nLOKWindowId == aOldTree.get<unsigned>("id", 0))
            {
                const std::string aOldAction = aOldTree.get<std::string>("action", "");
                if (aOldAction == "invalidate")
                    return true;
            }
            return false;
        });
    }

    // Queue this one.
    return false;
}

void CallbackFlushHandler::enqueueUpdatedTypes()
{
    if( m_updatedTypes.empty() && m_updatedTypesPerViewId.empty())
        return;
    assert(m_viewId >= 0);
    SfxViewShell* viewShell = SfxViewShell::GetFirst( false,
        [this](const SfxViewShell& shell) { return shell.GetViewShellId().get() == m_viewId; } );
    assert(viewShell != nullptr);

    // First move data to local structures, so that callbacks don't possibly modify it.
    std::vector<bool> updatedTypes;
    std::swap(updatedTypes, m_updatedTypes);
    boost::container::flat_map<int, std::vector<PerViewIdData>> updatedTypesPerViewId;
    std::swap(updatedTypesPerViewId, m_updatedTypesPerViewId);

    // Some types must always precede other types, for example
    // LOK_CALLBACK_TEXT_SELECTION_START and LOK_CALLBACK_TEXT_SELECTION_END
    // must always precede LOK_CALLBACK_TEXT_SELECTION if present.
    // Only these types should be present (see isUpdatedType()) and should be processed in this order.
    static const int orderedUpdatedTypes[] = {
        LOK_CALLBACK_TEXT_SELECTION_START, LOK_CALLBACK_TEXT_SELECTION_END, LOK_CALLBACK_TEXT_SELECTION };
    // Only these types should be present (see isUpdatedTypePerViewId()) and (as of now)
    // the order doesn't matter.
    static const int orderedUpdatedTypesPerViewId[] = {
        LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR,
        LOK_CALLBACK_INVALIDATE_VIEW_CURSOR,
        LOK_CALLBACK_TEXT_VIEW_SELECTION };

    for( int type : orderedUpdatedTypes )
    {
        if(o3tl::make_unsigned( type ) < updatedTypes.size() && updatedTypes[ type ])
        {
            enqueueUpdatedType( type, viewShell, m_viewId );
        }
    }
    for( const auto& it : updatedTypesPerViewId )
    {
        int viewId = it.first;
        const std::vector<PerViewIdData>& types = it.second;
        for( int type : orderedUpdatedTypesPerViewId )
        {
            if(o3tl::make_unsigned( type ) < types.size() && types[ type ].set)
            {
                SfxViewShell* sourceViewShell = viewShell;
                const int sourceViewId = types[ type ].sourceViewId;
                if( sourceViewId != m_viewId )
                {
                    assert(sourceViewId >= 0);
                    sourceViewShell = SfxViewShell::GetFirst( false,
                    [sourceViewId](const SfxViewShell& shell) { return shell.GetViewShellId().get() == sourceViewId; } );
                }
                if(sourceViewShell == nullptr)
                {
                    SAL_INFO("lok", "View #" << sourceViewId << " no longer found for updated event [" << type << "]");
                    continue; // View removed, probably cleaning up.
                }
                enqueueUpdatedType( type, sourceViewShell, viewId );
            }
        }
    }
}

void CallbackFlushHandler::enqueueUpdatedType( int type, const SfxViewShell* viewShell, int viewId )
{
    if (type == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR)
    {
        if (const SfxViewShell* viewShell2 = LokStarMathHelper(viewShell).GetSmViewShell())
            viewShell = viewShell2;
    }
    std::optional<OString> payload = viewShell->getLOKPayload( type, viewId );
    if(!payload)
        return; // No actual payload to send.
    CallbackData callbackData(*payload, viewId);
    m_queue1.emplace_back(type);
    m_queue2.emplace_back(callbackData);
    SAL_INFO("lok", "Queued updated [" << type << "]: [" << callbackData.getPayload()
        << "] to have " << m_queue1.size() << " entries.");
}

void CallbackFlushHandler::invoke()
{
    comphelper::ProfileZone aZone("CallbackFlushHandler::Invoke");

    if (!m_pCallback)
        return;

    // Get any pending invalidate tile events. This will call our callbacks,
    // so it must be done before taking the mutex.
    assert(m_viewId >= 0);
    if(SfxViewShell* viewShell = SfxViewShell::GetFirst( false,
        [this](const SfxViewShell& shell) { return shell.GetViewShellId().get() == m_viewId; } ))
    {
        viewShell->flushPendingLOKInvalidateTiles();
    }

    std::unique_lock<std::recursive_mutex> lock(m_mutex);

    // Append messages for updated types, fetch them only now.
    enqueueUpdatedTypes();

    SAL_INFO("lok", "Flushing " << m_queue1.size() << " elements.");
    auto it1 = m_queue1.begin();
    auto it2 = m_queue2.begin();
    for (; it1 != m_queue1.end(); ++it1, ++it2)
    {
        const int type = *it1;
        const auto& payload = it2->getPayload();
        const int viewId = lcl_isViewCallbackType(type) ? it2->getViewId() : -1;

        SAL_INFO("lok", "processing event: [" << type << ',' << viewId << "]: [" << payload << "].");

        // common code-path for events on this view:
        if (viewId == -1)
        {
            sal_Int32 idx;
            // key-value pairs
            if (type == LOK_CALLBACK_STATE_CHANGED &&
                (idx = payload.indexOf('=')) != -1)
            {
                OString key = payload.copy(0, idx);
                OString value = payload.copy(idx+1);
                const auto stateIt = m_lastStateChange.find(key);
                if (stateIt != m_lastStateChange.end())
                {
                    // If the value didn't change, it's safe to ignore.
                    if (stateIt->second == value)
                    {
                        SAL_INFO("lok", "Skipping new state duplicate: [" << type << "]: [" << payload << "].");
                        continue;
                    }
                    SAL_INFO("lok", "Replacing a state element [" << type << "]: [" << payload << "].");
                    stateIt->second = value;
                }
                else
                {
                    SAL_INFO("lok", "Inserted a new state element: [" << type << "]: [" << payload << "]");
                    m_lastStateChange.emplace(key, value);
                }
            }
            else
            {
                const auto stateIt = m_states.find(type);
                if (stateIt != m_states.end())
                {
                    // If the state didn't change, it's safe to ignore.
                    if (stateIt->second == payload)
                    {
                        SAL_INFO("lok", "Skipping duplicate [" << type << "]: [" << payload << "].");
                        continue;
                    }
                    stateIt->second = payload;
                }
            }
        }
        else // less common path for events relating to other views
        {
            const auto statesIt = m_viewStates.find(viewId);
            if (statesIt != m_viewStates.end())
            {
                auto& states = statesIt->second;
                const auto stateIt = states.find(type);
                if (stateIt != states.end())
                {
                    // If the state didn't change, it's safe to ignore.
                    if (stateIt->second == payload)
                    {
                        SAL_INFO("lok", "Skipping view duplicate [" << type << ',' << viewId << "]: [" << payload << "].");
                        continue;
                    }

                    SAL_INFO("lok", "Replacing an element in view states [" << type << ',' << viewId << "]: [" << payload << "].");
                    stateIt->second = payload;
                }
                else
                {
                    SAL_INFO("lok", "Inserted a new element in view states: [" << type << ',' << viewId << "]: [" << payload << "]");
                    states.emplace(type, payload);

                }
            }
        }

        m_pCallback(type, payload.getStr(), m_pData);
    }

    m_queue1.clear();
    m_queue2.clear();
    stop();
}

void CallbackFlushHandler::scheduleFlush()
{
    if (!m_pFlushEvent)
        m_pFlushEvent = Application::PostUserEvent(LINK(this, CallbackFlushHandler, FlushQueue));
}

IMPL_LINK_NOARG(CallbackFlushHandler, FlushQueue, void*, void)
{
    m_pFlushEvent = nullptr;
    invoke();
}

bool CallbackFlushHandler::removeAll(int type)
{
    bool bErased = false;
    auto it1 = m_queue1.begin();
    for(;;)
    {
        it1 = std::find(it1, m_queue1.end(), type);
        if(it1 == m_queue1.end())
            break;
        m_queue2.erase(toQueue2(it1));
        it1 = m_queue1.erase(it1);
        bErased = true;
    }
    return bErased;
}

bool CallbackFlushHandler::removeAll(int type, const std::function<bool (const CallbackData&)>& rTestFunc)
{
    bool bErased = false;
    auto it1 = m_queue1.begin();
    for(;;)
    {
        it1 = std::find(it1, m_queue1.end(), type);
        if(it1 == m_queue1.end())
            break;
        auto it2 = toQueue2(it1);
        if (rTestFunc(*it2))
        {
            m_queue2.erase(it2);
            it1 = m_queue1.erase(it1);
            bErased = true;
        }
        else
            ++it1;
    }
    return bErased;
}

void CallbackFlushHandler::addViewStates(int viewId)
{
    const auto& result = m_viewStates.emplace(viewId, decltype(m_viewStates)::mapped_type());
    if (!result.second && result.first != m_viewStates.end())
    {
        result.first->second.clear();
    }
}

void CallbackFlushHandler::removeViewStates(int viewId)
{
    m_viewStates.erase(viewId);
}


static void doc_destroy(LibreOfficeKitDocument *pThis)
{
    comphelper::ProfileZone aZone("doc_destroy");

    SolarMutexGuard aGuard;

#ifndef IOS
    LOKClipboardFactory::releaseClipboardForView(-1);
#endif

    LibLODocument_Impl *pDocument = static_cast<LibLODocument_Impl*>(pThis);
    delete pDocument;
}

static void                    lo_destroy       (LibreOfficeKit* pThis);
static int                     lo_initialize    (LibreOfficeKit* pThis, const char* pInstallPath, const char* pUserProfilePath);
static LibreOfficeKitDocument* lo_documentLoad  (LibreOfficeKit* pThis, const char* pURL);
static char *                  lo_getError      (LibreOfficeKit* pThis);
static void                    lo_freeError     (char* pFree);
static LibreOfficeKitDocument* lo_documentLoadWithOptions  (LibreOfficeKit* pThis,
                                                           const char* pURL,
                                                           const char* pOptions);
static void                    lo_registerCallback (LibreOfficeKit* pThis,
                                                    LibreOfficeKitCallback pCallback,
                                                    void* pData);
static char* lo_getFilterTypes(LibreOfficeKit* pThis);
static void                    lo_setOptionalFeatures(LibreOfficeKit* pThis, unsigned long long features);
static void                    lo_setDocumentPassword(LibreOfficeKit* pThis,
                                                       const char* pURL,
                                                       const char* pPassword);
static char*                   lo_getVersionInfo(LibreOfficeKit* pThis);
static int                     lo_runMacro      (LibreOfficeKit* pThis, const char* pURL);

static bool lo_signDocument(LibreOfficeKit* pThis,
                                   const char* pUrl,
                                   const unsigned char* pCertificateBinary,
                                   const int nCertificateBinarySize,
                                   const unsigned char* pPrivateKeyBinary,
                                   const int nPrivateKeyBinarySize);

static char* lo_extractRequest(LibreOfficeKit* pThis,
                                   const char* pFilePath);

static void lo_trimMemory(LibreOfficeKit* pThis, int nTarget);

static void*
lo_startURP(LibreOfficeKit* pThis, void* pReceiveURPFromLOContext, void* pSendURPToLOContext,
            int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
            int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen));

static void lo_stopURP(LibreOfficeKit* pThis, void* pSendURPToLOContext);

static int lo_joinThreads(LibreOfficeKit* pThis);

static void lo_setForkedChild(LibreOfficeKit* pThis, bool bIsChild);

static void lo_runLoop(LibreOfficeKit* pThis,
                       LibreOfficeKitPollCallback pPollCallback,
                       LibreOfficeKitWakeCallback pWakeCallback,
                       void* pData);

static void lo_sendDialogEvent(LibreOfficeKit* pThis,
                               unsigned long long int nLOKWindowId,
                               const char* pArguments);

static void lo_setOption(LibreOfficeKit* pThis, const char* pOption, const char* pValue);

static void lo_dumpState(LibreOfficeKit* pThis, const char* pOptions, char** pState);

LibLibreOffice_Impl::LibLibreOffice_Impl()
    : m_pOfficeClass( gOfficeClass.lock() )
    , maThread(nullptr)
    , mpCallback(nullptr)
    , mpCallbackData(nullptr)
    , mOptionalFeatures(0)
{
    if(!m_pOfficeClass) {
        m_pOfficeClass = std::make_shared<LibreOfficeKitClass>();
        m_pOfficeClass->nSize = sizeof(LibreOfficeKitClass);

        m_pOfficeClass->destroy = lo_destroy;
        m_pOfficeClass->documentLoad = lo_documentLoad;
        m_pOfficeClass->getError = lo_getError;
        m_pOfficeClass->freeError = lo_freeError;
        m_pOfficeClass->documentLoadWithOptions = lo_documentLoadWithOptions;
        m_pOfficeClass->registerCallback = lo_registerCallback;
        m_pOfficeClass->getFilterTypes = lo_getFilterTypes;
        m_pOfficeClass->setOptionalFeatures = lo_setOptionalFeatures;
        m_pOfficeClass->setDocumentPassword = lo_setDocumentPassword;
        m_pOfficeClass->getVersionInfo = lo_getVersionInfo;
        m_pOfficeClass->runMacro = lo_runMacro;
        m_pOfficeClass->signDocument = lo_signDocument;
        m_pOfficeClass->runLoop = lo_runLoop;
        m_pOfficeClass->sendDialogEvent = lo_sendDialogEvent;
        m_pOfficeClass->setOption = lo_setOption;
        m_pOfficeClass->dumpState = lo_dumpState;
        m_pOfficeClass->extractRequest = lo_extractRequest;
        m_pOfficeClass->trimMemory = lo_trimMemory;
        m_pOfficeClass->startURP = lo_startURP;
        m_pOfficeClass->stopURP = lo_stopURP;
        m_pOfficeClass->joinThreads = lo_joinThreads;
        m_pOfficeClass->setForkedChild = lo_setForkedChild;

        gOfficeClass = m_pOfficeClass;
    }

    pClass = m_pOfficeClass.get();
}

LibLibreOffice_Impl::~LibLibreOffice_Impl()
{
}

namespace
{

void setLanguageAndLocale(OUString const & aLangISO)
{
    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetLocaleConfigString(aLangISO);
    aLocalOptions.SetUILocaleConfigString(aLangISO);
    aLocalOptions.Commit();
}

void setFormatSpecificFilterData(std::u16string_view sFormat, comphelper::SequenceAsHashMap & rFilterDataMap)
{
    if (sFormat == u"pdf")
    {
        // always export bookmarks, which is needed for annotations
        rFilterDataMap[u"ExportBookmarks"_ustr] <<= true;
    }
}

} // anonymous namespace

// Wonder global state ...
static uno::Reference<css::uno::XComponentContext> xContext;
static uno::Reference<css::lang::XMultiServiceFactory> xSFactory;
static uno::Reference<css::lang::XMultiComponentFactory> xFactory;

static LibreOfficeKitDocument* lo_documentLoad(LibreOfficeKit* pThis, const char* pURL)
{
    return lo_documentLoadWithOptions(pThis, pURL, nullptr);
}

static LibreOfficeKitDocument* lo_documentLoadWithOptions(LibreOfficeKit* pThis, const char* pURL, const char* pOptions)
{
    comphelper::ProfileZone aZone("lo_documentLoadWithOptions");

    SolarMutexGuard aGuard;

    static int nDocumentIdCounter = 0;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->maLastExceptionMsg.clear();

    const OUString aURL(getAbsoluteURL(pURL));
    if (aURL.isEmpty())
    {
        pLib->maLastExceptionMsg = u"Filename to load was not provided."_ustr;
        SAL_INFO("lok", "URL for load is empty");
        return nullptr;
    }

    pLib->maLastExceptionMsg.clear();

    if (!xContext.is())
    {
        pLib->maLastExceptionMsg = u"ComponentContext is not available"_ustr;
        SAL_INFO("lok", "ComponentContext is not available");
        return nullptr;
    }

    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);

    if (!xComponentLoader.is())
    {
        pLib->maLastExceptionMsg = u"ComponentLoader is not available"_ustr;
        SAL_INFO("lok", "ComponentLoader is not available");
        return nullptr;
    }

    try
    {
        // 'Language=...' is an option that LOK consumes by itself, and does
        // not pass it as a parameter to the filter
        OUString aOptions = getUString(pOptions);
        const OUString aLanguage = extractParameter(aOptions, u"Language");

        if (!aLanguage.isEmpty() && LanguageTag::isValidBcp47(aLanguage, nullptr))
        {
            static bool isLoading = true;
            if (isLoading)
            {
                // Capture the language used to load the document.
                SfxLokHelper::setLoadLanguage(aLanguage);
                isLoading = false;
            }

            SfxLokHelper::setDefaultLanguage(aLanguage);
            // Set the LOK language tag, used for dialog tunneling.
            comphelper::LibreOfficeKit::setLanguageTag(LanguageTag(aLanguage));
            comphelper::LibreOfficeKit::setLocale(LanguageTag(aLanguage));

            SAL_INFO("lok", "Set document language to " << aLanguage);
            // use with care - it sets it for the entire core, not just the
            // document
            setLanguageAndLocale(aLanguage);
            // Need to reset the static initialized values
            SvNumberFormatter::resetTheCurrencyTable();
        }

        // Set the timezone, if not empty.
        const OUString aTimezone = extractParameter(aOptions, u"Timezone");
        if (!aTimezone.isEmpty())
        {
            SfxLokHelper::setDefaultTimezone(true, aTimezone);
        }
        else
        {
            // Default to the TZ envar, if set.
            const char* tz = ::getenv("TZ");
            if (tz)
            {
                SfxLokHelper::setDefaultTimezone(true,
                                                 OStringToOUString(tz, RTL_TEXTENCODING_UTF8));
            }
            else
            {
                SfxLokHelper::setDefaultTimezone(false, OUString());
            }
        }

        const OUString aDeviceFormFactor = extractParameter(aOptions, u"DeviceFormFactor");
        SfxLokHelper::setDeviceFormFactor(aDeviceFormFactor);

        const OUString aBatch = extractParameter(aOptions, u"Batch");
        if (!aBatch.isEmpty())
        {
             Application::SetDialogCancelMode(DialogCancelMode::LOKSilent);
        }

        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler("load"_ostr, pLib));
        auto const pair(pLib->mInteractionMap.insert(std::make_pair(aURL.toUtf8(), pInteraction)));
        comphelper::ScopeGuard const g([&] () {
                if (pair.second)
                {
                    pLib->mInteractionMap.erase(aURL.toUtf8());
                }
            });
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction);

        int nMacroSecurityLevel = 1;
        const OUString aMacroSecurityLevel = extractParameter(aOptions, u"MacroSecurityLevel");
        if (!aMacroSecurityLevel.isEmpty())
        {
            double nNumber;
            sal_uInt32 nFormat = 1;
            SvNumberFormatter aFormatter(::comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US);
            if (aFormatter.IsNumberFormat(aMacroSecurityLevel, nFormat, nNumber))
                nMacroSecurityLevel = static_cast<int>(nNumber);
        }
        SvtSecurityOptions::SetMacroSecurityLevel(nMacroSecurityLevel);

#if defined(ANDROID) && HAVE_FEATURE_ANDROID_LOK
        sal_Int16 nMacroExecMode = document::MacroExecMode::USE_CONFIG;
#else
        const OUString aEnableMacrosExecution = extractParameter(aOptions, u"EnableMacrosExecution");
        sal_Int16 nMacroExecMode = aEnableMacrosExecution == "true" ? document::MacroExecMode::USE_CONFIG :
            document::MacroExecMode::NEVER_EXECUTE;
#endif

        // set AsTemplate explicitly false to be able to load template files
        // as regular files, otherwise we cannot save them; it will try
        // to bring saveas dialog which cannot work with LOK case
        uno::Sequence<css::beans::PropertyValue> aFilterOptions{
            comphelper::makePropertyValue(u"FilterOptions"_ustr, aOptions),
            comphelper::makePropertyValue(u"InteractionHandler"_ustr, xInteraction),
            comphelper::makePropertyValue(u"MacroExecutionMode"_ustr, nMacroExecMode),
            comphelper::makePropertyValue(u"AsTemplate"_ustr, false),
            comphelper::makePropertyValue(u"Silent"_ustr, !aBatch.isEmpty())
        };

        /* TODO
        sal_Int16 nUpdateDoc = document::UpdateDocMode::ACCORDING_TO_CONFIG;
        aFilterOptions[3].Name = "UpdateDocMode";
        aFilterOptions[3].Value <<= nUpdateDoc;
        */

        OutputDevice::StartTrackingFontMappingUse();

        const int nThisDocumentId = nDocumentIdCounter++;
        SfxViewShell::SetCurrentDocId(ViewShellDocId(nThisDocumentId));
        uno::Reference<lang::XComponent> xComponent = xComponentLoader->loadComponentFromURL(
                                            aURL, u"_blank"_ustr, 0,
                                            aFilterOptions);

        assert(!xComponent.is() || pair.second); // concurrent loading of same URL ought to fail

        if (!xComponent.is())
        {
            pLib->maLastExceptionMsg = u"loadComponentFromURL returned an empty reference"_ustr;
            SAL_INFO("lok", "Document can't be loaded - " << pLib->maLastExceptionMsg);
            return nullptr;
        }

        LibLODocument_Impl* pDocument = new LibLODocument_Impl(xComponent, nThisDocumentId);

        // After loading the document, its initial view is the "current" view.
        if (pLib->mpCallback)
        {
            int nState = doc_getSignatureState(pDocument);
            pLib->mpCallback(LOK_CALLBACK_SIGNATURE_STATUS, OString::number(nState).getStr(), pLib->mpCallbackData);
        }

        auto aFontMappingUseData = OutputDevice::FinishTrackingFontMappingUse();

        if (aFontMappingUseData.size() > 0)
        {
            SAL_INFO("lok.fontsubst", "================ Original substitutions:");
            for (const auto &i : aFontMappingUseData)
            {
                SAL_INFO("lok.fontsubst", i.mOriginalFont);
                for (const auto &j : i.mUsedFonts)
                    SAL_INFO("lok.fontsubst", "    " << j);
            }
        }

        // Filter out font substitutions that actually aren't any substitutions, like "Liberation
        // Serif" -> "Liberation Serif/Regular". If even one of the "substitutions" of a font is to
        // the same font, don't count that as a missing font.

        std::erase_if
            (aFontMappingUseData,
                            [](OutputDevice::FontMappingUseItem x)
                            {
                                // If the original font had an empty style and one of its
                                // replacement fonts has the same family name, we assume the font is
                                // present. The root problem here is that the code that collects
                                // font substitutions tends to get just empty styles for the font
                                // that is being substituted, as vcl::Font::GetStyleName() tends to
                                // return an empty string. (Italicness is instead indicated by what
                                // vcl::Font::GetItalic() returns and boldness by what
                                // vcl::Font::GetWeight() returns.)

                                if (x.mOriginalFont.indexOf('/') == -1)
                                    for (const auto &j : x.mUsedFonts)
                                        if (j == x.mOriginalFont ||
                                            j.startsWith(Concat2View(x.mOriginalFont + "/")))
                                            return true;

                                return false;
                            });

        // Filter out substitutions where a proprietary font has been substituted by a
        // metric-compatible one. Obviously this is just a heuristic and implemented only for some
        // well-known cases.

        std::erase_if
            (aFontMappingUseData,
                            [](OutputDevice::FontMappingUseItem x)
                            {
                                // Again, handle only cases where the original font does not include
                                // a style. Unclear whether there ever will be a style part included
                                // in the mOriginalFont.

                                if (x.mOriginalFont.indexOf('/') == -1)
                                    for (const auto &j : x.mUsedFonts)
                                        if ((x.mOriginalFont == "Arial" &&
                                             j.startsWith("Liberation Sans/")) ||
                                            (x.mOriginalFont == "Times New Roman" &&
                                             j.startsWith("Liberation Serif/")) ||
                                            (x.mOriginalFont == "Courier New" &&
                                             j.startsWith("Liberation Mono/")) ||
                                            (x.mOriginalFont == "Arial Narrow" &&
                                             j.startsWith("Liberation Sans Narrow/")) ||
                                            (x.mOriginalFont == "Cambria" &&
                                             j.startsWith("Caladea/")) ||
                                            (x.mOriginalFont == "Calibri" &&
                                             j.startsWith("Carlito/")) ||
                                            (x.mOriginalFont == "Palatino Linotype" &&
                                             j.startsWith("P052/")) ||
                                            // Perhaps a risky heuristic? If some glyphs from Symbol
                                            // have been mapped to ones in OpenSymbol, don't warn
                                            // that Symbol is missing.
                                            (x.mOriginalFont == "Symbol" &&
                                             j.startsWith("OpenSymbol/")))
                                        {
                                            return true;
                                        }

                                return false;
                            });

        if (aFontMappingUseData.size() > 0)
        {
            SAL_INFO("lok.fontsubst", "================ Pruned substitutions:");
            for (const auto &i : aFontMappingUseData)
            {
                SAL_INFO("lok.fontsubst", i.mOriginalFont);
                for (const auto &j : i.mUsedFonts)
                    SAL_INFO("lok.fontsubst", "    " << j);
            }
        }

        for (std::size_t i = 0; i < aFontMappingUseData.size(); ++i)
        {
            pDocument->maFontsMissing.insert(aFontMappingUseData[i].mOriginalFont);
        }

        return pDocument;
    }
    catch (const uno::Exception& exception)
    {
        pLib->maLastExceptionMsg = exception.Message;
        TOOLS_INFO_EXCEPTION("lok", "Document can't be loaded");
    }

    return nullptr;
}

static int lo_runMacro(LibreOfficeKit* pThis, const char *pURL)
{
    comphelper::ProfileZone aZone("lo_runMacro");

    SolarMutexGuard aGuard;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->maLastExceptionMsg.clear();

    OUString sURL( pURL, strlen(pURL), RTL_TEXTENCODING_UTF8 );
    if (sURL.isEmpty())
    {
        pLib->maLastExceptionMsg = u"Macro to run was not provided."_ustr;
        SAL_INFO("lok", "Macro URL is empty");
        return false;
    }

    if (!sURL.startsWith("macro://"))
    {
        pLib->maLastExceptionMsg = u"This doesn't look like macro URL"_ustr;
        SAL_INFO("lok", "Macro URL is invalid");
        return false;
    }

    pLib->maLastExceptionMsg.clear();

    if (!xContext.is())
    {
        pLib->maLastExceptionMsg = u"ComponentContext is not available"_ustr;
        SAL_INFO("lok", "ComponentContext is not available");
        return false;
    }

    util::URL aURL;
    aURL.Complete = sURL;

    uno::Reference < util::XURLTransformer > xParser( util::URLTransformer::create( xContext ) );

    if( xParser.is() )
        xParser->parseStrict( aURL );

    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);

    if (!xComponentLoader.is())
    {
        pLib->maLastExceptionMsg = u"ComponentLoader is not available"_ustr;
        SAL_INFO("lok", "ComponentLoader is not available");
        return false;
    }

    xFactory = xContext->getServiceManager();

    if (!xFactory)
        return false;

    uno::Reference<frame::XDispatchProvider> xDP;
    xSFactory.set(xFactory, uno::UNO_QUERY_THROW);
    xDP.set( xSFactory->createInstance(u"com.sun.star.comp.sfx2.SfxMacroLoader"_ustr), uno::UNO_QUERY );
    uno::Reference<frame::XDispatch> xD = xDP->queryDispatch( aURL, OUString(), 0);

    if (!xD.is())
    {
        pLib->maLastExceptionMsg = u"Macro loader is not available"_ustr;
        SAL_INFO("lok", "Macro loader is not available");
        return false;
    }

    uno::Reference < frame::XSynchronousDispatch > xSyncDisp( xD, uno::UNO_QUERY_THROW );
    uno::Sequence<css::beans::PropertyValue> aEmpty;
    css::beans::PropertyValue aErr;
    uno::Any aRet = xSyncDisp->dispatchWithReturnValue( aURL, aEmpty );
    aRet >>= aErr;

    if (aErr.Name == "ErrorCode")
    {
        sal_uInt32 nErrCode = 0; // ERRCODE_NONE
        aErr.Value >>= nErrCode;

        pLib->maLastExceptionMsg = "An error occurred running macro (error code: " + OUString::number( nErrCode ) + ")";
        SAL_INFO("lok", "Macro execution terminated with error code " << nErrCode);

        return false;
    }

    return true;
}

static bool lo_signDocument(LibreOfficeKit* /*pThis*/,
                            const char* pURL,
                            const unsigned char* pCertificateBinary,
                            const int nCertificateBinarySize,
                            const unsigned char* pPrivateKeyBinary,
                            const int nPrivateKeyBinarySize)
{
    comphelper::ProfileZone aZone("lo_signDocument");

    OUString aURL(getAbsoluteURL(pURL));
    if (aURL.isEmpty())
       return false;

    if (!xContext.is())
        return false;

    uno::Sequence<sal_Int8> aCertificateSequence;

    std::string aCertificateString(reinterpret_cast<const char*>(pCertificateBinary), nCertificateBinarySize);
    std::string aCertificateBase64String = extractCertificate(aCertificateString);
    if (!aCertificateBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String);
        comphelper::Base64::decode(aCertificateSequence, aBase64OUString);
    }
    else
    {
        aCertificateSequence.realloc(nCertificateBinarySize);
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.getArray());
    }

    uno::Sequence<sal_Int8> aPrivateKeySequence;
    std::string aPrivateKeyString(reinterpret_cast<const char*>(pPrivateKeyBinary), nPrivateKeyBinarySize);
    std::string aPrivateKeyBase64String = extractPrivateKey(aPrivateKeyString);
    if (!aPrivateKeyBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aPrivateKeyBase64String);
        comphelper::Base64::decode(aPrivateKeySequence, aBase64OUString);
    }
    else
    {
        aPrivateKeySequence.realloc(nPrivateKeyBinarySize);
        std::copy(pPrivateKeyBinary, pPrivateKeyBinary + nPrivateKeyBinarySize, aPrivateKeySequence.getArray());
    }

    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    if (!xSecurityContext.is())
        return false;

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Reference<xml::crypto::XCertificateCreator> xCertificateCreator(xSecurityEnvironment, uno::UNO_QUERY);

    if (!xCertificateCreator.is())
        return false;

    uno::Reference<security::XCertificate> xCertificate = xCertificateCreator->createDERCertificateWithPrivateKey(aCertificateSequence, aPrivateKeySequence);

    if (!xCertificate.is())
        return false;

    sfx2::DocumentSigner aDocumentSigner(aURL);
    if (!aDocumentSigner.signDocument(xCertificate))
        return false;

    return true;
}


static char* lo_extractRequest(LibreOfficeKit* /*pThis*/, const char* pFilePath)
{
    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);
    uno::Reference< css::lang::XComponent > xComp;
    OUString aURL(getAbsoluteURL(pFilePath));
    if (!aURL.isEmpty())
    {
        if (xComponentLoader.is())
        {
            try
            {
                uno::Sequence<css::beans::PropertyValue> aFilterOptions(comphelper::InitPropertySequence(
                {
                    {u"Hidden"_ustr, css::uno::Any(true)},
                    {u"ReadOnly"_ustr, css::uno::Any(true)}
                }));
                xComp = xComponentLoader->loadComponentFromURL( aURL, u"_blank"_ustr, 0, aFilterOptions );
            }
            catch ( const lang::IllegalArgumentException& ex )
            {
                SAL_WARN("lok", "lo_extractRequest: IllegalArgumentException: " << ex.Message);
            }
            catch (...)
            {
                SAL_WARN("lok", "lo_extractRequest: Exception on loadComponentFromURL, url= " << aURL);
            }

            if (xComp.is())
            {
                uno::Reference< document::XLinkTargetSupplier > xLTS( xComp, uno::UNO_QUERY );

                if( xLTS.is() )
                {
                    tools::JsonWriter aJson;
                    {
                        auto aNode = aJson.startNode("Targets");
                        extractLinks(xLTS->getLinks(), false, aJson);
                    }
                    return convertOString(aJson.finishAndGetAsOString());
                }
                xComp->dispose();
            }
        }
    }
    return strdup("{ }");
}

static void lo_trimMemory(LibreOfficeKit* /* pThis */, int nTarget)
{
    vcl::lok::trimMemory(nTarget);

    if (nTarget > 2000)
    {
        SolarMutexGuard aGuard;

        // Flush all buffered VOC primitives from the pages.
        SfxViewShell* pViewShell = SfxViewShell::Current();
        if (pViewShell)
        {
            const SdrView* pView = pViewShell->GetDrawView();
            if (pView)
            {
                SdrPageView* pPageView = pView->GetSdrPageView();
                if (pPageView)
                {
                    SdrPage* pCurPage = pPageView->GetPage();
                    if (pCurPage)
                    {
                        SdrModel& sdrModel = pCurPage->getSdrModelFromSdrPage();
                        for (sal_uInt16 i = 0; i < sdrModel.GetPageCount(); ++i)
                        {
                            SdrPage* pPage = sdrModel.GetPage(i);
                            if (pPage)
                                pPage->GetViewContact().flushViewObjectContacts();
                        }
                    }
                }
            }
        }
    }

    if (nTarget > 1000)
    {
#ifdef HAVE_MALLOC_TRIM
        malloc_trim(0);
#endif
    }
}

namespace
{
class FunctionBasedURPInstanceProvider
    : public ::cppu::WeakImplHelper<css::bridge::XInstanceProvider>
{
private:
    css::uno::Reference<css::uno::XComponentContext> m_rContext;

public:
    FunctionBasedURPInstanceProvider(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    // XInstanceProvider
    virtual css::uno::Reference<css::uno::XInterface>
        SAL_CALL getInstance(const OUString& aName) override;
};

// InstanceProvider
FunctionBasedURPInstanceProvider::FunctionBasedURPInstanceProvider(
    const Reference<XComponentContext>& rxContext)
    : m_rContext(rxContext)
{
}

Reference<XInterface> FunctionBasedURPInstanceProvider::getInstance(const OUString& aName)
{
    Reference<XInterface> rInstance;

    if (aName == "StarOffice.ServiceManager")
    {
        rInstance.set(m_rContext->getServiceManager());
    }
    else if (aName == "StarOffice.ComponentContext")
    {
        rInstance = m_rContext;
    }
    else if (aName == "StarOffice.NamingService")
    {
        Reference<XNamingService> rNamingService(
            m_rContext->getServiceManager()->createInstanceWithContext(
                u"com.sun.star.uno.NamingService"_ustr, m_rContext),
            UNO_QUERY);
        if (rNamingService.is())
        {
            rNamingService->registerObject(u"StarOffice.ServiceManager"_ustr,
                                           m_rContext->getServiceManager());
            rNamingService->registerObject(u"StarOffice.ComponentContext"_ustr, m_rContext);
            rInstance = rNamingService;
        }
    }
    return rInstance;
}

class FunctionBasedURPConnection : public cppu::WeakImplHelper<css::connection::XConnection>
{
public:
    explicit FunctionBasedURPConnection(void*, int (*)(void* pContext, const signed char* pBuffer, int nLen),
                                        void*, int (*)(void* pContext, signed char* pBuffer, int nLen));
    ~FunctionBasedURPConnection();

    // These overridden member functions use "read" and "write" from the point of view of LO,
    // i.e. the opposite to how startURP() uses them.
    virtual sal_Int32 SAL_CALL read(Sequence<sal_Int8>& rReadBytes,
                                    sal_Int32 nBytesToRead) override;
    virtual void SAL_CALL write(const Sequence<sal_Int8>& aData) override;
    virtual void SAL_CALL flush() override;
    virtual void SAL_CALL close() override;
    virtual OUString SAL_CALL getDescription() override;
    void setBridge(const Reference<XBridge>&);
    void* getContext();
    inline static int g_connectionCount = 0;

private:
    void* m_pRecieveFromLOContext;
    void* m_pSendURPToLOContext;
    int (*m_fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen);
    int (*m_fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen);
    Reference<XBridge> m_URPBridge;
};

FunctionBasedURPConnection::FunctionBasedURPConnection(
    void* pRecieveFromLOContext,
    int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
    void* pSendURPToLOContext,
    int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen))
    : m_pRecieveFromLOContext(pRecieveFromLOContext)
    , m_pSendURPToLOContext(pSendURPToLOContext)
    , m_fnReceiveURPFromLO(fnReceiveURPFromLO)
    , m_fnSendURPToLO(fnSendURPToLO)
{
    g_connectionCount++;
}

FunctionBasedURPConnection::~FunctionBasedURPConnection()
{
    Reference<XComponent> xComp(m_URPBridge, UNO_QUERY_THROW);
    xComp->dispose(); // TODO: check this doesn't deadlock
}

void* FunctionBasedURPConnection::getContext() { return this; }

sal_Int32 FunctionBasedURPConnection::read(Sequence<sal_Int8>& rReadBytes, sal_Int32 nBytesToRead)
{
    if (nBytesToRead < 0)
        return 0;

    if (rReadBytes.getLength() != nBytesToRead)
        rReadBytes.realloc(nBytesToRead);

    // As with osl::StreamPipe, we must always read nBytesToRead...
    return m_fnSendURPToLO(m_pSendURPToLOContext, rReadBytes.getArray(), nBytesToRead);
}

void FunctionBasedURPConnection::write(const Sequence<sal_Int8>& rData)
{
    m_fnReceiveURPFromLO(m_pRecieveFromLOContext, rData.getConstArray(), rData.getLength());
}

void FunctionBasedURPConnection::flush() {}

void FunctionBasedURPConnection::close()
{
    SAL_INFO("lok.urp", "Requested to close FunctionBasedURPConnection");
}

OUString FunctionBasedURPConnection::getDescription() { return u""_ustr; }

void FunctionBasedURPConnection::setBridge(const Reference<XBridge>& xBridge) { m_URPBridge = xBridge; }
}

static void*
lo_startURP(LibreOfficeKit* /* pThis */, void* pRecieveFromLOContext, void* pSendToLOContext,
            int (*fnReceiveURPFromLO)(void* pContext, const signed char* pBuffer, int nLen),
            int (*fnSendURPToLO)(void* pContext, signed char* pBuffer, int nLen))
{
    // Here we will roughly do what desktop LO does when one passes a command-line switch like
    // --accept=socket,port=nnnn;urp;StarOffice.ServiceManager. Except that no listening socket will
    // be created. The communication to the URP will be through the nReceiveURPFromLO and nSendURPToLO
    // functions.

    rtl::Reference<FunctionBasedURPConnection> connection(
        new FunctionBasedURPConnection(pRecieveFromLOContext, fnReceiveURPFromLO,
                                       pSendToLOContext, fnSendURPToLO));

    Reference<XBridgeFactory> xBridgeFactory = css::bridge::BridgeFactory::create(xContext);

    Reference<XInstanceProvider> xInstanceProvider(new FunctionBasedURPInstanceProvider(xContext));

    Reference<XBridge> xBridge(xBridgeFactory->createBridge(
        "functionurp" + OUString::number(FunctionBasedURPConnection::g_connectionCount), u"urp"_ustr,
        connection, xInstanceProvider));

    connection->setBridge(std::move(xBridge));

    return connection->getContext();
}

/**
 * Stop a function based URP connection that you started with lo_startURP above
 *
 * @param pSendToLOContext a pointer to the context returned by lo_startURP */
static void lo_stopURP(LibreOfficeKit* /* pThis */,
                       void* pFunctionBasedURPConnection/* FunctionBasedURPConnection* */)
{
    static_cast<FunctionBasedURPConnection*>(pFunctionBasedURPConnection)->close();
}


static int lo_joinThreads(LibreOfficeKit* /* pThis */)
{
    comphelper::ThreadPool &pool = comphelper::ThreadPool::getSharedOptimalPool();
    pool.joinThreadsIfIdle();

//    if (comphelper::getWorkerCount() > 0)
//        return 0;

    // Grammar checker thread
    css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv =
        css::linguistic2::LinguServiceManager::create(xContext);

    auto joinable = dynamic_cast<comphelper::LibreOfficeKit::ThreadJoinable *>(xLangSrv.get());
    if (joinable && !joinable->joinThreads())
        return 0;

    return 1;
}

static void lo_setForkedChild(LibreOfficeKit* /* pThis */, bool bIsChild)
{
    comphelper::LibreOfficeKit::setForkedChild(bIsChild);
}

static void lo_registerCallback (LibreOfficeKit* pThis,
                                 LibreOfficeKitCallback pCallback,
                                 void* pData)
{
    SolarMutexGuard aGuard;

    Application* pApp = GetpApp();
    assert(pApp);

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->maLastExceptionMsg.clear();

    pApp->m_pCallback = pLib->mpCallback = pCallback;
    pApp->m_pCallbackData = pLib->mpCallbackData = pData;
}

static SfxObjectShell* getSfxObjectShell(LibreOfficeKitDocument* pThis)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    if (!pDocument)
        return nullptr;

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(pDocument->mxComponent.get());
    if (!pBaseModel)
        return nullptr;

    return pBaseModel->GetObjectShell();
}

static int doc_saveAs(LibreOfficeKitDocument* pThis, const char* sUrl, const char* pFormat, const char* pFilterOptions)
{
    comphelper::ProfileZone aZone("doc_saveAs");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    OUString sFormat = getUString(pFormat);
    OUString aURL(getAbsoluteURL(sUrl));

    uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);

    if (aURL.isEmpty())
    {
        SetLastExceptionMsg(u"Filename to save to was not provided."_ustr);
        SAL_INFO("lok", "URL for save is empty");
        return false;
    }

    try
    {
        std::span<const ExtensionMap> pMap;

        switch (doc_getDocumentType(pThis))
        {
        case LOK_DOCTYPE_SPREADSHEET:
            pMap = aCalcExtensionMap;
            break;
        case LOK_DOCTYPE_PRESENTATION:
            pMap = aImpressExtensionMap;
            break;
        case LOK_DOCTYPE_DRAWING:
            pMap = aDrawExtensionMap;
            break;
        case LOK_DOCTYPE_TEXT:
            pMap = aWriterExtensionMap;
            break;
        case LOK_DOCTYPE_OTHER:
        default:
            SAL_INFO("lok", "Can't save document - unsupported document type.");
            return false;
        }

        if (pFormat == nullptr)
        {
            // sniff from the extension
            sal_Int32 idx = aURL.lastIndexOf(".");
            if( idx > 0 )
            {
                sFormat = aURL.copy( idx + 1 );
            }
            else
            {
                SetLastExceptionMsg("input URL '" + aURL + "' lacks a suffix");
                return false;
            }
        }

        OUString aFilterName;
        for (const auto& item : pMap)
        {
            if (sFormat.equalsIgnoreAsciiCaseAscii(item.extn))
            {
                aFilterName = item.filterName;
                break;
            }
        }
        if (aFilterName.isEmpty())
        {
            SetLastExceptionMsg(u"no output filter found for provided suffix"_ustr);
            return false;
        }

        OUString aFilterOptions = getUString(pFilterOptions);

        // Check if watermark for pdf is passed by filteroptions...
        // It is not a real filter option so it must be filtered out.
        OUString watermarkText;
        std::u16string_view sFullSheetPreview;
        int aIndex = -1;
        if ((aIndex = aFilterOptions.indexOf(",Watermark=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("WATERMARKEND");
            watermarkText = aFilterOptions.subView(aIndex+11, bIndex-(aIndex+11));
            aFilterOptions = OUString::Concat(aFilterOptions.subView(0, aIndex)) + aFilterOptions.subView(bIndex+12);
        }

        if ((aIndex = aFilterOptions.indexOf(",FullSheetPreview=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("FULLSHEETPREVEND");
            sFullSheetPreview = aFilterOptions.subView(aIndex+18, bIndex-(aIndex+18));
            aFilterOptions = OUString::Concat(aFilterOptions.subView(0, aIndex)) + aFilterOptions.subView(bIndex+16);
        }

        bool bFullSheetPreview = sFullSheetPreview == u"true";

        OUString filePassword;
        if ((aIndex = aFilterOptions.indexOf(",Password=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("PASSWORDEND");
            filePassword = aFilterOptions.subView(aIndex + 10, bIndex - (aIndex + 10));
            aFilterOptions = OUString::Concat(aFilterOptions.subView(0, aIndex))
                             + aFilterOptions.subView(bIndex + 11);
        }
        OUString filePasswordToModify;
        if ((aIndex = aFilterOptions.indexOf(",PasswordToModify=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("PASSWORDTOMODIFYEND");
            filePassword = aFilterOptions.subView(aIndex + 18, bIndex - (aIndex + 18));
            aFilterOptions = OUString::Concat(aFilterOptions.subView(0, aIndex))
                             + aFilterOptions.subView(bIndex + 19);
        }

        // Select a pdf version if specified a valid one. If not specified then ignore.
        // If invalid then fail.
        sal_Int32 pdfVer = 0;
        if ((aIndex = aFilterOptions.indexOf(",PDFVer=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("PDFVEREND");
            std::u16string_view sPdfVer = aFilterOptions.subView(aIndex+8, bIndex-(aIndex+8));
            aFilterOptions = OUString::Concat(aFilterOptions.subView(0, aIndex)) + aFilterOptions.subView(bIndex+9);

            if (o3tl::equalsIgnoreAsciiCase(sPdfVer, u"PDF/A-1b"))
                pdfVer = 1;
            else if (o3tl::equalsIgnoreAsciiCase(sPdfVer, u"PDF/A-2b"))
                pdfVer = 2;
            else if (o3tl::equalsIgnoreAsciiCase(sPdfVer, u"PDF/A-3b"))
                pdfVer = 3;
            else if (o3tl::equalsIgnoreAsciiCase(sPdfVer, u"PDF-1.5"))
                pdfVer = 15;
            else if (o3tl::equalsIgnoreAsciiCase(sPdfVer, u"PDF-1.6"))
                pdfVer = 16;
            else
            {
                SetLastExceptionMsg(u"wrong PDF version"_ustr);
                return false;
            }
        }

        // 'TakeOwnership' == this is a 'real' SaveAs (that is, the document
        // gets a new name).  When this is not provided, the meaning of
        // saveAs() is more like save-a-copy, which allows saving to any
        // random format like PDF or PNG.
        // It is not a real filter option, so we have to filter it out.
        const uno::Sequence<OUString> aOptionSeq = comphelper::string::convertCommaSeparated(aFilterOptions);
        std::vector<OUString> aFilteredOptionVec;
        bool bTakeOwnership = false;
        bool bCreateFromTemplate = false;
        MediaDescriptor aSaveMediaDescriptor;
        for (const auto& rOption : aOptionSeq)
        {
            if (rOption == "TakeOwnership")
                bTakeOwnership = true;
            else if (rOption == "NoFileSync")
                aSaveMediaDescriptor[u"NoFileSync"_ustr] <<= true;
            else if (rOption == "FromTemplate")
                bCreateFromTemplate = true;
            else
                aFilteredOptionVec.push_back(rOption);
        }

        if (bCreateFromTemplate && bTakeOwnership)
        {
            if (SfxObjectShell* pObjectShell = getSfxObjectShell(pThis))
            {
                DateTime now( ::DateTime::SYSTEM );
                pObjectShell->getDocProperties()->setCreationDate(now.GetUNODateTime());
            }
        }

        aSaveMediaDescriptor[u"Overwrite"_ustr] <<= true;
        aSaveMediaDescriptor[u"FilterName"_ustr] <<= aFilterName;

        auto aFilteredOptionSeq = comphelper::containerToSequence<OUString>(aFilteredOptionVec);
        aFilterOptions = comphelper::string::convertCommaSeparated(aFilteredOptionSeq);
        aSaveMediaDescriptor[MediaDescriptor::PROP_FILTEROPTIONS] <<= aFilterOptions;

        comphelper::SequenceAsHashMap aFilterDataMap;

        // If filter options is JSON string, then make sure aFilterDataMap stays empty, otherwise we
        // would ignore the filter options.
        if (!aFilterOptions.startsWith("{"))
        {
            setFormatSpecificFilterData(sFormat, aFilterDataMap);
        }

        if (!watermarkText.isEmpty())
            aFilterDataMap[u"TiledWatermark"_ustr] <<= watermarkText;

        if (bFullSheetPreview)
            aFilterDataMap[u"SinglePageSheets"_ustr] <<= true;

        if (pdfVer)
            aFilterDataMap[u"SelectPdfVersion"_ustr] <<= pdfVer;

        if (!aFilterDataMap.empty())
        {
            aSaveMediaDescriptor[u"FilterData"_ustr] <<= aFilterDataMap.getAsConstPropertyValueList();
        }
        if (!filePassword.isEmpty())
            aSaveMediaDescriptor[u"Password"_ustr] <<= filePassword;
        if (!filePasswordToModify.isEmpty())
            aSaveMediaDescriptor[u"PasswordToModify"_ustr] <<= filePasswordToModify;

        // add interaction handler too
        if (gImpl)
        {
            // gImpl does not have to exist when running from a unit test
            rtl::Reference<LOKInteractionHandler> const pInteraction(
                    new LOKInteractionHandler("saveas"_ostr, gImpl, pDocument));
            uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction);

            aSaveMediaDescriptor[MediaDescriptor::PROP_INTERACTIONHANDLER] <<= xInteraction;
        }


        if (bTakeOwnership)
            xStorable->storeAsURL(aURL, aSaveMediaDescriptor.getAsConstPropertyValueList());
        else
            xStorable->storeToURL(aURL, aSaveMediaDescriptor.getAsConstPropertyValueList());

        return true;
    }
    catch (const uno::Exception& exception)
    {
        SetLastExceptionMsg("exception: " + exception.Message);
    }
    return false;
}

/**
 * Initialize UNO commands, in the sense that from now on, the LOK client gets updates for status
 * changes of these commands. This is necessary, because (unlike in the desktop case) there are no
 * toolbars hosting widgets these UNO commands, so no such status updates would be sent to the
 * headless LOK clients out of the box.
 */
static void doc_iniUnoCommands ()
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    static constexpr OUString sUnoCommands[] =
    {
        u".uno:AlignLeft"_ustr,
        u".uno:AlignHorizontalCenter"_ustr,
        u".uno:AlignRight"_ustr,
        u".uno:BackColor"_ustr,
        u".uno:BackgroundColor"_ustr,
        u".uno:TableCellBackgroundColor"_ustr,
        u".uno:Bold"_ustr,
        u".uno:CenterPara"_ustr,
        u".uno:CharBackColor"_ustr,
        u".uno:CharBackgroundExt"_ustr,
        u".uno:CharFontName"_ustr,
        u".uno:Color"_ustr,
        u".uno:ControlCodes"_ustr,
        u".uno:DecrementIndent"_ustr,
        u".uno:DefaultBullet"_ustr,
        u".uno:DefaultNumbering"_ustr,
        u".uno:FontColor"_ustr,
        u".uno:FontHeight"_ustr,
        u".uno:IncrementIndent"_ustr,
        u".uno:Italic"_ustr,
        u".uno:JustifyPara"_ustr,
        u".uno:JumpToMark"_ustr,
        u".uno:OutlineFont"_ustr,
        u".uno:LeftPara"_ustr,
        u".uno:LanguageStatus"_ustr,
        u".uno:RightPara"_ustr,
        u".uno:Shadowed"_ustr,
        u".uno:SubScript"_ustr,
        u".uno:SuperScript"_ustr,
        u".uno:Strikeout"_ustr,
        u".uno:StyleApply"_ustr,
        u".uno:Underline"_ustr,
        u".uno:ModifiedStatus"_ustr,
        u".uno:Undo"_ustr,
        u".uno:Redo"_ustr,
        u".uno:InsertPage"_ustr,
        u".uno:DeletePage"_ustr,
        u".uno:DuplicatePage"_ustr,
        u".uno:InsertSlide"_ustr,
        u".uno:DeleteSlide"_ustr,
        u".uno:DuplicateSlide"_ustr,
        u".uno:ChangeTheme"_ustr,
        u".uno:Cut"_ustr,
        u".uno:Copy"_ustr,
        u".uno:Paste"_ustr,
        u".uno:SelectAll"_ustr,
        u".uno:ReplyComment"_ustr,
        u".uno:ResolveComment"_ustr,
        u".uno:ResolveCommentThread"_ustr,
        u".uno:InsertRowsBefore"_ustr,
        u".uno:InsertRowsAfter"_ustr,
        u".uno:InsertColumnsBefore"_ustr,
        u".uno:InsertColumnsAfter"_ustr,
        u".uno:DeleteRows"_ustr,
        u".uno:DeleteColumns"_ustr,
        u".uno:DeleteTable"_ustr,
        u".uno:SelectTable"_ustr,
        u".uno:EntireRow"_ustr,
        u".uno:EntireColumn"_ustr,
        u".uno:EntireCell"_ustr,
        u".uno:AssignLayout"_ustr,
        u".uno:StatusDocPos"_ustr,
        u".uno:RowColSelCount"_ustr,
        u".uno:StatusPageStyle"_ustr,
        u".uno:InsertMode"_ustr,
        u".uno:SpellOnline"_ustr,
        u".uno:StatusSelectionMode"_ustr,
        u".uno:StateTableCell"_ustr,
        u".uno:StatusBarFunc"_ustr,
        u".uno:StatePageNumber"_ustr,
        u".uno:StateWordCount"_ustr,
        u".uno:SelectionMode"_ustr,
        u".uno:PageStatus"_ustr,
        u".uno:LayoutStatus"_ustr,
        u".uno:Scale"_ustr,
        u".uno:Context"_ustr,
        u".uno:WrapText"_ustr,
        u".uno:ToggleMergeCells"_ustr,
        u".uno:NumberFormatCurrency"_ustr,
        u".uno:NumberFormatPercent"_ustr,
        u".uno:NumberFormatDecimal"_ustr,
        u".uno:NumberFormatIncDecimals"_ustr,
        u".uno:NumberFormatDecDecimals"_ustr,
        u".uno:NumberFormatDate"_ustr,
        u".uno:EditHeaderAndFooter"_ustr,
        u".uno:FrameLineColor"_ustr,
        u".uno:SortAscending"_ustr,
        u".uno:SortDescending"_ustr,
        u".uno:TrackChanges"_ustr,
        u".uno:ShowTrackedChanges"_ustr,
        u".uno:NextTrackedChange"_ustr,
        u".uno:PreviousTrackedChange"_ustr,
        u".uno:AcceptAllTrackedChanges"_ustr,
        u".uno:RejectAllTrackedChanges"_ustr,
        u".uno:TableDialog"_ustr,
        u".uno:FormatCellDialog"_ustr,
        u".uno:FontDialog"_ustr,
        u".uno:ParagraphDialog"_ustr,
        u".uno:OutlineBullet"_ustr,
        u".uno:InsertIndexesEntry"_ustr,
        u".uno:DocumentRepair"_ustr,
        u".uno:TransformDialog"_ustr,
        u".uno:InsertPageHeader"_ustr,
        u".uno:InsertPageFooter"_ustr,
        u".uno:OnlineAutoFormat"_ustr,
        u".uno:InsertObjectChart"_ustr,
        u".uno:InsertSection"_ustr,
        u".uno:InsertAnnotation"_ustr,
        u".uno:DeleteAnnotation"_ustr,
        u".uno:InsertPagebreak"_ustr,
        u".uno:InsertColumnBreak"_ustr,
        u".uno:HyperlinkDialog"_ustr,
        u".uno:InsertSymbol"_ustr,
        u".uno:EditRegion"_ustr,
        u".uno:ThesaurusDialog"_ustr,
        u".uno:FormatArea"_ustr,
        u".uno:FormatLine"_ustr,
        u".uno:FormatColumns"_ustr,
        u".uno:Watermark"_ustr,
        u".uno:ResetAttributes"_ustr,
        u".uno:Orientation"_ustr,
        u".uno:ObjectAlignLeft"_ustr,
        u".uno:ObjectAlignRight"_ustr,
        u".uno:AlignCenter"_ustr,
        u".uno:TransformPosX"_ustr,
        u".uno:TransformPosY"_ustr,
        u".uno:TransformWidth"_ustr,
        u".uno:TransformHeight"_ustr,
        u".uno:ObjectBackOne"_ustr,
        u".uno:SendToBack"_ustr,
        u".uno:ObjectForwardOne"_ustr,
        u".uno:BringToFront"_ustr,
        u".uno:WrapRight"_ustr,
        u".uno:WrapThrough"_ustr,
        u".uno:WrapLeft"_ustr,
        u".uno:WrapIdeal"_ustr,
        u".uno:WrapOn"_ustr,
        u".uno:WrapOff"_ustr,
        u".uno:UpdateCurIndex"_ustr,
        u".uno:InsertCaptionDialog"_ustr,
        u".uno:FormatGroup"_ustr,
        u".uno:SplitTable"_ustr,
        u".uno:SplitCell"_ustr,
        u".uno:MergeCells"_ustr,
        u".uno:DeleteNote"_ustr,
        u".uno:AcceptChanges"_ustr,
        u".uno:FormatPaintbrush"_ustr,
        u".uno:SetDefault"_ustr,
        u".uno:ParaLeftToRight"_ustr,
        u".uno:ParaRightToLeft"_ustr,
        u".uno:ParaspaceIncrease"_ustr,
        u".uno:ParaspaceDecrease"_ustr,
        u".uno:AcceptTrackedChange"_ustr,
        u".uno:RejectTrackedChange"_ustr,
        u".uno:AcceptTrackedChangeToNext"_ustr,
        u".uno:RejectTrackedChangeToNext"_ustr,
        u".uno:ShowResolvedAnnotations"_ustr,
        u".uno:InsertBreak"_ustr,
        u".uno:InsertEndnote"_ustr,
        u".uno:InsertFootnote"_ustr,
        u".uno:InsertReferenceField"_ustr,
        u".uno:InsertBookmark"_ustr,
        u".uno:InsertAuthoritiesEntry"_ustr,
        u".uno:InsertMultiIndex"_ustr,
        u".uno:InsertField"_ustr,
        u".uno:PageNumberWizard"_ustr,
        u".uno:InsertPageNumberField"_ustr,
        u".uno:InsertPageCountField"_ustr,
        u".uno:InsertDateField"_ustr,
        u".uno:InsertTitleField"_ustr,
        u".uno:InsertFieldCtrl"_ustr,
        u".uno:CharmapControl"_ustr,
        u".uno:EnterGroup"_ustr,
        u".uno:LeaveGroup"_ustr,
        u".uno:AlignUp"_ustr,
        u".uno:AlignMiddle"_ustr,
        u".uno:AlignDown"_ustr,
        u".uno:TraceChangeMode"_ustr,
        u".uno:Combine"_ustr,
        u".uno:Merge"_ustr,
        u".uno:Dismantle"_ustr,
        u".uno:Substract"_ustr,
        u".uno:DistributeSelection"_ustr,
        u".uno:Intersect"_ustr,
        u".uno:BorderInner"_ustr,
        u".uno:BorderOuter"_ustr,
        u".uno:FreezePanes"_ustr,
        u".uno:FreezePanesColumn"_ustr,
        u".uno:FreezePanesRow"_ustr,
        u".uno:Sidebar"_ustr,
        u".uno:SheetRightToLeft"_ustr,
        u".uno:RunMacro"_ustr,
        u".uno:SpacePara1"_ustr,
        u".uno:SpacePara15"_ustr,
        u".uno:SpacePara2"_ustr,
        u".uno:InsertSparkline"_ustr,
        u".uno:DeleteSparkline"_ustr,
        u".uno:DeleteSparklineGroup"_ustr,
        u".uno:EditSparklineGroup"_ustr,
        u".uno:EditSparkline"_ustr,
        u".uno:GroupSparklines"_ustr,
        u".uno:UngroupSparklines"_ustr,
        u".uno:FormatSparklineMenu"_ustr,
        u".uno:DataDataPilotRun"_ustr,
        u".uno:RecalcPivotTable"_ustr,
        u".uno:DeletePivotTable"_ustr,
        u".uno:Protect"_ustr,
        u".uno:UnsetCellsReadOnly"_ustr,
        u".uno:ContentControlProperties"_ustr,
        u".uno:InsertCheckboxContentControl"_ustr,
        u".uno:InsertContentControl"_ustr,
        u".uno:InsertDateContentControl"_ustr,
        u".uno:InsertDropdownContentControl"_ustr,
        u".uno:InsertPlainTextContentControl"_ustr,
        u".uno:InsertPictureContentControl"_ustr,
        u".uno:DataFilterAutoFilter"_ustr,
        u".uno:CellProtection"_ustr,
        u".uno:MoveKeepInsertMode"_ustr,
        u".uno:ToggleSheetGrid"_ustr,
    };

    util::URL aCommandURL;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell ? &pViewShell->GetViewFrame() : nullptr;

    // check if Frame-Controller were created.
    if (!pViewFrame)
    {
        SAL_WARN("lok", "iniUnoCommands: No Frame-Controller created.");
        return;
    }

    if (!xContext.is())
        xContext = comphelper::getProcessComponentContext();
    if (!xContext.is())
    {
        SAL_WARN("lok", "iniUnoCommands: Component context is not available");
        return;
    }

#if !defined IOS && !defined ANDROID && !defined __EMSCRIPTEN__
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xContext);
    if (!xSEInitializer.is())
    {
        SAL_WARN("lok", "iniUnoCommands: XSEInitializer is not available");
        return;
    }

    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext =
        xSEInitializer->createSecurityContext(OUString());
    if (!xSecurityContext.is())
    {
        SAL_WARN("lok", "iniUnoCommands: failed to create security context");
    }
#endif

    SfxSlotPool& rSlotPool = SfxSlotPool::GetSlotPool(pViewFrame);
    uno::Reference<util::XURLTransformer> xParser(util::URLTransformer::create(xContext));

    for (const auto & sUnoCommand : sUnoCommands)
    {
        aCommandURL.Complete = sUnoCommand;
        xParser->parseStrict(aCommandURL);

        // when null, this command is not supported by the given component
        // (like eg. Calc does not have ".uno:DefaultBullet" etc.)
        if (const SfxSlot* pSlot = rSlotPool.GetUnoSlot(aCommandURL.Path))
        {
            // Initialize slot to dispatch .uno: Command.
            pViewFrame->GetBindings().GetDispatch(pSlot, aCommandURL, false);
        }
    }
}

static int doc_getDocumentType (LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getDocumentType");

    SolarMutexGuard aGuard;
    return getDocumentType(pThis);
}

static int doc_getParts (LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getParts");

    SolarMutexGuard aGuard;

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return 0;
    }

    return pDoc->getParts();
}

static int doc_getPart (LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getPart");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return 0;
    }

    return pDoc->getPart();
}

static void doc_setPartImpl(LibreOfficeKitDocument* pThis, int nPart, bool bAllowChangeFocus = true)
{
    comphelper::ProfileZone aZone("doc_setPart");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->setPart( nPart, bAllowChangeFocus );
}

static void doc_setPart(LibreOfficeKitDocument* pThis, int nPart)
{
    doc_setPartImpl(pThis, nPart, true);
}

static char* doc_getPartInfo(LibreOfficeKitDocument* pThis, int nPart)
{
    comphelper::ProfileZone aZone("doc_getPartInfo");

    SolarMutexGuard aGuard;
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    return convertOUString(pDoc->getPartInfo(nPart));
}

static void doc_selectPart(LibreOfficeKitDocument* pThis, int nPart, int nSelect)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->selectPart( nPart, nSelect );
}

static void doc_moveSelectedParts(LibreOfficeKitDocument* pThis, int nPosition, bool bDuplicate)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->moveSelectedParts(nPosition, bDuplicate);
}

static char* doc_getPartPageRectangles(LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getPartPageRectangles");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    return convertOUString(pDoc->getPartPageRectangles());
}

static char* doc_getA11yFocusedParagraph(LibreOfficeKitDocument* pThis)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    if (SfxViewShell* pViewShell = SfxViewShell::Current())
    {
        return convertOUString(pViewShell->getA11yFocusedParagraph());

    }
    return nullptr;
}

static int  doc_getA11yCaretPosition(LibreOfficeKitDocument* pThis)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return -1;
    }
    if (SfxViewShell* pViewShell = SfxViewShell::Current())
    {
        return pViewShell->getA11yCaretPosition();

    }
    return -1;

}

static char* doc_getPartName(LibreOfficeKitDocument* pThis, int nPart)
{
    comphelper::ProfileZone aZone("doc_getPartName");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    return convertOUString(pDoc->getPartName(nPart));
}

static char* doc_getPartHash(LibreOfficeKitDocument* pThis, int nPart)
{
    comphelper::ProfileZone aZone("doc_getPartHash");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    return convertOUString(pDoc->getPartHash(nPart));
}

static void doc_setPartMode(LibreOfficeKitDocument* pThis,
                            int nPartMode)
{
    comphelper::ProfileZone aZone("doc_setPartMode");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }


    int nCurrentPart = pDoc->getPart();

    pDoc->setPartMode(nPartMode);

    // We need to make sure the internal state is updated, just changing the mode
    // might not update the relevant shells (i.e. impress will keep rendering the
    // previous mode unless we do this).
    // TODO: we might want to do this within the relevant components rather than
    // here, but that's also dependent on how we implement embedded object
    // rendering I guess?
    // TODO: we could be clever and e.g. set to 0 when we change to/from
    // embedded object mode, and not when changing between slide/notes/combined
    // modes?
    if ( nCurrentPart < pDoc->getParts() )
    {
        pDoc->setPart( nCurrentPart );
    }
    else
    {
        pDoc->setPart( 0 );
    }
}

static int doc_getEditMode(LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getEditMode");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return 0;
    }

    return pDoc->getEditMode();
}

static void doc_paintTile(LibreOfficeKitDocument* pThis,
                          unsigned char* pBuffer,
                          const int nCanvasWidth, const int nCanvasHeight,
                          const int nTilePosX, const int nTilePosY,
                          const int nTileWidth, const int nTileHeight)
{
    comphelper::ProfileZone aZone("doc_paintTile");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SAL_INFO( "lok.tiledrendering", "paintTile: painting [" << nTileWidth << "x" << nTileHeight <<
              "]@(" << nTilePosX << ", " << nTilePosY << ") to [" <<
              nCanvasWidth << "x" << nCanvasHeight << "]px" );

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

#if defined(UNX) && !defined(MACOSX) || defined(_WIN32)

    // Painting of zoomed or HiDPI spreadsheets is special, we actually draw everything at 100%,
    // and only set cairo's (or CoreGraphic's, in the iOS case) scale factor accordingly, so that
    // everything is painted bigger or smaller. This is different to what Calc's internal scaling
    // would do - because that one is trying to fit the lines between cells to integer multiples of
    // pixels.
    comphelper::ScopeGuard dpiScaleGuard([]() { comphelper::LibreOfficeKit::setDPIScale(1.0); });

#if defined(IOS)
    double fDPIScale = 1.0;

    // Onine uses the LOK_TILEMODE_RGBA by default so flip the normal flags
    // to kCGImageAlphaPremultipliedLast | kCGImageByteOrder32Big
    CGContextRef pCGContext = CGBitmapContextCreate(pBuffer, nCanvasWidth, nCanvasHeight, 8,
                                                    nCanvasWidth * 4, CGColorSpaceCreateDeviceRGB(),
                                                    kCGImageAlphaPremultipliedLast | kCGImageByteOrder32Big);

    CGContextTranslateCTM(pCGContext, 0, nCanvasHeight);
    CGContextScaleCTM(pCGContext, fDPIScale, -fDPIScale);

    SAL_INFO( "lok.tiledrendering", "doc_paintTile: painting [" << nTileWidth << "x" << nTileHeight <<
              "]@(" << nTilePosX << ", " << nTilePosY << ") to [" <<
              nCanvasWidth << "x" << nCanvasHeight << "]px" );

    Size aCanvasSize(nCanvasWidth, nCanvasHeight);

    SystemGraphicsData aData;
    aData.rCGContext = reinterpret_cast<CGContextRef>(pCGContext);

    ScopedVclPtrInstance<VirtualDevice> pDevice(aData, Size(1, 1), DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixel(aCanvasSize);
    pDoc->paintTile(*pDevice, aCanvasSize.Width(), aCanvasSize.Height(),
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    CGContextRelease(pCGContext);
#else
    ScopedVclPtrInstance< VirtualDevice > pDevice(DeviceFormat::WITHOUT_ALPHA);

    // Set background to transparent by default.
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(
                Size(nCanvasWidth, nCanvasHeight), Fraction(1.0), Point(),
                pBuffer);

    pDoc->paintTile(*pDevice, nCanvasWidth, nCanvasHeight,
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    static bool bDebug = getenv("LOK_DEBUG_TILES") != nullptr;
    if (bDebug)
    {
        // Draw a small red rectangle in the top left corner so that it's easy to see where a new tile begins.
        tools::Rectangle aRect(0, 0, 5, 5);
        aRect = pDevice->PixelToLogic(aRect);
        pDevice->Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
        pDevice->SetFillColor(COL_LIGHTRED);
        pDevice->SetLineColor();
        pDevice->DrawRect(aRect);
        pDevice->Pop();
    }

#ifdef _WIN32
    // pBuffer was not used there
    pDevice->EnableMapMode(false);
    BitmapEx aBmpEx = pDevice->GetBitmapEx({ 0, 0 }, { nCanvasWidth, nCanvasHeight });
    Bitmap aBmp = aBmpEx.GetBitmap();
    AlphaMask aAlpha = aBmpEx.GetAlphaMask();
    BitmapScopedReadAccess sraBmp(aBmp);
    BitmapScopedReadAccess sraAlpha(aAlpha);

    assert(sraBmp->Height() == nCanvasHeight);
    assert(sraBmp->Width() == nCanvasWidth);
    assert(!sraAlpha || sraBmp->Height() == sraAlpha->Height());
    assert(!sraAlpha || sraBmp->Width() == sraAlpha->Width());
    auto p = pBuffer;
    for (tools::Long y = 0; y < sraBmp->Height(); ++y)
    {
        Scanline dataBmp = sraBmp->GetScanline(y);
        Scanline dataAlpha = sraAlpha ? sraAlpha->GetScanline(y) : nullptr;
        for (tools::Long x = 0; x < sraBmp->Width(); ++x)
        {
            BitmapColor color = sraBmp->GetPixelFromData(dataBmp, x);
            sal_uInt8 alpha = dataAlpha ? sraAlpha->GetPixelFromData(dataAlpha, x).GetBlue() : 255;
            *p++ = color.GetBlue();
            *p++ = color.GetGreen();
            *p++ = color.GetRed();
            *p++ = alpha;
        }
    }
#endif
#endif

#else
    (void) pBuffer;
#endif
}

static void doc_paintPartTile(LibreOfficeKitDocument* pThis,
                              unsigned char* pBuffer,
                              const int nPart,
                              const int nMode,
                              const int nCanvasWidth, const int nCanvasHeight,
                              const int nTilePosX, const int nTilePosY,
                              const int nTileWidth, const int nTileHeight)
{
    comphelper::ProfileZone aZone("doc_paintPartTile");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SAL_INFO( "lok.tiledrendering", "paintPartTile: painting @ " << nPart << " : " << nMode << " ["
               << nTileWidth << "x" << nTileHeight << "]@("
               << nTilePosX << ", " << nTilePosY << ") to ["
               << nCanvasWidth << "x" << nCanvasHeight << "]px" );

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    int nOrigViewId = doc_getView(pThis);

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    if (nOrigViewId < 0)
    {
        // tile painting always needs a SfxViewShell::Current(), but actually
        // it does not really matter which one - all of them should paint the
        // same thing. It's important to get a view for the correct document,
        // though.
        // doc_getViewsCount() returns the count of views for the document in the current view.
        int viewCount = doc_getViewsCount(pThis);
        if (viewCount == 0)
            return;

        std::vector<int> viewIds(viewCount);
        doc_getViewIds(pThis, viewIds.data(), viewCount);

        nOrigViewId = viewIds[0];
        doc_setView(pThis, nOrigViewId);
    }

    // Disable callbacks while we are painting.
    if (nOrigViewId >= 0)
    {
        const auto handlerIt = pDocument->mpCallbackFlushHandlers.find(nOrigViewId);
        if (handlerIt != pDocument->mpCallbackFlushHandlers.end())
            handlerIt->second->disableCallbacks();
    }

    try
    {
        // Text documents have a single coordinate system; don't change part.
        int nOrigPart = 0;
        const int aType = doc_getDocumentType(pThis);
        const bool isText = (aType == LOK_DOCTYPE_TEXT);
        const bool isCalc = (aType == LOK_DOCTYPE_SPREADSHEET);
        int nOrigEditMode = 0;
        bool bPaintTextEdit = true;
        int nViewId = nOrigViewId;
        int nLastNonEditorView = -1;
        int nViewMatchingMode = -1;
        SfxViewShell* pCurrentViewShell = SfxViewShell::Current();

        if (!isText)
        {
            // Check if just switching to another view is enough, that has
            // less side-effects.
            if (nPart != doc_getPart(pThis) || nMode != pDoc->getEditMode())
            {
                SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                while (pViewShell)
                {
                    bool bIsInEdit = pViewShell->GetDrawView() &&
                        pViewShell->GetDrawView()->GetTextEditOutliner();

                    OString sCurrentViewRenderState = pDoc->getViewRenderState(pCurrentViewShell);
                    OString sNewRenderState = pDoc->getViewRenderState(pViewShell);

                    if (sCurrentViewRenderState == sNewRenderState && !bIsInEdit)
                        nLastNonEditorView = pViewShell->GetViewShellId().get();

                    if (pViewShell->getPart() == nPart &&
                        pViewShell->getEditMode() == nMode &&
                        sCurrentViewRenderState == sNewRenderState &&
                        !bIsInEdit)
                    {
                        nViewId = pViewShell->GetViewShellId().get();
                        nViewMatchingMode = nViewId;
                        nLastNonEditorView = nViewId;
                        doc_setView(pThis, nViewId);
                        break;
                    }
                    else if (pViewShell->getEditMode() == nMode && sCurrentViewRenderState == sNewRenderState && !bIsInEdit)
                    {
                        nViewMatchingMode = pViewShell->GetViewShellId().get();
                    }

                    pViewShell = SfxViewShell::GetNext(*pViewShell);
                }
            }

            // if not found view with correct part
            // - at least avoid rendering active textbox, This is for Impress.
            // - prefer view with the same mode
            if (nViewMatchingMode >= 0 && nViewMatchingMode != nViewId)
            {
                nViewId = nViewMatchingMode;
                doc_setView(pThis, nViewId);
            }
            else if (!isCalc && nLastNonEditorView >= 0 && nLastNonEditorView != nViewId &&
                pCurrentViewShell && pCurrentViewShell->GetDrawView() &&
                pCurrentViewShell->GetDrawView()->GetTextEditOutliner())
            {
                nViewId = nLastNonEditorView;
                doc_setView(pThis, nViewId);
            }

            // Disable callbacks while we are painting - after setting the view
            if (nViewId != nOrigViewId && nViewId >= 0)
            {
                const auto handlerIt = pDocument->mpCallbackFlushHandlers.find(nViewId);
                if (handlerIt != pDocument->mpCallbackFlushHandlers.end())
                    handlerIt->second->disableCallbacks();
            }

            nOrigPart = doc_getPart(pThis);
            if (nPart != nOrigPart)
            {
                doc_setPartImpl(pThis, nPart, false);
            }

            nOrigEditMode = pDoc->getEditMode();
            if (nOrigEditMode != nMode)
            {
                SfxLokHelper::setEditMode(nMode, pDoc);
            }

            bPaintTextEdit = (nPart == nOrigPart && nMode == nOrigEditMode);
            pDoc->setPaintTextEdit(bPaintTextEdit);
        }

        doc_paintTile(pThis, pBuffer, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

        if (!isText)
        {
            pDoc->setPaintTextEdit(true);

            if (nMode != nOrigEditMode)
            {
                SfxLokHelper::setEditMode(nOrigEditMode, pDoc);
            }

            if (nPart != nOrigPart)
            {
                doc_setPartImpl(pThis, nOrigPart, false);
            }

            if (nViewId != nOrigViewId)
            {
                if (nViewId >= 0)
                {
                    const auto handlerIt = pDocument->mpCallbackFlushHandlers.find(nViewId);
                    if (handlerIt != pDocument->mpCallbackFlushHandlers.end())
                        handlerIt->second->enableCallbacks();
                }

                doc_setView(pThis, nOrigViewId);
            }
        }
    }
    catch (const std::exception&)
    {
        // Nothing to do but restore the PartTilePainting flag.
    }

    if (nOrigViewId >= 0)
    {
        const auto handlerIt = pDocument->mpCallbackFlushHandlers.find(nOrigViewId);
        if (handlerIt != pDocument->mpCallbackFlushHandlers.end())
            handlerIt->second->enableCallbacks();
    }
}

static int doc_getTileMode(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/)
{
    SetLastExceptionMsg();
#if ENABLE_CAIRO_RGBA || defined IOS
    return LOK_TILEMODE_RGBA;
#else
    return LOK_TILEMODE_BGRA;
#endif
}

static void doc_getDocumentSize(LibreOfficeKitDocument* pThis,
                                long* pWidth,
                                long* pHeight)
{
    comphelper::ProfileZone aZone("doc_getDocumentSize");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (pDoc)
    {
        Size aDocumentSize = pDoc->getDocumentSize();
        *pWidth = aDocumentSize.Width();
        *pHeight = aDocumentSize.Height();
    }
    else
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
    }
}

static void doc_getDataArea(LibreOfficeKitDocument* pThis,
                            long nTab,
                            long* pCol,
                            long* pRow)
{
    comphelper::ProfileZone aZone("doc_getDataArea");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (pDoc)
    {
        Size aDocumentSize = pDoc->getDataArea(nTab);
        *pCol = aDocumentSize.Width();
        *pRow = aDocumentSize.Height();
    }
    else
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
    }
}

static void doc_initializeForRendering(LibreOfficeKitDocument* pThis,
                                       const char* pArguments)
{
    comphelper::ProfileZone aZone("doc_initializeForRendering");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (pDoc)
    {
        doc_iniUnoCommands();
        pDoc->initializeForTiledRendering(
                comphelper::containerToSequence(jsonToPropertyValuesVector(pArguments)));
    }
}

static void doc_registerCallback(LibreOfficeKitDocument* pThis,
                                 LibreOfficeKitCallback pCallback,
                                 void* pData)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    const int nView = SfxLokHelper::getView();
    if (nView < 0)
        return;

    const size_t nId = nView;
    if (pCallback != nullptr)
    {
        for (auto& pair : pDocument->mpCallbackFlushHandlers)
        {
            if (pair.first == nId)
                continue;

            pair.second->addViewStates(nView);
        }
    }
    else
    {
        for (auto& pair : pDocument->mpCallbackFlushHandlers)
        {
            if (pair.first == nId)
                continue;

            pair.second->removeViewStates(nView);
        }
    }

    pDocument->mpCallbackFlushHandlers[nView] = std::make_shared<CallbackFlushHandler>(pThis, pCallback, pData);

    if (pCallback != nullptr)
    {
        for (const auto& pair : pDocument->mpCallbackFlushHandlers)
        {
            if (pair.first == nId)
                continue;

            pDocument->mpCallbackFlushHandlers[nView]->addViewStates(pair.first);
        }

        if (SfxViewShell* pViewShell = SfxViewShell::Current())
        {
            pDocument->mpCallbackFlushHandlers[nView]->setViewId(pViewShell->GetViewShellId().get());
            pViewShell->setLibreOfficeKitViewCallback(pDocument->mpCallbackFlushHandlers[nView].get());
        }

        if (!pDocument->maFontsMissing.empty())
        {
            OString sPayload = "{ \"fontsmissing\": [ "_ostr;
            bool bFirst = true;
            for (const auto &f : pDocument->maFontsMissing)
            {
                if (bFirst)
                    bFirst = false;
                else
                    sPayload += ", ";
                sPayload += "\"" + f.toUtf8() + "\"";
            }
            sPayload += " ] }";
            pCallback(LOK_CALLBACK_FONTS_MISSING, sPayload.getStr(), pData);
            pDocument->maFontsMissing.clear();
        }
    }
    else
    {
        if (SfxViewShell* pViewShell = SfxViewShell::Current())
        {
            pViewShell->setLibreOfficeKitViewCallback(nullptr);
            pDocument->mpCallbackFlushHandlers[nView]->setViewId(-1);
        }
    }
}

/// Returns the JSON representation of all the comments in the document
static char* getPostIts(LibreOfficeKitDocument* pThis)
{
    SetLastExceptionMsg();
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getPostIts(aJsonWriter);
    return convertOString(aJsonWriter.finishAndGetAsOString());
}

/// Returns the JSON representation of the positions of all the comments in the document
static char* getPostItsPos(LibreOfficeKitDocument* pThis)
{
    SetLastExceptionMsg();
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getPostItsPos(aJsonWriter);
    return convertOString(aJsonWriter.finishAndGetAsOString());
}

static char* getRulerState(LibreOfficeKitDocument* pThis)
{
    SetLastExceptionMsg();
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getRulerState(aJsonWriter);
    return convertOString(aJsonWriter.finishAndGetAsOString());
}

static void doc_postKeyEvent(LibreOfficeKitDocument* pThis, int nType, int nCharCode, int nKeyCode)
{
    comphelper::ProfileZone aZone("doc_postKeyEvent");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    try
    {
        pDoc->postKeyEvent(nType, nCharCode, nKeyCode);
    }
    catch (const uno::Exception& exception)
    {
        SetLastExceptionMsg(exception.Message);
        SAL_INFO("lok", "Failed to postKeyEvent " << exception.Message);
    }
}

static void doc_setBlockedCommandList(LibreOfficeKitDocument* /*pThis*/, int nViewId, const char* blockedCommandList)
{
    SolarMutexGuard aGuard;
    SfxLokHelper::setBlockedCommandList(nViewId, blockedCommandList);
}

static void doc_postWindowExtTextInputEvent(LibreOfficeKitDocument* pThis, unsigned nWindowId, int nType, const char* pText)
{
    comphelper::ProfileZone aZone("doc_postWindowExtTextInputEvent");

    SolarMutexGuard aGuard;
    VclPtr<vcl::Window> pWindow;
    if (nWindowId == 0)
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
            return;
        }
        pWindow = pDoc->getDocWindow();
    }
    else
    {
        pWindow = vcl::Window::FindLOKWindow(nWindowId);
    }

    if (!pWindow)
    {
        SetLastExceptionMsg("No window found for window id: " + OUString::number(nWindowId));
        return;
    }

    SfxLokHelper::postExtTextEventAsync(pWindow, nType, OUString::fromUtf8(std::string_view(pText, strlen(pText))));
}

static void doc_removeTextContext(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId, int nCharBefore, int nCharAfter)
{
    SolarMutexGuard aGuard;

    if (SfxViewShell::IsCurrentLokViewReadOnly())
        return;

    VclPtr<vcl::Window> pWindow;
    if (nLOKWindowId == 0)
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
            return;
        }
        pWindow = pDoc->getDocWindow();
    }
    else
    {
        pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    }

    if (!pWindow)
    {
        SetLastExceptionMsg("No window found for window id: " + OUString::number(nLOKWindowId));
        return;
    }

    // Annoyingly - backspace and delete are handled in the apps via an accelerator
    // which are PostMessage'd by SfxViewShell::ExecKey_Impl so to stay in the same
    // order we do this synchronously here, unless we're in a dialog.
    if (nCharBefore > 0)
    {
        // backspace
        if (nLOKWindowId == 0)
        {
            KeyEvent aEvt(8, KEY_BACKSPACE);
            for (int i = 0; i < nCharBefore; ++i)
                pWindow->KeyInput(aEvt);
        }
        else
            SfxLokHelper::postKeyEventAsync(pWindow, LOK_KEYEVENT_KEYINPUT, 8, KEY_BACKSPACE, nCharBefore - 1);
    }

    if (nCharAfter > 0)
    {
        // delete (forward)
        if (nLOKWindowId == 0)
        {
            KeyEvent aEvt(46, KEY_DELETE);
            for (int i = 0; i < nCharAfter; ++i)
                pWindow->KeyInput(aEvt);
        }
        else
            SfxLokHelper::postKeyEventAsync(pWindow, LOK_KEYEVENT_KEYINPUT, 46, KEY_DELETE, nCharAfter - 1);
    }
}

static void doc_postWindowKeyEvent(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId, int nType, int nCharCode, int nKeyCode)
{
    comphelper::ProfileZone aZone("doc_postWindowKeyEvent");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
        return;
    }

    KeyEvent aEvent(nCharCode, nKeyCode, 0);

    switch (nType)
    {
        case LOK_KEYEVENT_KEYINPUT:
            Application::PostKeyEvent(VclEventId::WindowKeyInput, pWindow, &aEvent);
            break;
        case LOK_KEYEVENT_KEYUP:
            Application::PostKeyEvent(VclEventId::WindowKeyUp, pWindow, &aEvent);
            break;
        default:
            assert(false);
            break;
    }
}

// To be an exportable selection, there must be something selected and that
// selection can't be "ScCellObj" which doesn't can't provide a svg.
//
// Typically a problem arises when double clicking a shape in calc. The 1st
// click selects the shape, triggering generation of a preview, but the second
// shape enters into edit mode before doc_renderShapeSelection has a chance to
// fire, at which point the shape is no longer selected. Rather than generate
// an error just return a 0 length result if there is no shape selected, so we
// continue to generate an error if a shape is selected, but could not provide
// an svg.
static bool doc_hasShapeSelection(const css::uno::Reference<css::lang::XComponent>& rComponent)
{
    uno::Reference<frame::XModel> xModel(rComponent, uno::UNO_QUERY);
    if (!xModel.is())
        return false;

    uno::Reference<frame::XController> xController(xModel->getCurrentController());
    if (!xController.is())
        return false;

    uno::Reference<view::XSelectionSupplier> xSelectionSupplier(xController, uno::UNO_QUERY);
    if (!xSelectionSupplier.is())
        return false;

    Any selection = xSelectionSupplier->getSelection();
    uno::Reference<lang::XServiceInfo> xSelection;
    selection >>= xSelection;

    return xSelection && xSelection->getImplementationName() != "ScCellObj";
}

static size_t doc_renderShapeSelection(LibreOfficeKitDocument* pThis, char** pOutput)
{
    comphelper::ProfileZone aZone("doc_renderShapeSelection");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LokChartHelper aChartHelper(SfxViewShell::Current());

    if (aChartHelper.GetWindow())
        return 0;

    try
    {
        LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

        if (!doc_hasShapeSelection(pDocument->mxComponent))
            return 0;

        uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);

        SvMemoryStream aOutStream;
        uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aOutStream);

        utl::MediaDescriptor aMediaDescriptor;
        switch (doc_getDocumentType(pThis))
        {
            case LOK_DOCTYPE_PRESENTATION:
                aMediaDescriptor[u"FilterName"_ustr] <<= u"impress_svg_Export"_ustr;
                break;
            case LOK_DOCTYPE_DRAWING:
                aMediaDescriptor[u"FilterName"_ustr] <<= u"draw_svg_Export"_ustr;
                break;
            case LOK_DOCTYPE_TEXT:
                aMediaDescriptor[u"FilterName"_ustr] <<= u"writer_svg_Export"_ustr;
                break;
            case LOK_DOCTYPE_SPREADSHEET:
                aMediaDescriptor[u"FilterName"_ustr] <<= u"calc_svg_Export"_ustr;
                break;
            default:
                SAL_WARN("lok", "Failed to render shape selection: Document type is not supported");
        }
        aMediaDescriptor[u"SelectionOnly"_ustr] <<= true;
        aMediaDescriptor[u"OutputStream"_ustr] <<= xOut;
        aMediaDescriptor[u"IsPreview"_ustr] <<= true; // will down-scale graphics

        xStorable->storeToURL(u"private:stream"_ustr, aMediaDescriptor.getAsConstPropertyValueList());

        if (pOutput)
        {
            const size_t nOutputSize = aOutStream.GetEndOfData();
            *pOutput = static_cast<char*>(malloc(nOutputSize));
            if (*pOutput)
            {
                std::memcpy(*pOutput, aOutStream.GetData(), nOutputSize);
                return nOutputSize;
            }
        }
    }
    catch (const uno::Exception& exception)
    {
        css::uno::Any exAny( cppu::getCaughtException() );
        SetLastExceptionMsg(exception.Message);
        SAL_WARN("lok", "Failed to render shape selection: " << exceptionToString(exAny));
    }

    return 0;
}

namespace {

/** Class to react on finishing of a dispatched command.

    This will call a LOK_COMMAND_FINISHED callback when postUnoCommand was
    called with the parameter requesting the notification.

    @see LibreOfficeKitCallbackType::LOK_CALLBACK_UNO_COMMAND_RESULT.
*/
class DispatchResultListener : public cppu::WeakImplHelper<css::frame::XDispatchResultListener>
{
    const OString maCommand; ///< Command for which this is the result.
    const std::shared_ptr<CallbackFlushHandler> mpCallback; ///< Callback to call.
    const std::chrono::steady_clock::time_point mSaveTime; //< The time we started saving.
    const bool mbWasModified; //< Whether or not the document was modified before saving.

public:
    DispatchResultListener(const char* pCommand, std::shared_ptr<CallbackFlushHandler> pCallback,
                           bool bWasModified)
        : maCommand(pCommand)
        , mpCallback(std::move(pCallback))
        , mSaveTime(std::chrono::steady_clock::now())
        , mbWasModified(bWasModified)
    {
        assert(mpCallback);
    }

    virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& rEvent) override
    {
        tools::JsonWriter aJson;
        aJson.put("commandName", maCommand);

        if (rEvent.State != frame::DispatchResultState::DONTKNOW)
        {
            bool bSuccess = (rEvent.State == frame::DispatchResultState::SUCCESS);
            aJson.put("success", bSuccess);
        }

        unoAnyToJson(aJson, "result", rEvent.Result);
        aJson.put("wasModified", mbWasModified);
        aJson.put("startUnixTimeMics",
                  std::chrono::time_point_cast<std::chrono::microseconds>(mSaveTime)
                      .time_since_epoch()
                      .count());
        aJson.put("saveDurationMics", std::chrono::duration_cast<std::chrono::microseconds>(
                                          std::chrono::steady_clock::now() - mSaveTime)
                                          .count());
        mpCallback->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.finishAndGetAsOString());
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override {}
};

} // anonymous namespace


static void lcl_sendDialogEvent(unsigned long long int nWindowId, const char* pArguments)
{
    SolarMutexGuard aGuard;

    StringMap aMap(jsdialog::jsonToStringMap(pArguments));

    if (aMap.find(u"id"_ustr) == aMap.end())
        return;

    sal_uInt64 nCurrentShellId = reinterpret_cast<sal_uInt64>(SfxViewShell::Current());

    try
    {
        OUString sControlId = aMap[u"id"_ustr];
        OUString sWindowId = OUString::number(nWindowId);
        OUString sCurrentShellId = OUString::number(nCurrentShellId);

        // special values for window id
        if (nWindowId == static_cast<unsigned long long int>(-1))
            sWindowId = sCurrentShellId + "sidebar";
        if (nWindowId == static_cast<unsigned long long int>(-2))
            sWindowId = sCurrentShellId + "notebookbar";
        if (nWindowId == static_cast<unsigned long long int>(-3))
            sWindowId = sCurrentShellId + "formulabar";

        // dialogs send own id but notebookbar and sidebar controls are remembered by SfxViewShell id
        if (jsdialog::ExecuteAction(sWindowId, sControlId, aMap))
            return;

        if (jsdialog::ExecuteAction(sCurrentShellId + "sidebar", sControlId, aMap))
            return;
        if (jsdialog::ExecuteAction(sCurrentShellId + "notebookbar", sControlId, aMap))
            return;
        if (jsdialog::ExecuteAction(sCurrentShellId + "formulabar", sControlId, aMap))
            return;
        // this is needed for dialogs shown before document is loaded: MacroWarning dialog, etc...
        // these dialogs are created with WindowId "0"
        if (!SfxViewShell::Current() && jsdialog::ExecuteAction(u"0"_ustr, sControlId, aMap))
            return;

        // force resend - used in mobile-wizard
        jsdialog::SendFullUpdate(sCurrentShellId + "sidebar", u"Panel"_ustr);

    } catch(...) {}
}


static void doc_sendDialogEvent(LibreOfficeKitDocument* /*pThis*/, unsigned long long int nWindowId, const char* pArguments)
{
    lcl_sendDialogEvent(nWindowId, pArguments);
}

static void lo_sendDialogEvent(LibreOfficeKit* /*pThis*/, unsigned long long int nWindowId, const char* pArguments)
{
    lcl_sendDialogEvent(nWindowId, pArguments);
}

static void lo_setOption(LibreOfficeKit* /*pThis*/, const char *pOption, const char* pValue)
{
    static char* pCurrentSalLogOverride = nullptr;

    if (strcmp(pOption, "traceeventrecording") == 0)
    {
        if (strcmp(pValue, "start") == 0)
        {
            comphelper::TraceEvent::setBufferSizeAndCallback(100, TraceEventDumper::flushRecordings);
            comphelper::TraceEvent::startRecording();
            if (traceEventDumper == nullptr)
                traceEventDumper = new TraceEventDumper();
        }
        else if (strcmp(pValue, "stop") == 0)
            comphelper::TraceEvent::stopRecording();
    }
    else if (strcmp(pOption, "sallogoverride") == 0)
    {
        if (pCurrentSalLogOverride != nullptr)
            free(pCurrentSalLogOverride);
        if (pValue == nullptr)
            pCurrentSalLogOverride = nullptr;
        else
            pCurrentSalLogOverride = strdup(pValue);

        if (pCurrentSalLogOverride == nullptr || pCurrentSalLogOverride[0] == '\0')
            sal_detail_set_log_selector(nullptr);
        else
            sal_detail_set_log_selector(pCurrentSalLogOverride);
    }
#ifdef LINUX
    else if (strcmp(pOption, "addfont") == 0)
    {
        if (memcmp(pValue, "file://", 7) == 0)
            pValue += 7;

        int fd = open(pValue, O_RDONLY);
        if (fd == -1)
        {
            std::cerr << "Could not open font file '" << pValue << "': " << strerror(errno) << std::endl;
            return;
        }

        OUString sMagicFileName = "file:///:FD:/" + OUString::number(fd);

        OutputDevice *pDevice = Application::GetDefaultDevice();
        OutputDevice::ImplClearAllFontData(false);
        pDevice->AddTempDevFont(sMagicFileName, u""_ustr);
        OutputDevice::ImplRefreshAllFontData(false);
    }
#endif
}

static void lo_dumpState (LibreOfficeKit* pThis, const char* /* pOptions */, char** pState)
{
    if (!pState)
        return;

    // NB. no SolarMutexGuard since this may be caused in some extremis / deadlock
    SetLastExceptionMsg();

    *pState = nullptr;
    OStringBuffer aState(4096*256);

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    pLib->dumpState(aState);

    *pState = convertOString(aState.makeStringAndClear());
}

void LibLibreOffice_Impl::dumpState(rtl::OStringBuffer &rState)
{
    rState.append("LibreOfficeKit state:"
                  "\n\tLastExceptionMsg:\t");
    rState.append(rtl::OUStringToOString(maLastExceptionMsg, RTL_TEXTENCODING_UTF8));
    rState.append("\n\tUnipoll:\t");
    rState.append(vcl::lok::isUnipoll() ? "yes" : "no: events on thread");
    rState.append("\n\tOptionalFeatures:\t0x");
    rState.append(static_cast<sal_Int64>(mOptionalFeatures), 16);
    rState.append("\n\tCallbackData:\t0x");
    rState.append(reinterpret_cast<sal_Int64>(mpCallback), 16);
    // TODO: dump mInteractionMap
    SfxLokHelper::dumpState(rState);
    vcl::lok::dumpState(rState);
}

// We have special handling for some uno commands and it seems we need to check for readonly state.
static bool isCommandAllowed(OUString& command) {
    static constexpr OUString nonAllowedList[] = { u".uno:Save"_ustr, u".uno:TransformDialog"_ustr, u".uno:SidebarShow"_ustr, u".uno:SidebarHide"_ustr };

    if (!SfxViewShell::IsCurrentLokViewReadOnly())
        return true;
    else
    {
        if (command == u".uno:Save"_ustr && SfxViewShell::Current() && SfxViewShell::Current()->IsAllowChangeComments())
            return true;

        for (size_t i = 0; i < std::size(nonAllowedList); i++)
        {
            if (nonAllowedList[i] == command)
                return false;
        }
        return true;
    }
}

static void doc_postUnoCommand(LibreOfficeKitDocument* pThis, const char* pCommand, const char* pArguments, bool bNotifyWhenFinished)
{
    comphelper::ProfileZone aZone("doc_postUnoCommand");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    OUString aCommand(pCommand, strlen(pCommand), RTL_TEXTENCODING_UTF8);

    if (!isCommandAllowed(aCommand))
        return;

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    std::vector<beans::PropertyValue> aPropertyValuesVector(jsonToPropertyValuesVector(pArguments));

    if (!vcl::lok::isUnipoll())
    {
        beans::PropertyValue aSynchronMode;
        aSynchronMode.Name = u"SynchronMode"_ustr;
        aSynchronMode.Value <<= false;
        aPropertyValuesVector.push_back(aSynchronMode);
    }

    int nView = SfxLokHelper::getView();
    if (nView < 0)
        return;

    if (gImpl && aCommand == ".uno:ToggleOrientation")
    {
        ExecuteOrientationChange();
        return;
    }

    // handle potential interaction
    if (gImpl && aCommand == ".uno:Save")
    {
        // Check if saving a PDF file
        OUString aMimeType = lcl_getCurrentDocumentMimeType(pDocument);
        if (pDocSh && pDocSh->IsModified() && aMimeType == "application/pdf")
        {
            // If we have a PDF file (for saving annotations for example), we need
            // to run save-as to the same file as the opened document. Plain save
            // doesn't work as the PDF is not a "native" format.
            uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);
            OUString aURL = xStorable->getLocation();
            OString aURLUtf8 = OUStringToOString(aURL, RTL_TEXTENCODING_UTF8);
            bool bResult = doc_saveAs(pThis, aURLUtf8.getStr(), "pdf", nullptr);

            // Send the result of save
            tools::JsonWriter aJson;
            aJson.put("commandName", pCommand);
            aJson.put("success", bResult);
            pDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.finishAndGetAsOString());
            return;
        }


        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler("save"_ostr, gImpl, pDocument));
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction);

        beans::PropertyValue aValue;
        aValue.Name = u"InteractionHandler"_ustr;
        aValue.Value <<= xInteraction;
        aPropertyValuesVector.push_back(aValue);

        bool bDontSaveIfUnmodified = false;
        std::erase_if(aPropertyValuesVector,
                                                   [&bDontSaveIfUnmodified](const beans::PropertyValue& aItem){
                                                       if (aItem.Name == "DontSaveIfUnmodified")
                                                       {
                                                           bDontSaveIfUnmodified = aItem.Value.get<bool>();
                                                           return true;
                                                       }
                                                       return false;
                                                   });

        // skip saving and tell the result via UNO_COMMAND_RESULT
        if (bDontSaveIfUnmodified && (!pDocSh || !pDocSh->IsModified()))
        {
            tools::JsonWriter aJson;
            aJson.put("commandName", pCommand);
            aJson.put("success", false);
            // Add the reason for not saving
            {
                auto resultNode = aJson.startNode("result");
                aJson.put("type", "string");
                aJson.put("value", "unmodified");
            }
            pDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.finishAndGetAsOString());
            return;
        }
    }
    else if (gImpl && aCommand == ".uno:TransformDialog")
    {
        bool bNeedConversion = false;
        SfxViewShell* pViewShell = SfxViewShell::Current();
        LokChartHelper aChartHelper(pViewShell);

        if (aChartHelper.GetWindow() )
        {
            bNeedConversion = true;
        }
        else if (const SdrView* pView = pViewShell->GetDrawView())
        {
            if (OutputDevice* pOutputDevice = pView->GetFirstOutputDevice())
            {
                bNeedConversion = (pOutputDevice->GetMapMode().GetMapUnit() == MapUnit::Map100thMM);
            }
        }

        if (bNeedConversion)
        {
            sal_Int32 value;
            for (beans::PropertyValue& rPropValue: aPropertyValuesVector)
            {
                if (rPropValue.Name == "TransformPosX"
                        || rPropValue.Name == "TransformPosY"
                        || rPropValue.Name == "TransformWidth"
                        || rPropValue.Name == "TransformHeight"
                        || rPropValue.Name == "TransformRotationX"
                        || rPropValue.Name == "TransformRotationY")
                {
                    rPropValue.Value >>= value;
                    value = o3tl::convert(value, o3tl::Length::twip, o3tl::Length::mm100);
                    rPropValue.Value <<= value;
                }
            }
        }

        if (aChartHelper.GetWindow() && aPropertyValuesVector.size() > 0)
        {
            if (aPropertyValuesVector[0].Name != "Action")
            {
                tools::Rectangle aChartBB = aChartHelper.GetChartBoundingBox();

                int nLeft = o3tl::convert(aChartBB.Left(), o3tl::Length::twip, o3tl::Length::mm100);
                int nTop = o3tl::convert(aChartBB.Top(), o3tl::Length::twip, o3tl::Length::mm100);

                for (beans::PropertyValue& rPropValue: aPropertyValuesVector)
                {
                    if (rPropValue.Name == "TransformPosX" || rPropValue.Name == "TransformRotationX")
                    {
                        auto const value = *o3tl::doAccess<sal_Int32>(rPropValue.Value);
                        rPropValue.Value <<= value - nLeft;
                    }
                    else if (rPropValue.Name == "TransformPosY" || rPropValue.Name == "TransformRotationY")
                    {
                        auto const value = *o3tl::doAccess<sal_Int32>(rPropValue.Value);
                        rPropValue.Value <<= value - nTop;
                    }
                }
            }
            util::URL aCommandURL;
            aCommandURL.Path = u"LOKTransform"_ustr;
            css::uno::Reference<css::frame::XDispatch>& aChartDispatcher = aChartHelper.GetXDispatcher();
            aChartDispatcher->dispatch(aCommandURL, comphelper::containerToSequence(aPropertyValuesVector));
            return;
        }
    }
    else if (gImpl && aCommand == ".uno:LOKSidebarWriterPage")
    {
        if (!sfx2::sidebar::Sidebar::Setup(u"WriterPageDeck"))
        {
            SetLastExceptionMsg(u"failed to set up sidebar"_ustr);
        }
        return;
    }
    else if (gImpl && aCommand == ".uno:SidebarShow")
    {
        if (!sfx2::sidebar::Sidebar::Setup(u""))
        {
            SetLastExceptionMsg(u"failed to set up sidebar"_ustr);
        }
        return;
    }
    else if (gImpl && aCommand == ".uno:SidebarHide")
    {
        hideSidebar();
        return;
    }

    bool bResult = false;
    LokChartHelper aChartHelper(SfxViewShell::Current());

    if (aChartHelper.GetWindow() && aCommand != ".uno:Save" )
    {
        util::URL aCommandURL;
        aCommandURL.Path = aCommand.copy(5);
        css::uno::Reference<css::frame::XDispatch>& aChartDispatcher = aChartHelper.GetXDispatcher();
        aChartDispatcher->dispatch(aCommandURL, comphelper::containerToSequence(aPropertyValuesVector));
        return;
    }
    if (LokStarMathHelper aMathHelper(SfxViewShell::Current());
        aMathHelper.GetGraphicWindow() && aCommand != ".uno:Save")
    {
        aMathHelper.Dispatch(aCommand, comphelper::containerToSequence(aPropertyValuesVector));
        return;
    }
    if (bNotifyWhenFinished && pDocument->mpCallbackFlushHandlers.count(nView))
    {
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector),
                new DispatchResultListener(pCommand, pDocument->mpCallbackFlushHandlers[nView],
                                           pDocSh->IsModified()));
    }
    else
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector));

    if (!bResult)
    {
        SetLastExceptionMsg("Failed to dispatch " + aCommand);
    }
}

static void doc_postMouseEvent(LibreOfficeKitDocument* pThis, int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    comphelper::ProfileZone aZone("doc_postMouseEvent");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }
    try
    {
        pDoc->postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier);
    }
    catch (const uno::Exception& exception)
    {
        SetLastExceptionMsg(exception.Message);
        SAL_INFO("lok", "Failed to postMouseEvent " << exception.Message);
    }
}

static void doc_postWindowMouseEvent(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId, int nType, int nX, int nY, int nCount, int nButtons, int nModifier)
{
    comphelper::ProfileZone aZone("doc_postWindowMouseEvent");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
        return;
    }

    const Point aPos(nX, nY);

    MouseEvent aEvent(aPos, nCount, MouseEventModifiers::SIMPLECLICK, nButtons, nModifier);

    vcl::EnableDialogInput(pWindow);

    switch (nType)
    {
        case LOK_MOUSEEVENT_MOUSEBUTTONDOWN:
            Application::PostMouseEvent(VclEventId::WindowMouseButtonDown, pWindow, &aEvent);
            break;
        case LOK_MOUSEEVENT_MOUSEBUTTONUP:
            Application::PostMouseEvent(VclEventId::WindowMouseButtonUp, pWindow, &aEvent);
            break;
        case LOK_MOUSEEVENT_MOUSEMOVE:
            Application::PostMouseEvent(VclEventId::WindowMouseMove, pWindow, &aEvent);
            break;
        default:
            assert(false);
            break;
    }
}

static void doc_postWindowGestureEvent(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId, const char* pType, int nX, int nY, int nOffset)
{
    comphelper::ProfileZone aZone("doc_postWindowGestureEvent");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
        return;
    }

    OString aType(pType);
    GestureEventPanType eEventType = GestureEventPanType::Update;

    if (aType == "panBegin")
        eEventType = GestureEventPanType::Begin;
    else if (aType == "panEnd")
        eEventType = GestureEventPanType::End;

    GestureEventPan aEvent {
        sal_Int32(nX),
        sal_Int32(nY),
        eEventType,
        sal_Int32(nOffset),
        PanningOrientation::Vertical,
    };

    vcl::EnableDialogInput(pWindow);

    Application::PostGestureEvent(VclEventId::WindowGestureEvent, pWindow, &aEvent);
}

static void doc_setTextSelection(LibreOfficeKitDocument* pThis, int nType, int nX, int nY)
{
    comphelper::ProfileZone aZone("doc_setTextSelection");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->setTextSelection(nType, nX, nY);
}

static void doc_setWindowTextSelection(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId, bool swap, int nX, int nY)
{
    comphelper::ProfileZone aZone("doc_setWindowTextSelection");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
        return;
    }


    Size aOffset(pWindow->GetOutOffXPixel(), pWindow->GetOutOffYPixel());
    Point aCursorPos(nX, nY);
    aCursorPos.Move(aOffset);
    sal_uInt16 nModifier = swap ? KEY_MOD1 + KEY_MOD2 : KEY_SHIFT;

    MouseEvent aCursorEvent(aCursorPos, 1, MouseEventModifiers::SIMPLECLICK, 0, nModifier);
    Application::PostMouseEvent(VclEventId::WindowMouseButtonDown, pWindow, &aCursorEvent);
    Application::PostMouseEvent(VclEventId::WindowMouseButtonUp, pWindow, &aCursorEvent);
}

static bool getFromTransferable(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aInMimeType, OString &aRet);

static bool encodeImageAsHTML(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aMimeType, OString &aRet)
{
    if (!getFromTransferable(xTransferable, aMimeType, aRet))
        return false;

    // Encode in base64.
    auto aSeq = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),
                                   aRet.getLength());
    OStringBuffer aBase64Data;
    comphelper::Base64::encode(aBase64Data, aSeq);

    // Embed in HTML.
    aRet = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<html><head>"
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/><meta "
        "name=\"generator\" content=\""
        + getGenerator().toUtf8()
        + "\"/>"
        "</head><body><img src=\"data:" + aMimeType + ";base64,"
        + aBase64Data + "\"/></body></html>";

    return true;
}

static bool encodeTextAsHTML(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aMimeType, OString &aRet)
{
    if (!getFromTransferable(xTransferable, aMimeType, aRet))
        return false;

    // Embed in HTML - FIXME: needs some escaping.
    aRet = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<html><head>"
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/><meta "
        "name=\"generator\" content=\""
        + getGenerator().toUtf8()
        + "\"/></head><body><pre>" + aRet + "</pre></body></html>";

    return true;
}

static bool getFromTransferable(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aInMimeType, OString &aRet)
{
    OString aMimeType(aInMimeType);

    // Take care of UTF-8 text here.
    bool bConvert = false;
    sal_Int32 nIndex = 0;
    if (o3tl::getToken(aMimeType, 0, ';', nIndex) == "text/plain")
    {
        if (o3tl::getToken(aMimeType, 0, ';', nIndex) == "charset=utf-8")
        {
            aMimeType = "text/plain;charset=utf-16"_ostr;
            bConvert = true;
        }
    }

    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(aMimeType);
    if (aMimeType == "text/plain;charset=utf-16")
        aFlavor.DataType = cppu::UnoType<OUString>::get();
    else
        aFlavor.DataType = cppu::UnoType< uno::Sequence<sal_Int8> >::get();

    if (!xTransferable->isDataFlavorSupported(aFlavor))
    {
        // Try harder for HTML it is our copy/paste meta-file format
        if (aInMimeType == "text/html")
        {
            // Desperate measures - convert text to HTML instead.
            if (encodeTextAsHTML(xTransferable, "text/plain;charset=utf-8"_ostr, aRet))
                return true;
            // If html is not supported, might be a graphic-selection,
            if (encodeImageAsHTML(xTransferable, "image/png"_ostr, aRet))
                return true;
        }

        SetLastExceptionMsg("Flavor " + aFlavor.MimeType + " is not supported");
        return false;
    }

    uno::Any aAny;
    try
    {
        aAny = xTransferable->getTransferData(aFlavor);
    }
    catch (const css::datatransfer::UnsupportedFlavorException& e)
    {
        SetLastExceptionMsg("Unsupported flavor " + aFlavor.MimeType + " exception " + e.Message);
        return false;
    }
    catch (const css::uno::Exception& e)
    {
        SetLastExceptionMsg("Exception getting " + aFlavor.MimeType + " exception " + e.Message);
        return false;
    }

    if (aFlavor.DataType == cppu::UnoType<OUString>::get())
    {
        OUString aString;
        aAny >>= aString;
        if (bConvert)
            aRet = OUStringToOString(aString, RTL_TEXTENCODING_UTF8);
        else
            aRet = OString(reinterpret_cast<const char *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode));
    }
    else
    {
        uno::Sequence<sal_Int8> aSequence;
        aAny >>= aSequence;
        aRet = OString(reinterpret_cast<const char*>(aSequence.getConstArray()), aSequence.getLength());
    }

    return true;
}

static char* doc_getTextSelection(LibreOfficeKitDocument* pThis, const char* pMimeType, char** pUsedMimeType)
{
    comphelper::ProfileZone aZone("doc_getTextSelection");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable = pDoc->getSelection();
    if (!xTransferable)
    {
        SetLastExceptionMsg(u"No selection available"_ustr);
        return nullptr;
    }

    OString aType
        = pMimeType && pMimeType[0] != '\0' ? OString(pMimeType) : "text/plain;charset=utf-8"_ostr;

    OString aRet;
    bool bSuccess = getFromTransferable(xTransferable, aType, aRet);
    if (!bSuccess)
        return nullptr;

    if (pUsedMimeType) // legacy
    {
        if (pMimeType)
            *pUsedMimeType = strdup(pMimeType);
        else
            *pUsedMimeType = nullptr;
    }

    return convertOString(aRet);
}

static int doc_getSelectionType(LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getSelectionType");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return LOK_SELTYPE_NONE;
    }

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable = pDoc->getSelection();
    if (!xTransferable)
    {
        SetLastExceptionMsg(u"No selection available"_ustr);
        return LOK_SELTYPE_NONE;
    }

    css::uno::Reference<css::datatransfer::XTransferable2> xTransferable2(xTransferable, css::uno::UNO_QUERY);
    if (xTransferable2.is() && xTransferable2->isComplex())
        return LOK_SELTYPE_COMPLEX;

    OString aRet;
    bool bSuccess = getFromTransferable(xTransferable, "text/plain;charset=utf-8"_ostr, aRet);
    if (!bSuccess)
        return LOK_SELTYPE_NONE;

    if (aRet.getLength() > 10000)
        return LOK_SELTYPE_COMPLEX;

    return !aRet.isEmpty() ? LOK_SELTYPE_TEXT : LOK_SELTYPE_NONE;
}

static int doc_getSelectionTypeAndText(LibreOfficeKitDocument* pThis, const char* pMimeType, char** pText, char** pUsedMimeType)
{
    // The purpose of this function is to avoid double call to pDoc->getSelection(),
    // which may be expensive.
    comphelper::ProfileZone aZone("doc_getSelectionTypeAndText");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return LOK_SELTYPE_NONE;
    }

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable = pDoc->getSelection();
    if (!xTransferable)
    {
        SetLastExceptionMsg(u"No selection available"_ustr);
        return LOK_SELTYPE_NONE;
    }

    css::uno::Reference<css::datatransfer::XTransferable2> xTransferable2(xTransferable, css::uno::UNO_QUERY);
    if (xTransferable2.is() && xTransferable2->isComplex())
        return LOK_SELTYPE_COMPLEX;

    OString aType
        = pMimeType && pMimeType[0] != '\0' ? OString(pMimeType) : "text/plain;charset=utf-8"_ostr;

    OString aRet;
    bool bSuccess = getFromTransferable(xTransferable, aType, aRet);
    if (!bSuccess)
        return LOK_SELTYPE_NONE;

    if (aRet.getLength() > 10000)
        return LOK_SELTYPE_COMPLEX;

    if (aRet.isEmpty())
        return LOK_SELTYPE_NONE;

    if (pText)
        *pText = convertOString(aRet);

    if (pUsedMimeType) // legacy
    {
        if (pMimeType)
            *pUsedMimeType = strdup(pMimeType);
        else
            *pUsedMimeType = nullptr;
    }

    return LOK_SELTYPE_TEXT;
}

static int doc_getClipboard(LibreOfficeKitDocument* pThis,
                            const char **pMimeTypes,
                            size_t      *pOutCount,
                            char      ***pOutMimeTypes,
                            size_t     **pOutSizes,
                            char      ***pOutStreams)
{
#ifdef IOS
    (void) pThis;
    (void) pMimeTypes;
    (void) pOutCount;
    (void) pOutMimeTypes;
    (void) pOutSizes;
    (void) pOutStreams;

    assert(!"doc_getClipboard should not be called on iOS");

    return 0;
#else
    comphelper::ProfileZone aZone("doc_getClipboard");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    assert (pOutCount);
    assert (pOutMimeTypes);
    assert (pOutSizes);
    assert (pOutStreams);

    *pOutCount = 0;
    *pOutMimeTypes = nullptr;
    *pOutSizes = nullptr;
    *pOutStreams = nullptr;

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return 0;
    }

    rtl::Reference<LOKClipboard> xClip(LOKClipboardFactory::getClipboardForCurView());

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable = xClip->getContents();
    SAL_INFO("lok", "Got from clip: " << xClip.get() << " transferable: " << xTransferable);
    if (!xTransferable)
    {
        SetLastExceptionMsg(u"No clipboard content available"_ustr);
        return 0;
    }

    std::vector<OString> aMimeTypes;
    if (!pMimeTypes) // everything
    {
        const uno::Sequence< css::datatransfer::DataFlavor > flavors = xTransferable->getTransferDataFlavors();
        if (!flavors.getLength())
        {
            SetLastExceptionMsg(u"Flavourless selection"_ustr);
            return 0;
        }
        for (const auto &it : flavors)
            aMimeTypes.push_back(OUStringToOString(it.MimeType, RTL_TEXTENCODING_UTF8));
    }
    else
    {
        for (size_t i = 0; pMimeTypes[i]; ++i)
            aMimeTypes.push_back(OString(pMimeTypes[i]));
    }

    *pOutCount = aMimeTypes.size();
    *pOutSizes = static_cast<size_t *>(malloc(*pOutCount * sizeof(size_t)));
    assert(*pOutSizes && "Don't handle OOM conditions");
    *pOutMimeTypes = static_cast<char **>(malloc(*pOutCount * sizeof(char *)));
    assert(*pOutMimeTypes && "Don't handle OOM conditions");
    *pOutStreams = static_cast<char **>(malloc(*pOutCount * sizeof(char *)));
    assert(*pOutStreams && "Don't handle OOM conditions");
    for (size_t i = 0; i < aMimeTypes.size(); ++i)
    {
        if (aMimeTypes[i] == "text/plain;charset=utf-16")
            (*pOutMimeTypes)[i] = strdup("text/plain;charset=utf-8");
        else
            (*pOutMimeTypes)[i] = convertOString(aMimeTypes[i]);

        OString aRet;
        bool bSuccess = getFromTransferable(xTransferable, (*pOutMimeTypes)[i], aRet);
        if (!bSuccess || aRet.getLength() < 1)
        {
            (*pOutSizes)[i] = 0;
            (*pOutStreams)[i] = nullptr;
        }
        else
        {
            (*pOutSizes)[i] = aRet.getLength();
            (*pOutStreams)[i] = convertOString(aRet);
        }
    }

    return 1;
#endif
}

static int doc_setClipboard(LibreOfficeKitDocument* pThis,
                            const size_t   nInCount,
                            const char   **pInMimeTypes,
                            const size_t  *pInSizes,
                            const char   **pInStreams)
{
#ifdef IOS
    (void) pThis;
    (void) nInCount;
    (void) pInMimeTypes;
    (void) pInSizes;
    (void) pInStreams;
#else
    comphelper::ProfileZone aZone("doc_setClipboard");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return false;
    }

    uno::Reference<datatransfer::XTransferable> xTransferable(new LOKTransferable(nInCount, pInMimeTypes, pInSizes, pInStreams));

    auto xClip = forceSetClipboardForCurrentView(pThis);
    xClip->setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());

    SAL_INFO("lok", "Set clip: " << xClip.get() << " to: " << xTransferable);

    if (!pDoc->isMimeTypeSupported())
    {
        SetLastExceptionMsg(u"Document doesn't support this mime type"_ustr);
        return false;
    }
#endif
    return true;
}

static bool doc_paste(LibreOfficeKitDocument* pThis, const char* pMimeType, const char* pData, size_t nSize)
{
    comphelper::ProfileZone aZone("doc_paste");

    SolarMutexGuard aGuard;

    const char *pInMimeTypes[1];
    const char *pInStreams[1];
    size_t pInSizes[1];
    pInMimeTypes[0] = pMimeType;
    pInSizes[0] = nSize;
    pInStreams[0] = pData;

    if (!doc_setClipboard(pThis, 1, pInMimeTypes, pInSizes, pInStreams))
        return false;

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"AnchorType", uno::Any(static_cast<sal_uInt16>(css::text::TextContentAnchorType_AS_CHARACTER))},
        {"IgnoreComments", uno::Any(true)},
    }));
    if (!comphelper::dispatchCommand(u".uno:Paste"_ustr, aPropertyValues))
    {
        SetLastExceptionMsg(u"Failed to dispatch the .uno: command"_ustr);
        return false;
    }

    return true;
}

static void doc_setGraphicSelection(LibreOfficeKitDocument* pThis, int nType, int nX, int nY)
{
    comphelper::ProfileZone aZone("doc_setGraphicSelection");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->setGraphicSelection(nType, nX, nY);
}

static void doc_resetSelection(LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_resetSelection");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->resetSelection();
}

static char* getDocReadOnly(LibreOfficeKitDocument* pThis)
{
    SfxObjectShell* pObjectShell = getSfxObjectShell(pThis);
    if (!pObjectShell)
        return nullptr;

    boost::property_tree::ptree aTree;
    aTree.put("commandName", ".uno:ReadOnly");
    aTree.put("success", pObjectShell->IsLoadReadonly());

    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    if (!pJson)
        return nullptr;

    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

static void addLocale(boost::property_tree::ptree& rValues, css::lang::Locale const & rLocale)
{
    boost::property_tree::ptree aChild;
    const LanguageTag aLanguageTag( rLocale );
    OUString sLanguage = SvtLanguageTable::GetLanguageString(aLanguageTag.getLanguageType());
    if (sLanguage.endsWith("}"))
        return;

    sLanguage += ";" + aLanguageTag.getBcp47(false);
    aChild.put("", sLanguage.toUtf8());
    rValues.push_back(std::make_pair("", aChild));
}

static char* getLanguages(const char* pCommand)
{
    css::uno::Sequence< css::lang::Locale > aLocales;
    css::uno::Sequence< css::lang::Locale > aGrammarLocales;

    if (xContext.is())
    {
        // SpellChecker
        css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv = css::linguistic2::LinguServiceManager::create(xContext);
        if (xLangSrv.is())
        {
            css::uno::Reference<css::linguistic2::XSpellChecker> xSpell = xLangSrv->getSpellChecker();
            if (xSpell.is())
                aLocales = xSpell->getLocales();
        }

        // LanguageTool
        if (LanguageToolCfg::IsEnabled::get())
        {
            uno::Reference< linguistic2::XProofreader > xGC(
                    xContext->getServiceManager()->createInstanceWithContext(u"org.openoffice.lingu.LanguageToolGrammarChecker"_ustr, xContext),
                    uno::UNO_QUERY_THROW );
            uno::Reference< linguistic2::XSupportedLocales > xSuppLoc( xGC, uno::UNO_QUERY_THROW );
            aGrammarLocales = xSuppLoc->getLocales();
        }
    }

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    boost::property_tree::ptree aValues;
    for (css::lang::Locale const& rLocale : aLocales)
        addLocale(aValues, rLocale);
    for (css::lang::Locale const& rLocale : aGrammarLocales)
        addLocale(aValues, rLocale);
    aTree.add_child("commandValues", aValues);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    assert(pJson); // Don't handle OOM conditions
    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

static char* getFonts (const char* pCommand)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if (!pDocSh)
        return nullptr;
    const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(
        pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    boost::property_tree::ptree aValues;
    if ( pList )
    {
        sal_uInt16 nFontCount = pList->GetFontNameCount();
        for (sal_uInt16 i = 0; i < nFontCount; ++i)
        {
            boost::property_tree::ptree aChildren;
            const FontMetric& rFontMetric = pList->GetFontName(i);
            const int* pAry = FontList::GetStdSizeAry();
            sal_uInt16 nSizeCount = 0;
            while (pAry[nSizeCount])
            {
                boost::property_tree::ptree aChild;
                aChild.put("", static_cast<float>(pAry[nSizeCount]) / 10);
                aChildren.push_back(std::make_pair("", aChild));
                nSizeCount++;
            }
            aValues.add_child(rFontMetric.GetFamilyName().toUtf8().getStr(), aChildren);
        }
    }
    aTree.add_child("commandValues", aValues);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    assert(pJson); // Don't handle OOM conditions
    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

static char* getFontSubset (std::string_view aFontName)
{
    OUString aFoundFont(::rtl::Uri::decode(OStringToOUString(aFontName, RTL_TEXTENCODING_UTF8), rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8));

    boost::property_tree::ptree aTree;
    aTree.put("commandName", ".uno:FontSubset");
    boost::property_tree::ptree aValues;

    if (const vcl::Font* pFont = FindFont(aFoundFont))
    {
        FontCharMapRef xFontCharMap (new FontCharMap());
        auto aDevice(VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA));

        aDevice->SetFont(*pFont);
        aDevice->GetFontCharMap(xFontCharMap);
        SubsetMap aSubMap(xFontCharMap);

        for (auto const& subset : aSubMap.GetSubsetMap())
        {
            boost::property_tree::ptree aChild;
            aChild.put("", static_cast<int>(ublock_getCode(subset.GetRangeMin())));
            aValues.push_back(std::make_pair("", aChild));
        }
    }

    aTree.add_child("commandValues", aValues);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    assert(pJson); // Don't handle OOM conditions
    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

static char* getStyles(LibreOfficeKitDocument* pThis, const char* pCommand)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    uno::Reference<css::style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(pDocument->mxComponent, uno::UNO_QUERY);
    const uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamiliesSupplier->getStyleFamilies();
    const uno::Sequence<OUString> aStyleFamilies = xStyleFamilies->getElementNames();

    static constexpr OUString aWriterStyles[] =
    {
        u"Text body"_ustr,
        u"Quotations"_ustr,
        u"Title"_ustr,
        u"Subtitle"_ustr,
        u"Heading 1"_ustr,
        u"Heading 2"_ustr,
        u"Heading 3"_ustr,
    };

    // We need to keep a list of the default style names
    // in order to filter these out later when processing
    // the full list of styles.
    std::set<OUString> aDefaultStyleNames;

    boost::property_tree::ptree aValues;
    for (OUString const & sStyleFam : aStyleFamilies)
    {
        boost::property_tree::ptree aChildren;
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName(sStyleFam), uno::UNO_QUERY);

        // Writer provides a huge number of styles, we have a list of 7 "default" styles which
        // should be shown in the normal dropdown, which we should add to the start of the list
        // to simplify their selection.
        if (sStyleFam == "ParagraphStyles"
            && doc_getDocumentType(pThis) == LOK_DOCTYPE_TEXT)
        {
            for (const OUString& rStyle: aWriterStyles)
            {
                aDefaultStyleNames.insert( rStyle );

                boost::property_tree::ptree aChild;
                aChild.put("", rStyle.toUtf8());
                aChildren.push_back(std::make_pair("", aChild));
            }
        }

        const uno::Sequence<OUString> aStyles = xStyleFamily->getElementNames();
        for (const OUString& rStyle: aStyles )
        {
            // Filter out the default styles - they are already at the top
            // of the list
            if (aDefaultStyleNames.find(rStyle) == aDefaultStyleNames.end() ||
                (sStyleFam != "ParagraphStyles" || doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT) )
            {
                boost::property_tree::ptree aChild;
                aChild.put("", rStyle.toUtf8());
                aChildren.push_back(std::make_pair("", aChild));
            }
        }
        aValues.add_child(sStyleFam.toUtf8().getStr(), aChildren);
    }

    // Header & Footer Styles
    {
        boost::property_tree::ptree aChild;
        boost::property_tree::ptree aChildren;
        static constexpr OUString sPageStyles(u"PageStyles"_ustr);
        uno::Reference<beans::XPropertySet> xProperty;
        uno::Reference<container::XNameContainer> xContainer;

        if (xStyleFamilies->hasByName(sPageStyles) && (xStyleFamilies->getByName(sPageStyles) >>= xContainer))
        {
            const uno::Sequence<OUString> aSeqNames = xContainer->getElementNames();
            for (OUString const & sName : aSeqNames)
            {
                bool bIsPhysical;
                xProperty.set(xContainer->getByName(sName), uno::UNO_QUERY);
                if (xProperty.is() && (xProperty->getPropertyValue(u"IsPhysical"_ustr) >>= bIsPhysical) && bIsPhysical)
                {
                    OUString displayName;
                    xProperty->getPropertyValue(u"DisplayName"_ustr) >>= displayName;
                    aChild.put("", displayName.toUtf8());
                    aChildren.push_back(std::make_pair("", aChild));
                }
            }
            aValues.add_child("HeaderFooter", aChildren);
        }
    }

    {
        boost::property_tree::ptree aCommandList;

        {
            boost::property_tree::ptree aChild;

            OUString sClearFormat = SvxResId(RID_SVXSTR_CLEARFORM);

            boost::property_tree::ptree aName;
            aName.put("", sClearFormat.toUtf8());
            aChild.push_back(std::make_pair("text", aName));

            boost::property_tree::ptree aCommand;
            aCommand.put("", ".uno:ResetAttributes");
            aChild.push_back(std::make_pair("id", aCommand));

            aCommandList.push_back(std::make_pair("", aChild));
        }

        aValues.add_child("Commands", aCommandList);
    }

    aTree.add_child("commandValues", aValues);
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    char* pJson = static_cast<char*>(malloc(aStream.str().size() + 1));
    assert(pJson); // Don't handle OOM conditions
    strcpy(pJson, aStream.str().c_str());
    pJson[aStream.str().size()] = '\0';
    return pJson;
}

namespace {

enum class UndoOrRedo
{
    UNDO,
    REDO
};

}

/// Returns the JSON representation of either an undo or a redo stack.
static char* getUndoOrRedo(LibreOfficeKitDocument* pThis, UndoOrRedo eCommand)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    auto pBaseModel = dynamic_cast<SfxBaseModel*>(pDocument->mxComponent.get());
    if (!pBaseModel)
        return nullptr;

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    if (!pObjectShell)
        return nullptr;

    SfxUndoManager* pUndoManager = pObjectShell->GetUndoManager();
    if (!pUndoManager)
        return nullptr;

    OUString aString;
    if (eCommand == UndoOrRedo::UNDO)
        aString = pUndoManager->GetUndoActionsInfo();
    else
        aString = pUndoManager->GetRedoActionsInfo();
    char* pJson = convertOUString(aString);
    return pJson;
}

/// Returns the JSON representation of the redline stack.
static char* getTrackedChanges(LibreOfficeKitDocument* pThis)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    uno::Reference<document::XRedlinesSupplier> xRedlinesSupplier(pDocument->mxComponent, uno::UNO_QUERY);
    tools::JsonWriter aJson;
    // We want positions of the track changes also which is not possible from
    // UNO. Enable positioning information for text documents only for now, so
    // construct the tracked changes JSON from inside the sw/, not here using UNO
    if (doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT && xRedlinesSupplier.is())
    {
        auto redlinesNode = aJson.startArray("redlines");
        uno::Reference<container::XEnumeration> xRedlines = xRedlinesSupplier->getRedlines()->createEnumeration();
        for (size_t nIndex = 0; xRedlines->hasMoreElements(); ++nIndex)
        {
            uno::Reference<beans::XPropertySet> xRedline(xRedlines->nextElement(), uno::UNO_QUERY);
            auto redlineNode = aJson.startStruct();
            aJson.put("index", static_cast<sal_Int32>(nIndex));

            OUString sAuthor;
            xRedline->getPropertyValue(u"RedlineAuthor"_ustr) >>= sAuthor;
            aJson.put("author", sAuthor);

            OUString sType;
            xRedline->getPropertyValue(u"RedlineType"_ustr) >>= sType;
            aJson.put("type", sType);

            OUString sComment;
            xRedline->getPropertyValue(u"RedlineComment"_ustr) >>= sComment;
            aJson.put("comment", sComment);

            OUString sDescription;
            xRedline->getPropertyValue(u"RedlineDescription"_ustr) >>= sDescription;
            aJson.put("description", sDescription);

            util::DateTime aDateTime;
            xRedline->getPropertyValue(u"RedlineDateTime"_ustr) >>= aDateTime;
            OUString sDateTime = utl::toISO8601(aDateTime);
            aJson.put("dateTime", sDateTime);
        }
    }
    else
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
            return nullptr;
        }
        pDoc->getTrackedChanges(aJson);
    }

    return convertOString(aJson.finishAndGetAsOString());
}


/// Returns the JSON representation of the redline author table.
static char* getTrackedChangeAuthors(LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getTrackedChangeAuthors(aJsonWriter);
    return convertOString(aJsonWriter.finishAndGetAsOString());
}

static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand)
{
    comphelper::ProfileZone aZone("doc_getCommandValues");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    const std::string_view aCommand(pCommand);
    static constexpr std::string_view aViewRowColumnHeaders(".uno:ViewRowColumnHeaders");
    static constexpr std::string_view aSheetGeometryData(".uno:SheetGeometryData");
    static constexpr std::string_view aFontSubset(".uno:FontSubset&name=");

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return nullptr;
    }

    if (aCommand == ".uno:ReadOnly")
    {
        return getDocReadOnly(pThis);
    }
    else if (aCommand == ".uno:LanguageStatus")
    {
        return getLanguages(pCommand);
    }
    else if (aCommand == ".uno:CharFontName")
    {
        return getFonts(pCommand);
    }
    else if (aCommand == ".uno:StyleApply")
    {
        return getStyles(pThis, pCommand);
    }
    else if (aCommand == ".uno:Undo")
    {
        return getUndoOrRedo(pThis, UndoOrRedo::UNDO);
    }
    else if (aCommand == ".uno:Redo")
    {
        return getUndoOrRedo(pThis, UndoOrRedo::REDO);
    }
    else if (aCommand == ".uno:AcceptTrackedChanges")
    {
        return getTrackedChanges(pThis);
    }
    else if (aCommand == ".uno:TrackedChangeAuthors")
    {
        return getTrackedChangeAuthors(pThis);
    }
    else if (aCommand == ".uno:ViewAnnotations")
    {
        return getPostIts(pThis);
    }
    else if (aCommand == ".uno:ViewAnnotationsPosition")
    {
        return getPostItsPos(pThis);
    }
    else if (aCommand == ".uno:RulerState")
    {
        return getRulerState(pThis);
    }
    else if (aCommand == ".uno:ViewRenderState")
    {
        return convertOString(pDoc->getViewRenderState());
    }
    else if (aCommand.starts_with(aViewRowColumnHeaders))
    {
        tools::Rectangle aRectangle;
        if (aCommand.size() > aViewRowColumnHeaders.size())
        {
            // Command has parameters.
            int nX = 0;
            int nY = 0;
            int nWidth = 0;
            int nHeight = 0;
            std::string_view aArguments = aCommand.substr(aViewRowColumnHeaders.size() + 1);
            sal_Int32 nParamIndex = 0;
            do
            {
                std::string_view aParamToken = o3tl::getToken(aArguments, 0, '&', nParamIndex);
                sal_Int32 nIndex = 0;
                std::string_view aKey;
                std::string_view aValue;
                do
                {
                    std::string_view aToken = o3tl::getToken(aParamToken, 0, '=', nIndex);
                    if (aKey.empty())
                        aKey = aToken;
                    else
                        aValue = aToken;
                }
                while (nIndex >= 0);
                if (aKey == "x")
                    nX = o3tl::toInt32(aValue);
                else if (aKey == "y")
                    nY = o3tl::toInt32(aValue);
                else if (aKey == "width")
                    nWidth = o3tl::toInt32(aValue);
                else if (aKey == "height")
                    nHeight = o3tl::toInt32(aValue);
            }
            while (nParamIndex >= 0);

            aRectangle = tools::Rectangle(nX, nY, nX + nWidth, nY + nHeight);
        }

        tools::JsonWriter aJsonWriter;
        pDoc->getRowColumnHeaders(aRectangle, aJsonWriter);
        return convertOString(aJsonWriter.finishAndGetAsOString());
    }
    else if (aCommand.starts_with(aSheetGeometryData))
    {
        bool bColumns = true;
        bool bRows = true;
        bool bSizes = true;
        bool bHidden = true;
        bool bFiltered = true;
        bool bGroups = true;
        if (aCommand.size() > aSheetGeometryData.size())
        {
            bColumns = bRows = bSizes = bHidden = bFiltered = bGroups = false;

            std::string_view aArguments = aCommand.substr(aSheetGeometryData.size() + 1);
            sal_Int32 nParamIndex = 0;
            do
            {
                std::string_view aParamToken = o3tl::getToken(aArguments, 0, '&', nParamIndex);
                sal_Int32 nIndex = 0;
                std::string_view aKey;
                std::string_view aValue;
                do
                {
                    std::string_view aToken = o3tl::getToken(aParamToken, 0, '=', nIndex);
                    if (aKey.empty())
                        aKey = aToken;
                    else
                        aValue = aToken;

                } while (nIndex >= 0);

                bool bEnableFlag = aValue.empty() ||
                    o3tl::equalsIgnoreAsciiCase(aValue, "true") || o3tl::toInt32(aValue) > 0;
                if (!bEnableFlag)
                    continue;

                if (aKey == "columns")
                    bColumns = true;
                else if (aKey == "rows")
                    bRows = true;
                else if (aKey == "sizes")
                    bSizes = true;
                else if (aKey == "hidden")
                    bHidden = true;
                else if (aKey == "filtered")
                    bFiltered = true;
                else if (aKey == "groups")
                    bGroups = true;

            } while (nParamIndex >= 0);
        }

        OString aGeomDataStr
            = pDoc->getSheetGeometryData(bColumns, bRows, bSizes, bHidden, bFiltered, bGroups);

        if (aGeomDataStr.isEmpty())
            return nullptr;

        return convertOString(aGeomDataStr);
    }
    else if (aCommand.starts_with(".uno:CellCursor"))
    {
        // Ignore command's deprecated parameters.
        tools::JsonWriter aJsonWriter;
        pDoc->getCellCursor(aJsonWriter);
        return convertOString(aJsonWriter.finishAndGetAsOString());
    }
    else if (aCommand.starts_with(aFontSubset))
    {
        return getFontSubset(aCommand.substr(aFontSubset.size()));
    }
    else if (pDoc->supportsCommand(INetURLObject(OUString::fromUtf8(aCommand)).GetURLPath()))
    {
        tools::JsonWriter aJsonWriter;
        pDoc->getCommandValues(aJsonWriter, aCommand);
        return convertOString(aJsonWriter.finishAndGetAsOString());
    }
    else
    {
        SetLastExceptionMsg(u"Unknown command, no values returned"_ustr);
        return nullptr;
    }
}

static void doc_setClientZoom(LibreOfficeKitDocument* pThis, int nTilePixelWidth, int nTilePixelHeight,
        int nTileTwipWidth, int nTileTwipHeight)
{
    comphelper::ProfileZone aZone("doc_setClientZoom");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->setClientZoom(nTilePixelWidth, nTilePixelHeight, nTileTwipWidth, nTileTwipHeight);
}

static void doc_setClientVisibleArea(LibreOfficeKitDocument* pThis, int nX, int nY, int nWidth, int nHeight)
{
    comphelper::ProfileZone aZone("doc_setClientVisibleArea");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    tools::Rectangle aRectangle(Point(nX, nY), Size(nWidth, nHeight));
    pDoc->setClientVisibleArea(aRectangle);
}

static void doc_setOutlineState(LibreOfficeKitDocument* pThis, bool bColumn, int nLevel, int nIndex, bool bHidden)
{
    comphelper::ProfileZone aZone("doc_setOutlineState");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->setOutlineState(bColumn, nLevel, nIndex, bHidden);
}

static int doc_createViewWithOptions(LibreOfficeKitDocument* pThis,
                                     const char* pOptions)
{
    comphelper::ProfileZone aZone("doc_createView");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    OUString aOptions = getUString(pOptions);
    const OUString aLanguage = extractParameter(aOptions, u"Language");

    if (!aLanguage.isEmpty())
    {
        // Set the LOK language tag, used for dialog tunneling.
        comphelper::LibreOfficeKit::setLanguageTag(LanguageTag(aLanguage));
        comphelper::LibreOfficeKit::setLocale(LanguageTag(aLanguage));
    }

    const OUString aDeviceFormFactor = extractParameter(aOptions, u"DeviceFormFactor");
    SfxLokHelper::setDeviceFormFactor(aDeviceFormFactor);

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    const int nId = SfxLokHelper::createView(pDocument->mnDocumentId);

    vcl::lok::numberOfViewsChanged(SfxLokHelper::getViewsCount(pDocument->mnDocumentId));

#ifdef IOS
    (void) pThis;
#else
    forceSetClipboardForCurrentView(pThis);
#endif

    return nId;
}

static int doc_createView(LibreOfficeKitDocument* pThis)
{
    return doc_createViewWithOptions(pThis, nullptr); // No options.
}

static void doc_destroyView(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* pThis, int nId)
{
    comphelper::ProfileZone aZone("doc_destroyView");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

#ifndef IOS
    LOKClipboardFactory::releaseClipboardForView(nId);
#endif

    SfxLokHelper::destroyView(nId);

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    vcl::lok::numberOfViewsChanged(SfxLokHelper::getViewsCount(pDocument->mnDocumentId));
}

static void doc_setView(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId)
{
    comphelper::ProfileZone aZone("doc_setView");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxLokHelper::setView(nId);
}

static int doc_getView(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/)
{
    comphelper::ProfileZone aZone("doc_getView");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    return SfxLokHelper::getView();
}

static int doc_getViewsCount(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getViewsCount");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    return SfxLokHelper::getViewsCount(pDocument->mnDocumentId);
}

static bool doc_getViewIds(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* pThis, int* pArray, size_t nSize)
{
    comphelper::ProfileZone aZone("doc_getViewsIds");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    return SfxLokHelper::getViewIds(pDocument->mnDocumentId, pArray, nSize);
}

static void doc_setViewLanguage(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId, const char* language)
{
    comphelper::ProfileZone aZone("doc_setViewLanguage");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    OUString sLanguage = OStringToOUString(language, RTL_TEXTENCODING_UTF8);
    SfxLokHelper::setViewLanguage(nId, sLanguage);
    SfxLokHelper::setViewLocale(nId, sLanguage);
}

unsigned char* doc_renderFont(LibreOfficeKitDocument* pThis,
                              const char* pFontName,
                              const char* pChar,
                              int* pFontWidth,
                              int* pFontHeight)
{
    return doc_renderFontOrientation(pThis, pFontName, pChar, pFontWidth, pFontHeight, 0);
}

unsigned char* doc_renderFontOrientation(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/,
                              const char* pFontName,
                              const char* pChar,
                              int* pFontWidth,
                              int* pFontHeight,
                              int pOrientation)
{
    comphelper::ProfileZone aZone("doc_renderFont");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    const int nDefaultFontSize = 25;

    auto aFont = FindFont_FallbackToDefault(OStringToOUString(pFontName, RTL_TEXTENCODING_UTF8));

    OUString aText(OStringToOUString(pChar, RTL_TEXTENCODING_UTF8));
    if (aText.isEmpty())
        aText = aFont.GetFamilyName();

    auto aDevice(VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA));
    ::tools::Rectangle aRect;
    aFont.SetFontSize(Size(0, nDefaultFontSize));
    aFont.SetOrientation(Degree10(pOrientation));
    aDevice->SetFont(aFont);
    aDevice->GetTextBoundRect(aRect, aText);
    if (aRect.IsEmpty())
        return nullptr;

    int nFontWidth = aRect.Right() + 1;
    int nFontHeight = aRect.Bottom() + 1;

    if (nFontWidth <= 0 || nFontHeight <= 0)
        return nullptr;

    if (*pFontWidth > 0 && *pFontHeight > 0)
    {
        double fScaleX = *pFontWidth / static_cast<double>(nFontWidth) / 1.5;
        double fScaleY = *pFontHeight / static_cast<double>(nFontHeight) / 1.5;

        double fScale = std::min(fScaleX, fScaleY);

        if (fScale >= 1.0)
        {
            int nFontSize = fScale * nDefaultFontSize;
            aFont.SetFontSize(Size(0, nFontSize));
            aDevice->SetFont(aFont);
        }

        aRect = tools::Rectangle(0, 0, *pFontWidth, *pFontHeight);

        nFontWidth = *pFontWidth;
        nFontHeight = *pFontHeight;

    }

    unsigned char* pBuffer = static_cast<unsigned char*>(malloc(4 * nFontWidth * nFontHeight));
    if (!pBuffer)
        return nullptr;

    memset(pBuffer, 0, nFontWidth * nFontHeight * 4);
    aDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    aDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(
                Size(nFontWidth, nFontHeight), Fraction(1.0), Point(),
                pBuffer);

    if (*pFontWidth > 0 && *pFontHeight > 0)
    {
        DrawTextFlags const nStyle =
                DrawTextFlags::Center
                | DrawTextFlags::VCenter
                | DrawTextFlags::MultiLine
                | DrawTextFlags::WordBreak;// | DrawTextFlags::WordBreakHyphenation ;

        aDevice->DrawText(aRect, aText, nStyle);
    }
    else
    {
        *pFontWidth = nFontWidth;
        *pFontHeight = nFontHeight;

        aDevice->DrawText(Point(0,0), aText);
    }


    return pBuffer;
}


static void doc_paintWindow(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId,
                            unsigned char* pBuffer,
                            const int nX, const int nY,
                            const int nWidth, const int nHeight)
{
    doc_paintWindowDPI(pThis, nLOKWindowId, pBuffer, nX, nY, nWidth, nHeight, 1.0);
}

static void doc_paintWindowDPI(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId,
                               unsigned char* pBuffer,
                               const int nX, const int nY,
                               const int nWidth, const int nHeight,
                               const double fDPIScale)
{
    doc_paintWindowForView(pThis, nLOKWindowId, pBuffer, nX, nY, nWidth, nHeight, fDPIScale, -1);
}

static void doc_paintWindowForView(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId,
                                   unsigned char* pBuffer, const int nX, const int nY,
                                   const int nWidth, const int nHeight,
                                   const double fDPIScale, int viewId)
{
    comphelper::ProfileZone aZone("doc_paintWindowDPI");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
        return;
    }

    // Used to avoid work in setView if set.
    comphelper::LibreOfficeKit::setDialogPainting(true);

    if (viewId >= 0)
        doc_setView(pThis, viewId);

    // Setup cairo (or CoreGraphics, in the iOS case) to draw with the changed DPI scale (and return
    // back to 1.0 when the painting finishes)
    comphelper::ScopeGuard dpiScaleGuard([]() { comphelper::LibreOfficeKit::setDPIScale(1.0); });
    comphelper::LibreOfficeKit::setDPIScale(fDPIScale);

#if defined(IOS)
    // Onine uses the LOK_TILEMODE_RGBA by default so flip the normal flags
    // to kCGImageAlphaNoneSkipLast | kCGImageByteOrder32Big
    CGContextRef cgc = CGBitmapContextCreate(pBuffer, nWidth, nHeight, 8, nWidth*4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaNoneSkipLast | kCGImageByteOrder32Big);

    CGContextTranslateCTM(cgc, 0, nHeight);
    CGContextScaleCTM(cgc, fDPIScale, -fDPIScale);

    SystemGraphicsData aData;
    aData.rCGContext = cgc;

    ScopedVclPtrInstance<VirtualDevice> pDevice(aData, Size(1, 1), DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixel(Size(nWidth, nHeight));

    MapMode aMapMode(pDevice->GetMapMode());
    aMapMode.SetOrigin(Point(-(nX / fDPIScale), -(nY / fDPIScale)));
    pDevice->SetMapMode(aMapMode);

    pWindow->PaintToDevice(pDevice.get(), Point(0, 0));

    CGContextRelease(cgc);

#else

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixelScaleOffsetAndLOKBuffer(Size(nWidth, nHeight), Fraction(1.0), Point(), pBuffer);

    MapMode aMapMode(pDevice->GetMapMode());
    aMapMode.SetOrigin(Point(-(nX / fDPIScale), -(nY / fDPIScale)));
    pDevice->SetMapMode(aMapMode);

    pWindow->PaintToDevice(pDevice.get(), Point(0, 0));
#endif

    comphelper::LibreOfficeKit::setDialogPainting(false);
}

static void doc_postWindow(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId, int nAction, const char* pData)
{
    comphelper::ProfileZone aZone("doc_postWindow");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog rendering, or window not found."_ustr);
        return;
    }

    if (nAction == LOK_WINDOW_CLOSE)
    {
        vcl::CloseTopLevel(pWindow);
    }
    else if (nAction == LOK_WINDOW_PASTE)
    {
#ifndef IOS
        OUString aMimeType;
        css::uno::Sequence<sal_Int8> aData;
        std::vector<beans::PropertyValue> aArgs(jsonToPropertyValuesVector(pData));
        {
            aArgs.size() == 2 &&
            aArgs[0].Name == "MimeType" && (aArgs[0].Value >>= aMimeType) &&
            aArgs[1].Name == "Data" && (aArgs[1].Value >>= aData);
        }

        if (!aMimeType.isEmpty() && aData.hasElements())
        {
            uno::Reference<datatransfer::XTransferable> xTransferable(new LOKTransferable(aMimeType, aData));
            uno::Reference<datatransfer::clipboard::XClipboard> xClipboard(new LOKClipboard);
            xClipboard->setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());
            pWindow->SetClipboard(xClipboard);

            KeyEvent aEvent(0, KEY_PASTE, 0);
            Application::PostKeyEvent(VclEventId::WindowKeyInput, pWindow, &aEvent);
        }
        else
            SetLastExceptionMsg(u"Window command 'paste': wrong parameters."_ustr);
#else
        (void) pData;
        assert(!"doc_postWindow() with LOK_WINDOW_PASTE should not be called on iOS");
#endif
    }
}

// CERTIFICATE AND DOCUMENT SIGNING
static bool doc_insertCertificate(LibreOfficeKitDocument* pThis,
                                  const unsigned char* pCertificateBinary, const int nCertificateBinarySize,
                                  const unsigned char* pPrivateKeyBinary, const int nPrivateKeySize)
{
    comphelper::ProfileZone aZone("doc_insertCertificate");

    if (!xContext.is())
        return false;

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    if (!pDocument->mxComponent.is())
        return false;

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(pDocument->mxComponent.get());
    if (!pBaseModel)
        return false;

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();

    if (!pObjectShell)
        return false;

    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    if (!xSecurityContext.is())
        return false;

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Reference<xml::crypto::XCertificateCreator> xCertificateCreator(xSecurityEnvironment, uno::UNO_QUERY);

    if (!xCertificateCreator.is())
        return false;

    uno::Sequence<sal_Int8> aCertificateSequence;

    std::string aCertificateString(reinterpret_cast<const char*>(pCertificateBinary), nCertificateBinarySize);
    std::string aCertificateBase64String = extractCertificate(aCertificateString);
    if (!aCertificateBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String);
        comphelper::Base64::decode(aCertificateSequence, aBase64OUString);
    }
    else
    {
        aCertificateSequence.realloc(nCertificateBinarySize);
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.getArray());
    }

    uno::Sequence<sal_Int8> aPrivateKeySequence;
    std::string aPrivateKeyString(reinterpret_cast<const char*>(pPrivateKeyBinary), nPrivateKeySize);
    std::string aPrivateKeyBase64String = extractPrivateKey(aPrivateKeyString);
    if (!aPrivateKeyBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aPrivateKeyBase64String);
        comphelper::Base64::decode(aPrivateKeySequence, aBase64OUString);
    }
    else
    {
        aPrivateKeySequence.realloc(nPrivateKeySize);
        std::copy(pPrivateKeyBinary, pPrivateKeyBinary + nPrivateKeySize, aPrivateKeySequence.getArray());
    }

    uno::Reference<security::XCertificate> xCertificate = xCertificateCreator->createDERCertificateWithPrivateKey(aCertificateSequence, aPrivateKeySequence);

    if (!xCertificate.is())
        return false;

    SolarMutexGuard aGuard;

    return pObjectShell->SignDocumentContentUsingCertificate(xCertificate);
}

static bool doc_addCertificate(LibreOfficeKitDocument* pThis,
                                  const unsigned char* pCertificateBinary, const int nCertificateBinarySize)
{
    comphelper::ProfileZone aZone("doc_addCertificate");

    if (!xContext.is())
        return false;

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    if (!pDocument->mxComponent.is())
        return false;

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(pDocument->mxComponent.get());
    if (!pBaseModel)
        return false;

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();

    if (!pObjectShell)
        return false;

    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    if (!xSecurityContext.is())
        return false;

    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Reference<xml::crypto::XCertificateCreator> xCertificateCreator(xSecurityEnvironment, uno::UNO_QUERY);

    if (!xCertificateCreator.is())
        return false;

    uno::Sequence<sal_Int8> aCertificateSequence;

    std::string aCertificateString(reinterpret_cast<const char*>(pCertificateBinary), nCertificateBinarySize);
    std::string aCertificateBase64String = extractCertificate(aCertificateString);
    if (!aCertificateBase64String.empty())
    {
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String);
        comphelper::Base64::decode(aCertificateSequence, aBase64OUString);
    }
    else
    {
        aCertificateSequence.realloc(nCertificateBinarySize);
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.getArray());
    }

    uno::Reference<security::XCertificate> xCertificate = xCertificateCreator->addDERCertificateToTheDatabase(aCertificateSequence, u"TCu,Cu,Tu"_ustr);

    if (!xCertificate.is())
        return false;

    SAL_INFO("lok", "Certificate Added = IssuerName: " << xCertificate->getIssuerName() << " SubjectName: " << xCertificate->getSubjectName());

    return true;
}

static int doc_getSignatureState(LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getSignatureState");

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    if (!pDocument->mxComponent.is())
        return int(SignatureState::UNKNOWN);

    SfxBaseModel* pBaseModel = dynamic_cast<SfxBaseModel*>(pDocument->mxComponent.get());
    if (!pBaseModel)
        return int(SignatureState::UNKNOWN);

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    if (!pObjectShell)
        return int(SignatureState::UNKNOWN);

    SolarMutexGuard aGuard;

    pObjectShell->RecheckSignature(false);

    return int(pObjectShell->GetDocumentSignatureState());
}

static void doc_resizeWindow(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId,
                             const int nWidth, const int nHeight)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        SetLastExceptionMsg(u"Document doesn't support dialog resizing, or window not found."_ustr);
        return;
    }

    pWindow->SetSizePixel(Size(nWidth, nHeight));
}

static void doc_completeFunction(LibreOfficeKitDocument* pThis, const char* pFunctionName)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    pDoc->completeFunction(OUString::fromUtf8(pFunctionName));
}


static void doc_sendFormFieldEvent(LibreOfficeKitDocument* pThis, const char* pArguments)
{
    SolarMutexGuard aGuard;

    // Supported in Writer only
    if (doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT)
            return;

    StringMap aMap(jsdialog::jsonToStringMap(pArguments));
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering!"_ustr);
        return;
    }

    // Sanity check
    if (aMap.find(u"type"_ustr) == aMap.end() || aMap.find(u"cmd"_ustr) == aMap.end())
    {
        SetLastExceptionMsg(u"Wrong arguments for sendFormFieldEvent!"_ustr);
        return;
    }

    pDoc->executeFromFieldEvent(aMap);
}

static bool doc_renderSearchResult(LibreOfficeKitDocument* pThis,
                                     const char* pSearchResult, unsigned char** pBitmapBuffer,
                                     int* pWidth, int* pHeight, size_t* pByteSize)
{
    if (doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT)
        return false;

    if (pBitmapBuffer == nullptr)
        return false;

    if (!pSearchResult || pSearchResult[0] == '\0')
        return false;

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return false;
    }

    auto aRectangleVector = pDoc->getSearchResultRectangles(pSearchResult);

    // combine into a rectangle union
    basegfx::B2DRange aRangeUnion;
    for (basegfx::B2DRange const & rRange : aRectangleVector)
    {
        aRangeUnion.expand(rRange);
    }

    int aPixelWidth = o3tl::convert(aRangeUnion.getWidth(), o3tl::Length::twip, o3tl::Length::px);
    int aPixelHeight = o3tl::convert(aRangeUnion.getHeight(), o3tl::Length::twip, o3tl::Length::px);

    size_t nByteSize = aPixelWidth * aPixelHeight * 4;

    *pWidth = aPixelWidth;
    *pHeight = aPixelHeight;
    *pByteSize = nByteSize;

    auto* pBuffer = static_cast<unsigned char*>(std::malloc(nByteSize));

    doc_paintTile(pThis, pBuffer,
        aPixelWidth, aPixelHeight,
        aRangeUnion.getMinX(), aRangeUnion.getMinY(),
        aRangeUnion.getWidth(), aRangeUnion.getHeight());

    *pBitmapBuffer = pBuffer;

    return true;
}

static void doc_sendContentControlEvent(LibreOfficeKitDocument* pThis, const char* pArguments)
{
    SolarMutexGuard aGuard;

    // Supported in Writer only
    if (doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT)
    {
        return;
    }

    if (SfxViewShell::IsCurrentLokViewReadOnly())
        return;

    StringMap aMap(jsdialog::jsonToStringMap(pArguments));
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg(u"Document doesn't support tiled rendering"_ustr);
        return;
    }

    // Sanity check
    if (aMap.find(u"type"_ustr) == aMap.end())
    {
        SetLastExceptionMsg(u"Missing 'type' argument for sendContentControlEvent"_ustr);
        return;
    }

    pDoc->executeContentControlEvent(aMap);
}

static void doc_setViewTimezone(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId,
                                const char* pTimezone)
{
    comphelper::ProfileZone aZone("doc_setViewTimezone");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    // Leave the default if we get a null timezone.
    if (pTimezone)
    {
        OUString sTimezone = OStringToOUString(pTimezone, RTL_TEXTENCODING_UTF8);
        SfxLokHelper::setViewTimezone(nId, true, sTimezone);
    }
}

static void doc_setViewReadOnly(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId, const bool readOnly)
{
    comphelper::ProfileZone aZone("doc_setViewReadOnly");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxLokHelper::setViewReadOnly(nId, readOnly);
}

static void doc_setAllowChangeComments(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId, const bool allow)
{
    comphelper::ProfileZone aZone("doc_setAllowChangeComments");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxLokHelper::setAllowChangeComments(nId, allow);
}

static void doc_setAccessibilityState(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* pThis, int nId, bool nEnabled)
{
    SolarMutexGuard aGuard;

    int nDocType = getDocumentType(pThis);
    if (!(nDocType == LOK_DOCTYPE_TEXT || nDocType == LOK_DOCTYPE_PRESENTATION || nDocType == LOK_DOCTYPE_SPREADSHEET))
        return;

    SfxLokHelper::setAccessibilityState(nId, nEnabled);
}

static char* lo_getError (LibreOfficeKit *pThis)
{
    comphelper::ProfileZone aZone("lo_getError");

    SolarMutexGuard aGuard;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    return convertOUString(pLib->maLastExceptionMsg);
}

static void lo_freeError(char* pFree)
{
    free(pFree);
}

static char* lo_getFilterTypes(LibreOfficeKit* pThis)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLibreOffice_Impl* pImpl = static_cast<LibLibreOffice_Impl*>(pThis);

    if (!xSFactory.is())
        xSFactory = comphelper::getProcessServiceFactory();

    if (!xSFactory.is())
    {
        pImpl->maLastExceptionMsg = u"Service factory is not available"_ustr;
        return nullptr;
    }

    uno::Reference<container::XNameAccess> xTypeDetection(xSFactory->createInstance(u"com.sun.star.document.TypeDetection"_ustr), uno::UNO_QUERY);
    const uno::Sequence<OUString> aTypes = xTypeDetection->getElementNames();
    tools::JsonWriter aJson;
    for (const OUString& rType : aTypes)
    {
        uno::Sequence<beans::PropertyValue> aValues;
        if (xTypeDetection->getByName(rType) >>= aValues)
        {
            auto it = std::find_if(std::cbegin(aValues), std::cend(aValues), [](const beans::PropertyValue& rValue) { return rValue.Name == "MediaType"; });
            OUString aValue;
            if (it != std::cend(aValues) && (it->Value >>= aValue) && !aValue.isEmpty())
            {
                auto typeNode = aJson.startNode(rType.toUtf8());
                aJson.put("MediaType", aValue.toUtf8());
            }
        }
    }

    return convertOString(aJson.finishAndGetAsOString());
}

static void lo_setOptionalFeatures(LibreOfficeKit* pThis, unsigned long long const features)
{
    comphelper::ProfileZone aZone("lo_setOptionalFeatures");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLibreOffice_Impl *const pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->mOptionalFeatures = features;
    if (features & LOK_FEATURE_PART_IN_INVALIDATION_CALLBACK)
        comphelper::LibreOfficeKit::setPartInInvalidation(true);
    if (features & LOK_FEATURE_NO_TILED_ANNOTATIONS)
        comphelper::LibreOfficeKit::setTiledAnnotations(false);
    if (features & LOK_FEATURE_RANGE_HEADERS)
        comphelper::LibreOfficeKit::setRangeHeaders(true);
    if (features & LOK_FEATURE_VIEWID_IN_VISCURSOR_INVALIDATION_CALLBACK)
        comphelper::LibreOfficeKit::setViewIdForVisCursorInvalidation(true);
}

static void lo_setDocumentPassword(LibreOfficeKit* pThis,
        const char* pURL, const char* pPassword)
{
    comphelper::ProfileZone aZone("lo_setDocumentPassword");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    assert(pThis);
    assert(pURL);
    LibLibreOffice_Impl *const pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    assert(pLib->mInteractionMap.find(OString(pURL)) != pLib->mInteractionMap.end());
    pLib->mInteractionMap.find(OString(pURL))->second->SetPassword(pPassword);
}

static char* lo_getVersionInfo(SAL_UNUSED_PARAMETER LibreOfficeKit* /*pThis*/)
{
    SetLastExceptionMsg();
    return convertOUString(ReplaceStringHookProc(
        u"{ "
        "\"ProductName\": \"%PRODUCTNAME\", "
        "\"ProductVersion\": \"%PRODUCTVERSION\", "
        "\"ProductExtension\": \"%PRODUCTEXTENSION\", "
        "\"BuildId\": \"%BUILDID\""
#if BUILDCONFIG_RECORDED
        ", \"BuildConfig\": \"" BUILDCONFIG "\""
#endif
        " }"_ustr));
}

static void aBasicErrorFunc(const OUString& rError, const OUString& rAction)
{
    OString aBuffer = "Unexpected dialog: " +
        OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US) +
        " Error: " +
        OUStringToOString(rError, RTL_TEXTENCODING_ASCII_US);

    fprintf(stderr, "Unexpected basic error dialog '%s'\n", aBuffer.getStr());
}

static bool initialize_uno(const OUString& aAppProgramURL)
{
#ifdef IOS
    // For iOS we already hardcode the inifile as "rc" in the .app directory.
    rtl::Bootstrap::setIniFilename(aAppProgramURL + "/" SAL_CONFIGFILE("fundamental"));
    xContext = cppu::defaultBootstrap_InitialComponentContext(aAppProgramURL + "/rc");
#elif defined MACOSX
    rtl::Bootstrap::setIniFilename(aAppProgramURL + "/../Resources/" SAL_CONFIGFILE("soffice"));
    xContext = cppu::defaultBootstrap_InitialComponentContext();
#else
    rtl::Bootstrap::setIniFilename(aAppProgramURL + "/" SAL_CONFIGFILE("soffice"));
    xContext = cppu::defaultBootstrap_InitialComponentContext();
#endif

    if (!xContext.is())
    {
        SetLastExceptionMsg(u"XComponentContext could not be created"_ustr);
        SAL_INFO("lok", "XComponentContext could not be created");
        return false;
    }

    xFactory = xContext->getServiceManager();
    if (!xFactory.is())
    {
        SetLastExceptionMsg(u"XMultiComponentFactory could not be created"_ustr);
        SAL_INFO("lok", "XMultiComponentFactory could not be created");
        return false;
    }

    xSFactory.set(xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    SAL_INFO("lok", "Uno initialized  - " <<  xContext.is());

    // set UserInstallation to user profile dir in test/user-template
//    rtl::Bootstrap aDefaultVars;
//    aDefaultVars.set(OUString("UserInstallation"), aAppProgramURL + "../registry" );
    // configmgr setup ?

    return true;
}

// pre-unipoll version.
static void lo_startmain(void*)
{
    osl_setThreadName("lo_startmain");

    if (comphelper::SolarMutex::get())
        Application::GetSolarMutex().tryToAcquire();

    Application::UpdateMainThread();

    soffice_main();

    Application::ReleaseSolarMutex();
}

// unipoll version.
static void lo_runLoop(LibreOfficeKit* /*pThis*/,
                       LibreOfficeKitPollCallback pPollCallback,
                       LibreOfficeKitWakeCallback pWakeCallback,
                       void* pData)
{
#if defined(IOS) || defined(ANDROID) || defined(__EMSCRIPTEN__)
    Application::GetSolarMutex().acquire();
#endif

    {
        SolarMutexGuard aGuard;

        vcl::lok::registerPollCallbacks(pPollCallback, pWakeCallback, pData);
        Application::UpdateMainThread();
        soffice_main();
    }
#if defined(IOS) || defined(ANDROID) || defined(__EMSCRIPTEN__)
    vcl::lok::unregisterPollCallbacks();
    Application::ReleaseSolarMutex();
#endif
}

static bool bInitialized = false;

static void lo_status_indicator_callback(void *data, comphelper::LibreOfficeKit::statusIndicatorCallbackType type, int percent, const char* pText)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(data);

    if (!pLib->mpCallback)
        return;

    switch (type)
    {
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::Start:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_START, pText, pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::SetValue:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE,
            OUString(OUString::number(percent)).toUtf8().getStr(), pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::Finish:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_FINISH, nullptr, pLib->mpCallbackData);
        break;
    }
}

/// Used by preloadData (LibreOfficeKit) for providing different shortcuts for different languages.
static void preLoadShortCutAccelerators()
{
    std::unordered_map<OUString, css::uno::Reference<com::sun::star::ui::XAcceleratorConfiguration>>& acceleratorConfs = SfxLokHelper::getAcceleratorConfs();
    css::uno::Sequence<OUString> installedLocales(officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
    OUString actualLang = officecfg::Setup::L10N::ooLocale::get();

    for (sal_Int32 i = 0; i < installedLocales.getLength(); i++)
    {
        // Set the UI language to current one, before creating the accelerator.
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Setup::L10N::ooLocale::set(installedLocales[i], batch);
        batch->commit();

        // Supported module names: Writer, Calc, Draw, Impress
        static constexpr OUString supportedModuleNames[] = {
            u"com.sun.star.text.TextDocument"_ustr,
            u"com.sun.star.sheet.SpreadsheetDocument"_ustr,
            u"com.sun.star.drawing.DrawingDocument"_ustr,
            u"com.sun.star.presentation.PresentationDocument"_ustr,
        };
        // Create the accelerators.
        for (const OUString& supportedModuleName : supportedModuleNames)
        {
            OUString key = supportedModuleName + installedLocales[i];
            acceleratorConfs[key] = svt::AcceleratorExecute::lok_createNewAcceleratorConfiguration(::comphelper::getProcessComponentContext(), supportedModuleName);
        }
    }

    // Set the UI language back to default one.
    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Setup::L10N::ooLocale::set(actualLang, batch);
    batch->commit();
}

void setLanguageToolConfig();

/// Used only by LibreOfficeKit when used by Online to pre-initialize
static void preloadData()
{
    comphelper::ProfileZone aZone("preload data");

    // Create user profile in the temp directory for loading the dictionaries
    OUString sUserPath;
    rtl::Bootstrap::get(u"UserInstallation"_ustr, sUserPath);
    utl::TempFileNamed aTempDir(nullptr, true);
    aTempDir.EnableKillingFile();
    rtl::Bootstrap::set(u"UserInstallation"_ustr, aTempDir.GetURL());

    // Register the bundled extensions
    desktop::Desktop::SynchronizeExtensionRepositories(true);
    bool bAbort = desktop::Desktop::CheckExtensionDependencies();
    if(bAbort)
        std::cerr << "CheckExtensionDependencies failed" << std::endl;

    // inhibit forced 2nd synchronization from Main
    ::rtl::Bootstrap::set( u"DISABLE_EXTENSION_SYNCHRONIZATION"_ustr, u"true"_ustr);

    std::cerr << "Preload textencodings"; // sal_textenc
    // Use RTL_TEXTENCODING_MS_1250 to trigger Impl_getTextEncodingData
    // to dlopen sal_textenclo
    (void)OUStringToOString(u"arbitrary string", RTL_TEXTENCODING_MS_1250);
    std::cerr << "\n";

    // setup LanguageTool config before spell checking init
    setLanguageToolConfig();

    // preload all available dictionaries
    linguistic2::DictionaryList::create(comphelper::getProcessComponentContext());
    css::uno::Reference<css::linguistic2::XLinguServiceManager> xLngSvcMgr =
        css::linguistic2::LinguServiceManager::create(comphelper::getProcessComponentContext());
    css::uno::Reference<linguistic2::XSpellChecker> xSpellChecker(xLngSvcMgr->getSpellChecker());

    std::cerr << "Preloading dictionaries: ";
    css::uno::Reference<linguistic2::XSupportedLocales> xSpellLocales(xSpellChecker, css::uno::UNO_QUERY_THROW);
    uno::Sequence< css::lang::Locale > aLocales = xSpellLocales->getLocales();
    for (auto& it : aLocales)
    {
        std::cerr << LanguageTag::convertToBcp47(it) << " ";
        css::beans::PropertyValues aNone;
        xSpellChecker->isValid(u"forcefed"_ustr, it, aNone);
    }
    std::cerr << "\n";

    // Hack to load and cache the module liblocaledata_others.so which is not loaded normally
    // (when loading dictionaries of just non-Asian locales). Creating a XCalendar4 of one Asian locale
    // will cheaply load this missing "others" locale library. Appending an Asian locale in
    // LOK_ALLOWLIST_LANGUAGES env-var also works but at the cost of loading that dictionary.
    css::uno::Reference< css::i18n::XCalendar4 > xCal = css::i18n::LocaleCalendar2::create(comphelper::getProcessComponentContext());
    css::lang::Locale aAsianLocale = { u"hi"_ustr, u"IN"_ustr, {} };
    xCal->loadDefaultCalendar(aAsianLocale);

    // preload all available thesauri
    css::uno::Reference<linguistic2::XThesaurus> xThesaurus(xLngSvcMgr->getThesaurus());
    css::uno::Reference<linguistic2::XSupportedLocales> xThesLocales(xSpellChecker, css::uno::UNO_QUERY_THROW);
    aLocales = xThesLocales->getLocales();
    std::cerr << "Preloading thesauri: ";
    for (auto& it : aLocales)
    {
        std::cerr << LanguageTag::convertToBcp47(it) << " ";
        css::beans::PropertyValues aNone;
        xThesaurus->queryMeanings(u"forcefed"_ustr, it, aNone);
    }
    std::cerr << "\n";

    std::cerr << "Preloading breakiterator\n";
    if (aLocales.getLength())
    {
        css::uno::Reference< css::i18n::XBreakIterator > xBreakIterator = css::i18n::BreakIterator::create(xContext);
        css::i18n::LineBreakUserOptions aUserOptions;
        css::i18n::LineBreakHyphenationOptions aHyphOptions( LinguMgr::GetHyphenator(), css::uno::Sequence<beans::PropertyValue>(), 1 );
        xBreakIterator->getLineBreak(u""_ustr, /*nMaxBreakPos*/0, aLocales[0], /*nMinBreakPos*/0, aHyphOptions, aUserOptions);
    }

    css::uno::Reference< css::ui::XAcceleratorConfiguration > xGlobalCfg = css::ui::GlobalAcceleratorConfiguration::create(
        comphelper::getProcessComponentContext());
    xGlobalCfg->getAllKeyEvents();

    std::cerr << "Preload icons\n";
    ImageTree &images = ImageTree::get();
    images.getImageUrl(u"forcefed.png"_ustr, u"style"_ustr, u"FO_oo"_ustr);

    std::cerr << "Preload short cut accelerators\n";
    preLoadShortCutAccelerators();

    std::cerr << "Preload languages\n";

    // force load language singleton
    SvtLanguageTable::HasLanguageType(LANGUAGE_SYSTEM);
    (void)LanguageTag::isValidBcp47(u"foo"_ustr, nullptr);

    std::cerr << "Preload fonts\n";

    // Initialize fonts.
    css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv = css::linguistic2::LinguServiceManager::create(xContext);
    if (xLangSrv.is())
    {
        css::uno::Reference<css::linguistic2::XSpellChecker> xSpell = xLangSrv->getSpellChecker();
        if (xSpell.is())
            aLocales = xSpell->getLocales();
    }

    for (const auto& aLocale : aLocales)
    {
        //TODO: Add more types and cache more aggressively. For now this initializes the fontcache.
        using namespace ::com::sun::star::i18n::ScriptType;
        LanguageType nLang;
        nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType(aLocale, false), LATIN);
        OutputDevice::GetDefaultFont(DefaultFontType::LATIN_SPREADSHEET, nLang, GetDefaultFontFlags::OnlyOne);
        nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType(aLocale, false), ASIAN);
        OutputDevice::GetDefaultFont(DefaultFontType::CJK_SPREADSHEET, nLang, GetDefaultFontFlags::OnlyOne);
        nLang = MsLangId::resolveSystemLanguageByScriptType(LanguageTag::convertToLanguageType(aLocale, false), COMPLEX);
        OutputDevice::GetDefaultFont(DefaultFontType::CTL_SPREADSHEET, nLang, GetDefaultFontFlags::OnlyOne);
    }

    std::cerr << "Preload config\n";
#if defined __GNUC__ || defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
    static SvtOptionsDialogOptions aDialogOptions;
    static SvtCTLOptions aSvtCTLOptions;
    static svtools::ColorConfig aColorConfig;
    static SvtMiscOptions aSvtMiscOptions;
    static SvtCommandOptions aSvtCommandOptions;
    static SvtLinguConfig aSvtLinguConfig;
    static SvtModuleOptions aSvtModuleOptions;
    static SvtPathOptions aSvtPathOptions;
    static SvtSearchOptions aSvtSearchOptions;
    static SvtSysLocaleOptions aSvtSysLocaleOptions;
    static SvtUserOptions aSvtUserOptions;
    //static SvtViewOptions aSvtViewOptions;
    static MouseSettings aMouseSettings;
    static StyleSettings aStyleSettings;
    static MiscSettings aMiscSettings;
    static HelpSettings aHelpSettings;
    static AllSettings aAllSettings;
#if defined __GNUC__ || defined __clang__
#pragma GCC diagnostic pop
#endif

    static constexpr OUString preloadComponents[] = {
        u"private:factory/swriter"_ustr,
        u"private:factory/scalc"_ustr,
        u"private:factory/simpress"_ustr,
        u"private:factory/sdraw"_ustr
    };
    // getting the remote LibreOffice service manager
    uno::Reference<frame::XDesktop2> xCompLoader(frame::Desktop::create(xContext));

    // Preload and close each of the main components once to initialize global state
    uno::Sequence<css::beans::PropertyValue> szEmptyArgs(0);
    for (const auto& component : preloadComponents)
    {
        auto xComp = xCompLoader->loadComponentFromURL(component, u"_blank"_ustr, 0, szEmptyArgs);
        xComp->dispose();
    }

    // Set user profile's path back to the original one
    rtl::Bootstrap::set(u"UserInstallation"_ustr, sUserPath);
}

namespace {

static void activateNotebookbar(std::u16string_view rApp)
{
    OUString aPath = OUString::Concat("org.openoffice.Office.UI.ToolbarMode/Applications/") + rApp;

    const utl::OConfigurationTreeRoot aAppNode(xContext, aPath, true);

    if (aAppNode.isValid())
    {
        static constexpr OUString sNoteBookbarName(u"notebookbar_online.ui"_ustr);
        aAppNode.setNodeValue(u"Active"_ustr, Any(sNoteBookbarName));

        const utl::OConfigurationNode aImplsNode = aAppNode.openNode(u"Modes"_ustr);
        const Sequence<OUString> aModeNodeNames( aImplsNode.getNodeNames() );

        for (const auto& rModeNodeName : aModeNodeNames)
        {
            const utl::OConfigurationNode aImplNode(aImplsNode.openNode(rModeNodeName));
            if (!aImplNode.isValid())
                continue;

            OUString aCommandArg = comphelper::getString(aImplNode.getNodeValue(u"CommandArg"_ustr));
            if (aCommandArg == "notebookbar.ui")
                aImplNode.setNodeValue(u"CommandArg"_ustr, Any(sNoteBookbarName));
        }

        aAppNode.commit();
    }
}

void setHelpRootURL()
{
    const char* pHelpRootURL = ::getenv("LOK_HELP_URL");
    if (pHelpRootURL)
    {
        OUString aHelpRootURL = OStringToOUString(pHelpRootURL, RTL_TEXTENCODING_UTF8);
        try
        {
            std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Help::HelpRootURL::set(aHelpRootURL, batch);
            batch->commit();
        }
        catch (uno::Exception const& rException)
        {
            SAL_WARN("lok", "Failed to set the help root URL: " << rException.Message);
        }
    }
}

void setCertificateDir()
{
    const char* pEnvVarString = ::getenv("LO_CERTIFICATE_DATABASE_PATH");
    if (pEnvVarString)
    {
        OUString aCertificateDatabasePath = OStringToOUString(pEnvVarString, RTL_TEXTENCODING_UTF8);
        try
        {
            std::shared_ptr<comphelper::ConfigurationChanges> pBatch(comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Security::Scripting::CertDir::set(aCertificateDatabasePath, pBatch);
            officecfg::Office::Common::Security::Scripting::ManualCertDir::set(aCertificateDatabasePath, pBatch);
            pBatch->commit();
        }
        catch (uno::Exception const& rException)
        {
            SAL_WARN("lok", "Failed to set the NSS certificate database directory: " << rException.Message);
        }
    }
}

void setDeeplConfig()
{
    const char* pAPIUrlString = ::getenv("DEEPL_API_URL");
    const char* pAuthKeyString = ::getenv("DEEPL_AUTH_KEY");
    if (pAPIUrlString && pAuthKeyString)
    {
        OUString aAPIUrl = OStringToOUString(pAPIUrlString, RTL_TEXTENCODING_UTF8);
        OUString aAuthKey = OStringToOUString(pAuthKeyString, RTL_TEXTENCODING_UTF8);
        try
        {
            std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
            officecfg::Office::Linguistic::Translation::Deepl::ApiURL::set(aAPIUrl, batch);
            officecfg::Office::Linguistic::Translation::Deepl::AuthKey::set(aAuthKey, batch);
            batch->commit();
        }
        catch(uno::Exception const& rException)
        {
            SAL_WARN("lok", "Failed to set Deepl API settings: " << rException.Message);
        }
    }
}

void setLanguageToolConfig()
{
    const char* pEnabled = ::getenv("LANGUAGETOOL_ENABLED");
    const char* pBaseUrlString = ::getenv("LANGUAGETOOL_BASEURL");

    if (pEnabled && pBaseUrlString)
    {
        const char* pUsername = ::getenv("LANGUAGETOOL_USERNAME");
        const char* pApikey = ::getenv("LANGUAGETOOL_APIKEY");
        const char* pSSLVerification = ::getenv("LANGUAGETOOL_SSL_VERIFICATION");
        const char* pRestProtocol = ::getenv("LANGUAGETOOL_RESTPROTOCOL");

        OUString aEnabled = OStringToOUString(pEnabled, RTL_TEXTENCODING_UTF8);
        if (aEnabled != "true")
            return;
        OUString aBaseUrl = OStringToOUString(pBaseUrlString, RTL_TEXTENCODING_UTF8);
        try
        {
            using LanguageToolCfg = officecfg::Office::Linguistic::GrammarChecking::LanguageTool;
            auto batch(comphelper::ConfigurationChanges::create());

            LanguageToolCfg::BaseURL::set(aBaseUrl, batch);
            LanguageToolCfg::IsEnabled::set(true, batch);
            if (pSSLVerification)
            {
                OUString aSSLVerification = OStringToOUString(pSSLVerification, RTL_TEXTENCODING_UTF8);
                LanguageToolCfg::SSLCertVerify::set(aSSLVerification == "true", batch);
            }
            if (pRestProtocol)
            {
                OUString aRestProtocol = OStringToOUString(pRestProtocol, RTL_TEXTENCODING_UTF8);
                LanguageToolCfg::RestProtocol::set(aRestProtocol, batch);
            }
            if (pUsername && pApikey)
            {
                OUString aUsername = OStringToOUString(pUsername, RTL_TEXTENCODING_UTF8);
                OUString aApiKey = OStringToOUString(pApikey, RTL_TEXTENCODING_UTF8);
                LanguageToolCfg::Username::set(aUsername, batch);
                LanguageToolCfg::ApiKey::set(aApiKey, batch);
            }
            batch->commit();

            css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv =
                css::linguistic2::LinguServiceManager::create(xContext);
            if (xLangSrv.is())
            {
                css::uno::Reference<css::linguistic2::XSpellChecker> xSpell = xLangSrv->getSpellChecker();
                if (xSpell.is())
                {
                    Sequence<OUString> aEmpty;
                    Sequence<css::lang::Locale> aLocales = xSpell->getLocales();

                    uno::Reference<linguistic2::XProofreader> xGC(
                        xContext->getServiceManager()->createInstanceWithContext(u"org.openoffice.lingu.LanguageToolGrammarChecker"_ustr, xContext),
                        uno::UNO_QUERY_THROW);
                    uno::Reference<linguistic2::XSupportedLocales> xSuppLoc(xGC, uno::UNO_QUERY_THROW);

                    for (int itLocale = 0; itLocale < aLocales.getLength(); itLocale++)
                    {
                        // turn off spell checker if LanguageTool supports the locale already
                        if (xSuppLoc->hasLocale(aLocales[itLocale]))
                            xLangSrv->setConfiguredServices(
                                SN_SPELLCHECKER, aLocales[itLocale], aEmpty);
                    }
                }
            }
        }
        catch(uno::Exception const& rException)
        {
            SAL_WARN("lok", "Failed to set LanguageTool API settings: " << rException.Message);
        }
    }
}

}

static int lo_initialize(LibreOfficeKit* pThis, const char* pAppPath, const char* pUserProfileUrl)
{
    enum {
        PRE_INIT,     // setup shared data in master process
        SECOND_INIT,  // complete init. after fork
        FULL_INIT     // do a standard complete init.
    } eStage;

    // Did we do a pre-initialize
    static bool bPreInited = false;
    static bool bUnipoll = false;
    static bool bProfileZones = false;
    static bool bNotebookbar = false;

    { // cf. string lifetime for preinit
        std::vector<OUString> aOpts;

        // ':' delimited options - avoiding ABI change for new parameters
        const char *pOptions = getenv("SAL_LOK_OPTIONS");
        if (pOptions)
            aOpts = comphelper::string::split(OUString(pOptions, strlen(pOptions), RTL_TEXTENCODING_UTF8), ':');
        for (const auto &it : aOpts)
        {
            if (it == "unipoll")
                bUnipoll = true;
            else if (it == "profile_events")
                bProfileZones = true;
            else if (it == "sc_no_grid_bg")
                comphelper::LibreOfficeKit::setCompatFlag(
                    comphelper::LibreOfficeKit::Compat::scNoGridBackground);
            else if (it == "sc_print_twips_msgs")
                comphelper::LibreOfficeKit::setCompatFlag(
                    comphelper::LibreOfficeKit::Compat::scPrintTwipsMsgs);
            else if (it == "notebookbar")
                bNotebookbar = true;
        }
    }

    char* pAllowlist = ::getenv("LOK_HOST_ALLOWLIST");
    if (pAllowlist)
    {
        HostFilter::setAllowedHostsRegex(pAllowlist);
    }

    // What stage are we at ?
    if (pThis == nullptr)
    {
        eStage = PRE_INIT;
        if (lok_preinit_2_called)
        {
            SAL_INFO("lok", "Create libreoffice object");
            gImpl = new LibLibreOffice_Impl();
        }
    }
    else if (bPreInited)
        eStage = SECOND_INIT;
    else
        eStage = FULL_INIT;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);

    if (bInitialized)
        return 1;

    // Turn profile zones on early
    if (bProfileZones && eStage == SECOND_INIT)
    {
        comphelper::TraceEvent::startRecording();
        traceEventDumper = new TraceEventDumper();
    }

    comphelper::ProfileZone aZone("lok-init");

    if (eStage == PRE_INIT)
    {
        rtl_alloc_preInit(true);

        // Set the default timezone to the TZ envar, if set.
        const char* tz = ::getenv("TZ");
        SfxLokHelper::setDefaultTimezone(!!tz, tz ? OStringToOUString(tz, RTL_TEXTENCODING_UTF8)
                                                  : OUString());
#ifdef UNX
        if (urandom < 0)
            urandom = open("/dev/urandom", O_RDONLY);
#endif
    }

    if (eStage != SECOND_INIT)
        comphelper::LibreOfficeKit::setActive();

    if (eStage != PRE_INIT)
        comphelper::LibreOfficeKit::setStatusIndicatorCallback(lo_status_indicator_callback, pLib);

    if (pUserProfileUrl && eStage != PRE_INIT)
    {
        OUString url(
            pUserProfileUrl, strlen(pUserProfileUrl), RTL_TEXTENCODING_UTF8);
        OUString path;
        if (url.startsWithIgnoreAsciiCase("vnd.sun.star.pathname:", &path))
        {
            OUString url2;
            osl::FileBase::RC e = osl::FileBase::getFileURLFromSystemPath(
                path, url2);
            if (e == osl::FileBase::E_None)
                url = url2;
            else
                SAL_WARN("lok", "resolving <" << url << "> failed with " << +e);
        }
        rtl::Bootstrap::set(u"UserInstallation"_ustr, url);
        if (eStage == SECOND_INIT)
        {
            comphelper::rng::reseed();
            utl::Bootstrap::reloadData();
        }
    }

    OUString aAppPath;
    if (pAppPath)
    {
        aAppPath = OUString(pAppPath, strlen(pAppPath), RTL_TEXTENCODING_UTF8);
    }
    else
    {
#if defined ANDROID || defined EMSCRIPTEN
        aAppPath = OUString::fromUtf8(lo_get_app_data_dir()) + "/program";
#else
        // Fun conversion dance back and forth between URLs and system paths...
        OUString aAppURL;
        ::osl::Module::getUrlFromAddress( reinterpret_cast< oslGenericFunction >(lo_initialize),
                                          aAppURL);
        osl::FileBase::getSystemPathFromFileURL( aAppURL, aAppPath );
#endif

#ifdef IOS
        // The above gives something like
        // "/private/var/containers/Bundle/Application/953AA851-CC15-4C60-A2CB-C2C6F24E6F71/Foo.app/Foo",
        // and we want to drop the final component (the binary name).
        sal_Int32 lastSlash = aAppPath.lastIndexOf('/');
        assert(lastSlash > 0);
        aAppPath = aAppPath.copy(0, lastSlash);
#endif
    }

    OUString aAppURL;
    if (osl::FileBase::getFileURLFromSystemPath(aAppPath, aAppURL) != osl::FileBase::E_None)
        return 0;

#ifdef IOS
    // A LibreOffice-using iOS app should have the ICU data file in the app bundle. Initialize ICU
    // to use that.
    NSString *bundlePath = [[NSBundle mainBundle] bundlePath];

    int fd = open([[bundlePath stringByAppendingPathComponent:@"ICU.dat"] UTF8String], O_RDONLY);
    if (fd == -1)
        NSLog(@"Could not open ICU data file %s", [[bundlePath stringByAppendingPathComponent:@"ICU.dat"] UTF8String]);
    else
    {
        struct stat st;
        if (fstat(fd, &st) == -1)
            NSLog(@"fstat on ICU data file failed: %s", strerror(errno));
        else
        {
            void *icudata = mmap(0, (size_t) st.st_size, PROT_READ, MAP_FILE|MAP_PRIVATE, fd, 0);
            if (icudata == MAP_FAILED)
                NSLog(@"mmap failed: %s", strerror(errno));
            else
            {
                UErrorCode icuStatus = U_ZERO_ERROR;
                udata_setCommonData(icudata, &icuStatus);
                if (U_FAILURE(icuStatus))
                    NSLog(@"udata_setCommonData failed");
                else
                {
                    // Quick test that ICU works...
                    UConverter *cnv = ucnv_open("iso-8859-3", &icuStatus);
                    if (U_SUCCESS(icuStatus))
                        ucnv_close(cnv);
                    else
                        NSLog(@"ucnv_open() failed: %s", u_errorName(icuStatus));
                }
            }
        }
        close(fd);
    }
#endif

    try
    {
        if (eStage != SECOND_INIT)
        {
            SAL_INFO("lok", "Attempting to initialize UNO");

            if (!initialize_uno(aAppURL))
                return false;

            // Force headless -- this is only for bitmap rendering.
            rtl::Bootstrap::set(u"SAL_USE_VCLPLUGIN"_ustr, u"svp"_ustr);

            // We specifically need to make sure we have the "headless"
            // command arg set (various code specifically checks via
            // CommandLineArgs):
            desktop::Desktop::GetCommandLineArgs().setHeadless();

#ifdef IOS
            if (InitVCL() && [NSThread isMainThread])
            {
                static bool bFirstTime = true;
                if (bFirstTime)
                {
                    Application::GetSolarMutex().release();
                    bFirstTime = false;
                }
            }
            SfxApplication::GetOrCreate();
#endif

#if HAVE_FEATURE_ANDROID_LOK
            // Register the bundled extensions - so that the dictionaries work
            desktop::Desktop::SynchronizeExtensionRepositories(false);
            bool bFailed = desktop::Desktop::CheckExtensionDependencies();
            if (bFailed)
                SAL_INFO("lok", "CheckExtensionDependencies failed");
#endif

            if (eStage == PRE_INIT)
            {
                {
                    comphelper::ProfileZone aInit("Init vcl");
                    std::cerr << "Init vcl\n";
                    InitVCL();
                }

                // pre-load all component libraries.
                if (!xContext.is())
                    throw css::uno::DeploymentException(u"preInit: XComponentContext is not created"_ustr);

                css::uno::Reference< css::uno::XInterface > xService;
                xContext->getValueByName(u"/singletons/com.sun.star.lang.theServiceManager"_ustr) >>= xService;
                if (!xService.is())
                    throw css::uno::DeploymentException(u"preInit: XMultiComponentFactory is not created"_ustr);

                css::uno::Reference<css::lang::XInitialization> aService(
                    xService, css::uno::UNO_QUERY_THROW);

                // pre-requisites:
                // In order to load implementations and invoke
                // component factory it is required:
                // 1) defaultBootstrap_InitialComponentContext()
                // 2) comphelper::setProcessServiceFactory(xSFactory);
                // 3) InitVCL()
                {
                    comphelper::ProfileZone aInit("preload");
                    aService->initialize({css::uno::Any(u"preload"_ustr)});
                }
                { // Force load some modules
                    comphelper::ProfileZone aInit("preload modules");
                    VclBuilderPreload();
                    VclAbstractDialogFactory::Create();
                }

                preloadData();

                // Release Solar Mutex, lo_startmain thread should acquire it.
                Application::ReleaseSolarMutex();
            }

            setLanguageAndLocale(u"en-US"_ustr);
        }

        if (eStage != PRE_INIT)
        {
            SAL_INFO("lok", "Re-initialize temp paths");
            SvtPathOptions aOptions;
            OUString aNewTemp;
            osl::FileBase::getTempDirURL(aNewTemp);
            aOptions.SetTempPath(aNewTemp);
            {
                const char *pWorkPath = getenv("LOK_WORKDIR");
                if (pWorkPath)
                {
                    OString sWorkPath(pWorkPath);
                    aOptions.SetWorkPath(OStringToOUString(sWorkPath, RTL_TEXTENCODING_UTF8));
                }
            }
            desktop::Desktop::CreateTemporaryDirectory();

            // The RequestHandler is specifically set to be ready when all the other
            // init in Desktop::Main (run from soffice_main) is done. We can enable
            // the RequestHandler here (without starting any IPC thread;
            // shortcutting the invocation in Desktop::Main that would start the IPC
            // thread), and can then use it to wait until we're definitely ready to
            // continue.

            SAL_INFO("lok", "Enabling RequestHandler");
            RequestHandler::Enable(false);
            SAL_INFO("lok", "Starting soffice_main");
            RequestHandler::SetReady(false);
            if (!bUnipoll)
            {
                // Start the main thread only in non-unipoll mode (i.e. multithreaded).
                pLib->maThread = osl_createThread(lo_startmain, nullptr);
                SAL_INFO("lok", "Waiting for RequestHandler");
                RequestHandler::WaitForReady();
                SAL_INFO("lok", "RequestHandler ready -- continuing");
            }
            else
                InitVCL();
        }

        if (eStage != SECOND_INIT)
            ErrorRegistry::RegisterDisplay(aBasicErrorFunc);

        SAL_INFO("lok", "LOK Initialized");
        if (eStage == PRE_INIT)
            bPreInited = true;
        else
            bInitialized = true;
    }
    catch (css::uno::Exception& exception)
    {
        fprintf(stderr, "Bootstrapping exception '%s'\n",
                 OUStringToOString(exception.Message, RTL_TEXTENCODING_UTF8).getStr());
    }

    if (eStage == PRE_INIT)
    {
        comphelper::ThreadPool::getSharedOptimalPool().shutdown();
    }

// Turn off quick editing on iOS, Android and Emscripten
#if defined IOS || defined ANDROID || defined __EMSCRIPTEN__
    if (officecfg::Office::Impress::Misc::TextObject::QuickEditing::get())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Impress::Misc::TextObject::QuickEditing::set(false, batch);
        batch->commit();
    }
#endif


    setHelpRootURL();
    setCertificateDir();
    setDeeplConfig();
    setLanguageToolConfig();

    if (bNotebookbar)
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::UI::ToolbarMode::ActiveWriter::set(u"notebookbar_online.ui"_ustr, batch);
        officecfg::Office::UI::ToolbarMode::ActiveCalc::set(u"notebookbar_online.ui"_ustr, batch);
        officecfg::Office::UI::ToolbarMode::ActiveImpress::set(u"notebookbar_online.ui"_ustr, batch);
        officecfg::Office::UI::ToolbarMode::ActiveDraw::set(u"notebookbar_online.ui"_ustr, batch);
        batch->commit();

        activateNotebookbar(u"Writer");
        activateNotebookbar(u"Calc");
        activateNotebookbar(u"Impress");
        activateNotebookbar(u"Draw");
    }

    // staticize all strings.
    if (eStage == PRE_INIT)
        rtl_alloc_preInit(false);

    return bInitialized;
}

SAL_JNI_EXPORT
LibreOfficeKit *libreofficekit_hook_2(const char* install_path, const char* user_profile_url)
{
    static bool alreadyCalled = false;

    if ((!lok_preinit_2_called && !gImpl) || (lok_preinit_2_called && !alreadyCalled))
    {
        alreadyCalled = true;

        if (!lok_preinit_2_called)
        {
            SAL_INFO("lok", "Create libreoffice object");
            gImpl = new LibLibreOffice_Impl();
        }

        if (!lo_initialize(gImpl, install_path, user_profile_url))
        {
            lo_destroy(gImpl);
        }
    }
    return static_cast<LibreOfficeKit*>(gImpl);
}

SAL_JNI_EXPORT
LibreOfficeKit *libreofficekit_hook(const char* install_path)
{
    return libreofficekit_hook_2(install_path, nullptr);
}

SAL_JNI_EXPORT
int lok_preinit(const char* install_path, const char* user_profile_url)
{
    return lo_initialize(nullptr, install_path, user_profile_url);
}

SAL_JNI_EXPORT
int lok_preinit_2(const char* install_path, const char* user_profile_url, LibreOfficeKit** kit)
{
    lok_preinit_2_called = true;
    int result = lo_initialize(nullptr, install_path, user_profile_url);
    if (kit != nullptr)
        *kit = gImpl;
    return result;
}

static void lo_destroy(LibreOfficeKit* pThis)
{
    SolarMutexClearableGuard aGuard;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    gImpl = nullptr;

    SAL_INFO("lok", "LO Destroy");

    comphelper::LibreOfficeKit::setStatusIndicatorCallback(nullptr, nullptr);
    uno::Reference <frame::XDesktop2> xDesktop = frame::Desktop::create ( ::comphelper::getProcessComponentContext() );
    // FIXME: the terminate() call here is a no-op because it detects
    // that LibreOfficeKit::isActive() and then returns early!
    bool bSuccess = xDesktop.is() && xDesktop->terminate();

    if (!bSuccess)
    {
        bSuccess = GetpApp() && GetpApp()->QueryExit();
    }

    if (!bSuccess)
    {
        Application::Quit();
    }

    aGuard.clear();

    osl_joinWithThread(pLib->maThread);
    osl_destroyThread(pLib->maThread);

    delete pLib;
    bInitialized = false;
    SAL_INFO("lok", "LO Destroy Done");
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
