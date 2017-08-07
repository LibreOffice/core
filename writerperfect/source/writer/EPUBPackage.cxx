/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "EPUBPackage.hxx"

using namespace com::sun::star;

namespace writerperfect
{

EPUBPackage::EPUBPackage(const uno::Reference<uno::XComponentContext> &xContext, const uno::Sequence<beans::PropertyValue> &/*rDescriptor*/)
    : mxContext(xContext)
{
}

EPUBPackage::~EPUBPackage()
{
}

void EPUBPackage::openXMLFile(const char *pName)
{
    SAL_WARN("writerperfect", "EPUBPackage::openXMLFile, " << pName << ": implement me");
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
    SAL_WARN("writerperfect", "EPUBPackage::closeXMLFile: implement me");
}

void EPUBPackage::openCSSFile(const char *pName)
{
    SAL_WARN("writerperfect", "EPUBPackage::openCSSFile, " << pName << ": implement me");
}

void EPUBPackage::insertRule(const librevenge::RVNGString &/*rSelector*/, const librevenge::RVNGPropertyList &/*rProperties*/)
{
    SAL_WARN("writerperfect", "EPUBPackage::insertRule: implement me");
}

void EPUBPackage::closeCSSFile()
{
    SAL_WARN("writerperfect", "EPUBPackage::closeCSSFile: implement me");
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
