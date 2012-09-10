/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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

extern void NotifyDocumentEvent( SdDrawDocument* pDocument, const rtl::OUString& rEventName, const Reference< XInterface >& xSource );

/*************************************************************************
|*
|* SetPresentationLayout, setzt: Layoutnamen, Masterpage-Verkn�pfung und
|* Vorlagen fuer Praesentationsobjekte
|*
|* Vorraussetzungen: - Die Seite muss bereits das richtige Model kennen!
|*                   - Die entsprechende Masterpage muss bereits im Model sein.
|*                   - Die entsprechenden StyleSheets muessen bereits im
|*                     im StyleSheetPool sein.
|*
|*  bReplaceStyleSheets = sal_True : Benannte StyleSheets werden ausgetauscht
|*                        sal_False: Alle StyleSheets werden neu zugewiesen
|*
|*  bSetMasterPage      = sal_True : MasterPage suchen und zuweisen
|*
|*  bReverseOrder       = sal_False: MasterPages von vorn nach hinten suchen
|*                        sal_True : MasterPages von hinten nach vorn suchen (fuer Undo-Action)
|*
\************************************************************************/

void SdPage::SetPresentationLayout(const String& rLayoutName,
                                   sal_Bool bReplaceStyleSheets,
                                   sal_Bool bSetMasterPage,
                                   sal_Bool bReverseOrder)
{
    /*********************************************************************
    |* Layoutname der Seite
    \********************************************************************/
    OUString aOldLayoutName(maLayoutName);    // merken
    OUStringBuffer aBuf(rLayoutName);
    aBuf.append(SD_LT_SEPARATOR).append(SdResId(STR_LAYOUT_OUTLINE).toString());
    maLayoutName = aBuf.makeStringAndClear();

    /*********************************************************************
    |* ggf. Masterpage suchen und setzen
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
    |* Vorlagen fuer Praesentationsobjekte
    \********************************************************************/
    // Listen mit:
    // - Vorlagenzeigern fuer Gliederungstextobjekt (alte und neue Vorlagen)
    // -Replacedaten fuer OutlinerParaObject
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
                String aFullName;
                String aOldFullName;
                SfxStyleSheetBase* pSheet = NULL;
                SfxStyleSheetBasePool* pStShPool = pModel->GetStyleSheetPool();

                for (sal_Int16 i = -1; i < 9; i++)
                {
                    aFullName = maLayoutName;
                    aOldFullName = aOldLayoutName;
                    aFullName += sal_Unicode( ' ' );
                    aFullName += String::CreateFromInt32( (sal_Int32) (i <= 0 ) ? 1 : i + 1);
                    aOldFullName += sal_Unicode( ' ' );
                    aOldFullName += String::CreateFromInt32( (sal_Int32) (i <= 0 ) ? 1 : i + 1 );

                    pSheet = pStShPool->Find(aOldFullName, SD_STYLE_FAMILY_MASTERPAGE);
                    DBG_ASSERT(pSheet, "alte Gliederungsvorlage nicht gefunden");
                    aOldOutlineStyles.push_back(pSheet);

                    pSheet = pStShPool->Find(aFullName, SD_STYLE_FAMILY_MASTERPAGE);
                    DBG_ASSERT(pSheet, "neue Gliederungsvorlage nicht gefunden");
                    aOutlineStyles.push_back(pSheet);

                    if (bReplaceStyleSheets && pSheet)
                    {
                        // Replace anstatt Set
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
            // PresObjKind nicht ueber GetPresObjKind() holen, da dort nur
            // die PresObjListe beruecksichtigt wird. Es sollen aber alle
            // "Titelobjekte" hier beruecksichtigt werden (Paste aus Clipboard usw.)
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
|* das Gliederungstextobjekt bei den Vorlagen fuer die Gliederungsebenen
|* abmelden
|*
\************************************************************************/

void SdPage::EndListenOutlineText()
{
    SdrObject* pOutlineTextObj = GetPresObj(PRESOBJ_OUTLINE);

    if (pOutlineTextObj)
    {
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*)pModel->GetStyleSheetPool();
        DBG_ASSERT(pSPool, "StyleSheetPool nicht gefunden");
        String aTrueLayoutName(maLayoutName);
        aTrueLayoutName.Erase( aTrueLayoutName.SearchAscii( SD_LT_SEPARATOR ));

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
|* Neues Model setzen
|*
\************************************************************************/

void SdPage::SetModel(SdrModel* pNewModel)
{
    DisconnectLink();

    // Model umsetzen
    FmFormPage::SetModel(pNewModel);

    ConnectLink();
}

/*************************************************************************
|*
|* Ist die Seite read-only?
|*
\************************************************************************/

bool SdPage::IsReadOnly() const
{
    return false;
}

/*************************************************************************
|*
|* Beim sfx2::LinkManager anmelden
|*
\************************************************************************/

void SdPage::ConnectLink()
{
    sfx2::LinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && !mpPageLink && maFileName.Len() && maBookmarkName.Len() &&
        mePageKind==PK_STANDARD && !IsMasterPage() &&
        ( (SdDrawDocument*) pModel)->IsNewOrLoadCompleted())
    {
        /**********************************************************************
        * Anmelden
        * Nur Standardseiten duerfen gelinkt sein
        **********************************************************************/
        ::sd::DrawDocShell* pDocSh = ((SdDrawDocument*) pModel)->GetDocSh();

        if (!pDocSh || !pDocSh->GetMedium()->GetOrigURL().equals(maFileName))
        {
            // Keine Links auf Dokument-eigene Seiten!
            mpPageLink = new SdPageLink(this, maFileName, maBookmarkName);
            String aFilterName(SdResId(STR_IMPRESS));
            pLinkManager->InsertFileLink(*mpPageLink, OBJECT_CLIENT_FILE,
                                         maFileName, &aFilterName, &maBookmarkName);
            mpPageLink->Connect();
        }
    }
}


/*************************************************************************
|*
|* Beim sfx2::LinkManager abmelden
|*
\************************************************************************/

void SdPage::DisconnectLink()
{
    sfx2::LinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && mpPageLink)
    {
        /**********************************************************************
        * Abmelden
        * (Bei Remove wird *pGraphicLink implizit deleted)
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

    SdrObject* pObj = 0;
    while((pObj = rSrcPage.maPresentationShapeList.getNextShape(pObj)) != 0)
        InsertPresObj(GetObj(pObj->GetOrdNum()), rSrcPage.GetPresObjKind(pObj));

    mbSelected           = sal_False;
    mnTransitionType    = rSrcPage.mnTransitionType;
    mnTransitionSubtype = rSrcPage.mnTransitionSubtype;
    mbTransitionDirection = rSrcPage.mbTransitionDirection;
    mnTransitionFadeColor = rSrcPage.mnTransitionFadeColor;
    mfTransitionDuration = rSrcPage.mfTransitionDuration;
    mePresChange            = rSrcPage.mePresChange;
    mnTime               = rSrcPage.mnTime;
    mbSoundOn            = rSrcPage.mbSoundOn;
    mbExcluded           = rSrcPage.mbExcluded;

    maLayoutName         = rSrcPage.maLayoutName;
    maSoundFile          = rSrcPage.maSoundFile;
    mbLoopSound          = rSrcPage.mbLoopSound;
    mbStopSound          = rSrcPage.mbStopSound;
    maCreatedPageName    = String();
    maFileName           = rSrcPage.maFileName;
    maBookmarkName       = rSrcPage.maBookmarkName;
    mbScaleObjects       = rSrcPage.mbScaleObjects;
    mbBackgroundFullSize = rSrcPage.mbBackgroundFullSize;
    meCharSet            = rSrcPage.meCharSet;
    mnPaperBin           = rSrcPage.mnPaperBin;
    meOrientation        = rSrcPage.meOrientation;

    // header footer
    setHeaderFooterSettings( rSrcPage.getHeaderFooterSettings() );

    mpPageLink           = NULL;    // Wird beim Einfuegen ueber ConnectLink() gesetzt
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
        Reference< XInterface > xSource( xAnnotation, UNO_QUERY );
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), "OnAnnotationInserted" , xSource );
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
        Reference< XInterface > xSource( xAnnotation, UNO_QUERY );
        NotifyDocumentEvent( static_cast< SdDrawDocument* >( pModel ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnAnnotationRemoved" ) ), xSource );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
