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

#include <sal/config.h>

#include <sdr/properties/textproperties.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itemiter.hxx>
#include <svl/hint.hxx>
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

using namespace com::sun::star;

namespace sdr
{
    namespace properties
    {
        std::unique_ptr<SfxItemSet> TextProperties::CreateObjectSpecificItemSet(SfxItemPool& rPool)
        {
            return std::make_unique<SfxItemSet>(rPool,

                // range from SdrAttrObj
                svl::Items<SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                SDRATTR_TEXTDIRECTION, SDRATTR_TEXTDIRECTION,

                // range from SdrTextObj
                EE_ITEMS_START, EE_ITEMS_END>{});
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

        std::unique_ptr<BaseProperties> TextProperties::Clone(SdrObject& rObj) const
        {
            return std::unique_ptr<BaseProperties>(new TextProperties(*this, rObj));
        }

        void TextProperties::ItemSetChanged(const SfxItemSet& rSet)
        {
            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());

            // #i101556# ItemSet has changed -> new version
            maVersion++;

            const svx::ITextProvider& rTextProvider(getTextProvider());
            sal_Int32 nText = rTextProvider.getTextCount();
            while (nText--)
            {
                SdrText* pText = rTextProvider.getText( nText );

                OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : nullptr;

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

                            SfxItemSet aNewSet(pOutliner->GetParaAttribs(0));
                            mpItemSet->Put(aNewSet);
                        }

                        std::unique_ptr<OutlinerParaObject> pTemp = pOutliner->CreateParaObject(0, nParaCount);
                        pOutliner->Clear();

                        rObj.NbcSetOutlinerParaObjectForText(std::move(pTemp),pText);
                    }
                }
            }

            // Extra-Repaint for radical layout changes (#43139#)
            if(SfxItemState::SET == rSet.GetItemState(SDRATTR_TEXT_CONTOURFRAME))
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
            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());

            // #i25616#
            sal_Int32 nOldLineWidth(0);

            if(XATTR_LINEWIDTH == nWhich && rObj.DoesSupportTextIndentingOnLineWidthChange())
            {
                nOldLineWidth = GetItem(XATTR_LINEWIDTH).GetValue();
            }

            if(pNewItem && (SDRATTR_TEXTDIRECTION == nWhich))
            {
                bool bVertical(css::text::WritingMode_TB_RL == static_cast<const SvxWritingModeItem*>(pNewItem)->GetValue());
                rObj.SetVerticalWriting(bVertical);
            }

            // #95501# reset to default
            if(!pNewItem && !nWhich && rObj.HasText() )
            {
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();

                const svx::ITextProvider& rTextProvider(getTextProvider());
                sal_Int32 nCount = rTextProvider.getTextCount();
                while (nCount--)
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
                            rOutliner.RemoveAttribs(aSelection, true, 0);

                            std::unique_ptr<OutlinerParaObject> pTemp = rOutliner.CreateParaObject(0, nParaCount);
                            rOutliner.Clear();

                            rObj.NbcSetOutlinerParaObjectForText( std::move(pTemp), pText );
                        }
                    }
                }
            }

            // call parent
            AttributeProperties::ItemChange( nWhich, pNewItem );

            // #i25616#
            if(XATTR_LINEWIDTH == nWhich && rObj.DoesSupportTextIndentingOnLineWidthChange())
            {
                const sal_Int32 nNewLineWidth(GetItem(XATTR_LINEWIDTH).GetValue());
                const sal_Int32 nDifference((nNewLineWidth - nOldLineWidth) / 2);

                if(nDifference)
                {
                    const bool bLineVisible(drawing::LineStyle_NONE != GetItem(XATTR_LINESTYLE).GetValue());

                    if(bLineVisible)
                    {
                        const sal_Int32 nLeftDist(GetItem(SDRATTR_TEXT_LEFTDIST).GetValue());
                        const sal_Int32 nRightDist(GetItem(SDRATTR_TEXT_RIGHTDIST).GetValue());
                        const sal_Int32 nUpperDist(GetItem(SDRATTR_TEXT_UPPERDIST).GetValue());
                        const sal_Int32 nLowerDist(GetItem(SDRATTR_TEXT_LOWERDIST).GetValue());

                        SetObjectItemDirect(makeSdrTextLeftDistItem(nLeftDist + nDifference));
                        SetObjectItemDirect(makeSdrTextRightDistItem(nRightDist + nDifference));
                        SetObjectItemDirect(makeSdrTextUpperDistItem(nUpperDist + nDifference));
                        SetObjectItemDirect(makeSdrTextLowerDistItem(nLowerDist + nDifference));
                    }
                }
            }
        }

        const svx::ITextProvider& TextProperties::getTextProvider() const
        {
            return static_cast<const SdrTextObj&>(GetSdrObject());
        }

        void TextProperties::SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr)
        {
            // call parent (always first thing to do, may create the SfxItemSet)
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            // #i101556# StyleSheet has changed -> new version
            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());
            maVersion++;

            if(!rObj.IsLinkedText() )
            {
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();

                const svx::ITextProvider& rTextProvider(getTextProvider());
                sal_Int32 nText = rTextProvider.getTextCount();
                while (nText--)
                {
                    SdrText* pText = rTextProvider.getText( nText );

                    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : nullptr;
                    if( !pParaObj )
                        continue;

                    // apply StyleSheet to all paragraphs
                    rOutliner.SetText(*pParaObj);
                    sal_Int32 nParaCount(rOutliner.GetParagraphCount());

                    if(nParaCount)
                    {
                        for(sal_Int32 nPara = 0; nPara < nParaCount; nPara++)
                        {
                            std::unique_ptr<SfxItemSet> pTempSet;

                            // since setting the stylesheet removes all para attributes
                            if(bDontRemoveHardAttr)
                            {
                                // we need to remember them if we want to keep them
                                pTempSet.reset(new SfxItemSet(rOutliner.GetParaAttribs(nPara)));
                            }

                            if(GetStyleSheet())
                            {
                                if((OBJ_OUTLINETEXT == rObj.GetTextKind()) && (SdrInventor::Default == rObj.GetObjInventor()))
                                {
                                    OUString aNewStyleSheetName(GetStyleSheet()->GetName());
                                    aNewStyleSheetName = aNewStyleSheetName.copy(0, aNewStyleSheetName.getLength() - 1);
                                    sal_Int16 nDepth = rOutliner.GetDepth(nPara);
                                    aNewStyleSheetName += OUString::number( nDepth <= 0 ? 1 : nDepth + 1);
                                    SfxStyleSheetBasePool* pStylePool(rObj.getSdrModelFromSdrObject().GetStyleSheetPool());
                                    SfxStyleSheet* pNewStyle = nullptr;
                                    if(pStylePool)
                                        pNewStyle = static_cast<SfxStyleSheet*>(pStylePool->Find(aNewStyleSheetName, GetStyleSheet()->GetFamily()));
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
                                rOutliner.SetStyleSheet(nPara, nullptr);
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
                                                rOutliner.RemoveCharAttribs(nPara, nW);
                                            }
                                        }
                                        pItem = aIter.NextItem();
                                    }
                                }
                            }
                        }

                        std::unique_ptr<OutlinerParaObject> pTemp = rOutliner.CreateParaObject(0, nParaCount);
                        rOutliner.Clear();
                        rObj.NbcSetOutlinerParaObjectForText(std::move(pTemp), pText);
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
            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());

            if( rObj.GetObjInventor() == SdrInventor::Default )
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
                mpItemSet->Put(XLineStyleItem(drawing::LineStyle_NONE));
                mpItemSet->Put(XFillColorItem(OUString(), COL_WHITE));
                mpItemSet->Put(XFillStyleItem(drawing::FillStyle_NONE));
            }
            else
            {
                mpItemSet->Put(SvxAdjustItem(SvxAdjust::Center, EE_PARA_JUST));
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
            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());

            if(!rObj.IsTextEditActive() && !rObj.IsLinkedText())
            {
                std::unique_ptr<Outliner> pOutliner = SdrMakeOutliner(OutlinerMode::OutlineObject, rObj.getSdrModelFromSdrObject());
                const svx::ITextProvider& rTextProvider(getTextProvider());
                sal_Int32 nText = rTextProvider.getTextCount();
                while (nText--)
                {
                    SdrText* pText = rTextProvider.getText( nText );

                    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : nullptr;
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

                                if(aSet.GetItemState(EE_CHAR_COLOR) == SfxItemState::SET)
                                {
                                    EditEngine* pEditEngine = const_cast<EditEngine*>(&(pOutliner->GetEditEngine()));
                                    std::vector<EECharAttrib> aAttribs;
                                    pEditEngine->GetCharAttribs(nPara, aAttribs);

                                    for(const auto& rAttrib : aAttribs)
                                    {
                                        if(rAttrib.pAttr && EE_FEATURE_FIELD == rAttrib.pAttr->Which())
                                        {
                                            const SvxFieldItem* pFieldItem = static_cast<const SvxFieldItem*>(rAttrib.pAttr);

                                            if(pFieldItem)
                                            {
                                                const SvxFieldData* pData = pFieldItem->GetField();

                                                if(dynamic_cast<const SvxURLField*>( pData))
                                                {
                                                    bHasURL = true;
                                                    break;
                                                }
                                            }
                                        }
                                    }

                                    if(bHasURL)
                                    {
                                        SfxItemSet aColorSet(*aSet.GetPool(), svl::Items<EE_CHAR_COLOR, EE_CHAR_COLOR>{} );
                                        aColorSet.Put(aSet, false);

                                        ESelection aSel(nPara, 0);

                                        for(const auto& rAttrib : aAttribs)
                                        {
                                            if(EE_FEATURE_FIELD == rAttrib.pAttr->Which())
                                            {
                                                aSel.nEndPos = rAttrib.nStart;

                                                if(aSel.nStartPos != aSel.nEndPos)
                                                    pEditEngine->QuickSetAttribs(aColorSet, aSel);

                                                aSel.nStartPos = rAttrib.nEnd;
                                            }
                                        }

                                        aSel.nEndPos = pEditEngine->GetTextLen(nPara);

                                        if(aSel.nStartPos != aSel.nEndPos)
                                        {
                                            pEditEngine->QuickSetAttribs( aColorSet, aSel );
                                        }
                                    }

                                }

                                aSet.Put(aParaSet, false);

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
                            std::unique_ptr<OutlinerParaObject> pTemp = pOutliner->CreateParaObject(0, nParaCount);
                            rObj.NbcSetOutlinerParaObjectForText(std::move(pTemp),pText);
                        }
                    }

                    pOutliner->Clear();
                }
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

            SdrTextObj& rObj = static_cast<SdrTextObj&>(GetSdrObject());
            if(rObj.HasText())
            {
                const svx::ITextProvider& rTextProvider(getTextProvider());
                if(dynamic_cast<const SfxStyleSheet *>(&rBC) != nullptr)
                {
                    SfxHintId nId(rHint.GetId());

                    if(SfxHintId::DataChanged == nId)
                    {
                        sal_Int32 nText = rTextProvider.getTextCount();
                        while (nText--)
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

                    if(SfxHintId::Dying == nId)
                    {
                        sal_Int32 nText = rTextProvider.getTextCount();
                        while (nText--)
                        {
                            OutlinerParaObject* pParaObj = rTextProvider.getText( nText )->GetOutlinerParaObject();
                            if( pParaObj )
                                pParaObj->ClearPortionInfo();
                        }
                    }
                }
                else if(dynamic_cast<const SfxStyleSheetBasePool *>(&rBC) != nullptr)
                {
                    const SfxStyleSheetModifiedHint* pExtendedHint = dynamic_cast<const SfxStyleSheetModifiedHint*>(&rHint);

                    if(pExtendedHint
                        && SfxHintId::StyleSheetModified == pExtendedHint->GetId())
                    {
                        const OUString& aOldName(pExtendedHint->GetOldName());
                        OUString aNewName(pExtendedHint->GetStyleSheet()->GetName());
                        SfxStyleFamily eFamily = pExtendedHint->GetStyleSheet()->GetFamily();

                        if(aOldName != aNewName)
                        {
                            sal_Int32 nText = rTextProvider.getTextCount();
                            while (nText--)
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
