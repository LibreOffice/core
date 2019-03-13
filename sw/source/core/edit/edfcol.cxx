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

#include <editsh.hxx>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/unoprnms.hxx>
#include <sfx2/classificationhelper.hxx>
#include <svx/ClassificationCommon.hxx>
#include <svl/cryptosign.hxx>
#include <svl/sigstruct.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/virdev.hxx>

#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <edimp.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <fmtpdsc.hxx>
#include <viewopt.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <docary.hxx>
#include <docsh.hxx>
#include <unoprnms.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <txtfrm.hxx>
#include <rdfhelper.hxx>
#include <sfx2/watermarkitem.hxx>

#include <unoparagraph.hxx>
#include <unotextrange.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <modeltoviewhelper.hxx>
#include <strings.hrc>
#include <undobj.hxx>
#include <UndoParagraphSignature.hxx>
#include <txtatr.hxx>
#include <fmtmeta.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <tools/diagnose_ex.h>
#include <IDocumentRedlineAccess.hxx>

#define WATERMARK_NAME "PowerPlusWaterMarkObject"
#define WATERMARK_AUTO_SIZE sal_uInt32(1)

namespace
{
static const OUString MetaFilename("tscp/bails.rdf");
static const OUString MetaNS("urn:bails");
static const OUString ParagraphSignatureRDFNamespace = "urn:bails:loext:paragraph:signature:";
static const OUString ParagraphSignatureIdRDFName = "urn:bails:loext:paragraph:signature:id";
static const OUString ParagraphSignatureDigestRDFName = ":digest";
static const OUString ParagraphSignatureDateRDFName = ":date";
static const OUString ParagraphSignatureUsageRDFName = ":usage";
static const OUString ParagraphSignatureLastIdRDFName = "urn:bails:loext:paragraph:signature:lastid";
static const OUString ParagraphClassificationNameRDFName = "urn:bails:loext:paragraph:classification:name";
static const OUString ParagraphClassificationValueRDFName = "urn:bails:loext:paragraph:classification:value";
static const OUString ParagraphClassificationAbbrRDFName = "urn:bails:loext:paragraph:classification:abbreviation";
static const OUString ParagraphClassificationFieldNamesRDFName = "urn:bails:loext:paragraph:classification:fields";
static const OUString MetadataFieldServiceName = "com.sun.star.text.textfield.MetadataField";
static const OUString DocInfoServiceName = "com.sun.star.text.TextField.DocInfo.Custom";

/// Find all page styles which are currently used in the document.
std::vector<OUString> lcl_getUsedPageStyles(SwViewShell const * pShell)
{
    std::vector<OUString> aReturn;

    SwRootFrame* pLayout = pShell->GetLayout();
    for (SwFrame* pFrame = pLayout->GetLower(); pFrame; pFrame = pFrame->GetNext())
    {
        SwPageFrame* pPage = static_cast<SwPageFrame*>(pFrame);
        if (const SwPageDesc *pDesc = pPage->FindPageDesc())
            aReturn.push_back(pDesc->GetName());
    }

    return aReturn;
}

/// Search for a field named rFieldName of type rServiceName in xText and return it.
uno::Reference<text::XTextField> lcl_findField(const uno::Reference<text::XText>& xText, const OUString& rServiceName, const OUString& rFieldName)
{
    uno::Reference<text::XTextField> xField;
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraphs->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
        while (xTextPortions->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
            OUString aTextPortionType;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
            if (aTextPortionType != UNO_NAME_TEXT_FIELD)
                continue;

            uno::Reference<lang::XServiceInfo> xTextField;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xTextField;
            if (!xTextField->supportsService(rServiceName))
                continue;

            OUString aName;
            uno::Reference<beans::XPropertySet> xPropertySet(xTextField, uno::UNO_QUERY);
            xPropertySet->getPropertyValue(UNO_NAME_NAME) >>= aName;
            if (aName == rFieldName)
            {
                xField = uno::Reference<text::XTextField>(xTextField, uno::UNO_QUERY);
                break;
            }
        }
    }

    return xField;
}

/// Search for a field named rFieldName of type rServiceName in xText and return true iff found.
bool lcl_hasField(const uno::Reference<text::XText>& xText, const OUString& rServiceName, const OUString& rFieldName)
{
    return lcl_findField(xText, rServiceName, rFieldName).is();
}

/// Search for a frame with WATERMARK_NAME in name of type rServiceName in xText. Returns found name in rShapeName.
uno::Reference<drawing::XShape> lcl_getWatermark(const uno::Reference<text::XText>& xText,
    const OUString& rServiceName, OUString& rShapeName, bool& bSuccess)
{
    bSuccess = false;
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraphs->nextElement(), uno::UNO_QUERY);
        if (!xTextPortionEnumerationAccess.is())
            continue;

        bSuccess = true;

        uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
        while (xTextPortions->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
            OUString aTextPortionType;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
            if (aTextPortionType != "Frame")
                continue;

            uno::Reference<container::XContentEnumerationAccess> xContentEnumerationAccess(xTextPortion, uno::UNO_QUERY);
            if (!xContentEnumerationAccess.is())
                continue;

            uno::Reference<container::XEnumeration> xEnumeration = xContentEnumerationAccess->createContentEnumeration("com.sun.star.text.TextContent");
            if (!xEnumeration->hasMoreElements())
                continue;

            uno::Reference<lang::XServiceInfo> xWatermark(xEnumeration->nextElement(), uno::UNO_QUERY);
            if (!xWatermark->supportsService(rServiceName))
                continue;

            uno::Reference<container::XNamed> xNamed(xWatermark, uno::UNO_QUERY);

            if (!xNamed->getName().match(WATERMARK_NAME))
                continue;

            rShapeName = xNamed->getName();

            uno::Reference<drawing::XShape> xShape(xWatermark, uno::UNO_QUERY);
            return xShape;
        }
    }

    return uno::Reference<drawing::XShape>();
}

/// Extract the text of the paragraph without any of the fields.
/// TODO: Consider moving to SwTextNode, or extend ModelToViewHelper.
OString lcl_getParagraphBodyText(const uno::Reference<text::XTextContent>& xText)
{
    OUStringBuffer strBuf;
    uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xText, uno::UNO_QUERY);
    if (!xTextPortionEnumerationAccess.is())
        return OString();

    uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
    while (xTextPortions->hasMoreElements())
    {
        uno::Any elem = xTextPortions->nextElement();

        //TODO: Consider including hidden and conditional texts/portions.
        OUString aTextPortionType;
        uno::Reference<beans::XPropertySet> xPropertySet(elem, uno::UNO_QUERY);
        xPropertySet->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
        if (aTextPortionType == "Text")
        {
            uno::Reference<text::XTextRange> xTextRange(elem, uno::UNO_QUERY);
            if (xTextRange.is())
                strBuf.append(xTextRange->getString());
        }
    }

    // Cleanup the dummy characters added by fields (which we exclude).
    comphelper::string::remove(strBuf, CH_TXT_ATR_INPUTFIELDSTART);
    comphelper::string::remove(strBuf, CH_TXT_ATR_INPUTFIELDEND);
    comphelper::string::remove(strBuf, CH_TXTATR_BREAKWORD);

    return strBuf.makeStringAndClear().trim().toUtf8();
}

template <typename T>
std::map<OUString, OUString> lcl_getRDFStatements(const uno::Reference<frame::XModel>& xModel,
                                                  const T& xRef)
{
    try
    {
        const css::uno::Reference<css::rdf::XResource> xSubject(xRef, uno::UNO_QUERY);
        return SwRDFHelper::getStatements(xModel, MetaNS, xSubject);
    }
    catch (const ::css::uno::Exception&)
    {
    }

    return std::map<OUString, OUString>();
}

/// Returns RDF (key, value) pair associated with the field, if any.
std::pair<OUString, OUString> lcl_getFieldRDFByPrefix(const uno::Reference<frame::XModel>& xModel,
                                                      const uno::Reference<css::text::XTextField>& xField,
                                                      const OUString& sPrefix)
{
    for (const auto& pair : lcl_getRDFStatements(xModel, xField))
    {
        if (pair.first.startsWith(sPrefix))
            return pair;
    }

    return std::make_pair(OUString(), OUString());
}

/// Returns RDF (key, value) pair associated with the field, if any.
template <typename T>
std::pair<OUString, OUString> lcl_getRDF(const uno::Reference<frame::XModel>& xModel,
                                         const T& xRef,
                                         const OUString& sRDFName)
{
    const std::map<OUString, OUString> aStatements = lcl_getRDFStatements(xModel, xRef);
    const auto it = aStatements.find(sRDFName);
    return (it != aStatements.end()) ? std::make_pair(it->first, it->second) : std::make_pair(OUString(), OUString());
}

/// Returns true iff the field in question is paragraph signature.
/// Note: must have associated RDF, since signatures are otherwise just metadata fields.
bool lcl_IsParagraphSignatureField(const uno::Reference<frame::XModel>& xModel,
                                   const uno::Reference<css::text::XTextField>& xField)
{
    return (lcl_getRDF(xModel, xField, ParagraphSignatureIdRDFName).first == ParagraphSignatureIdRDFName);
}

uno::Reference<text::XTextField> lcl_findFieldByRDF(const uno::Reference<frame::XModel>& xModel,
                                                    const uno::Reference<text::XTextContent>& xParagraph,
                                                    const OUString& sRDFName,
                                                    const OUString& sRDFValue)
{
    uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraph, uno::UNO_QUERY);
    if (!xTextPortionEnumerationAccess.is())
        return uno::Reference<text::XTextField>();

    uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
    if (!xTextPortions.is())
        return uno::Reference<text::XTextField>();

    while (xTextPortions->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
        OUString aTextPortionType;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
        if (aTextPortionType != UNO_NAME_TEXT_FIELD)
            continue;

        uno::Reference<lang::XServiceInfo> xTextField;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xTextField;
        if (!xTextField->supportsService(MetadataFieldServiceName))
            continue;

        uno::Reference<text::XTextField> xField(xTextField, uno::UNO_QUERY);
        const std::pair<OUString, OUString> pair = lcl_getRDF(xModel, xField, sRDFName);
        if (pair.first == sRDFName && (sRDFValue.isEmpty() || sRDFValue == pair.second))
            return xField;
    }

    return uno::Reference<text::XTextField>();
}

struct SignatureDescr
{
    OUString msSignature;
    OUString msUsage;
    OUString msDate;

    bool isValid() const { return !msSignature.isEmpty(); }
};

SignatureDescr lcl_getSignatureDescr(const uno::Reference<frame::XModel>& xModel,
                                     const uno::Reference<css::text::XTextContent>& xParagraph,
                                     const OUString& sFieldId)
{
    SignatureDescr aDescr;

    const OUString prefix = ParagraphSignatureRDFNamespace + sFieldId;
    const std::map<OUString, OUString> aStatements = lcl_getRDFStatements(xModel, xParagraph);

    const auto itSig = aStatements.find(prefix + ParagraphSignatureDigestRDFName);
    aDescr.msSignature = (itSig != aStatements.end() ? itSig->second : OUString());

    const auto itDate = aStatements.find(prefix + ParagraphSignatureDateRDFName);
    aDescr.msDate = (itDate != aStatements.end() ? itDate->second : OUString());

    const auto itUsage = aStatements.find(prefix + ParagraphSignatureUsageRDFName);
    aDescr.msUsage = (itUsage != aStatements.end() ? itUsage->second : OUString());

    return aDescr;
}

