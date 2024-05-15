/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "XMLTextMarkImportContext.hxx"


#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/odffields.hxx>
#include <xmloff/xmlement.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>

#include <com/sun/star/text/XFormField.hpp>
#include <comphelper/diagnose_ex.hxx>

#include <RDFaImportHelper.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;


XMLFieldParamImportContext::XMLFieldParamImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp ) :
        SvXMLImportContext(rImport),
        rHelper(rHlp)
{
}


void XMLFieldParamImportContext::startFastElement(sal_Int32 /*nElement*/, const css::uno::Reference< css::xml::sax::XFastAttributeList> & xAttrList)
{
    OUString sName;
    OUString sValue;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(FIELD, XML_NAME):
                sName = aIter.toString();
                break;
            case XML_ELEMENT(FIELD, XML_VALUE):
                sValue = aIter.toString();
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    if (rHelper.hasCurrentFieldCtx() && !sName.isEmpty()) {
        rHelper.addFieldParam(sName, sValue);
    }
}


XMLTextMarkImportContext::XMLTextMarkImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    uno::Reference<uno::XInterface> & io_rxCrossRefHeadingBookmark )
    : SvXMLImportContext(rImport)
    , m_rHelper(rHlp)
    , m_rxCrossRefHeadingBookmark(io_rxCrossRefHeadingBookmark)
    , m_isHidden(false)
    , m_bHaveAbout(false)
{
}

namespace {

enum lcl_MarkType { TypeReference, TypeReferenceStart, TypeReferenceEnd,
                    TypeBookmark, TypeBookmarkStart, TypeBookmarkEnd,
                    TypeFieldmark, TypeFieldmarkStart, TypeFieldmarkSeparator, TypeFieldmarkEnd
                  };

}

SvXMLEnumMapEntry<lcl_MarkType> const lcl_aMarkTypeMap[] =
{
    { XML_REFERENCE_MARK,         TypeReference },
    { XML_REFERENCE_MARK_START,   TypeReferenceStart },
    { XML_REFERENCE_MARK_END,     TypeReferenceEnd },
    { XML_BOOKMARK,               TypeBookmark },
    { XML_BOOKMARK_START,         TypeBookmarkStart },
    { XML_BOOKMARK_END,           TypeBookmarkEnd },
    { XML_FIELDMARK,              TypeFieldmark },
    { XML_FIELDMARK_START,        TypeFieldmarkStart },
    { XML_FIELDMARK_SEPARATOR,    TypeFieldmarkSeparator },
    { XML_FIELDMARK_END,          TypeFieldmarkEnd },
    { XML_TOKEN_INVALID,          lcl_MarkType(0) },
};


static OUString lcl_getFormFieldmarkName(std::u16string_view name)
{
    if (name == ODF_FORMCHECKBOX ||
        name == u"msoffice.field.FORMCHECKBOX" ||
        name == u"ecma.office-open-xml.field.FORMCHECKBOX")
        return ODF_FORMCHECKBOX;
    else if (name == ODF_FORMDROPDOWN ||
             name == u"ecma.office-open-xml.field.FORMDROPDOWN")
        return ODF_FORMDROPDOWN;
    else
        return OUString();
}

static OUString lcl_getFieldmarkName(OUString const& name)
{
    if (name == "msoffice.field.FORMTEXT" ||
        name == "ecma.office-open-xml.field.FORMTEXT")
        return ODF_FORMTEXT;
    else
        return name;
}


void XMLTextMarkImportContext::startFastElement( sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if (!FindName(xAttrList))
    {
        m_sBookmarkName.clear();
    }

    if ((nElement & TOKEN_MASK) == XML_FIELDMARK_START ||
        (nElement & TOKEN_MASK) == XML_FIELDMARK)
    {
        if (m_sBookmarkName.isEmpty())
        {
            m_sBookmarkName = "Unknown";
        }
        m_rHelper.pushFieldCtx( m_sBookmarkName, m_sFieldName );
    }

    if ((nElement & TOKEN_MASK) == XML_BOOKMARK_START)
    {
        m_rHelper.setBookmarkAttributes(m_sBookmarkName, m_isHidden, m_sCondition);
    }
}

