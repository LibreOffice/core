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

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include <osl/diagnose.h>

#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <shellio.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <acorrect.hxx>
#include <crsrsh.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;

void SwDoc::ReplaceUserDefinedDocumentProperties(
        const uno::Reference<document::XDocumentProperties>& xSourceDocProps)
{
    OSL_ENSURE(xSourceDocProps.is(), "null reference");

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties() );
    OSL_ENSURE(xDocProps.is(), "null reference");

    uno::Reference<beans::XPropertySet> xSourceUDSet(
        xSourceDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyContainer> xTargetUD(
        xDocProps->getUserDefinedProperties());
    uno::Reference<beans::XPropertySet> xTargetUDSet(xTargetUD,
        uno::UNO_QUERY_THROW);
    uno::Sequence<beans::Property> tgtprops
        = xTargetUDSet->getPropertySetInfo()->getProperties();

    for (sal_Int32 i = 0; i < tgtprops.getLength(); ++i) {
        try {
            xTargetUD->removeProperty(tgtprops [i].Name);
        } catch (uno::Exception &) {
            // ignore
        }
    }

    uno::Reference<beans::XPropertySetInfo> xSetInfo
        = xSourceUDSet->getPropertySetInfo();
    uno::Sequence<beans::Property> srcprops = xSetInfo->getProperties();

    for (sal_Int32 i = 0; i < srcprops.getLength(); ++i) {
        try {
            OUString name = srcprops[i].Name;
            xTargetUD->addProperty(name, srcprops[i].Attributes,
                xSourceUDSet->getPropertyValue(name));
        } catch (uno::Exception &) {
            // ignore
        }
    }
}

