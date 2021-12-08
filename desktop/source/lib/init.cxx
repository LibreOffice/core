/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_features.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
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
#include <vcl/errinf.hxx>
#include <vcl/lok.hxx>
#include <o3tl/any.hxx>
#include <o3tl/unit_conversion.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <svl/zforlist.hxx>
#include <cppuhelper/bootstrap.hxx>
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
#include <comphelper/sequenceashashmap.hxx>

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
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/datatransfer/XTransferable2.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>

#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XCertificateCreator.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/i18n/LocaleCalendar2.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

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
#include <sfx2/lokcharthelper.hxx>
#include <sfx2/DocumentSigner.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <svl/numformat.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <svx/ucsubset.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/GestureEvent.hxx>
#include <vcl/svapp.hxx>
#include <unotools/resmgr.hxx>
#include <tools/fract.hxx>
#include <tools/json_writer.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/graphicfilter.hxx>
#ifdef IOS
#include <vcl/sysdata.hxx>
#endif
#include <vcl/virdev.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/dialoghelper.hxx>
#include <unicode/uchar.h>
#include <unotools/securityoptions.hxx>
#include <unotools/configmgr.hxx>
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
#include <tools/diagnose_ex.h>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/jsdialog/executor.hxx>

// Needed for getUndoManager()
#include <com/sun/star/document/XUndoManager.hpp>
#include <com/sun/star/document/XUndoManagerSupplier.hpp>
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

using namespace css;
using namespace vcl;
using namespace desktop;
using namespace utl;

static LibLibreOffice_Impl *gImpl = nullptr;
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
    const char *extn;
    const char *filterName;
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
            aOutput.append(OUStringToOString(s, RTL_TEXTENCODING_UTF8));
            aOutput.append("\n");
        }
        if (aOutput.getLength() > 0)
        {
            OString aChunk = aOutput.makeStringAndClear();
            if (gImpl && gImpl->mpCallback)
                gImpl->mpCallback(LOK_CALLBACK_PROFILE_FRAME, aChunk.getStr(), gImpl->mpCallbackData);
        }
    }
};

} // unnamed namespace

static TraceEventDumper *traceEventDumper = nullptr;

const ExtensionMap aWriterExtensionMap[] =
{
    { "doc",   "MS Word 97" },
    { "docm",  "MS Word 2007 XML VBA" },
    { "docx",  "MS Word 2007 XML" },
    { "fodt",  "OpenDocument Text Flat XML" },
    { "html",  "HTML (StarWriter)" },
    { "odt",   "writer8" },
    { "ott",   "writer8_template" },
    { "pdf",   "writer_pdf_Export" },
    { "epub",  "EPUB" },
    { "rtf",   "Rich Text Format" },
    { "txt",   "Text" },
    { "xhtml", "XHTML Writer File" },
    { "png",   "writer_png_Export" },
    { "xml",   "writer_indexing_export" },
    { nullptr, nullptr }
};

const ExtensionMap aCalcExtensionMap[] =
{
    { "csv",   "Text - txt - csv (StarCalc)" },
    { "fods",  "OpenDocument Spreadsheet Flat XML" },
    { "html",  "HTML (StarCalc)" },
    { "ods",   "calc8" },
    { "ots",   "calc8_template" },
    { "pdf",   "calc_pdf_Export" },
    { "xhtml", "XHTML Calc File" },
    { "xls",   "MS Excel 97" },
    { "xlsm",  "Calc MS Excel 2007 VBA XML" },
    { "xlsx",  "Calc MS Excel 2007 XML" },
    { "png",   "calc_png_Export" },
    { nullptr, nullptr }
};

const ExtensionMap aImpressExtensionMap[] =
{
    { "fodp",  "OpenDocument Presentation Flat XML" },
    { "html",  "impress_html_Export" },
    { "odg",   "impress8_draw" },
    { "odp",   "impress8" },
    { "otp",   "impress8_template" },
    { "pdf",   "impress_pdf_Export" },
    { "potm",  "Impress MS PowerPoint 2007 XML Template" },
    { "pot",   "MS PowerPoint 97 Vorlage" },
    { "pptm",  "Impress MS PowerPoint 2007 XML VBA" },
    { "pptx",  "Impress MS PowerPoint 2007 XML" },
    { "pps",   "MS PowerPoint 97 Autoplay" },
    { "ppt",   "MS PowerPoint 97" },
    { "svg",   "impress_svg_Export" },
    { "xhtml", "XHTML Impress File" },
    { "png",   "impress_png_Export"},
    { nullptr, nullptr }
};

const ExtensionMap aDrawExtensionMap[] =
{
    { "fodg",  "draw_ODG_FlatXML" },
    { "html",  "draw_html_Export" },
    { "odg",   "draw8" },
    { "pdf",   "draw_pdf_Export" },
    { "svg",   "draw_svg_Export" },
    { "xhtml", "XHTML Draw File" },
    { "png",   "draw_png_Export"},
    { nullptr, nullptr }
};

static OUString getUString(const char* pString)
{
    if (pString == nullptr)
        return OUString();

    OString sString(pString, strlen(pString));
    return OStringToOUString(sString, RTL_TEXTENCODING_UTF8);
}

// Tolerate embedded \0s etc.
static char *convertOString(const OString &rStr)
{
    char* pMemory = static_cast<char*>(malloc(rStr.getLength() + 1));
    assert(pMemory); // don't tolerate failed allocations.
    memcpy(pMemory, rStr.getStr(), rStr.getLength() + 1);
    return pMemory;
}

static char *convertOUString(std::u16string_view aStr)
{
    return convertOString(OUStringToOString(aStr, RTL_TEXTENCODING_UTF8));
}

/// Try to convert a relative URL to an absolute one, unless it already looks like a URL.
static OUString getAbsoluteURL(const char* pURL)
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

static uno::Any jsonToUnoAny(const boost::property_tree::ptree& aTree)
{
    uno::Any aAny;
    uno::Any aValue;
    sal_Int32 nFields;
    uno::Reference< reflection::XIdlField > aField;
    boost::property_tree::ptree aNodeNull, aNodeValue, aNodeField;
    const std::string& rType = aTree.get<std::string>("type", "");
    const std::string& rValue = aTree.get<std::string>("value", "");
    uno::Sequence< uno::Reference< reflection::XIdlField > > aFields;
    uno::Reference< reflection:: XIdlClass > xIdlClass =
        css::reflection::theCoreReflection::get(comphelper::getProcessComponentContext())->forName(OUString::fromUtf8(rType.c_str()));
    if (xIdlClass.is())
    {
        uno::TypeClass aTypeClass = xIdlClass->getTypeClass();
        xIdlClass->createObject(aAny);
        aFields = xIdlClass->getFields();
        nFields = aFields.getLength();
        aNodeValue = aTree.get_child("value", aNodeNull);
        if (nFields > 0 && aNodeValue != aNodeNull)
        {
            for (sal_Int32 itField = 0; itField < nFields; ++itField)
            {
                aField = aFields[itField];
                aNodeField = aNodeValue.get_child(aField->getName().toUtf8().getStr(), aNodeNull);
                if (aNodeField != aNodeNull)
                {
                    aValue = jsonToUnoAny(aNodeField);
                    aField->set(aAny, aValue);
                }
            }
        }
        else if (!rValue.empty())
        {
            if (aTypeClass == uno::TypeClass_VOID)
                aAny.clear();
            else if (aTypeClass == uno::TypeClass_BYTE)
                aAny <<= static_cast<sal_Int8>(OString(rValue.c_str()).toInt32());
            else if (aTypeClass == uno::TypeClass_BOOLEAN)
                aAny <<= OString(rValue.c_str()).toBoolean();
            else if (aTypeClass == uno::TypeClass_SHORT)
                aAny <<= static_cast<sal_Int16>(OString(rValue.c_str()).toInt32());
            else if (aTypeClass == uno::TypeClass_UNSIGNED_SHORT)
                aAny <<= static_cast<sal_uInt16>(OString(rValue.c_str()).toUInt32());
            else if (aTypeClass == uno::TypeClass_LONG)
                aAny <<= OString(rValue.c_str()).toInt32();
            else if (aTypeClass == uno::TypeClass_UNSIGNED_LONG)
                aAny <<= static_cast<sal_uInt32>(OString(rValue.c_str()).toInt32());
            else if (aTypeClass == uno::TypeClass_FLOAT)
                aAny <<= OString(rValue.c_str()).toFloat();
            else if (aTypeClass == uno::TypeClass_DOUBLE)
                aAny <<= OString(rValue.c_str()).toDouble();
            else if (aTypeClass == uno::TypeClass_STRING)
                aAny <<= OUString::fromUtf8(rValue.c_str());
        }
    }
    return aAny;
}

std::vector<beans::PropertyValue> desktop::jsonToPropertyValuesVector(const char* pJSON)
{
    std::vector<beans::PropertyValue> aArguments;
    if (pJSON && pJSON[0] != '\0')
    {
        boost::property_tree::ptree aTree, aNodeNull, aNodeValue;
        std::stringstream aStream(pJSON);
        boost::property_tree::read_json(aStream, aTree);

        for (const auto& rPair : aTree)
        {
            const std::string& rType = rPair.second.get<std::string>("type", "");
            const std::string& rValue = rPair.second.get<std::string>("value", "");

            beans::PropertyValue aValue;
            aValue.Name = OUString::fromUtf8(rPair.first.c_str());
            if (rType == "string")
                aValue.Value <<= OUString::fromUtf8(rValue.c_str());
            else if (rType == "boolean")
                aValue.Value <<= OString(rValue.c_str()).toBoolean();
            else if (rType == "float")
                aValue.Value <<= OString(rValue.c_str()).toFloat();
            else if (rType == "long")
                aValue.Value <<= OString(rValue.c_str()).toInt32();
            else if (rType == "short")
                aValue.Value <<= sal_Int16(OString(rValue.c_str()).toInt32());
            else if (rType == "unsigned short")
                aValue.Value <<= sal_uInt16(OString(rValue.c_str()).toUInt32());
            else if (rType == "int64")
                aValue.Value <<= OString(rValue.c_str()).toInt64();
            else if (rType == "int32")
                aValue.Value <<= OString(rValue.c_str()).toInt32();
            else if (rType == "int16")
                aValue.Value <<= sal_Int16(OString(rValue.c_str()).toInt32());
            else if (rType == "uint64")
                aValue.Value <<= OString(rValue.c_str()).toUInt64();
            else if (rType == "uint32")
                aValue.Value <<= OString(rValue.c_str()).toUInt32();
            else if (rType == "uint16")
                aValue.Value <<= sal_uInt16(OString(rValue.c_str()).toUInt32());
            else if (rType == "[]byte")
            {
                aNodeValue = rPair.second.get_child("value", aNodeNull);
                if (aNodeValue != aNodeNull && aNodeValue.size() == 0)
                {
                    uno::Sequence< sal_Int8 > aSeqByte(reinterpret_cast<const sal_Int8*>(rValue.c_str()), rValue.size());
                    aValue.Value <<= aSeqByte;
                }
            }
            else if (rType == "[]any")
            {
                aNodeValue = rPair.second.get_child("value", aNodeNull);
                if (aNodeValue != aNodeNull && !aNodeValue.empty())
                {
                    uno::Sequence< uno::Any > aSeq(aNodeValue.size());
                    std::transform(aNodeValue.begin(), aNodeValue.end(), aSeq.getArray(),
                                   [](const auto& rSeqPair)
                                   { return jsonToUnoAny(rSeqPair.second); });
                    aValue.Value <<= aSeq;
                }
            }
            else
                SAL_WARN("desktop.lib", "jsonToPropertyValuesVector: unhandled type '"<<rType<<"'");
            aArguments.push_back(aValue);
        }
    }
    return aArguments;
}

