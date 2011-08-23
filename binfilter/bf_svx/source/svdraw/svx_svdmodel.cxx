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

#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#include "svdmodel.hxx"


#ifndef _SFX_WHITER_HXX 
#include <bf_svtools/whiter.hxx>
#endif

#ifndef _SVX_XIT_HXX 
#include "xit.hxx"
#endif

#ifndef _XDEF_HXX
#include <xdef.hxx>
#endif

#ifndef _SVX_XBTMPIT_HXX
#include "xbtmpit.hxx"
#endif
#ifndef _SVX_XLNDSIT_HXX
#include "xlndsit.hxx"
#endif
#ifndef _SVX_XLNEDIT_HXX //autogen
#include "xlnedit.hxx"
#endif
#ifndef _SVX_XFLFTRIT_HXX
#include "xflftrit.hxx"
#endif
#ifndef _SVX_XFLHTIT_HXX //autogen
#include "xflhtit.hxx"
#endif 
#ifndef _SVX_XLNSTIT_HXX
#include "xlnstit.hxx"
#endif


#include "editeng.hxx"   // Fuer EditEngine::CreatePool()

#include "xtable.hxx"

#include "svditer.hxx"
#include "svdio.hxx"
#include "svdpage.hxx"
#include "svdundo.hxx"
#include "svdpool.hxx"
#include "svdetc.hxx"
#include "svdoutl.hxx"
#include "svdoole2.hxx"
#include "svdstr.hrc"   // Objektname
#include "svdoutlinercache.hxx"

#include <eeitemid.hxx>

#ifndef SVX_LIGHT
#ifndef _SVX_ASIANCFG_HXX
#include "asiancfg.hxx"
#endif
#endif

#ifndef _EEITEM_HXX
#include "eeitem.hxx"
#endif

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
#include <bf_svtools/style.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif




#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include "forbiddencharacterstable.hxx"
#endif

#ifndef _PERSIST_HXX
#include <bf_so3/persist.hxx>
#endif

#ifndef _ZFORLIST_HXX
#include <bf_svtools/zforlist.hxx>
#endif


// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif


// #95114#
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _OSL_ENDIAN_H_
#include <osl/endian.h>
#endif

namespace binfilter {

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrModelInfo::SdrModelInfo(FASTBOOL bInit):
/*N*/ 	aCreationDate(Date(0),Time(0)),
/*N*/ 	aLastWriteDate(Date(0),Time(0)),
/*N*/ 	aLastReadDate(Date(0),Time(0)),
/*N*/ 	aLastPrintDate(Date(0),Time(0)),
/*N*/ 	eCreationCharSet(RTL_TEXTENCODING_DONTKNOW),
/*N*/ 	eLastWriteCharSet(RTL_TEXTENCODING_DONTKNOW),
/*N*/ 	eLastReadCharSet(RTL_TEXTENCODING_DONTKNOW)
/*N*/ {
/*N*/ 	if (bInit)
/*N*/ 	{
/*N*/ 		aCreationDate = DateTime();
/*N*/ 		eCreationCharSet = gsl_getSystemTextEncoding();
/*N*/ 	}
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrModelInfo& rModInfo)
/*N*/ {
/*N*/ 	SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrModelInfo");
/*N*/ #endif
/*N*/ 	rOut<<UINT32(rModInfo.aCreationDate.GetDate());
/*N*/ 	rOut<<UINT32(rModInfo.aCreationDate.GetTime());
/*N*/ 
/*N*/ 	// #90477# rOut<<UINT8( GetStoreCharSet( rModInfo.eCreationCharSet ) );
/*N*/ 	rOut << UINT8(GetSOStoreTextEncoding(rModInfo.eCreationCharSet, (sal_uInt16)rOut.GetVersion()));
/*N*/ 
    /* Since we removed old SV-stuff there is no way to determine system-speciefic informations, yet.
       We just have to write anythink in the file for compatibility:
            eCreationGUI	eLastWriteGUI	eLastReadGUI
            eCreationCPU	eLastWriteCPU	eLastReadCPU
            eCreationSys	eLastWriteSys	eLastReadSys


    */
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eCreationGUI);
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eCreationCPU);
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eCreationSys);
/*N*/ 
/*N*/ 	rOut<<UINT32(rModInfo.aLastWriteDate.GetDate());
/*N*/ 	rOut<<UINT32(rModInfo.aLastWriteDate.GetTime());
/*N*/ 
/*N*/ 	// #90477# rOut<<UINT8( GetStoreCharSet( rModInfo.eLastWriteCharSet ) );
/*N*/ 	rOut << UINT8(GetSOStoreTextEncoding(rModInfo.eLastWriteCharSet, (sal_uInt16)rOut.GetVersion()));
/*N*/ 
/*N*/ 	// see comment above
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eLastWriteGUI);
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eLastWriteCPU);
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eLastWriteSys);
/*N*/ 
/*N*/ 	rOut<<UINT32(rModInfo.aLastReadDate.GetDate());
/*N*/ 	rOut<<UINT32(rModInfo.aLastReadDate.GetTime());
/*N*/ 
/*N*/ 	// #90477# rOut<<UINT8( GetStoreCharSet( rModInfo.eLastReadCharSet ) );
/*N*/ 	rOut << UINT8(GetSOStoreTextEncoding(rModInfo.eLastReadCharSet, (sal_uInt16)rOut.GetVersion()));
/*N*/ 
/*N*/ 	// see comment above
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eLastReadGUI);
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eLastReadCPU);
/*N*/ 	rOut<<UINT8(0);		//	rOut<<UINT8(rModInfo.eLastReadSys);
/*N*/ 
/*N*/ 	rOut<<UINT32(rModInfo.aLastPrintDate.GetDate());
/*N*/ 	rOut<<UINT32(rModInfo.aLastPrintDate.GetTime());
/*N*/ 	return rOut;
/*N*/ }

/*N*/ SvStream& operator>>(SvStream& rIn, SdrModelInfo& rModInfo)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return rIn;
/*N*/ 	SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrModelInfo");
/*N*/ #endif
/*N*/ 	UINT8  n8;
/*N*/ 	UINT32 n32;
/*N*/ 	rIn>>n32; rModInfo.aCreationDate.SetDate(n32);
/*N*/ 	rIn>>n32; rModInfo.aCreationDate.SetTime(n32);
/*N*/ 
/*N*/ 	// #90477# rIn>>n8;  rModInfo.eCreationCharSet=rtl_TextEncoding(n8);
/*N*/ 	rIn >> n8;
/*N*/ 	n8 = (UINT8)GetSOLoadTextEncoding((rtl_TextEncoding)n8, (sal_uInt16)rIn.GetVersion());
/*N*/ 	rModInfo.eCreationCharSet = rtl_TextEncoding(n8);
/*N*/ 
    /* Since we removed old SV-stuff there is no way to determine system-speciefic informations, yet.
       We just have to write anythink in the file for compatibility:
            eCreationGUI	eLastWriteGUI	eLastReadGUI
            eCreationCPU	eLastWriteCPU	eLastReadCPU
            eCreationSys	eLastWriteSys	eLastReadSys


    */
/*N*/ 	rIn>>n8;  //	rModInfo.eCreationGUI=GUIType(n8);
/*N*/ 	rIn>>n8;  //	rModInfo.eCreationCPU=CPUType(n8);
/*N*/ 	rIn>>n8;  //	rModInfo.eCreationSys=SystemType(n8);
/*N*/ 	rIn>>n32; rModInfo.aLastWriteDate.SetDate(n32);
/*N*/ 	rIn>>n32; rModInfo.aLastWriteDate.SetTime(n32);
/*N*/ 
/*N*/ 	// #90477# rIn>>n8;  rModInfo.eLastWriteCharSet=rtl_TextEncoding(n8);
/*N*/ 	rIn >> n8;
/*N*/ 	n8 = (UINT8)GetSOLoadTextEncoding((rtl_TextEncoding)n8, (sal_uInt16)rIn.GetVersion());
/*N*/ 	rModInfo.eLastWriteCharSet = rtl_TextEncoding(n8);
/*N*/ 
/*N*/ 	// see comment above
/*N*/ 	rIn>>n8;  //	rModInfo.eLastWriteGUI=GUIType(n8);
/*N*/ 	rIn>>n8;  //	rModInfo.eLastWriteCPU=CPUType(n8);
/*N*/ 	rIn>>n8;  //	rModInfo.eLastWriteSys=SystemType(n8);
/*N*/ 
/*N*/ 	rIn>>n32; rModInfo.aLastReadDate.SetDate(n32);
/*N*/ 	rIn>>n32; rModInfo.aLastReadDate.SetTime(n32);
/*N*/ 
/*N*/ 	// #90477# rIn>>n8;  rModInfo.eLastReadCharSet=rtl_TextEncoding(n8);
/*N*/ 	rIn >> n8;
/*N*/ 	n8 = (UINT8)GetSOLoadTextEncoding((rtl_TextEncoding)n8, (sal_uInt16)rIn.GetVersion());
/*N*/ 	rModInfo.eLastReadCharSet = rtl_TextEncoding(n8);
/*N*/ 
/*N*/ 	// see comment above
/*N*/ 	rIn>>n8;  //	rModInfo.eLastReadGUI=GUIType(n8);
/*N*/ 	rIn>>n8;  //	rModInfo.eLastReadCPU=CPUType(n8);
/*N*/ 	rIn>>n8;  //	rModInfo.eLastReadSys=SystemType(n8);
/*N*/ 
/*N*/ 	rIn>>n32; rModInfo.aLastPrintDate.SetDate(n32);
/*N*/ 	rIn>>n32; rModInfo.aLastPrintDate.SetTime(n32);
/*N*/ 
/*N*/ 	return rIn;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ DBG_NAME(SdrModel)
/*N*/ TYPEINIT1(SdrModel,SfxBroadcaster);

