/*************************************************************************
 *
 *  $RCSfile: svdoattr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:25 $
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

#include "svdoattr.hxx"
#include "xpool.hxx"
#include "svditext.hxx"
#include "svdtouch.hxx"
#include "svdio.hxx"
#include "svdmodel.hxx"
#include "svdxout.hxx"
#include "svdpage.hxx"
#include "svdattr.hxx"
#include "svdattrx.hxx" // NotPersistItems
#include "svdpool.hxx"
#include "svdotext.hxx"
#include "svdocapt.hxx"
#include "svdograf.hxx" // fuer GetAttributes: Keine Linie, keine Flaeche
#include "svdoole2.hxx" // fuer GetAttributes: Keine Linie, keine Flaeche
#include "svdorect.hxx" // fuer GetAttributes: Eckenradius
#include "svdocirc.hxx" // fuer GetAttributes: Eckenradius
#include "svdomeas.hxx" // fuer GetAttributes: Kein Fontwork, kein ContourFrame

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _XENUM_HXX //autogen
#include "xenum.hxx"
#endif

#ifndef _SVX_XLINEIT0_HXX //autogen
#include "xlineit0.hxx"
#endif

#ifndef _SVX_XLNSTWIT_HXX //autogen
#include "xlnstwit.hxx"
#endif

#ifndef _SVX_XLNEDWIT_HXX //autogen
#include "xlnedwit.hxx"
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include "xfillit0.hxx"
#endif

#ifndef _SVX_XFLBMTIT_HXX //autogen
#include "xflbmtit.hxx"
#endif

#ifndef _SVX_TEXTIT0_HXX //autogen
#include "xtextit0.hxx"
#endif

#ifndef _SVX_XFLBSTIT_HXX //autogen
#include "xflbstit.hxx"
#endif

#ifndef _SVX_XFLBTOXY_HXX //autogen
#include "xflbtoxy.hxx"
#endif

#ifndef _SVX_XFTSHIT_HXX //autogen
#include "xftshit.hxx"
#endif

#ifndef _EEITEMID_HXX
#include <eeitemid.hxx>
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include "colritem.hxx"
#endif

#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#ifndef _SVX_FWDTITEM_HXX //autogen
#include "fwdtitem.hxx"
#endif

#ifndef _SVX_XLNSTCIT_HXX //autogen
#include <xlnstcit.hxx>
#endif

#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _SVX_XLNCLIT_HXX //autogen
#include <xlnclit.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include <xflclit.hxx>
#endif

#ifndef _SVX_XLNTRIT_HXX //autogen
#include <xlntrit.hxx>
#endif

#ifndef _SVX_XFLTRIT_HXX //autogen
#include <xfltrit.hxx>
#endif

#ifndef _SVX_XFLHTIT_HXX //autogen
#include <xflhtit.hxx>
#endif

#ifndef _SVX_XLNEDIT_HXX //autogen
#include <xlnedit.hxx>
#endif

#ifndef _SVX_XLNEDCIT_HXX //autogen
#include <xlnedcit.hxx>
#endif

#ifndef _SVX_XLNSTIT_HXX //autogen
#include <xlnstit.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrAttrObj,SdrObject);

SdrAttrObj::SdrAttrObj():
    pLineAttr(NULL),
    pFillAttr(NULL),
    pTextAttr(NULL),
    pShadAttr(NULL),
    pOutlAttr(NULL),
    pMiscAttr(NULL),
    pStyleSheet(NULL)
{
}

SdrAttrObj::~SdrAttrObj()
{
    pLineAttr=(XLineAttrSetItem*)  ImpSetNewAttr(pLineAttr,NULL,FALSE);
    pFillAttr=(XFillAttrSetItem*)  ImpSetNewAttr(pFillAttr,NULL,FALSE);
    pTextAttr=(XTextAttrSetItem*)  ImpSetNewAttr(pTextAttr,NULL,FALSE);
    pShadAttr=(SdrShadowSetItem*)  ImpSetNewAttr(pShadAttr,NULL,FALSE);
    pOutlAttr=(SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,NULL,FALSE);
    pMiscAttr=(SdrMiscSetItem*)    ImpSetNewAttr(pMiscAttr,NULL,FALSE);
}

const Rectangle& SdrAttrObj::GetSnapRect() const
{
    if (bSnapRectDirty) {
        ((SdrAttrObj*)this)->RecalcSnapRect();
        ((SdrAttrObj*)this)->bSnapRectDirty=FALSE;
    }
    return aSnapRect;
}

void SdrAttrObj::operator=(const SdrObject& rObj)
{
    SdrObject::operator=(rObj);
    SdrObject* pObj=(SdrObject*)&rObj;
    SdrAttrObj* pAO=PTR_CAST(SdrAttrObj,pObj);
    if (pAO!=NULL) {
        pLineAttr=(const XLineAttrSetItem*)  ImpSetNewAttr(pLineAttr,pAO->pLineAttr);
        pFillAttr=(const XFillAttrSetItem*)  ImpSetNewAttr(pFillAttr,pAO->pFillAttr);
        pTextAttr=(const XTextAttrSetItem*)  ImpSetNewAttr(pTextAttr,pAO->pTextAttr);
        pShadAttr=(const SdrShadowSetItem*)  ImpSetNewAttr(pShadAttr,pAO->pShadAttr);
        pOutlAttr=(const SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,pAO->pOutlAttr);
        pMiscAttr=(const SdrMiscSetItem*)    ImpSetNewAttr(pMiscAttr,pAO->pMiscAttr);
        RemoveStyleSheet();
        AddStyleSheet(pAO->pStyleSheet,TRUE);
    }
}

void SdrAttrObj::WriteData(SvStream& rOut) const
{
    SdrObject::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrAttrObj");
#endif

    SfxItemPool* pPool = GetItemPool();
    if (pPool!=NULL) {
        pPool->StoreSurrogate(rOut,pLineAttr);
        pPool->StoreSurrogate(rOut,pFillAttr);
        pPool->StoreSurrogate(rOut,pTextAttr);
        pPool->StoreSurrogate(rOut,pShadAttr);
        pPool->StoreSurrogate(rOut,pOutlAttr);
        pPool->StoreSurrogate(rOut,pMiscAttr);
    } else {
        rOut<<UINT16(SFX_ITEMS_NULL);
        rOut<<UINT16(SFX_ITEMS_NULL);
        rOut<<UINT16(SFX_ITEMS_NULL);
        rOut<<UINT16(SFX_ITEMS_NULL);
        rOut<<UINT16(SFX_ITEMS_NULL);
        rOut<<UINT16(SFX_ITEMS_NULL);
    }

    // StyleSheet-Pointer als Name, Familie abspeichern
    // wenn kein StyleSheet vorhanden: leeren String speichern
    if(pStyleSheet)
    {
        // UNICODE: rOut << pStyleSheet->GetName();
        rOut.WriteByteString(pStyleSheet->GetName());

        rOut << (UINT16)(int)(pStyleSheet->GetFamily());
    }
    else
    {
        // UNICODE: rOut << String();
        rOut.WriteByteString(String());
    }
    // ab Version 1 wird der CharacterSet gespeichert
    // rOut << (INT16) ::GetSystemCharSet(); // und ab V11 nicht mehr
}

void SdrAttrObj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    if (rIn.GetError()!=0) return;
    SdrObject::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrAttrObj");
#endif
    SfxItemPool* pPool=GetItemPool();

    if (pPool!=NULL) {
        USHORT nWhichRef;
        if (rHead.GetVersion()<11) { UINT16 nWhichDum; rIn>>nWhichDum; } // ab V11 keine WhichId mehr
        pLineAttr=(const XLineAttrSetItem*)ImpSetNewAttr(pLineAttr,NULL); // ggf altes rauswerfen
        nWhichRef=XATTRSET_LINE;
        pLineAttr=(const XLineAttrSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);

        if (rHead.GetVersion()<11) { UINT16 nWhichDum; rIn>>nWhichDum; } // ab V11 keine WhichId mehr
        pFillAttr=(const XFillAttrSetItem*)ImpSetNewAttr(pFillAttr,NULL); // ggf altes rauswerfen
        nWhichRef=XATTRSET_FILL;
        pFillAttr=(const XFillAttrSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);

        if (rHead.GetVersion()<11) { UINT16 nWhichDum; rIn>>nWhichDum; } // ab V11 keine WhichId mehr
        pTextAttr=(const XTextAttrSetItem*)ImpSetNewAttr(pTextAttr,NULL); // ggf altes rauswerfen
        nWhichRef=XATTRSET_TEXT;
        pTextAttr=(const XTextAttrSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);

        if (rHead.GetVersion()<11) { UINT16 nWhichDum; rIn>>nWhichDum; } // ab V11 keine WhichId mehr
        pShadAttr=(const SdrShadowSetItem*)ImpSetNewAttr(pShadAttr,NULL); // ggf altes rauswerfen
        nWhichRef=SDRATTRSET_SHADOW;
        pShadAttr=(const SdrShadowSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);

        if (rHead.GetVersion()>=5) {
            if (rHead.GetVersion()<11) { UINT16 nWhichDum; rIn>>nWhichDum; } // ab V11 keine WhichId mehr
            pOutlAttr=(const SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,NULL); // ggf altes rauswerfen
            nWhichRef=SDRATTRSET_OUTLINER;
            pOutlAttr=(const SdrOutlinerSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);
        }

        if (rHead.GetVersion()>=6) {
            if (rHead.GetVersion()<11) { UINT16 nWhichDum; rIn>>nWhichDum; } // ab V11 keine WhichId mehr
            pMiscAttr=(const SdrMiscSetItem*)ImpSetNewAttr(pMiscAttr,NULL); // ggf altes rauswerfen
            nWhichRef=SDRATTRSET_MISC;
            pMiscAttr=(const SdrMiscSetItem*)pPool->LoadSurrogate(rIn,nWhichRef,0);
        }
    } else {
        // an den Surrogaten und ggf. auch Whiches vorbeiseeken
        USHORT nAnz=4; // ganz zu anfang waren es 4 SetItems
        if (rHead.GetVersion()>=5) nAnz++;
        if (rHead.GetVersion()>=6) nAnz++;
        nAnz*=sizeof(UINT16);
        if (rHead.GetVersion()<11) nAnz*=2; // ab V11 keine WhichId mehr
        rIn.SeekRel(nAnz);
    }

    // TextToContour: altes Format(Flag) in neues Format(Item) wandeln
    if (rHead.GetVersion()<=4 && pPool!=NULL) {
        SfxItemSet aSet(*pPool);
        aSet.Put(XFormTextStyleItem(XFT_NONE));
        NbcSetAttributes(aSet,FALSE);
    }

    // Fuer die StyleSheetgeschichte gehoert eigentlich auch noch eine
    // Versionsabfrage hierher.
    // Name und Familie des StyleSheet einlesen, in Pointer auf StyleSheet
    // umwandeln lassen (SB)
    XubString aStyleSheetName;
    SfxStyleFamily eFamily;
    UINT16 nRead;

    // UNICODE: rIn>>aStyleSheetName;
    rIn.ReadByteString(aStyleSheetName);

    if(aStyleSheetName.Len())
    {
        rIn >> nRead;
        eFamily = (SfxStyleFamily)(int)nRead;

        // ab Version 1 wird der CharacterSet gelesen, ab V11 nicht mehr
        if(rHead.GetVersion() > 0 && rHead.GetVersion() < 11)
        {
            INT16 nCharSet;
            rIn >> nCharSet;
            //aStyleSheetName.Convert((CharSet)nCharSet);
            // nicht mehr noetig, da ab Vers 11 der CharSet bereits am
            // Stream gesetzt wird.
        }

        DBG_ASSERT( pModel!=NULL, "SdrAttrObj::ReadData(): pModel=NULL, StyleSheet kann nicht gesetzt werden!");
        if (pModel!=NULL) {
            SfxStyleSheetBasePool* pPool=pModel->GetStyleSheetPool();
#ifdef DBG_UTIL
            if( pPool == NULL )
                DBG_WARNING("SdrAttrObj::ReadData(): kein StyleSheetPool");
#endif
            if (pPool!=NULL) {
                SfxStyleSheet* pTmpStyleSheet=(SfxStyleSheet*)pPool->Find(aStyleSheetName,eFamily);
#ifdef DBG_UTIL
                if(pTmpStyleSheet == NULL)
                    DBG_WARNING("SdrAttrObj::ReadData(): StyleSheet nicht gefunden");
#endif
                if (pTmpStyleSheet!=NULL) {
                    StartListening(*pPool);
                    StartListening(*pTmpStyleSheet);
                    SfxItemSet* pParent=&pTmpStyleSheet->GetItemSet();
                    if (pLineAttr!=NULL) ((SfxItemSet*)&pLineAttr->GetItemSet())->SetParent(pParent);
                    if (pFillAttr!=NULL) ((SfxItemSet*)&pFillAttr->GetItemSet())->SetParent(pParent);
                    if (pTextAttr!=NULL) ((SfxItemSet*)&pTextAttr->GetItemSet())->SetParent(pParent);
                    if (pShadAttr!=NULL) ((SfxItemSet*)&pShadAttr->GetItemSet())->SetParent(pParent);
                    if (pOutlAttr!=NULL) ((SfxItemSet*)&pOutlAttr->GetItemSet())->SetParent(pParent);
                    if (pMiscAttr!=NULL) ((SfxItemSet*)&pMiscAttr->GetItemSet())->SetParent(pParent);
                    // ausnahmsweise die harte Attributierung nicht loeschen
                    //AddStyleSheet(pTmpStyleSheet,TRUE);
                    pStyleSheet=pTmpStyleSheet;
                }
            }
        }
    }
}

SfxItemPool* SdrAttrObj::ImpGetItemPool(const SfxPoolItem* pAlternateItem1, const SfxPoolItem* pAlternateItem2) const
{
    SfxItemPool* pPool= (pModel==NULL) ? NULL : &pModel->GetItemPool();
    if (pPool==NULL) {
        SfxSetItem* pSI;
        if (pAlternateItem1!=NULL) {
            pSI=PTR_CAST(SfxSetItem,pAlternateItem1);
            if (pSI!=NULL) pPool=(SfxItemPool*)pSI->GetItemSet().GetPool();
        }
        if (pPool==NULL && pAlternateItem2!=NULL) {
            pSI=PTR_CAST(SfxSetItem,pAlternateItem2);
            if (pSI!=NULL) pPool=(SfxItemPool*)pSI->GetItemSet().GetPool();
        }
    }
    return pPool;
}

SfxItemPool* SdrAttrObj::ImpGetItemPool() const
{
    SfxItemPool* pPool= (pModel==NULL) ? NULL : &pModel->GetItemPool();
    if (pPool==NULL) {
        USHORT nAnz=GetSetItemCount();
        for (USHORT i=0; i<nAnz && pPool==NULL; i++) {
            const SfxSetItem* pSI=GetSetItem(i);
            if (pSI!=NULL) pPool=pSI->GetItemSet().GetPool();
        }
    }
    return pPool;
}

const SfxPoolItem* SdrAttrObj::ImpSetNewAttr(const SfxPoolItem* pAkt, const SfxPoolItem* pNew, FASTBOOL bChg)
{
    SfxItemPool* pPool=ImpGetItemPool(pAkt,pNew);
    if (pPool!=NULL) {
        if (pAkt!=NULL) pPool->Remove(*pAkt);
        if (pNew!=NULL) pNew=&pPool->Put(*pNew);
    }
    return pNew;
}

static void ImpScaleItemSet(SfxItemSet& rSet, const Fraction& rScale)
{
    long nMul=rScale.GetNumerator();
    long nDiv=rScale.GetDenominator();
    if (!rScale.IsValid() || nDiv==0) return;
    SfxWhichIter aIter(rSet);
    USHORT nWhich=aIter.FirstWhich();
    while (nWhich!=0) {
        const SfxPoolItem *pItem=NULL;
        if (rSet.GetItemState(nWhich,FALSE,&pItem)==SFX_ITEM_SET) {
            if (pItem->HasMetrics()) {
                SfxPoolItem* pNewItem=pItem->Clone();
                pNewItem->ScaleMetrics(nMul,nDiv);
                rSet.Put(*pNewItem);
            }
        } // if (rSet.GetItemState(...)==SFX_ITEM_SET)
        nWhich=aIter.NextWhich();
    }
}

void SdrAttrObj::SetModel(SdrModel* pNewModel)
{
    SdrModel* pOldModel=pModel;
    SdrObject::SetModel(pNewModel);
    if (pOldModel!=pNewModel && pNewModel!=NULL && !pNewModel->IsLoading()) {
        // fuer ein frisch eingefuegtes Obj die Defaultattribute setzen
        if (pOldModel==NULL) ForceDefaultAttr(&pModel->GetItemPool());
        // fuer ein bereits "lebendes" Model die Attribute von einem Pool in den anderen schieben
        if (pOldModel!=NULL) {
            // Checken, ob sich die ScaleUnit geaendert hat.
            // Denn dann muessen naemlich alle MetrikItems umgerechnet werden.
            MapUnit aOldUnit(pOldModel->GetScaleUnit());
            MapUnit aNewUnit(pNewModel->GetScaleUnit());
            FASTBOOL bScaleUnitChanged=aNewUnit!=aOldUnit;
            Fraction aMetricFactor;
            if (bScaleUnitChanged) aMetricFactor=GetMapFactor(aOldUnit,aNewUnit).X();
            // Und nun alle Items auf die das Obj verweisst aus
            // dem alten Pools raus und in den neuen rein.
            SdrItemPool& rOldPool=(SdrItemPool&)pOldModel->GetItemPool();
            SdrItemPool& rNewPool=(SdrItemPool&)pModel->GetItemPool();

            DBG_ASSERT(&rOldPool!=NULL,"SdrAttrObj::SetModel(): Kein ItemPool am alten Model");
            DBG_ASSERT(&rNewPool!=NULL,"SdrAttrObj::SetModel(): Kein ItemPool am neuen Model");

            USHORT nItemSetAnz=GetSetItemCount();
            for (USHORT i=0; i<nItemSetAnz; i++) {
                const SfxSetItem* pSI=GetSetItem(i);
                if (pSI!=NULL) {
                    const SfxSetItem* pNewAttr=NULL;
                    if (!bScaleUnitChanged) {
                        pNewAttr=&(const SfxSetItem&)rNewPool.Put(*pSI);
                    } else  { // ggf. alle MetrikItems Skalieren
                        SfxSetItem* pNewSetItem=(SfxSetItem*)pSI->Clone(&rNewPool);
                        ImpScaleItemSet(pNewSetItem->GetItemSet(),aMetricFactor);
                        pNewAttr=&(const SfxSetItem&)rNewPool.Put(*pNewSetItem);
                    }
                    rOldPool.Remove(*pSI);
                    SetSetItem(i,pNewAttr);
                }
            }

            // ***** StyleSheets Anfang *****
            // ggfs. StyleSheet und dessen Parents kopieren
            // Follows werden nicht beruecksichtigt (ganz wie im Writer)
            if (pStyleSheet!=NULL) {
                SfxStyleSheetBase*     pSheet  =pStyleSheet;
                SfxStyleSheetBasePool* pOldPool=pOldModel->GetStyleSheetPool();
                SfxStyleSheetBasePool* pNewPool=pModel->GetStyleSheetPool();
                DBG_ASSERT(pOldPool!=NULL,"SdrAttrObj::SetModel(): Objekt hat StyleSheet aber keinen StyleSheetPool am SdrModel");
                if (pOldPool!=NULL) {
                    if (pNewPool!=NULL) {
                        // Liste der zu kopierenden Vorlagen
                        List aList;
                        SfxStyleSheetBase* pAnchor=NULL;
                        if (pOldPool!=NULL && pNewPool!=NULL) {
                            while (pSheet!=NULL) {
                                pAnchor=pNewPool->Find(pSheet->GetName(),pSheet->GetFamily());
                                if (pAnchor==NULL) {
                                    aList.Insert(pSheet,LIST_APPEND);
                                    pSheet=pOldPool->Find(pSheet->GetParent(),pSheet->GetFamily());
                                } else {
                                    pSheet=NULL;      // die gesuchte Vorlage gibt's schon
                                }
                            }
                        }

                        // kopieren und Parents der Kopien setzen
                        pSheet=(SfxStyleSheetBase*)aList.First();
                        SfxStyleSheetBase* pNewSheet=NULL;
                        SfxStyleSheetBase* pLastSheet=NULL;
                        SfxStyleSheetBase* pForThisObject=NULL;
                        if (pNewPool!=NULL) {
                            while (pSheet!=NULL) {
                                pNewSheet = &pNewPool->Make(pSheet->GetName(),
                                                            pSheet->GetFamily(),
                                                            pSheet->GetMask());
                                pNewSheet->GetItemSet().Put(pSheet->GetItemSet(),FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
                                if (bScaleUnitChanged) { // ggf. alle MetrikItems Skalieren
                                    ImpScaleItemSet(pNewSheet->GetItemSet(),aMetricFactor);
                                }
                                if (pLastSheet!=NULL) {
                                    pLastSheet->SetParent(pNewSheet->GetName());
                                }

                                if (pForThisObject==NULL)
                                    pForThisObject=pNewSheet;  // den kriegt das Objekt

                                pLastSheet=pNewSheet;
                                pSheet=(SfxStyleSheetBase*)aList.Next();
                            }
                        }

                        // Veknuepfung mit der im Zielpool gefundenen Vorlage
                        if (pAnchor!=NULL && pLastSheet!=NULL)
                            pLastSheet->SetParent(pAnchor->GetName());

                        // falls die Liste leer war (alle Vorlagen schon im Zielpool
                        // vorhanden) ist pForThisObject noch nicht gesetzt
                        if (pForThisObject==NULL && pAnchor!=NULL)
                            pForThisObject=pAnchor;

                        // am alten StyleSheet ab- und am neuen anmelden
                        RemoveStyleSheet();
                        AddStyleSheet((SfxStyleSheet*)pForThisObject,TRUE);
                    } else { // if (pNewPool!=NULL)
                        // Aha, im neuen Model gibt's also kein StyleSheetPool
                        // also setzte ich "einfach" alle Attribute des alten StyleSheets hart
                        List aList;
                        const SfxItemSet* pItemSet=&pStyleSheet->GetItemSet();
                        while (pItemSet!=NULL) {
                            aList.Insert((void*)pItemSet,CONTAINER_APPEND);
                            pItemSet=pItemSet->GetParent();
                        }
                        SfxItemSet aSet(pNewModel->GetItemPool());
                        pItemSet=(SfxItemSet*)aList.Last();
                        while (pItemSet!=NULL) {
                            aSet.Put(*pItemSet);
                            pItemSet=(SfxItemSet*)aList.Prev();
                        }
                        // Attribute, die schon vorher hart gesetzt
                        // waren muessen aber erhalten bleiben:
                        for (USHORT i=0; i<nItemSetAnz; i++) {
                            const SfxSetItem* pSI=GetSetItem(i);
                            if (pSI!=NULL) {
                                const SfxItemSet& rIS=pSI->GetItemSet();
                                SfxWhichIter aIter(rIS);
                                USHORT nWhich=aIter.FirstWhich();
                                while (nWhich!=0) {
                                    if (rIS.GetItemState(nWhich,FALSE)==SFX_ITEM_SET) {
                                        aSet.ClearItem(nWhich);
                                    }
                                    nWhich=aIter.NextWhich();
                                }
                            }
                        }
                        if (bScaleUnitChanged) { // ggf. alle MetrikItems Skalieren
                            ImpScaleItemSet(aSet,aMetricFactor);
                        }
                        SetAttributes(aSet,FALSE);
                        RemoveStyleSheet();
                    } // if (pNewPool!=NULL)
                } // if (pOldPool!=NULL)
            } // if (pStyleSheet!=NULL)
            // ***** StyleSheets Ende *****
        }
        // Jedes Objekt bekommt initial den DefaultStyleSheet
        // des Model, falls noch kein StyleSheet gesetzt.
        // Der DefaultStyleSheet wird jedoch nur gesetzt, wenn das Objekt auch
        // Attribute hat (Entscheidung anhand pLineAttr!=NULL). 3D-Objekte
        // haben naemlich z.T. keine eigenen Attribute. (Joe M. 04-12-1996)
        if (pLineAttr!=NULL && pStyleSheet==NULL && pModel!=NULL && !pModel->IsLoading()) {
            SfxStyleSheet* pDefSS=pModel->GetDefaultStyleSheet();
            if (pDefSS!=NULL) {
                // Dabei jedoch nicht die harte Attributierung loeschen!
                AddStyleSheet(pDefSS,TRUE);
            }
        }
    } // if (pOldModel!=pNewModel && pNewModel!=NULL && !pNewModel->IsLoading())
}

void SdrAttrObj::ForceDefaultAttr(SfxItemPool* pPool)
{
    if (pPool!=NULL && pLineAttr==NULL) {
        SdrTextObj* pText=PTR_CAST(SdrTextObj,this);
        SdrCaptionObj* pCapt=PTR_CAST(SdrCaptionObj,this);
        FASTBOOL bTextFrame=pText!=NULL && pText->IsTextFrame();
        FASTBOOL bCaption=pCapt!=NULL;

#ifdef SDRDEFITEMCACHE
        // SdrItemPool in der Verkettung suchen (wg. Writer)
        SdrItemPool* pSdrPool=NULL;
        SfxItemPool* pTmpPool=pPool;
        while (pTmpPool!=NULL && pSdrPool==NULL) {
            // Etwas komplizierte Abfrage und auch Hack, weil am
            // Pool kein RTTI vorhanden ist! Joe, 02-09-1997
            if (pTmpPool->GetFirstWhich()<=SDRATTR_START && pTmpPool->GetLastWhich()>=SDRATTR_END) {
                if (pTmpPool->GetName().EqualsAscii("XOutdevItemPool"))
                    pSdrPool=(SdrItemPool*)pTmpPool;
            }
            pTmpPool=pTmpPool->GetSecondaryPool();
        }
        DBG_ASSERT(pSdrPool!=NULL,"SdrAttrObj::ForceDefaultAttr(): Kein SdrItemPool gefunden!");
        if (pSdrPool->pDefLineAttr==NULL) pSdrPool->ImpMakeDefItems();
        if (bTextFrame) {
            pLineAttr=bCaption ? pSdrPool->pDefLineAttr : pSdrPool->pDefTextFrameLineAttr;
            pFillAttr=pSdrPool->pDefTextFrameFillAttr;
            pTextAttr=pSdrPool->pDefTextFrameTextAttr;
            pShadAttr=pSdrPool->pDefTextFrameShadAttr;
            pOutlAttr=pSdrPool->pDefTextFrameOutlAttr;
            pMiscAttr=pSdrPool->pDefTextFrameMiscAttr;
        } else {
            pLineAttr=pSdrPool->pDefLineAttr;
            pFillAttr=pSdrPool->pDefFillAttr;
            pTextAttr=pSdrPool->pDefTextAttr;
            pShadAttr=pSdrPool->pDefShadAttr;
            pOutlAttr=pSdrPool->pDefOutlAttr;
            pMiscAttr=pSdrPool->pDefMiscAttr;
        }
        pLineAttr->AddRef();
        pFillAttr->AddRef();
        pTextAttr->AddRef();
        pShadAttr->AddRef();
        pOutlAttr->AddRef();
        pMiscAttr->AddRef();
#else // SDRDEFITEMCACHE
        if (pLineAttr==NULL) {
            XLineAttrSetItem aSetItem(pPool);
            if (bTextFrame && !bCaption) {
                 aSetItem.GetItemSet().Put(XLineStyleItem(XLINE_NONE));
            }
            pLineAttr=(XLineAttrSetItem*)ImpSetNewAttr(pLineAttr,&aSetItem,FALSE);
        }
        if (pFillAttr==NULL) {
            XFillAttrSetItem aSetItem(pPool);
            if (bTextFrame) {
                aSetItem.GetItemSet().Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
                aSetItem.GetItemSet().Put(XFillStyleItem(XFILL_NONE));
            }
            pFillAttr=(XFillAttrSetItem*)ImpSetNewAttr(pFillAttr,&aSetItem,FALSE);
        }
        if (pTextAttr==NULL) {
            XTextAttrSetItem aSetItem(pPool);
            pTextAttr=(XTextAttrSetItem*)ImpSetNewAttr(pTextAttr,&aSetItem,FALSE);
        }
        if (pShadAttr==NULL) {
            SdrShadowSetItem aSetItem(pPool);
            pShadAttr=(SdrShadowSetItem*)ImpSetNewAttr(pShadAttr,&aSetItem,FALSE);
        }
        if (pOutlAttr==NULL) {
            SdrOutlinerSetItem aSetItem(pPool);
            if (!bTextFrame) {
                aSetItem.GetItemSet().Put(SvxAdjustItem(SVX_ADJUST_CENTER));
            }
            pOutlAttr=(SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,&aSetItem,FALSE);
        }
        if (pMiscAttr==NULL) {
            SdrMiscSetItem aSetItem(pPool);
            if (!bTextFrame) {
                aSetItem.GetItemSet().Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
                aSetItem.GetItemSet().Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
            }
            pMiscAttr=(SdrMiscSetItem*)ImpSetNewAttr(pMiscAttr,&aSetItem,FALSE);
        }
#endif // SDRDEFITEMCACHE
    }
}

void SdrAttrObj::TakeAttributes(SfxItemSet& rAttr, FASTBOOL bMerge, FASTBOOL bOnlyHardAttr) const
{
    USHORT nAnz=GetSetItemCount();
    FASTBOOL bHasText=GetOutlinerParaObject()!=NULL;
    SdrTextObj* pTextObj=PTR_CAST(SdrTextObj,this);
    FASTBOOL bIsContourFrame=pTextObj!=NULL && pTextObj->IsContourTextFrame();
    SdrMeasureObj* pMeasObj=PTR_CAST(SdrMeasureObj,this);
    if (!bHasText && pTextObj!=NULL && pTextObj->IsTextEditActive()) bHasText=TRUE;
    FASTBOOL bTextFrame=pTextObj!=NULL && pTextObj->IsTextFrame();
    FASTBOOL bCanFontwork=bHasText && pTextObj!=NULL && !bTextFrame && pMeasObj==NULL;
    FASTBOOL bCanContourFrame=bHasText && pTextObj!=NULL && !bTextFrame;
    FASTBOOL bGrafObj=HAS_BASE(SdrGrafObj,this) || HAS_BASE(SdrOle2Obj,this);
    FASTBOOL bHasEckRad=!bGrafObj && !bIsUnoObj && HAS_BASE(SdrRectObj,this) && !HAS_BASE(SdrCircObj,this);
    FASTBOOL bHasFill=bClosedObj;
    FASTBOOL bHasLEnd=!bClosedObj || HAS_BASE(SdrCaptionObj,this);
    for (USHORT i=0; i<nAnz; i++) {
        const SfxSetItem* pSI=GetSetItem(i);
        FASTBOOL bOk=pSI!=NULL;
        if (pSI==pFillAttr && !bHasFill) bOk=FALSE;
        if (pSI==pTextAttr && !bCanFontwork) bOk=FALSE;
        if (pSI==pOutlAttr && !bHasText) bOk=FALSE;
        if (bGrafObj && (pSI==pFillAttr || pSI==pLineAttr)) bOk=FALSE; // fuer Grafik und OLE
        if (pSI!=NULL && !bMerge && bOnlyHardAttr) bOk=TRUE; // z.B. im Falle Undo
        if (bOk) {
            const SfxItemSet& rIS=pSI->GetItemSet();
            SfxWhichIter aIter(rIS);
            USHORT nWhich=aIter.FirstWhich();
            while (nWhich!=0) {
                if ((bHasLEnd || nWhich<XATTR_LINESTART || nWhich>XATTR_LINEENDCENTER) &&
                    (bHasEckRad || nWhich!=SDRATTR_ECKENRADIUS) &&
                    (bHasText || nWhich<SDRATTR_TEXT_MINFRAMEHEIGHT || nWhich>SDRATTR_TEXT_CONTOURFRAME) &&
                    (bTextFrame || (nWhich!=SDRATTR_TEXT_AUTOGROWHEIGHT && nWhich!=SDRATTR_TEXT_MINFRAMEHEIGHT && nWhich!=SDRATTR_TEXT_MAXFRAMEHEIGHT &&
                                    nWhich!=SDRATTR_TEXT_AUTOGROWWIDTH && nWhich!=SDRATTR_TEXT_MINFRAMEWIDTH && nWhich!=SDRATTR_TEXT_MAXFRAMEWIDTH)) &&
                    (bCanContourFrame || nWhich!=SDRATTR_TEXT_CONTOURFRAME) &&
                    (!bIsContourFrame || nWhich<SDRATTR_TEXT_MINFRAMEHEIGHT || nWhich>SDRATTR_TEXT_HORZADJUST) &&
                    (nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) )
                {
                    if (!bOnlyHardAttr || rIS.GetItemState(nWhich,FALSE)==SFX_ITEM_SET) {
                        const SfxPoolItem& rItem=rIS.Get(nWhich);
                        if (bMerge) rAttr.MergeValue(rItem,TRUE);
                        else rAttr.Put(rItem);
                    }
                }
                nWhich=aIter.NextWhich();
            }
        }
    }
    if (!bMerge && pLineAttr!=NULL) {
        rAttr.SetParent(pLineAttr->GetItemSet().GetParent());
    }
}

void SdrAttrObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
{
    SfxItemPool* pPool=GetItemPool();
    if (pPool==NULL) pPool=(SfxItemPool*)rAttr.GetPool();
    ForceDefaultAttr(pPool);
    USHORT nAnz=GetSetItemCount();

    if (bReplaceAll) {
        SfxWhichIter aWhIter(rAttr);
        for (USHORT i=0; i<nAnz; i++) {
            const SfxSetItem* pOldSI=GetSetItem(i);
            if (pOldSI!=NULL) {
                const SfxItemSet& rOldIS=pOldSI->GetItemSet();
                const USHORT* pRanges=rOldIS.GetRanges();
                FASTBOOL bOneRange=pRanges[2]==0;
                USHORT nAnf=pRanges[0];
                USHORT nEnd=pRanges[1];
                SfxSetItem* pNewSI=NULL; //MakeNewSetItem(i,TRUE);
                SfxItemSet* pDstSet=NULL; //pNewSI->GetItemSet();
                // funkt nicht:
                // pNewSI->GetItemSet().Set(rAttr);
                // deshalb manuell #36313#
                pNewSI=MakeNewSetItem(i,TRUE);
                pDstSet=&pNewSI->GetItemSet();
                USHORT nWhich=aWhIter.FirstWhich();
                while (nWhich!=0) {
                    const SfxPoolItem* pItem=NULL;
                    SfxItemState eState=rAttr.GetItemState(nWhich,TRUE,&pItem);
                    if (eState==SFX_ITEM_DEFAULT) {
                        pDstSet->ClearItem(nWhich);
                    } else if (eState==SFX_ITEM_SET) {
                        if (nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) {
                            pDstSet->Put(*pItem);
                        }
                    }
                    nWhich=aWhIter.NextWhich();
                }
                if (pNewSI!=NULL) {
                    SetSetItem(i,(SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI));
                    delete pNewSI;
                }
            }
        }
    } else {
        SfxItemIter aItIter(rAttr);
        for (USHORT i=0; i<nAnz; i++) {
            const SfxSetItem* pOldSI=GetSetItem(i);
            if (pOldSI!=NULL) {
                const SfxItemSet& rOldIS=pOldSI->GetItemSet();
                const USHORT* pRanges=rOldIS.GetRanges();
                FASTBOOL bOneRange=pRanges[2]==0;
                USHORT nAnf=pRanges[0];
                USHORT nEnd=pRanges[1];
                SfxSetItem* pNewSI=NULL; //MakeNewSetItem(i,TRUE);
                SfxItemSet* pDstSet=NULL; //pNewSI->GetItemSet();
                // funkt nicht:
                //pNewSI->GetItemSet().Put(rAttr,FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
                // deshalb manuell. (Bug 21223)
                const SfxPoolItem* pItem=aItIter.FirstItem();
                BOOL bFillAttrChangeHint(FALSE);

                while (pItem!=NULL) {
                    if (!IsInvalidItem(pItem)) {
                        USHORT nWhich=pItem->Which();
                        if ((nWhich<EE_FEATURE_START || nWhich>EE_FEATURE_END) &&
                            (!bOneRange || (nWhich>=nAnf && nWhich<=nEnd))) {
                            if (pNewSI==NULL) {
                                pNewSI=MakeNewSetItem(i,TRUE);
                                pDstSet=&pNewSI->GetItemSet();
                            }

                            // Hinweis auf Veraenderung FillStyle
                            if(i==1 && nWhich == XATTR_FILLSTYLE)
                                bFillAttrChangeHint = TRUE;

                            pDstSet->Put(*pItem);
                        }
                    }
                    pItem=aItIter.NextItem();
                }

                if (pNewSI!=NULL)
                {
                    if(bFillAttrChangeHint)
                    {
                        // FillStyle hat sich eventuell geaendert
                        XFillStyle eFillStyle = ((XFillStyleItem&)(rAttr.Get(XATTR_FILLSTYLE))).GetValue();

                        // Alle vom momentanen Stil nicht benutzten Items
                        // rauswerfen. Dies gilt NICHT fuer die Fuellfarbe
                        // (XFILL_SOLID). Diese wird als immer vorhandene
                        // Grundfarbe des Objektes angesehen (wird von 3D
                        // bereits so verwendet)

                        if(eFillStyle != XFILL_GRADIENT)
                            pDstSet->ClearItem(XATTR_FILLGRADIENT);

                        if(eFillStyle != XFILL_HATCH)
                            pDstSet->ClearItem(XATTR_FILLHATCH);

                        if(eFillStyle != XFILL_BITMAP)
                            pDstSet->ClearItem(XATTR_FILLBITMAP);
                    }

                    SetSetItem(i,(SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI));
                    delete pNewSI;
                }
            }
        }
    }
    bBoundRectDirty=TRUE;
    SetRectsDirty(TRUE);
}

void SdrAttrObj::SetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetAttributes(rAttr,bReplaceAll);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_CHGATTR,aBoundRect0);
}

void __EXPORT SdrAttrObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SfxSimpleHint *pSimple = PTR_CAST(SfxSimpleHint,&rHint);
    FASTBOOL bDataChg = pSimple != NULL && pSimple->GetId() == SFX_HINT_DATACHANGED;

    SfxStyleSheetHint *pStyleHint = PTR_CAST(SfxStyleSheetHint, &rHint);
    FASTBOOL bDying = pStyleHint != NULL && pStyleHint->GetHint() == SFX_STYLESHEET_INDESTRUCTION;

    if (bDataChg || bDying) {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SfxStyleSheet* pNewStSh=NULL;
        FASTBOOL bBoundRectDirty0=bBoundRectDirty;
        if (bDying) {
            // wenn es einen Parent gibt, wird jetzt der genommen
            if (HAS_BASE(SfxStyleSheet,pStyleSheet) && pModel!=NULL) { // Sonst ist pStyleSheet schon zu weit weggestorben
                pNewStSh=(SfxStyleSheet*)pModel->GetStyleSheetPool()->Find(pStyleSheet->GetParent(),pStyleSheet->GetFamily());
            }
            // wenn es keinen Parent gibt, nehmen wir die Standardvorlage
            if (pNewStSh==NULL && pModel!=NULL) {
                pNewStSh=pModel->GetDefaultStyleSheet();
            }
            // alten StyleSheet vor SendRepaintBroadcast entfernen #42276#
            RemoveStyleSheet(); // setzt bBoundRectDirty=TRUE
        }
        if (!bBoundRectDirty0) {
            bBoundRectDirty=FALSE;  // fuer Broadcast mit dem alten Rect
            if(pPage && pPage->IsInserted())
                SendRepaintBroadcast(); // Erstmal mit dem alten Rect
            bBoundRectDirty=TRUE;
            SetRectsDirty(TRUE);    // Durch Vorlagenloeschung evtl. andere Linienbreite
        }
        if (pNewStSh && !bDying)
            AddStyleSheet(pNewStSh,TRUE);

        if(pPage && pPage->IsInserted())
            SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_CHGATTR,aBoundRect0);
    }
}

SfxStyleSheet* SdrAttrObj::GetStyleSheet() const
{
    return pStyleSheet;
}

void SdrAttrObj::RemoveStyleSheet()
{
    // Typ checken, weil bei dying sich der Typ abbaut (bis zum Broadcaster runter)
    if (pStyleSheet!=NULL && HAS_BASE(SfxStyleSheet,pStyleSheet))
    {
        EndListening(*pStyleSheet);
        EndListening(pStyleSheet->GetPool());

        // das ItemSet der Vorlage ist jetzt nicht mehr Parent der
        // eigenen ItemSets
        USHORT nAnz=GetSetItemCount();
        for (USHORT i=0; i<nAnz; i++) {
            SfxSetItem* pNewSI=MakeNewSetItem(i,TRUE);
            if (pNewSI!=NULL) {
                pNewSI->GetItemSet().SetParent(NULL);
                const SfxSetItem* pOldSI=GetSetItem(i);
                // Alle Items aus der Vorlage von pOldSI nach pNewSI putten,
                // also hart Attributieren.
                // ... fehlende Implementation

                const SfxSetItem* pNewSI2=(const SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI);
                SetSetItem(i,pNewSI2);
                delete pNewSI;
            }
        }
        bBoundRectDirty=TRUE;
        SetRectsDirty(TRUE);
    }
    pStyleSheet=NULL;
}

USHORT SdrAttrObj::GetSetItemCount() const
{
    return 6;
}

const SfxSetItem* SdrAttrObj::GetSetItem(USHORT nNum) const
{
    switch (nNum)
    {
        case 0 : return pLineAttr;
        case 1 : return pFillAttr;
        case 2 : return pTextAttr;
        case 3 : return pShadAttr;
        case 4 : return pOutlAttr;
        case 5 : return pMiscAttr;
    }
    return NULL; // damit der Compiler nicht meckert
}

void SdrAttrObj::SetSetItem(USHORT nNum, const SfxSetItem* pAttr)
{
    switch (nNum)
    {
        case 0 : pLineAttr = (const XLineAttrSetItem*)  pAttr; break;
        case 1 : pFillAttr = (const XFillAttrSetItem*)  pAttr; break;
        case 2 : pTextAttr = (const XTextAttrSetItem*)  pAttr; break;
        case 3 : pShadAttr = (const SdrShadowSetItem*)  pAttr; break;
        case 4 : pOutlAttr = (const SdrOutlinerSetItem*)pAttr; break;
        case 5 : pMiscAttr = (const SdrMiscSetItem*)    pAttr; break;
    }
}

SfxSetItem* SdrAttrObj::MakeNewSetItem(USHORT nNum, FASTBOOL bClone) const
{
    if (bClone)
    {
        switch (nNum)
        {
            case 0 : return new XLineAttrSetItem(*pLineAttr);
            case 1 : return new XFillAttrSetItem(*pFillAttr);
            case 2 : return new XTextAttrSetItem(*pTextAttr);
            case 3 : return new SdrShadowSetItem(*pShadAttr);
            case 4 : return new SdrOutlinerSetItem(*pOutlAttr);
            case 5 : return new SdrMiscSetItem(*pMiscAttr);
        }
    }
    else
    {
        switch (nNum)
        {
            case 0 : return new XLineAttrSetItem(GetItemPool());
            case 1 : return new XFillAttrSetItem(GetItemPool());
            case 2 : return new XTextAttrSetItem(GetItemPool());
            case 3 : return new SdrShadowSetItem(GetItemPool());
            case 4 : return new SdrOutlinerSetItem(GetItemPool());
            case 5 : return new SdrMiscSetItem(GetItemPool());
        }
    }
    return NULL; // damit der Compiler nicht meckert
}

void SdrAttrObj::AddStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    pStyleSheet=pNewStyleSheet;
    if (pStyleSheet!=NULL)
    {
        // als Listener anmelden
        StartListening(pStyleSheet->GetPool());
        StartListening(*pStyleSheet);

        // harte Attributierung dort loeschen, wo was in der Vorlage steht?
        const SfxItemSet& rStyle=pNewStyleSheet->GetItemSet();
        // pWhichRangesInStyle enthaelt Zahlenpaare der Which-Ranges.
        // am Ende steht eine 0.
        const USHORT* pWhichRangesInStyle=rStyle.GetRanges();
        USHORT nAnz=GetSetItemCount();
        for (USHORT i=0; i<nAnz; i++) {
            const SfxSetItem* pOldSI=GetSetItem(i);
            SfxSetItem* pNewSI=MakeNewSetItem(i,pOldSI!=NULL);
            pNewSI->GetItemSet().SetParent(NULL);
            SfxItemSet& rIS=pNewSI->GetItemSet();

            USHORT  nIndex  =0;
            USHORT  nCurrent=pWhichRangesInStyle[nIndex];
            USHORT  nEnd    =pWhichRangesInStyle[nIndex+1];

            while (nCurrent!=0) {
                if (!bDontRemoveHardAttr && rStyle.GetItemState(nCurrent)==SFX_ITEM_SET) {
                    rIS.ClearItem(nCurrent);
                }

                nCurrent++;
                if (nCurrent>nEnd) {
                    nIndex+=2;
                    nCurrent=pWhichRangesInStyle[nIndex];
                    if (nCurrent!=0) {         // Ende der Ranges? nein
                        nEnd=pWhichRangesInStyle[nIndex+1];
                    }
                }
            }
            // ItemSet des StyleSheets als Parent der ItemSets der SetItems setzen
            rIS.SetParent(&(pNewStyleSheet->GetItemSet()));
            // die Zeiger auf die SetItems neu setzen
            SetSetItem(i,(const SfxSetItem*)ImpSetNewAttr(pOldSI,pNewSI));
            delete pNewSI;
        }
    }
}

void SdrAttrObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    RemoveStyleSheet();
    AddStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
    bBoundRectDirty=TRUE;
    SetRectsDirty(TRUE);
}

void SdrAttrObj::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
    SendRepaintBroadcast();
    NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);
    SetChanged();
    SendRepaintBroadcast();
    SendUserCall(SDRUSERCALL_CHGATTR,aBoundRect0);
}

INT32 SdrAttrObj::ImpGetLineWdt() const
{
    if (pLineAttr!=NULL) {
        const SfxItemSet& rSet=pLineAttr->GetItemSet();
        XLineStyle eLine=((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
        if (eLine==XLINE_NONE) return 0; // Garkeine Linie da.
        INT32 nWdt=((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        return nWdt;
    }
    return 0; // Defaultannahme ist: Haarlinie
}

INT32 SdrAttrObj::ImpGetLineEndAdd() const
{
    if(pLineAttr)
    {
        const SfxItemSet& rSet = pLineAttr->GetItemSet();
        BOOL bStartSet(TRUE);
        BOOL bEndSet(TRUE);

        if(rSet.GetItemState( XATTR_LINESTART ) != SFX_ITEM_DONTCARE)
        {
            String aStr(((const XLineStartItem&)rSet.Get(XATTR_LINESTART)).GetName());
            if(!aStr.Len())
                bStartSet = FALSE;
        }

        if(rSet.GetItemState( XATTR_LINEEND ) != SFX_ITEM_DONTCARE)
        {
            String aStr(((const XLineEndItem&)rSet.Get(XATTR_LINEEND)).GetName());
            if(!aStr.Len())
                bEndSet = FALSE;
        }

        BOOL bLineEndSet = bStartSet || bEndSet;

        XLineStyle eLine=((XLineStyleItem&)(rSet.Get(XATTR_LINESTYLE))).GetValue();
        if (eLine==XLINE_NONE) return 0; // Garkeine Linie da.
        INT32 nLineWdt=((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke

        long nSttWdt=((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
        if (nSttWdt<0) nSttWdt=-nLineWdt*nSttWdt/100; // <0 = relativ

        if(!bLineEndSet)
            nSttWdt = 0;

        FASTBOOL bSttCenter=((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
        long nSttHgt=0;
        if (bSttCenter) { // Linienende steht um die Haelfe ueber
            XPolygon aSttPoly(((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue());
            nSttHgt=XOutputDevice::InitLineStartEnd(aSttPoly,nSttWdt,bSttCenter);
            // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
        }
        nSttWdt++;
        nSttWdt/=2;
        // Lieber etwas mehr, dafuer keine Wurzel ziehen
        long nSttAdd=Max(nSttWdt,nSttHgt);
        nSttAdd*=3;
        nSttAdd/=2;

        long nEndWdt=((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
        if (nEndWdt<0) nEndWdt=-nLineWdt*nEndWdt/100; // <0 = relativ

        if(!bLineEndSet)
            nEndWdt = 0;

        FASTBOOL bEndCenter=((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
        long nEndHgt=0;
        if (bEndCenter) { // Linienende steht um die Haelfe ueber
            XPolygon aEndPoly(((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue());
            nEndHgt=XOutputDevice::InitLineStartEnd(aEndPoly,nEndWdt,bEndCenter);
            // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
        }
        nEndWdt++;
        nEndWdt/=2;
        // Lieber etwas mehr, dafuer keine Wurzel ziehen
        long nEndAdd=Max(nEndWdt,nEndHgt);
        nEndAdd*=3;
        nEndAdd/=2;

        return Max(nSttAdd,nEndAdd);
    }
    return 0; // Defaultannahme ist: Keine Linienenden
}

FASTBOOL SdrAttrObj::ImpLineEndHitTest(const Point& rEndPt, double nSin, double nCos, FASTBOOL bStart, const Point& rHit, USHORT nTol) const
{
    if (pLineAttr!=NULL) {
        const SfxItemSet& rSet=pLineAttr->GetItemSet();
        long nWdt=0;
        FASTBOOL bCenter=FALSE;
        //long nHgt=0;
        XPolygon aXPoly;
        if (bStart) {
            nWdt=((const XLineStartWidthItem&)(rSet.Get(XATTR_LINESTARTWIDTH))).GetValue();
            bCenter=((const XLineStartCenterItem&)(rSet.Get(XATTR_LINESTARTCENTER))).GetValue();
            aXPoly=((const XLineStartItem&)(rSet.Get(XATTR_LINESTART))).GetValue();
        } else {
            nWdt=((const XLineEndWidthItem&)(rSet.Get(XATTR_LINEENDWIDTH))).GetValue();
            bCenter=((const XLineEndCenterItem&)(rSet.Get(XATTR_LINEENDCENTER))).GetValue();
            aXPoly=((const XLineEndItem&)(rSet.Get(XATTR_LINEEND))).GetValue();
        }
        if (nWdt<0) {
            INT32 nLineWdt=((XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue(); // Strichstaerke
            nWdt=-nLineWdt*nWdt/100; // <0 = relativ
        }
        // InitLineStartEnd liefert bei bCenter=TRUE die halbe Hoehe
        /*nHgt=*/XOutputDevice::InitLineStartEnd(aXPoly,nWdt,bCenter);
        RotateXPoly(aXPoly,Point(),nSin,nCos);
        Point aHit(rHit);
        aHit-=rEndPt;
        Rectangle aHitRect(aHit.X()-nTol,aHit.Y()-nTol,aHit.X()+nTol,aHit.Y()+nTol);
        FASTBOOL bHit=IsRectTouchesPoly(XOutCreatePolygon(aXPoly,NULL),aHitRect);
        return bHit;
    }
    return FALSE;
}