static void unoAnyToJson(tools::JsonWriter& rJson, const char * pNodeName, const uno::Any& anyItem)
{
    auto aNode = rJson.startNode(pNodeName);
    OUString aType = anyItem.getValueTypeName();
    rJson.put("type", aType.toUtf8().getStr());

    if (aType == "string")
        rJson.put("value", anyItem.get<OUString>().toUtf8().getStr());
    else if (aType == "unsigned long")
        rJson.put("value", OString::number(anyItem.get<sal_uInt32>()).getStr());
    else if (aType == "long")
        rJson.put("value", OString::number(anyItem.get<sal_Int32>()).getStr());
    else if (aType == "[]any")
    {
        uno::Sequence<uno::Any> aSeq;
        if (anyItem >>= aSeq)
        {
            auto valueNode = rJson.startNode("value");

            for (auto i = 0; i < aSeq.getLength(); ++i)
            {
                unoAnyToJson(rJson, OString::number(i).getStr(), aSeq[i]);
            }
        }
    }
}

static int lcl_getViewId(const std::string& payload);

namespace desktop {

RectangleAndPart RectangleAndPart::Create(const std::string& rPayload)
{
    RectangleAndPart aRet;
    if (rPayload.compare(0, 5, "EMPTY") == 0) // payload starts with "EMPTY"
    {
        aRet.m_aRectangle = tools::Rectangle(0, 0, SfxLokHelper::MaxTwips, SfxLokHelper::MaxTwips);
        if (comphelper::LibreOfficeKit::isPartInInvalidation())
            aRet.m_nPart = std::stol(rPayload.substr(6));

        return aRet;
    }

    // Read '<left>, <top>, <width>, <height>[, <part>]'. C++ streams are simpler but slower.
    const char* pos = rPayload.c_str();
    const char* end = rPayload.c_str() + rPayload.size();
    tools::Long nLeft = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    while( *pos != ',' )
        ++pos;
    ++pos;
    assert(pos < end);
    tools::Long nTop = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    while( *pos != ',' )
        ++pos;
    ++pos;
    assert(pos < end);
    tools::Long nWidth = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    while( *pos != ',' )
        ++pos;
    ++pos;
    assert(pos < end);
    tools::Long nHeight = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    tools::Long nPart = INT_MIN;
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
    {
        while( *pos != ',' )
            ++pos;
        ++pos;
        assert(pos < end);
        nPart = rtl_str_toInt64_WithLength(pos, 10, end - pos);
    }

    if (nWidth > 0 && nHeight > 0)
    {
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
        {
            aRet.m_aRectangle = tools::Rectangle(nLeft, nTop, nLeft + nWidth, nTop + nHeight);
        }
    }
    // else leave empty rect.

    aRet.m_nPart = nPart;
    return aRet;
}

const std::string& CallbackFlushHandler::CallbackData::getPayload() const
{
    if(PayloadString.empty())
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
    PayloadString = boost::trim_copy(aJSONStream.str());

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
            return aExpected == getPayload();
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

static int lcl_getViewId(const std::string& payload)
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
        return strtol(payload.substr(numberPos).c_str(), nullptr, 10);

    return 0;
}

namespace {

std::string extractCertificate(const std::string & certificate)
{
    const std::string header("-----BEGIN CERTIFICATE-----");
    const std::string footer("-----END CERTIFICATE-----");

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
    const std::string header("-----BEGIN PRIVATE KEY-----");
    const std::string footer("-----END PRIVATE KEY-----");

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
        return "";

    SfxObjectShell* pObjectShell = pBaseModel->GetObjectShell();
    if (!pObjectShell)
        return "";

    SfxMedium* pMedium = pObjectShell->GetMedium();
    if (!pMedium)
        return "";

    auto pFilter = pMedium->GetFilter();
    if (!pFilter)
        return "";

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
    SfxViewShell::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_LRSPACE,
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
    SfxViewShell::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_ULSPACE,
                                                          SfxCallMode::RECORD, { pPageULMarginItem });
}

// Main function which toggles page orientation of the Writer doc. Needed by ToggleOrientation
void ExecuteOrientationChange()
{
    std::unique_ptr<SvxPageItem> pPageItem(new SvxPageItem(SID_ATTR_PAGE));

    // 1mm in twips rounded
    // This should be in sync with MINBODY in sw/source/uibase/sidebar/PageMarginControl.hxx
    constexpr tools::Long MINBODY = o3tl::toTwips(1, o3tl::Length::mm);

    css::uno::Reference< css::document::XUndoManager > mxUndoManager(
                getUndoManager( SfxViewFrame::Current()->GetFrame().GetFrameInterface() ) );

    if ( mxUndoManager.is() )
        mxUndoManager->enterUndoContext( "" );


    const SfxPoolItem* pItem;


    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_SIZE, pItem);
    std::unique_ptr<SvxSizeItem> pPageSizeItem(&pItem->Clone()->StaticWhichCast(SID_ATTR_PAGE_SIZE));

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_LRSPACE, pItem);
    std::unique_ptr<SvxLongLRSpaceItem> pPageLRMarginItem(&pItem->Clone()->StaticWhichCast(SID_ATTR_PAGE_LRSPACE));

    SfxViewFrame::Current()->GetBindings().GetDispatcher()->QueryState(SID_ATTR_PAGE_ULSPACE, pItem);
    std::unique_ptr<SvxLongULSpaceItem> pPageULMarginItem(&pItem->Clone()->StaticWhichCast(SID_ATTR_PAGE_ULSPACE));

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
        if (SfxViewShell::Current())
        {
            SfxViewShell::Current()->GetDispatcher()->ExecuteList(SID_ATTR_PAGE_SIZE,
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

void setupSidebar(std::u16string_view sidebarDeckId = u"")
{
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell ? pViewShell->GetViewFrame() : nullptr;
    if (pViewFrame)
    {
        if (!pViewFrame->GetChildWindow(SID_SIDEBAR))
            pViewFrame->SetChildWindow(SID_SIDEBAR, false /* create it */, true /* focus */);

        pViewFrame->ShowChildWindow(SID_SIDEBAR, true);

        // Force synchronous population of panels
        SfxChildWindow *pChild = pViewFrame->GetChildWindow(SID_SIDEBAR);
        if (!pChild)
            return;

        auto pDockingWin = dynamic_cast<sfx2::sidebar::SidebarDockingWindow *>(pChild->GetWindow());
        if (!pDockingWin)
            return;

        OUString currentDeckId = pDockingWin->GetSidebarController()->GetCurrentDeckId();

        // check if it is the chart deck id, if it is, don't switch to default deck
        bool switchToDefault = true;

        if (currentDeckId == "ChartDeck")
            switchToDefault = false;

        if (!sidebarDeckId.empty())
        {
            pDockingWin->GetSidebarController()->SwitchToDeck(sidebarDeckId);
        }
        else
        {
            if (switchToDefault)
                pDockingWin->GetSidebarController()->SwitchToDefaultDeck();
        }

        pDockingWin->SyncUpdate();
    }
    else
        SetLastExceptionMsg("No view shell or sidebar");
}

void hideSidebar()
{
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell? pViewShell->GetViewFrame(): nullptr;
    if (pViewFrame)
        pViewFrame->SetChildWindow(SID_SIDEBAR, false , false );
    else
        SetLastExceptionMsg("No view shell or sidebar");
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
static void doc_paintTile(LibreOfficeKitDocument* pThis,
                          unsigned char* pBuffer,
                          const int nCanvasWidth, const int nCanvasHeight,
                          const int nTilePosX, const int nTilePosY,
                          const int nTileWidth, const int nTileHeight);
#ifdef IOS
static void doc_paintTileToCGContext(LibreOfficeKitDocument* pThis,
                                     void* rCGContext,
                                     const int nCanvasWidth, const int nCanvasHeight,
                                     const int nTilePosX, const int nTilePosY,
                                     const int nTileWidth, const int nTileHeight);
#endif
static void doc_paintPartTile(LibreOfficeKitDocument* pThis,
                              unsigned char* pBuffer,
                              const int nPart,
                              const int nCanvasWidth, const int nCanvasHeight,
                              const int nTilePosX, const int nTilePosY,
                              const int nTileWidth, const int nTileHeight);
static int doc_getTileMode(LibreOfficeKitDocument* pThis);
static void doc_getDocumentSize(LibreOfficeKitDocument* pThis,
                                long* pWidth,
                                long* pHeight);
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

const vcl::Font* FindFont(const OUString& rFontName)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFonts
        = static_cast<const SvxFontListItem*>(pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;
    if (pList && !rFontName.isEmpty())
        if (sal_Handle hMetric = pList->GetFirstFontMetric(rFontName))
            return &FontList::GetFontMetric(hMetric);
    return nullptr;
}

vcl::Font FindFont_FallbackToDefault(const OUString& rFontName)
{
    if (auto pFound = FindFont(rFontName))
        return *pFound;

    return OutputDevice::GetDefaultFont(DefaultFontType::SANS_UNICODE, LANGUAGE_NONE,
                                        GetDefaultFontFlags::NONE);
}
} // anonymous namespace

LibLODocument_Impl::LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent, int nDocumentId)
    : mxComponent(xComponent)
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
        m_pDocumentClass->paintTile = doc_paintTile;
#ifdef IOS
        m_pDocumentClass->paintTileToCGContext = doc_paintTileToCGContext;
#endif
        m_pDocumentClass->paintPartTile = doc_paintPartTile;
        m_pDocumentClass->getTileMode = doc_getTileMode;
        m_pDocumentClass->getDocumentSize = doc_getDocumentSize;
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
        Translate::ExpandVariables("%PRODUCTNAME %PRODUCTVERSION%PRODUCTEXTENSION (%1)"));
    OUString os("$_OS");
    ::rtl::Bootstrap::expandMacros(os);
    return sGenerator.replaceFirst("%1", os);
}

