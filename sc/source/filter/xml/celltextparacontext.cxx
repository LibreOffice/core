/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "celltextparacontext.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"

#include <com/sun/star/xml/sax/XAttributeList.hpp>

using namespace com::sun::star;

ScXMLCellTextParaContext::ScXMLCellTextParaContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLTableRowCellContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellTextParaContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
}

void ScXMLCellTextParaContext::EndElement()
{
    OUString aPara = maContent.makeStringAndClear();
    if (aPara.isEmpty())
        return;

    mrParentCxt.PushParagraph(aPara);
}

void ScXMLCellTextParaContext::Characters(const OUString& rChars)
{
    if (rChars.isEmpty())
        return;

    maContent.append(rChars);
}

SvXMLImportContext* ScXMLCellTextParaContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    const SvXMLTokenMap& rTokenMap = GetScImport().GetCellTextParaElemTokenMap();
    switch (rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_CELL_TEXT_SPAN:
            return new ScXMLCellTextSpanContext(GetScImport(), nPrefix, rLocalName, *this);
        break;
        default:
            ;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLCellTextParaContext::PushSpan(const OUString& rSpan)
{
    if (rSpan.isEmpty())
        return;

    maContent.append(rSpan);
}

ScXMLCellTextSpanContext::ScXMLCellTextSpanContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellTextSpanContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
}

void ScXMLCellTextSpanContext::EndElement()
{
    if (!maContent.isEmpty())
        mrParentCxt.PushSpan(maContent);
}

void ScXMLCellTextSpanContext::Characters(const OUString& rChars)
{
    maContent = rChars;
}

SvXMLImportContext* ScXMLCellTextSpanContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