static auto InsertFieldmark(SvXMLImport & rImport,
        XMLTextImportHelper & rHelper, bool const isFieldmarkSeparatorMissing) -> void
{
    assert(rHelper.hasCurrentFieldCtx()); // was set up in StartElement()

    // fdo#86795 check if it's actually a checkbox first
    auto const [ name, type ] = rHelper.getCurrentFieldType();
    OUString const fieldmarkTypeName = lcl_getFieldmarkName(type);
    if (fieldmarkTypeName == ODF_FORMCHECKBOX ||
        fieldmarkTypeName == ODF_FORMDROPDOWN)
    {   // sw can't handle checkbox with start+end
        SAL_INFO("xmloff.text", "invalid fieldmark-start/fieldmark-end ignored");
        return;
    }

    uno::Reference<text::XTextRange> const xStartRange(rHelper.getCurrentFieldStart());
    uno::Reference<text::XTextCursor> const xCursor(
        rHelper.GetText()->createTextCursorByRange(xStartRange));
    uno::Reference<text::XTextRangeCompare> const xCompare(rHelper.GetText(), uno::UNO_QUERY);
    if (xCompare->compareRegionStarts(xStartRange, rHelper.GetCursorAsRange()) < 0)
    {
        SAL_WARN("xmloff.text", "invalid field mark positions");
        assert(false);
    }
    xCursor->gotoRange(rHelper.GetCursorAsRange(), true);

    Reference<XTextContent> const xContent = XMLTextMarkImportContext::CreateAndInsertMark(
            rImport, u"com.sun.star.text.Fieldmark"_ustr, name, xCursor,
            OUString(), isFieldmarkSeparatorMissing);

    if (!xContent.is())
        return;

    // setup fieldmark...
    Reference<text::XFormField> const xFormField(xContent, UNO_QUERY);
    assert(xFormField.is());
    try {
        xFormField->setFieldType(fieldmarkTypeName);
    } catch (uno::RuntimeException const&) {
        // tdf#140437 somehow old documents had the field code in the type
        // attribute instead of field:param
        SAL_INFO("xmloff.text", "invalid fieldmark type, converting to param");
        // add without checking: FieldParamImporter::Import() catches ElementExistException
        rHelper.addFieldParam(ODF_CODE_PARAM, fieldmarkTypeName);
        xFormField->setFieldType(ODF_UNHANDLED);
    }
    rHelper.setCurrentFieldParamsTo(xFormField);
    // move cursor after setFieldType as that may delete/re-insert
    rHelper.GetCursor()->gotoRange(xContent->getAnchor()->getEnd(), false);
    rHelper.GetCursor()->goLeft(1, false); // move before CH_TXT_ATR_FIELDEND
    // tdf#129520: AppendTextNode() ignores the content index!
    // plan B: insert a spurious paragraph break now and join
    //         it in PopFieldmark()!
    rHelper.GetText()->insertControlCharacter(rHelper.GetCursor(),
            text::ControlCharacter::PARAGRAPH_BREAK, false);
    rHelper.GetCursor()->goLeft(1, false); // back to previous paragraph
}

static auto PopFieldmark(XMLTextImportHelper & rHelper) -> void
{
    // can't verify name because it's not written as an attribute...
    uno::Reference<text::XTextContent> const xField(rHelper.popFieldCtx(),
            uno::UNO_QUERY);
    if (!xField.is())
        return;

    if (rHelper.GetText() == xField->getAnchor()->getText())
    {
        try
        {   // skip CH_TXT_ATR_FIELDEND
            rHelper.GetCursor()->goRight(1, true);
            rHelper.GetCursor()->setString(OUString()); // undo AppendTextNode from InsertFieldmark
            rHelper.GetCursor()->gotoRange(xField->getAnchor()->getEnd(), false);
        }
        catch (uno::Exception const&)
        {
            assert(false); // must succeed
        }
    }
    else
    {
        SAL_INFO("xmloff.text", "fieldmark has invalid positions");
        // could either dispose it or leave it to end at the end of the document?
        xField->dispose();
    }
}