extern "C" {

CallbackFlushHandler::TimeoutIdle::TimeoutIdle( CallbackFlushHandler* handler )
    : Timer( "lokit timer callback" )
    , mHandler( handler )
{
    // A second timer with higher priority, it'll ensure we flush in reasonable time if we get too busy
    // to get POST_PAINT priority processing. Otherwise it could take a long time to flush.
    SetPriority(TaskPriority::DEFAULT);
    SetTimeout( 100 ); // 100 ms
}

void CallbackFlushHandler::TimeoutIdle::Invoke()
{
    mHandler->Invoke();
}

CallbackFlushHandler::CallbackFlushHandler(LibreOfficeKitDocument* pDocument, LibreOfficeKitCallback pCallback, void* pData)
    : Idle( "lokit idle callback" ),
      m_pDocument(pDocument),
      m_pCallback(pCallback),
      m_pData(pData),
      m_nDisableCallbacks(0),
      m_TimeoutIdle( this )
{
    SetPriority(TaskPriority::POST_PAINT);

    // Add the states that are safe to skip duplicates on, even when
    // not consequent (i.e. do no emit them if unchanged from last).
    m_states.emplace(LOK_CALLBACK_TEXT_SELECTION, "NIL");
    m_states.emplace(LOK_CALLBACK_GRAPHIC_SELECTION, "NIL");
    m_states.emplace(LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR, "NIL");
    m_states.emplace(LOK_CALLBACK_STATE_CHANGED, "NIL");
    m_states.emplace(LOK_CALLBACK_MOUSE_POINTER, "NIL");
    m_states.emplace(LOK_CALLBACK_CELL_CURSOR, "NIL");
    m_states.emplace(LOK_CALLBACK_CELL_FORMULA, "NIL");
    m_states.emplace(LOK_CALLBACK_CELL_ADDRESS, "NIL");
    m_states.emplace(LOK_CALLBACK_CURSOR_VISIBLE, "NIL");
    m_states.emplace(LOK_CALLBACK_SET_PART, "NIL");
    m_states.emplace(LOK_CALLBACK_TABLE_SELECTED, "NIL");
}

CallbackFlushHandler::~CallbackFlushHandler()
{
    Stop();
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

void CallbackFlushHandler::libreOfficeKitViewCallback(int nType, const char* pPayload)
{
    CallbackData callbackData(pPayload);
    queue(nType, callbackData);
}

void CallbackFlushHandler::libreOfficeKitViewCallbackWithViewId(int nType, const char* pPayload, int nViewId)
{
    CallbackData callbackData(pPayload, nViewId);
    queue(nType, callbackData);
}

void CallbackFlushHandler::libreOfficeKitViewInvalidateTilesCallback(const tools::Rectangle* pRect, int nPart)
{
    CallbackData callbackData(pRect, nPart);
    queue(LOK_CALLBACK_INVALIDATE_TILES, callbackData);
}

void CallbackFlushHandler::libreOfficeKitViewUpdatedCallback(int nType)
{
    assert(isUpdatedType( nType ));
    std::unique_lock<std::mutex> lock(m_mutex);
    SAL_INFO("lok", "Updated: [" << nType << "]");
    setUpdatedType(nType, true);
}

void CallbackFlushHandler::libreOfficeKitViewUpdatedCallbackPerViewId(int nType, int nViewId, int nSourceViewId)
{
    assert(isUpdatedTypePerViewId( nType ));
    std::unique_lock<std::mutex> lock(m_mutex);
    SAL_INFO("lok", "Updated: [" << nType << "]");
    setUpdatedTypePerViewId(nType, nViewId, nSourceViewId, true);
}

void CallbackFlushHandler::queue(const int type, const char* data)
{
    CallbackData callbackData(data);
    queue(type, callbackData);
}

void CallbackFlushHandler::queue(const int type, CallbackData& aCallbackData)
{
    comphelper::ProfileZone aZone("CallbackFlushHandler::queue");

    SAL_INFO("lok", "Queue: [" << type << "]: [" << aCallbackData.getPayload() << "] on " << m_queue1.size() << " entries.");

    bool bIsChartActive = false;
    if (type == LOK_CALLBACK_GRAPHIC_SELECTION)
    {
        LokChartHelper aChartHelper(SfxViewShell::Current());
        bIsChartActive = aChartHelper.GetWindow() != nullptr;
    }

    if (callbacksDisabled() && !bIsChartActive)
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
        aCallbackData.getPayload().find(", 0, 0, ") != std::string::npos &&
        aCallbackData.getPayload().find("\"hyperlink\":\"\"") == std::string::npos &&
        aCallbackData.getPayload().find("\"hyperlink\": {}") == std::string::npos)
    {
        // The cursor position is often the relative coordinates of the widget
        // issuing it, instead of the absolute one that we expect.
        // This is temporary however, and, once the control is created and initialized
        // correctly, it eventually emits the correct absolute coordinates.
        SAL_INFO("lok", "Skipping invalid event [" << type << "]: [" << aCallbackData.getPayload() << "].");
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

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
            {
                if (removeAll(type))
                    SAL_INFO("lok", "Removed dups of [" << type << "]: [" << aCallbackData.getPayload() << "].");
            }
            break;

            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones,
            // but only if the view is the same.
            case LOK_CALLBACK_CELL_VIEW_CURSOR:
            case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
            case LOK_CALLBACK_CALC_FUNCTION_LIST:
            case LOK_CALLBACK_FORM_FIELD_BUTTON:
            {
                // deleting the duplicate of visible cursor message can cause hyperlink popup not to show up on second/or more click on the same place.
                // If the hyperlink is not empty we can bypass that to show the popup
                const bool hyperLinkException = type == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR &&
                    aCallbackData.getPayload().find("\"hyperlink\":\"\"") == std::string::npos &&
                    aCallbackData.getPayload().find("\"hyperlink\": {}") == std::string::npos;
                if(!hyperLinkException)
                {
                    const int nViewId = aCallbackData.getViewId();
                    removeAll(type, [nViewId] (const CallbackData& elemData) {
                            return (nViewId == elemData.getViewId());
                        }
                    );
                }
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
                const auto pos = aCallbackData.getPayload().find('=');
                if (pos != std::string::npos)
                {
                    const std::string name = aCallbackData.getPayload().substr(0, pos + 1);
                    // This is needed because otherwise it creates some problems when
                    // a save occurs while a cell is still edited in Calc.
                    if (name != ".uno:ModifiedStatus=")
                    {
                        removeAll(type, [&name] (const CallbackData& elemData) {
                                return (elemData.getPayload().compare(0, name.size(), name) == 0);
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
                    { return (elemData.getPayload().find("INPLACE") == std::string::npos); });
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
    if (!IsActive())
    {
        Start();
    }
    if (!m_TimeoutIdle.IsActive())
        m_TimeoutIdle.Start();
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
        if (rcOld.isInfinite() && (rcOld.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart))
        {
            SAL_INFO("lok", "Skipping queue [" << type << "]: [" << aCallbackData.getPayload()
                                               << "] since all tiles need to be invalidated.");
            return true;
        }

        if (rcOld.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart)
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
            return (rcNew.m_nPart == -1 || rcNew.m_nPart == elemData.getRectangleAndPart().m_nPart);
        });
    }
    else
    {
        const auto rcOrig = rcNew;

        SAL_INFO("lok", "Have [" << type << "]: [" << aCallbackData.getPayload() << "] so merging overlapping.");
        removeAll(LOK_CALLBACK_INVALIDATE_TILES,[&rcNew](const CallbackData& elemData) {
            const RectangleAndPart& rcOld = elemData.getRectangleAndPart();
            if (rcNew.m_nPart != -1 && rcOld.m_nPart != -1 && rcOld.m_nPart != rcNew.m_nPart)
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
                if (rcNew.m_aRectangle.Contains(rcOld.m_aRectangle))
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
                if (rcOld.m_aRectangle.Contains(rcNew.m_aRectangle))
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
                const bool bOverlap = !rcOverlap.IsEmpty();
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
    const std::string& payload = aCallbackData.getPayload();

    boost::property_tree::ptree& aTree = aCallbackData.setJson(payload);
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
            gImpl->maLastExceptionMsg = "Document doesn't support dialog rendering, or window not found.";
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
        [this](const SfxViewShell* shell) { return shell->GetViewShellId().get() == m_viewId; } );
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
                    [sourceViewId](const SfxViewShell* shell) { return shell->GetViewShellId().get() == sourceViewId; } );
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
    bool ignore = false;
    OString payload = viewShell->getLOKPayload( type, viewId, &ignore );
    if(ignore)
        return; // No actual payload to send.
    CallbackData callbackData(payload.getStr(), viewId);
    m_queue1.emplace_back(type);
    m_queue2.emplace_back(callbackData);
    SAL_INFO("lok", "Queued updated [" << type << "]: [" << callbackData.getPayload()
        << "] to have " << m_queue1.size() << " entries.");
}

void CallbackFlushHandler::Invoke()
{
    comphelper::ProfileZone aZone("CallbackFlushHandler::Invoke");

    if (!m_pCallback)
        return;

    // Get any pending invalidate tile events. This will call our callbacks,
    // so it must be done before taking the mutex.
    assert(m_viewId >= 0);
    if(SfxViewShell* viewShell = SfxViewShell::GetFirst( false,
        [this](const SfxViewShell* shell) { return shell->GetViewShellId().get() == m_viewId; } ))
    {
        viewShell->flushPendingLOKInvalidateTiles();
    }

    std::scoped_lock<std::mutex> lock(m_mutex);

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

        if (viewId == -1)
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
        else
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

        m_pCallback(type, payload.c_str(), m_pData);
    }

    m_queue1.clear();
    m_queue2.clear();
    Stop();
    m_TimeoutIdle.Stop();
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

    LOKClipboardFactory::releaseClipboardForView(-1);

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

static void lo_runLoop(LibreOfficeKit* pThis,
                       LibreOfficeKitPollCallback pPollCallback,
                       LibreOfficeKitWakeCallback pWakeCallback,
                       void* pData);

static void lo_sendDialogEvent(LibreOfficeKit* pThis,
                               unsigned long long int nLOKWindowId,
                               const char* pArguments);

static void lo_setOption(LibreOfficeKit* pThis, const char* pOption, const char* pValue);

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

        gOfficeClass = m_pOfficeClass;
    }

    pClass = m_pOfficeClass.get();
}

LibLibreOffice_Impl::~LibLibreOffice_Impl()
{
}

namespace
{

#ifdef IOS
void paintTileToCGContext(ITiledRenderable* pDocument,
                          void* rCGContext, const Size nCanvasSize,
                          const int nTilePosX, const int nTilePosY,
                          const int nTileWidth, const int nTileHeight)
{
    SystemGraphicsData aData;
    aData.rCGContext = reinterpret_cast<CGContextRef>(rCGContext);

    ScopedVclPtrInstance<VirtualDevice> pDevice(aData, Size(1, 1), DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    pDevice->SetOutputSizePixel(nCanvasSize);
    pDocument->paintTile(*pDevice, nCanvasSize.Width(), nCanvasSize.Height(),
                    nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

void paintTileIOS(LibreOfficeKitDocument* pThis,
             unsigned char* pBuffer,
             const int nCanvasWidth, const int nCanvasHeight, const double fDPIScale,
             const int nTilePosX, const int nTilePosY,
             const int nTileWidth, const int nTileHeight)
{
    CGContextRef pCGContext = CGBitmapContextCreate(pBuffer, nCanvasWidth, nCanvasHeight, 8,
                                                    nCanvasWidth * 4, CGColorSpaceCreateDeviceRGB(),
                                                    kCGImageAlphaPremultipliedFirst | kCGImageByteOrder32Little);

    CGContextTranslateCTM(pCGContext, 0, nCanvasHeight);
    CGContextScaleCTM(pCGContext, fDPIScale, -fDPIScale);

    doc_paintTileToCGContext(pThis, (void*) pCGContext, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

    CGContextRelease(pCGContext);
}
#endif

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
        rFilterDataMap["ExportBookmarks"] <<= true;
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
        pLib->maLastExceptionMsg = "Filename to load was not provided.";
        SAL_INFO("lok", "URL for load is empty");
        return nullptr;
    }

    pLib->maLastExceptionMsg.clear();

    if (!xContext.is())
    {
        pLib->maLastExceptionMsg = "ComponentContext is not available";
        SAL_INFO("lok", "ComponentContext is not available");
        return nullptr;
    }

    uno::Reference<frame::XDesktop2> xComponentLoader = frame::Desktop::create(xContext);

    if (!xComponentLoader.is())
    {
        pLib->maLastExceptionMsg = "ComponentLoader is not available";
        SAL_INFO("lok", "ComponentLoader is not available");
        return nullptr;
    }

    try
    {
        // 'Language=...' is an option that LOK consumes by itself, and does
        // not pass it as a parameter to the filter
        OUString aOptions = getUString(pOptions);
        const OUString aLanguage = extractParameter(aOptions, u"Language");
        bool isValidLangTag = LanguageTag::isValidBcp47(aLanguage, nullptr);

        if (!aLanguage.isEmpty() && isValidLangTag)
        {
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

        const OUString aDeviceFormFactor = extractParameter(aOptions, u"DeviceFormFactor");
        SfxLokHelper::setDeviceFormFactor(aDeviceFormFactor);

        const OUString aBatch = extractParameter(aOptions, u"Batch");
        if (!aBatch.isEmpty())
        {
             Application::SetDialogCancelMode(DialogCancelMode::LOKSilent);
        }

        const OUString sFilterOptions = aOptions;

        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler("load", pLib));
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

        uno::Sequence<css::beans::PropertyValue> aFilterOptions{
            comphelper::makePropertyValue("FilterOptions", sFilterOptions),
            comphelper::makePropertyValue("InteractionHandler", xInteraction),
            comphelper::makePropertyValue("MacroExecutionMode", nMacroExecMode)
        };

        /* TODO
        sal_Int16 nUpdateDoc = document::UpdateDocMode::ACCORDING_TO_CONFIG;
        aFilterOptions[3].Name = "UpdateDocMode";
        aFilterOptions[3].Value <<= nUpdateDoc;
        */

        const int nThisDocumentId = nDocumentIdCounter++;
        SfxViewShell::SetCurrentDocId(ViewShellDocId(nThisDocumentId));
        uno::Reference<lang::XComponent> xComponent = xComponentLoader->loadComponentFromURL(
                                            aURL, "_blank", 0,
                                            aFilterOptions);

        assert(!xComponent.is() || pair.second); // concurrent loading of same URL ought to fail

        if (!xComponent.is())
        {
            pLib->maLastExceptionMsg = "loadComponentFromURL returned an empty reference";
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
        pLib->maLastExceptionMsg = "Macro to run was not provided.";
        SAL_INFO("lok", "Macro URL is empty");
        return false;
    }

    if (!sURL.startsWith("macro://"))
    {
        pLib->maLastExceptionMsg = "This doesn't look like macro URL";
        SAL_INFO("lok", "Macro URL is invalid");
        return false;
    }

    pLib->maLastExceptionMsg.clear();

    if (!xContext.is())
    {
        pLib->maLastExceptionMsg = "ComponentContext is not available";
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
        pLib->maLastExceptionMsg = "ComponentLoader is not available";
        SAL_INFO("lok", "ComponentLoader is not available");
        return false;
    }

    xFactory = xContext->getServiceManager();

    if (xFactory.is())
    {
        uno::Reference<frame::XDispatchProvider> xDP;
        xSFactory.set(xFactory, uno::UNO_QUERY_THROW);
        xDP.set( xSFactory->createInstance("com.sun.star.comp.sfx2.SfxMacroLoader"), uno::UNO_QUERY );
        uno::Reference<frame::XDispatch> xD = xDP->queryDispatch( aURL, OUString(), 0);

        if (!xD.is())
        {
            pLib->maLastExceptionMsg = "Macro loader is not available";
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

    return false;
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
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String.c_str());
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
        OUString aBase64OUString = OUString::createFromAscii(aPrivateKeyBase64String.c_str());
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
        SetLastExceptionMsg("Filename to save to was not provided.");
        SAL_INFO("lok", "URL for save is empty");
        return false;
    }

    try
    {
        const ExtensionMap* pMap;

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
                SetLastExceptionMsg("input filename without a suffix");
                return false;
            }
        }

        OUString aFilterName;
        for (sal_Int32 i = 0; pMap[i].extn; ++i)
        {
            if (sFormat.equalsIgnoreAsciiCaseAscii(pMap[i].extn))
            {
                aFilterName = getUString(pMap[i].filterName);
                break;
            }
        }
        if (aFilterName.isEmpty())
        {
            SetLastExceptionMsg("no output filter found for provided suffix");
            return false;
        }

        OUString aFilterOptions = getUString(pFilterOptions);

        // Check if watermark for pdf is passed by filteroptions...
        // It is not a real filter option so it must be filtered out.
        OUString watermarkText, sFullSheetPreview;
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

        bool bFullSheetPreview = sFullSheetPreview == "true";

        // Select a pdf version if specified a valid one. If not specified then ignore.
        // If invalid then fail.
        sal_Int32 pdfVer = 0;
        if ((aIndex = aFilterOptions.indexOf(",PDFVer=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("PDFVEREND");
            OUString sPdfVer;
            sPdfVer = aFilterOptions.subView(aIndex+8, bIndex-(aIndex+8));
            aFilterOptions = OUString::Concat(aFilterOptions.subView(0, aIndex)) + aFilterOptions.subView(bIndex+9);

            if (sPdfVer.equalsIgnoreAsciiCase("PDF/A-1b"))
                pdfVer = 1;
            else if (sPdfVer.equalsIgnoreAsciiCase("PDF/A-2b"))
                pdfVer = 2;
            else if (sPdfVer.equalsIgnoreAsciiCase("PDF/A-3b"))
                pdfVer = 3;
            else if (sPdfVer.equalsIgnoreAsciiCase("PDF-1.5"))
                pdfVer = 15;
            else if (sPdfVer.equalsIgnoreAsciiCase("PDF-1.6"))
                pdfVer = 16;
            else
            {
                SetLastExceptionMsg("wrong PDF version");
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
        MediaDescriptor aSaveMediaDescriptor;
        for (const auto& rOption : aOptionSeq)
        {
            if (rOption == "TakeOwnership")
                bTakeOwnership = true;
            else if (rOption == "NoFileSync")
                aSaveMediaDescriptor["NoFileSync"] <<= true;
            else
                aFilteredOptionVec.push_back(rOption);
        }

        aSaveMediaDescriptor["Overwrite"] <<= true;
        aSaveMediaDescriptor["FilterName"] <<= aFilterName;

        auto aFilteredOptionSeq = comphelper::containerToSequence<OUString>(aFilteredOptionVec);
        aFilterOptions = comphelper::string::convertCommaSeparated(aFilteredOptionSeq);
        aSaveMediaDescriptor[MediaDescriptor::PROP_FILTEROPTIONS] <<= aFilterOptions;

        comphelper::SequenceAsHashMap aFilterDataMap;

        setFormatSpecificFilterData(sFormat, aFilterDataMap);

        if (!watermarkText.isEmpty())
            aFilterDataMap["TiledWatermark"] <<= watermarkText;

        if (bFullSheetPreview)
            aFilterDataMap["SinglePageSheets"] <<= true;

        if (pdfVer)
            aFilterDataMap["SelectPdfVersion"] <<= pdfVer;

        if (!aFilterDataMap.empty())
        {
            aSaveMediaDescriptor["FilterData"] <<= aFilterDataMap.getAsConstPropertyValueList();
        }

        // add interaction handler too
        if (gImpl)
        {
            // gImpl does not have to exist when running from a unit test
            rtl::Reference<LOKInteractionHandler> const pInteraction(
                    new LOKInteractionHandler("saveas", gImpl, pDocument));
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

    OUString sUnoCommands[] =
    {
        OUString(".uno:AlignLeft"),
        OUString(".uno:AlignHorizontalCenter"),
        OUString(".uno:AlignRight"),
        OUString(".uno:BackColor"),
        OUString(".uno:BackgroundColor"),
        OUString(".uno:TableCellBackgroundColor"),
        OUString(".uno:Bold"),
        OUString(".uno:CenterPara"),
        OUString(".uno:CharBackColor"),
        OUString(".uno:CharBackgroundExt"),
        OUString(".uno:CharFontName"),
        OUString(".uno:Color"),
        OUString(".uno:ControlCodes"),
        OUString(".uno:DecrementIndent"),
        OUString(".uno:DefaultBullet"),
        OUString(".uno:DefaultNumbering"),
        OUString(".uno:FontColor"),
        OUString(".uno:FontHeight"),
        OUString(".uno:IncrementIndent"),
        OUString(".uno:Italic"),
        OUString(".uno:JustifyPara"),
        OUString(".uno:JumpToMark"),
        OUString(".uno:OutlineFont"),
        OUString(".uno:LeftPara"),
        OUString(".uno:LanguageStatus"),
        OUString(".uno:RightPara"),
        OUString(".uno:Shadowed"),
        OUString(".uno:SubScript"),
        OUString(".uno:SuperScript"),
        OUString(".uno:Strikeout"),
        OUString(".uno:StyleApply"),
        OUString(".uno:Underline"),
        OUString(".uno:ModifiedStatus"),
        OUString(".uno:Undo"),
        OUString(".uno:Redo"),
        OUString(".uno:InsertPage"),
        OUString(".uno:DeletePage"),
        OUString(".uno:DuplicatePage"),
        OUString(".uno:InsertSlide"),
        OUString(".uno:DeleteSlide"),
        OUString(".uno:DuplicateSlide"),
        OUString(".uno:Cut"),
        OUString(".uno:Copy"),
        OUString(".uno:Paste"),
        OUString(".uno:SelectAll"),
        OUString(".uno:ReplyComment"),
        OUString(".uno:ResolveComment"),
        OUString(".uno:ResolveCommentThread"),
        OUString(".uno:InsertRowsBefore"),
        OUString(".uno:InsertRowsAfter"),
        OUString(".uno:InsertColumnsBefore"),
        OUString(".uno:InsertColumnsAfter"),
        OUString(".uno:DeleteRows"),
        OUString(".uno:DeleteColumns"),
        OUString(".uno:DeleteTable"),
        OUString(".uno:SelectTable"),
        OUString(".uno:EntireRow"),
        OUString(".uno:EntireColumn"),
        OUString(".uno:EntireCell"),
        OUString(".uno:AssignLayout"),
        OUString(".uno:StatusDocPos"),
        OUString(".uno:RowColSelCount"),
        OUString(".uno:StatusPageStyle"),
        OUString(".uno:InsertMode"),
        OUString(".uno:SpellOnline"),
        OUString(".uno:StatusSelectionMode"),
        OUString(".uno:StateTableCell"),
        OUString(".uno:StatusBarFunc"),
        OUString(".uno:StatePageNumber"),
        OUString(".uno:StateWordCount"),
        OUString(".uno:SelectionMode"),
        OUString(".uno:PageStatus"),
        OUString(".uno:LayoutStatus"),
        OUString(".uno:Scale"),
        OUString(".uno:Context"),
        OUString(".uno:WrapText"),
        OUString(".uno:ToggleMergeCells"),
        OUString(".uno:NumberFormatCurrency"),
        OUString(".uno:NumberFormatPercent"),
        OUString(".uno:NumberFormatDecimal"),
        OUString(".uno:NumberFormatDate"),
        OUString(".uno:FrameLineColor"),
        OUString(".uno:SortAscending"),
        OUString(".uno:SortDescending"),
        OUString(".uno:TrackChanges"),
        OUString(".uno:ShowTrackedChanges"),
        OUString(".uno:NextTrackedChange"),
        OUString(".uno:PreviousTrackedChange"),
        OUString(".uno:AcceptAllTrackedChanges"),
        OUString(".uno:RejectAllTrackedChanges"),
        OUString(".uno:TableDialog"),
        OUString(".uno:FormatCellDialog"),
        OUString(".uno:FontDialog"),
        OUString(".uno:ParagraphDialog"),
        OUString(".uno:OutlineBullet"),
        OUString(".uno:InsertIndexesEntry"),
        OUString(".uno:DocumentRepair"),
        OUString(".uno:TransformDialog"),
        OUString(".uno:InsertPageHeader"),
        OUString(".uno:InsertPageFooter"),
        OUString(".uno:OnlineAutoFormat"),
        OUString(".uno:InsertObjectChart"),
        OUString(".uno:InsertSection"),
        OUString(".uno:InsertAnnotation"),
        OUString(".uno:DeleteAnnotation"),
        OUString(".uno:InsertPagebreak"),
        OUString(".uno:InsertColumnBreak"),
        OUString(".uno:HyperlinkDialog"),
        OUString(".uno:InsertSymbol"),
        OUString(".uno:EditRegion"),
        OUString(".uno:ThesaurusDialog"),
        OUString(".uno:FormatArea"),
        OUString(".uno:FormatLine"),
        OUString(".uno:FormatColumns"),
        OUString(".uno:Watermark"),
        OUString(".uno:ResetAttributes"),
        OUString(".uno:Orientation"),
        OUString(".uno:ObjectAlignLeft"),
        OUString(".uno:ObjectAlignRight"),
        OUString(".uno:AlignCenter"),
        OUString(".uno:TransformPosX"),
        OUString(".uno:TransformPosY"),
        OUString(".uno:TransformWidth"),
        OUString(".uno:TransformHeight"),
        OUString(".uno:ObjectBackOne"),
        OUString(".uno:SendToBack"),
        OUString(".uno:ObjectForwardOne"),
        OUString(".uno:BringToFront"),
        OUString(".uno:WrapRight"),
        OUString(".uno:WrapThrough"),
        OUString(".uno:WrapLeft"),
        OUString(".uno:WrapIdeal"),
        OUString(".uno:WrapOn"),
        OUString(".uno:WrapOff"),
        OUString(".uno:UpdateCurIndex"),
        OUString(".uno:InsertCaptionDialog"),
        OUString(".uno:FormatGroup"),
        OUString(".uno:SplitTable"),
        OUString(".uno:MergeCells"),
        OUString(".uno:DeleteNote"),
        OUString(".uno:AcceptChanges"),
        OUString(".uno:FormatPaintbrush"),
        OUString(".uno:SetDefault"),
        OUString(".uno:ParaLeftToRight"),
        OUString(".uno:ParaRightToLeft"),
        OUString(".uno:ParaspaceIncrease"),
        OUString(".uno:ParaspaceDecrease"),
        OUString(".uno:AcceptTrackedChange"),
        OUString(".uno:RejectTrackedChange"),
        OUString(".uno:ShowResolvedAnnotations"),
        OUString(".uno:InsertBreak"),
        OUString(".uno:InsertEndnote"),
        OUString(".uno:InsertFootnote"),
        OUString(".uno:InsertReferenceField"),
        OUString(".uno:InsertBookmark"),
        OUString(".uno:InsertAuthoritiesEntry"),
        OUString(".uno:InsertMultiIndex"),
        OUString(".uno:InsertField"),
        OUString(".uno:InsertPageNumberField"),
        OUString(".uno:InsertPageCountField"),
        OUString(".uno:InsertDateField"),
        OUString(".uno:InsertTitleField"),
        OUString(".uno:InsertFieldCtrl"),
        OUString(".uno:CharmapControl"),
        OUString(".uno:EnterGroup"),
        OUString(".uno:LeaveGroup"),
        OUString(".uno:AlignUp"),
        OUString(".uno:AlignMiddle"),
        OUString(".uno:AlignDown"),
        OUString(".uno:TraceChangeMode"),
        OUString(".uno:Combine"),
        OUString(".uno:Merge"),
        OUString(".uno:Dismantle"),
        OUString(".uno:Substract"),
        OUString(".uno:DistributeSelection"),
        OUString(".uno:Intersect"),
        OUString(".uno:BorderInner"),
        OUString(".uno:BorderOuter"),
        OUString(".uno:FreezePanes"),
        OUString(".uno:FreezePanesColumn"),
        OUString(".uno:FreezePanesRow"),
        OUString(".uno:Sidebar"),
        OUString(".uno:SheetRightToLeft"),
        OUString(".uno:RunMacro"),
        OUString(".uno:SpacePara1"),
        OUString(".uno:SpacePara15"),
        OUString(".uno:SpacePara2")
    };

    util::URL aCommandURL;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell? pViewShell->GetViewFrame(): nullptr;

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
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    try
    {
        uno::Reference<lang::XServiceInfo> xDocument(pDocument->mxComponent, uno::UNO_QUERY_THROW);

        if (xDocument->supportsService("com.sun.star.sheet.SpreadsheetDocument"))
        {
            return LOK_DOCTYPE_SPREADSHEET;
        }
        else if (xDocument->supportsService("com.sun.star.presentation.PresentationDocument"))
        {
            return LOK_DOCTYPE_PRESENTATION;
        }
        else if (xDocument->supportsService("com.sun.star.drawing.DrawingDocument"))
        {
            return LOK_DOCTYPE_DRAWING;
        }
        else if (xDocument->supportsService("com.sun.star.text.TextDocument") || xDocument->supportsService("com.sun.star.text.WebDocument"))
        {
            return LOK_DOCTYPE_TEXT;
        }
        else
        {
            SetLastExceptionMsg("unknown document type");
        }
    }
    catch (const uno::Exception& exception)
    {
        SetLastExceptionMsg("exception: " + exception.Message);
    }
    return LOK_DOCTYPE_OTHER;
}

static int doc_getParts (LibreOfficeKitDocument* pThis)
{
    comphelper::ProfileZone aZone("doc_getParts");

    SolarMutexGuard aGuard;

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }

    return convertOUString(pDoc->getPartInfo(nPart));
}

static void doc_selectPart(LibreOfficeKitDocument* pThis, int nPart, int nSelect)
{
    SolarMutexGuard aGuard;
    if (gImpl)
        gImpl->maLastExceptionMsg.clear();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
        return;
    }

    pDoc->selectPart( nPart, nSelect );
}

static void doc_moveSelectedParts(LibreOfficeKitDocument* pThis, int nPosition, bool bDuplicate)
{
    SolarMutexGuard aGuard;
    if (gImpl)
        gImpl->maLastExceptionMsg.clear();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }

    return convertOUString(pDoc->getPartPageRectangles());
}

static char* doc_getPartName(LibreOfficeKitDocument* pThis, int nPart)
{
    comphelper::ProfileZone aZone("doc_getPartName");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return;
    }

#if defined(UNX) && !defined(MACOSX)

    // Painting of zoomed or HiDPI spreadsheets is special, we actually draw everything at 100%,
    // and only set cairo's (or CoreGraphic's, in the iOS case) scale factor accordingly, so that
    // everything is painted bigger or smaller. This is different to what Calc's internal scaling
    // would do - because that one is trying to fit the lines between cells to integer multiples of
    // pixels.
    comphelper::ScopeGuard dpiScaleGuard([]() { comphelper::LibreOfficeKit::setDPIScale(1.0); });

#if defined(IOS)
    double fDPIScaleX = 1.0;
    paintTileIOS(pThis, pBuffer, nCanvasWidth, nCanvasHeight, fDPIScaleX, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
#else
    ScopedVclPtrInstance< VirtualDevice > pDevice(DeviceFormat::DEFAULT);

    // Set background to transparent by default.
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(
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
#endif

#else
    (void) pBuffer;
#endif
}

#ifdef IOS

// This function is separate only to be used by LibreOfficeLight. If that app can be retired, this
// function's code can be inlined.
static void doc_paintTileToCGContext(LibreOfficeKitDocument* pThis,
                                     void* rCGContext,
                                     const int nCanvasWidth, const int nCanvasHeight,
                                     const int nTilePosX, const int nTilePosY,
                                     const int nTileWidth, const int nTileHeight)
{
    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SAL_INFO( "lok.tiledrendering", "paintTileToCGContext: painting [" << nTileWidth << "x" << nTileHeight <<
              "]@(" << nTilePosX << ", " << nTilePosY << ") to [" <<
              nCanvasWidth << "x" << nCanvasHeight << "]px" );

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return;
    }

    Size aCanvasSize(nCanvasWidth, nCanvasHeight);
    paintTileToCGContext(pDoc, rCGContext, aCanvasSize, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
}

#endif

static void doc_paintPartTile(LibreOfficeKitDocument* pThis,
                              unsigned char* pBuffer,
                              const int nPart,
                              const int nCanvasWidth, const int nCanvasHeight,
                              const int nTilePosX, const int nTilePosY,
                              const int nTileWidth, const int nTileHeight)
{
    comphelper::ProfileZone aZone("doc_paintPartTile");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SAL_INFO( "lok.tiledrendering", "paintPartTile: painting @ " << nPart << " ["
               << nTileWidth << "x" << nTileHeight << "]@("
               << nTilePosX << ", " << nTilePosY << ") to ["
               << nCanvasWidth << "x" << nCanvasHeight << "]px" );

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    int nOrigViewId = doc_getView(pThis);

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
        const bool isText = (doc_getDocumentType(pThis) == LOK_DOCTYPE_TEXT);
        int nViewId = nOrigViewId;
        if (!isText)
        {
            // Check if just switching to another view is enough, that has
            // less side-effects.
            if (nPart != doc_getPart(pThis))
            {
                SfxViewShell* pViewShell = SfxViewShell::GetFirst();
                while (pViewShell)
                {
                    if (pViewShell->getPart() == nPart)
                    {
                        nViewId = pViewShell->GetViewShellId().get();
                        doc_setView(pThis, nViewId);
                        break;
                    }
                    pViewShell = SfxViewShell::GetNext(*pViewShell);
                }
            }

            nOrigPart = doc_getPart(pThis);
            if (nPart != nOrigPart)
            {
                doc_setPartImpl(pThis, nPart, false);
            }
        }

        doc_paintTile(pThis, pBuffer, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

        if (!isText && nPart != nOrigPart)
        {
            doc_setPartImpl(pThis, nOrigPart, false);
        }
        if (!isText && nViewId != nOrigViewId)
        {
            doc_setView(pThis, nOrigViewId);
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
    return LOK_TILEMODE_BGRA;
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getPostIts(aJsonWriter);
    return aJsonWriter.extractData();
}

/// Returns the JSON representation of the positions of all the comments in the document
static char* getPostItsPos(LibreOfficeKitDocument* pThis)
{
    SetLastExceptionMsg();
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getPostItsPos(aJsonWriter);
    return aJsonWriter.extractData();
}

static char* getRulerState(LibreOfficeKitDocument* pThis)
{
    SetLastExceptionMsg();
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getRulerState(aJsonWriter);
    return aJsonWriter.extractData();
}

static void doc_postKeyEvent(LibreOfficeKitDocument* pThis, int nType, int nCharCode, int nKeyCode)
{
    comphelper::ProfileZone aZone("doc_postKeyEvent");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
            SetLastExceptionMsg("Document doesn't support tiled rendering");
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
    VclPtr<vcl::Window> pWindow;
    if (nLOKWindowId == 0)
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            gImpl->maLastExceptionMsg = "Document doesn't support tiled rendering";
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
        gImpl->maLastExceptionMsg = "No window found for window id: " + OUString::number(nLOKWindowId);
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
            KeyEvent aEvt(8, 1283);
            for (int i = 0; i < nCharBefore; ++i)
                pWindow->KeyInput(aEvt);
        }
        else
            SfxLokHelper::postKeyEventAsync(pWindow, LOK_KEYEVENT_KEYINPUT, 8, 1283, nCharBefore - 1);
    }

    if (nCharAfter > 0)
    {
        // delete (forward)
        if (nLOKWindowId == 0)
        {
            KeyEvent aEvt(46, 1286);
            for (int i = 0; i < nCharAfter; ++i)
                pWindow->KeyInput(aEvt);
        }
        else
            SfxLokHelper::postKeyEventAsync(pWindow, LOK_KEYEVENT_KEYINPUT, 46, 1286, nCharAfter - 1);
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
        SetLastExceptionMsg("Document doesn't support dialog rendering, or window not found.");
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

        uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);

        SvMemoryStream aOutStream;
        uno::Reference<io::XOutputStream> xOut = new utl::OOutputStreamWrapper(aOutStream);

        utl::MediaDescriptor aMediaDescriptor;
        switch (doc_getDocumentType(pThis))
        {
            case LOK_DOCTYPE_PRESENTATION:
                aMediaDescriptor["FilterName"] <<= OUString("impress_svg_Export");
                break;
            case LOK_DOCTYPE_DRAWING:
                aMediaDescriptor["FilterName"] <<= OUString("draw_svg_Export");
                break;
            case LOK_DOCTYPE_TEXT:
                aMediaDescriptor["FilterName"] <<= OUString("writer_svg_Export");
                break;
            case LOK_DOCTYPE_SPREADSHEET:
                aMediaDescriptor["FilterName"] <<= OUString("calc_svg_Export");
                break;
            default:
                SAL_WARN("lok", "Failed to render shape selection: Document type is not supported");
        }
        aMediaDescriptor["SelectionOnly"] <<= true;
        aMediaDescriptor["OutputStream"] <<= xOut;

        xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());

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
    OString maCommand;                 ///< Command for which this is the result.
    std::shared_ptr<CallbackFlushHandler> mpCallback; ///< Callback to call.

public:
    DispatchResultListener(const char* pCommand, std::shared_ptr<CallbackFlushHandler> const & pCallback)
        : maCommand(pCommand)
        , mpCallback(pCallback)
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
        mpCallback->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.extractData());
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override {}
};

} // anonymous namespace


static void lcl_sendDialogEvent(unsigned long long int nWindowId, const char* pArguments)
{
    SolarMutexGuard aGuard;

    StringMap aMap(jsdialog::jsonToStringMap(pArguments));

    if (aMap.find("id") == aMap.end())
        return;

    sal_uInt64 nCurrentShellId = reinterpret_cast<sal_uInt64>(SfxViewShell::Current());

    try
    {
        OString sControlId = OUStringToOString(aMap["id"], RTL_TEXTENCODING_ASCII_US);

        // dialogs send own id but notebookbar and sidebar controls are remembered by SfxViewShell id
        bool bFoundWeldedControl = jsdialog::ExecuteAction(nWindowId, sControlId, aMap);
        if (!bFoundWeldedControl)
            bFoundWeldedControl = jsdialog::ExecuteAction(nCurrentShellId, sControlId, aMap);

        if (bFoundWeldedControl)
            return;

        // force resend - used in mobile-wizard
        jsdialog::SendFullUpdate(nCurrentShellId, "Panel");

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
}

static void doc_postUnoCommand(LibreOfficeKitDocument* pThis, const char* pCommand, const char* pArguments, bool bNotifyWhenFinished)
{
    comphelper::ProfileZone aZone("doc_postUnoCommand");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    OUString aCommand(pCommand, strlen(pCommand), RTL_TEXTENCODING_UTF8);
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    std::vector<beans::PropertyValue> aPropertyValuesVector(jsonToPropertyValuesVector(pArguments));

    if (!vcl::lok::isUnipoll())
    {
        beans::PropertyValue aSynchronMode;
        aSynchronMode.Name = "SynchronMode";
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
        if (pDocSh->IsModified() && aMimeType == "application/pdf")
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
            pDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.extractData());
            return;
        }


        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler("save", gImpl, pDocument));
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction);

        beans::PropertyValue aValue;
        aValue.Name = "InteractionHandler";
        aValue.Value <<= xInteraction;
        aPropertyValuesVector.push_back(aValue);

        bool bDontSaveIfUnmodified = false;
        aPropertyValuesVector.erase(std::remove_if(aPropertyValuesVector.begin(),
                                                   aPropertyValuesVector.end(),
                                                   [&bDontSaveIfUnmodified](const beans::PropertyValue& aItem){
                                                       if (aItem.Name == "DontSaveIfUnmodified")
                                                       {
                                                           bDontSaveIfUnmodified = aItem.Value.get<bool>();
                                                           return true;
                                                       }
                                                       return false;
                                                   }), aPropertyValuesVector.end());

        // skip saving and tell the result via UNO_COMMAND_RESULT
        if (bDontSaveIfUnmodified && !pDocSh->IsModified())
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
            pDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.extractData());
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
                    value = OutputDevice::LogicToLogic(value, MapUnit::MapTwip, MapUnit::Map100thMM);
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
            aCommandURL.Path = "LOKTransform";
            css::uno::Reference<css::frame::XDispatch>& aChartDispatcher = aChartHelper.GetXDispatcher();
            aChartDispatcher->dispatch(aCommandURL, comphelper::containerToSequence(aPropertyValuesVector));
            return;
        }
    }
    else if (gImpl && aCommand == ".uno:LOKSidebarWriterPage")
    {
        setupSidebar(u"WriterPageDeck");
        return;
    }
    else if (gImpl && aCommand == ".uno:SidebarShow")
    {
        setupSidebar();
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
    else if (bNotifyWhenFinished && pDocument->mpCallbackFlushHandlers.count(nView))
    {
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector),
                new DispatchResultListener(pCommand, pDocument->mpCallbackFlushHandlers[nView]));
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support dialog rendering, or window not found.");
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
        SetLastExceptionMsg("Document doesn't support dialog rendering, or window not found.");
        return;
    }

    OString aType(pType);
    GestureEventType eEventType = GestureEventType::PanningUpdate;

    if (aType == "panBegin")
        eEventType = GestureEventType::PanningBegin;
    else if (aType == "panEnd")
        eEventType = GestureEventType::PanningEnd;

    GestureEvent aEvent {
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support dialog rendering, or window not found.");
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

static bool getFromTransferrable(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aInMimeType, OString &aRet);

static bool encodeImageAsHTML(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aMimeType, OString &aRet)
{
    if (!getFromTransferrable(xTransferable, aMimeType, aRet))
        return false;

    // Encode in base64.
    auto aSeq = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),
                                   aRet.getLength());
    OUStringBuffer aBase64Data;
    comphelper::Base64::encode(aBase64Data, aSeq);

    // Embed in HTML.
    aRet = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n"
        "<html><head>"
        "<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/><meta "
        "name=\"generator\" content=\""
        + getGenerator().toUtf8()
        + "\"/>"
        "</head><body><img src=\"data:" + aMimeType + ";base64,"
        + aBase64Data.makeStringAndClear().toUtf8() + "\"/></body></html>";

    return true;
}

