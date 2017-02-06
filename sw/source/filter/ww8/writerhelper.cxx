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

#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <doc.hxx>
#include "writerhelper.hxx"
#include <msfilter.hxx>
#include <com/sun/star/container/XChild.hpp>

#include <algorithm>
#include <functional>
#include <svl/itemiter.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdoole2.hxx>
#include <svx/fmglob.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/tstpitem.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <fmtcntnt.hxx>
#include <swtable.hxx>
#include <frmfmt.hxx>
#include <flypos.hxx>
#include <fmtanchr.hxx>
#include <ndgrf.hxx>
#include <fmtfsize.hxx>
#include <SwStyleNameMapper.hxx>
#include <docary.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <unotools/streamwrap.hxx>
#include <numrule.hxx>
#include <vcl/svapp.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IMark.hxx>

using namespace com::sun::star;

namespace
{
    /*
     Stroustroup forgets copy_if, See C++ Programming language Chp 18, pg 530
    */
    template <typename In , typename Out , typename Pred>
        Out my_copy_if(In first, In last, Out res, Pred p)
    {
        while (first != last)
        {
            if (p(*first))
                *res = *first;
            ++first;
        }
        return res;
    }

    // #i98791# - adjust sorting
    // Utility to sort SwTextFormatColl's by their assigned outline style list level
    class outlinecmp : public
        std::binary_function<const SwTextFormatColl*, const SwTextFormatColl*, bool>
    {
    public:
        bool operator()(const SwTextFormatColl *pA, const SwTextFormatColl *pB) const
        {
            // #i98791#
            bool bResult( false );
            const bool bIsAAssignedToOutlineStyle( pA->IsAssignedToListLevelOfOutlineStyle() );
            const bool bIsBAssignedToOutlineStyle( pB->IsAssignedToListLevelOfOutlineStyle() );
            if ( bIsAAssignedToOutlineStyle != bIsBAssignedToOutlineStyle )
            {
                bResult = bIsBAssignedToOutlineStyle;
            }
            else if ( !bIsAAssignedToOutlineStyle )
            {
                // pA and pB are equal regarding the sorting criteria.
                // Thus return value does not matter.
                bResult = false;
            }
            else
            {
                bResult = pA->GetAssignedOutlineStyleLevel() < pB->GetAssignedOutlineStyleLevel();
            }

            return bResult;
       }
    };

    bool IsValidSlotWhich(sal_uInt16 nSlotId, sal_uInt16 nWhichId)
    {
        return (nSlotId != 0 && nWhichId != 0 && nSlotId != nWhichId);
    }

    /*
     Utility to convert a SwPosFlyFrames into a simple vector of ww8::Frames

     The crucial thing is that a ww8::Frame always has an anchor which
     points to some content in the document. This is a requirement of exporting
     to Word
    */
    ww8::Frames SwPosFlyFramesToFrames(const SwPosFlyFrames &rFlys)
    {
        ww8::Frames aRet;

        for(SwPosFlyFrames::const_iterator aIter(rFlys.begin()); aIter != rFlys.end(); ++aIter)
        {
            const SwFrameFormat &rEntry = (*aIter)->GetFormat();

            if (const SwPosition* pAnchor = rEntry.GetAnchor().GetContentAnchor())
            {
                // the anchor position will be invalidated by SetRedlineFlags
                // so set a dummy position and fix it in UpdateFramePositions
                SwPosition const dummy(SwNodeIndex(
                            const_cast<SwNodes&>(pAnchor->nNode.GetNodes())));
                aRet.push_back(ww8::Frame(rEntry, dummy));
            }
            else
            {
                SwPosition aPos((*aIter)->GetNdIndex());

                if (SwTextNode* pTextNd = aPos.nNode.GetNode().GetTextNode())
                {
                    aPos.nContent.Assign(pTextNd, 0);
                }

                aRet.push_back(ww8::Frame(rEntry, aPos));
            }
        }
        return aRet;
    }

