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

#include <vector>
#include <libxml/xmlwriter.h>
#include <sfx2/docfile.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdundo.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svx/svditer.hxx>
#include <com/sun/star/text/XTextCopy.hpp>
#include <tools/debug.hxx>
#include <svx/svddef.hxx>

#include <Annotation.hxx>
#include <notifydocumentevent.hxx>
#include <sdresid.hxx>
#include <sdpage.hxx>
#include <glob.hxx>
#include <strings.hrc>
#include <drawdoc.hxx>
#include <stlpool.hxx>
#include <pglink.hxx>

#include <strings.hxx>
#include <DrawDocShell.hxx>

#include <svl/itemset.hxx>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;

/*************************************************************************
|*
|* Sets: names of layout, master page links and templates for presentation
|* objects
|*
|* Preconditions: - The page has to know the correct model!
|*                - The corresponding master page has to be in the model.
|*                - The corresponding style sheets have to be in the style sheet
|*                  pool.
|*
|*  bReplaceStyleSheets = sal_True : Named style sheets are replaced
|*                        sal_False: All style sheets are reassigned
|*
|*  bSetMasterPage      = sal_True : search and assign master page
|*
|*  bReverseOrder       = sal_False: search master page from head to tail
|*                        sal_True : search master page from tail to head
|*                                   (for undo operations)
|*
\************************************************************************/

void SdPage::SetPresentationLayout(const OUString& rLayoutName,
                                   bool bReplaceStyleSheets,
                                   bool bSetMasterPage,
                                   bool bReverseOrder)
{
    /*********************************************************************
    |* Name of the layout of the page
    \********************************************************************/
    OUString aOldLayoutName(maLayoutName);    // memorize
    OUStringBuffer aBuf(rLayoutName);
    aBuf.append(SD_LT_SEPARATOR).append(STR_LAYOUT_OUTLINE);
    maLayoutName = aBuf.makeStringAndClear();

    /*********************************************************************
    |* search and replace master page if necessary
    \********************************************************************/
    if (bSetMasterPage && !IsMasterPage())
    {
        SdPage* pMaster;
        SdPage* pFoundMaster = nullptr;
        sal_uInt16 nMaster = 0;
        sal_uInt16 nMasterCount = getSdrModelFromSdrPage().GetMasterPageCount();

        if( !bReverseOrder )
        {
            for ( nMaster = 0; nMaster < nMasterCount; nMaster++ )
            {
                pMaster = static_cast<SdPage*>(getSdrModelFromSdrPage().GetMasterPage(nMaster));
                if (pMaster->GetPageKind() == mePageKind && pMaster->GetLayoutName() == maLayoutName)
                {
                    pFoundMaster = pMaster;
                    break;
                }
            }
        }
        else
        {
            for ( nMaster = nMasterCount; nMaster > 0; nMaster-- )
            {
                pMaster = static_cast<SdPage*>(getSdrModelFromSdrPage().GetMasterPage(nMaster - 1));
                if (pMaster->GetPageKind() == mePageKind && pMaster->GetLayoutName() == maLayoutName)
                {
                    pFoundMaster = pMaster;
                    break;
                }
            }
        }

        DBG_ASSERT(pFoundMaster, "Masterpage for presentation layout not found!");

        // this should never happen, but we play failsafe here
        if( pFoundMaster == nullptr )
            pFoundMaster = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetSdPage( 0, mePageKind );

        if( pFoundMaster )
            TRG_SetMasterPage(*pFoundMaster);
    }

    /*********************************************************************
    |* templates for presentation objects
    \********************************************************************/
    // list with:
    // - pointer to templates for outline text object (old and new templates)
    // - replace-data for OutlinerParaObject
    std::vector<SfxStyleSheetBase*> aOutlineStyles;
    std::vector<SfxStyleSheetBase*> aOldOutlineStyles;
    std::vector<StyleReplaceData> aReplList;
    bool bListsFilled = false;

    const size_t nObjCount = GetObjCount();

    for (size_t nObj = 0; nObj < nObjCount; ++nObj)
    {
        auto pObj = GetObj(nObj);

        if (pObj->GetObjInventor() == SdrInventor::Default &&
            pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            if (!bListsFilled || !bReplaceStyleSheets)
            {
                OUString aFullName;
                OUString aOldFullName;
                SfxStyleSheetBase* pSheet = nullptr;
                SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();

                for (sal_Int16 i = -1; i < 9; i++)
                {
                    aOldFullName = aOldLayoutName + " " +
                                OUString::number( (i <= 0 ) ? 1 : i + 1 );
                    aFullName = maLayoutName + " " +
                                OUString::number( (i <= 0 ) ? 1 : i + 1);
                    pSheet = pStShPool->Find(aOldFullName, SfxStyleFamily::Page);
                    DBG_ASSERT(pSheet, "Old outline style sheet not found");
                    aOldOutlineStyles.push_back(pSheet);

                    pSheet = pStShPool->Find(aFullName, SfxStyleFamily::Page);
                    DBG_ASSERT(pSheet, "New outline style sheet not found");
                    aOutlineStyles.push_back(pSheet);

                    if (bReplaceStyleSheets && pSheet)
                    {
                        // Replace instead Set
                        StyleReplaceData aReplData;
                        aReplData.nNewFamily = pSheet->GetFamily();
                        aReplData.nFamily    = pSheet->GetFamily();
                        aReplData.aNewName   = aFullName;
                        aReplData.aName      = aOldFullName;
                        aReplList.push_back(aReplData);
                    }
                    else
                    {
                        OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();

                        if( pOPO )
                            pOPO->SetStyleSheets( i,  aFullName, SfxStyleFamily::Page );
                    }
                }

                bListsFilled = true;
            }


            std::vector<SfxStyleSheetBase*>::iterator iterOldOut = aOldOutlineStyles.begin();

            for (auto& rpOut : aOutlineStyles)
            {
                SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>(rpOut);
                SfxStyleSheet* pOldSheet = static_cast<SfxStyleSheet*>(*iterOldOut);

                if (pSheet != pOldSheet)
                {
                    if (pOldSheet)
                        pObj->EndListening(*pOldSheet);

                    if (pSheet && !pObj->IsListening(*pSheet))
                        pObj->StartListening(*pSheet);
                }

                ++iterOldOut;
            }

            OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
            if ( bReplaceStyleSheets && pOPO )
            {
                for (const auto& rRepl : aReplList)
                {
                    pOPO->ChangeStyleSheets( rRepl.aName, rRepl.nFamily, rRepl.aNewName, rRepl.nNewFamily );
                }
            }
        }
        else if (pObj->GetObjInventor() == SdrInventor::Default &&
                 pObj->GetObjIdentifier() == OBJ_TITLETEXT)
        {
            // We do net get PresObjKind via GetPresObjKind() since there are
            // only PresObjListe considered. But we want to consider all "Title
            // objects" here (paste from clipboard etc.)
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_TITLE);

            if (pSheet)
                pObj->SetStyleSheet(pSheet, true);
        }
        else
        {
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(GetPresObjKind(pObj));

            if (pSheet)
                pObj->SetStyleSheet(pSheet, true);
        }
    }
}

