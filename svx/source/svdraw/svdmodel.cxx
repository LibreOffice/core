/*************************************************************************
 *
 *  $RCSfile: svdmodel.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-16 09:45:13 $
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

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "svdmodel.hxx"
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#include <unotools/ucbstreamhelper.hxx>

#ifndef _STRING_H
#include <tools/string.hxx>
#endif

#include "svditext.hxx"
#include "editeng.hxx"   // Fuer EditEngine::CreatePool()

#include "xtable.hxx"
#include "xoutx.hxx"

#include "svditer.hxx"
#include "svdtrans.hxx"
#include "svdio.hxx"
#include "svdpage.hxx"
#include "svdlayer.hxx"
#include "svdundo.hxx"
#include "svdpool.hxx"
#include "svdobj.hxx"
#include "svdotext.hxx"  // fuer ReformatAllTextObjects und CalcFieldValue
#include "svdetc.hxx"
#include "svdoutl.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname

#include <eeitemid.hxx>

#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include "colritem.hxx"
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX //autogen
#include <numitem.hxx>
#endif

#ifndef _SVX_BULITEM_HXX //autogen
#include <bulitem.hxx>
#endif

#ifndef _OUTLOBJ_HXX
#include <outlobj.hxx>
#endif

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrModelInfo::SdrModelInfo(FASTBOOL bInit):
    aCreationDate(Date(0),Time(0)),
    aLastWriteDate(Date(0),Time(0)),
    aLastReadDate(Date(0),Time(0)),
    aLastPrintDate(Date(0),Time(0)),
    eCreationCharSet(RTL_TEXTENCODING_DONTKNOW),
    /*  old SV-stuff, there is no possibility to determine this informations in another way
    eCreationGUI(GUI_DONTKNOW),
    eCreationCPU(CPU_DONTKNOW),
    eCreationSys(SYSTEM_DONTKNOW),
    */
    eLastWriteCharSet(RTL_TEXTENCODING_DONTKNOW),
    /*  old SV-stuff, there is no possibility to determine this informations in another way
    eLastWriteGUI(GUI_DONTKNOW),
    eLastWriteCPU(CPU_DONTKNOW),
    eLastWriteSys(SYSTEM_DONTKNOW),
    */
    eLastReadCharSet(RTL_TEXTENCODING_DONTKNOW)
    /*  old SV-stuff, there is no possibility to determine this informations in another way
    eLastReadGUI(GUI_DONTKNOW),
    eLastReadCPU(CPU_DONTKNOW),
    eLastReadSys(SYSTEM_DONTKNOW)
    */
{
    if (bInit)
    {
        aCreationDate = DateTime();
        eCreationCharSet = gsl_getSystemTextEncoding();
        /*  old SV-stuff, there is no possibility to determine this informations in another way
        eCreationGUI=System::GetGUIType();
        eCreationSys=System::GetSystemType();
        */
    }
}

SvStream& operator<<(SvStream& rOut, const SdrModelInfo& rModInfo)
{
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrModelInfo");
#endif
    rOut<<UINT32(rModInfo.aCreationDate.GetDate());
    rOut<<UINT32(rModInfo.aCreationDate.GetTime());
    rOut<<UINT8( GetStoreCharSet( rModInfo.eCreationCharSet ) );

    /* Since we removed old SV-stuff there is no way to determine system-speciefic informations, yet.
       We just have to write anythink in the file for compatibility:
            eCreationGUI    eLastWriteGUI   eLastReadGUI
            eCreationCPU    eLastWriteCPU   eLastReadCPU
            eCreationSys    eLastWriteSys   eLastReadSys


    */
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eCreationGUI);
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eCreationCPU);
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eCreationSys);

    rOut<<UINT32(rModInfo.aLastWriteDate.GetDate());
    rOut<<UINT32(rModInfo.aLastWriteDate.GetTime());
    rOut<<UINT8( GetStoreCharSet( rModInfo.eLastWriteCharSet ) );

    // see comment above
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eLastWriteGUI);
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eLastWriteCPU);
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eLastWriteSys);

    rOut<<UINT32(rModInfo.aLastReadDate.GetDate());
    rOut<<UINT32(rModInfo.aLastReadDate.GetTime());
    rOut<<UINT8( GetStoreCharSet( rModInfo.eLastReadCharSet ) );

    // see comment above
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eLastReadGUI);
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eLastReadCPU);
    rOut<<UINT8(0);     //  rOut<<UINT8(rModInfo.eLastReadSys);

    rOut<<UINT32(rModInfo.aLastPrintDate.GetDate());
    rOut<<UINT32(rModInfo.aLastPrintDate.GetTime());
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrModelInfo& rModInfo)
{
    if (rIn.GetError()!=0) return rIn;
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrModelInfo");
#endif
    UINT8  n8;
    UINT32 n32;
    rIn>>n32; rModInfo.aCreationDate.SetDate(n32);
    rIn>>n32; rModInfo.aCreationDate.SetTime(n32);
    rIn>>n8;  rModInfo.eCreationCharSet=rtl_TextEncoding(n8);

    /* Since we removed old SV-stuff there is no way to determine system-speciefic informations, yet.
       We just have to write anythink in the file for compatibility:
            eCreationGUI    eLastWriteGUI   eLastReadGUI
            eCreationCPU    eLastWriteCPU   eLastReadCPU
            eCreationSys    eLastWriteSys   eLastReadSys


    */
    rIn>>n8;  //    rModInfo.eCreationGUI=GUIType(n8);
    rIn>>n8;  //    rModInfo.eCreationCPU=CPUType(n8);
    rIn>>n8;  //    rModInfo.eCreationSys=SystemType(n8);
    rIn>>n32; rModInfo.aLastWriteDate.SetDate(n32);
    rIn>>n32; rModInfo.aLastWriteDate.SetTime(n32);
    rIn>>n8;  rModInfo.eLastWriteCharSet=rtl_TextEncoding(n8);

    // see comment above
    rIn>>n8;  //    rModInfo.eLastWriteGUI=GUIType(n8);
    rIn>>n8;  //    rModInfo.eLastWriteCPU=CPUType(n8);
    rIn>>n8;  //    rModInfo.eLastWriteSys=SystemType(n8);

    rIn>>n32; rModInfo.aLastReadDate.SetDate(n32);
    rIn>>n32; rModInfo.aLastReadDate.SetTime(n32);
    rIn>>n8;  rModInfo.eLastReadCharSet=rtl_TextEncoding(n8);

    // see comment above
    rIn>>n8;  //    rModInfo.eLastReadGUI=GUIType(n8);
    rIn>>n8;  //    rModInfo.eLastReadCPU=CPUType(n8);
    rIn>>n8;  //    rModInfo.eLastReadSys=SystemType(n8);

    rIn>>n32; rModInfo.aLastPrintDate.SetDate(n32);
    rIn>>n32; rModInfo.aLastPrintDate.SetTime(n32);

    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DBG_NAME(SdrModel);
TYPEINIT1(SdrModel,SfxBroadcaster);

void SdrModel::ImpCtor(SfxItemPool* pPool, SvPersist* pPers,
    FASTBOOL bUseExtColorTable, FASTBOOL bLoadRefCounts)
{
    aObjUnit=SdrEngineDefaults::GetMapFraction();
    eObjUnit=SdrEngineDefaults::GetMapUnit();
    eUIUnit=FUNIT_MM;
    aUIScale=Fraction(1,1);
    nUIUnitKomma=0;
    bUIOnlyKomma=FALSE;
    pLayerAdmin=NULL;
    pItemPool=pPool;
    bMyPool=FALSE;
    pPersist=pPers;
    pDrawOutliner=NULL;
    pHitTestOutliner=NULL;
    nDefTextHgt=SdrEngineDefaults::GetFontHeight();
    pRefOutDev=NULL;
    nProgressAkt=0;
    nProgressMax=0;
    nProgressOfs=0;
    pStyleSheetPool=NULL;
    pDefaultStyleSheet=NULL;
    pLinkManager=NULL;
    pLoadedModel=NULL;
    pUndoStack=NULL;
    pRedoStack=NULL;
    pAktPaintPV=NULL;
    nMaxUndoCount=16;
    pAktUndoGroup=NULL;
    nUndoLevel=0;
    nProgressPercent=0;
    nLoadVersion=0;
    bExtColorTable=FALSE;
    bChanged=FALSE;
    bInfoChanged=FALSE;
    bPagNumsDirty=FALSE;
    bMPgNumsDirty=FALSE;
    bPageNotValid=FALSE;
    bSavePortable=FALSE;
    bSaveCompressed=FALSE;
    bSaveNative=FALSE;
    bSwapGraphics=FALSE;
    nSwapGraphicsMode=SDR_SWAPGRAPHICSMODE_DEFAULT;
    bSaveOLEPreview=FALSE;
    bPasteResize=FALSE;
    bNoBitmapCaching=FALSE;
    bLoading=FALSE;
    bStreamingSdrModel=FALSE;
    bReadOnly=FALSE;
    nStreamCompressMode=COMPRESSMODE_NONE;
    nStreamNumberFormat=NUMBERFORMAT_INT_BIGENDIAN;
    nDefaultTabulator=0;
    pColorTable=NULL;
    pDashList=NULL;
    pLineEndList=NULL;
    pHatchList=NULL;
    pGradientList=NULL;
    pBitmapList=NULL;
    bTransparentTextFrames=FALSE;
    bStarDrawPreviewMode = FALSE;
    nStarDrawPreviewMasterPageNum = SDRPAGE_NOTFOUND;
    pModelStorage = NULL;

#ifdef __LITTLEENDIAN
    nStreamNumberFormat=NUMBERFORMAT_INT_LITTLEENDIAN;
#endif
    bExtColorTable=bUseExtColorTable;

    if (pPool==NULL) {
        pItemPool=new SdrItemPool(SDRATTR_START, SDRATTR_END, bLoadRefCounts);
        // Der Outliner hat keinen eigenen Pool, deshalb den der EditEngine
        SfxItemPool* pOutlPool=EditEngine::CreatePool( bLoadRefCounts );
        // OutlinerPool als SecondaryPool des SdrPool
        pItemPool->SetSecondaryPool(pOutlPool);
        // Merken, dass ich mir die beiden Pools selbst gemacht habe
        bMyPool=TRUE;
    }
    pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
    SetTextDefaults();

    pLayerAdmin=new SdrLayerAdmin;
    pLayerAdmin->SetModel(this);
    ImpSetUIUnit();

    // den DrawOutliner OnDemand erzeugen geht noch nicht, weil ich den Pool
    // sonst nicht kriege (erst ab 302!)
    pDrawOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this );
    ImpSetOutlinerDefaults(pDrawOutliner, TRUE);

    pHitTestOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this );
    ImpSetOutlinerDefaults(pHitTestOutliner, TRUE);

    ImpCreateTables();
}