SignatureDescr lcl_getSignatureDescr(const uno::Reference<frame::XModel>& xModel,
                                     const uno::Reference<css::text::XTextContent>& xParagraph,
                                     const uno::Reference<css::text::XTextField>& xField)
{
    const OUString sFieldId = lcl_getRDF(xModel, xField, ParagraphSignatureIdRDFName).second;
    if (!sFieldId.isEmpty())
        return lcl_getSignatureDescr(xModel, xParagraph, sFieldId);

    return SignatureDescr();
}

/// Validate and create the signature field display text from the fields.
std::pair<bool, OUString> lcl_MakeParagraphSignatureFieldText(const SignatureDescr& aDescr,
                                                              const OString& utf8Text)
{
    OUString msg = SwResId(STR_INVALID_SIGNATURE);
    bool valid = false;

    if (aDescr.isValid())
    {
        const sal_Char* pData = utf8Text.getStr();
        const std::vector<unsigned char> data(pData, pData + utf8Text.getLength());

        OString encSignature;
        if (aDescr.msSignature.convertToString(&encSignature, RTL_TEXTENCODING_UTF8, 0))
        {
            const std::vector<unsigned char> sig(svl::crypto::DecodeHexString(encSignature));
            SignatureInformation aInfo(0);
            valid = svl::crypto::Signing::Verify(data, false, sig, aInfo);
            valid = valid
                    && aInfo.nStatus == xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;

            msg = SwResId(STR_SIGNED_BY) + ": " + aInfo.ouSubject + ", ";
            msg += aDescr.msDate;
            msg += (!aDescr.msUsage.isEmpty() ? (" (" + aDescr.msUsage + "): ") : OUString(": "));
            msg += (valid ? SwResId(STR_VALID) : SwResId(STR_INVALID));
        }
    }

    return std::make_pair(valid, msg);
}

/// Validate and return validation result and signature field display text.
std::pair<bool, OUString>
lcl_MakeParagraphSignatureFieldText(const uno::Reference<frame::XModel>& xModel,
                                    const uno::Reference<css::text::XTextContent>& xParagraph,
                                    const uno::Reference<css::text::XTextField>& xField,
                                    const OString& utf8Text)
{
    const SignatureDescr aDescr = lcl_getSignatureDescr(xModel, xParagraph, xField);
    return lcl_MakeParagraphSignatureFieldText(aDescr, utf8Text);
}

/// Generate the next valid ID for the a new signature on this paragraph.
OUString lcl_getNextSignatureId(const uno::Reference<frame::XModel>& xModel,
                                const uno::Reference<text::XTextContent>& xParagraph)
{
    const OUString sFieldId = lcl_getRDF(xModel, xParagraph, ParagraphSignatureLastIdRDFName).second;
    return OUString::number(!sFieldId.isEmpty() ? sFieldId.toInt32() + 1 : 1);
}

/// Creates and inserts Paragraph Signature Metadata field and creates the RDF entry
uno::Reference<text::XTextField> lcl_InsertParagraphSignature(const uno::Reference<frame::XModel>& xModel,
                                                              const uno::Reference<text::XTextContent>& xParagraph,
                                                              const OUString& signature,
                                                              const OUString& usage)
{
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
    auto xField = uno::Reference<text::XTextField>(xMultiServiceFactory->createInstance(MetadataFieldServiceName), uno::UNO_QUERY);

    // Add the signature at the end.
    xField->attach(xParagraph->getAnchor()->getEnd());

    const OUString sId = lcl_getNextSignatureId(xModel, xParagraph);

    const css::uno::Reference<css::rdf::XResource> xSubject(xField, uno::UNO_QUERY);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xSubject, ParagraphSignatureIdRDFName, sId);

    // First convert the UTC UNIX timestamp to a tools::DateTime then to local time.
    DateTime aDateTime = DateTime::CreateFromUnixTime(time(nullptr));
    aDateTime.ConvertToLocalTime();
    OUStringBuffer rBuffer;
    rBuffer.append(static_cast<sal_Int32>(aDateTime.GetYear()));
    rBuffer.append('-');
    if (aDateTime.GetMonth() < 10)
        rBuffer.append('0');
    rBuffer.append(static_cast<sal_Int32>(aDateTime.GetMonth()));
    rBuffer.append('-');
    if (aDateTime.GetDay() < 10)
        rBuffer.append('0');
    rBuffer.append(static_cast<sal_Int32>(aDateTime.GetDay()));

    // Now set the RDF on the paragraph, since that's what is preserved in .doc(x).
    const css::uno::Reference<css::rdf::XResource> xParaSubject(xParagraph, uno::UNO_QUERY);
    const OUString prefix = ParagraphSignatureRDFNamespace + sId;
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xParaSubject, ParagraphSignatureLastIdRDFName, sId);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xParaSubject, prefix + ParagraphSignatureDigestRDFName, signature);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xParaSubject, prefix + ParagraphSignatureUsageRDFName, usage);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xParaSubject, prefix + ParagraphSignatureDateRDFName, rBuffer.makeStringAndClear());

    return xField;
}

/// Updates the signature field text if changed and returns true only iff updated.
bool lcl_DoUpdateParagraphSignatureField(SwDoc* pDoc,
                                         const uno::Reference<css::text::XTextField>& xField,
                                         const OUString& sDisplayText)
{
    // Disable undo to avoid introducing noise when we edit the metadata field.
    const bool isUndoEnabled = pDoc->GetIDocumentUndoRedo().DoesUndo();
    pDoc->GetIDocumentUndoRedo().DoUndo(false);
    comphelper::ScopeGuard const g([pDoc, isUndoEnabled]() {
        pDoc->GetIDocumentUndoRedo().DoUndo(isUndoEnabled);
    });

    try
    {
        uno::Reference<css::text::XTextRange> xText(xField, uno::UNO_QUERY);
        const OUString curText = xText->getString();
        if (curText != sDisplayText)
        {
            xText->setString(sDisplayText);
            return true;
        }
    }
    catch (const uno::Exception&)
    {
        // We failed; avoid crashing.
        DBG_UNHANDLED_EXCEPTION("sw.uno", "Failed to update paragraph signature");
    }

    return false;
}

/// Updates the signature field text if changed and returns true only iff updated.
bool lcl_UpdateParagraphSignatureField(SwDoc* pDoc,
                                       const uno::Reference<frame::XModel>& xModel,
                                       const uno::Reference<css::text::XTextContent>& xParagraph,
                                       const uno::Reference<css::text::XTextField>& xField,
                                       const OString& utf8Text)
{
    const OUString sDisplayText
        = lcl_MakeParagraphSignatureFieldText(xModel, xParagraph, xField, utf8Text).second;
    return lcl_DoUpdateParagraphSignatureField(pDoc, xField, sDisplayText);
}

void lcl_RemoveParagraphMetadataField(const uno::Reference<css::text::XTextField>& xField)
{
    uno::Reference<css::text::XTextContent> xFieldTextContent(xField, uno::UNO_QUERY);
    uno::Reference<css::text::XTextRange> xParagraph(xFieldTextContent->getAnchor());
    uno::Reference<css::text::XText> xParagraphText(xParagraph->getText(), uno::UNO_QUERY);
    xParagraphText->removeTextContent(xFieldTextContent);
}

/// Returns true iff the field in question is paragraph classification.
/// Note: must have associated RDF, since classifications are otherwise just metadata fields.
bool lcl_IsParagraphClassificationField(const uno::Reference<frame::XModel>& xModel,
                                        const uno::Reference<css::text::XTextField>& xField,
                                        const OUString& sKey)
{
    const std::pair<OUString, OUString> rdfPair = lcl_getRDF(xModel, xField, ParagraphClassificationNameRDFName);
    return rdfPair.first == ParagraphClassificationNameRDFName && (sKey.isEmpty() || rdfPair.second == sKey);
}

uno::Reference<text::XTextField> lcl_FindParagraphClassificationField(const uno::Reference<frame::XModel>& xModel,
                                                                      const uno::Reference<text::XTextContent>& xParagraph,
                                                                      const OUString& sKey = OUString())
{
    uno::Reference<text::XTextField> xTextField;

    uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraph, uno::UNO_QUERY);
    if (!xTextPortionEnumerationAccess.is())
        return xTextField;

    // Enumerate text portions to find metadata fields. This is expensive, best to enumerate fields only.
    uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
    while (xTextPortions->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
        OUString aTextPortionType;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
        if (aTextPortionType != UNO_NAME_TEXT_FIELD)
            continue;

        uno::Reference<lang::XServiceInfo> xServiceInfo;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xServiceInfo;
        if (!xServiceInfo->supportsService(MetadataFieldServiceName))
            continue;

        uno::Reference<text::XTextField> xField(xServiceInfo, uno::UNO_QUERY);
        if (lcl_IsParagraphClassificationField(xModel, xField, sKey))
        {
            xTextField = xField;
            break;
        }
    }

    return xTextField;
}

/// Creates and inserts Paragraph Classification Metadata field and creates the RDF entry
uno::Reference<text::XTextField> lcl_InsertParagraphClassification(const uno::Reference<frame::XModel>& xModel,
                                                                   const uno::Reference<text::XTextContent>& xParent)
{
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
    auto xField = uno::Reference<text::XTextField>(xMultiServiceFactory->createInstance(MetadataFieldServiceName), uno::UNO_QUERY);

    // Add the classification at the start.
    xField->attach(xParent->getAnchor()->getStart());
    return xField;
}

/// Updates the paragraph classification field text if changed and returns true only iff updated.
bool lcl_UpdateParagraphClassificationField(SwDoc* pDoc,
                                            const uno::Reference<frame::XModel>& xModel,
                                            const uno::Reference<css::text::XTextContent>& xTextNode,
                                            const OUString& sKey,
                                            const OUString& sValue,
                                            const OUString& sDisplayText)
{
    // Disable undo to avoid introducing noise when we edit the metadata field.
    const bool isUndoEnabled = pDoc->GetIDocumentUndoRedo().DoesUndo();
    pDoc->GetIDocumentUndoRedo().DoUndo(false);
    comphelper::ScopeGuard const g([pDoc, isUndoEnabled] () {
            pDoc->GetIDocumentUndoRedo().DoUndo(isUndoEnabled);
        });

    uno::Reference<text::XTextField> xField = lcl_InsertParagraphClassification(xModel, xTextNode);

    css::uno::Reference<css::rdf::XResource> xFieldSubject(xField, uno::UNO_QUERY);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xFieldSubject, sKey, sValue);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xFieldSubject, ParagraphClassificationNameRDFName, sKey);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xFieldSubject, ParagraphClassificationValueRDFName, sValue);

    css::uno::Reference<css::rdf::XResource> xNodeSubject(xTextNode, uno::UNO_QUERY);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xNodeSubject, sKey, sValue);

    return lcl_DoUpdateParagraphSignatureField(pDoc, xField, sDisplayText);
}