/*N*/ void SdrModel::ImpCtor(SfxItemPool* pPool, SvPersist* pPers,
/*N*/ 	FASTBOOL bUseExtColorTable, FASTBOOL bLoadRefCounts)
/*N*/ {
/*N*/ 	mbInDestruction=false;
/*N*/ 	aObjUnit=SdrEngineDefaults::GetMapFraction();
/*N*/ 	eObjUnit=SdrEngineDefaults::GetMapUnit();
/*N*/ 	eUIUnit=FUNIT_MM;
/*N*/ 	aUIScale=Fraction(1,1);
/*N*/ 	nUIUnitKomma=0;
/*N*/ 	bUIOnlyKomma=FALSE;
/*N*/ 	pLayerAdmin=NULL;
/*N*/ 	pItemPool=pPool;
/*N*/ 	bMyPool=FALSE;
/*N*/ 	pPersist=pPers;
/*N*/ 	pDrawOutliner=NULL;
/*N*/ 	pHitTestOutliner=NULL;
/*N*/ 	pRefOutDev=NULL;
/*N*/ 	nProgressAkt=0;
/*N*/ 	nProgressMax=0;
/*N*/ 	nProgressOfs=0;
/*N*/ 	pStyleSheetPool=NULL;
/*N*/ 	pDefaultStyleSheet=NULL;
/*N*/ 	pLinkManager=NULL;
/*N*/ 	pLoadedModel=NULL;
/*N*/ 	pUndoStack=NULL;
/*N*/ 	pRedoStack=NULL;
/*N*/ 	pAktPaintPV=NULL;
/*N*/ 	nMaxUndoCount=16;
/*N*/ 	pAktUndoGroup=NULL;
/*N*/ 	nUndoLevel=0;
/*N*/ 	nProgressPercent=0;
/*N*/ 	nLoadVersion=0;
/*N*/ 	bExtColorTable=FALSE;
/*N*/ 	bChanged=FALSE;
/*N*/ 	bInfoChanged=FALSE;
/*N*/ 	bPagNumsDirty=FALSE;
/*N*/ 	bMPgNumsDirty=FALSE;
/*N*/ 	bPageNotValid=FALSE;
/*N*/ 	bSavePortable=FALSE;
/*N*/ 	bSaveCompressed=FALSE;
/*N*/ 	bSaveNative=FALSE;
/*N*/ 	bSwapGraphics=FALSE;
/*N*/ 	nSwapGraphicsMode=SDR_SWAPGRAPHICSMODE_DEFAULT;
/*N*/ 	bSaveOLEPreview=FALSE;
/*N*/ 	bPasteResize=FALSE;
/*N*/ 	bNoBitmapCaching=FALSE;
/*N*/ 	bLoading=FALSE;
/*N*/ 	bStreamingSdrModel=FALSE;
/*N*/ 	bReadOnly=FALSE;
/*N*/ 	nStreamCompressMode=COMPRESSMODE_NONE;
/*N*/ 	nStreamNumberFormat=NUMBERFORMAT_INT_BIGENDIAN;
/*N*/ 	nDefaultTabulator=0;
/*N*/ 	pColorTable=NULL;
/*N*/ 	pDashList=NULL;
/*N*/ 	pLineEndList=NULL;
/*N*/ 	pHatchList=NULL;
/*N*/ 	pGradientList=NULL;
/*N*/ 	pBitmapList=NULL;
/*N*/ 	mpNumberFormatter = NULL;
/*N*/ 	bTransparentTextFrames=FALSE;
/*N*/ 	bStarDrawPreviewMode = FALSE;
/*N*/ 	nStarDrawPreviewMasterPageNum = SDRPAGE_NOTFOUND;
/*N*/ 	pModelStorage = NULL;
/*N*/ 	mpForbiddenCharactersTable = NULL;
/*N*/ 	mbModelLocked = FALSE;
/*N*/ 	mpOutlinerCache = NULL;
/*N*/ 	mbKernAsianPunctuation = sal_False;
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/     SvxAsianConfig aAsian;
/*N*/ 	mnCharCompressType = aAsian.GetCharDistanceCompression();
/*N*/ #else
/*N*/ 	mnCharCompressType = 0;
/*N*/ #endif
/*N*/ 
/*N*/ #ifdef OSL_LITENDIAN
/*N*/ 	nStreamNumberFormat=NUMBERFORMAT_INT_LITTLEENDIAN;
/*N*/ #endif
/*N*/ 	bExtColorTable=bUseExtColorTable;
/*N*/ 
/*N*/ 	if ( pPool == NULL )
/*N*/     {
/*N*/ 		pItemPool=new SdrItemPool(SDRATTR_START, SDRATTR_END, bLoadRefCounts);
/*N*/ 		// Der Outliner hat keinen eigenen Pool, deshalb den der EditEngine
/*N*/ 		SfxItemPool* pOutlPool=EditEngine::CreatePool( bLoadRefCounts );
/*N*/ 		// OutlinerPool als SecondaryPool des SdrPool
/*N*/ 		pItemPool->SetSecondaryPool(pOutlPool);
/*N*/ 		// Merken, dass ich mir die beiden Pools selbst gemacht habe
/*N*/ 		bMyPool=TRUE;
/*N*/ 	}
/*N*/ 	pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
/*N*/ 
/*N*/ // SJ: #95129# using static SdrEngineDefaults only if default SvxFontHeight item is not available
/*N*/ 	const SfxPoolItem* pPoolItem = pItemPool->GetPoolDefaultItem( ITEMID_FONTHEIGHT );
/*N*/     if ( pPoolItem )
/*N*/         nDefTextHgt = ((SvxFontHeightItem*)pPoolItem)->GetHeight();
/*N*/     else
/*N*/         nDefTextHgt = SdrEngineDefaults::GetFontHeight();
/*N*/ 
/*N*/ 	SetTextDefaults();
/*N*/ 	pLayerAdmin=new SdrLayerAdmin;
/*N*/ 	pLayerAdmin->SetModel(this);
/*N*/ 	ImpSetUIUnit();
/*N*/ 
/*N*/ 	// den DrawOutliner OnDemand erzeugen geht noch nicht, weil ich den Pool
/*N*/ 	// sonst nicht kriege (erst ab 302!)
/*N*/ 	pDrawOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this );
/*N*/ 	ImpSetOutlinerDefaults(pDrawOutliner, TRUE);
/*N*/ 
/*N*/ 	pHitTestOutliner = SdrMakeOutliner( OUTLINERMODE_TEXTOBJECT, this );
/*N*/ 	ImpSetOutlinerDefaults(pHitTestOutliner, TRUE);
/*N*/ 
/*N*/ 	ImpCreateTables();
/*N*/ }

/*N*/ SdrModel::SdrModel(SfxItemPool* pPool, SvPersist* pPers, INT32 bLoadRefCounts):
/*N*/ 	aInfo(TRUE),
/*N*/ 	aPages(1024,32,32),
/*N*/ 	aMaPag(1024,32,32)
/*N*/ {
/*N*/ #ifdef TIMELOG
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
/*N*/ #endif
/*N*/ 
/*N*/ 	DBG_CTOR(SdrModel,NULL);
/*N*/ 	ImpCtor(pPool,pPers,FALSE, (FASTBOOL)bLoadRefCounts);
/*N*/ }

/*N*/ SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, SvPersist* pPers, INT32 bLoadRefCounts):
/*N*/ 	aInfo(TRUE),
/*N*/ 	aPages(1024,32,32),
/*N*/ 	aMaPag(1024,32,32),
/*N*/ 	aTablePath(rPath)
/*N*/ {
/*N*/ #ifdef TIMELOG
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
/*N*/ #endif
/*N*/ 
/*N*/ 	DBG_CTOR(SdrModel,NULL);
/*N*/ 	ImpCtor(pPool,pPers,FALSE, (FASTBOOL)bLoadRefCounts);
/*N*/ }

/*N*/ SdrModel::SdrModel(const String& rPath, SfxItemPool* pPool, SvPersist* pPers, FASTBOOL bUseExtColorTable, INT32 bLoadRefCounts):
/*N*/ 	aInfo(TRUE),
/*N*/ 	aPages(1024,32,32),
/*N*/ 	aMaPag(1024,32,32),
/*N*/ 	aTablePath(rPath)
/*N*/ {
/*N*/ #ifdef TIMELOG
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
/*N*/ #endif
/*N*/ 
/*N*/ 	DBG_CTOR(SdrModel,NULL);
/*N*/ 	ImpCtor(pPool,pPers,bUseExtColorTable, (FASTBOOL)bLoadRefCounts);
/*N*/ }

/*N*/ SdrModel::SdrModel(const SdrModel& rSrcModel):
/*N*/ 	aPages(1024,32,32),
/*N*/ 	aMaPag(1024,32,32)
/*N*/ {
/*N*/ #ifdef TIMELOG
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::SdrModel(...)" );
/*N*/ #endif
/*N*/ 
/*N*/ 	// noch nicht implementiert
/*N*/ 	DBG_ERROR("SdrModel::CopyCtor() ist noch nicht implementiert");
/*N*/ }

/*N*/ SdrModel::~SdrModel()
/*N*/ {
/*N*/ #ifdef TIMELOG
/*N*/     RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "svx", "aw93748", "SdrModel::~SdrModel(...)" );
/*N*/ #endif
/*N*/ 
/*N*/ 	DBG_DTOR(SdrModel,NULL);
/*N*/ 
/*N*/ 	mbInDestruction = true;
/*N*/ 
/*N*/ 	Broadcast(SdrHint(HINT_MODELCLEARED));
/*N*/ 
/*N*/ 	delete mpOutlinerCache;
/*N*/ 
/*N*/ 	ClearUndoBuffer();
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if(pAktUndoGroup)
/*N*/ 	{
/*?*/ 		ByteString aStr("Im Dtor des SdrModel steht noch ein offenes Undo rum: \"");
/*?*/ 
/*?*/ 		aStr += ByteString(pAktUndoGroup->GetComment(), gsl_getSystemTextEncoding());
/*?*/ 		aStr += '\"';
/*?*/ 
/*?*/ 		DBG_ERROR(aStr.GetBuffer());
/*N*/ 	}
/*N*/ #endif
/*N*/ 	if (pAktUndoGroup!=NULL) delete pAktUndoGroup;
/*N*/ 	Clear();
/*N*/ 	delete pLayerAdmin;
/*N*/ 
/*N*/ 	// Den DrawOutliner erst nach dem ItemPool loeschen, da
/*N*/ 	// der ItemPool Items des DrawOutliners referenziert !!! (<- das war mal)
/*N*/ 	// Wg. Problem bei Malte Reihenfolge wieder umgestellt.
/*N*/ 	// Loeschen des Outliners vor dem loeschen des ItemPools
/*N*/ 	delete pHitTestOutliner;
/*N*/ 	delete pDrawOutliner;
/*N*/ 
/*N*/ 	// StyleSheetPool loeschen, abgeleitete Klassen sollten dies nicht tun,
/*N*/ 	// da die DrawingEngine moeglicherweise im Destruktor noch auf den
/*N*/ 	// StyleSheetPool zugreifen muss (SB)
/*N*/ 	delete pStyleSheetPool;
/*N*/ 
/*N*/ 	if (bMyPool) { // Pools loeschen, falls es meine sind
/*N*/ 		SfxItemPool* pOutlPool=pItemPool->GetSecondaryPool();
/*N*/ 		delete pItemPool;
/*N*/ 		// Der OutlinerPool muss nach dem ItemPool plattgemacht werden, da der
/*N*/ 		// ItemPool SetItems enthaelt die ihrerseits Items des OutlinerPools
/*N*/ 		// referenzieren (Joe)
/*N*/ 		delete pOutlPool;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( mpForbiddenCharactersTable )
/*N*/ 		mpForbiddenCharactersTable->release();
/*N*/ 
/*N*/ 	delete pLoadedModel;
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	// Tabellen, Listen und Paletten loeschen
/*N*/ 	if (!bExtColorTable) delete pColorTable;
/*N*/ 	delete pDashList;
/*N*/ 	delete pLineEndList;
/*N*/ 	delete pHatchList;
/*N*/ 	delete pGradientList;
/*N*/ 	delete pBitmapList;
/*N*/ #endif
/*N*/ 
/*N*/ 	if(mpNumberFormatter)
/*?*/ 		delete mpNumberFormatter;
/*N*/ }

/*N*/ bool SdrModel::IsInDestruction() const
/*N*/ {
/*N*/ 	return mbInDestruction;
/*N*/ }


// noch nicht implementiert:


/*N*/ void SdrModel::SetSwapGraphics( FASTBOOL bSwap )
/*N*/ {
/*N*/ 	bSwapGraphics = bSwap;
/*N*/ }

/*?*/ FASTBOOL SdrModel::IsReadOnly() const
/*?*/ {
/*?*/ 	return bReadOnly;
/*?*/ }

