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
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>

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
#include <svl/cryptosign.hxx>
#include <vcl/svapp.hxx>

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
#include <rdfhelper.hxx>
#include <sfx2/watermarkitem.hxx>
#include <DocumentDrawModelManager.hxx>

#include <unoparagraph.hxx>
#include <unotextrange.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <modeltoviewhelper.hxx>
#include <strings.hrc>

#define WATERMARK_NAME "PowerPlusWaterMarkObject"

namespace
{

/// Find all page styles which are currently used in the document.
std::set<OUString> lcl_getUsedPageStyles(SwViewShell const * pShell)
{
    std::set<OUString> aRet;

    SwRootFrame* pLayout = pShell->GetLayout();
    for (SwFrame* pFrame = pLayout->GetLower(); pFrame; pFrame = pFrame->GetNext())
    {
        SwPageFrame* pPage = static_cast<SwPageFrame*>(pFrame);
        if (const SwPageDesc *pDesc = pPage->FindPageDesc())
            aRet.insert(pDesc->GetName());
    }

    return aRet;
}

/// Search for a field named rFieldName of type rServiceName in xText.
bool lcl_hasField(const uno::Reference<text::XText>& xText, const OUString& rServiceName, const OUString& rFieldName)
{
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
            if (aName != rFieldName)
                continue;

            return true;
        }
    }

    return false;
}