/*************************************************************************
|*
|* disconnect outline text object from templates for outline levels
|*
\************************************************************************/

void SdPage::EndListenOutlineText()
{
    SdrObject* pOutlineTextObj = GetPresObj(PRESOBJ_OUTLINE);

    if (!pOutlineTextObj)
        return;

    SdStyleSheetPool* pSPool = static_cast<SdStyleSheetPool*>(getSdrModelFromSdrPage().GetStyleSheetPool());
    DBG_ASSERT(pSPool, "StyleSheetPool missing");
    OUString aTrueLayoutName(maLayoutName);
    sal_Int32 nIndex = aTrueLayoutName.indexOf( SD_LT_SEPARATOR );
    if( nIndex != -1 )
        aTrueLayoutName = aTrueLayoutName.copy(0, nIndex);

    std::vector<SfxStyleSheetBase*> aOutlineStyles;
    pSPool->CreateOutlineSheetList(aTrueLayoutName,aOutlineStyles);

    for (auto& rpStyle : aOutlineStyles)
    {
        SfxStyleSheet *pSheet = static_cast<SfxStyleSheet*>(rpStyle);
        pOutlineTextObj->EndListening(*pSheet);
    }
}

/*************************************************************************
|*
|* Is this page read-only?
|*
\************************************************************************/

bool SdPage::IsReadOnly() const
{
    return false;
}

/*************************************************************************
|*
|* Connect to sfx2::LinkManager
|*
\************************************************************************/

