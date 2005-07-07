/*************************************************************************
 *
 *  $RCSfile: sdpage2.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:34:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <svx/outliner.hxx>
#endif
#ifndef _SVXLINK_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif
#include <svtools/urihelper.hxx>

#ifndef _SVX_XMLCNITM_HXX
#include <svx/xmlcnitm.hxx>
#endif

#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

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

#ifdef MAC
#include "::ui:inc:strings.hrc"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "::ui:inc:DrawDocShell.hxx"
#endif
#else
#ifdef UNX
#include "../ui/inc/strings.hrc"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "../ui/inc/DrawDocShell.hxx"
#endif
#else
#include "..\ui\inc\cfgids.hxx"
#include "..\ui\inc\strings.hrc"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "..\ui\inc\DrawDocShell.hxx"
#endif
#endif
#endif

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif

using namespace ::sd;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* SetPresentationLayout, setzt: Layoutnamen, Masterpage-Verknpfung und
|* Vorlagen fuer Praesentationsobjekte
|*
|* Vorraussetzungen: - Die Seite muss bereits das richtige Model kennen!
|*                   - Die entsprechende Masterpage muss bereits im Model sein.
|*                   - Die entsprechenden StyleSheets muessen bereits im
|*                     im StyleSheetPool sein.
|*
|*  bReplaceStyleSheets = TRUE : Benannte StyleSheets werden ausgetauscht
|*                        FALSE: Alle StyleSheets werden neu zugewiesen
|*
|*  bSetMasterPage      = TRUE : MasterPage suchen und zuweisen
|*
|*  bReverseOrder       = FALSE: MasterPages von vorn nach hinten suchen
|*                        TRUE : MasterPages von hinten nach vorn suchen (fuer Undo-Action)
|*
\************************************************************************/

void SdPage::SetPresentationLayout(const String& rLayoutName,
                                   BOOL bReplaceStyleSheets,
                                   BOOL bSetMasterPage,
                                   BOOL bReverseOrder)
{
    /*********************************************************************
    |* Layoutname der Seite
    \********************************************************************/
    String aOldLayoutName(aLayoutName);     // merken
    aLayoutName = rLayoutName;
    aLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

    /*********************************************************************
    |* ggf. Masterpage suchen und setzen
    \********************************************************************/
    if (bSetMasterPage && !IsMasterPage())
    {
        SdPage* pMaster;
        SdPage* pFoundMaster = 0;
        USHORT nMaster = 0;
        USHORT nMasterCount = pModel->GetMasterPageCount();

        if( !bReverseOrder )
        {
            for ( nMaster = 0; nMaster < nMasterCount; nMaster++ )
            {
                pMaster = static_cast<SdPage*>(pModel->GetMasterPage(nMaster));
                if (pMaster->GetPageKind() == ePageKind && pMaster->GetLayoutName() == aLayoutName)
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
                if (pMaster->GetPageKind() == ePageKind && pMaster->GetLayoutName() == aLayoutName)
                {
                    pFoundMaster = pMaster;
                    break;
                }
            }
        }

        DBG_ASSERT(pFoundMaster, "Masterpage for presentation layout not found!");

        // this should never happen, but we play failsafe here
        if( pFoundMaster == 0 )
            pFoundMaster = static_cast< SdDrawDocument *>(pModel)->GetSdPage( 0, ePageKind );

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
    BOOL bListsFilled = FALSE;

    ULONG nObjCount = GetObjCount();

    for (ULONG nObj = 0; nObj < nObjCount; nObj++)
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

                for (USHORT i = 1; i < 10; i++)
                {
                    aFullName = aLayoutName;
                    aOldFullName = aOldLayoutName;
                    aFullName += sal_Unicode( ' ' );
                    aFullName += String::CreateFromInt32( (sal_Int32)i );
                    aOldFullName += sal_Unicode( ' ' );
                    aOldFullName += String::CreateFromInt32( (sal_Int32)i );

                    pSheet = pStShPool->Find(aOldFullName, SD_LT_FAMILY);
                    DBG_ASSERT(pSheet, "alte Gliederungsvorlage nicht gefunden");
                    aOldOutlineStyles.Insert(pSheet, LIST_APPEND);

                    pSheet = pStShPool->Find(aFullName, SD_LT_FAMILY);
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
                            pOPO->SetStyleSheets( i,  aFullName, SD_LT_FAMILY );
                    }
                }

                bListsFilled = TRUE;
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
                pObj->SetStyleSheet(pSheet, TRUE);
        }
        else
        {
            SfxStyleSheet* pSheet = GetStyleSheetForPresObj(GetPresObjKind(pObj));

            if (pSheet)
                pObj->SetStyleSheet(pSheet, TRUE);
        }
    }

    for (ULONG i = 0; i < aReplList.Count(); i++)
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
        SdStyleSheetPool* pSPool = (SdStyleSheetPool*)pModel->GetStyleSheetPool();
        DBG_ASSERT(pSPool, "StyleSheetPool nicht gefunden");
        String aTrueLayoutName(aLayoutName);
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

