/*************************************************************************
 *
 *  $RCSfile: sdpage2.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:56:30 $
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
#include "sdiocmpt.hxx"
#include "pglink.hxx"
#include "strmname.h"
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
        USHORT nMaster = 0;
        USHORT nMasterCount = pModel->GetMasterPageCount();

        if( !bReverseOrder )
        {
            for ( nMaster = 0; nMaster < nMasterCount; nMaster++ )
            {
                pMaster = (SdPage*)pModel->GetMasterPage(nMaster);
                if (pMaster->GetPageKind() == ePageKind &&
                    pMaster->GetLayoutName() == aLayoutName)
                {
                    break;
                }
            }
        }
        else
        {
            for ( nMaster = nMasterCount; nMaster > 0; nMaster-- )
            {
                pMaster = (SdPage*)pModel->GetMasterPage(nMaster - 1);
                if (pMaster->GetPageKind() == ePageKind &&
                    pMaster->GetLayoutName() == aLayoutName)
                {
                    break;
                }
            }
        }

        DBG_ASSERT(nMaster < nMasterCount, "Masterpage nicht gefunden");

        // falls es eine oder mehrere Masterpages gibt: die 1. ersetzen
        TRG_SetMasterPage(*pModel->GetMasterPage(nMaster));
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
|* schreibt Member der SdPage
|*
\************************************************************************/

void SdPage::WriteData(SvStream& rOut) const
{
    FmFormPage::WriteData( rOut );
    // #90477# rOut.SetStreamCharSet( ::GetStoreCharSet( gsl_getSystemTextEncoding()));
    rOut.SetStreamCharSet(GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion()));

    if ( pModel->IsStreamingSdrModel() )
    {
        // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
        // Anwendungsfall: svdraw Clipboard-Format
        return;
    }

    // letzter Parameter ist die aktuelle Versionsnummer des Codes
    SdIOCompat aIO(rOut, STREAM_WRITE, 7);

    BOOL bDummy = TRUE;
    BOOL bManual = ( PRESCHANGE_MANUAL == ePresChange ); // nur der Kompat.halber

    rOut<<bDummy;                      // ehem. bTemplateMode
    rOut<<bDummy;                      // ehem. bBackgroundMode
    rOut<<bDummy;                      // ehem. bOutlineMode

    UINT16 nUI16Temp = (UINT16) eAutoLayout;
    rOut << nUI16Temp;

    // Selektionskennung ist nicht persistent, wird nicht geschrieben

    ULONG nULTemp;
    nULTemp = (ULONG)eFadeSpeed;
    rOut << nULTemp;
    nULTemp = (ULONG)eFadeEffect;
    rOut << nULTemp;
    rOut << bManual;
    rOut << nTime;
    rOut << bSoundOn;
    rOut << bExcluded;
    rOut.WriteByteString( aLayoutName );

    // Liste der Praesentationsobjekt abspeichern
    UINT32 nUserCallCount = 0;
    UINT32 nCount = (UINT32)maPresObjList.size();
    UINT32 nValidCount = nCount;
    UINT32 nObj;

    // NULL-Pointer rauszaehlen. Eigentlich haben die nichts in der Liste
    // verloren, aber es gibt leider Kundenfiles, in denen so was vorkommt.
    PresentationObjectList::const_iterator aIter = maPresObjList.begin();
    for (nObj = 0; nObj < nCount; nObj++)
    {
        SdrObject* pObj = (*aIter++).mpObject;
        if (!pObj)
            nValidCount--;
    }
    rOut << nValidCount;

    aIter = maPresObjList.begin();
    for (nObj = 0; nObj < nCount; nObj++)
    {
        SdrObject* pObj = (*aIter++).mpObject;
        if (pObj)
        {
           rOut << pObj->GetOrdNum();

           if ( ( (SdPage*) pObj->GetUserCall() ) == this)
           {
               nUserCallCount++;
           }
        }
    }

    nUI16Temp = (UINT16)ePageKind;
    rOut << nUI16Temp;

    // Liste der Praesentationsobjekt abspeichern,
    // welche einen UserCall auf die Seite haben
    rOut << nUserCallCount;

    aIter = maPresObjList.begin();
    for (nObj = 0; nObj < nCount; nObj++)
    {
        SdrObject* pObj = (*aIter++).mpObject;

        if ( pObj && ( (SdPage*) pObj->GetUserCall() ) == this)
        {
            rOut << pObj->GetOrdNum();
        }
    }

    // #90477# INT16 nI16Temp = ::GetStoreCharSet( gsl_getSystemTextEncoding() );  // .EXEs vor 303 werten den aus
    INT16 nI16Temp = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());

    rOut << nI16Temp;

    rOut.WriteByteString( INetURLObject::AbsToRel(aSoundFile,
                                                  INetURLObject::WAS_ENCODED,
                                                  INetURLObject::DECODE_UNAMBIGUOUS));
    rOut.WriteByteString( INetURLObject::AbsToRel(aFileName,
                                                  INetURLObject::WAS_ENCODED,
                                                  INetURLObject::DECODE_UNAMBIGUOUS));
    rOut.WriteByteString( aBookmarkName );

    UINT16 nPaperBinTemp = nPaperBin;
    rOut << nPaperBinTemp;

    UINT16 nOrientationTemp = (UINT16) eOrientation;
    rOut << nOrientationTemp;

    UINT16 nPresChangeTemp = (UINT16) ePresChange; // ab 370 (IO-Version 7)
    rOut << nPresChangeTemp;
}