FASTBOOL SdrAttrObj::ImpGetShadowDist(INT32& nXDist, INT32& nYDist) const
{
    nXDist=0;
    nYDist=0;
    if (pShadAttr!=NULL) {
        FASTBOOL bShadOn=((SdrShadowItem&)(pShadAttr->GetItemSet().Get(SDRATTR_SHADOW))).GetValue();
        if (bShadOn) {
            nXDist=((SdrShadowXDistItem&)(pShadAttr->GetItemSet().Get(SDRATTR_SHADOWXDIST))).GetValue();
            nYDist=((SdrShadowYDistItem&)(pShadAttr->GetItemSet().Get(SDRATTR_SHADOWYDIST))).GetValue();
            return TRUE;
        }
    }
    return FALSE;
}

void SdrAttrObj::ImpAddShadowToBoundRect()
{
    INT32 nXDist;
    INT32 nYDist;
    if (ImpGetShadowDist(nXDist,nYDist)) {
        if (nXDist>0) aOutRect.Right()+=nXDist;
        else aOutRect.Left()+=nXDist;
        if (nYDist>0) aOutRect.Bottom()+=nYDist;
        else aOutRect.Top()+=nYDist;
    }
}

FASTBOOL SdrAttrObj::ImpSetShadowAttributes(ExtOutputDevice& rXOut, FASTBOOL bNoFill) const
{
    if (pShadAttr!=NULL) {
        FASTBOOL bShadOn=((SdrShadowItem&)(pShadAttr->GetItemSet().Get(SDRATTR_SHADOW))).GetValue();
        if (bShadOn) {
// LineAttr for shadow no longer necessary, lines and line shadows are drawn in Paint()
// routines individually (grep for CreateLinePoly())
//
//          if (pLineAttr!=NULL) {
//              XLineAttrSetItem aL(*pLineAttr);
//              aL.GetItemSet().Put(XLineColorItem(String(),aShadCol));
//              aL.GetItemSet().Put(XLineTransparenceItem(nTransp));
//              rXOut.SetLineAttr(aL);
//          }
            if (!bNoFill && pFillAttr!=NULL) {
                const SdrShadowColorItem& rShadColItem=((SdrShadowColorItem&)(pShadAttr->GetItemSet().Get(SDRATTR_SHADOWCOLOR)));
                Color aShadCol(rShadColItem.GetValue());
                USHORT nTransp=((SdrShadowTransparenceItem&)(pShadAttr->GetItemSet().Get(SDRATTR_SHADOWTRANSPARENCE))).GetValue();
                XFillStyle eStyle=((XFillStyleItem&)(pFillAttr->GetItemSet().Get(XATTR_FILLSTYLE))).GetValue();
                XFillAttrSetItem aF(*pFillAttr);
                if (eStyle==XFILL_HATCH) { // #41666#
                    XHatch aHatch=((XFillHatchItem&)(pFillAttr->GetItemSet().Get(XATTR_FILLHATCH))).GetValue();
                    aHatch.SetColor(aShadCol);
                    aF.GetItemSet().Put(XFillHatchItem(String(),aHatch));
                } else {
                    if (eStyle!=XFILL_NONE && eStyle!=XFILL_SOLID) {
                        // also fuer Gradient und Bitmap
                        aF.GetItemSet().Put(XFillStyleItem(XFILL_SOLID));
                    }
                    aF.GetItemSet().Put(XFillColorItem(String(),aShadCol));
                    aF.GetItemSet().Put(XFillTransparenceItem(nTransp));
                }
                rXOut.SetFillAttr(aF);
            }
            return TRUE;
        }
    }
    return FALSE;
}