void SdPage::ConnectLink()
{
    sfx2::LinkManager* pLinkManager(getSdrModelFromSdrPage().GetLinkManager());

    if (!(pLinkManager && !mpPageLink && !maFileName.isEmpty() && !maBookmarkName.isEmpty() &&
        mePageKind==PageKind::Standard && !IsMasterPage() &&
        static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).IsNewOrLoadCompleted()))
        return;

    /**********************************************************************
    * Connect
    * Only standard pages are allowed to be linked
    **********************************************************************/
    ::sd::DrawDocShell* pDocSh = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()).GetDocSh();

    if (!pDocSh || pDocSh->GetMedium()->GetOrigURL() != maFileName)
    {
        // No links to document owned pages!
        mpPageLink = new SdPageLink(this, maFileName, maBookmarkName);
        OUString aFilterName(SdResId(STR_IMPRESS));
        pLinkManager->InsertFileLink(*mpPageLink, OBJECT_CLIENT_FILE,
                                     maFileName, &aFilterName, &maBookmarkName);
        mpPageLink->Connect();
    }
}

/*************************************************************************
|*
|* Disconnect from sfx2::LinkManager
|*
\************************************************************************/

void SdPage::DisconnectLink()
{
    sfx2::LinkManager* pLinkManager(getSdrModelFromSdrPage().GetLinkManager());

    if (pLinkManager && mpPageLink)
    {
        /**********************************************************************
        * Disconnect
        * (remove deletes *pGraphicLink implicit)
        **********************************************************************/
        pLinkManager->Remove(mpPageLink);
        mpPageLink=nullptr;
    }
}

void SdPage::lateInit(const SdPage& rSrcPage)
{
    // call parent
    FmFormPage::lateInit(rSrcPage);

    // copy local variables (former stuff from copy constructor)
    mePageKind = rSrcPage.mePageKind;
    meAutoLayout = rSrcPage.meAutoLayout;
    mbSelected = false;
    mnTransitionType = rSrcPage.mnTransitionType;
    mnTransitionSubtype = rSrcPage.mnTransitionSubtype;
    mbTransitionDirection = rSrcPage.mbTransitionDirection;
    mnTransitionFadeColor = rSrcPage.mnTransitionFadeColor;
    mfTransitionDuration = rSrcPage.mfTransitionDuration;
    mePresChange = rSrcPage.mePresChange;
    mfTime = rSrcPage.mfTime;
    mbSoundOn = rSrcPage.mbSoundOn;
    mbExcluded = rSrcPage.mbExcluded;
    maLayoutName = rSrcPage.maLayoutName;
    maSoundFile = rSrcPage.maSoundFile;
    mbLoopSound = rSrcPage.mbLoopSound;
    mbStopSound = rSrcPage.mbStopSound;
    maCreatedPageName.clear();
    maFileName = rSrcPage.maFileName;
    maBookmarkName = rSrcPage.maBookmarkName;
    mbScaleObjects = rSrcPage.mbScaleObjects;
    mbBackgroundFullSize = rSrcPage.mbBackgroundFullSize;
    meCharSet = rSrcPage.meCharSet;
    mnPaperBin = rSrcPage.mnPaperBin;
    mpPageLink = nullptr;    // is set when inserting via ConnectLink()
    mbIsPrecious = false;

    // use shape list directly to preserve constness of rSrcPage
    const std::list< SdrObject* >& rShapeList = rSrcPage.maPresentationShapeList.getList();
    for( SdrObject* pObj : rShapeList )
    {
        InsertPresObj(GetObj(pObj->GetOrdNum()), rSrcPage.GetPresObjKind(pObj));
    }

    // header footer
    setHeaderFooterSettings( rSrcPage.getHeaderFooterSettings() );

    // animations
    rSrcPage.cloneAnimations(*this);

    // annotations
    for(const Reference< XAnnotation >& srcAnnotation : rSrcPage.maAnnotations)
    {
        Reference< XAnnotation > ref;
        createAnnotation(ref);
        ref->setPosition(srcAnnotation->getPosition());
        ref->setSize(srcAnnotation->getSize());
        ref->setAuthor(srcAnnotation->getAuthor());
        ref->setInitials(srcAnnotation->getInitials());
        ref->setDateTime(srcAnnotation->getDateTime());
        Reference< ::css::text::XTextCopy > srcRange ( srcAnnotation->getTextRange(), uno::UNO_QUERY);
        Reference< ::css::text::XTextCopy > range ( ref->getTextRange(), uno::UNO_QUERY);
        if(srcRange.is() && range.is())
            range->copyText( srcRange );
    }

    // fix user calls for duplicated slide
    SdrObjListIter aSourceIter( &rSrcPage, SdrIterMode::DeepWithGroups );
    SdrObjListIter aTargetIter( this, SdrIterMode::DeepWithGroups );

    while( aSourceIter.IsMore() && aTargetIter.IsMore() )
    {
        SdrObject* pSource = aSourceIter.Next();
        SdrObject* pTarget = aTargetIter.Next();

        if( pSource->GetUserCall() )
            pTarget->SetUserCall(this);
    }
}