void lcl_ValidateParagraphSignatures(SwDoc* pDoc, const uno::Reference<text::XTextContent>& xParagraph, const bool updateDontRemove)
{
    SwDocShell* pDocShell = pDoc->GetDocShell();
    if (!pDocShell)
        return;

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();

    // Check if the paragraph is signed.
    try
    {
        const std::pair<OUString, OUString> pair = lcl_getRDF(xModel, xParagraph, ParagraphSignatureLastIdRDFName);
        if (pair.second.isEmpty())
            return;
    }
    catch (const ::css::uno::Exception&)
    {
        return;
    }

    uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraph, uno::UNO_QUERY);
    if (!xTextPortionEnumerationAccess.is())
        return;

    uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
    if (!xTextPortions.is())
        return;

    // Get the text (without fields).
    const OString utf8Text = lcl_getParagraphBodyText(xParagraph);
    if (utf8Text.isEmpty())
        return;

    while (xTextPortions->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
        OUString aTextPortionType;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
        if (aTextPortionType != UNO_NAME_TEXT_FIELD)
            continue;

        uno::Reference<lang::XServiceInfo> xTextField;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xTextField;
        if (!xTextField->supportsService(MetadataFieldServiceName))
            continue;

        uno::Reference<text::XTextField> xField(xTextField, uno::UNO_QUERY);
        if (!lcl_IsParagraphSignatureField(xModel, xField))
        {
            continue;
        }

        if (updateDontRemove)
        {
            lcl_UpdateParagraphSignatureField(pDoc, xModel, xParagraph, xField, utf8Text);
        }
        else if (!lcl_MakeParagraphSignatureFieldText(xModel, xParagraph, xField, utf8Text).first)
        {
            pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::PARA_SIGN_ADD, nullptr);
            pDoc->GetIDocumentUndoRedo().AppendUndo(
                std::make_unique<SwUndoParagraphSigning>(pDoc, xField, xParagraph, false));
            lcl_RemoveParagraphMetadataField(xField);
            pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::PARA_SIGN_ADD, nullptr);
        }
    }
}

} // anonymous namespace

SwTextFormatColl& SwEditShell::GetDfltTextFormatColl() const
{
    return *GetDoc()->GetDfltTextFormatColl();
}

sal_uInt16 SwEditShell::GetTextFormatCollCount() const
{
    return GetDoc()->GetTextFormatColls()->size();
}

SwTextFormatColl& SwEditShell::GetTextFormatColl(sal_uInt16 nFormatColl) const
{
    return *((*(GetDoc()->GetTextFormatColls()))[nFormatColl]);
}

static void insertFieldToDocument(uno::Reference<lang::XMultiServiceFactory> const & rxMultiServiceFactory,
                           uno::Reference<text::XText> const & rxText, uno::Reference<text::XParagraphCursor> const & rxParagraphCursor,
                           OUString const & rsKey)
{
    uno::Reference<beans::XPropertySet> xField(rxMultiServiceFactory->createInstance(DocInfoServiceName), uno::UNO_QUERY);
    xField->setPropertyValue(UNO_NAME_NAME, uno::makeAny(rsKey));
    uno::Reference<text::XTextContent> xTextContent(xField, uno::UNO_QUERY);

    rxText->insertTextContent(rxParagraphCursor, xTextContent, false);
}

static void removeAllClassificationFields(OUString const & rPolicy, uno::Reference<text::XText> const & rxText)
{
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(rxText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraphs->nextElement(), uno::UNO_QUERY);
        uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
        while (xTextPortions->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
            OUString aTextPortionType;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
            if (aTextPortionType != UNO_NAME_TEXT_FIELD)
                continue;

            uno::Reference<lang::XServiceInfo> xTextField;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xTextField;
            if (!xTextField->supportsService(DocInfoServiceName))
                continue;

            OUString aName;
            uno::Reference<beans::XPropertySet> xPropertySet(xTextField, uno::UNO_QUERY);
            xPropertySet->getPropertyValue(UNO_NAME_NAME) >>= aName;
            if (aName.startsWith(rPolicy))
            {
                uno::Reference<text::XTextField> xField(xTextField, uno::UNO_QUERY);
                rxText->removeTextContent(xField);
            }
        }
    }
}

static sal_Int32 getNumberOfParagraphs(uno::Reference<text::XText> const & xText)
{
    uno::Reference<container::XEnumerationAccess> xParagraphEnumAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphEnum = xParagraphEnumAccess->createEnumeration();
    sal_Int32 nResult = 0;
    while (xParagraphEnum->hasMoreElements())
    {
        xParagraphEnum->nextElement();
        nResult++;
    }
    return nResult;
}

static void equaliseNumberOfParagraph(std::vector<svx::ClassificationResult> const & rResults, uno::Reference<text::XText> const & xText)
{
    sal_Int32 nNumberOfParagraphs = 0;
    for (svx::ClassificationResult const & rResult : rResults)
    {
        if (rResult.meType == svx::ClassificationType::PARAGRAPH)
            nNumberOfParagraphs++;
    }

    while (getNumberOfParagraphs(xText) < nNumberOfParagraphs)
    {
        uno::Reference<text::XParagraphAppend> xParagraphAppend(xText, uno::UNO_QUERY);
        xParagraphAppend->finishParagraph(uno::Sequence<beans::PropertyValue>());
    }
}

void SwEditShell::ApplyAdvancedClassification(std::vector<svx::ClassificationResult> const & rResults)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !SfxObjectShell::Current())
        return;

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);

    uno::Reference<document::XDocumentProperties> xDocumentProperties = SfxObjectShell::Current()->getDocProperties();

    const OUString sPolicy = SfxClassificationHelper::policyTypeToString(SfxClassificationHelper::getPolicyType());
    const std::vector<OUString> aUsedPageStyles = lcl_getUsedPageStyles(this);
    for (const OUString& rPageStyleName : aUsedPageStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);

        // HEADER
        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
        uno::Reference<text::XText> xHeaderText;
        if (bHeaderIsOn)
            xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;
        if (xHeaderText.is())
            removeAllClassificationFields(sPolicy, xHeaderText);

        // FOOTER
        bool bFooterIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_FOOTER_IS_ON) >>= bFooterIsOn;
        uno::Reference<text::XText> xFooterText;
        if (bFooterIsOn)
            xPageStyle->getPropertyValue(UNO_NAME_FOOTER_TEXT) >>= xFooterText;
        if (xFooterText.is())
            removeAllClassificationFields(sPolicy, xFooterText);
    }

    // Clear properties
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    svx::classification::removeAllProperties(xPropertyContainer);

    SfxClassificationHelper aHelper(xDocumentProperties);

    // Apply properties from the BA policy
    for (svx::ClassificationResult const & rResult : rResults)
    {
        if (rResult.meType == svx::ClassificationType::CATEGORY)
        {
            aHelper.SetBACName(rResult.msName, SfxClassificationHelper::getPolicyType());
        }
    }

    sfx::ClassificationKeyCreator aCreator(SfxClassificationHelper::getPolicyType());

    // Insert origin document property
    svx::classification::insertCreationOrigin(xPropertyContainer, aCreator, sfx::ClassificationCreationOrigin::MANUAL);

    // Insert full text as document property
    svx::classification::insertFullTextualRepresentationAsDocumentProperty(xPropertyContainer, aCreator, rResults);

    for (const OUString& rPageStyleName : aUsedPageStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);

        // HEADER
        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
        if (!bHeaderIsOn)
            xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_ON, uno::makeAny(true));
        uno::Reference<text::XText> xHeaderText;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;
        equaliseNumberOfParagraph(rResults, xHeaderText);

        // FOOTER
        bool bFooterIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_FOOTER_IS_ON) >>= bFooterIsOn;
        if (!bFooterIsOn)
            xPageStyle->setPropertyValue(UNO_NAME_FOOTER_IS_ON, uno::makeAny(true));
        uno::Reference<text::XText> xFooterText;
        xPageStyle->getPropertyValue(UNO_NAME_FOOTER_TEXT) >>= xFooterText;
        equaliseNumberOfParagraph(rResults, xFooterText);

        // SET/DELETE WATERMARK
        SfxWatermarkItem aWatermarkItem;
        aWatermarkItem.SetText(aHelper.GetDocumentWatermark());
        SetWatermark(aWatermarkItem);

        uno::Reference<text::XParagraphCursor> xHeaderParagraphCursor(xHeaderText->createTextCursor(), uno::UNO_QUERY);
        uno::Reference<text::XParagraphCursor> xFooterParagraphCursor(xFooterText->createTextCursor(), uno::UNO_QUERY);

        sal_Int32 nParagraph = -1;

        for (svx::ClassificationResult const & rResult : rResults)
        {
            switch(rResult.meType)
            {
                case svx::ClassificationType::TEXT:
                {
                    OUString sKey = aCreator.makeNumberedTextKey();

                    svx::classification::addOrInsertDocumentProperty(xPropertyContainer, sKey, rResult.msName);
                    insertFieldToDocument(xMultiServiceFactory, xHeaderText, xHeaderParagraphCursor, sKey);
                    insertFieldToDocument(xMultiServiceFactory, xFooterText, xFooterParagraphCursor, sKey);
                }
                break;

                case svx::ClassificationType::CATEGORY:
                {
                    OUString sKey = aCreator.makeCategoryNameKey();
                    insertFieldToDocument(xMultiServiceFactory, xHeaderText, xHeaderParagraphCursor, sKey);
                    insertFieldToDocument(xMultiServiceFactory, xFooterText, xFooterParagraphCursor, sKey);
                }
                break;

                case svx::ClassificationType::MARKING:
                {
                    OUString sKey = aCreator.makeNumberedMarkingKey();
                    svx::classification::addOrInsertDocumentProperty(xPropertyContainer, sKey, rResult.msName);
                    insertFieldToDocument(xMultiServiceFactory, xHeaderText, xHeaderParagraphCursor, sKey);
                    insertFieldToDocument(xMultiServiceFactory, xFooterText, xFooterParagraphCursor, sKey);
                }
                break;

                case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
                {
                    OUString sKey = aCreator.makeNumberedIntellectualPropertyPartKey();
                    svx::classification::addOrInsertDocumentProperty(xPropertyContainer, sKey, rResult.msName);
                    insertFieldToDocument(xMultiServiceFactory, xHeaderText, xHeaderParagraphCursor, sKey);
                    insertFieldToDocument(xMultiServiceFactory, xFooterText, xFooterParagraphCursor, sKey);
                }
                break;

                case svx::ClassificationType::PARAGRAPH:
                {
                    nParagraph++;

                    if (nParagraph != 0) // only jump to next paragraph, if we aren't at the first paragraph
                    {
                        xHeaderParagraphCursor->gotoNextParagraph(false);
                        xFooterParagraphCursor->gotoNextParagraph(false);
                    }

                    xHeaderParagraphCursor->gotoStartOfParagraph(false);
                    xFooterParagraphCursor->gotoStartOfParagraph(false);

                    uno::Reference<beans::XPropertySet> xHeaderPropertySet(xHeaderParagraphCursor, uno::UNO_QUERY_THROW);
                    uno::Reference<beans::XPropertySet> xFooterPropertySet(xFooterParagraphCursor, uno::UNO_QUERY_THROW);
                    if (rResult.msName == "BOLD")
                    {
                        xHeaderPropertySet->setPropertyValue("CharWeight", uno::makeAny(awt::FontWeight::BOLD));
                        xFooterPropertySet->setPropertyValue("CharWeight", uno::makeAny(awt::FontWeight::BOLD));
                    }
                    else
                    {
                        xHeaderPropertySet->setPropertyValue("CharWeight", uno::makeAny(awt::FontWeight::NORMAL));
                        xFooterPropertySet->setPropertyValue("CharWeight", uno::makeAny(awt::FontWeight::NORMAL));
                    }
                }
                break;

                default:
                break;
            }
        }
    }
}