    //Utility to test if a frame is anchored at a given node index
    class anchoredto: public std::unary_function<const ww8::Frame&, bool>
    {
    private:
        sal_uLong mnNode;
    public:
        explicit anchoredto(sal_uLong nNode) : mnNode(nNode) {}
        bool operator()(const ww8::Frame &rFrame) const
        {
            return (mnNode == rFrame.GetPosition().nNode.GetNode().GetIndex());
        }
    };
}

namespace ww8
{
    //For i120928,size conversion before exporting graphic of bullet
    Frame::Frame(const Graphic &rGrf, const SwPosition &rPos)
        : mpFlyFrame(nullptr)
        , maPos(rPos)
        , maSize()
        , maLayoutSize()
        , meWriterType(eBulletGrf)
        , mpStartFrameContent(nullptr)
        , mbIsInline(true)
        , mbForBullet(true)
        , maGrf(rGrf)
    {
        const MapMode aMap100mm( MapUnit::Map100thMM );
        Size    aSize( rGrf.GetPrefSize() );
        if ( MapUnit::MapPixel == rGrf.GetPrefMapMode().GetMapUnit() )
        {
            aSize = Application::GetDefaultDevice()->PixelToLogic(aSize, aMap100mm );
        }
        else
        {
            aSize = OutputDevice::LogicToLogic( aSize,rGrf.GetPrefMapMode(), aMap100mm );
        }
        maSize = aSize;
        maLayoutSize = maSize;
    }

    Frame::Frame(const SwFrameFormat &rFormat, const SwPosition &rPos)
        : mpFlyFrame(&rFormat)
        , maPos(rPos)
        , maSize()
        , maLayoutSize() // #i43447#
        , meWriterType(eTextBox)
        , mpStartFrameContent(nullptr)
        // #i43447# - move to initialization list
        , mbIsInline( (rFormat.GetAnchor().GetAnchorId() == FLY_AS_CHAR) )
        // #i120928# - handle graphic of bullet within existing implementation
        , mbForBullet(false)
        , maGrf()
    {
        switch (rFormat.Which())
        {
            case RES_FLYFRMFMT:
                if (const SwNodeIndex* pIdx = rFormat.GetContent().GetContentIdx())
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    const SwNode &rNd = aIdx.GetNode();
                    // #i43447# - determine layout size
                    {
                        SwRect aLayRect( rFormat.FindLayoutRect() );
                        Rectangle aRect( aLayRect.SVRect() );
                        // The Object is not rendered (e.g. something in unused
                        // header/footer) - thus, get the values from the format.
                        if ( aLayRect.IsEmpty() )
                        {
                            aRect.SetSize( rFormat.GetFrameSize().GetSize() );
                        }
                        maLayoutSize = aRect.GetSize();
                    }
                    switch (rNd.GetNodeType())
                    {
                        case SwNodeType::Grf:
                            meWriterType = eGraphic;
                            maSize = rNd.GetNoTextNode()->GetTwipSize();
                            break;
                        case SwNodeType::Ole:
                            meWriterType = eOle;
                            maSize = rNd.GetNoTextNode()->GetTwipSize();
                            break;
                        default:
                            meWriterType = eTextBox;
                            // #i43447# - Size equals layout size for text boxes
                            maSize = maLayoutSize;
                            break;
                    }
                    mpStartFrameContent = &rNd;
                }
                else
                {
                    OSL_ENSURE(false, "Impossible");
                    meWriterType = eTextBox;
                }
                break;
            default:
                if (const SdrObject* pObj = rFormat.FindRealSdrObject())
                {
                    if (pObj->GetObjInventor() == SdrInventor::FmForm)
                        meWriterType = eFormControl;
                    else
                        meWriterType = eDrawing;
                    maSize = pObj->GetSnapRect().GetSize();
                }
                else
                {
                    OSL_ENSURE(false, "Impossible");
                    meWriterType = eDrawing;
                }
                break;
        }
    }


    void Frame::ForceTreatAsInline()
    {
        mbIsInline = true;
    }
}

namespace sw
{
    namespace hack
    {

        sal_uInt16 TransformWhichBetweenPools(const SfxItemPool &rDestPool,
            const SfxItemPool &rSrcPool, sal_uInt16 nWhich)
        {
            sal_uInt16 nSlotId = rSrcPool.GetSlotId(nWhich);
            if (IsValidSlotWhich(nSlotId, nWhich))
                nWhich = rDestPool.GetWhich(nSlotId);
            else
                nWhich = 0;
            return nWhich;
        }

