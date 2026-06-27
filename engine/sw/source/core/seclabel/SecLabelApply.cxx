/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SecLabelApply.hxx>

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

using namespace css;

namespace sw::seclabel
{
namespace
{
uno::Reference<xml::dom::XDocument>
parseToDom(const uno::Reference<uno::XComponentContext>& xContext, std::u16string_view rXml)
{
    const OString aUtf8 = OUStringToOString(rXml, RTL_TEXTENCODING_UTF8);
    SvMemoryStream aStream;
    aStream.WriteBytes(aUtf8.getStr(), aUtf8.getLength());
    aStream.Seek(0);
    uno::Reference<io::XInputStream> xInput(new utl::OInputStreamWrapper(aStream));
    uno::Reference<xml::dom::XDocumentBuilder> xBuilder(xml::dom::DocumentBuilder::create(xContext));
    return xBuilder->parse(xInput);
}

void appendDom(comphelper::SequenceAsHashMap& rGrabBag, const OUString& rKey,
               const uno::Reference<xml::dom::XDocument>& xDom)
{
    cpo::uno::Sequence<uno::Reference<xml::dom::XDocument>> aList;
    rGrabBag[rKey] >>= aList; // empty if the key was absent
    const sal_Int32 nOld = aList.getLength();
    aList.realloc(nOld + 1);
    aList.getArray()[nOld] = xDom;
    rGrabBag[rKey] <<= aList;
}

// Set one page area (header or footer) to the marking text, formatted.
void setMarkingArea(const uno::Reference<beans::XPropertySet>& xPageStyle, const OUString& rIsOn,
                    const OUString& rTextProp, const OUString& rMarking, sal_Int32 nColor)
{
    xPageStyle->setPropertyValue(rIsOn, cpo::uno::Any(true));
    uno::Reference<text::XText> xText(xPageStyle->getPropertyValue(rTextProp), uno::UNO_QUERY);
    if (!xText.is())
        return;
    xText->setString(rMarking);
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    xCursor->gotoStart(false);
    xCursor->gotoEnd(true);
    uno::Reference<beans::XPropertySet> xProps(xCursor, uno::UNO_QUERY);
    if (!xProps.is())
        return;
    xProps->setPropertyValue(u"CharWeight"_ustr, cpo::uno::Any(awt::FontWeight::BOLD));
    xProps->setPropertyValue(u"CharColor"_ustr, cpo::uno::Any(nColor));
    xProps->setPropertyValue(u"ParaAdjust"_ustr, cpo::uno::Any(style::ParagraphAdjust_CENTER));
}

// Serialize a DOM document back to its XML string.
OUString domToString(const uno::Reference<xml::dom::XDocument>& xDom)
{
    uno::Reference<xml::sax::XSAXSerializable> xSer(xDom, uno::UNO_QUERY);
    if (!xSer.is())
        return OUString();
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xOut(new utl::OOutputStreamWrapper(aStream));
    uno::Reference<xml::sax::XWriter> xWriter(
        xml::sax::Writer::create(comphelper::getProcessComponentContext()));
    xWriter->setOutputStream(xOut);
    xSer->serialize(xWriter, cpo::uno::Sequence<beans::StringPair>());
    return OUString(static_cast<const char*>(aStream.GetData()),
                    static_cast<sal_Int32>(aStream.GetSize()), RTL_TEXTENCODING_UTF8);
}

// Parse a customXml part into rLabel. True when it carries a confidentiality label,
// whether a 4778 binding wrapper or a standalone 4774 label.
bool parseLabelDom(const uno::Reference<xml::dom::XDocument>& xDom, StanagLabel& rLabel)
{
    if (!xDom.is())
        return false;

    const OUString sXml = domToString(xDom);
    if (sXml.isEmpty())
        return false;

    const OString aUtf8 = OUStringToOString(sXml, RTL_TEXTENCODING_UTF8);
    SvMemoryStream aStream(const_cast<char*>(aUtf8.getStr()), aUtf8.getLength(),
                           StreamMode::READ);
    return rLabel.parse(aStream) && !rLabel.aClassification.isEmpty();
}

// Index of the label's customXml part, or -1 if absent. OOXCustomXml and
// OOXCustomXmlProps are parallel: the same index in one is the counterpart of the
// other. Matched on the part readLabel would read, so that a standalone 4774 label,
// or one another tool wrote, is replaced or removed instead of being left in the
// document beside a new one. The itemProps schema URI is the fallback, for a part
// whose payload we cannot parse.
sal_Int32 findStanagPart(comphelper::SequenceAsHashMap& rGrabBag)
{
    cpo::uno::Sequence<uno::Reference<xml::dom::XDocument>> aList;
    rGrabBag[u"OOXCustomXml"_ustr] >>= aList;
    for (sal_Int32 i = 0; i < aList.getLength(); ++i)
    {
        StanagLabel aLabel;
        if (parseLabelDom(aList[i], aLabel))
            return i;
    }

    cpo::uno::Sequence<uno::Reference<xml::dom::XDocument>> aProps;
    rGrabBag[u"OOXCustomXmlProps"_ustr] >>= aProps;
    for (sal_Int32 i = 0; i < aProps.getLength(); ++i)
    {
        if (aProps[i].is() && domToString(aProps[i]).indexOf(STANAG_BINDING_SCHEMA) >= 0)
            return i;
    }

    return -1;
}

// Drop the DOM at nIndex from the rKey list of rGrabBag.
void eraseDomAt(comphelper::SequenceAsHashMap& rGrabBag, const OUString& rKey, sal_Int32 nIndex)
{
    cpo::uno::Sequence<uno::Reference<xml::dom::XDocument>> aList;
    rGrabBag[rKey] >>= aList;
    if (nIndex < 0 || nIndex >= aList.getLength())
        return;
    cpo::uno::Sequence<uno::Reference<xml::dom::XDocument>> aOut(aList.getLength() - 1);
    auto* pOut = aOut.getArray();
    for (sal_Int32 i = 0, j = 0; i < aList.getLength(); ++i)
    {
        if (i != nIndex)
            pOut[j++] = aList[i];
    }
    rGrabBag[rKey] <<= aOut;
}
}

void storeLabelPart(const uno::Reference<frame::XModel>& xModel, std::u16string_view rBindingXml,
                    std::u16string_view rItemPropsXml)
{
    uno::Reference<beans::XPropertySet> xModelProps(xModel, uno::UNO_QUERY);
    if (!xModelProps.is())
        return;
    uno::Reference<beans::XPropertySetInfo> xInfo = xModelProps->getPropertySetInfo();
    if (!xInfo.is() || !xInfo->hasPropertyByName(u"InteropGrabBag"_ustr))
        return;

    const uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

    comphelper::SequenceAsHashMap aGrabBag(xModelProps->getPropertyValue(u"InteropGrabBag"_ustr));

    // Replace any label already present, so re-applying or re-labeling never
    // leaves a second STANAG customXml part behind.
    const sal_Int32 nExisting = findStanagPart(aGrabBag);
    if (nExisting >= 0)
    {
        eraseDomAt(aGrabBag, u"OOXCustomXml"_ustr, nExisting);
        eraseDomAt(aGrabBag, u"OOXCustomXmlProps"_ustr, nExisting);
    }

    appendDom(aGrabBag, u"OOXCustomXml"_ustr, parseToDom(xContext, rBindingXml));
    appendDom(aGrabBag, u"OOXCustomXmlProps"_ustr, parseToDom(xContext, rItemPropsXml));
    xModelProps->setPropertyValue(u"InteropGrabBag"_ustr,
                                  cpo::uno::Any(aGrabBag.getAsConstPropertyValueList()));
}

sal_Int32 resolveColor(const OUString& rColor)
{
    if (rColor.startsWith(u"#") && rColor.getLength() == 7)
        return o3tl::toInt32(rColor.subView(1), 16);

    static const struct
    {
        const char* pName;
        sal_Int32 nRgb;
    } aW3c[] = { { "aqua", 0x00FFFF },   { "black", 0x000000 }, { "blue", 0x0000FF },
                 { "fuchsia", 0xFF00FF }, { "gray", 0x808080 },  { "green", 0x008000 },
                 { "lime", 0x00FF00 },   { "maroon", 0x800000 }, { "navy", 0x000080 },
                 { "olive", 0x808000 },  { "purple", 0x800080 }, { "red", 0xFF0000 },
                 { "silver", 0xC0C0C0 }, { "teal", 0x008080 },   { "white", 0xFFFFFF },
                 { "yellow", 0xFFFF00 } };
    for (const auto& rEntry : aW3c)
        if (rColor.equalsAscii(rEntry.pName))
            return rEntry.nRgb;
    return 0x000000;
}

void applyMarking(const uno::Reference<frame::XModel>& xModel, const OUString& rMarking,
                  sal_Int32 nColor, const OUString& rPageStyleName)
{
    uno::Reference<style::XStyleFamiliesSupplier> xSupplier(xModel, uno::UNO_QUERY);
    if (!xSupplier.is())
        return;
    uno::Reference<container::XNameAccess> xPageStyles;
    xSupplier->getStyleFamilies()->getByName(u"PageStyles"_ustr) >>= xPageStyles;
    if (!xPageStyles.is() || !xPageStyles->hasByName(rPageStyleName))
        return;
    uno::Reference<beans::XPropertySet> xPageStyle(xPageStyles->getByName(rPageStyleName),
                                                   uno::UNO_QUERY);
    if (!xPageStyle.is())
        return;

    setMarkingArea(xPageStyle, u"HeaderIsOn"_ustr, u"HeaderText"_ustr, rMarking, nColor);
    setMarkingArea(xPageStyle, u"FooterIsOn"_ustr, u"FooterText"_ustr, rMarking, nColor);
}

bool readLabel(const uno::Reference<frame::XModel>& xModel, StanagLabel& rLabel)
{
    uno::Reference<beans::XPropertySet> xProps(xModel, uno::UNO_QUERY);
    if (!xProps.is())
        return false;
    uno::Reference<beans::XPropertySetInfo> xInfo = xProps->getPropertySetInfo();
    if (!xInfo.is() || !xInfo->hasPropertyByName(u"InteropGrabBag"_ustr))
        return false;

    comphelper::SequenceAsHashMap aGrabBag(xProps->getPropertyValue(u"InteropGrabBag"_ustr));
    cpo::uno::Sequence<uno::Reference<xml::dom::XDocument>> aList;
    aGrabBag[u"OOXCustomXml"_ustr] >>= aList;
    for (const auto& xDom : aList)
    {
        if (parseLabelDom(xDom, rLabel))
            return true;
    }
    return false;
}

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