/// Search for a frame with WATERMARK_NAME in name of type rServiceName in xText. Returns found name in rShapeName.
uno::Reference<drawing::XShape> lcl_getWatermark(const uno::Reference<text::XText>& xText, const OUString& rServiceName, OUString& rShapeName)
{
    uno::Reference<container::XEnumerationAccess> xParagraphEnumerationAccess(xText, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphEnumerationAccess->createEnumeration();
    while (xParagraphs->hasMoreElements())
    {
        uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParagraphs->nextElement(), uno::UNO_QUERY);
        if (!xTextPortionEnumerationAccess.is())
            continue;

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

/// Validate and return validation result and signature field display text.
std::pair<bool, OUString>
lcl_MakeParagraphSignatureFieldText(const uno::Reference<frame::XModel>& xModel,
                                    const uno::Reference<css::text::XTextField>& xField,
                                    const OString& utf8Text)
{
    static const OUString metaNS("urn:bails");

    OUString msg = "Invalid Signature";
    bool valid = false;

    const css::uno::Reference<css::rdf::XResource> xSubject(xField, uno::UNO_QUERY);
    std::map<OUString, OUString> aStatements = SwRDFHelper::getStatements(xModel, metaNS, xSubject);
    const auto it = aStatements.find("loext:signature:signature");
    if (it != aStatements.end())
    {
        const sal_Char* pData = utf8Text.getStr();
        const std::vector<unsigned char> data(pData, pData + utf8Text.getLength());

        OString encSignature;
        if (it->second.convertToString(&encSignature, RTL_TEXTENCODING_UTF8, 0))
        {
            const std::vector<unsigned char> sig(svl::crypto::DecodeHexString(encSignature));
            SignatureInformation aInfo(0);
            valid = svl::crypto::Signing::Verify(data, false, sig, aInfo);
            valid = valid && aInfo.nStatus == css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;

            msg = SwResId(STR_SIGNED_BY) + ": " + aInfo.ouSubject + ", " + aInfo.ouDateTime + ": ";
            if (valid)
                msg += SwResId(STR_VALID);
            else
                msg += SwResId(STR_INVALID);
        }
    }

    return std::make_pair(valid, msg);
}

/// Updates the signature field text if changed and returns true only iff updated.
bool lcl_UpdateParagraphSignatureField(const uno::Reference<frame::XModel>& xModel,
                                       const uno::Reference<css::text::XTextField>& xField,
                                       const OString& utf8Text)
{
    const std::pair<bool, OUString> res = lcl_MakeParagraphSignatureFieldText(xModel, xField, utf8Text);
    uno::Reference<css::text::XTextRange> xText(xField, uno::UNO_QUERY);
    const OUString curText = xText->getString();
    if (curText != res.second)
    {
        xText->setString(res.second);
        return true;
    }

    return false;
}

void lcl_RemoveParagraphSignatureField(const uno::Reference<css::text::XTextField>& xField)
{
    uno::Reference<css::text::XTextContent> xFieldTextContent(xField, uno::UNO_QUERY);
    uno::Reference<css::text::XTextRange> xParagraph(xFieldTextContent->getAnchor());
    uno::Reference<css::text::XText> xParagraphText(xParagraph->getText(), uno::UNO_QUERY);
    xParagraphText->removeTextContent(xFieldTextContent);
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

SwTextFormatColl& SwEditShell::GetTextFormatColl( sal_uInt16 nFormatColl) const
{
    return *((*(GetDoc()->GetTextFormatColls()))[nFormatColl]);
}

void SwEditShell::SetClassification(const OUString& rName, SfxClassificationPolicyType eType)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;

    SfxClassificationHelper aHelper(pDocShell->getDocProperties());

    bool bHadWatermark = !aHelper.GetDocumentWatermark().isEmpty();

    // This updates the infobar as well.
    aHelper.SetBACName(rName, eType);

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

    std::set<OUString> aUsedPageStyles = lcl_getUsedPageStyles(this);
    for (const OUString& rPageStyleName : aUsedPageStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);
        const OUString aServiceName = "com.sun.star.text.TextField.DocInfo.Custom";
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
                if (!lcl_hasField(xHeaderText, aServiceName, SfxClassificationHelper::PROP_PREFIX_INTELLECTUALPROPERTY() + SfxClassificationHelper::PROP_DOCHEADER()))
                {
                    // Append a field to the end of the header text.
                    uno::Reference<beans::XPropertySet> xField(xMultiServiceFactory->createInstance(aServiceName), uno::UNO_QUERY);
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
            if (!lcl_hasField(xFooterText, aServiceName, sFooter))
            {
                // Append a field to the end of the footer text.
                uno::Reference<beans::XPropertySet> xField(xMultiServiceFactory->createInstance(aServiceName), uno::UNO_QUERY);
                xField->setPropertyValue(UNO_NAME_NAME, uno::makeAny(sFooter));
                uno::Reference<text::XTextContent> xTextContent(xField, uno::UNO_QUERY);
                xFooterText->insertTextContent(xFooterText->getEnd(), xTextContent, /*bAbsorb=*/false);
            }
        }
    }
}

sal_Int16 lcl_GetAngle(const drawing::HomogenMatrix3& rMatrix)
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
    std::set<OUString> aUsedPageStyles = lcl_getUsedPageStyles(this);
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
        uno::Reference<drawing::XShape> xWatermark = lcl_getWatermark(xHeaderText, aShapeServiceName, sWatermark);

        if (xWatermark.is())
        {
            SfxWatermarkItem aItem;
            uno::Reference<text::XTextRange> xTextRange(xWatermark, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xPropertySet(xWatermark, uno::UNO_QUERY);
            sal_uInt32 nColor;
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

void lcl_placeWatermarkInHeader(const SfxWatermarkItem& rWatermark,
                            const uno::Reference<frame::XModel>& xModel,
                            const uno::Reference<beans::XPropertySet>& xPageStyle,
                            const uno::Reference<text::XText>& xHeaderText,
                            sal_Int16 nLayer)
{
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
    OUString aShapeServiceName = "com.sun.star.drawing.CustomShape";
    OUString sWatermark = WATERMARK_NAME;
    uno::Reference<drawing::XShape> xWatermark = lcl_getWatermark(xHeaderText, aShapeServiceName, sWatermark);

    bool bDeleteWatermark = rWatermark.GetText().isEmpty();
    if (xWatermark.is())
    {
        drawing::HomogenMatrix3 aMatrix;
        sal_uInt32 nColor = 0xc0c0c0;
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

    if (!xWatermark.is() && !bDeleteWatermark)
    {
        OUString sFont = rWatermark.GetFont();
        sal_Int16 nAngle = rWatermark.GetAngle();
        sal_Int16 nTransparency = rWatermark.GetTransparency();
        sal_uInt32 nColor = rWatermark.GetColor();

        // Calc the ratio.
        double fRatio = 0;
        OutputDevice* pOut = Application::GetDefaultDevice();
        vcl::Font aFont(pOut->GetFont());
        aFont.SetFamilyName(sFont);
        auto nTextWidth = pOut->GetTextWidth(rWatermark.GetText());
        if (nTextWidth)
        {
            fRatio = aFont.GetFontSize().Height();
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
        sal_Int32 nHeight = nWidth * fRatio;

        // Create and insert the shape.
        uno::Reference<drawing::XShape> xShape(xMultiServiceFactory->createInstance(aShapeServiceName), uno::UNO_QUERY);
        basegfx::B2DHomMatrix aTransformation;
        aTransformation.identity();
        aTransformation.scale(nWidth, nHeight);
        aTransformation.rotate(F_PI180 * -1 * nAngle);
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
        xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT_RELATION, uno::makeAny(static_cast<sal_Int16>(text::RelOrientation::PAGE_PRINT_AREA)));
        xPropertySet->setPropertyValue(UNO_NAME_LINESTYLE, uno::makeAny(drawing::LineStyle_NONE));
        xPropertySet->setPropertyValue(UNO_NAME_OPAQUE, uno::makeAny(false));
        xPropertySet->setPropertyValue(UNO_NAME_TEXT_AUTOGROWHEIGHT, uno::makeAny(false));
        xPropertySet->setPropertyValue(UNO_NAME_TEXT_AUTOGROWWIDTH, uno::makeAny(false));
        xPropertySet->setPropertyValue(UNO_NAME_TEXT_MINFRAMEHEIGHT, uno::makeAny(nHeight));
        xPropertySet->setPropertyValue(UNO_NAME_TEXT_MINFRAMEWIDTH, uno::makeAny(nWidth));
        xPropertySet->setPropertyValue(UNO_NAME_TEXT_WRAP, uno::makeAny(text::WrapTextMode_THROUGH));
        xPropertySet->setPropertyValue(UNO_NAME_VERT_ORIENT_RELATION, uno::makeAny(static_cast<sal_Int16>(text::RelOrientation::PAGE_PRINT_AREA)));
        xPropertySet->setPropertyValue(UNO_NAME_CHAR_FONT_NAME, uno::makeAny(sFont));
        xPropertySet->setPropertyValue("Transformation", uno::makeAny(aMatrix));
        xPropertySet->setPropertyValue(UNO_NAME_HORI_ORIENT, uno::makeAny(static_cast<sal_Int16>(text::HoriOrientation::CENTER)));
        xPropertySet->setPropertyValue(UNO_NAME_VERT_ORIENT, uno::makeAny(static_cast<sal_Int16>(text::VertOrientation::CENTER)));
        xPropertySet->setPropertyValue(UNO_NAME_MISC_OBJ_LAYERID, uno::makeAny(nLayer));

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

        uno::Reference<container::XNamed> xNamed(xShape, uno::UNO_QUERY);
        xNamed->setName(sWatermark);
        xLockable->removeActionLock();
    }
}

void SwEditShell::SetWatermark(const SfxWatermarkItem& rWatermark)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);

    sal_Int16 nHeavenId = GetDoc()->getIDocumentDrawModelAccess().GetHeavenId().get();

    std::set<OUString> aUsedPageStyles = lcl_getUsedPageStyles(this);
    for (const OUString& rPageStyleName : aUsedPageStyles)
    {
        uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);

        // If the header is off, turn it on.
        bool bHeaderIsOn = false;
        xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
        if (!bHeaderIsOn)
            xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_ON, uno::makeAny(true));

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
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderText, nHeavenId);

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT_FIRST) >>= xHeaderTextFirst;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderTextFirst, nHeavenId);

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT_LEFT) >>= xHeaderTextLeft;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderTextLeft, nHeavenId);

        xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT_RIGHT) >>= xHeaderTextRight;
        lcl_placeWatermarkInHeader(rWatermark, xModel, xPageStyle, xHeaderTextRight, nHeavenId);

        // tdf#108494 the header height was switched to height of a watermark
        // and shape was moved to the lower part of a page
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_HEIGHT, uno::makeAny((sal_Int32)11));
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_HEIGHT, uno::makeAny(nOldValue));
        xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_DYNAMIC_HEIGHT, uno::Any(bDynamicHeight));
    }
}