FASTBOOL SdPage::IsReadOnly() const
{
    BOOL bReadOnly = FALSE;

    if (pPageLink)
    {
        // Seite ist gelinkt
        // bReadOnly = TRUE wuerde dazu fuehren, dass diese Seite nicht
        // bearbeitet werden kann. Dieser Effekt ist jedoch z.Z. nicht
        // gewuenscht, daher auskommentiert:
//        bReadOnly = TRUE;
    }

    return (bReadOnly);
}

/*************************************************************************
|*
|* Beim LinkManager anmelden
|*
\************************************************************************/

void SdPage::ConnectLink()
{
    SvxLinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && !pPageLink && aFileName.Len() && aBookmarkName.Len() &&
        ePageKind==PK_STANDARD && !IsMasterPage() &&
        ( (SdDrawDocument*) pModel)->IsNewOrLoadCompleted())
    {
        /**********************************************************************
        * Anmelden
        * Nur Standardseiten duerfen gelinkt sein
        **********************************************************************/
        ::sd::DrawDocShell* pDocSh = ((SdDrawDocument*) pModel)->GetDocSh();

        if (!pDocSh || pDocSh->GetMedium()->GetOrigURL() != aFileName)
        {
            // Keine Links auf Dokument-eigene Seiten!
            pPageLink = new SdPageLink(this, aFileName, aBookmarkName);
            String aFilterName(SdResId(STR_IMPRESS));
            pLinkManager->InsertFileLink(*pPageLink, OBJECT_CLIENT_FILE,
                                         aFileName, &aFilterName, &aBookmarkName);
            pPageLink->Connect();
        }
    }
}


/*************************************************************************
|*
|* Beim LinkManager abmelden
|*
\************************************************************************/

void SdPage::DisconnectLink()
{
    SvxLinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && pPageLink)
    {
        /**********************************************************************
        * Abmelden
        * (Bei Remove wird *pGraphicLink implizit deleted)
        **********************************************************************/
        pLinkManager->Remove(pPageLink);
        pPageLink=NULL;
    }
}

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/

SdPage::SdPage(const SdPage& rSrcPage) :
    FmFormPage(rSrcPage),
    mpItems(NULL)
{
    ePageKind           = rSrcPage.ePageKind;
    eAutoLayout         = rSrcPage.eAutoLayout;
    bOwnArrangement     = FALSE;

    PresentationObjectList::const_iterator aIter( rSrcPage.maPresObjList.begin() );
    const PresentationObjectList::const_iterator aEnd( rSrcPage.maPresObjList.end() );

    for(; aIter != aEnd; aIter++)
    {
        InsertPresObj(GetObj((*aIter).mpObject->GetOrdNum()), (*aIter).meKind);
    }

    bSelected           = FALSE;
    mnTransitionType    = rSrcPage.mnTransitionType;
    mnTransitionSubtype = rSrcPage.mnTransitionSubtype;
    mbTransitionDirection = rSrcPage.mbTransitionDirection;
    mnTransitionFadeColor = rSrcPage.mnTransitionFadeColor;
    mfTransitionDuration = rSrcPage.mfTransitionDuration;
    ePresChange         = rSrcPage.ePresChange;
    nTime               = rSrcPage.nTime;
    bSoundOn            = rSrcPage.bSoundOn;
    bExcluded           = rSrcPage.bExcluded;

    aLayoutName         = rSrcPage.aLayoutName;
    aSoundFile          = rSrcPage.aSoundFile;
    aCreatedPageName    = String();
    aFileName           = rSrcPage.aFileName;
    aBookmarkName       = rSrcPage.aBookmarkName;
    bScaleObjects       = rSrcPage.bScaleObjects;
    bBackgroundFullSize = rSrcPage.bBackgroundFullSize;
    eCharSet            = rSrcPage.eCharSet;
    nPaperBin           = rSrcPage.nPaperBin;
    eOrientation        = rSrcPage.eOrientation;

    pPageLink           = NULL;    // Wird beim Einfuegen ueber ConnectLink() gesetzt
}



/*************************************************************************
|*
|* Clone
|*
\************************************************************************/

SdrPage* SdPage::Clone() const
{
    return new SdPage(*this);
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
