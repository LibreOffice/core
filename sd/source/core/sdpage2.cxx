/*************************************************************************
 *
 *  $RCSfile: sdpage2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
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

#ifndef SVX_LIGHT
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#endif // !SVX_LIGHT

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

#pragma hdrstop

#include "sdresid.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "glob.hrc"
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "sdiocmpt.hxx"
#include "pglink.hxx"
#include "strmname.h"

#ifndef SVX_LIGHT
#ifdef MAC
#include "::ui:inc:strings.hrc"
#include "::ui:inc:docshell.hxx"
#else
#ifdef UNX
#include "../ui/inc/strings.hrc"
#include "../ui/inc/docshell.hxx"
#else
#include "..\ui\inc\cfgids.hxx"
#include "..\ui\inc\strings.hrc"
#include "..\ui\inc\docshell.hxx"
#endif
#endif
#endif // !SVX_LIGHT

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

#ifndef SVX_LIGHT
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
            for ( nMaster = nMasterCount - 1; nMaster >= 0; nMaster-- )
            {
                pMaster = (SdPage*)pModel->GetMasterPage(nMaster);
                if (pMaster->GetPageKind() == ePageKind &&
                    pMaster->GetLayoutName() == aLayoutName)
                {
                    break;
                }
            }
        }

        DBG_ASSERT(nMaster < nMasterCount, "Masterpage nicht gefunden");

        // falls es eine oder mehrere Masterpages gibt: die 1. ersetzen
        if (GetMasterPageCount() > 0)
            SetMasterPageNum(nMaster, 0);
        // sonst: als 1. Masterpage einfuegen
        else
            InsertMasterPage(nMaster);
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
                        ( (SdrTextObj*) pObj)->GetOutlinerParaObject()->SetStyleSheets( i,  aFullName, SD_LT_FAMILY );
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
#endif // !SVX_LIGHT

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

#ifndef SVX_LIGHT
void __EXPORT SdPage::WriteData(SvStream& rOut) const
{
    FmFormPage::WriteData( rOut );
    rOut.SetStreamCharSet( ::GetStoreCharSet( gsl_getSystemTextEncoding()));

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
    UINT32 nCount = (UINT32)aPresObjList.Count();
    UINT32 nValidCount = nCount;

    // NULL-Pointer rauszaehlen. Eigentlich haben die nichts in der Liste
    // verloren, aber es gibt leider Kundenfiles, in denen so was vorkommt.
    for (UINT32 nObj = 0; nObj < nCount; nObj++)
    {
        SdrObject* pObj = (SdrObject*)aPresObjList.GetObject(nObj);
        if (!pObj)
            nValidCount--;
    }
    rOut << nValidCount;

    for (nObj = 0; nObj < nCount; nObj++)
    {
        SdrObject* pObj = (SdrObject*)aPresObjList.GetObject(nObj);
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
    for (nObj = 0; nObj < nCount; nObj++)
    {
        SdrObject* pObj = (SdrObject*)aPresObjList.GetObject(nObj);

        if ( pObj && ( (SdPage*) pObj->GetUserCall() ) == this)
        {
            rOut << pObj->GetOrdNum();
        }
    }

    INT16 nI16Temp = ::GetStoreCharSet( gsl_getSystemTextEncoding() );  // .EXEs vor 303 werten den aus
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
#endif // !SVX_LIGHT

/*************************************************************************
|*
|* liest Member der SdPage
|*
\************************************************************************/

void __EXPORT SdPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    FmFormPage::ReadData( rHead, rIn );
    rIn.SetStreamCharSet( ::GetStoreCharSet( gsl_getSystemTextEncoding()));

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
    rIn >> nCount;
    for (UINT32 nObj = 0; nObj < nCount; nObj++)
    {
        rIn >> nOrdNum;
        SdrObject* pObj = GetObj(nOrdNum);
        aPresObjList.Insert(pObj, LIST_APPEND);
    }

    // ab hier werden Daten der Versionen >=1 eingelesen
    if (aIO.GetVersion() >= 1)
    {
        UINT16 nPageKind;
        rIn >> nPageKind;
        ePageKind = (PageKind) nPageKind;
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
        eCharSet = (CharSet) nCharSet;

        String aSoundFileRel;
        rIn.ReadByteString( aSoundFileRel );
        INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aSoundFileRel, FALSE,
                                                         INetURLObject::WAS_ENCODED,
                                                         INetURLObject::DECODE_UNAMBIGUOUS));
        aSoundFile = aURLObj.PathToFileName();
    }

    // ab hier werden Daten der Versionen >=4 eingelesen
    if (aIO.GetVersion() >=4)
    {
        String aFileNameRel;
        rIn.ReadByteString( aFileNameRel );
        INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aFileNameRel, FALSE,
                                                         INetURLObject::WAS_ENCODED,
                                                         INetURLObject::DECODE_UNAMBIGUOUS));
        aFileName = aURLObj.GetMainURL();

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

void __EXPORT SdPage::SetModel(SdrModel* pNewModel)
{
#ifndef SVX_LIGHT
    DisconnectLink();
#endif

    // Model umsetzen
    FmFormPage::SetModel(pNewModel);

#ifndef SVX_LIGHT
    ConnectLink();
#endif
}



/*************************************************************************
|*
|* Ist die Seite read-only?
|*
\************************************************************************/

FASTBOOL __EXPORT SdPage::IsReadOnly() const
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

#ifndef SVX_LIGHT
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
        SdDrawDocShell* pDocSh = ((SdDrawDocument*) pModel)->GetDocSh();

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
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Beim LinkManager abmelden
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdPage::DisconnectLink()
{
    SvxLinkManager* pLinkManager = pModel!=NULL ? pModel->GetLinkManager() : NULL;

    if (pLinkManager && pPageLink)
    {
        /**********************************************************************
        * Abmelden
        * (Bei Remove wird *pGraphicLink implizit deleted)
        **********************************************************************/
        pLinkManager->Remove(*pPageLink);
        pPageLink=NULL;
    }
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/

SdPage::SdPage(const SdPage& rSrcPage) :
    FmFormPage(rSrcPage)
{
    ePageKind           = rSrcPage.ePageKind;
    eAutoLayout         = rSrcPage.eAutoLayout;
    bOwnArrangement     = FALSE;

    UINT32 nCount = (UINT32) rSrcPage.aPresObjList.Count();

    for (UINT32 nObj = 0; nObj < nCount; nObj++)
    {
        // Liste der Praesenationsobjekte fuellen
        SdrObject* pSrcObj = (SdrObject*) rSrcPage.aPresObjList.GetObject(nObj);

        if (pSrcObj)
        {
            aPresObjList.Insert(GetObj(pSrcObj->GetOrdNum()), LIST_APPEND);
        }
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
    return(pPage);
}