/*************************************************************************
|*
|* Clone
|*
\************************************************************************/

SdrPage* SdPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    SdDrawDocument& rSdDrawDocument(static_cast< SdDrawDocument& >(rTargetModel));
    SdPage* pClonedSdPage(
        new SdPage(
            rSdDrawDocument,
            IsMasterPage()));
    pClonedSdPage->lateInit(*this);
    return pClonedSdPage;
}

/*************************************************************************
|*
|* GetTextStyleSheetForObject
|*
\************************************************************************/

SfxStyleSheet* SdPage::GetTextStyleSheetForObject( SdrObject* pObj ) const
{
    const PresObjKind eKind = GetPresObjKind(pObj);
    if( eKind != PRESOBJ_NONE )
    {
        return GetStyleSheetForPresObj(eKind);
    }

    return FmFormPage::GetTextStyleSheetForObject( pObj );
}

SfxItemSet* SdPage::getOrCreateItems()
{
    if( mpItems == nullptr )
        mpItems = std::make_unique<SfxItemSet>( getSdrModelFromSdrPage().GetItemPool(), svl::Items<SDRATTR_XMLATTRIBUTES, SDRATTR_XMLATTRIBUTES>{} );

    return mpItems.get();
}

bool SdPage::setAlienAttributes( const css::uno::Any& rAttributes )
{
    SfxItemSet* pSet = getOrCreateItems();

    SvXMLAttrContainerItem aAlienAttributes( SDRATTR_XMLATTRIBUTES );
    if( aAlienAttributes.PutValue( rAttributes, 0 ) )
    {
        pSet->Put( aAlienAttributes );
        return true;
    }

    return false;
}

void SdPage::getAlienAttributes( css::uno::Any& rAttributes )
{
    const SfxPoolItem* pItem;

    if( (mpItems == nullptr) || ( SfxItemState::SET != mpItems->GetItemState( SDRATTR_XMLATTRIBUTES, false, &pItem ) ) )
    {
        SvXMLAttrContainerItem aAlienAttributes;
        aAlienAttributes.QueryValue( rAttributes );
    }
    else
    {
        static_cast<const SvXMLAttrContainerItem*>(pItem)->QueryValue( rAttributes );
    }
}

void SdPage::RemoveEmptyPresentationObjects()
{
    SdrObjListIter  aShapeIter( this, SdrIterMode::DeepWithGroups );

    for (SdrObject* pShape = aShapeIter.Next(); pShape; pShape = aShapeIter.Next())
    {
        if (pShape->IsEmptyPresObj())
        {
            RemoveObject( pShape->GetOrdNum() );
            SdrObject::Free( pShape );
        }
    }
}

void SdPage::setTransitionType( sal_Int16 nTransitionType )
{
    mnTransitionType = nTransitionType;
    ActionChanged();
}

void SdPage::setTransitionSubtype ( sal_Int16 nTransitionSubtype )
{
    mnTransitionSubtype = nTransitionSubtype;
    ActionChanged();
}

void SdPage::setTransitionDirection ( bool bTransitionbDirection )
{
    mbTransitionDirection = bTransitionbDirection;
    ActionChanged();
}

void SdPage::setTransitionFadeColor ( sal_Int32 nTransitionFadeColor )
{
    mnTransitionFadeColor = nTransitionFadeColor;
    ActionChanged();
}

void SdPage::setTransitionDuration ( double fTranstionDuration )
{
    mfTransitionDuration = fTranstionDuration;
    ActionChanged();
}