        sal_uInt16 GetSetWhichFromSwDocWhich(const SfxItemSet &rSet,
            const SwDoc &rDoc, sal_uInt16 nWhich)
        {
            if (RES_WHICHHINT_END < *(rSet.GetRanges()))
            {
                nWhich = TransformWhichBetweenPools(*rSet.GetPool(),
                    rDoc.GetAttrPool(), nWhich);
            }
            return nWhich;
        }

        DrawingOLEAdaptor::DrawingOLEAdaptor(SdrOle2Obj &rObj,
            SfxObjectShell &rPers)
            : mxIPRef(rObj.GetObjRef()), mrPers(rPers),
            mpGraphic( rObj.GetGraphic() )
        {
            rObj.AbandonObject();
        }

        bool DrawingOLEAdaptor::TransferToDoc( OUString &rName )
        {
            OSL_ENSURE(mxIPRef.is(), "Transferring invalid object to doc");
            if (!mxIPRef.is())
                return false;

            uno::Reference < container::XChild > xChild( mxIPRef, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( mrPers.GetModel() );

            bool bSuccess = mrPers.GetEmbeddedObjectContainer().InsertEmbeddedObject( mxIPRef, rName );
            if (bSuccess)
            {
                if ( mpGraphic )
                    ::svt::EmbeddedObjectRef::SetGraphicToContainer( *mpGraphic,
                                                                    mrPers.GetEmbeddedObjectContainer(),
                                                                    rName,
                                                                    OUString() );

                mxIPRef = nullptr;
            }

            return bSuccess;
        }

        DrawingOLEAdaptor::~DrawingOLEAdaptor()
        {
            if (mxIPRef.is())
            {
                OSL_ENSURE( !mrPers.GetEmbeddedObjectContainer().HasEmbeddedObject( mxIPRef ), "Object in adaptor is inserted?!" );
                try
                {
                    uno::Reference < css::util::XCloseable > xClose( mxIPRef, uno::UNO_QUERY );
                    if ( xClose.is() )
                        xClose->close(true);
                }
                catch ( const css::util::CloseVetoException& )
                {
                }

                mxIPRef = nullptr;
            }
        }
    }

    namespace util
    {
        SwTwips MakeSafePositioningValue(SwTwips nIn)
        {
            if (nIn > SHRT_MAX)
                nIn = SHRT_MAX;
            else if (nIn < SHRT_MIN)
                nIn = SHRT_MIN;
            return nIn;
        }

        void SetLayer::SendObjectToHell(SdrObject &rObject) const
        {
            SetObjectLayer(rObject, eHell);
        }

        void SetLayer::SendObjectToHeaven(SdrObject &rObject) const
        {
            SetObjectLayer(rObject, eHeaven);
        }

        void SetLayer::SetObjectLayer(SdrObject &rObject, Layer eLayer) const
        {
            if (SdrInventor::FmForm == rObject.GetObjInventor())
                rObject.SetLayer(mnFormLayer);
            else
            {
                switch (eLayer)
                {
                    case eHeaven:
                        rObject.SetLayer(mnHeavenLayer);
                        break;
                    case eHell:
                        rObject.SetLayer(mnHellLayer);
                        break;
                }
            }
        }

        //SetLayer boilerplate begin
        void SetLayer::Swap(SetLayer& rOther) throw()
        {
            std::swap(mnHeavenLayer, rOther.mnHeavenLayer);
            std::swap(mnHellLayer, rOther.mnHellLayer);
            std::swap(mnFormLayer, rOther.mnFormLayer);
        }

        // #i38889# - by default put objects into the invisible layers.
        SetLayer::SetLayer(const SwDoc &rDoc)
            : mnHeavenLayer(rDoc.getIDocumentDrawModelAccess().GetInvisibleHeavenId()),
              mnHellLayer(rDoc.getIDocumentDrawModelAccess().GetInvisibleHellId()),
              mnFormLayer(rDoc.getIDocumentDrawModelAccess().GetInvisibleControlsId())
        {
        }

