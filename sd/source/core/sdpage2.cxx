/*************************************************************************
 *
 *  $RCSfile: sdpage2.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-03 08:53:54 $
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
|* schreibt Member der SdPage
|*
\************************************************************************/

//BFS02void SdPage::WriteData(SvStream& rOut) const
//BFS02{
//BFS02 FmFormPage::WriteData( rOut );
//BFS02 // #90477# rOut.SetStreamCharSet( ::GetStoreCharSet( gsl_getSystemTextEncoding()));
//BFS02 rOut.SetStreamCharSet(GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion()));
//BFS02
//BFS02 if ( pModel->IsStreamingSdrModel() )
//BFS02 {
//BFS02     // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
//BFS02     // Anwendungsfall: svdraw Clipboard-Format
//BFS02     return;
//BFS02 }
//BFS02
//BFS02 // letzter Parameter ist die aktuelle Versionsnummer des Codes
//BFS02 SdIOCompat aIO(rOut, STREAM_WRITE, 7);
//BFS02
//BFS02 BOOL bDummy = TRUE;
//BFS02 BOOL bManual = ( PRESCHANGE_MANUAL == ePresChange ); // nur der Kompat.halber
//BFS02
//BFS02 rOut<<bDummy;                      // ehem. bTemplateMode
//BFS02 rOut<<bDummy;                      // ehem. bBackgroundMode
//BFS02 rOut<<bDummy;                      // ehem. bOutlineMode
//BFS02
//BFS02 UINT16 nUI16Temp = (UINT16) eAutoLayout;
//BFS02 rOut << nUI16Temp;
//BFS02
//BFS02 // Selektionskennung ist nicht persistent, wird nicht geschrieben
//BFS02
//BFS02 ULONG nULTemp;
//BFS02 nULTemp = (ULONG)eFadeSpeed;
//BFS02 rOut << nULTemp;
//BFS02 nULTemp = (ULONG)eFadeEffect;
//BFS02 rOut << nULTemp;
//BFS02 rOut << bManual;
//BFS02 rOut << nTime;
//BFS02 rOut << bSoundOn;
//BFS02 rOut << bExcluded;
//BFS02 rOut.WriteByteString( aLayoutName );
//BFS02
//BFS02 // Liste der Praesentationsobjekt abspeichern
//BFS02 UINT32 nUserCallCount = 0;
//BFS02 UINT32 nCount = (UINT32)maPresObjList.size();
//BFS02 UINT32 nValidCount = nCount;
//BFS02 UINT32 nObj;
//BFS02
//BFS02 // NULL-Pointer rauszaehlen. Eigentlich haben die nichts in der Liste
//BFS02 // verloren, aber es gibt leider Kundenfiles, in denen so was vorkommt.
//BFS02 PresentationObjectList::const_iterator aIter = maPresObjList.begin();
//BFS02 for (nObj = 0; nObj < nCount; nObj++)
//BFS02 {
//BFS02     SdrObject* pObj = (*aIter++).mpObject;
//BFS02     if (!pObj)
//BFS02         nValidCount--;
//BFS02 }
//BFS02 rOut << nValidCount;
//BFS02
//BFS02 aIter = maPresObjList.begin();
//BFS02 for (nObj = 0; nObj < nCount; nObj++)
//BFS02 {
//BFS02     SdrObject* pObj = (*aIter++).mpObject;
//BFS02     if (pObj)
//BFS02     {
//BFS02        rOut << pObj->GetOrdNum();
//BFS02
//BFS02        if ( ( (SdPage*) pObj->GetUserCall() ) == this)
//BFS02        {
//BFS02            nUserCallCount++;
//BFS02        }
//BFS02     }
//BFS02 }
//BFS02
//BFS02 nUI16Temp = (UINT16)ePageKind;
//BFS02 rOut << nUI16Temp;
//BFS02
//BFS02 // Liste der Praesentationsobjekt abspeichern,
//BFS02 // welche einen UserCall auf die Seite haben
//BFS02 rOut << nUserCallCount;
//BFS02
//BFS02 aIter = maPresObjList.begin();
//BFS02 for (nObj = 0; nObj < nCount; nObj++)
//BFS02 {
//BFS02     SdrObject* pObj = (*aIter++).mpObject;
//BFS02
//BFS02     if ( pObj && ( (SdPage*) pObj->GetUserCall() ) == this)
//BFS02     {
//BFS02         rOut << pObj->GetOrdNum();
//BFS02     }
//BFS02 }
//BFS02
//BFS02 // #90477# INT16 nI16Temp = ::GetStoreCharSet( gsl_getSystemTextEncoding() );  // .EXEs vor 303 werten den aus
//BFS02 INT16 nI16Temp = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());
//BFS02
//BFS02 rOut << nI16Temp;
//BFS02
//BFS02 rOut.WriteByteString( INetURLObject::AbsToRel(aSoundFile,
//BFS02                                               INetURLObject::WAS_ENCODED,
//BFS02                                               INetURLObject::DECODE_UNAMBIGUOUS));
//BFS02 rOut.WriteByteString( INetURLObject::AbsToRel(aFileName,
//BFS02                                               INetURLObject::WAS_ENCODED,
//BFS02                                               INetURLObject::DECODE_UNAMBIGUOUS));
//BFS02 rOut.WriteByteString( aBookmarkName );
//BFS02
//BFS02 UINT16 nPaperBinTemp = nPaperBin;
//BFS02 rOut << nPaperBinTemp;
//BFS02
//BFS02 UINT16 nOrientationTemp = (UINT16) eOrientation;
//BFS02 rOut << nOrientationTemp;
//BFS02
//BFS02 UINT16 nPresChangeTemp = (UINT16) ePresChange; // ab 370 (IO-Version 7)
//BFS02 rOut << nPresChangeTemp;
//BFS02}

