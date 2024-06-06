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
#include <rtl/math.hxx>
#include <svx/svdograf.hxx>

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
#include <svx/annotation/ObjectAnnotationData.hxx>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

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

void SdPage::SetPresentationLayout(std::u16string_view rLayoutName,
                                   bool bReplaceStyleSheets,
                                   bool bSetMasterPage,
                                   bool bReverseOrder)
{
    /*********************************************************************
    |* Name of the layout of the page
    \********************************************************************/
    OUString aOldLayoutName(maLayoutName);    // memorize
    maLayoutName = OUString::Concat(rLayoutName) + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE;

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

    for (const rtl::Reference<SdrObject>& pObj : *this)
    {
        if (pObj->GetObjInventor() == SdrInventor::Default &&
            pObj->GetObjIdentifier() == SdrObjKind::OutlineText)
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

            for (const auto& rpOut : aOutlineStyles)
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
                 pObj->GetObjIdentifier() == SdrObjKind::TitleText)
        {
            // We do not get PresObjKind via GetPresObjKind() since there are
            // only PresObjListe considered. But we want to consider all "Title
            // objects" here (paste from clipboard etc.)
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PresObjKind::Title);

            if (pSheet)
                pObj->SetStyleSheet(pSheet, true);
        }
        else
        {
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(GetPresObjKind(pObj.get()));

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
    SdrObject* pOutlineTextObj = GetPresObj(PresObjKind::Outline);

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

    for (const auto& rpStyle : aOutlineStyles)
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
        pLinkManager->InsertFileLink(*mpPageLink, sfx2::SvBaseLinkObjectType::ClientFile,
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
    meCharSet = rSrcPage.meCharSet;
    mnPaperBin = rSrcPage.mnPaperBin;
    mpPageLink = nullptr;    // is set when inserting via ConnectLink()
    mbIsPrecious = false;

    // use shape list directly to preserve constness of rSrcPage
    const std::list< SdrObject* >& rShapeList = rSrcPage.maPresentationShapeList.getList();
    const size_t nObjCount = GetObjCount();
    for( SdrObject* pObj : rShapeList )
    {
        size_t nOrdNum = pObj->GetOrdNum();
        InsertPresObj(nOrdNum < nObjCount ? GetObj(nOrdNum) : nullptr, rSrcPage.GetPresObjKind(pObj));
    }

    // header footer
    setHeaderFooterSettings( rSrcPage.getHeaderFooterSettings() );

    // animations
    rSrcPage.cloneAnimations(*this);

    // annotations
    for (auto const& rSourceAnnotation : rSrcPage.maAnnotations)
    {
        rtl::Reference<sdr::annotation::Annotation> aNewAnnotation = createAnnotation();
        aNewAnnotation->setPosition(rSourceAnnotation->getPosition());
        aNewAnnotation->setSize(rSourceAnnotation->getSize());
        aNewAnnotation->setAuthor(rSourceAnnotation->getAuthor());
        aNewAnnotation->setInitials(rSourceAnnotation->getInitials());
        aNewAnnotation->setDateTime(rSourceAnnotation->getDateTime());
        uno::Reference<css::text::XTextCopy> xSourceRange (rSourceAnnotation->getTextRange(), uno::UNO_QUERY);
        uno::Reference<css::text::XTextCopy> xRange (aNewAnnotation->getTextRange(), uno::UNO_QUERY);
        if(xSourceRange.is() && xRange.is())
            xRange->copyText(xSourceRange);
        addAnnotation(aNewAnnotation, -1);
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

rtl::Reference<SdrPage> SdPage::CloneSdrPage(SdrModel& rTargetModel) const
{
    SdDrawDocument& rSdDrawDocument(static_cast< SdDrawDocument& >(rTargetModel));
    rtl::Reference<SdPage> pClonedSdPage(
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
    if( eKind != PresObjKind::NONE )
    {
        return GetStyleSheetForPresObj(eKind);
    }

    return FmFormPage::GetTextStyleSheetForObject( pObj );
}

SfxItemSet* SdPage::getOrCreateItems()
{
    if( mpItems == nullptr )
        mpItems = std::make_unique<SfxItemSetFixed<SDRATTR_XMLATTRIBUTES, SDRATTR_XMLATTRIBUTES>>( getSdrModelFromSdrPage().GetItemPool());

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
    const SvXMLAttrContainerItem* pItem;

    if( (mpItems == nullptr) || !( pItem = mpItems->GetItemIfSet( SDRATTR_XMLATTRIBUTES, false ) ) )
    {
        SvXMLAttrContainerItem aAlienAttributes;
        aAlienAttributes.QueryValue( rAttributes );
    }
    else
    {
        pItem->QueryValue( rAttributes );
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

void SdPage::setTransitionDuration ( double fTransitionDuration )
{
    mfTransitionDuration = fTransitionDuration;
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
        IsBackgroundFullSize() != rOtherPage.IsBackgroundFullSize() || // ???
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

rtl::Reference<sdr::annotation::Annotation> SdPage::createAnnotation()
{
    return sd::createAnnotation(this);
}

void SdPage::addAnnotation(rtl::Reference<sdr::annotation::Annotation> const& xAnnotation, int nIndex)
{
    addAnnotationNoNotify(xAnnotation, nIndex);

    NotifyDocumentEvent(
        static_cast<SdDrawDocument&>(getSdrModelFromSdrPage()),
        u"OnAnnotationInserted"_ustr,
        uno::Reference<uno::XInterface>(static_cast<cppu::OWeakObject*>(xAnnotation.get()), UNO_QUERY));
}

void SdPage::addAnnotationNoNotify(rtl::Reference<sdr::annotation::Annotation> const& xAnnotation, int nIndex)
{
    if ((nIndex == -1) || (nIndex > int(maAnnotations.size())))
    {
        maAnnotations.push_back(xAnnotation);
    }
    else
    {
        maAnnotations.insert(maAnnotations.begin() + nIndex, xAnnotation);
    }

    SdrModel& rModel = getSdrModelFromSdrPage();

    if (rModel.IsUndoEnabled())
    {
        rtl::Reference<sdr::annotation::Annotation> xUnconstAnnotation(xAnnotation);
        std::unique_ptr<SdrUndoAction> pAction = CreateUndoInsertOrRemoveAnnotation(xUnconstAnnotation, true);
        if (pAction)
            rModel.AddUndo(std::move(pAction));
    }

    SetChanged();
}

void SdPage::removeAnnotation(rtl::Reference<sdr::annotation::Annotation> const& xAnnotation)
{
    removeAnnotationNoNotify(xAnnotation);

    NotifyDocumentEvent(
        static_cast<SdDrawDocument&>(getSdrModelFromSdrPage()),
        u"OnAnnotationRemoved"_ustr,
        uno::Reference<uno::XInterface>(static_cast<cppu::OWeakObject*>(xAnnotation.get()), UNO_QUERY));
}

void SdPage::removeAnnotationNoNotify(rtl::Reference<sdr::annotation::Annotation> const& xAnnotation)
{
    SdrModel& rModel = getSdrModelFromSdrPage();

    if (rModel.IsUndoEnabled())
    {
        rtl::Reference<sdr::annotation::Annotation> xUnconstAnnotation(xAnnotation);
        std::unique_ptr<SdrUndoAction> pAction = CreateUndoInsertOrRemoveAnnotation(xUnconstAnnotation, false);
        if (pAction)
            rModel.AddUndo(std::move(pAction));
    }

    for (size_t nObjectIndex = 0; nObjectIndex < GetObjCount(); ++nObjectIndex)
    {
        SdrObject* pObject = GetObj(nObjectIndex);
        if (pObject->isAnnotationObject() && pObject->getAnnotationData()->mxAnnotation == xAnnotation)
        {
            RemoveObject(nObjectIndex);
        }
    }

    auto iterator = std::find(maAnnotations.begin(), maAnnotations.end(), xAnnotation);
    if (iterator != maAnnotations.end())
        maAnnotations.erase(iterator);

    rModel.SetChanged();
}

void SdPage::getGraphicsForPrefetch(std::vector<Graphic*>& graphics) const
{
    for (const rtl::Reference<SdrObject>& obj : *this)
    {
        if( SdrGrafObj* grafObj = dynamic_cast<SdrGrafObj*>(obj.get()))
            if(!grafObj->GetGraphic().isAvailable())
                graphics.push_back( const_cast<Graphic*>(&grafObj->GetGraphic()));
        if( const Graphic* fillGraphic = obj->getFillGraphic())
            if(!fillGraphic->isAvailable())
                graphics.push_back( const_cast<Graphic*>(fillGraphic));
    }
}

void SdPage::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SdPage"));

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
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("mePageKind"), BAD_CAST(pPageKind));


    FmFormPage::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