std::vector<svx::ClassificationResult> SwEditShell::CollectAdvancedClassification()
{
    std::vector<svx::ClassificationResult> aResult;

    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !SfxObjectShell::Current())
        return aResult;

    const OUString sBlank;

    uno::Reference<document::XDocumentProperties> xDocumentProperties = SfxObjectShell::Current()->getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();
    sfx::ClassificationKeyCreator aCreator(SfxClassificationHelper::getPolicyType());

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);

    std::vector<OUString> aPageStyles = lcl_getUsedPageStyles(this);
    OUString aPageStyleString = aPageStyles.back();
    uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(aPageStyleString), uno::UNO_QUERY);

    bool bHeaderIsOn = false;
    xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
    if (!bHeaderIsOn)
    {
        const OUString aValue = svx::classification::getProperty(xPropertyContainer, aCreator.makeCategoryNameKey());
        if (!aValue.isEmpty())
            aResult.push_back({ svx::ClassificationType::CATEGORY, aValue, sBlank, sBlank });

        return aResult;
    }

    uno::Reference<text::XText> xHeaderText;
    xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;

    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xHeaderText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();

    // set to true if category was found in the header
    bool bFoundClassificationCategory = false;

    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraphs->nextElement(), uno::UNO_QUERY);
        if (!xTextPortionEnumerationAccess.is())
            continue;
        uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();

        // Check font weight
        uno::Reference<beans::XPropertySet> xParagraphPropertySet(xTextPortionEnumerationAccess, uno::UNO_QUERY_THROW);
        uno::Any aAny = xParagraphPropertySet->getPropertyValue("CharWeight");

        OUString sWeight = (aAny.get<float>() >= awt::FontWeight::BOLD) ? OUString("BOLD") : OUString("NORMAL");

        aResult.push_back({ svx::ClassificationType::PARAGRAPH, sWeight, sBlank, sBlank });

        // Process portions
        while (xTextPortions->hasMoreElements())
        {
            uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
            OUString aTextPortionType;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
            if (aTextPortionType != UNO_NAME_TEXT_FIELD)
                continue;

            uno::Reference<lang::XServiceInfo> xTextField;
            xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xTextField;
            if (!xTextField->supportsService(DocInfoServiceName))
                continue;

            OUString aName;
            uno::Reference<beans::XPropertySet> xPropertySet(xTextField, uno::UNO_QUERY);
            xPropertySet->getPropertyValue(UNO_NAME_NAME) >>= aName;

            if (aCreator.isMarkingTextKey(aName))
            {
                const OUString aValue = svx::classification::getProperty(xPropertyContainer, aName);
                if (!aValue.isEmpty())
                    aResult.push_back({ svx::ClassificationType::TEXT, aValue, sBlank, sBlank });
            }
            else if (aCreator.isCategoryNameKey(aName))
            {
                const OUString aValue = svx::classification::getProperty(xPropertyContainer, aName);
                if (!aValue.isEmpty())
                    aResult.push_back({ svx::ClassificationType::CATEGORY, aValue, sBlank, sBlank });
                bFoundClassificationCategory = true;
            }
            else if (aCreator.isCategoryIdentifierKey(aName))
            {
                const OUString aValue = svx::classification::getProperty(xPropertyContainer, aName);
                if (!aValue.isEmpty())
                    aResult.push_back({ svx::ClassificationType::CATEGORY, sBlank, sBlank, aValue });
                bFoundClassificationCategory = true;
            }
            else if (aCreator.isMarkingKey(aName))
            {
                const OUString aValue = svx::classification::getProperty(xPropertyContainer, aName);
                if (!aValue.isEmpty())
                    aResult.push_back({ svx::ClassificationType::MARKING, aValue, sBlank, sBlank });
            }
            else if (aCreator.isIntellectualPropertyPartKey(aName))
            {
                const OUString aValue = svx::classification::getProperty(xPropertyContainer, aName);
                if (!aValue.isEmpty())
                    aResult.push_back({ svx::ClassificationType::INTELLECTUAL_PROPERTY_PART, aValue, sBlank, sBlank });
            }
        }
    }

    if (!bFoundClassificationCategory)
    {
        const OUString aValue = svx::classification::getProperty(xPropertyContainer, aCreator.makeCategoryNameKey());
        if (!aValue.isEmpty())
            aResult.push_back({ svx::ClassificationType::CATEGORY, aValue, sBlank, sBlank });
    }

    return aResult;
}

void SwEditShell::SetClassification(const OUString& rName, SfxClassificationPolicyType eType)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;

    SfxClassificationHelper aHelper(pDocShell->getDocProperties());

    const bool bHadWatermark = !aHelper.GetDocumentWatermark().isEmpty();

    // This updates the infobar as well.
    aHelper.SetBACName(rName, eType);

    // Insert origin document property
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = pDocShell->getDocProperties()->getUserDefinedProperties();
    sfx::ClassificationKeyCreator aCreator(SfxClassificationHelper::getPolicyType());
    svx::classification::insertCreationOrigin(xPropertyContainer, aCreator, sfx::ClassificationCreationOrigin::BAF_POLICY);

    bool bHeaderIsNeeded = aHelper.HasDocumentHeader();
    bool bFooterIsNeeded = aHelper.HasDocumentFooter();
    OUString aWatermark = aHelper.GetDocumentWatermark();
    bool bWatermarkIsNeeded = !aWatermark.isEmpty();

    if (!bHeaderIsNeeded && !bFooterIsNeeded && !bWatermarkIsNeeded && !bHadWatermark)
        return;

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);
    uno::Sequence<OUString> aStyles = xStyleFamily->getElementNames();

    for (const OUString& rPageStyleName : aStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);
        uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);

        if (bHeaderIsNeeded || bWatermarkIsNeeded || bHadWatermark)
        {
            // If the header is off, turn it on.
            bool bHeaderIsOn = false;
            xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
            if (!bHeaderIsOn)
                xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_ON, uno::makeAny(true));

            // If the header already contains a document header field, no need to do anything.
            uno::Reference<text::XText> xHeaderText;
            xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;

            if (bHeaderIsNeeded)
            {
                if (!lcl_hasField(xHeaderText, DocInfoServiceName, SfxClassificationHelper::PROP_PREFIX_INTELLECTUALPROPERTY() + SfxClassificationHelper::PROP_DOCHEADER()))
                {
                    // Append a field to the end of the header text.
                    uno::Reference<beans::XPropertySet> xField(xMultiServiceFactory->createInstance(DocInfoServiceName), uno::UNO_QUERY);
                    xField->setPropertyValue(UNO_NAME_NAME, uno::makeAny(SfxClassificationHelper::PROP_PREFIX_INTELLECTUALPROPERTY() + SfxClassificationHelper::PROP_DOCHEADER()));
                    uno::Reference<text::XTextContent> xTextContent(xField, uno::UNO_QUERY);
                    xHeaderText->insertTextContent(xHeaderText->getEnd(), xTextContent, /*bAbsorb=*/false);
                }
            }

            SfxWatermarkItem aWatermarkItem;
            aWatermarkItem.SetText(aWatermark);
            SetWatermark(aWatermarkItem);
        }

        if (bFooterIsNeeded)
        {
            // If the footer is off, turn it on.
            bool bFooterIsOn = false;
            xPageStyle->getPropertyValue(UNO_NAME_FOOTER_IS_ON) >>= bFooterIsOn;
            if (!bFooterIsOn)
                xPageStyle->setPropertyValue(UNO_NAME_FOOTER_IS_ON, uno::makeAny(true));

            // If the footer already contains a document header field, no need to do anything.
            uno::Reference<text::XText> xFooterText;
            xPageStyle->getPropertyValue(UNO_NAME_FOOTER_TEXT) >>= xFooterText;
            static OUString sFooter = SfxClassificationHelper::PROP_PREFIX_INTELLECTUALPROPERTY() + SfxClassificationHelper::PROP_DOCFOOTER();
            if (!lcl_hasField(xFooterText, DocInfoServiceName, sFooter))
            {
                // Append a field to the end of the footer text.
                uno::Reference<beans::XPropertySet> xField(xMultiServiceFactory->createInstance(DocInfoServiceName), uno::UNO_QUERY);
                xField->setPropertyValue(UNO_NAME_NAME, uno::makeAny(sFooter));
                uno::Reference<text::XTextContent> xTextContent(xField, uno::UNO_QUERY);
                xFooterText->insertTextContent(xFooterText->getEnd(), xTextContent, /*bAbsorb=*/false);
            }
        }
    }
}

static void lcl_ApplyParagraphClassification(SwDoc* pDoc,
                                      const uno::Reference<frame::XModel>& xModel,
                                      const uno::Reference<text::XTextContent>& xParent,
                                      std::vector<svx::ClassificationResult> aResults)
{
    css::uno::Reference<css::rdf::XResource> xNodeSubject(xParent, uno::UNO_QUERY);
    if (!xNodeSubject.is())
        return;

    // Remove all paragraph classification fields.
    for (;;)
    {
        uno::Reference<text::XTextField> xTextField = lcl_FindParagraphClassificationField(xModel, xParent);
        if (!xTextField.is())
            break;
        lcl_RemoveParagraphMetadataField(xTextField);
    }

    if (aResults.empty())
        return;

    // Since we always insert at the start of the paragraph,
    // need to insert in reverse order.
    std::reverse(aResults.begin(), aResults.end());
    // Ignore "PARAGRAPH" types
    aResults.erase(std::remove_if(aResults.begin(),
                                  aResults.end(),
                                  [&](const svx::ClassificationResult& rResult)-> bool
                                            { return rResult.meType == svx::ClassificationType::PARAGRAPH; }),
                                  aResults.end());

    sfx::ClassificationKeyCreator aKeyCreator(SfxClassificationHelper::getPolicyType());
    std::vector<OUString> aFieldNames;
    for (size_t nIndex = 0; nIndex < aResults.size(); ++nIndex)
    {
        const svx::ClassificationResult& rResult = aResults[nIndex];

        const bool isLast = nIndex == 0;
        const bool isFirst = (nIndex == aResults.size() - 1);
        OUString sKey;
        OUString sValue = rResult.msName;
        switch (rResult.meType)
        {
            case svx::ClassificationType::TEXT:
            {
                sKey = aKeyCreator.makeNumberedTextKey();
            }
            break;

            case svx::ClassificationType::CATEGORY:
            {
                if (rResult.msIdentifier.isEmpty())
                {
                    sKey = aKeyCreator.makeCategoryNameKey();
                }
                else
                {
                    sValue = rResult.msIdentifier;
                    sKey = aKeyCreator.makeCategoryIdentifierKey();
                }
                SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xNodeSubject, ParagraphClassificationAbbrRDFName, rResult.msAbbreviatedName);
            }
            break;

            case svx::ClassificationType::MARKING:
            {
                sKey = aKeyCreator.makeNumberedMarkingKey();
            }
            break;

            case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
            {
                sKey = aKeyCreator.makeNumberedIntellectualPropertyPartKey();
            }
            break;

            default:
            break;
        }

        OUString sDisplayText = (isFirst ? ("(" + rResult.msAbbreviatedName) : rResult.msAbbreviatedName);
        if (isLast)
            sDisplayText += ")";
        lcl_UpdateParagraphClassificationField(pDoc, xModel, xParent, sKey, sValue, sDisplayText);
        aFieldNames.emplace_back(sKey);
    }

    // Correct the order
    std::reverse(aFieldNames.begin(), aFieldNames.end());
    OUStringBuffer sFieldNames;
    bool first = true;
    for (const OUString& rFieldName : aFieldNames)
    {
        if (!first)
            sFieldNames.append("/");
        sFieldNames.append(rFieldName);
        first = false;
    }

    const OUString sOldFieldNames = lcl_getRDF(xModel, xNodeSubject, ParagraphClassificationFieldNamesRDFName).second;
    SwRDFHelper::removeStatement(xModel, MetaNS, xNodeSubject, ParagraphClassificationFieldNamesRDFName, sOldFieldNames);
    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xNodeSubject, ParagraphClassificationFieldNamesRDFName, sFieldNames.makeStringAndClear());
}

