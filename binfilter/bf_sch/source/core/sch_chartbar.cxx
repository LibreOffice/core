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

#include "chtmodel.hxx"
#include "chaxis.hxx"
namespace binfilter {


/*N*/ void ChartBarDescriptor::SetGap(long nPercent)
/*N*/ {
/*N*/ 	nGapPercent=nPercent;
/*N*/ 	if(mpAxis && mpModel)
/*N*/ 	{
/*N*/ 		SfxItemSet aSet(mpModel->GetPool(),SCHATTR_BAR_GAPWIDTH,SCHATTR_BAR_GAPWIDTH);
/*N*/ 		aSet.Put(SfxInt32Item(SCHATTR_BAR_GAPWIDTH,nPercent));
/*N*/ 		mpAxis->GetItemSet()->Put(aSet);
/*N*/ 	}
/*N*/ }

/*N*/ void ChartBarDescriptor::SetOverlap(long nPercent)
/*N*/ {
/*N*/ 	nOverlapPercent=nPercent;
/*N*/ 	if(mpAxis && mpModel)
/*N*/ 	{
/*N*/ 		SfxItemSet aSet(mpModel->GetPool(),SCHATTR_BAR_OVERLAP,SCHATTR_BAR_OVERLAP);
/*N*/ 		aSet.Put(SfxInt32Item(SCHATTR_BAR_OVERLAP,nPercent));
/*N*/ 		mpAxis->GetItemSet()->Put(aSet);
/*N*/ 	}
/*N*/ }

/*N*/ long ChartBarDescriptor::GetOverlap() const
/*N*/ {
/*N*/ 	if(mpAxis)
/*N*/ 	{
/*N*/ 		SfxItemSet* pSet=mpAxis->GetItemSet();
/*N*/ 		const SfxPoolItem *pPoolItem = NULL;
/*N*/ 		if (pSet->GetItemState(SCHATTR_BAR_OVERLAP, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 		((ChartBarDescriptor*)this)->nOverlapPercent=((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/ 	}
/*N*/ 	return nOverlapPercent;
/*N*/ }

/*N*/ long ChartBarDescriptor::GetGap() const
/*N*/ {
/*N*/ 	if(mpAxis)
/*N*/ 	{
/*N*/ 		SfxItemSet* pSet=mpAxis->GetItemSet();
/*N*/ 		const SfxPoolItem *pPoolItem = NULL;
/*N*/ 		if (pSet->GetItemState(SCHATTR_BAR_GAPWIDTH, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 		((ChartBarDescriptor*)this)->nGapPercent=((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/ 	}
/*N*/ 	return nGapPercent;
/*N*/ }

/*N*/ void ChartBarDescriptor::Create(const Rectangle& aRect,long nColCnt,long nRowCnt)
/*N*/ {
/*N*/ 	nPartWidth=0;
/*N*/ 	nBarWidth=0;
/*N*/ 	nGap=nOver=nBarWidth=nPartWidth=nLeft=nCurrent=0;
/*N*/ 
/*N*/ 	nOverlapPercent=GetOverlap();
/*N*/ 	nGapPercent=GetGap();
/*N*/ 
/*N*/ 	if(!nColCnt||!nRowCnt)
/*N*/ 	{
/*?*/ 		DBG_TRACE("Chart :: Keine Daten!");
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	//gestapelt => alles übereinander, das entspricht im Prinzip dem Fall mit nur
/*N*/ 	//einer Datenreihe (interpretiere nRowCnt als Anzahl NEBENEINANDERSTEHENDER Balken)
/*N*/ 	if(mpModel->IsStacked())
/*N*/ 		nRowCnt = 1;
/*N*/ 
/*N*/ 	//Dieser Platz steht uns zur Verfügung:
/*N*/ 	BOOL bVerticalX = mpModel->IsXVertikal();
/*N*/ 	nPartWidth = (bVerticalX ? aRect.GetHeight() : aRect.GetWidth()) / nColCnt;
/*N*/ 
/*N*/ 	//Es gilt (bei MS-Excel97, empirisch ermittelt) :
/*N*/ 	//Barwidth = (Partwidth - Gapwidth) / (Rows - Overlap * (Rows -1) )
/*N*/ 	//Gapwidth = Partwidth - GapPercent/100 * Barwidth
/*N*/ 	// => Barwidth = Partwidth / (Rowfak + GapPercent/100)
/*N*/ 
/*N*/ 	double fGap	    = (double)nGapPercent/100.0;
/*N*/ 	double fOverlap = (double)nOverlapPercent/100.0;
/*N*/ 
/*N*/ 	//sicher ist sicher.... sollte aber nie negativ eingegeben werden können
/*N*/ 	if(fGap<0.0)
/*N*/ 		fGap=0.0;
/*N*/ 
/*N*/ 	//Datenreihenüberlappfaktor, Range = [1,(2*nRow-1)]
/*N*/ 	double fRowFak  = (double)nRowCnt - fOverlap * (double)(nRowCnt - 1);
/*N*/ 
/*N*/ 	//jetzt kann die Balkendicke ermittelt werden:
/*N*/ 	nBarWidth = (long) ((double)nPartWidth / (fRowFak + fGap));
/*N*/ 	nGap = (long) (fGap * (double)nBarWidth);
/*N*/ 
/*N*/ 	//Position des jeweils folgenden Balkens einer Spalte
/*N*/ 	nOver=(long)( (double)nBarWidth - fOverlap*(double)nBarWidth );
/*N*/ 
/*N*/ 	//jetzt nochmal nachsehen, ob die Berechnungen vernünftige Diagramme liefern:
/*N*/ 	const long nMinBarWidth = 40;//hat mal irgendwer festgelegt, gab auch mal nen Max...
/*N*/ 	if(nBarWidth < nMinBarWidth)
/*N*/ 	{
/*?*/ 		if(nRowCnt>1 && fOverlap < 0.0)
/*?*/ 		{
/*?*/ 			//dann erstmal den Überlapp vergrößern auf maximal kein Überlapp ;)
/*?*/ 			double fMinlap = ( -((double)nPartWidth/(double)nMinBarWidth) + fGap + (double)nRowCnt )
/*?*/ 				//--------------------------------------------------------------------------
/*?*/ 									   / (double)(nRowCnt-1);
/*?*/ 			if(fMinlap < 0.0) //sollte noch negativ (= Lücke) sein
/*?*/ 			{
/*?*/ 				fOverlap=fMinlap;
/*?*/ 				fRowFak  = (double)nRowCnt - fOverlap * (double)(nRowCnt - 1);
/*?*/ 				nBarWidth = (long) ((double)nPartWidth / (fRowFak + fGap));
/*?*/ 				nGap = (long) (fGap * (double)nBarWidth);
/*?*/ 				nOver=(long)( (double)nBarWidth - fOverlap*(double)nBarWidth );
/*?*/ 			}
/*?*/ 			else //hilft alles nix => erstmal ganz weg mit den Überlapp-Lücken
/*?*/ 			{	 //bei negativem Überlapp
/*?*/ 				nOver=0;
/*?*/ 				nBarWidth = (long) ((double)nPartWidth / ((double)nRowCnt + fGap));
/*?*/ 				nGap = (long) (fGap * (double)nBarWidth);
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if(nBarWidth < nMinBarWidth)//immer noch zu klein, dann reguläre Lücke verkleinern
/*?*/ 		{
/*?*/ 			double fMingap = (double)nPartWidth/(double)nMinBarWidth - fRowFak;
/*?*/ 			if(fMingap > 0.0 ) //noch kleiner sollte es besser nicht werden ....
/*?*/ 			{
/*?*/ 				fGap=fMingap;
/*?*/ 				fRowFak  = (double)nRowCnt - fOverlap * (double)(nRowCnt - 1);
/*?*/ 				nBarWidth = (long) ((double)nPartWidth / (fRowFak + fGap));
/*?*/ 				nGap = (long) (fGap * (double)nBarWidth);
/*?*/ 				nOver=(long)( (double)nBarWidth - fOverlap*(double)nBarWidth );
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{	//mehr geht eh nicht, ausser vielleicht Überlapp -> 1.0
/*?*/ 				nGap=0;
/*?*/ 				nBarWidth=nPartWidth / nRowCnt;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if( nBarWidth <= 0 )
/*?*/ 			DBG_WARNING(  "ChartBarDescriptor: cannot create bars with zero width" );
/*N*/ 	}
/*N*/ 
/*N*/ 	nLeft = nCurrent = (bVerticalX ? aRect.Top() : aRect.Left()) + nGap/2;
/*N*/ }
}