static bool encodeTextAsHTML(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aMimeType, OString &aRet)
{
    if (!getFromTransferrable(xTransferable, aMimeType, aRet))
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

static bool getFromTransferrable(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const OString &aInMimeType, OString &aRet)
{
    OString aMimeType(aInMimeType);

    // Take care of UTF-8 text here.
    bool bConvert = false;
    sal_Int32 nIndex = 0;
    if (aMimeType.getToken(0, ';', nIndex) == "text/plain")
    {
        if (aMimeType.getToken(0, ';', nIndex) == "charset=utf-8")
        {
            aMimeType = "text/plain;charset=utf-16";
            bConvert = true;
        }
    }

    datatransfer::DataFlavor aFlavor;
    aFlavor.MimeType = OUString::fromUtf8(aMimeType.getStr());
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
            if (encodeTextAsHTML(xTransferable, "text/plain;charset=utf-8", aRet))
                return true;
            // If html is not supported, might be a graphic-selection,
            if (encodeImageAsHTML(xTransferable, "image/png", aRet))
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable = pDoc->getSelection();
    if (!xTransferable)
    {
        SetLastExceptionMsg("No selection available");
        return nullptr;
    }

    const char *pType = pMimeType;
    if (!pType || pType[0] == '\0')
        pType = "text/plain;charset=utf-8";

    OString aRet;
    bool bSuccess = getFromTransferrable(xTransferable, OString(pType), aRet);
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return LOK_SELTYPE_NONE;
    }

    css::uno::Reference<css::datatransfer::XTransferable2> xTransferable(pDoc->getSelection(), css::uno::UNO_QUERY);
    if (!xTransferable)
    {
        SetLastExceptionMsg("No selection available");
        return LOK_SELTYPE_NONE;
    }

    if (xTransferable->isComplex())
        return LOK_SELTYPE_COMPLEX;

    OString aRet;
    bool bSuccess = getFromTransferrable(xTransferable, "text/plain;charset=utf-8", aRet);
    if (!bSuccess)
        return LOK_SELTYPE_NONE;

    if (aRet.getLength() > 10000)
        return LOK_SELTYPE_COMPLEX;

    return aRet.getLength() ? LOK_SELTYPE_TEXT : LOK_SELTYPE_NONE;
}