/*************************************************************************
|*
|* liest Member der SdPage
|*
\************************************************************************/

void SdPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    FmFormPage::ReadData( rHead, rIn );

    // #90477# rIn.SetStreamCharSet( ::GetStoreCharSet( gsl_getSystemTextEncoding()));
    rIn.SetStreamCharSet(GetSOLoadTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rIn.GetVersion()));

    if ( pModel->IsStreamingSdrModel() )
    {
        // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
        // Anwendungsfall: svdraw Clipboard-Format
        return;
    }

    SdIOCompat aIO(rIn, STREAM_READ);

    BOOL bDummy;
    BOOL bManual;

    rIn>>bDummy;                      // ehem. bTemplateMode
    rIn>>bDummy;                      // ehem. bBackgroundMode
    rIn>>bDummy;                      // ehem. bOutlineMode

    UINT16 nAutoLayout;
    rIn>>nAutoLayout;
    eAutoLayout = (AutoLayout) nAutoLayout;

    // Selektionskennung ist nicht persistent, wird nicht gelesen

    ULONG nULTemp;
    rIn >> nULTemp; eFadeSpeed  = (FadeSpeed)nULTemp;
    rIn >> nULTemp; eFadeEffect = (presentation::FadeEffect)nULTemp;
    rIn >> bManual;
    rIn >> nTime;
    rIn >> bSoundOn;
    rIn >> bExcluded;
    rIn.ReadByteString( aLayoutName );

    // Liste der Praesentationsobjekt einlesen
    if (IsObjOrdNumsDirty())        // Ordnungsnummern muessen dafuer stimmen
        RecalcObjOrdNums();

    UINT32 nCount;
    UINT32 nOrdNum;
    UINT32 nObj;

    rIn >> nCount;
    std::vector< SdrObject* > aPresObjList;

    for(nObj = 0; nObj < nCount; nObj++)
    {
        rIn >> nOrdNum;
        SdrObject* pObj = GetObj(nOrdNum);

        aPresObjList.push_back(pObj);
    }

    // ab hier werden Daten der Versionen >=1 eingelesen
    if (aIO.GetVersion() >= 1)
    {
        UINT16 nPageKind;
        rIn >> nPageKind;
        ePageKind = (PageKind) nPageKind;
    }

    for(nObj = 0; nObj < aPresObjList.size(); nObj++ )
    {
        PresObjKind ePresKind = PRESOBJ_NONE;

        SdrObject* pObj = aPresObjList[nObj];
        SdrObjKind eSdrObjKind = (SdrObjKind) pObj->GetObjIdentifier();

        switch( eSdrObjKind )
        {
        case OBJ_TITLETEXT:
            ePresKind = PRESOBJ_TITLE;
            break;
        case OBJ_OUTLINETEXT:
            ePresKind = PRESOBJ_OUTLINE;
            break;
        case OBJ_TEXT:
            if( ePageKind == PK_NOTES )
                ePresKind = PRESOBJ_NOTES;
            else
                ePresKind = PRESOBJ_TEXT;
            break;
        case OBJ_RECT:
            ePresKind = PRESOBJ_BACKGROUND;
            break;
        case OBJ_GRAF:
            ePresKind = PRESOBJ_GRAPHIC;
            break;
        case OBJ_OLE2:
            ePresKind = PRESOBJ_OBJECT;
            break;
        case OBJ_PAGE:
            if( ePageKind == PK_NOTES )
                ePresKind = PRESOBJ_PAGE;
            else
                ePresKind = PRESOBJ_HANDOUT;
            break;
        }
        InsertPresObj( pObj, ePresKind );
    }

    // ab hier werden Daten der Versionen >=2 eingelesen
    if (aIO.GetVersion() >=2)
    {
        UINT32 nUserCallCount;
        UINT32 nUserCallOrdNum;
        rIn >> nUserCallCount;
        for (UINT32 nObj = 0; nObj < nUserCallCount; nObj++)
        {
            rIn >> nUserCallOrdNum;
            SdrObject* pObj = GetObj(nUserCallOrdNum);

            if (pObj)
                pObj->SetUserCall(this);
        }
    }

    // ab hier werden Daten der Versionen >=3 eingelesen
    if (aIO.GetVersion() >=3)
    {
        INT16 nCharSet;
        rIn >> nCharSet;    // nur Einlesen, Konvertierung ab 303 durch Stream

        // #90477# eCharSet = (CharSet) nCharSet;
        eCharSet = (CharSet)GetSOLoadTextEncoding((rtl_TextEncoding)nCharSet, (sal_uInt16)rIn.GetVersion());

        String aSoundFileRel;
        rIn.ReadByteString( aSoundFileRel );
        INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aSoundFileRel, FALSE,
                                                         INetURLObject::WAS_ENCODED,
                                                         INetURLObject::DECODE_UNAMBIGUOUS));
        aSoundFile = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
    }

    // ab hier werden Daten der Versionen >=4 eingelesen
    if (aIO.GetVersion() >=4)
    {
        String aFileNameRel;
        rIn.ReadByteString( aFileNameRel );
        INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aFileNameRel, FALSE,
                                                         INetURLObject::WAS_ENCODED,
                                                         INetURLObject::DECODE_UNAMBIGUOUS));
        aFileName = aURLObj.GetMainURL( INetURLObject::NO_DECODE );

        rIn.ReadByteString( aBookmarkName );
    }

    // ab hier werden Daten der Versionen >=5 eingelesen
    if (aIO.GetVersion() >=5)
    {
        UINT16 nPaperBinTemp;
        rIn >> nPaperBinTemp;
        nPaperBin = nPaperBinTemp;
    }

    // ab hier werden Daten der Versionen >=6 eingelesen
    if (aIO.GetVersion() >=6)
    {
        UINT16 nOrientationTemp;
        rIn >> nOrientationTemp;
        eOrientation = (Orientation) nOrientationTemp;
    }
    else
    {
        // In aelteren Versionen wird die Orientation aus der Seitengroesse bestimmt
        Size aPageSize(GetSize());

        if (aPageSize.Width() > aPageSize.Height())
        {
            eOrientation = ORIENTATION_LANDSCAPE;
        }
        else
        {
            eOrientation = ORIENTATION_PORTRAIT;
        }
    }

    // ab hier werden Daten der Versionen >=7 eingelesen
    if( aIO.GetVersion() >= 7 )
    {
        UINT16 nPresChangeTemp;
        rIn >> nPresChangeTemp;
        ePresChange = (PresChange) nPresChangeTemp;
    }
    else
        ePresChange = ( bManual ? PRESCHANGE_MANUAL : PRESCHANGE_AUTO );
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
    eFadeSpeed          = rSrcPage.eFadeSpeed;
    eFadeEffect         = rSrcPage.eFadeEffect;
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
    SdPage* pPage = new SdPage(*this);

    if( (PK_STANDARD == ePageKind) && !IsMasterPage() )
    {
        // preserve presentation order on slide duplications
        SdrObjListIter aSrcIter( *this, IM_DEEPWITHGROUPS );
        SdrObjListIter aDstIter( *pPage, IM_DEEPWITHGROUPS );

        while( aSrcIter.IsMore() && aDstIter.IsMore() )
        {
            SdrObject* pSrc = aSrcIter.Next();
            SdrObject* pDst = aDstIter.Next();

            SdAnimationInfo* pSrcInfo = ((SdDrawDocument*)pModel)->GetAnimationInfo(pSrc);
            if( pSrcInfo && (pSrcInfo->nPresOrder != LIST_APPEND) )
            {
                SdAnimationInfo* pDstInfo = ((SdDrawDocument*)pModel)->GetAnimationInfo(pDst);
                DBG_ASSERT( pDstInfo, "shape should have an animation info after clone!" );

                if( pDstInfo )
                    pDstInfo->nPresOrder = pSrcInfo->nPresOrder;
            }
        }

        DBG_ASSERT( !aSrcIter.IsMore() && !aDstIter.IsMore(), "unequal shape numbers after a page clone?" );
    }

    return(pPage);
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

