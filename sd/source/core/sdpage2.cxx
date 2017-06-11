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
#include <o3tl/make_unique.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <svl/urihelper.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svx/svditer.hxx>

#include "Annotation.hxx"
#include "notifydocumentevent.hxx"
#include "sdresid.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "strings.hrc"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "pglink.hxx"
#include "anminfo.hxx"

#include "strings.hrc"
#include "strings.hxx"
#include "DrawDocShell.hxx"

#include <tools/tenccvt.hxx>
#include <svl/itemset.hxx>
#include <rtl/strbuf.hxx>

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
    OUString aOldLayoutName(maLayoutName);    // merken
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
        sal_uInt16 nMasterCount = pModel->GetMasterPageCount();

        if( !bReverseOrder )
        {
            for ( nMaster = 0; nMaster < nMasterCount; nMaster++ )
            {
                pMaster = static_cast<SdPage*>(pModel->GetMasterPage(nMaster));
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
                pMaster = static_cast<SdPage*>(pModel->GetMasterPage(nMaster - 1));
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
            pFoundMaster = static_cast< SdDrawDocument *>(pModel)->GetSdPage( 0, mePageKind );

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
                SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();

                for (sal_Int16 i = -1; i < 9; i++)
                {
                    aOldFullName = aOldLayoutName + " " +
                                OUString::number( (i <= 0 ) ? 1 : i + 1 );
                    aFullName = maLayoutName + " " +
                                OUString::number( (i <= 0 ) ? 1 : i + 1);
                    pSheet = pStShPool->Find(aOldFullName, SD_STYLE_FAMILY_MASTERPAGE);
                    DBG_ASSERT(pSheet, "Old outline style sheet not found");
                    aOldOutlineStyles.push_back(pSheet);

                    pSheet = pStShPool->Find(aFullName, SD_STYLE_FAMILY_MASTERPAGE);
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
                            pOPO->SetStyleSheets( i,  aFullName, SD_STYLE_FAMILY_MASTERPAGE );
                    }
                }

                bListsFilled = true;
            }


            std::vector<SfxStyleSheetBase*>::iterator iterOut = aOutlineStyles.begin();
            std::vector<SfxStyleSheetBase*>::iterator iterOldOut = aOldOutlineStyles.begin();

            while (iterOut != aOutlineStyles.end())
            {
                SfxStyleSheet* pSheet = static_cast<SfxStyleSheet*>(*iterOut);
                SfxStyleSheet* pOldSheet = static_cast<SfxStyleSheet*>(*iterOldOut);

                if (pSheet != pOldSheet)
                {
                    if (pOldSheet)
                        pObj->EndListening(*pOldSheet);

                    if (pSheet && !pObj->IsListening(*pSheet))
                        pObj->StartListening(*pSheet);
                }

                ++iterOut;
                ++iterOldOut;
            }

            OutlinerParaObject* pOPO = pObj->GetOutlinerParaObject();
            if ( bReplaceStyleSheets && pOPO )
            {
                std::vector<StyleReplaceData>::const_iterator it = aReplList.begin();
                while (it != aReplList.end())
                {
                    pOPO->ChangeStyleSheets( it->aName, it->nFamily, it->aNewName, it->nNewFamily );
                    ++it;
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

    if (pOutlineTextObj)
    {
        SdStyleSheetPool* pSPool = static_cast<SdStyleSheetPool*>(pModel->GetStyleSheetPool());
        DBG_ASSERT(pSPool, "StyleSheetPool missing");
        OUString aTrueLayoutName(maLayoutName);
        sal_Int32 nIndex = aTrueLayoutName.indexOf( SD_LT_SEPARATOR );
        if( nIndex != -1 )
            aTrueLayoutName = aTrueLayoutName.copy(0, nIndex);

        std::vector<SfxStyleSheetBase*> aOutlineStyles;
        pSPool->CreateOutlineSheetList(aTrueLayoutName,aOutlineStyles);

        std::vector<SfxStyleSheetBase*>::iterator iter;
        for (iter = aOutlineStyles.begin(); iter != aOutlineStyles.end(); ++iter)
        {
            SfxStyleSheet *pSheet = static_cast<SfxStyleSheet*>(*iter);
            pOutlineTextObj->EndListening(*pSheet);
        }
    }
}

/*************************************************************************
|*
|* Set new model
|*
\************************************************************************/

void SdPage::SetModel(SdrModel* pNewModel)
{
    DisconnectLink();

    // assign model
    FmFormPage::SetModel(pNewModel);

    ConnectLink();
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
    sfx2::LinkManager* pLinkManager = pModel!=nullptr ? pModel->GetLinkManager() : nullptr;

    if (pLinkManager && !mpPageLink && !maFileName.isEmpty() && !maBookmarkName.isEmpty() &&
        mePageKind==PageKind::Standard && !IsMasterPage() &&
        static_cast<SdDrawDocument*>(pModel)->IsNewOrLoadCompleted())
    {
        /**********************************************************************
        * Connect
        * Only standard pages are allowed to be linked
        **********************************************************************/
        ::sd::DrawDocShell* pDocSh = static_cast<SdDrawDocument*>(pModel)->GetDocSh();

        if (!pDocSh || !pDocSh->GetMedium()->GetOrigURL().equals(maFileName))
        {
            // No links to document owned pages!
            mpPageLink = new SdPageLink(this, maFileName, maBookmarkName);
            OUString aFilterName(SdResId(STR_IMPRESS));
            pLinkManager->InsertFileLink(*mpPageLink, OBJECT_CLIENT_FILE,
                                         maFileName, &aFilterName, &maBookmarkName);
            mpPageLink->Connect();
        }
    }
}

/*************************************************************************
|*
|* Disconnect from sfx2::LinkManager
|*
\************************************************************************/

void SdPage::DisconnectLink()
{
    sfx2::LinkManager* pLinkManager = pModel!=nullptr ? pModel->GetLinkManager() : nullptr;

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

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/

SdPage::SdPage(const SdPage& rSrcPage)
:   FmFormPage(rSrcPage)
,   SdrObjUserCall()
,   mpItems(nullptr)
,   mnPageId(mnLastPageId++)
{
    mePageKind           = rSrcPage.mePageKind;
    meAutoLayout         = rSrcPage.meAutoLayout;

    mbSelected           = false;
    mnTransitionType    = rSrcPage.mnTransitionType;
    mnTransitionSubtype = rSrcPage.mnTransitionSubtype;
    mbTransitionDirection = rSrcPage.mbTransitionDirection;
    mnTransitionFadeColor = rSrcPage.mnTransitionFadeColor;
    mfTransitionDuration = rSrcPage.mfTransitionDuration;
    mePresChange            = rSrcPage.mePresChange;
    mfTime               = rSrcPage.mfTime;
    mbSoundOn            = rSrcPage.mbSoundOn;
    mbExcluded           = rSrcPage.mbExcluded;

    maLayoutName         = rSrcPage.maLayoutName;
    maSoundFile          = rSrcPage.maSoundFile;
    mbLoopSound          = rSrcPage.mbLoopSound;
    mbStopSound          = rSrcPage.mbStopSound;
    maCreatedPageName.clear();
    maFileName           = rSrcPage.maFileName;
    maBookmarkName       = rSrcPage.maBookmarkName;
    mbScaleObjects       = rSrcPage.mbScaleObjects;
    mbBackgroundFullSize = rSrcPage.mbBackgroundFullSize;
    meCharSet            = rSrcPage.meCharSet;
    mnPaperBin           = rSrcPage.mnPaperBin;
    meOrientation        = rSrcPage.meOrientation;

    mpPageLink           = nullptr;    // is set when inserting via ConnectLink()

    mbIsPrecious         = false;
}

void SdPage::lateInit(const SdPage& rSrcPage)
{
    FmFormPage::lateInit(rSrcPage);

    // use shape list directly to preserve constness of rSrcPage
    const std::list< SdrObject* >& rShapeList = rSrcPage.maPresentationShapeList.getList();
    for( std::list< SdrObject* >::const_iterator aIter = rShapeList.begin();
         aIter != rShapeList.end(); ++aIter )
    {
        SdrObject* pObj = *aIter;
        InsertPresObj(GetObj(pObj->GetOrdNum()), rSrcPage.GetPresObjKind(pObj));
    }

    // header footer
    setHeaderFooterSettings( rSrcPage.getHeaderFooterSettings() );
}

/*************************************************************************
|*
|* Clone
|*
\************************************************************************/

SdrPage* SdPage::Clone() const
{
    return Clone(nullptr);
}

SdrPage* SdPage::Clone(SdrModel* pNewModel) const
{
    DBG_ASSERT( pNewModel == nullptr, "sd::SdPage::Clone(), new page ignored, please check code! CL" );
    (void)pNewModel;

    SdPage* pNewPage = new SdPage(*this);
    pNewPage->lateInit( *this );

    cloneAnimations( *pNewPage );

    // fix user calls for duplicated slide
    SdrObjListIter aSourceIter( *this, SdrIterMode::DeepWithGroups );
    SdrObjListIter aTargetIter( *pNewPage, SdrIterMode::DeepWithGroups );

    while( aSourceIter.IsMore() && aTargetIter.IsMore() )
    {
        SdrObject* pSource = aSourceIter.Next();
        SdrObject* pTarget = aTargetIter.Next();

        if( pSource->GetUserCall() )
            pTarget->SetUserCall( pNewPage );
    }

    return pNewPage;
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
        mpItems = o3tl::make_unique<SfxItemSet>( pModel->GetItemPool(), svl::Items<SDRATTR_XMLATTRIBUTES, SDRATTR_XMLATTRIBUTES>{} );

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
    SdrObjListIter  aShapeIter( *this, SdrIterMode::DeepWithGroups );

    SdrObject* pShape;
    for( pShape = aShapeIter.Next(); pShape; pShape = aShapeIter.Next() )
    {
        if( pShape && pShape->IsEmptyPresObj() )
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

OString SdPage::stringify() const
{
    OStringBuffer aString(100);
    aString.append((sal_Int32)mePageKind).append((sal_Int32)meAutoLayout).append((sal_Int32)mePresChange).append(mfTime).append(mbSoundOn).append(mbExcluded).
             append(OUStringToOString( maLayoutName, RTL_TEXTENCODING_UTF8 )).
             append(OUStringToOString(maSoundFile, RTL_TEXTENCODING_UTF8 )).
             append(mbLoopSound).append(mbStopSound).
             /*append(OUStringToOString(maCreatedPageName, RTL_TEXTENCODING_UTF8)).
             append(OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8)).*/
             append(OUStringToOString(maBookmarkName, RTL_TEXTENCODING_UTF8)).
             append(mbScaleObjects).append(mbBackgroundFullSize).append((sal_Int32)meCharSet).append((sal_Int32)mnPaperBin).
             append((sal_Int32)meOrientation).append((sal_Int32)mnTransitionType).append((sal_Int32)mnTransitionSubtype).append(mbTransitionDirection).
             append(mnTransitionFadeColor).append(mfTransitionDuration);//.append(mbIsPrecious);

    const size_t n = GetObjCount();
    for(size_t i = 0; i < n; ++i)
        aString.append(GetObj(i)->stringify());
    return aString.makeStringAndClear();
}

void SdPage::createAnnotation( css::uno::Reference< css::office::XAnnotation >& xAnnotation )
{
    sd::createAnnotation( xAnnotation, this );
}

void SdPage::addAnnotation( const Reference< XAnnotation >& xAnnotation, int nIndex )
{
    if( (nIndex == -1) || (nIndex > (int)maAnnotations.size()) )
    {
        maAnnotations.push_back( xAnnotation );
    }
    else
    {
        maAnnotations.insert( maAnnotations.begin() + nIndex, xAnnotation );
    }

    if( pModel && pModel->IsUndoEnabled() )
    {
        SdrUndoAction* pAction = CreateUndoInsertOrRemoveAnnotation( xAnnotation, true );
        if( pAction )
            pModel->AddUndo( pAction );
    }

    SetChanged();

    if( pModel )
    {
        pModel->SetChanged();
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), "OnAnnotationInserted", Reference<XInterface>( xAnnotation, UNO_QUERY ) );
    }
}

void SdPage::removeAnnotation( const Reference< XAnnotation >& xAnnotation )
{
    if( pModel && pModel->IsUndoEnabled() )
    {
        SdrUndoAction* pAction = CreateUndoInsertOrRemoveAnnotation( xAnnotation, false );
        if( pAction )
            pModel->AddUndo( pAction );
    }

    AnnotationVector::iterator iter = std::find( maAnnotations.begin(), maAnnotations.end(), xAnnotation );
    if( iter != maAnnotations.end() )
        maAnnotations.erase( iter );

    if( pModel )
    {
        pModel->SetChanged();
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), "OnAnnotationRemoved", Reference<XInterface>( xAnnotation, UNO_QUERY ) );
    }
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