void SwEditShell::ApplyParagraphClassification(std::vector<svx::ClassificationResult> aResults)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !GetCursor() || !GetCursor()->Start())
        return;

    SwTextNode* pNode = GetCursor()->Start()->nNode.GetNode().GetTextNode();
    if (pNode == nullptr)
        return;

    // Prevent recursive validation since this is triggered on node updates, which we do below.
    const bool bOldValidationFlag = SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([this, bOldValidationFlag]() {
        SetParagraphSignatureValidation(bOldValidationFlag);
    });

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<text::XTextContent> xParent = SwXParagraph::CreateXParagraph(*pNode->GetDoc(), pNode);
    lcl_ApplyParagraphClassification(GetDoc(), xModel, xParent, std::move(aResults));
}

static std::vector<svx::ClassificationResult> lcl_CollectParagraphClassification(const uno::Reference<frame::XModel>& xModel, const uno::Reference<text::XTextContent>& xParagraph)
{
    std::vector<svx::ClassificationResult> aResult;

    uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraph, uno::UNO_QUERY);
    if (!xTextPortionEnumerationAccess.is())
        return aResult;

    uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();

    const sfx::ClassificationKeyCreator aKeyCreator(SfxClassificationHelper::getPolicyType());

    while (xTextPortions->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
        OUString aTextPortionType;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
        if (aTextPortionType != UNO_NAME_TEXT_FIELD)
            continue;

        uno::Reference<lang::XServiceInfo> xField;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xField;
        if (!xField->supportsService(MetadataFieldServiceName))
            continue;

        uno::Reference<text::XTextField> xTextField(xField, uno::UNO_QUERY);
        const OUString sPolicy = SfxClassificationHelper::policyTypeToString(SfxClassificationHelper::getPolicyType());
        const std::pair<OUString, OUString> rdfNamePair = lcl_getFieldRDFByPrefix(xModel, xTextField, sPolicy);

        uno::Reference<text::XTextRange> xTextRange(xField, uno::UNO_QUERY);
        const OUString aName = rdfNamePair.first;
        const OUString aValue = rdfNamePair.second;
        const OUString sBlank("");
        if (aKeyCreator.isMarkingTextKey(aName))
        {
            aResult.push_back({ svx::ClassificationType::TEXT, aValue, sBlank, sBlank });
        }
        else if (aKeyCreator.isCategoryNameKey(aName))
        {
            aResult.push_back({ svx::ClassificationType::CATEGORY, aValue, sBlank, sBlank });
        }
        else if (aKeyCreator.isCategoryIdentifierKey(aName))
        {
            aResult.push_back({ svx::ClassificationType::CATEGORY, sBlank, sBlank, aValue });
        }
        else if (aKeyCreator.isMarkingKey(aName))
        {
            aResult.push_back({ svx::ClassificationType::MARKING, aValue, sBlank, sBlank });
        }
        else if (aKeyCreator.isIntellectualPropertyPartKey(aName))
        {
            aResult.push_back({ svx::ClassificationType::INTELLECTUAL_PROPERTY_PART, xTextRange->getString(), sBlank, sBlank });
        }
    }

    return aResult;
}

std::vector<svx::ClassificationResult> SwEditShell::CollectParagraphClassification()
{
    std::vector<svx::ClassificationResult> aResult;

    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !GetCursor() || !GetCursor()->Start())
        return aResult;

    SwTextNode* pNode = GetCursor()->Start()->nNode.GetNode().GetTextNode();
    if (pNode == nullptr)
        return aResult;

    uno::Reference<text::XTextContent> xParent = SwXParagraph::CreateXParagraph(*pNode->GetDoc(), pNode);
    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    return lcl_CollectParagraphClassification(xModel, xParent);
}

static sal_Int16 lcl_GetAngle(const drawing::HomogenMatrix3& rMatrix)
{
    basegfx::B2DHomMatrix aTransformation;
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate = 0;
    double fShear = 0;

    aTransformation.set(0, 0, rMatrix.Line1.Column1);
    aTransformation.set(0, 1, rMatrix.Line1.Column2);
    aTransformation.set(0, 2, rMatrix.Line1.Column3);
    aTransformation.set(1, 0, rMatrix.Line2.Column1);
    aTransformation.set(1, 1, rMatrix.Line2.Column2);
    aTransformation.set(1, 2, rMatrix.Line2.Column3);
    aTransformation.set(2, 0, rMatrix.Line3.Column1);
    aTransformation.set(2, 1, rMatrix.Line3.Column2);
    aTransformation.set(2, 2, rMatrix.Line3.Column3);

    aTransformation.decompose(aScale, aTranslate, fRotate, fShear);
    sal_Int16 nDeg = round(basegfx::rad2deg(fRotate));
    return nDeg < 0 ? round(nDeg) * -1 : round(360.0 - nDeg);
}

SfxWatermarkItem SwEditShell::GetWatermark()
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return SfxWatermarkItem();

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);
    std::vector<OUString> aUsedPageStyles = lcl_getUsedPageStyles(this);
    for (const OUString& rPageStyleName : aUsedPageStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);
        uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);

        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
        if (!bHeaderIsOn)
            return SfxWatermarkItem();

        uno::Reference<text::XText> xHeaderText;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;

        OUString aShapeServiceName = "com.sun.star.drawing.CustomShape";
        OUString sWatermark = "";
        bool bSuccess = false;
        uno::Reference<drawing::XShape> xWatermark = lcl_getWatermark(xHeaderText, aShapeServiceName, sWatermark, bSuccess);

        if (xWatermark.is())
        {
            SfxWatermarkItem aItem;
            uno::Reference<text::XTextRange> xTextRange(xWatermark, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xPropertySet(xWatermark, uno::UNO_QUERY);
            Color nColor;
            sal_Int16 nTransparency;
            OUString aFont;
            drawing::HomogenMatrix3 aMatrix;

            aItem.SetText(xTextRange->getString());

            if (xPropertySet->getPropertyValue(UNO_NAME_CHAR_FONT_NAME) >>= aFont)
                aItem.SetFont(aFont);
            if (xPropertySet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= nColor)
                aItem.SetColor(nColor);
            if (xPropertySet->getPropertyValue("Transformation") >>= aMatrix)
                aItem.SetAngle(lcl_GetAngle(aMatrix));
            if (xPropertySet->getPropertyValue(UNO_NAME_FILL_TRANSPARENCE) >>= nTransparency)
                aItem.SetTransparency(nTransparency);

            return aItem;
        }
    }
    return SfxWatermarkItem();
}

