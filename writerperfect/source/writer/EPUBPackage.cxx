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
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <comphelper/storagehelper.hxx>
#include <unotools/mediadescriptor.hxx>

using namespace com::sun::star;

namespace writerperfect
{

EPUBPackage::EPUBPackage(const uno::Reference<uno::XComponentContext> &xContext, const uno::Sequence<beans::PropertyValue> &rDescriptor)
    : mxContext(xContext)
{
    // Extract the output stream from the descriptor.
    utl::MediaDescriptor aMediaDesc(rDescriptor);
    auto xStream = aMediaDesc.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_STREAMFOROUTPUT(), uno::Reference<io::XStream>());
    const sal_Int32 nOpenMode = embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE;
    mxStorage.set(comphelper::OStorageHelper::GetStorageOfFormatFromStream(ZIP_STORAGE_FORMAT_STRING, xStream, nOpenMode, mxContext), uno::UNO_QUERY);

    // The zipped content represents an EPUB Publication.
    mxOutputStream.set(mxStorage->openStreamElementByHierarchicalName("mimetype", embed::ElementModes::READWRITE), uno::UNO_QUERY);
    const OString aMimeType("application/epub+zip");
    uno::Sequence<sal_Int8> aData(reinterpret_cast<const sal_Int8 *>(aMimeType.getStr()), aMimeType.getLength());
    mxOutputStream->writeBytes(aData);
    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();
    mxOutputStream.clear();
}

EPUBPackage::~EPUBPackage()
{
    uno::Reference<embed::XTransactedObject> xTransactedObject(mxStorage, uno::UNO_QUERY);
    xTransactedObject->commit();
}

void EPUBPackage::openXMLFile(const char *pName)
{
    assert(pName);
    assert(!mxOutputStream.is());

    mxOutputStream.set(mxStorage->openStreamElementByHierarchicalName(OUString::fromUtf8(pName), embed::ElementModes::READWRITE), uno::UNO_QUERY);
}

void EPUBPackage::openElement(const char *pName, const librevenge::RVNGPropertyList &/*rAttributes*/)
{
    SAL_WARN("writerperfect", "EPUBPackage::openElement, " << pName << ": implement me");
}

void EPUBPackage::closeElement(const char *pName)
{
    SAL_WARN("writerperfect", "EPUBPackage::closeElement, " << pName << ": implement me");
}

void EPUBPackage::insertCharacters(const librevenge::RVNGString &rCharacters)
{
    SAL_WARN("writerperfect", "EPUBPackage::insertCharacters, " << rCharacters.cstr() << ": implement me");
}

void EPUBPackage::closeXMLFile()
{
    assert(mxOutputStream.is());

    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();
    mxOutputStream.clear();
}

void EPUBPackage::openCSSFile(const char *pName)
{
    assert(pName);
    assert(!mxOutputStream.is());

    mxOutputStream.set(mxStorage->openStreamElementByHierarchicalName(OUString::fromUtf8(pName), embed::ElementModes::READWRITE), uno::UNO_QUERY);
}

void EPUBPackage::insertRule(const librevenge::RVNGString &/*rSelector*/, const librevenge::RVNGPropertyList &/*rProperties*/)
{
    SAL_WARN("writerperfect", "EPUBPackage::insertRule: implement me");
}

void EPUBPackage::closeCSSFile()
{
    assert(mxOutputStream.is());

    uno::Reference<embed::XTransactedObject> xTransactedObject(mxOutputStream, uno::UNO_QUERY);
    xTransactedObject->commit();
    mxOutputStream.clear();
}

void EPUBPackage::openBinaryFile(const char *pName)
{
    SAL_WARN("writerperfect", "EPUBPackage::openBinaryFile, " << pName << ": implement me");
}

void EPUBPackage::insertBinaryData(const librevenge::RVNGBinaryData &/*rData*/)
{
    SAL_WARN("writerperfect", "EPUBPackage::insertBinaryData: implement me");
}

void EPUBPackage::closeBinaryFile()
{
    SAL_WARN("writerperfect", "EPUBPackage::closeBinaryFile: implement me");
}

void EPUBPackage::openTextFile(const char *pName)
{
    SAL_WARN("writerperfect", "EPUBPackage::openTextFile, " << pName << ": implement me");
}

void EPUBPackage::insertText(const librevenge::RVNGString &/*rCharacters*/)
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
