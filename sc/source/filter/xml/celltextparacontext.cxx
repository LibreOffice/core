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

#include "xmloff/nmspmap.hxx"

#include <com/sun/star/xml/sax/XAttributeList.hpp>

using namespace com::sun::star;

ScXMLCellTextParaContext::ScXMLCellTextParaContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLTableRowCellContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellTextParaContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLCellTextParaContext::EndElement()
{
    if (!maContent.isEmpty())
        mrParentCxt.PushParagraphSpan(maContent, OUString());

    mrParentCxt.PushParagraphEnd();
}

void ScXMLCellTextParaContext::Characters(const OUString& rChars)
{
    maContent = rChars;
}

SvXMLImportContext* ScXMLCellTextParaContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushParagraphSpan(maContent, OUString());
        maContent = OUString();
    }

    const SvXMLTokenMap& rTokenMap = GetScImport().GetCellTextParaElemTokenMap();
    switch (rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_CELL_TEXT_SPAN:
            return new ScXMLCellTextSpanContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_SHEET_NAME:
            return new ScXMLCellFieldSheetNameContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_DATE:
            return new ScXMLCellFieldDateContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_TITLE:
            return new ScXMLCellFieldTitleContext(GetScImport(), nPrefix, rLocalName, *this);
        default:
            ;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLCellTextParaContext::PushSpan(const OUString& rSpan, const OUString& rStyleName)
{
    mrParentCxt.PushParagraphSpan(rSpan, rStyleName);
}

void ScXMLCellTextParaContext::PushFieldSheetName()
{
    mrParentCxt.PushParagraphFieldSheetName();
}

void ScXMLCellTextParaContext::PushFieldDate(const OUString& rOutput)
{
}

void ScXMLCellTextParaContext::PushFieldTitle()
{
    mrParentCxt.PushParagraphFieldDocTitle();
}

ScXMLCellTextSpanContext::ScXMLCellTextSpanContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellTextSpanContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if (!xAttrList.is())
        return;

    OUString aLocalName;
    sal_Int16 nAttrCount = xAttrList->getLength();

    const SvXMLTokenMap& rTokenMap = GetScImport().GetCellTextSpanAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
            xAttrList->getNameByIndex(i), &aLocalName);

        const OUString& rAttrValue = xAttrList->getValueByIndex(i);
        sal_uInt16 nToken = rTokenMap.Get(nAttrPrefix, aLocalName);
        switch (nToken)
        {
            case XML_TOK_CELL_TEXT_SPAN_ATTR_STYLE_NAME:
                maStyleName = rAttrValue;
            break;
            default:
                ;
        }
    }
}

void ScXMLCellTextSpanContext::EndElement()
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushSpan(maContent, maStyleName);
    }
}

void ScXMLCellTextSpanContext::Characters(const OUString& rChars)
{
    maContent = rChars;
}

SvXMLImportContext* ScXMLCellTextSpanContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLCellFieldSheetNameContext::ScXMLCellFieldSheetNameContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldSheetNameContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    // <text:sheet-name> has no attributes (that I'm aware of).
}

void ScXMLCellFieldSheetNameContext::EndElement()
{
    mrParentCxt.PushFieldSheetName();
}

void ScXMLCellFieldSheetNameContext::Characters(const OUString& /*rChars*/)
{
}

SvXMLImportContext* ScXMLCellFieldSheetNameContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLCellFieldDateContext::ScXMLCellFieldDateContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldDateContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLCellFieldDateContext::EndElement()
{
}

void ScXMLCellFieldDateContext::Characters(const OUString& rChars)
{
    maDate = rChars;
}

SvXMLImportContext* ScXMLCellFieldDateContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLCellFieldTitleContext::ScXMLCellFieldTitleContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldTitleContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLCellFieldTitleContext::EndElement()
{
    mrParentCxt.PushFieldTitle();
}

void ScXMLCellFieldTitleContext::Characters(const OUString& rChars)
{
    maTitle = rChars;
}

SvXMLImportContext* ScXMLCellFieldTitleContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