void SwEditShell::SignParagraph(SwPaM* pPaM)
{
    if (!pPaM)
        return;
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;
    const SwPosition* pPosStart = pPaM->Start();
    if (!pPosStart)
        return;
    SwTextNode* pNode = pPosStart->nNode.GetNode().GetTextNode();
    if (!pNode)
        return;

    // 1. Get the text (without fields).
    const uno::Reference<text::XTextContent> xParent = SwXParagraph::CreateXParagraph(*pNode->GetDoc(), pNode);
    const OString utf8Text = lcl_getParagraphBodyText(xParent);
    if (utf8Text.isEmpty())
        return;

    // 2. Get certificate and SignatureInformation (needed to show signer name).
    //FIXME: Temporary until the Paragraph Signing Dialog is available.
    uno::Reference<uno::XComponentContext> xComponentContext = cppu::defaultBootstrap_InitialComponentContext();
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();
    if (!aCertificates.hasElements())
        return;

    uno::Reference<security::XCertificate> xCert = aCertificates[0];
    if (!xCert.is())
        return;

    // 3. Sign it.
    svl::crypto::Signing signing(xCert);
    signing.AddDataRange(utf8Text.getStr(), utf8Text.getLength());
    OStringBuffer sigBuf;
    if (!signing.Sign(sigBuf))
        return;

    const OString signature = sigBuf.makeStringAndClear();

    // 4. Add metadata
    static const OUString metaNS("urn:bails");
    static const OUString metaFile("bails.rdf");

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
    uno::Reference<css::text::XTextField> xField(xMultiServiceFactory->createInstance("com.sun.star.text.textfield.MetadataField"), uno::UNO_QUERY);

    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xContent->attach(xParent->getAnchor()->getStart());

    uno::Reference<rdf::XResource> xRes(xField, uno::UNO_QUERY);
    const OUString name = "loext:signature:signature";
    SwRDFHelper::addStatement(xModel, metaNS, metaFile, xRes, name, OStringToOUString(signature, RTL_TEXTENCODING_UTF8, 0));

    const std::pair<bool, OUString> res = lcl_MakeParagraphSignatureFieldText(xModel, xField, utf8Text);
    uno::Reference<css::text::XTextRange> xText(xField, uno::UNO_QUERY);
    xText->setString(res.second + " ");
}