void XMLTextMarkImportContext::endFastElement(sal_Int32 nElement)
{
    static constexpr OUString sAPI_bookmark = u"com.sun.star.text.Bookmark"_ustr;

    lcl_MarkType nTmp{};
    if (!SvXMLUnitConverter::convertEnum(nTmp, SvXMLImport::getNameFromToken(nElement), lcl_aMarkTypeMap))
        return;

    if (m_sBookmarkName.isEmpty() && TypeFieldmarkEnd != nTmp && TypeFieldmarkSeparator != nTmp)
        return;

    switch (nTmp)
    {
        case TypeReference:
            // export point reference mark
            CreateAndInsertMark(GetImport(),
                u"com.sun.star.text.ReferenceMark"_ustr,
                m_sBookmarkName,
                m_rHelper.GetCursorAsRange()->getStart());
            break;

        case TypeBookmark:
            {
                // tdf#94804: detect duplicate heading cross reference bookmarks
                if (m_sBookmarkName.startsWith("__RefHeading__"))
                {
                    if (m_rxCrossRefHeadingBookmark.is())
                    {
                        uno::Reference<container::XNamed> const xNamed(
                            m_rxCrossRefHeadingBookmark, uno::UNO_QUERY);
                        m_rHelper.AddCrossRefHeadingMapping(
                            m_sBookmarkName, xNamed->getName());
                        break; // don't insert
                    }
                }
            }
            [[fallthrough]];
        case TypeFieldmark:
            {
                const OUString formFieldmarkName=lcl_getFormFieldmarkName(m_sFieldName);
                bool bImportAsField = (nTmp==TypeFieldmark && !formFieldmarkName.isEmpty()); //@TODO handle abbreviation cases...
                // export point bookmark
                const Reference<XInterface> xContent(
                    CreateAndInsertMark(GetImport(),
                                (bImportAsField ? u"com.sun.star.text.FormFieldmark"_ustr : sAPI_bookmark),
                        m_sBookmarkName,
                        m_rHelper.GetCursorAsRange()->getStart(),
                        m_sXmlId) );
                if (nTmp==TypeFieldmark) {
                    if (xContent.is() && bImportAsField) {
                        // setup fieldmark...
                        Reference< css::text::XFormField> xFormField(xContent, UNO_QUERY);
                        xFormField->setFieldType(formFieldmarkName);
                        if (xFormField.is() && m_rHelper.hasCurrentFieldCtx()) {
                            m_rHelper.setCurrentFieldParamsTo(xFormField);
                        }
                    }
                    m_rHelper.popFieldCtx();
                }
                if (TypeBookmark == nTmp
                    && m_sBookmarkName.startsWith("__RefHeading__"))
                {
                    assert(xContent.is());
                    m_rxCrossRefHeadingBookmark = xContent;
                }
            }
            break;

        case TypeBookmarkStart:
            // save XTextRange for later construction of bookmark
            {
                std::shared_ptr< ::xmloff::ParsedRDFaAttributes >
                    xRDFaAttributes;
                if (m_bHaveAbout && TypeBookmarkStart == nTmp)
                {
                    xRDFaAttributes =
                        GetImport().GetRDFaImportHelper().ParseRDFa(
                            m_sAbout, m_sProperty,
                            m_sContent, m_sDatatype);
                }
                m_rHelper.InsertBookmarkStartRange(
                    m_sBookmarkName,
                    m_rHelper.GetCursorAsRange()->getStart(),
                    m_sXmlId, xRDFaAttributes);
            }
            break;

        case TypeBookmarkEnd:
        {
            // tdf#94804: detect duplicate heading cross reference bookmarks
            if (m_sBookmarkName.startsWith("__RefHeading__"))
            {
                if (m_rxCrossRefHeadingBookmark.is())
                {
                    uno::Reference<container::XNamed> const xNamed(
                        m_rxCrossRefHeadingBookmark, uno::UNO_QUERY);
                    m_rHelper.AddCrossRefHeadingMapping(
                        m_sBookmarkName, xNamed->getName());
                    break; // don't insert
                }
            }

            // get old range, and construct
            Reference<XTextRange> xStartRange;
            std::shared_ptr< ::xmloff::ParsedRDFaAttributes >
                xRDFaAttributes;
            if (m_rHelper.FindAndRemoveBookmarkStartRange(
                    m_sBookmarkName, xStartRange,
                    m_sXmlId, xRDFaAttributes))
            {
                Reference<XTextRange> xEndRange(
                    m_rHelper.GetCursorAsRange()->getStart());

                // check if beginning and end are in same XText
                if (xStartRange.is() && xEndRange.is() && xStartRange->getText() == xEndRange->getText())
                {
                    // create range for insertion
                    Reference<XTextCursor> xInsertionCursor =
                        m_rHelper.GetText()->createTextCursorByRange(
                            xEndRange);
                    try {
                        xInsertionCursor->gotoRange(xStartRange, true);
                    } catch (uno::Exception&) {
                        TOOLS_WARN_EXCEPTION("xmloff.text",
                            "cannot go to end position of bookmark");
                    }

                    //DBG_ASSERT(! xInsertionCursor->isCollapsed(),
                    //              "we want no point mark");
                    // can't assert, because someone could
                    // create a file with subsequence
                    // start/end elements

                    Reference<XInterface> xContent;
                    // insert reference
                    xContent = CreateAndInsertMark(GetImport(),
                            sAPI_bookmark,
                            m_sBookmarkName,
                            xInsertionCursor,
                            m_sXmlId);
                    if (xRDFaAttributes)
                    {
                        const Reference<rdf::XMetadatable>
                            xMeta(xContent, UNO_QUERY);
                        GetImport().GetRDFaImportHelper().AddRDFa(
                            xMeta, xRDFaAttributes);
                    }
                    const Reference<XPropertySet> xPropertySet(xContent, UNO_QUERY);
                    if (xPropertySet.is())
                    {
                        xPropertySet->setPropertyValue(u"BookmarkHidden"_ustr,    uno::Any(m_rHelper.getBookmarkHidden(m_sBookmarkName)));
                        xPropertySet->setPropertyValue(u"BookmarkCondition"_ustr, uno::Any(m_rHelper.getBookmarkCondition(m_sBookmarkName)));
                    }
                    if (m_sBookmarkName.startsWith("__RefHeading__"))
                    {
                        assert(xContent.is());
                        m_rxCrossRefHeadingBookmark = xContent;
                    }
                }
                // else: beginning/end in different XText -> ignore!
            }
            // else: no start found -> ignore!
            break;
        }
        case TypeFieldmarkStart:
        {
            break;
        }
        case TypeFieldmarkSeparator:
        {
            InsertFieldmark(GetImport(), m_rHelper, false);
            break;
        }
        case TypeFieldmarkEnd:
        {
            if (m_rHelper.hasCurrentFieldCtx() && !m_rHelper.hasCurrentFieldSeparator())
            {   // backward compat for old files without separator
                InsertFieldmark(GetImport(), m_rHelper, true);
            }
            PopFieldmark(m_rHelper);
            break;
        }
        case TypeReferenceStart:
        case TypeReferenceEnd:
            OSL_FAIL("reference start/end are handled in txtparai !");
            break;

        default:
            OSL_FAIL("unknown mark type");
            break;
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextMarkImportContext::createFastChildContext(
    sal_Int32 ,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    return new XMLFieldParamImportContext(GetImport(), m_rHelper);
}


Reference<XTextContent> XMLTextMarkImportContext::CreateAndInsertMark(
    SvXMLImport& rImport,
    const OUString& sServiceName,
    const OUString& sMarkName,
    const Reference<XTextRange> & rRange,
    const OUString& i_rXmlId,
    bool const isFieldmarkSeparatorMissing)
{
    // create mark
    const Reference<XMultiServiceFactory> xFactory(rImport.GetModel(),
        UNO_QUERY);
    Reference<XInterface> xIfc;

    if (xFactory.is())
    {
        xIfc = xFactory->createInstance(sServiceName);

        if (!xIfc.is())
        {
            OSL_FAIL("CreateAndInsertMark: cannot create service?");
            return nullptr;
        }

        // set name (unless there is no name (text:meta))
        const Reference<XNamed> xNamed(xIfc, UNO_QUERY);
        if (xNamed.is())
        {
            xNamed->setName(sMarkName);
        }
        else
        {
            if (!sMarkName.isEmpty())
            {
                OSL_FAIL("name given, but XNamed not supported?");
                return nullptr;
            }
        }

        if (isFieldmarkSeparatorMissing)
        {
            uno::Reference<beans::XPropertySet> const xProps(xIfc, uno::UNO_QUERY_THROW);
            xProps->setPropertyValue(u"PrivateSeparatorAtStart"_ustr, uno::Any(true));
        }

        // cast to XTextContent and attach to document
        const Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
        if (xTextContent.is())
        {
            try
            {
                // if inserting marks, bAbsorb==sal_False will cause
                // collapsing of the given XTextRange.
                rImport.GetTextImport()->GetText()->insertTextContent(rRange,
                    xTextContent, true);

                // xml:id for RDF metadata -- after insertion!
                rImport.SetXmlId(xIfc, i_rXmlId);

                return xTextContent;
            }
            catch (css::lang::IllegalArgumentException &)
            {
                OSL_FAIL("CreateAndInsertMark: cannot insert?");
                return nullptr;
            }
        }
    }
    return nullptr;
}

bool XMLTextMarkImportContext::FindName(
    const Reference<XFastAttributeList> & xAttrList)
{
    bool bNameOK = false;

    // find name attribute first
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_NAME):
                m_sBookmarkName = sValue;
                bNameOK = true;
                break;
            case XML_ELEMENT(XML, XML_ID):
                m_sXmlId = sValue;
                break;
            // RDFa
            case XML_ELEMENT(XHTML, XML_ABOUT):
                m_sAbout = sValue;
                m_bHaveAbout = true;
                break;
            case XML_ELEMENT(XHTML, XML_PROPERTY):
                m_sProperty = sValue;
                break;
            case XML_ELEMENT(XHTML, XML_CONTENT):
                m_sContent = sValue;
                break;
            case XML_ELEMENT(XHTML, XML_DATATYPE):
                m_sDatatype = sValue;
                break;
            case XML_ELEMENT(FIELD, XML_TYPE):
                m_sFieldName = sValue;
                break;
            case XML_ELEMENT(LO_EXT, XML_HIDDEN):
                ::sax::Converter::convertBool(m_isHidden, sValue);
                break;
            case XML_ELEMENT(LO_EXT, XML_CONDITION):
                m_sCondition = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    return bNameOK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
