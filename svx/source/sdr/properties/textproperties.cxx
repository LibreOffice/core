/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textproperties.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:47:30 $
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
#include <svx/svddef.hxx>
#endif

//#ifndef _EEITEM_HXX
//#include <svx/eeitem.hxx>
//#endif

#ifndef _SVDOTEXT_HXX
#include <svx/svdotext.hxx>
#endif

#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif

#ifndef _SVX_WRITINGMODEITEM_HXX
#include <svx/writingmodeitem.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif

#ifndef _SVX_XFLCLIT_HXX
#include <svx/xflclit.hxx>
#endif


#ifndef _SVX_ADJITEM_HXX
#include <svx/adjitem.hxx>
#endif

#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif

#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif

#ifndef _SVX_FLDITEM_HXX
#include <svx/flditem.hxx>
#endif

// #i25616#
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
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
                SDRATTR_START, SDRATTR_SHADOW_LAST,
                SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
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


            sal_Int32 nText = rObj.getTextCount();

            while( --nText >= 0 )
            {
                SdrText* pText = rObj.getText( nText );

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

                    sal_uInt32 nParaCount(pOutliner->GetParagraphCount());

                    for(sal_uInt32 nPara(0L); nPara < nParaCount; nPara++)
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

                        OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
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

                sal_Int32 nCount = rObj.getTextCount();
                while( nCount-- )
                {
                    SdrText* pText = rObj.getText( nCount );
                    OutlinerParaObject* pParaObj = pText->GetOutlinerParaObject();
                    rOutliner.SetText(*pParaObj);
                    sal_uInt32 nParaCount(rOutliner.GetParagraphCount());

                    if(nParaCount)
                    {
                        ESelection aSelection( 0, 0, EE_PARA_ALL, EE_PARA_ALL);
                        rOutliner.RemoveAttribs(aSelection, sal_True, 0);

                        OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, (sal_uInt16)nParaCount);
                        rOutliner.Clear();

                        rObj.NbcSetOutlinerParaObjectForText( pTemp, pText );
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

            // call parent
            AttributeProperties::SetStyleSheet(pNewStyleSheet, bDontRemoveHardAttr);

            if( rObj.GetModel() && !rObj.IsTextEditActive() && !rObj.IsLinkedText() )
            {
                SdrOutliner& rOutliner = rObj.ImpGetDrawOutliner();

                sal_Int32 nText = rObj.getTextCount();

                while( --nText >= 0 )
                {
                    SdrText* pText = rObj.getText( nText );

                    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;
                    if( !pParaObj )
                        continue;

                    // apply StyleSheet to all paragraphs
                    rOutliner.SetText(*pParaObj);
                    sal_uInt32 nParaCount(rOutliner.GetParagraphCount());

                    if(nParaCount)
                    {
                        for(sal_uInt32 nPara(0L); nPara < nParaCount; nPara++)
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
                                                rOutliner.QuickRemoveCharAttribs((sal_uInt16)nPara, nW);
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

                        OutlinerParaObject* pTemp = rOutliner.CreateParaObject(0, (sal_uInt16)nParaCount);
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
                sal_Int32 nText = rObj.getTextCount();

                while( --nText >= 0 )
                {
                    SdrText* pText = rObj.getText( nText );

                    OutlinerParaObject* pParaObj = pText ? pText->GetOutlinerParaObject() : 0;
                    if( !pParaObj )
                        continue;

                    pOutliner->SetText(*pParaObj);

                    sal_uInt32 nParaCount(pOutliner->GetParagraphCount());

                    if(nParaCount)
                    {
                        sal_Bool bBurnIn(sal_False);

                        for(sal_uInt32 nPara(0L); nPara < nParaCount; nPara++)
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
                                bBurnIn = sal_True; // #i51163# Flag was set wrong
                            }
                        }

                        if(bBurnIn)
                        {
                            OutlinerParaObject* pTemp = pOutliner->CreateParaObject(0, (sal_uInt16)nParaCount);
                            rObj.NbcSetOutlinerParaObjectForText(pTemp,pText);
                        }
                    }

                    pOutliner->Clear();
                }
                delete pOutliner;
            }
        }

        void TextProperties::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
        {
            // call parent
            AttributeProperties::Notify(rBC, rHint);

            SdrTextObj& rObj = (SdrTextObj&)GetSdrObject();
            if(rObj.HasText())
            {
                if(HAS_BASE(SfxStyleSheet, &rBC))
                {
                    SfxSimpleHint* pSimple = PTR_CAST(SfxSimpleHint, &rHint);
                    sal_uInt32 nId(pSimple ? pSimple->GetId() : 0L);

                    if(SFX_HINT_DATACHANGED == nId)
                    {
                        rObj.SetPortionInfoChecked(sal_False);

                        sal_Int32 nText = rObj.getTextCount();
                        while( --nText > 0 )
                        {
                            OutlinerParaObject* pParaObj = rObj.getText(nText )->GetOutlinerParaObject();
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
                    }

                    if(SFX_HINT_DYING == nId)
                    {
                        rObj.SetPortionInfoChecked(sal_False);
                        sal_Int32 nText = rObj.getTextCount();
                        while( --nText > 0 )
                        {
                            OutlinerParaObject* pParaObj = rObj.getText(nText )->GetOutlinerParaObject();
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
                        String aOldName(pExtendedHint->GetOldName());
                        String aNewName(pExtendedHint->GetStyleSheet()->GetName());
                        SfxStyleFamily eFamily = pExtendedHint->GetStyleSheet()->GetFamily();

                        if(!aOldName.Equals(aNewName))
                        {
                            sal_Int32 nText = rObj.getTextCount();
                            while( --nText > 0 )
                            {
                                OutlinerParaObject* pParaObj = rObj.getText(nText )->GetOutlinerParaObject();
                                if( pParaObj )
                                    pParaObj->ChangeStyleSheetName(eFamily, aOldName, aNewName);
                            }
                        }
                    }
                }
            }
        }
    } // end of namespace properties
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
