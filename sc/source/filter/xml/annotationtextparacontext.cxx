/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "annotationtextparacontext.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#include "xmlannoi.hxx"

#include "xmloff/nmspmap.hxx"
#include "comphelper/string.hxx"

#include <com/sun/star/xml/sax/XAttributeList.hpp>

using namespace com::sun::star;

ScXMLAnnotationTextParaContext::ScXMLAnnotationTextParaContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLAnnotationTextParaContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLAnnotationTextParaContext::EndElement()
{
    if (!maContent.isEmpty())
        mrParentCxt.PushParagraphSpan(maContent, OUString());

    mrParentCxt.PushParagraphEnd();
}

void ScXMLAnnotationTextParaContext::Characters(const OUString& rChars)
{
    maContent += rChars;
}

SvXMLImportContext* ScXMLAnnotationTextParaContext::CreateChildContext(
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
            return new ScXMLAnnotationFieldSContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_SPAN:
            return new ScXMLAnnotationTextSpanContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_SHEET_NAME:
            return new ScXMLAnnotationFieldSheetNameContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_DATE:
            return new ScXMLAnnotationFieldDateContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_TITLE:
            return new ScXMLAnnotationFieldTitleContext(GetScImport(), nPrefix, rLocalName, *this);
        case XML_TOK_CELL_TEXT_URL:
            return new ScXMLAnnotationFieldURLContext(GetScImport(), nPrefix, rLocalName, *this);
        default:
            ;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

void ScXMLAnnotationTextParaContext::PushSpan(const OUString& rSpan, const OUString& rStyleName)
{
    mrParentCxt.PushParagraphSpan(rSpan, rStyleName);
}

void ScXMLAnnotationTextParaContext::PushFieldSheetName(const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldSheetName(rStyleName);
}

void ScXMLAnnotationTextParaContext::PushFieldDate(const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldDate(rStyleName);
}

void ScXMLAnnotationTextParaContext::PushFieldTitle(const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldDocTitle(rStyleName);
}

void ScXMLAnnotationTextParaContext::PushFieldURL(
    const OUString& rURL, const OUString& rRep, const OUString& rStyleName)
{
    mrParentCxt.PushParagraphFieldURL(rURL, rRep, rStyleName);
}

//-------------------------------------------------------------------------------

ScXMLAnnotationTextSpanContext::ScXMLAnnotationTextSpanContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLAnnotationTextSpanContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
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

void ScXMLAnnotationTextSpanContext::EndElement()
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushSpan(maContent, maStyleName);
    }
}

void ScXMLAnnotationTextSpanContext::Characters(const OUString& rChars)
{
    maContent += rChars;
}

SvXMLImportContext* ScXMLAnnotationTextSpanContext::CreateChildContext(
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
            ScXMLAnnotationFieldSheetNameContext* p = new ScXMLAnnotationFieldSheetNameContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_DATE:
        {
            ScXMLAnnotationFieldDateContext* p = new ScXMLAnnotationFieldDateContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_TITLE:
        {
            ScXMLAnnotationFieldTitleContext* p = new ScXMLAnnotationFieldTitleContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_URL:
        {
            ScXMLAnnotationFieldURLContext* p = new ScXMLAnnotationFieldURLContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_TOK_CELL_TEXT_SPAN_ELEM_S:
        {
            ScXMLAnnotationFieldSContext* p = new ScXMLAnnotationFieldSContext(GetScImport(), nPrefix, rLocalName, mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        default:
            ;
    }

    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

//------------------------------------------------------------------

ScXMLAnnotationFieldSheetNameContext::ScXMLAnnotationFieldSheetNameContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLAnnotationFieldSheetNameContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLAnnotationFieldSheetNameContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    // <text:sheet-name> has no attributes (that I'm aware of).
}

void ScXMLAnnotationFieldSheetNameContext::EndElement()
{
    mrParentCxt.PushFieldSheetName(maStyleName);
}

void ScXMLAnnotationFieldSheetNameContext::Characters(const OUString& /*rChars*/)
{
}

SvXMLImportContext* ScXMLAnnotationFieldSheetNameContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLAnnotationFieldDateContext::ScXMLAnnotationFieldDateContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLAnnotationFieldDateContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLAnnotationFieldDateContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLAnnotationFieldDateContext::EndElement()
{
    mrParentCxt.PushFieldDate(maStyleName);
}

void ScXMLAnnotationFieldDateContext::Characters(const OUString& /*rChars*/)
{
}

SvXMLImportContext* ScXMLAnnotationFieldDateContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLAnnotationFieldTitleContext::ScXMLAnnotationFieldTitleContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLAnnotationFieldTitleContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLAnnotationFieldTitleContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
}

void ScXMLAnnotationFieldTitleContext::EndElement()
{
    mrParentCxt.PushFieldTitle(maStyleName);
}

void ScXMLAnnotationFieldTitleContext::Characters(const OUString& /*rChars*/)
{
}

SvXMLImportContext* ScXMLAnnotationFieldTitleContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLAnnotationFieldURLContext::ScXMLAnnotationFieldURLContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent)
{
}

void ScXMLAnnotationFieldURLContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLAnnotationFieldURLContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
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

void ScXMLAnnotationFieldURLContext::EndElement()
{
    mrParentCxt.PushFieldURL(maURL, maRep, maStyleName);
}

void ScXMLAnnotationFieldURLContext::Characters(const OUString& rChars)
{
    maRep += rChars;
}

SvXMLImportContext* ScXMLAnnotationFieldURLContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& /*xAttrList*/)
{
    return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}

ScXMLAnnotationFieldSContext::ScXMLAnnotationFieldSContext(
    ScXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLName, ScXMLAnnotationTextParaContext& rParent) :
    ScXMLImportContext(rImport, nPrefix, rLName),
    mrParentCxt(rParent),
    mnCount(1)
{
}

void ScXMLAnnotationFieldSContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void ScXMLAnnotationFieldSContext::StartElement(const uno::Reference<xml::sax::XAttributeList>& xAttrList)
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

void ScXMLAnnotationFieldSContext::EndElement()
{
    if (mnCount)
        PushSpaces();
}

SvXMLImportContext* ScXMLAnnotationFieldSContext::CreateChildContext(
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

void ScXMLAnnotationFieldSContext::PushSpaces()
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