void SwDoc::ReplaceDocumentProperties(const SwDoc& rSource, bool mailMerge)
{
    uno::Reference<document::XDocumentPropertiesSupplier> xSourceDPS(
        rSource.GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xSourceDocProps(
        xSourceDPS->getDocumentProperties() );
    OSL_ENSURE(xSourceDocProps.is(), "null reference");

    uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
        GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
    uno::Reference<document::XDocumentProperties> xDocProps(
        xDPS->getDocumentProperties() );
    OSL_ENSURE(xDocProps.is(), "null reference");

    xDocProps->setAuthor(xSourceDocProps->getAuthor());
    xDocProps->setGenerator(xSourceDocProps->getGenerator());
    xDocProps->setCreationDate(xSourceDocProps->getCreationDate());
    xDocProps->setTitle(xSourceDocProps->getTitle());
    xDocProps->setSubject(xSourceDocProps->getSubject());
    xDocProps->setDescription(xSourceDocProps->getDescription());
    xDocProps->setKeywords(xSourceDocProps->getKeywords());
    xDocProps->setLanguage(xSourceDocProps->getLanguage());
    // Note: These below originally weren't copied for mailmerge, but I don't see why not.
    xDocProps->setModifiedBy(xSourceDocProps->getModifiedBy());
    xDocProps->setModificationDate(xSourceDocProps->getModificationDate());
    xDocProps->setPrintedBy(xSourceDocProps->getPrintedBy());
    xDocProps->setPrintDate(xSourceDocProps->getPrintDate());
    xDocProps->setTemplateName(xSourceDocProps->getTemplateName());
    xDocProps->setTemplateURL(xSourceDocProps->getTemplateURL());
    xDocProps->setTemplateDate(xSourceDocProps->getTemplateDate());
    xDocProps->setAutoloadURL(xSourceDocProps->getAutoloadURL());
    xDocProps->setAutoloadSecs(xSourceDocProps->getAutoloadSecs());
    xDocProps->setDefaultTarget(xSourceDocProps->getDefaultTarget());
    xDocProps->setDocumentStatistics(xSourceDocProps->getDocumentStatistics());
    xDocProps->setEditingCycles(xSourceDocProps->getEditingCycles());
    xDocProps->setEditingDuration(xSourceDocProps->getEditingDuration());

    if( mailMerge ) // Note: Not sure this is needed.
    {
        // Manually set the creation date, otherwise author field isn't filled
        // during MM, as it's set when saving the document the first time.
        xDocProps->setCreationDate( xSourceDocProps->getModificationDate() );
    }

    ReplaceUserDefinedDocumentProperties( xSourceDocProps );
}

/// inserts an AutoText block
bool SwDoc::InsertGlossary( SwTextBlocks& rBlock, const OUString& rEntry,
                            SwPaM& rPaM, SwCursorShell* pShell )
{
    bool bRet = false;
    const sal_uInt16 nIdx = rBlock.GetIndex( rEntry );
    if( USHRT_MAX != nIdx )
    {
        bool bSav_IsInsGlossary = mbInsOnlyTextGlssry;
        mbInsOnlyTextGlssry = rBlock.IsOnlyTextBlock( nIdx );

        if( rBlock.BeginGetDoc( nIdx ) )
        {
            SwDoc* pGDoc = rBlock.GetDoc();

            // Update all fixed fields, with the right DocInfo.
            // FIXME: UGLY: Because we cannot limit the range in which to do
            // field updates, we must update the fixed fields at the glossary
            // entry document.
            // To be able to do this, we copy the document properties of the
            // target document to the glossary document
            // OSL_ENSURE(GetDocShell(), "no SwDocShell"); // may be clipboard!
            OSL_ENSURE(pGDoc->GetDocShell(), "no SwDocShell at glossary");
            if (GetDocShell() && pGDoc->GetDocShell())
                pGDoc->ReplaceDocumentProperties( *this );
            pGDoc->getIDocumentFieldsAccess().SetFixFields(nullptr);

            // StartAllAction();
            getIDocumentFieldsAccess().LockExpFields();

            SwNodeIndex aStt( pGDoc->GetNodes().GetEndOfExtras(), 1 );
            SwContentNode* pContentNd = pGDoc->GetNodes().GoNext( &aStt );
            const SwTableNode* pTableNd = pContentNd->FindTableNode();
            SwPaM aCpyPam( pTableNd ? *const_cast<SwNode*>(static_cast<SwNode const *>(pTableNd)) : *static_cast<SwNode*>(pContentNd) );
            aCpyPam.SetMark();

            // till the nodes array's end
            aCpyPam.GetPoint()->nNode = pGDoc->GetNodes().GetEndOfContent().GetIndex()-1;
            pContentNd = aCpyPam.GetContentNode();
            aCpyPam.GetPoint()->nContent.Assign(
                    pContentNd, pContentNd ? pContentNd->Len() : 0 );

            GetIDocumentUndoRedo().StartUndo( SwUndoId::INSGLOSSARY, nullptr );
            SwPaM *_pStartCursor = &rPaM, *_pStartCursor2 = _pStartCursor;
            do {

                SwPosition& rInsPos = *_pStartCursor->GetPoint();
                SwStartNode* pBoxSttNd = const_cast<SwStartNode*>(rInsPos.nNode.GetNode().
                                            FindTableBoxStartNode());

                if( pBoxSttNd && 2 == pBoxSttNd->EndOfSectionIndex() -
                                      pBoxSttNd->GetIndex() &&
                    aCpyPam.GetPoint()->nNode != aCpyPam.GetMark()->nNode )
                {
                    // We copy more than one Node to the current Box.
                    // However, we have to remove the BoxAttributes then.
                    ClearBoxNumAttrs( rInsPos.nNode );
                }

                SwDontExpandItem aACD;
                aACD.SaveDontExpandItems( rInsPos );

                pGDoc->getIDocumentContentOperations().CopyRange( aCpyPam, rInsPos, /*bCopyAll=*/false, /*bCheckPos=*/true );

                aACD.RestoreDontExpandItems( rInsPos );
                if( pShell )
                    pShell->SaveTableBoxContent( &rInsPos );
            } while( (_pStartCursor = _pStartCursor->GetNext()) !=
                        _pStartCursor2 );
            GetIDocumentUndoRedo().EndUndo( SwUndoId::INSGLOSSARY, nullptr );

            getIDocumentFieldsAccess().UnlockExpFields();
            if( !getIDocumentFieldsAccess().IsExpFieldsLocked() )
                getIDocumentFieldsAccess().UpdateExpFields(nullptr, true);
            bRet = true;
        }
        mbInsOnlyTextGlssry = bSav_IsInsGlossary;
    }
    rBlock.EndGetDoc();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
