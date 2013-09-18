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
#include <boost/ptr_container/ptr_vector.hpp>

#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <editeng/outliner.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outlobj.hxx>
#include <svl/urihelper.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svx/svditer.hxx>

#include "sdresid.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "pglink.hxx"
#include "anminfo.hxx"

#include "../ui/inc/strings.hrc"
#include "../ui/inc/DrawDocShell.hxx"

#include <tools/tenccvt.hxx>
#include <svl/itemset.hxx>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;

extern void NotifyDocumentEvent( SdDrawDocument* pDocument, const OUString& rEventName, const Reference< XInterface >& xSource );

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
                                   sal_Bool bReplaceStyleSheets,
                                   sal_Bool bSetMasterPage,
                                   sal_Bool bReverseOrder)
{
    /*********************************************************************
    |* Name of the layout of the page
    \********************************************************************/
    OUString aOldLayoutName(maLayoutName);    // merken
    OUStringBuffer aBuf(rLayoutName);
    aBuf.append(SD_LT_SEPARATOR).append(SdResId(STR_LAYOUT_OUTLINE).toString());
    maLayoutName = aBuf.makeStringAndClear();

    /*********************************************************************
    |* search and replace master page if necessary
    \********************************************************************/
    if (bSetMasterPage && !IsMasterPage())
    {
        SdPage* pMaster;
        SdPage* pFoundMaster = 0;
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
        if( pFoundMaster == 0 )
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
    boost::ptr_vector<StyleReplaceData> aReplList;
    bool bListsFilled = false;

    sal_uLong nObjCount = GetObjCount();

    for (sal_uLong nObj = 0; nObj < nObjCount; nObj++)
    {
        SdrTextObj* pObj = (SdrTextObj*) GetObj(nObj);

        if (pObj->GetObjInventor() == SdrInventor &&
            pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            if (!bListsFilled || !bReplaceStyleSheets)
            {
                OUString aFullName;
                OUString aOldFullName;
                SfxStyleSheetBase* pSheet = NULL;
                SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();

                for (sal_Int16 i = -1; i < 9; i++)
                {
                    aOldFullName = aOldLayoutName;
                    aFullName = maLayoutName + " " +
                                OUString::number( (sal_Int32) (i <= 0 ) ? 1 : i + 1) +
                                " " +
                                OUString::number( (sal_Int32) (i <= 0 ) ? 1 : i + 1 );
                    pSheet = pStShPool->Find(aOldFullName, SD_STYLE_FAMILY_MASTERPAGE);
                    DBG_ASSERT(pSheet, "Old outline style sheet not found");
                    aOldOutlineStyles.push_back(pSheet);

                    pSheet = pStShPool->Find(aFullName, SD_STYLE_FAMILY_MASTERPAGE);
                    DBG_ASSERT(pSheet, "New outline style sheet not found");
                    aOutlineStyles.push_back(pSheet);

                    if (bReplaceStyleSheets && pSheet)
                    {
                        // Replace instead Set
                        StyleReplaceData* pReplData = new StyleReplaceData;
                        pReplData->nNewFamily = pSheet->GetFamily();
                        pReplData->nFamily    = pSheet->GetFamily();
                        pReplData->aNewName   = aFullName;
                        pReplData->aName      = aOldFullName;
                        aReplList.push_back(pReplData);
                    }
                    else
                    {
                        OutlinerParaObject* pOPO = ((SdrTextObj*)pObj)->GetOutlinerParaObject();

                        if( pOPO )
                            pOPO->SetStyleSheets( i,  aFullName, SD_STYLE_FAMILY_MASTERPAGE );
                    }
                }

                bListsFilled = true;
            }

            SfxStyleSheet* pSheet = NULL;
            SfxStyleSheet* pOldSheet = NULL;

            std::vector<SfxStyleSheetBase*>::iterator iterOut = aOutlineStyles.begin();
            std::vector<SfxStyleSheetBase*>::iterator iterOldOut = aOldOutlineStyles.begin();

            while (iterOut != aOutlineStyles.end())
            {
                pSheet = static_cast<SfxStyleSheet*>(*iterOut);
                pOldSheet = static_cast<SfxStyleSheet*>(*iterOldOut);

                if (pSheet != pOldSheet)
                {
                    pObj->EndListening(*pOldSheet);

                    if (!pObj->IsListening(*pSheet))
                        pObj->StartListening(*pSheet);
                }

                ++iterOut;
                ++iterOldOut;
            }

            OutlinerParaObject* pOPO = ((SdrTextObj*)pObj)->GetOutlinerParaObject();
            if ( bReplaceStyleSheets && pOPO )
            {
                boost::ptr_vector<StyleReplaceData>::const_iterator it = aReplList.begin();
                while (it != aReplList.end())
                {
                    pOPO->ChangeStyleSheets( it->aName, it->nFamily, it->aNewName, it->nNewFamily );
                    ++it;
                }
            }
        }
        else if (pObj->GetObjInventor() == SdrInventor &&
                 pObj->GetObjIdentifier() == OBJ_TITLETEXT)
        {
            // We do net get PresObjKind via GetPresObjKind() since there are
            // only PresObjListe considered. But we want to consider all "Title
            // objects" here (paste from clipboard etc.)
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(PRESOBJ_TITLE);

            if (pSheet)
                pObj->SetStyleSheet(pSheet, sal_True);
        }
        else
        {
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(GetPresObjKind(pObj));

            if (pSheet)
                pObj->SetStyleSheet(pSheet, sal_True);
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
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*)pModel->GetStyleSheetPool();
        DBG_ASSERT(pSPool, "StyleSheetPool missing");
        OUString aTrueLayoutName(maLayoutName);
        aTrueLayoutName = aTrueLayoutName.copy(0, aTrueLayoutName.indexOf( SD_LT_SEPARATOR ));

        SfxStyleSheet *pSheet = NULL;
        std::vector<SfxStyleSheetBase*> aOutlineStyles;
        pSPool->CreateOutlineSheetList(aTrueLayoutName,aOutlineStyles);

        std::vector<SfxStyleSheetBase*>::iterator iter;
        for (iter = aOutlineStyles.begin(); iter != aOutlineStyles.end(); ++iter)
        {
            pSheet = static_cast<SfxStyleSheet*>(*iter);
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
    sfx2::LinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && !mpPageLink && !maFileName.isEmpty() && !maBookmarkName.isEmpty() &&
        mePageKind==PK_STANDARD && !IsMasterPage() &&
        ( (SdDrawDocument*) pModel)->IsNewOrLoadCompleted())
    {
        /**********************************************************************
        * Connect
        * Only standard pages are allowed to be linked
        **********************************************************************/
        ::sd::DrawDocShell* pDocSh = ((SdDrawDocument*) pModel)->GetDocSh();

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
    sfx2::LinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && mpPageLink)
    {
        /**********************************************************************
        * Disconnect
        * (remove deletes *pGraphicLink implicit)
        **********************************************************************/
        pLinkManager->Remove(mpPageLink);
        mpPageLink=NULL;
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
,   mpItems(NULL)
{
    mePageKind           = rSrcPage.mePageKind;
    meAutoLayout         = rSrcPage.meAutoLayout;

    // use shape list directly to preserve constness of rSrcPage
    const std::list< SdrObject* >& rShapeList = rSrcPage.maPresentationShapeList.getList();
    for( std::list< SdrObject* >::const_iterator aIter = rShapeList.begin();
         aIter != rShapeList.end(); ++aIter )
    {
        SdrObject* pObj = *aIter;
        InsertPresObj(GetObj(pObj->GetOrdNum()), rSrcPage.GetPresObjKind(pObj));
    }

    mbSelected           = sal_False;
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
    maCreatedPageName    = "";
    maFileName           = rSrcPage.maFileName;
    maBookmarkName       = rSrcPage.maBookmarkName;
    mbScaleObjects       = rSrcPage.mbScaleObjects;
    mbBackgroundFullSize = rSrcPage.mbBackgroundFullSize;
    meCharSet            = rSrcPage.meCharSet;
    mnPaperBin           = rSrcPage.mnPaperBin;
    meOrientation        = rSrcPage.meOrientation;

    // header footer
    setHeaderFooterSettings( rSrcPage.getHeaderFooterSettings() );

    mpPageLink           = NULL;    // is set when inserting via ConnectLink()
}



/*************************************************************************
|*
|* Clone
|*
\************************************************************************/

SdrPage* SdPage::Clone() const
{
    return Clone(NULL);
}

SdrPage* SdPage::Clone(SdrModel* pNewModel) const
{
    DBG_ASSERT( pNewModel == 0, "sd::SdPage::Clone(), new page ignored, please check code! CL" );
    (void)pNewModel;

    SdPage* pNewPage = new SdPage(*this);

    cloneAnimations( *pNewPage );

    // fix user calls for duplicated slide
    SdrObjListIter aSourceIter( *this, IM_DEEPWITHGROUPS );
    SdrObjListIter aTargetIter( *pNewPage, IM_DEEPWITHGROUPS );

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
    const PresObjKind eKind = ((SdPage*)this)->GetPresObjKind(pObj);
    if( eKind != PRESOBJ_NONE )
    {
        return ((SdPage*)this)->GetStyleSheetForPresObj(eKind);
    }

    return FmFormPage::GetTextStyleSheetForObject( pObj );
}

SfxItemSet* SdPage::getOrCreateItems()
{
    if( mpItems == NULL )
        mpItems = new SfxItemSet( pModel->GetItemPool(), SDRATTR_XMLATTRIBUTES, SDRATTR_XMLATTRIBUTES );

    return mpItems;
}


sal_Bool SdPage::setAlienAttributes( const com::sun::star::uno::Any& rAttributes )
{
    SfxItemSet* pSet = getOrCreateItems();

    SvXMLAttrContainerItem aAlienAttributes( SDRATTR_XMLATTRIBUTES );
    if( aAlienAttributes.PutValue( rAttributes, 0 ) )
    {
        pSet->Put( aAlienAttributes );
        return sal_True;
    }

    return sal_False;
}

void SdPage::getAlienAttributes( com::sun::star::uno::Any& rAttributes )
{
    const SfxPoolItem* pItem;

    if( (mpItems == NULL) || ( SFX_ITEM_SET != mpItems->GetItemState( SDRATTR_XMLATTRIBUTES, sal_False, &pItem ) ) )
    {
        SvXMLAttrContainerItem aAlienAttributes;
        aAlienAttributes.QueryValue( rAttributes, 0 );
    }
    else
    {
        ((SvXMLAttrContainerItem*)pItem)->QueryValue( rAttributes, 0 );
    }
}

void SdPage::RemoveEmptyPresentationObjects()
{
    SdrObjListIter  aShapeIter( *this, IM_DEEPWITHGROUPS );

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

sal_Int16 SdPage::getTransitionType (void) const
{
    return mnTransitionType;
}

void SdPage::setTransitionType( sal_Int16 nTransitionType )
{
    mnTransitionType = nTransitionType;
    ActionChanged();
}

sal_Int16 SdPage::getTransitionSubtype (void) const
{
    return mnTransitionSubtype;
}

void SdPage::setTransitionSubtype ( sal_Int16 nTransitionSubtype )
{
    mnTransitionSubtype = nTransitionSubtype;
    ActionChanged();
}

sal_Bool SdPage::getTransitionDirection (void) const
{
    return mbTransitionDirection;
}

void SdPage::setTransitionDirection ( sal_Bool bTransitionbDirection )
{
    mbTransitionDirection = bTransitionbDirection;
    ActionChanged();
}

sal_Int32 SdPage::getTransitionFadeColor (void) const
{
    return mnTransitionFadeColor;
}

void SdPage::setTransitionFadeColor ( sal_Int32 nTransitionFadeColor )
{
    mnTransitionFadeColor = nTransitionFadeColor;
    ActionChanged();
}

double SdPage::getTransitionDuration (void) const
{
    return mfTransitionDuration;
}

void SdPage::setTransitionDuration ( double fTranstionDuration )
{
    mfTransitionDuration = fTranstionDuration;
    ActionChanged();
}

namespace sd {
extern void createAnnotation( Reference< XAnnotation >& xAnnotation, SdPage* pPage );
extern SdrUndoAction* CreateUndoInsertOrRemoveAnnotation( const Reference< XAnnotation >& xAnnotation, bool bInsert );
}

void SdPage::createAnnotation( ::com::sun::star::uno::Reference< ::com::sun::star::office::XAnnotation >& xAnnotation )
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
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), "OnAnnotationInserted" , xAnnotation );
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
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), OUString( "OnAnnotationRemoved" ), xAnnotation );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