static void lcl_placeWatermarkInHeader(const SfxWatermarkItem& rWatermark,
                            const uno::Reference<frame::XModel>& xModel,
                            const uno::Reference<beans::XPropertySet>& xPageStyle,
                            const uno::Reference<text::XText>& xHeaderText)
{
    if (!xHeaderText.is())
        return;

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
    OUString aShapeServiceName = "com.sun.star.drawing.CustomShape";
    OUString sWatermark = WATERMARK_NAME;
    bool bSuccess = false;
    uno::Reference<drawing::XShape> xWatermark = lcl_getWatermark(xHeaderText, aShapeServiceName, sWatermark, bSuccess);

    bool bDeleteWatermark = rWatermark.GetText().isEmpty();
    if (xWatermark.is())
    {
        drawing::HomogenMatrix3 aMatrix;
        Color nColor = 0xc0c0c0;
        sal_Int16 nTransparency = 50;
        sal_Int16 nAngle = 45;
        OUString aFont = "";

        uno::Reference<beans::XPropertySet> xPropertySet(xWatermark, uno::UNO_QUERY);
        xPropertySet->getPropertyValue(UNO_NAME_CHAR_FONT_NAME) >>= aFont;
        xPropertySet->getPropertyValue(UNO_NAME_FILLCOLOR) >>= nColor;
        xPropertySet->getPropertyValue(UNO_NAME_FILL_TRANSPARENCE) >>= nTransparency;
        xPropertySet->getPropertyValue("Transformation") >>= aMatrix;
        nAngle = lcl_GetAngle(aMatrix);

        // If the header already contains a watermark, see if it its text is up to date.
        uno::Reference<text::XTextRange> xTextRange(xWatermark, uno::UNO_QUERY);
        if (xTextRange->getString() != rWatermark.GetText()
            || aFont != rWatermark.GetFont()
            || nColor != rWatermark.GetColor()
            || nAngle != rWatermark.GetAngle()
            || nTransparency != rWatermark.GetTransparency()
            || bDeleteWatermark)
        {
            // No: delete it and we'll insert a replacement.
            uno::Reference<lang::XComponent> xComponent(xWatermark, uno::UNO_QUERY);
            xComponent->dispose();
            xWatermark.clear();
        }
    }

    if (!bSuccess || xWatermark.is() || bDeleteWatermark)
        return;

    const OUString& sFont = rWatermark.GetFont();
    sal_Int16 nAngle = rWatermark.GetAngle();
    sal_Int16 nTransparency = rWatermark.GetTransparency();
    Color nColor = rWatermark.GetColor();

    // Calc the ratio.
    double fRatio = 0;

    ScopedVclPtrInstance<VirtualDevice> pDevice;
    vcl::Font aFont = pDevice->GetFont();
    aFont.SetFamilyName(sFont);
    aFont.SetFontSize(Size(0, 96));
    pDevice->SetFont(aFont);

    auto nTextWidth = pDevice->GetTextWidth(rWatermark.GetText());
    if (nTextWidth)
    {
        fRatio = pDevice->GetTextHeight();
        fRatio /= nTextWidth;
    }

    // Calc the size.
    sal_Int32 nWidth = 0;
    awt::Size aSize;
    xPageStyle->getPropertyValue(UNO_NAME_SIZE) >>= aSize;
    if (aSize.Width < aSize.Height)
    {
        // Portrait.
        sal_Int32 nLeftMargin = 0;
        xPageStyle->getPropertyValue(UNO_NAME_LEFT_MARGIN) >>= nLeftMargin;
        sal_Int32 nRightMargin = 0;
        xPageStyle->getPropertyValue(UNO_NAME_RIGHT_MARGIN) >>= nRightMargin;
        nWidth = aSize.Width - nLeftMargin - nRightMargin;
    }
    else
    {
        // Landscape.
        sal_Int32 nTopMargin = 0;
        xPageStyle->getPropertyValue(UNO_NAME_TOP_MARGIN) >>= nTopMargin;
        sal_Int32 nBottomMargin = 0;
        xPageStyle->getPropertyValue(UNO_NAME_BOTTOM_MARGIN) >>= nBottomMargin;
        nWidth = aSize.Height - nTopMargin - nBottomMargin;
    }
    sal_Int32 nHeight = fRatio * nWidth;

    // Create and insert the shape.
    uno::Reference<drawing::XShape> xShape(xMultiServiceFactory->createInstance(aShapeServiceName), uno::UNO_QUERY);
    basegfx::B2DHomMatrix aTransformation;
    aTransformation.identity();
    aTransformation.scale(nWidth, nHeight);
    aTransformation.rotate(-basegfx::deg2rad(nAngle));
    drawing::HomogenMatrix3 aMatrix;
    aMatrix.Line1.Column1 = aTransformation.get(0, 0);
    aMatrix.Line1.Column2 = aTransformation.get(0, 1);
    aMatrix.Line1.Column3 = aTransformation.get(0, 2);
    aMatrix.Line2.Column1 = aTransformation.get(1, 0);
    aMatrix.Line2.Column2 = aTransformation.get(1, 1);
    aMatrix.Line2.Column3 = aTransformation.get(1, 2);
    aMatrix.Line3.Column1 = aTransformation.get(2, 0);
    aMatrix.Line3.Column2 = aTransformation.get(2, 1);
    aMatrix.Line3.Column3 = aTransformation.get(2, 2);
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
    xPropertySet->setPropertyValue(UNO_NAME_ANCHOR_TYPE, uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<text::XTextContent> xTextContent(xShape, uno::UNO_QUERY);
    xHeaderText->insertTextContent(xHeaderText->getEnd(), xTextContent, false);

    // The remaining properties have to be set after the shape is inserted: do that in one batch to avoid flickering.
    uno::Reference<document::XActionLockable> xLockable(xShape, uno::UNO_QUERY);
    xLockable->addActionLock();
    xPropertySet->setPropertyValue(UNO_NAME_FILLCOLOR, uno::makeAny(static_cast<sal_Int32>(nColor)));
    xPropertySet->setPropertyValue(UNO_NAME_FILLSTYLE, uno::makeAny(drawing::FillStyle_SOLID));
    xPropertySet->setPropertyValue(UNO_NAME_FILL_TRANSPARENCE, uno::makeAny(nTransparency));
    xPropertySet->setPropertyValue(UNO_NAME_LINESTYLE, uno::makeAny(drawing::LineStyle_NONE));
    xPropertySet->setPropertyValue(UNO_NAME_OPAQUE, uno::makeAny(false));
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT, uno::makeAny(false));
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_AUTOGROWWIDTH, uno::makeAny(false));
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_MINFRAMEHEIGHT, uno::makeAny(nHeight));
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_MINFRAMEWIDTH, uno::makeAny(nWidth));
    xPropertySet->setPropertyValue(UNO_NAME_TEXT_WRAP, uno::makeAny(text::WrapTextMode_THROUGH));
    xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION, uno::makeAny(static_cast<sal_Int16>(text::RelOrientation::PAGE_PRINT_AREA)));
    xPropertySet->setPropertyValue(UNO_NAME_VERT_ORIENT_RELATION, uno::makeAny(static_cast<sal_Int16>(text::RelOrientation::PAGE_PRINT_AREA)));
    xPropertySet->setPropertyValue(UNO_NAME_CHAR_FONT_NAME, uno::makeAny(sFont));
    xPropertySet->setPropertyValue(UNO_NAME_CHAR_HEIGHT, uno::makeAny(WATERMARK_AUTO_SIZE));
    xPropertySet->setPropertyValue("Transformation", uno::makeAny(aMatrix));

    uno::Reference<text::XTextRange> xTextRange(xShape, uno::UNO_QUERY);
    xTextRange->setString(rWatermark.GetText());

    uno::Reference<drawing::XEnhancedCustomShapeDefaulter> xDefaulter(xShape, uno::UNO_QUERY);
    xDefaulter->createCustomShapeDefaults("fontwork-plain-text");

    auto aGeomPropSeq = xPropertySet->getPropertyValue("CustomShapeGeometry").get< uno::Sequence<beans::PropertyValue> >();
    auto aGeomPropVec = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aGeomPropSeq);
    uno::Sequence<beans::PropertyValue> aPropertyValues(comphelper::InitPropertySequence(
    {
        {"TextPath", uno::makeAny(true)},
    }));
    auto it = std::find_if(aGeomPropVec.begin(), aGeomPropVec.end(), [](const beans::PropertyValue& rValue)
    {
        return rValue.Name == "TextPath";
    });
    if (it == aGeomPropVec.end())
        aGeomPropVec.push_back(comphelper::makePropertyValue("TextPath", aPropertyValues));
    else
        it->Value <<= aPropertyValues;
    xPropertySet->setPropertyValue("CustomShapeGeometry", uno::makeAny(comphelper::containerToSequence(aGeomPropVec)));

    // tdf#108494, tdf#109313 the header height was switched to height of a watermark
    // and shape was moved to the lower part of a page, force position update
    xPropertySet->getPropertyValue("Transformation") >>= aMatrix;
    xPropertySet->setPropertyValue("Transformation", uno::makeAny(aMatrix));

    xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT, uno::makeAny(static_cast<sal_Int16>(text::HoriOrientation::CENTER)));
    xPropertySet->setPropertyValue(UNO_NAME_VERT_ORIENT, uno::makeAny(static_cast<sal_Int16>(text::VertOrientation::CENTER)));

    uno::Reference<container::XNamed> xNamed(xShape, uno::UNO_QUERY);
    xNamed->setName(sWatermark);
    xLockable->removeActionLock();
}

void SwEditShell::SetWatermark(const SfxWatermarkItem& rWatermark)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;
    const bool bNoWatermark = rWatermark.GetText().isEmpty();

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);
    uno::Sequence<OUString> aStyles = xStyleFamily->getElementNames();

    for (const OUString& rPageStyleName : aStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);

        // If the header is off, turn it on.
        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
        if (!bHeaderIsOn)
        {
            if (bNoWatermark)
                continue; // the style doesn't have any watermark - no need to do anything

            xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_ON, uno::makeAny(true));
        }

        // backup header height
        bool bDynamicHeight = true;
        sal_Int32 nOldValue;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_HEIGHT) >>= nOldValue;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT) >>= bDynamicHeight;
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT, uno::Any(false));

        // If the header already contains a document header field, no need to do anything.
        uno::Reference<text::XText> xHeaderText;
        uno::Reference<text::XText> xHeaderTextFirst;
        uno::Reference<text::XText> xHeaderTextLeft;
        uno::Reference<text::XText> xHeaderTextRight;

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderText);

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT_FIRST) >>= xHeaderTextFirst;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderTextFirst);

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT_LEFT) >>= xHeaderTextLeft;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderTextLeft);

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT_RIGHT) >>= xHeaderTextRight;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderTextRight);

        // tdf#108494 the header height was switched to height of a watermark
        // and shape was moved to the lower part of a page
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_HEIGHT, uno::makeAny(sal_Int32(11)));
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_HEIGHT, uno::makeAny(nOldValue));
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT, uno::Any(bDynamicHeight));
    }
}

SwUndoParagraphSigning::SwUndoParagraphSigning(SwDoc* pDoc,
                                               const uno::Reference<text::XTextField>& xField,
                                               const uno::Reference<text::XTextContent>& xParent,
                                               const bool bRemove)
  : SwUndo(SwUndoId::PARA_SIGN_ADD, pDoc),
    m_pDoc(pDoc),
    m_xField(xField),
    m_xParent(xParent),
    m_bRemove(bRemove)
{
    // Save the metadata and field content to undo/redo.
    uno::Reference<frame::XModel> xModel = m_pDoc->GetDocShell()->GetBaseModel();
    const std::map<OUString, OUString> aStatements = lcl_getRDFStatements(xModel, m_xField);
    const auto it = aStatements.find(ParagraphSignatureIdRDFName);
    if (it != aStatements.end())
        m_signature = it->second;

    const auto it2 = aStatements.find(ParagraphSignatureUsageRDFName);
    if (it2 != aStatements.end())
        m_usage = it2->second;

    uno::Reference<css::text::XTextRange> xText(m_xField, uno::UNO_QUERY);
    m_display = xText->getString();
}

void SwUndoParagraphSigning::UndoImpl(::sw::UndoRedoContext&)
{
    if (m_bRemove)
        Remove();
    else
        Insert();
}

void SwUndoParagraphSigning::RedoImpl(::sw::UndoRedoContext&)
{
    if (m_bRemove)
        Insert();
    else
        Remove();
}

void SwUndoParagraphSigning::RepeatImpl(::sw::RepeatContext&)
{
}

void SwUndoParagraphSigning::Insert()
{
    // Disable undo to avoid introducing noise when we edit the metadata field.
    const bool isUndoEnabled = m_pDoc->GetIDocumentUndoRedo().DoesUndo();
    m_pDoc->GetIDocumentUndoRedo().DoUndo(false);

    // Prevent validation since this will trigger a premature validation
    // upon inserting, but before setting the metadata.
    SwEditShell* pEditSh = m_pDoc->GetEditShell();
    const bool bOldValidationFlag = pEditSh->SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([&] () {
            pEditSh->SetParagraphSignatureValidation(bOldValidationFlag);
            m_pDoc->GetIDocumentUndoRedo().DoUndo(isUndoEnabled);
        });

    m_xField = lcl_InsertParagraphSignature(m_pDoc->GetDocShell()->GetBaseModel(), m_xParent, m_signature, m_usage);
    lcl_DoUpdateParagraphSignatureField(m_pDoc, m_xField, m_display);
}

void SwUndoParagraphSigning::Remove()
{
    // Disable undo to avoid introducing noise when we edit the metadata field.
    const bool isUndoEnabled = m_pDoc->GetIDocumentUndoRedo().DoesUndo();
    m_pDoc->GetIDocumentUndoRedo().DoUndo(false);

    // Prevent validation since this will trigger a premature validation
    // upon removing.
    SwEditShell* pEditSh = m_pDoc->GetEditShell();
    const bool bOldValidationFlag = pEditSh->SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([&] () {
            pEditSh->SetParagraphSignatureValidation(bOldValidationFlag);
            m_pDoc->GetIDocumentUndoRedo().DoUndo(isUndoEnabled);
        });

    lcl_RemoveParagraphMetadataField(m_xField);
}