static int doc_getClipboard(LibreOfficeKitDocument* pThis,
                            const char **pMimeTypes,
                            size_t      *pOutCount,
                            char      ***pOutMimeTypes,
                            size_t     **pOutSizes,
                            char      ***pOutStreams)
{
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return 0;
    }

    rtl::Reference<LOKClipboard> xClip(LOKClipboardFactory::getClipboardForCurView());

    css::uno::Reference<css::datatransfer::XTransferable> xTransferable = xClip->getContents();
    SAL_INFO("lok", "Got from clip: " << xClip.get() << " transferrable: " << xTransferable);
    if (!xTransferable)
    {
        SetLastExceptionMsg("No clipboard content available");
        return 0;
    }

    std::vector<OString> aMimeTypes;
    if (!pMimeTypes) // everything
    {
        const uno::Sequence< css::datatransfer::DataFlavor > flavors = xTransferable->getTransferDataFlavors();
        if (!flavors.getLength())
        {
            SetLastExceptionMsg("Flavourless selection");
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
    *pOutMimeTypes = static_cast<char **>(malloc(*pOutCount * sizeof(char *)));
    *pOutStreams = static_cast<char **>(malloc(*pOutCount * sizeof(char *)));
    for (size_t i = 0; i < aMimeTypes.size(); ++i)
    {
        if (aMimeTypes[i] == "text/plain;charset=utf-16")
            (*pOutMimeTypes)[i] = strdup("text/plain;charset=utf-8");
        else
            (*pOutMimeTypes)[i] = strdup(aMimeTypes[i].getStr());

        OString aRet;
        bool bSuccess = getFromTransferrable(xTransferable, (*pOutMimeTypes)[i], aRet);
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return false;
    }

    uno::Reference<datatransfer::XTransferable> xTransferable(new LOKTransferable(nInCount, pInMimeTypes, pInSizes, pInStreams));

    auto xClip = forceSetClipboardForCurrentView(pThis);
    xClip->setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());

    SAL_INFO("lok", "Set clip: " << xClip.get() << " to: " << xTransferable);

    if (!pDoc->isMimeTypeSupported())
    {
        SetLastExceptionMsg("Document doesn't support this mime type");
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
        {"AnchorType", uno::makeAny(static_cast<sal_uInt16>(css::text::TextContentAnchorType_AS_CHARACTER))},
        {"IgnoreComments", uno::makeAny(true)},
    }));
    if (!comphelper::dispatchCommand(".uno:Paste", aPropertyValues))
    {
        SetLastExceptionMsg("Failed to dispatch the .uno: command");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return;
    }

    pDoc->resetSelection();
}

