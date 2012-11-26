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
#include "precompiled_sd.hxx"
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <editeng/outliner.hxx>
#ifndef _SVXLINK_HXX
#include <sfx2/linkmgr.hxx>
#endif
#include <svx/svdotext.hxx>
#include <tools/urlobj.hxx>
#include <editeng/outlobj.hxx>
#include <svl/urihelper.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svx/svditer.hxx>
#include <tools/list.hxx>

#include "sdresid.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "stlpool.hxx"
//#include "sdiocmpt.hxx"
#include "pglink.hxx"
//#include "strmname.h"
#include "anminfo.hxx"

#include "../ui/inc/strings.hrc"
#include "../ui/inc/DrawDocShell.hxx"

// #90477#
#include <tools/tenccvt.hxx>
#include <svl/itemset.hxx>

using namespace ::sd;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;

extern void NotifyDocumentEvent( SdDrawDocument* pDocument, const rtl::OUString& rEventName, const Reference< XInterface >& xSource );

/*************************************************************************
|*
|* SetPresentationLayout, setzt: Layoutnamen, Masterpage-VerknÅpfung und
|* Vorlagen fuer Praesentationsobjekte
|*
|* Vorraussetzungen: - Die Seite muss bereits das richtige Model kennen!
|*                   - Die entsprechende Masterpage muss bereits im Model sein.
|*                   - Die entsprechenden StyleSheets muessen bereits im
|*                     im StyleSheetPool sein.
|*
|*  bReplaceStyleSheets = true : Benannte StyleSheets werden ausgetauscht
|*                        false: Alle StyleSheets werden neu zugewiesen
|*
|*  bSetMasterPage      = true : MasterPage suchen und zuweisen
|*
|*  bReverseOrder       = false: MasterPages von vorn nach hinten suchen
|*                        true : MasterPages von hinten nach vorn suchen (fuer Undo-Action)
|*
\************************************************************************/