        SetLayer::SetLayer(const SetLayer& rOther) throw()
            : mnHeavenLayer(rOther.mnHeavenLayer),
            mnHellLayer(rOther.mnHellLayer),
            mnFormLayer(rOther.mnFormLayer)
        {
        }

        SetLayer& SetLayer::operator=(const SetLayer& rOther) throw()
        {
            SetLayer aTemp(rOther);
            Swap(aTemp);
            return *this;
        }
        //SetLayer boilerplate end

        void GetPoolItems(const SfxItemSet &rSet, ww8::PoolItems &rItems, bool bExportParentItemSet )
        {
            if( bExportParentItemSet )
            {
                sal_uInt16 nTotal = rSet.TotalCount();
                for( sal_uInt16 nItem =0; nItem < nTotal; ++nItem )
                {
                    const SfxPoolItem* pItem = nullptr;
                    if( SfxItemState::SET == rSet.GetItemState( rSet.GetWhichByPos( nItem ), true, &pItem ) )
                    {
                        rItems[pItem->Which()] = pItem;
                    }
                }
            }
            else if( rSet.Count())
            {
                SfxItemIter aIter(rSet);
                if (const SfxPoolItem *pItem = aIter.GetCurItem())
                {
                    do
                        rItems[pItem->Which()] = pItem;
                    while (!aIter.IsAtEnd() && nullptr != (pItem = aIter.NextItem()));
                }
            }
        }

        const SfxPoolItem *SearchPoolItems(const ww8::PoolItems &rItems,
            sal_uInt16 eType)
        {
            ww8::cPoolItemIter aIter = rItems.find(eType);
            if (aIter != rItems.end())
                return aIter->second;
            return nullptr;
        }

        void ClearOverridesFromSet(const SwFormatCharFormat &rFormat, SfxItemSet &rSet)
        {
            if (const SwCharFormat* pCharFormat = rFormat.GetCharFormat())
            {
                if (pCharFormat->GetAttrSet().Count())
                {
                    SfxItemIter aIter(pCharFormat->GetAttrSet());
                    const SfxPoolItem *pItem = aIter.GetCurItem();
                    do
                        rSet.ClearItem(pItem->Which());
                    while (!aIter.IsAtEnd() && nullptr != (pItem = aIter.NextItem()));
                }
            }
        }

        ww8::ParaStyles GetParaStyles(const SwDoc &rDoc)
        {
            ww8::ParaStyles aStyles;
            typedef ww8::ParaStyles::size_type mysizet;

            const SwTextFormatColls *pColls = rDoc.GetTextFormatColls();
            mysizet nCount = pColls ? pColls->size() : 0;
            aStyles.reserve(nCount);
            for (mysizet nI = 0; nI < nCount; ++nI)
                aStyles.push_back((*pColls)[ static_cast< sal_uInt16 >(nI) ]);
            return aStyles;
        }

        SwTextFormatColl* GetParaStyle(SwDoc &rDoc, const OUString& rName)
        {
            // Search first in the Doc-Styles
            SwTextFormatColl* pColl = rDoc.FindTextFormatCollByName(rName);
            if (!pColl)
            {
                // Collection not found, try in Pool ?
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(rName,
                    SwGetPoolIdFromName::TxtColl);
                if (n != SAL_MAX_UINT16)       // found or standard
                    pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(n, false);
            }
            return pColl;
        }

        SwCharFormat* GetCharStyle(SwDoc &rDoc, const OUString& rName)
        {
            SwCharFormat *pFormat = rDoc.FindCharFormatByName(rName);
            if (!pFormat)
            {
                // Collection not found, try in Pool ?
                sal_uInt16 n = SwStyleNameMapper::GetPoolIdFromUIName(rName,
                    SwGetPoolIdFromName::ChrFmt);
                if (n != SAL_MAX_UINT16)       // found or standard
                    pFormat = rDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool(n);
            }
            return pFormat;
        }

        // #i98791# - adjust sorting algorithm
        void SortByAssignedOutlineStyleListLevel(ww8::ParaStyles &rStyles)
        {
            std::sort(rStyles.begin(), rStyles.end(), outlinecmp());
        }