SdrModel::SdrModel(SfxItemPool* pPool, SvPersist* pPers, INT32 bLoadRefCounts):
    aInfo(TRUE),
    aPages(1024,32,32),
    aMaPag(1024,32,32)
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,FALSE, (FASTBOOL)bLoadRefCounts);
}

SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, SvPersist* pPers, INT32 bLoadRefCounts):
    aInfo(TRUE),
    aPages(1024,32,32),
    aMaPag(1024,32,32),
    aTablePath(rPath)
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,FALSE, (FASTBOOL)bLoadRefCounts);
}

SdrModel::SdrModel(SfxItemPool* pPool, SvPersist* pPers, FASTBOOL bUseExtColorTable, INT32 bLoadRefCounts):
    aInfo(TRUE),
    aPages(1024,32,32),
    aMaPag(1024,32,32)
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,bUseExtColorTable, (FASTBOOL)bLoadRefCounts);
}

SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, SvPersist* pPers, FASTBOOL bUseExtColorTable, INT32 bLoadRefCounts):
    aInfo(TRUE),
    aPages(1024,32,32),
    aMaPag(1024,32,32),
    aTablePath(rPath)
{
    DBG_CTOR(SdrModel,NULL);
    ImpCtor(pPool,pPers,bUseExtColorTable, (FASTBOOL)bLoadRefCounts);
}

SdrModel::SdrModel(const SdrModel& rSrcModel):
    aPages(1024,32,32),
    aMaPag(1024,32,32)
{
    // noch nicht implementiert
    DBG_ERROR("SdrModel::CopyCtor() ist noch nicht implementiert");
}

SdrModel::~SdrModel()
{
    DBG_DTOR(SdrModel,NULL);
    Broadcast(SdrHint(HINT_MODELCLEARED));

    ClearUndoBuffer();
#ifdef DBG_UTIL
    if(pAktUndoGroup)
    {
        ByteString aStr("Im Dtor des SdrModel steht noch ein offenes Undo rum: \"");

        aStr += ByteString(pAktUndoGroup->GetComment(), gsl_getSystemTextEncoding());
        aStr += '\"';

        DBG_ERROR(aStr.GetBuffer());
    }
#endif
    if (pAktUndoGroup!=NULL) delete pAktUndoGroup;
    Clear();
    delete pLayerAdmin;

    // Den DrawOutliner erst nach dem ItemPool loeschen, da
    // der ItemPool Items des DrawOutliners referenziert !!! (<- das war mal)
    // Wg. Problem bei Malte Reihenfolge wieder umgestellt.
    // Loeschen des Outliners vor dem loeschen des ItemPools
    delete pHitTestOutliner;
    delete pDrawOutliner;

    // StyleSheetPool loeschen, abgeleitete Klassen sollten dies nicht tun,
    // da die DrawingEngine moeglicherweise im Destruktor noch auf den
    // StyleSheetPool zugreifen muss (SB)
    delete pStyleSheetPool;

    if (bMyPool) { // Pools loeschen, falls es meine sind
        SfxItemPool* pOutlPool=pItemPool->GetSecondaryPool();
        delete pItemPool;
        // Der OutlinerPool muss nach dem ItemPool plattgemacht werden, da der
        // ItemPool SetItems enthaelt die ihrerseits Items des OutlinerPools
        // referenzieren (Joe)
        delete pOutlPool;
    }

    delete pLoadedModel;

#ifndef SVX_LIGHT
    // Tabellen, Listen und Paletten loeschen
    if (!bExtColorTable) delete pColorTable;
    delete pDashList;
    delete pLineEndList;
    delete pHatchList;
    delete pGradientList;
    delete pBitmapList;
#endif
}

// noch nicht implementiert:
void SdrModel::operator=(const SdrModel& rSrcModel)
{
    DBG_ERROR("SdrModel::operator=() ist noch nicht implementiert");
}

FASTBOOL SdrModel::operator==(const SdrModel& rCmpModel) const
{
    DBG_ERROR("SdrModel::operator==() ist noch nicht implementiert");
    return FALSE;
}

void SdrModel::SetSwapGraphics( FASTBOOL bSwap )
{
    bSwapGraphics = bSwap;
}

FASTBOOL SdrModel::IsReadOnly() const
{
    return bReadOnly;
}

void SdrModel::SetReadOnly(FASTBOOL bYes)
{
    bReadOnly=bYes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrModel::DoProgress(ULONG nVal)
{
    if (aIOProgressLink.IsSet()) {
        if (nVal==0) { // Anfang
            USHORT nVal=0;
            aIOProgressLink.Call(&nVal);
            nProgressPercent=0;
            nProgressAkt=0;
        } else if (nVal==0xFFFFFFFF) { // Ende
            USHORT nVal=100;
            aIOProgressLink.Call(&nVal);
            nProgressPercent=100;
            nProgressAkt=nProgressMax;
        } else if (nVal!=nProgressAkt && nProgressMax!=0) { // dazwischen
            USHORT nPercent;
            if (nVal>nProgressOfs) nVal-=nProgressOfs; else nVal=0;
            if (nVal>nProgressMax) nVal=nProgressMax;
            if (nVal<=0x00FFFFFF) nPercent=USHORT(nVal*100/nProgressMax);
            else {
                ULONG nBla=nProgressMax/100; // Weil sonst Ueberlauf!
                nPercent=USHORT(nVal/=nBla);
            }
            if (nPercent==0) nPercent=1;
            if (nPercent>99) nPercent=99;
            if (nPercent>nProgressPercent) {
                aIOProgressLink.Call(&nPercent);
                nProgressPercent=nPercent;
            }
            if (nVal>nProgressAkt) {
                nProgressAkt=nVal;
            }
        }
    }
}

void SdrModel::SetMaxUndoActionCount(ULONG nAnz)
{
    if (nAnz<1) nAnz=1;
    nMaxUndoCount=nAnz;
    if (pUndoStack!=NULL) {
        while (pUndoStack->Count()>nMaxUndoCount) {
            delete (SfxUndoAction*) pUndoStack->Remove(pUndoStack->Count());
        }
    }
}

void SdrModel::ClearUndoBuffer()
{
    if (pUndoStack!=NULL) {
        while (pUndoStack->Count()!=0) {
            delete (SfxUndoAction*) pUndoStack->Remove(pUndoStack->Count()-1);
        }
        delete pUndoStack;
        pUndoStack=NULL;
    }
    if (pRedoStack!=NULL) {
        while (pRedoStack->Count()!=0) {
            delete (SfxUndoAction*) pRedoStack->Remove(pRedoStack->Count()-1);
        }
        delete pRedoStack;
        pRedoStack=NULL;
    }
}

FASTBOOL SdrModel::Undo()
{
    FASTBOOL bRet=FALSE;
    SfxUndoAction* pDo=(SfxUndoAction*)GetUndoAction(0);
    if (pDo!=NULL) {
        pDo->Undo();
        if (pRedoStack==NULL) pRedoStack=new Container(1024,16,16);
        pRedoStack->Insert(pUndoStack->Remove((ULONG)0),(ULONG)0);
    }
    return bRet;
}

FASTBOOL SdrModel::Redo()
{
    FASTBOOL bRet=FALSE;
    SfxUndoAction* pDo=(SfxUndoAction*)GetRedoAction(0);
    if (pDo!=NULL) {
        pDo->Redo();
        if (pUndoStack==NULL) pUndoStack=new Container(1024,16,16);
        pUndoStack->Insert(pRedoStack->Remove((ULONG)0),(ULONG)0);
    }
    return bRet;
}

FASTBOOL SdrModel::Repeat(SfxRepeatTarget& rView)
{
    FASTBOOL bRet=FALSE;
    SfxUndoAction* pDo=(SfxUndoAction*)GetUndoAction(0);
    if (pDo!=NULL) {
        if (pDo->CanRepeat(rView)) {
            pDo->Repeat(rView);
            bRet=TRUE;
        }
    }
    return bRet;
}

void SdrModel::ImpPostUndoAction(SdrUndoAction* pUndo)
{
    if (aUndoLink.IsSet()) {
        aUndoLink.Call(pUndo);
    } else {
        if (pUndoStack==NULL) pUndoStack=new Container(1024,16,16);
        pUndoStack->Insert(pUndo,(ULONG)0);
        while (pUndoStack->Count()>nMaxUndoCount) {
            delete (SfxUndoAction*)  pUndoStack->Remove(pUndoStack->Count()-1);
        }
        if (pRedoStack!=NULL) pRedoStack->Clear();
    }
}

void SdrModel::BegUndo()
{
    if (pAktUndoGroup==NULL) {
        pAktUndoGroup=new SdrUndoGroup(*this);
        nUndoLevel=1;
    } else {
        nUndoLevel++;
    }
}

void SdrModel::BegUndo(const XubString& rComment)
{
    BegUndo();
    if (nUndoLevel==1) {
        pAktUndoGroup->SetComment(rComment);
    }
}

void SdrModel::BegUndo(const XubString& rComment, const XubString& rObjDescr, SdrRepeatFunc eFunc)
{
    BegUndo();
    if (nUndoLevel==1) {
        pAktUndoGroup->SetComment(rComment);
        pAktUndoGroup->SetObjDescription(rObjDescr);
        pAktUndoGroup->SetRepeatFunction(eFunc);
    }
}

void SdrModel::BegUndo(SdrUndoGroup* pUndoGrp)
{
    if (pAktUndoGroup==NULL) {
        pAktUndoGroup=pUndoGrp;
        nUndoLevel=1;
    } else {
        delete pUndoGrp;
        nUndoLevel++;
    }
}

void SdrModel::EndUndo()
{
    DBG_ASSERT(nUndoLevel!=0,"SdrModel::EndUndo(): UndoLevel ist bereits auf 0!");
    if (pAktUndoGroup!=NULL) {
        nUndoLevel--;
        if (nUndoLevel==0) {
            if (pAktUndoGroup->GetActionCount()!=0) {
                SdrUndoAction* pUndo=pAktUndoGroup;
                pAktUndoGroup=NULL;
                ImpPostUndoAction(pUndo);
            } else { // war nix drin
                delete pAktUndoGroup;
                pAktUndoGroup=NULL;
            }
        }
    }
}

void SdrModel::SetUndoComment(const XubString& rComment)
{
    DBG_ASSERT(nUndoLevel!=0,"SdrModel::SetUndoComment(): UndoLevel ist auf 0!");
    if (nUndoLevel==1) {
        pAktUndoGroup->SetComment(rComment);
    }
}

void SdrModel::SetUndoComment(const XubString& rComment, const XubString& rObjDescr)
{
    DBG_ASSERT(nUndoLevel!=0,"SdrModel::SetUndoComment(): UndoLevel ist auf 0!");
    if (nUndoLevel==1) {
        pAktUndoGroup->SetComment(rComment);
        pAktUndoGroup->SetObjDescription(rObjDescr);
    }
}

void SdrModel::AddUndo(SdrUndoAction* pUndo)
{
    if (pAktUndoGroup!=NULL) {
        pAktUndoGroup->AddAction(pUndo);
    } else {
        ImpPostUndoAction(pUndo);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrModel::ImpCreateTables()
{
#ifndef SVX_LIGHT
    // der Writer hat seinen eigenen ColorTable
    if (!bExtColorTable) pColorTable=new XColorTable(aTablePath,(XOutdevItemPool*)pItemPool);
    pDashList    =new XDashList    (aTablePath,(XOutdevItemPool*)pItemPool);
    pLineEndList =new XLineEndList (aTablePath,(XOutdevItemPool*)pItemPool);
    pHatchList   =new XHatchList   (aTablePath,(XOutdevItemPool*)pItemPool);
    pGradientList=new XGradientList(aTablePath,(XOutdevItemPool*)pItemPool);
    pBitmapList  =new XBitmapList  (aTablePath,(XOutdevItemPool*)pItemPool);
#endif
}

void SdrModel::Clear()
{
    sal_Int32 i;
    // delete all drawing pages
    sal_Int32 nAnz=GetPageCount();
    for (i=nAnz-1; i>=0; i--)
    {
        DeletePage( (USHORT)i );
    }
    aPages.Clear();

    // delete all Masterpages
    nAnz=GetMasterPageCount();
    for(i=nAnz-1; i>=0; i--)
    {
        DeleteMasterPage( (USHORT)i );
    }
    aMaPag.Clear();

    pLayerAdmin->ClearLayer();
    pLayerAdmin->ClearLayerSets();
    //SetChanged();
}

SdrModel* SdrModel::AllocModel() const
{
    SdrModel* pModel=new SdrModel;
    pModel->SetScaleUnit(eObjUnit,aObjUnit);
    return pModel;
}

SdrPage* SdrModel::AllocPage(FASTBOOL bMasterPage)
{
    return new SdrPage(*this,bMasterPage);
}

const SdrModel* SdrModel::LoadModel(const String& rFileName)
{
    if(pLoadedModel && aLoadedModelFN.Equals(rFileName))
    {
        return pLoadedModel;
    }
    else
    {
        delete pLoadedModel;
        pLoadedModel = NULL;
        aLoadedModelFN = String();

        SdrModel*           pModel = new SdrModel;
        const INetURLObject aFileURL( rFileName );

        DBG_ASSERT( aFileURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );

        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aFileURL.GetMainURL(), STREAM_READ );

        if( pIStm )
        {
            pModel->GetItemPool().Load( *pIStm );
            (*pIStm) >> *pModel;

            if( pIStm->GetError() )
                delete pModel, pModel = NULL;
            else
            {
                pLoadedModel = pModel;
                aLoadedModelFN = rFileName;
            }

            delete pIStm;
        }
        else
            delete pModel, pModel = NULL;

        return pModel;
    }
}

void SdrModel::DisposeLoadedModels()
{
    delete pLoadedModel;
    pLoadedModel = NULL;
    aLoadedModelFN = String();
}

void SdrModel::SetTextDefaults() const
{
    SvxFontItem aSvxFontItem;
    SvxFontItem aSvxFontItemCJK( EE_CHAR_FONTINFO_CJK );
    SvxFontItem aSvxFontItemCTL( EE_CHAR_FONTINFO_CTL );
    GetDefaultFonts( aSvxFontItem, aSvxFontItemCJK, aSvxFontItemCTL );
    pItemPool->SetPoolDefaultItem( aSvxFontItem );
    pItemPool->SetPoolDefaultItem( aSvxFontItemCJK );
    pItemPool->SetPoolDefaultItem( aSvxFontItemCTL );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CJK ) );
    pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CTL ) );
    pItemPool->SetPoolDefaultItem( SvxColorItem(SdrEngineDefaults::GetFontColor()) );
}

