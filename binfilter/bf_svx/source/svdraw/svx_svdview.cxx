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

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#include "svdio.hxx"
#include "svdstr.hrc"   // Namen aus der Resource
#include "svdglob.hxx"  // StringCache
#include "svdpagv.hxx"
#include "svdopath.hxx" // fuer GetContext
#include "svdograf.hxx" // fuer GetContext
#include "svdview.hxx"
#include "editview.hxx" // fuer GetField
#define ITEMID_FIELD EE_FEATURE_FIELD  /* wird fuer #include <flditem.hxx> benoetigt */


#ifndef _SVX_ITEMDATA_HXX
#include "itemdata.hxx"
#endif

#include "flditem.hxx"  // fuer URLField
#include "svdoutl.hxx"

// #90477#
#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif
namespace binfilter {

/*N*/ TYPEINIT1(SdrView,SdrCreateView);

/*N*/ SdrView::SdrView(SdrModel* pModel1, OutputDevice* pOut):
/*N*/ 	SdrCreateView(pModel1,pOut),
/*N*/ 	bNoExtendedMouseDispatcher(FALSE),
/*N*/ 	bNoExtendedKeyDispatcher(FALSE),
/*N*/ 	bNoExtendedCommandDispatcher(FALSE)
/*N*/ {
/*N*/ 	bTextEditOnObjectsWithoutTextIfTextTool=FALSE;
/*N*/ 
/*N*/ 
/*N*/ 	onAccessibilityOptionsChanged();
/*N*/ }

/*N*/ SdrView::SdrView(SdrModel* pModel1):
/*N*/ 	SdrCreateView(pModel1,(OutputDevice*)NULL),
/*N*/ 	bNoExtendedMouseDispatcher(FALSE),
/*N*/ 	bNoExtendedKeyDispatcher(FALSE),
/*N*/ 	bNoExtendedCommandDispatcher(FALSE)
/*N*/ {
/*N*/ 	bTextEditOnObjectsWithoutTextIfTextTool=FALSE;
/*N*/ 
/*N*/ 
/*N*/ 	onAccessibilityOptionsChanged();
/*N*/ }

/*N*/ SdrView::~SdrView()
/*N*/ {
/*N*/ }






/* new interface src537 */



// Mit der Maus draggen (Move)
// Beispiel beim erzeugen eines Rechtecks. MouseDown muss ohne
// ModKey erfolgen, weil sonst i.d.R. Markieren forciert wird (s.u.)
// Drueckt man dann beim MouseMove gleichzeitig Shift, Ctrl, und Alt,
// so erzeugt man ein zentrisches Quadrat ohne Fang.
// Die Doppelbelegung von Ortho und Shift stellt i.d.R. kein Problem dar,
// da sich beides meisst gegenseitig ausschliesst. Einzig Shear (das beim
// Verzerren, nicht dass beim Drehen) beruecksichtigt beides gleichzeitig.
// Dass muss der Anwender erstmal noch umschiffen (z.B. mit einer Hilfslinie).
#define MODKEY_NoSnap    bCtrl  /* Fang temporaer aus */
#define MODKEY_Ortho     bShift /* na eben ortho */
#define MODKEY_Center    bAlt   /* Zentrisch erzeugen/resizen */
//#define MODKEY_BigOrtho  bAlt
#define MODKEY_AngleSnap bShift
#define MODKEY_CopyDrag  bCtrl  /* Draggen mit kopieren */

// irgendwo hinklicken (MouseDown)
#define MODKEY_PolyPoly  bAlt   /* Neues Poly bei InsPt und bei Create */
#define MODKEY_MultiMark bShift /* MarkObj ohne vorher UnmarkAll */
#define MODKEY_Unmark    bAlt   /* Unmark durch Rahmenaufziehen */
#define MODKEY_ForceMark bCtrl  /* Rahmenaufziehen erzwingen, auch wenn Obj an MausPos */
#define MODKEY_DeepMark  bAlt   /* MarkNextObj */
#define MODKEY_DeepBackw bShift /* MarkNextObj rueckwaerts */

} //namespace binfilter
#ifndef _OUTLOBJ_HXX
#include "outlobj.hxx"
#endif
namespace binfilter {//STRIP009

/*N*/ void SdrView::UnmarkAll()
/*N*/ {
/*N*/ 	if (IsTextEdit()) {
/*?*/ 		ESelection eSel=GetTextEditOutlinerView()->GetSelection();
/*?*/ 		eSel.nStartPara=eSel.nEndPara;
/*?*/ 		eSel.nStartPos=eSel.nEndPos;
/*?*/ 		GetTextEditOutlinerView()->SetSelection(eSel);
/*?*/ #ifndef SVX_LIGHT
/*?*/ 		if (pItemBrowser!=NULL) {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 pItemBrowser->SetDirty();
/*?*/ #endif
/*N*/ 	} else if (HasMarkedGluePoints()) UnmarkAllGluePoints();
/*N*/ 	else if (HasMarkedPoints()) {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 UnmarkAllPoints(); // ! Marked statt Markable !
/*N*/ 	else UnmarkAllObj();
/*N*/ }








/*N*/ void SdrView::DeleteMarked()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }



/*N*/ void SdrView::WriteRecords(SvStream& rOut) const
/*N*/ {
/*N*/ 	{
/*N*/ 		// Der CharSet muss! als erstes rausgestreamt werden
/*N*/ 		SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWCHARSET);
/*N*/ 
/*N*/ 		// UNICODE:
/*N*/ 		// rtl_TextEncoding eOutCharSet=rOut.GetStreamCharSet();
/*N*/ 		rtl_TextEncoding eOutCharSet = gsl_getSystemTextEncoding();
/*N*/ 
/*N*/ 		// #90477# rOut << UINT16( GetStoreCharSet( eOutCharSet ) );
/*N*/ 		rOut << (UINT16)GetSOStoreTextEncoding(eOutCharSet, (sal_uInt16)rOut.GetVersion());
/*N*/ 	}
/*N*/ 	SdrCreateView::WriteRecords(rOut);
/*N*/ }

/*N*/ BOOL SdrView::ReadRecord(const SdrIOHeader& rViewHead,
/*N*/ 	const SdrNamedSubRecord& rSubHead,
/*N*/ 	SvStream& rIn)
/*N*/ {
/*N*/ 	BOOL bRet=FALSE;
/*N*/ 	if (rSubHead.GetInventor()==SdrInventor) {
/*N*/ 		bRet=TRUE;
/*N*/ 		switch (rSubHead.GetIdentifier()) {
/*N*/ 			case SDRIORECNAME_VIEWCHARSET: {
/*N*/ 				UINT16 nCharSet;
/*N*/ 				// #90477# rIn>>nCharSet;
/*N*/ 				// rIn.SetStreamCharSet(rtl_TextEncoding(nCharSet));
/*N*/ 				rIn >> nCharSet;
/*N*/ 				rIn.SetStreamCharSet(GetSOLoadTextEncoding((rtl_TextEncoding)nCharSet, (sal_uInt16)rIn.GetVersion()));
/*N*/ 			} break;
/*N*/ 			default: bRet=FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if (!bRet) bRet=SdrCreateView::ReadRecord(rViewHead,rSubHead,rIn);
/*N*/ 	return bRet;
/*N*/ }

/*N*/ SvStream& operator<<(SvStream& rOut, const SdrView& rView)
/*N*/ {
/*N*/ 	SdrIOHeader aHead(rOut,STREAM_WRITE,SdrIOViewID);
/*N*/ 	rView.WriteRecords(rOut);
/*N*/ 	return rOut;
/*N*/ }

/*N*/ SvStream& operator>>(SvStream& rIn, SdrView& rView)
/*N*/ {
/*N*/ 	if (rIn.GetError()!=0) return rIn;
/*N*/ 	rView.BrkAction();
/*N*/ 	rView.Clear();
/*N*/ 	SdrIOHeader aHead(rIn,STREAM_READ);
/*N*/ 	if (!aHead.IsMagic()) {
/*N*/ 		rIn.SetError(SVSTREAM_FILEFORMAT_ERROR); // Format-Fehler
/*N*/ 		return rIn;
/*N*/ 	}
/*N*/ 	rtl_TextEncoding eStreamCharSetMerker=rIn.GetStreamCharSet(); // Der StreamCharSet wird von SdrView::ReadRecord() gesetzt
/*N*/ 	while (aHead.GetBytesLeft()>0 && rIn.GetError()==0 && !rIn.IsEof()) {
/*N*/ 		SdrNamedSubRecord aSubRecord(rIn,STREAM_READ);
/*N*/ 		rView.ReadRecord(aHead,aSubRecord,rIn);
/*N*/ 	}
/*N*/ 
/*N*/ 	rIn.SetStreamCharSet(eStreamCharSetMerker); // StreamCharSet wieder restaurieren
/*N*/ 
/*N*/ 	rView.InvalidateAllWin();
/*N*/ 	return rIn;
/*N*/ }


/*N*/ void SdrView::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
/*N*/ {
/*N*/ 	if( rHint.ISA( SfxSimpleHint ) && ( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_ACCESSIBILITY_CHANGED )
/*N*/ 	{
/*?*/ 		onAccessibilityOptionsChanged();
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrCreateView::SFX_NOTIFY(rBC, rBCType, rHint, rHintType );
/*N*/ }

    
/** method is called whenever the global SvtAccessibilityOptions is changed */
/*N*/ void SdrView::onAccessibilityOptionsChanged()
/*N*/ {
/*N*/ }

}
