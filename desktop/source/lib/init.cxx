/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_folders.h>

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
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

#include <LibreOfficeKit/LibreOfficeKit.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sal/log.hxx>
#include <vcl/errinf.hxx>
#include <vcl/lok.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <rtl/bootstrap.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/uri.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/profilezone.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/threadpool.hxx>
#include <comphelper/base64.hxx>

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
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>

#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSEInitializer.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/xml/crypto/XCertificateCreator.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <editeng/fontitem.hxx>
#include <editeng/flstitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/lokcharthelper.hxx>
#include <sfx2/DocumentSigner.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/strings.hrc>
#include <svx/ruler.hxx>
#include <svx/svdview.hxx>
#include <svx/svxids.hrc>
#include <svx/ucsubset.hxx>
#include <vcl/vclevent.hxx>
#include <vcl/GestureEvent.hxx>
#include <vcl/svapp.hxx>
#include <unotools/resmgr.hxx>
#include <tools/fract.hxx>
#include <svtools/ctrltool.hxx>
#include <svtools/langtab.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/fontcharmap.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/ptrstyle.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/virdev.hxx>
#include <vcl/ImageTree.hxx>
#include <vcl/ITiledRenderable.hxx>
#include <vcl/IDialogRenderable.hxx>
#include <vcl/dialog.hxx>
#include <unicode/uchar.h>
#include <unotools/configmgr.hxx>
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
#include <vcl/builder.hxx>
#include <vcl/abstdlg.hxx>
#include <tools/diagnose_ex.h>

#include <app.hxx>

#include "../app/cmdlineargs.hxx"
// We also need to hackily be able to start the main libreoffice thread:
#include "../app/sofficemain.h"
#include "../app/officeipcthread.hxx"
#include <lib/init.hxx>

#include "lokinteractionhandler.hxx"
#include "lokclipboard.hxx"
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
    if (gImpl)
        gImpl->maLastExceptionMsg = s;
}

struct ExtensionMap
{
    const char *extn;
    const char *filterName;
};

static const ExtensionMap aWriterExtensionMap[] =
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
    { nullptr, nullptr }
};

static const ExtensionMap aCalcExtensionMap[] =
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

static const ExtensionMap aImpressExtensionMap[] =
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
    { "swf",   "impress_flash_Export" },
    { "xhtml", "XHTML Impress File" },
    { "png",   "impress_png_Export"},
    { nullptr, nullptr }
};