static char* getLanguages(const char* pCommand)
{
    css::uno::Sequence< css::lang::Locale > aLocales;

    if (xContext.is())
    {
        css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv = css::linguistic2::LinguServiceManager::create(xContext);
        if (xLangSrv.is())
        {
            css::uno::Reference<css::linguistic2::XSpellChecker> xSpell = xLangSrv->getSpellChecker();
            if (xSpell.is())
                aLocales = xSpell->getLocales();
        }
    }

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    boost::property_tree::ptree aValues;
    boost::property_tree::ptree aChild;
    OUString sLanguage;
    for ( css::lang::Locale const & locale : std::as_const(aLocales) )
    {
        const LanguageTag aLanguageTag( locale );
        sLanguage = SvtLanguageTable::GetLanguageString(aLanguageTag.getLanguageType());
        if (sLanguage.startsWith("{") && sLanguage.endsWith("}"))
            continue;

        sLanguage += ";" + aLanguageTag.getBcp47(false);
        aChild.put("", sLanguage.toUtf8());
        aValues.push_back(std::make_pair("", aChild));
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

static char* getFonts (const char* pCommand)
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
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
        auto aDevice(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));

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

    static const std::vector<OUString> aWriterStyles =
    {
        "Text body",
        "Quotations",
        "Title",
        "Subtitle",
        "Heading 1",
        "Heading 2",
        "Heading 3"
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
        static const OUStringLiteral sPageStyles(u"PageStyles");
        uno::Reference<beans::XPropertySet> xProperty;
        uno::Reference<container::XNameContainer> xContainer;

        if (xStyleFamilies->hasByName(sPageStyles) && (xStyleFamilies->getByName(sPageStyles) >>= xContainer))
        {
            const uno::Sequence<OUString> aSeqNames = xContainer->getElementNames();
            for (OUString const & sName : aSeqNames)
            {
                bool bIsPhysical;
                xProperty.set(xContainer->getByName(sName), uno::UNO_QUERY);
                if (xProperty.is() && (xProperty->getPropertyValue("IsPhysical") >>= bIsPhysical) && bIsPhysical)
                {
                    OUString displayName;
                    xProperty->getPropertyValue("DisplayName") >>= displayName;
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
    char* pJson = strdup(aString.toUtf8().getStr());
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
            xRedline->getPropertyValue("RedlineAuthor") >>= sAuthor;
            aJson.put("author", sAuthor);

            OUString sType;
            xRedline->getPropertyValue("RedlineType") >>= sType;
            aJson.put("type", sType);

            OUString sComment;
            xRedline->getPropertyValue("RedlineComment") >>= sComment;
            aJson.put("comment", sComment);

            OUString sDescription;
            xRedline->getPropertyValue("RedlineDescription") >>= sDescription;
            aJson.put("description", sDescription);

            util::DateTime aDateTime;
            xRedline->getPropertyValue("RedlineDateTime") >>= aDateTime;
            OUString sDateTime = utl::toISO8601(aDateTime);
            aJson.put("dateTime", sDateTime);
        }
    }
    else
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg("Document doesn't support tiled rendering");
            return nullptr;
        }
        pDoc->getTrackedChanges(aJson);
    }

    return aJson.extractData();
}