        /*
           Utility to extract FlyFormats from a document, potentially from a
           selection.
           */
        ww8::Frames GetFrames(const SwDoc &rDoc, SwPaM *pPaM /*, bool bAll*/)
        {
            SwPosFlyFrames aFlys(rDoc.GetAllFlyFormats(pPaM, true));
            ww8::Frames aRet(SwPosFlyFramesToFrames(aFlys));
            return aRet;
        }

        void UpdateFramePositions(ww8::Frames & rFrames)
        {
            for (ww8::Frame & rFrame : rFrames)
            {
                SwFormatAnchor const& rAnchor = rFrame.GetFrameFormat().GetAnchor();
                if (SwPosition const*const pAnchor = rAnchor.GetContentAnchor())
                {
                    rFrame.SetPosition(*pAnchor);
                }
                else
                {   // these don't need to be corrected, they're not in redlines
                    assert(FLY_AT_PAGE == rAnchor.GetAnchorId());
                }
            }
        }

        ww8::Frames GetFramesInNode(const ww8::Frames &rFrames, const SwNode &rNode)
        {
            ww8::Frames aRet;
            my_copy_if(rFrames.begin(), rFrames.end(),
                std::back_inserter(aRet), anchoredto(rNode.GetIndex()));
            return aRet;
        }

        const SwNumFormat* GetNumFormatFromSwNumRuleLevel(const SwNumRule &rRule,
            int nLevel)
        {
            if (nLevel < 0 || nLevel >= MAXLEVEL)
            {
                OSL_FAIL("Invalid level");
                return nullptr;
            }
            return &(rRule.Get( static_cast< sal_uInt16 >(nLevel) ));
        }

        const SwNumFormat* GetNumFormatFromTextNode(const SwTextNode &rTextNode)
        {
            const SwNumRule *pRule = nullptr;
            if (
                rTextNode.IsNumbered() && rTextNode.IsCountedInList() &&
                nullptr != (pRule = rTextNode.GetNumRule())
                )
            {
                return GetNumFormatFromSwNumRuleLevel(*pRule,
                    rTextNode.GetActualListLevel());
            }

            OSL_ENSURE(rTextNode.GetDoc(), "No document for node?, suspicious");
            if (!rTextNode.GetDoc())
                return nullptr;

            if (
                rTextNode.IsNumbered() && rTextNode.IsCountedInList() &&
                nullptr != (pRule = rTextNode.GetDoc()->GetOutlineNumRule())
                )
            {
                return GetNumFormatFromSwNumRuleLevel(*pRule,
                    rTextNode.GetActualListLevel());
            }

            return nullptr;
        }

        const SwNumRule* GetNumRuleFromTextNode(const SwTextNode &rTextNode)
        {
            return GetNormalNumRuleFromTextNode(rTextNode);
        }

        const SwNumRule* GetNormalNumRuleFromTextNode(const SwTextNode &rTextNode)
        {
            const SwNumRule *pRule = nullptr;

            if (
                rTextNode.IsNumbered() && rTextNode.IsCountedInList() &&
                nullptr != (pRule = rTextNode.GetNumRule())
               )
            {
                return pRule;
            }
            return nullptr;
        }

        SwNoTextNode *GetNoTextNodeFromSwFrameFormat(const SwFrameFormat &rFormat)
        {
            const SwNodeIndex *pIndex = rFormat.GetContent().GetContentIdx();
            OSL_ENSURE(pIndex, "No NodeIndex in SwFrameFormat ?, suspicious");
            if (!pIndex)
                return nullptr;
            SwNodeIndex aIdx(*pIndex, 1);
            return aIdx.GetNode().GetNoTextNode();
        }