void SwEditShell::ValidateParagraphSignatures(bool updateDontRemove)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell || m_bIsValidatingParagraphSignature)
        return;

    SwPaM* pPaM = GetCursor();
    const SwPosition* pPosStart = pPaM->Start();
    SwTextNode* pNode = pPosStart->nNode.GetNode().GetTextNode();
    if (!pNode)
        return;

    const uno::Reference<text::XTextContent> xParent = SwXParagraph::CreateXParagraph(*pNode->GetDoc(), pNode);

    // 1. Get the text (without fields).
    const OString utf8Text = lcl_getParagraphBodyText(xParent);
    if (utf8Text.isEmpty())
        return;

    // 2. For each signature field, update it.
    uno::Reference<container::XEnumerationAccess> xTextPortionEnumerationAccess(xParent, uno::UNO_QUERY);
    if (!xTextPortionEnumerationAccess.is())
        return;

    uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
    uno::Reference<container::XEnumeration> xTextPortions = xTextPortionEnumerationAccess->createEnumeration();
    m_bIsValidatingParagraphSignature = true;
    comphelper::ScopeGuard const g([this] () {
            m_bIsValidatingParagraphSignature = false;
        });

    while (xTextPortions->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xTextPortion(xTextPortions->nextElement(), uno::UNO_QUERY);
        OUString aTextPortionType;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_PORTION_TYPE) >>= aTextPortionType;
        if (aTextPortionType != UNO_NAME_TEXT_FIELD)
            continue;

        uno::Reference<lang::XServiceInfo> xTextField;
        xTextPortion->getPropertyValue(UNO_NAME_TEXT_FIELD) >>= xTextField;
        if (!xTextField->supportsService("com.sun.star.text.textfield.MetadataField"))
            continue;

        uno::Reference<text::XTextField> xContent(xTextField, uno::UNO_QUERY);

        const bool isUndoEnabled = GetDoc()->GetIDocumentUndoRedo().DoesUndo();
        GetDoc()->GetIDocumentUndoRedo().DoUndo(false);
        if (updateDontRemove)
            lcl_UpdateParagraphSignatureField(xModel, xContent, utf8Text);
        else if (!lcl_MakeParagraphSignatureFieldText(xModel, xContent, utf8Text).first)
            lcl_RemoveParagraphSignatureField(xContent);

        GetDoc()->GetIDocumentUndoRedo().DoUndo(isUndoEnabled);
    }
}