SdrOutliner& SdrModel::GetDrawOutliner(const SdrTextObj* pObj) const
{
    pDrawOutliner->SetTextObj(pObj);
    return *pDrawOutliner;
}

const SdrTextObj* SdrModel::GetFormattingTextObj() const
{
    if (pDrawOutliner!=NULL) {
        return pDrawOutliner->GetTextObj();
    }
    return NULL;
}

void SdrModel::ImpSetOutlinerDefaults( SdrOutliner* pOutliner, BOOL bInit )
{
    /**************************************************************************
    * Initialisierung der Outliner fuer Textausgabe und HitTest
    **************************************************************************/
    if( bInit )
    {
        pOutliner->EraseVirtualDevice();
        pOutliner->SetUpdateMode(FALSE);
        pOutliner->SetEditTextObjectPool(pItemPool);
        pOutliner->SetDefTab(nDefaultTabulator);
    }

    pOutliner->SetRefDevice(GetRefDevice());
    if ( !GetRefDevice() )
    {
        MapMode aMapMode(eObjUnit, Point(0,0), aObjUnit, aObjUnit);
        pOutliner->SetRefMapMode(aMapMode);
    }
}

void SdrModel::SetRefDevice(OutputDevice* pDev)
{
    pRefOutDev=pDev;
    ImpSetOutlinerDefaults( pDrawOutliner );
    ImpSetOutlinerDefaults( pHitTestOutliner );
    RefDeviceChanged();
}

void SdrModel::ImpReformatAllTextObjects()
{
    USHORT nAnz=GetMasterPageCount();
    USHORT nNum;
    for (nNum=0; nNum<nAnz; nNum++) {
        GetMasterPage(nNum)->ReformatAllTextObjects();
    }
    nAnz=GetPageCount();
    for (nNum=0; nNum<nAnz; nNum++) {
        GetPage(nNum)->ReformatAllTextObjects();
    }
}

SvStream* SdrModel::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
{
    return NULL;
}

// Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.
void SdrModel::BurnInStyleSheetAttributes( BOOL bPseudoSheetsOnly )
{
    USHORT nAnz=GetMasterPageCount();
    USHORT nNum;
    for (nNum=0; nNum<nAnz; nNum++) {
        GetMasterPage(nNum)->BurnInStyleSheetAttributes( bPseudoSheetsOnly );
    }
    nAnz=GetPageCount();
    for (nNum=0; nNum<nAnz; nNum++) {
        GetPage(nNum)->BurnInStyleSheetAttributes( bPseudoSheetsOnly );
    }
}

void SdrModel::RemoveNotPersistentObjects(FASTBOOL bNoBroadcast)
{
    USHORT nAnz=GetMasterPageCount();
    USHORT nNum;
    for (nNum=0; nNum<nAnz; nNum++) {
        GetMasterPage(nNum)->RemoveNotPersistentObjects(bNoBroadcast);
    }
    nAnz=GetPageCount();
    for (nNum=0; nNum<nAnz; nNum++) {
        GetPage(nNum)->RemoveNotPersistentObjects(bNoBroadcast);
    }
}

void SdrModel::RefDeviceChanged()
{
    Broadcast(SdrHint(HINT_REFDEVICECHG));
    ImpReformatAllTextObjects();
}

void SdrModel::SetDefaultFontHeight(ULONG nVal)
{
    if (nVal!=nDefTextHgt) {
        nDefTextHgt=nVal;
        Broadcast(SdrHint(HINT_DEFFONTHGTCHG));
        ImpReformatAllTextObjects();
    }
}

void SdrModel::SetDefaultTabulator(USHORT nVal)
{
    if (nDefaultTabulator!=nVal) {
        nDefaultTabulator=nVal;
        Outliner& rOutliner=GetDrawOutliner();
        rOutliner.SetDefTab(nVal);
        Broadcast(SdrHint(HINT_DEFAULTTABCHG));
        ImpReformatAllTextObjects();
    }
}

