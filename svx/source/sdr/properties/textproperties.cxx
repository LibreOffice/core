/*************************************************************************
 *
 *  $RCSfile: textproperties.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:20:05 $
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

#ifndef _SDR_PROPERTIES_TEXTPROPERTIES_HXX
#include <svx/sdr/properties/textproperties.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#ifndef _SVDDEF_HXX
#include <svddef.hxx>
#endif

#ifndef _EEITEM_HXX
#include <eeitem.hxx>
#endif

#ifndef _SVDOTEXT_HXX
#include <svdotext.hxx>
#endif

#ifndef _SVDOUTL_HXX
#include <svdoutl.hxx>
#endif

#ifndef _SVX_WRITINGMODEITEM_HXX
#include <writingmodeitem.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif

#ifndef _OUTLOBJ_HXX
#include <outlobj.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <xflclit.hxx>
#endif

#ifndef _EEITEMID_HXX
#include <eeitemid.hxx>
#endif

#ifndef _SVX_ADJITEM_HXX
#include <adjitem.hxx>
#endif

#ifndef _SVDETC_HXX
#include <svdetc.hxx>
#endif

#ifndef _MyEDITENG_HXX
#include <editeng.hxx>
#endif

#ifndef _SVX_FLDITEM_HXX
#include <flditem.hxx>
#endif

// #i25616#
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <xlnwtit.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        SfxItemSet& TextProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTRSET_SHADOW,
                SDRATTRSET_OUTLINER, SDRATTRSET_MISC,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        TextProperties::TextProperties(SdrObject& rObj)
        :   AttributeProperties(rObj)
        {
        }

        TextProperties::TextProperties(const TextProperties& rProps, SdrObject& rObj)
        :   AttributeProperties(rProps, rObj)
        {
        }

        TextProperties::~TextProperties()
        {
        }

        BaseProperties& TextProperties::Clone(SdrObject& rObj) const
        {
            return *(new TextProperties(*this, rObj));
        }

        void TextProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            if(pParaObj)
            {
                // handle outliner attributes
                GetObjectItemSet();
                Outliner* pOutliner = rObj.GetTextEditOutliner();

                if(!rObj.IsTextEditActive())
                {
                    pOutliner = &rObj.ImpGetDrawOutliner();
                    pOutliner->SetText(*pParaObj);
                }

                sal_uInt32 nParaCount(pOutliner->GetParagraphCount());

                for(sal_uInt32 nPara(0L); nPara < nParaCount; nPara++)
                {
                    SfxItemSet aSet(pOutliner->GetParaAttribs(nPara));
                    aSet.Put(rSet);
                    pOutliner->SetParaAttribs(nPara, aSet);
                }

                if(!rObj.IsTextEditActive())
                {
                    if(nParaCount)
                    {
                        // force ItemSet
                        GetObjectItemSet();

                        SfxItemSet aNewSet(pOutliner->GetParaAttribs(0L));
                        mpItemSet->Put(aNewSet);
                    }

                    OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
                    pOutliner->Clear();

                    rObj.NbcSetOutlinerParaObject(pTemp);
                }
            }

            // Extra-Repaint for radical layout changes (#43139#)
            if(SFX_ITEM_SET == rSet.GetItemState(SDRATTR_TEXT_CONTOURFRAME))
            {
                // Here only repaint wanted
                rObj.ActionChanged();
                //rObj.BroadcastObjectChange();
            }

            // call parent
            AttributeProperties::ItemSetChanged(rSet);
        }

        void TextProperties::ItemChange(const sal_uInt16 nWhich, const SfxPoolItem* pNewItem)
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            // #i25616#
            sal_Int32 nOldLineWidth;

            if(XATTR_LINEWIDTH == nWhich && rObj.DoesSupportTextIndentingOnLineWidthChange())
            {
                nOldLineWidth = ((const XLineWidthItem&)GetItem(XATTR_LINEWIDTH)).GetValue();
            }

            if(pNewItem && (SDRATTR_TEXTDIRECTION == nWhich))
            {
                sal_Bool bVertical(com::sun::star::text::WritingMode_TB_RL == ((SvxWritingModeItem*)pNewItem)->GetValue());

                if(bVertical || pParaObj)
                {
                    rObj.SetVerticalWriting(bVertical);
                }
            }

            // #95501# reset to default
            if(!pNewItem && !nWhich && pParaObj)
            {
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();
                rOutliner.SetText(*pParaObj);
                sal_uInt32 nParaCount(rOutliner.GetParagraphCount());

                if(nParaCount)
                {
                    ESelection aSelection( 0, 0, EE_PARA_ALL, EE_PARA_ALL);
                    rOutliner.RemoveAttribs(aSelection, sal_True, 0);

                    OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, (sal_uInt16)nParaCount);
                    rOutliner.Clear();

                    rObj.NbcSetOutlinerParaObject(pTemp);
                }
            }

            // call parent
            AttributeProperties::ItemChange( nWhich, pNewItem );

            // #i25616#
            if(XATTR_LINEWIDTH == nWhich && rObj.DoesSupportTextIndentingOnLineWidthChange())
            {
                const sal_Int32 nNewLineWidth(((const XLineWidthItem&)GetItem(XATTR_LINEWIDTH)).GetValue());
                const sal_Int32 nDifference((nNewLineWidth - nOldLineWidth) / 2);

                if(nDifference)
                {
                    const sal_Bool bLineVisible(XLINE_NONE != ((const XLineStyleItem&)(GetItem(XATTR_LINESTYLE))).GetValue());

                    if(bLineVisible)
                    {
                        const sal_Int32 nLeftDist(((const SdrTextLeftDistItem&)GetItem(SDRATTR_TEXT_LEFTDIST)).GetValue());
                        const sal_Int32 nRightDist(((const SdrTextRightDistItem&)GetItem(SDRATTR_TEXT_RIGHTDIST)).GetValue());
                        const sal_Int32 nUpperDist(((const SdrTextUpperDistItem&)GetItem(SDRATTR_TEXT_UPPERDIST)).GetValue());
                        const sal_Int32 nLowerDist(((const SdrTextLowerDistItem&)GetItem(SDRATTR_TEXT_LOWERDIST)).GetValue());

                        SetObjectItemDirect(SdrTextLeftDistItem(nLeftDist + nDifference));
                        SetObjectItemDirect(SdrTextRightDistItem(nRightDist + nDifference));
                        SetObjectItemDirect(SdrTextUpperDistItem(nUpperDist + nDifference));
                        SetObjectItemDirect(SdrTextLowerDistItem(nLowerDist + nDifference));
                    }
                }
            }
        }

        void TextProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            // call parent
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            if(pParaObj && !rObj.IsTextEditActive() && !rObj.IsLinkedText() )
            {
                // apply StyleSheet to all paragraphs
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();
                rOutliner.SetText(*pParaObj);
                sal_uInt32 nParaCount(rOutliner.GetParagraphCount());

                if(nParaCount)
                {
                    SfxItemSet* pTempSet;

                    for(sal_uInt32 nPara(0L); nPara < nParaCount; nPara++)
                    {
                        // since setting the stylesheet removes all para attributes
                        if(bDontRemoveHardAttr)
                        {
                            // we need to remember them if we want to keep them
                            pTempSet = new SfxItemSet(rOutliner.GetParaAttribs(nPara));
                        }

                        if(GetStyleSheet())
                        {
                            if((OBJ_OUTLINETEXT == rObj.GetTextKind()) && (SdrInventor == rObj.GetObjInventor()))
                            {
                                String aNewStyleSheetName(GetStyleSheet()->GetName());
                                aNewStyleSheetName.Erase(aNewStyleSheetName.Len() - 1, 1);
                                aNewStyleSheetName += String::CreateFromInt32(rOutliner.GetDepth((sal_uInt16)nPara));

                                SdrModel* pModel = rObj.GetModel();
                                SfxStyleSheetBasePool* pStylePool = (pModel != NULL) ? pModel->GetStyleSheetPool() : 0L;
                                SfxStyleSheet* pNewStyle = (SfxStyleSheet*)pStylePool->Find(aNewStyleSheetName, GetStyleSheet()->GetFamily());
                                DBG_ASSERT( pNewStyle, "AutoStyleSheetName - Style not found!" );

                                if(pNewStyle)
                                {
                                    rOutliner.SetStyleSheet(nPara, pNewStyle);
                                }
                            }
                            else
                            {
                                rOutliner.SetStyleSheet(nPara, GetStyleSheet());
                            }
                        }
                        else
                        {
                            // remove StyleSheet
                            rOutliner.SetStyleSheet(nPara, 0L);
                        }

                        if(bDontRemoveHardAttr)
                        {
                            // restore para attributes
                            rOutliner.SetParaAttribs(nPara, *pTempSet);

                            delete pTempSet;
                        }
                        else
                        {
                            if(pNewStyleSheet)
                            {
                                // remove all hard paragraph attributes
                                // which occur in StyleSheet, take care of
                                // parents (!)
                                SfxItemIter aIter(pNewStyleSheet->GetItemSet());
                                const SfxPoolItem* pItem = aIter.FirstItem();

                                while(pItem)
                                {
                                    if(!IsInvalidItem(pItem))
                                    {
                                        sal_uInt16 nW(pItem->Which());

                                        if(nW >= EE_ITEMS_START && nW <= EE_ITEMS_END)
                                        {
                                            rOutliner.QuickRemoveCharAttribs((sal_uInt16)nPara, nW);
                                        }
                                    }
                                    pItem = aIter.NextItem();
                                }
                            }
                        }
                    }

                    OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, (sal_uInt16)nParaCount);
                    rOutliner.Clear();
                    rObj.NbcSetOutlinerParaObject(pTemp);
                }
            }

            if(rObj.IsTextFrame())
            {
                rObj.NbcAdjustTextFrameWidthAndHeight();
            }
        }

        void TextProperties::PreProcessSave()
        {
            // call parent
            AttributeProperties::PreProcessSave();

            // Prepare OutlinerParaObjects for storing
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            if(pParaObj && rObj.GetModel())
            {
                pParaObj->PrepareStore((SfxStyleSheetPool*)rObj.GetModel()->GetStyleSheetPool());
            }
        }

        void TextProperties::ForceDefaultAttributes()
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            sal_Bool bTextFrame(rObj.IsTextFrame());

            // force ItemSet
            GetObjectItemSet();

            if(bTextFrame)
            {
                mpItemSet->Put(XLineStyleItem(XLINE_NONE));
                mpItemSet->Put(XFillColorItem(String(), Color(COL_WHITE)));
                mpItemSet->Put(XFillStyleItem(XFILL_NONE));
            }
            else
            {
                mpItemSet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));
                mpItemSet->Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
                mpItemSet->Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
            }
        }

        void TextProperties::ForceStyleToHardAttributes(sal_Bool bPseudoSheetsOnly)
        {
            // call parent
            AttributeProperties::ForceStyleToHardAttributes(bPseudoSheetsOnly);

            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            if(rObj.GetModel()
                && pParaObj
                && !rObj.IsTextEditActive()
                && !rObj.IsLinkedText())
            {
                Outliner* pOutliner = SdrMakeOutliner(OUTLINERMODE_OUTLINEOBJECT, rObj.GetModel());
                pOutliner->SetText(*pParaObj);

                sal_uInt32 nParaCount(pOutliner->GetParagraphCount());

                if(nParaCount)
                {
                    sal_Bool bBurnIn(sal_False);

                    for(sal_uInt32 nPara(0L); nPara < nParaCount; nPara++)
                    {
                        SfxStyleSheet* pSheet = pOutliner->GetStyleSheet(nPara);

                        if(pSheet && (!bPseudoSheetsOnly || (SFX_STYLE_FAMILY_PSEUDO == pSheet->GetFamily())))
                        {
                            SfxItemSet aParaSet(pOutliner->GetParaAttribs(nPara));
                            SfxItemSet aSet(*aParaSet.GetPool());
                            aSet.Put(pSheet->GetItemSet());

                            /** the next code handles a special case for paragraphs that contain a
                                url field. The color for URL fields is either the system color for
                                urls or the char color attribute that formats the portion in which the
                                url field is contained.
                                When we set a char color attribute to the paragraphs item set from the
                                styles item set, we would have this char color attribute as an attribute
                                that is spanned over the complete paragraph after xml import due to some
                                problems in the xml import (using a XCursor on import so it does not know
                                the paragraphs and can't set char attributes to paragraphs ).

                                To avoid this, as soon as we try to set a char color attribute from the style
                                we
                                1. check if we have at least one url field in this paragraph
                                2. if we found at least one url field, we span the char color attribute over
                                all portions that are not url fields and remove the char color attribute
                                from the paragraphs item set
                            */

                            sal_Bool bHasURL(sal_False);

                            if(aSet.GetItemState(EE_CHAR_COLOR) == SFX_ITEM_SET)
                            {
                                EditEngine* pEditEngine = const_cast<EditEngine*>(&(pOutliner->GetEditEngine()));
                                EECharAttribArray aAttribs;
                                pEditEngine->GetCharAttribs((sal_uInt16)nPara, aAttribs);
                                sal_uInt16 nAttrib;

                                for(nAttrib = 0; nAttrib < aAttribs.Count(); nAttrib++)
                                {
                                    struct EECharAttrib aAttrib(aAttribs.GetObject(nAttrib));

                                    if(EE_FEATURE_FIELD == aAttrib.pAttr->Which())
                                    {
                                        if(aAttrib.pAttr)
                                        {
                                            SvxFieldItem* pFieldItem = (SvxFieldItem*)aAttrib.pAttr;

                                            if(pFieldItem)
                                            {
                                                const SvxFieldData* pData = pFieldItem->GetField();

                                                if(pData && pData->ISA(SvxURLField))
                                                {
                                                    bHasURL = sal_True;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }

                                if(bHasURL)
                                {
                                    SfxItemSet aColorSet(*aSet.GetPool(), EE_CHAR_COLOR, EE_CHAR_COLOR );
                                    aColorSet.Put(aSet, FALSE);

                                    ESelection aSel((sal_uInt16)nPara, 0);

                                    for(nAttrib = 0; nAttrib < aAttribs.Count(); nAttrib++)
                                    {
                                        struct EECharAttrib aAttrib(aAttribs.GetObject(nAttrib));

                                        if(EE_FEATURE_FIELD == aAttrib.pAttr->Which())
                                        {
                                            aSel.nEndPos = aAttrib.nStart;

                                            if(aSel.nStartPos != aSel.nEndPos)
                                            {
                                                pEditEngine->QuickSetAttribs(aColorSet, aSel);
                                            }

                                            aSel.nStartPos = aAttrib.nEnd;
                                        }
                                    }

                                    aSel.nEndPos = pEditEngine->GetTextLen((sal_uInt16)nPara);

                                    if(aSel.nStartPos != aSel.nEndPos)
                                    {
                                        pEditEngine->QuickSetAttribs( aColorSet, aSel );
                                    }
                                }

                            }

                            aSet.Put(aParaSet, FALSE);

                            if(bHasURL)
                            {
                                aSet.ClearItem(EE_CHAR_COLOR);
                            }

                            pOutliner->SetParaAttribs(nPara, aSet);
                            bBurnIn = sal_False;
                        }
                    }

                    if(bBurnIn)
                    {
                        OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
                        rObj.NbcSetOutlinerParaObject(pTemp);
                    }
                }

                delete pOutliner;
            }
        }

        void TextProperties::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
        {
            // call parent
            AttributeProperties::Notify(rBC, rHint);

            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            OutlinerParaObject* pParaObj = rObj.GetOutlinerParaObject();

            if(pParaObj)
            {
                if(HAS_BASE(SfxStyleSheet, &rBC))
                {
                    SfxSimpleHint* pSimple = PTR_CAST(SfxSimpleHint, &rHint);
                    sal_uInt32 nId(pSimple ? pSimple->GetId() : 0L);

                    if(SFX_HINT_DATACHANGED == nId)
                    {
                        rObj.SetPortionInfoChecked(sal_False);
                        pParaObj->ClearPortionInfo();
                        rObj.SetTextSizeDirty();

                        if(rObj.IsTextFrame() && rObj.NbcAdjustTextFrameWidthAndHeight())
                        {
                            // here only repaint wanted
                            rObj.ActionChanged();
                            //rObj.BroadcastObjectChange();
                        }
                    }

                    if(SFX_HINT_DYING == nId)
                    {
                        rObj.SetPortionInfoChecked(sal_False);
                        pParaObj->ClearPortionInfo();
                    }
                }
                else if(HAS_BASE(SfxStyleSheetBasePool, &rBC))
                {
                    SfxStyleSheetHintExtended* pExtendedHint = PTR_CAST(SfxStyleSheetHintExtended, &rHint);

                    if(pExtendedHint
                        && SFX_STYLESHEET_MODIFIED == pExtendedHint->GetHint())
                    {
                        String aOldName(pExtendedHint->GetOldName());
                        String aNewName(pExtendedHint->GetStyleSheet()->GetName());
                        SfxStyleFamily eFamily = pExtendedHint->GetStyleSheet()->GetFamily();

                        if(!aOldName.Equals(aNewName))
                        {
                            pParaObj->ChangeStyleSheetName(eFamily, aOldName, aNewName);
                        }
                    }
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