// #i62675#
void SwEditShell::SetTextFormatColl(SwTextFormatColl *pFormat,
                                const bool bResetListAttrs)
{
    SwTextFormatColl *pLocal = pFormat? pFormat: (*GetDoc()->GetTextFormatColls())[0];
    StartAllAction();

    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, pLocal->GetName());

    GetDoc()->GetIDocumentUndoRedo().StartUndo(SwUndoId::SETFMTCOLL, &aRewriter);
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {

        if ( !rPaM.HasReadonlySel( GetViewOptions()->IsFormView() ) )
        {
            // Change the paragraph style to pLocal and remove all direct paragraph formatting.
            GetDoc()->SetTextFormatColl( rPaM, pLocal, true, bResetListAttrs );

            // If there are hints on the nodes which cover the whole node, then remove those, too.
            SwPaM aPaM(*rPaM.Start(), *rPaM.End());
            if (SwTextNode* pEndTextNode = aPaM.End()->nNode.GetNode().GetTextNode())
            {
                aPaM.Start()->nContent = 0;
                aPaM.End()->nContent = pEndTextNode->GetText().getLength();
            }
            GetDoc()->RstTextAttrs(aPaM, /*bInclRefToxMark=*/false, /*bExactRange=*/true);
        }

    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo(SwUndoId::SETFMTCOLL, &aRewriter);
    EndAllAction();
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
    const SfxItemSet* pSet = pCnt->GetpSwAttrSet();
    if( pSet )
    {
        // JP 05.10.98: Special treatment if one of the attribues Break/PageDesc/NumRule(auto) is
        //      in the ItemSet. Otherwise there will be too much or wrong processing (NumRules!)
        //      Bug 57568

        // Do NOT copy AutoNumRules into the template
        const SfxPoolItem* pItem;
        const SwNumRule* pRule = nullptr;
        if( SfxItemState::SET == pSet->GetItemState( RES_BREAK, false ) ||
            SfxItemState::SET == pSet->GetItemState( RES_PAGEDESC,false ) ||
            ( SfxItemState::SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                false, &pItem ) && nullptr != (pRule = GetDoc()->FindNumRulePtr(
                static_cast<const SwNumRuleItem*>(pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() )
            )
        {
            SfxItemSet aSet( *pSet );
            aSet.ClearItem( RES_BREAK );
            aSet.ClearItem( RES_PAGEDESC );

            if( pRule || (SfxItemState::SET == pSet->GetItemState( RES_PARATR_NUMRULE,
                false, &pItem ) && nullptr != (pRule = GetDoc()->FindNumRulePtr(
                static_cast<const SwNumRuleItem*>(pItem)->GetValue() )) &&
                pRule && pRule->IsAutoRule() ))
                aSet.ClearItem( RES_PARATR_NUMRULE );

            if( aSet.Count() )
                GetDoc()->ChgFormat(*pColl, aSet );
        }
        else
            GetDoc()->ChgFormat(*pColl, *pSet );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
