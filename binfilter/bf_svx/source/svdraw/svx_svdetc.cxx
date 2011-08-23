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

#include "forbiddencharacterstable.hxx"

#include "svdetc.hxx"
#include "svditext.hxx"
#include "svdstr.hrc"
#include "svdviter.hxx"
#include "svdview.hxx"
#include "svdoutl.hxx"

#include <vcl/bmpacc.hxx>

#include <eeitem.hxx>



#include <bf_svtools/cacheoptions.hxx>








#include "xflclit.hxx"

#include "xflhtit.hxx"

#include "xbtmpit.hxx"

#include "xflgrit.hxx"

#include "svdoole2.hxx"



#include <bf_svtools/syslocale.hxx>

// #97870# 
#include "xflbckit.hxx"

#include <bf_goodies/b3dcolor.hxx>

namespace binfilter {

/******************************************************************************
* Globale Daten der DrawingEngine
******************************************************************************/

/*N*/ SdrGlobalData::SdrGlobalData() :
/*N*/ 	pOutliner(NULL),
/*N*/ 	pDefaults(NULL),
/*N*/ 	pResMgr(NULL),
/*N*/ 	pStrCache(NULL),
/*N*/ 	nExchangeFormat(0)
/*N*/ {
/*N*/     pSysLocale = new SvtSysLocale;
/*N*/     pCharClass = pSysLocale->GetCharClassPtr();
/*N*/     pLocaleData = pSysLocale->GetLocaleDataPtr();
/*N*/ }

/*N*/ SdrGlobalData::~SdrGlobalData()
/*N*/ {
/*N*/ 	delete pOutliner;
/*N*/ 	delete pDefaults;
/*N*/ 	delete pResMgr;
/*N*/ 	delete [] pStrCache;
/*N*/     //! do NOT delete pCharClass and pLocaleData
/*N*/     delete pSysLocale;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ OLEObjCache::OLEObjCache()
/*N*/ :	Container( 0 )
/*N*/ {
/*N*/ 	SvtCacheOptions aCacheOptions;
/*N*/ 
/*N*/ 	nSize = aCacheOptions.GetDrawingEngineOLE_Objects();
/*N*/ 	pTimer = new AutoTimer();
/*N*/     Link aLink = LINK(this, OLEObjCache, UnloadCheckHdl);
/*N*/ 
/*N*/     pTimer->SetTimeoutHdl(aLink);
/*N*/     pTimer->SetTimeout(20000);
/*N*/     pTimer->Start();
/*N*/ 
/*N*/ 	aLink.Call(pTimer);
/*N*/ }

/*N*/ OLEObjCache::~OLEObjCache()
/*N*/ {
/*N*/ 	pTimer->Stop();
/*N*/ 	delete pTimer;
/*N*/ 	// Kein Unload notwendig, da zu diesem Zeitpunkt
/*N*/ 	// die Objekte nicht mehr vorhanden sind
/*N*/ }


/*N*/ void OLEObjCache::InsertObj(SdrOle2Obj* pObj)
/*N*/ {
/*N*/ 	if (nSize <= Count())
/*N*/ 	{
/*?*/ 		// Eintraege reduzieren
/*?*/ 		ULONG nIndex = Count() - 1;
/*?*/ 
/*?*/ 		for (ULONG i = nIndex; i + 1 >= nSize; i--)
/*?*/ 		{
/*?*/ 			// Pruefen, ob Objekte entfernt werden koennen
/*?*/ 			SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(i);
/*?*/ 
/*?*/ 			if ( pCacheObj != pObj &&  UnloadObj(pCacheObj) )
/*?*/ 			{
/*?*/ 			 	Remove(i);
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Objekt ggf. entfernen und an erster Position einfuegen
/*N*/ 	Remove(pObj);
/*N*/ 	Insert(pObj, (ULONG) 0L);
/*N*/ }

/*N*/ void OLEObjCache::RemoveObj(SdrOle2Obj* pObj)
/*N*/ {
/*N*/ 	UnloadObj( (SdrOle2Obj*) Remove(pObj) );
/*N*/ }

/*N*/ BOOL OLEObjCache::UnloadObj(SdrOle2Obj* pObj)
/*N*/ {
/*N*/ 	BOOL bUnloaded = FALSE;
/*N*/ 
/*N*/ 	if (pObj)
/*N*/ 	{
/*N*/ 		BOOL bVisible = FALSE;
/*N*/   		SdrViewIter aIter(pObj);
/*N*/     	SdrView* pView = aIter.FirstView();
/*N*/ 
/*N*/     	while (!bVisible && pView!=NULL)
/*N*/ 		{
/*?*/         	bVisible = !pView->IsGrafDraft();
/*?*/ 
/*?*/ 			if (!bVisible)
/*?*/ 				pView = aIter.NextView();
/*N*/     	}
/*N*/ 
/*N*/ 		if (!bVisible)
/*N*/ 			bUnloaded = pObj->Unload();
/*N*/ 	}
/*N*/ 
/*N*/ 	return bUnloaded;
/*N*/ }

/*N*/ IMPL_LINK(OLEObjCache, UnloadCheckHdl, AutoTimer*, pTim)
/*N*/ {
/*N*/ 	if (nSize <= Count())
/*N*/ 	{
/*?*/ 		// Eintraege reduzieren
/*?*/ 		ULONG nIndex = Count() - 1;
/*?*/ 
/*?*/ 		for (ULONG i = nIndex; i + 1 >= nSize; i--)
/*?*/ 		{
/*?*/ 			// Pruefen, ob Objekte entfernt werden koennen
/*?*/ 			SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(i);
/*?*/ 
/*?*/ 			if ( UnloadObj(pCacheObj) )
/*?*/ 				Remove(i);
/*?*/ 		}
/*?*/ 	}
/*N*/ 
/*N*/     return 0;
/*N*/ }



/*N*/ void ContainerSorter::DoSort(ULONG a, ULONG b) const
/*N*/ {
/*N*/ 	ULONG nAnz=rCont.Count();
/*N*/ 	if (b>nAnz) b=nAnz;
/*N*/ 	if (b>0) b--;
/*N*/ 	if (a<b) ImpSubSort(a,b);
/*N*/ }


/*?*/ void ContainerSorter::ImpSubSort(long nL, long nR) const
/*?*/ {
/*?*/ 	long i,j;
/*?*/ 	const void* pX;
/*?*/ 	void* pI;
/*?*/ 	void* pJ;
/*?*/ 	i=nL;
/*?*/ 	j=nR;
/*?*/ 	pX=rCont.GetObject((nL+nR)/2);
/*?*/ 	do {
/*?*/ 		pI=rCont.Seek(i);
/*?*/ 		while (pI!=pX && Compare(pI,pX)<0) { i++; pI=rCont.Next(); }
/*?*/ 		pJ=rCont.Seek(j);
/*?*/ 		while (pJ!=pX && Compare(pX,pJ)<0) { j--; pJ=rCont.Prev(); }
/*?*/ 		if (i<=j) {
/*?*/ 			rCont.Replace(pJ,i);
/*?*/ 			rCont.Replace(pI,j);
/*?*/ 			i++;
/*?*/ 			j--;
/*?*/ 		}
/*?*/ 	} while (i<=j);
/*?*/ 	if (nL<j) ImpSubSort(nL,j);
/*?*/ 	if (i<nR) ImpSubSort(i,nR);
/*?*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////

/*?*/ class ImpClipMerk {
/*?*/ 	Region aClip;
/*?*/ 	FASTBOOL   bClip;
/*?*/ public:
/*?*/ 	ImpClipMerk(const OutputDevice& rOut): aClip(rOut.GetClipRegion()),bClip(rOut.IsClipRegion()) {}
/*?*/ 	void Restore(OutputDevice& rOut)
/*?*/ 	{
/*?*/ 		// Kein Clipping in die Metafileaufzeichnung
/*?*/ 		GDIMetaFile* pMtf=rOut.GetConnectMetaFile();
/*?*/ 		if (pMtf!=NULL && (!pMtf->IsRecord() || pMtf->IsPause())) pMtf=NULL;
/*?*/ 		if (pMtf!=NULL) pMtf->Pause(TRUE);
/*?*/ 		if (bClip) rOut.SetClipRegion(aClip);
/*?*/ 		else rOut.SetClipRegion();
/*?*/ 		if (pMtf!=NULL) pMtf->Pause(FALSE);
/*?*/ 	}
/*?*/ };
/*?*/ 
/*?*/ class ImpColorMerk {
/*?*/ 	Color aLineColor;
/*?*/ 	Color aFillColor;
/*?*/ 	Color aBckgrdColor;
/*?*/ 	Font  aFont;
/*?*/ public:
/*?*/ 	ImpColorMerk(const OutputDevice& rOut):
/*?*/ 		aLineColor( rOut.GetLineColor() ),
/*?*/ 		aFillColor( rOut.GetFillColor() ),
/*?*/ 		aBckgrdColor( rOut.GetBackground().GetColor() ),
/*?*/ 		aFont (rOut.GetFont()) {}
/*?*/ 
/*?*/ 	ImpColorMerk(const OutputDevice& rOut, USHORT nMode)
/*?*/ 	{
/*?*/ 		if ( (nMode & SDRHDC_SAVEPEN) == SDRHDC_SAVEPEN )
/*?*/ 			aLineColor = rOut.GetLineColor();
/*?*/ 
/*?*/ 		if ( (nMode & SDRHDC_SAVEBRUSH) == SDRHDC_SAVEBRUSH)
/*?*/ 		{
/*?*/ 			aFillColor = rOut.GetFillColor();
/*?*/ 			aBckgrdColor = rOut.GetBackground().GetColor();
/*?*/ 		}
/*?*/ 
/*?*/ 		if ( (nMode & SDRHDC_SAVEFONT) == SDRHDC_SAVEFONT)
/*?*/ 			aFont=rOut.GetFont();
/*?*/ 	}
/*?*/ 
/*?*/ 	void Restore(OutputDevice& rOut, USHORT nMode=SDRHDC_SAVEPENANDBRUSHANDFONT)
/*?*/ 	{
/*?*/ 		if ( (nMode & SDRHDC_SAVEPEN) == SDRHDC_SAVEPEN)
/*?*/ 			rOut.SetLineColor( aLineColor );
/*?*/ 
/*?*/ 		if ( (nMode & SDRHDC_SAVEBRUSH) == SDRHDC_SAVEBRUSH)
/*?*/ 		{
/*?*/ 			rOut.SetFillColor( aFillColor );
/*?*/ 			rOut.SetBackground( Wallpaper( aBckgrdColor ) );
/*?*/ 		}
/*?*/ 		if ((nMode & SDRHDC_SAVEFONT) ==SDRHDC_SAVEFONT)
/*?*/ 		{
/*?*/ 			if (!rOut.GetFont().IsSameInstance(aFont))
/*?*/ 			{
/*?*/ 				rOut.SetFont(aFont);
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*?*/ 
/*?*/ 	const Color& GetLineColor() const { return aLineColor; }
/*?*/ };

/*N*/ ImpSdrHdcMerk::ImpSdrHdcMerk(const OutputDevice& rOut, USHORT nNewMode, FASTBOOL bAutoMerk):
/*N*/ 	pFarbMerk(NULL),
/*N*/ 	pClipMerk(NULL),
/*N*/ 	pLineColorMerk(NULL),
/*N*/ 	nMode(nNewMode)
/*N*/ {
/*N*/ 	if (bAutoMerk) Save(rOut);
/*N*/ }

/*N*/ ImpSdrHdcMerk::~ImpSdrHdcMerk()
/*N*/ {
/*N*/ 	if (pFarbMerk!=NULL) delete pFarbMerk;
/*N*/ 	if (pClipMerk!=NULL) delete pClipMerk;
/*N*/ 	if (pLineColorMerk !=NULL) delete pLineColorMerk;
/*N*/ }

/*N*/ void ImpSdrHdcMerk::Save(const OutputDevice& rOut)
/*N*/ {
/*N*/ 	if (pFarbMerk!=NULL)
/*N*/ 	{
/*N*/ 		delete pFarbMerk;
/*N*/ 		pFarbMerk=NULL;
/*N*/ 	}
/*N*/ 	if (pClipMerk!=NULL)
/*N*/ 	{
/*N*/ 		delete pClipMerk;
/*N*/ 		pClipMerk=NULL;
/*N*/ 	}
/*N*/ 	if (pLineColorMerk !=NULL)
/*N*/ 	{
/*N*/ 		delete pLineColorMerk ;
/*N*/ 		pLineColorMerk =NULL;
/*N*/ 	}
/*N*/ 	if ((nMode & SDRHDC_SAVECLIPPING) ==SDRHDC_SAVECLIPPING)
/*N*/ 		pClipMerk=new ImpClipMerk(rOut);
/*N*/ 
/*N*/ 	USHORT nCol=nMode & SDRHDC_SAVEPENANDBRUSHANDFONT;
/*N*/ 
/*N*/ 	if (nCol==SDRHDC_SAVEPEN)
/*N*/ 		pLineColorMerk=new Color( rOut.GetLineColor() );
/*N*/ 	else if (nCol==SDRHDC_SAVEPENANDBRUSHANDFONT)
/*N*/ 		pFarbMerk=new ImpColorMerk(rOut);
/*N*/ 	else if (nCol!=0)
/*N*/ 		pFarbMerk=new ImpColorMerk(rOut,nCol);
/*N*/ }

/*N*/ void ImpSdrHdcMerk::Restore(OutputDevice& rOut, USHORT nMask) const
/*N*/ {
/*N*/ 	nMask&=nMode; // nur restaurieren, was auch gesichert wurde
/*N*/ 
/*N*/ 	if ((nMask & SDRHDC_SAVECLIPPING) ==SDRHDC_SAVECLIPPING && pClipMerk!=NULL)
/*N*/ 		pClipMerk->Restore(rOut);
/*N*/ 
/*N*/ 	USHORT nCol=nMask & SDRHDC_SAVEPENANDBRUSHANDFONT;
/*N*/ 
/*N*/ 	if (nCol==SDRHDC_SAVEPEN)
/*N*/ 	{
/*N*/ 		if (pLineColorMerk!=NULL)
/*N*/ 			rOut.SetLineColor(*pLineColorMerk);
/*N*/ 		else if (pFarbMerk!=NULL)
/*N*/ 			rOut.SetLineColor( pFarbMerk->GetLineColor() );
/*N*/ 	} else if (nCol!=0 && pFarbMerk!=NULL)
/*N*/ 		pFarbMerk->Restore(rOut,nCol);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void SdrLinkList::Clear()
/*N*/ {
/*N*/ 	unsigned nAnz=GetLinkCount();
/*N*/ 	for (unsigned i=0; i<nAnz; i++) {
/*N*/ 		delete (Link*)aList.GetObject(i);
/*N*/ 	}
/*N*/ 	aList.Clear();
/*N*/ }

/*N*/ unsigned SdrLinkList::FindEntry(const Link& rLink) const
/*N*/ {
/*N*/ 	unsigned nAnz=GetLinkCount();
/*N*/ 	for (unsigned i=0; i<nAnz; i++) {
/*N*/ 		if (GetLink(i)==rLink) return i;
/*N*/ 	}
/*N*/ 	return 0xFFFF;
/*N*/ }

/*N*/ void SdrLinkList::InsertLink(const Link& rLink, unsigned nPos)
/*N*/ {
/*N*/ 	unsigned nFnd=FindEntry(rLink);
/*N*/ 	if (nFnd==0xFFFF) {
/*N*/ 		if (rLink.IsSet()) {
/*N*/ 			aList.Insert(new Link(rLink),nPos);
/*N*/ 		} else {
/*N*/ 			DBG_ERROR("SdrLinkList::InsertLink(): Versuch, einen nicht gesetzten Link einzufuegen");
/*N*/ 		}
/*N*/ 	} else {
/*N*/ 		DBG_ERROR("SdrLinkList::InsertLink(): Link schon vorhanden");
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void SdrLinkList::RemoveLink(const Link& rLink)
/*N*/ {
/*N*/ 	unsigned nFnd=FindEntry(rLink);
/*N*/ 	if (nFnd!=0xFFFF) {
/*N*/ 		Link* pLink=(Link*)aList.Remove(nFnd);
/*N*/ 		delete pLink;
/*N*/ 	} else {
/*N*/ 		DBG_ERROR("SdrLinkList::RemoveLink(): Link nicht gefunden");
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// #98988# Re-implement GetDraftFillColor(...)

/*N*/ FASTBOOL GetDraftFillColor(const SfxItemSet& rSet, Color& rCol)
/*N*/ {
/*N*/ 	XFillStyle eFill=((XFillStyleItem&)rSet.Get(XATTR_FILLSTYLE)).GetValue();
/*N*/ 	FASTBOOL bRetval(FALSE);
/*N*/ 
/*N*/ 	switch(eFill) 
/*N*/ 	{
/*N*/ 		case XFILL_SOLID: 
/*N*/ 		{
/*N*/ 			rCol = ((XFillColorItem&)rSet.Get(XATTR_FILLCOLOR)).GetValue(); 
/*N*/ 			bRetval = TRUE;
/*N*/ 			
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case XFILL_HATCH: 
/*N*/ 		{
/*?*/ 			Color aCol1(((XFillHatchItem&)rSet.Get(XATTR_FILLHATCH)).GetValue().GetColor());
/*?*/ 			Color aCol2(COL_WHITE);
/*?*/ 			
/*?*/ 			// #97870# when hatch background is activated, use object fill color as hatch color
/*?*/ 			sal_Bool bFillHatchBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();
/*?*/ 			if(bFillHatchBackground)
/*?*/ 			{
/*?*/ 				aCol2 = ((const XFillColorItem&)(rSet.Get(XATTR_FILLCOLOR))).GetValue();
/*?*/ 			}
/*?*/ 
/*?*/ 			((B3dColor&)rCol).CalcMiddle(aCol1, aCol2);
/*?*/ 			bRetval = TRUE;
/*?*/ 			
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		case XFILL_GRADIENT: {
/*N*/ 			const XGradient& rGrad=((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetValue();
/*N*/ 			Color aCol1(rGrad.GetStartColor());
/*N*/ 			Color aCol2(rGrad.GetEndColor());
/*N*/ 			((B3dColor&)rCol).CalcMiddle(aCol1, aCol2);
/*N*/ 			bRetval = TRUE;
/*N*/ 
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case XFILL_BITMAP: 
/*N*/ 		{
/*?*/ 			const Bitmap& rBitmap = ((XFillBitmapItem&)rSet.Get(XATTR_FILLBITMAP)).GetValue().GetBitmap();
/*?*/ 			const Size aSize(rBitmap.GetSizePixel());
/*?*/ 			const sal_uInt32 nWidth = aSize.Width();
/*?*/ 			const sal_uInt32 nHeight = aSize.Height();
/*?*/ 			Bitmap aBitmap(rBitmap);
/*?*/ 			BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
/*?*/ 
/*?*/ 			if(pAccess && nWidth > 0 && nHeight > 0)
/*?*/ 			{
/*?*/ 				sal_uInt32 nRt(0L);
/*?*/ 				sal_uInt32 nGn(0L);
/*?*/ 				sal_uInt32 nBl(0L);
/*?*/ 				const sal_uInt32 nMaxSteps(8L);
/*?*/ 				const sal_uInt32 nXStep((nWidth > nMaxSteps) ? nWidth / nMaxSteps : 1L);
/*?*/ 				const sal_uInt32 nYStep((nHeight > nMaxSteps) ? nHeight / nMaxSteps : 1L);
/*?*/ 				sal_uInt32 nAnz(0L);
/*?*/ 
/*?*/ 				for(sal_uInt32 nY(0L); nY < nHeight; nY += nYStep)
/*?*/ 				{
/*?*/ 					for(sal_uInt32 nX(0L); nX < nWidth; nX += nXStep)
/*?*/ 					{
/*?*/ 						const BitmapColor& rCol = (pAccess->HasPalette())
/*?*/ 							? pAccess->GetPaletteColor((BYTE)pAccess->GetPixel(nY, nX))
/*?*/ 							: pAccess->GetPixel(nY, nX);
/*?*/ 
/*?*/ 						nRt += rCol.GetRed(); 
/*?*/ 						nGn += rCol.GetGreen(); 
/*?*/ 						nBl += rCol.GetBlue();
/*?*/ 						nAnz++;
/*?*/ 					}
/*?*/ 				}
/*?*/ 
/*?*/ 				nRt /= nAnz;
/*?*/ 				nGn /= nAnz;
/*?*/ 				nBl /= nAnz;
/*?*/ 
/*?*/ 				rCol = Color(UINT8(nRt), UINT8(nGn), UINT8(nBl));
/*?*/ 
/*?*/ 				bRetval = TRUE;
/*?*/ 			}
/*?*/ 			
/*?*/ 			if(pAccess)
/*?*/ 			{
/*?*/ 				aBitmap.ReleaseAccess(pAccess);
/*?*/ 			}
/*?*/ 			
/*?*/ 			break;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRetval;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrEngineDefaults::SdrEngineDefaults():
/*N*/ 	aFontName( OutputDevice::GetDefaultFont( DEFAULTFONT_SERIF, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE ).GetName() ),
/*N*/ 	eFontFamily(FAMILY_ROMAN),
/*N*/ 	aFontColor(COL_AUTO),
/*N*/ 	nFontHeight(847),			  // 847/100mm = ca. 24 Point
/*N*/ 	eMapUnit(MAP_100TH_MM),
/*N*/ 	aMapFraction(1,1)
/*N*/ {
/*N*/ }
/*N*/ 
/*N*/ SdrEngineDefaults& SdrEngineDefaults::GetDefaults()
/*N*/ {
/*N*/ 	SdrGlobalData& rGlobalData=GetSdrGlobalData();
/*N*/ 	if (rGlobalData.pDefaults==NULL) {
/*N*/ 		rGlobalData.pDefaults=new SdrEngineDefaults;
/*N*/ 	}
/*N*/ 	return *rGlobalData.pDefaults;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pModel )
/*N*/ {
/*N*/ 	SdrEngineDefaults& rDefaults = SdrEngineDefaults::GetDefaults();

/*
    MapUnit  eUn( (pMod==NULL) ? rDefaults.eMapUnit : pMod->GetScaleUnit());
    Fraction aFr( *((pMod==NULL) ? &rDefaults.aMapFraction : &pMod->GetScaleFraction()));

    if ( pMod->GetRefDevice() )
        pOutl->SetRefDevice( pMod->GetRefDevice() );
    else
    {
        MapMode aMapMode(eUn,Point(0,0),aFr,aFr);
        pOutl->SetRefMapMode( aMapMode );
    }

    SfxItemSet aSet(pOutl->GetEmptyItemSet());
    aSet.Put(SvxFontItem(rDefaults.eFontFamily, rDefaults.aFontName, String(), PITCH_DONTKNOW, gsl_getSystemTextEncoding() ) );
    aSet.Put(SvxColorItem(rDefaults.aFontColor));
    ULONG nHgt=rDefaults.nFontHeight;
    FASTBOOL bDifUn=(eUn!=rDefaults.eMapUnit); // different MapUnits
    FASTBOOL bDifFr=(aFr!=rDefaults.aMapFraction); // different MapFractions
    if (bDifUn || bDifFr) { // Wenn pMod!=NULL und pMod->Map!=rDef.Map
        long nTmpLong=long(nHgt); // caasting im Ctor bringt unter MSVC sehr merkwuerdige Fehlermeldungen
        BigInt aHgt1(nTmpLong); // umrechnen von DefMap in ModMap
        FrPair aUnitMul(GetMapFactor(rDefaults.eMapUnit,eUn));

        if (bDifUn) aHgt1*=aUnitMul.Y().GetNumerator();
        if (bDifFr) aHgt1*=aFr.GetNumerator();
        if (bDifFr) aHgt1*=rDefaults.aMapFraction.GetDenominator();
        if (bDifUn) aHgt1/=aUnitMul.Y().GetDenominator();
        if (bDifFr) aHgt1/=aFr.GetDenominator();
        if (bDifFr) aHgt1/=rDefaults.aMapFraction.GetNumerator();

        nHgt=ULONG(long(aHgt1));
    }
    aSet.Put(SvxFontHeightItem(nHgt));
    pOutl->SetDefaults(aSet);
*/

/*N*/ 	SfxItemPool* pPool = &pModel->GetItemPool();
/*N*/ 	SdrOutliner* pOutl = new SdrOutliner( pPool, nOutlinerMode );
/*N*/ 	pOutl->SetEditTextObjectPool( pPool );
/*N*/ 	pOutl->SetStyleSheetPool( (SfxStyleSheetPool*) pModel->GetStyleSheetPool() );
/*N*/ 	pOutl->SetDefTab( pModel->GetDefaultTabulator() );
/*N*/ 	pOutl->SetForbiddenCharsTable( pModel->GetForbiddenCharsTable() );
/*N*/ 	pOutl->SetAsianCompressionMode( pModel->GetCharCompressType() );
/*N*/ 	pOutl->SetKernAsianPunctuation( pModel->IsKernAsianPunctuation() );
/*N*/ 
/*N*/ 	return pOutl;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////


/*N*/ SdrLinkList& ImpGetUserMakeObjHdl()
/*N*/ {
/*N*/ 	SdrGlobalData& rGlobalData=GetSdrGlobalData();
/*N*/ 	return rGlobalData.aUserMakeObjHdl;
/*N*/ }

/*N*/ SdrLinkList& ImpGetUserMakeObjUserDataHdl()
/*N*/ {
/*N*/ 	SdrGlobalData& rGlobalData=GetSdrGlobalData();
/*N*/ 	return rGlobalData.aUserMakeObjUserDataHdl;
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ ResMgr* ImpGetResMgr()
/*N*/ {
/*N*/ 	SdrGlobalData& rGlobalData = GetSdrGlobalData();
/*N*/ 
/*N*/ 	if(!rGlobalData.pResMgr)
/*N*/ 	{
/*N*/ #ifndef SVX_LIGHT
/*N*/ 		ByteString aName("bf_svx");			//STRIP005
/*N*/ #else
/*N*/ 		ByteString aName("bf_svl");			//STRIP005
/*N*/ #endif
/*N*/ 		rGlobalData.pResMgr =
/*N*/             ResMgr::CreateResMgr( aName.GetBuffer(), Application::GetSettings().GetUILocale() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return rGlobalData.pResMgr;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