void SdrModel::ImpSetUIUnit()
{
    if (aUIScale.GetNumerator()==0 || aUIScale.GetDenominator()==0) aUIScale=Fraction(1,1);
    FASTBOOL bMapInch=IsInch(eObjUnit);
    FASTBOOL bMapMetr=IsMetric(eObjUnit);
    FASTBOOL bUIInch=IsInch(eUIUnit);
    FASTBOOL bUIMetr=IsMetric(eUIUnit);
    nUIUnitKomma=0;
    long nMul=1;
    long nDiv=1;

    // Zunaechst normalisieren auf m bzw. "
    switch (eObjUnit) {
        case MAP_100TH_MM   : nUIUnitKomma+=5; break;
        case MAP_10TH_MM    : nUIUnitKomma+=4; break;
        case MAP_MM         : nUIUnitKomma+=3; break;
        case MAP_CM         : nUIUnitKomma+=2; break;
        case MAP_1000TH_INCH: nUIUnitKomma+=3; break;
        case MAP_100TH_INCH : nUIUnitKomma+=2; break;
        case MAP_10TH_INCH  : nUIUnitKomma+=1; break;
        case MAP_INCH       : nUIUnitKomma+=0; break;
        case MAP_POINT      : nDiv=72;     break;          // 1Pt   = 1/72"
        case MAP_TWIP       : nDiv=144; nUIUnitKomma++; break; // 1Twip = 1/1440"
        case MAP_PIXEL      : break;
        case MAP_SYSFONT    : break;
        case MAP_APPFONT    : break;
        case MAP_RELATIVE   : break;
    } // switch

    // 1 mile    =  8 furlong = 63.360" = 1.609.344,0mm
    // 1 furlong = 10 chains  =  7.920" =   201.168,0mm
    // 1 chain   =  4 poles   =    792" =    20.116,8mm
    // 1 pole    =  5 1/2 yd  =    198" =     5.029,2mm
    // 1 yd      =  3 ft      =     36" =       914,4mm
    // 1 ft      = 12 "       =      1" =       304,8mm
    switch (eUIUnit) {
        case FUNIT_NONE   : break;
        // Metrisch
        case FUNIT_100TH_MM: nUIUnitKomma-=5; break;
        case FUNIT_MM     : nUIUnitKomma-=3; break;
        case FUNIT_CM     : nUIUnitKomma-=2; break;
        case FUNIT_M      : nUIUnitKomma+=0; break;
        case FUNIT_KM     : nUIUnitKomma+=3; break;
        // Inch
        case FUNIT_TWIP   : nMul=144; nUIUnitKomma--;  break;  // 1Twip = 1/1440"
        case FUNIT_POINT  : nMul=72;     break;            // 1Pt   = 1/72"
        case FUNIT_PICA   : nMul=6;      break;            // 1Pica = 1/6"  ?
        case FUNIT_INCH   : break;                         // 1"    = 1"
        case FUNIT_FOOT   : nDiv*=12;    break;            // 1Ft   = 12"
        case FUNIT_MILE   : nDiv*=6336; nUIUnitKomma++; break; // 1mile = 63360"
        // sonstiges
        case FUNIT_CUSTOM : break;
        case FUNIT_PERCENT: nUIUnitKomma+=2; break;
    } // switch

    if (bMapInch && bUIMetr) {
        nUIUnitKomma+=4;
        nMul*=254;
    }
    if (bMapMetr && bUIInch) {
        nUIUnitKomma-=4;
        nDiv*=254;
    }

    // Temporaere Fraction zum Kuerzen
    Fraction aTempFract(nMul,nDiv);
    nMul=aTempFract.GetNumerator();
    nDiv=aTempFract.GetDenominator();
    // Nun mit dem eingestellten Masstab verknuepfen
    BigInt nBigMul(nMul);
    BigInt nBigDiv(nDiv);
    BigInt nBig1000(1000);
    nBigMul*=aUIScale.GetDenominator();
    nBigDiv*=aUIScale.GetNumerator();
    while (nBigMul>nBig1000) {
        nUIUnitKomma--;
        nBigMul/=10;
    }
    while (nBigDiv>nBig1000) {
        nUIUnitKomma++;
        nBigDiv/=10;
    }
    nMul=long(nBigMul);
    nDiv=long(nBigDiv);
    switch ((short)nMul) {
        case   10: nMul=1; nUIUnitKomma--; break;
        case  100: nMul=1; nUIUnitKomma-=2; break;
        case 1000: nMul=1; nUIUnitKomma-=3; break;
    } // switch
    switch ((short)nDiv) {
        case   10: nDiv=1; nUIUnitKomma++; break;
        case  100: nDiv=1; nUIUnitKomma+=2; break;
        case 1000: nDiv=1; nUIUnitKomma+=3; break;
    } // switch
    aUIUnitFact=Fraction(nMul,nDiv);
    bUIOnlyKomma=nMul==nDiv;
    TakeUnitStr(eUIUnit,aUIUnitStr);
}

