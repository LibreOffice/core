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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <hintids.hxx>
#include <editeng/formatbreakitem.hxx>
#include <sfx2/classificationhelper.hxx>
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

namespace
{

/// Find all page styles which are currently used in the document.
std::set<OUString> lcl_getUsedPageStyles(SwViewShell* pShell)
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

} // anonymous namespace

SwTextFormatColl& SwEditShell::GetDfltTextFormatColl() const
{
    return *static_cast<SwTextFormatColl*>( (GetDoc()->GetDfltTextFormatColl()));
}

sal_uInt16 SwEditShell::GetTextFormatCollCount() const
{
    return GetDoc()->GetTextFormatColls()->size();
}

SwTextFormatColl& SwEditShell::GetTextFormatColl( sal_uInt16 nFormatColl) const
{
    return *((*(GetDoc()->GetTextFormatColls()))[nFormatColl]);
}

void SwEditShell::SetClassification(const OUString& rName)
{
    SwDocShell* pDocShell = GetDoc()->GetDocShell();
    if (!pDocShell)
        return;

    SfxClassificationHelper aHelper(*pDocShell);
    // This updates the infobar as well.
    aHelper.SetBACName(rName);

    bool bHeaderIsNeeded = aHelper.HasDocumentHeader();
    bool bFooterIsNeeded = aHelper.HasDocumentFooter();

    if (bHeaderIsNeeded || bFooterIsNeeded)
    {
        uno::Reference<frame::XModel> xModel = pDocShell->GetBaseModel();
        uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(xModel, uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamilies(xStyleFamiliesSupplier->getStyleFamilies(), uno::UNO_QUERY);
        uno::Reference<container::XNameAccess> xStyleFamily(xStyleFamilies->getByName("PageStyles"), uno::UNO_QUERY);

        std::set<OUString> aUsedPageStyles = lcl_getUsedPageStyles(this);
        for (const OUString& rPageStyleName : aUsedPageStyles)
        {
            uno::Reference<beans::XPropertySet> xPageStyle(xStyleFamily->getByName(rPageStyleName), uno::UNO_QUERY);
            OUString aServiceName = "com.sun.star.text.TextField.DocInfo.Custom";

            if (bHeaderIsNeeded)
            {
                // If the header is off, turn it on.
                bool bHeaderIsOn = false;
                xPageStyle->getPropertyValue(UNO_NAME_HEADER_IS_ON) >>= bHeaderIsOn;
                if (!bHeaderIsOn)
                    xPageStyle->setPropertyValue(UNO_NAME_HEADER_IS_ON, uno::makeAny(true));

                // If the header already contains a document header field, no need to do anything.
                uno::Reference<text::XText> xHeaderText;
                xPageStyle->getPropertyValue(UNO_NAME_HEADER_TEXT) >>= xHeaderText;
                if (!lcl_hasField(xHeaderText, aServiceName, SfxClassificationHelper::PROP_DOCHEADER()))
                {
                    // Append a field to the end of the header text.
                    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xField(xMultiServiceFactory->createInstance(aServiceName), uno::UNO_QUERY);
                    xField->setPropertyValue(UNO_NAME_NAME, uno::makeAny(SfxClassificationHelper::PROP_DOCHEADER()));
                    uno::Reference<text::XTextContent> xTextContent(xField, uno::UNO_QUERY);
                    xHeaderText->insertTextContent(xHeaderText->getEnd(), xTextContent, /*bAbsorb=*/false);
                }
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
                if (!lcl_hasField(xFooterText, aServiceName, SfxClassificationHelper::PROP_DOCFOOTER()))
                {
                    // Append a field to the end of the footer text.
                    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xModel, uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xField(xMultiServiceFactory->createInstance(aServiceName), uno::UNO_QUERY);
                    xField->setPropertyValue(UNO_NAME_NAME, uno::makeAny(SfxClassificationHelper::PROP_DOCFOOTER()));
                    uno::Reference<text::XTextContent> xTextContent(xField, uno::UNO_QUERY);
                    xFooterText->insertTextContent(xFooterText->getEnd(), xTextContent, /*bAbsorb=*/false);
                }
            }
        }
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

    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_SETFMTCOLL, &aRewriter);
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
    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_SETFMTCOLL, &aRewriter);
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

void SwEditShell::FillByEx(SwTextFormatColl* pColl, bool bReset)
{
    if( bReset )
    {
        pColl->ResetAllFormatAttr();
    }

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
