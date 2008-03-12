/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdotxat.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:55:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svtools/style.hxx>
#include <svx/svdotext.hxx>
#include "svditext.hxx"
#include <svx/svdmodel.hxx> // fuer GetMaxObjSize und GetStyleSheetPool
#include <svx/svdoutl.hxx>
#include <svx/svdorect.hxx> // fuer SetDirty bei NbcAdjustTextFrameWidthAndHeight
#include <svx/svdocapt.hxx> // fuer SetDirty bei NbcAdjustTextFrameWidthAndHeight
#include <svx/svdetc.hxx>
#include <svx/writingmodeitem.hxx>
#include <svx/editeng.hxx>
#include <svx/eeitem.hxx>
#include <svx/flditem.hxx>
#include <svx/sdtfchim.hxx>


#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#ifndef _EDITOBJ_HXX //autogen
#include <svx/editobj.hxx>
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif

#include <svx/charscaleitem.hxx>

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif

#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif

#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@@@ @@@@@ @@   @@ @@@@@@  @@@@  @@@@@  @@@@@@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@     @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@@@    @@@     @@   @@  @@ @@@@@      @@
//    @@   @@     @@@@@    @@   @@  @@ @@  @@     @@
//    @@   @@    @@@ @@@   @@   @@  @@ @@  @@ @@  @@
//    @@   @@@@@ @@   @@   @@    @@@@  @@@@@   @@@@
//
//  Attribute, StyleSheets und AutoGrow
//
////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL SdrTextObj::AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt, FASTBOOL bWdt) const
{
    if (bTextFrame && pModel!=NULL && !rR.IsEmpty())
    {
        SdrFitToSizeType eFit=GetFitToSize();
        FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
        FASTBOOL bWdtGrow=bWdt && IsAutoGrowWidth();
        FASTBOOL bHgtGrow=bHgt && IsAutoGrowHeight();
        SdrTextAniKind eAniKind=GetTextAniKind();
        SdrTextAniDirection eAniDir=GetTextAniDirection();
        FASTBOOL bScroll=eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE;
        FASTBOOL bHScroll=bScroll && (eAniDir==SDRTEXTANI_LEFT || eAniDir==SDRTEXTANI_RIGHT);
        FASTBOOL bVScroll=bScroll && (eAniDir==SDRTEXTANI_UP || eAniDir==SDRTEXTANI_DOWN);
        if (!bFitToSize && (bWdtGrow || bHgtGrow))
        {
            Rectangle aR0(rR);
            long nHgt=0,nMinHgt=0,nMaxHgt=0;
            long nWdt=0,nMinWdt=0,nMaxWdt=0;
            Size aSiz(rR.GetSize()); aSiz.Width()--; aSiz.Height()--;
            Size aMaxSiz(100000,100000);
            Size aTmpSiz(pModel->GetMaxObjSize());
            if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
            if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
            if (bWdtGrow)
            {
                nMinWdt=GetMinTextFrameWidth();
                nMaxWdt=GetMaxTextFrameWidth();
                if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
                if (nMinWdt<=0) nMinWdt=1;
                aSiz.Width()=nMaxWdt;
            }
            if (bHgtGrow)
            {
                nMinHgt=GetMinTextFrameHeight();
                nMaxHgt=GetMaxTextFrameHeight();
                if (nMaxHgt==0 || nMaxHgt>aMaxSiz.Height()) nMaxHgt=aMaxSiz.Height();
                if (nMinHgt<=0) nMinHgt=1;
                aSiz.Height()=nMaxHgt;
            }
            long nHDist=GetTextLeftDistance()+GetTextRightDistance();
            long nVDist=GetTextUpperDistance()+GetTextLowerDistance();
            aSiz.Width()-=nHDist;
            aSiz.Height()-=nVDist;
            if (aSiz.Width()<2) aSiz.Width()=2;   // Mindestgroesse 2
            if (aSiz.Height()<2) aSiz.Height()=2; // Mindestgroesse 2

            // #101684#
            BOOL bInEditMode = IsInEditMode();

            if(!bInEditMode)
            {
                if (bHScroll) aSiz.Width()=0x0FFFFFFF; // Laufschrift nicht umbrechen
                if (bVScroll) aSiz.Height()=0x0FFFFFFF;
            }

            if(pEdtOutl)
            {
                pEdtOutl->SetMaxAutoPaperSize(aSiz);
                if (bWdtGrow) {
                    Size aSiz2(pEdtOutl->CalcTextSize());
                    nWdt=aSiz2.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // lieber etwas Tolleranz
                } else {
                    nHgt=pEdtOutl->GetTextHeight()+1; // lieber etwas Tolleranz
                }
            } else {
                Outliner& rOutliner=ImpGetDrawOutliner();
                rOutliner.SetPaperSize(aSiz);
                rOutliner.SetUpdateMode(TRUE);
                // !!! hier sollte ich wohl auch noch mal die Optimierung mit
                // bPortionInfoChecked usw einbauen
                OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
                if ( pOutlinerParaObject != NULL )
                {
                    rOutliner.SetText(*pOutlinerParaObject);
                    rOutliner.SetFixedCellHeight(((const SdrTextFixedCellHeightItem&)GetMergedItem(SDRATTR_TEXT_USEFIXEDCELLHEIGHT)).GetValue());
                }
                if (bWdtGrow)
                {
                    Size aSiz2(rOutliner.CalcTextSize());
                    nWdt=aSiz2.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz2.Height()+1; // lieber etwas Tolleranz
                } else {
                    nHgt=rOutliner.GetTextHeight()+1; // lieber etwas Tolleranz
                }
                rOutliner.Clear();
            }
            if (nWdt<nMinWdt) nWdt=nMinWdt;
            if (nWdt>nMaxWdt) nWdt=nMaxWdt;
            nWdt+=nHDist;
            if (nWdt<1) nWdt=1; // nHDist kann auch negativ sein
            if (nHgt<nMinHgt) nHgt=nMinHgt;
            if (nHgt>nMaxHgt) nHgt=nMaxHgt;
            nHgt+=nVDist;
            if (nHgt<1) nHgt=1; // nVDist kann auch negativ sein
            long nWdtGrow=nWdt-(rR.Right()-rR.Left());
            long nHgtGrow=nHgt-(rR.Bottom()-rR.Top());
            if (nWdtGrow==0) bWdtGrow=FALSE;
            if (nHgtGrow==0) bHgtGrow=FALSE;
            if (bWdtGrow || bHgtGrow) {
                if (bWdtGrow) {
                    SdrTextHorzAdjust eHAdj=GetTextHorizontalAdjust();
                    if (eHAdj==SDRTEXTHORZADJUST_LEFT) rR.Right()+=nWdtGrow;
                    else if (eHAdj==SDRTEXTHORZADJUST_RIGHT) rR.Left()-=nWdtGrow;
                    else {
                        long nWdtGrow2=nWdtGrow/2;
                        rR.Left()-=nWdtGrow2;
                        rR.Right()=rR.Left()+nWdt;
                    }
                }
                if (bHgtGrow) {
                    SdrTextVertAdjust eVAdj=GetTextVerticalAdjust();
                    if (eVAdj==SDRTEXTVERTADJUST_TOP) rR.Bottom()+=nHgtGrow;
                    else if (eVAdj==SDRTEXTVERTADJUST_BOTTOM) rR.Top()-=nHgtGrow;
                    else {
                        long nHgtGrow2=nHgtGrow/2;
                        rR.Top()-=nHgtGrow2;
                        rR.Bottom()=rR.Top()+nHgt;
                    }
                }
                if (aGeo.nDrehWink!=0) {
                    Point aD1(rR.TopLeft());
                    aD1-=aR0.TopLeft();
                    Point aD2(aD1);
                    RotatePoint(aD2,Point(),aGeo.nSin,aGeo.nCos);
                    aD2-=aD1;
                    rR.Move(aD2.X(),aD2.Y());
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

FASTBOOL SdrTextObj::NbcAdjustTextFrameWidthAndHeight(FASTBOOL bHgt, FASTBOOL bWdt)
{
    FASTBOOL bRet=AdjustTextFrameWidthAndHeight(aRect,bHgt,bWdt);
    if (bRet) {
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // mal wieder 'nen Hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // mal wieder 'nen Hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }
    }
    return bRet;
}

FASTBOOL SdrTextObj::AdjustTextFrameWidthAndHeight(FASTBOOL bHgt, FASTBOOL bWdt)
{
    Rectangle aNeuRect(aRect);
    FASTBOOL bRet=AdjustTextFrameWidthAndHeight(aNeuRect,bHgt,bWdt);
    if (bRet) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        // #110094#-14 SendRepaintBroadcast();
        aRect=aNeuRect;
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // mal wieder 'nen Hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // mal wieder 'nen Hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
    return bRet;
}

void SdrTextObj::ImpSetTextStyleSheetListeners()
{
    SfxStyleSheetBasePool* pStylePool=pModel!=NULL ? pModel->GetStyleSheetPool() : NULL;
    if (pStylePool!=NULL)
    {
        Container aStyles(1024,64,64);
        OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
        if (pOutlinerParaObject!=NULL)
        {
            // Zunaechst werden alle im ParaObject enthaltenen StyleSheets
            // im Container aStyles gesammelt. Dazu wird die Family jeweils
            // ans Ende des StyleSheet-Namen drangehaengt.
            const EditTextObject& rTextObj=pOutlinerParaObject->GetTextObject();
            XubString aStyleName;
            SfxStyleFamily eStyleFam;
            USHORT nParaAnz=rTextObj.GetParagraphCount();

            for(UINT16 nParaNum(0); nParaNum < nParaAnz; nParaNum++)
            {
                rTextObj.GetStyleSheet(nParaNum, aStyleName, eStyleFam);

                if(aStyleName.Len())
                {
                    XubString aFam = UniString::CreateFromInt32((UINT16)eStyleFam);
                    aFam.Expand(5);

                    aStyleName += sal_Unicode('|');
                    aStyleName += aFam;

                    BOOL bFnd(FALSE);
                    UINT32 nNum(aStyles.Count());

                    while(!bFnd && nNum > 0)
                    {
                        // kein StyleSheet doppelt!
                        nNum--;
                        bFnd = (aStyleName.Equals(*(XubString*)aStyles.GetObject(nNum)));
                    }

                    if(!bFnd)
                    {
                        aStyles.Insert(new XubString(aStyleName), CONTAINER_APPEND);
                    }
                }
            }
        }

        // nun die Strings im Container durch StyleSheet* ersetzten
        ULONG nNum=aStyles.Count();
        while (nNum>0) {
            nNum--;
            XubString* pName=(XubString*)aStyles.GetObject(nNum);

            // UNICODE: String aFam(pName->Cut(pName->Len()-6));
            String aFam = pName->Copy(0, pName->Len() - 6);

            aFam.Erase(0,1);
            aFam.EraseTrailingChars();

            // UNICODE: USHORT nFam=USHORT(aFam);
            UINT16 nFam = (UINT16)aFam.ToInt32();

            SfxStyleFamily eFam=(SfxStyleFamily)nFam;
            SfxStyleSheetBase* pStyleBase=pStylePool->Find(*pName,eFam);
            SfxStyleSheet* pStyle=PTR_CAST(SfxStyleSheet,pStyleBase);
            delete pName;
            if (pStyle!=NULL && pStyle!=GetStyleSheet()) {
                aStyles.Replace(pStyle,nNum);
            } else {
                aStyles.Remove(nNum);
            }
        }
        // jetzt alle ueberfluessigen StyleSheets entfernen
        nNum=GetBroadcasterCount();
        while (nNum>0) {
            nNum--;
            SfxBroadcaster* pBroadcast=GetBroadcasterJOE((USHORT)nNum);
            SfxStyleSheet* pStyle=PTR_CAST(SfxStyleSheet,pBroadcast);
            if (pStyle!=NULL && pStyle!=GetStyleSheet()) { // Sonderbehandlung fuer den StyleSheet des Objekts
                if (aStyles.GetPos(pStyle)==CONTAINER_ENTRY_NOTFOUND) {
                    EndListening(*pStyle);
                }
            }
        }
        // und schliesslich alle in aStyles enthaltenen StyleSheets mit den vorhandenen Broadcastern mergen
        nNum=aStyles.Count();
        while (nNum>0) {
            nNum--;
            SfxStyleSheet* pStyle=(SfxStyleSheet*)aStyles.GetObject(nNum);
            // StartListening soll selbst nachsehen, ob hier nicht evtl. schon gehorcht wird
            StartListening(*pStyle,TRUE);
        }
    }
}

void SdrTextObj::NbcResizeTextAttributes(const Fraction& xFact, const Fraction& yFact)
{
    OutlinerParaObject* pOutlinerParaObject = GetOutlinerParaObject();
    if (pOutlinerParaObject!=NULL && xFact.IsValid() && yFact.IsValid())
    {
        Fraction n100(100,1);
        long nX=long(xFact*n100);
        long nY=long(yFact*n100);
        if (nX<0) nX=-nX;
        if (nX<1) nX=1;
        if (nX>0xFFFF) nX=0xFFFF;
        if (nY<0) nY=-nY;
        if (nY<1) nY=1;
        if (nY>0xFFFF) nY=0xFFFF;
        if (nX!=100 || nY!=100)
        {
            // Rahmenattribute
            const SfxItemSet& rSet = GetObjectItemSet();
            const SvxCharScaleWidthItem& rOldWdt=(SvxCharScaleWidthItem&)rSet.Get(EE_CHAR_FONTWIDTH);
            const SvxFontHeightItem& rOldHgt=(SvxFontHeightItem&)rSet.Get(EE_CHAR_FONTHEIGHT);

            // erstmal die alten Werte holen
            long nRelWdt=rOldWdt.GetValue();
            long nAbsHgt=rOldHgt.GetHeight();
            long nRelHgt=rOldHgt.GetProp();

            // Relative Breite aendern
            nRelWdt*=nX;
            nRelWdt/=nY;
            if (nRelWdt<0) nRelWdt=-nRelWdt; // nicht negativ
            if (nRelWdt<=0) nRelWdt=1;       // und mind. 1%
            if (nRelWdt>0xFFFF) nRelWdt=0xFFFF;

            // Absolute Hoehe aendern
            nAbsHgt*=nY;
            nAbsHgt/=100;
            if (nAbsHgt<0) nAbsHgt=-nAbsHgt; // nicht negativ
            if (nAbsHgt<=0) nAbsHgt=1;       // und mind. 1
            if (nAbsHgt>0xFFFF) nAbsHgt=0xFFFF;

            // und nun attributieren
            SetObjectItem(SvxCharScaleWidthItem( (USHORT) nRelWdt, EE_CHAR_FONTWIDTH));
            SetObjectItem(SvxFontHeightItem(nAbsHgt,(USHORT)nRelHgt, EE_CHAR_FONTHEIGHT));
            // Zeichen- und Absatzattribute innerhalb des OutlinerParaObjects
            Outliner& rOutliner=ImpGetDrawOutliner();
            rOutliner.SetPaperSize(Size(LONG_MAX,LONG_MAX));
            rOutliner.SetText(*pOutlinerParaObject);
            rOutliner.DoStretchChars((USHORT)nX,(USHORT)nY);
            OutlinerParaObject* pNewPara=rOutliner.CreateParaObject();
            NbcSetOutlinerParaObject(pNewPara);
            rOutliner.Clear();
        }
    }
}

/** #103836# iterates over the paragraphs of a given SdrObject and removes all
             hard set character attributes with the which ids contained in the
             given vector
*/
void SdrTextObj::RemoveOutlinerCharacterAttribs( const std::vector<sal_uInt16>& rCharWhichIds )
{
    sal_Int32 nText = getTextCount();

    while( --nText >= 0 )
    {
        SdrText* pText = getText( nText );
        OutlinerParaObject* pOutlinerParaObject = pText ? pText->GetOutlinerParaObject() : 0;

        if(pOutlinerParaObject)
        {
            Outliner* pOutliner = 0;

            if( pEdtOutl || (pText == getActiveText()) )
                pOutliner = pEdtOutl;

            if(!pOutliner)
            {
                pOutliner = &ImpGetDrawOutliner();
                pOutliner->SetText(*pOutlinerParaObject);
            }

            ESelection aSelAll( 0, 0, 0xffff, 0xffff );
            std::vector<sal_uInt16>::const_iterator aIter( rCharWhichIds.begin() );
            while( aIter != rCharWhichIds.end() )
            {
                pOutliner->RemoveAttribs( aSelAll, false, (*aIter++) );
            }

            if(!pEdtOutl || (pText != getActiveText()) )
            {
                const sal_uInt32 nParaCount = pOutliner->GetParagraphCount();
                OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
                pOutliner->Clear();
                NbcSetOutlinerParaObjectForText(pTemp, pText);
            }
        }
    }
}

bool SdrTextObj::HasText() const
{
    if( pEdtOutl )
        return HasEditText();

    OutlinerParaObject* pOPO = GetOutlinerParaObject();

    bool bHasText = false;
    if( pOPO )
    {
        const EditTextObject& rETO = pOPO->GetTextObject();
        USHORT nParaCount = rETO.GetParagraphCount();

        if( nParaCount > 0 )
            bHasText = (nParaCount > 1) || (rETO.GetText( 0 ).Len() != 0);
    }

    return bHasText;
}
