/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SecLabelApply.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
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
    appendDom(aGrabBag, u"OOXCustomXml"_ustr, parseToDom(xContext, rBindingXml));
    appendDom(aGrabBag, u"OOXCustomXmlProps"_ustr, parseToDom(xContext, rItemPropsXml));
    xModelProps->setPropertyValue(u"InteropGrabBag"_ustr,
                                  cpo::uno::Any(aGrabBag.getAsConstPropertyValueList()));
}

} // namespace sw::seclabel

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
