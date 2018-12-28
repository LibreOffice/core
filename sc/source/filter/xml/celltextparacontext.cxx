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

#include <comphelper/string.hxx>
#include <xmloff/xmlnmspe.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLCellTextParaContext::ScXMLCellTextParaContext(
    ScXMLImport& rImport, ScXMLTableRowCellContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent)
{
}

void SAL_CALL ScXMLCellTextParaContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!maContent.isEmpty())
        mrParentCxt.PushParagraphSpan(maContent, OUString());

    mrParentCxt.PushParagraphEnd();
}

void SAL_CALL ScXMLCellTextParaContext::characters( const OUString& rChars )
{
    maContent += rChars;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLCellTextParaContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushParagraphSpan(maContent, OUString());
        maContent.clear();
    }

    switch (nElement)
    {
        case XML_ELEMENT( TEXT, XML_S ):
            return new ScXMLCellFieldSContext(GetScImport(), *this);
        case XML_ELEMENT( TEXT, XML_SPAN ):
            return new ScXMLCellTextSpanContext(GetScImport(), *this);
        case XML_ELEMENT( TEXT, XML_SHEET_NAME ):
            return new ScXMLCellFieldSheetNameContext(GetScImport(), *this);
        case XML_ELEMENT( TEXT, XML_DATE ):
            return new ScXMLCellFieldDateContext(GetScImport(), *this);
        case XML_ELEMENT( TEXT, XML_TITLE ):
            return new ScXMLCellFieldTitleContext(GetScImport(), *this);
        case XML_ELEMENT( TEXT, XML_A ):
            return new ScXMLCellFieldURLContext(GetScImport(), *this);
        default:
            ;
    }

    return new SvXMLImportContext(GetImport());
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
    const OUString& rURL, const OUString& rRep, const OUString& rStyleName, const OUString& rTargetFrame)
{
    mrParentCxt.PushParagraphFieldURL(rURL, rRep, rStyleName, rTargetFrame);
}

ScXMLCellTextSpanContext::ScXMLCellTextSpanContext(
    ScXMLImport& rImport, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent)
{
}

void SAL_CALL ScXMLCellTextSpanContext::startFastElement( sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    if ( xAttrList.is() )
    {
        sax_fastparser::FastAttributeList *pAttribList =
            sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

        for (auto &aIter : *pAttribList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TEXT, XML_STYLE_NAME ):
                    maStyleName = aIter.toString();
                break;
                default:
                    ;
            }
        }
    }
}

void SAL_CALL ScXMLCellTextSpanContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushSpan(maContent, maStyleName);
    }
}

void SAL_CALL ScXMLCellTextSpanContext::characters( const OUString& rChars )
{
    maContent += rChars;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLCellTextSpanContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    if (!maContent.isEmpty())
    {
        mrParentCxt.PushSpan(maContent, maStyleName);
        maContent.clear();
    }

    switch (nElement)
    {
        case XML_ELEMENT( TEXT, XML_SHEET_NAME ):
        {
            ScXMLCellFieldSheetNameContext* p = new ScXMLCellFieldSheetNameContext(GetScImport(), mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_ELEMENT( TEXT, XML_DATE ):
        {
            ScXMLCellFieldDateContext* p = new ScXMLCellFieldDateContext(GetScImport(), mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_ELEMENT( TEXT, XML_TITLE ):
        {
            ScXMLCellFieldTitleContext* p = new ScXMLCellFieldTitleContext(GetScImport(), mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_ELEMENT( TEXT, XML_A ):
        {
            ScXMLCellFieldURLContext* p = new ScXMLCellFieldURLContext(GetScImport(), mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        case XML_ELEMENT( TEXT, XML_S ):
        {
            ScXMLCellFieldSContext* p = new ScXMLCellFieldSContext(GetScImport(), mrParentCxt);
            p->SetStyleName(maStyleName);
            return p;
        }
        default:
            ;
    }

    return new SvXMLImportContext(GetImport());
}

ScXMLCellFieldSheetNameContext::ScXMLCellFieldSheetNameContext(
    ScXMLImport& rImport, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldSheetNameContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void SAL_CALL ScXMLCellFieldSheetNameContext::startFastElement( sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    // <text:sheet-name> has no attributes (that I'm aware of).
}

void SAL_CALL ScXMLCellFieldSheetNameContext::endFastElement( sal_Int32 /*nElement*/ )
{
    mrParentCxt.PushFieldSheetName(maStyleName);
}

ScXMLCellFieldDateContext::ScXMLCellFieldDateContext(
    ScXMLImport& rImport, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldDateContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void SAL_CALL ScXMLCellFieldDateContext::endFastElement( sal_Int32 /*nElement*/ )
{
    mrParentCxt.PushFieldDate(maStyleName);
}

ScXMLCellFieldTitleContext::ScXMLCellFieldTitleContext(
    ScXMLImport& rImport, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldTitleContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void SAL_CALL ScXMLCellFieldTitleContext::endFastElement( sal_Int32 /*nElement*/ )
{
    mrParentCxt.PushFieldTitle(maStyleName);
}

ScXMLCellFieldURLContext::ScXMLCellFieldURLContext(
    ScXMLImport& rImport, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent)
{
}

void ScXMLCellFieldURLContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void SAL_CALL ScXMLCellFieldURLContext::startFastElement( sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    if ( xAttrList.is() )
    {
        sax_fastparser::FastAttributeList *pAttribList =
            sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

        for (auto &aIter : *pAttribList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( XLINK, XML_HREF ):
                    maURL = aIter.toString();
                break;
                case XML_ELEMENT( XLINK, XML_TYPE ):
                    // Ignored for now.
                break;
                case XML_ELEMENT( OFFICE, XML_TARGET_FRAME_NAME ):
                    maTargetFrame = aIter.toString();
                break;
                default:
                    ;
            }
        }
    }
}

void SAL_CALL ScXMLCellFieldURLContext::endFastElement( sal_Int32 /*nElement*/ )
{
    mrParentCxt.PushFieldURL(maURL, maRep, maStyleName, maTargetFrame);
}

void SAL_CALL ScXMLCellFieldURLContext::characters( const OUString& rChars )
{
    maRep += rChars;
}

ScXMLCellFieldSContext::ScXMLCellFieldSContext(
    ScXMLImport& rImport, ScXMLCellTextParaContext& rParent) :
    ScXMLImportContext(rImport),
    mrParentCxt(rParent),
    mnCount(1)
{
}

void ScXMLCellFieldSContext::SetStyleName(const OUString& rStyleName)
{
    maStyleName = rStyleName;
}

void SAL_CALL ScXMLCellFieldSContext::startFastElement( sal_Int32 /*nElement*/,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    if ( xAttrList.is() )
    {
        sax_fastparser::FastAttributeList *pAttribList =
            sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

        for (auto &aIter : *pAttribList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TEXT, XML_C ):
                    mnCount = aIter.toInt32();
                    if (mnCount <= 0)
                        mnCount = 1;     // worth a warning?
                break;
                default:
                    ;
            }
        }
    }
}

void SAL_CALL ScXMLCellFieldSContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (mnCount)
        PushSpaces();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLCellFieldSContext::createFastChildContext(
    sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    // <text:s> does not have child elements, but ...
    if (mnCount)
    {
        PushSpaces();
        mnCount = 0;
    }

    return new SvXMLImportContext(GetImport());
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