/// Returns the JSON representation of the redline author table.
static char* getTrackedChangeAuthors(LibreOfficeKitDocument* pThis)
{
    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return nullptr;
    }
    tools::JsonWriter aJsonWriter;
    pDoc->getTrackedChangeAuthors(aJsonWriter);
    return aJsonWriter.extractData();
}

static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand)
{
    comphelper::ProfileZone aZone("doc_getCommandValues");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    OString aCommand(pCommand);
    static constexpr OStringLiteral aViewRowColumnHeaders(".uno:ViewRowColumnHeaders");
    static constexpr OStringLiteral aSheetGeometryData(".uno:SheetGeometryData");
    static constexpr OStringLiteral aCellCursor(".uno:CellCursor");
    static constexpr OStringLiteral aFontSubset(".uno:FontSubset&name=");

    if (!strcmp(pCommand, ".uno:LanguageStatus"))
    {
        return getLanguages(pCommand);
    }
    else if (!strcmp(pCommand, ".uno:CharFontName"))
    {
        return getFonts(pCommand);
    }
    else if (!strcmp(pCommand, ".uno:StyleApply"))
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
    else if (aCommand.startsWith(aViewRowColumnHeaders))
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg("Document doesn't support tiled rendering");
            return nullptr;
        }

        tools::Rectangle aRectangle;
        if (aCommand.getLength() > aViewRowColumnHeaders.getLength())
        {
            // Command has parameters.
            int nX = 0;
            int nY = 0;
            int nWidth = 0;
            int nHeight = 0;
            OString aArguments = aCommand.copy(aViewRowColumnHeaders.getLength() + 1);
            sal_Int32 nParamIndex = 0;
            do
            {
                OString aParamToken = aArguments.getToken(0, '&', nParamIndex);
                sal_Int32 nIndex = 0;
                OString aKey;
                OString aValue;
                do
                {
                    OString aToken = aParamToken.getToken(0, '=', nIndex);
                    if (!aKey.getLength())
                        aKey = aToken;
                    else
                        aValue = aToken;
                }
                while (nIndex >= 0);
                if (aKey == "x")
                    nX = aValue.toInt32();
                else if (aKey == "y")
                    nY = aValue.toInt32();
                else if (aKey == "width")
                    nWidth = aValue.toInt32();
                else if (aKey == "height")
                    nHeight = aValue.toInt32();
            }
            while (nParamIndex >= 0);

            aRectangle = tools::Rectangle(nX, nY, nX + nWidth, nY + nHeight);
        }

        tools::JsonWriter aJsonWriter;
        pDoc->getRowColumnHeaders(aRectangle, aJsonWriter);
        return aJsonWriter.extractData();
    }
    else if (aCommand.startsWith(aSheetGeometryData))
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg("Document doesn't support tiled rendering");
            return nullptr;
        }

        bool bColumns = true;
        bool bRows = true;
        bool bSizes = true;
        bool bHidden = true;
        bool bFiltered = true;
        bool bGroups = true;
        if (aCommand.getLength() > aSheetGeometryData.getLength())
        {
            bColumns = bRows = bSizes = bHidden = bFiltered = bGroups = false;

            OString aArguments = aCommand.copy(aSheetGeometryData.getLength() + 1);
            sal_Int32 nParamIndex = 0;
            do
            {
                OString aParamToken = aArguments.getToken(0, '&', nParamIndex);
                sal_Int32 nIndex = 0;
                OString aKey;
                OString aValue;
                do
                {
                    OString aToken = aParamToken.getToken(0, '=', nIndex);
                    if (!aKey.getLength())
                        aKey = aToken;
                    else
                        aValue = aToken;

                } while (nIndex >= 0);

                bool bEnableFlag = aValue.isEmpty() ||
                    aValue.equalsIgnoreAsciiCase("true") || aValue.toInt32() > 0;
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
    else if (aCommand.startsWith(aCellCursor))
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg("Document doesn't support tiled rendering");
            return nullptr;
        }
        // Ignore command's deprecated parameters.
        tools::JsonWriter aJsonWriter;
        pDoc->getCellCursor(aJsonWriter);
        return aJsonWriter.extractData();
    }
    else if (aCommand.startsWith(aFontSubset))
    {
        return getFontSubset(std::string_view(pCommand + aFontSubset.getLength()));
    }
    else
    {
        SetLastExceptionMsg("Unknown command, no values returned");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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

    LOKClipboardFactory::releaseClipboardForView(nId);

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

    auto aDevice(VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT));
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
    aDevice->SetOutputSizePixelScaleOffsetAndBuffer(
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
        SetLastExceptionMsg("Document doesn't support dialog rendering, or window not found.");
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

    CGContextRef cgc = CGBitmapContextCreate(pBuffer, nWidth, nHeight, 8, nWidth*4, CGColorSpaceCreateDeviceRGB(), kCGImageAlphaNoneSkipFirst | kCGImageByteOrder32Little);

    CGContextTranslateCTM(cgc, 0, nHeight);
    CGContextScaleCTM(cgc, fDPIScale, -fDPIScale);

    SystemGraphicsData aData;
    aData.rCGContext = cgc;

    ScopedVclPtrInstance<VirtualDevice> pDevice(aData, Size(1, 1), DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixel(Size(nWidth, nHeight));

    MapMode aMapMode(pDevice->GetMapMode());
    aMapMode.SetOrigin(Point(-(nX / fDPIScale), -(nY / fDPIScale)));
    pDevice->SetMapMode(aMapMode);

    pWindow->PaintToDevice(pDevice.get(), Point(0, 0));

    CGContextRelease(cgc);

#else

    ScopedVclPtrInstance<VirtualDevice> pDevice(DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nWidth, nHeight), Fraction(1.0), Point(), pBuffer);

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
        SetLastExceptionMsg("Document doesn't support dialog rendering, or window not found.");
        return;
    }

    if (nAction == LOK_WINDOW_CLOSE)
    {
        vcl::CloseTopLevel(pWindow);
    }
    else if (nAction == LOK_WINDOW_PASTE)
    {
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
            SetLastExceptionMsg("Window command 'paste': wrong parameters.");
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
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String.c_str());
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
        OUString aBase64OUString = OUString::createFromAscii(aPrivateKeyBase64String.c_str());
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
        OUString aBase64OUString = OUString::createFromAscii(aCertificateBase64String.c_str());
        comphelper::Base64::decode(aCertificateSequence, aBase64OUString);
    }
    else
    {
        aCertificateSequence.realloc(nCertificateBinarySize);
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.getArray());
    }

    uno::Reference<security::XCertificate> xCertificate = xCertificateCreator->addDERCertificateToTheDatabase(aCertificateSequence, "TCu,Cu,Tu");

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
    if (gImpl)
        gImpl->maLastExceptionMsg.clear();

    VclPtr<Window> pWindow = vcl::Window::FindLOKWindow(nLOKWindowId);
    if (!pWindow)
    {
        gImpl->maLastExceptionMsg = "Document doesn't support dialog resizing, or window not found.";
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering!");
        return;
    }

    // Sanity check
    if (aMap.find("type") == aMap.end() || aMap.find("cmd") == aMap.end())
    {
        SetLastExceptionMsg("Wrong arguments for sendFormFieldEvent!");
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
        SetLastExceptionMsg("Document doesn't support tiled rendering");
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
        pImpl->maLastExceptionMsg = "Service factory is not available";
        return nullptr;
    }

    uno::Reference<container::XNameAccess> xTypeDetection(xSFactory->createInstance("com.sun.star.document.TypeDetection"), uno::UNO_QUERY);
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
                auto typeNode = aJson.startNode(rType.toUtf8().getStr());
                aJson.put("MediaType", aValue.toUtf8());
            }
        }
    }

    return aJson.extractData();
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
        "{ "
        "\"ProductName\": \"%PRODUCTNAME\", "
        "\"ProductVersion\": \"%PRODUCTVERSION\", "
        "\"ProductExtension\": \"%PRODUCTEXTENSION\", "
        "\"BuildId\": \"%BUILDID\" "
        "}"));
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
        SetLastExceptionMsg("XComponentContext could not be created");
        SAL_INFO("lok", "XComponentContext could not be created");
        return false;
    }

    xFactory = xContext->getServiceManager();
    if (!xFactory.is())
    {
        SetLastExceptionMsg("XMultiComponentFactory could not be created");
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
#if defined(IOS) || defined(ANDROID)
    Application::GetSolarMutex().acquire();
#endif

    {
        SolarMutexGuard aGuard;

        vcl::lok::registerPollCallbacks(pPollCallback, pWakeCallback, pData);
        Application::UpdateMainThread();
        soffice_main();
    }
#if defined(IOS) || defined(ANDROID)
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

/// Used only by LibreOfficeKit when used by Online to pre-initialize
static void preloadData()
{
    comphelper::ProfileZone aZone("preload data");

    // Create user profile in the temp directory for loading the dictionaries
    OUString sUserPath;
    rtl::Bootstrap::get("UserInstallation", sUserPath);
    utl::TempFile aTempDir(nullptr, true);
    aTempDir.EnableKillingFile();
    rtl::Bootstrap::set("UserInstallation", aTempDir.GetURL());

    // Register the bundled extensions
    desktop::Desktop::SynchronizeExtensionRepositories(true);
    bool bAbort = desktop::Desktop::CheckExtensionDependencies();
    if(bAbort)
        std::cerr << "CheckExtensionDependencies failed" << std::endl;

    // preload all available dictionaries
    css::uno::Reference<css::linguistic2::XLinguServiceManager> xLngSvcMgr =
        css::linguistic2::LinguServiceManager::create(comphelper::getProcessComponentContext());
    css::uno::Reference<linguistic2::XSpellChecker> xSpellChecker(xLngSvcMgr->getSpellChecker());

    std::cerr << "Preloading dictionaries: ";
    css::uno::Reference<linguistic2::XSupportedLocales> xSpellLocales(xSpellChecker, css::uno::UNO_QUERY_THROW);
    uno::Sequence< css::lang::Locale > aLocales = xSpellLocales->getLocales();
    for (auto &it : std::as_const(aLocales))
    {
        std::cerr << LanguageTag::convertToBcp47(it) << " ";
        css::beans::PropertyValues aNone;
        xSpellChecker->isValid("forcefed", it, aNone);
    }
    std::cerr << "\n";

    // Hack to load and cache the module liblocaledata_others.so which is not loaded normally
    // (when loading dictionaries of just non-Asian locales). Creating a XCalendar4 of one Asian locale
    // will cheaply load this missing "others" locale library. Appending an Asian locale in
    // LOK_ALLOWLIST_LANGUAGES env-var also works but at the cost of loading that dictionary.
    css::uno::Reference< css::i18n::XCalendar4 > xCal = css::i18n::LocaleCalendar2::create(comphelper::getProcessComponentContext());
    css::lang::Locale aAsianLocale = {"hi", "IN", ""};
    xCal->loadDefaultCalendar(aAsianLocale);

    // preload all available thesauri
    css::uno::Reference<linguistic2::XThesaurus> xThesaurus(xLngSvcMgr->getThesaurus());
    css::uno::Reference<linguistic2::XSupportedLocales> xThesLocales(xSpellChecker, css::uno::UNO_QUERY_THROW);
    aLocales = xThesLocales->getLocales();
    std::cerr << "Preloading thesauri: ";
    for (auto &it : std::as_const(aLocales))
    {
        std::cerr << LanguageTag::convertToBcp47(it) << " ";
        css::beans::PropertyValues aNone;
        xThesaurus->queryMeanings("forcefed", it, aNone);
    }
    std::cerr << "\n";

    css::uno::Reference< css::ui::XAcceleratorConfiguration > xGlobalCfg = css::ui::GlobalAcceleratorConfiguration::create(
        comphelper::getProcessComponentContext());
    xGlobalCfg->getAllKeyEvents();

    std::cerr << "Preload icons\n";
    ImageTree &images = ImageTree::get();
    images.getImageUrl("forcefed.png", "style", "FO_oo");

    std::cerr << "Preload languages\n";

    // force load language singleton
    SvtLanguageTable::HasLanguageType(LANGUAGE_SYSTEM);
    (void)LanguageTag::isValidBcp47("foo", nullptr);

    std::cerr << "Preload fonts\n";

    // Initialize fonts.
    css::uno::Reference<css::linguistic2::XLinguServiceManager2> xLangSrv = css::linguistic2::LinguServiceManager::create(xContext);
    if (xLangSrv.is())
    {
        css::uno::Reference<css::linguistic2::XSpellChecker> xSpell = xLangSrv->getSpellChecker();
        if (xSpell.is())
            aLocales = xSpell->getLocales();
    }

    for (const auto& aLocale : std::as_const(aLocales))
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

    // Set user profile's path back to the original one
    rtl::Bootstrap::set("UserInstallation", sUserPath);
}

namespace {

static void activateNotebookbar(std::u16string_view rApp)
{
    OUString aPath = OUString::Concat("org.openoffice.Office.UI.ToolbarMode/Applications/") + rApp;

    const utl::OConfigurationTreeRoot aAppNode(xContext, aPath, true);

    if (aAppNode.isValid())
    {
        aAppNode.setNodeValue("Active", makeAny(OUString("notebookbar_online.ui")));
        aAppNode.commit();
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

    // What stage are we at ?
    if (pThis == nullptr)
        eStage = PRE_INIT;
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
        rtl_alloc_preInit(true);

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
        rtl::Bootstrap::set("UserInstallation", url);
        if (eStage == SECOND_INIT)
            utl::Bootstrap::reloadData();
    }

    OUString aAppPath;
    if (pAppPath)
    {
        aAppPath = OUString(pAppPath, strlen(pAppPath), RTL_TEXTENCODING_UTF8);
    }
    else
    {
#ifdef ANDROID
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
            rtl::Bootstrap::set("SAL_USE_VCLPLUGIN", "svp");

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
                    throw css::uno::DeploymentException("preInit: XComponentContext is not created");

                css::uno::Reference< css::uno::XInterface > xService;
                xContext->getValueByName("/singletons/com.sun.star.lang.theServiceManager") >>= xService;
                if (!xService.is())
                    throw css::uno::DeploymentException("preInit: XMultiComponentFactory is not created");

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
                    aService->initialize({css::uno::makeAny<OUString>("preload")});
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

            setLanguageAndLocale("en-US");
        }

        if (eStage != PRE_INIT)
        {
            SAL_INFO("lok", "Re-initialize temp paths");
            SvtPathOptions aOptions;
            OUString aNewTemp;
            osl::FileBase::getTempDirURL(aNewTemp);
            aOptions.SetTempPath(aNewTemp);
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

// Turn off quick editing on IOS and ANDROID
#if defined IOS || defined ANDROID
    if (officecfg::Office::Impress::Misc::TextObject::QuickEditing::get())
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
        officecfg::Office::Impress::Misc::TextObject::QuickEditing::set(false, batch);
        batch->commit();
    }
#endif

    setCertificateDir();

    if (bNotebookbar)
    {
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
    if (!gImpl)
    {
        SAL_INFO("lok", "Create libreoffice object");

        gImpl = new LibLibreOffice_Impl();
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

#ifdef IOS

// Used by the unmaintained LibreOfficeLight app. Once that has been retired, get rid of this, too.

__attribute__((visibility("default")))
void temporaryHackToInvokeCallbackHandlers(LibreOfficeKitDocument* pThis)
{
    SolarMutexGuard aGuard;
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    int nOrigViewId = doc_getView(pThis);

    if (nOrigViewId >= 0 && pDocument->mpCallbackFlushHandlers[nOrigViewId])
    {
        pDocument->mpCallbackFlushHandlers[nOrigViewId]->Invoke();
    }
}

#endif

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
