/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/sdr/properties/textproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itemiter.hxx>
#include <svl/smplhint.hxx>
#include <svx/svddef.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/writingmodeitem.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/outlobj.hxx>
#include <svx/xflclit.hxx>
#include <editeng/adjustitem.hxx>
#include <svx/svdetc.hxx>
#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/svdpool.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace properties
    {
        SfxItemSet& TextProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return *(new SfxItemSet(rPool,

                // range from SdrAttrObj
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END,

                // end
                0, 0));
        }

        TextProperties::TextProperties(SdrObject& rObj)
        :   AttributeProperties(rObj),
            maVersion(0)
        {
        }

        TextProperties::TextProperties(const TextProperties& rProps, SdrObject& rObj)
        :   AttributeProperties(rProps, rObj),
            maVersion(rProps.getVersion())
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
            const svx::ITextProvider& rTextProvider(getTextProvider());
            sal_Int32 nText = rTextProvider.getTextCount();

            // #i101556# ItemSet has changed -> new version
            maVersion++;

            while( --nText >= 0 )
            {
                SdrText* pText = rTextProvider.getText( nText );

                OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;

                if(pParaObj)
                {
                    const bool bTextEdit = rObj.IsTextEditActive() && (rObj.getActiveText() == pText);

                    // handle outliner attributes
                    GetObjectItemSet();
                    Outliner* pOutliner = rObj.GetTextEditOutliner();

                    if(!bTextEdit)
                    {
                        pOutliner = &rObj.ImpGetDrawOutliner();
                        pOutliner->SetText(*pParaObj);
                    }

                    sal_Int32 nParaCount(pOutliner->GetParagraphCount());

                    for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
                    {
                        SfxItemSet aSet(pOutliner->GetParaAttribs(nPara));
                        aSet.Put(rSet);
                        pOutliner->SetParaAttribs(nPara, aSet);
                    }

                    if(!bTextEdit)
                    {
                        if(nParaCount)
                        {
                            // force ItemSet
                            GetObjectItemSet();

                            SfxItemSet aNewSet(pOutliner->GetParaAttribs(0L));
                            mpItemSet->Put(aNewSet);
                        }

                        OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, nParaCount);
                        pOutliner->Clear();

                        rObj.NbcSetOutlinerParaObjectForText(pTemp,pText);
                    }
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

            // #i25616#
            sal_Int32 nOldLineWidth(0L);

            if(XATTR_LINEWIDTH == nWhich && rObj.DoesSupportTextIndentingOnLineWidthChange())
            {
                nOldLineWidth = ((const XLineWidthItem&)GetItem(XATTR_LINEWIDTH)).GetValue();
            }

            if(pNewItem && (SDRATTR_TEXTDIRECTION == nWhich))
            {
                sal_Bool bVertical(com::sun::star::text::WritingMode_TB_RL == ((SvxWritingModeItem*)pNewItem)->GetValue());
                rObj.SetVerticalWriting(bVertical);
            }

            // #95501# reset to default
            if(!pNewItem && !nWhich && rObj.HasText() )
            {
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();

                const svx::ITextProvider& rTextProvider(getTextProvider());
                sal_Int32 nCount = rTextProvider.getTextCount();
                while( nCount-- )
                {
                    SdrText* pText = rTextProvider.getText( nCount );
                    OutlinerParaObject* pParaObj = pText->GetOutlinerParaObject();
                    if( pParaObj )
                    {
                        rOutliner.SetText(*pParaObj);
                        sal_Int32 nParaCount(rOutliner.GetParagraphCount());

                        if(nParaCount)
                        {
                            ESelection aSelection( 0, 0, EE_PARA_ALL, EE_TEXTPOS_ALL);
                            rOutliner.RemoveAttribs(aSelection, sal_True, 0);

                            OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, nParaCount);
                            rOutliner.Clear();

                            rObj.NbcSetOutlinerParaObjectForText( pTemp, pText );
                        }
                    }
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
                    const bool bLineVisible(XLINE_NONE != ((const XLineStyleItem&)(GetItem(XATTR_LINESTYLE))).GetValue());

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

        const svx::ITextProvider& TextProperties::getTextProvider() const
        {
            return static_cast<const SdrTextObj&>(GetSdrObject());
        }

        void TextProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, sal_Bool bDontRemoveHardAttr)
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();

            // call parent
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // #i101556# StyleSheet has changed -> new version
            maVersion++;

            if( rObj.GetModel() /*&& !rObj.IsTextEditActive()*/ && !rObj.IsLinkedText() )
            {
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();

                const svx::ITextProvider& rTextProvider(getTextProvider());
                sal_Int32 nText = rTextProvider.getTextCount();

                while( --nText >= 0 )
                {
                    SdrText* pText = rTextProvider.getText( nText );

                    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;
                    if( !pParaObj )
                        continue;

                    // apply StyleSheet to all paragraphs
                    rOutliner.SetText(*pParaObj);
                    sal_Int32 nParaCount(rOutliner.GetParagraphCount());

                    if(nParaCount)
                    {
                        for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
                        {
                            SfxItemSet* pTempSet = 0L;

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
                                    OUString aNewStyleSheetName(GetStyleSheet()->GetName());
                                    aNewStyleSheetName = aNewStyleSheetName.copy(0, aNewStyleSheetName.getLength() - 1);
                                    sal_Int16 nDepth = rOutliner.GetDepth(nPara);
                                    aNewStyleSheetName += OUString::number( nDepth <= 0 ? 1 : nDepth + 1);

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
                                if(pTempSet)
                                {
                                    // restore para attributes
                                    rOutliner.SetParaAttribs(nPara, *pTempSet);
                                }
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
                                                rOutliner.QuickRemoveCharAttribs(nPara, nW);
                                            }
                                        }
                                        pItem = aIter.NextItem();
                                    }
                                }
                            }

                            if(pTempSet)
                            {
                                delete pTempSet;
                            }
                        }

                        OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, nParaCount);
                        rOutliner.Clear();
                        rObj.NbcSetOutlinerParaObjectForText(pTemp, pText);
                    }
                }
            }

            if(rObj.IsTextFrame())
            {
                rObj.NbcAdjustTextFrameWidthAndHeight();
            }
        }

        void TextProperties::ForceDefaultAttributes()
        {
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();

            if( rObj.GetObjInventor() == SdrInventor )
            {
                const sal_uInt16 nSdrObjKind = rObj.GetObjIdentifier();

                if( nSdrObjKind == OBJ_TITLETEXT || nSdrObjKind == OBJ_OUTLINETEXT )
                    return; // no defaults for presentation objects
            }

            bool bTextFrame(rObj.IsTextFrame());

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
                mpItemSet->Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST));
                mpItemSet->Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
                mpItemSet->Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
            }
        }

        void TextProperties::ForceStyleToHardAttributes()
        {
            // #i61284# call parent first to get the hard ObjectItemSet
            AttributeProperties::ForceStyleToHardAttributes();

            // #i61284# push hard ObjectItemSet to OutlinerParaObject attributes
            // using existing functionality
            GetObjectItemSet(); // force ItemSet
            ItemSetChanged(*mpItemSet);

            // now the standard TextProperties stuff
            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();

            if(rObj.GetModel()
                && !rObj.IsTextEditActive()
                && !rObj.IsLinkedText())
            {
                Outliner* pOutliner = SdrMakeOutliner(OUTLINERMODE_OUTLINEOBJECT, rObj.GetModel());
                const svx::ITextProvider& rTextProvider(getTextProvider());
                sal_Int32 nText = rTextProvider.getTextCount();

                while( --nText >= 0 )
                {
                    SdrText* pText = rTextProvider.getText( nText );

                    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;
                    if( !pParaObj )
                        continue;

                    pOutliner->SetText(*pParaObj);

                    sal_Int32 nParaCount(pOutliner->GetParagraphCount());

                    if(nParaCount)
                    {
                        bool bBurnIn(false);

                        for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
                        {
                            SfxStyleSheet* pSheet = pOutliner->GetStyleSheet(nPara);

                            if(pSheet)
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

                                bool bHasURL(false);

                                if(aSet.GetItemState(EE_CHAR_COLOR) == SFX_ITEM_SET)
                                {
                                    EditEngine* pEditEngine = const_cast<EditEngine*>(&(pOutliner->GetEditEngine()));
                                    std::vector<EECharAttrib> aAttribs;
                                    pEditEngine->GetCharAttribs(nPara, aAttribs);

                                    for(std::vector<EECharAttrib>::iterator i = aAttribs.begin(); i < aAttribs.end(); ++i)
                                    {
                                        if(EE_FEATURE_FIELD == i->pAttr->Which())
                                        {
                                            if(i->pAttr)
                                            {
                                                SvxFieldItem* pFieldItem = (SvxFieldItem*)(i->pAttr);

                                                if(pFieldItem)
                                                {
                                                    const SvxFieldData* pData = pFieldItem->GetField();

                                                    if(pData && pData->ISA(SvxURLField))
                                                    {
                                                        bHasURL = true;
                                                        break;
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if(bHasURL)
                                    {
                                        SfxItemSet aColorSet(*aSet.GetPool(), EE_CHAR_COLOR, EE_CHAR_COLOR );
                                        aColorSet.Put(aSet, sal_False);

                                        ESelection aSel(nPara, 0);

                                        for(std::vector<EECharAttrib>::iterator i = aAttribs.begin(); i < aAttribs.end(); ++i)
                                        {
                                            if(EE_FEATURE_FIELD == i->pAttr->Which())
                                            {
                                                aSel.nEndPos = i->nStart;

                                                if(aSel.nStartPos != aSel.nEndPos)
                                                    pEditEngine->QuickSetAttribs(aColorSet, aSel);

                                                aSel.nStartPos = i->nEnd;
                                            }
                                        }

                                        aSel.nEndPos = pEditEngine->GetTextLen(nPara);

                                        if(aSel.nStartPos != aSel.nEndPos)
                                        {
                                            pEditEngine->QuickSetAttribs( aColorSet, aSel );
                                        }
                                    }

                                }

                                aSet.Put(aParaSet, sal_False);

                                if(bHasURL)
                                {
                                    aSet.ClearItem(EE_CHAR_COLOR);
                                }

                                pOutliner->SetParaAttribs(nPara, aSet);
                                bBurnIn = true; // #i51163# Flag was set wrong
                            }
                        }

                        if(bBurnIn)
                        {
                            OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, nParaCount);
                            rObj.NbcSetOutlinerParaObjectForText(pTemp,pText);
                        }
                    }

                    pOutliner->Clear();
                }
                delete pOutliner;
            }
        }

        void TextProperties::SetObjectItemNoBroadcast(const SfxPoolItem& rItem)
        {
            GetObjectItemSet();
            mpItemSet->Put(rItem);
        }


        void TextProperties::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
        {
            // call parent
            AttributeProperties::Notify(rBC, rHint);

            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            if(rObj.HasText())
            {
                const svx::ITextProvider& rTextProvider(getTextProvider());
                if(HAS_BASE(SfxStyleSheet, &rBC))
                {
                    SfxSimpleHint* pSimple = PTR_CAST(SfxSimpleHint, &rHint);
                    sal_uInt32 nId(pSimple ? pSimple->GetId() : 0L);

                    if(SFX_HINT_DATACHANGED == nId)
                    {
                        rObj.SetPortionInfoChecked(sal_False);

                        sal_Int32 nText = rTextProvider.getTextCount();
                        while( --nText > 0 )
                        {
                            OutlinerParaObject* pParaObj = rTextProvider.getText( nText )->GetOutlinerParaObject();
                            if( pParaObj )
                                pParaObj->ClearPortionInfo();
                        }
                        rObj.SetTextSizeDirty();

                        if(rObj.IsTextFrame() && rObj.NbcAdjustTextFrameWidthAndHeight())
                        {
                            // here only repaint wanted
                            rObj.ActionChanged();
                            //rObj.BroadcastObjectChange();
                        }

                        // #i101556# content of StyleSheet has changed -> new version
                        maVersion++;
                    }

                    if(SFX_HINT_DYING == nId)
                    {
                        rObj.SetPortionInfoChecked(sal_False);
                        sal_Int32 nText = rTextProvider.getTextCount();
                        while( --nText > 0 )
                        {
                            OutlinerParaObject* pParaObj = rTextProvider.getText( nText )->GetOutlinerParaObject();
                            if( pParaObj )
                                pParaObj->ClearPortionInfo();
                        }
                    }
                }
                else if(HAS_BASE(SfxStyleSheetBasePool, &rBC))
                {
                    SfxStyleSheetHintExtended* pExtendedHint = PTR_CAST(SfxStyleSheetHintExtended, &rHint);

                    if(pExtendedHint
                        && SFX_STYLESHEET_MODIFIED == pExtendedHint->GetHint())
                    {
                        OUString aOldName(pExtendedHint->GetOldName());
                        OUString aNewName(pExtendedHint->GetStyleSheet()->GetName());
                        SfxStyleFamily eFamily = pExtendedHint->GetStyleSheet()->GetFamily();

                        if(aOldName != aNewName)
                        {
                            sal_Int32 nText = rTextProvider.getTextCount();
                            while( --nText > 0 )
                            {
                                OutlinerParaObject* pParaObj = rTextProvider.getText( nText )->GetOutlinerParaObject();
                                if( pParaObj )
                                    pParaObj->ChangeStyleSheetName(eFamily, aOldName, aNewName);
                            }
                        }
                    }
                }
            }
        }

        // #i101556# Handout version information
        sal_uInt32 TextProperties::getVersion() const
        {
            return maVersion;
        }
    } // end of namespace properties
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