        bool HasPageBreak(const SwNode &rNd)
        {
            const SvxFormatBreakItem *pBreak = nullptr;
            if (rNd.IsTableNode() && rNd.GetTableNode())
            {
                const SwTable& rTable = rNd.GetTableNode()->GetTable();
                const SwFrameFormat* pApply = rTable.GetFrameFormat();
                OSL_ENSURE(pApply, "impossible");
                if (pApply)
                    pBreak = &(ItemGet<SvxFormatBreakItem>(*pApply, RES_BREAK));
            }
            else if (const SwContentNode *pNd = rNd.GetContentNode())
                pBreak = &(ItemGet<SvxFormatBreakItem>(*pNd, RES_BREAK));

            if (pBreak && pBreak->GetBreak() == SvxBreak::PageBefore)
                return true;
            return false;
        }

        tools::Polygon PolygonFromPolyPolygon(const tools::PolyPolygon &rPolyPoly)
        {
            if(1 == rPolyPoly.Count())
            {
                return rPolyPoly[0];
            }
            else
            {
                // This method will now just concatenate the polygons contained
                // in the given PolyPolygon. Anything else which might be thought of
                // for reducing to a single polygon will just need nore power and
                // cannot create more correct results.
                sal_uInt32 nPointCount(0L);
                sal_uInt16 a;

                for(a = 0; a < rPolyPoly.Count(); a++)
                {
                    nPointCount += (sal_uInt32)rPolyPoly[a].GetSize();
                }

                if(nPointCount > 0x0000ffff)
                {
                    OSL_FAIL("PolygonFromPolyPolygon: too many points for a single polygon (!)");
                    nPointCount = 0x0000ffff;
                }

                tools::Polygon aRetval((sal_uInt16)nPointCount);
                sal_uInt32 nAppendIndex(0L);

                for(a = 0; a < rPolyPoly.Count(); a++)
                {
                    const tools::Polygon& rCandidate = rPolyPoly[a];

                    for(sal_uInt16 b(0); nAppendIndex <= nPointCount && b < rCandidate.GetSize(); b++)
                    {
                        aRetval[(sal_uInt16)nAppendIndex++] = rCandidate[b];
                    }
                }

                return aRetval;
            }
        }

        tools::Polygon CorrectWordWrapPolygonForExport(const tools::PolyPolygon& rPolyPoly, const SwNoTextNode* pNd)
        {
            tools::Polygon aPoly(PolygonFromPolyPolygon(rPolyPoly));
            const Size &rOrigSize = pNd->GetGraphic().GetPrefSize();
            Fraction aMapPolyX(ww::nWrap100Percent, rOrigSize.Width());
            Fraction aMapPolyY(ww::nWrap100Percent, rOrigSize.Height());
            aPoly.Scale(aMapPolyX, aMapPolyY);

            /*
             a) stretch right bound by 15twips
             b) shrink bottom bound to where it would have been in word
             c) Move it to the left by 15twips

             See the import for details
            */
            const Size &rSize = pNd->GetTwipSize();
            Fraction aMoveHack(ww::nWrap100Percent, rSize.Width());
            aMoveHack *= Fraction(15, 1);
            long nMove(aMoveHack);

            Fraction aHackX(ww::nWrap100Percent + nMove,
                    ww::nWrap100Percent);
            Fraction aHackY(ww::nWrap100Percent - nMove,
                    ww::nWrap100Percent);
            aPoly.Scale(aHackX, aHackY);

            aPoly.Move(-nMove, 0);
            return aPoly;
        }

        void RedlineStack::open(const SwPosition& rPos, const SfxPoolItem& rAttr)
        {
            OSL_ENSURE(rAttr.Which() == RES_FLTR_REDLINE, "not a redline");
            maStack.push_back(new SwFltStackEntry(rPos,rAttr.Clone()));
        }

        class SameOpenRedlineType :
            public std::unary_function<const SwFltStackEntry*, bool>
        {
        private:
            RedlineType_t meType;
        public:
            explicit SameOpenRedlineType(RedlineType_t eType) : meType(eType) {}
            bool operator()(const SwFltStackEntry *pEntry) const
            {
                const SwFltRedline *pTest = static_cast<const SwFltRedline *>
                    (pEntry->pAttr.get());
                return (pEntry->bOpen && (pTest->eType == meType));
            }
        };

