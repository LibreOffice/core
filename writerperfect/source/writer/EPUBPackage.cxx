/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBPackage.hxx"

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>

#include <sal/log.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <xmloff/attrlist.hxx>

using namespace com::sun::star;

namespace writerperfect
{
EPUBPackage::EPUBPackage(uno::Reference<uno::XComponentContext> xContext,
                         const uno::Sequence<beans::PropertyValue>& rDescriptor)
    : mxContext(std::move(xContext))
{
    // Extract the output stream from the descriptor.
    utl::MediaDescriptor aMediaDesc(rDescriptor);
    auto xStream = aMediaDesc.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_STREAMFOROUTPUT(), uno::Reference<io::XStream>());
    const sal_Int32 nOpenMode = embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE;
    mxStorage.set(comphelper::OStorageHelper::GetStorageOfFormatFromStream(
                      ZIP_STORAGE_FORMAT_STRING, xStream, nOpenMode, mxContext),
                  uno::UNO_QUERY);

    // The zipped content represents an EPUB Publication.
    mxOutputStream.set(
        mxStorage->openStreamElementByHierarchicalName("mimetype", embed::ElementModes::READWRITE),
        uno::UNO_QUERY);
    const OString aMimeType("application/epub+zip");
    uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8*>(aMimeType.getStr()),
                                  aMimeType.getLength());
    mxOutputStream->writeBytes(aData);
    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();

    // MIME type must be uncompressed.
    uno::Reference<beans::XPropertySet> xPropertySet(mxOutputStream, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("Compressed", uno::makeAny(false));
    mxOutputStream.clear();
}

EPUBPackage::~EPUBPackage()
{
    uno::Reference<embed::XTransactedObject> xTransactedObject(mxStorage, uno::UNO_QUERY);
    xTransactedObject->commit();
}

void EPUBPackage::openXMLFile(const char* pName)
{
    assert(pName);
    assert(!mxOutputStream.is());
    assert(!mxOutputWriter.is());

    mxOutputStream.set(mxStorage->openStreamElementByHierarchicalName(
                           OUString::fromUtf8(pName), embed::ElementModes::READWRITE),
                       uno::UNO_QUERY);
    mxOutputWriter = xml::sax::Writer::create(mxContext);
    mxOutputWriter->setOutputStream(mxOutputStream);
    mxOutputWriter->startDocument();
}

void EPUBPackage::openElement(const char* pName, const librevenge::RVNGPropertyList& rAttributes)
{
    assert(mxOutputWriter.is());

    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());

    librevenge::RVNGPropertyList::Iter it(rAttributes);
    for (it.rewind(); it.next();)
        pAttributeList->AddAttribute(OUString::fromUtf8(it.key()),
                                     OUString::fromUtf8(it()->getStr().cstr()));

    mxOutputWriter->startElement(OUString::fromUtf8(pName), pAttributeList);
}

void EPUBPackage::closeElement(const char* pName)
{
    assert(mxOutputWriter.is());

    mxOutputWriter->endElement(OUString::fromUtf8(pName));
}

void EPUBPackage::insertCharacters(const librevenge::RVNGString& rCharacters)
{
    mxOutputWriter->characters(OUString::fromUtf8(rCharacters.cstr()));
}

void EPUBPackage::closeXMLFile()
{
    assert(mxOutputWriter.is());
    assert(mxOutputStream.is());

    mxOutputWriter->endDocument();
    mxOutputWriter.clear();

    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();
    mxOutputStream.clear();
}

void EPUBPackage::openCSSFile(const char* pName)
{
    assert(pName);
    assert(!mxOutputStream.is());

    mxOutputStream.set(mxStorage->openStreamElementByHierarchicalName(
                           OUString::fromUtf8(pName), embed::ElementModes::READWRITE),
                       uno::UNO_QUERY);
}

void EPUBPackage::insertRule(const librevenge::RVNGString& rSelector,
                             const librevenge::RVNGPropertyList& rProperties)
{
    assert(mxOutputStream.is());

    uno::Reference<io::XSeekable> xSeekable(mxOutputStream, uno::UNO_QUERY);
    std::stringstream aStream;
    if (xSeekable->getPosition() != 0)
        aStream << '\n';
    aStream << rSelector.cstr() << " {\n";

    librevenge::RVNGPropertyList::Iter it(rProperties);
    for (it.rewind(); it.next();)
    {
        if (it())
            aStream << "  " << it.key() << ": " << it()->getStr().cstr() << ";\n";
    }

    aStream << "}\n";
    std::string aString = aStream.str();
    uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8*>(aString.c_str()),
                                  aString.size());
    mxOutputStream->writeBytes(aData);
}

void EPUBPackage::closeCSSFile()
{
    assert(mxOutputStream.is());

    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();
    mxOutputStream.clear();
}

void EPUBPackage::openBinaryFile(const char* pName)
{
    assert(pName);
    assert(!mxOutputStream.is());

    mxOutputStream.set(mxStorage->openStreamElementByHierarchicalName(
                           OUString::fromUtf8(pName), embed::ElementModes::READWRITE),
                       uno::UNO_QUERY);
}

void EPUBPackage::insertBinaryData(const librevenge::RVNGBinaryData& rData)
{
    assert(mxOutputStream.is());

    if (rData.empty())
        return;

    uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8*>(rData.getDataBuffer()),
                                  rData.size());
    mxOutputStream->writeBytes(aData);
}

void EPUBPackage::closeBinaryFile()
{
    assert(mxOutputStream.is());

    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();
    mxOutputStream.clear();
}

void EPUBPackage::openTextFile(const char* pName)
{
    SAL_WARN("writerperfect", "EPUBPackage::openTextFile, " << pName << ": implement me");
}

void EPUBPackage::insertText(const librevenge::RVNGString& /*rCharacters*/)
{
    SAL_WARN("writerperfect", "EPUBPackage::insertText: implement me");
}

void EPUBPackage::insertLineBreak()
{
    SAL_WARN("writerperfect", "EPUBPackage::insertLineBreak: implement me");
}

void EPUBPackage::closeTextFile()
{
    SAL_WARN("writerperfect", "EPUBPackage::closeTextFile: implement me");
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