void SdrModel::SetScaleUnit(MapUnit eMap, const Fraction& rFrac)
{
    if (eObjUnit!=eMap || aObjUnit!=rFrac) {
        eObjUnit=eMap;
        aObjUnit=rFrac;
        pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
        ImpReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetScaleUnit(MapUnit eMap)
{
    if (eObjUnit!=eMap) {
        eObjUnit=eMap;
        pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
        ImpReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetScaleFraction(const Fraction& rFrac)
{
    if (aObjUnit!=rFrac) {
        aObjUnit=rFrac;
        ImpSetUIUnit();
        ImpSetOutlinerDefaults( pDrawOutliner );
        ImpSetOutlinerDefaults( pHitTestOutliner );
        ImpReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit)
{
    if (eUIUnit!=eUnit) {
        eUIUnit=eUnit;
        ImpSetUIUnit();
        ImpReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetUIScale(const Fraction& rScale)
{
    if (aUIScale!=rScale) {
        aUIScale=rScale;
        ImpSetUIUnit();
        ImpReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::SetUIUnit(FieldUnit eUnit, const Fraction& rScale)
{
    if (eUIUnit!=eUnit || aUIScale!=rScale) {
        eUIUnit=eUnit;
        aUIScale=rScale;
        ImpSetUIUnit();
        ImpReformatAllTextObjects(); // #40424#
    }
}

void SdrModel::TakeUnitStr(FieldUnit eUnit, XubString& rStr) const
{
    switch(eUnit)
    {
        default:
        case FUNIT_NONE   :
        case FUNIT_CUSTOM :
        {
            rStr = String();
            break;
        }
        case FUNIT_100TH_MM:
        {
            sal_Char aText[] = "/100mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_MM     :
        {
            sal_Char aText[] = "mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_CM     :
        {
            sal_Char aText[] = "cm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_M      :
        {
            rStr = String();
            rStr += sal_Unicode('m');
            break;
        }
        case FUNIT_KM     :
        {
            sal_Char aText[] = "km";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_TWIP   :
        {
            sal_Char aText[] = "twip";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_POINT  :
        {
            sal_Char aText[] = "pt";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_PICA   :
        {
            sal_Char aText[] = "pica";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_INCH   :
        {
            rStr = String();
            rStr += sal_Unicode('"');
            break;
        }
        case FUNIT_FOOT   :
        {
            sal_Char aText[] = "ft";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_MILE   :
        {
            sal_Char aText[] = "mile(s)";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_PERCENT:
        {
            rStr = String();
            rStr += sal_Unicode('%');
            break;
        }
    }
}

void SdrModel::TakeMetricStr(long nVal, XubString& rStr, FASTBOOL bNoUnitChars) const
{
    if(!bUIOnlyKomma)
        nVal = (nVal * aUIUnitFact.GetNumerator()) / aUIUnitFact.GetDenominator();

    BOOL bNeg(nVal < 0);

    if(bNeg)
        nVal = -nVal;

    International aInter(Application::GetAppInternational());
    sal_Int32 nKomma(nUIUnitKomma);
    sal_Int32 nNumDigits(aInter.GetNumDigits());

    while(nKomma > nNumDigits)
    {
        // das bedeutet teilen und runden
        sal_Int32 nDiff(nKomma - nNumDigits);

        switch(nDiff)
        {
            case 1: nVal = (nVal+5)/10; nKomma--; break;
            case 2: nVal = (nVal+50)/100; nKomma-=2; break;
            case 3: nVal = (nVal+500)/1000; nKomma-=3; break;
            case 4: nVal = (nVal+5000)/10000; nKomma-=4; break;
            case 5: nVal = (nVal+50000)/100000; nKomma-=5; break;
            case 6: nVal = (nVal+500000)/1000000; nKomma-=6; break;
            case 7: nVal = (nVal+5000000)/10000000; nKomma-=7; break;
            default:nVal = (nVal+50000000)/100000000; nKomma-=8; break;
        }
    }

    rStr = UniString::CreateFromInt32(nVal);

    if(nKomma < 0)
    {
        // Negatives Komma bedeutet: Nullen dran
        sal_Int32 nAnz(-nKomma);

        for(sal_Int32 i=0; i<nAnz; i++)
            rStr += sal_Unicode('0');

        nKomma = 0;
    }

    // #83257# the second condition needs to be <= since inside this loop
    // also the leading zero is inserted.
    if(nKomma > 0 && rStr.Len() <= nKomma)
    {
        // Fuer Komma evtl. vorne Nullen dran
        sal_Int32 nAnz(nKomma - rStr.Len());

        if(nAnz >= 0 && aInter.IsNumLeadingZero())
            nAnz++;

        for(sal_Int32 i=0; i<nAnz; i++)
            rStr.Insert(sal_Unicode('0'), 0);
    }

    // KommaChar einfuegen
    sal_Int32 nVorKomma(rStr.Len() - nKomma);

    if(nKomma > 0)
    {
        sal_Unicode cDec(aInter.GetNumDecimalSep());

        rStr.Insert(cDec, nVorKomma);
    }

    if(!aInter.IsNumTrailingZeros())
    {
        while(rStr.Len() && rStr.GetChar(rStr.Len() - 1) == sal_Unicode('0'))
            rStr.Erase(rStr.Len() - 1);

        sal_Unicode cDec(aInter.GetNumDecimalSep());

        if(rStr.Len() && rStr.GetChar(rStr.Len() - 1) == cDec)
            rStr.Erase(rStr.Len() - 1);
    }

    // ggf. Trennpunkte bei jedem Tausender einfuegen
    if(nVorKomma > 3 && aInter.IsNumThousandSep())
    {
        sal_Unicode cDot(aInter.GetNumThousandSep());
        sal_Int32 i(nVorKomma - 3);

        while(i > 0) // #78311#
        {
            rStr.Insert(cDot, (xub_StrLen)i);
            i -= 3;
        }
    }

    if(!rStr.Len())
    {
        rStr = String();
        rStr += sal_Unicode('0');
    }

    if(bNeg)
    {
        rStr.Insert(sal_Unicode('-'), 0);
    }

    if(!bNoUnitChars)
        rStr += aUIUnitStr;
}

void SdrModel::TakeWinkStr(long nWink, XubString& rStr, FASTBOOL bNoDegChar) const
{
    BOOL bNeg(nWink < 0);

    if(bNeg)
        nWink = -nWink;

    rStr = UniString::CreateFromInt32(nWink);

    International aInter(Application::GetAppInternational());
    xub_StrLen nAnz(2);

    if(aInter.IsNumLeadingZero())
        nAnz++;

    while(rStr.Len() < nAnz)
        rStr.Insert(sal_Unicode('0'), 0);

    sal_Unicode cDec(aInter.GetNumDecimalSep());

    rStr.Insert(cDec, rStr.Len() - 2);

    if(bNeg)
        rStr.Insert(sal_Unicode('-'), 0);

    if(!bNoDegChar)
        rStr += DEGREE_CHAR;
}

void SdrModel::TakePercentStr(const Fraction& rVal, XubString& rStr, FASTBOOL bNoPercentChar) const
{
    INT32 nMul(rVal.GetNumerator());
    INT32 nDiv(rVal.GetDenominator());
    BOOL bNeg(nMul < 0);

    if(nDiv < 0)
        bNeg = !bNeg;

    if(nMul < 0)
        nMul = -nMul;

    if(nDiv < 0)
        nDiv = -nDiv;

    nMul *= 100;
    nMul += nDiv/2;
    nMul /= nDiv;

    rStr = UniString::CreateFromInt32(nMul);

    if(bNeg)
        rStr.Insert(sal_Unicode('-'), 0);

    if(!bNoPercentChar)
        rStr += sal_Unicode('%');
}

void SdrModel::SetChanged(FASTBOOL bFlg)
{
    bChanged=bFlg;
}

void SdrModel::RecalcPageNums(FASTBOOL bMaster)
{
    Container& rPL=*(bMaster ? &aMaPag : &aPages);
    USHORT nAnz=USHORT(rPL.Count());
    USHORT i;
    for (i=0; i<nAnz; i++) {
        SdrPage* pPg=(SdrPage*)(rPL.GetObject(i));
        pPg->SetPageNum(i);
    }
    if (bMaster) bMPgNumsDirty=FALSE;
    else bPagNumsDirty=FALSE;
}

void SdrModel::InsertPage(SdrPage* pPage, USHORT nPos)
{
    USHORT nAnz=GetPageCount();
    if (nPos>nAnz) nPos=nAnz;
    aPages.Insert(pPage,nPos);
    pPage->SetInserted(TRUE);
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nAnz) bPagNumsDirty=TRUE;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPage);
    Broadcast(aHint);
}

void SdrModel::DeletePage(USHORT nPgNum)
{
    SdrPage* pPg=RemovePage(nPgNum);
    delete pPg;
}

SdrPage* SdrModel::RemovePage(USHORT nPgNum)
{
    SdrPage* pPg=(SdrPage*)aPages.Remove(nPgNum);
    if (pPg!=NULL) {
        pPg->SetInserted(FALSE);
    }
    bPagNumsDirty=TRUE;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPg);
    Broadcast(aHint);
    return pPg;
}

void SdrModel::MovePage(USHORT nPgNum, USHORT nNewPos)
{
    SdrPage* pPg=(SdrPage*)aPages.Remove(nPgNum);
    if (pPg!=NULL) {
        pPg->SetInserted(FALSE);
        InsertPage(pPg,nNewPos);
    }
}

void SdrModel::InsertMasterPage(SdrPage* pPage, USHORT nPos)
{
    USHORT nAnz=GetMasterPageCount();
    if (nPos>nAnz) nPos=nAnz;
    aMaPag.Insert(pPage,nPos);
    pPage->SetInserted(TRUE);
    pPage->SetPageNum(nPos);
    pPage->SetModel(this);
    if (nPos<nAnz) {
        bMPgNumsDirty=TRUE;
        // Anpassen der MasterPageDescriptoren
        USHORT nPageAnz=GetPageCount();
        for (USHORT np=0; np<nPageAnz; np++) {
            GetPage(np)->ImpMasterPageInserted(nPos);
        }
    }
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPage);
    Broadcast(aHint);
}

void SdrModel::DeleteMasterPage(USHORT nPgNum)
{
    SdrPage* pPg=RemoveMasterPage(nPgNum);
    if (pPg!=NULL) delete pPg;
}

SdrPage* SdrModel::RemoveMasterPage(USHORT nPgNum)
{
    SdrPage* pRetPg=(SdrPage*)aMaPag.Remove(nPgNum);
    // Nun die Verweise der normalen Zeichenseiten auf die entfernte MasterPage loeschen
    // und Verweise auf dahinterliegende Masterpages anpassen.
    USHORT nPageAnz=GetPageCount();
    for (USHORT np=0; np<nPageAnz; np++) {
        GetPage(np)->ImpMasterPageRemoved(nPgNum);
    }
    if (pRetPg!=NULL) {
        pRetPg->SetInserted(FALSE);
    }
    bMPgNumsDirty=TRUE;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pRetPg);
    Broadcast(aHint);
    return pRetPg;
}

void SdrModel::MoveMasterPage(USHORT nPgNum, USHORT nNewPos)
{
    SdrPage* pPg=(SdrPage*)aMaPag.Remove(nPgNum);
    if (pPg!=NULL) {
        pPg->SetInserted(FALSE);
        aMaPag.Insert(pPg,nNewPos);
        // Anpassen der MasterPageDescriptoren
        USHORT nPageAnz=GetPageCount();
        for (USHORT np=0; np<nPageAnz; np++) {
            GetPage(np)->ImpMasterPageMoved(nPgNum,nNewPos);
        }
    }
    bMPgNumsDirty=TRUE;
    SetChanged();
    SdrHint aHint(HINT_PAGEORDERCHG);
    aHint.SetPage(pPg);
    Broadcast(aHint);
}

void SdrModel::WriteData(SvStream& rOut) const
{
    const ULONG nOldCompressMode = nStreamCompressMode;
    ULONG       nNewCompressMode = nStreamCompressMode;

    if( SOFFICE_FILEFORMAT_40 <= rOut.GetVersion() )
    {
        if( IsSaveCompressed() )
            nNewCompressMode |= COMPRESSMODE_ZBITMAP;

        if( IsSaveNative() )
            nNewCompressMode |= COMPRESSMODE_NATIVE;
    }

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
    aCompat.SetID("SdrModel");
#endif

    // damit ich meine eigenen SubRecords erkenne (ab V11)
    rOut.Write(SdrIOJoeMagic, 4);

    {
        // Focus fuer aModelMiscCompat
        // ab V11 eingepackt
        SdrDownCompat aModelMiscCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
        aModelMiscCompat.SetID("SdrModel(Miscellaneous)");
#endif

        // ModelInfo muss hier ganz am Anfang stehen!
        ((SdrModel*)this)->aInfo.aLastWriteDate = DateTime();
        rtl_TextEncoding eOutCharSet = rOut.GetStreamCharSet();
        if(eOutCharSet == ((rtl_TextEncoding)9) /* == RTL_TEXTENCODING_SYSTEM */ )
            eOutCharSet = gsl_getSystemTextEncoding();
        ((SdrModel*)this)->aInfo.eLastWriteCharSet = GetStoreCharSet(eOutCharSet);

        // UNICODE: set the target charset on the stream to access it as parameter
        // in all streaming operations for UniString->ByteString conversions
        rOut.SetStreamCharSet(aInfo.eLastWriteCharSet);

        /* old SV-stuff, there is no possibility to determine this informations in another way
        ((SdrModel*)this)->aInfo.eLastWriteGUI=System::GetGUIType();
        ((SdrModel*)this)->aInfo.eLastWriteCPU=System::GetCPUType();
        ((SdrModel*)this)->aInfo.eLastWriteSys=System::GetSystemType();
        */

        if(aReadDate.IsValid())
        {
            ((SdrModel*)this)->aInfo.aLastReadDate = aReadDate;
            ((SdrModel*)this)->aInfo.eLastReadCharSet = GetStoreCharSet(gsl_getSystemTextEncoding());

            /* old SV-stuff, there is no possibility to determine this informations in another way
            ((SdrModel*)this)->aInfo.eLastReadGUI=System::GetGUIType();
            ((SdrModel*)this)->aInfo.eLastReadCPU=System::GetCPUType();
            ((SdrModel*)this)->aInfo.eLastReadSys=System::GetSystemType();
            */
        }
        rOut << aInfo; // DateiInfo rausschreiben (ab V11)

        { // ein Bereich fuer Statistik reservieren (V11) (kommt spaeter vielleicht mal dazu)
            SdrDownCompat aModelStatisticCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
            aModelStatisticCompat.SetID("SdrModel(Statistic)");
#endif
        }

        {
            // ab V11
            SdrDownCompat aModelFormatCompat(rOut, STREAM_WRITE);

#ifdef DBG_UTIL
            aModelFormatCompat.SetID("SdrModel(Format)");
#endif

            // ab V11
            rOut << nNewCompressMode;

            // ab V11
            rOut << UINT16(rOut.GetNumberFormatInt());

            rOut.SetCompressMode(nNewCompressMode);
            // CompressMode erst an dieser Stelle setzen, damit konform zu ReadData()
        }

        rOut << INT32(aObjUnit.GetNumerator());
        rOut << INT32(aObjUnit.GetDenominator());
        rOut << UINT16(eObjUnit);
        // Komprimiert ?
        rOut << UINT16(0);
        // Nur eine DummyPage, jedoch mit relevanten Objekten?
        rOut << UINT8(bPageNotValid);
        // Reserve DummyByte
        rOut << UINT8(0);

        // Tabellen-, Listen- und Palettennamen schreiben
        // rOut<<INT16(::GetSystemCharSet()); seit V11 hier kein CharSet mehr
        XubString aEmptyStr;

        if(bExtColorTable)
        {
            // der Writer hat seinen eigenen ColorTable
            // UNICODE: rOut << aEmptyStr;
            rOut.WriteByteString(aEmptyStr);
        }
        else
        {
            if(pColorTable && !pColorTable->GetName().Equals(pszStandard))
            {
                // UNICODE: rOut << pColorTable->GetName();
                rOut.WriteByteString(pColorTable->GetName());
            }
            else
            {
                // UNICODE: rOut << aEmptyStr;
                rOut.WriteByteString(aEmptyStr);
            }
        }

        if(pDashList && !pDashList->GetName().Equals(pszStandard))
        {
            // UNICODE: rOut<<pDashList->GetName();
            rOut.WriteByteString(pDashList->GetName());
        }
        else
        {
            // UNICODE: rOut << aEmptyStr;
            rOut.WriteByteString(aEmptyStr);
        }

        if(pLineEndList && !pLineEndList->GetName().Equals(pszStandard))
        {
            // UNICODE: rOut<<pLineEndList->GetName();
            rOut.WriteByteString(pLineEndList->GetName());
        }
        else
        {
            // UNICODE: rOut << aEmptyStr;
            rOut.WriteByteString(aEmptyStr);
        }

        if(pHatchList && !pHatchList->GetName().Equals(pszStandard))
        {
            // UNICODE: rOut<<pHatchList->GetName();
            rOut.WriteByteString(pHatchList->GetName());
        }
        else
        {
            // UNICODE: rOut << aEmptyStr;
            rOut.WriteByteString(aEmptyStr);
        }

        if(pGradientList && !pGradientList->GetName().Equals(pszStandard))
        {
            // UNICODE: rOut<<pGradientList->GetName();
            rOut.WriteByteString(pGradientList->GetName());
        }
        else
        {
            // UNICODE: rOut << aEmptyStr;
            rOut.WriteByteString(aEmptyStr);
        }

        if(pBitmapList && !pBitmapList->GetName().Equals(pszStandard))
        {
            // UNICODE: rOut<<pBitmapList->GetName();
            rOut.WriteByteString(pBitmapList->GetName());
        }
        else
        {
            // UNICODE: rOut << aEmptyStr;
            rOut.WriteByteString(aEmptyStr);
        }

        // ab 09-02-1996
        rOut << INT32(aUIScale.GetNumerator());
        rOut << INT32(aUIScale.GetDenominator());
        rOut << UINT16(eUIUnit);

        // ab 09-04-1997 fuer #37710#
        rOut << INT32(nDefTextHgt);
        rOut << INT32(nDefaultTabulator);

        // StarDraw-Preview: Nummer der MasterPage der ersten Standard-Seite
        if(GetPageCount() >= 3 && GetPage(1)->GetMasterPageCount())
        {
            ((SdrModel*)this)->nStarDrawPreviewMasterPageNum =
                GetPage(1)->GetMasterPageNum(0);
        }
        rOut << nStarDrawPreviewMasterPageNum;
    }

    UINT16 i;

    for(i=0; i < GetLayerAdmin().GetLayerCount(); i++)
    {
        rOut << *GetLayerAdmin().GetLayer(i);
    }

    for(i=0; i < GetLayerAdmin().GetLayerSetCount(); i++)
    {
        rOut << *GetLayerAdmin().GetLayerSet(i);
    }

    for(i=0; i < GetMasterPageCount(); i++)
    {
        const SdrPage* pPg = GetMasterPage(i);
        rOut << *pPg;
    }

    for(i=0; i < GetPageCount(); i++)
    {
        const SdrPage* pPg = GetPage(i);
        rOut << *pPg;
    }

    // Endemarke
    SdrIOHeader(rOut, STREAM_WRITE, SdrIOEndeID);
}

void SdrModel::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
{
    if(rIn.GetError())
        return;

    // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
    SdrDownCompat aCompat(rIn, STREAM_READ);

#ifdef DBG_UTIL
    aCompat.SetID("SdrModel");
#endif

    if(rHead.GetVersion() >= 11)
    {
        // damit ich meine eigenen SubRecords erkenne (ab V11)
        char cMagic[4];
        if(rIn.Read(cMagic, 4) != 4 || memcmp(cMagic, SdrIOJoeMagic, 4))
        {
            rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
            return;
        }
    }

    DoProgress(rIn.Tell());

    {
        // Focus fuer aModelMiscCompat
        SdrDownCompat* pModelMiscCompat = NULL;

        if(rHead.GetVersion() >= 11)
        {
            // MiscellaneousData ab V11 eingepackt
            // MiscellaneousData ist alles von Recordbeginn bis
            // zum Anfang der Pages, Layer, ...
            pModelMiscCompat = new SdrDownCompat(rIn, STREAM_READ);

#ifdef DBG_UTIL
            pModelMiscCompat->SetID("SdrModel(Miscellaneous)");
#endif
        }

        if(rHead.GetVersion() >= 11)
        {
            // ModelInfo ab V11
            // DateiInfo lesen
            rIn >> aInfo;

            // StreamCharSet setzen, damit Strings beim
            // Lesen automatisch konvertiert werden
            rIn.SetStreamCharSet(aInfo.eLastWriteCharSet);
        }

        if(rHead.GetVersion() >= 11)
        {
            // reserviert fuer Statistik
            SdrDownCompat aModelStatisticCompat(rIn, STREAM_READ);

#ifdef DBG_UTIL
            aModelStatisticCompat.SetID("SdrModel(Statistik)");
#endif
        }

        if(rHead.GetVersion() >= 11)
        {
            // Info ueber Dateiformat
            SdrDownCompat aModelFormatCompat(rIn,STREAM_READ);

#ifdef DBG_UTIL
            aModelFormatCompat.SetID("SdrModel(Format)");
#endif

            if(aModelFormatCompat.GetBytesLeft() >= 4)
            {
                rIn >> nStreamCompressMode;
                rIn >> nStreamNumberFormat;
                rIn.SetCompressMode(nStreamCompressMode);
            }
        }

        INT32 nNum,nDen;
        UINT16 nTmp;
        UINT8  nTmp8;

        rIn >> nNum;
        rIn >> nDen;

        aObjUnit = Fraction(nNum,nDen);

        rIn >> nTmp;

        eObjUnit = MapUnit(nTmp);

        // Komprimiert ?
        rIn >> nTmp;

        //rIn.nJoeDummy=(nTmp==1);
        rIn >> nTmp8;

        bPageNotValid = (nTmp == 1);

        rIn >> nTmp8; // Reserve DummyByte

        BOOL bExtFiles(rHead.GetVersion() >= 1);

        if(bExtFiles)
        {
            // Tabellen-, Listen- und Palettennamen lesen
            XubString aName;

            if(rHead.GetVersion() < 11)
            {
                // vor V11 gab's noch keine ModelInfo, deshalb CharSet von hier
                // und rein zufaellig gab's genau bis inkl. zur V10
                // an dieser Stelle einen CharSet
                INT16 nCharSet;

                rIn >> nCharSet;

                // StreamCharSet setzen, damit Strings beim
                // Lesen automatisch konvertiert werden
                rIn.SetStreamCharSet(rtl_TextEncoding(nCharSet));
            }

            // Tabellen- und Listennamen lesen (Tabellen/Listen existieren schon) // SOH!!!
            // UNICODE: rIn >> aName;
            rIn.ReadByteString(aName);

            if(!bExtColorTable)
            {
                // der Writer hat seinen eigenen ColorTable
                if(!aName.Len())
                    aName = pszStandard;

                if(pColorTable)
                    pColorTable->SetName(aName);
            }

            rIn.ReadByteString(aName);
            if(!aName.Len())
                aName = pszStandard;
            if(pDashList)
                pDashList->SetName(aName);

            rIn.ReadByteString(aName);
            if(!aName.Len())
                aName = pszStandard;
            if(pLineEndList)
                pLineEndList->SetName(aName);

            rIn.ReadByteString(aName);
            if(!aName.Len())
                aName = pszStandard;
            if(pHatchList)
                pHatchList->SetName(aName);

            rIn.ReadByteString(aName);
            if(!aName.Len())
                aName = pszStandard;
            if(pGradientList)
                pGradientList->SetName(aName);

            rIn.ReadByteString(aName);
            if(!aName.Len())
                aName = pszStandard;
            if(pBitmapList)
                pBitmapList->SetName(aName);

            // Wenn gewuenscht kann hier SetDirty() an den Tabellen gesetzt werden, ist m.M. nach aber ueberfluessig ! SOH.
        }
        else
        {
            // Ansonsten altes Format: Listen und Tables sind embedded

#ifdef DBG_UTIL
            ByteString aMsg("Das Format dieser Datei ist noch von April '95 (Version ");
            aMsg += rHead.GetVersion();
            aMsg += "). Mit dieser Programmversion kann das nicht mehr gelesen werden";

            DBG_ERROR(aMsg.GetBuffer());
#endif

            // Version zu alt
            rIn.SetError(SVSTREAM_WRONGVERSION);

            return;
        }

        // UIUnit wird ab V12 gestreamt
        if(rHead.GetVersion() >= 12 && pModelMiscCompat->GetBytesLeft() > 0)
        {
            rIn >> nNum;
            rIn >> nDen;

            aUIScale = Fraction(nNum, nDen);

            rIn >> nTmp;

            eUIUnit = FieldUnit(nTmp);
        }

        // ab 09-04-1997 fuer #37710#: Text in Dafaultgroesse vom Writer ins Draw und umgekehrt
        if(rHead.GetVersion() >= 13 && pModelMiscCompat->GetBytesLeft() > 0)
        {
            rIn >> nNum;
            nDefTextHgt = nNum;

            rIn >> nNum;
            nDefaultTabulator = (UINT16)nNum;

            Outliner& rOutliner = GetDrawOutliner();
            rOutliner.SetDefTab(nDefaultTabulator);
        }

        if(rHead.GetVersion() >= 14 && pModelMiscCompat->GetBytesLeft() > 0)
        {
            // StarDraw-Preview: Nummer der MasterPage der ersten Standard-Seite
            rIn >> nStarDrawPreviewMasterPageNum;
        }

        if(pModelMiscCompat)
        {
            delete pModelMiscCompat;
        }
    }

    DoProgress(rIn.Tell());
    //SdrIOHeader aHead;

    // Seiten, Layer und LayerSets einlesen
    BOOL bEnde(FALSE);
    UINT16 nMasterPageNum(0);
    BOOL bAllPagesLoaded(TRUE);

    while(!rIn.GetError() && !rIn.IsEof() && !bEnde)
    {
        SdrIOHeaderLookAhead aHead(rIn);
        //ULONG nPos0=rIn.Tell();
        //rIn>>aHead;

        if(!aHead.IsMagic())
        {
            // Format-Fehler
            rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
            return;
        }
        else
        {
            if(!aHead.IsEnde())
            {
                //rIn.Seek(nPos0); // Die Headers wollen alle selbst lesen
                if(aHead.IsID(SdrIOPageID))
                {
                    if(!bStarDrawPreviewMode || GetPageCount() < 3)
                    {
                        // Page lesen
                        SdrPage* pPg = AllocPage(FALSE);

                        rIn >> *pPg;
                        InsertPage(pPg);
                    }
                    else
                    {
                        bAllPagesLoaded = FALSE;
                        aHead.SkipRecord();
                    }
                }
                else if(aHead.IsID(SdrIOMaPgID))
                {
                    if(!bStarDrawPreviewMode
                        || nStarDrawPreviewMasterPageNum == SDRPAGE_NOTFOUND
                        || nMasterPageNum == 0
                        || nMasterPageNum <= nStarDrawPreviewMasterPageNum
                        || nMasterPageNum <= nStarDrawPreviewMasterPageNum + 1)
                    {
                        // Im StarDrawPreviewMode Standard und Notizseite lesen!
                        // MasterPage lesen
                        SdrPage* pPg = AllocPage(TRUE);

                        rIn >> *pPg;
                        InsertMasterPage(pPg);
                    }
                    else
                    {
                        bAllPagesLoaded = FALSE;
                        aHead.SkipRecord();
                    }

                    nMasterPageNum++;
                }
                else if(aHead.IsID(SdrIOLayrID))
                {
                    //SdrLayer* pLay=GetLayer().NewLayer("");
                    // Layerdefinition lesen
                    SdrLayer* pLay = new SdrLayer;

                    rIn >> *pLay;
                    GetLayerAdmin().InsertLayer(pLay);
                }
                else if(aHead.IsID(SdrIOLSetID))
                {
                    //SdrLayerSet* pSet=GetLayer().NewLayerSet("");
                    SdrLayerSet* pSet = new SdrLayerSet; // Layersetdefinition lesen

                    rIn >> *pSet;
                    GetLayerAdmin().InsertLayerSet(pSet);
                }
                else
                {
                    // aha, das wil keiner. Also ueberlesen.
                    aHead.SkipRecord();
                    //rIn.Seek(nPos0+aHead.nBlkSize);
                }
            }
            else
            {
                bEnde = TRUE;

                // Endemarke weglesen
                aHead.SkipRecord();
            }
        }
        DoProgress(rIn.Tell());
    }

    if(bStarDrawPreviewMode && bAllPagesLoaded)
    {
        // Obwohl StarDrawPreviewMode wurden doch alle Seiten geladen,
        // um dieses kenntlich zu machen, wird das Flag zurueckgesetzt
        bStarDrawPreviewMode = FALSE;
    }
}

void SdrModel::AfterRead()
{
    // alle MasterPages und alle Pages durchlaufen
    UINT16 nAnz(GetMasterPageCount());
    UINT16 i;

    for(i=0; i < nAnz; i++)
    {
        GetMasterPage(i)->AfterRead();
    }

    nAnz = GetPageCount();

    for(i=0; i < nAnz; i++)
    {
        GetPage(i)->AfterRead();
    }
}

ULONG SdrModel::ImpCountAllSteamComponents() const
{
    UINT32 nCnt(0);
    UINT16 nAnz(GetMasterPageCount());
    UINT16 nNum;

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        nCnt += GetMasterPage(nNum)->CountAllObjects();
    }

    nAnz = GetPageCount();

    for(nNum = 0; nNum < nAnz; nNum++)
    {
        nCnt += GetPage(nNum)->CountAllObjects();
    }

    return nCnt;
}

SvStream& operator<<(SvStream& rOut, const SdrModel& rMod)
{
    ((SdrModel*)&rMod)->nProgressOfs=0;
    ((SdrModel*)&rMod)->nProgressMax=rMod.ImpCountAllSteamComponents(); // Hier passenden Wert einsetzen
    ((SdrModel*)&rMod)->DoProgress(0);
    ULONG nPos0=rOut.Tell();
    SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOModlID);
    USHORT nCompressMerk=rOut.GetCompressMode(); // Der CompressMode wird von SdrModel::ReadData() gesetzt
    rMod.WriteData(rOut);
    rOut.SetCompressMode(nCompressMerk); // CompressMode wieder restaurieren
    ((SdrModel*)&rMod)->DoProgress(0xFFFFFFFF);
    ((SdrModel*)&rMod)->Broadcast(SdrHint(HINT_MODELSAVED)); // #43095#
    return rOut;
}

SvStream& operator>>(SvStream& rIn, SdrModel& rMod)
{
    if (rIn.GetError()!=0) return rIn;
    rMod.aReadDate=DateTime(); // Zeitpunkt des Lesens merken
    rMod.nProgressOfs=rIn.Tell();
    rMod.nProgressMax=0xFFFFFFFF; // Vorlaeufiger Wert
    rMod.DoProgress(0);
    rMod.Clear();
    SdrIOHeader aHead(rIn,STREAM_READ);
    rMod.nLoadVersion=aHead.GetVersion();
    if (!aHead.IsMagic()) {
        rIn.SetError(SVSTREAM_FILEFORMAT_ERROR); // Format-Fehler
        return rIn;
    }
    if (aHead.GetMajorVersion()>nAktSdrFileMajorVersion) {
        rIn.SetError(SVSTREAM_WRONGVERSION); // Datei zu neu / Programm zu alt
        return rIn;
    }
    rMod.nProgressMax=aHead.GetBlockSize();
    rMod.DoProgress(rIn.Tell());
    rMod.bLoading=TRUE;
    rtl_TextEncoding eStreamCharSetMerker=rIn.GetStreamCharSet(); // Der StreamCharSet wird von SdrModel::ReadData() gesetzt
    USHORT nCompressMerk=rIn.GetCompressMode(); // Der CompressMode wird von SdrModel::ReadData() gesetzt
    rMod.ReadData(aHead,rIn);
    rIn.SetCompressMode(nCompressMerk); // CompressMode wieder restaurieren
    rIn.SetStreamCharSet(eStreamCharSetMerker); // StreamCharSet wieder restaurieren
    rMod.bLoading=FALSE;
    rMod.DoProgress(rIn.Tell());
    rMod.AfterRead();
    rMod.DisposeLoadedModels();

    rMod.ImpSetUIUnit(); // weil ggf. neues Scaling eingelesen
    rMod.DoProgress(0xFFFFFFFF);
    return rIn;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrModel::WriteModelInfo(SvStream& rOut) const
{
    FASTBOOL bRet=FALSE;
    if (rOut.GetError()!=0) return bRet;
    SdrIOHeaderLookAhead aLookAhead(rOut);
    ULONG nCompat;
    rOut>>nCompat;
    char cMagic[4];
    FASTBOOL bJoeMagicOk=(rOut.Read(cMagic,4)==4) && memcmp(cMagic,SdrIOJoeMagic,4)==0;
    if (aLookAhead.GetVersion()>=11 && aLookAhead.IsMagic() &&
        bJoeMagicOk && aLookAhead.IsID(SdrIOModlID) && rOut.GetError()==0)
    {
        rOut<<aInfo;
        bRet=rOut.GetError()==0;
    }
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrModel::CheckConsistence() const
{
    FASTBOOL bRet=TRUE;
#ifdef DBG_UTIL
    DBG_CHKTHIS(SdrModel,NULL);
#endif
    return bRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// #48289#
void SdrModel::CopyPages(USHORT nFirstPageNum, USHORT nLastPageNum,
                         USHORT nDestPos,
                         FASTBOOL bUndo, FASTBOOL bMoveNoCopy)
{
    if (bUndo) {
        BegUndo(ImpGetResStr(STR_UndoMergeModel));
    }
    USHORT nPageAnz=GetPageCount();
    USHORT nMaxPage=nPageAnz; if (nMaxPage!=0) nMaxPage--;
    if (nFirstPageNum>nMaxPage) nFirstPageNum=nMaxPage;
    if (nLastPageNum>nMaxPage)  nLastPageNum =nMaxPage;
    FASTBOOL bReverse=nLastPageNum<nFirstPageNum;
    if (nDestPos>nPageAnz) nDestPos=nPageAnz;

    // Zunaechst die Zeiger der betroffenen Seiten in einem Array sichern
    USHORT nPageNum=nFirstPageNum;
    USHORT nCopyAnz=((!bReverse)?(nLastPageNum-nFirstPageNum):(nFirstPageNum-nLastPageNum))+1;
    SdrPage** pPagePtrs=new SdrPage*[nCopyAnz];
    USHORT nCopyNum;
    for (nCopyNum=0; nCopyNum<nCopyAnz; nCopyNum++) {
        pPagePtrs[nCopyNum]=GetPage(nPageNum);
        if (bReverse) nPageNum--;
        else nPageNum++;
    }

    // Jetzt die Seiten kopieren
    USHORT nDestNum=nDestPos;
    for (nCopyNum=0; nCopyNum<nCopyAnz; nCopyNum++) {
        SdrPage* pPg=pPagePtrs[nCopyNum];
        USHORT nPageNum=pPg->GetPageNum();
        if (!bMoveNoCopy) {
            const SdrPage* pPg1=GetPage(nPageNum);
            pPg=pPg1->Clone();
            InsertPage(pPg,nDestNum);
            if (bUndo) AddUndo(new SdrUndoCopyPage(*pPg));
            nDestNum++;
        } else {
            // Move ist nicht getestet!
            if (nDestNum>nPageNum) nDestNum--;
            if (bUndo) AddUndo(new SdrUndoSetPageNum(*GetPage(nPageNum),nPageNum,nDestNum));
            pPg=RemovePage(nPageNum);
            InsertPage(pPg,nDestNum);
            nDestNum++;
        }

        if (bReverse) nPageNum--;
        else nPageNum++;
    }

    delete[] pPagePtrs;
    if (bUndo) EndUndo();
}

void SdrModel::Merge(SdrModel& rSourceModel,
                     USHORT nFirstPageNum, USHORT nLastPageNum,
                     USHORT nDestPos,
                     FASTBOOL bMergeMasterPages, FASTBOOL bAllMasterPages,
                     FASTBOOL bUndo, FASTBOOL bTreadSourceAsConst)
{
    if (&rSourceModel==this) { // #48289#
        CopyPages(nFirstPageNum,nLastPageNum,nDestPos,bUndo,!bTreadSourceAsConst);
        return;
    }
    if (bUndo) {
        BegUndo(ImpGetResStr(STR_UndoMergeModel));
    }
    USHORT nSrcPageAnz=rSourceModel.GetPageCount();
    USHORT nSrcMasterPageAnz=rSourceModel.GetMasterPageCount();
    USHORT nDstMasterPageAnz=GetMasterPageCount();
    FASTBOOL bInsPages=(nFirstPageNum<nSrcPageAnz || nLastPageNum<nSrcPageAnz);
    USHORT nMaxSrcPage=nSrcPageAnz; if (nMaxSrcPage!=0) nMaxSrcPage--;
    if (nFirstPageNum>nMaxSrcPage) nFirstPageNum=nMaxSrcPage;
    if (nLastPageNum>nMaxSrcPage)  nLastPageNum =nMaxSrcPage;
    FASTBOOL bReverse=nLastPageNum<nFirstPageNum;

    USHORT*   pMasterMap=NULL;
    FASTBOOL* pMasterNeed=NULL;
    USHORT    nMasterNeed=0;
    if (bMergeMasterPages && nSrcMasterPageAnz!=0) {
        // Feststellen, welche MasterPages aus rSrcModel benoetigt werden
        pMasterMap=new USHORT[nSrcMasterPageAnz];
        pMasterNeed=new FASTBOOL[nSrcMasterPageAnz];
        memset(pMasterMap,0xFF,nSrcMasterPageAnz*sizeof(USHORT));
        if (bAllMasterPages) {
            memset(pMasterNeed,TRUE,nSrcMasterPageAnz*sizeof(FASTBOOL));
        } else {
            memset(pMasterNeed,FALSE,nSrcMasterPageAnz*sizeof(FASTBOOL));
            USHORT nAnf= bReverse ? nLastPageNum : nFirstPageNum;
            USHORT nEnd= bReverse ? nFirstPageNum : nLastPageNum;
            for (USHORT i=nAnf; i<=nEnd; i++) {
                const SdrPage* pPg=rSourceModel.GetPage(i);
                USHORT nMasterDescrAnz=pPg->GetMasterPageCount();
                for (USHORT j=0; j<nMasterDescrAnz; j++) {
                    const SdrMasterPageDescriptor& rMaster=pPg->GetMasterPageDescriptor(j);
                    USHORT nMPgNum=rMaster.GetPageNum();
                    if (nMPgNum<nSrcMasterPageAnz) {
                        pMasterNeed[nMPgNum]=TRUE;
                    }
                }
            }
        }
        // Nun das Mapping der MasterPages bestimmen
        USHORT nAktMaPagNum=nDstMasterPageAnz;
        for (USHORT i=0; i<nSrcMasterPageAnz; i++) {
            if (pMasterNeed[i]) {
                pMasterMap[i]=nAktMaPagNum;
                nAktMaPagNum++;
                nMasterNeed++;
            }
        }
    }

    // rueberholen der Zeichenseiten
    if (bInsPages) {
        USHORT nSourcePos=nFirstPageNum;
        USHORT nMergeCount=USHORT(Abs((long)((long)nFirstPageNum-nLastPageNum))+1);
        if (nDestPos>GetPageCount()) nDestPos=GetPageCount();
        while (nMergeCount>0) {
            SdrPage* pPg=NULL;
            if (bTreadSourceAsConst) {
                const SdrPage* pPg1=rSourceModel.GetPage(nSourcePos);
                pPg=pPg1->Clone();
            } else {
                pPg=rSourceModel.RemovePage(nSourcePos);
            }
            if (pPg!=NULL) {
                InsertPage(pPg,nDestPos);
                if (bUndo) AddUndo(new SdrUndoNewPage(*pPg));
                // und nun zu den MasterPageDescriptoren
                USHORT nMasterDescrAnz=pPg->GetMasterPageCount();
                for (USHORT nMaster=nMasterDescrAnz; nMaster>0;) {
                    nMaster--;
                    const SdrMasterPageDescriptor& rConstMaster=pPg->GetMasterPageDescriptor(nMaster);
                    USHORT nMaPgNum=rConstMaster.GetPageNum();
                    if (bMergeMasterPages) {
                        USHORT nNeuNum=0xFFFF;
                        if (pMasterMap!=NULL) nNeuNum=pMasterMap[nMaPgNum];
                        if (nNeuNum!=0xFFFF) {
                            if (bUndo) AddUndo(new SdrUndoPageChangeMasterPage(*pPg,nNeuNum));
                            SdrMasterPageDescriptor& rMaster=pPg->GetMasterPageDescriptor(nMaster);
                            rMaster.SetPageNum(nNeuNum);
                        }
                        DBG_ASSERT(nNeuNum!=0xFFFF,"SdrModel::Merge(): Irgendwas ist krumm beim Mappen der MasterPages");
                    } else {
                        if (nMaPgNum>=nDstMasterPageAnz) {
                            // Aha, die ist ausserbalb des urspruenglichen Bereichs der Masterpages des DstModel
                            pPg->RemoveMasterPage(nMaster);
                        }
                    }
                }
            } else {
                DBG_ERROR("SdrModel::Merge(): Zeichenseite im SourceModel nicht gefunden");
            }
            nDestPos++;
            if (bReverse) nSourcePos--;
            else if (bTreadSourceAsConst) nSourcePos++;
            nMergeCount--;
        }
    }

    // rueberholen der Masterpages
    if (pMasterMap!=NULL && pMasterNeed!=NULL && nMasterNeed!=0) {
        for (USHORT i=nSrcMasterPageAnz; i>0;) {
            i--;
            if (pMasterNeed[i]) {
                SdrPage* pPg=NULL;
                if (bTreadSourceAsConst) {
                    const SdrPage* pPg1=rSourceModel.GetMasterPage(i);
                    pPg=pPg1->Clone();
                } else {
                    pPg=rSourceModel.RemoveMasterPage(i);
                }
                if (pPg!=NULL) {
                    // und alle ans einstige Ende des DstModel reinschieben.
                    // nicht InsertMasterPage() verwenden da die Sache
                    // inkonsistent ist bis alle drin sind
                    aMaPag.Insert(pPg,nDstMasterPageAnz);
                    pPg->SetInserted(TRUE);
                    pPg->SetModel(this);
                    bMPgNumsDirty=TRUE;
                    if (bUndo) AddUndo(new SdrUndoNewPage(*pPg));
                } else {
                    DBG_ERROR("SdrModel::Merge(): MasterPage im SourceModel nicht gefunden");
                }
            }
        }
    }

    delete [] pMasterMap;
    delete [] pMasterNeed;

    bMPgNumsDirty=TRUE;
    bPagNumsDirty=TRUE;

    SetChanged();
    // Fehlt: Mergen und Mapping der Layer
    // an den Objekten sowie an den MasterPageDescriptoren
    if (bUndo) EndUndo();
}

void SdrModel::SetStarDrawPreviewMode(BOOL bPreview)
{
    if (!bPreview && bStarDrawPreviewMode && GetPageCount())
    {
        // Das Zuruecksetzen ist nicht erlaubt, da das Model ev. nicht vollstaendig geladen wurde
        DBG_ASSERT(FALSE,"SdrModel::SetStarDrawPreviewMode(): Zuruecksetzen nicht erlaubt, da Model ev. nicht vollstaendig");
    }
    else
    {
        bStarDrawPreviewMode = bPreview;
    }
}


void SdrModel::PrepareStore()
{
    // is done by PreSave now
    DBG_ERROR("Please call PreSave now. It'll do the desired job.");
}

void SdrModel::PreSave()
{
    sal_uInt16 nCnt(GetMasterPageCount());

    for(sal_uInt16 a(0); a < nCnt; a++)
    {
        // MasterPages
        const SdrPage& rPage = *GetMasterPage(a);
        SdrObject* pObj = rPage.GetBackgroundObj();
        if( pObj )
            pObj->PreSave();

        for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
            rPage.GetObj(b)->PreSave();
    }

    nCnt = GetPageCount();
    for(a = 0; a < nCnt; a++)
    {
        // Pages
        const SdrPage& rPage = *GetPage(a);
        SdrObject* pObj = rPage.GetBackgroundObj();
        if( pObj )
            pObj->PreSave();

        for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
            rPage.GetObj(b)->PreSave();
    }
}

void SdrModel::PostSave()
{
    sal_uInt16 nCnt(GetMasterPageCount());

    for(sal_uInt16 a(0); a < nCnt; a++)
    {
        // MasterPages
        const SdrPage& rPage = *GetMasterPage(a);
        SdrObject* pObj = rPage.GetBackgroundObj();
        if( pObj )
            pObj->PostSave();

        for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
            rPage.GetObj(b)->PostSave();
    }

    nCnt = GetPageCount();
    for(a = 0; a < nCnt; a++)
    {
        // Pages
        const SdrPage& rPage = *GetPage(a);
        SdrObject* pObj = rPage.GetBackgroundObj();
        if( pObj )
            pObj->PostSave();

        for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
            rPage.GetObj(b)->PostSave();
    }
}

uno::Reference< uno::XInterface > SdrModel::getUnoModel()
{
    // try weak reference first
    uno::Reference< uno::XInterface > xModel( mxUnoModel );

#ifndef SVX_LIGHT
    if( !xModel.is() )
    {
        // create one
        xModel = createUnoModel();

        mxUnoModel = xModel;
    }
#endif

    return xModel;

}

uno::Reference< uno::XInterface > SdrModel::createUnoModel()
{
    DBG_ERROR( "SdrModel::createUnoModel() - base implementation should not be called!" );
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt;
    return xInt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrHint,SfxHint);

SdrHint::SdrHint(const SdrPage& rNewPage)
{
    aRect=Rectangle(0,0,rNewPage.GetWdt(),rNewPage.GetHgt());
    pPage=&rNewPage;
    pObj=NULL;
    pObjList=&rNewPage;
    bNeedRepaint=TRUE;
    eHint=HINT_PAGECHG;
}

SdrHint::SdrHint(const SdrObject& rNewObj)
{
    aRect=rNewObj.GetBoundRect();
    pPage=rNewObj.GetPage();
    pObj=&rNewObj;
    pObjList=rNewObj.GetObjList();
    bNeedRepaint=TRUE;
    eHint=HINT_OBJCHG;
}

SdrHint::SdrHint(const SdrObject& rNewObj, const Rectangle& rRect)
{
    aRect = rRect;
    pPage = rNewObj.GetPage();
    pObj = &rNewObj;
    pObjList = rNewObj.GetObjList();
    bNeedRepaint = TRUE;
    eHint = HINT_OBJCHG;
}

