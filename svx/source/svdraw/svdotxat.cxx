/*************************************************************************
 *
 *  $RCSfile: svdotxat.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2000-10-30 11:11:37 $
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

#include <svtools/style.hxx>
#include "svdotext.hxx"
#include "svditext.hxx"
#include "svdmodel.hxx" // fuer GetMaxObjSize und GetStyleSheetPool
#include "svdoutl.hxx"
#include "svdorect.hxx" // fuer SetDirty bei NbcAdjustTextFrameWidthAndHeight
#include "svdocapt.hxx" // fuer SetDirty bei NbcAdjustTextFrameWidthAndHeight
#include <svdetc.hxx>

#ifndef _MyEDITVIEW_HXX
#include "editview.hxx"
#endif

#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _OUTLOBJ_HXX //autogen
#include <outlobj.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include "outliner.hxx"
#endif

#ifndef _EEITEM_HXX //autogen
#include "eeitem.hxx"
#endif

#ifndef _EDITOBJ_HXX //autogen
#include <editobj.hxx>
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#ifndef _SVX_FWDTITEM_HXX //autogen
#include "fwdtitem.hxx"
#endif

#ifndef _SFXSTYLE_HXX //autogen
#include <svtools/style.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#define ITEMID_LRSPACE          EE_PARA_LRSPACE
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <lrspitem.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#ifndef _SVX_NUMITEM_HXX //autogen
#include <numitem.hxx>
#endif

#ifndef _MyEDITENG_HXX //autogen
#include <editeng.hxx>
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

void __EXPORT SdrTextObj::SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    SdrAttrObj::SFX_NOTIFY(rBC,rBCType,rHint,rHintType);
    if (pOutlinerParaObject!=NULL)
    {
        if (HAS_BASE(SfxStyleSheet, &rBC))
        {
            SfxSimpleHint* pSimple=PTR_CAST(SfxSimpleHint,&rHint);
            ULONG nId=pSimple==NULL ? 0 : pSimple->GetId();
            if (nId==SFX_HINT_DATACHANGED)
            {
                bPortionInfoChecked=FALSE;
                pOutlinerParaObject->ClearPortionInfo();
                SetTextSizeDirty();
                if (bTextFrame && NbcAdjustTextFrameWidthAndHeight())
                {
                    SendRepaintBroadcast();
                }
            }
            if (nId==SFX_HINT_DYING)
            {
                bPortionInfoChecked=FALSE;
                pOutlinerParaObject->ClearPortionInfo();
            }
        }
        else if (HAS_BASE(SfxStyleSheetBasePool, &rBC))
        {
            SfxStyleSheetHintExtended* pExtendedHint = PTR_CAST(SfxStyleSheetHintExtended, &rHint);

            if (pExtendedHint && pExtendedHint->GetHint() == SFX_STYLESHEET_MODIFIED)
            {
                String aOldName(pExtendedHint->GetOldName());
                String aNewName(pExtendedHint->GetStyleSheet()->GetName());
                SfxStyleFamily eFamily = pExtendedHint->GetStyleSheet()->GetFamily();

                if(!aOldName.Equals(aNewName))
                    pOutlinerParaObject->ChangeStyleSheetName(eFamily, aOldName, aNewName);
            }
        }
    }
}

void SdrTextObj::NbcSetStyleSheet(SfxStyleSheet* pNewStyleSheet, FASTBOOL bDontRemoveHardAttr)
{
    SdrAttrObj::NbcSetStyleSheet(pNewStyleSheet,bDontRemoveHardAttr);

    if ( pOutlinerParaObject && !pEdtOutl && !IsLinkedText() )
    {
        // StyleSheet auf alle Absaetze anwenden
        SdrOutliner& rOutliner=ImpGetDrawOutliner();
        rOutliner.SetText(*pOutlinerParaObject);
        USHORT nParaCount=(USHORT)rOutliner.GetParagraphCount();
        if (nParaCount!=0) {
            for (USHORT nPara=0; nPara<nParaCount; nPara++)
            {
                if ( GetStyleSheet() )
                {
                    if( nPara > 0 && eTextKind == OBJ_OUTLINETEXT && GetObjInventor() == SdrInventor )
                    {
                        String aNewStyleSheetName( GetStyleSheet()->GetName() );
                        aNewStyleSheetName.Erase( aNewStyleSheetName.Len()-1, 1 );
                        aNewStyleSheetName += String::CreateFromInt32( rOutliner.GetDepth( nPara ) );
                        SfxStyleSheetBasePool* pStylePool = pModel!=NULL ? pModel->GetStyleSheetPool() : NULL;
                        SfxStyleSheet* pNewStyle = (SfxStyleSheet*) pStylePool->Find( aNewStyleSheetName, GetStyleSheet()->GetFamily() );
                        DBG_ASSERT( pNewStyle, "AutoStyleSheetName - Style not found!" );
                        if ( pNewStyle )
                            rOutliner.SetStyleSheet( nPara, pNewStyle );
                    }
                    else
                        rOutliner.SetStyleSheet( nPara, GetStyleSheet() );
                }
                else
                    rOutliner.SetStyleSheet( nPara, NULL ); // StyleSheet entfernen

                if (!bDontRemoveHardAttr && pNewStyleSheet!=NULL) {
                    // Harte Absatz-Attributierung aller im
                    // StyleSheet vorhandenen Items entfernen
                    // -> Parents beruecksichtigen !!!
                    SfxItemIter aIter(pNewStyleSheet->GetItemSet());
                    const SfxPoolItem* pItem=aIter.FirstItem();
                    while (pItem!=NULL) {
                        if (!IsInvalidItem(pItem)) {
                            USHORT nW=pItem->Which();
                            if (nW>=EE_ITEMS_START && nW<=EE_ITEMS_END) {
                                // gibts noch nicht, baut Malte aber ein:
                                rOutliner.QuickRemoveCharAttribs(nPara,nW);
                            }
                        }
                        pItem=aIter.NextItem();
                    }
                }
            }
            OutlinerParaObject* pTemp=rOutliner.CreateParaObject( 0, nParaCount );
            rOutliner.Clear();
            NbcSetOutlinerParaObject(pTemp);
        }
    }
    if (bTextFrame) {
        NbcAdjustTextFrameWidthAndHeight();
    }
}

//-/void SdrTextObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/    SfxItemSet aAttr( rAttr );
//-/
//-/    BOOL bCreateLRSpaceItems = FALSE;
//-/
//-/    if ( aAttr.GetItemState( EE_PARA_LRSPACE ) == SFX_ITEM_ON )
//-/    {
//-/        // SvxLRSpaceItem hart gesetzt: SvxNumBulletItem anpassen
//-/        SfxItemSet aSet( *aAttr.GetPool(), EE_PARA_LRSPACE, EE_PARA_LRSPACE, 0 );
//-/        TakeAttributes( aSet, TRUE, TRUE);
//-/
//-/        if ( aSet.GetItemState( EE_PARA_LRSPACE ) != SFX_ITEM_ON ||
//-/            ((const SvxLRSpaceItem&) aSet.Get( EE_PARA_LRSPACE )) !=
//-/            ((const SvxLRSpaceItem&) aAttr.Get( EE_PARA_LRSPACE )) )
//-/        {
//-/            SvxNumBulletItem* pNumBullet = NULL;
//-/
//-/            if ( aAttr.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
//-/                pNumBullet = new SvxNumBulletItem( (const SvxNumBulletItem&) aAttr.Get(EE_PARA_NUMBULLET) );
//-/            else
//-/            {
//-/                pNumBullet = new SvxNumBulletItem( (const SvxNumBulletItem&) aSet.Get(EE_PARA_NUMBULLET) );
//-/
//-/                if( eTextKind == OBJ_OUTLINETEXT &&
//-/                    pNumBullet->GetNumRule()->GetNumRuleType() != SVX_RULETYPE_PRESENTATION_NUMBERING )
//-/                {
//-/                    // Das ist das 10er-Item aus den Defaults.
//-/                    // Dieses darf bei Gliederungsobjekten nicht verwendet werden!
//-/                    // Daher wird das Item aus der Vorlage geholt
//-/                    delete pNumBullet;
//-/                    pNumBullet = NULL;
//-/
//-/                    if( GetStyleSheet() &&
//-/                        GetStyleSheet()->GetItemSet().GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
//-/                        pNumBullet = new SvxNumBulletItem( (const SvxNumBulletItem&) GetStyleSheet()->GetItemSet().Get(EE_PARA_NUMBULLET) );
//-/                }
//-/            }
//-/
//-/            if( pNumBullet )
//-/            {
//-/                USHORT nLevel = 0;
//-/                if( pNumBullet->GetNumRule()->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING )
//-/                    nLevel = 1;
//-/
//-/                EditEngine::ImportBulletItem( *pNumBullet, nLevel, NULL,
//-/                                              &(const SvxLRSpaceItem&) aAttr.Get( EE_PARA_LRSPACE ) );
//-/                ( (SfxItemSet&) aAttr).Put( *pNumBullet );
//-/
//-/                delete pNumBullet;
//-/            }
//-/            else
//-/                DBG_ASSERT(FALSE, "SdrTextObj::NbcSetAttributes: SvxNumBulletItem nicht angepasst!");
//-/        }
//-/    }
//-/    else if ( aAttr.GetItemState( EE_PARA_NUMBULLET ) == SFX_ITEM_ON )
//-/    {
//-/        // SvxNumBulletItem hart gesetzt: SvxLRSpaceItem anpassen
//-/        SfxItemSet aSet( *aAttr.GetPool(), EE_PARA_NUMBULLET, EE_PARA_NUMBULLET, 0 );
//-/        TakeAttributes( aSet, TRUE, TRUE);
//-/
//-/        if ( aSet.GetItemState( EE_PARA_NUMBULLET ) != SFX_ITEM_ON ||
//-/            ((const SvxNumBulletItem&) aSet.Get( EE_PARA_NUMBULLET )) !=
//-/            ((const SvxNumBulletItem&) aAttr.Get( EE_PARA_NUMBULLET )) )
//-/        {
//-/            SvxLRSpaceItem aLRSpace( (const SvxLRSpaceItem&) aSet.Get(EE_PARA_LRSPACE) );
//-/            USHORT nLevel = 0;
//-/            if( eTextKind == OBJ_OUTLINETEXT )
//-/                nLevel = 1;
//-/
//-/            if ( Outliner::CreateLRSpaceItem( (const SvxNumBulletItem&) aAttr.Get( EE_PARA_NUMBULLET ), nLevel, aLRSpace ) )
//-/                ( (SfxItemSet&) aAttr).Put( aLRSpace );
//-/
//-/            bCreateLRSpaceItems = TRUE;
//-/        }
//-/    }
//-/
//-/    if ( aAttr.GetItemState( SDRATTR_TEXT_CONTOURFRAME ) == SFX_ITEM_ON )
//-/    {
//-/        // Extra-Repaint wenn das Layout so radikal geaendert wird (#43139#)
//-/        SendRepaintBroadcast();
//-/
//-/        if(IsTextEditActive())
//-/        {
//-/            // Text wird gerade editiert, zusaetzlich redraw auf EditText
//-/            const EditEngine& rEditEng = pEdtOutl->GetEditEngine();
//-/            for(UINT16 a=0;a<rEditEng.GetViewCount();a++)
//-/            {
//-/                EditView* pEdView = rEditEng.GetView(a);
//-/
//-/                if(pEdView)
//-/                {
//-/                    pEdView->Invalidate();
//-/                }
//-/            }
//-/        }
//-/    }
//-/
//-/    SdrAttrObj::NbcSetAttributes(aAttr,bReplaceAll);
//-/    FASTBOOL bGrowChecked=FALSE;
//-/    // Attribute auf den gesamten Text anwenden
//-/    if (pOutlinerParaObject!=NULL)
//-/    {
//-/        if ((pEdtOutl==NULL || bCreateLRSpaceItems) && !IsLinkedText())
//-/        {
//-/            // #46762# Sonderbehandlung fuer bReplaceAll
//-/            if (SearchOutlinerItems(aAttr,bReplaceAll))
//-/            {
//-/                Outliner* pOutliner;
//-/                if(pEdtOutl == NULL)
//-/                {
//-/                    pOutliner = &ImpGetDrawOutliner();
//-/                    pOutliner->SetText(*pOutlinerParaObject);
//-/                }
//-/                else
//-/                {
//-/                    pOutliner = pEdtOutl;
//-/                }
//-/                USHORT nParaCount=(USHORT)pOutliner->GetParagraphCount();
//-/                if (nParaCount!=0)
//-/                {
//-/                    for (USHORT nPara=0; nPara<nParaCount; nPara++)
//-/                    {
//-/                        // Neue Attribute mit alten (bereits vorhandenen) mergen
//-/                        SfxItemSet aTmpSet(pOutliner->GetParaAttribs(nPara));
//-/
//-/                        SvxLRSpaceItem aLRSpace( (const SvxLRSpaceItem&) aTmpSet.Get(EE_PARA_LRSPACE) );
//-/
//-/                        if(pEdtOutl==NULL)
//-/                        {
//-/                            if(bReplaceAll)
//-/                            {
//-/                                // #46762#: bReplaceAll beachten
//-/                                SfxWhichIter aIter(aAttr);
//-/                                USHORT nWh=aIter.FirstWhich();
//-/                                while (nWh!=0)
//-/                                {
//-/                                    if (nWh>=EE_ITEMS_START && nWh<=EE_ITEMS_END)
//-/                                    {
//-/                                        SfxItemState eState=aAttr.GetItemState(nWh,FALSE);
//-/                                        if (eState==SFX_ITEM_DEFAULT) aTmpSet.ClearItem(nWh);
//-/                                    }
//-/                                    nWh=aIter.NextWhich();
//-/                                }
//-/                            }
//-/                            aTmpSet.Put(aAttr,FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
//-/                        }
//-/
//-/                        if(bCreateLRSpaceItems)
//-/                        {
//-/                            const INT16 nLevel = pOutliner->GetDepth( nPara );
//-/
//-/                            if ( Outliner::CreateLRSpaceItem( (const SvxNumBulletItem&) aAttr.Get( EE_PARA_NUMBULLET ), nLevel, aLRSpace ) )
//-/                                aTmpSet.Put( aLRSpace );
//-/                        }
//-/
//-/                        pOutliner->SetParaAttribs(nPara,aTmpSet);
//-/
//-/                        if(pEdtOutl==NULL)
//-/                        {
//-/                            // Fuer alle hinzugekommenen Attribute:
//-/                            // entsprechende CharAttribs entfernen.
//-/                            if (bReplaceAll)
//-/                            {
//-/                                if( aAttr.Count() == 0 && aTmpSet.Count() == 0 )
//-/                                {
//-/                                    // Format -> Standard
//-/                                    pOutliner->QuickRemoveCharAttribs(nPara, 0);
//-/                                }
//-/                                else
//-/                                {
//-/                                    SfxItemIter aIter( aTmpSet );
//-/                                    const SfxPoolItem* pItem = aIter.FirstItem();
//-/                                    while ( pItem )
//-/                                    {
//-/                                        if ( !IsInvalidItem(pItem) )
//-/                                        {
//-/                                            USHORT nW = pItem->Which();
//-/                                            if ( nW >= EE_CHAR_START && nW <= EE_CHAR_END )
//-/                                                pOutliner->QuickRemoveCharAttribs( nPara, nW );
//-/                                        }
//-/
//-/                                        pItem=aIter.NextItem();
//-/                                    }
//-/                                }
//-/                            }
//-/                        }
//-/                    }
//-/                    if(pEdtOutl == NULL)
//-/                    {
//-/                        OutlinerParaObject* pTemp=pOutliner->CreateParaObject( 0, nParaCount );
//-/                        pOutliner->Clear();
//-/                        NbcSetOutlinerParaObject(pTemp);
//-/                    }
//-/                    bGrowChecked=TRUE;
//-/                }
//-/            }
//-/        }
//-/
//-/        // #36989#: AutoGrow auch bei TextEdit
//-/        if (bTextFrame && !bGrowChecked)
//-/        {
//-/            NbcAdjustTextFrameWidthAndHeight();
//-/            bGrowChecked=TRUE;
//-/        }
//-/    }
//-/}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrTextObj::SetItem( const SfxPoolItem& rItem )
{
    const SfxItemSet& rSet = GetItemSet();

    if(rSet.Get(rItem.Which()) != rItem)
    {
        SfxItemSet aNewSet(rSet);
        aNewSet.Put(rItem);
        SdrTextObj::SetItemSet(aNewSet);
    }
    else
        SdrAttrObj::SetItem(rItem);
}

void SdrTextObj::ClearItem( USHORT nWhich )
{
    const SfxItemSet& rSet = GetItemSet();

    if(SFX_ITEM_UNKNOWN != rSet.GetItemState(nWhich))
    {
        SfxItemSet aNewSet(rSet);
        aNewSet.ClearItem(nWhich);
        SdrTextObj::SetItemSet(aNewSet);
    }
    else
        SdrAttrObj::ClearItem(nWhich);
}

void SdrTextObj::SetItemSet( const SfxItemSet& rSet )
{
    ImpCheckItemSetChanges(rSet);
    SdrAttrObj::SetItemSet(rSet);
}

void SdrTextObj::ImpCheckItemSetChanges(const SfxItemSet& rAttr)
{
    BOOL bReplaceAll = TRUE;
    BOOL bCreateLRSpaceItems(FALSE);
    const SfxItemSet& rSet = GetItemSet();

    if(SFX_ITEM_ON == rAttr.GetItemState(EE_PARA_LRSPACE))
    {
        // SvxLRSpaceItem hart gesetzt: SvxNumBulletItem anpassen
        if(rSet.GetItemState(EE_PARA_LRSPACE) != SFX_ITEM_ON
            || ((const SvxLRSpaceItem&)rSet.Get(EE_PARA_LRSPACE))
                != ((const SvxLRSpaceItem&)rAttr.Get(EE_PARA_LRSPACE)))
        {
            SvxNumBulletItem* pNumBullet = NULL;

            if(SFX_ITEM_ON == rAttr.GetItemState(EE_PARA_NUMBULLET))
            {
                pNumBullet = new SvxNumBulletItem((const SvxNumBulletItem&)rAttr.Get(EE_PARA_NUMBULLET));
            }
            else
            {
                pNumBullet = new SvxNumBulletItem((const SvxNumBulletItem&)rSet.Get(EE_PARA_NUMBULLET));

                if(OBJ_OUTLINETEXT == eTextKind
                    && pNumBullet->GetNumRule()->GetNumRuleType() != SVX_RULETYPE_PRESENTATION_NUMBERING)
                {
                    // Das ist das 10er-Item aus den Defaults.
                    // Dieses darf bei Gliederungsobjekten nicht verwendet werden!
                    // Daher wird das Item aus der Vorlage geholt
                    delete pNumBullet;
                    pNumBullet = NULL;

                    if(GetStyleSheet()
                        && SFX_ITEM_ON == GetStyleSheet()->GetItemSet().GetItemState(EE_PARA_NUMBULLET))
                    {
                        pNumBullet = new SvxNumBulletItem((const SvxNumBulletItem&)
                            GetStyleSheet()->GetItemSet().Get(EE_PARA_NUMBULLET));
                    }
                }
            }

            if(pNumBullet)
            {
                sal_uInt16 nLevel(0);

                if(pNumBullet->GetNumRule()->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING)
                    nLevel = 1;

                EditEngine::ImportBulletItem(*pNumBullet, nLevel, NULL,
                    &(const SvxLRSpaceItem&)rAttr.Get(EE_PARA_LRSPACE));

                ((SfxItemSet&)rAttr).Put(*pNumBullet);

                delete pNumBullet;
            }
            else
                DBG_ASSERT(FALSE, "SdrTextObj::NbcSetAttributes: SvxNumBulletItem nicht angepasst!");
        }
    }
    else if(SFX_ITEM_ON == rAttr.GetItemState(EE_PARA_NUMBULLET))
    {
        // SvxNumBulletItem hart gesetzt: SvxLRSpaceItem anpassen
        SfxItemSet aSet(*rAttr.GetPool(),
            EE_PARA_NUMBULLET,  EE_PARA_NUMBULLET,
            0, 0);
        aSet.Put(rSet);

        if(SFX_ITEM_ON != aSet.GetItemState(EE_PARA_NUMBULLET)
            || ((const SvxNumBulletItem&)aSet.Get(EE_PARA_NUMBULLET))
                != ((const SvxNumBulletItem&) rAttr.Get(EE_PARA_NUMBULLET)))
        {
            SvxLRSpaceItem aLRSpace((const SvxLRSpaceItem&)aSet.Get(EE_PARA_LRSPACE));
            sal_uInt16 nLevel(0);

            if(eTextKind == OBJ_OUTLINETEXT)
                nLevel = 1;

            if(Outliner::CreateLRSpaceItem((const SvxNumBulletItem&)rAttr.Get(EE_PARA_NUMBULLET), nLevel, aLRSpace))
                ((SfxItemSet&)rAttr).Put(aLRSpace);

            bCreateLRSpaceItems = TRUE;
        }
    }

    if(rAttr.GetItemState(SDRATTR_TEXT_CONTOURFRAME) == SFX_ITEM_ON)
    {
        // Extra-Repaint wenn das Layout so radikal geaendert wird (#43139#)
        SendRepaintBroadcast();

        if(IsTextEditActive())
        {
            // Text wird gerade editiert, zusaetzlich redraw auf EditText
            const EditEngine& rEditEng = pEdtOutl->GetEditEngine();
            for(sal_uInt16 a(0); a < rEditEng.GetViewCount(); a++)
            {
                EditView* pEdView = rEditEng.GetView(a);
                if(pEdView)
                    pEdView->Invalidate();
            }
        }
    }

    BOOL bGrowChecked(FALSE);
    // Attribute auf den gesamten Text anwenden
    if(pOutlinerParaObject)
    {
        if((!pEdtOutl || bCreateLRSpaceItems) && !IsLinkedText())
        {
            // #46762# Sonderbehandlung fuer bReplaceAll
            if(SearchOutlinerItems(rAttr, bReplaceAll))
            {
                Outliner* pOutliner;

                if(!pEdtOutl)
                {
                    pOutliner = &ImpGetDrawOutliner();
                    pOutliner->SetText(*pOutlinerParaObject);
                }
                else
                {
                    pOutliner = pEdtOutl;
                }

                sal_uInt16 nParaCount((USHORT)pOutliner->GetParagraphCount());

                if(nParaCount)
                {
                    for(sal_uInt16 nPara(0); nPara < nParaCount; nPara++)
                    {
                        // Neue Attribute mit alten (bereits vorhandenen) mergen
                        SfxItemSet aTmpSet(pOutliner->GetParaAttribs(nPara));
                        SvxLRSpaceItem aLRSpace((const SvxLRSpaceItem&)aTmpSet.Get(EE_PARA_LRSPACE));

                        if(!pEdtOutl)
                        {
                            if(bReplaceAll)
                            {
                                // #46762#: bReplaceAll beachten
                                SfxWhichIter aIter(rAttr);
                                sal_uInt16 nWh(aIter.FirstWhich());

                                while(nWh)
                                {
                                    if(nWh >= EE_ITEMS_START && nWh <= EE_ITEMS_END)
                                    {
                                        SfxItemState eState = rAttr.GetItemState(nWh, FALSE);
                                        if(eState == SFX_ITEM_DEFAULT)
                                            aTmpSet.ClearItem(nWh);
                                    }
                                    nWh = aIter.NextWhich();
                                }
                            }
                            aTmpSet.Put(rAttr, FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
                        }

                        if(bCreateLRSpaceItems)
                        {
                            const sal_Int16 nLevel(pOutliner->GetDepth(nPara));

                            if( Outliner::CreateLRSpaceItem((const SvxNumBulletItem&)rAttr.Get(EE_PARA_NUMBULLET), nLevel, aLRSpace))
                                aTmpSet.Put(aLRSpace);
                        }

                        pOutliner->SetParaAttribs(nPara, aTmpSet);

                        if(!pEdtOutl)
                        {
                            // Fuer alle hinzugekommenen Attribute:
                            // entsprechende CharAttribs entfernen.
                            if(bReplaceAll)
                            {
                                if(!rAttr.Count() && !aTmpSet.Count())
                                {
                                    // Format -> Standard
                                    pOutliner->QuickRemoveCharAttribs(nPara, 0);
                                }
                                else
                                {
                                    SfxItemIter aIter(aTmpSet);
                                    const SfxPoolItem* pItem = aIter.FirstItem();

                                    while(pItem)
                                    {
                                        if (!IsInvalidItem(pItem))
                                        {
                                            sal_uInt16 nW(pItem->Which());

                                            if(nW >= EE_CHAR_START && nW <= EE_CHAR_END)
                                                pOutliner->QuickRemoveCharAttribs(nPara, nW);
                                        }
                                        pItem = aIter.NextItem();
                                    }
                                }
                            }
                        }
                    }

                    if(!pEdtOutl)
                    {
                        OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, nParaCount);
                        pOutliner->Clear();
                        NbcSetOutlinerParaObject(pTemp);
                    }

                    bGrowChecked = TRUE;
                }
            }
        }

        // #36989#: AutoGrow auch bei TextEdit
        if(bTextFrame && !bGrowChecked)
        {
            NbcAdjustTextFrameWidthAndHeight();
            bGrowChecked = TRUE;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrTextObj::BurnInStyleSheetAttributes()
{
    if ( pModel && pOutlinerParaObject && !pEdtOutl && !IsLinkedText() )
    {
        Outliner* pOutliner = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, pModel );
        pOutliner->SetText( *pOutlinerParaObject );

        SdrAttrObj::BurnInStyleSheetAttributes();

        USHORT nParaCount = (USHORT) pOutliner->GetParagraphCount();
        if ( nParaCount > 0 )
        {
            for ( USHORT nPara = 0; nPara < nParaCount; nPara++ )
            {
                SfxStyleSheet* pSheet = pOutliner->GetStyleSheet( nPara );

                if( pSheet )
                {
                    SfxItemSet aSet( pSheet->GetItemSet() );
                    aSet.Put( pOutliner->GetParaAttribs( nPara ), FALSE );
                    pOutliner->SetParaAttribs( nPara, aSet );
                }
            }

            OutlinerParaObject* pTemp = pOutliner->CreateParaObject( 0, nParaCount );
            NbcSetOutlinerParaObject( pTemp );
            delete pOutliner;
        }
    }
    else
        SdrAttrObj::BurnInStyleSheetAttributes();
}

FASTBOOL SdrTextObj::AdjustTextFrameWidthAndHeight(Rectangle& rR, FASTBOOL bHgt, FASTBOOL bWdt) const
{
    if (bTextFrame && pModel!=NULL && !rR.IsEmpty()) {
        SdrFitToSizeType eFit=GetFitToSize();
        FASTBOOL bFitToSize=(eFit==SDRTEXTFIT_PROPORTIONAL || eFit==SDRTEXTFIT_ALLLINES);
        FASTBOOL bWdtGrow=bWdt && IsAutoGrowWidth();
        FASTBOOL bHgtGrow=bHgt && IsAutoGrowHeight();
        SdrTextAniKind eAniKind=GetTextAniKind();
        SdrTextAniDirection eAniDir=GetTextAniDirection();
        FASTBOOL bScroll=eAniKind==SDRTEXTANI_SCROLL || eAniKind==SDRTEXTANI_ALTERNATE || eAniKind==SDRTEXTANI_SLIDE;
        FASTBOOL bHScroll=bScroll && (eAniDir==SDRTEXTANI_LEFT || eAniDir==SDRTEXTANI_RIGHT);
        FASTBOOL bVScroll=bScroll && (eAniDir==SDRTEXTANI_UP || eAniDir==SDRTEXTANI_DOWN);
        if (!bFitToSize && (bWdtGrow || bHgtGrow)) {
            Rectangle aR0(rR);
            long nHgt=0,nMinHgt=0,nMaxHgt=0;
            long nWdt=0,nMinWdt=0,nMaxWdt=0;
            Size aSiz(rR.GetSize()); aSiz.Width()--; aSiz.Height()--;
            Size aMaxSiz(100000,100000);
            Size aTmpSiz(pModel->GetMaxObjSize());
            if (aTmpSiz.Width()!=0) aMaxSiz.Width()=aTmpSiz.Width();
            if (aTmpSiz.Height()!=0) aMaxSiz.Height()=aTmpSiz.Height();
            if (bWdtGrow) {
                nMinWdt=GetMinTextFrameWidth();
                nMaxWdt=GetMaxTextFrameWidth();
                if (nMaxWdt==0 || nMaxWdt>aMaxSiz.Width()) nMaxWdt=aMaxSiz.Width();
                if (nMinWdt<=0) nMinWdt=1;
                aSiz.Width()=nMaxWdt;
            }
            if (bHgtGrow) {
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
            if (bHScroll) aSiz.Width()=0x0FFFFFFF; // Laufschrift nicht umbrechen
            if (bVScroll) aSiz.Height()=0x0FFFFFFF;
            if (pEdtOutl!=NULL) {
                pEdtOutl->SetMaxAutoPaperSize(aSiz);
                if (bWdtGrow) {
                    Size aSiz(pEdtOutl->CalcTextSize());
                    nWdt=aSiz.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz.Height()+1; // lieber etwas Tolleranz
                } else {
                    nHgt=pEdtOutl->GetTextHeight()+1; // lieber etwas Tolleranz
                }
            } else {
                Outliner& rOutliner=ImpGetDrawOutliner();
                rOutliner.SetPaperSize(aSiz);
                rOutliner.SetUpdateMode(TRUE);
                // !!! hier sollte ich wohl auch noch mal die Optimierung mit
                // bPortionInfoChecked usw einbauen
                if (pOutlinerParaObject!=NULL) rOutliner.SetText(*pOutlinerParaObject);
                if (bWdtGrow) {
                    Size aSiz(rOutliner.CalcTextSize());
                    nWdt=aSiz.Width()+1; // lieber etwas Tolleranz
                    if (bHgtGrow) nHgt=aSiz.Height()+1; // lieber etwas Tolleranz
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
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetBoundRect();
        SendRepaintBroadcast();
        aRect=aNeuRect;
        SetRectsDirty();
        if (HAS_BASE(SdrRectObj,this)) { // mal wieder 'nen Hack
            ((SdrRectObj*)this)->SetXPolyDirty();
        }
        if (HAS_BASE(SdrCaptionObj,this)) { // mal wieder 'nen Hack
            ((SdrCaptionObj*)this)->ImpRecalcTail();
        }
        SetChanged();
        SendRepaintBroadcast();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
    return bRet;
}

void SdrTextObj::ImpSetTextStyleSheetListeners()
{
    SfxStyleSheetBasePool* pStylePool=pModel!=NULL ? pModel->GetStyleSheetPool() : NULL;
    if (pStylePool!=NULL) {
        Container aStyles(1024,64,64);
        if (pOutlinerParaObject!=NULL) {
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
    if (pOutlinerParaObject!=NULL && xFact.IsValid() && yFact.IsValid()) {
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
//-/            if(mpObjectItemSet)
//-/            {
            // Rahmenattribute
            const SfxItemSet& rSet = GetItemSet();
            const SvxFontWidthItem& rOldWdt=(SvxFontWidthItem&)rSet.Get(EE_CHAR_FONTWIDTH);
            const SvxFontHeightItem& rOldHgt=(SvxFontHeightItem&)rSet.Get(EE_CHAR_FONTHEIGHT);

            // erstmal die alten Werte holen
            long nAbsWdt=rOldWdt.GetWidth();
            long nRelWdt=rOldWdt.GetProp();
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
            SetItem(SvxFontWidthItem((USHORT)nAbsWdt,(USHORT)nRelWdt));
            SetItem(SvxFontHeightItem(nAbsHgt,(USHORT)nRelHgt));
//-/                SdrOutlinerSetItem aNewOI(*pOutlAttr);
//-/                SfxItemSet& rNewSet=aNewOI.GetItemSet();
//-/                rNewSet.Put(SvxFontWidthItem((USHORT)nAbsWdt,(USHORT)nRelWdt));
//-/                rNewSet.Put(SvxFontHeightItem(nAbsHgt,(USHORT)nRelHgt));
//-/                pOutlAttr=(SdrOutlinerSetItem*)ImpSetNewAttr(pOutlAttr,&aNewOI);
//-/            }
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