        bool RedlineStack::close(const SwPosition& rPos, RedlineType_t eType)
        {
            //Search from end for same type
            myriter aResult = std::find_if(maStack.rbegin(), maStack.rend(),
                SameOpenRedlineType(eType));
            if (aResult != maStack.rend())
            {
                SwTextNode *const pNode(rPos.nNode.GetNode().GetTextNode());
                sal_Int32 const nIndex(rPos.nContent.GetIndex());
                // HACK to prevent overlap of field-mark and redline,
                // which would destroy field-mark invariants when the redline
                // is hidden: move the redline end one to the left
                if (pNode && nIndex > 0
                    && pNode->GetText()[nIndex - 1] == CH_TXT_ATR_FIELDEND)
                {
                    SwPosition const end(*rPos.nNode.GetNode().GetTextNode(),
                                         nIndex - 1);
                    sw::mark::IFieldmark *const pFieldMark(
                        rPos.GetDoc()->getIDocumentMarkAccess()->getFieldmarkFor(end));
                    assert(pFieldMark);
                    if (pFieldMark->GetMarkPos().nNode.GetIndex() == (*aResult)->m_aMkPos.m_nNode.GetIndex()+1
                        && pFieldMark->GetMarkPos().nContent.GetIndex() < (*aResult)->m_aMkPos.m_nContent)
                    {
                        (*aResult)->SetEndPos(end);
                        return true;
                    }
                }
                (*aResult)->SetEndPos(rPos);
                return true;
            }
            return false;
        }

        void RedlineStack::closeall(const SwPosition& rPos)
        {
            std::for_each(maStack.begin(), maStack.end(), SetEndIfOpen(rPos));
        }

        void RedlineStack::MoveAttrs( const SwPosition& rPos )
        {
            size_t nCnt = maStack.size();
            sal_uLong nPosNd = rPos.nNode.GetIndex();
            sal_Int32 nPosCt = rPos.nContent.GetIndex() - 1;

            for (size_t i=0; i < nCnt; ++i)
            {
                SwFltStackEntry& rEntry = *maStack[i];
                bool const isPoint(rEntry.m_aMkPos == rEntry.m_aPtPos);
                if ((rEntry.m_aMkPos.m_nNode.GetIndex()+1 == nPosNd) &&
                    (nPosCt <= rEntry.m_aMkPos.m_nContent))
                {
                    rEntry.m_aMkPos.m_nContent++;
                    SAL_WARN_IF(rEntry.m_aMkPos.m_nContent > rPos.nNode.GetNodes()[nPosNd]->GetContentNode()->Len(),
                            "sw.ww8", "redline ends after end of line");
                    if (isPoint) // sigh ... important special case...
                    {
                        rEntry.m_aPtPos.m_nContent++;
                        continue;
                    }
                }
                // for the end position, leave it alone if it's *on* the dummy
                // char position, that should remain *before*
                if ((rEntry.m_aPtPos.m_nNode.GetIndex()+1 == nPosNd) &&
                    (nPosCt < rEntry.m_aPtPos.m_nContent))
                {
                    rEntry.m_aPtPos.m_nContent++;
                    SAL_WARN_IF(rEntry.m_aPtPos.m_nContent > rPos.nNode.GetNodes()[nPosNd]->GetContentNode()->Len(),
                            "sw.ww8", "redline ends after end of line");
                }
            }
        }

        void SetInDocAndDelete::operator()(SwFltStackEntry *pEntry)
        {
            SwPaM aRegion(pEntry->m_aMkPos.m_nNode);
            if (
                pEntry->MakeRegion(&mrDoc, aRegion, true) &&
                (*aRegion.GetPoint() != *aRegion.GetMark())
            )
            {
                mrDoc.getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::On | RedlineFlags::ShowInsert |
                                         RedlineFlags::ShowDelete);
                const SwFltRedline *pFltRedline = static_cast<const SwFltRedline*>
                    (pEntry->pAttr.get());

                if (USHRT_MAX != pFltRedline->nAutorNoPrev)
                {
                    SwRedlineData aData(pFltRedline->eTypePrev,
                        pFltRedline->nAutorNoPrev, pFltRedline->aStampPrev, OUString(),
                        nullptr);

                    mrDoc.getIDocumentRedlineAccess().AppendRedline(new SwRangeRedline(aData, aRegion), true);
                }

                SwRedlineData aData(pFltRedline->eType, pFltRedline->nAutorNo,
                        pFltRedline->aStamp, OUString(), nullptr);

                SwRangeRedline *const pNewRedline(new SwRangeRedline(aData, aRegion));
                // the point node may be deleted in AppendRedline, so park
                // the PaM somewhere safe
                aRegion.DeleteMark();
                *aRegion.GetPoint() = SwPosition(SwNodeIndex(mrDoc.GetNodes()));
                mrDoc.getIDocumentRedlineAccess().AppendRedline(pNewRedline, true);
                mrDoc.getIDocumentRedlineAccess().SetRedlineFlags(RedlineFlags::NONE | RedlineFlags::ShowInsert |
                     RedlineFlags::ShowDelete );
            }
            delete pEntry;
        }

