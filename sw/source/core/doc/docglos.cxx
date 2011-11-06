/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <shellio.hxx>
#include <pam.hxx>
#include <swundo.hxx>
#include <ndtxt.hxx>
#include <acorrect.hxx>
#include <crsrsh.hxx>
#include <docsh.hxx>


using namespace ::com::sun::star;


/// copy document properties via public interface
static void lcl_copyDocumentProperties(
        uno::Reference<document::XDocumentProperties> i_xSource,
        uno::Reference<document::XDocumentProperties> i_xTarget) {
    DBG_ASSERT(i_xSource.is(), "null reference");
    DBG_ASSERT(i_xTarget.is(), "null reference");

    i_xTarget->setAuthor(i_xSource->getAuthor());
    i_xTarget->setGenerator(i_xSource->getGenerator());
    i_xTarget->setCreationDate(i_xSource->getCreationDate());
    i_xTarget->setTitle(i_xSource->getTitle());
    i_xTarget->setSubject(i_xSource->getSubject());
    i_xTarget->setDescription(i_xSource->getDescription());
    i_xTarget->setKeywords(i_xSource->getKeywords());
    i_xTarget->setLanguage(i_xSource->getLanguage());
    i_xTarget->setModifiedBy(i_xSource->getModifiedBy());
    i_xTarget->setModificationDate(i_xSource->getModificationDate());
    i_xTarget->setPrintedBy(i_xSource->getPrintedBy());
    i_xTarget->setPrintDate(i_xSource->getPrintDate());
    i_xTarget->setTemplateName(i_xSource->getTemplateName());
    i_xTarget->setTemplateURL(i_xSource->getTemplateURL());
    i_xTarget->setTemplateDate(i_xSource->getTemplateDate());
    i_xTarget->setAutoloadURL(i_xSource->getAutoloadURL());
    i_xTarget->setAutoloadSecs(i_xSource->getAutoloadSecs());
    i_xTarget->setDefaultTarget(i_xSource->getDefaultTarget());
    i_xTarget->setDocumentStatistics(i_xSource->getDocumentStatistics());
    i_xTarget->setEditingCycles(i_xSource->getEditingCycles());
    i_xTarget->setEditingDuration(i_xSource->getEditingDuration());

    uno::Reference<beans::XPropertySet> xSourceUDSet(
        i_xSource->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertyContainer> xTargetUD(
        i_xTarget->getUserDefinedProperties());
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
    try {
        uno::Reference<beans::XPropertySetInfo> xSetInfo
            = xSourceUDSet->getPropertySetInfo();
        uno::Sequence<beans::Property> srcprops = xSetInfo->getProperties();
        for (sal_Int32 i = 0; i < srcprops.getLength(); ++i) {
            ::rtl::OUString name = srcprops[i].Name;
            xTargetUD->addProperty(name, srcprops[i].Attributes,
                xSourceUDSet->getPropertyValue(name));
        }
    } catch (uno::Exception &) {
        // ignore
    }
}

/* -----------------22.07.99 11:47-------------------
    Description: inserts an AutoText block
 --------------------------------------------------*/
sal_Bool SwDoc::InsertGlossary( SwTextBlocks& rBlock, const String& rEntry,
                            SwPaM& rPaM, SwCrsrShell* pShell )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nIdx = rBlock.GetIndex( rEntry );
    if( (sal_uInt16) -1 != nIdx )
    {
        // Bug #70238# ask the TextOnly-Flag before BeginGetDoc, because
        //              the method closed the Storage!
        sal_Bool bSav_IsInsGlossary = mbInsOnlyTxtGlssry;
        mbInsOnlyTxtGlssry = rBlock.IsOnlyTextBlock( nIdx );

        if( rBlock.BeginGetDoc( nIdx ) )
        {
            SwDoc* pGDoc = rBlock.GetDoc();

            // alle FixFelder aktualisieren. Dann aber auch mit der
            // richtigen DocInfo!
            // FIXME: UGLY: Because we cannot limit the range in which to do
            // field updates, we must update the fixed fields at the glossary
            // entry document.
            // To be able to do this, we copy the document properties of the
            // target document to the glossary document
//            DBG_ASSERT(GetDocShell(), "no SwDocShell"); // may be clipboard!
            DBG_ASSERT(pGDoc->GetDocShell(), "no SwDocShell at glossary");
            if (GetDocShell() && pGDoc->GetDocShell()) {
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps(
                    xDPS->getDocumentProperties() );
                uno::Reference<document::XDocumentPropertiesSupplier> xGlosDPS(
                    pGDoc->GetDocShell()->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xGlosDocProps(
                    xGlosDPS->getDocumentProperties() );
                lcl_copyDocumentProperties(xDocProps, xGlosDocProps);
        }
            pGDoc->SetFixFields(false, NULL);

            //StartAllAction();
            LockExpFlds();

            SwNodeIndex aStt( pGDoc->GetNodes().GetEndOfExtras(), 1 );
            SwCntntNode* pCntntNd = pGDoc->GetNodes().GoNext( &aStt );
            const SwTableNode* pTblNd = pCntntNd->FindTableNode();
            SwPaM aCpyPam( pTblNd ? *(SwNode*)pTblNd : *(SwNode*)pCntntNd );
            aCpyPam.SetMark();

            // dann bis zum Ende vom Nodes Array
            aCpyPam.GetPoint()->nNode = pGDoc->GetNodes().GetEndOfContent().GetIndex()-1;
            pCntntNd = aCpyPam.GetCntntNode();
            aCpyPam.GetPoint()->nContent.Assign( pCntntNd, pCntntNd->Len() );

            GetIDocumentUndoRedo().StartUndo( UNDO_INSGLOSSARY, NULL );
            SwPaM *_pStartCrsr = &rPaM, *__pStartCrsr = _pStartCrsr;
            do {

                SwPosition& rInsPos = *_pStartCrsr->GetPoint();
                SwStartNode* pBoxSttNd = (SwStartNode*)rInsPos.nNode.GetNode().
                                            FindTableBoxStartNode();

                if( pBoxSttNd && 2 == pBoxSttNd->EndOfSectionIndex() -
                                      pBoxSttNd->GetIndex() &&
                    aCpyPam.GetPoint()->nNode != aCpyPam.GetMark()->nNode )
                {
                    // es wird mehr als 1 Node in die akt. Box kopiert.
                    // Dann muessen die BoxAttribute aber entfernt werden.
                    ClearBoxNumAttrs( rInsPos.nNode );
                }

                SwDontExpandItem aACD;
                aACD.SaveDontExpandItems( rInsPos );

                pGDoc->CopyRange( aCpyPam, rInsPos, false );

                aACD.RestoreDontExpandItems( rInsPos );
                if( pShell )
                    pShell->SaveTblBoxCntnt( &rInsPos );
            } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) !=
                        __pStartCrsr );
            GetIDocumentUndoRedo().EndUndo( UNDO_INSGLOSSARY, NULL );

            UnlockExpFlds();
            if( !IsExpFldsLocked() )
                UpdateExpFlds(NULL, true);
            bRet = sal_True;
        }
        mbInsOnlyTxtGlssry = bSav_IsInsGlossary;
    }
    rBlock.EndGetDoc();
    return bRet;
}