void SdrAttrObj::BurnInStyleSheetAttributes()
{
    SfxItemPool* pPool=ImpGetItemPool();
    if (pPool!=NULL) {
        SfxItemSet aSet(*pPool,
            SDRATTR_START,SDRATTR_NOTPERSIST_FIRST-1,
            SDRATTR_NOTPERSIST_LAST+1, SDRATTR_END,
            EE_ITEMS_START,EE_ITEMS_END,
            0,0);
        TakeAttributes(aSet,FALSE,FALSE);
        NbcSetAttributes(aSet,FALSE);
    }
}

void SdrAttrObj::CopyAttributes(SdrObject* pDestObj) const
{
    // ...
}

// ItemPool fuer dieses Objekt wechseln
void SdrAttrObj::MigrateItemPool(SfxItemPool* pSrcPool, SfxItemPool* pDestPool)
{
    // call parent
    SdrObject::MigrateItemPool(pSrcPool, pDestPool);

    // eigene Reaktion
    if(pDestPool && pSrcPool)
    {
        UINT16 nItemSetAnz = GetSetItemCount();
        for(UINT16 i=0;i<nItemSetAnz;i++)
        {
            const SfxSetItem* pSI = GetSetItem(i);
            if(pSI != NULL)
            {
                const SfxSetItem* pNewAttr = &(const SfxSetItem&)pDestPool->Put(*pSI);
                pSrcPool->Remove(*pSI);
                SetSetItem(i, pNewAttr);
            }
        }
    }
}