bool SdPage::Equals(const SdPage& rOtherPage) const
{
    if (GetObjCount() != rOtherPage.GetObjCount() ||
        mePageKind != rOtherPage.mePageKind ||
        meAutoLayout != rOtherPage.meAutoLayout ||
        mePresChange != rOtherPage.mePresChange ||
        !rtl::math::approxEqual(mfTime, rOtherPage.mfTime) ||
        mbSoundOn != rOtherPage.mbSoundOn ||
        mbExcluded != rOtherPage.mbExcluded ||
        maLayoutName != rOtherPage.maLayoutName ||
        maSoundFile != rOtherPage.maSoundFile ||
        mbLoopSound != rOtherPage.mbLoopSound ||
        mbStopSound != rOtherPage.mbStopSound ||
        maBookmarkName != rOtherPage.maBookmarkName ||
        mbScaleObjects != rOtherPage.mbScaleObjects ||
        mbBackgroundFullSize != rOtherPage.mbBackgroundFullSize ||
        meCharSet != rOtherPage.meCharSet ||
        mnPaperBin != rOtherPage.mnPaperBin ||
        mnTransitionType != rOtherPage.mnTransitionType ||
        mnTransitionSubtype != rOtherPage.mnTransitionSubtype ||
        mbTransitionDirection != rOtherPage.mbTransitionDirection ||
        mnTransitionFadeColor != rOtherPage.mnTransitionFadeColor ||
        !rtl::math::approxEqual(mfTransitionDuration, rOtherPage.mfTransitionDuration))
        return false;

    for(size_t i = 0; i < GetObjCount(); ++i)
        if (!GetObj(i)->Equals(*(rOtherPage.GetObj(i))))
            return false;

    return true;
 }

void SdPage::createAnnotation( css::uno::Reference< css::office::XAnnotation >& xAnnotation )
{
    sd::createAnnotation( xAnnotation, this );
}

void SdPage::addAnnotation( const Reference< XAnnotation >& xAnnotation, int nIndex )
{
    if( (nIndex == -1) || (nIndex > static_cast<int>(maAnnotations.size())) )
    {
        maAnnotations.push_back( xAnnotation );
    }
    else
    {
        maAnnotations.insert( maAnnotations.begin() + nIndex, xAnnotation );
    }

    if( getSdrModelFromSdrPage().IsUndoEnabled() )
    {
        std::unique_ptr<SdrUndoAction> pAction = CreateUndoInsertOrRemoveAnnotation( xAnnotation, true );
        if( pAction )
            getSdrModelFromSdrPage().AddUndo( std::move(pAction) );
    }

    SetChanged();
    getSdrModelFromSdrPage().SetChanged();
    NotifyDocumentEvent(
        static_cast< SdDrawDocument& >(getSdrModelFromSdrPage()),
        "OnAnnotationInserted",
        Reference<XInterface>(xAnnotation, UNO_QUERY));
}

void SdPage::removeAnnotation( const Reference< XAnnotation >& xAnnotation )
{
    if( getSdrModelFromSdrPage().IsUndoEnabled() )
    {
        std::unique_ptr<SdrUndoAction> pAction = CreateUndoInsertOrRemoveAnnotation( xAnnotation, false );
        if( pAction )
            getSdrModelFromSdrPage().AddUndo( std::move(pAction) );
    }

    AnnotationVector::iterator iter = std::find( maAnnotations.begin(), maAnnotations.end(), xAnnotation );
    if( iter != maAnnotations.end() )
        maAnnotations.erase( iter );

    getSdrModelFromSdrPage().SetChanged();
    NotifyDocumentEvent(
        static_cast< SdDrawDocument& >( getSdrModelFromSdrPage() ),
        "OnAnnotationRemoved",
        Reference<XInterface>( xAnnotation, UNO_QUERY ) );
}

void SdPage::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdPage"));

    const char* pPageKind = nullptr;
    switch (mePageKind)
    {
    case PageKind::Standard:
        pPageKind = "PageKind::Standard";
    break;
    case PageKind::Notes:
        pPageKind = "PageKind::Notes";
        break;
    case PageKind::Handout:
        pPageKind = "PageKind::Handout";
        break;
    }
    if (pPageKind)
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mePageKind"), BAD_CAST(pPageKind));


    FmFormPage::dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