void SdPage::SetPresentationLayout(const String& rLayoutName,
                                   bool bReplaceStyleSheets,
                                   bool bSetMasterPage,
                                   bool bReverseOrder)
{
    /*********************************************************************
    |* Layoutname der Seite
    \********************************************************************/
    String aOldLayoutName(maLayoutName);    // merken
    maLayoutName = rLayoutName;
    maLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    maLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

    /*********************************************************************
    |* ggf. Masterpage suchen und setzen
    \********************************************************************/
    if (bSetMasterPage && !IsMasterPage())
    {
        SdPage* pMaster;
        SdPage* pFoundMaster = 0;
        sal_uInt32 nMaster = 0;
        sal_uInt32 nMasterCount = getSdrModelFromSdrPage().GetMasterPageCount();

        if( !bReverseOrder )
        {
            for ( nMaster = 0; nMaster < nMasterCount; nMaster++ )
            {
                pMaster = static_cast< SdPage* >(getSdrModelFromSdrPage().GetMasterPage(nMaster));
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
                pMaster = static_cast< SdPage* >(getSdrModelFromSdrPage().GetMasterPage(nMaster - 1));
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
            pFoundMaster = static_cast< SdDrawDocument &>(getSdrModelFromSdrPage()).GetSdPage( 0, mePageKind );

        if( pFoundMaster )
            TRG_SetMasterPage(*pFoundMaster);
    }

    /*********************************************************************
    |* Vorlagen fuer Praesentationsobjekte
    \********************************************************************/
    // Listen mit:
    // - Vorlagenzeigern fuer Gliederungstextobjekt (alte und neue Vorlagen)
    // -Replacedaten fuer OutlinerParaObject
    List aOutlineStyles;
    List aOldOutlineStyles;
    List aReplList;
    bool bListsFilled = false;

    sal_uInt32 nObjCount = GetObjCount();

    for (sal_uInt32 nObj = 0; nObj < nObjCount; nObj++)
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
                SfxStyleSheetBasePool* pStShPool = getSdrModelFromSdrPage().GetStyleSheetPool();

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
                    aOldOutlineStyles.Insert(pSheet, LIST_APPEND);

                    pSheet = pStShPool->Find(aFullName, SD_STYLE_FAMILY_MASTERPAGE);
                    DBG_ASSERT(pSheet, "neue Gliederungsvorlage nicht gefunden");
                    aOutlineStyles.Insert(pSheet, LIST_APPEND);

                    if (bReplaceStyleSheets && pSheet)
                    {
                        // Replace anstatt Set
                        StyleReplaceData* pReplData = new StyleReplaceData;
                        pReplData->nNewFamily = pSheet->GetFamily();
                        pReplData->nFamily    = pSheet->GetFamily();
                        pReplData->aNewName   = aFullName;
                        pReplData->aName      = aOldFullName;
                        aReplList.Insert(pReplData, LIST_APPEND);
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

            SfxStyleSheet* pSheet = (SfxStyleSheet*)aOutlineStyles.First();
            SfxStyleSheet* pOldSheet = (SfxStyleSheet*)aOldOutlineStyles.First();
            while (pSheet)
            {
                if (pSheet != pOldSheet)
                {
                    pObj->EndListening(*pOldSheet);

                    if (!pObj->IsListening(*pSheet))
                        pObj->StartListening(*pSheet);
                }

                pSheet = (SfxStyleSheet*)aOutlineStyles.Next();
                pOldSheet = (SfxStyleSheet*)aOldOutlineStyles.Next();
            }

            OutlinerParaObject* pOPO = ((SdrTextObj*)pObj)->GetOutlinerParaObject();
            if ( bReplaceStyleSheets && pOPO )
            {
                StyleReplaceData* pReplData = (StyleReplaceData*) aReplList.First();

                while( pReplData )
                {
                    pOPO->ChangeStyleSheets( pReplData->aName, pReplData->nFamily, pReplData->aNewName, pReplData->nNewFamily );
                    pReplData = (StyleReplaceData*) aReplList.Next();
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
                pObj->SetStyleSheet(pSheet, true);
        }
        else
        {
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(GetPresObjKind(pObj));

            if (pSheet)
                pObj->SetStyleSheet(pSheet, true);
        }
    }

    for (sal_uInt32 i = 0; i < aReplList.Count(); i++)
    {
        delete (StyleReplaceData*) aReplList.GetObject(i);
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
        SdStyleSheetPool* pSPool = dynamic_cast< SdStyleSheetPool* >(getSdrModelFromSdrPage().GetStyleSheetPool());
        DBG_ASSERT(pSPool, "StyleSheetPool nicht gefunden");
        String aTrueLayoutName(maLayoutName);
        aTrueLayoutName.Erase( aTrueLayoutName.SearchAscii( SD_LT_SEPARATOR ));
        List* pOutlineStyles = pSPool->CreateOutlineSheetList(aTrueLayoutName);
        for (SfxStyleSheet* pSheet = (SfxStyleSheet*)pOutlineStyles->First();
             pSheet;
             pSheet = (SfxStyleSheet*)pOutlineStyles->Next())
            {
                pOutlineTextObj->EndListening(*pSheet);
            }

        delete pOutlineStyles;
    }
}

/*************************************************************************
|*
|* Beim sfx2::LinkManager anmelden
|*
\************************************************************************/

void SdPage::ConnectLink()
{
    sfx2::LinkManager* pLinkManager = getSdrModelFromSdrPage().GetLinkManager();
    SdDrawDocument& rSdDrawDocument = static_cast< SdDrawDocument& >(getSdrModelFromSdrPage());

    if (pLinkManager && !mpPageLink && maFileName.Len() && maBookmarkName.Len() &&
        mePageKind==PK_STANDARD && !IsMasterPage() &&
        rSdDrawDocument.IsNewOrLoadCompleted())
    {
        /**********************************************************************
        * Anmelden
        * Nur Standardseiten duerfen gelinkt sein
        **********************************************************************/
        ::sd::DrawDocShell* pDocSh = rSdDrawDocument.GetDocSh();

        if (!pDocSh || pDocSh->GetMedium()->GetOrigURL() != maFileName)
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
    sfx2::LinkManager* pLinkManager = getSdrModelFromSdrPage().GetLinkManager();

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

//SdPage::SdPage(const SdPage& rSrcPage)
//: FmFormPage(rSrcPage)
//, SfxListener()
//, mpItems(NULL)
//{
//  mePageKind           = rSrcPage.mePageKind;
//  meAutoLayout         = rSrcPage.meAutoLayout;
//
//  const SdrObject* pObj = 0;
//  while((pObj = rSrcPage.maPresentationShapeList.getNextShape(pObj)) != 0)
//      InsertPresObj(GetObj(pObj->GetNavigationPosition()), rSrcPage.GetPresObjKind(pObj));
//
//  mbSelected           = false;
//  mnTransitionType    = rSrcPage.mnTransitionType;
//  mnTransitionSubtype = rSrcPage.mnTransitionSubtype;
//  mbTransitionDirection = rSrcPage.mbTransitionDirection;
//  mnTransitionFadeColor = rSrcPage.mnTransitionFadeColor;
//  mfTransitionDuration = rSrcPage.mfTransitionDuration;
//  mePresChange            = rSrcPage.mePresChange;
//  mnTime               = rSrcPage.mnTime;
//  mbSoundOn            = rSrcPage.mbSoundOn;
//  mbExcluded           = rSrcPage.mbExcluded;
//
//  maLayoutName         = rSrcPage.maLayoutName;
//  maSoundFile          = rSrcPage.maSoundFile;
//  mbLoopSound          = rSrcPage.mbLoopSound;
//  mbStopSound          = rSrcPage.mbStopSound;
//  maCreatedPageName    = String();
//  maFileName           = rSrcPage.maFileName;
//  maBookmarkName       = rSrcPage.maBookmarkName;
//  mbScaleObjects       = rSrcPage.mbScaleObjects;
//  mbBackgroundFullSize = rSrcPage.mbBackgroundFullSize;
//  meCharSet            = rSrcPage.meCharSet;
//  mnPaperBin           = rSrcPage.mnPaperBin;
//  meOrientation        = rSrcPage.meOrientation;
//
//  // header footer
//  setHeaderFooterSettings( rSrcPage.getHeaderFooterSettings() );
//
//  mpPageLink           = NULL;    // Wird beim Einfuegen ueber ConnectLink() gesetzt
//}

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
    if( !mpItems )
        mpItems = new SfxItemSet( getSdrModelFromSdrPage().GetItemPool(), SDRATTR_XMLATTRIBUTES, SDRATTR_XMLATTRIBUTES );

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
            RemoveObjectFromSdrObjList( pShape->GetNavigationPosition() );
            deleteSdrObjectSafeAndClearPointer( pShape );
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

    if( getSdrModelFromSdrPage().IsUndoEnabled() )
    {
        SdrUndoAction* pAction = CreateUndoInsertOrRemoveAnnotation( xAnnotation, true );
        if( pAction )
            getSdrModelFromSdrPage().AddUndo( pAction );
    }

    SetChanged();
    getSdrModelFromSdrPage().SetChanged();
        Reference< XInterface > xSource( xAnnotation, UNO_QUERY );
    NotifyDocumentEvent( static_cast< SdDrawDocument* >( &getSdrModelFromSdrPage() ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnAnnotationInserted" ) ), xSource );
}

void SdPage::removeAnnotation( const Reference< XAnnotation >& xAnnotation )
{
    if( getSdrModelFromSdrPage().IsUndoEnabled() )
    {
        SdrUndoAction* pAction = CreateUndoInsertOrRemoveAnnotation( xAnnotation, false );
        if( pAction )
            getSdrModelFromSdrPage().AddUndo( pAction );
    }

    AnnotationVector::iterator iter = std::find( maAnnotations.begin(), maAnnotations.end(), xAnnotation );
    if( iter != maAnnotations.end() )
        maAnnotations.erase( iter );

    getSdrModelFromSdrPage().SetChanged();
        Reference< XInterface > xSource( xAnnotation, UNO_QUERY );
    NotifyDocumentEvent( static_cast< SdDrawDocument* >( &getSdrModelFromSdrPage() ), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnAnnotationRemoved" ) ), xSource );
}
