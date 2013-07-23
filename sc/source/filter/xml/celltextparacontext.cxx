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
#include "comphelper/string.hxx"

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
    maContent += rChars;
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
        case XML_TOK_CELL_TEXT_S:
            return new ScXMLCellFieldSContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_SPAN:
            return new ScXMLCellTextSpanContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_SHEET_NAME:
            return new ScXMLCellFieldSheetNameContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_DATE:
            return new ScXMLCellFieldDateContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_TITLE:
            return new ScXMLCellFieldTitleContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_URL:
            return new ScXMLCellFieldURLContext(GetScImport(), nPrefix, rLocalName, *this);
        default:
            ;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLCellTextParaContext::PushSpan(const OUString& rSpan, const OUString& rStyleName)
{
    mrParentCxt.PushParagraphSpan(rSpan, rStyleName);
}

void ScXMLCellTextParaContext::PushFieldSheetName(const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldSheetName(rStyleName);
}

void ScXMLCellTextParaContext::PushFieldDate(const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldDate(rStyleName);
}

void ScXMLCellTextParaContext::PushFieldTitle(const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldDocTitle(rStyleName);
}

void ScXMLCellTextParaContext::PushFieldURL(
    const OUString& rURL, const OUString& rRep, const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldURL(rURL, rRep, rStyleName);
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
    maContent += rChars;
}

SvXMLImportContext* ScXMLCellTextSpanContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushSpan(maContent, maStyleName);
        maContent = OUString();
    }

    const SvXMLTokenMap& rTokenMap = GetScImport().GetCellTextSpanElemTokenMap();
    switch (rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_CELL_TEXT_SPAN_ELEM_SHEET_NAME:
        {
            ScXMLCellFieldSheetNameContext* p = new ScXMLCellFieldSheetNameContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_DATE:
        {
            ScXMLCellFieldDateContext* p = new ScXMLCellFieldDateContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_TITLE:
        {
            ScXMLCellFieldTitleContext* p = new ScXMLCellFieldTitleContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_URL:
        {
            ScXMLCellFieldURLContext* p = new ScXMLCellFieldURLContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_S:
        {
            ScXMLCellFieldSContext* p = new ScXMLCellFieldSContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        default:
            ;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLCellFieldSheetNameContext::ScXMLCellFieldSheetNameContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldSheetNameContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLCellFieldSheetNameContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    // <text:sheet-name> has no attributes (that I'm aware of).
}

void ScXMLCellFieldSheetNameContext::EndElement()
{
    mrParentCxt.PushFieldSheetName(maStyleName);
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

void ScXMLCellFieldDateContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLCellFieldDateContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLCellFieldDateContext::EndElement()
{
    mrParentCxt.PushFieldDate(maStyleName);
}

void ScXMLCellFieldDateContext::Characters(const OUString& /*rChars*/)
{
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

void ScXMLCellFieldTitleContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLCellFieldTitleContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLCellFieldTitleContext::EndElement()
{
    mrParentCxt.PushFieldTitle(maStyleName);
}

void ScXMLCellFieldTitleContext::Characters(const OUString& /*rChars*/)
{
}

SvXMLImportContext* ScXMLCellFieldTitleContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLCellFieldURLContext::ScXMLCellFieldURLContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldURLContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLCellFieldURLContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if (!xAttrList.is())
        return;

    OUString aLocalName;
    sal_Int16 nAttrCount = xAttrList->getLength();

    const SvXMLTokenMap& rTokenMap = GetScImport().GetCellTextURLAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
            xAttrList->getNameByIndex(i), &aLocalName);

        const OUString& rAttrValue = xAttrList->getValueByIndex(i);
        sal_uInt16 nToken = rTokenMap.Get(nAttrPrefix, aLocalName);
        switch (nToken)
        {
            case XML_TOK_CELL_TEXT_URL_ATTR_UREF:
                maURL = rAttrValue;
            break;
            case XML_TOK_CELL_TEXT_URL_ATTR_TYPE:
                // Ignored for now.
            break;
            default:
                ;
        }
    }
}

void ScXMLCellFieldURLContext::EndElement()
{
    mrParentCxt.PushFieldURL(maURL, maRep, maStyleName);
}

void ScXMLCellFieldURLContext::Characters(const OUString& rChars)
{
    maRep += rChars;
}

SvXMLImportContext* ScXMLCellFieldURLContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLCellFieldSContext::ScXMLCellFieldSContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent),
    mnCount(1)
{
}

void ScXMLCellFieldSContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLCellFieldSContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
{
    if (!xAttrList.is())
        return;

    OUString aLocalName;
    sal_Int16 nAttrCount = xAttrList->getLength();

    const SvXMLTokenMap& rTokenMap = GetScImport().GetCellTextSAttrTokenMap();
    for (sal_Int16 i = 0; i < nAttrCount; ++i)
    {
        sal_uInt16 nAttrPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
            xAttrList->getNameByIndex(i), &aLocalName);

        const OUString& rAttrValue = xAttrList->getValueByIndex(i);
        sal_uInt16 nToken = rTokenMap.Get(nAttrPrefix, aLocalName);
        switch (nToken)
        {
            case XML_TOK_CELL_TEXT_S_ATTR_C:
                mnCount = rAttrValue.toInt32();
                if (mnCount <= 0)
                    mnCount = 1;     // worth a warning?
            break;
            default:
                ;
        }
    }
}

void ScXMLCellFieldSContext::EndElement()
{
    if (mnCount)
        PushSpaces();
}

SvXMLImportContext* ScXMLCellFieldSContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    // <text:s> does not have child elements, but ...
    if (mnCount)
    {
        PushSpaces();
        mnCount = 0;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLCellFieldSContext::PushSpaces()
{
    if (mnCount > 0)
    {
        if (mnCount == 1)
            mrParentCxt.PushSpan(" ", maStyleName);
        else
        {
            OUStringBuffer aBuf( mnCount);
            comphelper::string::padToLength( aBuf, mnCount, ' ');
            mrParentCxt.PushSpan( aBuf.makeStringAndClear(), maStyleName);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