        bool CompareRedlines::operator()(const SwFltStackEntry *pOneE,
            const SwFltStackEntry *pTwoE) const
        {
            const SwFltRedline *pOne= static_cast<const SwFltRedline*>
                (pOneE->pAttr.get());
            const SwFltRedline *pTwo= static_cast<const SwFltRedline*>
                (pTwoE->pAttr.get());

            //Return the earlier time, if two have the same time, prioritize
            //inserts over deletes
            if (pOne->aStamp == pTwo->aStamp)
                return (pOne->eType == nsRedlineType_t::REDLINE_INSERT && pTwo->eType != nsRedlineType_t::REDLINE_INSERT);
            else
                return (pOne->aStamp < pTwo->aStamp);
        }

        RedlineStack::~RedlineStack()
        {
            std::sort(maStack.begin(), maStack.end(), CompareRedlines());
            std::for_each(maStack.begin(), maStack.end(), SetInDocAndDelete(mrDoc));
        }

        sal_uInt16 WrtRedlineAuthor::AddName( const OUString& rNm )
        {
            sal_uInt16 nRet;
            typedef std::vector<OUString>::iterator myiter;
            myiter aIter = std::find(maAuthors.begin(), maAuthors.end(), rNm);
            if (aIter != maAuthors.end())
                nRet = static_cast< sal_uInt16 >(aIter - maAuthors.begin());
            else
            {
                nRet = static_cast< sal_uInt16 >(maAuthors.size());
                maAuthors.push_back(rNm);
            }
            return nRet;
        }
    }

    namespace util
    {
        InsertedTableClient::InsertedTableClient(SwTableNode & rNode)
        {
            rNode.Add(this);
        }

        SwTableNode * InsertedTableClient::GetTableNode()
        {
            return dynamic_cast<SwTableNode *> (GetRegisteredInNonConst());
        }

        InsertedTablesManager::InsertedTablesManager(const SwDoc &rDoc)
            : mbHasRoot(rDoc.getIDocumentLayoutAccess().GetCurrentLayout())
        {
        }

        void InsertedTablesManager::DelAndMakeTableFrames()
        {
            if (!mbHasRoot)
                return;
            TableMapIter aEnd = maTables.end();
            for (TableMapIter aIter = maTables.begin(); aIter != aEnd; ++aIter)
            {
                // exitiert schon ein Layout, dann muss an dieser Tabelle die BoxFrames
                // neu erzeugt
                SwTableNode *pTable = aIter->first->GetTableNode();
                OSL_ENSURE(pTable, "Why no expected table");
                if (pTable)
                {
                    SwFrameFormat * pFrameFormat = pTable->GetTable().GetFrameFormat();

                    if (pFrameFormat != nullptr)
                    {
                        SwNodeIndex *pIndex = aIter->second;
                        pTable->DelFrames();
                        pTable->MakeFrames(pIndex);
                    }
                }
            }
        }

        void InsertedTablesManager::InsertTable(SwTableNode &rTableNode, SwPaM &rPaM)
        {
            if (!mbHasRoot)
                return;
            //Associate this tablenode with this after position, replace an //old
            //node association if necessary

            InsertedTableClient * pClient = new InsertedTableClient(rTableNode);

            maTables.insert(TableMap::value_type(pClient, &(rPaM.GetPoint()->nNode)));
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