static const ExtensionMap aDrawExtensionMap[] =
{
    { "fodg",  "draw_ODG_FlatXML" },
    { "html",  "draw_html_Export" },
    { "odg",   "draw8" },
    { "pdf",   "draw_pdf_Export" },
    { "svg",   "draw_svg_Export" },
    { "swf",   "draw_flash_Export" },
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
    uno::TypeClass aTypeClass;
    uno::Reference< reflection::XIdlField > aField;
    boost::property_tree::ptree aNodeNull, aNodeValue, aNodeField;
    const std::string& rType = aTree.get<std::string>("type", "");
    const std::string& rValue = aTree.get<std::string>("value", "");
    uno::Sequence< uno::Reference< reflection::XIdlField > > aFields;
    uno::Reference< reflection:: XIdlClass > xIdlClass =
        css::reflection::theCoreReflection::get(comphelper::getProcessComponentContext())->forName(OUString::fromUtf8(rType.c_str()));
    if (xIdlClass.is())
    {
        aTypeClass = xIdlClass->getTypeClass();
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
                aValue.Value <<= static_cast<sal_Int16>(OString(rValue.c_str()).toInt32());
            else if (rType == "unsigned short")
                aValue.Value <<= static_cast<sal_uInt16>(OString(rValue.c_str()).toUInt32());
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
                    sal_Int32 itSeq = 0;
                    uno::Sequence< uno::Any > aSeq(aNodeValue.size());
                    for (const auto& rSeqPair : aNodeValue)
                        aSeq[itSeq++] = jsonToUnoAny(rSeqPair.second);
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

static boost::property_tree::ptree unoAnyToPropertyTree(const uno::Any& anyItem)
{
    boost::property_tree::ptree aTree;
    OUString aType = anyItem.getValueTypeName();
    aTree.put("type", aType.toUtf8().getStr());

    if (aType == "string")
        aTree.put("value", anyItem.get<OUString>().toUtf8().getStr());
    else if (aType == "unsigned long")
        aTree.put("value", OString::number(anyItem.get<sal_uInt32>()).getStr());
    else if (aType == "long")
        aTree.put("value", OString::number(anyItem.get<sal_Int32>()).getStr());
    else if (aType == "[]any")
    {
        uno::Sequence<uno::Any> aSeq;
        if (anyItem >>= aSeq)
        {
            boost::property_tree::ptree aSubTree;

            for (auto i = 0; i < aSeq.getLength(); ++i)
            {
                aSubTree.add_child(OString::number(i).getStr(), unoAnyToPropertyTree(aSeq[i]));
            }
            aTree.add_child("value", aSubTree);
        }
    }

    // TODO: Add more as required

    return aTree;
}

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

    std::istringstream aStream(rPayload);
    long nLeft, nTop, nWidth, nHeight;
    long nPart = INT_MIN;
    char nComma;
    if (comphelper::LibreOfficeKit::isPartInInvalidation())
    {
        aStream >> nLeft >> nComma >> nTop >> nComma >> nWidth >> nComma >> nHeight >> nComma >> nPart;
    }
    else
    {
        aStream >> nLeft >> nComma >> nTop >> nComma >> nWidth >> nComma >> nHeight;
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

RectangleAndPart& CallbackFlushHandler::CallbackData::setRectangleAndPart(const std::string& payload)
{
    setRectangleAndPart(RectangleAndPart::Create(payload));

    // Return reference to the cached object.
    return boost::get<RectangleAndPart>(PayloadObject);
}

void CallbackFlushHandler::CallbackData::setRectangleAndPart(const RectangleAndPart& rRectAndPart)
{
    PayloadString = rRectAndPart.toString().getStr();
    PayloadObject = rRectAndPart;
}

const RectangleAndPart& CallbackFlushHandler::CallbackData::getRectangleAndPart() const
{
    assert(PayloadObject.which() == 1);
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

bool CallbackFlushHandler::CallbackData::validate() const
{
    switch (PayloadObject.which())
    {
        // Not cached.
        case 0:
            return true;

        // RectangleAndPart.
        case 1:
            return getRectangleAndPart().toString().getStr() == PayloadString;

        // Json.
        case 2:
        {
            std::stringstream aJSONStream;
            boost::property_tree::write_json(aJSONStream, getJson(), false);
            const std::string aExpected = boost::trim_copy(aJSONStream.str());
            return aExpected == PayloadString;
        }

        default:
            assert(!"Unknown variant type; please add an entry to validate.");
    }

    return false;
}

}

namespace {

bool lcl_isViewCallbackType(const int type)
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

int lcl_getViewId(const std::string& payload)
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

int lcl_getViewId(const desktop::CallbackFlushHandler::CallbackData& rCallbackData)
{
    if (rCallbackData.isCached())
        return rCallbackData.getJson().get<int>("viewId");
    return lcl_getViewId(rCallbackData.PayloadString);
}

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

}  // end anonymous namespace

// Could be anonymous in principle, but for the unit testing purposes, we
// declare it in init.hxx.
OUString desktop::extractParameter(OUString& rOptions, const OUString& rName)
{
    OUString aValue;

    OUString aNameEquals(rName + "=");
    OUString aCommaNameEquals("," + rName + "=");

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
            rOptions = rOptions.copy(0, nIndex) + rOptions.copy(nComma);
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
static void doc_postWindowExtTextInputEvent(LibreOfficeKitDocument* pThis,
                                            unsigned nWindowId,
                                            int nType,
                                            const char* pText);
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
static void doc_setTextSelection (LibreOfficeKitDocument* pThis,
                                  int nType,
                                  int nX,
                                  int nY);
static char* doc_getTextSelection(LibreOfficeKitDocument* pThis,
                                  const char* pMimeType,
                                  char** pUsedMimeType);
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

static void doc_postWindow(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId, int nAction, const char* pData);

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

LibLODocument_Impl::LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent)
    : mxComponent(xComponent)
{
    if (!(m_pDocumentClass = gDocumentClass.lock()))
    {
        m_pDocumentClass.reset(new LibreOfficeKitDocumentClass);

        m_pDocumentClass->nSize = sizeof(LibreOfficeKitDocumentClass);

        m_pDocumentClass->destroy = doc_destroy;
        m_pDocumentClass->saveAs = doc_saveAs;
        m_pDocumentClass->getDocumentType = doc_getDocumentType;
        m_pDocumentClass->getParts = doc_getParts;
        m_pDocumentClass->getPartPageRectangles = doc_getPartPageRectangles;
        m_pDocumentClass->getPart = doc_getPart;
        m_pDocumentClass->setPart = doc_setPart;
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
        m_pDocumentClass->postWindowKeyEvent = doc_postWindowKeyEvent;
        m_pDocumentClass->postMouseEvent = doc_postMouseEvent;
        m_pDocumentClass->postWindowMouseEvent = doc_postWindowMouseEvent;
        m_pDocumentClass->postUnoCommand = doc_postUnoCommand;
        m_pDocumentClass->setTextSelection = doc_setTextSelection;
        m_pDocumentClass->getTextSelection = doc_getTextSelection;
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
        m_pDocumentClass->getPartHash = doc_getPartHash;

        m_pDocumentClass->paintWindow = doc_paintWindow;
        m_pDocumentClass->paintWindowDPI = doc_paintWindowDPI;
        m_pDocumentClass->postWindow = doc_postWindow;

        m_pDocumentClass->setViewLanguage = doc_setViewLanguage;

        m_pDocumentClass->getPartInfo = doc_getPartInfo;

        m_pDocumentClass->insertCertificate = doc_insertCertificate;
        m_pDocumentClass->addCertificate = doc_addCertificate;
        m_pDocumentClass->getSignatureState = doc_getSignatureState;

        m_pDocumentClass->renderShapeSelection = doc_renderShapeSelection;
        m_pDocumentClass->postWindowGestureEvent = doc_postWindowGestureEvent;

        m_pDocumentClass->createViewWithOptions = doc_createViewWithOptions;

        gDocumentClass = m_pDocumentClass;
    }
    pClass = m_pDocumentClass.get();
}

LibLODocument_Impl::~LibLODocument_Impl()
{
    try
    {
        mxComponent->dispose();
    }
    catch (const css::lang::DisposedException& rException)
    {
        SAL_WARN("lok", "failed to dispose document:" << rException.Message);
    }
}

CallbackFlushHandler::CallbackFlushHandler(LibreOfficeKitDocument* pDocument, LibreOfficeKitCallback pCallback, void* pData)
    : Idle( "lokit timer callback" ),
      m_pDocument(pDocument),
      m_pCallback(pCallback),
      m_pData(pData),
      m_bPartTilePainting(false),
      m_bEventLatch(false)
{
    SetPriority(TaskPriority::POST_PAINT);

    // Add the states that are safe to skip duplicates on,
    // even when not consequent.
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

    Start();
}

CallbackFlushHandler::~CallbackFlushHandler()
{
    Stop();
}

void CallbackFlushHandler::callback(const int type, const char* payload, void* data)
{
    CallbackFlushHandler* self = static_cast<CallbackFlushHandler*>(data);
    if (self)
    {
        self->queue(type, payload);
    }
}

void CallbackFlushHandler::queue(const int type, const char* data)
{
    comphelper::ProfileZone aZone("CallbackFlushHander::queue");

    CallbackData aCallbackData(type, (data ? data : "(nil)"));
    const std::string& payload = aCallbackData.PayloadString;
    SAL_INFO("lok", "Queue: " << type << " : " << payload);

#ifdef DBG_UTIL
    {
        // Dump the queue state and validate cached data.
        int i = 1;
        std::ostringstream oss;
        oss << '\n';
        for (const CallbackData& c : m_queue)
            oss << i++ << ": [" << c.Type << "] [" << c.PayloadString << "].\n";
        const std::string aQueued = oss.str();
        SAL_INFO("lok", "Current Queue: " << (aQueued.empty() ? "Empty" : aQueued));
        for (const CallbackData& c : m_queue)
            assert(c.validate());
    }
#endif

    bool bIsChartActive = false;
    if (type == LOK_CALLBACK_GRAPHIC_SELECTION)
    {
        LokChartHelper aChartHelper(SfxViewShell::Current());
        bIsChartActive = aChartHelper.GetWindow() != nullptr;
    }

    if (m_bPartTilePainting && !bIsChartActive)
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
            type != LOK_CALLBACK_TEXT_SELECTION)
        {
            SAL_INFO("lok", "Skipping while painting [" << type << "]: [" << payload << "].");
            return;
        }

        // In Writer we drop all notifications during painting.
        if (doc_getDocumentType(m_pDocument) == LOK_DOCTYPE_TEXT)
            return;
    }

    // Suppress invalid payloads.
    if (type == LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR &&
        payload.find(", 0, 0, ") != std::string::npos)
    {
        // The cursor position is often the relative coordinates of the widget
        // issuing it, instead of the absolute one that we expect.
        // This is temporary however, and, once the control is created and initialized
        // correctly, it eventually emits the correct absolute coordinates.
        SAL_INFO("lok", "Skipping invalid event [" << type << "]: [" << payload << "].");
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex);

    // drop duplicate callbacks for the listed types
    switch (type)
    {
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
        case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
        case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR :
        case LOK_CALLBACK_STATE_CHANGED:
        case LOK_CALLBACK_MOUSE_POINTER:
        case LOK_CALLBACK_CELL_CURSOR:
        case LOK_CALLBACK_CELL_VIEW_CURSOR:
        case LOK_CALLBACK_CELL_FORMULA:
        case LOK_CALLBACK_CELL_ADDRESS:
        case LOK_CALLBACK_CURSOR_VISIBLE:
        case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
        case LOK_CALLBACK_SET_PART:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        case LOK_CALLBACK_INVALIDATE_HEADER:
        case LOK_CALLBACK_WINDOW:
        {
            const auto& pos = std::find_if(m_queue.rbegin(), m_queue.rend(),
                    [type] (const queue_type::value_type& elem) { return (elem.Type == type); });

            if (pos != m_queue.rend() && pos->PayloadString == payload)
            {
                SAL_INFO("lok", "Skipping queue duplicate [" << type << + "]: [" << payload << "].");
                return;
            }
        }
        break;
    }

    if (type == LOK_CALLBACK_TEXT_SELECTION && payload.empty())
    {
        const auto& posStart = std::find_if(m_queue.rbegin(), m_queue.rend(),
                [] (const queue_type::value_type& elem) { return (elem.Type == LOK_CALLBACK_TEXT_SELECTION_START); });
        if (posStart != m_queue.rend())
            posStart->PayloadString.clear();

        const auto& posEnd = std::find_if(m_queue.rbegin(), m_queue.rend(),
                [] (const queue_type::value_type& elem) { return (elem.Type == LOK_CALLBACK_TEXT_SELECTION_END); });
        if (posEnd != m_queue.rend())
            posEnd->PayloadString.clear();
    }

    // When payload is empty discards any previous state.
    if (payload.empty())
    {
        switch (type)
        {
            case LOK_CALLBACK_TEXT_SELECTION_START:
            case LOK_CALLBACK_TEXT_SELECTION_END:
            case LOK_CALLBACK_TEXT_SELECTION:
            case LOK_CALLBACK_GRAPHIC_SELECTION:
            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            case LOK_CALLBACK_INVALIDATE_TILES:
                SAL_INFO("lok", "Removing dups of [" << type << "]: [" << payload << "].");
                removeAll([type] (const queue_type::value_type& elem) { return (elem.Type == type); });
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
            case LOK_CALLBACK_GRAPHIC_SELECTION:
            case LOK_CALLBACK_MOUSE_POINTER:
            case LOK_CALLBACK_CELL_CURSOR:
            case LOK_CALLBACK_CELL_FORMULA:
            case LOK_CALLBACK_CELL_ADDRESS:
            case LOK_CALLBACK_CURSOR_VISIBLE:
            case LOK_CALLBACK_SET_PART:
            case LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE:
            case LOK_CALLBACK_RULER_UPDATE:
            {
                removeAll([type] (const queue_type::value_type& elem) { return (elem.Type == type); });
            }
            break;

            // These are safe to use the latest state and ignore previous
            // ones (if any) since the last overrides previous ones,
            // but only if the view is the same.
            case LOK_CALLBACK_CELL_VIEW_CURSOR:
            case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            case LOK_CALLBACK_INVALIDATE_VIEW_CURSOR:
            case LOK_CALLBACK_TEXT_VIEW_SELECTION:
            case LOK_CALLBACK_VIEW_CURSOR_VISIBLE:
            {
                const int nViewId = lcl_getViewId(payload);
                removeAll(
                    [type, nViewId] (const queue_type::value_type& elem) {
                        return (elem.Type == type && nViewId == lcl_getViewId(elem));
                    }
                );
            }
            break;

            case LOK_CALLBACK_INVALIDATE_VISIBLE_CURSOR:
            {
                removeAll(
                    [type, &payload] (const queue_type::value_type& elem) {
                        return (elem.Type == type && elem.PayloadString == payload);
                    }
                );
            }
            break;

            case LOK_CALLBACK_INVALIDATE_TILES:
            {
                RectangleAndPart& rcNew = aCallbackData.setRectangleAndPart(payload);
                if (rcNew.isEmpty())
                {
                    SAL_INFO("lok", "Skipping invalid event [" << type << "]: [" << payload << "].");
                    return;
                }

                // If we have to invalidate all tiles, we can skip any new tile invalidation.
                // Find the last INVALIDATE_TILES entry, if any to see if it's invalidate-all.
                const auto& pos = std::find_if(m_queue.rbegin(), m_queue.rend(),
                        [] (const queue_type::value_type& elem) { return (elem.Type == LOK_CALLBACK_INVALIDATE_TILES); });
                if (pos != m_queue.rend())
                {
                    const RectangleAndPart& rcOld = pos->getRectangleAndPart();
                    if (rcOld.isInfinite() && (rcOld.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart))
                    {
                        SAL_INFO("lok", "Skipping queue [" << type << "]: [" << payload << "] since all tiles need to be invalidated.");
                        return;
                    }

                    if (rcOld.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart)
                    {
                        // If fully overlapping.
                        if (rcOld.m_aRectangle.IsInside(rcNew.m_aRectangle))
                        {
                            SAL_INFO("lok", "Skipping queue [" << type << "]: [" << payload << "] since overlaps existing all-parts.");
                            return;
                        }
                    }
                }

                if (rcNew.isInfinite())
                {
                    SAL_INFO("lok", "Have Empty [" << type << "]: [" << payload << "] so removing all with part " << rcNew.m_nPart << ".");
                    removeAll(
                        [&rcNew] (const queue_type::value_type& elem) {
                            if (elem.Type == LOK_CALLBACK_INVALIDATE_TILES)
                            {
                                // Remove exiting if new is all-encompassing, or if of the same part.
                                const RectangleAndPart rcOld = RectangleAndPart::Create(elem.PayloadString);
                                return (rcNew.m_nPart == -1 || rcOld.m_nPart == rcNew.m_nPart);
                            }

                            // Keep others.
                            return false;
                        }
                    );
                }
                else
                {
                    const auto rcOrig = rcNew;

                    SAL_INFO("lok", "Have [" << type << "]: [" << payload << "] so merging overlapping.");
                    removeAll(
                        [&rcNew] (const queue_type::value_type& elem) {
                            if (elem.Type == LOK_CALLBACK_INVALIDATE_TILES)
                            {
                                const RectangleAndPart& rcOld = elem.getRectangleAndPart();
                                if (rcNew.m_nPart != -1 && rcOld.m_nPart != -1 && rcOld.m_nPart != rcNew.m_nPart)
                                {
                                    SAL_INFO("lok", "Nothing to merge between new: " << rcNew.toString() << ", and old: " << rcOld.toString());
                                    return false;
                                }

                                if (rcNew.m_nPart == -1)
                                {
                                    // Don't merge unless fully overlapped.
                                    SAL_INFO("lok", "New " << rcNew.toString() << " has " << rcOld.toString() << "?");
                                    if (rcNew.m_aRectangle.IsInside(rcOld.m_aRectangle))
                                    {
                                        SAL_INFO("lok", "New " << rcNew.toString() << " engulfs old " << rcOld.toString() << ".");
                                        return true;
                                    }
                                }
                                else if (rcOld.m_nPart == -1)
                                {
                                    // Don't merge unless fully overlapped.
                                    SAL_INFO("lok", "Old " << rcOld.toString() << " has " << rcNew.toString() << "?");
                                    if (rcOld.m_aRectangle.IsInside(rcNew.m_aRectangle))
                                    {
                                        SAL_INFO("lok", "New " << rcNew.toString() << " engulfs old " << rcOld.toString() << ".");
                                        return true;
                                    }
                                }
                                else
                                {
                                    const tools::Rectangle rcOverlap = rcNew.m_aRectangle.GetIntersection(rcOld.m_aRectangle);
                                    const bool bOverlap = !rcOverlap.IsEmpty();
                                    SAL_INFO("lok", "Merging " << rcNew.toString() << " & " << rcOld.toString() << " => " <<
                                            rcOverlap.toString() << " Overlap: " << bOverlap);
                                    if (bOverlap)
                                    {
                                        rcNew.m_aRectangle.Union(rcOld.m_aRectangle);
                                        SAL_INFO("lok", "Merged: " << rcNew.toString());
                                        return true;
                                    }
                                }
                            }

                            // Keep others.
                            return false;
                        }
                    );

                    if (rcNew.m_aRectangle != rcOrig.m_aRectangle)
                    {
                        SAL_INFO("lok", "Replacing: " << rcOrig.toString() << " by " << rcNew.toString());
                        if (rcNew.m_aRectangle.GetWidth() < rcOrig.m_aRectangle.GetWidth() ||
                            rcNew.m_aRectangle.GetHeight() < rcOrig.m_aRectangle.GetHeight())
                        {
                            SAL_WARN("lok", "Error: merged rect smaller.");
                        }
                    }
                }

                aCallbackData.setRectangleAndPart(rcNew);
            }
            break;

            // State changes with same name override previous ones with a different value.
            // Ex. ".uno:PageStatus=Slide 20 of 83" overwrites any previous PageStatus.
            case LOK_CALLBACK_STATE_CHANGED:
            {
                // Compare the state name=value and overwrite earlier entries with same name.
                const auto pos = payload.find('=');
                if (pos != std::string::npos)
                {
                    const std::string name = payload.substr(0, pos + 1);
                    removeAll(
                        [type, &name] (const queue_type::value_type& elem) {
                            return (elem.Type == type) && (elem.PayloadString.compare(0, name.size(), name) == 0);
                        }
                    );
                }
            }
            break;

            case LOK_CALLBACK_WINDOW:
            {
                // reading JSON by boost might be slow?
                boost::property_tree::ptree& aTree = aCallbackData.setJson(payload);
                const unsigned nLOKWindowId = aTree.get<unsigned>("id", 0);
                if (aTree.get<std::string>("action", "") == "invalidate")
                {
                    std::string aRectStr = aTree.get<std::string>("rectangle", "");
                    // no 'rectangle' field => invalidate all of the window =>
                    // remove all previous window part invalidations
                    if (aRectStr.empty())
                    {
                        removeAll([&nLOKWindowId] (const queue_type::value_type& elem) {
                                if (elem.Type == LOK_CALLBACK_WINDOW)
                                {
                                    const boost::property_tree::ptree& aOldTree = elem.getJson();
                                    const unsigned nOldDialogId = aOldTree.get<unsigned>("id", 0);
                                    if (aOldTree.get<std::string>("action", "") == "invalidate" &&
                                        nLOKWindowId == nOldDialogId)
                                    {
                                        return true;
                                    }
                                }
                                return false;
                            });
                    }
                    else
                    {
                        // if we have to invalidate all of the window, ignore
                        // any part invalidation message
                        const auto invAllExist = std::any_of(m_queue.rbegin(), m_queue.rend(),
                                                       [&nLOKWindowId] (const queue_type::value_type& elem)
                                                       {
                                                           if (elem.Type != LOK_CALLBACK_WINDOW)
                                                               return false;

                                                           const boost::property_tree::ptree& aOldTree = elem.getJson();
                                                           const unsigned nOldDialogId = aOldTree.get<unsigned>("id", 0);
                                                           return aOldTree.get<std::string>("action", "") == "invalidate" &&
                                                               nLOKWindowId == nOldDialogId &&
                                                               aOldTree.get<std::string>("rectangle", "").empty();
                                                       });

                        // we found an invalidate-all window callback
                        if (invAllExist)
                        {
                            SAL_INFO("lok.dialog", "Skipping queue [" << type << "]: [" << payload << "] since whole window needs to be invalidated.");
                            return;
                        }

                        std::istringstream aRectStream(aRectStr);
                        long nLeft, nTop, nWidth, nHeight;
                        char nComma;
                        aRectStream >> nLeft >> nComma >> nTop >> nComma >> nWidth >> nComma >> nHeight;
                        tools::Rectangle aNewRect(nLeft, nTop, nLeft + nWidth, nTop + nHeight);
                        bool currentIsRedundant = false;
                        removeAll([&aNewRect, &nLOKWindowId, &currentIsRedundant] (const queue_type::value_type& elem) {
                                if (elem.Type != LOK_CALLBACK_WINDOW)
                                    return false;

                                const boost::property_tree::ptree& aOldTree = elem.getJson();
                                if (aOldTree.get<std::string>("action", "") == "invalidate")
                                {
                                    const unsigned nOldDialogId = aOldTree.get<unsigned>("id", 0);
                                    std::string aOldRectStr = aOldTree.get<std::string>("rectangle", "");
                                    // not possible that we encounter an empty
                                    // rectangle here; we already handled this
                                    // case before
                                    std::istringstream aOldRectStream(aOldRectStr);
                                    long nOldLeft, nOldTop, nOldWidth, nOldHeight;
                                    char nOldComma;
                                    aOldRectStream >> nOldLeft >> nOldComma >> nOldTop >> nOldComma >> nOldWidth >> nOldComma >> nOldHeight;
                                    tools::Rectangle aOldRect = tools::Rectangle(nOldLeft, nOldTop, nOldLeft + nOldWidth, nOldTop + nOldHeight);

                                    if (nLOKWindowId == nOldDialogId)
                                    {
                                        // new one engulfs the old one?
                                        if (aNewRect.IsInside(aOldRect))
                                        {
                                            SAL_INFO("lok.dialog", "New " << aNewRect.toString() << " engulfs old " << aOldRect.toString() << ".");
                                            return true;
                                        }
                                        // old one engulfs the new one?
                                        else if (aOldRect.IsInside(aNewRect))
                                        {
                                            SAL_INFO("lok.dialog", "Old " << aOldRect.toString() << " engulfs new " << aNewRect.toString() << ".");
                                            // we have a rectangle in the queue
                                            // already that makes the current
                                            // Callback useless
                                            currentIsRedundant = true;
                                            return false;
                                        }
                                        else
                                        {
                                            SAL_INFO("lok.dialog", "Merging " << aNewRect.toString() << " & " << aOldRect.toString());
                                            aNewRect.Union(aOldRect);
                                            SAL_INFO("lok.dialog", "Merged: " << aNewRect.toString());
                                            return true;
                                        }
                                    }
                                }

                                // keep rest
                                return false;
                            });

                        if (currentIsRedundant)
                        {
                            SAL_INFO("lok.dialog", "Current payload is engulfed by one already in the queue. Skipping redundant payload: " << aNewRect.toString());
                            return;
                        }

                        aTree.put("rectangle", aNewRect.toString().getStr());
                        aCallbackData.setJson(aTree);
                        assert(aCallbackData.validate() && "Validation after setJson failed!");
                    }
                }
            }
            break;
        }
    }

    // Validate that the cached data and the payload string are identical.
    assert(aCallbackData.validate() && "Cached callback payload object and string mismatch!");
    m_queue.emplace_back(aCallbackData);
    SAL_INFO("lok", "Queued #" << (m_queue.size() - 1) <<
             " [" << type << "]: [" << payload << "] to have " << m_queue.size() << " entries.");

    lock.unlock();
    if (!IsActive())
    {
        Start();
    }
}

void CallbackFlushHandler::Invoke()
{
    comphelper::ProfileZone aZone("CallbackFlushHander::Invoke");

    if (m_pCallback && !m_bEventLatch)
    {
        std::scoped_lock<std::mutex> lock(m_mutex);

        SAL_INFO("lok", "Flushing " << m_queue.size() << " elements.");
        for (const auto& rCallbackData : m_queue)
        {
            const int type = rCallbackData.Type;
            const auto& payload = rCallbackData.PayloadString;
            const int viewId = lcl_isViewCallbackType(type) ? lcl_getViewId(rCallbackData) : -1;

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

        m_queue.clear();
    }
}

void CallbackFlushHandler::removeAll(const std::function<bool (const CallbackFlushHandler::queue_type::value_type&)>& rTestFunc)
{
    auto newEnd = std::remove_if(m_queue.begin(), m_queue.end(), rTestFunc);
    m_queue.erase(newEnd, m_queue.end());
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

LibLibreOffice_Impl::LibLibreOffice_Impl()
    : m_pOfficeClass( gOfficeClass.lock() )
    , maThread(nullptr)
    , mpCallback(nullptr)
    , mpCallbackData(nullptr)
    , mOptionalFeatures(0)
{
    if(!m_pOfficeClass) {
        m_pOfficeClass.reset(new LibreOfficeKitClass);
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

        gOfficeClass = m_pOfficeClass;
    }

    pClass = m_pOfficeClass.get();
}

LibLibreOffice_Impl::~LibLibreOffice_Impl()
{
}

namespace
{

ITiledRenderable* getTiledRenderable(LibreOfficeKitDocument* pThis)
{
    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);
    return dynamic_cast<ITiledRenderable*>(pDocument->mxComponent.get());
}

#ifdef IOS
void paintTileToCGContext(ITiledRenderable* pDocument,
                          void* rCGContext, const Size nCanvasSize,
                          const int nTilePosX, const int nTilePosY,
                          const int nTileWidth, const int nTileHeight)
{
    SystemGraphicsData aData;
    aData.rCGContext = reinterpret_cast<CGContextRef>(rCGContext);

    ScopedVclPtrInstance<VirtualDevice> pDevice(&aData, Size(1, 1), DeviceFormat::DEFAULT);
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

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->maLastExceptionMsg.clear();

    OUString aURL(getAbsoluteURL(pURL));
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
        const OUString aLanguage = extractParameter(aOptions, "Language");

        if (!aLanguage.isEmpty())
        {
            // use with care - it sets it for the entire core, not just the
            // document
            SvtSysLocaleOptions aSysLocaleOptions;
            aSysLocaleOptions.SetLocaleConfigString(aLanguage);
            aSysLocaleOptions.SetUILocaleConfigString(aLanguage);
            // Set the LOK language tag, used for dialog tunneling.
            comphelper::LibreOfficeKit::setLanguageTag(aSysLocaleOptions.GetLanguageTag());
        }

        uno::Sequence<css::beans::PropertyValue> aFilterOptions(2);
        aFilterOptions[0] = css::beans::PropertyValue( "FilterOptions",
                                                       0,
                                                       uno::makeAny(aOptions),
                                                       beans::PropertyState_DIRECT_VALUE);

        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler("load", pLib));
        auto const pair(pLib->mInteractionMap.insert(std::make_pair(aURL.toUtf8(), pInteraction)));
        comphelper::ScopeGuard const g([&] () {
                if (pair.second)
                {
                    pLib->mInteractionMap.erase(aURL.toUtf8());
                }
            });
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction.get());
        aFilterOptions[1].Name = "InteractionHandler";
        aFilterOptions[1].Value <<= xInteraction;

        /* TODO
        sal_Int16 nMacroExecMode = document::MacroExecMode::USE_CONFIG;
        aFilterOptions[2].Name = "MacroExecutionMode";
        aFilterOptions[2].Value <<= nMacroExecMode;

        sal_Int16 nUpdateDoc = document::UpdateDocMode::ACCORDING_TO_CONFIG;
        aFilterOptions[3].Name = "UpdateDocMode";
        aFilterOptions[3].Value <<= nUpdateDoc;
        */

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

        LibLODocument_Impl* pDocument = new LibLODocument_Impl(xComponent);
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
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.begin());
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
        std::copy(pPrivateKeyBinary, pPrivateKeyBinary + nPrivateKeyBinarySize, aPrivateKeySequence.begin());
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

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    pLib->maLastExceptionMsg.clear();

    pLib->mpCallback = pCallback;
    pLib->mpCallbackData = pData;
}