/*?*/ void SdrModel::SetReadOnly(FASTBOOL bYes)
/*?*/ {
/*?*/ 	bReadOnly=bYes;
/*?*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrModel::DoProgress(ULONG nVal)
/*N*/ {
/*N*/ 	if (aIOProgressLink.IsSet()) {
/*N*/ 		if (nVal==0) { // Anfang
/*N*/ 			USHORT nVal=0;
/*N*/ 			aIOProgressLink.Call(&nVal);
/*N*/ 			nProgressPercent=0;
/*N*/ 			nProgressAkt=0;
/*N*/ 		} else if (nVal==0xFFFFFFFF) { // Ende
/*N*/ 			USHORT nVal=100;
/*N*/ 			aIOProgressLink.Call(&nVal);
/*N*/ 			nProgressPercent=100;
/*N*/ 			nProgressAkt=nProgressMax;
/*N*/ 		} else if (nVal!=nProgressAkt && nProgressMax!=0) { // dazwischen
/*N*/ 			USHORT nPercent;
/*N*/ 			if (nVal>nProgressOfs) nVal-=nProgressOfs; else nVal=0;
/*N*/ 			if (nVal>nProgressMax) nVal=nProgressMax;
/*N*/ 			if (nVal<=0x00FFFFFF) nPercent=USHORT(nVal*100/nProgressMax);
/*N*/ 			else {
/*?*/ 				ULONG nBla=nProgressMax/100; // Weil sonst Ueberlauf!
/*?*/ 				nPercent=USHORT(nVal/=nBla);
/*N*/ 			}
/*N*/ 			if (nPercent==0) nPercent=1;
/*N*/ 			if (nPercent>99) nPercent=99;
/*N*/ 			if (nPercent>nProgressPercent) {
/*N*/ 				aIOProgressLink.Call(&nPercent);
/*N*/ 				nProgressPercent=nPercent;
/*N*/ 			}
/*N*/ 			if (nVal>nProgressAkt) {
/*N*/ 				nProgressAkt=nVal;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetMaxUndoActionCount(ULONG nAnz)
/*N*/ {
/*N*/ 	if (nAnz<1) nAnz=1;
/*N*/ 	nMaxUndoCount=nAnz;
/*N*/ 	if (pUndoStack!=NULL) {
/*?*/ 		while (pUndoStack->Count()>nMaxUndoCount) {
/*?*/ 			delete (SfxUndoAction*) pUndoStack->Remove(pUndoStack->Count());
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::ClearUndoBuffer()
/*N*/ {
/*N*/ 	if (pUndoStack!=NULL) {
/*?*/ 		while (pUndoStack->Count()!=0) {
/*?*/ 			delete (SfxUndoAction*) pUndoStack->Remove(pUndoStack->Count()-1);
/*?*/ 		}
/*?*/ 		delete pUndoStack;
/*?*/ 		pUndoStack=NULL;
/*N*/ 	}
/*N*/ 	if (pRedoStack!=NULL) {
/*?*/ 		while (pRedoStack->Count()!=0) {
/*?*/ 			delete (SfxUndoAction*) pRedoStack->Remove(pRedoStack->Count()-1);
/*?*/ 		}
/*?*/ 		delete pRedoStack;
/*?*/ 		pRedoStack=NULL;
/*N*/ 	}
/*N*/ }












/*N*/ void SdrModel::AddUndo(SdrUndoAction* pUndo)
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrModel::ImpCreateTables()
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	// der Writer hat seinen eigenen ColorTable
/*N*/ 	if (!bExtColorTable) pColorTable=new XColorTable(aTablePath,(XOutdevItemPool*)pItemPool);
/*N*/ 	pDashList    =new XDashList    (aTablePath,(XOutdevItemPool*)pItemPool);
/*N*/ 	pLineEndList =new XLineEndList (aTablePath,(XOutdevItemPool*)pItemPool);
/*N*/ 	pHatchList   =new XHatchList   (aTablePath,(XOutdevItemPool*)pItemPool);
/*N*/ 	pGradientList=new XGradientList(aTablePath,(XOutdevItemPool*)pItemPool);
/*N*/ 	pBitmapList  =new XBitmapList  (aTablePath,(XOutdevItemPool*)pItemPool);
/*N*/ #endif
/*N*/ }

/*N*/ void SdrModel::Clear()
/*N*/ {
/*N*/ 	mbInDestruction = true;
/*N*/ 
/*N*/ 	sal_Int32 i;
/*N*/ 	// delete all drawing pages
/*N*/ 	sal_Int32 nAnz=GetPageCount();
/*N*/ 	for (i=nAnz-1; i>=0; i--)
/*N*/ 	{
/*N*/ 		DeletePage( (USHORT)i );
/*N*/ 	}
/*N*/ 	aPages.Clear();
/*N*/ 
/*N*/ 	// delete all Masterpages
/*N*/ 	nAnz=GetMasterPageCount();
/*N*/ 	for(i=nAnz-1; i>=0; i--)
/*N*/ 	{
/*N*/ 		DeleteMasterPage( (USHORT)i );
/*N*/ 	}
/*N*/ 	aMaPag.Clear();
/*N*/ 
/*N*/ 	pLayerAdmin->ClearLayer();
/*N*/ 	pLayerAdmin->ClearLayerSets();
/*N*/ 	//SetChanged();
/*N*/ }

/*N*/ SdrModel* SdrModel::AllocModel() const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	SdrModel* pModel=new SdrModel;
/*N*/ 	return pModel;
/*N*/ }

/*N*/ SdrPage* SdrModel::AllocPage(FASTBOOL bMasterPage)
/*N*/ {
/*N*/ 	return new SdrPage(*this,bMasterPage);
/*N*/ }


/*N*/ void SdrModel::DisposeLoadedModels()
/*N*/ {
/*N*/ 	delete pLoadedModel;
/*N*/ 	pLoadedModel = NULL;
/*N*/ 	aLoadedModelFN = String();
/*N*/ }

/*N*/ void SdrModel::SetTextDefaults() const
/*N*/ {
/*N*/ 	SetTextDefaults( pItemPool, nDefTextHgt );
/*N*/ }


/*N*/ void SdrModel::SetTextDefaults( SfxItemPool* pItemPool, ULONG nDefTextHgt )
/*N*/ {
/*N*/ 	// #95114# set application-language specific dynamic pool language defaults
/*N*/ 	SvxFontItem aSvxFontItem;
/*N*/ 	SvxFontItem aSvxFontItemCJK(EE_CHAR_FONTINFO_CJK);
/*N*/ 	SvxFontItem aSvxFontItemCTL(EE_CHAR_FONTINFO_CTL);
/*N*/ 	sal_uInt16 nLanguage(Application::GetSettings().GetLanguage());
/*N*/ 
/*N*/ 	// get DEFAULTFONT_LATIN_TEXT and set at pool as dynamic default
/*N*/ 	Font aFont(OutputDevice::GetDefaultFont(DEFAULTFONT_LATIN_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
/*N*/ 	aSvxFontItem.GetFamily() = aFont.GetFamily();
/*N*/ 	aSvxFontItem.GetFamilyName() = aFont.GetName();
/*N*/ 	aSvxFontItem.GetStyleName().Erase();
/*N*/ 	aSvxFontItem.GetPitch() = aFont.GetPitch();
/*N*/ 	aSvxFontItem.GetCharSet() = aFont.GetCharSet();
/*N*/ 	pItemPool->SetPoolDefaultItem(aSvxFontItem);
/*N*/ 
/*N*/ 	// get DEFAULTFONT_CJK_TEXT and set at pool as dynamic default
/*N*/ 	Font aFontCJK(OutputDevice::GetDefaultFont(DEFAULTFONT_CJK_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
/*N*/ 	aSvxFontItemCJK.GetFamily() = aFontCJK.GetFamily();
/*N*/ 	aSvxFontItemCJK.GetFamilyName() = aFontCJK.GetName();
/*N*/ 	aSvxFontItemCJK.GetStyleName().Erase();
/*N*/ 	aSvxFontItemCJK.GetPitch() = aFontCJK.GetPitch();
/*N*/ 	aSvxFontItemCJK.GetCharSet() = aFontCJK.GetCharSet();
/*N*/ 	pItemPool->SetPoolDefaultItem(aSvxFontItemCJK);
/*N*/ 
/*N*/ 	// get DEFAULTFONT_CTL_TEXT and set at pool as dynamic default
/*N*/ 	Font aFontCTL(OutputDevice::GetDefaultFont(DEFAULTFONT_CTL_TEXT, nLanguage, DEFAULTFONT_FLAGS_ONLYONE, 0));
/*N*/ 	aSvxFontItemCTL.GetFamily() = aFontCTL.GetFamily();
/*N*/ 	aSvxFontItemCTL.GetFamilyName() = aFontCTL.GetName();
/*N*/ 	aSvxFontItemCTL.GetStyleName().Erase();
/*N*/ 	aSvxFontItemCTL.GetPitch() = aFontCTL.GetPitch();
/*N*/ 	aSvxFontItemCTL.GetCharSet() = aFontCTL.GetCharSet();
/*N*/ 	pItemPool->SetPoolDefaultItem(aSvxFontItemCTL);
/*N*/ 
/*N*/ 	// set dynamic FontHeight defaults
/*N*/ 	pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT ) );
/*N*/ 	pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CJK ) );
/*N*/ 	pItemPool->SetPoolDefaultItem( SvxFontHeightItem(nDefTextHgt, 100, EE_CHAR_FONTHEIGHT_CTL ) );
/*N*/ 
/*N*/ 	// set FontColor defaults
/*N*/ 	pItemPool->SetPoolDefaultItem( SvxColorItem(SdrEngineDefaults::GetFontColor()) );
/*N*/ }

/*N*/ SdrOutliner& SdrModel::GetDrawOutliner(const SdrTextObj* pObj) const
/*N*/ {
/*N*/ 	pDrawOutliner->SetTextObj(pObj);
/*N*/ 	return *pDrawOutliner;
/*N*/ }


/*N*/ void SdrModel::ImpSetOutlinerDefaults( SdrOutliner* pOutliner, BOOL bInit )
/*N*/ {
    /**************************************************************************
    * Initialisierung der Outliner fuer Textausgabe und HitTest
    **************************************************************************/
/*N*/ 	if( bInit )
/*N*/ 	{
/*N*/ 		pOutliner->EraseVirtualDevice();
/*N*/ 		pOutliner->SetUpdateMode(FALSE);
/*N*/ 		pOutliner->SetEditTextObjectPool(pItemPool);
/*N*/ 		pOutliner->SetDefTab(nDefaultTabulator);
/*N*/ 	}

/*N*/ 	pOutliner->SetRefDevice(GetRefDevice());
/*N*/ 	pOutliner->SetForbiddenCharsTable(GetForbiddenCharsTable());
/*N*/ 	pOutliner->SetAsianCompressionMode( mnCharCompressType );
/*N*/ 	pOutliner->SetKernAsianPunctuation( IsKernAsianPunctuation() );
/*N*/ 
/*N*/ 	if ( !GetRefDevice() )
/*N*/ 	{
/*N*/ 		MapMode aMapMode(eObjUnit, Point(0,0), aObjUnit, aObjUnit);
/*N*/ 		pOutliner->SetRefMapMode(aMapMode);
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetRefDevice(OutputDevice* pDev)
/*N*/ {
/*N*/ 	pRefOutDev=pDev;
/*N*/ 	ImpSetOutlinerDefaults( pDrawOutliner );
/*N*/ 	ImpSetOutlinerDefaults( pHitTestOutliner );
/*N*/ 	RefDeviceChanged();
/*N*/ }

/*N*/ void SdrModel::ImpReformatAllTextObjects()
/*N*/ {
/*N*/ 	if( isLocked() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	USHORT nAnz=GetMasterPageCount();
/*N*/ 	USHORT nNum;
/*N*/ 	for (nNum=0; nNum<nAnz; nNum++) {
/*N*/ 		GetMasterPage(nNum)->ReformatAllTextObjects();
/*N*/ 	}
/*N*/ 	nAnz=GetPageCount();
/*N*/ 	for (nNum=0; nNum<nAnz; nNum++) {
/*N*/ 		GetPage(nNum)->ReformatAllTextObjects();
/*N*/ 	}
/*N*/ }

/** #103122#
    steps over all available pages and sends notify messages to
    all edge objects that are connected to other objects so that
    they may reposition itselfs
*/
/*N*/ void SdrModel::ImpReformatAllEdgeObjects()
/*N*/ {
/*N*/ 	if( isLocked() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	sal_uInt16 nAnz=GetMasterPageCount();
/*N*/ 	sal_uInt16 nNum;
/*N*/ 	for (nNum=0; nNum<nAnz; nNum++)
/*N*/ 	{
/*N*/ 		GetMasterPage(nNum)->ReformatAllEdgeObjects();
/*N*/ 	}
/*N*/ 	nAnz=GetPageCount();
/*N*/ 	for (nNum=0; nNum<nAnz; nNum++)
/*N*/ 	{
/*N*/ 		GetPage(nNum)->ReformatAllEdgeObjects();
/*N*/ 	}
/*N*/ }

/*N*/ SvStream* SdrModel::GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const
/*N*/ {
/*N*/ 	return NULL;
/*N*/ }

// Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.


/*N*/ void SdrModel::RefDeviceChanged()
/*N*/ {
/*N*/ 	Broadcast(SdrHint(HINT_REFDEVICECHG));
/*N*/ 	ImpReformatAllTextObjects();
/*N*/ }
/*N*/ 
/*N*/ void SdrModel::SetDefaultFontHeight(ULONG nVal)
/*N*/ {
/*N*/ 	if (nVal!=nDefTextHgt) {
/*?*/ 		nDefTextHgt=nVal;
/*?*/ 		Broadcast(SdrHint(HINT_DEFFONTHGTCHG));
/*?*/ 		ImpReformatAllTextObjects();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetDefaultTabulator(USHORT nVal)
/*N*/ {
/*N*/ 	if (nDefaultTabulator!=nVal) {
/*N*/ 		nDefaultTabulator=nVal;
/*N*/ 		Outliner& rOutliner=GetDrawOutliner();
/*N*/ 		rOutliner.SetDefTab(nVal);
/*N*/ 		Broadcast(SdrHint(HINT_DEFAULTTABCHG));
/*N*/ 		ImpReformatAllTextObjects();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::ImpSetUIUnit()
/*N*/ {
/*N*/ 	if (aUIScale.GetNumerator()==0 || aUIScale.GetDenominator()==0) aUIScale=Fraction(1,1);
/*N*/ 	FASTBOOL bMapInch=IsInch(eObjUnit);
/*N*/ 	FASTBOOL bMapMetr=IsMetric(eObjUnit);
/*N*/ 	FASTBOOL bUIInch=IsInch(eUIUnit);
/*N*/ 	FASTBOOL bUIMetr=IsMetric(eUIUnit);
/*N*/ 	nUIUnitKomma=0;
/*N*/ 	long nMul=1;
/*N*/ 	long nDiv=1;
/*N*/ 
/*N*/ 	// Zunaechst normalisieren auf m bzw. "
/*N*/ 	switch (eObjUnit) {
/*N*/ 		case MAP_100TH_MM   : nUIUnitKomma+=5; break;
/*N*/ 		case MAP_10TH_MM    : nUIUnitKomma+=4; break;
/*N*/ 		case MAP_MM         : nUIUnitKomma+=3; break;
/*N*/ 		case MAP_CM         : nUIUnitKomma+=2; break;
/*N*/ 		case MAP_1000TH_INCH: nUIUnitKomma+=3; break;
/*N*/ 		case MAP_100TH_INCH : nUIUnitKomma+=2; break;
/*N*/ 		case MAP_10TH_INCH  : nUIUnitKomma+=1; break;
/*N*/ 		case MAP_INCH       : nUIUnitKomma+=0; break;
/*N*/ 		case MAP_POINT      : nDiv=72;     break;          // 1Pt   = 1/72"
/*N*/ 		case MAP_TWIP       : nDiv=144; nUIUnitKomma++; break; // 1Twip = 1/1440"
/*N*/ 		case MAP_PIXEL      : break;
/*N*/ 		case MAP_SYSFONT    : break;
/*N*/ 		case MAP_APPFONT    : break;
/*N*/ 		case MAP_RELATIVE   : break;
/*N*/ 	} // switch
/*N*/ 
/*N*/ 	// 1 mile    =  8 furlong = 63.360" = 1.609.344,0mm
/*N*/ 	// 1 furlong = 10 chains  =  7.920" =   201.168,0mm
/*N*/ 	// 1 chain   =  4 poles   =    792" =    20.116,8mm
/*N*/ 	// 1 pole    =  5 1/2 yd  =    198" =     5.029,2mm
/*N*/ 	// 1 yd      =  3 ft      =     36" =       914,4mm
/*N*/ 	// 1 ft      = 12 "       =      1" =       304,8mm
/*N*/ 	switch (eUIUnit) {
/*N*/ 		case FUNIT_NONE   : break;
/*N*/ 		// Metrisch
/*N*/ 		case FUNIT_100TH_MM: nUIUnitKomma-=5; break;
/*N*/ 		case FUNIT_MM     : nUIUnitKomma-=3; break;
/*N*/ 		case FUNIT_CM     : nUIUnitKomma-=2; break;
/*N*/ 		case FUNIT_M      : nUIUnitKomma+=0; break;
/*N*/ 		case FUNIT_KM     : nUIUnitKomma+=3; break;
/*N*/ 		// Inch
/*N*/ 		case FUNIT_TWIP   : nMul=144; nUIUnitKomma--;  break;  // 1Twip = 1/1440"
/*N*/ 		case FUNIT_POINT  : nMul=72;     break;            // 1Pt   = 1/72"
/*N*/ 		case FUNIT_PICA   : nMul=6;      break;            // 1Pica = 1/6"  ?
/*N*/ 		case FUNIT_INCH   : break;                         // 1"    = 1"
/*N*/ 		case FUNIT_FOOT   : nDiv*=12;    break;            // 1Ft   = 12"
/*N*/ 		case FUNIT_MILE   : nDiv*=6336; nUIUnitKomma++; break; // 1mile = 63360"
/*N*/ 		// sonstiges
/*N*/ 		case FUNIT_CUSTOM : break;
/*N*/ 		case FUNIT_PERCENT: nUIUnitKomma+=2; break;
/*N*/ 	} // switch
/*N*/ 
/*N*/ 	if (bMapInch && bUIMetr) {
/*N*/ 		nUIUnitKomma+=4;
/*N*/ 		nMul*=254;
/*N*/ 	}
/*N*/ 	if (bMapMetr && bUIInch) {
/*N*/ 		nUIUnitKomma-=4;
/*N*/ 		nDiv*=254;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Temporaere Fraction zum Kuerzen
/*N*/ 	Fraction aTempFract(nMul,nDiv);
/*N*/ 	nMul=aTempFract.GetNumerator();
/*N*/ 	nDiv=aTempFract.GetDenominator();
/*N*/ 	// Nun mit dem eingestellten Masstab verknuepfen
/*N*/ 	BigInt nBigMul(nMul);
/*N*/ 	BigInt nBigDiv(nDiv);
/*N*/ 	BigInt nBig1000(1000);
/*N*/ 	nBigMul*=aUIScale.GetDenominator();
/*N*/ 	nBigDiv*=aUIScale.GetNumerator();
/*N*/ 	while (nBigMul>nBig1000) {
/*N*/ 		nUIUnitKomma--;
/*N*/ 		nBigMul/=10;
/*N*/ 	}
/*N*/ 	while (nBigDiv>nBig1000) {
/*N*/ 		nUIUnitKomma++;
/*N*/ 		nBigDiv/=10;
/*N*/ 	}
/*N*/ 	nMul=long(nBigMul);
/*N*/ 	nDiv=long(nBigDiv);
/*N*/ 	switch ((short)nMul) {
/*N*/ 		case   10: nMul=1; nUIUnitKomma--; break;
/*N*/ 		case  100: nMul=1; nUIUnitKomma-=2; break;
/*N*/ 		case 1000: nMul=1; nUIUnitKomma-=3; break;
/*N*/ 	} // switch
/*N*/ 	switch ((short)nDiv) {
/*N*/ 		case   10: nDiv=1; nUIUnitKomma++; break;
/*N*/ 		case  100: nDiv=1; nUIUnitKomma+=2; break;
/*N*/ 		case 1000: nDiv=1; nUIUnitKomma+=3; break;
/*N*/ 	} // switch
/*N*/ 	aUIUnitFact=Fraction(nMul,nDiv);
/*N*/ 	bUIOnlyKomma=nMul==nDiv;
/*N*/ 	TakeUnitStr(eUIUnit,aUIUnitStr);
/*N*/ }


/*N*/ void SdrModel::SetScaleUnit(MapUnit eMap)
/*N*/ {
/*N*/ 	if (eObjUnit!=eMap) {
/*N*/ 		eObjUnit=eMap;
/*N*/ 		pItemPool->SetDefaultMetric((SfxMapUnit)eObjUnit);
/*N*/ 		ImpSetUIUnit();
/*N*/ 		ImpSetOutlinerDefaults( pDrawOutliner );
/*N*/ 		ImpSetOutlinerDefaults( pHitTestOutliner );
/*N*/ 		ImpReformatAllTextObjects(); // #40424#
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetScaleFraction(const Fraction& rFrac)
/*N*/ {
/*N*/ 	if (aObjUnit!=rFrac) {
/*?*/ 		aObjUnit=rFrac;
/*?*/ 		ImpSetUIUnit();
/*?*/ 		ImpSetOutlinerDefaults( pDrawOutliner );
/*?*/ 		ImpSetOutlinerDefaults( pHitTestOutliner );
/*?*/ 		ImpReformatAllTextObjects(); // #40424#
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetUIUnit(FieldUnit eUnit)
/*N*/ {
/*N*/ 	if (eUIUnit!=eUnit) {
/*?*/ 		eUIUnit=eUnit;
/*?*/ 		ImpSetUIUnit();
/*?*/ 		ImpReformatAllTextObjects(); // #40424#
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetUIScale(const Fraction& rScale)
/*N*/ {
/*N*/ 	if (aUIScale!=rScale) {
/*?*/ 		aUIScale=rScale;
/*?*/ 		ImpSetUIUnit();
/*?*/ 		ImpReformatAllTextObjects(); // #40424#
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetUIUnit(FieldUnit eUnit, const Fraction& rScale)
/*N*/ {
/*N*/ 	if (eUIUnit!=eUnit || aUIScale!=rScale) {
/*N*/ 		eUIUnit=eUnit;
/*N*/ 		aUIScale=rScale;
/*N*/ 		ImpSetUIUnit();
/*N*/ 		ImpReformatAllTextObjects(); // #40424#
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::TakeUnitStr(FieldUnit eUnit, XubString& rStr) const
/*N*/ {
/*N*/ 	switch(eUnit)
/*N*/ 	{
/*N*/ 		default:
/*N*/ 		case FUNIT_NONE   :
/*N*/ 		case FUNIT_CUSTOM :
/*N*/ 		{
/*?*/ 			rStr = String();
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_100TH_MM:
/*?*/ 		{
/*?*/ 			sal_Char aText[] = "/100mm";
/*?*/ 			rStr = UniString(aText, sizeof(aText-1));
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_MM     :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "mm";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_CM     :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "cm";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_M      :
/*N*/ 		{
/*N*/ 			rStr = String();
/*N*/ 			rStr += sal_Unicode('m');
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_KM     :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "km";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_TWIP   :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "twip";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_POINT  :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "pt";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_PICA   :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "pica";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_INCH   :
/*N*/ 		{
/*N*/ 			rStr = String();
/*N*/ 			rStr += sal_Unicode('"');
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_FOOT   :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "ft";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_MILE   :
/*N*/ 		{
/*N*/ 			sal_Char aText[] = "mile(s)";
/*N*/ 			rStr = UniString(aText, sizeof(aText-1));
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case FUNIT_PERCENT:
/*N*/ 		{
/*N*/ 			rStr = String();
/*N*/ 			rStr += sal_Unicode('%');
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*?*/ void SdrModel::TakeMetricStr(long nVal, XubString& rStr, FASTBOOL bNoUnitChars, sal_Int32 nNumDigits) const
/*?*/ {
/*?*/ 	if(!bUIOnlyKomma)
/*?*/ 		nVal = (nVal * aUIUnitFact.GetNumerator()) / aUIUnitFact.GetDenominator();
/*?*/ 
/*?*/ 	BOOL bNeg(nVal < 0);
/*?*/ 
/*?*/ 	if(bNeg)
/*?*/ 		nVal = -nVal;
/*?*/ 
/*?*/     SvtSysLocale aSysLoc;
/*?*/     const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();
/*?*/ 	sal_Int32 nKomma(nUIUnitKomma);
/*?*/ 	if( -1 == nNumDigits )
/*?*/ 		nNumDigits = rLoc.getNumDigits();
/*?*/ 
/*?*/ 	while(nKomma > nNumDigits)
/*?*/ 	{
/*?*/ 		// das bedeutet teilen und runden
/*?*/ 		sal_Int32 nDiff(nKomma - nNumDigits);
/*?*/ 
/*?*/ 		switch(nDiff)
/*?*/ 		{
/*?*/ 			case 1: nVal = (nVal+5)/10; nKomma--; break;
/*?*/ 			case 2: nVal = (nVal+50)/100; nKomma-=2; break;
/*?*/ 			case 3: nVal = (nVal+500)/1000; nKomma-=3; break;
/*?*/ 			case 4: nVal = (nVal+5000)/10000; nKomma-=4; break;
/*?*/ 			case 5: nVal = (nVal+50000)/100000; nKomma-=5; break;
/*?*/ 			case 6: nVal = (nVal+500000)/1000000; nKomma-=6; break;
/*?*/ 			case 7: nVal = (nVal+5000000)/10000000; nKomma-=7; break;
/*?*/ 			default:nVal = (nVal+50000000)/100000000; nKomma-=8; break;
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	rStr = UniString::CreateFromInt32(nVal);
/*?*/ 
/*?*/ 	if(nKomma < 0)
/*?*/ 	{
/*?*/ 		// Negatives Komma bedeutet: Nullen dran
/*?*/ 		sal_Int32 nAnz(-nKomma);
/*?*/ 
/*?*/ 		for(sal_Int32 i=0; i<nAnz; i++)
/*?*/ 			rStr += sal_Unicode('0');
/*?*/ 
/*?*/ 		nKomma = 0;
/*?*/ 	}
/*?*/ 
/*?*/ 	// #83257# the second condition needs to be <= since inside this loop
/*?*/ 	// also the leading zero is inserted.
/*?*/ 	if(nKomma > 0 && rStr.Len() <= nKomma)
/*?*/ 	{
/*?*/ 		// Fuer Komma evtl. vorne Nullen dran
/*?*/ 		sal_Int32 nAnz(nKomma - rStr.Len());
/*?*/ 
/*?*/         if(nAnz >= 0 && rLoc.isNumLeadingZero())
/*?*/ 			nAnz++;
/*?*/ 
/*?*/ 		for(sal_Int32 i=0; i<nAnz; i++)
/*?*/ 			rStr.Insert(sal_Unicode('0'), 0);
/*?*/ 	}
/*?*/ 
/*?*/     sal_Unicode cDec( rLoc.getNumDecimalSep().GetChar(0) );
/*?*/ 
/*?*/ 	// KommaChar einfuegen
/*?*/ 	sal_Int32 nVorKomma(rStr.Len() - nKomma);
/*?*/ 
/*?*/ 	if(nKomma > 0)
/*?*/         rStr.Insert(cDec, (xub_StrLen) nVorKomma);
/*?*/ 
/*?*/     if(!rLoc.isNumTrailingZeros())
/*?*/ 	{
/*?*/ 		while(rStr.Len() && rStr.GetChar(rStr.Len() - 1) == sal_Unicode('0'))
/*?*/ 			rStr.Erase(rStr.Len() - 1);
/*?*/ 
/*?*/ 		if(rStr.Len() && rStr.GetChar(rStr.Len() - 1) == cDec)
/*?*/ 			rStr.Erase(rStr.Len() - 1);
/*?*/ 	}
/*?*/ 
/*?*/ 	// ggf. Trennpunkte bei jedem Tausender einfuegen
/*?*/     if( nVorKomma > 3 )
/*?*/ 	{
/*?*/         String aThoSep( rLoc.getNumThousandSep() );
/*?*/         if ( aThoSep.Len() > 0 )
/*?*/         {
/*?*/             sal_Unicode cTho( aThoSep.GetChar(0) );
/*?*/             sal_Int32 i(nVorKomma - 3);
/*?*/ 
/*?*/             while(i > 0) // #78311#
/*?*/             {
/*?*/                 rStr.Insert(cTho, (xub_StrLen)i);
/*?*/                 i -= 3;
/*?*/             }
/*?*/         }
/*?*/ 	}
/*?*/ 
/*?*/ 	if(!rStr.Len())
/*?*/ 	{
/*?*/ 		rStr = String();
/*?*/ 		rStr += sal_Unicode('0');
/*?*/ 	}
/*?*/ 
/*?*/ 	if(bNeg)
/*?*/ 	{
/*?*/ 		rStr.Insert(sal_Unicode('-'), 0);
/*?*/ 	}
/*?*/ 
/*?*/ 	if(!bNoUnitChars)
/*?*/ 		rStr += aUIUnitStr;
/*?*/ }



/*N*/ void SdrModel::SetChanged(FASTBOOL bFlg)
/*N*/ {
/*N*/ 	bChanged=bFlg;
/*N*/ }

/*N*/ void SdrModel::RecalcPageNums(FASTBOOL bMaster)
/*N*/ {
/*N*/ 	Container& rPL=*(bMaster ? &aMaPag : &aPages);
/*N*/ 	USHORT nAnz=USHORT(rPL.Count());
/*N*/ 	USHORT i;
/*N*/ 	for (i=0; i<nAnz; i++) {
/*N*/ 		SdrPage* pPg=(SdrPage*)(rPL.GetObject(i));
/*N*/ 		pPg->SetPageNum(i);
/*N*/ 	}
/*N*/ 	if (bMaster) bMPgNumsDirty=FALSE;
/*N*/ 	else bPagNumsDirty=FALSE;
/*N*/ }

/*N*/ void SdrModel::InsertPage(SdrPage* pPage, USHORT nPos)
/*N*/ {
/*N*/ 	USHORT nAnz=GetPageCount();
/*N*/ 	if (nPos>nAnz) nPos=nAnz;
/*N*/ 	aPages.Insert(pPage,nPos);
/*N*/ 	pPage->SetInserted(TRUE);
/*N*/ 	pPage->SetPageNum(nPos);
/*N*/ 	pPage->SetModel(this);
/*N*/ 	if (nPos<nAnz) bPagNumsDirty=TRUE;
/*N*/ 	SetChanged();
/*N*/ 	SdrHint aHint(HINT_PAGEORDERCHG);
/*N*/ 	aHint.SetPage(pPage);
/*N*/ 	Broadcast(aHint);
/*N*/ }

/*N*/ void SdrModel::DeletePage(USHORT nPgNum)
/*N*/ {
/*N*/ 	SdrPage* pPg=RemovePage(nPgNum);
/*N*/ 	delete pPg;
/*N*/ }

/*N*/ SdrPage* SdrModel::RemovePage(USHORT nPgNum)
/*N*/ {
/*N*/ 	SdrPage* pPg=(SdrPage*)aPages.Remove(nPgNum);
/*N*/ 	if (pPg!=NULL) {
/*N*/ 		pPg->SetInserted(FALSE);
/*N*/ 	}
/*N*/ 	bPagNumsDirty=TRUE;
/*N*/ 	SetChanged();
/*N*/ 	SdrHint aHint(HINT_PAGEORDERCHG);
/*N*/ 	aHint.SetPage(pPg);
/*N*/ 	Broadcast(aHint);
/*N*/ 	return pPg;
/*N*/ }


/*N*/ void SdrModel::InsertMasterPage(SdrPage* pPage, USHORT nPos)
/*N*/ {
/*N*/ 	USHORT nAnz=GetMasterPageCount();
/*N*/ 	if (nPos>nAnz) nPos=nAnz;
/*N*/ 	aMaPag.Insert(pPage,nPos);
/*N*/ 	pPage->SetInserted(TRUE);
/*N*/ 	pPage->SetPageNum(nPos);
/*N*/ 	pPage->SetModel(this);
/*N*/ 	if (nPos<nAnz) {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	SetChanged();
/*N*/ 	SdrHint aHint(HINT_PAGEORDERCHG);
/*N*/ 	aHint.SetPage(pPage);
/*N*/ 	Broadcast(aHint);
/*N*/ }

/*N*/ void SdrModel::DeleteMasterPage(USHORT nPgNum)
/*N*/ {
/*N*/ 	SdrPage* pPg=RemoveMasterPage(nPgNum);
/*N*/ 	if (pPg!=NULL) delete pPg;
/*N*/ }

/*N*/ SdrPage* SdrModel::RemoveMasterPage(USHORT nPgNum)
/*N*/ {
/*N*/ 	SdrPage* pRetPg=(SdrPage*)aMaPag.Remove(nPgNum);
/*N*/ 	// Nun die Verweise der normalen Zeichenseiten auf die entfernte MasterPage loeschen
/*N*/ 	// und Verweise auf dahinterliegende Masterpages anpassen.
/*N*/ 	USHORT nPageAnz=GetPageCount();
/*N*/ 	for (USHORT np=0; np<nPageAnz; np++) {
/*N*/ 		GetPage(np)->ImpMasterPageRemoved(nPgNum);
/*N*/ 	}
/*N*/ 	if (pRetPg!=NULL) {
/*N*/ 		pRetPg->SetInserted(FALSE);
/*N*/ 	}
/*N*/ 	bMPgNumsDirty=TRUE;
/*N*/ 	SetChanged();
/*N*/ 	SdrHint aHint(HINT_PAGEORDERCHG);
/*N*/ 	aHint.SetPage(pRetPg);
/*N*/ 	Broadcast(aHint);
/*N*/ 	return pRetPg;
/*N*/ }

/*N*/ void SdrModel::MoveMasterPage(USHORT nPgNum, USHORT nNewPos)
/*N*/ {
/*N*/ 	SdrPage* pPg=(SdrPage*)aMaPag.Remove(nPgNum);
/*N*/ 	if (pPg!=NULL) {
/*N*/ 		pPg->SetInserted(FALSE);
/*N*/ 		aMaPag.Insert(pPg,nNewPos);
/*N*/ 		// Anpassen der MasterPageDescriptoren
/*N*/ 		USHORT nPageAnz=GetPageCount();
/*N*/ 		for (USHORT np=0; np<nPageAnz; np++) {
/*N*/ 			GetPage(np)->ImpMasterPageMoved(nPgNum,nNewPos);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	bMPgNumsDirty=TRUE;
/*N*/ 	SetChanged();
/*N*/ 	SdrHint aHint(HINT_PAGEORDERCHG);
/*N*/ 	aHint.SetPage(pPg);
/*N*/ 	Broadcast(aHint);
/*N*/ }

/*N*/ void SdrModel::WriteData(SvStream& rOut) const
/*N*/ {
/*N*/ 	const sal_uInt32 nOldCompressMode = nStreamCompressMode;
/*N*/ 	sal_uInt32 nNewCompressMode = nStreamCompressMode;
/*N*/ 
/*N*/ 	if( SOFFICE_FILEFORMAT_40 <= rOut.GetVersion() )
/*N*/ 	{
/*N*/ 		if( IsSaveCompressed() )
/*N*/ 			nNewCompressMode |= COMPRESSMODE_ZBITMAP;
/*N*/ 
/*N*/ 		if( IsSaveNative() )
/*N*/ 			nNewCompressMode |= COMPRESSMODE_NATIVE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rOut, STREAM_WRITE);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrModel");
/*N*/ #endif
/*N*/ 
/*N*/ 	// damit ich meine eigenen SubRecords erkenne (ab V11)
/*N*/ 	rOut.Write(SdrIOJoeMagic, 4);
/*N*/ 
/*N*/ 	{
/*N*/ 		// Focus fuer aModelMiscCompat
/*N*/ 		// ab V11 eingepackt
/*N*/ 		SdrDownCompat aModelMiscCompat(rOut, STREAM_WRITE);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		aModelMiscCompat.SetID("SdrModel(Miscellaneous)");
/*N*/ #endif
/*N*/ 
/*N*/ 		// ModelInfo muss hier ganz am Anfang stehen!
/*N*/ 		((SdrModel*)this)->aInfo.aLastWriteDate = DateTime();
/*N*/ 		rtl_TextEncoding eOutCharSet = rOut.GetStreamCharSet();
/*N*/ 		if(eOutCharSet == ((rtl_TextEncoding)9) /* == RTL_TEXTENCODING_SYSTEM */ )
/*?*/ 			eOutCharSet = gsl_getSystemTextEncoding();
/*N*/ 
/*N*/ 		// #90477# ((SdrModel*)this)->aInfo.eLastWriteCharSet = GetStoreCharSet(eOutCharSet);
/*N*/ 		((SdrModel*)this)->aInfo.eLastWriteCharSet = GetSOStoreTextEncoding(eOutCharSet, (sal_uInt16)rOut.GetVersion());
/*N*/ 
/*N*/ 		// UNICODE: set the target charset on the stream to access it as parameter
/*N*/ 		// in all streaming operations for UniString->ByteString conversions
/*N*/ 		rOut.SetStreamCharSet(aInfo.eLastWriteCharSet);
/*N*/ 
        /* old SV-stuff, there is no possibility to determine this informations in another way
        ((SdrModel*)this)->aInfo.eLastWriteGUI=System::GetGUIType();
        ((SdrModel*)this)->aInfo.eLastWriteCPU=System::GetCPUType();
        ((SdrModel*)this)->aInfo.eLastWriteSys=System::GetSystemType();
        */
/*N*/ 
/*N*/ 		if(aReadDate.IsValid())
/*N*/ 		{
/*N*/ 			((SdrModel*)this)->aInfo.aLastReadDate = aReadDate;
/*N*/ 
/*N*/ 			// ((SdrModel*)this)->aInfo.eLastReadCharSet = GetStoreCharSet(gsl_getSystemTextEncoding());
/*N*/ 			((SdrModel*)this)->aInfo.eLastReadCharSet = GetSOStoreTextEncoding(gsl_getSystemTextEncoding(), (sal_uInt16)rOut.GetVersion());
/*N*/ 
            /* old SV-stuff, there is no possibility to determine this informations in another way
            ((SdrModel*)this)->aInfo.eLastReadGUI=System::GetGUIType();
            ((SdrModel*)this)->aInfo.eLastReadCPU=System::GetCPUType();
            ((SdrModel*)this)->aInfo.eLastReadSys=System::GetSystemType();
            */
/*N*/ 		}
/*N*/ 		rOut << aInfo; // DateiInfo rausschreiben (ab V11)
/*N*/ 
/*N*/ 		{ // ein Bereich fuer Statistik reservieren (V11) (kommt spaeter vielleicht mal dazu)
/*N*/ 			SdrDownCompat aModelStatisticCompat(rOut, STREAM_WRITE);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aModelStatisticCompat.SetID("SdrModel(Statistic)");
/*N*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		{
/*N*/ 			// ab V11
/*N*/ 			SdrDownCompat aModelFormatCompat(rOut, STREAM_WRITE);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aModelFormatCompat.SetID("SdrModel(Format)");
/*N*/ #endif
/*N*/ 
/*N*/ 			// ab V11
/*N*/ 			rOut << nNewCompressMode;
/*N*/ 
/*N*/ 			// ab V11
/*N*/ 			rOut << UINT16(rOut.GetNumberFormatInt());
/*N*/ 
/*N*/ 			rOut.SetCompressMode( (sal_uInt16)nNewCompressMode);
/*N*/ 			// CompressMode erst an dieser Stelle setzen, damit konform zu ReadData()
/*N*/ 		}
/*N*/ 
/*N*/ 		rOut << INT32(aObjUnit.GetNumerator());
/*N*/ 		rOut << INT32(aObjUnit.GetDenominator());
/*N*/ 		rOut << UINT16(eObjUnit);
/*N*/ 		// Komprimiert ?
/*N*/ 		rOut << UINT16(0);
/*N*/ 		// Nur eine DummyPage, jedoch mit relevanten Objekten?
/*N*/ 		rOut << UINT8(bPageNotValid);
/*N*/ 		// Reserve DummyByte
/*N*/ 		rOut << UINT8(0);
/*N*/ 
/*N*/ 		// Tabellen-, Listen- und Palettennamen schreiben
/*N*/ 		// rOut<<INT16(::GetSystemCharSet()); seit V11 hier kein CharSet mehr
/*N*/ 		XubString aEmptyStr;
/*N*/ 
/*N*/ 		if(bExtColorTable)
/*N*/ 		{
/*N*/ 			// der Writer hat seinen eigenen ColorTable
/*N*/ 			// UNICODE: rOut << aEmptyStr;
/*N*/ 			rOut.WriteByteString(aEmptyStr);
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if(pColorTable && !pColorTable->GetName().Equals(pszStandard))
/*N*/ 			{
/*N*/ 				// UNICODE: rOut << pColorTable->GetName();
/*N*/ 				rOut.WriteByteString(pColorTable->GetName());
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// UNICODE: rOut << aEmptyStr;
/*N*/ 				rOut.WriteByteString(aEmptyStr);
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pDashList && !pDashList->GetName().Equals(pszStandard))
/*N*/ 		{
/*N*/ 			// UNICODE: rOut<<pDashList->GetName();
/*N*/ 			rOut.WriteByteString(pDashList->GetName());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// UNICODE: rOut << aEmptyStr;
/*N*/ 			rOut.WriteByteString(aEmptyStr);
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pLineEndList && !pLineEndList->GetName().Equals(pszStandard))
/*N*/ 		{
/*N*/ 			// UNICODE: rOut<<pLineEndList->GetName();
/*N*/ 			rOut.WriteByteString(pLineEndList->GetName());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// UNICODE: rOut << aEmptyStr;
/*N*/ 			rOut.WriteByteString(aEmptyStr);
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pHatchList && !pHatchList->GetName().Equals(pszStandard))
/*N*/ 		{
/*N*/ 			// UNICODE: rOut<<pHatchList->GetName();
/*N*/ 			rOut.WriteByteString(pHatchList->GetName());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// UNICODE: rOut << aEmptyStr;
/*N*/ 			rOut.WriteByteString(aEmptyStr);
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pGradientList && !pGradientList->GetName().Equals(pszStandard))
/*N*/ 		{
/*N*/ 			// UNICODE: rOut<<pGradientList->GetName();
/*N*/ 			rOut.WriteByteString(pGradientList->GetName());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// UNICODE: rOut << aEmptyStr;
/*N*/ 			rOut.WriteByteString(aEmptyStr);
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pBitmapList && !pBitmapList->GetName().Equals(pszStandard))
/*N*/ 		{
/*N*/ 			// UNICODE: rOut<<pBitmapList->GetName();
/*N*/ 			rOut.WriteByteString(pBitmapList->GetName());
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// UNICODE: rOut << aEmptyStr;
/*N*/ 			rOut.WriteByteString(aEmptyStr);
/*N*/ 		}
/*N*/ 
/*N*/ 		// ab 09-02-1996
/*N*/ 		rOut << INT32(aUIScale.GetNumerator());
/*N*/ 		rOut << INT32(aUIScale.GetDenominator());
/*N*/ 		rOut << UINT16(eUIUnit);
/*N*/ 
/*N*/ 		// ab 09-04-1997 fuer #37710#
/*N*/ 		rOut << INT32(nDefTextHgt);
/*N*/ 		rOut << INT32(nDefaultTabulator);
/*N*/ 
/*N*/ 		// StarDraw-Preview: Nummer der MasterPage der ersten Standard-Seite
/*N*/ 		if(GetPageCount() >= 3 && GetPage(1)->GetMasterPageCount())
/*N*/ 		{
/*N*/ 			((SdrModel*)this)->nStarDrawPreviewMasterPageNum =
/*N*/ 				GetPage(1)->GetMasterPageNum(0);
/*N*/ 		}
/*N*/ 		rOut << nStarDrawPreviewMasterPageNum;
/*N*/ 	}
/*N*/ 
/*N*/ 	UINT16 i;
/*N*/ 
/*N*/ 	for(i=0; i < GetLayerAdmin().GetLayerCount(); i++)
/*N*/ 	{
/*N*/ 		rOut << *GetLayerAdmin().GetLayer(i);
/*N*/ 	}
/*N*/ 
/*N*/ 	for(i=0; i < GetLayerAdmin().GetLayerSetCount(); i++)
/*N*/ 	{
/*?*/ 		rOut << *GetLayerAdmin().GetLayerSet(i);
/*N*/ 	}
/*N*/ 
/*N*/ 	for(i=0; i < GetMasterPageCount(); i++)
/*N*/ 	{
/*N*/ 		const SdrPage* pPg = GetMasterPage(i);
/*N*/ 		rOut << *pPg;
/*N*/ 	}
/*N*/ 
/*N*/ 	for(i=0; i < GetPageCount(); i++)
/*N*/ 	{
/*N*/ 		const SdrPage* pPg = GetPage(i);
/*N*/ 		rOut << *pPg;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Endemarke
/*N*/ 	SdrIOHeader(rOut, STREAM_WRITE, SdrIOEndeID);
/*N*/ }

/*N*/ void SdrModel::ReadData(const SdrIOHeader& rHead, SvStream& rIn)
/*N*/ {
/*N*/ 	if(rIn.GetError())
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
/*N*/ 	SdrDownCompat aCompat(rIn, STREAM_READ);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	aCompat.SetID("SdrModel");
/*N*/ #endif
/*N*/ 
/*N*/ 	if(rHead.GetVersion() >= 11)
/*N*/ 	{
/*N*/ 		// damit ich meine eigenen SubRecords erkenne (ab V11)
/*N*/ 		char cMagic[4];
/*N*/ 		if(rIn.Read(cMagic, 4) != 4 || memcmp(cMagic, SdrIOJoeMagic, 4))
/*N*/ 		{
/*N*/ 			rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
/*N*/ 			return;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	DoProgress(rIn.Tell());
/*N*/ 
/*N*/ 	{
/*N*/ 		// Focus fuer aModelMiscCompat
/*N*/ 		SdrDownCompat* pModelMiscCompat = NULL;
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 11)
/*N*/ 		{
/*N*/ 			// MiscellaneousData ab V11 eingepackt
/*N*/ 			// MiscellaneousData ist alles von Recordbeginn bis
/*N*/ 			// zum Anfang der Pages, Layer, ...
/*N*/ 			pModelMiscCompat = new SdrDownCompat(rIn, STREAM_READ);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			pModelMiscCompat->SetID("SdrModel(Miscellaneous)");
/*N*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 11)
/*N*/ 		{
/*N*/ 			// ModelInfo ab V11
/*N*/ 			// DateiInfo lesen
/*N*/ 			rIn >> aInfo;
/*N*/ 
/*N*/ 			// StreamCharSet setzen, damit Strings beim
/*N*/ 			// Lesen automatisch konvertiert werden
/*N*/ 			rIn.SetStreamCharSet(aInfo.eLastWriteCharSet);
/*N*/ 		}
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 11)
/*N*/ 		{
/*N*/ 			// reserviert fuer Statistik
/*N*/ 			SdrDownCompat aModelStatisticCompat(rIn, STREAM_READ);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aModelStatisticCompat.SetID("SdrModel(Statistik)");
/*N*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 11)
/*N*/ 		{
/*N*/ 			// Info ueber Dateiformat
/*N*/ 			SdrDownCompat aModelFormatCompat(rIn,STREAM_READ);
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 			aModelFormatCompat.SetID("SdrModel(Format)");
/*N*/ #endif
/*N*/ 
/*N*/ 			if(aModelFormatCompat.GetBytesLeft() >= 4)
/*N*/ 			{
/*N*/ 				rIn >> nStreamCompressMode;
/*N*/ 				rIn >> nStreamNumberFormat;
/*N*/ 				rIn.SetCompressMode(nStreamCompressMode);
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		INT32 nNum,nDen;
/*N*/ 		UINT16 nTmp;
/*N*/ 		UINT8  nTmp8;
/*N*/ 
/*N*/ 		rIn >> nNum;
/*N*/ 		rIn >> nDen;
/*N*/ 
/*N*/ 		aObjUnit = Fraction(nNum,nDen);
/*N*/ 
/*N*/ 		rIn >> nTmp;
/*N*/ 
/*N*/ 		eObjUnit = MapUnit(nTmp);
/*N*/ 
/*N*/ 		// Komprimiert ?
/*N*/ 		rIn >> nTmp;
/*N*/ 
/*N*/ 		//rIn.nJoeDummy=(nTmp==1);
/*N*/ 		rIn >> nTmp8;
/*N*/ 
/*N*/ 		bPageNotValid = (nTmp == 1);
/*N*/ 
/*N*/ 		rIn >> nTmp8; // Reserve DummyByte
/*N*/ 
/*N*/ 		BOOL bExtFiles(rHead.GetVersion() >= 1);
/*N*/ 
/*N*/ 		if(bExtFiles)
/*N*/ 		{
/*N*/ 			// Tabellen-, Listen- und Palettennamen lesen
/*N*/ 			XubString aName;
/*N*/ 
/*N*/ 			if(rHead.GetVersion() < 11)
/*N*/ 			{
/*N*/ 				// vor V11 gab's noch keine ModelInfo, deshalb CharSet von hier
/*N*/ 				// und rein zufaellig gab's genau bis inkl. zur V10
/*N*/ 				// an dieser Stelle einen CharSet
/*N*/ 				INT16 nCharSet;
/*N*/ 
/*N*/ 				// #90477# rIn >> nCharSet;
/*N*/ 				rIn >> nCharSet;
/*N*/ 				nCharSet = (INT16)GetSOLoadTextEncoding((rtl_TextEncoding)nCharSet, (sal_uInt16)rIn.GetVersion());
/*N*/ 
/*N*/ 				// StreamCharSet setzen, damit Strings beim
/*N*/ 				// Lesen automatisch konvertiert werden
/*N*/ 				// #90477# rIn.SetStreamCharSet(rtl_TextEncoding(nCharSet));
/*N*/ 				rIn.SetStreamCharSet(GetSOLoadTextEncoding(rtl_TextEncoding(nCharSet), (sal_uInt16)rIn.GetVersion()));
/*N*/ 			}
/*N*/ 
/*N*/ 			// Tabellen- und Listennamen lesen (Tabellen/Listen existieren schon) // SOH!!!
/*N*/ 			// UNICODE: rIn >> aName;
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 
/*N*/ 			if(!bExtColorTable)
/*N*/ 			{
/*N*/ 				// der Writer hat seinen eigenen ColorTable
/*N*/ 				if(!aName.Len())
/*N*/ 					aName = pszStandard;
/*N*/ 
/*N*/ 				if(pColorTable)
/*N*/ 					pColorTable->SetName(aName);
/*N*/ 			}
/*N*/ 
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			if(!aName.Len())
/*N*/ 				aName = pszStandard;
/*N*/ 			if(pDashList)
/*N*/ 				pDashList->SetName(aName);
/*N*/ 
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			if(!aName.Len())
/*N*/ 				aName = pszStandard;
/*N*/ 			if(pLineEndList)
/*N*/ 				pLineEndList->SetName(aName);
/*N*/ 
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			if(!aName.Len())
/*N*/ 				aName = pszStandard;
/*N*/ 			if(pHatchList)
/*N*/ 				pHatchList->SetName(aName);
/*N*/ 
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			if(!aName.Len())
/*N*/ 				aName = pszStandard;
/*N*/ 			if(pGradientList)
/*N*/ 				pGradientList->SetName(aName);
/*N*/ 
/*N*/ 			rIn.ReadByteString(aName);
/*N*/ 			if(!aName.Len())
/*N*/ 				aName = pszStandard;
/*N*/ 			if(pBitmapList)
/*N*/ 				pBitmapList->SetName(aName);
/*N*/ 
/*N*/ 			// Wenn gewuenscht kann hier SetDirty() an den Tabellen gesetzt werden, ist m.M. nach aber ueberfluessig ! SOH.
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// Ansonsten altes Format: Listen und Tables sind embedded
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*?*/ 			ByteString aMsg("Das Format dieser Datei ist noch von April '95 (Version ");
/*?*/ 			aMsg += ByteString::CreateFromInt32( rHead.GetVersion() );
/*?*/ 			aMsg += "). Mit dieser Programmversion kann das nicht mehr gelesen werden";
/*?*/ 
/*?*/ 			DBG_ERROR(aMsg.GetBuffer());
/*?*/ #endif
/*?*/ 
/*?*/ 			// Version zu alt
/*?*/ 			rIn.SetError(SVSTREAM_WRONGVERSION);
/*?*/ 
/*?*/ 			return;
/*N*/ 		}
/*N*/ 
/*N*/ 		// UIUnit wird ab V12 gestreamt
/*N*/ 		if(rHead.GetVersion() >= 12 && pModelMiscCompat->GetBytesLeft() > 0)
/*N*/ 		{
/*N*/ 			rIn >> nNum;
/*N*/ 			rIn >> nDen;
/*N*/ 
/*N*/ 			aUIScale = Fraction(nNum, nDen);
/*N*/ 
/*N*/ 			rIn >> nTmp;
/*N*/ 
/*N*/ 			eUIUnit = FieldUnit(nTmp);
/*N*/ 		}
/*N*/ 
/*N*/ 		// ab 09-04-1997 fuer #37710#: Text in Dafaultgroesse vom Writer ins Draw und umgekehrt
/*N*/ 		if(rHead.GetVersion() >= 13 && pModelMiscCompat->GetBytesLeft() > 0)
/*N*/ 		{
/*N*/ 			rIn >> nNum;
/*N*/ 			nDefTextHgt = nNum;
/*N*/ 
/*N*/ 			rIn >> nNum;
/*N*/ 			nDefaultTabulator = (UINT16)nNum;
/*N*/ 
/*N*/ 			Outliner& rOutliner = GetDrawOutliner();
/*N*/ 			rOutliner.SetDefTab(nDefaultTabulator);
/*N*/ 		}
/*N*/ 
/*N*/ 		if(rHead.GetVersion() >= 14 && pModelMiscCompat->GetBytesLeft() > 0)
/*N*/ 		{
/*N*/ 			// StarDraw-Preview: Nummer der MasterPage der ersten Standard-Seite
/*N*/ 			rIn >> nStarDrawPreviewMasterPageNum;
/*N*/ 		}
/*N*/ 
/*N*/ 		if(pModelMiscCompat)
/*N*/ 		{
/*N*/ 			delete pModelMiscCompat;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	DoProgress(rIn.Tell());
/*N*/ 	//SdrIOHeader aHead;
/*N*/ 
/*N*/ 	// Seiten, Layer und LayerSets einlesen
/*N*/ 	BOOL bEnde(FALSE);
/*N*/ 	UINT16 nMasterPageNum(0);
/*N*/ 	BOOL bAllPagesLoaded(TRUE);
/*N*/ 
/*N*/ 	while(!rIn.GetError() && !rIn.IsEof() && !bEnde)
/*N*/ 	{
/*N*/ 		SdrIOHeaderLookAhead aHead(rIn);
/*N*/ 		//ULONG nPos0=rIn.Tell();
/*N*/ 		//rIn>>aHead;
/*N*/ 
/*N*/ 		if(!aHead.IsMagic())
/*N*/ 		{
/*N*/ 			// Format-Fehler
/*N*/ 			rIn.SetError(SVSTREAM_FILEFORMAT_ERROR);
/*N*/ 			return;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if(!aHead.IsEnde())
/*N*/ 			{
/*N*/ 				//rIn.Seek(nPos0); // Die Headers wollen alle selbst lesen
/*N*/ 				if(aHead.IsID(SdrIOPageID))
/*N*/ 				{
/*N*/ 					if(!bStarDrawPreviewMode || GetPageCount() < 3)
/*N*/ 					{
/*N*/ 						// Page lesen
/*N*/ 						SdrPage* pPg = AllocPage(FALSE);
/*N*/ 
/*N*/ 						rIn >> *pPg;
/*N*/ 						InsertPage(pPg);
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bAllPagesLoaded = FALSE;
/*N*/ 						aHead.SkipRecord();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if(aHead.IsID(SdrIOMaPgID))
/*N*/ 				{
/*N*/ 					if(!bStarDrawPreviewMode
/*N*/ 						|| nStarDrawPreviewMasterPageNum == SDRPAGE_NOTFOUND
/*N*/ 						|| nMasterPageNum == 0
/*N*/ 						|| nMasterPageNum <= nStarDrawPreviewMasterPageNum
/*N*/ 						|| nMasterPageNum <= nStarDrawPreviewMasterPageNum + 1)
/*N*/ 					{
/*N*/ 						// Im StarDrawPreviewMode Standard und Notizseite lesen!
/*N*/ 						// MasterPage lesen
/*N*/ 						SdrPage* pPg = AllocPage(TRUE);
/*N*/ 
/*N*/ 						rIn >> *pPg;
/*N*/ 						InsertMasterPage(pPg);
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						bAllPagesLoaded = FALSE;
/*N*/ 						aHead.SkipRecord();
/*N*/ 					}
/*N*/ 
/*N*/ 					nMasterPageNum++;
/*N*/ 				}
/*N*/ 				else if(aHead.IsID(SdrIOLayrID))
/*N*/ 				{
/*N*/ 					//SdrLayer* pLay=GetLayer().NewLayer("");
/*N*/ 					// Layerdefinition lesen
/*N*/ 					SdrLayer* pLay = new SdrLayer;
/*N*/ 
/*N*/ 					rIn >> *pLay;
/*N*/ 					GetLayerAdmin().InsertLayer(pLay);
/*N*/ 				}
/*N*/ 				else if(aHead.IsID(SdrIOLSetID))
/*N*/ 				{
/*N*/ 					//SdrLayerSet* pSet=GetLayer().NewLayerSet("");
/*N*/ 					SdrLayerSet* pSet = new SdrLayerSet; // Layersetdefinition lesen
/*N*/ 
/*N*/ 					rIn >> *pSet;
/*N*/ 					GetLayerAdmin().InsertLayerSet(pSet);
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					// aha, das wil keiner. Also ueberlesen.
/*N*/ 					aHead.SkipRecord();
/*N*/ 					//rIn.Seek(nPos0+aHead.nBlkSize);
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				bEnde = TRUE;
/*N*/ 
/*N*/ 				// Endemarke weglesen
/*N*/ 				aHead.SkipRecord();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		DoProgress(rIn.Tell());
/*N*/ 	}
/*N*/ 
/*N*/ 	if(bStarDrawPreviewMode && bAllPagesLoaded)
/*N*/ 	{
/*N*/ 		// Obwohl StarDrawPreviewMode wurden doch alle Seiten geladen,
/*N*/ 		// um dieses kenntlich zu machen, wird das Flag zurueckgesetzt
/*N*/ 		bStarDrawPreviewMode = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::AfterRead()
/*N*/ {
/*N*/ 	// alle MasterPages und alle Pages durchlaufen
/*N*/ 	UINT16 nCnt(GetMasterPageCount());
/*N*/ 	UINT16 i;
/*N*/ 
/*N*/ 	for(i=0; i < nCnt; i++)
/*N*/ 	{
/*N*/ 		GetMasterPage(i)->AfterRead();
/*N*/ 	}
/*N*/ 
/*N*/ 	nCnt = GetPageCount();
/*N*/ 
/*N*/ 	for(i=0; i < nCnt; i++)
/*N*/ 	{
/*N*/ 		GetPage(i)->AfterRead();
/*N*/ 	}
/*N*/ 
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	// Investigation of bMyPool to check if it's allowed to delete the OLE objects.
/*N*/ 	// If bMyPool == FALSE it's not allowed (Writer)
/*N*/ 	if( pPersist && bMyPool )
/*N*/ 	{
/*N*/ 		SvInfoObjectMemberList* pList = (SvInfoObjectMemberList*) pPersist->GetObjectList();
/*N*/ 
/*N*/ 		if( pList )
/*N*/ 		{
/*N*/ 			SvInfoObjectRef pInfo = pList->First();
/*N*/ 			while( pInfo.Is() )
/*N*/ 			{
/*N*/ 				BOOL bFound = FALSE;
/*N*/ 				String aName = pInfo->GetObjName();
/*N*/ 
/*N*/ 				nCnt = GetPageCount();
/*N*/ 				UINT16 a; for( a = 0; a < nCnt && !bFound; a++ )
/*N*/ 				{
/*N*/ 					// Pages
/*N*/ 					SdrObjListIter aIter( *GetPage(a) );
/*N*/ 					while( !bFound && aIter.IsMore() )
/*N*/ 					{
/*N*/ 						SdrObject* pObj = aIter.Next();
/*N*/ 						if( pObj->ISA(SdrOle2Obj) )
/*N*/ 						{
/*N*/ 							if( aName == static_cast< SdrOle2Obj* >( pObj )->GetPersistName() )
/*N*/ 								bFound = TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				nCnt = GetMasterPageCount();
/*N*/ 				for( a = 0; a < nCnt && !bFound; a++ )
/*N*/ 				{
/*N*/ 					// MasterPages
/*N*/ 					SdrObjListIter aIter( *GetMasterPage(a) );
/*N*/ 					while( !bFound && aIter.IsMore() )
/*N*/ 					{
/*N*/ 						SdrObject* pObj = aIter.Next();
/*N*/ 						if( pObj->ISA(SdrOle2Obj) )
/*N*/ 						{
/*N*/ 							if( aName == static_cast< SdrOle2Obj* >( pObj )->GetPersistName() )
/*N*/ 								bFound = TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 
/*N*/ 				if( !bFound )
/*N*/ 					pInfo->SetDeleted(TRUE);
/*N*/ 
/*N*/ 				pInfo = pList->Next();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ }

/*N*/ ULONG SdrModel::ImpCountAllSteamComponents() const
/*N*/ {
/*N*/ 	UINT32 nCnt(0);
/*N*/ 	UINT16 nAnz(GetMasterPageCount());
/*N*/ 	UINT16 nNum;
/*N*/ 
/*N*/ 	for(nNum = 0; nNum < nAnz; nNum++)
/*N*/ 	{
/*N*/ 		nCnt += GetMasterPage(nNum)->CountAllObjects();
/*N*/ 	}
/*N*/ 
/*N*/ 	nAnz = GetPageCount();
/*N*/ 
/*N*/ 	for(nNum = 0; nNum < nAnz; nNum++)
/*N*/ 	{
/*N*/ 		nCnt += GetPage(nNum)->CountAllObjects();
/*N*/ 	}
/*N*/ 
/*N*/ 	return nCnt;
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrModel& rMod)
/*N*/ {
/*N*/ 	((SdrModel*)&rMod)->nProgressOfs=0;
/*N*/ 	((SdrModel*)&rMod)->nProgressMax=rMod.ImpCountAllSteamComponents(); // Hier passenden Wert einsetzen
/*N*/ 	((SdrModel*)&rMod)->DoProgress(0);
/*N*/ 	ULONG nPos0=rOut.Tell();
/*N*/ 	SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOModlID);
/*N*/ 	USHORT nCompressMerk=rOut.GetCompressMode(); // Der CompressMode wird von SdrModel::ReadData() gesetzt
/*N*/ 	rMod.WriteData(rOut);
/*N*/ 	rOut.SetCompressMode(nCompressMerk); // CompressMode wieder restaurieren
/*N*/ 	((SdrModel*)&rMod)->DoProgress(0xFFFFFFFF);
/*N*/ 	((SdrModel*)&rMod)->Broadcast(SdrHint(HINT_MODELSAVED)); // #43095#
/*N*/ 	return rOut;
/*N*/ }

/*N*/ SvStream& operator>>(SvStream& rIn, SdrModel& rMod)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return rIn;
/*N*/ 	rMod.aReadDate=DateTime(); // Zeitpunkt des Lesens merken
/*N*/ 	rMod.nProgressOfs=rIn.Tell();
/*N*/ 	rMod.nProgressMax=0xFFFFFFFF; // Vorlaeufiger Wert
/*N*/ 	rMod.DoProgress(0);
/*N*/ 	rMod.Clear();
/*N*/ 	SdrIOHeader aHead(rIn,STREAM_READ);
/*N*/ 	rMod.nLoadVersion=aHead.GetVersion();
/*N*/ 	if (!aHead.IsMagic()) {
/*?*/ 		rIn.SetError(SVSTREAM_FILEFORMAT_ERROR); // Format-Fehler
/*?*/ 		return rIn;
/*N*/ 	}
/*N*/ 	if (aHead.GetMajorVersion()>nAktSdrFileMajorVersion) {
/*?*/ 		rIn.SetError(SVSTREAM_WRONGVERSION); // Datei zu neu / Programm zu alt
/*?*/ 		return rIn;
/*N*/ 	}
/*N*/ 	rMod.nProgressMax=aHead.GetBlockSize();
/*N*/ 	rMod.DoProgress(rIn.Tell());
/*N*/ 	rMod.bLoading=TRUE;
/*N*/ 	rtl_TextEncoding eStreamCharSetMerker=rIn.GetStreamCharSet(); // Der StreamCharSet wird von SdrModel::ReadData() gesetzt
/*N*/ 	USHORT nCompressMerk=rIn.GetCompressMode(); // Der CompressMode wird von SdrModel::ReadData() gesetzt
/*N*/ 	rMod.ReadData(aHead,rIn);
/*N*/ 	rIn.SetCompressMode(nCompressMerk); // CompressMode wieder restaurieren
/*N*/ 
/*N*/ 	rIn.SetStreamCharSet(eStreamCharSetMerker); // StreamCharSet wieder restaurieren
/*N*/ 
/*N*/ 	rMod.bLoading=FALSE;
/*N*/ 	rMod.DoProgress(rIn.Tell());
/*N*/ 	rMod.AfterRead();
/*N*/ 	rMod.DisposeLoadedModels();
/*N*/ 
/*N*/ 	rMod.ImpSetUIUnit(); // weil ggf. neues Scaling eingelesen
/*N*/ 	rMod.DoProgress(0xFFFFFFFF);
/*N*/ 	return rIn;
/*N*/ }

/*N*/ void SdrModel::PreSave()
/*N*/ {
/*N*/ 	sal_uInt16 nCnt(GetMasterPageCount());
/*N*/ 
/*N*/ 	sal_uInt16 a; for(a=0; a < nCnt; a++)
/*N*/ 	{
/*N*/ 		// MasterPages
/*N*/ 		const SdrPage& rPage = *GetMasterPage(a);
/*N*/ 		SdrObject* pObj = rPage.GetBackgroundObj();
/*N*/ 		if( pObj )
/*?*/ 			pObj->PreSave();
/*N*/ 
/*N*/ 		for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
/*N*/ 			rPage.GetObj(b)->PreSave();
/*N*/ 	}
/*N*/ 
/*N*/ 	nCnt = GetPageCount();
/*N*/ 	for(a = 0; a < nCnt; a++)
/*N*/ 	{
/*N*/ 		// Pages
/*N*/ 		const SdrPage& rPage = *GetPage(a);
/*N*/ 		SdrObject* pObj = rPage.GetBackgroundObj();
/*N*/ 		if( pObj )
/*N*/ 			pObj->PreSave();
/*N*/ 
/*N*/ 		for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
/*N*/ 			rPage.GetObj(b)->PreSave();
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::PostSave()
/*N*/ {
/*N*/ 	sal_uInt16 nCnt(GetMasterPageCount());
/*N*/ 
/*N*/ 	sal_uInt16 a; for(a=0; a < nCnt; a++)
/*N*/ 	{
/*N*/ 		// MasterPages
/*N*/ 		const SdrPage& rPage = *GetMasterPage(a);
/*N*/ 		SdrObject* pObj = rPage.GetBackgroundObj();
/*N*/ 		if( pObj )
/*?*/ 			pObj->PostSave();
/*N*/ 
/*N*/ 		for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
/*N*/ 			rPage.GetObj(b)->PostSave();
/*N*/ 	}
/*N*/ 
/*N*/ 	nCnt = GetPageCount();
/*N*/ 	for(a = 0; a < nCnt; a++)
/*N*/ 	{
/*N*/ 		// Pages
/*N*/ 		const SdrPage& rPage = *GetPage(a);
/*N*/ 		SdrObject* pObj = rPage.GetBackgroundObj();
/*N*/ 		if( pObj )
/*N*/ 			pObj->PostSave();
/*N*/ 
/*N*/ 		for(sal_uInt32 b(0); b < rPage.GetObjCount(); b++)
/*N*/ 			rPage.GetObj(b)->PostSave();
/*N*/ 	}
/*N*/ }

/*N*/ uno::Reference< uno::XInterface > SdrModel::getUnoModel()
/*N*/ {
/*N*/ 	// try weak reference first
/*N*/ 	uno::Reference< uno::XInterface > xModel( mxUnoModel );
/*N*/ 
/*N*/ 	if( !xModel.is() )
/*N*/ 	{
/*N*/ 		// create one
/*N*/ 		xModel = createUnoModel();
/*N*/ 
/*N*/ 		mxUnoModel = xModel;
/*N*/ 	}
/*N*/ 
/*N*/ 	return xModel;
/*N*/ 
/*N*/ }

/*?*/ uno::Reference< uno::XInterface > SdrModel::createUnoModel()
/*?*/ {
/*?*/ 	DBG_ERROR( "SdrModel::createUnoModel() - base implementation should not be called!" );
/*?*/ 	::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt;
/*?*/ 	return xInt;
/*?*/ }

/*N*/ void SdrModel::setLock( BOOL bLock )
/*N*/ {
/*N*/ 	if( mbModelLocked != bLock )
/*N*/ 	{
/*N*/ 		mbModelLocked = bLock;
/*N*/ 		if( sal_False == bLock )
/*N*/ 		{
/*N*/ 			// ReformatAllTextObjects(); #103122# due to a typo in the above if, this code was never
/*N*/ 			//							 executed, so I remove it until we discover that we need it here
/*N*/ 			ImpReformatAllEdgeObjects();	// #103122#
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrModel::MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModel )
/*N*/ {
/*N*/ 	if( pSourceSet && pDestSet && (pSourceSet != pDestSet ) )
/*N*/ 	{
/*N*/ 		if( pNewModel == NULL )
/*N*/ 			pNewModel = this;
/*N*/ 
/*N*/ 		SfxWhichIter aWhichIter(*pSourceSet);
/*N*/ 		sal_uInt16 nWhich(aWhichIter.FirstWhich());
/*N*/ 		const SfxPoolItem *pPoolItem;
/*N*/ 
/*N*/ 		while(nWhich)
/*N*/ 		{
/*N*/ 			if(SFX_ITEM_SET == pSourceSet->GetItemState(nWhich, FALSE, &pPoolItem))
/*N*/ 			{
/*N*/ 				const SfxPoolItem* pItem = pPoolItem;
/*N*/ 
/*N*/ 				switch( nWhich )
/*N*/ 				{
/*N*/ 				case XATTR_FILLBITMAP:
/*N*/ 					pItem = ((XFillBitmapItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				case XATTR_LINEDASH:
/*N*/ 					pItem = ((XLineDashItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				case XATTR_LINESTART:
/*N*/ 					pItem = ((XLineStartItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				case XATTR_LINEEND:
/*N*/ 					pItem = ((XLineEndItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				case XATTR_FILLGRADIENT:
/*N*/ 					pItem = ((XFillGradientItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				case XATTR_FILLFLOATTRANSPARENCE:
/*N*/ 					// #85953# allow all kinds of XFillFloatTransparenceItem to be set
/*N*/ 					pItem = ((XFillFloatTransparenceItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				case XATTR_FILLHATCH:
/*N*/ 					pItem = ((XFillHatchItem*)pItem)->checkForUniqueItem( pNewModel );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 
/*N*/ 				// set item
/*N*/ 				if( pItem )
/*N*/ 				{
/*N*/ 					pDestSet->Put(*pItem);
/*N*/ 
/*N*/ 					// delete item if it was a generated one
/*N*/ 					if( pItem != pPoolItem)
/*N*/ 						delete (SfxPoolItem*)pItem;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nWhich = aWhichIter.NextWhich();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrModel::SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars )
/*N*/ {
/*N*/ 	if( mpForbiddenCharactersTable )
/*N*/ 		mpForbiddenCharactersTable->release();
/*N*/ 
/*N*/ 	mpForbiddenCharactersTable = xForbiddenChars.getBodyPtr();
/*N*/ 
/*N*/ 	if( mpForbiddenCharactersTable )
/*N*/ 		mpForbiddenCharactersTable->acquire();
/*N*/ 
/*N*/ 	ImpSetOutlinerDefaults( pDrawOutliner );
/*N*/ 	ImpSetOutlinerDefaults( pHitTestOutliner );
/*N*/ }

/*N*/ vos::ORef<SvxForbiddenCharactersTable> SdrModel::GetForbiddenCharsTable() const
/*N*/ {
/*N*/ 	return mpForbiddenCharactersTable;
/*N*/ }

/*N*/ void SdrModel::SetCharCompressType( UINT16 nType )
/*N*/ {
/*N*/ 	if( nType != mnCharCompressType )
/*N*/ 	{
/*?*/ 		mnCharCompressType = nType;
/*?*/ 		ImpSetOutlinerDefaults( pDrawOutliner );
/*?*/ 		ImpSetOutlinerDefaults( pHitTestOutliner );
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::SetKernAsianPunctuation( sal_Bool bEnabled )
/*N*/ {
/*N*/ 	if( mbKernAsianPunctuation != bEnabled )
/*N*/ 	{
/*?*/ 		mbKernAsianPunctuation = bEnabled;
/*?*/ 		ImpSetOutlinerDefaults( pDrawOutliner );
/*?*/ 		ImpSetOutlinerDefaults( pHitTestOutliner );
/*N*/ 	}
/*N*/ }

/*N*/ void SdrModel::ReformatAllTextObjects()
/*N*/ {
/*N*/ 	ImpReformatAllTextObjects();
/*N*/ }


/*N*/ SdrOutliner* SdrModel::createOutliner( USHORT nOutlinerMode )
/*N*/ {
/*N*/ 	if( NULL == mpOutlinerCache )
/*N*/ 		mpOutlinerCache = new SdrOutlinerCache(this);
/*N*/ 
/*N*/ 	return mpOutlinerCache->createOutliner( nOutlinerMode );
/*N*/ }

/*N*/ void SdrModel::disposeOutliner( SdrOutliner* pOutliner )
/*N*/ {
/*N*/ 	if( mpOutlinerCache )
/*N*/ 	{
/*N*/ 		mpOutlinerCache->disposeOutliner( pOutliner );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		delete pOutliner;
/*N*/ 	}
/*N*/ }

/*?*/ SvxNumType SdrModel::GetPageNumType() const
/*?*/ {
/*?*/ 	return SVX_ARABIC;
/*?*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1(SdrHint,SfxHint);

/*N*/ SdrHint::SdrHint(const SdrPage& rNewPage)
/*N*/ {
/*N*/ 	aRect=Rectangle(0,0,rNewPage.GetWdt(),rNewPage.GetHgt());
/*N*/ 	pPage=&rNewPage;
/*N*/ 	pObj=NULL;
/*N*/ 	pObjList=&rNewPage;
/*N*/ 	bNeedRepaint=TRUE;
/*N*/ 	eHint=HINT_PAGECHG;
/*N*/ }

/*N*/ SdrHint::SdrHint(const SdrObject& rNewObj)
/*N*/ {
/*N*/ 	aRect=rNewObj.GetBoundRect();
/*N*/ 	pPage=rNewObj.GetPage();
/*N*/ 	pObj=&rNewObj;
/*N*/ 	pObjList=rNewObj.GetObjList();
/*N*/ 	bNeedRepaint=TRUE;
/*N*/ 	eHint=HINT_OBJCHG;
/*N*/ }
/*N*/ 
/*N*/ SdrHint::SdrHint(const SdrObject& rNewObj, const Rectangle& rRect)
/*N*/ {
/*N*/ 	aRect = rRect;
/*N*/ 	pPage = rNewObj.GetPage();
/*N*/ 	pObj = &rNewObj;
/*N*/ 	pObjList = rNewObj.GetObjList();
/*N*/ 	bNeedRepaint = TRUE;
/*N*/ 	eHint = HINT_OBJCHG;
/*N*/ }
}