void SwEditShell::SignParagraph()
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !GetCursor() || !GetCursor()->Start())
        return;
    const SwPosition* pPosStart = GetCursor()->Start();
    if (!pPosStart)
        return;
    SwTextNode* pNode = pPosStart->nNode.GetNode().GetTextNode();
    if (!pNode)
        return;

    // Table text signing is not supported.
    if (pNode->FindTableNode() != nullptr)
        return;

    // 1. Get the text (without fields).
    const uno::Reference<text::XTextContent> xParagraph = SwXParagraph::CreateXParagraph(*pNode->GetDoc(), pNode);
    const OString utf8Text = lcl_getParagraphBodyText(xParagraph);
    if (utf8Text.isEmpty())
        return;

    // 2. Get certificate.
    uno::Reference<security::XDocumentDigitalSignatures> xSigner(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2" ) );

    uno::Sequence<css::beans::PropertyValue> aProperties;
    uno::Reference<security::XCertificate> xCertificate = xSigner->chooseCertificateWithProps(aProperties);
    if (!xCertificate.is())
        return;

    // 3. Sign it.
    svl::crypto::Signing signing(xCertificate);
    signing.AddDataRange(utf8Text.getStr(), utf8Text.getLength());
    OStringBuffer sigBuf;
    if (!signing.Sign(sigBuf))
        return;

    const OUString signature = OStringToOUString(sigBuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8, 0);

    std::vector<css::beans::PropertyValue> vec = comphelper::sequenceToContainer<std::vector<css::beans::PropertyValue>>(aProperties);
    auto it = std::find_if(vec.begin(), vec.end(), [](const beans::PropertyValue& rValue)
                                                    {
                                                        return rValue.Name == "Usage";
                                                    });

    OUString aUsage;
    if (it != vec.end())
        it->Value >>= aUsage;

    // 4. Add metadata
    // Prevent validation since this will trigger a premature validation
    // upon inserting, but before setting the metadata.
    const bool bOldValidationFlag = SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([this, bOldValidationFlag] () {
            SetParagraphSignatureValidation(bOldValidationFlag);
        });

    GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::PARA_SIGN_ADD, nullptr);

    const uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<css::text::XTextField> xField = lcl_InsertParagraphSignature(xModel, xParagraph, signature, aUsage);

    lcl_UpdateParagraphSignatureField(GetDoc(), xModel, xParagraph, xField, utf8Text);

    GetDoc()->GetIDocumentUndoRedo().AppendUndo(
        std::make_unique<SwUndoParagraphSigning>(GetDoc(), xField, xParagraph, true));

    GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::PARA_SIGN_ADD, nullptr);
}

void SwEditShell::ValidateParagraphSignatures(SwTextNode* pNode, bool updateDontRemove)
{
    if (!pNode || !IsParagraphSignatureValidationEnabled())
        return;

    // Table text signing is not supported.
    if (pNode->FindTableNode() != nullptr)
        return;

    // Prevent recursive validation since this is triggered on node updates, which we do below.
    const bool bOldValidationFlag = SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([this, bOldValidationFlag] () {
            SetParagraphSignatureValidation(bOldValidationFlag);
        });

    uno::Reference<text::XTextContent> xParentText = SwXParagraph::CreateXParagraph(*GetDoc(), pNode);
    lcl_ValidateParagraphSignatures(GetDoc(), xParentText, updateDontRemove);
}

void SwEditShell::ValidateCurrentParagraphSignatures(bool updateDontRemove)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !GetCursor() || !GetCursor()->Start() || !IsParagraphSignatureValidationEnabled())
        return;

    SwPaM* pPaM = GetCursor();
    const SwPosition* pPosStart = pPaM->Start();
    SwTextNode* pNode = pPosStart->nNode.GetNode().GetTextNode();
    ValidateParagraphSignatures(pNode, updateDontRemove);
}

void SwEditShell::ValidateAllParagraphSignatures(bool updateDontRemove)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !IsParagraphSignatureValidationEnabled())
        return;

    // Prevent recursive validation since this is triggered on node updates, which we do below.
    const bool bOldValidationFlag = SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([this, bOldValidationFlag] () {
            SetParagraphSignatureValidation(bOldValidationFlag);
        });

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    const uno::Reference<text::XTextDocument> xDoc(xModel, uno::UNO_QUERY);
    uno::Reference<text::XText> xParent = xDoc->getText();
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xParent, uno::UNO_QUERY);
    if (!xParagraphEnumerationAccess.is())
        return;
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    if (!xParagraphs.is())
        return;
    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<text::XTextContent> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
        lcl_ValidateParagraphSignatures(GetDoc(), xParagraph, updateDontRemove);
    }
}

static uno::Reference<text::XTextField> lcl_GetParagraphMetadataFieldAtIndex(const SwDocShell* pDocSh, SwTextNode const * pNode, const sal_uLong index)
{
    uno::Reference<text::XTextField> xTextField;
    if (pNode != nullptr && pDocSh != nullptr)
    {
        SwTextAttr* pAttr = pNode->GetTextAttrAt(index, RES_TXTATR_METAFIELD);
        SwTextMeta* pTextMeta = static_txtattr_cast<SwTextMeta*>(pAttr);
        if (pTextMeta != nullptr)
        {
            SwFormatMeta& rFormatMeta(static_cast<SwFormatMeta&>(pTextMeta->GetAttr()));
            if (::sw::Meta* pMeta = rFormatMeta.GetMeta())
            {
                const css::uno::Reference<css::rdf::XResource> xSubject(pMeta->MakeUnoObject(), uno::UNO_QUERY);
                uno::Reference<frame::XModel> xModel = pDocSh->GetBaseModel();
                const std::map<OUString, OUString> aStatements = lcl_getRDFStatements(xModel, xSubject);
                if (aStatements.find(ParagraphSignatureIdRDFName) != aStatements.end() ||
                    aStatements.find(ParagraphClassificationNameRDFName) != aStatements.end())
                {
                    xTextField = uno::Reference<text::XTextField>(xSubject, uno::UNO_QUERY);
                }
            }
        }
    }

    return xTextField;
}

void SwEditShell::RestoreMetadataFieldsAndValidateParagraphSignatures()
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || !IsParagraphSignatureValidationEnabled())
        return;

    // Prevent recursive validation since this is triggered on node updates, which we do below.
    const bool bOldValidationFlag = SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([this, bOldValidationFlag] () {
            SetParagraphSignatureValidation(bOldValidationFlag);
        });

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    const uno::Reference<text::XTextDocument> xDoc(xModel, uno::UNO_QUERY);
    uno::Reference<text::XText> xParent = xDoc->getText();
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xParent, uno::UNO_QUERY);
    if (!xParagraphEnumerationAccess.is())
        return;
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    if (!xParagraphs.is())
        return;

    static const OUString sBlank("");
    const sfx::ClassificationKeyCreator aKeyCreator(SfxClassificationHelper::getPolicyType());
    const css::uno::Sequence<css::uno::Reference<rdf::XURI>> aGraphNames = SwRDFHelper::getGraphNames(xModel, MetaNS);

    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<text::XTextContent> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);

        try
        {
            const css::uno::Reference<css::rdf::XResource> xSubject(xParagraph, uno::UNO_QUERY);
            const std::map<OUString, OUString> aStatements = SwRDFHelper::getStatements(xModel, aGraphNames, xSubject);

            const auto it = aStatements.find(ParagraphClassificationFieldNamesRDFName);
            const OUString sFieldNames = (it != aStatements.end() ? it->second : sBlank);
            std::vector<svx::ClassificationResult> aResults;
            if (!sFieldNames.isEmpty())
            {
                // Order the fields
                sal_Int32 nIndex = 0;
                do
                {
                    const OUString sCurFieldName = sFieldNames.getToken(0, '/', nIndex);
                    if (sCurFieldName.isEmpty())
                        break;

                    const auto it2 = aStatements.find(sCurFieldName);
                    const OUString sName = (it2 != aStatements.end() ? it->first : sBlank);
                    const OUString sValue = (it2 != aStatements.end() ? it->second : sBlank);

                    if (aKeyCreator.isMarkingTextKey(sName))
                    {
                        aResults.push_back({ svx::ClassificationType::TEXT, sValue, sValue, sBlank });
                    }
                    else if (aKeyCreator.isCategoryNameKey(sName))
                    {
                        const auto it3 = aStatements.find(ParagraphClassificationAbbrRDFName);
                        const OUString sAbbreviatedName = (it3 != aStatements.end() && !it3->second.isEmpty() ? it3->second : sValue);
                        aResults.push_back({ svx::ClassificationType::CATEGORY, sValue, sAbbreviatedName, sBlank });
                    }
                    else if (aKeyCreator.isCategoryIdentifierKey(sName))
                    {
                        const auto it3 = aStatements.find(ParagraphClassificationAbbrRDFName);
                        const OUString sAbbreviatedName = (it3 != aStatements.end() && !it3->second.isEmpty() ? it3->second : sValue);
                        aResults.push_back({ svx::ClassificationType::CATEGORY, sBlank, sAbbreviatedName, sValue });
                    }
                    else if (aKeyCreator.isMarkingKey(sName))
                    {
                        aResults.push_back({ svx::ClassificationType::MARKING, sValue, sValue, sBlank });
                    }
                    else if (aKeyCreator.isIntellectualPropertyPartKey(sName))
                    {
                        aResults.push_back({ svx::ClassificationType::INTELLECTUAL_PROPERTY_PART, sValue, sValue, sBlank });
                    }
                }
                while (nIndex >= 0);
            }

            // Update classification based on results.
            lcl_ApplyParagraphClassification(GetDoc(), xModel, xParagraph, aResults);

            // Get Signatures
            std::map<OUString, SignatureDescr> aSignatures;
            for (const auto& pair : lcl_getRDFStatements(xModel, xParagraph))
            {
                const OUString& sName = pair.first;
                if (sName.startsWith(ParagraphSignatureRDFNamespace))
                {
                    const OUString sSuffix = sName.copy(ParagraphSignatureRDFNamespace.getLength());
                    const sal_Int32 index = sSuffix.indexOf(":");
                    if (index >= 0)
                    {
                        const OUString id = sSuffix.copy(0, index);
                        const OUString type = sSuffix.copy(index);
                        const OUString& sValue = pair.second;
                        if (type == ParagraphSignatureDateRDFName)
                            aSignatures[id].msDate = sValue;
                        else if (type == ParagraphSignatureUsageRDFName)
                            aSignatures[id].msUsage = sValue;
                        else if (type == ParagraphSignatureDigestRDFName)
                            aSignatures[id].msSignature = sValue;
                    }
                }
            }

            for (const auto& pair : aSignatures)
            {
                uno::Reference<text::XTextField> xField = lcl_findFieldByRDF(xModel, xParagraph, ParagraphSignatureIdRDFName, pair.first);
                if (!xField.is())
                {
                    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
                    xField = uno::Reference<text::XTextField>(xMultiServiceFactory->createInstance(MetadataFieldServiceName), uno::UNO_QUERY);

                    // Add the signature at the end.
                    xField->attach(xParagraph->getAnchor()->getEnd());

                    const css::uno::Reference<css::rdf::XResource> xFieldSubject(xField, uno::UNO_QUERY);
                    SwRDFHelper::addStatement(xModel, MetaNS, MetaFilename, xFieldSubject, ParagraphSignatureIdRDFName, pair.first);

                    const OString utf8Text = lcl_getParagraphBodyText(xParagraph);
                    lcl_UpdateParagraphSignatureField(GetDoc(), xModel, xParagraph, xField, utf8Text);
                }
            }

            lcl_ValidateParagraphSignatures(GetDoc(), xParagraph, true); // Validate and Update signatures.
        }
        catch (const std::exception&)
        {
        }
    }
}

