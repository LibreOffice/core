/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/seclabel/SecLabelStore.hxx>
#include <svx/seclabel/SpifPolicy.hxx>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XWriter.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <o3tl/string_view.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>

using namespace css;

namespace svx::seclabel
{
namespace
{
uno::Reference<xml::dom::XDocument>
parseToDom(const uno::Reference<cpo::uno::XComponentContext>& xContext, std::u16string_view rXml)
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

// Storing or removing the label is a document change; mark it modified so the user is
// prompted to save. The grab-bag write alone does not reliably dirty the document, and
// a no-op marking placement would otherwise leave it clean.
void markModified(const uno::Reference<frame::XModel>& xModel)
{
    if (uno::Reference<util::XModifiable> xModifiable{ xModel, uno::UNO_QUERY })
        xModifiable->setModified(true);
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

    const uno::Reference<cpo::uno::XComponentContext> xContext
        = comphelper::getProcessComponentContext();

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
    markModified(xModel);
}

void removeLabelPart(const uno::Reference<frame::XModel>& xModel)
{
    uno::Reference<beans::XPropertySet> xModelProps(xModel, uno::UNO_QUERY);
    if (!xModelProps.is())
        return;
    uno::Reference<beans::XPropertySetInfo> xInfo = xModelProps->getPropertySetInfo();
    if (!xInfo.is() || !xInfo->hasPropertyByName(u"InteropGrabBag"_ustr))
        return;

    comphelper::SequenceAsHashMap aGrabBag(xModelProps->getPropertyValue(u"InteropGrabBag"_ustr));
    const sal_Int32 nIndex = findStanagPart(aGrabBag);
    if (nIndex < 0)
        return;
    eraseDomAt(aGrabBag, u"OOXCustomXml"_ustr, nIndex);
    eraseDomAt(aGrabBag, u"OOXCustomXmlProps"_ustr, nIndex);
    xModelProps->setPropertyValue(u"InteropGrabBag"_ustr,
                                  cpo::uno::Any(aGrabBag.getAsConstPropertyValueList()));
    markModified(xModel);
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

OUString readMarking(const uno::Reference<frame::XModel>& xModel)
{
    StanagLabel aLabel;
    if (!readLabel(xModel, aLabel))
        return OUString();

    // Render with the policy the label was created under (matched by OID) -- the
    // authoritative source. When it is not provisioned this session, fall back to the
    // marking cached in the 4778 binding at apply time, and only if that is absent to
    // the label's self-describing summary.
    SpifPolicySet aPolicies;
    aPolicies.loadProvisioned();
    if (const SpifPolicy* pPolicy = aPolicies.findByLabel(aLabel))
        return pPolicy->deriveMarking(aLabel);
    if (!aLabel.aMarking.isEmpty())
        return aLabel.aMarking;
    return aLabel.summary();
}

bool modelSupportsLabel(const uno::Reference<frame::XModel>& xModel)
{
    // The loading filter is authoritative for the actual format (the file extension
    // may not match). OOXML filters come in two name families -- "... 2007 ... XML"
    // and "... Office Open XML ..." -- and the modifiers (Template, VBA, AutoPlay)
    // land on either side, so match the markers rather than one fixed phrase:
    // "Calc MS Excel 2007 VBA XML" (xlsm) is why. Every other format (ODF, RTF, and
    // the binary MSO ones including "Calc MS Excel 2007 Binary") lacks a marker.
    SfxObjectShell* pShell = SfxObjectShell::GetShellFromComponent(xModel);
    if (!pShell || !pShell->GetMedium() || !pShell->GetMedium()->GetFilter())
        return false;
    const OUString aName = pShell->GetMedium()->GetFilter()->GetFilterName();
    return (aName.indexOf(u"2007") >= 0 && aName.indexOf(u"XML") >= 0)
           || aName.indexOf(u"Office Open XML") >= 0;
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

} // namespace svx::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