static int doc_saveAs(LibreOfficeKitDocument* pThis, const char* sUrl, const char* pFormat, const char* pFilterOptions)
{
    comphelper::ProfileZone aZone("doc_saveAs");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    LibLODocument_Impl* pDocument = static_cast<LibLODocument_Impl*>(pThis);

    OUString sFormat = getUString(pFormat);
    OUString aURL(getAbsoluteURL(sUrl));
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

        // Check if watermark for pdf is passed by filteroptions..
        // It is not a real filter option so it must be filtered out.
        OUString watermarkText;
        int aIndex = -1;
        if ((aIndex = aFilterOptions.indexOf(",Watermark=")) >= 0)
        {
            int bIndex = aFilterOptions.indexOf("WATERMARKEND");
            watermarkText = aFilterOptions.copy(aIndex+11, bIndex-(aIndex+11));
            if(aIndex > 0)
            {
                OUString temp = aFilterOptions.copy(0, aIndex);
                aFilterOptions = temp + aFilterOptions.copy(bIndex+12);
            }
            else
            {
                aFilterOptions.clear();
            }
        }

        // 'TakeOwnership' == this is a 'real' SaveAs (that is, the document
        // gets a new name).  When this is not provided, the meaning of
        // saveAs() is more like save-a-copy, which allows saving to any
        // random format like PDF or PNG.
        // It is not a real filter option, so we have to filter it out.
        uno::Sequence<OUString> aOptionSeq = comphelper::string::convertCommaSeparated(aFilterOptions);
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
        aSaveMediaDescriptor[MediaDescriptor::PROP_FILTEROPTIONS()] <<= aFilterOptions;
        if(!watermarkText.isEmpty())
        {
            uno::Sequence< beans::PropertyValue > aFilterData( 1 );
            aFilterData[ 0 ].Name = "TiledWatermark";
            aFilterData[ 0 ].Value <<= watermarkText;
            aSaveMediaDescriptor["FilterData"] <<= aFilterData;
        }

        // add interaction handler too
        if (gImpl)
        {
            // gImpl does not have to exist when running from an unit test
            rtl::Reference<LOKInteractionHandler> const pInteraction(
                    new LOKInteractionHandler("saveas", gImpl, pDocument));
            uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction.get());

            aSaveMediaDescriptor[MediaDescriptor::PROP_INTERACTIONHANDLER()] <<= xInteraction;
        }

        uno::Reference<frame::XStorable> xStorable(pDocument->mxComponent, uno::UNO_QUERY_THROW);

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
        OUString(".uno:Cut"),
        OUString(".uno:Copy"),
        OUString(".uno:Paste"),
        OUString(".uno:SelectAll"),
        OUString(".uno:InsertAnnotation"),
        OUString(".uno:DeleteAnnotation"),
        OUString(".uno:ReplyComment"),
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
        OUString(".uno:Context"),
        OUString(".uno:WrapText"),
        OUString(".uno:ToggleMergeCells"),
        OUString(".uno:NumberFormatCurrency"),
        OUString(".uno:NumberFormatPercent"),
        OUString(".uno:NumberFormatDate"),
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
        OUString(".uno:InsertSymbol"),
        OUString(".uno:EditRegion"),
        OUString(".uno:ThesaurusDialog")
    };

    util::URL aCommandURL;
    SfxViewShell* pViewShell = SfxViewShell::Current();
    SfxViewFrame* pViewFrame = pViewShell? pViewShell->GetViewFrame(): nullptr;

    // check if Frame-Controller were created.
    if (!pViewShell && !pViewFrame)
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
        const SfxSlot* pSlot = nullptr;

        aCommandURL.Complete = sUnoCommand;
        xParser->parseStrict(aCommandURL);
        pSlot = rSlotPool.GetUnoSlot(aCommandURL.Path);

        // when null, this command is not supported by the given component
        // (like eg. Calc does not have ".uno:DefaultBullet" etc.)
        if (pSlot)
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