/*************************************************************************
|*
|* liest Member der SdPage
|*
\************************************************************************/

//BFS02void SdPage::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
//BFS02{
//BFS02 FmFormPage::ReadData( rHead, rIn );
//BFS02
//BFS02 // #90477# rIn.SetStreamCharSet( ::GetStoreCharSet( gsl_getSystemTextEncoding()));
//BFS02 rIn.SetStreamCharSet(GetSOLoadTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rIn.GetVersion()));
//BFS02
//BFS02 if ( pModel->IsStreamingSdrModel() )
//BFS02 {
//BFS02     // Es wird nur das SdrModel gestreamt, nicht das SdDrawDocument!
//BFS02     // Anwendungsfall: svdraw Clipboard-Format
//BFS02     return;
//BFS02 }
//BFS02
//BFS02 SdIOCompat aIO(rIn, STREAM_READ);
//BFS02
//BFS02 BOOL bDummy;
//BFS02 BOOL bManual;
//BFS02
//BFS02 rIn>>bDummy;                      // ehem. bTemplateMode
//BFS02 rIn>>bDummy;                      // ehem. bBackgroundMode
//BFS02 rIn>>bDummy;                      // ehem. bOutlineMode
//BFS02
//BFS02 UINT16 nAutoLayout;
//BFS02 rIn>>nAutoLayout;
//BFS02 eAutoLayout = (AutoLayout) nAutoLayout;
//BFS02
//BFS02 // Selektionskennung ist nicht persistent, wird nicht gelesen
//BFS02
//BFS02 ULONG nULTemp;
//BFS02 rIn >> nULTemp; eFadeSpeed  = (FadeSpeed)nULTemp;
//BFS02 rIn >> nULTemp; eFadeEffect = (presentation::FadeEffect)nULTemp;
//BFS02 rIn >> bManual;
//BFS02 rIn >> nTime;
//BFS02 rIn >> bSoundOn;
//BFS02 rIn >> bExcluded;
//BFS02 rIn.ReadByteString( aLayoutName );
//BFS02
//BFS02 // Liste der Praesentationsobjekt einlesen
//BFS02 if (IsObjOrdNumsDirty())        // Ordnungsnummern muessen dafuer stimmen
//BFS02     RecalcObjOrdNums();
//BFS02
//BFS02 UINT32 nCount;
//BFS02 UINT32 nOrdNum;
//BFS02 UINT32 nObj;
//BFS02
//BFS02 rIn >> nCount;
//BFS02 std::vector< SdrObject* > aPresObjList;
//BFS02
//BFS02 for(nObj = 0; nObj < nCount; nObj++)
//BFS02 {
//BFS02     rIn >> nOrdNum;
//BFS02     SdrObject* pObj = GetObj(nOrdNum);
//BFS02
//BFS02     aPresObjList.push_back(pObj);
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=1 eingelesen
//BFS02 if (aIO.GetVersion() >= 1)
//BFS02 {
//BFS02     UINT16 nPageKind;
//BFS02     rIn >> nPageKind;
//BFS02     ePageKind = (PageKind) nPageKind;
//BFS02 }
//BFS02
//BFS02 for(nObj = 0; nObj < aPresObjList.size(); nObj++ )
//BFS02 {
//BFS02     PresObjKind ePresKind = PRESOBJ_NONE;
//BFS02
//BFS02     SdrObject* pObj = aPresObjList[nObj];
//BFS02     SdrObjKind eSdrObjKind = (SdrObjKind) pObj->GetObjIdentifier();
//BFS02
//BFS02     switch( eSdrObjKind )
//BFS02     {
//BFS02     case OBJ_TITLETEXT:
//BFS02         ePresKind = PRESOBJ_TITLE;
//BFS02         break;
//BFS02     case OBJ_OUTLINETEXT:
//BFS02         ePresKind = PRESOBJ_OUTLINE;
//BFS02         break;
//BFS02     case OBJ_TEXT:
//BFS02         if( ePageKind == PK_NOTES )
//BFS02             ePresKind = PRESOBJ_NOTES;
//BFS02         else
//BFS02             ePresKind = PRESOBJ_TEXT;
//BFS02         break;
//BFS02     case OBJ_RECT:
//BFS02         ePresKind = PRESOBJ_BACKGROUND;
//BFS02         break;
//BFS02     case OBJ_GRAF:
//BFS02         ePresKind = PRESOBJ_GRAPHIC;
//BFS02         break;
//BFS02     case OBJ_OLE2:
//BFS02         ePresKind = PRESOBJ_OBJECT;
//BFS02         break;
//BFS02     case OBJ_PAGE:
//BFS02         if( ePageKind == PK_NOTES )
//BFS02             ePresKind = PRESOBJ_PAGE;
//BFS02         else
//BFS02             ePresKind = PRESOBJ_HANDOUT;
//BFS02         break;
//BFS02     }
//BFS02     InsertPresObj( pObj, ePresKind );
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=2 eingelesen
//BFS02 if (aIO.GetVersion() >=2)
//BFS02 {
//BFS02     UINT32 nUserCallCount;
//BFS02     UINT32 nUserCallOrdNum;
//BFS02     rIn >> nUserCallCount;
//BFS02     for (UINT32 nObj = 0; nObj < nUserCallCount; nObj++)
//BFS02     {
//BFS02         rIn >> nUserCallOrdNum;
//BFS02         SdrObject* pObj = GetObj(nUserCallOrdNum);
//BFS02
//BFS02         if (pObj)
//BFS02             pObj->SetUserCall(this);
//BFS02     }
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=3 eingelesen
//BFS02 if (aIO.GetVersion() >=3)
//BFS02 {
//BFS02     INT16 nCharSet;
//BFS02     rIn >> nCharSet;    // nur Einlesen, Konvertierung ab 303 durch Stream
//BFS02
//BFS02     // #90477# eCharSet = (CharSet) nCharSet;
//BFS02     eCharSet = (CharSet)GetSOLoadTextEncoding((rtl_TextEncoding)nCharSet, (sal_uInt16)rIn.GetVersion());
//BFS02
//BFS02     String aSoundFileRel;
//BFS02     rIn.ReadByteString( aSoundFileRel );
//BFS02     INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aSoundFileRel, FALSE,
//BFS02                                                      INetURLObject::WAS_ENCODED,
//BFS02                                                      INetURLObject::DECODE_UNAMBIGUOUS));
//BFS02     aSoundFile = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=4 eingelesen
//BFS02 if (aIO.GetVersion() >=4)
//BFS02 {
//BFS02     String aFileNameRel;
//BFS02     rIn.ReadByteString( aFileNameRel );
//BFS02     INetURLObject aURLObj(::URIHelper::SmartRelToAbs(aFileNameRel, FALSE,
//BFS02                                                      INetURLObject::WAS_ENCODED,
//BFS02                                                      INetURLObject::DECODE_UNAMBIGUOUS));
//BFS02     aFileName = aURLObj.GetMainURL( INetURLObject::NO_DECODE );
//BFS02
//BFS02     rIn.ReadByteString( aBookmarkName );
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=5 eingelesen
//BFS02 if (aIO.GetVersion() >=5)
//BFS02 {
//BFS02     UINT16 nPaperBinTemp;
//BFS02     rIn >> nPaperBinTemp;
//BFS02     nPaperBin = nPaperBinTemp;
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=6 eingelesen
//BFS02 if (aIO.GetVersion() >=6)
//BFS02 {
//BFS02     UINT16 nOrientationTemp;
//BFS02     rIn >> nOrientationTemp;
//BFS02     eOrientation = (Orientation) nOrientationTemp;
//BFS02 }
//BFS02 else
//BFS02 {
//BFS02     // In aelteren Versionen wird die Orientation aus der Seitengroesse bestimmt
//BFS02     Size aPageSize(GetSize());
//BFS02
//BFS02     if (aPageSize.Width() > aPageSize.Height())
//BFS02     {
//BFS02         eOrientation = ORIENTATION_LANDSCAPE;
//BFS02     }
//BFS02     else
//BFS02     {
//BFS02         eOrientation = ORIENTATION_PORTRAIT;
//BFS02     }
//BFS02 }
//BFS02
//BFS02 // ab hier werden Daten der Versionen >=7 eingelesen
//BFS02 if( aIO.GetVersion() >= 7 )
//BFS02 {
//BFS02     UINT16 nPresChangeTemp;
//BFS02     rIn >> nPresChangeTemp;
//BFS02     ePresChange = (PresChange) nPresChangeTemp;
//BFS02 }
//BFS02 else
//BFS02     ePresChange = ( bManual ? PRESCHANGE_MANUAL : PRESCHANGE_AUTO );
//BFS02}



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