bool SwEditShell::IsCursorInParagraphMetadataField() const
{
    if (GetCursor() && GetCursor()->Start())
    {
        SwTextNode* pNode = GetCursor()->Start()->nNode.GetNode().GetTextNode();
        const sal_uLong index = GetCursor()->Start()->nContent.GetIndex();
        uno::Reference<text::XTextField> xField = lcl_GetParagraphMetadataFieldAtIndex(GetDoc()->GetDocShell(), pNode, index);
        return xField.is();
    }

    return false;
}

bool SwEditShell::RemoveParagraphMetadataFieldAtCursor()
{
    if (GetCursor() && GetCursor()->Start())
    {
        SwTextNode* pNode = GetCursor()->Start()->nNode.GetNode().GetTextNode();
        sal_uLong index = GetCursor()->Start()->nContent.GetIndex();
        uno::Reference<text::XTextField> xField = lcl_GetParagraphMetadataFieldAtIndex(GetDoc()->GetDocShell(), pNode, index);
        if (!xField.is())
        {
            // Try moving the cursor to see if we're _facing_ a metafield or not,
            // as opposed to being within one.
            index--; // Backspace moves left

            xField = lcl_GetParagraphMetadataFieldAtIndex(GetDoc()->GetDocShell(), pNode, index);
        }

        if (xField.is())
        {
            lcl_RemoveParagraphMetadataField(xField);
            return true;
        }
    }

    return false;
}

static OUString lcl_GetParagraphClassification(SfxClassificationHelper & rHelper, sfx::ClassificationKeyCreator const & rKeyCreator,
                                        const uno::Reference<frame::XModel>& xModel, const uno::Reference<text::XTextContent>& xParagraph)
{
    uno::Reference<text::XTextField> xTextField;
    xTextField = lcl_FindParagraphClassificationField(xModel, xParagraph, rKeyCreator.makeCategoryIdentifierKey());
    if (xTextField.is())
    {
        const std::pair<OUString, OUString> rdfValuePair = lcl_getRDF(xModel, xTextField, ParagraphClassificationValueRDFName);
        return rHelper.GetBACNameForIdentifier(rdfValuePair.second);
    }

    xTextField = lcl_FindParagraphClassificationField(xModel, xParagraph, rKeyCreator.makeCategoryNameKey());
    if (xTextField.is())
    {
        return lcl_getRDF(xModel, xTextField, ParagraphClassificationNameRDFName).second;
    }

    return OUString();
}

static OUString lcl_GetHighestClassificationParagraphClass(SwPaM* pCursor)
{
    OUString sHighestClass;

    SwTextNode* pNode = pCursor->Start()->nNode.GetNode().GetTextNode();
    if (pNode == nullptr)
        return sHighestClass;

    SwDocShell* pDocShell = pNode->GetDoc()->GetDocShell();
    if (!pDocShell)
        return sHighestClass;

    SfxClassificationHelper aHelper(pDocShell->getDocProperties());
    sfx::ClassificationKeyCreator aKeyCreator(SfxClassificationHelper::getPolicyType());

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    const uno::Reference< text::XTextDocument > xDoc(xModel, uno::UNO_QUERY);
    uno::Reference<text::XText> xParent = xDoc->getText();

    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xParent, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<text::XTextContent> xParagraph(xParagraphs->nextElement(), uno::UNO_QUERY);
        const OUString sCurrentClass = lcl_GetParagraphClassification(aHelper, aKeyCreator, xModel, xParagraph);
        sHighestClass = aHelper.GetHigherClass(sHighestClass, sCurrentClass);
    }

    return sHighestClass;
}

void SwEditShell::ClassifyDocPerHighestParagraphClass()
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;

    // Bail out as early as possible if we don't have paragraph classification.
    if (!SwRDFHelper::hasMetadataGraph(pDocShell->GetBaseModel(), MetaNS))
        return;

    uno::Reference<document::XDocumentProperties> xDocumentProperties = pDocShell->getDocProperties();
    uno::Reference<beans::XPropertyContainer> xPropertyContainer = xDocumentProperties->getUserDefinedProperties();

    sfx::ClassificationKeyCreator aKeyCreator(SfxClassificationHelper::getPolicyType());
    SfxClassificationHelper aHelper(xDocumentProperties);

    OUString sHighestClass = lcl_GetHighestClassificationParagraphClass(GetCursor());

    const OUString aClassificationCategory = svx::classification::getProperty(xPropertyContainer, aKeyCreator.makeCategoryNameKey());

    if (!aClassificationCategory.isEmpty())
    {
        sHighestClass = aHelper.GetHigherClass(sHighestClass, aClassificationCategory);
    }

    if (aClassificationCategory != sHighestClass)
    {
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(nullptr,
                                                       VclMessageType::Question, VclButtonsType::Ok,
                                                       SwResId(STR_CLASSIFICATION_LEVEL_CHANGED)));
        xQueryBox->run();
    }

    const SfxClassificationPolicyType eHighestClassType = SfxClassificationHelper::stringToPolicyType(sHighestClass);

    // Prevent paragraph signature validation since the below changes (f.e. watermarking) are benign.
    const bool bOldValidationFlag = SetParagraphSignatureValidation(false);
    comphelper::ScopeGuard const g([this, bOldValidationFlag]() {
        SetParagraphSignatureValidation(bOldValidationFlag);
    });

    // Check the origin, if "manual" (created via advanced classification dialog),
    // then we just need to set the category name.
    if (sfx::getCreationOriginProperty(xPropertyContainer, aKeyCreator) == sfx::ClassificationCreationOrigin::MANUAL)
    {
        aHelper.SetBACName(sHighestClass, eHighestClassType);
        ApplyAdvancedClassification(CollectAdvancedClassification());
    }
    else
    {
        SetClassification(sHighestClass, eHighestClassType);
    }
}

// #i62675#
void SwEditShell::SetTextFormatColl(SwTextFormatColl *pFormat,
                                const bool bResetListAttrs)
{
    SwTextFormatColl *pLocal = pFormat? pFormat: (*GetDoc()->GetTextFormatColls())[0];
    StartAllAction();

    RedlineFlags eRedlMode = GetDoc()->getIDocumentRedlineAccess().GetRedlineFlags(), eOldMode = eRedlMode;

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, pLocal->GetName());

    GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::SETFMTCOLL, &aRewriter);
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {

        if ( !rPaM.HasReadonlySel( GetViewOptions()->IsFormView() ) )
        {
            // tdf#105413 turn off ShowChanges mode for the next loops to apply styles permanently with redlining,
            // ie. in all directly preceding deleted paragraphs at the actual cursor positions
            if ( IDocumentRedlineAccess::IsShowChanges(eRedlMode) &&
               // is there redlining at beginning of the position (possible redline block before the modified node)
               GetDoc()->getIDocumentRedlineAccess().GetRedlinePos( (*rPaM.Start()).nNode.GetNode(), USHRT_MAX ) <
                   GetDoc()->getIDocumentRedlineAccess().GetRedlineTable().size() )
            {
                eRedlMode = RedlineFlags::ShowInsert | RedlineFlags::Ignore;
                GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eRedlMode );
            }

            // Change the paragraph style to pLocal and remove all direct paragraph formatting.
            GetDoc()->SetTextFormatColl(rPaM, pLocal, true, bResetListAttrs, GetLayout());

            // If there are hints on the nodes which cover the whole node, then remove those, too.
            SwPaM aPaM(*rPaM.Start(), *rPaM.End());
            if (SwTextNode* pEndTextNode = aPaM.End()->nNode.GetNode().GetTextNode())
            {
                aPaM.Start()->nContent = 0;
                aPaM.End()->nContent = pEndTextNode->GetText().getLength();
            }
            GetDoc()->RstTextAttrs(aPaM, /*bInclRefToxMark=*/false, /*bExactRange=*/true, GetLayout());
        }

    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::SETFMTCOLL, &aRewriter);
    EndAllAction();

    GetDoc()->getIDocumentRedlineAccess().SetRedlineFlags( eOldMode );
}

SwTextFormatColl* SwEditShell::MakeTextFormatColl(const OUString& rFormatCollName,
        SwTextFormatColl* pParent)
{
    SwTextFormatColl *pColl;
    if ( pParent == nullptr )
        pParent = &GetTextFormatColl(0);
    if (  (pColl=GetDoc()->MakeTextFormatColl(rFormatCollName, pParent)) == nullptr )
    {
        OSL_FAIL( "MakeTextFormatColl failed" );
    }
    return pColl;

}

void SwEditShell::FillByEx(SwTextFormatColl* pColl)
{
    SwPaM * pCursor = GetCursor();
    SwContentNode * pCnt = pCursor->GetContentNode();
    if (pCnt->IsTextNode()) // uhm... what nonsense would happen if not?
    {   // only need properties-node because BREAK/PAGEDESC filtered anyway!
        pCnt = sw::GetParaPropsNode(*GetLayout(), pCursor->GetPoint()->nNode);
    }
    const SfxItemSet* pSet = pCnt->GetpSwAttrSet();
    if( pSet )
    {
        // JP 05.10.98: Special treatment if one of the attributes Break/PageDesc/NumRule(auto) is
        //      in the ItemSet. Otherwise there will be too much or wrong processing (NumRules!)
        //      Bug 57568

        // Do NOT copy AutoNumRules into the template
        const SfxPoolItem* pItem;
        const SwNumRule* pRule = nullptr;
        if (SfxItemState::SET == pSet->GetItemState(RES_BREAK, false)
            || SfxItemState::SET == pSet->GetItemState(RES_PAGEDESC, false)
            || (SfxItemState::SET == pSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem)
                && nullptr != (pRule = GetDoc()->FindNumRulePtr(
                        static_cast<const SwNumRuleItem*>(pItem)->GetValue()))
                && pRule->IsAutoRule()))
        {
            SfxItemSet aSet( *pSet );
            aSet.ClearItem( RES_BREAK );
            aSet.ClearItem( RES_PAGEDESC );

            if (pRule
                || (SfxItemState::SET == pSet->GetItemState(RES_PARATR_NUMRULE, false, &pItem)
                    && nullptr != (pRule = GetDoc()->FindNumRulePtr(
                            static_cast<const SwNumRuleItem*>(pItem)->GetValue()))
                    && pRule->IsAutoRule()))
                aSet.ClearItem( RES_PARATR_NUMRULE );

            if( aSet.Count() )
                GetDoc()->ChgFormat(*pColl, aSet );
        }
        else
            GetDoc()->ChgFormat(*pColl, *pSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