static void doc_setPart(LibreOfficeKitDocument* pThis, int nPart)
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

    pDoc->setPart( nPart );
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

    OUString aPartInfo = pDoc->getPartInfo( nPart );
    OString aString = OUStringToOString(aPartInfo, RTL_TEXTENCODING_UTF8);

    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    assert(pMemory); // Don't handle OOM conditions
    strcpy(pMemory, aString.getStr());
    return pMemory;
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

    OUString sRectangles = pDoc->getPartPageRectangles();
    OString aString = OUStringToOString(sRectangles, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    assert(pMemory); // Don't handle OOM conditions
    strcpy(pMemory, aString.getStr());
    return pMemory;

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

    OUString sName = pDoc->getPartName( nPart );
    OString aString = OUStringToOString(sName, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    assert(pMemory); // Don't handle OOM conditions
    strcpy(pMemory, aString.getStr());
    return pMemory;

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

    OUString sHash = pDoc->getPartHash(nPart);
    OString aString = OUStringToOString(sHash, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    assert(pMemory); // Don't handle OOM conditions
    strcpy(pMemory, aString.getStr());
    return pMemory;

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

#if defined(ANDROID)
/// For the distinction if the LOK is used for the 'old' (JNI-based) or the
/// 'new' (loolwsd-based) app.  Default to the 'new', ie. not used from JNI as
/// implemented in sal/android/libreofficekit-jni.c.
bool android_lok_from_jni = false;
#endif

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

#if defined(UNX) && !defined(MACOSX) && !defined(ENABLE_HEADLESS)

    // Painting of zoomed or HiDPI spreadsheets is special, we actually draw everything at 100%,
    // and only set cairo's (or CoreGraphic's, in the iOS case) scale factor accordingly, so that
    // everything is painted bigger or smaller. This is different to what Calc's internal scaling
    // would do - because that one is trying to fit the lines between cells to integer multiples of
    // pixels.
    comphelper::ScopeGuard dpiScaleGuard([]() { comphelper::LibreOfficeKit::setDPIScale(1.0); });
    double fDPIScaleX = 1.0;
    if (doc_getDocumentType(pThis) == LOK_DOCTYPE_SPREADSHEET)
    {
        fDPIScaleX = (nCanvasWidth * 3840.0) / (256.0 * nTileWidth);
        assert(fabs(fDPIScaleX - ((nCanvasHeight * 3840.0) / (256.0 * nTileHeight))) < 0.0001);
        comphelper::LibreOfficeKit::setDPIScale(fDPIScaleX);
    }

#if defined(IOS)
    paintTileIOS(pThis, pBuffer, nCanvasWidth, nCanvasHeight, fDPIScaleX, nTilePosX, nTilePosY, nTileWidth, nTileHeight);
#else
    ScopedVclPtrInstance< VirtualDevice > pDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT) ;

#if defined(ANDROID)
    if (!android_lok_from_jni)
#endif
    {
        // Set background to transparent by default.
        // [Unless it is the 'old' (JNI-based) Android app - no idea why it
        // needs avoiding this.]
        pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));
    }

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
        // same thing.
        int viewCount = doc_getViewsCount(pThis);
        if (viewCount == 0)
            return;

        std::vector<int> viewIds(viewCount);
        doc_getViewIds(pThis, viewIds.data(), viewCount);

        nOrigViewId = viewIds[0];
        doc_setView(pThis, nOrigViewId);
    }

    // Disable callbacks while we are painting.
    if (nOrigViewId >= 0 && pDocument->mpCallbackFlushHandlers[nOrigViewId])
        pDocument->mpCallbackFlushHandlers[nOrigViewId]->setPartTilePainting(true);

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
                        nViewId = static_cast<sal_Int32>(pViewShell->GetViewShellId());
                        doc_setView(pThis, nViewId);
                        break;
                    }
                    pViewShell = SfxViewShell::GetNext(*pViewShell);
                }
            }

            nOrigPart = doc_getPart(pThis);
            if (nPart != nOrigPart)
            {
                doc_setPart(pThis, nPart);
            }
        }

        doc_paintTile(pThis, pBuffer, nCanvasWidth, nCanvasHeight, nTilePosX, nTilePosY, nTileWidth, nTileHeight);

        if (!isText && nPart != nOrigPart)
        {
            doc_setPart(pThis, nOrigPart);
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

    if (nOrigViewId >= 0 && pDocument->mpCallbackFlushHandlers[nOrigViewId])
        pDocument->mpCallbackFlushHandlers[nOrigViewId]->setPartTilePainting(false);
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

    int nView = SfxLokHelper::getView();
    if (nView < 0)
        return;

    if (pCallback != nullptr)
    {
        size_t nId = nView;
        for (auto& pair : pDocument->mpCallbackFlushHandlers)
        {
            if (pair.first == nId)
                continue;

            pair.second->addViewStates(nView);
        }
    }
    else
    {
        size_t nId = nView;
        for (auto& pair : pDocument->mpCallbackFlushHandlers)
        {
            if (pair.first == nId)
                continue;

            pair.second->removeViewStates(nView);
        }
    }

    pDocument->mpCallbackFlushHandlers[nView].reset(new CallbackFlushHandler(pThis, pCallback, pData));

    if (pCallback != nullptr)
    {
        size_t nId = nView;
        for (const auto& pair : pDocument->mpCallbackFlushHandlers)
        {
            if (pair.first == nId)
                continue;

            pDocument->mpCallbackFlushHandlers[nView]->addViewStates(pair.first);
        }
    }

    if (SfxViewShell* pViewShell = SfxViewShell::Current())
    {
        pViewShell->registerLibreOfficeKitViewCallback(CallbackFlushHandler::callback, pDocument->mpCallbackFlushHandlers[nView].get());
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
    OUString aComments = pDoc->getPostIts();
    return strdup(aComments.toUtf8().getStr());
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
    OUString aComments = pDoc->getPostItsPos();
    return strdup(aComments.toUtf8().getStr());
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
    OUString state = pDoc->getRulerState();
    return strdup(state.toUtf8().getStr());
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

    pDoc->postKeyEvent(nType, nCharCode, nKeyCode);
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

    switch (nType)
    {
    case LOK_EXT_TEXTINPUT:
        pWindow->PostExtTextInputEvent(VclEventId::ExtTextInput,
                                       OUString::fromUtf8(OString(pText, strlen(pText))));
        break;
    case LOK_EXT_TEXTINPUT_END:
        pWindow->PostExtTextInputEvent(VclEventId::EndExtTextInput,
                                       OUString::fromUtf8(OString(pText, strlen(pText))));
        break;
    default:
        assert(false && "Unhandled External Text input event!");
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
        boost::property_tree::ptree aTree;
        aTree.put("commandName", maCommand.getStr());

        if (rEvent.State != frame::DispatchResultState::DONTKNOW)
        {
            bool bSuccess = (rEvent.State == frame::DispatchResultState::SUCCESS);
            aTree.put("success", bSuccess);
        }

        aTree.add_child("result", unoAnyToPropertyTree(rEvent.Result));

        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        OString aPayload = aStream.str().c_str();
        mpCallback->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aPayload.getStr());
    }

    virtual void SAL_CALL disposing(const css::lang::EventObject&) override {}
};

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

    // handle potential interaction
    if (gImpl && aCommand == ".uno:Save")
    {
        rtl::Reference<LOKInteractionHandler> const pInteraction(
            new LOKInteractionHandler("save", gImpl, pDocument));
        uno::Reference<task::XInteractionHandler2> const xInteraction(pInteraction.get());

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
            boost::property_tree::ptree aTree;
            aTree.put("commandName", pCommand);
            aTree.put("success", false);

            // Add the reason for not saving
            const uno::Any aResultValue = uno::makeAny(OUString("unmodified"));
            aTree.add_child("result", unoAnyToPropertyTree(aResultValue));

            std::stringstream aStream;
            boost::property_tree::write_json(aStream, aTree);
            OString aPayload = aStream.str().c_str();
            pDocument->mpCallbackFlushHandlers[nView]->queue(LOK_CALLBACK_UNO_COMMAND_RESULT, aPayload.getStr());
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

        if (aChartHelper.GetWindow())
        {
            if (aPropertyValuesVector[0].Name != "Action")
            {
                tools::Rectangle aChartBB = aChartHelper.GetChartBoundingBox();
                int nLeft = OutputDevice::LogicToLogic(aChartBB.Left(), MapUnit::MapTwip, MapUnit::Map100thMM);
                int nTop = OutputDevice::LogicToLogic(aChartBB.Top(), MapUnit::MapTwip, MapUnit::Map100thMM);

                sal_Int32 value;
                for (beans::PropertyValue& rPropValue: aPropertyValuesVector)
                {
                    if (rPropValue.Name == "TransformPosX" || rPropValue.Name == "TransformRotationX")
                    {
                        rPropValue.Value >>= value;
                        rPropValue.Value <<= value - nLeft;
                    }
                    else if (rPropValue.Name == "TransformPosY" || rPropValue.Name == "TransformRotationY")
                    {
                        rPropValue.Value >>= value;
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

    bool bResult = false;
    if (bNotifyWhenFinished && pDocument->mpCallbackFlushHandlers[nView])
    {
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector),
                new DispatchResultListener(pCommand, pDocument->mpCallbackFlushHandlers[nView]));
    }
    else
        bResult = comphelper::dispatchCommand(aCommand, comphelper::containerToSequence(aPropertyValuesVector));

    if (!bResult)
    {
        SetLastExceptionMsg("Failed to dispatch the .uno: command");
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

    pDoc->postMouseEvent(nType, nX, nY, nCount, nButtons, nModifier);
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

    Point aPos(nX, nY);
    MouseEvent aEvent(aPos, nCount, MouseEventModifiers::SIMPLECLICK, nButtons, nModifier);

    if (Dialog* pDialog = dynamic_cast<Dialog*>(pWindow.get()))
    {
        pDialog->EnableInput();
    }

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

    if (Dialog* pDialog = dynamic_cast<Dialog*>(pWindow.get()))
    {
        pDialog->EnableInput();
    }

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

static bool getFromTransferrable(
    const css::uno::Reference<css::datatransfer::XTransferable> &xTransferable,
    const char *pMimeType, OString &aRet)
{
    // Take care of UTF-8 text here.
    OString aMimeType(pMimeType);
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
            aRet = OString(reinterpret_cast<const sal_Char *>(aString.getStr()), aString.getLength() * sizeof(sal_Unicode));
    }
    else
    {
        uno::Sequence<sal_Int8> aSequence;
        aAny >>= aSequence;
        aRet = OString(reinterpret_cast<sal_Char*>(aSequence.getArray()), aSequence.getLength());
    }

    return true;;
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
    bool bSuccess = getFromTransferrable(xTransferable, pType, aRet);
    if (!bSuccess)
        return nullptr;

    char* pMemory = static_cast<char*>(malloc(aRet.getLength() + 1));
    assert(pMemory); // Don't handle OOM conditions
    strcpy(pMemory, aRet.getStr());

    if (pUsedMimeType) // legacy
    {
        if (pMimeType)
        {
            *pUsedMimeType = static_cast<char*>(malloc(strlen(pMimeType) + 1));
            strcpy(*pUsedMimeType, pMimeType);
        }
        else
            *pUsedMimeType = nullptr;
    }

    return pMemory;
}

static bool doc_paste(LibreOfficeKitDocument* pThis, const char* pMimeType, const char* pData, size_t nSize)
{
    comphelper::ProfileZone aZone("doc_paste");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    ITiledRenderable* pDoc = getTiledRenderable(pThis);
    if (!pDoc)
    {
        SetLastExceptionMsg("Document doesn't support tiled rendering");
        return false;
    }

    uno::Reference<datatransfer::XTransferable> xTransferable(new LOKTransferable(pMimeType, pData, nSize));
    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard(new LOKClipboard);
    xClipboard->setContents(xTransferable, uno::Reference<datatransfer::clipboard::XClipboardOwner>());
    pDoc->setClipboard(xClipboard);
    if (!pDoc->isMimeTypeSupported())
    {
        SetLastExceptionMsg("Document doesn't support this mime type");
        return false;
    }

    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"AnchorType", uno::makeAny(static_cast<sal_uInt16>(text::TextContentAnchorType_AS_CHARACTER))},
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
            css::uno::Reference<css::linguistic2::XSpellChecker> xSpell(xLangSrv->getSpellChecker(), css::uno::UNO_QUERY);
            css::uno::Reference<css::linguistic2::XSupportedLocales> xLocales(xSpell, css::uno::UNO_QUERY);

            if (xLocales.is())
                aLocales = xLocales->getLocales();
        }
    }

    boost::property_tree::ptree aTree;
    aTree.put("commandName", pCommand);
    boost::property_tree::ptree aValues;
    boost::property_tree::ptree aChild;
    OUString sLanguage;
    for ( sal_Int32 itLocale = 0; itLocale < aLocales.getLength(); itLocale++ )
    {
        sLanguage = SvtLanguageTable::GetLanguageString(LanguageTag::convertToLanguageType(aLocales[itLocale]));
        if (sLanguage.startsWith("{") && sLanguage.endsWith("}"))
            continue;

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
            const sal_IntPtr* pAry = pList->GetSizeAry(rFontMetric);
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

static char* getFontSubset (const OString& aFontName)
{
    OUString aFoundFont(::rtl::Uri::decode(OStringToOUString(aFontName, RTL_TEXTENCODING_UTF8), rtl_UriDecodeStrict, RTL_TEXTENCODING_UTF8));
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(
        pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;

    boost::property_tree::ptree aTree;
    aTree.put("commandName", ".uno:FontSubset");
    boost::property_tree::ptree aValues;

    if ( pList && !aFoundFont.isEmpty() )
    {
        sal_uInt16 nFontCount = pList->GetFontNameCount();
        sal_uInt16 nItFont = 0;
        for (; nItFont < nFontCount; ++nItFont)
        {
            if (aFoundFont == pList->GetFontName(nItFont).GetFamilyName())
            {
                break;
            }
        }

        if ( nItFont < nFontCount )
        {
            FontCharMapRef xFontCharMap (new FontCharMap());
            auto aDevice(VclPtr<VirtualDevice>::Create(nullptr, Size(1, 1), DeviceFormat::DEFAULT));
            const vcl::Font& aFont(pList->GetFontName(nItFont));

            aDevice->SetFont(aFont);
            aDevice->GetFontCharMap(xFontCharMap);
            SubsetMap aSubMap(xFontCharMap);

            for (auto const& subset : aSubMap.GetSubsetMap())
            {
                boost::property_tree::ptree aChild;
                aChild.put("", static_cast<int>(ublock_getCode(subset.GetRangeMin())));
                aValues.push_back(std::make_pair("", aChild));
            }
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
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Sequence<OUString> aStyleFamilies = xStyleFamilies->getElementNames();

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
    for (sal_Int32 nStyleFam = 0; nStyleFam < aStyleFamilies.getLength(); ++nStyleFam)
    {
        boost::property_tree::ptree aChildren;
        OUString sStyleFam = aStyleFamilies[nStyleFam];
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

        uno::Sequence<OUString> aStyles = xStyleFamily->getElementNames();
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
        OUString sName;
        boost::property_tree::ptree aChild;
        boost::property_tree::ptree aChildren;
        const OUString sPageStyles("PageStyles");
        uno::Reference<beans::XPropertySet> xProperty;
        uno::Reference<container::XNameContainer> xContainer;

        if (xStyleFamilies->hasByName(sPageStyles) && (xStyleFamilies->getByName(sPageStyles) >>= xContainer))
        {
            uno::Sequence<OUString> aSeqNames = xContainer->getElementNames();
            for (sal_Int32 itName = 0; itName < aSeqNames.getLength(); itName++)
            {
                bool bIsPhysical;
                sName = aSeqNames[itName];
                xProperty.set(xContainer->getByName(sName), uno::UNO_QUERY);
                if (xProperty.is() && (xProperty->getPropertyValue("IsPhysical") >>= bIsPhysical) && bIsPhysical)
                {
                    xProperty->getPropertyValue("DisplayName") >>= sName;
                    aChild.put("", sName.toUtf8());
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

enum class UndoOrRedo
{
    UNDO,
    REDO
};

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
    std::stringstream aStream;
    // We want positions of the track changes also which is not possible from
    // UNO. Enable positioning information for text documents only for now, so
    // construct the tracked changes JSON from inside the sw/, not here using UNO
    if (doc_getDocumentType(pThis) != LOK_DOCTYPE_TEXT && xRedlinesSupplier.is())
    {
        uno::Reference<container::XEnumeration> xRedlines = xRedlinesSupplier->getRedlines()->createEnumeration();
        boost::property_tree::ptree aRedlines;
        for (size_t nIndex = 0; xRedlines->hasMoreElements(); ++nIndex)
        {
            uno::Reference<beans::XPropertySet> xRedline(xRedlines->nextElement(), uno::UNO_QUERY);
            boost::property_tree::ptree aRedline;
            aRedline.put("index", nIndex);

            OUString sAuthor;
            xRedline->getPropertyValue("RedlineAuthor") >>= sAuthor;
            aRedline.put("author", sAuthor.toUtf8().getStr());

            OUString sType;
            xRedline->getPropertyValue("RedlineType") >>= sType;
            aRedline.put("type", sType.toUtf8().getStr());

            OUString sComment;
            xRedline->getPropertyValue("RedlineComment") >>= sComment;
            aRedline.put("comment", sComment.toUtf8().getStr());

            OUString sDescription;
            xRedline->getPropertyValue("RedlineDescription") >>= sDescription;
            aRedline.put("description", sDescription.toUtf8().getStr());

            util::DateTime aDateTime;
            xRedline->getPropertyValue("RedlineDateTime") >>= aDateTime;
            OUString sDateTime = utl::toISO8601(aDateTime);
            aRedline.put("dateTime", sDateTime.toUtf8().getStr());

            aRedlines.push_back(std::make_pair("", aRedline));
        }

        boost::property_tree::ptree aTree;
        aTree.add_child("redlines", aRedlines);
        boost::property_tree::write_json(aStream, aTree);
    }
    else
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg("Document doesn't support tiled rendering");
            return nullptr;
        }
        OUString aTrackedChanges = pDoc->getTrackedChanges();
        aStream << aTrackedChanges.toUtf8();
    }

    char* pJson = strdup(aStream.str().c_str());
    return pJson;
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
    OUString aAuthors = pDoc->getTrackedChangeAuthors();
    return strdup(aAuthors.toUtf8().getStr());
}

static char* doc_getCommandValues(LibreOfficeKitDocument* pThis, const char* pCommand)
{
    comphelper::ProfileZone aZone("doc_getCommandValues");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    OString aCommand(pCommand);
    static const OString aViewRowColumnHeaders(".uno:ViewRowColumnHeaders");
    static const OString aCellCursor(".uno:CellCursor");
    static const OString aFontSubset(".uno:FontSubset&name=");

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

        OUString aHeaders = pDoc->getRowColumnHeaders(aRectangle);
        if (aHeaders.isEmpty())
            return nullptr;

        OString aString = OUStringToOString(aHeaders, RTL_TEXTENCODING_UTF8);

        char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
        assert(pMemory); // Don't handle OOM conditions
        strcpy(pMemory, aString.getStr());
        return pMemory;
    }
    else if (aCommand.startsWith(aCellCursor))
    {
        ITiledRenderable* pDoc = getTiledRenderable(pThis);
        if (!pDoc)
        {
            SetLastExceptionMsg("Document doesn't support tiled rendering");
            return nullptr;
        }

        // Command has parameters.
        int nOutputWidth = 0;
        int nOutputHeight = 0;
        long nTileWidth = 0;
        long nTileHeight = 0;
        if (aCommand.getLength() > aCellCursor.getLength())
        {
            OString aArguments = aCommand.copy(aCellCursor.getLength() + 1);
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
                if (aKey == "outputWidth")
                    nOutputWidth = aValue.toInt32();
                else if (aKey == "outputHeight")
                    nOutputHeight = aValue.toInt32();
                else if (aKey == "tileWidth")
                    nTileWidth = aValue.toInt64();
                else if (aKey == "tileHeight")
                    nTileHeight = aValue.toInt64();
            }
            while (nParamIndex >= 0);
        }

        OString aString = pDoc->getCellCursor(nOutputWidth, nOutputHeight, nTileWidth, nTileHeight);

        char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
        assert(pMemory); // Don't handle OOM conditions
        strcpy(pMemory, aString.getStr());
        return pMemory;
    }
    else if (aCommand.startsWith(aFontSubset))
    {
        return getFontSubset(OString(pCommand + aFontSubset.getLength()));
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

static int doc_createViewWithOptions(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/,
                                     const char* pOptions)
{
    comphelper::ProfileZone aZone("doc_createView");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    OUString aOptions = getUString(pOptions);
    const OUString aLanguage = extractParameter(aOptions, "Language");

    if (!aLanguage.isEmpty())
    {
        // Set the LOK language tag, used for dialog tunneling.
        comphelper::LibreOfficeKit::setLanguageTag(LanguageTag(aLanguage));
    }

    return SfxLokHelper::createView();
}

static int doc_createView(LibreOfficeKitDocument* pThis)
{
    return doc_createViewWithOptions(pThis, nullptr); // No options.
}

static void doc_destroyView(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId)
{
    comphelper::ProfileZone aZone("doc_destroyView");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxLokHelper::destroyView(nId);
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

static int doc_getViewsCount(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/)
{
    comphelper::ProfileZone aZone("doc_getViewsCount");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    return SfxLokHelper::getViewsCount();
}

static bool doc_getViewIds(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int* pArray, size_t nSize)
{
    comphelper::ProfileZone aZone("doc_getViewsIds");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    return SfxLokHelper::getViewIds(pArray, nSize);
}

static void doc_setViewLanguage(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/, int nId, const char* language)
{
    comphelper::ProfileZone aZone("doc_setViewLanguage");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    SfxLokHelper::setViewLanguage(nId, OStringToOUString(language, RTL_TEXTENCODING_UTF8));
}

unsigned char* doc_renderFont(SAL_UNUSED_PARAMETER LibreOfficeKitDocument* /*pThis*/,
                              const char* pFontName,
                              const char* pChar,
                              int* pFontWidth,
                              int* pFontHeight)
{
    comphelper::ProfileZone aZone("doc_renderFont");

    SolarMutexGuard aGuard;
    SetLastExceptionMsg();

    OString aSearchedFontName(pFontName);
    OUString aText(OStringToOUString(pChar, RTL_TEXTENCODING_UTF8));
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SvxFontListItem* pFonts = static_cast<const SvxFontListItem*>(
        pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
    const FontList* pList = pFonts ? pFonts->GetFontList() : nullptr;

    const int nDefaultFontSize = 25;

    if ( pList )
    {
        sal_uInt16 nFontCount = pList->GetFontNameCount();
        for (sal_uInt16 i = 0; i < nFontCount; ++i)
        {
            const FontMetric& rFontMetric = pList->GetFontName(i);
            const OUString& aFontName = rFontMetric.GetFamilyName();
            if (aSearchedFontName != aFontName.toUtf8())
                continue;

            if (aText.isEmpty())
                aText = rFontMetric.GetFamilyName();

            auto aDevice(
                VclPtr<VirtualDevice>::Create(
                    nullptr, Size(1, 1), DeviceFormat::DEFAULT));
            ::tools::Rectangle aRect;
            vcl::Font aFont(rFontMetric);
            aFont.SetFontSize(Size(0, nDefaultFontSize));
            aDevice->SetFont(aFont);
            aDevice->GetTextBoundRect(aRect, aText);
            if (aRect.IsEmpty())
                break;

            int nFontWidth = aRect.BottomRight().X() + 1;
            int nFontHeight = aRect.BottomRight().Y() + 1;

            if (!(nFontWidth > 0 && nFontHeight > 0))
                break;

            if (*pFontWidth > 0 && *pFontHeight > 0)
            {
                double fScaleX = *pFontWidth / static_cast<double>(nFontWidth);
                double fScaleY = *pFontHeight / static_cast<double>(nFontHeight);

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
                break;

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
                        | DrawTextFlags::WordBreakHyphenation;// | DrawTextFlags::WordBreak ;

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
    }
    return nullptr;
}

static void doc_paintWindow(LibreOfficeKitDocument* pThis, unsigned nLOKWindowId,
                            unsigned char* pBuffer,
                            const int nX, const int nY,
                            const int nWidth, const int nHeight)
{
    doc_paintWindowDPI(pThis, nLOKWindowId, pBuffer, nX, nY, nWidth, nHeight, 1.0);
}

static void doc_paintWindowDPI(LibreOfficeKitDocument* /*pThis*/, unsigned nLOKWindowId,
                               unsigned char* pBuffer,
                               const int nX, const int nY,
                               const int nWidth, const int nHeight,
                               const double fDPIScale)
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

    ScopedVclPtrInstance<VirtualDevice> pDevice(&aData, Size(1, 1), DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixel(Size(nWidth, nHeight));

    MapMode aMapMode(pDevice->GetMapMode());
    aMapMode.SetOrigin(Point(-(nX / fDPIScale), -(nY / fDPIScale)));
    pDevice->SetMapMode(aMapMode);

    comphelper::LibreOfficeKit::setDialogPainting(true);
    pWindow->PaintToDevice(pDevice.get(), Point(0, 0), Size());
    comphelper::LibreOfficeKit::setDialogPainting(false);

    CGContextRelease(cgc);

#else

    ScopedVclPtrInstance<VirtualDevice> pDevice(nullptr, Size(1, 1), DeviceFormat::DEFAULT);
    pDevice->SetBackground(Wallpaper(COL_TRANSPARENT));

    pDevice->SetOutputSizePixelScaleOffsetAndBuffer(Size(nWidth, nHeight), Fraction(1.0), Point(), pBuffer);

    MapMode aMapMode(pDevice->GetMapMode());
    aMapMode.SetOrigin(Point(-(nX / fDPIScale), -(nY / fDPIScale)));
    pDevice->SetMapMode(aMapMode);

    comphelper::LibreOfficeKit::setDialogPainting(true);
    pWindow->PaintToDevice(pDevice.get(), Point(0, 0), Size());
    comphelper::LibreOfficeKit::setDialogPainting(false);

#endif
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
        if (Dialog* pDialog = dynamic_cast<Dialog*>(pWindow.get()))
            pDialog->Close();
        else if (FloatingWindow* pFloatWin = dynamic_cast<FloatingWindow*>(pWindow.get()))
            pFloatWin->EndPopupMode(FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll);
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
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.begin());
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
        std::copy(pPrivateKeyBinary, pPrivateKeyBinary + nPrivateKeySize, aPrivateKeySequence.begin());
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
        std::copy(pCertificateBinary, pCertificateBinary + nCertificateBinarySize, aCertificateSequence.begin());
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

static char* lo_getError (LibreOfficeKit *pThis)
{
    comphelper::ProfileZone aZone("lo_getError");

    SolarMutexGuard aGuard;

    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(pThis);
    OString aString = OUStringToOString(pLib->maLastExceptionMsg, RTL_TEXTENCODING_UTF8);
    char* pMemory = static_cast<char*>(malloc(aString.getLength() + 1));
    assert(pMemory); // Don't handle OOM conditions
    strcpy(pMemory, aString.getStr());
    return pMemory;
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
    uno::Sequence<OUString> aTypes = xTypeDetection->getElementNames();
    boost::property_tree::ptree aTree;
    for (const OUString& rType : aTypes)
    {
        uno::Sequence<beans::PropertyValue> aValues;
        if (xTypeDetection->getByName(rType) >>= aValues)
        {
            auto it = std::find_if(aValues.begin(), aValues.end(), [](const beans::PropertyValue& rValue) { return rValue.Name == "MediaType"; });
            OUString aValue;
            if (it != aValues.end() && (it->Value >>= aValue) && !aValue.isEmpty())
            {
                boost::property_tree::ptree aChild;
                aChild.put("MediaType", aValue.toUtf8());
                aTree.add_child(rType.toUtf8().getStr(), aChild);
            }
        }
    }
    std::stringstream aStream;
    boost::property_tree::write_json(aStream, aTree);
    return strdup(aStream.str().c_str());
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
    const OUString sVersionStrTemplate(
        "{ "
        "\"ProductName\": \"%PRODUCTNAME\", "
        "\"ProductVersion\": \"%PRODUCTVERSION\", "
        "\"ProductExtension\": \"%PRODUCTEXTENSION\", "
        "\"BuildId\": \"%BUILDID\" "
        "}"
    );
    const OString sVersionStr = OUStringToOString(ReplaceStringHookProc(sVersionStrTemplate), RTL_TEXTENCODING_UTF8);

    char* pVersion = static_cast<char*>(malloc(sVersionStr.getLength() + 1));
    assert(pVersion); // Don't handle OOM conditions
    strcpy(pVersion, sVersionStr.getStr());
    return pVersion;
}

static void force_c_locale()
{
    // force locale (and resource files loaded) to en-US
    OUString aLangISO("en-US");
    SvtSysLocaleOptions aLocalOptions;
    aLocalOptions.SetLocaleConfigString(aLangISO);
    aLocalOptions.SetUILocaleConfigString(aLangISO);
}

static void aBasicErrorFunc(const OUString& rError, const OUString& rAction)
{
    OStringBuffer aBuffer("Unexpected dialog: ");
    aBuffer.append(OUStringToOString(rAction, RTL_TEXTENCODING_ASCII_US));
    aBuffer.append(" Error: ");
    aBuffer.append(OUStringToOString(rError, RTL_TEXTENCODING_ASCII_US));

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

static void lo_status_indicator_callback(void *data, comphelper::LibreOfficeKit::statusIndicatorCallbackType type, int percent)
{
    LibLibreOffice_Impl* pLib = static_cast<LibLibreOffice_Impl*>(data);

    if (!pLib->mpCallback)
        return;

    switch (type)
    {
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::Start:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_START, nullptr, pLib->mpCallbackData);
        break;
    case comphelper::LibreOfficeKit::statusIndicatorCallbackType::SetValue:
        pLib->mpCallback(LOK_CALLBACK_STATUS_INDICATOR_SET_VALUE, OUString::number(percent).toUtf8().getStr(), pLib->mpCallbackData);
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
    for (auto &it : aLocales)
    {
        std::cerr << it.Language << "_" << it.Country << " ";
        css::beans::PropertyValues aNone;
        xSpellChecker->isValid("forcefed", it, aNone);
    }
    std::cerr << "\n";

    // preload all available thesauri
    css::uno::Reference<linguistic2::XThesaurus> xThesaurus(xLngSvcMgr->getThesaurus());
    css::uno::Reference<linguistic2::XSupportedLocales> xThesLocales(xSpellChecker, css::uno::UNO_QUERY_THROW);
    aLocales = xThesLocales->getLocales();
    std::cerr << "Preloading thesauri: ";
    for (auto &it : aLocales)
    {
        std::cerr << it.Language << "_" << it.Country << " ";
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
        css::uno::Reference<css::linguistic2::XSpellChecker> xSpell(xLangSrv->getSpellChecker(), css::uno::UNO_QUERY);
        css::uno::Reference<css::linguistic2::XSupportedLocales> xLocales(xSpell, css::uno::UNO_QUERY);
        if (xLocales.is())
            aLocales = xLocales->getLocales();
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

    // Set user profile's path back to the original one
    rtl::Bootstrap::set("UserInstallation", sUserPath);
}

class ProfileZoneDumper : public AutoTimer
{
    static const int dumpTimeoutMS = 5000;
public:
    ProfileZoneDumper() : AutoTimer( "zone dumper" )
    {
        SetTimeout(dumpTimeoutMS);
        Start();
    }
    virtual void Invoke() override
    {
        css::uno::Sequence<OUString> aEvents =
            comphelper::ProfileRecording::getRecordingAndClear();
        OStringBuffer aOutput;
        for (auto &s : aEvents)
        {
            aOutput.append(OUStringToOString(s, RTL_TEXTENCODING_UTF8));
            aOutput.append("\n");
        }
        OString aChunk = aOutput.makeStringAndClear();
        if (gImpl && gImpl->mpCallback)
            gImpl->mpCallback(LOK_CALLBACK_PROFILE_FRAME, aChunk.getStr(), gImpl->mpCallbackData);
    }
};

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

    { // cf. string lifetime for preinit
        std::vector<OUString> aOpts;

        // ':' delimited options - avoiding ABI change for new parameters
        const char *pOptions = getenv("SAL_LOK_OPTIONS");
        if (pOptions)
            aOpts = comphelper::string::split(OUString(pOptions, strlen(pOptions), RTL_TEXTENCODING_UTF8), ':');
        for (auto &it : aOpts)
        {
            if (it == "unipoll")
                bUnipoll = true;
            else if (it == "profile_events")
                bProfileZones = true;
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
        comphelper::ProfileRecording::startRecording(true);
        new ProfileZoneDumper();
    }

    comphelper::ProfileZone aZone("lok-init");

    if (eStage == PRE_INIT)
        rtl_alloc_preInit(true);
    else if (eStage == SECOND_INIT)
        rtl_alloc_preInit(false);

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
                    NSLog(@"ucnv_open(iso-8859-3)-> %p, err = %s, name=%s",
                          (void *)cnv, u_errorName(icuStatus), (!cnv)?"?":ucnv_getName(cnv,&icuStatus));
                    if (U_SUCCESS(icuStatus))
                        ucnv_close(cnv);
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
                    VclBuilder::preload();
                    VclAbstractDialogFactory::Create();
                }

                preloadData();

                // Release Solar Mutex, lo_startmain thread should acquire it.
                Application::ReleaseSolarMutex();
            }

            force_c_locale();
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

}

#ifdef IOS

// Used by the unmaintained LibreOfficeLight app. Once that has been retired, get rid of this, too.

extern "C"
{
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
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
